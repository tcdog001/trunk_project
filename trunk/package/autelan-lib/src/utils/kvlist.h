#ifndef __KVLIST_H_8C5629DA78BBA785C56EC6C3E35088BB
#define __KVLIST_H_8C5629DA78BBA785C56EC6C3E35088BB
/******************************************************************************/
#ifndef KVIFS
#define KVIFS       '='
#endif

#ifndef KVLINELEN
#define KVLINELEN   1023
#endif

struct kventry {
    /* 
    * must NOT include '=' 
    * must NOT include ' ' 
    */
    string_t *k;
    /* 
    * must NOT include '=' 
    * maybe include ' ' 
    */
    string_t *v;

    struct mlist_node node;
};

struct kvcontainer {
    int count;
    struct mlist_head head;
};
#define KVCONTAINER(container) { .head = MLIST_HEAD_INIT(container.head) }

static inline int
kv_insert(struct kvcontainer *container, struct kventry *entry)
{
    int err;
    
    if (NULL==container) {
        return -EINVAL9;
    }
    else if (NULL==entry) {
        return -EINVAL8;
    }
    
    int hash(void)
    {
        return mlist_string_hash_idx(entry->k->string);
    }

    err = mlist_insert(&container->head, &entry->node, hash);
    if (0==err) {
        container->count++;
    }

    return err;
}

static inline int
kv_remove(struct kvcontainer *container, struct kventry *entry)
{
    int err;
    
    if (NULL==container) {
        return -EINVAL9;
    }
    else if (NULL==entry) {
        return -EINVAL8;
    }
    
    err = mlist_remove(&container->head, &entry->node);
    if (0==err) {
        container->count--;
    }
}

static inline struct kventry *
kv_get(struct kvcontainer *container, char *name)
{
    if (NULL==container || NULL==name) {
        return NULL;
    }
    
    int hash(void)
    {
        return mlist_string_hash_idx(name);
    }
    
    int eq(struct mlist_node *node)
    {
        struct kventry *entry = container_of(node, struct kventry, node);
        
        return 0==os_strcmp(entry->k->string, name);
    }
    
    struct mlist_node *node = mlist_get(&container->head, name, hash, eq);

    return node?container_of(node, struct kventry, node):NULL;
}

static inline void
__kv_destroy(struct kventry *entry)
{
    if (entry) {
        os_free(entry->k);
        os_free(entry->v);
        os_free(entry);
    }
}

#define kv_destroy(_entry)  do{ \
    __kv_destroy(_entry);       \
    _entry = NULL;              \
}while(0)

static inline struct kventry *
__kv_create(char *name, char *value)
{
    struct kventry *entry = (struct kventry *)os_zalloc(sizeof(*entry));
    if (NULL==entry) {
        goto error;
    }

    entry->k = string_create(name);
    if (NULL==entry->k) {
        goto error;
    }

    entry->v = string_create(value);
    if (NULL==entry->v) {
        goto error;
    }

    return entry;
error:
    __kv_destroy(entry);

    return NULL;
}

static inline struct kventry *
kv_create(char *name, char *value)
{
    if (NULL==name || NULL==value) {
        return NULL;
    }

    return __kv_create(name, value);
}

static inline int
kv_delete(struct kvcontainer *container, char *name)
{
    struct kventry *entry = kv_get(container, name);
    if (NULL==entry) {
        return -ENOEXIST;
    }

    int err = kv_remove(container, entry);
    if (err<0) {
        return err;
    }

    kv_destroy(entry);

    return 0;
}

static inline int
kv_foreach(struct kvcontainer *container, multi_value_t (*cb)(struct kventry *entry))
{
    struct kventry *entry;
    struct mlist_node *node, *tmp;
    
    list_for_each_entry_safe(node, tmp, &container->head.list, list) {
        multi_value_u mv;
        
        entry = container_of(node, struct kventry, node);
        
        mv.value = (*cb)(entry);
        if (mv2_is_break(mv)) {
            return mv2_result(mv);
        }
    }

    return 0;
}

static inline void 
kv_container_release(struct kvcontainer *container)
{
    multi_value_t release(struct kventry *entry)
    {
        kv_remove(container, entry);
        kv_destroy(entry);

        return mv2_OK;
    }
    
    kv_foreach(container, release);
}

static inline int
__kv_from_line(struct kvcontainer *container, char *line)
{
    char *ifs;

    for (ifs = os_strrchr(line, KVIFS); ifs; ifs = os_strrchr(line, KVIFS)) {
        *ifs = 0;

        char *space = os_strrchr(line, ' ');
        if (NULL==space) {
            kv_insert(container, kv_create(line, ifs + 1));
            
            break;
        } else {
            *space = 0;

            kv_insert(container, kv_create(space + 1, ifs + 1));
        }
    }

    return 0;
}

static inline int
kv_from_line(struct kvcontainer *container, char *line)
{
    if (NULL==container) {
        return -EINVAL9;
    }
    else if (NULL==line) {
        return -EINVAL8;
    }
    else if (os_strlen(line) > KVLINELEN) {
        return -ETOOBIG;
    }
    
    string_t *s = string_create(line);
    int err = __kv_from_line(container, s->string);
    os_free(s);

    return err;
}

static inline int
__kv_from_argv(struct kvcontainer *container, int argc, char *argv[])
{
    int i;
    int err;
    
    for (i=0; i<argc; i++) {
        char buf[1+KVLINELEN] = {0};
        char name[1+KVLINELEN] = {0};
        char value[1+KVLINELEN]= {0};
        
        if (NULL==argv[i] || 0==argv[i][0]) {
            err = -EINVAL7;

            goto error;
        }

        os_strdcpy(buf, argv[i]);
        __string_strim(buf, NULL);
        
        if (2!=os_sscanf(buf, "%s=%s", name, value)) {
            err = -EINVAL9;
            
            goto error;
        }
        
        kv_insert(container, kv_create(name, value));
    }

    return 0;
error:
    kv_container_release(container);

    return err;
}

static inline int
kv_from_argv(struct kvcontainer *container, int argc, char *argv[])
{
    if (NULL==container) {
        return -EINVAL9;
    }
    else if (0==argc) {
        return -EINVAL8;
    }

    return __kv_from_argv(container, argc, argv);
}

static inline int
__kv_to_line(struct kvcontainer *container, char *line, int size)
{
    multi_value_t kv_to_line_cb(struct kventry *entry)
    {
        int len = os_strlen(line);

        os_snprintf(line + len, size - len, 
            "%s=%s", 
            entry->k->string, 
            entry->v->string);

        return mv2_OK;
    }
    
    return kv_foreach(container, kv_to_line_cb);
}

static inline int
kv_to_line(struct kvcontainer *container, char *line, int size)
{
    if (NULL==container) {
        return -EINVAL9;
    }
    else if (0==line) {
        return -EINVAL8;
    }
    else if (size<=0) {
        return -EINVAL7;
    }

    return __kv_to_line(container, line, size);
}
/******************************************************************************/
#endif /* __KVLIST_H_8C5629DA78BBA785C56EC6C3E35088BB */

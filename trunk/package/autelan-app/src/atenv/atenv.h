#ifndef __ATENV_H_31502FF802B81D17AEDABEB5EDB5C121__
#define __ATENV_H_31502FF802B81D17AEDABEB5EDB5C121__
#include "utils.h"
/******************************************************************************/
#ifndef AT_ENV_SIZE
#define AT_ENV_SIZE         4096
#endif

#ifndef AT_ENV_OFFSET
#define AT_ENV_OFFSET       (512*1024) /* boot size */
#endif

#define AT_BLOCK_SIZE       512
#define AT_BLOCK_COUNT      (AT_ENV_SIZE/AT_BLOCK_SIZE) /* 8 */

#define AT_TRYS             3

#ifndef AT_VENDOR
#define AT_VENDOR           "Autelan"
#endif

#ifndef AT_COMPANY
#define AT_COMPANY          "Beijing Autelan Technology Co.,Ltd."
#endif

#ifndef AT_PRODUCT_MODEL
#define AT_PRODUCT_MODEL    "AQ2000-LV1"
#endif

#ifndef AT_PCBA_MODEL
#define AT_PCBA_MODEL       ""
#endif

#ifndef AT_PCBA_VERSION
#define AT_PCBA_VERSION     ""
#endif

#define AT_OS_COUNT         7
#define AT_MARK_COUNT        (AT_BLOCK_SIZE/sizeof(unsigned int)) /* 128 */
#define AT_VAR_SIZE         64
#define AT_VAR_COUNT        ((AT_BLOCK_COUNT-3)*AT_BLOCK_SIZE/AT_VAR_SIZE) /* 5*512/64=40 */

/*
* ok > unknow > fail
*/
enum {
    AT_FSM_FAIL,
    AT_FSM_UNKNOW,
    AT_FSM_OK,

    AT_FSM_END
};
#define AT_FSM_INVALID  AT_FSM_END

#define AT_FSM_STRINGS      {   \
    [AT_FSM_FAIL]   = "fail",   \
    [AT_FSM_UNKNOW] = "unknow", \
    [AT_FSM_OK]     = "ok",     \
}   /* end */

static inline bool
is_good_at_fsm(int fsm)
{
    return is_good_enum(fsm, AT_FSM_END);
}

static inline bool
is_canused_at_fsm(int fsm)
{
    return is_good_value(fsm, AT_FSM_UNKNOW, AT_FSM_END);
}

static inline char **
__at_fsm_strings(void)
{
    static char *array[AT_FSM_END] = AT_FSM_STRINGS;

    return array;
}

static inline char *
at_fsm_string(int fsm)
{
    char **array = __at_fsm_strings();

    return is_good_at_fsm(fsm)?array[fsm]:__unknow;
}

static inline int
at_fsm_idx(char *fsm)
{
    char **array = __at_fsm_strings();

    return os_getstringarrayidx(array, fsm, 0, AT_FSM_END);
}

static inline int
at_fsm_cmp(int a, int b)
{
    return a - b;
}

#define at_obj_deft(_obj, _deft)    do{ \
    typeof(*_obj) new = _deft;          \
    os_objcpy(_obj, &new);              \
}while(0)

#define AT_COOKIE_SIZE      (0  \
    + sizeof(AT_VENDOR)         \
    + sizeof(AT_COMPANY)        \
    + sizeof(AT_PCBA_MODEL)     \
    + sizeof(AT_PCBA_VERSION)   \
)   /* end */

typedef struct {
    struct {
        char vendor[sizeof(AT_VENDOR)];
        char company[sizeof(AT_COMPANY)];
    } product;
    
    struct {
        char model[sizeof(AT_PCBA_MODEL)];
        char version[sizeof(AT_PCBA_VERSION)];
    } pcba;
    
    char reserved[AT_BLOCK_SIZE - AT_COOKIE_SIZE];
} at_cookie_t;   /* 512 */

#define AT_DEFT_COOKIE              {   \
    .product        = {                 \
        .vendor     = AT_VENDOR,        \
        .company    = AT_COMPANY,       \
    },                                  \
    .pcba           = {                 \
        .model      = AT_PCBA_MODEL,    \
        .version    = AT_PCBA_VERSION,  \
    },                                  \
    .reserved       = {0},              \
}   /* end */

typedef struct { 
    unsigned int number[4]; 
} at_version_t;

#define AT_MIN_VERSION_NUMBER   0
#define AT_MAX_VERSION_NUMBER   9999
#define AT_MIN_VERSION_STRING   "0.0.0.0"
#define AT_MAX_VERSION_STRING   "9999.9999.9999.9999"
#define AT_MIN_VERSION          {   \
    .number = {                     \
        AT_MIN_VERSION_NUMBER,      \
        AT_MIN_VERSION_NUMBER,      \
        AT_MIN_VERSION_NUMBER,      \
        AT_MIN_VERSION_NUMBER,      \
    },                              \
}   /* end */

#define AT_MAX_VERSION          {   \
    .number = {                     \
        AT_MAX_VERSION_NUMBER,      \
        AT_MAX_VERSION_NUMBER,      \
        AT_MAX_VERSION_NUMBER,      \
        AT_MAX_VERSION_NUMBER,      \
    },                              \
}   /* end */

#define AT_INVALID_VERSION      AT_MIN_VERSION
#define AT_DEFT_VERSION         AT_MIN_VERSION

#define AT_VERSION_STRING_SIZE  sizeof(AT_MAX_VERSION_STRING) /* 20 */

static inline bool
is_good_at_version(at_version_t *version)
{
    return version->number[0] >= AT_MIN_VERSION_NUMBER
        && version->number[0] <= AT_MAX_VERSION_NUMBER
        && version->number[1] >= AT_MIN_VERSION_NUMBER
        && version->number[1] <= AT_MAX_VERSION_NUMBER
        && version->number[2] >= AT_MIN_VERSION_NUMBER
        && version->number[2] <= AT_MAX_VERSION_NUMBER
        && version->number[3] >= AT_MIN_VERSION_NUMBER
        && version->number[3] <= AT_MAX_VERSION_NUMBER;
}

static inline char *
__at_version_itoa(at_version_t *version, char string[])
{
    os_sprintf(string, "%u.%u.%u.%u", 
        version->number[0],
        version->number[1],
        version->number[2],
        version->number[3]);

    return string;
}

static inline char *
at_version_itoa(at_version_t *version)
{
    static char string[AT_VERSION_STRING_SIZE];

    return __at_version_itoa(version, string);
}

static inline int
at_version_atoi(at_version_t *version, char *string)
{
    char line[1+OS_LINE_LEN] = {0};
    char *number[4] = {line, NULL, NULL, NULL};
    int i;

    os_strcpy(line, string);
    
    for (i=1; i<4; i++) {
        number[i] = os_strchr(number[i-1], '.');
        if (NULL==number[i]) {
            return -EFORMAT;
        }
        *number[i]++ = 0;
    }
    
    for (i=0; i<4; i++) {
        char *end = NULL;
        
        version->number[i] = os_strtoul(number[i], &end, 0);
        if (end) {
            return -EFORMAT;
        }
    }
    
    return 0;
}

static inline int
at_version_cmp(at_version_t *a, at_version_t *b)
{
    return __os_objcmp(a, b, is_good_at_version, os_objcmp);
}

typedef struct {
    unsigned int self;
    unsigned int other;
    unsigned int upgrade;
    unsigned int error;
    
    at_version_t version;
} at_vcs_t; /* 32 */

#define __AT_VCS(_self, _other, _upgrade_, _error, _version)   { \
    .self       = _self,        \
    .other      = _other,       \
    .upgrade    = _upgrade_,    \
    .error      = _error,       \
    .version    = _version,     \
}   /* end */

#define AT_DEFT_VCS \
        __AT_VCS(AT_FSM_UNKNOW, AT_FSM_UNKNOW, AT_FSM_OK, 0, AT_DEFT_VERSION)
#define AT_MIN_VCS \
        __AT_VCS(AT_FSM_FAIL, AT_FSM_FAIL, AT_FSM_FAIL, AT_TRYS, AT_MIN_VERSION)
#define AT_MAX_VCS \
        __AT_VCS(AT_FSM_OK, AT_FSM_OK, AT_FSM_OK, 0, AT_MAX_VERSION)

#define is_good_at_error(_error)    ((_error) < AT_TRYS)

static inline int
at_error_cmp(unsigned int a, unsigned b)
{
    return __os_objcmp(a, b, is_good_at_error, os_cmp_allways_eq);
}

static inline bool
at_vcs_is_good(at_vcs_t *vcs)
{
    return is_good_at_error(vcs->error)
        && AT_FSM_OK==vcs->upgrade
        && is_canused_at_fsm(vcs->self);
}

static inline bool
at_vcs_match(at_vcs_t *vcs, at_vcs_t *filter)
{
    at_version_t invalid = AT_INVALID_VERSION;
    
    if (NULL==filter) {
        return true;
    }
    else if (is_good_at_error(filter->error) != is_good_at_error(vcs->error)) {
        return false;
    }
    else if (OS_INVALID!=filter->self && filter->self!=vcs->self) {
        return false;
    }
    else if (OS_INVALID!=filter->other && filter->other!=vcs->other) {
        return false;
    }
    else if (OS_INVALID!=filter->upgrade && filter->upgrade!=vcs->upgrade) {
        return false;
    }
    else if (false==os_objeq(&invalid, &filter->version) 
        && false==os_objeq(&vcs->version, &filter->version)) {
        return false;
    }
    else {
        return true;
    }
}

typedef struct {
    unsigned int current;
    
    at_vcs_t vcs[AT_OS_COUNT];
} at_firmware_t; /* 4 + 7*32 = 228 */

#define AT_DEFT_FIRMWARE  { \
    .current = 1,           \
    .vcs = {                \
        [0 ... (AT_OS_COUNT-1)] = AT_DEFT_VCS,  \
    },                      \
}   /* end */

#define AT_OS_RESERVED      \
    (AT_BLOCK_SIZE - 2*sizeof(at_firmware_t)) /* 512 - 2*228 = 56 */

typedef struct {
    at_firmware_t kernel, rootfs;

    unsigned char reserved[AT_OS_RESERVED];
} at_os_t; /* 512 */

#define AT_DEFT_OS    {   \
    .kernel     = AT_DEFT_FIRMWARE, \
    .rootfs     = AT_DEFT_FIRMWARE, \
    .reserved   = {0},              \
}   /* end */

typedef struct {
    unsigned int key[AT_MARK_COUNT];
} at_mark_t; /* 512 */

#define AT_DEFT_MARK { .key = {0} }

typedef struct {
    char var[AT_VAR_COUNT][AT_VAR_SIZE];
} at_info_t;

#define AT_DEFT_INFO { .var = {{0}} }

typedef struct {
    at_cookie_t     cookie;
    at_os_t         os;
    at_mark_t       mark;
    at_info_t       info;
} at_env_t;

#define AT_DEFT_ENV         {   \
    .cookie = AT_DEFT_COOKIE,   \
    .os     = AT_DEFT_OS,       \
    .mark   = AT_DEFT_MARK,     \
    .info   = AT_DEFT_INFO,     \
}   /* end */

typedef struct struct_at_ops at_ops_t;
struct struct_at_ops {
    char *path;
    unsigned int offset;
    unsigned int flag;
    
    int  (*check)(at_ops_t *ops, char *value);
    void (*write)(at_ops_t *ops, char *value);
    void (*show)(at_ops_t *ops);
};

typedef struct {
    char *value;    /* for write */
    bool showit;    /* for show */
} at_cache_t;

static inline int
at_ops_check(at_ops_t *ops, char *value)
{
    if (ops->check) {
        return (*ops->check)(ops, value);
    } else {
        return -ENOSUPPORT;
    }
}

static inline void
at_ops_write(at_ops_t *ops, char *value)
{
    if (ops->write) {
        (*ops->write)(ops, value);
    }
}

static inline void
at_ops_show(at_ops_t *ops)
{
    if (ops->show) {
        (*ops->show)(ops);
    }
}

static inline bool
__at_ops_match_wildcard(at_ops_t *ops, char *path, int len)
{
    return 0==len || 0==os_memcmp(path, ops->path, len);
}

static inline bool
at_ops_match(at_ops_t *ops, char *path, int len, bool wildcard)
{
    if (wildcard) {
        return __at_ops_match_wildcard(ops, path, len - 1);
    }
    else {
        return len==os_strlen(ops->path) && 0==os_memcmp(ops->path, path);
    }
}

typedef struct {
    at_env_t *env;
    at_ops_t *ops;
    at_cache_t *cache;
    int ops_count;
    
    bool dirty[AT_BLOCK_COUNT];
    
#if defined(__BUSYBOX__) || defined(__APP__)
    FILE *stream;
#endif

    int (*lock)(void);
    int (*unlock)(void);

    int show_count;
    int  argc;
    char **argv;
} 
at_control_t;

#define AT_CONTROL_DEFT(_env, _ops, _cache) {   \
    .env        = _env,                         \
    .ops        = _ops,                         \
    .cache      = _cache,                       \
    .ops_count  = os_count_of(_ops),            \
}   /* end */

extern at_control_t     at_control;
#define __at_control    (&at_control)

#define __at_stream     __at_control->stream
#define __at_env        __at_control->env
#define __at_ops        __at_control->ops
#define __at_ops_count  __at_control->ops_count
#define __at_show_count __at_control->show_count
#define __at_cache      __at_control->cache
#define __at_cookie     (&__at_env->cookie)
#define __at_os         (&__at_env->os)
#define __at_kernel     (&__at_os->kernel)
#define __at_rootfs     (&__at_os->rootfs)
#define __at_mark       (&__at_env->mark)
#define __at_info       (&__at_env->info)

#define at_ops(_idx)            (&__at_ops[_idx])
#define at_ops_idx(_ops)        ((at_ops_t *)(_ops) - __at_ops)
#define __at_ops_obj(_ops)      ((char *)__at_env + (_ops)->offset)
#define at_ops_obj(_type, _ops) ((_type *)__at_ops_obj(_ops))

#define at_cache(_ops)          (&__at_cache[at_ops_idx(_ops)])
#define at_cache_value(_ops)    at_cache(_ops)->value
#define at_cache_showit(_ops)   at_cache(_ops)->showit

#define at_kernel(_idx)         (&__at_kernel->vcs[_idx])
#define at_rootfs(_idx)         (&__at_rootfs->vcs[_idx])
#define at_mark(_idx)           __at_mark->key[_idx]
#define at_info(_idx)           __at_info->var[_idx]

static inline bool
is_at_cookie_deft(void)
{
    at_cookie_t deft = AT_DEFT_COOKIE;

    return os_objeq(&deft, __at_cookie);
}

static inline int
at_lock(void)
{
    if (__at_control->lock) {
        return (*__at_control->lock)();
    } else {
        return 0;
    }
}

static inline int
at_unlock(void)
{
    if (__at_control->unlock) {
        return (*__at_control->unlock)();
    } else {
        return 0;
    }
}

static inline int
at_init(void)
{
    BUILD_BUG_ON(AT_COOKIE_SIZE > AT_BLOCK_SIZE);
    
    at_lock();
    
    os_memzero(__at_cache, __at_ops_count * sizeof(at_cache_t));

    os_arrayzero(__at_control->dirty);

    __at_control->write       = false;
    __at_control->wildcard    = false;
    __at_control->argc        = 0;
    __at_control->argv        = NULL;

    return 0;
}

static inline int
at_fini(void)
{
    at_unlock();

    return 0;
}

static inline void
at_deft(void)
{
    /*
    * recover vendor/firmware
    *
    * NOT recover key/var
    */
    at_obj_deft(__at_cookie,    AT_DEFT_COOKIE);
    at_obj_deft(__at_os,        AT_DEFT_OS);
}

#define AT_OS_SORT_COUNT    (AT_OS_COUNT - 2)

/*
* 1. error, bad < good
* 2. version, small < big
* 3. upgrade, fail < unknow < ok
* 4. other, fail < unknow < ok
* 5. self, fail < unknow < ok
*/
static inline int
at_vcs_cmp(at_vcs_t *a, at_vcs_t *b)
{
    int ret;

    ret = at_error_cmp(a->error, b->error);
    if (ret) {
        return ret;
    }

    ret = at_version_cmp(&a->version, &b->version);
    if (ret) {
        return ret;
    }

    ret = at_fsm_cmp(a->upgrade, b->upgrade);
    if (ret) {
        return ret;
    }

    ret = at_fsm_cmp(a->other, b->other);
    if (ret) {
        return ret;
    }

    ret = at_fsm_cmp(a->self, b->self);
    if (ret) {
        return ret;
    }

    return 0;
}

/*
* return array idx
*/
static inline int
at_os_min(at_firmware_t *firmware, int array[], int count)
{
    int i, min = count;
    at_vcs_t vmax = AT_MAX_VCS;
    at_vcs_t *vmin = &vmax;
    at_vcs_t *vcs;
    
    for (i=0; i<count; i++) {
        vcs = &firmware->vcs[array[i]];

        /*
        * rootfs <= rootfs_min
        */
        if (at_vcs_cmp(vcs, vmin) <= 0) {
            vmin = vcs; min = i;
        }
    }

    return min;
}

static inline void
at_os_sort_init(int current, int array[AT_OS_SORT_COUNT])
{
    int i, idx = 0;

    for (i=1; i<AT_OS_COUNT; i++) {
        if (i==current) {
            continue;
        }

        array[idx++] = i;
    }
}


static inline void
__at_os_sort(at_firmware_t *firmware, int array[], int count)
{
    if (count <= 1) {
        return;
    }
    
    int min = at_os_min(firmware, array, count);

    if (min) {
        os_swap_value(array[0], array[min]);
    }

    __at_os_sort(firmware, array + 1, count - 1);
}

/*
* up-sort, bad==>good
*
* 1. error, bad==>good
* 2. version, small==>big
* 3. upgrade, fail==>unknow==>ok
* 4. other, fail==>unknow==>ok
* 5. self, fail==>unknow==>ok
*/
static inline void
at_os_sort(at_firmware_t *firmware, int array[AT_OS_SORT_COUNT])
{   
    at_os_sort_init(firmware->current, array);

    __at_os_sort(firmware, array, AT_OS_SORT_COUNT);
}

static inline int
at_os_foreach(at_firmware_t *firmware, at_vcs_t *filter, multi_value_t (*foreach)(at_vcs_t *vcs))
{
    int i;
    multi_value_u mv;
    
    for (i=1; i<AT_OS_COUNT; i++) {
        at_vcs_t *vcs = &firmware->vcs[i];
        
        if (at_vcs_match(vcs, filter)) {
            mv.value = (*foreach)(vcs);
            if (mv2_is_break(mv)) {
                return mv2_result(mv);
            }
        }
    }

    return 0;
}

#if 0
static int
__at_get_rootfs_ok(at_vcs_t *filter)
{
    int idx = -1;
    at_version_t version = AT_MIN_VERSION;
    
    multi_value_t foreach(at_vcs_t *info)
    {
        if (memcmp(&version, &info->version) > 0) {
            idx = info - at_rootfs->info;
        }
        
        return mv2_OK;
    }
    
    at_os_foreach(filter, foreach);

    return idx;
}

static int
at_get_rootfs_self_ok(void)
{
    at_vcs_t filter = __AT_VCS(AT_FSM_OK, AT_FSM_INVALID, AT_FSM_INVALID, AT_FSM_INVALID, AT_INVALID_VERSION);

    return __at_get_rootfs_ok(&filter);
}

static int
at_get_rootfs_other_ok(void)
{
    at_vcs_t filter = __AT_VCS(AT_FSM_INVALID, AT_FSM_OK, AT_FSM_INVALID, AT_FSM_INVALID, AT_INVALID_VERSION);

    return __at_get_rootfs_ok(&filter);
}

static int
at_get_rootfs_ok(void)
{
    int idx = -1;

    idx = at_get_rootfs_self_ok();
    if (idx>0) {
        return idx;
    }

    idx = at_get_rootfs_other_ok();
    if (idx>0) {
        return idx;
    }

    return 0;
}
#endif

static inline void 
at_show_header(at_ops_t *ops)
{
    if (__at_show_count > 1) {
        os_printf("%s=", ops->path);
    }
}

static inline void 
at_show_uint(at_ops_t *ops)
{
    at_show_header(ops);

    os_println("%u", *at_ops_obj(unsigned int, ops));
}

static inline void 
at_show_string(at_ops_t *ops)
{
    char *string = at_ops_obj(char, ops);

    if (string[0]) {
        at_show_header(ops);

        os_println("%s", string);
    }
}

static inline void
at_set_dirty(at_ops_t *ops)
{
    int offset  = at_ops_obj(char, ops) - (char *)__at_env;
    int idx     = os_align(offset, AT_BLOCK_SIZE);
    
    __at_control->dirty[idx] = true;
}

static inline void 
at_set_uint(at_ops_t *ops, char *value)
{
    *at_ops_obj(unsigned int, ops) = 
        (unsigned int)(value[0]?os_atoi(value):0);
}

static inline void 
at_set_string(at_ops_t *ops, char *value)
{
    char *string = at_ops_obj(char, ops);
    
    if (value[0]) {
        os_strcpy(string, value);
    } else {
        string[0] = 0;
    }
}


static inline int 
at_check_version(at_ops_t *ops, char *value)
{
    at_version_t version = AT_INVALID_VERSION;
    
    if (at_version_atoi(&version, value)) {
        /*
        * when set version, must input value
        */
        return -EFORMAT;
    } else {
        return is_good_at_version(&version)?0:-EFORMAT;
    }
}

static inline void 
at_show_version(at_ops_t *ops)
{
    at_show_header(ops);

    os_println("%s", at_version_itoa(at_ops_obj(at_version_t, ops)));
}

static inline void 
at_set_version(at_ops_t *ops, char *value)
{
    at_version_t *v = at_ops_obj(at_version_t, ops);
    
    if (value[0]) {
        at_version_atoi(v, value);
    } else {
        at_version_t version = AT_DEFT_VERSION;

        os_objscpy(v, &version);
    }
}

static inline int 
at_check_fsm(at_ops_t *ops, char *value)
{
    if (value[0]) {
        return is_good_at_fsm(at_fsm_idx(value))?0:-EFORMAT;
    } else {
        /*
        * when set self/other/upgrade, must input value
        */
        return -EFORMAT;
    }
}

static inline void 
at_show_fsm(at_ops_t *ops)
{
    int fsm = *at_ops_obj(unsigned int, ops);
    
    at_show_header(ops);
    
    os_println("%s", at_fsm_string(fsm));
}

static inline void 
at_set_fsm(at_ops_t *ops, char *value)
{
    unsigned int fsm;
    
    if (value[0]) {
        fsm = at_fsm_idx(value);
    } else {
        fsm = AT_FSM_UNKNOW;
    }

    *at_ops_obj(unsigned int, ops) = fsm;
}

static inline int 
at_check_current(at_ops_t *ops, char *value)
{
    if (value[0]) {
        int v = os_atoi(value);

        return is_good_enum(v, AT_OS_COUNT);
    } else {
        /*
        * when set kernel/rootfs current, must input value
        */
        return -EFORMAT;
    }
}

static inline int 
at_check_var_string(at_ops_t *ops, char *value)
{
    if (value[0]) {
        return (os_strlen(value) < AT_VAR_SIZE)?0:-EFORMAT;
    } else {
        /*
        * when set var string, may NOT input value
        */
        return 0;
    }
}

#define AT_OPS(_path, _member, _check, _write, _show) { \
    .path   = _path,    \
    .offset = offsetof(at_env_t, _member), \
    .check  = _check,   \
    .write  = _write,   \
    .show   = _show,    \
}   /* end */

#define __AT_COOKIE_OPS(_path, _addr, _show) \
    AT_OPS(_path, _addr, NULL, NULL, _show)

#define AT_COOKIE_OPS \
    __AT_COOKIE_OPS("cookies/product/vendor",   \
        cookie.product.vendor,  at_show_string),\
    __AT_COOKIE_OPS("cookies/product/company",  \
        cookie.product.company, at_show_string),\
    __AT_COOKIE_OPS("cookies/pcba/model",       \
        cookie.pcba.model,      at_show_string),\
    __AT_COOKIE_OPS("cookies/pcba/version",     \
        cookie.pcba.version,    at_show_string) \
    /* end */

#define __AT_OS_OPS(_obj, _idx)             \
    AT_OPS(#_obj "/" #_idx "/self",         \
        os._obj.vcs[_idx].self,             \
        at_check_fsm, at_set_fsm, at_show_fsm), \
    AT_OPS(#_obj "/" #_idx "/other",        \
        os._obj.vcs[_idx].other,            \
        at_check_fsm, at_set_fsm, at_show_fsm), \
    AT_OPS(#_obj "/" #_idx "/upgrade",      \
        os._obj.vcs[_idx].upgrade,          \
        at_check_fsm, at_set_fsm, at_show_fsm), \
    AT_OPS(#_obj "/" #_idx "/error",        \
        os._obj.vcs[_idx].error,            \
        NULL, at_set_uint, at_show_uint),   \
    AT_OPS(#_obj "/" #_idx "/version",      \
        os._obj.vcs[_idx].version,          \
        at_check_version, at_set_version, at_show_version) \
    /* end */

#define _AT_OS_OPS(_obj) \
    AT_OPS(#_obj "/current", os._obj.current, \
        at_check_current, at_set_uint, at_show_uint), \
    __AT_OS_OPS(_obj, 0), \
    __AT_OS_OPS(_obj, 1), \
    __AT_OS_OPS(_obj, 2), \
    __AT_OS_OPS(_obj, 3), \
    __AT_OS_OPS(_obj, 4), \
    __AT_OS_OPS(_obj, 5), \
    __AT_OS_OPS(_obj, 6)  \
    /* end */
    
#define AT_OS_OPS       \
    _AT_OS_OPS(kernel), \
    _AT_OS_OPS(rootfs)  \
    /* end */

#define __AT_MARK_OPS(_path, _idx, _check) \
    AT_OPS("marks/" _path, mark.key[_idx], _check, at_set_uint, at_show_uint)

#define AT_MARK_OPS_NAMES_COMMON \
    __AT_MARK_OPS("boot/debug",      0, NULL),   \
    __AT_MARK_OPS("ptest/control",   1, NULL),   \
    __AT_MARK_OPS("ptest/result",    2, NULL)    \
    /* end */

#ifdef __BOOT__
#define AT_MARK_OPS_NAMES    \
    AT_MARK_OPS_NAMES_COMMON \
    /* end */

#define AT_MARK_OPS      \
    AT_MARK_OPS_NAMES    \
    /* end */

#elif defined(__BUSYBOX__) || defined(__APP__)
#define AT_MARK_OPS_NAMES    \
    AT_MARK_OPS_NAMES_COMMON \
    /* end */

#define __AT_MARK_OPS_IDX(_idx) \
    __AT_MARK_OPS(#_idx, _idx, NULL)

#define AT_MARK_OPS_IDX \
    __AT_MARK_OPS_IDX(3), \
    __AT_MARK_OPS_IDX(4), \
    __AT_MARK_OPS_IDX(5), \
    __AT_MARK_OPS_IDX(6), \
    __AT_MARK_OPS_IDX(7), \
    __AT_MARK_OPS_IDX(8), \
    __AT_MARK_OPS_IDX(9), \
                         \
    __AT_MARK_OPS_IDX(10), \
    __AT_MARK_OPS_IDX(11), \
    __AT_MARK_OPS_IDX(12), \
    __AT_MARK_OPS_IDX(13), \
    __AT_MARK_OPS_IDX(14), \
    __AT_MARK_OPS_IDX(15), \
    __AT_MARK_OPS_IDX(16), \
    __AT_MARK_OPS_IDX(17), \
    __AT_MARK_OPS_IDX(18), \
    __AT_MARK_OPS_IDX(19), \
                          \
    __AT_MARK_OPS_IDX(20), \
    __AT_MARK_OPS_IDX(21), \
    __AT_MARK_OPS_IDX(22), \
    __AT_MARK_OPS_IDX(23), \
    __AT_MARK_OPS_IDX(24), \
    __AT_MARK_OPS_IDX(25), \
    __AT_MARK_OPS_IDX(26), \
    __AT_MARK_OPS_IDX(27), \
    __AT_MARK_OPS_IDX(28), \
    __AT_MARK_OPS_IDX(29), \
                          \
    __AT_MARK_OPS_IDX(30), \
    __AT_MARK_OPS_IDX(31), \
    __AT_MARK_OPS_IDX(32), \
    __AT_MARK_OPS_IDX(33), \
    __AT_MARK_OPS_IDX(34), \
    __AT_MARK_OPS_IDX(35), \
    __AT_MARK_OPS_IDX(36), \
    __AT_MARK_OPS_IDX(37), \
    __AT_MARK_OPS_IDX(38), \
    __AT_MARK_OPS_IDX(39), \
    __AT_MARK_OPS_IDX(40), \
                          \
    __AT_MARK_OPS_IDX(41), \
    __AT_MARK_OPS_IDX(42), \
    __AT_MARK_OPS_IDX(43), \
    __AT_MARK_OPS_IDX(44), \
    __AT_MARK_OPS_IDX(45), \
    __AT_MARK_OPS_IDX(46), \
    __AT_MARK_OPS_IDX(47), \
    __AT_MARK_OPS_IDX(48), \
    __AT_MARK_OPS_IDX(49), \
                          \
    __AT_MARK_OPS_IDX(50), \
    __AT_MARK_OPS_IDX(51), \
    __AT_MARK_OPS_IDX(52), \
    __AT_MARK_OPS_IDX(53), \
    __AT_MARK_OPS_IDX(54), \
    __AT_MARK_OPS_IDX(55), \
    __AT_MARK_OPS_IDX(56), \
    __AT_MARK_OPS_IDX(57), \
    __AT_MARK_OPS_IDX(58), \
    __AT_MARK_OPS_IDX(59), \
                          \
    __AT_MARK_OPS_IDX(60), \
    __AT_MARK_OPS_IDX(61), \
    __AT_MARK_OPS_IDX(62), \
    __AT_MARK_OPS_IDX(63), \
    __AT_MARK_OPS_IDX(64), \
    __AT_MARK_OPS_IDX(65), \
    __AT_MARK_OPS_IDX(66), \
    __AT_MARK_OPS_IDX(67), \
    __AT_MARK_OPS_IDX(68), \
    __AT_MARK_OPS_IDX(69), \
                          \
    __AT_MARK_OPS_IDX(70), \
    __AT_MARK_OPS_IDX(71), \
    __AT_MARK_OPS_IDX(72), \
    __AT_MARK_OPS_IDX(73), \
    __AT_MARK_OPS_IDX(74), \
    __AT_MARK_OPS_IDX(75), \
    __AT_MARK_OPS_IDX(76), \
    __AT_MARK_OPS_IDX(77), \
    __AT_MARK_OPS_IDX(78), \
    __AT_MARK_OPS_IDX(79), \
                          \
    __AT_MARK_OPS_IDX(80), \
    __AT_MARK_OPS_IDX(81), \
    __AT_MARK_OPS_IDX(82), \
    __AT_MARK_OPS_IDX(83), \
    __AT_MARK_OPS_IDX(84), \
    __AT_MARK_OPS_IDX(85), \
    __AT_MARK_OPS_IDX(86), \
    __AT_MARK_OPS_IDX(87), \
    __AT_MARK_OPS_IDX(88), \
    __AT_MARK_OPS_IDX(89), \
                          \
    __AT_MARK_OPS_IDX(90), \
    __AT_MARK_OPS_IDX(91), \
    __AT_MARK_OPS_IDX(92), \
    __AT_MARK_OPS_IDX(93), \
    __AT_MARK_OPS_IDX(94), \
    __AT_MARK_OPS_IDX(95), \
    __AT_MARK_OPS_IDX(96), \
    __AT_MARK_OPS_IDX(97), \
    __AT_MARK_OPS_IDX(98), \
    __AT_MARK_OPS_IDX(99), \
                          \
    __AT_MARK_OPS_IDX(100), \
    __AT_MARK_OPS_IDX(101), \
    __AT_MARK_OPS_IDX(102), \
    __AT_MARK_OPS_IDX(103), \
    __AT_MARK_OPS_IDX(104), \
    __AT_MARK_OPS_IDX(105), \
    __AT_MARK_OPS_IDX(106), \
    __AT_MARK_OPS_IDX(107), \
    __AT_MARK_OPS_IDX(108), \
    __AT_MARK_OPS_IDX(109), \
                           \
    __AT_MARK_OPS_IDX(110), \
    __AT_MARK_OPS_IDX(111), \
    __AT_MARK_OPS_IDX(112), \
    __AT_MARK_OPS_IDX(113), \
    __AT_MARK_OPS_IDX(114), \
    __AT_MARK_OPS_IDX(115), \
    __AT_MARK_OPS_IDX(116), \
    __AT_MARK_OPS_IDX(117), \
    __AT_MARK_OPS_IDX(118), \
    __AT_MARK_OPS_IDX(119), \
                           \
    __AT_MARK_OPS_IDX(120), \
    __AT_MARK_OPS_IDX(121), \
    __AT_MARK_OPS_IDX(122), \
    __AT_MARK_OPS_IDX(123), \
    __AT_MARK_OPS_IDX(124), \
    __AT_MARK_OPS_IDX(125), \
    __AT_MARK_OPS_IDX(126), \
    __AT_MARK_OPS_IDX(127)  \
    /* end */

#define AT_MARK_OPS      \
    AT_MARK_OPS_NAMES,   \
    AT_MARK_OPS_IDX      \
    /* end */
#endif

#define __AT_INFO_OPS(_path, _idx) \
    AT_OPS("infos/" _path, info.var[_idx], at_check_var_string, at_set_string, at_show_string)

#define AT_INFO_OPS_NAMES_COMMON            \
    __AT_INFO_OPS("pcba/model",     0),     \
    __AT_INFO_OPS("pcba/version",   1),     \
    __AT_INFO_OPS("product/vendor", 2),     \
    __AT_INFO_OPS("product/company",3),     \
    __AT_INFO_OPS("product/model",  4),     \
    __AT_INFO_OPS("product/mac",    5),     \
    __AT_INFO_OPS("product/sn",     6),     \
    __AT_INFO_OPS("product/lms",    7),     \
                                            \
    __AT_INFO_OPS("oem/vendor",     10),    \
    __AT_INFO_OPS("oem/company",    11),    \
    __AT_INFO_OPS("oem/model",      12),    \
    __AT_INFO_OPS("oem/mac",        13),    \
    __AT_INFO_OPS("oem/sn",         14),    \
    __AT_INFO_OPS("oem/lms",        15)     \
    /* end */

#ifdef __BOOT__
#define AT_INFO_OPS_NAMES \
    AT_INFO_OPS_NAMES_COMMON

#define AT_INFO_OPS \
    AT_INFO_OPS_NAMES

#elif defined(__BUSYBOX__) || defined(__APP__)
#define AT_INFO_OPS_NAMES \
    AT_INFO_OPS_NAMES_COMMON

#define __AT_INFO_OPS_IDX(_idx) \
    __AT_INFO_OPS(#_idx, _idx)

#define AT_INFO_OPS_IDX     \
    __AT_INFO_OPS_IDX(8),   \
    __AT_INFO_OPS_IDX(9),   \
                            \
    __AT_INFO_OPS_IDX(16),  \
    __AT_INFO_OPS_IDX(17),  \
    __AT_INFO_OPS_IDX(18),  \
    __AT_INFO_OPS_IDX(19),  \
                            \
    __AT_INFO_OPS_IDX(20),  \
    __AT_INFO_OPS_IDX(21),  \
    __AT_INFO_OPS_IDX(22),  \
    __AT_INFO_OPS_IDX(23),  \
    __AT_INFO_OPS_IDX(24),  \
    __AT_INFO_OPS_IDX(25),  \
    __AT_INFO_OPS_IDX(26),  \
    __AT_INFO_OPS_IDX(27),  \
    __AT_INFO_OPS_IDX(28),  \
    __AT_INFO_OPS_IDX(29),  \
                            \
    __AT_INFO_OPS_IDX(30),  \
    __AT_INFO_OPS_IDX(31),  \
    __AT_INFO_OPS_IDX(32),  \
    __AT_INFO_OPS_IDX(33),  \
    __AT_INFO_OPS_IDX(34),  \
    __AT_INFO_OPS_IDX(35),  \
    __AT_INFO_OPS_IDX(36),  \
    __AT_INFO_OPS_IDX(37),  \
    __AT_INFO_OPS_IDX(38),  \
    __AT_INFO_OPS_IDX(39)   \
    /* end */

#define AT_INFO_OPS      \
    AT_INFO_OPS_NAMES,   \
    AT_INFO_OPS_IDX      \
    /* end */
#endif

#define AT_DEFT_OPS {   \
    AT_COOKIE_OPS,      \
    AT_OS_OPS,          \
    AT_MARK_OPS,        \
    AT_INFO_OPS,        \
}  /* end */

static inline void
__at_show_byprefix(char *prefix)
{
    int i;
    int len = prefix?os_strlen(prefix):0;
    bool wildcard = __at_control->wildcard; __at_control->wildcard = true;
    
    for (i=0; i<__at_ops_count; i++) {
        at_ops_t *ops = at_ops(i);
        
        if (__at_ops_match_wildcard(ops, prefix, len)) {
            at_ops_show(ops);
        }
    }

    __at_control->wildcard = wildcard;
}

#define at_show_byprefix(_fmt, _args...) do{\
    char prefix[1+OS_LINE_LEN];             \
                                            \
    os_saprintf(prefix, _fmt, ##_args);     \
                                            \
    __at_show_byprefix(prefix);             \
}while(0)


static inline void
__at_handle_write(at_ops_t *ops)
{
    char *value = at_cache_value(ops);
    
    if (value) {
        at_ops_write(ops, value);

        at_set_dirty(ops);
    }
}

static inline void
__at_handle_show(at_ops_t *ops)
{
    at_ops_show(ops);
}

static inline void
__at_handle(at_ops_t *ops)
{
    /*
    * show
    */
    if (at_cache_showit(ops)) {
        __at_handle_show(ops);
    }
    /*
    * wirite
    */
    else if (at_cache_value(ops)) {
        __at_handle_write(ops);
    }
}

static inline void
at_handle(int argc, char *argv[])
{
    int i;

    for (i=0; i<__at_ops_count; i++) {
        __at_handle(at_ops(i));
    }
}

static inline int
at_usage(void)
{
    char *self = __at_control->argv[0];
    
    os_fprintln(stderr, "%s name ==> show env by name", self);
    os_fprintln(stderr, "%s name1=value1 name2=value2 ... ==> set env by name and value", self);

    return -EHELP;
}

static inline int
__at_analysis_write(at_ops_t *ops, char *args)
{
    char *path  = args;
    char *eq    = os_strchr(args, '=');
    char *value = eq + 1;
    int err;
    
    if (at_ops_match(ops, path, eq - path, false)) {
        err = at_ops_check(ops, value);
        if (err) {
            return err;
        }
        
        at_cache_value(ops) = value;
    }

    return 0;
}

static inline int
at_analysis_write(char *args)
{
    int i, err = 0;

    for (i=0; i<__at_ops_count; i++) {
        err = __at_analysis_write(at_ops(i), args);
        if (err) {
            return err;
        }
    }

    return 0;
}

static inline int
__at_analysis_show(at_ops_t *ops, char *args)
{
    int i, err = 0;

    char *wildcard = os_strlast(args, '*');
    
    /*
    * if found '*'
    *   first '*' is not last '*'
    * if NOT found '*'
    *   but found last '*'
    */
    if (os_strchr(args, '*') != wildcard) {
        return -EFORMAT;
    } else if (at_ops_match(ops, args, os_strlen(args), !!wildcard)) {
        at_cache_showit(ops) = true;
        
        __at_show_count++;
        
        return 0;
    }
}


static inline int
at_analysis_show(char *args)
{
    int i, err = 0;

    for (i=0; i<__at_ops_count; i++) {
        err = __at_analysis_show(at_ops(i), args);
        if (err) {
            return err;
        }
    }

    return 0;
}

static inline int
__at_analysis(char *args)
{
    int err;

    /*
    * first is '=', bad format
    */
    if ('=' == args[0]) {
        return -EFORMAT;
    }
    /*
    * found '=', is wirte
    */
    else if (os_strchr(args, '=')) {
        return at_analysis_write(args);
    }
    /*
    * no found '=', is show
    */
    else {
        return at_analysis_show(args);
    }
}

static inline int
at_analysis(int argc, char *argv[])
{
    int i, err;

    for (i=0; i<argc; i++) {
        err = __at_analysis(argv[i]);
        if (err) {
            return err;
        }
    }

    return 0;
}

static inline int
at_command(int argc, char *argv[])
{
    int err;

    if (0==argc) {
        return at_usage();
    }

    err = at_analysis(argc, argv);
    if (err) {
        return err;
    }

    at_handle(argc, argv);

    return 0;
}

extern int __at_save(int idx /* atenv's block */);

static inline void
at_save(void)
{
    int i, err;

    /* 
    * block 0 is read only 
    */
    for (i=1; i<AT_BLOCK_COUNT; i++) {
        if (__at_control->dirty[i]) {
            err = __at_save(i);
            if (err) {
                /* todo: log */
            }
            
            __at_control->dirty[i] = false;
        }
    }
}

static inline void *
at_obj(char *path)
{
    int i;

    for (i=0; i<__at_ops_count; i++) {
        at_ops_t *ops = at_ops(i);

        if (0==os_strcmp(path, ops->path)) {
            return __at_ops_obj(ops);
        }
    }

    return NULL;
}

static inline int
__at_main(int argc, char *argv[])
{
    int err;
        
    __at_control->argc = argc--;
    __at_control->argv = argv++;
    
    err = at_command(argc, argv);
    if (0==err) {
        at_save();
    }
        
    return err;
}

static inline int
at_main(int argc, char *argv[])
{
    return os_call(at_init, at_fini, __at_main, argc, argv);
}

/******************************************************************************/
#endif /* __ATENV_H_31502FF802B81D17AEDABEB5EDB5C121__ */

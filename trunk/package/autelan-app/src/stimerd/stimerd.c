/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#include "stimer/stimer.h"

static char RX[1 + STIMER_RXSIZE];
static char TX[1 + STIMER_TXSIZE];

static struct {
    struct {
        int unit;
        int fd;
    } timer;

    struct {
        int fd;

        struct sockaddr_un addr;
    } server_handle;
    
    int count;
    struct mlist_head head;
} 
stimerd = {
    .head = MLIST_HEAD_INIT(stimerd.head),

    .timer = {
        .unit   = STIMER_MSEC,
        .fd     = -1,
    },

    .server_handle = {
        .fd     = -1,
        
        .addr   = {
            .sun_family = AF_UNIX,
        },
    },
};

#define NAMESIZE    32
#define CMDSIZE     127
struct stimer {
    char name[1+NAMESIZE];
    char command[1+CMDSIZE];
    
    int delay;
    int interval;
    int limit;
    
    int times;
    int consume;

    struct mlist_node node;
    tm_node_t timer;
};

static struct stimer *
__entry(tm_node_t *timer)
{
    return container_of(timer, struct stimer, timer);
}

static int
__insert(struct stimer *entry)
{
    int err;
    
    if (NULL==entry) {
        return -EKEYNULL;
    }
    
    int hash(void)
    {
        return mlist_string_hash_idx(entry->name);
    }

    err = mlist_insert(&stimerd.head, &entry->node, hash);
    if (0==err) {
        stimerd.count++;
    }
    
    return err;
}

static int
__remove(struct stimer *entry)
{
    int err;

    if (NULL==entry) {
        return -EKEYNULL;
    }

    err = mlist_remove(&stimerd.head, &entry->node);
    if (0==err) {
        stimerd.count--;
    }

    return err;
}

static struct stimer *
__get(char *name)
{
    if (NULL==name) {
        return NULL;
    }
    
    int hash(void)
    {
        return mlist_string_hash_idx(name);
    }

    int eq(struct mlist_node *node)
    {
        struct stimer *entry = container_of(node, struct stimer, node);
        
        return 0==os_strcmp(name, entry->name);
    }

    return mlist_get(&stimerd.head, name, hash, eq);
}

static struct stimer *
__create(char *name, char *command, int delay, int interval, int limit)
{
    struct stimer *entry = __get(name);

    if (NULL==entry) {
        entry = (struct stimer *)os_zalloc(sizeof(*entry));
        if (NULL==entry) {
            return NULL;
        }
        
        os_strdcpy(entry->name, name);
    } else if (false==os_tm_is_pending(&entry->node.timer)) {
        /*
        * have exist and timer is NOT pending, re-use it
        */
        os_tm_remove(&entry->node.timer);
        os_memzero(entry, offsetof(struct stimer, node));
    } else {
        /*
        * have exist and timer is pending, do nothing
        */
        return entry;
    }
    
    os_strdcpy(entry->command, command);
    
    entry->delay    = delay;
    entry->interval = interval;
    entry->limit    = limit;

    __insert(entry);
}

static void
__destroy(struct stimer *entry)
{
    if (entry) {
        __remove(entry);
        os_tm_remove(&entry->node.timer);
        os_free(entry);
    }
}

#define NEXT(_string)   __string_next_byifs(_string, ' ')

static int
handle(struct stimerd_table map[], int count, char *tag, char *args)
{
    int i;
    
    for (i=0; i<count; i++) {
        if (0==os_strcmp(map[i].tag, tag)) {
            return (*map[i].cb)(args);
        }
    }

    return -EINVAL;
}

static int
handle_insert(char *args)
{
    char *name      = args; args = NEXT(args);
    char *delay     = args; args = NEXT(args);
    char *interval  = args; args = NEXT(args);
    char *limit     = args; args = NEXT(args);
    char *command   = args;

    if (NULL==name ||
        NULL==delay ||
        NULL==interval ||
        NULL==limit ||
        NULL==command) {
        return -EINVAL;
    }
    
    int i_delay     = atoi(delay);
    int i_interval  = atoi(interval);
    int i_limit     = atoi(limit));
    
    if (0==i_interval && 0==i_delay) {
        return -EINVAL;
    }

    __create(name, command, i_delay, i_interval, i_limit);

    return 0;
}

static int
handle_remove(char *args)
{
    char *name = args; args = NEXT(args);

    if (NULL==name) {
        return -EINVAL;
    }
    
    return 0;
}

static int
handle_show_log_byname(char *name)
{
    return 0;
}

static int
handle_show_log_all(void)
{
    return 0;
}

static int
handle_show_log(char *args)
{
    char *name = args; args = NEXT(args);

    if (NULL==name) {
        return handle_show_log_all();
    } else {
        return handle_show_log_byname(name);
    }
}

static int
handle_show_status_byname(char *name)
{
    return 0;
}

static int
handle_show_status_all(void)
{
    return 0;
}

static int
handle_show_status(char *args)
{
    char *name = args; args = NEXT(args);

    if (NULL==name) {
        return handle_show_status_all();
    } else {
        return handle_show_status_byname(name);
    }
}


static int
handle_show(char *args)
{
    static struct stimerd_table table[] = {
        STIMER_TABLE_ITEM("log",     handle_show_log),
        STIMER_TABLE_ITEM("status",  handle_show_status),
    };
    
    char *obj = args; args = NEXT(args);
    
    if (NULL==obj || NULL==args) {
        return -EINVAL;
    }
    
    return handle(table, os_count_of(table), obj, args);
}

static int
client_handle(void)
{
    static struct stimerd_table table[] = {
        STIMER_TABLE_ITEM("insert",  handle_insert),
        STIMER_TABLE_ITEM("remove",  handle_remove),
        STIMER_TABLE_ITEM("show",    handle_show),
    };

    char *method = TX;
    char *args   = TX;
    int i;
    
    __string_strim(method);

    args = NEXT(args);

    return handle(table, os_count_of(table), method, args);
}

static int
__client(int fd)
{
    int err;
    
    os_objzero(RX);
    os_objzero(TX);
    
    err = read(fd, RX, sizeof(RX));
    if (err <=0) {
        return errno;
    }
    
    err = client_handle();
    if (err<0) {
        return err;
    }
    
    err = write(fd, TX, os_strlen(TX));
    if (err <=0) {
        return errno;
    }
}

static int
client(void)
{
    struct sockaddr_in addr;
    int len = sizeof(addr);
    int fd;
    int err = 0;
    
    fd = accept(stimerd.server_handle.fd, (struct sockaddr *)&addr, &len);
    if (fd<0) {
        return errno;
    }
    
    err = __client(fd);
    close(fd);

    return err;
}

static int
server_handle(void)
{
    fd_set r;
    struct timeval tv = {
        .tv_sec     = STIMER_SEC,
        .tv_usec    = 0,
    };
    int maxfd = os_max(stimerd.timer.fd, stimerd.server_handle.fd);
    int err;
    
    FD_ZERO(&r);

    FD_SET(stimerd.timer.fd, &r);
    FD_SET(stimerd.server_handle.fd, &r);

    err = select(maxfd+1, &r, NULL, NULL, &tv);
    switch(err) {
        case -1:/* error */
            if (EINTR==errno) {
                // is breaked
                return 0;
            } else {
                return errno;
            }
        case 0: /* timeout, retry */
            return 0;
        default: /* to accept */
            break;
    }

    if (FD_ISSET(stimerd.timer.fd, &r)) {
        uint64_t timeout = 0;
        uint32_t times;
        
        err = read(stimerd.timer.fd, &timeout, sizeof(timeout));
        times = (err<0)?1:(uint32_t)timeout;
        os_tm_trigger(times);
    }

    if (FD_ISSET(stimerd.server_handle.fd, &r)) {
        client();
    }
}

static int 
server(void)
{
    while (1) {
        server_handle();
    }

    return 0;
}

static int
init_env() 
{
    char *env;
    
    env = getenv(ENV_STIMER_MSEC);
    if (is_good_env(env)) {
        stimerd.timer.unit = atoi(env);
    }
    
    env = getenv(ENV_STIMER_PATH);
    if (is_good_env(env)) {
        if (os_strlen(env) > stimerd.server_handle.addr.sun_path) {
            return -ETOOBIG;
        }
        os_strdcpy(stimerd.server_handle.addr.sun_path, env);
    }

    return 0;
}

static int
init_timerfd(void)
{
    struct itimerspec old;
    struct itimerspec new = {
        .it_interval = {
            .tv_sec     = STIMER_SEC,
            .tv_nsec    = STIMER_NSEC,
        },
    };
    int fd;
    
    fd = timerfd_create(CLOCK_REALTIME, 0);
    if (fd<0) {
        return errno;
    }
    debug_test("create timer fd(%d)", fd);

    new.it_value.tv_sec += STIMER_SEC;
    new.it_value.tv_nsec += STIMER_NSEC;
    if (new.it_value.tv_nsec >= 1000*1000*1000) {
        new.it_value.tv_nsec = 0;
        new.it_value.tv_sec++;
    }
    
    timerfd_settime(fd, 0, &new, &old);
    debug_test("set timer fd(%d)", fd);
    
    stimerd.timer.fd = fd;

    return 0;
}

static int
init_server(void)
{
    int fd;
    int err;
    
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd<0) {
        return errno;
    }

#if 0
    int opt = 1;
    err = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (err<0) {
        return errno;
    }
#endif

    err = bind(fd, (struct sockaddr *)&stimerd.server_handle.addr, sizeof(stimerd.server_handle.addr));
    if (err<0) {
        return errno;
    }

    err = listen(fd, 1);
    if (err<0) {
        return errno;
    }

    stimerd.server_handle.fd = fd;

    return 0;
}

int main(int argc, char *argv[])
{
    int err;

    err = init_env();
    if (err < 0) {
        return err;
    }

    err = init_timerfd();
    if (err < 0) {
        return err;
    }

    err = init_server();
    if (err < 0) {
        return err;
    }
    
	err = server();
    if (err < 0) {
        return err;
    }

    return 0;
}

/******************************************************************************/

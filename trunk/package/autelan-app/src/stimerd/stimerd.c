/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#include "stimer/stimer.h"

static char TX[1 + STIMER_RESSIZE];
static struct stimer_response *RES = (struct stimer_response *)TX;

#define res_sprintf(fmt, args...) ({ \
    int len = stimer_res_sprintf(RES, fmt, ##args); \
    debug_trace(fmt, ##args); \
    len; \
})

static inline int
res_error(int err)
{
    return stimer_res_error(RES, err);
}

#define res_ok  res_error(0)

static struct {
    struct {
        int ticks;  /* ms */
        int timeout;/* ms */
        
        int fd;
    } timer;

    struct {
        int fd;

        struct sockaddr_un addr;
    } server;

    struct {
        struct mlist_table mlist;
    } head;
} 
stimerd = {
    .timer = {
        .timeout= STIMER_TIMEOUT_TICKS,
        .ticks  = STIMER_TICKS,
        .fd     = -1,
    },

    .server = {
        .fd     = -1,
        
        .addr   = {
            .sun_family = AF_UNIX,
        },
    },
};

struct stimer {
    char name[1+STIMER_NAMESIZE];
    char command[1+STIMER_CMDSIZE];
    
    int delay;      /* ms */
    int interval;   /* ms */
    int limit;
    
    int triggers;

    struct {
        struct mlist_node   mlist;
        tm_node_t timer;
    } node;
};

static inline int
__ticks(void)
{
    return stimerd.timer.ticks;
}

static inline int
__sec(void)
{
    return time_sec(__ticks());
}

static inline int
__usec(void)
{
    return time_usec(__ticks());
}

static inline int
__nsec(void)
{
    return time_nsec(__ticks());
}

static inline struct stimer *
__tm_entry(tm_node_t *timer)
{
    return container_of(timer, struct stimer, node.timer);
}

static inline struct stimer *
__mlist_entry(struct mlist_node *node)
{
    return container_of(node, struct stimer, node.mlist);
}

static inline uint32_t 
__hash(char *name)
{
    return __string_bkdr(name) & (stimerd.head.mlist.size - 1);
}

static int
__insert(struct stimer *entry)
{
    if (NULL==entry) {
        return -EKEYNULL;
    }

    int hash(struct mlist_node *node)
    {
        return __hash(entry->name);
    }
    
    struct mlist_node *node = 
        __mlist_insert(&stimerd.head.mlist, &entry->node.mlist, hash);

    return node?0:-EINLIST;
}

static int
__remove(struct stimer *entry)
{
    if (NULL==entry) {
        return -EKEYNULL;
    }

    os_tm_remove(&entry->node.timer);

    return mlist_remove(&stimerd.head.mlist, &entry->node.mlist);
}

static struct stimer *
__get(char *name)
{
    if (NULL==name) {
        return NULL;
    }

    int hash(void)
    {
        return __hash(name);
    }
    
    bool eq(struct mlist_node *node)
    {
        struct stimer *entry = __mlist_entry(node);
        
        return 0==os_stracmp(entry->name, name);
    }

    struct mlist_node *node = mlist_find(&stimerd.head.mlist, hash, eq);
    if (NULL==node) {
        return NULL;
    }

    return __mlist_entry(node);
}

static int
__foreach(multi_value_t (*cb)(struct stimer *entry))
{
    multi_value_t foreach(struct mlist_node *node)
    {
        struct stimer *entry = __mlist_entry(node);

        return (*cb)(entry);
    }
    
    return mlist_foreach(&stimerd.head.mlist, foreach);
}

static struct stimer *
__create(char *name, char *command, int delay, int interval, int limit)
{
    struct stimer *entry = __get(name);

    if (NULL==entry) {
        /*
        * no found, create new
        */
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

    entry->triggers = 0;
    
    __insert(entry);

    return entry;
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
handle(struct cmd_table map[], int count, char *tag, char *args)
{
    int i, err;
    
    for (i=0; i<count; i++) {
        if (0==os_strcmp(map[i].tag, tag)) {
            err = os_cmd_line_cb(&map[i], args);

            return res_error(err);
        }
    }

    return res_error(-ESTIMER_INVAL0);
}

static int 
stimer_cb(tm_node_t *timer)
{
    struct stimer *entry = __tm_entry(timer);

    entry->triggers++;
    os_v_system("%s &", entry->command);

    debug_trace("trigger timer:%s"
                ", command:%s"
                ", delay:%d"
                ", interval:%d"
                ", limit:%d"
                ", triggers:%d",
                entry->name,
                entry->command,
                entry->delay,
                entry->interval,
                entry->limit,
                entry->triggers);

    if (entry->interval && entry->limit && entry->triggers < entry->limit) {
        debug_trace("re-insert timer:%s", entry->name);

        os_tm_insert(&entry->node.timer, entry->interval/__ticks(), stimer_cb, false);
    }

    return 0;
}

static int
handle_insert(char *args)
{
    char *name      = args; args = NEXT(args);
    char *delay     = args; args = NEXT(args);
    char *interval  = args; args = NEXT(args);
    char *limit     = args; args = NEXT(args);
    char *command   = args;
    int err;
    
    if (NULL==name      ||
        NULL==delay     ||
        NULL==interval  ||
        NULL==limit     ||
        NULL==command) {
        debug_error("NULL name|delay|interval|limit|command");
        
        return res_error(-ESTIMER_INVAL1);
    }

    debug_test("function:%s name:%s delay:%s interval:%s, limit:%s, command:%s", 
        __func__, 
        name, 
        delay,
        interval,
        limit,
        command);
    
    int i_delay     = os_atoi(delay);
    int i_interval  = os_atoi(interval);
    int i_limit     = os_atoi(limit);
    
    if (false==is_good_stimer_args(i_delay, i_interval, i_limit)) {
        debug_error("invalid args, delay:%d, interval:%d, limit:%d",
            i_delay, i_interval, i_limit);
        
        return res_error(-ESTIMER_INVAL2);
    }

    struct stimer *entry = __create(name, command, i_delay, i_interval, i_limit);
    if (NULL==entry) {
        return res_error(-ESTIMER_NOMEM);
    }

    int after;
    bool circle;
    
    if (i_interval) {
        after   = i_delay + i_interval;
        circle  = i_limit?false:true;
    } else {
        after   = i_delay;
        circle  = false;
    }
    
    debug_trace("insert timer:%s"
                ", command:%s"
                ", delay:%d"
                ", interval:%d"
                ", limit:%d"
                ", after:%d"
                ", circle:%d"
                ", pending:%s",
                entry->name,
                entry->command,
                entry->delay,
                entry->interval,
                entry->limit,
                after,
                circle,
                os_tm_is_pending(&entry->node.timer)?__true:__false);
    
    err = os_tm_insert(&entry->node.timer, after/__ticks(), stimer_cb, circle);
    if (err) {
        return res_error(-ESTIMER_EXIST);
    }

    return res_ok;
}

static int
handle_remove(char *args)
{
    char *name = args; args = NEXT(args);
    if (NULL==name) {
        debug_trace("remove timer without name");
        
        return res_error(-ESTIMER_INVAL3);
    }
    
    struct stimer *entry = __get(name);
    if (NULL==entry) {
        debug_trace("remove timer(%s) nofound", name);
        
        return res_error(-ESTIMER_NOEXIST);
    }

    debug_trace("remove timer(%s)", name);
    
    __destroy(entry);
    
    return res_ok;
}

static void
show(struct stimer *entry)
{
    res_sprintf("%s %d %d %d %d %d %s" __crlf,
        entry->name,
        entry->delay,
        entry->interval,
        entry->limit,
        entry->triggers,
        os_tm_left(&entry->node.timer) * __ticks(),
        entry->command);
}

static int
handle_show_status(char *args)
{
    char *name = args; args = NEXT(args);
    bool empty = true;
    
    res_sprintf("#name delay interval limit triggers left command" __crlf);
    
    multi_value_t cb(struct stimer *entry)
    {
        if (NULL==name || 0==os_stracmp(entry->name, name)) {
            show(entry);

            empty = false;
        }

        return mv2_OK;
    }
    
    __foreach(cb);
    
    if (empty) {
        /*
        * drop head line
        */
        RES->buf[0] = 0;
        RES->len = 0;

        debug_trace("remove timer(%s) nofound", name);
        
        return res_error(-ESTIMER_NOEXIST);
    }

    return res_ok;
}


static int
handle_show(char *args)
{
    static struct cmd_table table[] = {
        CMD_ENTRY("status",  handle_show_status),
    };
    
    char *obj = args; args = NEXT(args);
    
    if (NULL==obj) {
        return res_error(-ESTIMER_INVAL4);
    }

    return handle(table, os_count_of(table), obj, args);
}

static int
client_handle(char *buf)
{
    static struct cmd_table table[] = {
        CMD_ENTRY("insert",  handle_insert),
        CMD_ENTRY("remove",  handle_remove),
        CMD_ENTRY("show",    handle_show),
    };

    char *method = buf;
    char *args   = buf;

    __string_strim_both(method, NULL);
    __string_reduce(method, NULL);

    args = NEXT(args);

    int err = handle(table, os_count_of(table), method, args);
    
    debug_trace("method:%s, args:%s, buf:%s, error:%d, len:%d", 
        method, 
        args,
        RES->buf,
        RES->err,
        RES->len);
    
    return err;
}

static int
__client(int fd)
{
    int err;
    char buf[1+STIMER_REQSIZE] = {0};
    
    os_memzero(TX, os_count_of(TX));

    /*
    * todo: use select for timeout
    */
    err = os_io_read(fd, buf, sizeof(buf), stimerd.timer.timeout);
    if (err <0) {
        return err;
    }
    
    err = client_handle(buf);
    if (err) {
        /* just log, NOT return */
    }

    err = os_io_write(fd, (char *)RES, stimer_res_size(RES));
    if (err) {
        return err;
    }
    
    return 0;
}

static int
client(void)
{
    struct sockaddr_un addr;
    socklen_t len = sizeof(addr);

    int fd = accept(stimerd.server.fd, (struct sockaddr *)&addr, &len);
    if (fd<0) {
        debug_error("accept error:%d", -errno);
        return -errno;
    }
    debug_trace("accept ok");
    
    int err = __client(fd);
    close(fd);

    return err;
}

static void
timer(void)
{
    uint64_t timeout = 0;

    int err = read(stimerd.timer.fd, &timeout, sizeof(timeout));
    uint32_t times = (err<0)?1:(uint32_t)timeout;
    os_tm_trigger(times);
}

static int
__server_handle(fd_set *r)
{
    int err = 0;
    
    if (FD_ISSET(stimerd.timer.fd, r)) {
        timer();
    }

    if (FD_ISSET(stimerd.server.fd, r)) {
        err = client();
    }

    return err;
}

static int
server_handle(void)
{
    fd_set rset;
    struct timeval tv = {
        .tv_sec     = __sec(),
        .tv_usec    = __usec(),
    };
    int maxfd = os_max(stimerd.timer.fd, stimerd.server.fd);
    int err;
    
    FD_ZERO(&rset);
    FD_SET(stimerd.timer.fd, &rset);
    FD_SET(stimerd.server.fd, &rset);

    while(1) {
        err = select(maxfd+1, &rset, NULL, NULL, &tv);
        switch(err) {
            case -1:/* error */
                if (EINTR==errno) {
                    // is breaked
                    debug_trace("%s select breaked", __func__);
                    continue;
                } else {
                    debug_trace("%s select error:%d", __func__, -errno);
                    return -errno;
                }
            case 0: /* timeout, retry */
                debug_trace("%s select timeout", __func__);
                return -ETIMEOUT;
            default: /* to accept */
                debug_trace("%s select ok", __func__);
                
                return __server_handle(&rset);
        }
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
    int err;
    
    stimerd.timer.ticks     = get_stimer_ticks_env();
    stimerd.timer.timeout   = get_stimer_timeout_env();
    os_tm_unit_set(__ticks());

    err = get_stimer_path_env(&stimerd.server.addr);
    if (err) {
        return err;
    }

    return 0;
}

static int
init_timerfd(void)
{
    int fd = os_timer_fd(__sec(), __nsec());
    if (fd<0) {
        return fd;
    } else {
        stimerd.timer.fd = fd;

        return 0;
    }
}

static int
init_server(void)
{
    int fd;
    int err;
    
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd<0) {
    	debug_error("socket error:%d", -errno);
        return -errno;
    }

#if 0
    int opt = 1;
    err = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (err) {
        debug_error("setsockopt error:%d", -errno);
        return -errno;
    }
#endif

    unlink(stimerd.server.addr.sun_path);

    err = bind(fd, (struct sockaddr *)&stimerd.server.addr, sizeof(stimerd.server.addr));
    if (err) {
        debug_error("bind error:%d", -errno);
        return -errno;
    }

    err = listen(fd, SOMAXCONN);
    if (err) {
        debug_error("listen error:%d", -errno);
        return -errno;
    }

    stimerd.server.fd = fd;

    return 0;
}

int main(int argc, char *argv[])
{
    int err;

    err = init_env(); debug_ok_error(err, "init env");
    if (err < 0) {
        return err;
    }

    err = init_timerfd(); debug_ok_error(err, "init timerfd");
    if (err < 0) {
        return err;
    }
    
    err = init_server(); debug_ok_error(err, "init server");
    if (err < 0) {
        return err;
    }
    
    err = server(); debug_ok_error(err, "run server");
    if (err < 0) {
        return err;
    }
    
    return 0;
}

#ifndef STIMERD_HASHSIZE
#define STIMERD_HASHSIZE    1024
#endif

static os_constructor void
__init(void)
{
    int err = 0;

    err = mlist_table_init(&stimerd.head.mlist, STIMERD_HASHSIZE);
    debug_ok_error(err, "__init timerfd");
}

/******************************************************************************/
AKID_DEBUGER; /* must last os_constructor */

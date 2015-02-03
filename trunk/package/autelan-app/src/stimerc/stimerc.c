/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#include "stimer/stimer.h"

static char RX[1 + STIMER_RXSIZE];

static struct {
    char *self;

    int ticks;
    int timeout;
    
    struct sockaddr_un server;
} stimerc = {
    .server     = { .sun_family = AF_UNIX },
    .timeout    = STIMER_TIMEOUT_TICKS,
    .ticks      = STIMER_TICKS,
};

static void
dump_args(const char *func, int argc, char *argv[])
{
    int i;

    for (i=0; i<argc; i++) {
        debug_test("function:%s argv[%d]=%s", func, i, argv[i]);
    }
}

static int
usage(void)
{
    os_fprintln(stderr, "%s insert name delay interval limit command", stimerc.self);
    os_fprintln(stderr, "%s remove name", stimerc.self);
    os_fprintln(stderr, "%s show status [name]", stimerc.self);
    
    return -EINVAL;
}

static int
handle(struct stimer_table map[], int count, int argc, char *argv[])
{
    int i;

    if (argc < 1) {
        return usage();
    }
    
    for (i=0; i<count; i++) {
        if (0==os_strcmp(map[i].tag, argv[0])) {
            debug_test("tag:%s, method:%s", map[i].tag, argv[0]);
            
            return (*map[i].u.ccb)(argc-1, argv+1);
        }
    }

    return usage();
}

static int
__client(bool rpc, char *buf)
{
    int fd;
    int err;
    int len;
    
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd<0) {
        debug_error("socket error:%d", -errno);
        return -errno;
    }
    
#if 0
    int opt = 1;
    err = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (err<0) {
        return -errno;
    }
#endif

    // unlink(stimerc.server.sun_path);
    err = connect(fd, (struct sockaddr *)&stimerc.server, sizeof(stimerc.server));
    if (err < 0) {
        debug_error("connect error:%d", -errno);
        return -errno;
    }

    len = strlen(buf);
    err = stimer_write(fd, buf, len);
    if (err<0) {
        return err;
    }

    if (rpc) {
        err = stimer_read(fd, RX, sizeof(RX), stimerc.timeout);
        if (err<0) {
            return err;
        }

        os_println("%s", RX);
    }
    
    return 0;
}

#define client(rpc, fmt, args...)    ({ \
    char buf[1+OS_LINE_LEN] = {0};      \
    int err = 0;                        \
                                        \
    os_saprintf(buf, fmt, ##args);      \
    err = __client(rpc, buf);           \
                                        \
    err;                                \
})

static int
__insert(int argc, char *argv[])
{
    char *name      = argv[0];
    char *delay     = argv[1];
    char *interval  = argv[2];
    char *limit     = argv[3];
    char *command   = argv[4];

    dump_args(__func__, argc, argv);
    
    if (5!=argc) {
        return usage();
    }
    
    int i_delay     = atoi(delay);
    int i_interval  = atoi(interval);
    int i_limit     = atoi(limit);
    
    if (false==is_good_stimer_args(i_delay, i_interval, i_limit)) {
        return usage();
    }
    
    return client(false,
                "insert %s %s %s %s %s",
                name,
                delay,
                interval,
                limit,
                command);
}

static int
__remove(int argc, char *argv[])
{
    char *name = argv[0];

    dump_args(__func__, argc, argv);
    
    if (1!=argc) {
        return usage();
    }

    return client(false, "remove %s", name);
}

static int
show_status(int argc, char *argv[])
{
    char *name = argv[0];

    dump_args(__func__, argc, argv);
    
    if (0!=argc && 1!=argc) {
        return usage();
    }
    
    if (name) {
        return client(true, "show status %s", name);
    } else {
        return __client(true, "show status");
    }
}

static int
__show(int argc, char *argv[])
{
    static struct stimer_table table[] = {
        STIMER_ENTRY("status", show_status),
    };

    dump_args(__func__, argc, argv);
    
    int err = handle(table, os_count_of(table), argc, argv);
    if (err<0) {
        debug_error("show %s %s error:%d", 
            argv[0], 
            argv[1]?argv[1]:"",
            err);
    }
    
    return err;
}

static int
command(int argc, char *argv[])
{
    static struct stimer_table table[] = {
        STIMER_ENTRY("insert",  __insert),
        STIMER_ENTRY("remove",  __remove),
        STIMER_ENTRY("show",    __show),
    };

    dump_args(__func__, argc, argv);
    
    int err = handle(table, os_count_of(table), argc, argv);
    if (err<0) {
        debug_error("%s error:%d", argv[0], err);
    }
    
    return err;
}

static int
init_env() 
{
    stimerc.ticks   = get_stimer_ticks_env();
    stimerc.timeout = get_stimer_timeout_env();
    
    return get_stimer_path_env(&stimerc.server);
}

int main(int argc, char *argv[])
{
    int err;

    dump_args(__func__, argc, argv);
    stimerc.self = argv[0];
    
    os_memzero(RX, os_count_of(RX));

    err = init_env();
    if (err < 0) {
        debug_error("init env error:%d", err);
        return err;
    }
    debug_ok("init env ok.");
    
    err = command(argc-1, argv+1);
    if (err < 0) {
        return err;
    }

    return 0;
}
/******************************************************************************/
AKID_DEBUGER; /* must last os_constructor */

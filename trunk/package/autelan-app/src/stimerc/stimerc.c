/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#include "stimer/stimer.h"

static char RX[1 + STIMER_RESSIZE];
static struct stimer_response *RES = (struct stimer_response *)RX;

static struct {
    char *name;

    int ticks;
    int timeout;
    
    struct sockaddr_un server;
} stimerc = {
    .server     = { .sun_family = AF_UNIX },
    .timeout    = STIMER_TIMEOUT_TICKS,
    .ticks      = STIMER_TICKS,
};

#define dump_argv(_argc, _argv) \
        cmd_dump_argv(debug_trace, _argc, _argv)

static int
usage(int error)
{
    os_fprintln(stderr, "%s insert name delay interval limit command", stimerc.name);
    os_fprintln(stderr, "%s remove name", stimerc.name);
    os_fprintln(stderr, "%s show status [name]", stimerc.name);
    
    return error;
}

static int
handle(struct cmd_table map[], int count, int argc, char *argv[])
{
    int i;

    if (argc < 1) {
        return usage(-EINVAL);
    }
    
    for (i=0; i<count; i++) {
        if (0==os_strcmp(map[i].tag, argv[0])) {
            debug_test("tag:%s, method:%s", map[i].tag, argv[0]);
            
            return (*map[i].u.argv_cb)(argc-1, argv+1);
        }
    }

    return usage(-EINVAL);
}

static int
__client(char *buf)
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

    len = os_strlen(buf);
    err = io_write(fd, buf, len);
    if (err<0) {
        return err;
    }

    err = io_read(fd, RX, sizeof(RX), stimerc.timeout);
    if (err<0) {
        return err;
    }

    if (0==RES->err) {
        os_println("%s", RES->buf);
    }
    debug_trace("RES buf:%s, error:%d, len:%d", RES->buf, RES->err, RES->len);
    
    return shell_error(RES->err);
}

#define client(fmt, args...)        ({ \
    char buf[1+STIMER_REQSIZE] = {0};   \
    int err = 0;                        \
                                        \
    os_saprintf(buf, fmt, ##args);      \
    err = __client(buf);                \
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
    
    dump_argv(argc, argv);
    
    if (5!=argc) {
        return usage(-EINVAL);
    }
    else if (os_strlen(name) > STIMER_NAMESIZE) {
        return usage(-ETOOBIG);
    }
    else if (os_strlen(command) > STIMER_CMDSIZE) {
        return usage(-ETOOBIG);
    }
    
    int i_delay     = atoi(delay);
    int i_interval  = atoi(interval);
    int i_limit     = atoi(limit);
    
    if (false==is_good_stimer_args(i_delay, i_interval, i_limit)) {
        return usage(-EINVAL);
    }
    
    return client("insert %s %s %s %s %s",
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

    dump_argv(argc, argv);
    
    if (1!=argc) {
        return usage(-EINVAL);
    }
    else if (os_strlen(name) > STIMER_NAMESIZE) {
        return usage(-ETOOBIG);
    }
    else {
        return client("remove %s", name);
    }
}

static int
show_status(int argc, char *argv[])
{
    char *name = argv[0];

    dump_argv(argc, argv);
    
    if (0!=argc && 1!=argc) {
        return usage(-EINVAL);
    }
    else if (NULL==name) {
        return __client("show status");
    }
    else if (os_strlen(name) > STIMER_NAMESIZE) {
        return usage(-ETOOBIG);
    }
    else {
        return client("show status %s", name);
    }
}

static int
__show(int argc, char *argv[])
{
    static struct cmd_table table[] = {
        CMD_ENTRY("status", show_status),
    };

    dump_argv(argc, argv);
    
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
    static struct cmd_table table[] = {
        CMD_ENTRY("insert",  __insert),
        CMD_ENTRY("remove",  __remove),
        CMD_ENTRY("show",    __show),
    };

    dump_argv(argc, argv);
    
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

    dump_argv(argc, argv);
    stimerc.name = argv[0];
    
    os_memzero(RX, os_count_of(RX));

    err = init_env();
    if (err < 0) {
        debug_error("init env error:%d", err);
        return err;
    }
    debug_ok("init env ok.");
    
    err = command(argc-1, argv+1);
    if (err < 0) {
        /* just log, NOT return */
    }

    return shell_error(err);
}
/******************************************************************************/
AKID_DEBUGER; /* must last os_constructor */

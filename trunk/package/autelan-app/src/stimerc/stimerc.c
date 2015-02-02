/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#include "stimer/stimer.h"

static char RX[1 + STIMER_RXSIZE];

static struct {
    char *self;
    
    struct sockaddr_un server;
} stimerc = {
    .server = { .sun_family = AF_UNIX },
};

static int
usage(int argc, char *argv[])
{
    "stimerc insert name delay interval limit command"
    "stimerc remove name"
    "stimerc show status [name]"
    return -EINVAL;
}

static int
handle(struct stimerc_table map[], int count, int argc, char *argv[])
{
    int i;
    
    for (i=0; i<count; i++) {
        if (0==os_strcmp(map[i].tag, argv[0])) {
            return (*map[i].cb)(argc-1, argv+1);
        }
    }

    return -EINVAL;
}

static int
__client(char *buf)
{
    int fd;
    int err;
    int count;
    int len;
    
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
    
    err = connect(fd, (struct sockaddr *)&stimerc.server, sizeof(stimerc.server));
    if (err < 0) {
        return err;
    }

    len = strlen(buf);
    count = write(fd, buf, len);
    if (len!=count) {
        return -EIO;
    }
    
    count = read(fd, RX, sizeof(RX));
    if (count<=0) {
        return errno;
    }
    
    os_println("%s", RX);
    
    return 0;
}

#define client(fmt, args...)         ({ \
    char buf[1+OS_LINE_LEN] = {0};      \
    int err = 0;                        \
                                        \
    os_saprintf(buf, fmt, ##args);      \
    err = __client(buf);                \
                                        \
    err;                                \
})

static int
insert(int argc, char *argv[])
{
    char *name      = argv[0];
    char *delay     = argv[1];
    char *interval  = argv[2];
    char *limit     = argv[3];
    char *command   = argv[4];

    int i_delay, i_interval, i_limit;

    if (5!=argc) {
        return -EINVAL;
    }
    
    int i_delay     = atoi(delay);
    int i_interval  = atoi(interval);
    int i_limit     = atoi(limit));
    
    if (false==is_good_stimer_args(i_delay, i_interval, i_limit)) {
        return -EINVAL;
    }
    
    return client("insert %s %s %s %s %s",
                name,
                delay,
                interval,
                limit,
                command);
}

static int
remove(int argc, char *argv[])
{
    char *name = argv[0];

    if (1!=argc) {
        return -EINVAL;
    }

    return client("remove %s", name);
}

#if STIMER_SHOW_LOG
static int
show_log(int argc, char *argv[])
{
    char *name = argv[0];

    if (0!=argc || 1!=argc) {
        return -EINVAL;
    }
    
    if (name) {
        return client("show log %s", name);
    } else {
        return __client("show log");
    }
}
#endif

static int
show_status(int argc, char *argv[])
{
    char *name = argv[0];

    if (0!=argc || 1!=argc) {
        return -EINVAL;
    }
    
    if (name) {
        return client("show status %s", name);
    } else {
        return __client("show status");
    }
}

static int
show(int argc, char *argv[])
{
    static struct stimerc_table table[] = {
#if STIMER_SHOW_LOG
        STIMER_ENTRY("log",    show_log),
#endif
        STIMER_ENTRY("status", show_status),
    };
    
    return handle(table, os_count_of(table), argc, argv);
}

static int
command(int argc, char *argv[])
{
    static struct stimerc_table table[] = {
        STIMER_ENTRY("insert",  insert),
        STIMER_ENTRY("remove",  remove),
        STIMER_ENTRY("show",    show),
    };

    return handle(table, os_count_of(table), argc, argv);
}

static int
init_env() 
{
    return get_env_stimer_path(&stimerc.server);
}

int main(int argc, char *argv[])
{
    int err;

    stimerc.self = argv[0]; argc--; argv++;
    
    os_objzero(RX);

    err = init_env();
    if (err < 0) {
        return err;
    }

	err = command(argc, argv);
    if (err < 0) {
        return err;
    }

    return 0;
}
/******************************************************************************/
AKID_DEBUGER; /* must last os_constructor */
/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#include "stimer/stimer.h"

static char RX[1 + STIMER_RXSIZE];

static struct {
    struct sockaddr_un server;
} stimerc = {
    .server = { .sun_family = AF_UNIX },
};

static int
client(char *buf)
{
    int fd;
    int err;
    int count;
    int len;
    
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd<0) {
        return errno;
    }

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

#define CLIENT(fmt, args...)         ({ \
    char buf[1+OS_LINE_LEN] = {0};      \
    int err = 0;                        \
                                        \
    os_saprintf(buf, fmt, ##args);      \
    err = handle(buf);                  \
                                        \
    err;                                \
})

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
    
    if (0==i_interval && 0==i_delay) {
        return -EINVAL;
    }

    return CLIENT("insert %s %s %s %s %s",
                name,
                delay,
                interval,
                limit,
                command);
}

static int
remove(int argc, char *argv[])
{
    char *name      = argv[0];

    if (1!=argc) {
        return -EINVAL;
    }

    return CLIENT("remove %s", name);
}

static int
show_log(int argc, char *argv[])
{
    char *name = argv[0];

    if (0!=argc || 1!=argc) {
        return -EINVAL;
    }

    if (name) {
        return CLIENT("show log %s", name);
    } else {
        return client("show log");
    }
}

static int
show_status(int argc, char *argv[])
{
    char *name = argv[0];

    if (0!=argc || 1!=argc) {
        return -EINVAL;
    }
    
    if (name) {
        return CLIENT("show status %s", name);
    } else {
        return client("show status");
    }
}

static int
show(int argc, char *argv[])
{
    static struct stimerc_table table[] = {
        STIMER_TABLE_ITEM("log",    show_log),
        STIMER_TABLE_ITEM("status", show_status),
    };
    
    return handle(table, os_count_of(table), argc, argv);
}

static int
command(int argc, char *argv[])
{
    static struct stimerc_table table[] = {
        STIMER_TABLE_ITEM("insert",  insert),
        STIMER_TABLE_ITEM("remove",  remove),
        STIMER_TABLE_ITEM("show",    show),
    };

    return handle(table, os_count_of(table), argc, argv);
}

static int
init_env() 
{
    char *env;
    
    env = getenv(ENV_STIMER_PATH);
    if (is_good_env(env)) {
        if (os_strlen(env) > stimerc.addr.sun_path) {
            return -ETOOBIG;
        }
        
        os_strdcpy(stimerc.addr.sun_path, env);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int err;
    
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

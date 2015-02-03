#ifndef __STIMER_H_03B9C4BFCDC8EF03E5E39A08D5201373__
#define __STIMER_H_03B9C4BFCDC8EF03E5E39A08D5201373__
#if defined(STIMERD) || defined(STIMERC)
/******************************************************************************/
#include "utils.h"
#include "timer/timer.h"
/******************************************************************************/
#ifndef ENV_STIMER_PATH
#define ENV_STIMER_PATH             "__STIMER_PATH__"
#endif

#ifndef ENV_STIMER_TICKS
#define ENV_STIMER_TICKS            "__STIMER_TICKS__"
#endif

#ifndef ENV_STIMER_TIMEOUT_TICKS
#define ENV_STIMER_TIMEOUT_TICKS    "__STIMER_TIMEOUT_TICKS__"
#endif

#ifndef STIMER_PATH
#define STIMER_PATH                 "/tmp/.stimer.unix"
#endif

#ifndef STIMER_TXSIZE
#define STIMER_TXSIZE               (1024*1024 - 1)
#endif

#ifndef STIMER_RXSIZE
#define STIMER_RXSIZE               (1024*1024 - 1)
#endif

#ifndef STIMER_TICKS
#define STIMER_TICKS                1000
#endif

#ifndef STIMER_TIMEOUT_TICKS
#define STIMER_TIMEOUT_TICKS        5000
#endif

typedef int stimerd_f(char *args);
typedef int stimerc_f(int argc, char *argv[]);

struct stimer_table {
    char *tag;

    union {
        void *cb; /* stimerd_f or stimerc_f */
        stimerd_f *dcb;
        stimerc_f *ccb;
    } u;
};

#define STIMER_ENTRY(_tag, _cb)   { \
    .tag    = _tag,         \
    .u      = {             \
        .cb = _cb,          \
    },                      \
}

static inline int
stimer_sec(int ticks /* ms */)
{
    return ticks/1000;
}

static inline int
stimer_msec(int ticks /* ms */)
{
    return ticks;
}

static inline int
stimer_usec(int ticks /* ms */)
{
    return (ticks * 1000) % 1000;
}

static inline int
stimer_nsec(int ticks /* ms */)
{
    return (ticks * 1000 * 1000) % 1000;
}

#if 0
insert  name delay interval limit command
remove  name
show    status  [name]
#endif

static bool
is_good_stimer_args(int delay, int interval, int limit)
{
    if (delay<0 || interval<0 || limit<0) {
        return false;
    } else {
        return interval || (delay && 1==limit);
    }
}

static inline int
get_stimer_path_env(struct sockaddr_un *addr) 
{
    char *env = getenv(ENV_STIMER_PATH);
    if (false==is_good_env(env)) {
        env = STIMER_PATH;
    }
    if (os_strlen(env) > sizeof(addr->sun_path) - 1) {
        return -ETOOBIG;
    }
    os_strdcpy(addr->sun_path, env);
    debug_trace("unix path:%s", addr->sun_path);
    
    return 0;
}

static inline int
get_stimer_ticks_env(void) 
{
    char *env = getenv(ENV_STIMER_TICKS);
    if (false==is_good_env(env)) {
        debug_trace("no-found ticks env");
        
        return STIMER_TICKS;
    }

    int ticks = atoi(env);
    if (ticks<=0) {
        debug_error("bad ticks env:%d", ticks);
        
        return STIMER_TICKS;
    }
    
    return ticks;
}

static inline int
get_stimer_timeout_env(void) 
{
    char *env = getenv(ENV_STIMER_TIMEOUT_TICKS);
    if (false==is_good_env(env)) {
        debug_trace("no-found timeout env");
        
        return STIMER_TIMEOUT_TICKS;
    }

    int timeout = atoi(env);
    if (timeout<=0) {
        debug_error("bad timeout env:%d", timeout);
        
        return STIMER_TIMEOUT_TICKS;
    }
    
    return timeout;
}

static int
stimer_read(int fd, char *buf, int size, int timeout /* ms */)
{
    fd_set rset;
    struct timeval tv = {
        .tv_sec     = stimer_sec(timeout),
        .tv_usec    = stimer_usec(timeout),
    };
    int err, count;
    
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    while(1) {
        err = select(fd + 1, &rset, NULL, NULL, &tv);
        switch(err) {
            case -1:/* error */
                if (EINTR==errno) {
                    // is breaked
                    debug_trace("read breaked");
                    
                    continue;
                } else {
                    debug_trace("read error:%d", -errno);
                    
                    return -errno;
                }
            case 0: /* timeout, retry */
                debug_trace("read timeout");
                
                return -ETIMEOUT;
            default: /* to accept */
                count = read(fd, buf, size);

                debug_trace("read:%s", buf);
                
                return count;
        }
    }
}

static int
stimer_write(int fd, char *buf, int len)
{
    int count = 0;
    int err;

    while(count < len) {
        err = write(fd, buf + count, len - count);
        if (err<0) {
            debug_error("write error:%d", -errno);
            return -errno;
        } else {
            count += err;
        }
    }

    debug_trace("write:%s", buf);
    
    return 0;
}
/******************************************************************************/
#endif
#endif /* __STIMER_H_03B9C4BFCDC8EF03E5E39A08D5201373__ */

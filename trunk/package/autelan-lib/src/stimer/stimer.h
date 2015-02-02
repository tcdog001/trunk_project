#ifndef __STIMER_H_03B9C4BFCDC8EF03E5E39A08D5201373__
#define __STIMER_H_03B9C4BFCDC8EF03E5E39A08D5201373__
/******************************************************************************/
#include "utils.h"
#include "timer/timer.h"
/******************************************************************************/
#ifndef ENV_STIMER_MSEC
#define ENV_STIMER_MSEC     "__STIMER_MSEC__"
#endif

#ifndef ENV_STIMER_PATH
#define ENV_STIMER_PATH     "__STIMER_PATH__"
#endif

#ifndef STIMER_PATH
#define STIMER_PATH         "/tmp/stimer_path"#endif

#ifndef STIMER_MSEC
#define STIMER_MSEC         5000
#endif

#define STIMER_SEC          (STIMER_MSEC/1000)
#define STIMER_NSEC         ((STIMER_MSEC%1000) * 1000 * 1000)

#ifndef STIMER_TXSIZE
#define STIMER_TXSIZE       (1024*1024 - 1)
#endif

#ifndef STIMER_RXSIZE
#define STIMER_RXSIZE       (1024*1024 - 1)
#endif

#define STIMER_SHOW_LOG     0

struct stimerd_table {
    char *tag;
    int (*cb)(char *args);
};

struct stimerc_table {
    char *tag;
    int (*cb)(int argc, char **argv);
};

#define STIMER_ENTRY(_tag, _cb)   { \
    .tab    = _tag,         \
    .cb     = _cb,          \
}

#if 0
insert  name delay interval limit command
remove  name
show    log     [name]
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
get_env_stimer_path(struct sockaddr_un *addr) 
{
    char *env= getenv(ENV_STIMER_PATH);
    if (false==is_good_env(env)) {
        env = STIMER_PATH;
    }
    if (os_strlen(env) > sizeof(addr->sun_path - 1)) {
        return -ETOOBIG;
    }
    os_strdcpy(addr->sun_path, env);

    return 0;
}

/******************************************************************************/
#endif /* __STIMER_H_03B9C4BFCDC8EF03E5E39A08D5201373__ */

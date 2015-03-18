#ifndef __STIMER_H_03B9C4BFCDC8EF03E5E39A08D5201373__
#define __STIMER_H_03B9C4BFCDC8EF03E5E39A08D5201373__
/******************************************************************************/
#include "utils.h"
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

#ifndef STIMER_REQSIZE
#define STIMER_REQSIZE              OS_LINE_LEN
#endif

#ifndef STIMER_RESSIZE
#define STIMER_RESSIZE              (1024*1024 - 1)
#endif

#ifndef STIMER_TICKS
#define STIMER_TICKS                1000
#endif

#ifndef STIMER_TIMEOUT_TICKS
#define STIMER_TIMEOUT_TICKS        5000
#endif

#ifndef STIMER_NAMESIZE
#define STIMER_NAMESIZE             31
#endif

#ifndef STIMER_CMDSIZE
#define STIMER_CMDSIZE              127
#endif

#define ERRNO_STIMER                __ERRNO(__ERRNO_STIMER) /* 3000 */

enum {
    ESTIMER_INVAL0    = ERRNO_STIMER,
    ESTIMER_INVAL1,
    ESTIMER_INVAL2,
    ESTIMER_INVAL3,
    ESTIMER_INVAL4,
    ESTIMER_INVAL5,
    ESTIMER_INVAL6,
    ESTIMER_INVAL7,
    ESTIMER_INVAL8,
    ESTIMER_INVAL9,
    ESTIMER_NOMEM,      /* 10 */
    ESTIMER_EXIST,      /* 11 */
    ESTIMER_NOEXIST,    /* 12 */
};

struct stimer_request {
    char buf[0];
};

struct stimer_response {
    int err;
    int len;
    char buf[0];
};
#define stimer_res_bufsize  (STIMER_RESSIZE - sizeof(struct stimer_response))

static inline int
stimer_res_size(struct stimer_response *res)
{
    return sizeof(struct stimer_response) + res->len;
}

static inline int
stimer_res_error(struct stimer_response *res, int err)
{
    return res?(res->err = err):err;
}

#define stimer_res_sprintf(_res, _fmt, args...)     ({  \
    int len = os_snprintf((_res)->buf + (_res)->len,    \
            stimer_res_bufsize - (_res)->len,           \
            _fmt,                                       \
            ##args);                                    \
    (_res)->len += len;                                 \
                                                        \
    len;                                                \
})

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
    return copy_string_env(
                ENV_STIMER_PATH, 
                STIMER_PATH, 
                addr->sun_path, 
                sizeof(addr->sun_path));
}

static inline int
get_stimer_ticks_env(void) 
{
    return get_int_env(ENV_STIMER_TICKS, STIMER_TICKS);
}

static inline int
get_stimer_timeout_env(void) 
{
    return get_int_env(ENV_STIMER_TIMEOUT_TICKS, STIMER_TIMEOUT_TICKS);
}
/******************************************************************************/
#endif /* __STIMER_H_03B9C4BFCDC8EF03E5E39A08D5201373__ */

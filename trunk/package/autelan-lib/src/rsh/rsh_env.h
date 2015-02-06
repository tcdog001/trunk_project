#ifndef __RSH_ENV_H_C02C6E3CE5B1852D787859FAD6B1D087__
#define __RSH_ENV_H_C02C6E3CE5B1852D787859FAD6B1D087__
/******************************************************************************/
#ifndef ENV_RSHA_BOARDTYPE
#define ENV_RSHA_BOARDTYPE          "__RSHA_BOARDTYPE__"
#endif

#ifndef ENV_RSHD_PATH
#define ENV_RSHD_PATH               "__RSHD_PATH__"
#endif

#ifndef ENV_RSHW_PATH
#define ENV_RSHW_PATH               "__RSHW_PATH__"
#endif

#ifndef ENV_RSHD_CONF
#define ENV_RSHD_CONF               "__RSHD_CONF__"
#endif

#ifndef ENV_RSHA_CONF
#define ENV_RSHA_CONF               "__RSHA_CONF__"
#endif

#ifndef ENV_RSH_ONLINE_KEEPALIVE
#define ENV_RSH_ONLINE_KEEPALIVE    "__RSH_ONLINE_KEEPALIVE__"
#endif

#ifndef ENV_RSH_ONLINE_INTERVAL
#define ENV_RSH_ONLINE_INTERVAL     "__RSH_ONLINE_INTERVAL__"
#endif

#ifndef ENV_RSH_WORKER_KEEPALIVE
#define ENV_RSH_WORKER_KEEPALIVE    "__RSH_WORKER_KEEPALIVE__"
#endif

#ifndef ENV_RSH_WORKER_INTERVAL
#define ENV_RSH_WORKER_INTERVAL     "__RSH_WORKER_INTERVAL__"
#endif

#ifndef RSHD_PATH
#define RSHD_PATH                   "/tmp/.rshd.unix"
#endif

#ifndef RSHW_PATH
#define RSHW_PATH                   "/tmp/.rshw.unix.%d"
#endif

#ifndef RSHD_CONF
#define RSHD_CONF                   "/etc/rsh/rshd.conf"
#endif

#ifndef RSHA_CONF
#define RSHA_CONF                   "/etc/rsh/rsha.conf"
#endif

#ifndef RSH_ONLINE_KEEPALIVE
#define RSH_ONLINE_KEEPALIVE        5000 /* ms */
#endif

#ifndef RSH_ONLINE_INTERVAL
#define RSH_ONLINE_INTERVAL         6   /* times */
#endif

#ifndef RSH_WORKER_KEEPALIVE
#define RSH_WORKER_KEEPALIVE        3000 /* ms */
#endif

#ifndef RSH_WORKER_INTERVAL
#define RSH_WORKER_INTERVAL         5   /* times */
#endif

enum {
    RSHA_BOARDTYPE_COMMON,
    RSHA_BOARDTYPE_MD           = RSHA_BOARDTYPE_COMMON,
    RSHA_BOARDTYPE_AP,

    RSHA_BOARDTYPE_END
};

#ifndef RSHA_BOARDTYPE
#define RSHA_BOARDTYPE          RSHA_BOARDTYPE_MD
#endif

#define RSHA_BOARDTYPE_STRINGS                  \
    [RSHA_BOARDTYPE_COMMON]     = "common",     \
    [RSHA_BOARDTYPE_AP]         = "ap",         \
    /* end of RSHA_BOARDTYPE_STRINGS */

static inline bool
is_good_rsha_boardtype(int type)
{
    return is_good_enum(type);
}

static inline char *
rsha_boardtype_string(int type)
{
    static char *array[RSHA_BOARDTYPE_END] = {RSHA_BOARDTYPE_STRINGS};

    return is_good_rsha_boardtype(type)?array[type]:__unknow;
}


static inline int
get_rsha_boardtype_env(void) 
{
    return get_int_env(ENV_RSHA_BOARDTYPE, RSHA_BOARDTYPE);
}

static inline int
get_rshd_path_env(char path[], int size) 
{
    return copy_string_env(ENV_RSHD_PATH, RSHD_PATH, path, size);
}

static inline int
get_rshw_path_env(int id /* worker id */, char path[], int size) 
{
    const char *env = get_string_env(ENV_RSHW_PATH, RSHW_PATH);
    if (os_strlen(env) > size - 1) {
        return os_assert_value(-ETOOBIG);
    }
    
    os_snprintf(path, size, env, id);
    debug_trace("unix path:%s", path);
    
    return 0;
}

static inline int
get_rshd_conf_env(char conf[], int size) 
{
    return copy_string_env(ENV_RSHD_CONF, RSHD_CONF, conf, size);
}

static inline int
get_rsha_conf_env(char conf[], int size) 
{
    return copy_string_env(ENV_RSHA_CONF, RSHA_CONF, conf, size);
}

static inline int
get_rsh_online_keepalive_env(void) 
{
    return get_int_env(ENV_RSH_ONLINE_KEEPALIVE, RSH_ONLINE_KEEPALIVE);
}

static inline int
get_rsh_online_interval_env(void) 
{
    return get_int_env(ENV_RSH_ONLINE_INTERVAL, RSH_ONLINE_INTERVAL);
}

static inline int
get_rsh_worker_keepalive_env(void) 
{
    return get_int_env(ENV_RSH_WORKER_KEEPALIVE, RSH_WORKER_KEEPALIVE);
}

static inline int
get_rsh_worker_interval_env(void) 
{
    return get_int_env(ENV_RSH_WORKER_INTERVAL, RSH_WORKER_INTERVAL);
}

/******************************************************************************/
#endif /* __RSH_ENV_H_C02C6E3CE5B1852D787859FAD6B1D087__ */

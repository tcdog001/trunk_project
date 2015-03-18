#ifndef __UM_H_CC431B9A6A7A07C3356E10656BDA3BDD__
#define __UM_H_CC431B9A6A7A07C3356E10656BDA3BDD__
/******************************************************************************/
#include <libubox/uloop.h>
#include <libubox/utils.h>
#include <uci_blob.h>
#include <libubus.h>

#ifndef UM_HASHSIZE
#define UM_HASHSIZE             64
#endif

#define UM_HASHMASK             (UM_HASHSIZE-1)

#ifndef UM_SCRIPT
#define UM_SCRIPT               "/etc/jsock/msg/umevent.sh"
#endif

#ifndef UM_TIMERMS_WIFI
#define UM_TIMERMS_WIFI         5000
#endif

#ifndef UM_TIMERMS_AGING
#define UM_TIMERMS_AGING        5000
#endif

#ifndef UM_TIMERMS_ONLINE
#define UM_TIMERMS_ONLINE       5000
#endif

#ifndef UM_TIMERMS_REPORT
#define UM_TIMERMS_REPORT       5000
#endif

#ifndef UM_TIMERMS_FLOW
#define UM_TIMERMS_FLOW         5000
#endif

#ifndef UM_AGING_TIMES
#define UM_AGING_TIMES          2
#endif

#if (UM_AGING_TIMES * UM_TIMERMS_AGING) <= UM_TIMERMS_FLOW
#error "(UM_AGING_TIMES * UM_TIMERMS_AGING) <= UM_TIMERMS_FLOW"
#endif

#ifndef UM_EV_CONNECT_DEFT
#define UM_EV_CONNECT_DEFT      OS_OFF
#endif

#ifndef UM_EV_DISCONNECT_DEFT
#define UM_EV_DISCONNECT_DEFT   OS_OFF
#endif

#ifndef UM_EV_BIND_DEFT
#define UM_EV_BIND_DEFT         OS_OFF
#endif

#ifndef UM_EV_UNBIND_DEFT
#define UM_EV_UNBIND_DEFT       OS_OFF
#endif

#ifndef UM_EV_AUTH_DEFT
#define UM_EV_AUTH_DEFT         OS_OFF
#endif

#ifndef UM_EV_DEAUTH_DEFT
#define UM_EV_DEAUTH_DEFT       OS_OFF
#endif

#ifndef UM_EV_UPDATE_DEFT
#define UM_EV_UPDATE_DEFT       OS_OFF
#endif

#ifndef UM_EV_REPORT_DEFT
#define UM_EV_REPORT_DEFT       OS_OFF
#endif

#ifndef UM_SH_CONNECT_DEFT
#define UM_SH_CONNECT_DEFT      OS_ON
#endif

#ifndef UM_SH_DISCONNECT_DEFT
#define UM_SH_DISCONNECT_DEFT   OS_ON
#endif

#ifndef UM_SH_BIND_DEFT
#define UM_SH_BIND_DEFT         OS_ON
#endif

#ifndef UM_SH_UNBIND_DEFT
#define UM_SH_UNBIND_DEFT       OS_ON
#endif

#ifndef UM_SH_AUTH_DEFT
#define UM_SH_AUTH_DEFT         OS_ON
#endif

#ifndef UM_SH_DEAUTH_DEFT
#define UM_SH_DEAUTH_DEFT       OS_ON
#endif

#ifndef UM_SH_UPDATE_DEFT
#define UM_SH_UPDATE_DEFT       OS_OFF
#endif

#ifndef UM_SH_REPORT_DEFT
#define UM_SH_REPORT_DEFT       OS_OFF
#endif

#define UM_STATE(x)     UM_STATE_##x
enum {
    UM_STATE_INVALID,
    
    UM_STATE_DISCONNECT,
    
    /*
    * connect wifi
    */
    UM_STATE_CONNECT,
    /*
    * get ip by dhcp
    */
    UM_STATE_BIND,
    /*
    * auth by portal
    */
    UM_STATE_AUTH,

    UM_STATE_END
};

#define UM_STATE_STRINGS                {   \
    [UM_STATE_INVALID]      = "invalid",    \
    [UM_STATE_DISCONNECT]   = "disconnect", \
    [UM_STATE_CONNECT]      = "connect",    \
    [UM_STATE_BIND]         = "bind",       \
    [UM_STATE_AUTH]         = "auth",       \
}   /* end */

static inline bool
is_good_um_user_state(int state)
{
    return is_good_value(state, UM_STATE_DISCONNECT, UM_STATE_END);
}

static inline bool
is_online_um_user_state(int state)
{
    return is_good_value(state, UM_STATE_CONNECT, UM_STATE_END);
}

static inline char **
__um_user_state_strings(void)
{
    static char *array[UM_STATE_END] = UM_STATE_STRINGS;

    return array;
}

static inline char *
um_user_state_string(int state)
{
    char **array = __um_user_state_strings();
    
    return is_good_um_user_state(state)?array[state]:__unknow;
}

static inline int
um_user_state_idx(char *state)
{
    char **array = __um_user_state_strings();
    
    return os_getstringarrayidx(array, state, UM_STATE_DISCONNECT, UM_STATE_END);
}

enum {
    UM_DEAUTH_ONLINETIME,
    UM_DEAUTH_FLOWLIMIT,
    UM_DEAUTH_ADMIN,
    UM_DEAUTH_INITIATIVE,

    UM_DEAUTH_END
};

#define UM_DEAUTH_REASONS               {   \
    [UM_DEAUTH_ONLINETIME]  = "onlinetime", \
    [UM_DEAUTH_FLOWLIMIT]   = "flowlimit",  \
    [UM_DEAUTH_ADMIN]       = "admin",      \
    [UM_DEAUTH_INITIATIVE]  = "initiative", \
}   /* end */

static inline bool
is_good_um_user_deauth_reason(int reason)
{
    return is_good_enum(reason, UM_DEAUTH_END);
}

static inline char **
__um_user_deauth_reason_strings(void)
{
    static char *array[UM_DEAUTH_END] = UM_DEAUTH_REASONS;

    return array;
}

static inline char *
um_user_deauth_reason_string(int reason)
{
    char **array = __um_user_deauth_reason_strings();

    return is_good_um_user_deauth_reason(reason)?array[reason]:__unknow;
}

static inline int
um_user_deauth_reason_idx(char *reason)
{
    char **array = __um_user_deauth_reason_strings();
    
    return os_getstringarrayidx(array, reason, 0, UM_STATE_END);
}

enum {
    UM_CLASS_NORMAL,
    UM_CLASS_VIP,

    UM_CLASS_END
};

#define UM_CLASS_STRINGS            {   \
    [UM_CLASS_NORMAL]   = "normal",     \
    [UM_CLASS_VIP]      = "vip",        \
}   /* end */

static inline bool
is_good_um_user_class(int class)
{
    return is_good_enum(class, UM_CLASS_END);
}

static inline char **
__um_user_class_strings(void)
{
    static char *array[UM_CLASS_END] = UM_CLASS_STRINGS;

    return array;
}

static inline char *
um_user_class_string(int class)
{
    char **array = __um_user_class_strings();

    return is_good_um_user_class(class)?array[class]:__unknow;
}

static inline int
um_user_class_idx(char *class)
{
    char **array = __um_user_class_strings();

    return os_getstringarrayidx(array, class, 0, UM_CLASS_END);
}

enum {
    UM_LEVEL_NORMAL,
    UM_LEVEL_LOW,

    UM_LEVEL_END
};

#define UM_LEVEL_STRINGS        {   \
    [UM_LEVEL_NORMAL]   = "normal", \
    [UM_LEVEL_LOW]      = "low",    \
}   /* end */

static inline bool
is_good_um_user_level(int level)
{
    return is_good_enum(level, UM_LEVEL_END);
}

static inline char **
__um_user_level_strings(void)
{
    static char *array[UM_LEVEL_END] = UM_LEVEL_STRINGS;

    return array;
}

static inline char *
um_user_level_string(int level)
{
    char **array = __um_user_level_strings();

    return is_good_um_user_level(level)?array[level]:__unknow;
}

static inline int
um_user_level_idx(char *level)
{
    char **array = __um_user_level_strings();

    return os_getstringarrayidx(array, level, 0, UM_LEVEL_END);
}

struct userlimit {
    uint32_t online;
    
    struct {
        struct {
            uint64_t max;
        } flow;

        struct {
            uint32_t max;
            uint32_t avg;
        } rate;
    } up, down, all;
};

struct userinfo {
    uint32_t uptime;
    
    struct {
        struct {
            uint64_t total;
            uint64_t cache;
        } flow;

        struct {
            uint32_t now;
        } rate;
    } up, down, all;
};

struct apuser {
    int state;
    int aging;
    uint32_t ip;
    
    unsigned char mac[OS_MACSIZE];
    unsigned char ap[OS_MACSIZE];
    unsigned char vap[OS_MACSIZE];
    
    char ifname[1+OS_IFNAMELEN]; /* wlan ifname */
    struct um_intf *intf;
    int radioid;
    int wlanid;
    
    int class;
    int level;
    int reason;
    
    struct {
        struct userlimit wifi, auth;
    } limit;
    
    struct {
        struct userinfo wifi, auth;
    } info;
    
    struct {
        struct hlist_node mac; /* hash node in umc.hash */
        struct hlist_node ip; /* hash node in umc.hash */
        struct list_head  list; /* list node in umc.list */
    } node;
};
#define UM_USER_ENTRY_SIZE  offsetof(struct apuser, node)

static inline void
um_user_init(struct apuser *user)
{
    os_objzero(user);

    user->state = UM_STATE_DISCONNECT;
    
    INIT_HLIST_NODE(&user->node.mac);
    INIT_HLIST_NODE(&user->node.ip);
    INIT_LIST_HEAD(&user->node.list);
}

typedef multi_value_t um_foreach_f(struct apuser *user);
typedef multi_value_t um_get_f(struct apuser *user);

#define UM_POLICY_INITER(_id, _name, _type) \
        [_id] = { .name = _name, .type = BLOBMSG_TYPE_##_type }

#define UM_PARAM_INITER(_policy) \
        { .params = _policy, .n_params = os_count_of(_policy) }

#define __UM_USER_INFO(_a, _b)          UM_USER_INFO_##_a##_##_b
#define __UM_USER_LIMIT(_a, _b)         UM_USER_LIMIT_##_a##_##_b
#define UM_USER_INFO(_a, _b, _c, _d)    UM_USER_INFO_##_a##_##_b##_##_c##_##_d
#define UM_USER_LIMIT(_a, _b, _c, _d)   UM_USER_LIMIT_##_a##_##_b##_##_c##_##_d

#define __UM_USER_LISTS(_obj) \
    UM_USER_##_obj##_##UPTIME, \
    UM_USER_INFO(_obj, UPFLOWTOTAL),    \
    UM_USER_INFO(_obj, up, flow, cache),    \
    UM_USER_INFO(_obj, down, flow, total),  \
    UM_USER_INFO(_obj, down, flow, cache),  \
    UM_USER_INFO(_obj, all, flow, total),   \
    UM_USER_INFO(_obj, all, flow, cache),   \
                                            \
    __UM_USER_LIMIT(_obj, online),          \
    UM_USER_LIMIT(_obj, up, flow, max),     \
    UM_USER_LIMIT(_obj, up, rate, max),     \
    UM_USER_LIMIT(_obj, up, rate, avg),     \
    UM_USER_LIMIT(_obj, down, flow, max),   \
    UM_USER_LIMIT(_obj, down, rate, max),   \
    UM_USER_LIMIT(_obj, down, rate, avg),   \
    UM_USER_LIMIT(_obj, all, flow, max),    \
    UM_USER_LIMIT(_obj, all, rate, max),    \
    UM_USER_LIMIT(_obj, all, rate, avg)     \
    /* end */

#define UM_USER(_OBJ)          \
    UM_USER_##_OBJ##_UPTIME,        \
    UM_USER_##_OBJ##_ONLINE,        \
    UM_USER_##_OBJ##_UPFLOWTOTAL,   \
    UM_USER_##_OBJ##_UPFLOWCACHE,   \
    UM_USER_##_OBJ##_UPFLOWMAX,     \
    UM_USER_##_OBJ##_UPRATENOW,     \
    UM_USER_##_OBJ##_UPRATEMAX,     \
    UM_USER_##_OBJ##_UPRATEAVG,     \
    UM_USER_##_OBJ##_DOWNFLOWTOTAL, \
    UM_USER_##_OBJ##_DOWNFLOWCACHE, \
    UM_USER_##_OBJ##_DOWNFLOWMAX,   \
    UM_USER_##_OBJ##_DOWNRATENOW,   \
    UM_USER_##_OBJ##_DOWNRATEMAX,   \
    UM_USER_##_OBJ##_DOWNRATEAVG,   \
    UM_USER_##_OBJ##_ALLFLOWTOTAL,  \
    UM_USER_##_OBJ##_ALLFLOWCACHE,  \
    UM_USER_##_OBJ##_ALLFLOWMAX,    \
    UM_USER_##_OBJ##_ALLRATENOW,    \
    UM_USER_##_OBJ##_ALLRATEMAX,    \
    UM_USER_##_OBJ##_ALLRATEAVG     \
    /* end */
    
enum {
	UM_USER_AP,
	UM_USER_VAP,
	UM_USER_WLANID,
	UM_USER_RADIOID,
	UM_USER_IFNAME,
	UM_USER_MAC,
	UM_USER_IP,
	UM_USER_STATE,
	UM_USER_CLASS,
	UM_USER_LEVEL,
	UM_USER_REASON,
	
    UM_USER(WIFI),
    UM_USER(AUTH),
    
	UM_USER_END,
};

#define __UM_USER_POLICY_INITER(_OBJ, _obj) \
    UM_POLICY_INITER(UM_USER_##_OBJ##_UPTIME,       #_obj ".uptime",        INT32), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_ONLINE,       #_obj ".online",        INT32), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_UPFLOWTOTAL,  #_obj ".upflowtotal",   INT64), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_UPFLOWCACHE,  #_obj ".upflowcache",   INT64), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_UPFLOWMAX,    #_obj ".upflowmax",     INT64), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_UPRATENOW,    #_obj ".upratenow",     INT32), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_UPRATEMAX,    #_obj ".upratemax",     INT32), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_UPRATEAVG,    #_obj ".uprateavg",     INT32), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_DOWNFLOWTOTAL,#_obj ".downflowtotal", INT64), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_DOWNFLOWCACHE,#_obj ".downflowcache", INT64), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_DOWNFLOWMAX,  #_obj ".downflowmax",   INT64), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_DOWNRATENOW,  #_obj ".downratenow",   INT32), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_DOWNRATEMAX,  #_obj ".downratemax",   INT32), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_DOWNRATEAVG,  #_obj ".downrateavg",   INT32), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_ALLFLOWTOTAL, #_obj ".allflowtotal",  INT64), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_ALLFLOWCACHE, #_obj ".allflowcache",  INT64), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_ALLFLOWMAX,   #_obj ".allflowmax",    INT64), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_ALLRATENOW,   #_obj ".allratenow",    INT32), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_ALLRATEMAX,   #_obj ".allratemax",    INT32), \
    UM_POLICY_INITER(UM_USER_##_OBJ##_ALLRATEAVG,   #_obj ".allrateavg",    INT32) \
    /* end */

#define UM_USER_POLICY_INITER  { \
    UM_POLICY_INITER(UM_USER_AP,        "ap",       STRING), /* "XX:XX:XX:XX:XX:XX" */  \
    UM_POLICY_INITER(UM_USER_VAP,       "vap",      STRING), /* "XX:XX:XX:XX:XX:XX" */  \
    UM_POLICY_INITER(UM_USER_WLANID,    "wlanid",   INT32),     \
    UM_POLICY_INITER(UM_USER_RADIOID,   "radioid",  INT32),     \
    UM_POLICY_INITER(UM_USER_IFNAME,    "ifname",   STRING),    \
    UM_POLICY_INITER(UM_USER_MAC,       "mac",      STRING), /* "XX:XX:XX:XX:XX:XX" */  \
    UM_POLICY_INITER(UM_USER_IP,        "ip",       STRING), /* "xxx.xxx.xxx.xxx" */    \
    UM_POLICY_INITER(UM_USER_STATE,     "state",    STRING), \
    UM_POLICY_INITER(UM_USER_CLASS,     "class",    STRING), \
    UM_POLICY_INITER(UM_USER_LEVEL,     "level",    STRING), \
    UM_POLICY_INITER(UM_USER_REASON,    "reason",   STRING), \
    \
    __UM_USER_POLICY_INITER(WIFI, wifi), \
    __UM_USER_POLICY_INITER(AUTH, auth), \
}   /* end */

enum {
    UM_GETUSER_STATE,
    
    UM_GETUSER_AP,
    UM_GETUSER_APMASK,
    
    UM_GETUSER_MAC,
    UM_GETUSER_MACMASK,
    
    UM_GETUSER_IP,
    UM_GETUSER_IPMASK,
    
    UM_GETUSER_RADIO,
    UM_GETUSER_WLAN,

    UM_GETUSER_END,
};

#define UM_GETUSER_POLICY_INITER    { \
    UM_POLICY_INITER(UM_GETUSER_STATE,    "state",    STRING), \
    UM_POLICY_INITER(UM_GETUSER_AP,       "ap",       STRING), \
    UM_POLICY_INITER(UM_GETUSER_APMASK,   "apmask",   STRING), \
    UM_POLICY_INITER(UM_GETUSER_MAC,      "mac",      STRING), \
    UM_POLICY_INITER(UM_GETUSER_MACMASK,  "macmask",  STRING), \
    UM_POLICY_INITER(UM_GETUSER_IP,       "ip",       STRING), \
    UM_POLICY_INITER(UM_GETUSER_IPMASK,   "ipmask",   STRING), \
    UM_POLICY_INITER(UM_GETUSER_RADIO,    "radio",    INT32),  \
    UM_POLICY_INITER(UM_GETUSER_WLAN,     "wlan",     INT32),  \
}   /* end */

#define UM_INTFPOLICY_DISABLE   0
#define UM_INTFPOLICY_UM        1

enum {
    UM_RADIOPOLICY_DISABLE  = UM_INTFPOLICY_DISABLE,
    UM_RADIOPOLICY_UM       = UM_INTFPOLICY_UM,
    
    UM_RADIOPOLICY_END
};

#define UM_RADIOPOLICY_INITER    { \
	UM_POLICY_INITER(UM_RADIOPOLICY_DISABLE,"disable",  BOOL),  \
	UM_POLICY_INITER(UM_RADIOPOLICY_UM,     "um",       BOOL),  \
}

enum {
    UM_WLANPOLICY_DISABLE   = UM_INTFPOLICY_DISABLE,
    UM_WLANPOLICY_UM        = UM_INTFPOLICY_UM,
    UM_WLANPOLICY_IFNAME,

    UM_WLANPOLICY_END
};

#define UM_WLANPOLICY_INITER    { \
	UM_POLICY_INITER(UM_WLANPOLICY_DISABLE, "disable",  BOOL), \
	UM_POLICY_INITER(UM_WLANPOLICY_UM,      "um",       BOOL), \
	UM_POLICY_INITER(UM_WLANPOLICY_IFNAME,  "ifname",   STRING), \
}

enum {
    UM_LIMITPOLICY_ONLINE,
    UM_LIMITPOLICY_UPFLOWMAX,
    UM_LIMITPOLICY_UPRATEMAX,
    UM_LIMITPOLICY_UPRATEAVG,
    UM_LIMITPOLICY_DOWNFLOWMAX,
    UM_LIMITPOLICY_DOWNRATEMAX,
    UM_LIMITPOLICY_DOWNRATEAVG,
    UM_LIMITPOLICY_ALLFLOWMAX,
    UM_LIMITPOLICY_ALLRATEMAX,
    UM_LIMITPOLICY_ALLRATEAVG,

    UM_LIMITPOLICY_END
};

#define UM_LIMITPOLICY_INITER    { \
	UM_POLICY_INITER(UM_LIMITPOLICY_ONLINE,     "online",       INT32), \
	                                                                    \
	UM_POLICY_INITER(UM_LIMITPOLICY_UPFLOWMAX,  "upflowmax",    INT64), \
	UM_POLICY_INITER(UM_LIMITPOLICY_UPRATEMAX,  "upratemax",    INT32), \
	UM_POLICY_INITER(UM_LIMITPOLICY_UPRATEAVG,  "uprateavg",    INT32), \
	                                                                    \
	UM_POLICY_INITER(UM_LIMITPOLICY_DOWNFLOWMAX,"downflowmax",  INT64), \
	UM_POLICY_INITER(UM_LIMITPOLICY_DOWNRATEMAX,"downratemax",  INT32), \
	UM_POLICY_INITER(UM_LIMITPOLICY_DOWNRATEAVG,"downrateavg",  INT32), \
	                                                                    \
	UM_POLICY_INITER(UM_LIMITPOLICY_ALLFLOWMAX, "allflowmax",   INT64), \
	UM_POLICY_INITER(UM_LIMITPOLICY_ALLRATEMAX, "allratemax",   INT32), \
	UM_POLICY_INITER(UM_LIMITPOLICY_ALLRATEAVG, "allrateavg",   INT32), \
}   /* end */

struct um_timer {
    struct uloop_timeout tm;
    appkey_t akid;
    unsigned int deft;
};

#define UM_TIMER_INITER(_deft, _cb) { \
    .tm = {                         \
        .cb = _cb,                  \
    },                              \
    .deft = _deft,                  \
}   /* end */

enum {
    UM_INTF_RADIO,
    UM_INTF_WLAN,

    UM_INTF_END
};

#define UM_UCI_INTF_RADIO   "wifi-device"
#define UM_UCI_INTF_WLAN    "wifi-iface"
#define UM_UCI_LIMIT        "limit"

struct um_intf {
    /*
    * uci ifname
    */
    int type;
    char ifname[1+OS_IFNAMELEN];
    unsigned char mac[OS_MACSIZE];
    int radioid;
    int wlanid;
    
    struct list_head node;
};

struct um_uci {
    struct uci_blob_param_list param;
    struct list_head cfg;
    struct list_head tmp;
    char *uci_type;
};

struct um_control {
    unsigned char basemac[OS_MACSIZE]; /* local ap's base mac */

    struct {
        struct hlist_head mac[UM_HASHSIZE];
        struct hlist_head ip[UM_HASHSIZE];
        struct list_head list;
        uint32_t count;
    } head;

    struct ubus_context *ctx;
    char *path;
    
    struct {
        struct um_timer begin;
        
        struct um_timer wifi;
        struct um_timer aging;
        struct um_timer online;
        struct um_timer flow;
        struct um_timer report;
        
        struct um_timer end;
    } timer;

    struct {
        struct {
            appkey_t akid;
            unsigned int deft;
        }
        report, /* just for ev */
        update, /* no support now */
        connect, disconnect, 
        bind, unbind, 
        auth, deauth;
    } ev, sh;
    
    struct {
        struct blobmsg_policy getuser[UM_GETUSER_END];
        struct blobmsg_policy user[UM_USER_END];
        struct blobmsg_policy radio[UM_RADIOPOLICY_END];
        struct blobmsg_policy wlan[UM_WLANPOLICY_END];
        struct blobmsg_policy limit[UM_LIMITPOLICY_END];
    } policy;

    struct {
        struct userlimit wifi, auth;
    } limit[UM_CLASS_END][UM_LEVEL_END];
    
    struct {
        struct uci_context *ctx;
        
        struct {
            struct um_uci radio, wlan;
        } intf;
        
        struct {
            struct um_uci wifi, auth;
        } limit[UM_CLASS_END][UM_LEVEL_END];
    } uci;
    
    struct {
        struct ubus_object object;
        struct ubus_object_type type;
        struct ubus_method *methods;
        struct ubus_subscriber subscriber;
    } ubus;
    
    struct {
        appkey_t uci;
        appkey_t ubus;
        appkey_t user;
        appkey_t userscan;
        appkey_t flowscan;
    } debug;
};

struct user_filter {
    int state;
    
    unsigned char ap[OS_MACSIZE];
    unsigned char apmask[OS_MACSIZE];  /* zero, not use ap as filter */
    
    unsigned char mac[OS_MACSIZE];
    unsigned char macmask[OS_MACSIZE]; /* zero, not use mac as filter */
    
    uint32_t ip;
    uint32_t ipmask;/* zero, not use ip as filter */
    
    int radioid;    /* <0, not use radioid as filter */
    int wlanid;     /* <0, not use wlanid as filter */
};

#define USER_FILTER_INITER  {   \
    .radioid= -1,               \
    .wlanid = -1,               \
}   /* end */
/******************************************************************************/
extern struct um_intf *
um_intf_get(char *ifname);

extern int
um_uci_load(void);
/******************************************************************************/
extern int
um_ubus_handle_restart(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
);

extern int
um_ubus_handle_reload(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
);

extern int
um_ubus_handle_getuser(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
);

extern int
um_ubus_handle_connect(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
);

extern int
um_ubus_handle_disconnect(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
);

extern int
um_ubus_handle_bind(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
);

extern int
um_ubus_handle_unbind(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
);

extern int
um_ubus_handle_auth(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
);

extern int
um_ubus_handle_deauth(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
);

extern int
um_ubus_report(void);

extern void 
um_ubus_common_notify(struct apuser *user, char *event);

extern void 
um_ubus_deauth_notify(struct apuser *user, int reason);

extern void
um_ubus_update_notify(struct apuser *old, struct apuser *new);

typedef void um_user_update_f(bool created, struct apuser *dst, struct apuser *src);

extern int 
um_ubus_init(char *path);

extern void
um_ubus_fini(void);
/******************************************************************************/
extern void
um_user_dump(struct apuser *user, char *action);

extern struct apuser *
user_connect(unsigned char mac[], struct um_intf *intf);

extern void
user_disconnect(struct apuser *user);

extern struct apuser *
user_bind(struct apuser *user, uint32_t ip, struct um_intf *intf);

extern void
user_unbind(struct apuser *user);

extern struct apuser *
user_auth(struct apuser *user, int class);

extern void
user_deauth(struct apuser *user, int reason);

extern struct apuser *
um_user_connect(unsigned char mac[], char *ifname);

extern void
um_user_disconnect(unsigned char mac[]);

extern struct apuser *
um_user_bind(unsigned char mac[], uint32_t ip, char *ifname);

extern void
um_user_unbind(unsigned char mac[]);

extern struct apuser *
um_user_auth(unsigned char mac[], int class);

extern void
um_user_deauth(unsigned char mac[], int reason);

extern struct apuser *
um_user_update(struct apuser *old, struct apuser *new);

extern int
um_user_foreach(um_foreach_f *foreach);

extern struct apuser *
um_user_getbymac(unsigned char mac[]);

extern struct apuser *
um_user_getbyip(uint32_t ip);

extern int
um_user_getby(struct user_filter *filter, um_get_f *get);

extern int
um_user_del(struct apuser *user);

extern int
um_user_delbymac(unsigned char mac[]);

extern int
um_user_delbyip(uint32_t ip);

extern int
um_user_delby(struct user_filter *filter);

extern int 
um_user_update_limit(void);
/******************************************************************************/
extern int
um_user_scan(void);

extern struct um_control umc;
extern struct blob_buf b;

static inline void
um_blob_buf_init(void)
{
    blob_buf_init(&b, 0);
}

#define um_is_ev_enable(_var)       appkey_get(umc.ev._var.akid, umc.ev._var.deft)
#define um_is_sh_enable(_var)       appkey_get(umc.sh._var.akid, umc.sh._var.deft)
#define um_user_policy_name(_id)    umc.policy.user[_id].name

#define um_open_table(_name)        blobmsg_open_table(&b, _name)
#define um_open_array(_name)        blobmsg_open_array(&b, _name)
#define um_close_table(_handle)     blobmsg_close_table(&b, _handle)
#define um_close_array(_handle)     blobmsg_close_array(&b, _handle)

#define um_add_bool(_name, _val)    blobmsg_add_u8(&b, _name, _val)
#define um_add_string(_name, _val)  blobmsg_add_string(&b, _name, _val)
#define um_add_u32(_name, _val)     blobmsg_add_u32(&b, _name, _val)
#define um_add_u64(_name, _val)     blobmsg_add_u64(&b, _name, _val)

#define um_user_add_bool(_id, _val)         um_add_bool(um_user_policy_name(_id), _val)
#define um_user_add_string(_id, _val)       um_add_string(um_user_policy_name(_id), _val)
#define um_user_add_u32(_id, _val)          um_add_u32(um_user_policy_name(_id), _val)
#define um_user_add_u64(_id, _val)          um_add_u64(um_user_policy_name(_id), _val)
#define um_user_add_macstring(_id, _mac)    um_user_add_string(_id, os_macstring(mac))

#define um_ubus_send_reply(_ctx, _req)      ubus_send_reply(_ctx, _req, b.head)

/******************************************************************************/
#define um_debug(var, _fmt, _args...)   do{ \
    if (appkey_get(umc.debug.var, 0)) {     \
        __debug_with_prefix(_fmt, ##_args); \
    }                                       \
}while(0)

#define um_debug_ok(var, _fmt, _args...) do{\
    if (appkey_get(umc.debug.var, 0)) {     \
        debug_ok(_fmt, ##_args);            \
    }                                       \
}while(0)

#define um_debug_error(var, _fmt, _args...) do{ \
    if (appkey_get(umc.debug.var, 0)) { \
        debug_error(_fmt, ##_args);     \
    }                                   \
}while(0)

#define um_debug_trace(var, _fmt, _args...)   do{ \
    if (appkey_get(umc.debug.var, 0)) { \
        debug_trace(_fmt, ##_args);     \
    }                                   \
}while(0)

#define um_debug_test(var, _fmt, _args...)    do{ \
    if (appkey_get(umc.debug.var, 0)) { \
        debug_test(_fmt, ##_args);      \
    }                                   \
}while(0)

#define debug_uci_ok(_fmt, _args...)        um_debug_ok(uci, _fmt, ##_args)
#define debug_uci_error(_fmt, _args...)     um_debug_error(uci, _fmt, ##_args)
#define debug_uci_trace(_fmt, _args...)     um_debug_trace(uci, _fmt, ##_args)
#define debug_uci_test(_fmt, _args...)      um_debug_test(uci, _fmt, ##_args)

#define debug_ubus_ok(_fmt, _args...)       um_debug_ok(ubus, _fmt, ##_args)
#define debug_ubus_error(_fmt, _args...)    um_debug_error(ubus, _fmt, ##_args)
#define debug_ubus_trace(_fmt, _args...)    um_debug_trace(ubus, _fmt, ##_args)
#define debug_ubus_test(_fmt, _args...)     um_debug_test(ubus, _fmt, ##_args)

#define debug_user_ok(_fmt, _args...)       um_debug_ok(user, _fmt, ##_args)
#define debug_user_error(_fmt, _args...)    um_debug_error(user, _fmt, ##_args)
#define debug_user_trace(_fmt, _args...)    um_debug_trace(user, _fmt, ##_args)
#define debug_user_test(_fmt, _args...)     um_debug_test(user, _fmt, ##_args)

#define debug_userscan_ok(_fmt, _args...)       um_debug_ok(userscan, _fmt, ##_args)
#define debug_userscan_error(_fmt, _args...)    um_debug_error(userscan, _fmt, ##_args)
#define debug_userscan_trace(_fmt, _args...)    um_debug_trace(userscan, _fmt, ##_args)
#define debug_userscan_test(_fmt, _args...)     um_debug_test(userscan, _fmt, ##_args)

#define debug_flowscan_ok(_fmt, _args...)       um_debug_ok(flowscan, _fmt, ##_args)
#define debug_flowscan_error(_fmt, _args...)    um_debug_error(flowscan, _fmt, ##_args)
#define debug_flowscan_trace(_fmt, _args...)    um_debug_trace(flowscan, _fmt, ##_args)
#define debug_flowscan_test(_fmt, _args...)     um_debug_test(flowscan, _fmt, ##_args)
/******************************************************************************/
#endif /* __UM_H_CC431B9A6A7A07C3356E10656BDA3BDD__ */

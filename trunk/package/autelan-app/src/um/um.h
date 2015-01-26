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
#define UM_SCRIPT               "/usr/sbin/umnotify"
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

enum {
    UM_USER_STATE_DISCONNECT = 1,
    
    /*
    * connect wifi
    */
    UM_USER_STATE_CONNECT,
    /*
    * get ip by dhcp
    */
    UM_USER_STATE_BIND,
    /*
    * auth by portal
    */
    UM_USER_STATE_AUTH,

    UM_USER_STATE_END
};

#define UM_USER_STRINGS                         \
    [0] = "all",                                \
    [UM_USER_STATE_DISCONNECT]  = "disconnect", \
    [UM_USER_STATE_CONNECT]     = "connect",    \
    [UM_USER_STATE_BIND]        = "bind",       \
    [UM_USER_STATE_AUTH]        = "auth",       \
    /* end of UM_USER_STRINGS */

static inline bool
is_good_um_user_state(int statet)
{
    return is_good_value(statet, UM_USER_STATE_DISCONNECT, UM_USER_STATE_END);
}

static inline char *
um_user_state_string(int state)
{
    static char *array[UM_USER_STATE_END] = {UM_USER_STRINGS};
    
    return os_enum_string(is_good_um_user_state, array, state);
}

static inline int
um_user_state_idx(char *state)
{
    static char *array[UM_USER_STATE_END] = {UM_USER_STRINGS};

    return os_getarraystringidx(array, state, 
                UM_USER_STATE_DISCONNECT, 
                UM_USER_STATE_END);
}

enum {
    UM_USER_DEAUTH_ONLINETIME,
    UM_USER_DEAUTH_FLOWLIMIT,
    UM_USER_DEAUTH_ADMIN,
    UM_USER_DEAUTH_INITIATIVE,

    UM_USER_DEAUTH_END
};

#define UM_USER_DEAUTH_REASONS                  \
    [UM_USER_DEAUTH_ONLINETIME] = "onlinetime", \
    [UM_USER_DEAUTH_FLOWLIMIT]  = "flowlimit",  \
    [UM_USER_DEAUTH_ADMIN]      = "admin",      \
    [UM_USER_DEAUTH_INITIATIVE] = "initiative", \
    /* end of UM_USER_DEAUTH_REASONS */

static inline bool
is_good_um_user_deauth_reason(int statet)
{
    return is_good_enum(statet, UM_USER_DEAUTH_END);
}

static inline char *
um_user_deauth_reason_string(int reason)
{
    static char *array[UM_USER_DEAUTH_END] = {UM_USER_DEAUTH_REASONS};

    return os_enum_string(is_good_um_user_deauth_reason, array, reason);
}

struct apuser {
    bool local;
    int state;
    int aging;
    uint32_t ip;

    byte mac[OS_MACSIZE];
    byte ap[OS_MACSIZE];
    byte vap[OS_MACSIZE];
    
    char ifname[1+OS_IFNAMELEN]; /* wlan ifname */
    struct um_intf *intf;
    int radioid;
    int wlanid;
    
    struct {
        uint32_t uptime;
        uint32_t onlinelimit;   /* just for auth */
        int      signal;        /* just for wifi */
        
        struct {
            uint64_t flowtotal;
            uint64_t flowlimit; /* just for auth */
            uint32_t ratelimit;
        } up, down, all;
    } wifi, auth;
    
    struct {
        struct hlist_node mac; /* hash node in umc.hash */
        struct hlist_node ip; /* hash node in umc.hash */
        struct list_head  list; /* list node in umc.list */
    } node;
};
#define UM_USER_ENTRY_SIZE  offsetof(struct apuser, node)

static inline void
um_user_init(struct apuser *user, bool local)
{
    os_objzero(user);

    user->local = local;
    
    INIT_HLIST_NODE(&user->node.mac);
    INIT_HLIST_NODE(&user->node.ip);
    INIT_LIST_HEAD(&user->node.list);
}

typedef multi_value_t um_foreach_f(struct apuser *user, void *data);
typedef multi_value_t um_get_f(struct apuser *user, void *data);

#define UM_POLICY_INITER(_id, _name, _type) \
        [_id] = { .name = _name, .type = BLOBMSG_TYPE_##_type }
#define UM_PARAM_INITER(_policy) \
        { .params = _policy, .n_params = os_count_of(_policy) }

enum {
	UM_USER_AP,
	UM_USER_VAP,
	UM_USER_WLANID,
	UM_USER_RADIOID,
	UM_USER_IFNAME,
	UM_USER_MAC,
	UM_USER_IP,
	UM_USER_STATE,
	UM_USER_LOCAL,
	
	UM_USER_WIFI_UPTIME,
	UM_USER_WIFI_SIGNAL,
	UM_USER_WIFI_ONLINELIMIT,
    UM_USER_WIFI_UPFLOWTOTAL,
    UM_USER_WIFI_UPFLOWLIMIT,
    UM_USER_WIFI_UPRATELIMIT,
    UM_USER_WIFI_DOWNFLOWTOTAL,
    UM_USER_WIFI_DOWNFLOWLIMIT,
    UM_USER_WIFI_DOWNRATELIMIT,
    UM_USER_WIFI_ALLFLOWTOTAL,
    UM_USER_WIFI_ALLFLOWLIMIT,
    UM_USER_WIFI_ALLRATELIMIT,
	
	UM_USER_AUTH_UPTIME,
	UM_USER_AUTH_SIGNAL,
	UM_USER_AUTH_ONLINELIMIT,
    UM_USER_AUTH_UPFLOWTOTAL,
    UM_USER_AUTH_UPFLOWLIMIT,
    UM_USER_AUTH_UPRATELIMIT,
    UM_USER_AUTH_DOWNFLOWTOTAL,
    UM_USER_AUTH_DOWNFLOWLIMIT,
    UM_USER_AUTH_DOWNRATELIMIT,
    UM_USER_AUTH_ALLFLOWTOTAL,
    UM_USER_AUTH_ALLFLOWLIMIT,
    UM_USER_AUTH_ALLRATELIMIT,
    
	UM_USER_END,
};

#define UM_USER_POLICY_INITER  { \
    UM_POLICY_INITER(UM_USER_AP,            "ap",           STRING), /* "XX:XX:XX:XX:XX:XX" */ \
    UM_POLICY_INITER(UM_USER_VAP,           "vap",          STRING), /* "XX:XX:XX:XX:XX:XX" */ \
    UM_POLICY_INITER(UM_USER_WLANID,        "wlanid",       INT32), \
    UM_POLICY_INITER(UM_USER_RADIOID,       "radioid",      INT32), \
    UM_POLICY_INITER(UM_USER_MAC,           "mac",          STRING), /* "XX:XX:XX:XX:XX:XX" */ \
    UM_POLICY_INITER(UM_USER_IP,            "ip",           STRING), /* "xxx.xxx.xxx.xxx" */ \
    UM_POLICY_INITER(UM_USER_STATE,         "state",        STRING), \
    UM_POLICY_INITER(UM_USER_LOCAL,         "local",        BOOL), \
    \
    UM_POLICY_INITER(UM_USER_WIFI_UPTIME,       "wifi_uptime",  INT32), \
    UM_POLICY_INITER(UM_USER_WIFI_SIGNAL,       "wifi_signal",  INT32), \
    UM_POLICY_INITER(UM_USER_WIFI_ONLINELIMIT,  "wifi_onlinelimit",  INT32), \
    UM_POLICY_INITER(UM_USER_WIFI_UPFLOWTOTAL,  "wifi_upflowtotal",  INT64), \
    UM_POLICY_INITER(UM_USER_WIFI_UPFLOWLIMIT,  "wifi_upflowlimit",  INT64), \
    UM_POLICY_INITER(UM_USER_WIFI_UPRATELIMIT,  "wifi_upratelimit",  INT32), \
    UM_POLICY_INITER(UM_USER_WIFI_DOWNFLOWTOTAL,"wifi_downflowtotal",  INT64), \
    UM_POLICY_INITER(UM_USER_WIFI_DOWNFLOWLIMIT,"wifi_downflowlimit",  INT64), \
    UM_POLICY_INITER(UM_USER_WIFI_DOWNRATELIMIT,"wifi_downratelimit",  INT32), \
    UM_POLICY_INITER(UM_USER_WIFI_ALLFLOWTOTAL, "wifi_allflowtotal",  INT64), \
    UM_POLICY_INITER(UM_USER_WIFI_ALLFLOWLIMIT, "wifi_allflowlimit",  INT64), \
    UM_POLICY_INITER(UM_USER_WIFI_ALLRATELIMIT, "wifi_allratelimit",  INT32), \
    \
    UM_POLICY_INITER(UM_USER_AUTH_UPTIME,       "auth_uptime",  INT32), \
    UM_POLICY_INITER(UM_USER_AUTH_SIGNAL,       "auth_signal",  INT32), \
    UM_POLICY_INITER(UM_USER_AUTH_ONLINELIMIT,  "auth_onlinelimit",  INT32), \
    UM_POLICY_INITER(UM_USER_AUTH_UPFLOWTOTAL,  "auth_upflowtotal",  INT64), \
    UM_POLICY_INITER(UM_USER_AUTH_UPFLOWLIMIT,  "auth_upflowlimit",  INT64), \
    UM_POLICY_INITER(UM_USER_AUTH_UPRATELIMIT,  "auth_upratelimit",  INT32), \
    UM_POLICY_INITER(UM_USER_AUTH_DOWNFLOWTOTAL,"auth_downflowtotal",  INT64), \
    UM_POLICY_INITER(UM_USER_AUTH_DOWNFLOWLIMIT,"auth_downflowlimit",  INT64), \
    UM_POLICY_INITER(UM_USER_AUTH_DOWNRATELIMIT,"auth_downratelimit",  INT32), \
    UM_POLICY_INITER(UM_USER_AUTH_ALLFLOWTOTAL, "auth_allflowtotal",  INT64), \
    UM_POLICY_INITER(UM_USER_AUTH_ALLFLOWLIMIT, "auth_allflowlimit",  INT64), \
    UM_POLICY_INITER(UM_USER_AUTH_ALLRATELIMIT, "auth_allratelimit",  INT32), \
}

enum {
    UM_GETUSER_LOCAL,
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
	UM_POLICY_INITER(UM_GETUSER_LOCAL,  "local",    BOOL),      \
	UM_POLICY_INITER(UM_GETUSER_STATE,  "state",    STRING),    \
	UM_POLICY_INITER(UM_GETUSER_AP,     "ap",       STRING),    \
	UM_POLICY_INITER(UM_GETUSER_APMASK, "apmask",   STRING),    \
	UM_POLICY_INITER(UM_GETUSER_MAC,    "mac",      STRING),    \
	UM_POLICY_INITER(UM_GETUSER_MACMASK,"macmask",  STRING),    \
	UM_POLICY_INITER(UM_GETUSER_IP,     "ip",       STRING),    \
	UM_POLICY_INITER(UM_GETUSER_IPMASK, "ipmask",   STRING),    \
	UM_POLICY_INITER(UM_GETUSER_RADIO,  "radio",    INT32),     \
	UM_POLICY_INITER(UM_GETUSER_WLAN,   "wlan",     INT32),     \
}

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
}

enum {
    UM_INTF_RADIO,
    UM_INTF_WLAN,

    UM_INTF_END
};

#define UM_UCI_INTF_RADIO   "wifi-device"
#define UM_UCI_INTF_WLAN    "wifi-iface"

struct um_intf {
    /*
    * uci ifname
    */
    int type;
    char ifname[1+OS_IFNAMELEN];
    byte mac[OS_MACSIZE];
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
    byte basemac[OS_MACSIZE]; /* local ap's base mac */

    struct {
        struct hlist_head mac[UM_HASHSIZE];
        struct hlist_head ip[UM_HASHSIZE];
        struct list_head list;
        uint32_t count;
    } head;

    struct ubus_context *ctx;
    char *path;
    
    struct {
        struct um_timer wifi;
        struct um_timer aging;
        struct um_timer online;
        struct um_timer flow;
        struct um_timer report;
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
    } policy;

    struct {
        struct uci_context *ctx;

        struct um_uci radio;
        struct um_uci wlan;
    } uci;
    
    struct {
        struct ubus_object object;
        struct ubus_object_type type;
        struct ubus_method *methods;
        struct ubus_subscriber subscriber;
    } obj;

    struct {
        appkey_t uci;
        appkey_t ubus;
        appkey_t user;
        appkey_t userscan;
        appkey_t flowscan;
    } debug;
};

struct user_filter {
    /*
    * true: just match local user
    * false: match all user
    */
    bool local;
    int state;
    
    byte ap[OS_MACSIZE];
    byte apmask[OS_MACSIZE];  /* zero, not use ap as filter */
    
    byte mac[OS_MACSIZE];
    byte macmask[OS_MACSIZE]; /* zero, not use mac as filter */
    
    uint32_t ip;
    uint32_t ipmask;/* zero, not use ip as filter */
    
    int radioid;    /* <0, not use radioid as filter */
    int wlanid;     /* <0, not use wlanid as filter */
};

#define USER_FILTER_INITER(_local)  { \
    .local  = _local,   \
    .radioid= -1,       \
    .wlanid = -1,       \
}
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

#define um_ubus_notify(_user, _event) do{       \
    if (um_is_ev_enable(_event)) {              \
        um_ubus_common_notify(_user, "um." #_event); \
    }                                           \
}while(0)

#define um_script_notify(_user, _event) do{     \
    if (um_is_sh_enable(_event)) {              \
        os_v_system(UM_SCRIPT " %s %s %s %s &", \
            #_event,                            \
            (_user)->ifname,                    \
            os_macstring((_user)->mac),         \
            os_ipstring((_user)->ip));          \
    }                                           \
}while(0)

extern void
um_ubus_update_notify(struct apuser *old, struct apuser *new);

typedef void um_user_update_f(bool created, struct apuser *dst, struct apuser *src);

extern int 
um_ubus_init(char *path);

extern void
um_ubus_fini(void);
/******************************************************************************/
extern void
__um_user_dump(struct apuser *user, char *action);

#define um_user_dump(user, action)  do{ \
    if (appkey_get(umc.debug.user, 0)) {   \
        __um_user_dump(user, action);   \
    }                                   \
}while(0)

extern struct apuser *
user_connect(byte mac[], struct um_intf *intf);

extern void
user_disconnect(struct apuser *user);

extern struct apuser *
user_bind(struct apuser *user, uint32_t ip, struct um_intf *intf);

extern void
user_unbind(struct apuser *user);

extern struct apuser *
user_auth(struct apuser *user);

extern void
user_deauth(struct apuser *user, int reason);

extern struct apuser *
um_user_connect(byte mac[], char *ifname);

extern void
um_user_disconnect(byte mac[]);

extern struct apuser *
um_user_bind(byte mac[], uint32_t ip, char *ifname);

extern void
um_user_unbind(byte mac[]);

extern struct apuser *
um_user_auth(byte mac[]);

extern void
um_user_deauth(byte mac[], int reason);

extern struct apuser *
um_user_update(struct apuser *old, struct apuser *new);

extern int
um_user_foreach(um_foreach_f *foreach, void *data);

extern struct apuser *
um_user_getbymac(byte mac[]);

extern struct apuser *
um_user_getbyip(uint32_t ip);

extern int
um_user_getby(struct user_filter *filter, um_get_f *get, void *data);

extern int
um_user_del(struct apuser *user);

extern int
um_user_delbymac(byte mac[]);

extern int
um_user_delbyip(uint32_t ip);

extern int
um_user_delby(struct user_filter *filter);
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
#define um_user_policy_name(id)     umc.policy.user[id].name

#define um_open_table(name)         blobmsg_open_table(&b, name)
#define um_open_array(name)         blobmsg_open_array(&b, name)
#define um_close_table(handle)      blobmsg_close_table(&b, handle)
#define um_close_array(handle)      blobmsg_close_array(&b, handle)

#define um_add_bool(name, val)      blobmsg_add_u8(&b, name, val)
#define um_add_string(name, val)    blobmsg_add_string(&b, name, val)
#define um_add_u32(name, val)       blobmsg_add_u32(&b, name, val)
#define um_add_u64(name, val)       blobmsg_add_u64(&b, name, val)

#define um_user_add_bool(id, val)   um_add_bool(um_user_policy_name(id), val)
#define um_user_add_string(id, val) um_add_string(um_user_policy_name(id), val)
#define um_user_add_u32(id, val)    um_add_u32(um_user_policy_name(id), val)
#define um_user_add_u64(id, val)    um_add_u64(um_user_policy_name(id), val)
#define um_user_add_macstring(id, mac)  um_user_add_string(id, os_macstring(mac))

#define um_ubus_send_reply(ctx, req)    ubus_send_reply(ctx, req, b.head)

/******************************************************************************/
#define um_debug(var, fmt, args...)     do{ \
    if (appkey_get(umc.debug.var, 0)) {     \
        __debug_with_prefix(fmt, ##args);   \
    }                                       \
}while(0)

#define um_debug_ok(var, fmt, args...)      do{ \
    if (appkey_get(umc.debug.var, 0)) {         \
        debug_ok(fmt, ##args);                  \
    }                                           \
}while(0)

#define um_debug_error(var, fmt, args...)   do{ \
    if (appkey_get(umc.debug.var, 0)) {         \
        debug_error(fmt, ##args);               \
    }                                           \
}while(0)

#define um_debug_trace(var, fmt, args...)   do{ \
    if (appkey_get(umc.debug.var, 0)) {         \
        debug_trace(fmt, ##args);               \
    }                                           \
}while(0)

#define um_debug_test(var, fmt, args...)    do{ \
    if (appkey_get(umc.debug.var, 0)) {         \
        debug_test(fmt, ##args);                \
    }                                           \
}while(0)

#define debug_uci_ok(fmt, args...)          um_debug_ok(uci, fmt, ##args)
#define debug_uci_error(fmt, args...)       um_debug_error(uci, fmt, ##args)
#define debug_uci_trace(fmt, args...)       um_debug_trace(uci, fmt, ##args)
#define debug_uci_test(fmt, args...)        um_debug_test(uci, fmt, ##args)

#define debug_ubus_ok(fmt, args...)         um_debug_ok(ubus, fmt, ##args)
#define debug_ubus_error(fmt, args...)      um_debug_error(ubus, fmt, ##args)
#define debug_ubus_trace(fmt, args...)      um_debug_trace(ubus, fmt, ##args)
#define debug_ubus_test(fmt, args...)       um_debug_test(ubus, fmt, ##args)

#define debug_user_ok(fmt, args...)         um_debug_ok(user, fmt, ##args)
#define debug_user_error(fmt, args...)      um_debug_error(user, fmt, ##args)
#define debug_user_trace(fmt, args...)      um_debug_trace(user, fmt, ##args)
#define debug_user_test(fmt, args...)       um_debug_test(user, fmt, ##args)

#define debug_userscan_ok(fmt, args...)     um_debug_ok(userscan, fmt, ##args)
#define debug_userscan_error(fmt, args...)  um_debug_error(userscan, fmt, ##args)
#define debug_userscan_trace(fmt, args...)  um_debug_trace(userscan, fmt, ##args)
#define debug_userscan_test(fmt, args...)   um_debug_test(userscan, fmt, ##args)

#define debug_flowscan_ok(fmt, args...)     um_debug_ok(flowscan, fmt, ##args)
#define debug_flowscan_error(fmt, args...)  um_debug_error(flowscan, fmt, ##args)
#define debug_flowscan_trace(fmt, args...)  um_debug_trace(flowscan, fmt, ##args)
#define debug_flowscan_test(fmt, args...)   um_debug_test(flowscan, fmt, ##args)
/******************************************************************************/
#endif /* __UM_H_CC431B9A6A7A07C3356E10656BDA3BDD__ */

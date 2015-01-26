#include "utils.h"
#include "um.h"

static void 
disconnect_cb(struct apuser *user);

static void 
connect_cb(struct apuser *user);

static inline int
hashbuf(byte *buf, int len, int mask)
{
    int i;
    int sum = 0;
    
    for (i=0; i<len; i++) {
        sum += (int)buf[i];
    }

    return sum & mask;
}

static inline int
hashmac(byte mac[])
{
    return hashbuf(mac, OS_MACSIZE, UM_HASHMASK);
}

static inline int
haship(uint32_t ip)
{
    return hashbuf((byte *)&ip, sizeof(ip), UM_HASHMASK);
}

static inline bool
in_list(struct list_head *node)
{
    return  (node->next && node->prev) && false==list_empty(node);
}

static inline void
__state(struct apuser *user, int state)
{
    user->state = state;
}

static int
__uptime(struct apuser *user)
{
    char line[1+OS_LINE_LEN] = {0};
    int err = 0;
    int connected_time = 0;
    
    err = os_v_pgets(line, sizeof(line),
                "cat /sys/kernel/debug/ieee80211/phy%d/netdev:%s/stations/%s/connected_time"
                    " | grep clock"
                    " | awk '{print $3}'",
                user->radioid,
                user->ifname,
                os_macstring(user->mac));
    if (0==err) {
        int hour    = 0;
        int minute  = 0;
        int second  = 0;
        
        os_sscanf(line, "%d:%d:%d", &hour, &minute, &second);

        connected_time = hour * 3600 + minute * 60 + second;
    }

    return time(NULL) - connected_time;
}

static void
__bindif(struct apuser *user, struct um_intf *intf)
{
    os_maccpy(user->ap, umc.basemac);
    os_maccpy(user->vap, intf->mac);
    os_strdcpy(user->ifname, intf->ifname);

    user->intf      = intf;
    user->radioid   = intf->radioid;
    user->wlanid    = intf->wlanid;
    user->wifi.uptime = __uptime(user);
}

static inline struct apuser *
__get(byte mac[])
{
    struct apuser *user;
    struct hlist_head *head = &umc.head.mac[hashmac(mac)];
    
    hlist_for_each_entry(user, head, node.mac) {
        if (os_maceq(user->mac, mac)) {
            return user;
        }
    }

    return NULL;
}

static inline int
__remove(struct apuser *user, void (*cb)(struct apuser *user))
{
    if (NULL==user) {
        return -EKEYNULL;
    }
    /*
    * not in list
    */
    else if (false==in_list(&user->node.list)) {
        debug_trace("__remove nothing(not in list)");
        
        return 0;
    }
    
    list_del(&user->node.list);
    if (is_good_mac(user->mac)) {
        hlist_del_init(&user->node.mac);
    }
    if (user->ip) {
        hlist_del_init(&user->node.ip);
    }
    umc.head.count--;

    if (cb) {
        cb(user);
    }

    return 0;
}

static inline int
__insert(struct apuser *user, void (*cb)(struct apuser *user))
{
    char *action;
    
    if (NULL==user) {
        return -EKEYNULL;
    }
    /*
    * have in list
    */
    else if (in_list(&user->node.list)) {
        return -EINLIST;
    }
        
    list_add(&user->node.list, &umc.head.list);
    if (is_good_mac(user->mac)) {
        hlist_add_head(&user->node.mac, &umc.head.mac[hashmac(user->mac)]);
    }
    if (user->ip) {
        hlist_add_head(&user->node.ip,  &umc.head.ip[haship(user->ip)]);
    }
    umc.head.count++;

    if (cb) {
        cb(user);

        action = "create";
    } else {
        action = "update";
    }

    debug_trace("%s user, count(%d)", action, umc.head.count);
    um_user_dump(user, action);
    
    return 0;
}

static struct apuser *
__create(byte mac[], struct um_intf *intf)
{
    if (NULL==intf) {
        return NULL;
    }
    
    struct apuser *user = (struct apuser *)os_malloc(sizeof(*user));
    if (NULL==user) {
        return NULL;
    }

    um_user_init(user, true);
    __bindif(user, intf);
    
    os_maccpy(user->mac, mac);

    return user;
}

static inline struct apuser *
__update(struct apuser *old, struct apuser *new, void (*cb)(struct apuser *old, struct apuser *new))
{
    if (NULL==old || NULL==new) {
        return NULL;
    }

    if (cb) {
        (*cb)(old, new);
    }
    
    os_memcpy(old, new, UM_USER_ENTRY_SIZE);
    old->aging = UM_AGING_TIMES;
    
    return old;
}

static inline void
__deauth(struct apuser *user, int reason, void (*cb)(struct apuser *user, int reason))
{
    if (NULL==user) {
        return;
    }
    else if (UM_USER_STATE_AUTH != user->state) {
        return;
    }
    
    if (cb) {
        (*cb)(user, reason);
    }
    
    /*
    * auth==>bind
    */
    __state(user, UM_USER_STATE_BIND);
    
    user->auth.uptime = 0;
}

static inline void
__unbind(struct apuser *user, void (*cb)(struct apuser *user))
{
    if (NULL==user) {
        return;
    }
    
    __deauth(user, UM_USER_DEAUTH_INITIATIVE, NULL);
    
    if (UM_USER_STATE_BIND != user->state) {
        return;
    }
    
    if (cb) {
        (*cb)(user);
    }
    
    /*
    * bind==>connect
    */
    __state(user, UM_USER_STATE_CONNECT);
    
    user->ip = 0;
}

static inline void
__disconnect(struct apuser *user, void (*cb)(struct apuser *user))
{
    if (NULL==user) {
        return;
    }
    
    __unbind(user, NULL);
    
    if (UM_USER_STATE_CONNECT != user->state) {
        return;
    }
    
    if (cb) {
        (*cb)(user);
    }
    
    /*
    * connect==>disconnect
    */
    __state(user, UM_USER_STATE_DISCONNECT);
    
    user->radioid       = 0;
    user->wlanid        = 0;
    user->wifi.uptime   = 0;
    
    os_objzero(user->ap);
    os_objzero(user->vap);
    os_objzero(user->ifname);
    user->intf = NULL;
}

static inline void
__connect(struct apuser *user, struct um_intf *intf, void (*cb)(struct apuser *user))
{
    if (NULL==user) {
        return;
    }
    else if (NULL==intf) {
        return;
    }
    else if (UM_USER_STATE_DISCONNECT != user->state) {
        return;
    }
    else if (intf==user->intf) {
        return;
    }
    
    __bindif(user, intf);
    user->aging = UM_AGING_TIMES;
    
    __state(user, UM_USER_STATE_CONNECT);
    
    if (cb) {
        (*cb)(user);
    }
}

static inline void
__bind(struct apuser *user, uint32_t ip, struct um_intf *intf, void (*cb)(struct apuser *user))
{
    if (NULL==user) {
        return;
    }
    else if (NULL==intf) {
        return;
    }
    
    /*
    * user ip changed
    *   1. del user
    *   2. re-bind
    */
    if (is_good_um_user_state(user->state) && ip!=user->ip) {
        __disconnect(user, disconnect_cb);
        __connect(user, intf, connect_cb);
    }
    
    user->ip    = ip;
    user->aging = UM_AGING_TIMES;
    
    __state(user, UM_USER_STATE_BIND);
    
    if (cb) {
        (*cb)(user);
    }
}

static inline void
__auth(struct apuser *user, void (*cb)(struct apuser *user))
{
    if (NULL==user) {
        return;
    }
    else if (UM_USER_STATE_BIND != user->state) {
        return;
    }
    
    user->aging = UM_AGING_TIMES;
    if (0==user->auth.uptime) {
        user->auth.uptime = time(NULL);
    }
    /*
    * bind==>auth
    */
    __state(user, UM_USER_STATE_AUTH);
    
    if (cb) {
        (*cb)(user);
    }
}

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

static inline void
um_script_deauth_notify(struct apuser *user, int reason)
{
    if (um_is_sh_enable(deauth)) {
        os_v_system(UM_SCRIPT " deauth %s %s %s %s &",
            user->ifname,
            os_macstring(user->mac),
            os_ipstring(user->ip),
            um_user_deauth_reason_string(reason));
    }
}

static inline struct apuser *
__find_and_create(byte mac[], struct um_intf *intf)
{
    struct apuser *user = __get(mac);
    if (NULL==user) {
        user = __create(mac, intf);
    }
    
    return user;
}

static void 
connect_cb(struct apuser *user)
{
    um_ubus_notify(user, connect);
    um_script_notify(user, connect);
}

static void 
disconnect_cb(struct apuser *user)
{
    um_ubus_notify(user, disconnect);
    um_script_notify(user, disconnect);
}

static void 
bind_cb(struct apuser *user)
{
    um_ubus_notify(user, bind);
    um_script_notify(user, bind);
}

static void 
unbind_cb(struct apuser *user)
{
    um_ubus_notify(user, unbind);
    um_script_notify(user, unbind);
}

static void 
auth_cb(struct apuser *user)
{
    um_ubus_notify(user, auth);
    um_script_notify(user, auth);
}

static void 
deauth_cb(struct apuser *user, int reason)
{
    um_ubus_deauth_notify(user, reason);
    um_script_deauth_notify(user, reason);
}

static void 
update_cb(struct apuser *old, struct apuser *new)
{
    um_ubus_update_notify(old, new);
    /*
    * todo: call local script
    */
}

struct apuser *
user_connect(byte mac[], struct um_intf *intf)
{
    struct apuser *user = __find_and_create(mac, intf);
    
    __connect(user, intf, connect_cb);

    return user;
}

void
user_disconnect(struct apuser *user)
{
    __disconnect(user, disconnect_cb);
}

struct apuser *
user_bind(struct apuser *user, uint32_t ip, struct um_intf *intf)
{
    __bind(user, ip, intf, bind_cb);
    
    return user;
}

void
user_unbind(struct apuser *user)
{
    __unbind(user, unbind_cb);
}

struct apuser *
user_auth(struct apuser *user)
{
    __auth(user, auth_cb);

    return user;
}

void
user_deauth(struct apuser *user, int reason)
{
    if (is_good_um_user_deauth_reason(reason)) {
        __deauth(user, reason, deauth_cb);
    }
}

struct apuser *
um_user_connect(byte mac[], char *ifname)
{
    return user_connect(mac, um_intf_get(ifname));
}

void
um_user_disconnect(byte mac[])
{
    user_disconnect(__get(mac));
}

struct apuser *
um_user_bind(byte mac[], uint32_t ip, char *ifname)
{
    struct um_intf *intf = um_intf_get(ifname);
    /*
    * connect state is from scan
    * 
    * so, maybe first bind
    */
    return user_bind(user_connect(mac, intf), ip, intf);
}

void
um_user_unbind(byte mac[])
{
    user_unbind(__get(mac));
}

struct apuser *
um_user_auth(byte mac[])
{
    return user_auth(__get(mac));
}

void
um_user_deauth(byte mac[], int reason)
{
    user_deauth(__get(mac), reason);
}

struct apuser *
um_user_update(struct apuser *old, struct apuser *new)
{
    return __update(old, new, update_cb);
}

int
um_user_foreach(um_foreach_f *foreach, void *data)
{
    multi_value_u mv;
    struct apuser *user, *n;
    
    list_for_each_entry_safe(user, n, &umc.head.list, node.list) {
        mv.value = (*foreach)(user, data);
        
        if (mv2_is_break(mv)) {
            return mv2_result(mv);
        }
    }
    
    return 0;
}


struct apuser *
um_user_getbymac(byte mac[])
{
    return __get(mac);
}

struct apuser *
um_user_getbyip(uint32_t ip)
{
    struct apuser *user;
    struct hlist_head *head = &umc.head.ip[haship(ip)];
    
    hlist_for_each_entry(user, head, node.ip) {
        if (user->ip==ip) {
            return user;
        }
    }

    return NULL;
}

int
um_user_del(struct apuser *user)
{
    /*
    * just set disconnect, NOT free
    */
    __disconnect(user, disconnect_cb);
    
    return 0;
}

int
um_user_delbymac(byte mac[])
{
    return um_user_del(um_user_getbymac(mac));
}

int
um_user_delbyip(uint32_t ip)
{
    return um_user_del(um_user_getbyip(ip));
}

void
um_user_bindif(struct apuser *user, struct um_intf *intf)
{
    if (user && intf) {
        __bindif(user, intf);
    }
}

void
__um_user_dump(struct apuser *user, char *action)
{
    os_println("=====%s user begin======", action);

#define __dump(_fmt, args...)   os_println(__tab _fmt, ##args)
    __dump("ap          = %s",  os_macstring(user->ap));
    __dump("vap         = %s",  os_macstring(user->vap));
    __dump("mac         = %s",  os_macstring(user->mac));
    __dump("ip          = %s",  os_ipstring(user->ip));
    __dump("state       = %s",  um_user_state_string(user->state));
    __dump("ifname      = %s",  user->ifname);
    __dump("radioid     = %d",  user->radioid);
    __dump("wlanid      = %d",  user->wlanid);
    
    __dump("wifi.uptime = %u",  user->wifi.uptime);
    __dump("wifi.onlinelimie = %u",user->wifi.onlinelimit);
    __dump("wifi.signal = %u",  user->wifi.signal);
    __dump("wifi.up.flowtotal = %llu",  user->wifi.up.flowtotal);
    __dump("wifi.up.flowcache = %llu",  user->wifi.up.flowcache);
    __dump("wifi.up.flowlimit = %llu",  user->wifi.up.flowlimit);
    __dump("wifi.up.ratelimit = %u",  user->wifi.up.ratelimit);
    __dump("wifi.down.flowtotal = %llu",  user->wifi.down.flowtotal);
    __dump("wifi.down.flowcache = %llu",  user->wifi.down.flowcache);
    __dump("wifi.down.flowlimit = %llu",  user->wifi.down.flowlimit);
    __dump("wifi.down.ratelimit = %u",  user->wifi.down.ratelimit);
    __dump("wifi.all.flowtotal = %llu",  user->wifi.all.flowtotal);
    __dump("wifi.all.flowcache = %llu",  user->wifi.all.flowcache);
    __dump("wifi.all.flowlimit = %llu",  user->wifi.all.flowlimit);
    __dump("wifi.all.ratelimit = %u",  user->wifi.all.ratelimit);
    
    __dump("auth.uptime = %u",  user->auth.uptime);
    __dump("auth.onlinelimie = %u",user->auth.onlinelimit);
    __dump("auth.up.flowtotal = %llu",  user->auth.up.flowtotal);
    __dump("auth.up.flowcache = %llu",  user->auth.up.flowcache);
    __dump("auth.up.flowlimit = %llu",  user->auth.up.flowlimit);
    __dump("auth.up.ratelimit = %u",  user->auth.up.ratelimit);
    __dump("auth.down.flowtotal = %llu",  user->auth.down.flowtotal);
    __dump("auth.down.flowcache = %llu",  user->auth.down.flowcache);
    __dump("auth.down.flowlimit = %llu",  user->auth.down.flowlimit);
    __dump("auth.down.ratelimit = %u",  user->auth.down.ratelimit);
    __dump("auth.all.flowtotal = %llu",  user->auth.all.flowtotal);
    __dump("auth.all.flowcache = %llu",  user->auth.all.flowcache);
    __dump("auth.all.flowlimit = %llu",  user->auth.all.flowlimit);
    __dump("auth.all.ratelimit = %u",  user->auth.all.ratelimit);
#undef __dump

    os_println("=====%s user end======", action);
    os_println(__crlf2);
}

static inline bool
macmatch(byte umac[], byte fmac[], byte mask[])
{
    if (is_good_mac(fmac)) {
        if (is_zero_mac(mask)) {
            /*
            * mac NOT zero
            * macmask zero
            *
            * use mac filter
            */
            if (false==os_maceq(umac, fmac)) {
                return false;
            }
        } else {
            /*
            * mac NOT zero
            * macmask NOT zero
            *
            * use mac/macmask filter
            */
            if (false==os_macmaskmach(umac, fmac, mask)) {
                return false;
            }
        }
    }

    return true;
}


static inline bool
ipmatch(unsigned int uip, unsigned int fip, unsigned int mask)
{
    if (fip) {
        if (0==mask) {
            /*
            * ip NOT zero
            * ipmask zero
            *
            * use ip filter
            */
            if (uip != fip) {
                return false;
            }
        } else {
            /*
            * ip NOT zero
            * ipmask NOT zero
            *
            * use ip/ipmask filter
            */
            if (false==os_ipmatch(uip, fip, mask)) {
                return false;
            }
        }
    }

    return true;
}

static bool
match(struct apuser *user, struct user_filter *filter)
{    
    /* local not matched */
    if (filter->local && false==user->local) {
        return false;
    }
    
    if (is_good_um_user_state(filter->state) && filter->state != user->state) {
        return false;
    }
    
    if (false==macmatch(user->mac, filter->mac, filter->macmask)) {
        return false;
    }
    
    if (false==macmatch(user->ap, filter->ap, filter->apmask)) {
        return false;
    }
    
    if (false==ipmatch(user->ip, filter->ip, filter->ipmask)) {
        return false;
    }
    
    if (filter->radioid>=0 && user->radioid!=filter->radioid) {
        return false;
    }

    if (filter->wlanid>=0 && user->wlanid!=filter->wlanid) {
        return false;
    }

    /* all matched */
    return true;
}

static multi_value_t
delby_cb(struct apuser *user, void *data)
{
    struct user_filter *filter = (struct user_filter *)data;

    if (match(user, filter)) {
        um_user_del(user);
    }

    return mv2_OK;
}

int
um_user_delby(struct user_filter *filter)
{
    return um_user_foreach(delby_cb, filter);
}

static multi_value_t
getby_cb(struct apuser *user, void *data)
{
    void **param = (void **)data;
    struct user_filter *filter = (struct user_filter *)param[0];
    um_get_f *get = (um_get_f *)param[1];
    void *arg = param[2];
    
    if (match(user, filter)) {
        return (*get)(user, arg);
    } else {
        return mv2_OK;
    }
}

int
um_user_getby(struct user_filter *filter, um_get_f *get, void *data)
{
    void *param[] = {
        (void *)filter,
        (void *)get,
        (void *)data,
    };
    
    return um_user_foreach(getby_cb, param);
}
/******************************************************************************/

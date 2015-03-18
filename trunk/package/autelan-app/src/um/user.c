#include "utils.h"
#include "um.h"

static void 
disconnect_cb(struct apuser *user);

static void 
connect_cb(struct apuser *user);

static void 
deauth_cb(struct apuser *user, int reason);

static void 
unbind_cb(struct apuser *user);

#define __update_limit(_user, _obj)         do{ \
    struct userlimit *dst = &(_user)->limit._obj; \
    struct userlimit *src = &umc.limit[(_user)->class][(_user)->level]._obj; \
                                                \
    if (false==os_objeq(dst, src)) {            \
        os_objdcpy(dst, src);                   \
    }                                           \
}while(0)

static void
update_limit(struct apuser *user)
{
    __update_limit(user, wifi);
    __update_limit(user, auth);
}

static int
hashbuf(unsigned char *buf, int len, int mask)
{
    int i;
    int sum = 0;
    
    for (i=0; i<len; i++) {
        sum += (int)buf[i];
    }

    return sum & mask;
}

static inline int
hashmac(unsigned char mac[])
{
    return hashbuf(mac, OS_MACSIZE, UM_HASHMASK);
}

static inline int
haship(uint32_t ip)
{
    return hashbuf((unsigned char *)&ip, sizeof(ip), UM_HASHMASK);
}

static inline struct hlist_head *
headmac(unsigned char mac[])
{
    return &umc.head.mac[hashmac(mac)];
}

static inline struct hlist_head *
headip(uint32_t ip)
{
    return &umc.head.ip[haship(ip)];
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
    if (intf!=user->intf) {
        os_maccpy(user->ap, umc.basemac);
        os_maccpy(user->vap, intf->mac);
        os_strdcpy(user->ifname, intf->ifname);

        user->intf      = intf;
        user->radioid   = intf->radioid;
        user->wlanid    = intf->wlanid;
        if (0==user->info.wifi.uptime) {
            user->info.wifi.uptime = __uptime(user);
        }
    }
}

static struct apuser *
__get(unsigned char mac[])
{
    struct apuser *user;
    
    hlist_for_each_entry(user, headmac(mac), node.mac) {
        if (os_maceq(user->mac, mac)) {
            return user;
        }
    }

    return NULL;
}

static inline void
__add_iphash(struct apuser *user)
{
    hlist_add_head(&user->node.ip,  headip(user->ip));
}

static inline void
__del_iphash(struct apuser *user)
{
    hlist_del_init(&user->node.ip);
}

static inline void
__add_machash(struct apuser *user)
{
    hlist_add_head(&user->node.mac, headmac(user->mac));
}

static inline void
__del_machash(struct apuser *user)
{
    hlist_del_init(&user->node.mac);
}

static struct apuser *
__remove(struct apuser *user)
{
    if (NULL==user) {
        return NULL;
    }
    /*
    * not in list
    */
    else if (false==is_in_list(&user->node.list)) {
        debug_trace("__remove nothing(not in list)");
        
        return user;
    }
    
    list_del(&user->node.list);
    if (is_good_mac(user->mac)) {
        __del_machash(user);
        
    }
    if (user->ip) {
        __del_iphash(user);
    }
    umc.head.count--;
    
    um_user_dump(user, "remove");

    return user;
}

static struct apuser *
__insert(struct apuser *user)
{
    if (NULL==user) {
        return NULL;
    }
    /*
    * have in list
    */
    else if (is_in_list(&user->node.list)) {
        return user;
    }
    
    list_add(&user->node.list, &umc.head.list);
    if (is_good_mac(user->mac)) {
        __add_machash(user);
    }
    if (user->ip) {
        __add_iphash(user);
    }
    umc.head.count++;
    
    um_user_dump(user, "insert");
    
    return user;
}

static struct apuser *
__reinsert_byip(struct apuser *user, uint32_t ip)
{
    if (NULL==user) {
        return NULL;
    }
    /*
    * NOT in list
    */
    else if (false==is_in_list(&user->node.list)) {
        __insert(user);
    }
    
    if (user->ip) {
        __del_iphash(user);
    }
    user->ip = ip;
    if (user->ip) {
        __add_iphash(user);
    }

    return user;
}

static struct apuser *
__create(unsigned char mac[], struct um_intf *intf)
{
    if (NULL==intf) {
        return NULL;
    }
    
    struct apuser *user = (struct apuser *)os_zalloc(sizeof(*user));
    if (NULL==user) {
        return NULL;
    }

    um_user_init(user);
    os_maccpy(user->mac, mac);
    __bindif(user, intf);
    
    update_limit(user);
    
    return __insert(user);
}

static struct apuser *
__update(struct apuser *old, struct apuser *new, void (*cb)(struct apuser *old, struct apuser *new))
{
    if (NULL==old || NULL==new) {
        return NULL;
    }
    else if (cb) {
        (*cb)(old, new);
    }
    
    os_memcpy(old, new, UM_USER_ENTRY_SIZE);
    old->aging = UM_AGING_TIMES;
    
    return old;
}

static void
__deauth(struct apuser *user, int reason, void (*cb)(struct apuser *user, int reason))
{
    if (NULL==user || UM_STATE_AUTH != user->state) {
        return;
    }
    else if (cb) {
        (*cb)(user, reason);
    }
    
    /*
    * auth==>bind
    */
    user->state = UM_STATE_BIND;
    user->info.auth.uptime = 0;
}

static void
__unbind(struct apuser *user, void (*cb)(struct apuser *user))
{
    __deauth(user, UM_DEAUTH_INITIATIVE, deauth_cb);
    
    if (NULL==user || UM_STATE_BIND != user->state) {
        return;
    }
    else if (cb) {
        (*cb)(user);
    }
    
    /*
    * bind==>connect
    */
    user->state = UM_STATE_CONNECT;
    
    user->info.wifi.up.flow.cache   = 0;
    user->info.wifi.down.flow.cache = 0;
    user->info.wifi.all.flow.cache  = 0;

    __reinsert_byip(user, 0);
}

static void
__disconnect(struct apuser *user, void (*cb)(struct apuser *user))
{
    __unbind(user, NULL);
    
    if (NULL==user || UM_STATE_CONNECT != user->state) {
        return;
    }
    else if (cb) {
        (*cb)(user);
    }
    
    /*
    * connect==>disconnect
    */
    user->state = UM_STATE_DISCONNECT;
    
    user->info.wifi.uptime  = 0;
    
    user->radioid       = 0;
    user->wlanid        = 0;
    user->intf          = NULL;
    
    os_objzero(user->ap);
    os_objzero(user->vap);
    os_objzero(user->ifname);
}

static inline void
__connect(struct apuser *user, struct um_intf *intf, void (*cb)(struct apuser *user))
{
    if (NULL==user || 
        NULL==intf || 
        is_online_um_user_state(user->state)) {
        return;
    }

    __bindif(user, intf);
    user->aging = UM_AGING_TIMES;
    user->state = UM_STATE_CONNECT;
    
    if (cb) {
        (*cb)(user);
    }
}

static void
__bind(struct apuser *user, uint32_t ip, struct um_intf *intf, void (*cb)(struct apuser *user))
{
    if (NULL==user ||
        NULL==intf ||
        false==is_good_um_user_state(user->state)) {
        return;
    }
    /*
    * user is disconnected
    */
    else if (false==is_online_um_user_state(user->state)) {
        __connect(user, intf, connect_cb);
    }
    /*
    * user is online and intf changed
    */
    else if (intf!=user->intf) {
        __disconnect(user, disconnect_cb);
        __connect(user, intf, connect_cb);
    }
    /*
    * user is online and same intf and user ip changed
    */
    else if (ip!=user->ip) {
        __unbind(user, unbind_cb);
    }
    
    __reinsert_byip(user, ip);
    
    user->aging = UM_AGING_TIMES;
    user->state = UM_STATE_BIND;
    
    if (cb) {
        (*cb)(user);
    }
}

static void
__auth(struct apuser *user, int class, void (*cb)(struct apuser *user))
{
    if (NULL==user ||
        UM_STATE_BIND != user->state) {
        return;
    }
    
    user->aging = UM_AGING_TIMES;
    if (0==user->info.auth.uptime) {
        user->info.auth.uptime = time(NULL);
    }
    /*
    * bind==>auth
    */
    user->state = UM_STATE_AUTH;
    
    /*
    * init limit
    */
    if (class!=user->class) {
        user->class = class;

        os_objdcpy(&user->limit.wifi, &umc.limit[class][user->level].wifi);
        os_objdcpy(&user->limit.auth, &umc.limit[class][user->level].auth);
    }
    
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
        os_v_system(UM_SCRIPT " " #_event " %s %s %s &", \
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

static struct apuser *
__find_and_create(unsigned char mac[], struct um_intf *intf)
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
user_connect(unsigned char mac[], struct um_intf *intf)
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
user_auth(struct apuser *user, int class)
{
    __auth(user, class, auth_cb);

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
um_user_connect(unsigned char mac[], char *ifname)
{
    return user_connect(mac, um_intf_get(ifname));
}

void
um_user_disconnect(unsigned char mac[])
{
    user_disconnect(__get(mac));
}

struct apuser *
um_user_bind(unsigned char mac[], uint32_t ip, char *ifname)
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
um_user_unbind(unsigned char mac[])
{
    user_unbind(__get(mac));
}

struct apuser *
um_user_auth(unsigned char mac[], int class)
{
    return user_auth(__get(mac), class);
}

void
um_user_deauth(unsigned char mac[], int reason)
{
    user_deauth(__get(mac), reason);
}

struct apuser *
um_user_update(struct apuser *old, struct apuser *new)
{
    return __update(old, new, update_cb);
}

int
um_user_foreach(um_foreach_f *foreach)
{
    multi_value_u mv;
    struct apuser *user, *n;
    
    list_for_each_entry_safe(user, n, &umc.head.list, node.list) {
        mv.value = (*foreach)(user);
        
        if (mv2_is_break(mv)) {
            return mv2_result(mv);
        }
    }
    
    return 0;
}


struct apuser *
um_user_getbymac(unsigned char mac[])
{
    return __get(mac);
}

struct apuser *
um_user_getbyip(uint32_t ip)
{
    struct apuser *user;
    
    hlist_for_each_entry(user, headip(ip), node.ip) {
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
um_user_delbymac(unsigned char mac[])
{
    return um_user_del(__get(mac));
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

#define __um_user_dump(_fmt, _args...)      os_println(__tab _fmt, ##_args)
#define __um_user_dump_objs(_user, _obj)    do{ \
    __um_user_dump(#_obj ".uptime = %u",  _user->info._obj.uptime);                     \
    __um_user_dump(#_obj ".up.flow.total  = %llu",  _user->info._obj.up.flow.total);    \
    __um_user_dump(#_obj ".up.flow.cache  = %llu",  _user->info._obj.up.flow.cache);    \
    __um_user_dump(#_obj ".up.rate.now    = %u",    _user->info._obj.up.rate.now);      \
    __um_user_dump(#_obj ".down.flow.total= %llu",  _user->info._obj.down.flow.total);  \
    __um_user_dump(#_obj ".down.flow.cache= %llu",  _user->info._obj.down.flow.cache);  \
    __um_user_dump(#_obj ".down.rate.now  = %u",    _user->info._obj.down.rate.now);    \
    __um_user_dump(#_obj ".all.flow.total = %llu",  _user->info._obj.all.flow.total);   \
    __um_user_dump(#_obj ".all.flow.cache = %llu",  _user->info._obj.all.flow.cache);   \
    __um_user_dump(#_obj ".all.rate.now   = %u",    _user->info._obj.all.rate.now);     \
                                                                                        \
    __um_user_dump(#_obj ".online = %u",  _user->limit._obj.online);                    \
    __um_user_dump(#_obj ".up.flow.max    = %llu",  _user->limit._obj.up.flow.max);     \
    __um_user_dump(#_obj ".up.rate.max    = %u",    _user->limit._obj.up.rate.max);     \
    __um_user_dump(#_obj ".up.rate.avg    = %u",    _user->limit._obj.up.rate.avg);     \
    __um_user_dump(#_obj ".down.flow.max  = %llu",  _user->limit._obj.down.flow.max);   \
    __um_user_dump(#_obj ".down.rate.max  = %u",    _user->limit._obj.down.rate.max);   \
    __um_user_dump(#_obj ".down.rate.avg  = %u",    _user->limit._obj.down.rate.avg);   \
    __um_user_dump(#_obj ".all.flow.max   = %llu",  _user->limit._obj.all.flow.max);    \
    __um_user_dump(#_obj ".all.rate.max   = %u",    _user->limit._obj.all.rate.max);    \
    __um_user_dump(#_obj ".all.rate.avg   = %u",    _user->limit._obj.all.rate.avg);    \
}while(0)

void
um_user_dump(struct apuser *user, char *action)
{
    debug_trace("after %s user, count is %d", action, umc.head.count);
    
    if (false==appkey_get(umc.debug.user, 0)) {
        return;
    }
    
    os_println("=====%s user begin======", action);

    __um_user_dump("ap          = %s",  os_macstring(user->ap));
    __um_user_dump("vap         = %s",  os_macstring(user->vap));
    __um_user_dump("mac         = %s",  os_macstring(user->mac));
    __um_user_dump("ip          = %s",  os_ipstring(user->ip));
    __um_user_dump("state       = %s",  um_user_state_string(user->state));
    __um_user_dump("ifname      = %s",  user->ifname);
    __um_user_dump("radioid     = %d",  user->radioid);
    __um_user_dump("wlanid      = %d",  user->wlanid);

    __um_user_dump_objs(user, wifi);
    __um_user_dump_objs(user, auth);

    os_println("=====%s user end======", action);
    os_println(__crlf2);
}

static inline bool
match_mac(unsigned char umac[], unsigned char fmac[], unsigned char mask[])
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
match_ip(unsigned int uip, unsigned int fip, unsigned int mask)
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
match_user(struct apuser *user, struct user_filter *filter)
{
    if (is_good_um_user_state(filter->state) && filter->state != user->state) {
        return false;
    }
    
    if (false==match_mac(user->mac, filter->mac, filter->macmask)) {
        return false;
    }
    
    if (false==match_mac(user->ap, filter->ap, filter->apmask)) {
        return false;
    }
    
    if (false==match_ip(user->ip, filter->ip, filter->ipmask)) {
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

int
um_user_delby(struct user_filter *filter)
{
    multi_value_t cb(struct apuser *user)
    {
        if (match_user(user, filter)) {
            um_user_del(user);
        }

        return mv2_OK;
    }
    
    return um_user_foreach(cb);
}

int
um_user_getby(struct user_filter *filter, um_get_f *get)
{
    multi_value_t cb(struct apuser *user)
    {
        if (match_user(user, filter)) {
            return (*get)(user);
        } else {
            return mv2_OK;
        }
    }
    
    return um_user_foreach(cb);
}

int 
um_user_update_limit(void)
{
    multi_value_t cb(struct apuser *user)
    {
        update_limit(user);
        
        return mv2_OK;
    }
    
    return um_user_foreach(cb);
}

/******************************************************************************/

#include "utils.h"
#include "um.h"

struct blob_buf b;

static inline void
notify(char *event)
{
    ubus_notify(umc.ctx, &umc.obj.object, event, b.head, -1);
}

static void
pushuser(struct apuser *user, bool init, char *name, char *reason)
{
    void *handle;
    
    if (NULL==user) {
        return;
    }
    else if (init) {
        um_blob_buf_init();
    }
    
    handle = um_open_table(name);

	um_user_add_macstring(UM_USER_MAC, user->mac);
	um_user_add_macstring(UM_USER_AP, user->ap);
	um_user_add_macstring(UM_USER_VAP, user->vap);
	
	um_user_add_u32(UM_USER_WLANID, user->wlanid);
	um_user_add_u32(UM_USER_RADIOID, user->radioid);
	um_user_add_string(UM_USER_IP, os_ipstring(user->ip));
	um_user_add_string(UM_USER_STATE, um_user_state_string(user->state));

	um_user_add_u32(UM_USER_WIFI_UPTIME, user->wifi.uptime);
	um_user_add_u32(UM_USER_WIFI_SIGNAL, user->wifi.signal);
	um_user_add_u32(UM_USER_WIFI_ONLINELIMIT, user->wifi.onlinelimit);
	um_user_add_u64(UM_USER_WIFI_UPFLOWTOTAL, user->wifi.up.flowtotal);
	um_user_add_u64(UM_USER_WIFI_UPFLOWCACHE, user->wifi.up.flowcache);
	um_user_add_u64(UM_USER_WIFI_UPFLOWLIMIT, user->wifi.up.flowlimit);
	um_user_add_u32(UM_USER_WIFI_UPRATELIMIT, user->wifi.up.ratelimit);
	um_user_add_u64(UM_USER_WIFI_DOWNFLOWTOTAL, user->wifi.down.flowtotal);
	um_user_add_u64(UM_USER_WIFI_DOWNFLOWCACHE, user->wifi.down.flowcache);
	um_user_add_u64(UM_USER_WIFI_DOWNFLOWLIMIT, user->wifi.down.flowlimit);
	um_user_add_u32(UM_USER_WIFI_DOWNRATELIMIT, user->wifi.down.ratelimit);
	um_user_add_u64(UM_USER_WIFI_ALLFLOWTOTAL, user->wifi.all.flowtotal);
	um_user_add_u64(UM_USER_WIFI_ALLFLOWCACHE, user->wifi.all.flowcache);
	um_user_add_u64(UM_USER_WIFI_ALLFLOWLIMIT, user->wifi.all.flowlimit);
	um_user_add_u32(UM_USER_WIFI_ALLRATELIMIT, user->wifi.all.ratelimit);
	
	um_user_add_u32(UM_USER_AUTH_UPTIME, user->auth.uptime);
	um_user_add_u32(UM_USER_AUTH_ONLINELIMIT, user->auth.onlinelimit);
	um_user_add_u64(UM_USER_AUTH_UPFLOWTOTAL, user->auth.up.flowtotal);
	um_user_add_u64(UM_USER_AUTH_UPFLOWCACHE, user->auth.up.flowcache);
	um_user_add_u64(UM_USER_AUTH_UPFLOWLIMIT, user->auth.up.flowlimit);
	um_user_add_u32(UM_USER_AUTH_UPRATELIMIT, user->auth.up.ratelimit);
	um_user_add_u64(UM_USER_AUTH_DOWNFLOWTOTAL, user->auth.down.flowtotal);
	um_user_add_u64(UM_USER_AUTH_DOWNFLOWCACHE, user->auth.down.flowcache);
	um_user_add_u64(UM_USER_AUTH_DOWNFLOWLIMIT, user->auth.down.flowlimit);
	um_user_add_u32(UM_USER_AUTH_DOWNRATELIMIT, user->auth.down.ratelimit);
	um_user_add_u64(UM_USER_AUTH_ALLFLOWTOTAL, user->auth.all.flowtotal);
	um_user_add_u64(UM_USER_AUTH_ALLFLOWCACHE, user->auth.all.flowcache);
	um_user_add_u64(UM_USER_AUTH_ALLFLOWLIMIT, user->auth.all.flowlimit);
	um_user_add_u32(UM_USER_AUTH_ALLRATELIMIT, user->auth.all.ratelimit);

	if (reason) {
	    um_user_add_string(UM_USER_DEAUTH_REASON, reason);
    }
    
    um_close_table(handle);
}

static multi_value_t
get_cb(struct apuser *user, void *data)
{
    char *name = (char *)data;
    
    pushuser(user, false, name, NULL);

    return mv2_OK;
}

static int
pushuserby(struct user_filter *filter)
{
    void *handle;
    int err;
    
    um_blob_buf_init();
    
    handle = um_open_array("users");
    err = um_user_getby(filter, get_cb, NULL);
    um_close_array(handle);
    
    return err;
}

void 
um_ubus_common_notify(struct apuser *user, char *event)
{
    if (is_good_um_user_state(user->state)) {        
        pushuser(user, true, "user", NULL);
    	notify(event);

    	debug_ubus_trace("%s user(%s)", event, os_macstring(user->mac));
    }
}

void 
um_ubus_deauth_notify(struct apuser *user, int reason)
{
    if (false==um_is_ev_enable(deauth)) {
        return;
    }
    
    char *reasonstring= um_user_deauth_reason_string(reason);
    
    pushuser(user, true, "user", reasonstring);
	notify("um.deauth");

	debug_ubus_trace("um.deauth user(%s) reason(%s)", 
	    os_macstring(user->mac), 
	    reasonstring);
}

void
um_ubus_update_notify(struct apuser *old, struct apuser *new)
{
    if (false==um_is_ev_enable(update) ||
        false==is_online_um_user_state(new->state)) {
        return;
    }

    pushuser(old, true, "old", NULL);
    pushuser(new, false, "new", NULL);
	
	// user info
    notify("um.update");
    
	debug_ubus_trace("um.update user(%s)", os_macstring(old->mac));
}

static int
restart(void)
{
    os_println("not support, now!");
    
    return 0;
}

int
um_ubus_handle_restart(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
)
{
	restart();
	return 0;
}

static int
reload(void)
{
    um_uci_load();
    
    return 0;
}

int
um_ubus_handle_reload(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
)
{
	reload();
	return 0;
}

static int
report(void)
{
    struct user_filter f = USER_FILTER_INITER;
    int err;
    
    err = pushuserby(&f);
    if (err<0) {
        return err;
    }

    notify("um.report");

    return 0;
}

int
um_ubus_report(void)
{
    if (um_is_ev_enable(report)) {
        return report();
    } else {
        return 0;
    }
}

static bool
setfilter_ip(struct user_filter *filter, struct blob_attr *attr[])
{
    struct blob_attr *p     = attr[UM_GETUSER_IP];
    struct blob_attr *mask  = attr[UM_GETUSER_IPMASK];

    /* ip filter */
	if (p) {
	    struct blob_attr *onlyip[UM_GETUSER_END] = {
	        [UM_GETUSER_IP] = p,
        };
	    uint32_t ip = inet_addr(blobmsg_get_string(p));
	    
	    if (os_objeq(&onlyip, attr)) { // get user by only ip
            pushuser(um_user_getbyip(ip), true, NULL, NULL);
            
            return true;
	    } else { // get user by ip/ipmask
            filter->ip = ip;
            
            if (mask) {
        	    filter->ipmask = inet_addr(blobmsg_get_string(mask));
        	}
	    }
	}

	return false;
}

static bool
setfilter_mac(struct user_filter *filter, struct blob_attr *attr[])
{
    struct blob_attr *p     = attr[UM_GETUSER_MAC];
    struct blob_attr *mask  = attr[UM_GETUSER_MACMASK];

    /* mac filter */
	if (p) {
	    struct blob_attr *onlymac[UM_GETUSER_END] = {
	        [UM_GETUSER_MAC] = p,
        };
	    byte mac[OS_MACSIZE];
	    
	    os_getmac_bystring(mac, blobmsg_get_string(p));
	    if (os_objeq(&onlymac, attr)) { // get user by only mac
            pushuser(um_user_getbymac(mac), true, NULL, NULL);

            return true;
	    } else { // get user by mac/macmask
            os_maccpy(filter->mac, mac);
            if (mask) {
                os_getmac_bystring(filter->macmask, blobmsg_get_string(mask));
        	}
    	}
	}

	return false;
}

static void
setfilter_state(struct user_filter *filter, struct blob_attr *attr[])
{
    struct blob_attr *p = attr[UM_GETUSER_STATE];
    
    filter->state = 0;
	if (p) {
	    int state = um_user_state_idx(blobmsg_get_string(p));
	    
	    if (is_good_um_user_state(state)) {
            filter->state = state;
	    }
	}
}

static void
setfilter_ap(struct user_filter *filter, struct blob_attr *attr[])
{
    struct blob_attr *p     = attr[UM_GETUSER_AP];
    struct blob_attr *mask  = attr[UM_GETUSER_APMASK];
    
    /* ap filter */
	if (p) {
        os_getmac_bystring(filter->ap, blobmsg_get_string(p));
        if (mask) {
            os_getmac_bystring(filter->apmask, blobmsg_get_string(mask));
    	}
	}
}

static void
setfilter_radio(struct user_filter *filter, struct blob_attr *attr[])
{
    struct blob_attr *p = attr[UM_GETUSER_RADIO];

    /* radio filter */
	p = attr[UM_GETUSER_RADIO];
	if (p) {
        filter->radioid = (int)blobmsg_get_u32(p);
	}
}

static void
setfilter_wlan(struct user_filter *filter, struct blob_attr *attr[])
{
    struct blob_attr *p = attr[UM_GETUSER_WLAN];

    /* wlan filter */
	p = attr[UM_GETUSER_WLAN];
	if (p) {
        filter->wlanid = (int)blobmsg_get_u32(p);
	}
}

int
um_ubus_handle_getuser(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
)
{
    struct user_filter f = USER_FILTER_INITER;
	struct blob_attr *attr[UM_GETUSER_END] = {NULL};
	struct blob_attr *zero[UM_GETUSER_END] = {NULL};
	int err;

	blobmsg_parse(umc.policy.getuser, os_count_of(umc.policy.getuser), attr, blob_data(msg), blob_len(msg));
    if (os_objeq(&zero, attr)) {
        goto filter_ok; /* default get all user */
    }

    setfilter_state(&f, attr);

    if (setfilter_ip(&f, attr) || setfilter_mac(&f, attr)) {
        goto push_ok;
    }
    
    setfilter_ap(&f, attr);
    setfilter_radio(&f, attr);
    setfilter_wlan(&f, attr);
    
filter_ok:
    err = pushuserby(&f);
    if (err<0) {
        return err;
    }

    /* down */
push_ok:
	um_ubus_send_reply(ctx, req);
	
	return 0;
}

static byte *
getmac(struct blob_attr *msg)
{
    struct blob_attr *attr[UM_USER_END] = {NULL};

    blobmsg_parse(umc.policy.user, os_count_of(umc.policy.user), attr, blob_data(msg), blob_len(msg));

    return attr[UM_USER_MAC]?os_getmac(blobmsg_get_string(attr[UM_USER_MAC])):NULL;
}

int
um_ubus_handle_connect(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
)
{
    struct blob_attr *attr[UM_USER_END] = {NULL};
    struct apuser *user;
    
    blobmsg_parse(umc.policy.user, os_count_of(umc.policy.user), attr, blob_data(msg), blob_len(msg));
    if (NULL==attr[UM_USER_MAC] || NULL==attr[UM_USER_IFNAME]) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }
    
    user = um_user_connect(os_getmac(blobmsg_get_string(attr[UM_USER_MAC])), 
                blobmsg_get_string(attr[UM_USER_IFNAME]));
    
    return user?0:-ENOMEM;
}

int
um_ubus_handle_disconnect(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
)
{
    byte *mac = getmac(msg);
    if (NULL==mac) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }
    
    um_user_disconnect(mac);

    return 0;
}

int
um_ubus_handle_bind(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
)
{
    struct blob_attr *attr[UM_USER_END] = {NULL};
    struct apuser *user;
    
    blobmsg_parse(umc.policy.user, os_count_of(umc.policy.user), attr, blob_data(msg), blob_len(msg));
    if (NULL==attr[UM_USER_IP] || NULL==attr[UM_USER_MAC] || NULL==attr[UM_USER_IFNAME]) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }
    
    user = um_user_bind(os_getmac(blobmsg_get_string(attr[UM_USER_MAC])), 
                inet_addr(blobmsg_get_string(attr[UM_USER_IP])), 
                blobmsg_get_string(attr[UM_USER_IFNAME]));

    return user?0:-ENOMEM;
}

int
um_ubus_handle_unbind(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
)
{
    byte *mac = getmac(msg);
    if (NULL==mac) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }
    
    um_user_unbind(mac);

    return 0;
}

int
um_ubus_handle_auth(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
)
{
    byte *mac = getmac(msg);
    if (NULL==mac) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    um_user_auth(mac);

    return 0;
}

int
um_ubus_handle_deauth(
    struct ubus_context *ctx, 
    struct ubus_object *obj,
    struct ubus_request_data *req, 
    const char *method,
    struct blob_attr *msg
)
{
    byte *mac = getmac(msg);
    if (NULL==mac) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }
    
    um_user_deauth(mac, UM_USER_DEAUTH_INITIATIVE);
    
    return 0;
}

static void
add_fd(void)
{
    ubus_add_uloop(umc.ctx);
    os_fd_set_cloexec(umc.ctx->sock.fd);
}

static void
reconnect_timer(struct uloop_timeout *timeout)
{
	static struct uloop_timeout retry = {
		.cb = reconnect_timer,
	};
	int t = 2;

	if (ubus_reconnect(umc.ctx, umc.path) != 0) {
		uloop_timeout_set(&retry, t * 1000);
		return;
	}

	add_fd();
}

static inline int 
add_object(struct ubus_object *obj)
{
	int err = 0;

	err = ubus_add_object(umc.ctx, obj);
	if (err) {
		debug_ubus_error("Failed to publish object '%s': %s\n", obj->name, ubus_strerror(err));
	}

    return 0;
}

static inline int 
add_subscriber(struct ubus_subscriber *subscriber)
{
	int err = 0;

	err = ubus_register_subscriber(umc.ctx, subscriber);
	if (err) {
		debug_ubus_error("Failed to register subscriber(%s)", ubus_strerror(err));
	}

    return 0;
}

static void
connection_lost(struct ubus_context *ctx)
{
	reconnect_timer(NULL);
}

static inline int
um_ubus_connect(char *path)
{
    umc.path = path;
	umc.ctx = ubus_connect(path);
	if (NULL==umc.ctx) {
	    debug_ubus_error("connect ubus failed");
	    
		return -EIO;
    }
	umc.ctx->connection_lost = connection_lost;

    add_fd();
    
    return 0;
}

int 
um_ubus_init(char *path)
{
	int err;

    uloop_init();
	err = um_ubus_connect(path);
	if (err<0) {
        return err;
	}
    
	add_object(&umc.obj.object);
    add_subscriber(&umc.obj.subscriber);
    
    debug_ubus_ok("ubus init");
    
	return 0;
}

void
um_ubus_fini(void)
{
    if (umc.ctx) {
	    ubus_free(umc.ctx);
	    uloop_done();
	}

	debug_ubus_ok("ubus fini");
}

/******************************************************************************/

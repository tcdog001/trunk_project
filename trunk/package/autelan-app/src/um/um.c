#include "utils.h"
#include "um.h"

static inline unsigned int
timerms(struct um_timer *utm)
{
    return appkey_get(utm->akid, utm->deft);
}

static void
usertimer(struct uloop_timeout *timeout, int (*timer)(void))
{
    struct um_timer *utm = container_of(timeout, struct um_timer, tm);
    
    uloop_timeout_set(timeout, timerms(utm));
    
    (*timer)();
}

static int
stascan(struct um_intf *intf, char *macstring)
{
    struct apuser *user;
    
    user = um_user_connect(os_getmac(macstring), intf->ifname);
    
    return user?0:-ENOMEM;
}

static int 
wifiscan_byif(struct um_intf *intf)
{
    FILE *fd = os_v_popen("iw dev %s station dump"
                                " | grep Station"
                                " | awk '{print $2}'", intf->ifname);
    if (NULL==fd) {
        // todo: log
        return -EPIPE;
    }

    while(1) {
        char macstring[1+OS_LINE_LEN] = {0};
        
        if (NULL==fgets(macstring, sizeof(macstring), fd)) {
            break;
        }
        
        stascan(intf, macstring);
    }

    pclose(fd);

    return 0;
}

static int
wifiscan(void)
{
    struct um_intf *intf;
   
    list_for_each_entry(intf, &umc.uci.intf.wlan.cfg, node) {
        wifiscan_byif(intf);
    }

    return 0;
}

static void
wifitimer(struct uloop_timeout *timeout)
{
    usertimer(timeout, wifiscan);
}

static multi_value_t 
aging_cb(struct apuser *user, void *data)
{
    if (UM_USER_STATE_DISCONNECT==user->state) {
        return mv2_OK;
    }
    
    user->aging--;
    if (user->aging <= 0) {
        user_disconnect(user);
    }
    
    return mv2_OK;
}

static int
aging(void)
{
    return um_user_foreach(aging_cb, NULL);
}

static void
agingtimer(struct uloop_timeout *timeout)
{
    usertimer(timeout, aging);
}

static multi_value_t 
online_cb(struct apuser *user, void *data)
{
    if (user->auth.onlinelimit &&
        UM_USER_STATE_DISCONNECT!=user->state) {
        time_t now = time(NULL);
        time_t used = now - user->auth.uptime;
        
        if (used > user->auth.onlinelimit) {
            user_deauth(user, UM_USER_DEAUTH_ONLINETIME);
        }
    }
    
    return mv2_OK;
}

static int
online(void)
{
    return um_user_foreach(online_cb, NULL);
}

static void
onlinetimer(struct uloop_timeout *timeout)
{
    usertimer(timeout, online);
}

static void
reporttimer(struct uloop_timeout *timeout)
{
    usertimer(timeout, um_ubus_report);
}

static void
flow_update(struct apuser *user)
{
    struct apuser new;

    os_memcpy(&new, user, UM_USER_ENTRY_SIZE);
    
    /*
    * todo: update new user's up/down/all total flow
    */

    um_user_update(user, &new);
}

static multi_value_t 
flow_cb(struct apuser *user, void *data)
{
    flow_update(user);
    
    if (user->auth.up.flowlimit &&
        user->auth.up.flowlimit < user->auth.up.flowtotal) {
        user_deauth(user, UM_USER_DEAUTH_FLOWLIMIT);
    }
    else if (user->auth.down.flowlimit &&
        user->auth.down.flowlimit < user->auth.down.flowtotal) {
        user_deauth(user, UM_USER_DEAUTH_FLOWLIMIT);
    }
    else if (user->auth.all.flowlimit &&
        user->auth.all.flowlimit < user->auth.all.flowtotal) {
        user_deauth(user, UM_USER_DEAUTH_FLOWLIMIT);
    }
    
    return mv2_OK;
}

static int
flow(void)
{
    return um_user_foreach(flow_cb, NULL);
}

static void
flowtimer(struct uloop_timeout *timeout)
{
    usertimer(timeout, flow);
}

struct ubus_method um_user_object_methods[] = {
	{ .name = "restart",    .handler = um_ubus_handle_restart },
	{ .name = "reload",     .handler = um_ubus_handle_reload },
	
	UBUS_METHOD("connect",  um_ubus_handle_connect,     umc.policy.user),
	UBUS_METHOD("disconnect",um_ubus_handle_disconnect, umc.policy.user),
	UBUS_METHOD("bind",     um_ubus_handle_bind,        umc.policy.user),
	UBUS_METHOD("unbind",   um_ubus_handle_unbind,      umc.policy.user),
	UBUS_METHOD("auth",     um_ubus_handle_auth,        umc.policy.user),
	UBUS_METHOD("deauth",   um_ubus_handle_deauth,      umc.policy.user),
	
	UBUS_METHOD("getuser",  um_ubus_handle_getuser,     umc.policy.getuser),
};

struct um_control umc = {
    .head   = {
        .mac    = {HLIST_HEAD_INIT},
        .ip     = {HLIST_HEAD_INIT},
        .list   = LIST_HEAD_INIT(umc.head.list),
    },

    .timer = {
        .wifi   = UM_TIMER_INITER(UM_TIMERMS_WIFI, wifitimer),
        .aging  = UM_TIMER_INITER(UM_TIMERMS_AGING, agingtimer),
        .online = UM_TIMER_INITER(UM_TIMERMS_ONLINE, onlinetimer),
        .flow = UM_TIMER_INITER(UM_TIMERMS_FLOW, flowtimer),
        .report = UM_TIMER_INITER(UM_TIMERMS_REPORT, reporttimer),
    },

    .ev = {
        .connect    = { .deft = UM_EV_CONNECT_DEFT},
        .disconnect = { .deft = UM_EV_DISCONNECT_DEFT},
        .bind       = { .deft = UM_EV_BIND_DEFT},
        .unbind     = { .deft = UM_EV_UNBIND_DEFT},
        .auth       = { .deft = UM_EV_AUTH_DEFT},
        .deauth     = { .deft = UM_EV_DEAUTH_DEFT},
        .update     = { .deft = UM_EV_UPDATE_DEFT},
        .report     = { .deft = UM_EV_REPORT_DEFT},
    },

    .sh = {
        .connect    = { .deft = UM_SH_CONNECT_DEFT},
        .disconnect = { .deft = UM_SH_DISCONNECT_DEFT},
        .bind       = { .deft = UM_SH_BIND_DEFT},
        .unbind     = { .deft = UM_SH_UNBIND_DEFT},
        .auth       = { .deft = UM_SH_AUTH_DEFT},
        .deauth     = { .deft = UM_SH_DEAUTH_DEFT},
        .update     = { .deft = UM_SH_UPDATE_DEFT},
        .report     = { .deft = UM_SH_REPORT_DEFT},
    },
    
    .policy = {
        .user   = UM_USER_POLICY_INITER,
        .getuser= UM_GETUSER_POLICY_INITER,
        .radio  = UM_RADIOPOLICY_INITER,
        .wlan   = UM_WLANPOLICY_INITER,
        .limit  = UM_LIMITPOLICY_INITER,
    },

    .uci = {
        .intf = {
            .radio = {
                .param  = UM_PARAM_INITER(umc.policy.radio),
                .cfg    = LIST_HEAD_INIT(umc.uci.intf.radio.cfg),
                .tmp    = LIST_HEAD_INIT(umc.uci.intf.radio.tmp),
                .uci_type = UM_UCI_INTF_RADIO,
            },
            .wlan = {
                .param  = UM_PARAM_INITER(umc.policy.wlan),
                .cfg    = LIST_HEAD_INIT(umc.uci.intf.wlan.cfg),
                .tmp    = LIST_HEAD_INIT(umc.uci.intf.wlan.tmp),
                .uci_type = UM_UCI_INTF_WLAN,
            },
        },
        
        .limit = {
            .wifi = {
                .param  = UM_PARAM_INITER(umc.policy.limit),
                .cfg    = LIST_HEAD_INIT(umc.uci.limit.wifi.cfg),
                .tmp    = LIST_HEAD_INIT(umc.uci.limit.wifi.tmp),
                .uci_type = UM_UCI_LIMIT_WIFI,
            },
            
            .auth = {
                .param  = UM_PARAM_INITER(umc.policy.limit),
                .cfg    = LIST_HEAD_INIT(umc.uci.limit.auth.cfg),
                .tmp    = LIST_HEAD_INIT(umc.uci.limit.auth.tmp),
                .uci_type = UM_UCI_LIMIT_AUTH,
            },
        },
    },
    
    .obj = {
        .methods= um_user_object_methods,
        .type   = UBUS_OBJECT_TYPE("umd", um_user_object_methods),
        .object = {
        	.name = "user-manage",
        	.type = &umc.obj.type,
        	.methods = um_user_object_methods,
        	.n_methods = os_count_of(um_user_object_methods),
        }
    },
};

static void
handle_signal(int signo)
{
	uloop_end();
}

/*
* copy/change from netifd
*/
static void
setup_signals(void)
{
	struct sigaction s;

	memset(&s, 0, sizeof(s));
	s.sa_handler = handle_signal;
	s.sa_flags = 0;
	sigaction(SIGINT, &s, NULL);
	sigaction(SIGTERM, &s, NULL);
	sigaction(SIGUSR1, &s, NULL);
	sigaction(SIGUSR2, &s, NULL);

	s.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &s, NULL);

    os_sigaction_callstack();
    
	debug_ok("setup signal");
}

static void
addtimer(struct um_timer *utm)
{
    uloop_timeout_set(&utm->tm, appkey_get(utm->akid, utm->deft));
}

int main(int argc, char **argv)
{
    char *path = NULL;
    int err = 0;
        
	setup_signals();
	
	err = um_uci_load();
    if (err<0) {
		goto finish;
	}

    err = um_ubus_init(path);
    if (err<0) {
		goto finish;
	}
    
    addtimer(&umc.timer.wifi);
    addtimer(&umc.timer.aging);
    addtimer(&umc.timer.online);
    addtimer(&umc.timer.flow);
    addtimer(&umc.timer.report);
    
	uloop_run();
    err = 0;
    
finish:
	um_ubus_fini();
	
	return err;
}


#define UM_AKID_INIT(_akid, _name, _deft) do{ \
    _akid = appkey_getbyname(_name); \
    debug_trace("%s=%d", _name, appkey_get(_akid, _deft)); \
}while(0)

#define UM_DEBUG_AKID_INIT(_var) \
        UM_AKID_INIT(umc.debug._var, "debug_" #_var, OS_OFF)

#define UM_TIMER_AKID_INIT(_var) \
        UM_AKID_INIT(umc.timer._var.akid, "timer_" #_var, umc.timer._var.deft)

#define UM_UBUS_AKID(_var) \
        UM_AKID_INIT(umc.ev._var.akid, "ubus_" #_var, umc.ev._var.deft)

#define UM_SCRIPT_AKID_INIT(_var) \
        UM_AKID_INIT(umc.ev._var.akid, "script_" #_var, umc.ev._var.deft)

static os_constructor void 
um_akid_initer(void)
{
    UM_DEBUG_AKID_INIT(uci);
    UM_DEBUG_AKID_INIT(ubus);
    UM_DEBUG_AKID_INIT(user);
    UM_DEBUG_AKID_INIT(userscan);
    UM_DEBUG_AKID_INIT(flowscan);
    
    UM_TIMER_AKID_INIT(wifi);
    UM_TIMER_AKID_INIT(aging);
    UM_TIMER_AKID_INIT(online);
    UM_TIMER_AKID_INIT(flow);
    UM_TIMER_AKID_INIT(report);

    UM_UBUS_AKID(connect);
    UM_UBUS_AKID(disconnect);
    UM_UBUS_AKID(bind);
    UM_UBUS_AKID(unbind);
    UM_UBUS_AKID(auth);
    UM_UBUS_AKID(deauth);
    UM_UBUS_AKID(update);
    UM_UBUS_AKID(report);

    UM_SCRIPT_AKID_INIT(connect);
    UM_SCRIPT_AKID_INIT(disconnect);
    UM_SCRIPT_AKID_INIT(bind);
    UM_SCRIPT_AKID_INIT(unbind);
    UM_SCRIPT_AKID_INIT(auth);
    UM_SCRIPT_AKID_INIT(deauth);
    UM_SCRIPT_AKID_INIT(update);
    UM_SCRIPT_AKID_INIT(report);
}

AKID_DEBUGER; /* must last os_constructor */
/******************************************************************************/

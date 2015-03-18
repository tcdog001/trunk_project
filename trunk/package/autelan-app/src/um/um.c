#include "utils.h"
#include "um.h"

USE_INLINE_TIMER;

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
aging_cb(struct apuser *user)
{
    if (false==is_online_um_user_state(user->state)) {
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
    return um_user_foreach(aging_cb);
}

static void
agingtimer(struct uloop_timeout *timeout)
{
    usertimer(timeout, aging);
}

static multi_value_t 
online_cb(struct apuser *user)
{
    if (0==user->limit.auth.online || false==is_online_um_user_state(user->state)) {
        return mv2_OK;
    }
    
    time_t now = time(NULL);
    time_t used = now - user->info.auth.uptime;
    
    if (used > user->limit.auth.online) {
        user_deauth(user, UM_DEAUTH_ONLINETIME);
    }
    
    return mv2_OK;
}

static int
online(void)
{
    return um_user_foreach(online_cb);
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
flow_cb(struct apuser *user)
{
    if (false==is_online_um_user_state(user->state)) {
        return mv2_OK;
    }
    
    flow_update(user);
    
    if (user->limit.auth.up.flow.max &&
        user->limit.auth.up.flow.max < user->info.auth.up.flow.total) {
        user_deauth(user, UM_DEAUTH_FLOWLIMIT);
    }
    else if (user->limit.auth.down.flow.max &&
        user->limit.auth.down.flow.max < user->info.auth.down.flow.total) {
        user_deauth(user, UM_DEAUTH_FLOWLIMIT);
    }
    else if (user->limit.auth.all.flow.max &&
        user->limit.auth.all.flow.max < user->info.auth.all.flow.total) {
        user_deauth(user, UM_DEAUTH_FLOWLIMIT);
    }
    
    return mv2_OK;
}

static int
flow(void)
{
    return um_user_foreach(flow_cb);
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
            [0 ... (UM_CLASS_END-1)] = {
                [0 ... (UM_LEVEL_END-1)] = {
                    .wifi = {
                        .param  = UM_PARAM_INITER(umc.policy.limit),
                        .uci_type = UM_UCI_LIMIT,
                    },
                    
                    .auth = {
                        .param  = UM_PARAM_INITER(umc.policy.limit),
                        .uci_type = UM_UCI_LIMIT,
                    },
                }
            },
        },
    },
    
    .ubus = {
        .methods= um_user_object_methods,
        .type   = UBUS_OBJECT_TYPE("umd", um_user_object_methods),
        .object = {
        	.name = "user-manage",
        	.type = &umc.ubus.type,
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

static void
um_timer_init(void)
{
    struct um_timer *utm;
    
    for (utm = &umc.timer.begin + 1; utm < &umc.timer.end; utm++) {
        addtimer(utm);
    }
}

#define um_akid_init(_akid, _name, _deft) do{ \
    _akid = appkey_getbyname(_name); \
    debug_trace("%s=%d", _name, appkey_get(_akid, _deft)); \
}while(0)

#define um_debug_akid_init(_var) \
        um_akid_init(umc.debug._var, "debug." #_var, OS_OFF)

#define um_timer_akid_init(_var) \
        um_akid_init(umc.timer._var.akid, "timer." #_var, umc.timer._var.deft)

#define um_ubus_akid_init(_var) \
        um_akid_init(umc.ev._var.akid, "ubus." #_var, umc.ev._var.deft)

#define um_script_akid_init(_var) \
        um_akid_init(umc.sh._var.akid, "script." #_var, umc.sh._var.deft)

#define um_ipc_akid_init(_var)  do{ \
    um_ubus_akid_init(_var);        \
    um_script_akid_init(_var);      \
}while(0)
    
static int
fini(void)
{
    return 0;
}

static int
init(void) 
{
    int i, j;

    for (i=0; i<UM_CLASS_END; i++) {
        for (j=0; j<UM_LEVEL_END; j++) {
            struct um_uci *uci;
            
            uci = &umc.uci.limit[i][j].wifi;
            INIT_LIST_HEAD(&uci->cfg);
            INIT_LIST_HEAD(&uci->tmp);
            
            uci = &umc.uci.limit[i][j].auth;
            INIT_LIST_HEAD(&uci->cfg);
            INIT_LIST_HEAD(&uci->tmp);
        }
    }
    
    um_debug_akid_init(uci);
    um_debug_akid_init(ubus);
    um_debug_akid_init(user);
    um_debug_akid_init(userscan);
    um_debug_akid_init(flowscan);
    
    um_timer_akid_init(wifi);
    um_timer_akid_init(aging);
    um_timer_akid_init(online);
    um_timer_akid_init(flow);
    um_timer_akid_init(report);

    um_ipc_akid_init(connect);
    um_ipc_akid_init(disconnect);
    um_ipc_akid_init(bind);
    um_ipc_akid_init(unbind);
    um_ipc_akid_init(auth);
    um_ipc_akid_init(deauth);
    um_ipc_akid_init(update);
    um_ipc_akid_init(report);

    return 0;
}

static int
__main(int argc, char **argv)
{
    char *path = NULL;
    int err = 0;
    
	setup_signals();
	
	err = um_uci_load();
    if (err) {
		goto finish;
	}

    err = um_ubus_init(path);
    if (err) {
		goto finish;
	}
    
    um_timer_init();
    
	uloop_run();
	
finish:
	um_ubus_fini();
	
	return err;
}

#ifndef __BUSYBOX__
#define um_main  main
#endif

int um_main(int argc, char **argv)
{
    return os_call(init, fini, __main, argc, argv);
}
/******************************************************************************/
AKID_DEBUGER; /* must last os_constructor */


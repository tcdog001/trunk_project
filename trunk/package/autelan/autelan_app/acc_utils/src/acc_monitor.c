/****************************
 *
 * to use in the user space 
 * with the module acc
 * autelan
 * 2014.05.21
 *
 ****************************/
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "check_acc.h"
#include <syslog.h>
#include <time.h>

#define ACC_UCI
#ifdef ACC_UCI
// --------- These are for uci --------- //  
#include <uci.h>
#include <uci_blob.h>

#define DEFAULT_CONFIG_PATH	"/etc/config" /* use the default set in libuci */
const char *config_path = DEFAULT_CONFIG_PATH;
static struct uci_context *uci_ctx;
static struct uci_package *uci_network;
#endif

#if 1
// --------- These are for netlink --------- //  
#include <sys/stat.h>  
#include <sys/socket.h>  
#include <sys/types.h>  
#include <asm/types.h>  
#include <linux/netlink.h>  
#include <linux/socket.h>  
  
#define NETLINK_ACCMODULE     22  
#define MAX_PAYLOAD 1024
#endif

#define GET_VOLTAGE_RAW "cat /sys/devices/platform/i2c-gpio.0/i2c-0/0-0052/iio:device0/in_voltage_raw"
static FILE* logfp;
static struct vcc_struct {
	unsigned int vcc_level;
	unsigned int vcc_interval;
	char Timeaccoff[30];
}vcc = {0, 0};

int fd_acc;

#ifdef LTEFI_V2
#define ACC_GPIO  	11
#define SHUT_GPIO 	12
#endif
#ifdef LTEFI_V3
#define ACC_GPIO  	17
#define SHUT_GPIO 	25		/*AR934X_GPIO_COUNT+2*/
#endif
#define MODDEV				"/dev/acc"

#define ACC_UBUS
#ifdef ACC_UBUS
// --------- These are for ubus --------- //  
#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/utils.h>
#include <libubus.h>
#include <libubox/avl.h>
#include <libubox/safe_list.h>
#include <libubox/blobmsg_json.h>
#include "acc_monitor.h"

static struct ubus_context *ctx;
static struct ubus_subscriber test_event;

#define MAX_BUF 4096
#define DEFAULT_CONFIGFILE "/etc/autelan"
enum {
	SWITCH_ON,
	SWITCH_OFF,
};
enum {
	__SYSUPGRADE_CTRL = 0,
	__DOWNLOAD_CTRL,
	__ELSE_CTRL,
	__CTRL_MAX
};
int controll_flag = 0;

static struct acc_struct {
	unsigned int timer;
	unsigned int trigger;
	float voltage;
	char *vcc_dev;
	char *controller[__CTRL_MAX];
    char configfile[255];
}acc;

enum {
	WATCH_ID,
	WATCH_COUNTER,
	__WATCH_MAX
};
static const struct blobmsg_policy watch_policy[__WATCH_MAX] = {
	[WATCH_ID] = { .name = "id", .type = BLOBMSG_TYPE_INT32 },
	[WATCH_COUNTER] = { .name = "counter", .type = BLOBMSG_TYPE_INT32 },
};

enum {
	CREATE_DELAYTIME,
	CREATE_CONTROLER,
	__CREATE_MAX
};
static const struct blobmsg_policy create_policy[__CREATE_MAX] = {
	[CREATE_DELAYTIME] = { .name = "timer", .type = BLOBMSG_TYPE_INT32 },
	[CREATE_CONTROLER] = { .name = "controller", .type = BLOBMSG_TYPE_ARRAY },
};

enum {
	GPIO_PIN,
	GPIO_VAL,
	__GPIO_MAX
};
static const struct blobmsg_policy gpio_set_policy[] = {
	[GPIO_PIN] = { .name = "pin", .type = BLOBMSG_TYPE_INT32 },
	[GPIO_VAL] = { .name = "val", .type = BLOBMSG_TYPE_INT32 },
};
static const struct blobmsg_policy gpio_get_policy[] = {
	[GPIO_PIN] = { .name = "pin", .type = BLOBMSG_TYPE_INT32 },
};

static const struct ubus_method acc_methods[] = {
	UBUS_METHOD("create", acc_create, create_policy),
	{ .name = "status", .handler = acc_status},
	UBUS_METHOD("setgpio", ubus_gpio_set, gpio_set_policy),
	UBUS_METHOD("getgpio", ubus_gpio_get, gpio_get_policy),
	UBUS_METHOD("watch", acc_watch, watch_policy),
	{ .name = "reload", .handler = acc_reload },
};

static struct ubus_object_type acc_object_type = \
	UBUS_OBJECT_TYPE("autelan.acc", acc_methods);

static struct ubus_object acc_object = {
	.name = "autelan.acc",
	.type = &acc_object_type,
	.methods = acc_methods,
	.n_methods = ARRAY_SIZE(acc_methods),
};

static void 
acc_ubus_reply(struct ubus_request_data *req)
{
	static struct blob_buf b;
	struct ubus_request_data new_req;

	ubus_defer_request(ctx, req, &new_req);
	
	blob_buf_init(&b, 0);
	blobmsg_add_u8(&b, "return_status", 1);
	ubus_send_reply(ctx, &new_req, b.head);
	ubus_complete_deferred_request(ctx, &new_req, 0);
	
	return;
}
static int
acc_reload(struct ubus_context *ctx, struct ubus_object *obj,
		     struct ubus_request_data *req, const char *method,
		     struct blob_attr *msg)
{
	int ret = 0;
	acc_op_args op_args;

	if (SWITCH_OFF != acc.trigger) {
		uci_monitor_init();
	} else {
		printf("acc_monitor: SWITCH_OFF, cannot reload!\n");
		return -1;
	}

	return 0;
}

static int 
acc_create(struct ubus_context *ctx, struct ubus_object *obj,
			  struct ubus_request_data *req, const char *method,
			  struct blob_attr *msg)
{
	acc_create_func(msg);
	acc_ubus_reply(req);
}

static int
acc_status(struct ubus_context *ctx, struct ubus_object *obj,
			  struct ubus_request_data *req, const char *method,
			  struct blob_attr *msg)
{
	static struct blob_buf b;
	void *controller;
	int i;

	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "timer", acc.timer);
	blobmsg_add_u32(&b, "trigger", acc.trigger);
//	blobmsg_add_u32(&b, "voltage", acc.voltage);

	controller = blobmsg_open_array(&b, "controller");
	for (i = 0; i < __CTRL_MAX; i++) {
		if (acc.controller[i]) {
			blobmsg_add_string(&b, "controller", acc.controller[i]);
		}
	}
	blobmsg_close_array(&b, controller);

	ubus_send_reply(ctx, req, b.head);

}

static void
test_handle_remove(struct ubus_context *ctx, struct ubus_subscriber *s,
                   uint32_t id)
{
	fprintf(stderr, "%s:Object %08x went away\n", __FILE__, id);
}

static int
test_notify(struct ubus_context *ctx, struct ubus_object *obj,
	    struct ubus_request_data *req, const char *method,
	    struct blob_attr *msg)
{
	char *str;

	str = blobmsg_format_json(msg, true);
	fprintf(stderr, "%s:Received notification '%s': %s\n", __FILE__, method, str);
	free(str);
}

static int 
acc_watch(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__WATCH_MAX];
	int ret;

	blobmsg_parse(watch_policy, __WATCH_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WATCH_ID])
		return UBUS_STATUS_INVALID_ARGUMENT;

	test_event.remove_cb = test_handle_remove;
	test_event.cb = test_notify;
	ret = ubus_subscribe(ctx, &test_event, blobmsg_get_u32(tb[WATCH_ID]));
	fprintf(stderr, "%s:Watching object %08x: %s\n", __FILE__, blobmsg_get_u32(tb[WATCH_ID]), ubus_strerror(ret));
	return ret;
}

static int 
ubus_gpio_set(struct ubus_context *ctx, struct ubus_object *obj,
			  struct ubus_request_data *req, const char *method,
			  struct blob_attr *msg)
{
	struct blob_attr *tb[__CREATE_MAX];
	acc_op_args op_args;
	
	blobmsg_parse(gpio_set_policy, ARRAY_SIZE(gpio_set_policy), tb, blob_data(msg), blob_len(msg));
	
	if (tb[GPIO_PIN])
	{	
		op_args.num = blobmsg_get_u32(tb[GPIO_PIN]);
		//printf("%s:pin %d\n", __FILE__, op_args.num);
	}
	if (tb[GPIO_VAL])
	{
		op_args.status = blobmsg_get_u32(tb[GPIO_VAL]);
		//printf("%s:val is %d\n", __FILE__, op_args.num);
	}
	gpio_set(&op_args);
	
}

static int 
ubus_gpio_get(struct ubus_context *ctx, struct ubus_object *obj,
			  struct ubus_request_data *req, const char *method,
			  struct blob_attr *msg)
{
	struct blob_attr *tb[__CREATE_MAX];
	acc_op_args op_args;
	
	blobmsg_parse(gpio_get_policy, ARRAY_SIZE(gpio_get_policy), tb, blob_data(msg), blob_len(msg));
	
	if (tb[GPIO_PIN]) {	
		op_args.num = blobmsg_get_u32(tb[CREATE_DELAYTIME]);
		//printf("%s:pin %d\n", __FILE__, op_args.num);
	}
	gpio_get(&op_args);
}

static void server_main(void)
{
	int ret;

	ret = ubus_add_object(ctx, &acc_object);
	if (ret)
		fprintf(stderr, "%s:Failed to add object: %s\n", __FILE__, ubus_strerror(ret));

	ret = ubus_register_subscriber(ctx, &test_event);
	if (ret)
		fprintf(stderr, "%s:Failed to add watch handler: %s\n", __FILE__, ubus_strerror(ret));

	uloop_run();
}
int acc_create_func(struct blob_attr *msg)
{
	struct blob_attr *tb[__CREATE_MAX];
	int delay_time = 0;
	acc_op_args op_args;
	char *controller = NULL;
	struct blob_attr *cur;
	int rem, i = 0;

	blobmsg_parse(create_policy, ARRAY_SIZE(create_policy), tb, blobmsg_data(msg), blobmsg_data_len(msg));

	if (tb[CREATE_DELAYTIME])
	{	
		//printf("%s:acc.trigger is %d\n", __FILE__, acc.trigger);
		if (SWITCH_OFF != acc.trigger)
		{
			delay_time = blobmsg_get_u32(tb[CREATE_DELAYTIME]);
			printf("%s:create delay_time is %d\n", __FILE__, delay_time);
			//printf("%s:fd_acc: %d\n", __FILE__, fd_acc);
			//printf("acc_monitor: set timer %d\n", acc.timer);
			if (acc.timer != delay_time) {
				acc.timer = delay_time;
				op_args.num = delay_time;
				set_acc_gpio_timer(&op_args);
			}
		} else {
			printf("acc_monitor: SWITCH_OFF, set timer failed! acc.timer=%d\n", acc.timer);
			return -1;
		}
	}
	if ((cur = tb[CREATE_CONTROLER]) != NULL)
	{
		blobmsg_for_each_attr(cur, tb[CREATE_CONTROLER], rem) {
			if (blobmsg_type(cur) != BLOBMSG_TYPE_STRING) {
				printf("%s:blobmsg_type error %s\n", __FILE__, cur);
				break;
			}
			if (!blobmsg_check_attr(cur, NULL)) {
				printf("%s:check_attr error %s\n", __FILE__, cur);
				break;
			}		
//			printf("%s:array_get_string is %d, %s, %s\n", __FILE__, rem, cur, blobmsg_data(cur));
			if (0 == i)
				controll_flag = 0;
			set_controll_flag(blobmsg_data(cur));
			if (strcmp(blobmsg_data(cur), "none")) {
				acc.controller[i] = blobmsg_data(cur);
				i ++;
			} else {
				for (i = 0; i < __CTRL_MAX; i++) {
					acc.controller[i] = NULL;
				}				
				break;
			}
		}
		get_controll_flag();
	}
	return 0;
}

#endif

static unsigned int gpio_set(acc_op_args * ops)
{
	int retval;

	retval = ioctl(fd_acc, SET_GPIO, ops);
	if (0 == retval) {
		//printf("\nioctl success\n");
	} else {
		printf("Read failed return [%d]\n",retval);
	}		

	return retval;	
}

static unsigned int gpio_get(acc_op_args * ops)
{
	int retval;
	unsigned int val;

	retval = ioctl(fd_acc, GET_GPIO, ops);
	if (0 == retval) {
		//printf("\nioctl success\n");
	} else {
		printf("Read failed return [%d]\n",retval);
	}		

	val = ops->status;
	printf("gpio %d is %d\n", ops->num, val);

	return val;
//	return;
}

int set_acc_gpio_timer(acc_op_args * ops)
{
	int retval;

	retval = ioctl(fd_acc, SET_ACC_GPIO_TIMER, ops);
	if (0 == retval) {
		//printf("\nioctl success\n");
	} else {
		printf("Read failed return [%d]\n",retval);
	}		

	return retval;	
}
int get_acc_gpio_timer(void)
{
	int retval;

#if 0
	printf("\n-----get_acc_gpio_timer-----%d<<(%d), %d<<(%d), %d<<0, %d<<(%d)\n",_IOC_READ|_IOC_WRITE, _IOC_DIRSHIFT,10,_IOC_TYPESHIFT,2,(_IOC_TYPECHECK(acc_op_args)),_IOC_SIZESHIFT);
	printf("\n-----get_acc_gpio_timer-----%d %d %d %d \n", (_IOC_READ|_IOC_WRITE)<<(_IOC_DIRSHIFT), 10<<8, 2<<0, (_IOC_TYPECHECK(acc_op_args))<<16);
	printf("\n-----get_acc_gpio_timer-----%d\n", ((_IOC_READ|_IOC_WRITE)<<(29))|(10<<8)|( 2<<0)|( (_IOC_TYPECHECK(acc_op_args))<<16));
	printf("\n-----get_acc_gpio_timer-----%d = %d :%d, %d \n", GET_ACC_GPIO_TIMER, _IOWR(ACC_IOC_MAGIC,2,acc_op_args), ACC_IOC_MAGIC, ACC_IOC_MAXNR);
#endif
	retval = ioctl(fd_acc, GET_ACC_GPIO_TIMER, NULL);
	if (0 == retval) {
		//printf("\nioctl success\n");
	} else {
		printf("Read failed return [%d]\n",retval);
	}		

	return retval;	
}

void signal_f(int signum)
{
	int stdout_dev = 0, proc_id = 0;
  	int self_pid = getpid();
	acc_op_args op_args;
	unsigned int switch_val = 0;
	char Timereport[30] = {0};
	struct tm *t;
	time_t tt;
	
	//printf("check_lte got a signal=%d\n", signum);
	op_args.num = ACC_GPIO;
	switch_val = gpio_get(&op_args);

	if ((SWITCH_ON == switch_val)||(SWITCH_OFF == switch_val)) {
		time(&tt);
		t = localtime(&tt);

		sprintf(Timereport, "%4d-%02d-%02d-%02d:%02d:%02d", 
			t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
		if (Timereport) {
			memset(vcc.Timeaccoff, 0, sizeof(vcc.Timeaccoff));
			strcpy(vcc.Timeaccoff, Timereport);
		}
		printf("acc switch: %s\n", vcc.Timeaccoff);

		acc.trigger = switch_val;
	} else
		acc.trigger = -1;
	
	printf("acc_gpio is %d\n", acc.trigger);

	return;
}

static int acc_SIGIO_init(void)
{
	printf("%s:fd_acc: %d\n", __func__, fd_acc);

	signal(SIGIO, signal_f);
	//printf("fcntl(fd_acc)\n");
	fcntl(fd_acc, F_SETOWN, getpid());  
	fcntl(fd_acc, F_SETFL, fcntl(fd_acc, F_GETFL) | FASYNC); 
}

static int acc_monitor_init(void)
{
	//printf("acc_monitor: open acc\n");
	acc_op_args op_args;
	fd_acc = open(MODDEV, O_RDWR);
	if (fd_acc < 0) {
		printf("acc_monitor: can't open acc mode dev!\n");	
		return fd_acc;
	}
#if 0
	signal_f(0);

	if (SWITCH_OFF != acc.trigger) {
		//printf("acc_monitor: set timer %d\n", acc.timer);
		op_args.num = acc.timer;
		set_acc_gpio_timer(&op_args);
	} else {
		printf("acc_monitor: SWITCH_OFF, set timer failed! acc.timer=%d\n", acc.timer);
	}
#endif
	acc_SIGIO_init();
}

int PopenFile (char *cmd_str, char *str, int len)
{
	FILE *fp=NULL; 

	if (cmd_str == NULL||str == NULL)
		return -1;

	memset(str, 0, len);		   
	fp = popen(cmd_str, "r");  
//	printf("%s\n", cmd_str);
	if (fp) 
	{	   
		fgets(str, len, fp);	   
		if (str[strlen(str)-1] == '\n')	   
		{		   
		   str[strlen(str)-1] = '\0';	   
		}	   
		pclose(fp); 	   
		return 0;    
	}  
	else
	{	   
		perror("popen");	   
		str = NULL;
		return -1;   
	}
	return 0;
}

void acc_shutdown(void)
{
	sleep(20);	
	if (SWITCH_OFF == acc.trigger) {
		system("/bin/echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:offdelay/brightness");	
	} else {
		system("/sbin/reboot");
	}
}

void acc_poweroff(void)
{
	sleep(20);	
	system("echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:offdelay/brightness");
	system("echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:shutdown/brightness");
}

void vcc_report(void)
{
	char temp_str[256] = {0};
	char Timereport[30] = {0};
	struct tm *t;
	time_t tt;

	time(&tt);
	t = localtime(&tt);
	sprintf(Timereport, "%4d-%02d-%02d-%02d:%02d:%02d", 
		t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	printf("time report: %s\n", Timereport);
	printf("voltage:%f level:%d\n", acc.voltage, vcc.vcc_level);

	sprintf(temp_str, "{\"recordtime\":\"%s\",\"accofftime\":\"%s\",\"currentvolt\":\"%f\",\"rated\":\"%d\"}",
	Timereport, vcc.Timeaccoff, acc.voltage, vcc.vcc_level);
	printf("%s\n", temp_str);
	if (logfp != (FILE*)0) {	
		(void)fprintf(logfp, "%s\n", temp_str);
		(void)fflush(logfp);
	}
}

void *vcc_monitor(void *arg)
{  
	char str[256];

	while (1)
	{
		if (SWITCH_OFF == acc.trigger) {
			PopenFile(GET_VOLTAGE_RAW, str, sizeof(str));
			if (strlen(str)) {
				float voltage = ((3.3 / 256) * atoi(str) * (120 + 10)) / 10;
				acc.voltage = voltage;
				printf("voltage is %d, %f(v)\n", atoi(str), voltage);
	
				if (0 == vcc.vcc_level) {
					if (voltage < 17) vcc.vcc_level = 10;
					if (voltage >= 17) vcc.vcc_level = 20;
				} else {
					if (voltage < vcc.vcc_level) {
						/* ap force shutdown*/
						vcc_report();
						printf("ap force shutdown\n");	
						/* jmsg send vcc force shutdown script and tftp vcc log to mediaboard */
						system("/sbin/power_off.sh");
						acc_poweroff();
					}
				}
				vcc_report();
			} else {
				printf("cannot get voltage\n");
			}
		}
		sleep(vcc.vcc_interval);
	}
}
void set_controll_flag(char *str)
{
	if (str == NULL) {
		return;
	}
	if (strcmp(str, "none") == 0) {
		controll_flag = 0;
	} else if(strcmp(str, "sysupgrade") == 0) {
		controll_flag |= (0x1 << __SYSUPGRADE_CTRL);
	} else if (strcmp(str, "download") == 0) {
		controll_flag |= (0x1 << __DOWNLOAD_CTRL);
	} else {
		controll_flag |= (0x1 << __ELSE_CTRL);
	}
}
void get_controll_flag(void)
{
	char *str="none";
	
	printf("get_controller:%d", controll_flag);
	if (0 == controll_flag) {
		str = "none";
		printf(" %s", str);
	}
	if (controll_flag&(0x1 << __SYSUPGRADE_CTRL)) {
		str = "sysupgrade";
		printf(" %s", str);
	} 
	if (controll_flag&(0x1 << __DOWNLOAD_CTRL)) {
		str = "download";
		printf(" %s", str);
	}
	if (controll_flag&(0x1 << __ELSE_CTRL)) {
		str = "else";
		printf(" %s", str);
	}	
	printf("\n");
}
void *netlink(void *arg) 
{  
    int state;  
    struct sockaddr_nl src_addr, dest_addr;  
    struct nlmsghdr *nlh = NULL; 
    struct iovec iov;  
    struct msghdr msg;  
    int sock_fd, retval;  
    int state_smg = 0;  

    // Create a socket    
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ACCMODULE);  
    if (sock_fd == -1) {  
        printf("error getting socket: %s", strerror(errno));  
        exit(-1);  
    }  
  
    // To prepare binding
    memset(&msg, 0, sizeof(msg));  
    memset(&src_addr, 0, sizeof(src_addr));  
    src_addr.nl_family = AF_NETLINK;  
    src_addr.nl_pid = getpid(); // self pid  
  
    src_addr.nl_groups = 0; // multi cast  
  
  
    retval = bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));  
    if (retval < 0) {  
        printf("bind failed: %s", strerror(errno));  
        close(sock_fd);  
        exit(-1);  
    }  
  
    // To prepare recvmsg
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));  
    if (!nlh) {  
        printf("malloc nlmsghdr error!\n");  
        close(sock_fd);  
        exit(-1);  
    }  
  
    memset(&dest_addr, 0, sizeof(dest_addr));  
    dest_addr.nl_family = AF_NETLINK;  
    dest_addr.nl_pid = 0;  
    dest_addr.nl_groups = 0;  
  
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);  
    nlh->nlmsg_pid = getpid();  
    nlh->nlmsg_flags = 0;  
    strcpy(NLMSG_DATA(nlh), "Hello you!");  
  
    iov.iov_base = (void *)nlh;  
    iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);  
    // iov.iov_len = nlh->nlmsg_len;  
  
    memset(&msg, 0, sizeof(msg));  
     
    msg.msg_name = (void *)&dest_addr;  
    msg.msg_namelen = sizeof(dest_addr);  
    msg.msg_iov = &iov;  
    msg.msg_iovlen = 1;  
  
//    printf("state_smg\n");  
    state_smg = sendmsg(sock_fd, &msg, 0);  
  
    if (state_smg == -1) {  
        printf("get error sendmsg = %s\n", strerror(errno));  
    }  
  
    memset(nlh,0,NLMSG_SPACE(MAX_PAYLOAD));  
//    printf("waiting received!\n");  

    // Read message from kernel  
	while (1) {
		state = recvmsg(sock_fd, &msg, 0);  
	    if (state < 0) {  
	        printf("state<1");  
	    }  
//	    printf("Received message: %s\n", (char *) NLMSG_DATA(nlh));  
		if (strcmp("acc off", (char *) NLMSG_DATA(nlh)) == 0) {	
			printf("received:       acc off!\n");
			get_controll_flag();
			system("/sbin/acc_off.sh >> /root/acc_monitor.log &");
		} else if (strcmp("acc on", (char *) NLMSG_DATA(nlh)) == 0) {	
			printf("received:       acc on!\n");
			system("/sbin/acc_on.sh >> /root/acc_monitor.log &");
		} else if (strcmp("delay off", (char *) NLMSG_DATA(nlh)) == 0) {	
			printf("received:       delay off!\n");
			/* jmsg send to tell off delay */
			system("/sbin/time_off.sh >> /root/acc_monitor.log &");
			
			if (0 == controll_flag) {
				system("/sbin/off_delay.sh >> /root/acc_monitor.log &");
			} else {
				get_controll_flag();	
			}
			acc_shutdown();
		}
	}
	
    close(sock_fd);  
  
    return;  
}  
void default_config_init(void)
{
	int i;

	printf("Setting default acc parameters\n");
	acc.timer = ACC_DELAY_TIMER;
	acc.trigger = SWITCH_ON;
	acc.voltage = 0;
	for (i = 0; i < __CTRL_MAX; i++) {
		acc.controller[i] = NULL;
	}
	vcc.vcc_interval = 10;
	vcc.vcc_level = 0;
	strncpy(acc.configfile, DEFAULT_CONFIGFILE, sizeof(acc.configfile));	

	system("/bin/mv /root/acc_monitor.log /tmp/acc_monitor.log > /dev/null 2&>1");
}

#ifdef ACC_UCI
// --------- These are for uci --------- //  
const struct uci_blob_param_list acc_attr_list = {
	.n_params = __CREATE_MAX,
	.params = create_policy,
};

static struct uci_package *config_init_package(const char *config)
{
	struct uci_context *ctx = uci_ctx;
	struct uci_package *p = NULL;

	if (!ctx) {
		ctx = uci_alloc_context();
		uci_ctx = ctx;

		ctx->flags &= ~UCI_FLAG_STRICT;
		if (config_path)
			uci_set_confdir(ctx, config_path);

#ifdef DUMMY_MODE
		uci_set_savedir(ctx, "./tmp");
#endif
	} else {
		p = uci_lookup_package(ctx, config);
		if (p)
			uci_unload(ctx, p);
	}

	if (uci_load(ctx, config, &p))
		return NULL;

	return p;
}

static void config_parse_acc(struct uci_section *s)
{
	void *acc_val;
	static struct blob_buf b;
	
	blob_buf_init(&b, 0);
	acc_val = blobmsg_open_array(&b, "acc");
	uci_to_blob(&b, s, &acc_attr_list);
	blobmsg_close_array(&b, acc_val);

	acc_create_func(blob_data(b.head));
}

static void config_init_acc(void)
{
	struct uci_element *e;

	uci_foreach_element(&uci_network->sections, e) {
		struct uci_section *s = uci_to_section(e);
		if (!strcmp(s->type, "acc")){
			config_parse_acc(s);
		}
	}
}

void *uci_monitor_init(void)
{  
	uci_network = config_init_package("autelan");
	if (!uci_network) {
		fprintf(stderr, "Failed to load network config\n");
		return;
	}
	config_init_acc();
}
#endif

int main(int argc, char **argv) 
{
	int opt, err;
	pthread_t netlink_tid, vcc_tid;
	char * logfile = (char*)0;
	
	default_config_init();
	while ((opt = getopt(argc,argv,"t:l:h")) != -1) {
		switch (opt) {
			case 'h':
				printf("acc_monitor [-t interval] [-l logfile]\n");
				printf("          -h (get help)\n");
				printf("          -t 10 (vcc report interval 5~300 sec)\n");
				printf("          -l /tmp/vcc.log (logfile for vcc)\n");
				return 0;				
			case 'l':
				logfile = optarg;			
				/* Log file. */
				if (logfile != (char*)0) {
					logfp = fopen(logfile, "a");
					if (logfp == (FILE*)0) {
						syslog(LOG_CRIT, "%s - %m", logfile);
						perror(logfile);
						exit(1);
					}
					if (logfile[0] != '/') {
						syslog( LOG_WARNING, "logfile is not an absolute path, you may not be able to re-open it" );
						(void) fprintf( stderr, "%s: logfile is not an absolute path, you may not be able to re-open it\n", argv[0] );
					}
				}
				break;
			case 't':
				vcc.vcc_interval = strtoul(optarg, NULL, 10);
				if ((vcc.vcc_interval < 5)||(vcc.vcc_interval > 300)) {
					printf("		  -t 10 (vcc report interval 5~300 sec)\n");
					vcc.vcc_interval = 10;
				}
				break;
			default:
				break;
		}
	}

	acc_monitor_init();
#ifdef ACC_UCI
	uci_monitor_init();
#endif

	err = pthread_create(&netlink_tid, NULL, netlink, NULL);
	if (err != 0)
		printf("can't create thread: %s\n", strerror(err));

	err = pthread_create(&vcc_tid, NULL, vcc_monitor, NULL);
	if (err != 0)
		printf("can't create thread: %s\n", strerror(err));

#ifdef ACC_UBUS
	const char *ubus_socket = NULL;

	uloop_init();
	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		fprintf(stderr, "%s:Failed to connect to ubus\n", __FILE__);
		return -1;
	}
	ubus_add_uloop(ctx);
	server_main();
	ubus_free(ctx);
	uloop_done();
#endif

	if (logfp != (FILE*)0)
		fclose(logfp);
	if (fd_acc != 0)
		close(fd_acc);
	return 0;
}


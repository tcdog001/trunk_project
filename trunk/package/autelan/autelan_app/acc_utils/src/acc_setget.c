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
acc_op_args acc_u_op_args;

#define ACC_UBUS
#ifdef ACC_UBUS
#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/utils.h>

#include <libubus.h>
#include <libubox/avl.h>
#include <libubox/safe_list.h>

#include <libubox/blobmsg_json.h>
//#include "libubus.h"
static struct ubus_context *ctx;
#endif

static struct acc_struct {
	int timer;
	int trigger;
	char *controller;
}acc = {0, 0, "none"};

enum {
	RETURN_STATUS,
	_RETURN_MAX
};
static const struct blobmsg_policy return_policy[] = {
	[RETURN_STATUS] = { .name = "return_status", .type = BLOBMSG_TYPE_INT8 },
	
};
static void receive_call_result_data(struct ubus_request *req, int type, struct blob_attr *msg)
{	
	struct blob_attr *tb[_RETURN_MAX];
	uint8_t result;	
	
	if (!msg)
		return;

	blobmsg_parse(return_policy, ARRAY_SIZE(return_policy), tb, blob_data(msg), blob_len(msg));

	result = blobmsg_get_u8(tb[RETURN_STATUS]);

	printf("%s:receive is status %d\n", __FILE__, result);

	return ;
}
static void client_main(void)
{
	uint32_t id;
	static struct blob_buf b;
	
	if (ubus_lookup_id(ctx, "autelan.acc", &id)) {
		fprintf(stderr, "%s:Failed to look up test object\n", __FILE__);
		return;
	}

	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "timer", 300);
	blobmsg_add_string(&b, "controller", "acc");
	
	ubus_invoke(ctx, id, "create", b.head, receive_call_result_data, 0, 3000);

//	uloop_run();
}
static void get_status(void)
{
	uint32_t id;
	static struct blob_buf b;
#if 1	
	if (ubus_lookup_id(ctx, "autelan.acc", &id)) {
		fprintf(stderr, "%s:Failed to look up test object\n", __FILE__);
		return;
	}

	blob_buf_init(&b, 0);
	
	ubus_invoke(ctx, id, "status", b.head, receive_call_result_data, 0, 3000);
#else
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "timer", acc.timer);
	blobmsg_add_u32(&b, "trigger", acc.trigger);
	if (acc.controller)
		blobmsg_add_string(&b, "controller", acc.controller);

	ubus_send_reply(ctx, req, b.head);
#endif
}
static void set_timer(int time)
{
	uint32_t id;
	static struct blob_buf b;
	
	if (ubus_lookup_id(ctx, "autelan.acc", &id)) {
		fprintf(stderr, "%s:Failed to look up test object\n", __FILE__);
		return;
	}

	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "timer", time);

	ubus_invoke(ctx, id, "create", b.head, receive_call_result_data, 0, 3000);
}
static void set_controller(char *controller)
{
	uint32_t id;
	static struct blob_buf b;
	
	if(NULL == controller)
		return;

	if (ubus_lookup_id(ctx, "autelan.acc", &id)) {
		fprintf(stderr, "%s:Failed to look up test object\n", __FILE__);
		return;
	}

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "controller", controller);
	
	ubus_invoke(ctx, id, "create", b.head, receive_call_result_data, 0, 3000);
}
static void client_init(void)
{
	const char *ubus_socket = NULL;

	uloop_init();				
	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		fprintf(stderr, "%s:Failed to connect to ubus\n", __FILE__);
		return;
	}
	
	ubus_add_uloop(ctx);
}
static void test_client_subscribe_cb(struct ubus_context *ctx, struct ubus_object *obj)
{
	fprintf(stderr, "%s:Subscribers active: %d\n", __FILE__, obj->has_subscribers);
}
static struct ubus_object test_client_object = {
	.subscribe_cb = test_client_subscribe_cb,
};
static void test_client_notify_cb(struct uloop_timeout *timeout)
{
	static struct blob_buf tell_b;
	static int counter = 0;
	int err;
	struct timeval tv1, tv2;
	int max = 10;
	long delta;
	int i = 0;

	blob_buf_init(&tell_b, 0);
	blobmsg_add_u32(&tell_b, "counter", counter++);

	gettimeofday(&tv1, NULL);
	for (i = 0; i < max; i++)
		err = ubus_notify(ctx, &test_client_object, "ping", tell_b.head, 1000);
	gettimeofday(&tv2, NULL);
	if (err)
		fprintf(stderr, "%s:Notify failed: %s\n", __FILE__, ubus_strerror(err));

	delta = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
	fprintf(stderr, "%s:Avg time per iteration: %ld usec\n", __FILE__, delta / max);

	uloop_timeout_set(timeout, 1000);
}

static struct uloop_timeout notify_timer = {
	.cb = test_client_notify_cb,
};
static void tell_monitor(void)
{
	uint32_t id;
	int ret;
	static struct blob_buf b;

	ret = ubus_add_object(ctx, &test_client_object);
	if (ret) {
		fprintf(stderr, "%s:Failed to add_object object: %s\n", __FILE__, ubus_strerror(ret));
		return;
	}

	if (ubus_lookup_id(ctx, "autelan.acc", &id)) {
		fprintf(stderr, "%s:Failed to look up autelan.acc object\n", __FILE__);
		return;
	}

	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "id", test_client_object.id);
	ubus_invoke(ctx, id, "watch", b.head, NULL, 0, 3000);
	test_client_notify_cb(&notify_timer);
	uloop_run();

}
static void client_quit(void)
{
	ubus_free(ctx);
	uloop_done();
}
int main(int argc, char **argv) 
{
	int ret, opt;
	const char *ubus_socket = NULL;

	client_init();
	while ((opt = getopt(argc,argv,"psrc:t:h")) != -1)
	{
		switch (opt) {
			case 'h':
				printf("check_lte\n");
				printf("          -h (get help, exist immediately)\n");
				printf("          -t 300 (set acc_shut_down Timer(sec) of autelan.acc)\n");
				printf("          -c controller (set acc_shut_down Controller of autelan.acc)\n");
				printf("          -r (get status of autelan.acc in ubus)\n");
				return 0;				
			case 't':
				acc.timer = strtoul(optarg,NULL,10);
				set_timer(acc.timer);
				break;
			case 'c':
				acc.controller = optarg;
				set_controller(acc.controller);
				break;
#if 0
			case 's':
				client_init();
				client_main();				
				break;
#endif
			case 'r':
				get_status();				
				break;
			case 'p':
				tell_monitor();
				break;
			default :
				printf("wrong input\n");
				break;
		}		
	}
	client_quit();
	return 0;
}


static unsigned int gpio_set(acc_op_args * ops);
static unsigned int gpio_get(acc_op_args * ops);
void signal_f(int signum);
static void acc_ubus_reply(struct ubus_request_data *req);

static int acc_create(struct ubus_context *ctx, struct ubus_object *obj,
			  struct ubus_request_data *req, const char *method,
			  struct blob_attr *msg);
static int acc_status(struct ubus_context *ctx, struct ubus_object *obj,
			  struct ubus_request_data *req, const char *method,
			  struct blob_attr *msg);
static void test_handle_remove(struct ubus_context *ctx, struct ubus_subscriber *s,
                   uint32_t id);
static int test_notify(struct ubus_context *ctx, struct ubus_object *obj,
	    struct ubus_request_data *req, const char *method,
	    struct blob_attr *msg);
static int acc_watch(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);
static int ubus_gpio_get(struct ubus_context *ctx, struct ubus_object *obj,
			  struct ubus_request_data *req, const char *method,
			  struct blob_attr *msg);
static int ubus_gpio_set(struct ubus_context *ctx, struct ubus_object *obj,
				  struct ubus_request_data *req, const char *method,
				  struct blob_attr *msg);
static int acc_SIGIO_init(void);

static int acc_reload(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg);
static struct uci_package *config_init_package(const char *config);
static void config_init_acc(void);

void *uci_monitor_init(void);
void get_controll_flag(void);
void set_controll_flag(char *str);


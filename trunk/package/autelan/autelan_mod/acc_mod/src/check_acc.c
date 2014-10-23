/****************************
 *
 * to use in the kernel space 
 * the module of acc(for LTEFI_V3)
 * autelan
 * 2014.04.13
 *
 ****************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <net/sock.h>  
#include <net/netlink.h>

//#include <linux/gpio.h>

#include "check_acc.h"
#include "ar9350.h"

#define ACC_MINOR 	134
#define BUTTON_WAIT 3 		/*button irq wait*/
#ifdef LTEFI_V2
#define ACC_GPIO  	11
#define SHUT_GPIO 	12
#else
#define ACC_GPIO  					17
#define SHUT_GPIO 					25		/*AR934X_GPIO_COUNT+2*/
#define DB120_GPIO_POWER			18
#define DB120_GPIO_FORCE_SHUTDOWN	19

#endif
#define ACC_TIMER 			100
#define DELAY_AFTER_OFF 	10

struct timer_list 		acc_timer;
struct timer_list 		gcc_timer;
struct timer_list 		sig_timer;
struct tasklet_struct 	acc_tasklet;
struct semaphore 		acc_sem;
struct fasync_struct 	*acc_async;

acc_op_args acc_ioc_op_args;
static int 	acc_is_open;
static int 	acc_flag 	= 0;
static int 	acc_flag1	= 0;
static int 	acc_time 	= 1;
static int 	gcc_time 	= 1;
static int 	total_time 	= 0;
static int 	total_time1 = ACC_DELAY_TIMER;
static int 	sem_ret = 0;

#if 1
// --------- These are for netlink --------- //  
#define MAX_MSGSIZE 1024  
#define NETLINK_ACCMODULE     22  
struct sock *nl_sk = NULL;  
int pid;  
struct msghdr msg;  
static void sendnlmsg(char *message);  
static int msg_flag = 0;
static int delay_flag = 0;
#endif

void turn_off_pwr_led(void)
{
	ar7100_gpio_config_output(DB120_GPIO_POWER);
	ar7100_gpio_out_val(DB120_GPIO_POWER, 1);
}

void turn_on_pwr_led(void)
{
	ar7100_gpio_config_output(DB120_GPIO_POWER);
	ar7100_gpio_out_val(DB120_GPIO_POWER, 0);
}

void ap_shutdown(unsigned long val)
{
	if (1 == msg_flag) {
		sendnlmsg("delay off");
	}
	printk("acc_mod: delay off, Send SIGIO!\n");
	kill_fasync(&acc_async, SIGIO, POLL_IN);
	delay_flag ++;
	del_timer(&sig_timer);
	if (delay_flag < 6) {
		sig_timer.expires = jiffies + DELAY_AFTER_OFF*ACC_TIMER;
		add_timer(&sig_timer);
	} else {
		kernel_restart("restart for acc off");
	}
}

void check_acc_stage2(unsigned long val)
{
	unsigned int reg_val = 0, button_flag = 0;

	reg_val = ar7100_gpio_in_val(ACC_GPIO);
	//printk("%s: reg_val=%d\n", __func__, reg_val);
	if (1 == reg_val) {
		button_flag = 0;
	} else {	
		button_flag = 1;
	}
	
	if (1 == button_flag) {
		printk("acc_mod: Timer stop!\n");
		//printk("%s %d, unlock=%d\n\n", __func__, __LINE__, acc_sem);
		up(&acc_sem);
		//printk("%s %d, unlock=%d\n\n", __func__, __LINE__, acc_sem);
		acc_flag = 0;
		turn_on_pwr_led();
		if (1 == msg_flag) {
			sendnlmsg("acc on");
		}

		printk("acc_mod: acc on, Send SIGIO!\n");
		kill_fasync(&acc_async, SIGIO, POLL_IN);

		gcc_timer.expires = jiffies + gcc_time*ACC_TIMER;
		add_timer(&gcc_timer);

	} else if (total_time >= total_time1) {
		printk("acc_mod: Time's up!\n");
		//printk("%s %d, unlock=%d\n\n", __func__, __LINE__, acc_sem);
		up(&acc_sem);
		//printk("%s %d, unlock=%d\n\n", __func__, __LINE__, acc_sem);
		acc_flag = 0;

		if (1 == msg_flag) {
			sendnlmsg("delay off");
		}

		printk("acc_mod: delay off, Send SIGIO!\n");
		kill_fasync(&acc_async, SIGIO, POLL_IN);

		del_timer(&sig_timer);
		sig_timer.function = ap_shutdown;
		sig_timer.expires = jiffies + DELAY_AFTER_OFF*ACC_TIMER;
		add_timer(&sig_timer);
		//printk("%s %d, unlock=%d\n\n", __func__, __LINE__, acc_sem);

	} else {
		acc_timer.expires = jiffies + acc_time*ACC_TIMER;
		total_time += acc_time;
		add_timer(&acc_timer);
//		printk("ACC_TIMER: Time is %d(s),\n", total_time);

	}
}

void check_acc_stage1(unsigned long val)
{
	unsigned int reg_val = 0, button_flag = 0;

	reg_val = ar7100_gpio_in_val(ACC_GPIO);
	//printk("%s: reg_val=%d, acc_flag1=%d\n", __func__, reg_val, acc_flag1);
	if (1 == reg_val) {
		acc_flag1 ++;
		if (acc_flag1 >= 3) {
			acc_flag1 = 0;
			button_flag = 1;
		}
	} else {	
		acc_flag1 = 0;
		button_flag = 0;
	}
	
	if (button_flag) {
		//printk("%s %d, lock=%d\n\n", __func__, __LINE__, acc_sem);
		sem_ret = down_trylock(&acc_sem);
		//printk("%s %d, lock=%d, sem_ret=%d\n\n", __func__, __LINE__, acc_sem, sem_ret);

		if (sem_ret == 0) {
			total_time = 0;
			tasklet_schedule(&acc_tasklet);
			return;
		}
	}

	gcc_timer.expires = jiffies + gcc_time*ACC_TIMER;
	add_timer(&gcc_timer);

	return;
}

void tasklet_action(unsigned long arg)
{
	acc_flag = 1;
	turn_off_pwr_led();
	if (1 == msg_flag) {
		sendnlmsg("acc off");
	}

	del_timer(&acc_timer);

	acc_timer.function = check_acc_stage2;
	acc_timer.expires = jiffies + acc_time*ACC_TIMER;
	total_time += acc_time;

	printk("acc_mod: acc off, Send SIGIO!\n");
	kill_fasync(&acc_async, SIGIO, POLL_IN);

	printk("acc_mod: Delay shutdown %d(s), recheck interval %d(s)\n", total_time1, acc_time);
	printk("acc_mod: Timer start!\n");
	add_timer(&acc_timer);
}

static long
acc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
//	acc_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int gpio_pin, gpio_val, ret;
	long reg_addr, reg_value;
	
#if 0
	printk("\n\n\n-----acc_ioctl-----((%d) >> %d) & %d\n", cmd,_IOC_TYPESHIFT,_IOC_TYPEMASK);
    printk("\n-----acc_ioctl-----%d == %d\n", _IOC_TYPE(cmd), (((cmd) >> _IOC_TYPESHIFT) & _IOC_TYPEMASK));
	printk("\n-----acc_ioctl-----((%d) >> %d) & %d\n", cmd,_IOC_NRSHIFT,_IOC_NRMASK);
	printk("\n-----acc_ioctl-----%d == %d\n", _IOC_NR(cmd), (((cmd) >> _IOC_NRSHIFT) & _IOC_NRMASK));
#endif
	if (_IOC_TYPE(cmd) != ACC_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > ACC_IOC_MAXNR) return -ENOTTY;

	memset(&acc_ioc_op_args, 0, sizeof(acc_op_args));

	switch (cmd) {
		case SET_ACC_GPIO_TIMER:
			if (0 == acc_flag) {
				copy_from_user(&acc_ioc_op_args,(acc_op_args *)arg,sizeof(acc_op_args));
				total_time1 = acc_ioc_op_args.num;		 
				printk("acc_mod: set acc_time=%d(sec)\n", total_time1);
			} else {
				printk("acc_mod: acc_time=%d(sec), Cannot be modified\n", total_time1);
			}
			break;

		case GET_ACC_GPIO_TIMER:
			if (0 == acc_flag) {
				printk("acc_mod: get acc_time=%d(sec)\n", total_time1);
			} else {
				printk("acc_mod: acc_time=%d(sec), Cannot be modified\n", total_time1);
			}
			break;

		case SET_GPIO:
			copy_from_user(&acc_ioc_op_args, (acc_op_args *)arg, sizeof(acc_op_args));
			gpio_pin = acc_ioc_op_args.num;		 
			gpio_val = acc_ioc_op_args.status;		 

			if (ACC_GPIO != gpio_pin) {
				//ar7100_gpio_config_output(gpio_pin);
				ar7100_gpio_out_val(gpio_pin, (gpio_val & 0x1));		
//				printk("\nset: gpio %d = %d\n", gpio_pin, (gpio_val & 0x1));
			}
			break;

		case GET_GPIO:
			copy_from_user(&acc_ioc_op_args, (acc_op_args *)arg, sizeof(acc_op_args));
			gpio_pin = acc_ioc_op_args.num;		 
			if (ACC_GPIO == gpio_pin) {
				gpio_val = ar7100_gpio_in_val(gpio_pin);
			} else {
				gpio_val = ar7100_gpio_get_out_val(gpio_pin);
			}
//			printk("\nget: gpio %d = %d\n", gpio_pin, gpio_val);
			acc_ioc_op_args.status = gpio_val;
			ret = copy_to_user((acc_op_args *)arg, &acc_ioc_op_args, sizeof(acc_ioc_op_args));
//			printk("copy_to_user=%d\n", ret);
			break;

		case SET_REGISTER:
			copy_from_user(&acc_ioc_op_args, (acc_op_args *)arg, sizeof(acc_op_args));
			reg_addr = acc_ioc_op_args.op_addr; 	 
			reg_value = acc_ioc_op_args.op_value;		 
			ar7100_reg_wr(reg_addr, reg_value);
			break;
		
		case GET_REGISTER:
			copy_from_user(&acc_ioc_op_args,(acc_op_args *)arg,sizeof(acc_op_args));
			reg_addr = acc_ioc_op_args.op_addr; 	 
			reg_value = ar7100_reg_rd(reg_addr);
		
//			printk("\nget: gpio %d = %d\n", reg_addr, reg_value);
			break;
		default:
			return -1;
			break;
	}

	return 0;
}

 static int acc_open(struct inode* inode, struct file* file)
 {
	 switch (MINOR(inode->i_rdev)) {
		 case ACC_MINOR:
			 if (acc_is_open)
				 return -EBUSY;
			 acc_is_open = 1;

			 return 0;

		 default:
			 return -ENODEV;
	 }
 }
static int acc_close(struct inode* inode, struct file* file)
{
	if (MINOR(inode->i_rdev) == ACC_MINOR) {
		acc_is_open = 0;
	}

	return 0;
}
static int acc_fasync(int fd, struct file* filp, int mode)
{
	return fasync_helper(fd, filp, mode, &acc_async);
}

int stringlength(const char *s)  
{  
    int slen = 0;  
    for(; *s; s++){  
        slen++;  
    }  
    return slen;  
}  

static void sendnlmsg(char *message)  
{  
    struct sk_buff *skb_1;  
    struct nlmsghdr *nlh;  
    int len = NLMSG_SPACE(MAX_MSGSIZE);  
    int slen = 0;  
    char buffer[128];  
//    const char *message = "hello i am kernel";  
    if (!message || !nl_sk) {  
        return ;  
    }  
    skb_1 = alloc_skb(len, GFP_KERNEL);  
    if (!skb_1) {  
        printk(KERN_ERR "my_net_link:alloc_skb_1 error\n");  
    }  
    nlh = nlmsg_put(skb_1,0,0,0,MAX_MSGSIZE,0);  
  
    NETLINK_CB(skb_1).portid = 0;  
    NETLINK_CB(skb_1).dst_group = 0;  
  
    slen = stringlength(message);  
    memset(buffer, 0, sizeof(buffer));  
    memcpy(buffer, message, slen);  
    memcpy(NLMSG_DATA(nlh), buffer, slen+1);  
//    printk("my_net_link:send message '%s'.\n", (char *)NLMSG_DATA(nlh));  
  
    netlink_unicast(nl_sk, skb_1, pid, MSG_DONTWAIT);  
}  

void nl_data_ready(struct sk_buff *__skb)  
{  
    struct sk_buff *skb;  
    struct nlmsghdr *nlh;  
    char str[100];  
    struct completion cmpl;  
    int i = 10;  

	skb = skb_get(__skb);  
	if(skb->len >= NLMSG_SPACE(0)){  
        nlh = nlmsg_hdr(skb);  
  
        memcpy(str, NLMSG_DATA(nlh), sizeof(str));  
//        printk("Message received:%s\n",str) ;  
        pid = nlh->nlmsg_pid;  

		while(i--){  
            //init_completion(&cmpl);  
            //wait_for_completion_timeout(&cmpl, 3 * HZ);  
            sendnlmsg("hello i am kernel");  
            break;  
        }  
        msg_flag = 1;  
        kfree_skb(skb);  
    }  
  
 } 

static int build_netlink(void)  
{  
	struct netlink_kernel_cfg cfg = {
		.input  = nl_data_ready,
	};
	nl_sk = netlink_kernel_create(&init_net, NETLINK_ACCMODULE, &cfg);  
//    nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, 1,nl_data_ready, NULL, THIS_MODULE);  
  
    if(!nl_sk){  
        printk(KERN_ERR "my_net_link: create netlink socket error.\n");  
        return 1;  
    }  
  
//    printk("my_net_link_3: create netlink socket ok.\n");  
    return 0;  

}  
static void destroy_netlink(void)  
{
	if(nl_sk != NULL)  
	{  
		sock_release(nl_sk->sk_socket);  
	}  
//    printk("my_net_link: self module exited\n");  
} 

static struct file_operations acc_fops = { \
	owner:		THIS_MODULE,	\
	open:		acc_open,		\
	release:	acc_close,		\
	fasync:		acc_fasync,		\
	unlocked_ioctl:	acc_ioctl,	\
};

static struct miscdevice acc_miscdev = { \
	ACC_MINOR,	\
	"acc",		\
	&acc_fops,	\
};

static int 
acc_gpio_timer_init(void)
{
	init_timer(&acc_timer);
	init_timer(&sig_timer);
	init_timer(&gcc_timer);

	//printk("GCC_TIMER start lalal!\n");
	gcc_timer.function = check_acc_stage1;
	gcc_timer.expires = jiffies + gcc_time*ACC_TIMER;
	add_timer(&gcc_timer);
	//printk("check interval %d(s)\n", gcc_time);

	return 0;
}

static int 
acc_gpio_timer_exit(void)
{
	del_timer(&gcc_timer);
	del_timer(&acc_timer);
	del_timer(&sig_timer);

	return 0;
}
//extern int __gpio_get_value(unsigned gpio);
//extern void __gpio_set_value(unsigned gpio, int value);

static int __init
acc_init(void)
{
	printk("acc_mod init\n");
	printk(KERN_INFO "MAJOR:10 MINOR:%d\n", ACC_MINOR);
	misc_register(&acc_miscdev);

	/* init semaphore */
	sema_init(&acc_sem, 1);

	/* init tasklet */
	tasklet_init(&acc_tasklet, tasklet_action, 789);

#if LTEFI_V2
	/* 0:enable LTE 1 & 2; 1:disable LTE 1 & 2 */
	ar7100_gpio_config_output(16);
	ar7100_gpio_out_val(16, 0);		
#endif

#if LTEFI_V2
	/* init gpio for acc */
	ar7100_gpio_config_output(SHUT_GPIO);
	ar7100_gpio_out_val(SHUT_GPIO, 0);
	ar7100_gpio_config_input(ACC_GPIO);
#else
	/* init gpio for acc */
	ar7100_gpio_config_output(SHUT_GPIO);
	ar7100_gpio_out_val(SHUT_GPIO, 0);
	ar7100_gpio_config_input(ACC_GPIO);
#endif

	/* init timer */
	acc_gpio_timer_init();

#if 0
	/* read watchdog */
	int val = ar7100_reg_rd(0x18060008);
	printk("_reg_rd(0x%x: 0x%x)\n", 0x18060008, val);
	val = ar7100_reg_rd(0x1806000c);
	printk("_reg_rd(0x%x: 0x%x)\n", 0x1806000c, val);
#endif

#if 1
	// --------- These are for netlink --------- //  
	build_netlink();

#endif



	/* init power led */
	//turn_on_pwr_led();
	

	return 0;
}

static void __exit
acc_exit(void)
{
	/* del timer */
	acc_gpio_timer_exit();
	destroy_netlink();

	/* del tasklet */
	tasklet_kill(&acc_tasklet);

	misc_deregister(&acc_miscdev);
	printk(KERN_INFO "acc_mod exit\n");
	printk(KERN_INFO "MAJOR:10 MINOR:%d\n", ACC_MINOR);
}
	
module_init(acc_init);
module_exit(acc_exit);

MODULE_AUTHOR("liuhj@autelan");
MODULE_DESCRIPTION("Support for Atheros WiSoC Register");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif


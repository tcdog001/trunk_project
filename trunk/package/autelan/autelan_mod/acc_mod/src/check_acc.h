#ifndef CHECK_ACC_H
#define CHECK_ACC_H

typedef struct acc_op_args_s {
	unsigned long  	op_addr;
	unsigned long  	op_value; 	/* ignore on read in arg, fill read value on write return value */
	unsigned int	offset;
	unsigned short 	op_len;
	unsigned short 	op_ret; 	/* 0 for success, other value indicate different failure. */
	unsigned int 	num;		/* which gpio */
	unsigned int 	status;		/* gpio value 0/1 */
} acc_op_args;

enum {
	SET_TIMER_IOCTL,
	GET_TIMER_IOCTL,
	SET_GPIO_IOCTL,
	GET_GPIO_IOCTL,
	SET_REG_IOCTL,
	GET_REG_IOCTL,
	__IOCTL_MAX
};

#define ACC_IOC_MAGIC 10 
#define ACC_IOC_RESET		_IO(ACC_IOC_MAGIC,0)
#define SET_ACC_GPIO_TIMER 	_IOWR(ACC_IOC_MAGIC,SET_TIMER_IOCTL,acc_op_args)	/* SET_ACC_GPIO_TIMER */
#define GET_ACC_GPIO_TIMER 	_IOWR(ACC_IOC_MAGIC,GET_TIMER_IOCTL,acc_op_args)	/* GET_ACC_GPIO_TIMER */
#define SET_GPIO			_IOWR(ACC_IOC_MAGIC,SET_GPIO_IOCTL,acc_op_args)		/* SET_GPIO */
#define GET_GPIO			_IOWR(ACC_IOC_MAGIC,GET_GPIO_IOCTL,acc_op_args)		/* GET_GPIO */
#define SET_REGISTER		_IOWR(ACC_IOC_MAGIC,SET_REG_IOCTL,acc_op_args)	/* SET_REGISTER */
#define GET_REGISTER		_IOWR(ACC_IOC_MAGIC,GET_REG_IOCTL,acc_op_args)	/* GET_REGISTER */
#define ACC_IOC_MAXNR 		__IOCTL_MAX

#define ACC_DELAY_TIMER 60

#endif

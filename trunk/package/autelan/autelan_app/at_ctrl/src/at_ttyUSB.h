#ifndef __AT_TTYUSB_H_04c8447f0f5e440ea3bdf0cfc2a27949__
#define __AT_TTYUSB_H_04c8447f0f5e440ea3bdf0cfc2a27949__

static inline void serial_init(int fd)
{
	struct termios options;
	tcgetattr(fd, &options);
	options.c_cflag |= ( CLOCAL | CREAD );
	options.c_cflag &= ~CSIZE;
	options.c_cflag &= ~CRTSCTS;
	options.c_cflag |= CS8;
	options.c_cflag &= ~CSTOPB;
	options.c_iflag |= IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;

//	cfsetispeed(&options, B9600);
//	cfsetospeed(&options, B9600);
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);
	tcsetattr(fd,TCSANOW,&options);
}

static inline int send(int fd, char *cmdbuf)
{
	int nread,nwrite;
	char buff[128];
	char reply[128];
	char *pbuf = buff;
	(void)cmdbuf;
	memset(buff,0,sizeof(buff));
	pbuf += sprintf(pbuf, "%s", cmdbuf);
	pbuf += sprintf(pbuf, "\r");


	nwrite = write(fd,buff,strlen(buff));

	sleep(1);
	memset(reply,0,sizeof(reply));
	nread = read(fd,reply,sizeof(reply));
	printf("Response(%d):%s\n",nread,reply);

	return 0;
}

static inline int at_ttyUSB_main(int argc, char *argv[])
{
	int fd;
	char cmdbuf[128];

	fd = open( TTYUSB, O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 != fd)
	{
		if (argv[1] == NULL)
		{
			printf("Usage: ctrl_lte [AT command]\n");
		}
		else
		{	
			serial_init(fd);
			printf("Please input AT command cmd=");
			send(fd, argv[1]);
		}
		close(fd);
	}
	else 
		perror("Can't Open Serial Port");

	return 0;
}
#endif /* __AT_TTYUSB_H_04c8447f0f5e440ea3bdf0cfc2a27949__ */

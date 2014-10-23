#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>
#include<netdb.h>
#include<sys/socket.h>
#include<stdarg.h>
#include<syslog.h>
#include<sys/time.h>

#define RD_SIZE 1024
//#define TEST //if dont have the gps enviorment , use define TEST

static struct gps_struct {
	char str_host[20];
	char east_west;
	char north_south;
	char gps_Lng[20];
	char gps_Lat[20];
	char gps_Date[30];
	char gps_Time[30];
	char gps_Velocity[10];
	char gps_Orientation[10];
	char gps_Elevation[10];
}rgps;
static FILE* logfp;
int flag_GPGGA;
int flag_GPRMC;

int print_timestr(char *str)
{
	char *sep = ":";
	char *token;
	unsigned long tm_int[3]; 
	int i = 1;

	if (NULL == str) {
		return -1;
	}

	token = strtok(str, sep);
	if (NULL != token) {
		tm_int[0] = strtoul(token, NULL, 10);
	}

	while ((token != NULL)&&(i < 3)) {
		token = strtok(NULL, sep);
		if (NULL != token) {
			tm_int[i] = strtoul(token, NULL, 10);
		}
		i++;
	}

	tm_int[0] = tm_int[0] + 8;
	if (tm_int[0] > 24) {
		tm_int[0] = tm_int[0]- 24;
	}

	//printf("Time      %02d:%02d:%02d \n", tm_int[0], tm_int[1], tm_int[2]);
	sprintf(rgps.gps_Time, "%02d-%02d-%02d", tm_int[0], tm_int[1], tm_int[2]);

	return 0;
}

void serial_init(int fd)
{
	struct termios options;
	tcgetattr(fd, &options);
	options.c_cflag |= ( CLOCAL|CREAD );
	options.c_cflag &= ~CSIZE;
	options.c_cflag &= ~CRTSCTS;
	options.c_cflag |= CS8;
	options.c_cflag &= ~CSTOPB;
	options.c_iflag |= IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;

	options.c_cc[VTIME]	= 0;
	options.c_cc[VMIN] = 500;				//VMIN:非规范模式读取时的最小字符数
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);
	tcsetattr(fd, TCSANOW, &options);
}

int PopenFile (char *cmd_str, char *str, int len)
{
	FILE *fp=NULL; 

	if (cmd_str == NULL||str == NULL)
		return -1;

	memset(str, 0, len);		   
	fp = popen(cmd_str, "r");  
	//	printf("%s\n", cmd_str);
	if (fp) {	   
		fgets(str, len, fp);	   
		if (str[strlen(str)-1] == '\n')	{		   
			str[strlen(str)-1] = '\0';	   
		}	   
		pclose(fp); 	   
		return 0;    
	} else {	   
		perror("popen");	   
		str = NULL;
		return -1;   
	}
	return 0;
}

void print_gpsinfo(char *buf)
{
	char ReceivingF = 0;
	char EndF = 0;
	char NewByteF = '0';

	unsigned char GPS_time[9] = {0};			//UTCê±??
	unsigned char GPS_wd[20] = {0};				//?3?è
	unsigned char GPS_jd[12] = {0};				//?-?è
	unsigned char GPS_status = 0;				// ?¨??×′ì?
	unsigned char GPS_alt[8] = {0};				//o￡°?
	unsigned char GPS_sv[3] = {0};				//ê1ó??àD?
	unsigned char GPS_precision[6] = {0};		//???????è
	unsigned char GPS_speed[6] = {0};			//?ù?è
	unsigned char GPS_direction[7] = {0};		//·??ò??
	unsigned char GPS_date[9] = {0};			//UTCè??ú	
	unsigned char GPS_warn = 0;					//?¨???ˉ??
	unsigned char Segment = 0;					//?oo???êy
	unsigned char null_flag = 0;
	unsigned char Bytes_counter = 0;
	unsigned char Command = 0;

	if (buf == NULL) {
		return;
	}
	if (*buf == '\n') {
		*buf ++;
	}
//	printf("----------%c---------\n", *buf);
	if (*buf == '$') {				//?eê?±ê??
		ReceivingF = 1;
		Bytes_counter = 0;
		Segment = 0;   				//????ó???????êy?÷
		*buf ++;
	}
	
//	printf("----------%d-%d-%d-------\n", ReceivingF, Bytes_counter, Segment);
	while (ReceivingF)
	{                
//		printf("----------%c---------\n", *buf);
		if (*buf == NULL) {
			return;
		}
		if (*buf == ',') {
			++Segment;
			*buf ++;
			Bytes_counter = 0;		//??????×??ú??êy?÷
		}
		if (*buf == '*') {			//ê?μ??áê?±ê??
			//Command = 0;
			//ReceivingF = 0;
			//EndF = 1;

			if ((Command == 1)&&(flag_GPGGA == 0)) {

				/*printf("Latitude  %s \n", GPS_wd);
				printf("north_south%c \n", rgps.north_south);
				printf("Longitude %s \n", GPS_jd);
				printf("east_west %c\n", rgps.east_west);*/

				print_timestr(GPS_time);
				//printf("UTC time %s \n", GPS_time);
				
				/*printf("Elevation %s \n", GPS_alt);*/
				//printf("Satellites %s \n", GPS_sv);
				//printf("Locate status %c \n", GPS_status);
				flag_GPGGA=1;

				memset(rgps.gps_Lng, 0, sizeof(rgps.gps_Lng));
				memcpy(rgps.gps_Lng, GPS_jd, strlen(GPS_jd));
				memset(rgps.gps_Lat, 0, sizeof(rgps.gps_Lat));
				memcpy(rgps.gps_Lat, GPS_wd, strlen(GPS_wd));
				memset(rgps.gps_Elevation, 0, sizeof(rgps.gps_Elevation));
				memcpy(rgps.gps_Elevation, GPS_alt, strlen(GPS_alt));
			}
			if (Command == 2&&(flag_GPRMC == 0)) {
				/*printf("Speed     %s \n", GPS_speed);
				printf("UTC date  %s \n", GPS_date);
				//printf("Locate warn %c \n", GPS_warn);
				printf("Direction %s \n", GPS_direction);*/
				flag_GPRMC=1;

				memset(rgps.gps_Velocity, 0, sizeof(rgps.gps_Velocity));
				memcpy(rgps.gps_Velocity, GPS_speed, strlen(GPS_speed));
				memset(rgps.gps_Date, 0, sizeof(rgps.gps_Date));
				memcpy(rgps.gps_Date, GPS_date, strlen(GPS_date));
				memset(rgps.gps_Orientation, 0, sizeof(rgps.gps_Orientation));
				memcpy(rgps.gps_Orientation, GPS_direction, strlen(GPS_direction));
			}
			return;
		}
		
		switch (Segment) {					//分段处理
			case 0:
				if (Bytes_counter == 3) 	//段0，语句类型判断
				switch (*buf) {
					case 'G':
						if (1 == flag_GPGGA) {	
							Command = 0;
							ReceivingF = 0;
						} else
							Command = 1;	//语句类型 $GPGGA
						break;
					case 'M':
						if (1 == flag_GPRMC) {
							Command = 0;
							ReceivingF = 0;
						} else						
							Command = 2;	//语句类型 $GPRMC
						break;
#if 0
					case 'T':
						Command = 3;		//this is GPVTG
						break;	
#endif
					default:
						Command = 0; 		//非有效数据类型，终止当前数据接收
						ReceivingF = 0;
					break;
				}
				break;
			
			case 1:
				switch (Command) {
					case 1:        
						if ((Bytes_counter == 2)||(Bytes_counter == 5)) {        //$GPGGA段1，UTC时间，hhmmss（时分秒）格式,取前6位 转换为HH:MM:SS格式
				        	GPS_time[Bytes_counter] = *buf;
				        	GPS_time[Bytes_counter] = ':';
							Bytes_counter++;
						}
				        //if(Bytes_counter>1 && Bytes_counter<4)
							//GPS_time[Bytes_counter] = *buf;
						if (Bytes_counter < 8) {
							GPS_time[Bytes_counter] = *buf;
						}
						if (Bytes_counter == 8) {
							GPS_time[Bytes_counter]='\0';
						}
				    	break;

					case 2:
						break;						//$GPRMC第1段处理 忽略
					case 3:
						if (Bytes_counter < 6) {	//$GPVTG第1段处理 地面航向
							GPS_direction[Bytes_counter] = *buf;
							GPS_direction[Bytes_counter+1] = '\0';
						}
						break;
				}
				break;

			case 2:
				switch (Command) {
					case 1:        
				        if (Bytes_counter == 10) {
							GPS_wd[Bytes_counter] = '\0';
							break;
				        }
						if (Bytes_counter == 2) {               //$GPGGA 段2处理 纬度ddmm.mmmm（度分）格式
							GPS_wd[Bytes_counter] = '.';        //接收第二个字节后插入'.'
			                ++ Bytes_counter;
//							GPS_wd[11] = '\0';
				        }
						if (Bytes_counter == 5) {				//去掉分mm.mmmm之间的"."
							*buf ++;
							GPS_wd[Bytes_counter] = *buf;
						}
						else
							GPS_wd[Bytes_counter] = *buf;   
						break;
					case 2:        
						GPS_warn = *buf;						//$GPRMC第二段处理 定位状态，A=有效定位，V=无效定位
 						if ('A' != GPS_warn) {
							Command = 0;
							ReceivingF = 0;
						}
						break;
				}
				break;
			
			case 3:
				switch (Command) {
					case 1:        
						rgps.north_south = *buf;	//$GPGGA第3段处理 纬度半球N（北半球）或S（南半球）
						break;
					case 2:							//$GPRMC第3段处理 忽略
					    break;
				}
				break;
				
			case 4:
				switch (Command) {
					case 1:        
				        if (Bytes_counter == 11) {
							GPS_jd[Bytes_counter] = '\0';
							break;
						}
						if (Bytes_counter == 3) {               //$GPGGA 段4处理 经度dddmm.mmmm（度分）格式
			                GPS_jd[Bytes_counter] = '.';        //接收第3个字节后插入'.'
			                ++ Bytes_counter;
				        }
						if (Bytes_counter == 6) {
							*buf ++;
							GPS_jd[Bytes_counter] = *buf;
							break;
						}
						else
							GPS_jd[Bytes_counter] = *buf;          
						break;
					case 2:										//$GPRMC第4段 忽略
						break;
					case 3:
						if (Bytes_counter < 5) {				//$GPVTG 第4段处理 地面速率（000.0~999.9节，前面的0也将被传输）
							GPS_speed[Bytes_counter] = *buf;
							GPS_speed[Bytes_counter + 1] = '\0';
						}
						break;
				}
				break;
				
			case 5:        
				switch (Command) {
					case 1:        
						rgps.east_west = *buf;		//$GPGGA第5段处理 经度半球E（东经）或W（西经）
						break;
					case 2:                         //$GPRMC第5段处理 忽略
						break;
				}
				break;
			
			case 6:        
				switch (Command) {
					case 1:        
						GPS_status = *buf;         //$GPGGA第6段处理 GPS状态：0=未定位，1=非差分定位，2=差分定位，6=正在估算
						if ('0' == GPS_status) {
							Command = 0;
							ReceivingF = 0;
						}
					    break;
					case 2:							//$GPRMC第6段处理 忽略
						break;
				}
				break;  
			
			case 7:        
				switch (Command) {			                                                          
					case 1:        
						if (Bytes_counter < 2)		//$GPGGA第7段处理  正在使用解算位置的卫星数量（00~12）（前面的0也将被传输）
							GPS_sv[Bytes_counter] = *buf;
						GPS_sv[2] = '\0';         
						break;
					case 2:       	
	 					if (Bytes_counter < 5) {	//$GPRMC第7段处理 地面速率（000.0~999.9节，前面的0也将被传输）
							GPS_speed[Bytes_counter] = *buf;
							GPS_speed[Bytes_counter + 1] = '\0';
						}
						break;
				}
				break;   
			case 8:        
				switch (Command) {                                                          
					case 1:        
						/*if(Bytes_counter<5) {			//$GPGGA第8段处理  水平精度
							//GPS_precision[Bytes_counter] = *buf;
							//GPS_precision[Bytes_counter+1] = '\0';         
						}*/
						break;
					case 2:  
						if (*buf == ',') {
							null_flag = 1;
						} else {   
	 						if (Bytes_counter < 6) {	//$GPRMC第8段处理 地面航向
							
								GPS_direction[Bytes_counter] = *buf;
								GPS_direction[Bytes_counter + 1] = '\0';
							}
						}
						break;
				}
				break;  	
			case 9:        
				switch (Command)
				{
					case 1:        
						if (Bytes_counter < 7) {		//$GPGGA第9段处理 海拔高度（-9999.9~99999.9）
							GPS_alt[Bytes_counter] = *buf;
							GPS_alt[Bytes_counter + 1] = '\0';
				        } 
						break;
					case 2:        
						if (Bytes_counter < 2) {		//$GPRMC第9段处理 UTC日期，ddmmyy（日月年）格式转换为yy.mm.dd
							GPS_date[6 + Bytes_counter] = *buf;
				        }
				        if ((Bytes_counter > 1)&&(Bytes_counter < 4)) {	//月
							GPS_date[1 + Bytes_counter] = *buf;
							GPS_date[5] = '-';
				        }
				        if ((Bytes_counter > 3)&&(Bytes_counter < 6)) {	//年
							GPS_date[Bytes_counter-4] = *buf;
							GPS_date[2] = '-';
							GPS_date[8] = '\0';
				        }
				        break;
				}
				break;
				
			default:
				break;
		}
		if (!null_flag) {
			++Bytes_counter;
			*buf ++;
		} else {
			++Segment;
			*buf ++;
		}
		null_flag = 0;
	}
		
	NewByteF = 0;
}

void freeCharPtr(char ** ch, ...)
{
          va_list ap;
          char ** p;
          
          va_start(ap, ch);
          free(*ch);
          *ch = NULL;
          while (p = va_arg(ap, char ** ))
          {
                  free(*p);
                  *p = NULL;
          }
}
#define BUFFSIZE_H 1024
int WEBQuest(const char * Url, const char * pUrl)
{
#if 0 //no this declaration, socket create failed.
    WSADATA WsaData;

    if (WSAStartup(MAKEWORD(2,2),&WsaData))
    {
        printf("The socket failed");
        return false;
    }
#endif
	struct hostent *hptr;
	char **pptr;
	char str[32], d_addr[32] = {0};
	int false = 1, ture = 0;
	
	if( (hptr = gethostbyname(Url) ) == NULL )
	{
		printf("gethostbyname error for host:%s\n", Url);
		return false; /* 如果调用gethostbyname发生错误，返回1 */
	}
	/* 将主机的规范名打出来 */
	printf("official hostname:%s\n",hptr->h_name);
	/* 根据地址类型，将地址打出来 */
	switch(hptr->h_addrtype)
	{
		case AF_INET:
		case AF_INET6:
			pptr=hptr->h_addr_list;
			/* 将刚才得到的所有地址都打出来。其中调用了inet_ntop()函数 */
			for(;*pptr!=NULL;pptr++)
			{
				//printf(" address:%s\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
				memcpy(d_addr, inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)), strlen(inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str))));
				printf("address:%s\n", d_addr); 
			}
			break;
		default:
			printf("unknown address type\n");
		break;
	}

	int sockeId;
	struct sockaddr_in addr;
	bzero(&addr,sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htons(INADDR_ANY);
    addr.sin_port = htons(0);

	if((sockeId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Create Socket Failed!\n");
		return false;
	}
	if(bind(sockeId, (struct sockaddr*)&addr, sizeof(addr)))
	{
		printf("Client Bind Port Failed!\n");
		return false;
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	if(inet_aton(d_addr, &server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数
	{
		printf("Server IP Address Error!\n");
		return false;
	}

	server_addr.sin_port = htons(80);
	socklen_t server_addr_length = sizeof(server_addr);
	//向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
	if(connect(sockeId, (struct sockaddr*)&server_addr, server_addr_length) < 0)
	{
		printf("Can Not Connect To %s!\n", d_addr);
		return false;
	}

    /// < 分离url中的主机地址和相对路径 考虑到内存和性能，下面不是好习惯！
    char * head =(char * ) malloc(BUFFSIZE_H * sizeof(char));
    memset(head, '\0', BUFFSIZE_H);

    /**
    * 组织发送到web服务器的信息
    * 为何要发送下面的信息connect请参考HTTP协议的约定
    */
    strcat(head, "POST /");
	strcat(head, pUrl);
    strcat(head, " HTTP/1.1\r\n");
	strcat(head, "host: ");
    strcat(head, Url);
    strcat(head, "\r\nConnection: Close\r\n\r\n");
    printf("\n");
    printf(head);

    if (-1 == send(sockeId, head, strlen(head), 0))
    {
        printf("send &header error!\n");
        close(sockeId);
		freeCharPtr(&head, NULL);

        return false;
    }
	if ( logfp != (FILE*) 0 )
	{	
		(void) fprintf( logfp, "%s\n", head );
		(void) fflush( logfp );
    }
    memset(head, '\0', BUFFSIZE_H);
#if 0
	FILE *fp;
    while (recv(sockeId, head, BUFFSIZE_H, 0) > 0)
    {
        fputs(head, fp);
        memset(head, '\0', BUFFSIZE_H);
    }
    freeCharPtr(&host, &GET, &head, NULL);
#endif
	close(sockeId);

	return ture;
}

void gps_report(char * url, int status)
{
	char temp_str[256] = {0};

	memset(temp_str, 0, sizeof(temp_str));
	if (0 == status) {
		sprintf(temp_str, "{\"dmac\":\"%s\",\"status\":\"%d\"}", rgps.str_host, status);
		printf("%s\n", temp_str);
		system("/sbin/gps_invalid.sh");
	} else {
		sprintf(temp_str, "{\"dmac\":\"%s\",\"status\":\"%d\",\"latitude\":\"%s\",\"east_west\":\"%c\",\"date\":\"%s-%s\",\"speed\":\"%s\",\"north_south\":\"%c\",\"longitude\":\"%s\",\"height\":\"%s\"}",
		rgps.str_host, status, rgps.gps_Lat, rgps.east_west, rgps.gps_Date, rgps.gps_Time, rgps.gps_Velocity, rgps.north_south, rgps.gps_Lng, rgps.gps_Elevation);
		printf("%s\n", temp_str);
		system("/sbin/gps_valid.sh");
	}
	if (logfp != (FILE*)0) {	
		(void)fprintf(logfp, "%s\n", temp_str);
		(void)fflush(logfp);
	}
	if (url == NULL) {
		//printf("-----------warning: url is null, cannot report-----------\n");
		return;
	}
	WEBQuest(url, temp_str);
}

int main(int argc, char *argv[])
{
	int opt, sleep_time = 15;
	char temp_str[256] = {0};
	char temp_str1[256] = {0};
	int fd1, nread, i = 0, j = 0;
	char buf[RD_SIZE];
	char buf1[RD_SIZE] = {0};
	char *dev_path = (char*)0;
	char *dev_path1 = "/tmp/.gps.log";
	char *dev_path2 = "/tmp/gps_info";
	char *url = (char*)0;	
	char *logfile = (char*)0;
	struct timeval tv_begin, tv_end;
	int tv_intval;
	
	while ((opt = getopt(argc,argv,"t:d:l:u:h")) != -1) {
		switch (opt) {
			case 'h':
				printf("rgps [-t interval] [-u url] [-l logfile]\n");
				printf("          -h (get help)\n");
				printf("          -d [gps_device or gps_log] (ttyusb device /dev/ttyUSB3 or /tmp/run/nmea file)\n");
				printf("          -t 15 (report interval 5~300 sec)\n");
				printf("          -u www.weizhigps.com (url for report)\n");
				printf("          -l /tmp/gps.log (logfile for gps)\n");
				return 0;				
			case 'd':
				dev_path = optarg;
				break;
			case 'u':
				url = optarg;
				break;
			case 'l':
				logfile = optarg;
				break;
			case 't':
				sleep_time = strtoul(optarg, NULL, 10);
				if ((sleep_time < 5)||(sleep_time > 300)) {
					printf("		  -t 15 (report interval 5~300 sec)\n");
					sleep_time = 15;
				}
				break;
			default:
				break;
		}
	}

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
#ifdef TEST

	while (1) {
		char *buf3 = "$GPRMC,090716.0,A,4002.732804,N,11617.504693,E,0.0,331.0,091013,0.0,E,A*06";
		char *buf2 = "$GPGGA,050509.50,3114.361111,N,12129.968941,E,1,06,2.1,-42.1,M,,,,*2C";
		//char *buf3 = "$GPVTG,087.40,T,,M,0.00,N,0.00,K*1C";

		print_gpsinfo(buf2);
		print_gpsinfo(buf3);
		if((1 == flag_GPGGA) && (1 == flag_GPRMC))
			gps_report(url, 1);
		else
			gps_report(url, 0);
		sleep(sleep_time);
	}

#else

	sprintf(temp_str, "showsysinfo | awk -F 'MAC:' '/MAC/{print $2}'| sed 's/:/-/g'");
	PopenFile(temp_str, rgps.str_host, sizeof(rgps.str_host));

	printf("\n GPS positioning ...\n");
	sprintf(temp_str1, "head -n 30 %s > %s ; cp %s %s", dev_path, dev_path1, dev_path1, dev_path2);

#if 0
	fd1 = open(dev_path, O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd1 == -1) {
		printf("open dev %s fail !!\n", dev_path);
		exit(1);
	}
	serial_init(fd1);
	close(fd1);
#endif

	while(1) {
		printf("\n GPS waiting ...\n");
		sleep(sleep_time);
		flag_GPGGA = 0;
		flag_GPRMC = 0;
		gettimeofday(&tv_begin, NULL);

		do {
			printf("\n GPS reading ...\n");
			system(temp_str1);
			fd1 = open(dev_path1, O_RDONLY);
			if (fd1 == -1) {
				printf("open dev %s fail !!\n", dev_path1);
				exit(1);
			}
			memset(buf, 0, RD_SIZE);
			memset(buf1, 0, RD_SIZE);
			nread = read(fd1, buf, RD_SIZE-1);
			close(fd1);

			if (nread > 0) {
				printf("\n GPS DATALen=%d\n", nread);
				buf[nread] = '\0';
				i = 0;
				j = 0;
				//printf("%s           %d\n\n", buf, nread);			
				while (nread > 1) {
					buf1[j] = buf[i];
					i++;
					j++;
					if ('\n' == buf[i]) {
						buf1[j] = '\0';
						//printf( "\n\n\n-----%d----------buf1 %s\n", nread, buf1);
						print_gpsinfo(buf1);
						memset(buf1, 0, RD_SIZE);
						j = 0;
						i ++;
					}
					nread --;
				}
				if ((1 == flag_GPGGA)&&(1 == flag_GPRMC)) {
					gps_report(url, 1);
					break;
				}
			}
			else
				printf("\n read fail %d\n", nread);

			gettimeofday(&tv_end, NULL);
			tv_intval = tv_end.tv_sec - tv_begin.tv_sec;
			printf("begin:%d, end:%d, interval:%d\n", tv_begin.tv_sec, tv_end.tv_sec, tv_intval);
			if (tv_intval > 3) {
				gps_report(url, 0);
				break;
			}
		} while ((1 != flag_GPGGA)||(1 != flag_GPRMC));
		printf("\n GPS closeing ...\n");
	}

#endif
	if (logfp != (FILE*)0)
		fclose(logfp);

	return 0;
}


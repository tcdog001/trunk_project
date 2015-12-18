#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define TTYUSB "/dev/ttyUSB0"

#include "at_ttyUSB.h"

#ifndef __BUSYBOX__
#define at_ttyUSB0_main  main
#endif

int at_ttyUSB0_main(int argc, char *argv[])
{
	return at_ttyUSB_main(argc, argv);
}

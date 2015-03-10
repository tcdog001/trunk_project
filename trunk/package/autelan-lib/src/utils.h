#ifndef __UTILS_H_F5F47009AF9E5B811C77BFEA13C326CB__
#define __UTILS_H_F5F47009AF9E5B811C77BFEA13C326CB__
/******************************************************************************/
#ifdef __FASTBOOT__
#define __BOOT__
#endif

#ifdef __BOOT__
#   undef __KERNEL__
#   define __AKID_DEBUG __boot_akid
#   include <malloc.h>
#   include <command.h>
#   ifndef __FASTBOOT__
#       include <common.h>
#   endif
#   include <linux/ctype.h>
#   include <linux/string.h>
#elif defined(__KERNEL__)
#   include <linux/moduleparam.h>
#   include <linux/module.h>
#   include <linux/skbuff.h>
#   include <linux/gfp.h>
#   include <linux/init.h>
#   include <linux/in.h>
#   include <linux/inet.h>
#   include <linux/netlink.h>
#   include <linux/rtc.h>
#   include <linux/ctype.h>
#   include <linux/string.h>
#   include <linux/time.h>
#   include <linux/errno.h>
#   include <linux/list.h>
#   include <net/sock.h>
#else /* __APP__ */
#   define __APP__
#   include <stdint.h>
#   include <stdarg.h>
#   include <stdlib.h>
#   include <stdio.h>
#   include <signal.h>
#   include <string.h>
#   include <malloc.h>
#   include <assert.h>
#   include <unistd.h>
#   include <fcntl.h>
#   include <ctype.h>
#   include <errno.h>
#   include <time.h>
#   include <dirent.h>
#   include <syslog.h>
#   include <ucontext.h>
#   include <byteswap.h>
#   include <libgen.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <sys/file.h>
#   include <sys/epoll.h>
#   include <sys/wait.h>
#   include <sys/ipc.h>
#   include <sys/msg.h>
#   include <sys/sem.h>
#   include <sys/shm.h>
#   include <sys/socket.h>
#   include <sys/un.h>
#   include <sys/timerfd.h>
#   include <netinet/ether.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <netinet/udp.h>
#   include <arpa/inet.h>
#endif /* !__APP__ */

#include "utils/type.h"
#include "utils/base.h"
#include "utils/bits.h"
#include "utils/assert.h"
#include "utils/errno.h"
#include "utils/list.h"
#include "utils/number.h"
#include "utils/memory.h"
#include "utils/string.h"
#include "utils/printf.h"
#include "utils/appkey.h"
#include "utils/debug.h"
#include "utils/dump.h"
#include "utils/time.h"
#include "utils/sem.h"
#include "utils/shm.h"
#include "utils/sfile.h"
#include "utils/crc32.h"
#include "utils/cmd.h"
#include "utils/env.h"
#include "utils/mac.h"
#include "utils/ip.h"
#include "utils/io.h"
#include "utils/kv.h"
/******************************************************************************/
#endif /* __UTILS_H_F5F47009AF9E5B811C77BFEA13C326CB__ */

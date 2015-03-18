#ifndef __IO_H_D4A9C58D7819A78E004AD0C1E795940A__
#define __IO_H_D4A9C58D7819A78E004AD0C1E795940A__
/******************************************************************************/
#include "utils/time.h"
/******************************************************************************/
#if defined(__BUSYBOX__) || defined(__APP__)
static inline int
io_read(int fd, char *buf, int size, int timeout /* ms */)
{
    fd_set rset;
    struct timeval tv = {
        .tv_sec     = time_sec(timeout),
        .tv_usec    = time_usec(timeout),
    };
    int err, count;
    
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    while(1) {
        err = select(fd + 1, &rset, NULL, NULL, &tv);
        switch(err) {
            case -1:/* error */
                if (EINTR==errno) {
                    // is breaked
                    debug_trace("read breaked");
                    
                    continue;
                } else {
                    debug_trace("read error:%d", -errno);
                    
                    return -errno;
                }
            case 0: /* timeout, retry */
                debug_trace("read timeout");
                
                return -ETIMEOUT;
            default: /* to accept */
                count = read(fd, buf, size);

                if (count >=0 && __is_debug_init_trace) {
                    os_dump_buffer(buf, count, NULL);
                }
                
                return count;
        }
    }
}

static inline int
io_write(int fd, char *buf, int len)
{
    int count = 0;
    int err;

    while(count < len) {
        err = write(fd, buf + count, len - count);
        if (err) {
            debug_error("write error:%d", -errno);
            return -errno;
        } else {
            count += err;
        }
    }
    
    if (__is_debug_init_trace) {
        os_dump_buffer(buf, len, NULL);
    }
    
    return 0;
}
#endif
/******************************************************************************/
#endif /* __IO_H_D4A9C58D7819A78E004AD0C1E795940A__ */

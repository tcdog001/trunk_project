#ifndef __ERRNO_H_C4F00CBE99389CDDE88C03900D7FC85A__
#define __ERRNO_H_C4F00CBE99389CDDE88C03900D7FC85A__

#ifndef __ERRNO_BASE
#define __ERRNO_BASE    1000
#endif

#ifndef __ERRNO_STEP
#define __ERRNO_STEP    1000
#endif

#define __ERRNO(_mod)   (__ERRNO_BASE + (_mod) * __ERRNO_STEP)

enum {
    __ERRNO_COMMON      = 0,
    __ERRNO_APPKEY      = 1,
    __ERRNO_STIMER      = 2,
    
    ERRNO_END
};

#define ERRNO_COMMON        __ERRNO(__ERRNO_COMMON) /* 1000 */

static inline int
shell_error(int err)
{
    if (err) {
        err = -err;
    }

    if (err < __ERRNO_BASE) {
        return err;
    } else {
        err -= __ERRNO_BASE;

        return (err % __ERRNO_STEP);
    }
}

enum {
    EINVAL0 = ERRNO_COMMON,
    EINVAL1,
    EINVAL2,
    EINVAL3,
    EINVAL4,
    EINVAL5,
    EINVAL6,
    EINVAL7,
    EINVAL8,
    EINVAL9,
    ENOSUPPORT, /* 10 */
    ENOINIT,    /* 11 */
    ENOEXIST,   /* 12 */
    ENOSPACE,   /* 13 */
    ENOEMPTY,   /* 14 */
    ENOINLIST,  /* 15 */
    EBADIDX,    /* 16 */
    EBADREF,    /* 17 */
    EBADCRC,    /* 18 */
    EKEYNULL,   /* 19 */
    EKEYBAD,    /* 20 */
    ESYSTEM,    /* 21 */
    EFORMAT,    /* 22 */
    ETOOBIG,    /* 23 */
    ELIMIT,     /* 24 */
    EINLIST,    /* 25 */
    ETIMEOUT,   /* 26 */
    EEMPTY,     /* 27 */
    EHELP,      /* 28 */
    ETRYEXCEED, /* 29 */
    EHACKED,    /* 30 */
};


#ifdef __BOOT__

/* 
* linux/include/uapi/asm/generic/errno-base.h 
*/
#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		     5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */

#define errno       EINVAL

#endif
/******************************************************************************/
#endif /* __ERRNO_H_C4F00CBE99389CDDE88C03900D7FC85A__ */

#ifndef __PRINTF_H_6115A6C5DFF707CC6D8400E2B7190543__
#define __PRINTF_H_6115A6C5DFF707CC6D8400E2B7190543__
/******************************************************************************/
#if defined(__BOOT__) || defined(__APP__)
#define os_printf(_fmt, _args...)               printf(_fmt, ##_args)
#define os_vprintf(_fmt, _args)                 vprintf(_fmt, _args)
#else
#define os_printf(_fmt, _args...)               printk(KERN_ALERT _fmt, ##_args)
#define os_vprintf(_fmt, _args)                 vprintk(KERN_ALERT _fmt, _args)
#endif

#ifdef __APP__
#define os_fprintf(_stream, _fmt, _args...)     fprintf(_stream, _fmt, ##_args)
#define os_vfprintf(_stream, _fmt, _args)       vfprintf(_stream, _fmt, _args)
#else
#define os_fprintf(_stream, _fmt, _args...)     os_printf(_fmt, ##_args)
#define os_vfprintf(_stream, _fmt, _args)       os_vprintf(_fmt, _args)
#endif

#define os_sscanf(_buf, _fmt, _args...)         sscanf(_buf, _fmt, ##_args)
#define os_vsscanf(_buf, _fmt, _args)           vsscanf(_buf, _fmt, _args)

#define os_println(_fmt, _args...)              os_printf(_fmt __crlf, ##_args)
#define os_vprintln(_fmt, _args)                os_vprintf(_fmt __crlf, _args)

#define os_printab(_count, _fmt, _args...) do{ \
    int i;                      \
                                \
    for (i=0; i<_count; i++) {  \
        os_printf(__tab);       \
    }                           \
    os_println(_fmt, ##_args);  \
}while(0)

#define os_vprintab(count, _fmt, _args) do{ \
    int i;                      \
                                \
    for (i=0; i<count; i++) {   \
        os_printf(__tab);       \
    }                           \
    os_vprintln(_fmt, _args);   \
}while(0)

#define os_fprintln(_stream, _fmt, _args...)    os_fprintf(_stream, _fmt __crlf, ##_args)
#define os_vfprintln(_stream, _fmt, _args)      os_vfprintf(_stream, _fmt __crlf, _args)

#define os_sprintf(_buf, _fmt, _args...)        sprintf(_buf, _fmt, ##_args)
#define os_vsprintf(_buf, _fmt, _args)          vsprintf(_buf, _fmt, _args)

#define os_snprintf_is_full(_buffer_space, _snprintf_return_value) \
        ((_snprintf_return_value) >= (_buffer_space))

#ifdef __BOOT__
#ifndef BOOT_SNPRINTF_BUFSIZE
#define BOOT_SNPRINTF_BUFSIZE   4096
#endif

#define os_snprintf(_buf, _size, _fmt, _args...)  ({ \
    int nsize = BOOT_SNPRINTF_BUFSIZE;  \
    int bsize = (_size)?(_size)-1:0;    \
    int len;                            \
    char *p = (char *)os_zalloc(nsize); \
    if (p) {                            \
        len = os_sprintf(p, _fmt, ##_args); \
        if (len <= bsize) {             \
            os_strcpy(_buf, p);         \
        } else {                        \
            os_memcpy(_buf, p, bsize);  \
        }                               \
        os_free(p);                     \
    } else {                            \
        len = 0;                        \
    }                                   \
                                        \
    len;                                \
})
#define os_vsnprintf(_buf, _size, _fmt, _args)  BUILD_BUG_ON(true)

/*
* change from uclibc(vasprintf)
*/
#define os_asprintf(_pbuf, _fmt, _args...)       ({ \
	int rv;                                         \
	char **pbuf = (char **)(_pbuf);                 \
                                                    \
 	rv = os_snprintf(NULL, 0, _fmt, ##_args);       \
	*pbuf = NULL;                                   \
                                                    \
	if (rv >= 0) {                                  \
		if ((*pbuf = os_malloc(++rv)) != NULL) {    \
			if ((rv = os_snprintf(*pbuf, rv, _fmt, ##_args)) < 0) { \
				os_free(*pbuf);                     \
			}                                       \
		}                                           \
	}                                               \
                                                    \
	os_assert(rv >= -1);                            \
                                                    \
	return rv;                                      \
})
#define os_vasprintf(_pbuf, _fmt, _args)        BUILD_BUG_ON(true)

#else
#define os_snprintf(_buf, _size, _fmt, _args...) snprintf(_buf, _size, _fmt, ##_args)
#define os_vsnprintf(_buf, _size, _fmt, _args)  vsnprintf(_buf, _size, _fmt, _args)

#define os_asprintf(_pbuf, _fmt, _args...)      asprintf(_buf, _size, _fmt, ##_args)
#define os_vasprintf(_pbuf, _fmt, _args)        vasprintf(_buf, _size, _fmt, _args)
#endif


/*
* snprintf for array buffer
*/
#define os_saprintf(_buf, _fmt, _args...)       os_snprintf(_buf, sizeof(_buf), _fmt, ##_args)
#define os_vsaprintf(_buf, _fmt, _args)         os_vsnprintf(_buf, sizeof(_buf), _fmt, _args)
/*
* snprintf for array buffer + offset
*/
#define os_soprintf(_buf, _offset, _fmt, _args...)  \
    os_snprintf(_buf+(_offset), sizeof(_buf)-(_offset), _fmt, ##_args)
#define os_voaprintf(_buf, _offset, _fmt, _args)    \
    os_vsnprintf(_buf+(_offset), sizeof(_buf)-(_offset), _fmt, _args)

#define os_eprintf(_err, _fmt, _args...)    \
    (os_printf(_fmt, ##_args), (_err))
#define os_eprintln(_err, _fmt, _args...)   \
    (os_println(_fmt, ##_args), (_err))

#ifdef __APP__
#define os_v_xopen(_type, _func, _mod, _fmt, _args...) ({  \
    char buf[1+OS_LINE_LEN] = {0};  \
    _type tvar;                     \
                                    \
    os_saprintf(buf, _fmt, ##_args);\
    tvar = _func(buf, _mod);        \
                                    \
    tvar;                           \
})

#define os_v_popen(_fmt, _args...)      \
    os_v_xopen(FILE*, popen, "r", _fmt, ##_args)
#define os_v_fopen(_mod, _fmt, _args...) \
    os_v_xopen(FILE*, fopen, _mod, _fmt, ##_args)
#define os_v_open(_flag, _fmt, _args...) \
    os_v_xopen(int, open, _flag, _fmt, ##_args)

#define os_system(_cmd)    ({  \
    int ret = 0, err = 0;       \
                                \
    err = system(_cmd);         \
    if (-1==err) {/* fork/exec failed */ \
        ret = -ESYSTEM;         \
    } else if(WIFEXITED(err)) { /* 正常结束子进程 */ \
        ret = WEXITSTATUS(err); /* 子进程 exit()返回的结束代码 */ \
    } else if(WIFSIGNALED(err)) { /* 异常结束子进程 */ \
        ret = WTERMSIG(err); /* 子进程因信号而中止的信号代码 */ \
    } else if(WIFSTOPPED(err)) { /* 暂停子进程 */ \
        ret = WSTOPSIG(err); /* 引发子进程暂停的信号代码 */ \
	} else {                    \
        ret = -errno;           \
	}                           \
                                \
    ret;                        \
})

#define __os_v_system(_is_traced, _fmt, _args...) ({  \
    char cmd[1+OS_LINE_LEN] = {0};  \
    int len, ret = 0;               \
                                    \
    if (_is_traced) {               \
        len = os_saprintf(cmd, _fmt, ##_args);   \
        os_println("%s", cmd);      \
    } else {                        \
        len = os_saprintf(cmd, _fmt " > /dev/null 2>&1", ##_args); \
    }                               \
    if (os_snprintf_is_full(sizeof(cmd), len)) { \
        ret = -ENOSPACE;            \
    } else {                        \
        ret = os_system(cmd);       \
	}                               \
                                    \
	ret;                            \
})  /* end */

#define os_v_system(_fmt, _args...)   \
        __os_v_system(__is_debug_init_trace, _fmt, ##_args)

#define os_v_pgets(_line, _space, _fmt, _args...)   ({ \
    int err = 0;                                \
                                                \
    FILE *fd = os_v_popen(_fmt, ##_args);       \
    if (NULL==fd) {                             \
        err = -EPIPE;                           \
    } else {                                    \
        if (NULL==fgets(_line, _space, fd)) {   \
            err = -errno;                       \
        } else {                                \
            err = 0;                            \
        }                                       \
                                                \
        pclose(fd);                             \
    }                                           \
                                                \
    err;                                        \
})  /* end */

#define os_v_pread(_buf, _size, _fmt, _args...) ({  \
    int err = 0;                                \
                                                \
    FILE *fd = os_v_popen(_fmt, ##_args);       \
    if (NULL==fd) {                             \
        err = -EPIPE;                           \
    } else {                                    \
        int count = fread(_buf, 1, _size, fd);  \
        if (count <= 0) {                       \
            err = -errno;                       \
        } else {                                \
            err = 0;                            \
        }                                       \
                                                \
        pclose(fd);                             \
    }                                           \
                                                \
    err;                                        \
})  /* end */
        

#endif

/******************************************************************************/
#endif /* __PRINTF_H_6115A6C5DFF707CC6D8400E2B7190543__ */

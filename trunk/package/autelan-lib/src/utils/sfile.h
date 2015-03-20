#ifndef __SFILE_H_CAF3737A24BFD00978F62CEB481C30B0__
#define __SFILE_H_CAF3737A24BFD00978F62CEB481C30B0__
#ifdef __APP__
/******************************************************************************/
/* simpile file api */
/******************************************************************************/

static inline void 
os_fd_set_cloexec(int fd)
{
#ifdef FD_CLOEXEC
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
#endif
}

static inline bool 
os_sfscan_match_all(char *context)
{   /*
    * just "*"
    */
    return '*'==context[0] && 0==context[1];
}

static inline bool
os_sfcan_is_dir_self(char *filename/* not include path */)
{
    return '.'==filename[0] && 0==filename[1];
}

static inline bool
os_sfcan_is_dir_father(char *filename/* not include path */)
{
    return '.'==filename[0] && '.'==filename[1] && 0==filename[2];
}

/*
* @filename: not include path
*/
typedef multi_value_t os_sfscan_line_handle_t(char *filename, char *line, void *control);
typedef multi_value_t os_sfscan_file_handle_t(char *path, char *filename, 
                            os_sfscan_line_handle_t *line_handle, void *control);
typedef bool os_sfscan_file_filter_t(char *path, char *filename, void *control);

static inline multi_value_t 
os_sfscan_file_handle
(
    char *path,
    char *filename,
    os_sfscan_line_handle_t *line_handle,
    void *control
)
{
    FILE *stream = NULL;
    char line[1+OS_LINE_LEN];
    multi_value_u mv;

    if (NULL==path) {
        return os_assert_value(mv2_GO(-EINVAL1));
    }
    else if (NULL==filename) {
        return os_assert_value(mv2_GO(-EINVAL2));
    }
    else if (NULL==line_handle) {
        return os_assert_value(mv2_GO(-EINVAL3));
    }
    
    stream = os_v_fopen("r", "%s/%s", path, filename);
    if (NULL==stream) {
        mv2_result(mv) = -errno;

        goto error;
    }

    while(false==feof(stream)) {
        os_objzero(&line); /* os_objzero(&array) */
        fgets(line, OS_LINE_LEN, stream);
        
        /*
        * strim left/right blank
        */
        __string_strim_both(line, NULL);
        
        /*
        * replace blank("\t \r\n") to ' '
        */
        __string_replace(line, NULL, ' ');
        
        /*
        * reduce ' '
        */
        __string_reduce(line, NULL);
        
        /*
        * skip blank line
        */
        if (__is_blank_line(line)) {
            continue;
        }
        
        /*
        * skip notes line
        */
        if (__is_notes_line_deft(line)) {
            continue;
        }
        
        mv.value = (*line_handle)(filename, line, control);
        if (mv2_is_break(mv)) {
            goto error;
        }
    }

    mv.value = mv2_OK;
error:
    if (stream) {
        fclose(stream);
    }
    
    return mv.value;
}

/*
* @filefilter: if return true, ignore file
*/
static inline int 
__os_sfscan_dir
(
    char *path, 
    bool recur,
    os_sfscan_file_filter_t *file_filter,
    os_sfscan_file_handle_t *file_handle,
    os_sfscan_line_handle_t *line_handle,
    void *control
)
{
    DIR *dir = NULL;
    struct dirent *file = NULL;
    struct stat st;
    multi_value_u mv;
    int err = 0;

    if (NULL==path) {
        return os_assert_value(-EINVAL1);
    }
    else if (NULL==file_handle) {
        return os_assert_value(-EINVAL2);
    }
    
    dir = opendir(path);
    if (NULL == dir) {
        err = -errno;
        
        goto error;
    }
    
    while (NULL != (file=readdir(dir))) {
        char *filename = file->d_name; /* just name, not include path */
        
        /*
        * skip . and ..
        */
        if (os_sfcan_is_dir_self(filename) || os_sfcan_is_dir_father(filename)) {
            continue;
        }
        
        /*
        * dir
        */
        if (stat(filename, &st) >= 0 && S_ISDIR(st.st_mode)) {
            if (recur) {
                err = __os_sfscan_dir(path, recur, file_filter, file_handle, line_handle, control);
                if (err) {
                    goto error;
                }
            } else {
                continue;
            }
        }
        
        /*
        * file filter
        */
        if (file_filter && (*file_filter)(path, filename, control)) {
            continue;
        }
        
        /*
        * file handle
        */
        mv.value = (*file_handle)(path, filename, line_handle, control);
        if (mv2_is_break(mv)) {
            err = mv2_result(mv);

            goto error;
        }
    }
    
error:
    if (dir) {
        closedir(dir);
    }

    return err;
}

static inline int 
os_sfscan_dir
(
    char *path, 
    bool recur,
    os_sfscan_file_filter_t *file_filter,
    os_sfscan_line_handle_t *line_handle,
    void *control
)
{
    return __os_sfscan_dir(path, recur, file_filter, os_sfscan_file_handle, line_handle, control);
}
/******************************************************************************/
#define __os_sgetx(_prefix, _stream, _vfmt, _pv) ({ \
    int err = 0;                    \
                                    \
    if (NULL==(_stream)) {          \
        err = -errno;               \
    } else if (1!=fscanf(_stream,   \
                    _vfmt, _pv)) {  \
        err = -EFORMAT;             \
    }                               \
                                    \
    if (_stream) {                  \
        _prefix##close(_stream);    \
    }                               \
                                    \
    err;                            \
})

#define __os_sgets(_prefix, _stream, _line, _space) ({ \
    int err = 0;                    \
                                    \
    if (NULL==(_line)) {            \
        err = -EINVAL9;             \
    } else if ((_space)<=0) {       \
        err = -EINVAL8;             \
    } else if (NULL==(_stream)) {   \
        err = -errno;               \
    } else if (NULL==fgets(_line,   \
                _space, _stream)) { \
        err = -errno;               \
    } else {                        \
        __string_strim_both(_line, NULL);   \
    }                               \
                                    \
    if (_stream) {                  \
        _prefix##close(_stream);    \
    }                               \
                                    \
    err;                            \
}) /* end */

#define os_sgetx(_prefix, _vfmt, _pv, _filename) ({     \
    FILE *stream = _prefix##open("r", _filename);       \
    int err = __os_sgetx(_prefix, stream, _vfmt, _pv);  \
    err; \
})  /* end */

#define os_sgets(_prefix, _line, _space, _filename) ({  \
    FILE *stream = _prefix##open("r", _filename);       \
    int err = __os_sgets(_prefix, stream, _line, _space); \
    err;                                                \
})  /* end */

#define os_sgeti(_prefix, _pi, _filename)    \
    os_sgetx(_prefix, "%u", _pi, _filename)
#define os_sgetll(_prefix, _pll, _filename)  \
    os_sgetx(_prefix, "%llu", _pll, _filename)


#define os_v_sgetx(_prefix, _vfmt, _pv, _fmt, _args...)     ({  \
    FILE *stream = os_v_##_prefix##open("r", _fmt, ##_args);    \
    int err = __os_sgetx(_prefix, stream, _vfmt, _pv);          \
    err;                                                        \
})  /* end */

#define os_v_sgets(_prefix, _line, _space, _fmt, _args...)  ({  \
    FILE *stream = os_v_##_prefix##open("r", _fmt, ##_args);    \
    int err = __os_sgets(_prefix, stream, _line, _space);       \
    err;                                                        \
})  /* end */

#define os_v_sgeti(_prefix, _pi, _fmt, _args...)     \
    os_v_sgetx(_prefix, "%u", _pi, _fmt, ##_args)
#define os_v_sgetll(_prefix, _pll, _fmt, _args...)   \
    os_v_sgetx(_prefix, "%u", _pll, _fmt, ##_args)


/*
* get (_string/int/long long int) from file
*/
#define os_sfgets(_line, _space, _filename)         os_sgets(f, _line, _space, _filename)
#define os_sfgeti(_pi, _filename)                   os_sgeti(f, _pi, _filename)
#define os_sfgetll(_pll, _filename)                 os_sgetll(f, _pll, _filename)

#define os_v_sfgets(_line, _space, _fmt, _args...)  os_v_sgets(f, _line, _space, _fmt, ##_args)
#define os_v_sfgeti(_pi, _fmt, _args...)            os_v_sgeti(f, _pi, _fmt, ##_args)
#define os_v_sfgetll(_pll, _fmt, _args...)          os_v_sgeti(f, _pll, _fmt, ##_args)

/*
* get (_string/int/long long int) from pipe
*/
#define os_spgets(_line, _space, _filename)         os_sgets(p, _line, _space, _filename)
#define os_spgeti(_pi, _filename)                   os_sgeti(p, _pi, _filename)
#define os_spgetll(_pll, _filename)                 os_sgetll(p, _pll, _filename)

#define os_v_spgets(_line, _space, _fmt, _args...)  os_v_sgets(p, _line, _space, _fmt, ##_args)
#define os_v_spgeti(_pi, _fmt, _args...)            os_v_sgeti(p, _pi, _fmt, ##_args)
#define os_v_spgetll(_pll, _fmt, _args...)          os_v_sgeti(p, _pll, _fmt, ##_args)


#define __os_ssetx(_prefix, _stream, _vfmt, v) ({ \
    int err = 0;                \
                                \
    if (NULL==(_stream)) {      \
        err = -errno;           \
    } else {                    \
        err = fprintf(_stream,  \
                _vfmt, v);      \
    }                           \
                                \
    if (_stream) {              \
        _prefix##close(_stream);\
    }                           \
                                \
    err;                        \
})  /* end */

#define os_ssetx(_prefix, _vfmt, _pv, _filename)    ({  \
    FILE *stream = _prefix##open(_filename, "w");       \
    int err = __os_ssetx(_prefix, stream, _vfmt, _pv);  \
    err;                                                \
})  /* end */

#define os_v_ssetx(_prefix, _vfmt, _pv, _fmt, _args...) ({  \
    FILE *stream = os_v_##_prefix##open("w", _fmt, ##_args);\
    int err = __os_ssetx(_prefix, stream, _vfmt, _pv);      \
    err;                                                    \
})  /* end */
    
#define os_ssets(_prefix, _string, _filename)          os_ssetx(_prefix, "%s", _string, _filename)
#define os_sseti(_prefix, _vi, _filename)              os_ssetx(_prefix, "%u", _vi, _filename)
#define os_ssetll(_prefix, _vll, _filename)            os_ssetx(_prefix, "%llu", _vll, _filename)

#define os_v_ssets(_prefix, _string, _fmt, _args...)    os_v_ssetx(_prefix, "%s", _string, _fmt, ##_args)
#define os_v_sseti(_prefix, _vi, _fmt, _args...)        os_v_ssetx(_prefix, "%u", _vi, _fmt, ##_args)
#define os_v_ssetll(_prefix, _vll, _fmt, _args...)      os_v_ssetx(_prefix, "%llu", _vll, _fmt, ##_args)

/*
* set (_string/int/long long int) to file
*/
#define os_sfsets(_string, _filename)                 os_ssets(f, _string, _filename)
#define os_sfseti(_vi, _filename)                     os_sseti(f, _vi, _filename)
#define os_sfsetll(_vll, _filename)                   os_ssetll(f, _vll, _filename)

#define os_v_sfsets(_string, _fmt, _args...)           os_v_ssets(f, _string, _fmt, ##_args)
#define os_v_sfseti(_vi, _fmt, _args...)               os_v_sseti(f, _vi, _fmt, ##_args)
#define os_v_sfsetll(_vll, _fmt, _args...)             os_v_ssetll(f, _vll, _fmt, ##_args)

/*
* get file size by full filename(include path)
*/
static inline int
os_sfsize(char *filename)
{
    struct stat st;
    int err;
    
    err = stat(filename, &st);
    if (err) {
        return -errno;
    } else {
        return st.st_size;
    }
}

/*
* get file size
*/
#define os_v_sfsize(_fmt, _args...) ({  \
    int size;                           \
    char buf[1+OS_LINE_LEN] = {0};      \
                                        \
    os_saprintf(buf, _fmt, ##_args);    \
    size = os_sfsize(buf);              \
                                        \
    size;                               \
})
/******************************************************************************/
#endif /* __KERNEL__ */
#endif /* __SFILE_H_CAF3737A24BFD00978F62CEB481C30B0__ */

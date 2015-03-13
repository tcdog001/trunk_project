#ifndef __ATENV_H_31502FF802B81D17AEDABEB5EDB5C121__
#define __ATENV_H_31502FF802B81D17AEDABEB5EDB5C121__
#include "utils.h"
/******************************************************************************/
#ifndef AT_ENV_SIZE
#define AT_ENV_SIZE         4096
#endif

#define AT_BLOCK_SIZE       512
#define AT_BLOCK_COUNT      (AT_ENV_SIZE/AT_BLOCK_SIZE)

#define AT_TRYS             3

#define AT_COOKIE           0xf00d1e00
#define AT_OID              0 /* todo: autelan oid ??? */
#define AT_COMPANY          "autelan"
#define AT_COPYRIGHT        "Copyright (c) 2012-2015, Autelan Networks. All rights reserved."

#define AT_KERNEL_COUNT     2
#define AT_ROOTFS_COUNT     7
#define AT_KEY_COUNT        (AT_BLOCK_SIZE/sizeof(unsigned int)) /* 128 */
#define AT_VAR_SIZE         32
#define AT_VAR_COUNT        (5*AT_BLOCK_SIZE/AT_VAR_SIZE) /* 80 */

/*
* ok > unknow > fail
*/
enum {
    AT_FSM_FAIL,
    AT_FSM_UNKNOW,
    AT_FSM_OK,

    AT_FSM_END
};
#define AT_FSM_INVALID  AT_FSM_END

#define AT_FSM_STRINGS      {   \
    [AT_FSM_FAIL]   = "fail",   \
    [AT_FSM_UNKNOW] = "unknow", \
    [AT_FSM_OK]     = "ok",     \
}   /* end of AT_FSM_STRINGS */

static inline bool
is_good_at_fsm(int fsm)
{
    return is_good_enum(fsm, AT_FSM_END);
}

static inline bool
is_canused_at_fsm(int fsm)
{
    return is_good_value(fsm, AT_FSM_UNKNOW, AT_FSM_END);
}

static inline char **
__at_fsm_strings(void)
{
    static char *array[AT_FSM_END] = AT_FSM_STRINGS;

    return array;
}

static inline char *
at_fsm_string(int fsm)
{
    char **array = __at_fsm_strings();

    return is_good_at_fsm(fsm)?array[fsm]:__unknow;
}

static inline int
at_fsm_idx(char *fsm)
{
    char **array = __at_fsm_strings();

    return os_getstringarrayidx(array, fsm, 0, AT_FSM_END);
}

static inline int
at_fsm_cmp(int a, int b)
{
    return a - b;
}

#define at_obj_deft(obj, deft)  do{ \
    typeof(*obj) new = deft;        \
    os_memcpy(obj, &new, sizeof(new)); \
}while(0)

#define AT_COPYRIGHT_SIZE   sizeof(AT_COPYRIGHT) /* 64(include '\0') */
#define AT_VENDOR_RESERVED  (AT_BLOCK_SIZE   \
    - sizeof(unsigned int) /* vendor */ \
    - sizeof(unsigned int) /* oid */    \
    - sizeof(AT_COMPANY) /* name */        \
    - sizeof(AT_COPYRIGHT)/* copyright */ \
) /* 512-4-4-8-64 = 432 */

typedef struct {
    unsigned int cookie;
    unsigned int oid;
    unsigned char oui[3], pad;
    char name[sizeof(AT_COMPANY)];
    char copyright[sizeof(AT_COPYRIGHT)];
    char reserved[AT_VENDOR_RESERVED];
} at_vendor_t;   /* 512 */

#define AT_DEFT_VENDOR      {   \
    .cookie     = AT_COOKIE,    \
    .oid        = AT_OID,       \
    .name       = AT_COMPANY,   \
    .copyright  = AT_COPYRIGHT, \
    .reserved   = {0},          \
} /* end of AT_DEFT_VENDOR */

typedef struct { 
    unsigned int number[4]; 
} at_version_t;

#define AT_MIN_VERSION_NUMBER   0
#define AT_MAX_VERSION_NUMBER   9999
#define AT_MIN_VERSION_STRING   "0.0.0.0"
#define AT_MAX_VERSION_STRING   "9999.9999.9999.9999"
#define AT_MIN_VERSION          {   \
    .number = {                     \
        AT_MIN_VERSION_NUMBER,      \
        AT_MIN_VERSION_NUMBER,      \
        AT_MIN_VERSION_NUMBER,      \
        AT_MIN_VERSION_NUMBER,      \
    },                              \
} /* end of AT_MIN_VERSION */
#define AT_MAX_VERSION          {   \
    .number = {                     \
        AT_MAX_VERSION_NUMBER,      \
        AT_MAX_VERSION_NUMBER,      \
        AT_MAX_VERSION_NUMBER,      \
        AT_MAX_VERSION_NUMBER,      \
    },                              \
} /* end of AT_MAX_VERSION */
#define AT_INVALID_VERSION      AT_MIN_VERSION
#define AT_DEFT_VERSION         AT_MIN_VERSION

#define AT_VERSION_STRING_SIZE  sizeof(AT_MAX_VERSION_STRING) /* 20 */

static inline bool
is_good_at_version(at_version_t *version)
{
    return version->number[0] >= AT_MIN_VERSION_NUMBER
        && version->number[0] <= AT_MAX_VERSION_NUMBER
        && version->number[1] >= AT_MIN_VERSION_NUMBER
        && version->number[1] <= AT_MAX_VERSION_NUMBER
        && version->number[2] >= AT_MIN_VERSION_NUMBER
        && version->number[2] <= AT_MAX_VERSION_NUMBER
        && version->number[3] >= AT_MIN_VERSION_NUMBER
        && version->number[3] <= AT_MAX_VERSION_NUMBER;
}

static inline char *
__at_version_itoa(at_version_t *version, char string[])
{
    os_sprintf(string, "%u.%u.%u.%u", 
        version->number[0],
        version->number[1],
        version->number[2],
        version->number[3]);

    return string;
}

static inline char *
at_version_itoa(at_version_t *version)
{
    static char string[AT_VERSION_STRING_SIZE];

    return __at_version_itoa(version, string);
}

static inline int
at_version_atoi(at_version_t *version, char *string)
{
    int count = os_sscanf(string, "%u.%u.%u.%u", 
        &version->number[0],
        &version->number[1],
        &version->number[2],
        &version->number[3]);
    if (4!=count) {
        return -EFORMAT;
    } else {
        return 0;
    }
}

static inline int
at_version_cmp(at_version_t *a, at_version_t *b)
{
    return os_object_cmp(a, b, is_good_at_version, os_objcmp);
}

typedef struct {
    unsigned int self;
    unsigned int other;
    unsigned int upgrade;
    unsigned int error;
    
    at_version_t version;
} at_info_t; /* 48 */

#define __AT_INFO(_self, _other, _upgrade_, _error, _version)   { \
    .self       = _self,        \
    .other      = _other,       \
    .upgrade    = _upgrade_,    \
    .error      = _error,       \
    .version    = _version,     \
} /* end of AT_DEFT_INFO */

#define AT_DEFT_INFO \
        __AT_INFO(AT_FSM_UNKNOW, AT_FSM_UNKNOW, AT_FSM_OK, 0, AT_DEFT_VERSION)
#define AT_MIN_INFO \
        __AT_INFO(AT_FSM_FAIL, AT_FSM_FAIL, AT_FSM_FAIL, AT_TRYS, AT_MIN_VERSION)
#define AT_MAX_INFO \
        __AT_INFO(AT_FSM_OK, AT_FSM_OK, AT_FSM_OK, 0, AT_MAX_VERSION)

#define is_good_at_error(_error)    ((_error) < AT_TRYS)

static inline int
at_error_cmp(unsigned int a, unsigned b)
{
#define __at_error_cmp(_a, _b)  0
    return os_object_cmp(a, b, is_good_at_error, __at_error_cmp);
#undef  __at_error_cmp
}

static inline bool
at_info_is_good(at_info_t *info)
{
    return info->error < AT_TRYS
        && AT_FSM_OK==info->upgrade
        && is_canused_at_fsm(info->self);
}

static inline bool
at_info_match(at_info_t *info, at_info_t *filter)
{
    at_version_t invalid = AT_INVALID_VERSION;
    
    if (NULL==filter) {
        return true;
    }
    else if (filter->error >= AT_TRYS && info->error < AT_TRYS) {
        return false;
    }
    else if (filter->error < AT_TRYS && info->error >= AT_TRYS) {
        return false;
    }
    else if (OS_INVALID!=filter->self && filter->self!=info->self) {
        return false;
    }
    else if (OS_INVALID!=filter->other && filter->other!=info->other) {
        return false;
    }
    else if (OS_INVALID!=filter->upgrade && filter->upgrade!=info->upgrade) {
        return false;
    }
    else if (false==os_objeq(&invalid, &filter->version) 
        && false==os_objeq(&info->version, &filter->version)) {
        return false;
    }
    else {
        return true;
    }
}

typedef struct {
    unsigned int current;
    
    at_info_t info[0];
} at_osinfo_t;

#define AT_DEFT_OSINFO(_count, _current)  { \
    .current = _current,                    \
    .info = {                               \
        [0 ... (_count-1)] = AT_DEFT_INFO,  \
    }, \
} /* end of AT_DEFT_OSINFO */

typedef struct {
    unsigned int current;
    
    at_info_t info[AT_KERNEL_COUNT];
} at_kernel_t;

#define AT_DEFT_KERNEL   AT_DEFT_OSINFO(AT_KERNEL_COUNT, 0)

typedef struct {
    unsigned int current;
        
    at_info_t info[AT_ROOTFS_COUNT];
} at_rootfs_t;

#define AT_DEFT_ROOTFS   AT_DEFT_OSINFO(AT_ROOTFS_COUNT, 1)

#define AT_FIRMWARE_RESERVED   (AT_BLOCK_SIZE   \
    - 2 * sizeof(unsigned int) \
    - (AT_KERNEL_COUNT + AT_ROOTFS_COUNT) * sizeof(at_info_t) \
) /* 512 - 2*4 - (2+7)*48 = 72 */

typedef struct {
    at_kernel_t kernel;
    at_rootfs_t rootfs;

    unsigned char reserved[AT_FIRMWARE_RESERVED];
} at_firmware_t; /* 512 */

#define AT_DEFT_FIRMWARE    {   \
    .kernel = AT_DEFT_KERNEL,   \
    .rootfs = AT_DEFT_ROOTFS,   \
} /* end of AT_DEFT_FIRMWARE */

typedef struct {
    unsigned int key[AT_KEY_COUNT];
} at_key_t; /* 512 */

#define AT_DEFT_KEY { .key = {0} }

typedef struct {
    char var[AT_VAR_COUNT][AT_VAR_SIZE];
} at_var_t; /* 512 */

#define AT_DEFT_VAR { .var = {{0}} }

typedef struct {
    at_vendor_t     vendor;
    at_firmware_t   firmware;
    at_key_t        key;
    at_var_t        var;
} at_env_t; /* 4k */

#define AT_DEFT_ENV             {   \
    .vendor = AT_DEFT_VENDOR,       \
    .firmware = AT_DEFT_FIRMWARE,   \
    .key = AT_DEFT_KEY,             \
    .var = AT_DEFT_VAR,             \
} /* end of AT_DEFT_ENV */

typedef struct struct_at_ops at_ops_t;
struct struct_at_ops {
    char *path;
    unsigned int offset;
    
    int  (*check)(at_ops_t *ops, char *value);
    void (*set)(at_ops_t *ops, char *value);
    void (*show)(at_ops_t *ops);
};

typedef struct {
    char *value;
} at_ops_cache_t;

static inline int
at_ops_check(at_ops_t *ops, char *value)
{
    if (ops->check) {
        return (*ops->check)(ops, value);
    } else {
        return 0;
    }
}

static inline void
at_ops_set(at_ops_t *ops, char *value)
{
    if (ops->set) {
        (*ops->set)(ops, value);
    }
}

static inline void
at_ops_show(at_ops_t *ops)
{
    if (ops->show) {
        (*ops->show)(ops);
    }
}

static inline bool
__at_ops_match(at_ops_t *ops, char *path, int len)
{
    return 0==len || 0==os_memcmp(path, ops->path, len);
}

static inline bool
at_ops_match(at_ops_t *ops, char *path, bool isall)
{
    if (isall) {
        return __at_ops_match(ops, path, os_strlen(path) - 1);
    }
    else {
        return 0==os_strcmp(path, ops->path);
    }
}

typedef struct {
    at_env_t *env;
    at_ops_t *ops;
    at_ops_cache_t *ops_cache;
    int ops_count;
    
#ifdef __APP__
    bool dirty[AT_BLOCK_COUNT];
    FILE *f;
#endif

    bool isset;
    bool isall;
    int  argc;
    char **argv;
} at_control_t;

#define AT_CONTROL_DEFT(_env, _ops, _ops_cache)   { \
    .env = _env,                        \
    .ops = _ops,                        \
    .ops_cache = _ops_cache,            \
    .ops_count = os_count_of(_ops),     \
} /* end of AT_CONTROL_DEFT */

extern at_control_t at_control;

#define __at_ops                at_control.ops
#define __at_ops_count          at_control.ops_count
#define __at_ops_cache          at_control.ops_cache

#define __at_env                at_control.env
#define __at_vendor             (&__at_env->vendor)
#define __at_firmware           (&__at_env->firmware)
#define __at_kernel             (&__at_firmware->kernel)
#define __at_rootfs             (&__at_firmware->rootfs)
#define __at_key                (&__at_env->key)
#define __at_var                (&__at_env->var)
#define __at_info(_obj)         __at_##_obj->info

#define at_info(_obj, _idx)     (__at_info(_obj) + (_idx))
#define at_kernel(_idx)         at_info(kernel, _idx)
#define at_rootfs(_idx)         at_info(rootfs, _idx)
#define at_key(_idx)            __at_key->key[_idx]
#define at_var(_idx)            __at_var->var[_idx]

#define at_ops(_idx)                (&__at_ops[_idx])
#define at_ops_idx(_ops)            ((at_ops_t *)(_ops) - __at_ops)
#define at_ops_cache(_ops)          (&__at_ops_cache[at_ops_idx(_ops)])
#define at_ops_cache_value(_ops)    at_ops_cache(_ops)->value
#define at_ops_address(_type, _ops) ((_type *)((char *)__at_env + (_ops)->offset))

static inline void
at_init(void)
{
    os_memzero(__at_ops_cache, __at_ops_count * sizeof(at_ops_cache_t));

#ifdef __APP__
    os_arrayzero(at_control.dirty);
#endif

    at_control.isset    = false;
    at_control.isall    = false;
    at_control.argc     = 0;
    at_control.argv     = NULL;
}

#define AT_ROOTFS_SORT_COUNT    (AT_ROOTFS_COUNT - 2)

/*
* 1. error, bad < good
* 2. version, small < big
* 3. upgrade, fail < unknow < ok
* 4. other, fail < unknow < ok
* 5. self, fail < unknow < ok
*/
static inline int
at_rootfs_cmp(at_info_t *a, at_info_t *b)
{
    int ret;

    ret = at_error_cmp(a->error, b->error);
    if (ret) {
        return ret;
    }

    ret = at_version_cmp(&a->version, &b->version);
    if (ret) {
        return ret;
    }

    ret = at_fsm_cmp(a->upgrade, b->upgrade);
    if (ret) {
        return ret;
    }

    ret = at_fsm_cmp(a->other, b->other);
    if (ret) {
        return ret;
    }

    ret = at_fsm_cmp(a->self, b->self);
    if (ret) {
        return ret;
    }

    return 0;
}

/*
* return array idx
*/
static inline int
at_rootfs_min(int array[], int count)
{
    int i, min = count;
    at_info_t rootfs_max = AT_MAX_INFO;
    at_info_t *rootfs_min = &rootfs_max;
    at_info_t *rootfs;
    
    for (i=0; i<count; i++) {
        rootfs = at_rootfs(array[i]);

        /*
        * rootfs <= rootfs_min
        */
        if (at_rootfs_cmp(rootfs, rootfs_min) <= 0) {
            rootfs_min = rootfs; min = i;
        }
    }

    return min;
}

static inline void
at_rootfs_sort_init(int current, int array[AT_ROOTFS_SORT_COUNT])
{
    int i, idx = 0;

    for (i=1; i<AT_ROOTFS_COUNT; i++) {
        if (i==current) {
            continue;
        }

        array[idx++] = i;
    }
}


static inline void
__at_rootfs_sort(int array[], int count)
{
    if (count <= 1) {
        return;
    }
    
    int min = at_rootfs_min(array, count);

    if (min) {
        os_swap_value(array[0], array[min]);
    }

    __at_rootfs_sort(array + 1, count - 1);
}

/*
* up-sort, bad==>good
*
* 1. error, bad==>good
* 2. version, small==>big
* 3. upgrade, fail==>unknow==>ok
* 4. other, fail==>unknow==>ok
* 5. self, fail==>unknow==>ok
*/
static inline void
at_rootfs_sort(int current, int array[AT_ROOTFS_SORT_COUNT])
{   
    at_rootfs_sort_init(current, array);

    __at_rootfs_sort(array, AT_ROOTFS_SORT_COUNT);
}

static inline int
at_rootfs_foreach(at_info_t *filter, multi_value_t (*foreach)(at_info_t *info))
{
    int i;
    multi_value_u mv;
    
    for (i=1; i<AT_ROOTFS_COUNT; i++) {
        at_info_t *rootfs = at_rootfs(i);
        
        if (at_info_match(rootfs, filter)) {
            mv.value = (*foreach)(rootfs);
            if (mv2_is_break(mv)) {
                return mv2_result(mv);
            }
        }
    }

    return 0;
}

#if 0
static int
__at_get_rootfs_ok(at_info_t *filter)
{
    int idx = -1;
    at_version_t version = AT_MIN_VERSION;
    
    multi_value_t foreach(at_info_t *info)
    {
        if (memcmp(&version, &info->version) > 0) {
            idx = info - at_rootfs->info;
        }
        
        return mv2_OK;
    }
    
    at_rootfs_foreach(filter, foreach);

    return idx;
}

static int
at_get_rootfs_self_ok(void)
{
    at_info_t filter = __AT_INFO(AT_FSM_OK, AT_FSM_INVALID, AT_FSM_INVALID, AT_FSM_INVALID, AT_INVALID_VERSION);

    return __at_get_rootfs_ok(&filter);
}

static int
at_get_rootfs_other_ok(void)
{
    at_info_t filter = __AT_INFO(AT_FSM_INVALID, AT_FSM_OK, AT_FSM_INVALID, AT_FSM_INVALID, AT_INVALID_VERSION);

    return __at_get_rootfs_ok(&filter);
}

static int
at_get_rootfs_ok(void)
{
    int idx = -1;

    idx = at_get_rootfs_self_ok();
    if (idx>0) {
        return idx;
    }

    idx = at_get_rootfs_other_ok();
    if (idx>0) {
        return idx;
    }

    return 0;
}
#endif

static inline void 
at_show_header(at_ops_t *ops)
{
    if (false==at_control.isset
        && false==at_control.isall
        && 1==at_control.argc) {
        os_printf("%s=", ops->path);
    }
}

static inline void 
at_show_uint(at_ops_t *ops)
{
    at_show_header(ops);

    os_println("%u", *at_ops_address(unsigned int, ops));
}

static inline void 
at_show_string(at_ops_t *ops)
{
    char *string = at_ops_address(char, ops);

    if (string[0]) {
        at_show_header(ops);

        os_println("%s", string);
    }
}

static inline void
at_set_dirty(at_ops_t *ops)
{
#ifdef __APP__
    int offset  = at_ops_address(char, ops) - (char *)__at_env;
    int idx     = os_align(offset, AT_BLOCK_SIZE);
    
    at_control.dirty[idx] = true;
#endif
}

static inline void 
at_set_uint(at_ops_t *ops, char *value)
{
    *at_ops_address(unsigned int, ops) = 
        (unsigned int)(value[0]?os_atoi(value):0);
}

static inline void 
at_set_string(at_ops_t *ops, char *value)
{
    char *string = at_ops_address(char, ops);
    
    if (value[0]) {
        os_strcpy(string, value);
    } else {
        string[0] = 0;
    }
}


static inline int 
at_check_firmware_version(at_ops_t *ops, char *value)
{
    at_version_t version = AT_INVALID_VERSION;
    
    if (at_version_atoi(&version, value)) {
        /*
        * when set version, must input value
        */
        return -EFORMAT;
    } else {
        return is_good_at_version(&version)?0:-EFORMAT;
    }
}

static inline void 
at_show_firmware_version(at_ops_t *ops)
{
    at_show_header(ops);

    os_println("%s", at_version_itoa(at_ops_address(at_version_t, ops)));
}

static inline void 
at_set_firmware_version(at_ops_t *ops, char *value)
{
    at_version_t *v = at_ops_address(at_version_t, ops);
    
    if (value[0]) {
        at_version_atoi(v, value);
    } else {
        at_version_t version = AT_DEFT_VERSION;

        os_objscpy(v, &version);
    }
}

static inline int 
at_check_firmware_fsm(at_ops_t *ops, char *value)
{
    if (value[0]) {
        return is_good_at_fsm(at_fsm_idx(value))?0:-EFORMAT;
    } else {
        /*
        * when set self/other/upgrade, must input value
        */
        return -EFORMAT;
    }
}

static inline void 
at_show_firmware_fsm(at_ops_t *ops)
{
    int fsm = *at_ops_address(unsigned int, ops);
    
    at_show_header(ops);
    
    os_println("%s", at_fsm_string(fsm));
}

static inline void 
at_set_firmware_fsm(at_ops_t *ops, char *value)
{
    unsigned int fsm;
    
    if (value[0]) {
        fsm = at_fsm_idx(value);
    } else {
        fsm = AT_FSM_UNKNOW;
    }

    *at_ops_address(unsigned int, ops) = fsm;
}

static inline int 
at_check_kernel_current(at_ops_t *ops, char *value)
{
    if (value[0]) {
        int v = os_atoi(value);

        return is_good_enum(v, AT_KERNEL_COUNT);
    } else {
        /*
        * when set kernel current, must input value
        */
        return -EFORMAT;
    }
}

static inline int 
at_check_rootfs_current(at_ops_t *ops, char *value)
{
    if (value[0]) {
        int v = os_atoi(value);

        return is_good_enum(v, AT_ROOTFS_COUNT)?0:-EFORMAT;
    } else {
        /*
        * when set rootfs current, must input value
        */
        return -EFORMAT;
    }
}

static inline bool 
at_check_var_string(at_ops_t *ops, char *value)
{
    if (value[0]) {
        return (os_strlen(value) < AT_VAR_SIZE)?0:-EFORMAT;
    } else {
        /*
        * when set var string, may NOT input value
        */
        return 0;
    }
}

#define AT_OPS_OFFSET(_addr)    (unsigned int)((char *)_addr - (char *)__at_env)
#define AT_OPS(_path, _addr, _check, _set, _show) { \
    .path   = _path,    \
    .offset = AT_OPS_OFFSET(_addr), \
    .check  = _check,   \
    .set    = _set,     \
    .show   = _show,    \
} /* end of __AT_OPS */

#define __AT_VENDOR_OPS(_path, _addr, _show) \
    AT_OPS(_path, _addr, NULL, NULL, _show)

#define AT_VENDOR_OPS \
    __AT_VENDOR_OPS("vendor/cookie",    &__at_vendor->cookie,   at_show_uint),  \
    __AT_VENDOR_OPS("vendor/oid",       &__at_vendor->oid,      at_show_uint),  \
    __AT_VENDOR_OPS("vendor/name",      __at_vendor->name,      at_show_string),\
    __AT_VENDOR_OPS("vendor/copyright", __at_vendor->copyright, at_show_string) \
    /* end of AT_VENDOR_OPS */

#define __AT_FIRMWARE_OPS(_obj, _idx) \
    AT_OPS(#_obj "/" #_idx "/self", &at_info(_obj, _idx)->self, \
        at_check_firmware_fsm, at_set_firmware_fsm, at_show_firmware_fsm), \
    AT_OPS(#_obj "/" #_idx "/other", &at_info(_obj, _idx)->other, \
        at_check_firmware_fsm, at_set_firmware_fsm, at_show_firmware_fsm), \
    AT_OPS(#_obj "/" #_idx "/upgrade", &at_info(_obj, _idx)->upgrade, \
        at_check_firmware_fsm, at_set_firmware_fsm, at_show_firmware_fsm), \
    AT_OPS(#_obj "/" #_idx "/error", &at_info(_obj, _idx)->error, \
        NULL, at_set_uint, at_show_uint), \
    AT_OPS(#_obj "/" #_idx "/version", &at_info(_obj, _idx)->version, \
        at_check_firmware_version, at_set_firmware_version, at_show_firmware_version) \
    /* end of __AT_FIRMWARE_OPS */

#define AT_FIRMWARE_OPS \
    AT_OPS("kernel/current", &__at_kernel->current, \
        at_check_kernel_current, at_set_uint, at_show_uint), \
    __AT_FIRMWARE_OPS(kernel, 0), \
    __AT_FIRMWARE_OPS(kernel, 1), \
    AT_OPS("rootfs/current", &__at_rootfs->current, \
        at_check_rootfs_current, at_set_uint, at_show_uint), \
    __AT_FIRMWARE_OPS(rootfs, 0), \
    __AT_FIRMWARE_OPS(rootfs, 1), \
    __AT_FIRMWARE_OPS(rootfs, 2), \
    __AT_FIRMWARE_OPS(rootfs, 3), \
    __AT_FIRMWARE_OPS(rootfs, 4), \
    __AT_FIRMWARE_OPS(rootfs, 5), \
    __AT_FIRMWARE_OPS(rootfs, 6)  \
    /* end of AT_FIRMWARE_OPS */

#define __AT_KEY_OPS(_path, _idx, _check) \
    AT_OPS("key/" _path, &at_key(_idx), _check, at_set_uint, at_show_uint)

#define AT_KEY_OPS_NAMES_COMMON \
    __AT_KEY_OPS("boot/debug", 0, NULL)

#ifdef __BOOT__
#define AT_KEY_OPS_NAMES    \
    AT_KEY_OPS_NAMES_COMMON \
    /* end of AT_KEY_OPS_NAMES */

#define AT_KEY_OPS      \
    AT_KEY_OPS_NAMES    \
    /* end of AT_KEY_OPS */

#elif defined(__APP__)
#define AT_KEY_OPS_NAMES    \
    AT_KEY_OPS_NAMES_COMMON \
    /* end of AT_KEY_OPS_NAMES */

#define __AT_KEY_OPS_IDX(_idx) \
    __AT_KEY_OPS(#_idx, _idx, NULL)

#define AT_KEY_OPS_IDX \
    __AT_KEY_OPS_IDX(1), \
    __AT_KEY_OPS_IDX(2), \
    __AT_KEY_OPS_IDX(3), \
    __AT_KEY_OPS_IDX(4), \
    __AT_KEY_OPS_IDX(5), \
    __AT_KEY_OPS_IDX(6), \
    __AT_KEY_OPS_IDX(7), \
    __AT_KEY_OPS_IDX(8), \
    __AT_KEY_OPS_IDX(9), \
    __AT_KEY_OPS_IDX(11), \
    __AT_KEY_OPS_IDX(12), \
    __AT_KEY_OPS_IDX(13), \
    __AT_KEY_OPS_IDX(14), \
    __AT_KEY_OPS_IDX(15), \
    __AT_KEY_OPS_IDX(16), \
    __AT_KEY_OPS_IDX(17), \
    __AT_KEY_OPS_IDX(18), \
    __AT_KEY_OPS_IDX(19), \
    __AT_KEY_OPS_IDX(20), \
    __AT_KEY_OPS_IDX(21), \
    __AT_KEY_OPS_IDX(22), \
    __AT_KEY_OPS_IDX(23), \
    __AT_KEY_OPS_IDX(24), \
    __AT_KEY_OPS_IDX(25), \
    __AT_KEY_OPS_IDX(26), \
    __AT_KEY_OPS_IDX(27), \
    __AT_KEY_OPS_IDX(28), \
    __AT_KEY_OPS_IDX(29), \
    __AT_KEY_OPS_IDX(30), \
    __AT_KEY_OPS_IDX(31), \
    __AT_KEY_OPS_IDX(32), \
    __AT_KEY_OPS_IDX(33), \
    __AT_KEY_OPS_IDX(34), \
    __AT_KEY_OPS_IDX(35), \
    __AT_KEY_OPS_IDX(36), \
    __AT_KEY_OPS_IDX(37), \
    __AT_KEY_OPS_IDX(38), \
    __AT_KEY_OPS_IDX(39), \
    __AT_KEY_OPS_IDX(40), \
    __AT_KEY_OPS_IDX(41), \
    __AT_KEY_OPS_IDX(42), \
    __AT_KEY_OPS_IDX(43), \
    __AT_KEY_OPS_IDX(44), \
    __AT_KEY_OPS_IDX(45), \
    __AT_KEY_OPS_IDX(46), \
    __AT_KEY_OPS_IDX(47), \
    __AT_KEY_OPS_IDX(48), \
    __AT_KEY_OPS_IDX(49), \
    __AT_KEY_OPS_IDX(50), \
    __AT_KEY_OPS_IDX(51), \
    __AT_KEY_OPS_IDX(52), \
    __AT_KEY_OPS_IDX(53), \
    __AT_KEY_OPS_IDX(54), \
    __AT_KEY_OPS_IDX(55), \
    __AT_KEY_OPS_IDX(56), \
    __AT_KEY_OPS_IDX(57), \
    __AT_KEY_OPS_IDX(58), \
    __AT_KEY_OPS_IDX(59), \
    __AT_KEY_OPS_IDX(60), \
    __AT_KEY_OPS_IDX(61), \
    __AT_KEY_OPS_IDX(62), \
    __AT_KEY_OPS_IDX(63), \
    __AT_KEY_OPS_IDX(64), \
    __AT_KEY_OPS_IDX(65), \
    __AT_KEY_OPS_IDX(66), \
    __AT_KEY_OPS_IDX(67), \
    __AT_KEY_OPS_IDX(68), \
    __AT_KEY_OPS_IDX(69), \
    __AT_KEY_OPS_IDX(70), \
    __AT_KEY_OPS_IDX(71), \
    __AT_KEY_OPS_IDX(72), \
    __AT_KEY_OPS_IDX(73), \
    __AT_KEY_OPS_IDX(74), \
    __AT_KEY_OPS_IDX(75), \
    __AT_KEY_OPS_IDX(76), \
    __AT_KEY_OPS_IDX(77), \
    __AT_KEY_OPS_IDX(78), \
    __AT_KEY_OPS_IDX(79), \
    __AT_KEY_OPS_IDX(80), \
    __AT_KEY_OPS_IDX(81), \
    __AT_KEY_OPS_IDX(82), \
    __AT_KEY_OPS_IDX(83), \
    __AT_KEY_OPS_IDX(84), \
    __AT_KEY_OPS_IDX(85), \
    __AT_KEY_OPS_IDX(86), \
    __AT_KEY_OPS_IDX(87), \
    __AT_KEY_OPS_IDX(88), \
    __AT_KEY_OPS_IDX(89), \
    __AT_KEY_OPS_IDX(90), \
    __AT_KEY_OPS_IDX(91), \
    __AT_KEY_OPS_IDX(92), \
    __AT_KEY_OPS_IDX(93), \
    __AT_KEY_OPS_IDX(94), \
    __AT_KEY_OPS_IDX(95), \
    __AT_KEY_OPS_IDX(96), \
    __AT_KEY_OPS_IDX(97), \
    __AT_KEY_OPS_IDX(98), \
    __AT_KEY_OPS_IDX(99), \
    __AT_KEY_OPS_IDX(100), \
    __AT_KEY_OPS_IDX(101), \
    __AT_KEY_OPS_IDX(102), \
    __AT_KEY_OPS_IDX(103), \
    __AT_KEY_OPS_IDX(104), \
    __AT_KEY_OPS_IDX(105), \
    __AT_KEY_OPS_IDX(106), \
    __AT_KEY_OPS_IDX(107), \
    __AT_KEY_OPS_IDX(108), \
    __AT_KEY_OPS_IDX(109), \
    __AT_KEY_OPS_IDX(110), \
    __AT_KEY_OPS_IDX(111), \
    __AT_KEY_OPS_IDX(112), \
    __AT_KEY_OPS_IDX(113), \
    __AT_KEY_OPS_IDX(114), \
    __AT_KEY_OPS_IDX(115), \
    __AT_KEY_OPS_IDX(116), \
    __AT_KEY_OPS_IDX(117), \
    __AT_KEY_OPS_IDX(118), \
    __AT_KEY_OPS_IDX(119), \
    __AT_KEY_OPS_IDX(120), \
    __AT_KEY_OPS_IDX(121), \
    __AT_KEY_OPS_IDX(122), \
    __AT_KEY_OPS_IDX(123), \
    __AT_KEY_OPS_IDX(124), \
    __AT_KEY_OPS_IDX(125), \
    __AT_KEY_OPS_IDX(126), \
    __AT_KEY_OPS_IDX(127)  \
    /* end of AT_KEY_OPS_IDX */

#define AT_KEY_OPS      \
    AT_KEY_OPS_NAMES,   \
    AT_KEY_OPS_IDX      \
    /* end of AT_KEY_OPS */
#endif

#define __AT_VAR_OPS(_path, _idx) \
    AT_OPS("var/" _path, at_var(_idx), at_check_var_string, at_set_string, at_show_string)

#define AT_VAR_OPS_NAMES_COMMON     \
    __AT_VAR_OPS("ptest",       0), \
    __AT_VAR_OPS("ptest/result",1), \
    __AT_VAR_OPS("pcba/type",   2), \
    __AT_VAR_OPS("pcba/version",3), \
    __AT_VAR_OPS("pcba/mac",    4), \
    __AT_VAR_OPS("pcba/sn",     5), \
    __AT_VAR_OPS("oem/name",    6)  \
    /* end of AT_VAR_OPS_NAMES */

#ifdef __BOOT__
#define AT_VAR_OPS_NAMES \
    AT_VAR_OPS_NAMES_COMMON

#define AT_VAR_OPS \
    AT_VAR_OPS_NAMES

#elif defined(__APP__)
#define AT_VAR_OPS_NAMES \
    AT_VAR_OPS_NAMES_COMMON

#define __AT_VAR_OPS_IDX(_idx) \
    __AT_VAR_OPS(#_idx, _idx)

#define AT_VAR_OPS_IDX \
    __AT_VAR_OPS_IDX(7), \
    __AT_VAR_OPS_IDX(8), \
    __AT_VAR_OPS_IDX(9), \
    __AT_VAR_OPS_IDX(11), \
    __AT_VAR_OPS_IDX(12), \
    __AT_VAR_OPS_IDX(13), \
    __AT_VAR_OPS_IDX(14), \
    __AT_VAR_OPS_IDX(15), \
    __AT_VAR_OPS_IDX(16), \
    __AT_VAR_OPS_IDX(17), \
    __AT_VAR_OPS_IDX(18), \
    __AT_VAR_OPS_IDX(19), \
    __AT_VAR_OPS_IDX(20), \
    __AT_VAR_OPS_IDX(21), \
    __AT_VAR_OPS_IDX(22), \
    __AT_VAR_OPS_IDX(23), \
    __AT_VAR_OPS_IDX(24), \
    __AT_VAR_OPS_IDX(25), \
    __AT_VAR_OPS_IDX(26), \
    __AT_VAR_OPS_IDX(27), \
    __AT_VAR_OPS_IDX(28), \
    __AT_VAR_OPS_IDX(29), \
    __AT_VAR_OPS_IDX(30), \
    __AT_VAR_OPS_IDX(31), \
    __AT_VAR_OPS_IDX(32), \
    __AT_VAR_OPS_IDX(33), \
    __AT_VAR_OPS_IDX(34), \
    __AT_VAR_OPS_IDX(35), \
    __AT_VAR_OPS_IDX(36), \
    __AT_VAR_OPS_IDX(37), \
    __AT_VAR_OPS_IDX(38), \
    __AT_VAR_OPS_IDX(39), \
    __AT_VAR_OPS_IDX(40), \
    __AT_VAR_OPS_IDX(41), \
    __AT_VAR_OPS_IDX(42), \
    __AT_VAR_OPS_IDX(43), \
    __AT_VAR_OPS_IDX(44), \
    __AT_VAR_OPS_IDX(45), \
    __AT_VAR_OPS_IDX(46), \
    __AT_VAR_OPS_IDX(47), \
    __AT_VAR_OPS_IDX(48), \
    __AT_VAR_OPS_IDX(49), \
    __AT_VAR_OPS_IDX(50), \
    __AT_VAR_OPS_IDX(51), \
    __AT_VAR_OPS_IDX(52), \
    __AT_VAR_OPS_IDX(53), \
    __AT_VAR_OPS_IDX(54), \
    __AT_VAR_OPS_IDX(55), \
    __AT_VAR_OPS_IDX(56), \
    __AT_VAR_OPS_IDX(57), \
    __AT_VAR_OPS_IDX(58), \
    __AT_VAR_OPS_IDX(59), \
    __AT_VAR_OPS_IDX(60), \
    __AT_VAR_OPS_IDX(61), \
    __AT_VAR_OPS_IDX(62), \
    __AT_VAR_OPS_IDX(63), \
    __AT_VAR_OPS_IDX(64), \
    __AT_VAR_OPS_IDX(65), \
    __AT_VAR_OPS_IDX(66), \
    __AT_VAR_OPS_IDX(67), \
    __AT_VAR_OPS_IDX(68), \
    __AT_VAR_OPS_IDX(69), \
    __AT_VAR_OPS_IDX(70), \
    __AT_VAR_OPS_IDX(71), \
    __AT_VAR_OPS_IDX(72), \
    __AT_VAR_OPS_IDX(73), \
    __AT_VAR_OPS_IDX(74), \
    __AT_VAR_OPS_IDX(75), \
    __AT_VAR_OPS_IDX(76), \
    __AT_VAR_OPS_IDX(77), \
    __AT_VAR_OPS_IDX(78), \
    __AT_VAR_OPS_IDX(79)  \
    /* end of AT_VAR_OPS_IDX */

#define AT_VAR_OPS      \
    AT_VAR_OPS_NAMES,   \
    AT_VAR_OPS_IDX      \
    /* end of AT_VAR_OPS */
#endif

#define AT_DEFT_OPS {   \
    AT_VENDOR_OPS,      \
    AT_FIRMWARE_OPS,    \
    AT_KEY_OPS,         \
    AT_VAR_OPS,         \
} /* end of AT_DEFT_OPS */

static inline void
__at_show_byprefix(char *prefix)
{
    int i;
    int len = prefix?os_strlen(prefix):0;
    
    for (i=0; i<__at_ops_count; i++) {
        at_ops_t *ops = at_ops(i);
        
        if (__at_ops_match(ops, prefix, len)) {
            at_ops_show(ops);
        }
    }
}

#define at_show_byprefix(_fmt, _args...) do{\
    char prefix[1+OS_LINE_LEN];             \
                                            \
    os_saprintf(prefix, _fmt, ##args);      \
                                            \
    __at_show_byprefix(prefix);             \
}while(0)

static inline void
at_handle(void)
{
    int i;

    for (i=0; i<__at_ops_count; i++) {
        at_ops_t *ops = at_ops(i);
        
        if (at_control.isset) {
            at_ops_set(ops, at_ops_cache_value(ops));

            at_set_dirty(ops);
        } else {
            at_ops_show(ops);
        }
    }
}

static inline int
at_usage(void)
{
    char *self = at_control.argv[0];
    
    os_println("%s name ==> show env by name", self);
    os_println("%s name1=value1 name2=value2 ... ==> set env by name and value", self);

    return -EHELP;
}

static inline char *
__at_asterisk(char *args)
{
    return os_strlast(args, '*');
}

static inline bool
__at_isset(char *args)
{
    return NULL != os_strchr(args, '=');
}

static inline int
__at_analysis_args(char *path, char *value, bool isall)
{
    int i, err, count = 0;

    for (i=0; i<__at_ops_count; i++) {
        at_ops_t *ops = at_ops(i);
        
        if (false==at_ops_match(ops, path, isall)) {
            continue;
        }

        /*
        * isset
        */
        if (value) {
            err = at_ops_check(ops, value);
            if (err) {
                return err;
            }
            
            at_ops_cache_value(ops) = value;
        }
        
        count++;
    }

    return count;
}

static inline int
at_analysis_args(char *path, char *value, bool isall)
{
    int count;
    
    /*
    * check path
    */
    if (isall && os_strchr(path, '*') != __os_strlast(path)) {
        return -EFORMAT;
    }

    count = __at_analysis_args(path, value, isall);
    if (0==count) {
        return -ENOEXIST;
    }

    return 0;
}

static inline int
__at_check(char *args)
{
    bool isall = __at_asterisk(args)?true:false;
    
    char *eq = os_strchr(args, '=');
    if (NULL==eq) {
        /*
        * no found '=', just show
        */
        return at_analysis_args(args, NULL, isall);
    }
    else if (isall) {
        /*
        * isall ==> show
        *
        * found '=' ==>set
        */
        return -EFORMAT;
    }
    else {
        /*
        * set
        */
        char *value = eq + 1;
        char path[1+OS_LINE_LEN];
        
        os_strmcpy(path, args, eq - args);
        
        return at_analysis_args(path, value, isall);
    }
}

static inline void
at_check_isall(int argc, char *argv[])
{
    int i;
    
    for (i=0; i<argc; i++) {
        char *args = argv[i];

        if (__at_asterisk(args)) {
            at_control.isall = true;

            return;
        }
    }
}

static inline int
at_check_rw(int argc, char *argv[])
{
    int i, count = 0;

    /*
    * isset ?
    */
    for (i=0; i<argc; i++) {
        char *args = argv[i];
        
        if (__at_isset(args)) {
            count++;
        }
    }
    
    if (0==count) {
        /*
        * all is get
        */
        at_control.isset = false;
    }
    else if (argc==count) {
        /*
        * all is set
        */
        at_control.isset = true;
    }
    else {
        return -EHELP;
    }
    
    return 0;
}

static inline int
at_check(int argc, char *argv[])
{
    int i;
    
    for (i=0; i<argc; i++) {
        int err = __at_check(argv[i]);
        if (err) {
            return err;
        }
    }
    
    return 0;
}

static inline int
at_command(int argc, char *argv[])
{
    int err;
    
    if (0==argc) {
        return at_usage();
    }

    at_check_isall(argc, argv);

    err = at_check_rw(argc, argv);
    if (err) {
        return err;
    }
    
    err = at_check(argc, argv);
    if (err) {
        return err;
    }

    at_handle();

    return 0;
}

static inline int
at_main(int argc, char *argv[])
{
    at_init();
    
    at_control.argc = argc--;
    at_control.argv = argv++;
    
    return at_command(argc, argv);
}

/******************************************************************************/
#endif /* __ATENV_H_31502FF802B81D17AEDABEB5EDB5C121__ */

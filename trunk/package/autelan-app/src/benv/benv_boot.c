/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#include <mmc.h>

static at_ops_t __ops[]= AT_DEFT_OPS;
static at_ops_cache_t __ops_cache[os_count_of(__ops)];
at_control_t at_control     = AT_CONTROL_DEFT(NULL, __ops, __ops_cache);

static int 
read_emmc(unsigned int begin, void *buf, int size)
{
    struct mmc *mmc = find_mmc_device(0);
    int ret;

    if (!mmc) {
        os_println("init mmc error");
        return -1;
    }

    ret = mmc->block_dev.block_read(0, begin >> 9,
        size >> 9, buf);
    if (ret != (size >> 9)){
        os_println("read emmc error, begin:0x%x, size:0x%x", begin, size);
        return -1;
    }

    return ret << 9;
}

static int 
write_emmc(unsigned int begin, void *buf, int size)
{
    struct mmc *mmc = find_mmc_device(0);
    int ret;

    if (!mmc) {
        os_println("init mmc error");
        return -1;
    }

    ret = mmc->block_dev.block_write(0, begin >> 9,
        size >> 9, buf);
    if (ret != (size >> 9)) {
        os_println("write emmc error, begin:0x%x, size:0x%x", begin, size);
        return -1;
    }
    
    return ret << 9;
}

#if 0
static char *
get_first_env(void)
{
    char *first = (char *)env_ptr->data;

    if (first[0]) {
        dprintln("get first env:%s", first);

        return first;
    } else {
        os_println("no found first env");
        
        return NULL;
    }
}

static char *
get_next_env(char *env)
{
    int len = strlen(env);
    char *next = env + len + 1; /* skip '\0' */

    if (next[0]) {
        dprintln("get next env:%s", next);

        return next;
    } else {
        dprintln("no found next env");
        
        return NULL;
    }
}

static char *
get_env_byname(char *name)
{
    char *env = get_first_env();

    while(env) {
        if (0==memcmp(env, name, strlen(name))) {
            return env;
        }
        
        env = get_next_env(env);
    }

    os_println("no found env:%s", name);
    
    return NULL;    
}
#endif

static char *
__change_bootenv(char *tag, char *name, char *find, int idx, int new)
{
    char *env, *key, *value;
    
    env = getenv(name);
    if (NULL==env) {
        return NULL;
    }
    
    key = strstr(env, find);
    if (NULL==key) {
        return NULL;
    }
    
    value = key + strlen(find);
    if (*value != new) {
        os_println("%s changed from %c to %c", tag, *value, idx);
        *value = new;
    }

    return env;
}

static char *
change_bootcmd(int idx)
{
    int idx = __at_kernel->current;
    
    return __change_bootenv(
                "kernel", 
                "bootcmd", 
                CONFIG_BOOTCOMMAND_BEGIN, 
                idx, 
                idx?AT_KERNEL_BASE1:AT_KERNEL_BASE0);
}

static char *
change_bootargs(int idx)
{
    int idx = __at_rootfs->current;
    
    return __change_bootenv(
                "rootfs", 
                "bootargs", 
                "root=/dev/mmcblk0p1", 
                idx, 
                AT_ROOTFS_BASE + idx);
}

static void
change_bootenv()
{
    change_bootargs();
    change_bootcmd();
}

static void 
firmware_init(void) 
{
    at_vendor_t deft = AT_DEFT_VENDOR;
    
    __at_show_byprefix("vendor");

    if (false==os_objeq(&deft, __at_vendor)) {
        os_println("autelan env first init...");

        at_obj_deft(__at_env, AT_DEFT_ENV);
        
        __at_show_byprefix(NULL);
    }
}

static void
__kernel_select(bool try_buddy) 
{
    int current = __at_kernel->current;
    struct at_info_t *kernel = at_kernel(current);
    
    if (at_info_is_good(kernel)) {
        os_println("kernel%d is good", current);
        
        kernel->error++;
    }
    else if(try_buddy) {
        os_println("kernel%d is bad, try kernel%d" __crlf, current, !current);
        
        __at_kernel->current = !current;
        
        __kernel_select(false);
    }
    else {
        os_println("all kernel is bad, force use kernel%d" __crlf, current);
        
        kernel->error++;
    }
}

static void
kernel_select(void)
{
    __at_show_byprefix("kernel");

    __kernel_select(true);
}

static int
__rootfs_select(void)
{
    int array[AT_ROOTFS_SORT_COUNT] = {0};
    int i;
    
    at_rootfs_sort(__at_rootfs->current, array);

    for (i=AT_ROOTFS_SORT_COUNT-1; i>=0; i--) {
        int idx = array[i];

        if (idx && at_info_is_good(at_rootfs(idx))) {
            return idx;
        }
    }

    return -ENOEXIST;
}

static void
rootfs_select(void)
{
    int idx;
    int current = __at_rootfs->current;
    struct at_info_t *rootfs = at_rootfs(current);

    __at_show_byprefix("rootfs");
    
    if (at_info_is_good(rootfs)) {
        os_println("rootfs%d is good", current);
        
        rootfs->error++;
    }
    else if ((idx = __rootfs_select()) < 0) {
        os_println("all rootfs is bad, force use rootfs0" __crlf);

        __at_rootfs->current = 0;
    }
    else {
        os_println("rootfs%d is bad, try rootfs%d" __crlf, current, idx);
        
        __at_rootfs->current = idx;

        rootfs->error++;
    }
}

static void 
firmware_select() 
{
    kernel_select();
    rootfs_select();
    change_bootenv();
}

static void
firmware_save(void)
{
    env_crc_update();
    saveenv();
}

static void
at_select(void)
{
    at_control.env = &env_ptr->env;
    
    at_init();
    
    firmware_init();

    firmware_select();

    firmware_save();
}
/******************************************************************************/

/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#define _LINUX_LIST_H
#include <mmc.h>
#include <environment.h>

#ifdef CONFIG_BOOTARGS
#undef CONFIG_BOOTARGS
#endif

#define CONFIG_BOOTARGS         \
    "mem=2G"                    \
        " "                     \
    "console=ttyAMA0,115200"    \
        " "                     \
    "root=" AT_DEV(14)          \
        " "                     \
    "rootfstype=ext4"           \
        " "                     \
    "rootwait"                  \
        " "                     \
    "ro"                        \
        " "                     \
    "blkdevparts="              \
        "mmcblk0:"              \
        "512K(fastboot),"/*01 */\
        "512K(bootenv),"/* 02 */\
        "4M(baseparam),"/* 03 */\
        "4M(pqparam),"  /* 04 */\
        "3M(logo),"     /* 05 */\
                                \
        "16M(kernel0)," /* 06 */\
        "16M(kernel1)," /* 07 */\
        "16M(kernel2)," /* 08 */\
        "16M(kernel3)," /* 09 */\
        "16M(kernel4)," /* 10 */\
        "16M(kernel5)," /* 11 */\
        "16M(kernel6)," /* 12 */\
                                \
        "256M(rootfs0),"/* 13 */\
        "256M(rootfs1),"/* 14 */\
        "256M(rootfs2),"/* 15 */\
        "256M(rootfs3),"/* 16 */\
        "256M(rootfs4),"/* 17 */\
        "256M(rootfs5),"/* 18 */\
        "256M(rootfs6),"/* 19 */\
                                \
        "32M(config0)," /* 20 */\
        "32M(config1)," /* 21 */\
                                \
        "820M(data0),"  /* 22 */\
        "820M(data1),"  /* 23 */\
        "-(others)"             \
        " "                     \
    "mmz=ddr,0,0,300M"

#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif
/*
* kernel0 begin(block count) =  12M/512 =  24K = 0x06000
* kernel1 begin(block count) =  28M/512 =  56K = 0x0E000
* kernel2 begin(block count) =  44M/512 =  88K = 0x16000
* kernel3 begin(block count) =  60M/512 = 120K = 0x1E000
* kernel3 begin(block count) =  76M/512 = 152K = 0x26000
* kernel3 begin(block count) =  92M/512 = 184K = 0x2E000
* kernel3 begin(block count) = 108M/512 = 216K = 0x36000
*
* kernel  size (block count) = 16M/512 =  32K = 0x8000
*/
#define CONFIG_BOOTCOMMAND_BEGIN        "mmc read 0 0x1000000 "
#define CONFIG_BOOTCOMMAND_KERNEL0      "0x06000"
#define CONFIG_BOOTCOMMAND_KERNEL1      "0x0E000"
#define CONFIG_BOOTCOMMAND_KERNEL2      "0x16000"
#define CONFIG_BOOTCOMMAND_KERNEL3      "0x1E000"
#define CONFIG_BOOTCOMMAND_KERNEL4      "0x26000"
#define CONFIG_BOOTCOMMAND_KERNEL5      "0x2E000"
#define CONFIG_BOOTCOMMAND_KERNEL6      "0x36000"
#define CONFIG_BOOTCOMMAND_END          " 0x8000;bootm 0x1000000"
#define CONFIG_BOOTCOMMAND              \
            CONFIG_BOOTCOMMAND_BEGIN    \
            CONFIG_BOOTCOMMAND_KERNEL1  \
            CONFIG_BOOTCOMMAND_END      \
            /* end */

static at_ops_t __ops[]     = AT_DEFT_OPS;
static at_cache_t __cache[os_count_of(__ops)];
at_control_t at_control     = AT_CONTROL_DEFT(NULL, __ops, __cache);

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


int
__at_load(int idx /* atenv's block */)
{
    int offset  = AT_BLOCK_SIZE * idx;
    void *obj   = (char *)__at_env + offset;
    
    return read_emmc(AT_ENV_OFFSET + offset, obj, AT_BLOCK_SIZE);
}

int
__at_save(int idx /* atenv's block */)
{
    int offset  = AT_BLOCK_SIZE * idx;
    void *obj   = (char *)__at_env + offset;
    
    return write_emmc(AT_ENV_OFFSET + offset, obj, AT_BLOCK_SIZE);
}

static bool
__change_bootenv(char *name, char *find, char *replace)
{
    char *env, *key, *value;
    
    env = getenv(name);
    if (NULL==env) {
        return false;
    }
    
    key = strstr(env, find);
    if (NULL==key) {
        return false;
    }

    value = key + strlen(find);

    int len = strlen(replace);
    if (0==memcmp(value, replace, len)) {
        return false;
    }
    
    memcpy(value, replace, len);
    
    return true;
}

static bool
change_bootcmd(void)
{
    int idx = __at_kernel->current;
    
    char *array[AT_OS_COUNT] = {
        [0] = CONFIG_BOOTCOMMAND_KERNEL0,
        [1] = CONFIG_BOOTCOMMAND_KERNEL1,
        [2] = CONFIG_BOOTCOMMAND_KERNEL2,
        [3] = CONFIG_BOOTCOMMAND_KERNEL3,
        [4] = CONFIG_BOOTCOMMAND_KERNEL4,
        [5] = CONFIG_BOOTCOMMAND_KERNEL5,
        [6] = CONFIG_BOOTCOMMAND_KERNEL6,
    };
    
    return __change_bootenv(
                "bootcmd", 
                CONFIG_BOOTCOMMAND_BEGIN, 
                array[idx]);
}

static bool
change_bootargs(void)
{
    int idx = __at_rootfs->current;
    
    char *array[AT_OS_COUNT] = {
        [0] = "13",
        [1] = "14",
        [2] = "15",
        [3] = "16",
        [4] = "17",
        [5] = "18",
        [6] = "19",
    };

    return __change_bootenv(
                "bootargs", 
                "root=" __AT_DEV, 
                array[idx]);
}

static void
change_bootenv(void)
{
    change_bootargs();
    change_bootcmd();
}

static void 
at_boot_check(void) 
{
    __at_show_byprefix("vendor");

    if (false==is_at_cookie_deft()) {
        os_println("autelan env first init...");
        
        at_deft();
        
        __at_show_byprefix(NULL);
    }
}

static int
__at_select(at_firmware_t *firmware)
{
    int array[AT_OS_SORT_COUNT] = {0};
    int i;
    
    at_os_sort(firmware, array);

    for (i=AT_OS_SORT_COUNT-1; i>=0; i--) {
        int idx = array[i];

        if (idx && at_vcs_is_good(&firmware->vcs[idx])) {
            return idx;
        }
    }

    return -ENOEXIST;
}

static void
at_select(at_firmware_t *firmware, char *obj)
{
    int idx;
    int current = firmware->current;
    at_vcs_t *vcs = &firmware->vcs[current];

    at_show_byprefix("%s", obj);
    
    if (at_vcs_is_good(vcs)) {
        os_println("%s%d is good", obj, current);
        
        vcs->error++;
    }
    else if ((idx = __at_select(firmware)) < 0) {
        os_println("all %s is bad, force use rootfs0" __crlf, obj);

        firmware->current = 0;
    }
    else {
        os_println("%s%d is bad, try %s%d" __crlf, obj, current, obj, idx);
        
        firmware->current = idx;

        vcs->error++;
    }
}

static void 
at_boot_select(void) 
{
    at_select(__at_kernel, "kernel");
    at_select(__at_rootfs, "rootfs");
    
    change_bootenv();
}

static void
at_boot_save(void)
{
    env_crc_update();
    saveenv();
}

extern env_t *env_ptr;

/*
* call it in fastboot
*/
static void
at_boot(void)
{
    __at_control->env = (at_env_t *)(env_ptr->env);
    
    __at_init();
    
    at_boot_check();
    at_boot_select();
    at_boot_save();
}
/******************************************************************************/

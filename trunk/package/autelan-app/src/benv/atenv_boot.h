#ifndef __ATENV_BOOT_H_991A96C25F9225BC8AE258275E7F6713__
#define __ATENV_BOOT_H_991A96C25F9225BC8AE258275E7F6713__
/******************************************************************************/
#ifdef CONFIG_BOOTARGS
#undef CONFIG_BOOTARGS
#endif

#define CONFIG_BOOTARGS         \
    "mem=2G"                    \
        " "                     \
    "console=ttyAMA0,115200"    \
        " "                     \
    "root=/dev/mmcblk0p11"      \
        " "                     \
    "rootfstype=ext4"           \
        " "                     \
    "rootwait"                  \
        " "                     \
    "ro"                        \
        " "                     \
    "blkdevparts="              \
        "mmcblk0:"              \
        "512K(fastboot),"/* 01 */\
        "512K(bootenv),"/* 02 */\
        "4M(baseparam),"/* 03 */\
        "4M(pqparam),"  /* 04 */\
        "4M(logo),"     /* 05 */\
        "1M(small),"    /* 06 */\
        "2M(big),"      /* 07 */\
        "12M(kernel0)," /* 08 */\
        "12M(kernel1)," /* 09 */\
        "256M(rootfs0),"/* 10 */\
        "256M(rootfs1),"/* 11 */\
        "256M(rootfs2),"/* 12 */\
        "256M(rootfs3),"/* 13 */\
        "256M(rootfs4),"/* 14 */\
        "256M(rootfs5),"/* 15 */\
        "256M(rootfs6),"/* 16 */\
        "64M(config0)," /* 17 */\
        "64M(config1)," /* 18 */\
        "830M(data0),"  /* 19 */\
        "830M(data1),"  /* 20 */\
        "-(others)"             \
        " "                     \
    "mmz=ddr,0,0,300M"

#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif
/*
* kernel0 begin block = 16M/512 = 32K = 0x8000
* kernel1 begin block = 28M/512 = 56K = 0xE000
* kernel size(block count) = 12M/512 = 24K = 0x6000
*/
#define CONFIG_BOOTCOMMAND_BEGIN        "mmc read 0 0x1000000 0x"
#define CONFIG_BOOTCOMMAND_KERNEL0      "8000"
#define CONFIG_BOOTCOMMAND_KERNEL1      "E000"
#define CONFIG_BOOTCOMMAND_END          " 0x6000;bootm 0x1000000"
#define CONFIG_BOOTCOMMAND              \
            CONFIG_BOOTCOMMAND_BEGIN    \ 
            CONFIG_BOOTCOMMAND_KERNEL0  \
            CONFIG_BOOTCOMMAND_END      \
            /* end of CONFIG_BOOTCOMMAND */

#define AT_KERNEL_BASE0         '8'
#define AT_KERNEL_BASE1         'E'
#define AT_ROOTFS_BASE          '0'
/******************************************************************************/
#endif /* __ATENV_BOOT_H_991A96C25F9225BC8AE258275E7F6713__ */

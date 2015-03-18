/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#include "utils.h"
#include "utils/cmd.h"
#include "hi_unf_wdg.h"

#define ATDOG_COUNT     2

#if 0
HI_S32 HI_UNF_WDG_Init(HI_VOID);
HI_S32 HI_UNF_WDG_DeInit(HI_VOID);
HI_S32 HI_UNF_WDG_Enable(HI_U32 u32WdgNum);
HI_S32 HI_UNF_WDG_Disable(HI_U32 u32WdgNum);
HI_S32 HI_UNF_WDG_GetTimeout(HI_U32 u32WdgNum, HI_U32 *pu32Value);
HI_S32 HI_UNF_WDG_SetTimeout(HI_U32 u32WdgNum, HI_U32 u32Value);
HI_S32 HI_UNF_WDG_Clear(HI_U32 u32WdgNum);
HI_S32 HI_UNF_WDG_Reset(HI_U32 u32WdgNum);
#endif

#define method(_method, _dog, _args...)  ({ \
    int err;                                \
                                            \
    err = HI_UNF_WDG_##_method(_dog, ##_args); \
    if (err) {                              \
        debug_error("dog%d " #_method " error:%d", _dog, err); \
    }                                       \
                                            \
    err;                                    \
})

static int
enable(int dog)
{
    return method(Enable, dog);
}

static int
disable(int dog)
{
    return method(Disable, dog);
}

static int
clear(int dog)
{
    return method(Clear, dog);
}

static int
reset(int dog)
{
    return method(Reset, dog);
}

static int
get_timeout(int dog, int *timeout)
{
    return method(GetTimeout, dog, timeout);
}

static int
set_timeout(int dog, int timeout)
{
    return method(SetTimeout, dog, timeout);
}

#define foreach(_method, _args...)   ({ \
    int i, err;                         \
                                        \
    for (i=0; i<ATDOG_COUNT; i++) {     \
        err = _method(i, ##_args);      \
        if (err) {                      \
            return err;                 \
        }                               \
    }                                   \
                                        \
    0;                                  \
})

static int
cmd_enable(int argc, char *argv[])
{
    return foreach(enable);
}

static int
cmd_disable(int argc, char *argv[])
{
    return foreach(disable);
}

static int
cmd_clear(int argc, char *argv[])
{
    return foreach(clear);
}

static int
cmd_reset(int argc, char *argv[])
{
    return foreach(reset);
}

static int
cmd_get_timeout(int argc, char *argv[])
{
    int i, err, timeout[ATDOG_COUNT];

    for (i=0; i<ATDOG_COUNT; i++) {
        err = get_timeout(i, &timeout[i]);
        if (err) {
            return err;
        }
    }
    
    for (i=1; i<ATDOG_COUNT; i++) {
        if (timeout[i-1] != timeout[i]) {
            debug_error("dog%d's timeout(%d) != dog%d's timeout(%d)",
                i-1,
                timeout[i-1],
                i,
                timeout[i]);
            
            return -1;
        }
    }

    os_println("%d", timeout[0]);
    
    return 0;
}

static int
cmd_set_timeout(int argc, char *argv[])
{
    int timeout;
    char *end = NULL;
    
    timeout = os_strtol(argv[1], &end, 0);
    if (end || timeout <= 0) {
        os_println("bad timeout:%s", argv[1]);
        
        return -EFORMAT;
    } else {
        return foreach(set_timeout, timeout);
    }
}

static struct simpile_cmd dog[] = {
    {
        .argc = 1,
        .argv = {"enable"},
        cmd_enable,
    },
    {
        .argc = 1,
        .argv = {"disable"},
        cmd_disable,
    },
    {
        .argc = 1,
        .argv = {"clear"},
        cmd_clear,
    },
    {
        .argc = 1,
        .argv = {"reset"},
        cmd_reset,
    },
    {
        .argc = 1,
        .argv = {"timeout"},
        cmd_get_timeout,
    },
    {
        .argc = 2,
        .argv = {"timeout", NULL},
        cmd_set_timeout,
    },
};

static char *self;

static int
usage(void)
{
    os_fprintln(stderr, "%s enable", self);
    os_fprintln(stderr, "%s disable",self);
    os_fprintln(stderr, "%s clear",  self);
    os_fprintln(stderr, "%s reset",  self);
    os_fprintln(stderr, "%s timeout [timeout(ms)]", self);

    return -EFORMAT;
}

#ifndef __BUSYBOX__
#define atdog_main  main
#endif

/*
* dog have enabled when boot
*/
int atdog_main(int argc, char *argv[])
{
    self = argv[0];

    return simpile_cmd_handle(dog, argc-1, argv+1, usage);
}

#define __method(_method)           do{ \
    int err = HI_UNF_WDG_##_method();   \
    if (err) {                          \
        debug_error(#_method " error:%d", err); \
    }                                   \
}while(0)

static os_destructor void
__fini(void)
{
    __method(DeInit);
}

static os_constructor void
__init(void)
{
    __method(Init);
}
/******************************************************************************/
AKID_DEBUGER; /* must last os_constructor */


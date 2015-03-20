/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_NAME
#define __THIS_NAME     "atotp"
#endif

#ifndef __AKID_DEBUG
#define __AKID_DEBUG    __atotp_debug
#endif

#ifndef __THIS_FILE
#define __THIS_FILE     1
#endif

#include "atotp/atotp.h"

AKID_DEBUGER;

static char *self;

static int
usage(void)
{
    os_fprintln(stderr, "%s custom [show]", self);
    os_fprintln(stderr, "%s private [show]",self);

    return -EFORMAT;
}

static int
cmd_custom_show(int argc, char *argv[])
{
    return at_otp_custom_show();
}

static int
cmd_custom_check(int argc, char *argv[])
{
    return at_otp_custom_check();
}

static int
cmd_private_show(int argc, char *argv[])
{
    return at_otp_private_show();
}

static int
cmd_private_check(int argc, char *argv[])
{
    return at_otp_private_check();
}

static int
cmd_show(int argc, char *argv[])
{
    return at_otp_show();
}

static int
cmd_check(int argc, char *argv[])
{
    return at_otp_check();
}

static struct simpile_cmd otp[] = {
    {
        .argc = 1,
        .argv = {"show"},
        cmd_show,
    },
    {
        .argc = 1,
        .argv = {"check"},
        cmd_check,
    },
    {
        .argc = 2,
        .argv = {"custom", "show"},
        cmd_custom_check,
    },
    {
        .argc = 2,
        .argv = {"private", "check"},
        cmd_private_check,
    },
    {
        .argc = 2,
        .argv = {"custom", "show"},
        cmd_custom_check,
    },
    {
        .argc = 2,
        .argv = {"private", "check"},
        cmd_private_check,
    },
};

static int
__fini(void)
{
    appkey_fini();
}

static int
__init(void)
{
    appkey_init();

    return 0;
}

#ifndef __BUSYBOX__
#define atotp_main  main
#endif

/*
* otp have enabled when boot
*/
static int
__main(int argc, char *argv[])
{
    self = argv[0];

    return simpile_cmd_handle(otp, argc-1, argv+1, usage);
}

/*
* otp have enabled when boot
*/
int atotp_main(int argc, char *argv[])
{
    return os_call(__init, __fini, __main, argc, argv);
}
/******************************************************************************/

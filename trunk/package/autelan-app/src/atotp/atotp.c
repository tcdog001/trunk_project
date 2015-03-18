/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#include "atotp/atopt.h"

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


#ifndef __BUSYBOX__
#define atdog_main  main
#endif

/*
* otp have enabled when boot
*/
int atdog_main(int argc, char *argv[])
{
    self = argv[0];

    return simpile_cmd_handle(otp, argc-1, argv+1, usage);
}
/******************************************************************************/
AKID_DEBUGER; /* must last os_constructor */


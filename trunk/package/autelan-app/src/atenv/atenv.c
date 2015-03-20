/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_NAME
#define __THIS_NAME     "atenv"
#endif

#ifndef __AKID_DEBUG
#define __AKID_DEBUG    __atenv_debug
#endif

#ifndef __THIS_FILE
#define __THIS_FILE     1
#endif

#include "atenv/atenv.h"

#ifdef __BOOT__
#include "atenv_boot.c"
#elif defined(__APP__)
#include "atenv_app.c"
#endif
/******************************************************************************/

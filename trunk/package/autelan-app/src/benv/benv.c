/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#include "atenv.h"

#ifdef __BOOT__
#include "atenv_boot.h"
#include "benv_boot.c"
#elif defined(__APP__)
#include "benv_app.c"
#else
#error "no support"
#endif

/******************************************************************************/
AKID_DEBUGER; /* must last os_constructor */


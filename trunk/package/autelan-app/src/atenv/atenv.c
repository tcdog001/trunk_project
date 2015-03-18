/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#include "atenv.h"

#ifdef __BOOT__
#include "atenv_boot.c"
#elif defined(__BUSYBOX__) || defined(__APP__)
#include "atenv_app.c"
#endif
/******************************************************************************/

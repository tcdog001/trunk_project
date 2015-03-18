/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef ATENV_DEV
#define ATENV_DEV       "/dev/mmcblk0p2" /* boot env */
#endif

static at_env_t __env       = AT_DEFT_ENV;
static at_ops_t __ops[]     = AT_DEFT_OPS;
static at_cache_t __cache[os_count_of(__ops)];
at_control_t at_control     = AT_CONTROL_DEFT(&__env, __ops, __cache);

static int
load(void)
{
    if (1 != fread(__at_env, AT_ENV_SIZE, 1, __at_stream)) {
        return ferror(__at_stream);
    } else {
        return 0;
    }
}

int
__at_save(int idx /* atenv's block */)
{
    int offset  = AT_BLOCK_SIZE * idx;
    void *obj   = (char *)__at_env + offset;
    
    if (fseek(__at_stream, offset, 0)) {
        return ferror(__at_stream);
    } else if (1 != fwrite(obj, AT_BLOCK_SIZE, 1, __at_stream)) {
        return ferror(__at_stream);
    } else {
        return 0;
    }
}

static int
fini(void)
{
    at_save();
    
    if (__at_stream) {
        fclose(__at_stream);
    }

    return 0;
}

static int
init(void)
{
    __at_stream= fopen(ATENV_DEV, "r+");
    
    load();

    return 0;
}

#ifndef __BUSYBOX__
#define atenv_main  main
#endif

int atenv_main(int argc, char *argv[])
{
    return os_call(init, fini, at_main, argc, argv);
}
/******************************************************************************/
AKID_DEBUGER; /* must last os_constructor */

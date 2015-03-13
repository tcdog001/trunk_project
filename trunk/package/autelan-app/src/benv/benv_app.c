/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef BENV_DEV
#define BENV_DEV        "/dev/mmcblk0p2" /* boot env */
#endif

static at_env_t __env       = AT_DEFT_ENV;
static at_ops_t __ops[]     = AT_DEFT_OPS;
static at_ops_cache_t __ops_cache[os_count_of(__ops)];
at_control_t at_control     = AT_CONTROL_DEFT(&__env, __ops, __ops_cache);

static int
load(void)
{
    if (1 != fread(__env, AT_ENV_SIZE, 1, at_control.f)) {
        return ferror(at_control.f);
    } else {
        return 0;
    }
}

static int
__save(int idx/* block */)
{
    int offset  = AT_BLOCK_SIZE * idx;
    void *obj = (char *)__env + offset;
    
    if (fseek(at_control.f, offset, 0)) {
        return ferror(at_control.f);
    }
    else if (1 != fwrite(obj, AT_BLOCK_SIZE, 1, at_control.f)) {
        return ferror(at_control.f);
    } else {
        return 0;
    }
}

static int
save(void)
{
    int i;

    for (i=0; i<AT_BLOCK_COUNT; i++) {
        if (at_control.dirty[i]) {
            __save(i);
        }
    }
}

int main(int argc, char *argv[])
{
    int err;
    
    err = load();
    if (err) {
        return err;
    }
    
    err = at_main(argc, argv);
    if (err) {
        return err;
    }
    
    err = save();
    if (err) {
        return err;
    }

    return 0;
}

static os_destructor void
__fini(void)
{
    if (at_control.f) {
        fclose(at_control.f);
    }
}

static os_constructor void
__init(void)
{
    at_control.f = fopen(BENV_DEV, "r+");
}
/******************************************************************************/

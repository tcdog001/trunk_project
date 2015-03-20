#ifndef __THIS_NAME
#define __THIS_NAME     "appkey"
#endif

#ifndef __AKID_DEBUG
#define __AKID_DEBUG    __appkey_debug
#endif

#ifndef __THIS_FILE
#define __THIS_FILE     1
#endif

#include "utils.h"
#include "utils/cmd.h"

AKID_DEBUGER;

/* {"-load"} */
static int 
reload(int argc, char *argv[])
{
    appkey_reload();
    
    return 0;
}

#ifndef __BUSYBOX__
#define appkey_main  main
#endif

int appkey_main(int argc, char *argv[])
{
    struct command_item commands[] = {
        {
            .list = {"-reload"},
            .func = reload,
            .help = "load appkey",
        },
    };
    struct command_ctrl ctrl = COMMAND_CTRL_INITER(commands);

    appkey_init();
    
    os_sigaction_default();
    
    return os_do_command(argc, argv, &ctrl);
}

/******************************************************************************/ 
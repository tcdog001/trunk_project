#ifndef __ENV_H_AE12157DFFE5C6220F9C55AA8D427F7__
#define __ENV_H_AE12157DFFE5C6220F9C55AA8D427F7__
/******************************************************************************/
static inline bool
is_good_env(char *env)
{
    return env && env[0];
}

static inline char *
get_string_env(char *envname, const char *deft) 
{
    if (NULL==envname) {
        return os_assert_value(NULL);
    } else if (NULL==deft) {
        return os_assert_value(NULL);
    }
    
    char *env = getenv(envname);
    if (false==is_good_env(env)) {
        debug_trace("no-found env:%s", envname);
        
        return deft;
    } else {
        return env;
    }
}

static inline int
get_int_env(char *envname, int deft) 
{
    if (NULL==envname) {
        return -EINVAL9;
    }
    
    char *env = getenv(envname);
    if (false==is_good_env(env)) {
        debug_trace("no-found env:%s", envname);
        
        return deft;
    } else {
        return atoi(env);
    }
}

/******************************************************************************/
#endif /* __ENV_H_AE12157DFFE5C6220F9C55AA8D427F7__ */

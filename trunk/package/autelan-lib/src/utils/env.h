#ifndef __ENV_H_AE12157DFFE5C6220F9C55AA8D427F7__
#define __ENV_H_AE12157DFFE5C6220F9C55AA8D427F7__
/******************************************************************************/
static inline bool
is_good_env(char *env)
{
    return env && env[0];
}

static inline const char *
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
        debug_trace("get env:%s=%s", envname, env);
        
        return env;
    }
}

static inline int
copy_string_env(char *envname, const char *deft, char string[], int size) 
{
    const char *env = get_string_env(envname, deft);
    if (os_strlen(env) > size - 1) {
        return os_assert_value(-ETOOBIG);
    }
    
    os_strlcpy(string, env, size);
    
    return 0;
}

static inline int
get_int_env(char *envname, int deft) 
{
    if (NULL==envname) {
        return os_assert_value(-EINVAL9);
    }
    
    char *env = getenv(envname);
    if (false==is_good_env(env)) {
        debug_trace("no-found env:%s, use default:%d", envname, deft);
        
        return deft;
    } else {
        int value = os_atoi(env);

        debug_trace("get env:%s=%d", envname, value);

        return value;
    }
}

/******************************************************************************/
#endif /* __ENV_H_AE12157DFFE5C6220F9C55AA8D427F7__ */

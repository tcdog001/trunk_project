#ifndef __NUMBER_H_D1912A84AB1249D4A3313CDD1D2C0B44__
#define __NUMBER_H_D1912A84AB1249D4A3313CDD1D2C0B44__
/******************************************************************************/

static inline int
os_digitchar2int(int ch)
{
    /*
    * "0123456789"
    */
    if (ch>='0' && ch<='9') {
        return ch - '0';
    }
    /*
    * "abcdef"
    */
    else if (ch>='a' && ch<='f') {
        return ch - 'a' + 10;
    }
    /*
    * "ABCDEF"
    */
    else if (ch>='A' && ch<='F') {
        return ch - 'A' + 10;
    }
    else {
        return os_assert_value(ch - '0');
    }
}

#define os_digitstring2number(_digitstring, _len, _base, _type) ({ \
    _type n = 0;                                \
    char *str = (char *)_digitstring;           \
    int k;                                      \
                                                \
    for (k=0; k<_len; k++)  {                   \
        n *= (_type)_base;                      \
        n += (_type)os_digitchar2int(str[k]);   \
    }                                           \
                                                \
    n;                                          \
})

#ifdef __BOOT__
#define os_atoi(_string)    simple_strtol(_string, NULL, 0)
#define os_atol(_string)    simple_strtol(_string, NULL, 0)
#define os_atoll(_string)   simple_strtoull(_string, NULL, 0)
#elif defined(__APP__)
#define os_atoi(_string)    atoi(_string)
#define os_atol(_string)    atol(_string)
#define os_atoll(_string)   atoll(_string)
#else
#error "need to define atoi/atol/atoll"
#endif

#define os_isprint(_x)      ((_x)>=0x20 && (_x)<=0x7e)
/******************************************************************************/
#endif /* __NUMBER_H_D1912A84AB1249D4A3313CDD1D2C0B44__ */

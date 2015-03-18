#ifndef __AT_OTP_H_739039F132EB8833CE84495D183364C7__
#define __AT_OTP_H_739039F132EB8833CE84495D183364C7__
/******************************************************************************/
#include "utils.h"
#include "hi_unf_otp.h"

#define AT_OTP_SIZE         16
#define AT_OTP_PRIVATE      "lms.autelan.com\n"
#define AT_OTP_CUSTOM       "www.autelan.com\n"
#define AT_OTP_HASH         "\xE3\xE4\xEA\x5C\xC2\xEA\xE8\xCA\xD8\xC2\xDC\x5C\xC6\xDE\xDA\x14"
#if 0
77 77 77 2E 61 75 74 65 6C 61 6E 2E 63 6F 6D 0A lms.autelan.com
6C 6D 73 2E 61 75 74 65 6C 61 6E 2E 63 6F 6D 0A www.autelan.com
E3 E4 EA 5C C2 EA E8 CA D8 C2 DC 5C C6 DE DA 14 

#endif
#define AT_OTP_ZERO         "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
#define AT_OTP_TRY          3

#define AT_OTP_CUSTOM_METHOD    CustomerKey
#define AT_OPT_PRIVATE_METHOD   StbPrivData

#define AT_OTP_ERR_HACKED   1
#define AT_OTP_ERR_INIT     2
#define AT_OTP_ERR_FINI     3
#define AT_OTP_ERR_READ     4
#define AT_OTP_ERR_WRITE    5
#define AT_OTP_ERR_TRY      6
#define AT_OTP_ERR_UNKNOW   7

#if 0
HI_S32 HI_UNF_OTP_Init(HI_VOID);
HI_S32 HI_UNF_OTP_DeInit(HI_VOID);
HI_S32 HI_UNF_OTP_SetCustomerKey(HI_U8 *pKey, HI_U32 u32KeyLen);
HI_S32 HI_UNF_OTP_GetCustomerKey(HI_U8 *pKey, HI_U32 u32KeyLen);
HI_S32 HI_UNF_OTP_SetStbPrivData(HI_U32 u32Offset, HI_U8 u8Data);
HI_S32 HI_UNF_OTP_GetStbPrivData(HI_U32 u32Offset, HI_U8 *pu8Data);
#endif

static inline void
__otp_dump(unsigned char otp[AT_OTP_SIZE])
{
    printf(
        "%.2X" "%.2X" "%.2X" "%.2X"
        "%.2X" "%.2X" "%.2X" "%.2X"
        "%.2X" "%.2X" "%.2X" "%.2X"
        "%.2X" "%.2X" "%.2X" "%.2X"
        "\n",
        otp[0],  otp[1],  otp[2],  otp[3],
        otp[4],  otp[5],  otp[6],  otp[7],
        otp[8],  otp[9],  otp[10], otp[11],
        otp[12], otp[13], otp[14], otp[15]);
}

static inline int
__otp_init(void)
{
    int err;
    
    err = HI_UNF_OTP_Init();
    if (err) {
        debug_error("otp init error:%d", err);
        
        return AT_OTP_ERR_INIT;
    }

    return 0;
}

static inline int
__otp_fini(void)
{
    int err;
    
    err = HI_UNF_OTP_DeInit();
    if (err) {
        debug_error("otp fini error:%d", err);
        
        return AT_OTP_ERR_FINI;
    }

    return 0;
}

#define __otp_do(_obj, _action, _err, _otp)         ({  \
    int err;                                            \
                                                        \
    err = HI_UNF_OTP_##_action##_obj(otp, AT_OTP_SIZE); \
    if (err) {                                          \
        debug_error(#_action " " #_obj " error:%d", err); \
    }                                                   \
                                                        \
    err;                                                \
})  /* end */

#define __otp_show(_obj, _otp)  ({  \
    int err;                        \
                                    \
    err = __otp_do(_obj, Get,       \
        AT_OTP_ERR_READ, _otp);     \
    if (0==err) {                   \
        __otp_dump(_otp);           \
    }                               \
                                    \
    err;                            \
})  /* end */


#define __otp_check(_obj, _otp, _deft)  ({  \
    __label__ try_again;                    \
    __label__ error;                        \
    int err, try = 0;                       \
                                            \
try_again:                                  \
    err = __otp_do(_obj, Get,               \
        AT_OTP_ERR_READ, _otp);             \
    if (err) {                              \
        goto error;                         \
    }                                       \
                                            \
    if (os_arrayeq(otp, AT_OTP_ZERO)) {     \
        err = __otp_do(_obj, Set,           \
            AT_OTP_ERR_WRITE, _deft);       \
        if (err) {                          \
            goto error;                     \
        }                                   \
                                            \
        if (try++ < AT_OTP_TRY) {           \
            goto try_again;                 \
        } else {                            \
            err = AT_OTP_ERR_TRY;           \
        }                                   \
    } else if (os_arrayeq(otp, _deft)) {    \
        err = 0;                            \
    } else {                                \
        err = AT_OTP_ERR_HACKED;            \
    }                                       \
                                            \
error:                                      \
    err;                                    \
})  /* end */

static inline int
__at_otp_custom_show(unsigned char otp[AT_OTP_SIZE])
{
    return __otp_show(AT_OTP_CUSTOM_METHOD, otp);
}

static inline int
__at_otp_custom_check(unsigned char otp[AT_OTP_SIZE])
{
    return __otp_check(AT_OTP_CUSTOM_METHOD, otp, AT_OTP_CUSTOM);
}

static inline int
__at_otp_private_show(unsigned char otp[AT_OTP_SIZE])
{
    return __otp_show(AT_OPT_PRIVATE_METHOD, otp);
}

static inline int
__at_otp_private_check(unsigned char otp[AT_OTP_SIZE])
{
    return __otp_check(AT_OPT_PRIVATE_METHOD, otp, AT_OTP_PRIVATE);
}


static inline int
__at_otp_show(unsigned char otp[AT_OTP_SIZE])
{
    int err = 0;
    
    printf("custom=");  
    err = __at_otp_custom_show(otp);
    if (err) {
        printf("\n");

        return err;
    }
    
    printf("private=");
    err = __at_otp_private_show(otp);
    if (err) {
        printf("\n");

        return err;
    }

    return 0;
}

static inline int
__at_otp_check(unsigned char otp[AT_OTP_SIZE])
{
    int err = 0;

    err = __at_otp_custom_check(otp);
    if (err) {
        return err;
    }
    
    err = __at_otp_private_check(otp);
    if (err) {
        return err;
    }
    
    return err;
}

#define __at_otp_call(_call, _otp) \
    os_call(__otp_init, __otp_fini, _call, _otp)

static inline int
at_otp_custom_show(void)
{
    unsigned char otp[AT_OTP_SIZE] = {0};
    
    return __at_otp_call(__at_otp_custom_show, otp);
}

static inline int
at_otp_custom_check(void)
{
    unsigned char otp[AT_OTP_SIZE] = {0};
    
    return __at_otp_call(__at_otp_custom_check, otp);
}

static inline int
at_otp_private_show(void)
{
    unsigned char otp[AT_OTP_SIZE] = {0};
    
    return __at_otp_call(__at_otp_private_show, otp);
}

static inline int
at_otp_private_check(void)
{
    unsigned char otp[AT_OTP_SIZE] = {0};
    
    return __at_otp_call(__at_otp_private_check, otp);
}

static inline int
at_otp_show(void)
{
    unsigned char otp[AT_OTP_SIZE] = {0};
    
    return __at_otp_call(__at_otp_show, otp);
}

static inline int
at_otp_check(void)
{
    unsigned char otp[AT_OTP_SIZE] = {0};
    
    return __at_otp_call(__at_otp_check, otp);
}
/******************************************************************************/
#endif /* __AT_OTP_H_739039F132EB8833CE84495D183364C7__ */

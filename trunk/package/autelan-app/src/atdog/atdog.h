#ifndef __AT_DOG_H_DE28C2611313B5650AF7DB2BFB5828D4__
#define __AT_DOG_H_DE28C2611313B5650AF7DB2BFB5828D4__
/******************************************************************************/
/*
* just for fastboot !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
#if defined(__FASTBOOT__)
#define io_read( _addr)             (*(volatile unsigned int *)(_addr))
#define io_write(_addr, _val)       (*(volatile unsigned int *)(_addr)=(_val))

#define AT_DOG0         0xF8A2C000
#define AT_DOG1         0xF8A2D000
#define AT_DOG(_dog)    AT_DOG##_dog

#define AT_DOG_LOAD     0x0000
#define AT_DOG_VALUE    0x0004
#define AT_DOG_CONTROL  0x0008
#define AT_DOG_INTCLR   0x000C
#define AT_DOG_RIS      0x0010
#define AT_DOG_MIS      0x0014
#define AT_DOG_LOCK     0x0C00

#define dog_reg(_dog, _reg)         (AT_DOG(_dog) + (_reg))

#define AT_DOG_LOAD_DEFAULT         0x07270E00
#define AT_DOG_LOCK_OPEN            0x1ACCE551
#define AT_DOG_LOCK_CLOSE           0
#define AT_DOG_CONTROL_ENABLE       0x3
#define AT_DOG_CONTROL_DISABLE      0

#define dog_read( _dog, _reg)       io_read(dog_reg( _dog, _reg))
#define dog_wirte(_dog, _reg, _val) io_write(dog_reg(_dog, _reg), _val)

#define AT_DOG_ENABLE(_dog) do{ \
    dog_wirte(_dog, AT_DOG_LOCK,    AT_DOG_LOCK_OPEN);      \
    dog_wirte(_dog, AT_DOG_LOAD,    AT_DOG_LOAD_DEFAULT);   \
    dog_wirte(_dog, AT_DOG_CONTROL, AT_DOG_CONTROL_ENABLE); \
    dog_wirte(_dog, AT_DOG_LOCK,    AT_DOG_LOCK_CLOSE);     \
}while(0)

#define AT_DOG_DISABLE(_dog) do{ \
    dog_wirte(AT_DOG(_dog) + AT_DOG_LOCK,    AT_DOG_LOCK_OPEN);         \
    dog_wirte(AT_DOG(_dog) + AT_DOG_CONTROL, AT_DOG_CONTROL_DISABLE);   \
    dog_wirte(AT_DOG(_dog) + AT_DOG_LOCK,    AT_DOG_LOCK_CLOSE);        \
}while(0)

#if 1
#define at_dog_reg_dump(_dog, _reg) \
    printf("%d.%.15s = 0x%x\n", _dog, #_reg, dog_read(_dog, _reg))
    
#define at_dog_dump(_dog, _msg)         do{ \
    printf("%d.%s\n", _dog, _msg);          \
    at_dog_reg_dump(_dog, AT_DOG_LOAD);     \
    at_dog_reg_dump(_dog, AT_DOG_VALUE);    \
    at_dog_reg_dump(_dog, AT_DOG_CONTROL);  \
    at_dog_reg_dump(_dog, AT_DOG_RIS);      \
    at_dog_reg_dump(_dog, AT_DOG_MIS);      \
    at_dog_reg_dump(_dog, AT_DOG_LOCK);     \
}while(0)
#else
#define at_dog_dump(_dog, msg) do{}while(0)
#endif

#define at_dog_start(_dog) do{ \
    at_dog_dump(_dog, "before watchdog enable");\
    AT_DOG_ENABLE(_dog);                        \
    at_dog_dump(_dog, "after watchdog enable"); \
}while(0)
/******************************************************************************/
#define AT_OTP              0xF8AB0000
#define AT_OTP_PRIVATER     (AT_OTP + 0x2B0)
#define AT_OTP_CUSTOMER     (AT_OTP + 0x2C0)
#endif
/******************************************************************************/
#endif /* __AT_DOG_H_DE28C2611313B5650AF7DB2BFB5828D4__ */

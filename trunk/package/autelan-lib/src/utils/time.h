#ifndef __TIME_H_1E5C2E2238BA2157D2C85F1285F8A579__
#define __TIME_H_1E5C2E2238BA2157D2C85F1285F8A579__
/******************************************************************************/
static inline int
time_sec(int ms)
{
    return ms/1000;
}

static inline int
time_usec(int ms)
{
    return (ms * 1000) % 1000;
}

static inline int
time_nsec(int ms)
{
    return (ms * 1000 * 1000) % 1000;
}
/******************************************************************************/
#endif /* __TIME_H_1E5C2E2238BA2157D2C85F1285F8A579__ */

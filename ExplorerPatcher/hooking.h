#ifndef _H_HOOKING_H_
#define _H_HOOKING_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <KNSoft/SlimDetours/SlimDetours.h>

typedef struct funchook funchook_t;

inline funchook_t* funchook_create(void)
{
    return (funchook_t*)1;
}

inline int funchook_uninstall(
    funchook_t* _this,
    int flags
)
{
    return 0;
}

inline int funchook_destroy(funchook_t* _this)
{
    return 0;
}

inline int funchook_prepare(
    funchook_t* funchook,
    void** target_func,
    void* hook_func
)
{
    HRESULT hr = SlimDetoursInlineHook(TRUE, target_func, hook_func);
    return SUCCEEDED(hr) ? 0 : hr;
}

inline int funchook_install(
    funchook_t* funchook,
    int flags
)
{
    return 0;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif

#define HOOKING_SUCCESS 0

#ifdef __cplusplus
extern "C"
{
#endif

DECLSPEC_SELECTANY funchook_t* funchook = NULL;

#ifdef __cplusplus
} // extern "C"
#endif

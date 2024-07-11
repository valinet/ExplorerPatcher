#ifndef _H_HOOKING_H_
#define _H_HOOKING_H_

#define STRAT_REPLACE_ANY_TYPE_OF_JUMP_WITH_NOP 0
#define STRAT_REPLACE_ANY_TYPE_OF_JUMP_WITH_ALWAYS_JUMP 1
#define HOOK_WITH_FUNCHOOK 0
#define HOOK_WITH_DETOURS 1
#define HOW_TO_HOOK HOOK_WITH_FUNCHOOK

#if HOW_TO_HOOK == HOOK_WITH_FUNCHOOK

#include <funchook.h>
#pragma comment(lib, "funchook.lib")
#pragma comment(lib, "Psapi.lib") // required by funchook
#if defined(_M_IX86) || defined(_M_X64)
#pragma comment(lib, "distorm.lib")
#else
#pragma comment(lib, "capstone.lib")
#endif

#elif HOW_TO_HOOK == HOOK_WITH_DETOURS

#ifdef __cplusplus
extern "C"
{
#endif

#include <detours.h>
#pragma comment(lib, "detours.lib")

#ifdef __cplusplus
inline
#endif
void* funchook_create(void)
{
    return 1;
}

#ifdef __cplusplus
inline
#endif
int funchook_uninstall(
    void* _this,
    int flags
)
{
    return 0;
}

#ifdef __cplusplus
inline
#endif
int funchook_destroy(void* _this)
{
    return 0;
}

#ifdef __cplusplus
inline
#endif
int funchook_prepare(
    void* funchook,
    void** target_func,
    void* hook_func
)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(target_func, hook_func);
    return DetourTransactionCommit();
}

#ifdef __cplusplus
inline
#endif
int funchook_install(
    void* funchook,
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

#if HOW_TO_HOOK == HOOK_WITH_FUNCHOOK
#ifdef __cplusplus
inline
#endif
funchook_t* funchook;
#elif HOW_TO_HOOK == HOOK_WITH_DETOURS
#ifdef __cplusplus
inline
#endif
void* funchook;
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif
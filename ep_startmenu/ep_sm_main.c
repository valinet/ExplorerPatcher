#include <Windows.h>
#include <valinet/hooking/iatpatch.h>
#include "ep_sm_forwards.h"
#pragma comment(lib, "Dbghelp.lib")

HMODULE hModule = NULL;
HMODULE hOrig = NULL;
wchar_t* (*pGetCmdArguments)(int*) = NULL;
SRWLOCK lockInstanced = { .Ptr = SRWLOCK_INIT };
BOOL bInstanced = FALSE;

BOOL start_GetProductInfo(DWORD dwOSMajorVersion, DWORD dwOSMinorVersion, DWORD dwSpMajorVersion, DWORD dwSpMinorVersion, PDWORD pdwReturnedProductType)
{
    *pdwReturnedProductType = 119;
    return TRUE;
}

void Init()
{
    DWORD dwStartShowClassicMode = 0, dwSize = sizeof(DWORD);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", L"Start_ShowClassicMode", RRF_RT_DWORD, NULL, &dwStartShowClassicMode, &dwSize);
    if (dwStartShowClassicMode)
    {
        VnPatchIAT(GetModuleHandleW(NULL), "api-ms-win-core-sysinfo-l1-2-0.dll", "GetProductInfo", start_GetProductInfo);
    }
    HMODULE hMod;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, hModule, &hMod);
    bInstanced = TRUE;
}

#pragma comment(linker, "/export:?GetCmdArguments@Details@Platform@@YAPEAPEA_WPEAH@Z=GetCmdArguments,@130")
wchar_t* GetCmdArguments(int* a1)
{
    AcquireSRWLockExclusive(&lockInstanced);
    if (!hOrig)
    {
        hOrig = LoadLibraryW(L"wincorlib_orig.dll");
        if (hOrig)
        {
            pGetCmdArguments = GetProcAddress(hOrig, "?GetCmdArguments@Details@Platform@@YAPEAPEA_WPEAH@Z");
        }
    }
    if (pGetCmdArguments)
    {
        if (!bInstanced) Init();
        ReleaseSRWLockExclusive(&lockInstanced);
        return pGetCmdArguments(a1);
    }
    ReleaseSRWLockExclusive(&lockInstanced);
    return NULL;
}

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        hModule = hinstDLL;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

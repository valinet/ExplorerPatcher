#include <Windows.h>
#include <initguid.h>
// #include <valinet/hooking/iatpatch.h>
#include <valinet/utility/memmem.h>
#include "../ExplorerPatcher/utility.h"
#include "ep_sm_forwards.h"
#pragma comment(lib, "Dbghelp.lib")

HMODULE hModule = NULL;
HMODULE hOrig = NULL;
SRWLOCK lockInstanced = { .Ptr = SRWLOCK_INIT };
BOOL bInstanced = FALSE;
BOOL g_bIsUsingOwnJumpViewUI = FALSE;
BOOL g_bIsUsingOwnStartUI = FALSE;

DEFINE_GUID(IID_StartDocked_App, 0x4C2CAEAD, 0x9DA8, 0x30EC, 0xB6, 0xD3, 0xCB, 0xD5, 0x74, 0xED, 0xCB, 0x35); // 4C2CAEAD-9DA8-30EC-B6D3-CBD574EDCB35
DEFINE_GUID(IID_StartUI_App, 0x1ECDC9E0, 0xBDB1, 0x3551, 0x8C, 0xEE, 0x4B, 0x77, 0x54, 0x0C, 0x44, 0xB3); // 1ECDC9E0-BDB1-3551-8CEE-4B77540C44B3
DEFINE_GUID(IID_StartDocked_XamlMetaDataProvider, 0xD5783E97, 0x0462, 0x3A6B, 0xAA, 0x60, 0x50, 0x0D, 0xB1, 0x1D, 0x3E, 0xF6); // D5783E97-0462-3A6B-AA60-500DB11D3EF6
DEFINE_GUID(IID_StartUI_XamlMetaDataProvider, 0xF2777C41, 0xD2CC, 0x34B6, 0xA7, 0xEA, 0x19, 0xF6, 0xC6, 0x5F, 0x0C, 0x19); // F2777C41-D2CC-34B6-A7EA-19F6C65F0C19

/*BOOL start_GetProductInfo(DWORD dwOSMajorVersion, DWORD dwOSMinorVersion, DWORD dwSpMajorVersion, DWORD dwSpMinorVersion, PDWORD pdwReturnedProductType)
{
    *pdwReturnedProductType = 119;
    return TRUE;
}*/

BOOL GetStartUIName(WCHAR* out, int cch)
{
    if (out && cch)
        out[0] = 0;

    WCHAR szPath[MAX_PATH];
    wcscpy_s(szPath, MAX_PATH, L"StartUI.dll");
    if (FileExistsW(szPath))
    {
        if (out && cch)
            wcscpy_s(out, cch, szPath);
        return TRUE;
    }

    wcscpy_s(szPath, MAX_PATH, L"StartUI_.dll");
    if (FileExistsW(szPath))
    {
        g_bIsUsingOwnStartUI = TRUE;
        if (out && cch)
            wcscpy_s(out, cch, szPath);
        return TRUE;
    }

    return FALSE;
}

WCHAR g_szStartUIName[MAX_PATH];

BOOL GetStartShowClassicMode()
{
    DWORD dwStartShowClassicMode = 0;
    DWORD dwSize = sizeof(DWORD);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", L"Start_ShowClassicMode", RRF_RT_DWORD, NULL, &dwStartShowClassicMode, &dwSize);
    if (dwStartShowClassicMode == 0)
        return FALSE;

    if (!GetStartUIName(g_szStartUIName, ARRAYSIZE(g_szStartUIName)))
        return FALSE;

    return TRUE;
}

void PatchXamlMetaDataProviderGuid()
{
    static BOOL bPatched = FALSE;
    if (bPatched)
    {
        return;
    }
    bPatched = TRUE;

    PBYTE beginRData = NULL;
    DWORD sizeRData = 0;
    RDataSectionBeginAndSize(GetModuleHandleW(NULL), &beginRData, &sizeRData);
    if (!beginRData || !sizeRData)
    {
        return;
    }

    GUID* pguidTarget = memmem(beginRData, sizeRData, (void*)&IID_StartDocked_XamlMetaDataProvider, sizeof(GUID));
    if (!pguidTarget)
    {
        return;
    }

    DWORD dwOldProtect = 0;
    if (VirtualProtect(pguidTarget, sizeof(GUID), PAGE_EXECUTE_READWRITE, &dwOldProtect))
    {
        *pguidTarget = IID_StartUI_XamlMetaDataProvider;
        VirtualProtect(pguidTarget, sizeof(GUID), dwOldProtect, &dwOldProtect);
    }
}

void Init()
{
    if (GetStartShowClassicMode())
    {
        // VnPatchIAT(GetModuleHandleW(NULL), "api-ms-win-core-sysinfo-l1-2-0.dll", "GetProductInfo", start_GetProductInfo);
        PatchXamlMetaDataProviderGuid();
        if (g_bIsUsingOwnStartUI)
        {
            LoadLibraryW(g_szStartUIName);
        }
        if (FileExistsW(L"JumpViewUI_.dll"))
        {
            LoadLibraryW(L"JumpViewUI_.dll");
            g_bIsUsingOwnJumpViewUI = TRUE;
        }

        PBYTE beginText = NULL;
        DWORD sizeText = 0;
        TextSectionBeginAndSize(GetModuleHandleW(NULL), &beginText, &sizeText);
        if (beginText && sizeText)
        {
            // Fix 0x800704DA (The service is already registered) exception when feature flag 58205615 is enabled
            // Feature flag introduced in:
            // - Germanium Client 26100.5742+
            // - Germanium Server 26461+
            // - Bromine Canary 27924+ (reworked in 27938)
            // Used to be inlined in StartMenuExperienceHost::App::OnLaunched(), the rework made it be called using
            // std::call_once, therefore we have a function that we can make it do nothing.

            // StartMenuExperienceHost::App::SetExperienceManagerPropertiesAsync()
            // Early return that function
#if defined(_M_X64)
            // TODO Improve pattern
            // 40 53 57 48 83 EC 28 E8 ?? ?? ?? ?? 48 8B D8 48 89 44 24 40 48 8B C8
            PBYTE match = FindPattern(
                beginText,
                sizeText,
                "\x40\x53\x57\x48\x83\xEC\x28\xE8\x00\x00\x00\x00\x48\x8B\xD8\x48\x89\x44\x24\x40\x48\x8B\xC8",
                "xxxxxxxx????xxxxxxxxxxx"
            );
            if (match)
            {
                DWORD dwOldProtect = 0;
                if (VirtualProtect(match, 1, PAGE_EXECUTE_READWRITE, &dwOldProtect))
                {
                    match[0] = 0xC3; // ret
                    VirtualProtect(match, 1, dwOldProtect, &dwOldProtect);
                }
            }
#elif defined(_M_ARM64)
            // TODO Improve pattern
            // 7F 23 03 D5 F3 53 BF A9 FD 7B BC A9 FD 03 00 91 30 00 80 92
            // ----------- PACIBSP, don't scan for this because it's everywhere
            PBYTE match = FindPattern(
                beginText,
                sizeText,
                "\xF3\x53\xBF\xA9\xFD\x7B\xBC\xA9\xFD\x03\x00\x91\x30\x00\x80\x92",
                "xxxxxxxxxxxxxxxx"
            );
            if (match)
            {
                match -= 4; // include PACIBSP
                DWORD dwOldProtect = 0;
                if (VirtualProtect(match, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect))
                {
                    *(DWORD*)match = 0xD65F03C0; // RET
                    VirtualProtect(match, 4, dwOldProtect, &dwOldProtect);
                }
            }
#endif
        }
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
    }
    static wchar_t* (*pGetCmdArguments)(int*) = NULL;
    if (!pGetCmdArguments && hOrig)
    {
        pGetCmdArguments = GetProcAddress(hOrig, "?GetCmdArguments@Details@Platform@@YAPEAPEA_WPEAH@Z");
    }
    if (!pGetCmdArguments)
    {
        ReleaseSRWLockExclusive(&lockInstanced);
        return NULL;
    }

    if (!bInstanced) Init();
    ReleaseSRWLockExclusive(&lockInstanced);
    return pGetCmdArguments(a1);
}

extern HRESULT LoadOurShellCommonPri();
extern HRESULT GetActivationFactoryByPCWSTR_InStartUI(PCWSTR activatableClassId, REFIID riid, void** ppv);
extern HRESULT GetActivationFactoryByPCWSTR_InJumpViewUI(PCWSTR activatableClassId, REFIID riid, void** ppv);

#pragma comment(linker, "/export:?GetActivationFactoryByPCWSTR@@YAJPEAXAEAVGuid@Platform@@PEAPEAX@Z=GetActivationFactoryByPCWSTR,@129")
HRESULT GetActivationFactoryByPCWSTR(PCWSTR activatableClassId, REFIID riid, void** ppv)
{
    if (!hOrig)
    {
        hOrig = LoadLibraryW(L"wincorlib_orig.dll");
    }
    static HRESULT (*pGetActivationFactoryByPCWSTR)(PCWSTR, REFIID, void**) = NULL;
    if (!pGetActivationFactoryByPCWSTR && hOrig)
    {
        pGetActivationFactoryByPCWSTR = GetProcAddress(hOrig, "?GetActivationFactoryByPCWSTR@@YAJPEAXAEAVGuid@Platform@@PEAPEAX@Z");
    }
    if (!pGetActivationFactoryByPCWSTR)
    {
        return E_FAIL;
    }

    if (!wcscmp(activatableClassId, L"StartDocked.App") && IsEqualGUID(riid, &IID_StartDocked_App))
    {
        if (GetStartShowClassicMode())
        {
            LoadOurShellCommonPri();
            return GetActivationFactoryByPCWSTR_InStartUI(L"StartUI.App", &IID_StartUI_App, ppv);
        }
    }
    else if (!wcscmp(activatableClassId, L"StartDocked.startdocked_XamlTypeInfo.XamlMetaDataProvider"))
    {
        if (GetStartShowClassicMode())
        {
            return GetActivationFactoryByPCWSTR_InStartUI(L"StartUI.startui_XamlTypeInfo.XamlMetaDataProvider", riid, ppv);
        }
    }
    else if (wcsncmp(activatableClassId, L"StartUI.", 8) == 0)
    {
        return GetActivationFactoryByPCWSTR_InStartUI(activatableClassId, riid, ppv);
    }
    else if (wcsncmp(activatableClassId, L"JumpViewUI.", 11) == 0)
    {
        if (g_bIsUsingOwnJumpViewUI)
        {
            return GetActivationFactoryByPCWSTR_InJumpViewUI(activatableClassId, riid, ppv);
        }
    }

    return pGetActivationFactoryByPCWSTR(activatableClassId, riid, ppv);
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

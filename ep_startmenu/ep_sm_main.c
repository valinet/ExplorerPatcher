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

    // Our target is in .rdata
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)GetModuleHandleW(NULL);
    if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE)
    {
        PIMAGE_NT_HEADERS64 ntHeader = (PIMAGE_NT_HEADERS64)((u_char*)dosHeader + dosHeader->e_lfanew);
        if (ntHeader->Signature == IMAGE_NT_SIGNATURE)
        {
			PIMAGE_SECTION_HEADER firstSection = IMAGE_FIRST_SECTION(ntHeader);
            for (unsigned int i = 0; i < ntHeader->FileHeader.NumberOfSections; ++i)
            {
                PIMAGE_SECTION_HEADER section = firstSection + i;
                if (!strncmp(section->Name, ".rdata", 6))
                {
                    beginRData = (PBYTE)dosHeader + section->VirtualAddress;
                    sizeRData = section->SizeOfRawData;
                    break;
                }
            }
        }
    }
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

    if (wcsncmp(activatableClassId, L"StartUI.", 8) == 0)
    {
        return GetActivationFactoryByPCWSTR_InStartUI(activatableClassId, riid, ppv);
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

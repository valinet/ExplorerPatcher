#include <initguid.h>
#include <Windows.h>
#include "../libs/libvalinet/valinet/hooking/iatpatch.h"
#include "../libs/sws/SimpleWindowSwitcher/sws_WindowHelpers.h"
#pragma comment(lib, "Uxtheme.lib")

HMODULE hModule = NULL;
HMODULE hAltTab = NULL;
IOleCommandTarget* pAltTabSSO = NULL;

DEFINE_GUID(CLSID_AltTabSSO,
    0xA1607060, 0x5D4C, 0x467A, 0xB7, 0x11, 0x2B, 0x59, 0xA6, 0xF2, 0x59, 0x57);

HRESULT AltTab_DwmpActivateLivePreview(int s, HWND hWnd, int c, int d) {
    return S_OK;
}

int AltTab_LoadStringW(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int cchBufferMax) {
    if (uID == 0x3E8) {
        swprintf_s(lpBuffer, cchBufferMax, L"AltTab"); return 6;
    }
    else if (uID == 0x3EA) {
        if (cchBufferMax < MAX_PATH) return 0;
        sws_WindowHelpers_GetDesktopText(lpBuffer);
        int len = wcslen(lpBuffer);
        for (int i = 0; i < len; ++i) if (lpBuffer[i] == L'&') lpBuffer[i] = L'\u200E';
        return len;
    }
    return LoadStringW(hInstance, uID, lpBuffer, cchBufferMax);
}

HTHEME AltTab_OpenThemeData(HWND hwnd, LPCWSTR pszClassList) {
    if (!wcscmp(pszClassList, L"AltTab")) return OpenThemeData(hwnd, L"WINDOW");
    return OpenThemeData(hwnd, pszClassList);
}

HRESULT AltTab_DrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect, const DTTOPTS* pOptions) {
    HRESULT hr = S_OK;
    HTHEME hTheme2 = OpenThemeData(NULL, L"TEXTSTYLE");
    if (hTheme2) hr = DrawThemeTextEx(hTheme2, hdc, iPartId + 1, iStateId, pszText, cchText, dwTextFlags, pRect, pOptions);
    if (hTheme2) CloseThemeData(hTheme2);
    return hr;
}

BOOL AltTab_IsWindowEnabled(HWND hWnd) {
    if (!IsWindowEnabled(hWnd)) return FALSE;
    BOOL isCloaked;
    DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &isCloaked, sizeof(BOOL));
    if (isCloaked) return FALSE;
    if (sws_IsShellFrameWindow(hWnd) && !_sws_GhostWindowFromHungWindow(hWnd)) return TRUE;
    if (_sws_IsShellManagedWindow(hWnd) && !sws_WindowHelpers_ShouldTreatShellManagedWindowAsNotShellManaged(hWnd)) return FALSE;
    if (sws_WindowHelpers_IsWindowShellManagedByExplorerPatcher(hWnd)) return FALSE;
    return TRUE;
}

HRESULT AltTab_DwmExtendFrameIntoClientArea(HWND hWnd, const MARGINS* pMarInset) {
    HRESULT hr = DwmExtendFrameIntoClientArea(hWnd, pMarInset);
    sws_WindowHelpers_SetMicaMaterialForThisWindow(hWnd, TRUE);
    return hr;
}

BOOL AltTab_PostMessageW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (hWnd == FindWindowW(L"Shell_TrayWnd", NULL) && uMsg == 0x5B7 && wParam == 0 && lParam == 0) {
        return PostMessageW(hWnd, WM_COMMAND, 407, 0);
    }
    return PostMessageW(hWnd, uMsg, wParam, lParam);
}

__declspec(dllexport) void clean() {
    if (pAltTabSSO) pAltTabSSO->lpVtbl->Release(pAltTabSSO);
    if (hAltTab) sws_WindowHelpers_Clear();
}

__declspec(dllexport) int setup() {
    hAltTab = LoadLibraryW(L"AltTab.dll");
    if (hAltTab) {
        sws_WindowHelpers_Initialize();
        VnPatchIAT(hAltTab, "dwmapi.dll", "DwmExtendFrameIntoClientArea", AltTab_DwmExtendFrameIntoClientArea);
        VnPatchIAT(hAltTab, "dwmapi.dll", (LPCSTR)113, AltTab_DwmpActivateLivePreview);
        VnPatchIAT(hAltTab, "user32.dll", "PostMessageW", AltTab_PostMessageW);
        VnPatchIAT(hAltTab, "user32.dll", "LoadStringW", AltTab_LoadStringW);
        VnPatchIAT(hAltTab, "user32.dll", "IsWindowEnabled", AltTab_IsWindowEnabled);
        VnPatchDelayIAT(hAltTab, "uxtheme.dll", "OpenThemeData", AltTab_OpenThemeData);
        VnPatchDelayIAT(hAltTab, "uxtheme.dll", "DrawThemeTextEx", AltTab_DrawThemeTextEx);
        HRESULT(*pDllGetClassObject)(REFCLSID, REFIID, LPVOID) = GetProcAddress(hAltTab, "DllGetClassObject");
        IClassFactory* pFactory = NULL;
        if (pDllGetClassObject && SUCCEEDED(pDllGetClassObject(&CLSID_AltTabSSO, &IID_IClassFactory, &pFactory)) && pFactory) {
            if (SUCCEEDED(pFactory->lpVtbl->CreateInstance(pFactory, NULL, &IID_IOleCommandTarget, &pAltTabSSO)) && pAltTabSSO) {
                if (SUCCEEDED(pAltTabSSO->lpVtbl->Exec(pAltTabSSO, &CGID_ShellServiceObject, 2, 0, NULL, NULL))) {
                    printf(">>> Using Windows 7 AltTab\n");
                }
            }
            pFactory->lpVtbl->Release(pFactory);
        }
        FreeLibrary(hAltTab);
        return 0;
    }
    return 1;
}

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
) {
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
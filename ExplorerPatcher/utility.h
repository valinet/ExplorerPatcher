#ifndef _H_UTILITY_H_
#define _H_UTILITY_H_
#if __has_include("ep_private.h")
#define USE_PRIVATE_INTERFACES
#endif
#include <Windows.h>
#include <tchar.h>
#include <windows.data.xml.dom.h>
#include <accctrl.h>
#include <aclapi.h>
#include <sddl.h>
#include <Shlobj_core.h>
#define _LIBVALINET_INCLUDE_UNIVERSAL
#include <valinet/universal/toast/toast.h>

#define APPID L"Microsoft.Windows.Explorer"
#define REGPATH "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ExplorerPatcher"
#define SPECIAL_FOLDER CSIDL_APPDATA
#define APP_RELATIVE_PATH "\\ExplorerPatcher"

// This allows compiling with older Windows SDKs as well
#ifndef DWMWA_USE_HOSTBACKDROPBRUSH
#define DWMWA_USE_HOSTBACKDROPBRUSH 17            // [set] BOOL, Allows the use of host backdrop brushes for the window.
#endif
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20          // [set] BOOL, Allows a window to either use the accent color, or dark, according to the user Color Mode preferences.
#endif
#ifndef DWMWCP_DEFAULT
#define DWMWCP_DEFAULT 0
#endif
#ifndef DWMWCP_DONOTROUND
#define DWMWCP_DONOTROUND 1
#endif
#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif
#ifndef DWMWCP_ROUNDSMALL
#define DWMWCP_ROUNDSMALL 3
#endif
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33         // [set] WINDOW_CORNER_PREFERENCE, Controls the policy that rounds top-level window corners
#endif
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34                     // [set] COLORREF, The color of the thin border around a top-level window
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35                    // [set] COLORREF, The color of the caption
#endif
#ifndef DWMWA_TEXT_COLOR
#define DWMWA_TEXT_COLOR 36                       // [set] COLORREF, The color of the caption text
#endif
#ifndef DWMWA_VISIBLE_FRAME_BORDER_THICKNESS
#define DWMWA_VISIBLE_FRAME_BORDER_THICKNESS 37   // [get] UINT, width of the visible border around a thick frame window
#endif
#ifndef DWMWA_MICA_EFFFECT
#define DWMWA_MICA_EFFFECT 1029
#endif

#pragma region "Weird stuff"
INT64 nimpl4_1(INT64 a1, DWORD* a2);
INT64 nimpl4_0(INT64 a1, DWORD* a2);
__int64 __fastcall nimpl2(__int64 a1, uintptr_t* a2);
ULONG nimpl3();
HRESULT nimpl();
HRESULT nimpl1(__int64 a1, uintptr_t* a2, uintptr_t* a3);
HRESULT nimpl1_2(__int64 a1, uintptr_t* a2, uintptr_t* a3);
HRESULT nimpl1_3(__int64 a1, uintptr_t* a2, uintptr_t* a3);
__int64 nimpl4(__int64 a1, __int64 a2, __int64 a3, BYTE* a4);
typedef struct _IActivationFactoryAA
{
    CONST_VTBL struct IActivationFactoryVtbl* lpVtbl;
    struct IActivationFactoryVtbl* lpVtbl2;
    struct IActivationFactoryVtbl* lpVtbl3;
} IActivationFactoryAA;
extern const IActivationFactoryAA XamlExtensionsFactory;
#pragma endregion

int FileExistsW(wchar_t* file);

// https://stackoverflow.com/questions/1672677/print-a-guid-variable
void printf_guid(GUID guid);

LRESULT CALLBACK BalloonWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

__declspec(dllexport) CALLBACK ZZTestBalloon(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllexport) CALLBACK ZZTestToast(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllexport) CALLBACK ZZLaunchExplorer(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllexport) CALLBACK ZZLaunchExplorerDelayed(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

POINT GetDefaultWinXPosition(BOOL bUseRcWork, BOOL* lpBottom, BOOL* lpRight, BOOL bAdjust);

void QueryVersionInfo(HMODULE hModule, WORD Resource, DWORD*, DWORD*, DWORD*, DWORD*);

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

FARPROC SHRegGetValueFromHKCUHKLMFunc;

inline LSTATUS SHRegGetValueFromHKCUHKLMWithOpt(
    PCWSTR pwszKey,
    PCWSTR pwszValue,
    REGSAM samDesired,
    void* pvData,
    DWORD* pcbData
)
{
    LSTATUS lRes = ERROR_FILE_NOT_FOUND;
    HKEY hKey = NULL;

    RegOpenKeyExW(
        HKEY_CURRENT_USER,
        pwszKey,
        0,
        samDesired,
        &hKey
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        lRes = RegQueryValueExW(
            hKey,
            pwszValue,
            0,
            NULL,
            pvData,
            pcbData
        );
        RegCloseKey(hKey);
        if (lRes == ERROR_SUCCESS || lRes == ERROR_MORE_DATA)
        {
            return lRes;
        }
    }
    RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        pwszKey,
        0,
        samDesired,
        &hKey
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        lRes = RegQueryValueExW(
            hKey,
            pwszValue,
            0,
            NULL,
            pvData,
            pcbData
        );
        RegCloseKey(hKey);
        if (lRes == ERROR_SUCCESS || lRes == ERROR_MORE_DATA)
        {
            return lRes;
        }
    }
    return lRes;
}

static HWND(WINAPI* CreateWindowInBand)(
    _In_ DWORD dwExStyle,
    _In_opt_ ATOM atom,
    _In_opt_ LPCWSTR lpWindowName,
    _In_ DWORD dwStyle,
    _In_ int X,
    _In_ int Y,
    _In_ int nWidth,
    _In_ int nHeight,
    _In_opt_ HWND hWndParent,
    _In_opt_ HMENU hMenu,
    _In_opt_ HINSTANCE hInstance,
    _In_opt_ LPVOID lpParam,
    DWORD band
    );

BOOL(WINAPI* GetWindowBand)(HWND hWnd, PDWORD pdwBand);

BOOL(WINAPI* SetWindowBand)(HWND hWnd, HWND hwndInsertAfter, DWORD dwBand);

INT64(*SetWindowCompositionAttribute)(HWND, void*);

static void(*SetPreferredAppMode)(INT64 bAllowDark);

static void(*AllowDarkModeForWindow)(HWND hWnd, INT64 bAllowDark);

static BOOL(*ShouldAppsUseDarkMode)();

static void(*GetThemeName)(void*, void*, void*);

static BOOL AppsShouldUseDarkMode() { return TRUE; }

void* ReadFromFile(wchar_t* wszFileName, DWORD* dwSize);

inline long long milliseconds_now() {
    LARGE_INTEGER s_frequency;
    BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
    if (s_use_qpc) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / s_frequency.QuadPart;
    }
    else {
        return GetTickCount();
    }
}

inline BOOL IsAppRunningAsAdminMode()
{
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;

    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pAdministratorsGroup))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Determine whether the SID of administrators group is enabled in 
    // the primary access token of the process.
    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError)
    {
        return FALSE;
    }

    return fIsRunAsAdmin;
}
#endif
#ifndef _H_UTILITY_H_
#define _H_UTILITY_H_
#if __has_include("ep_private.h")
//#define USE_PRIVATE_INTERFACES
#endif
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.data.xml.dom.h>
#include <accctrl.h>
#include <aclapi.h>
#include <sddl.h>
#include <Shobjidl.h>
#include <Shlobj_core.h>
#include <restartmanager.h>
#pragma comment(lib, "Rstrtmgr.lib")
#define _LIBVALINET_INCLUDE_UNIVERSAL
#include <valinet/universal/toast/toast.h>
#include "osutility.h"
#include "queryversion.h"
#pragma comment(lib, "Psapi.lib")
#include <activscp.h>
#include <netlistmgr.h>

#include "def.h"

#define WM_MSG_GUI_SECTION WM_USER + 1
#define WM_MSG_GUI_SECTION_GET 1

DEFINE_GUID(CLSID_ImmersiveShell,
    0xc2f03a33,
    0x21f5, 0x47fa, 0xb4, 0xbb,
    0x15, 0x63, 0x62, 0xa2, 0xf2, 0x39
);

DEFINE_GUID(IID_OpenControlPanel,
    0xD11AD862,
    0x66De, 0x4DF4, 0xBf, 0x6C,
    0x1F, 0x56, 0x21, 0x99, 0x6A, 0xF1
);

DEFINE_GUID(CLSID_VBScript,
    0xB54F3741, 
    0x5B07, 0x11CF, 0xA4, 0xB0, 
    0x00, 0xAA, 0x00, 0x4A, 0x55, 0xE8
);

DEFINE_GUID(CLSID_NetworkListManager,
    0xDCB00C01, 0x570F, 0x4A9B, 0x8D, 0x69, 0x19, 0x9F, 0xDB, 0xA5, 0x72, 0x3B);

DEFINE_GUID(IID_NetworkListManager,
    0xDCB00000, 0x570F, 0x4A9B, 0x8D, 0x69, 0x19, 0x9F, 0xDB, 0xA5, 0x72, 0x3B);

typedef struct _StuckRectsData
{
    int pvData[6];
    RECT rc;
    POINT pt;
} StuckRectsData;

HRESULT FindDesktopFolderView(REFIID riid, void** ppv);

HRESULT GetDesktopAutomationObject(REFIID riid, void** ppv);

HRESULT ShellExecuteFromExplorer(
    PCWSTR pszFile,
    PCWSTR pszParameters,
    PCWSTR pszDirectory,
    PCWSTR pszOperation,
    int nShowCmd
);

void ToggleTaskbarAutohide();

#pragma region "Weird stuff"
typedef interface WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics;

DEFINE_GUID(IID_WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics,
    0x18c02f2e,
    0x2754, 0x5a20, 0x8b, 0xd5,
    0x0b, 0x34, 0xce, 0x79, 0xda, 0x2b
);

typedef struct WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStaticsVtbl // : IInspectableVtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* Current)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics* This,
        /* [out] */ __RPC__out void** _instance_of_winrt_WindowsUdk_ApplicationModel_AppExtensions_XamlExtensions);

    END_INTERFACE
} WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStaticsVtbl;

interface WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics // : IInspectable
{
    const struct WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStaticsVtbl* lpVtbl;
};

typedef interface WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2 WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2;

DEFINE_GUID(IID_WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2,
    0x0fe87da5,
    0xa7a6, 0x5de3, 0x83, 0x5f,
    0xd9, 0x8c, 0x87, 0x56, 0x01, 0x44
);

typedef struct WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2Vtbl // : IInspectableVtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* GetForCategory)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2* This,
        __RPC__in HSTRING a2,
        /* [out] */ __RPC__out void** _instance_of_winrt_WindowsUdk_ApplicationModel_AppExtensions_XamlExtensions);

    END_INTERFACE
} WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2Vtbl;

interface WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2 // : IInspectable
{
    const struct WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2Vtbl* lpVtbl;
};

typedef interface WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2 WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2;

DEFINE_GUID(IID_WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2,
    0x34a95314,
    0xca5c, 0x5fad, 0xae, 0x7c,
    0x1a, 0x90, 0x18, 0x11, 0x66, 0xc1
);

typedef struct WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2Vtbl // : IInspectableVtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* IsExtensionAvailable)(
        __RPC__in WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2* This,
        __RPC__in HSTRING a2,
        __RPC__in HSTRING a3,
        /* [out] */ __RPC__out BYTE* a4);

    END_INTERFACE
} WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2Vtbl;

interface WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2 // : IInspectable
{
    const struct WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2Vtbl* lpVtbl;
};

extern const WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics instanceof_WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics;
extern const WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2 instanceof_WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensionsStatics2;
extern const WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2 instanceof_WindowsUdk_ApplicationModel_AppExtensions_IXamlExtensions2;
#pragma endregion

inline int FileExistsW(wchar_t* file)
{
    WIN32_FIND_DATAW FindFileData;
    HANDLE handle = FindFirstFileW(file, &FindFileData);
    int found = handle != INVALID_HANDLE_VALUE;
    if (found)
    {
        FindClose(handle);
    }
    return found;
}

// https://stackoverflow.com/questions/1672677/print-a-guid-variable
void printf_guid(GUID guid);

LRESULT CALLBACK BalloonWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

__declspec(dllexport) CALLBACK ZZTestBalloon(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllexport) CALLBACK ZZTestToast(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllexport) CALLBACK ZZLaunchExplorer(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllexport) CALLBACK ZZLaunchExplorerDelayed(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllexport) CALLBACK ZZRestartExplorer(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

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

static BOOL(*ShouldSystemUseDarkMode)();

static void(*GetThemeName)(void*, void*, void*);

static BOOL AppsShouldUseDarkMode() { return TRUE; }

void* ReadFromFile(wchar_t* wszFileName, DWORD* dwSize);

int ComputeFileHash(LPCWSTR filename, LPSTR hash, DWORD dwHash);

int ComputeFileHash2(HMODULE hModule, LPCWSTR filename, LPSTR hash, DWORD dwHash);

void LaunchPropertiesGUI(HMODULE hModule);

BOOL SystemShutdown(BOOL reboot);

LSTATUS RegisterDWMService(DWORD dwDesiredState, DWORD dwOverride);

char* StrReplaceAllA(const char* s, const char* oldW, const char* newW, int* dwNewSize);

WCHAR* StrReplaceAllW(const WCHAR* s, const WCHAR* oldW, const WCHAR* newW, int* dwNewSize);

HRESULT InputBox(BOOL bPassword, HWND hWnd, LPCWSTR wszPrompt, LPCWSTR wszTitle, LPCWSTR wszDefault, LPWSTR wszAnswer, DWORD cbAnswer, BOOL* bCancelled);

inline BOOL IsHighContrast()
{
    HIGHCONTRASTW highContrast;
    ZeroMemory(&highContrast, sizeof(HIGHCONTRASTW));
    highContrast.cbSize = sizeof(highContrast);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, FALSE))
        return highContrast.dwFlags & HCF_HIGHCONTRASTON;
    return FALSE;
}

// https://codereview.stackexchange.com/questions/29198/random-string-generator-in-c
static inline WCHAR* rand_string(WCHAR* str, size_t size)
{
    const WCHAR charset[] = L"abcdefghijklmnopqrstuvwxyz";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int)((sizeof(charset) / sizeof(WCHAR)) - 1);
            str[n] = charset[key];
        }
        str[size] = L'\0';
    }
    return str;
}

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

inline BOOL IsDesktopWindowAlreadyPresent()
{
    return (FindWindowExW(NULL, NULL, L"Progman", NULL) || FindWindowExW(NULL, NULL, L"Proxy Desktop", NULL));
}

// https://jiangsheng.net/2013/01/22/how-to-restart-windows-explorer-programmatically-using-restart-manager/
inline RM_UNIQUE_PROCESS GetExplorerApplication()
{
    HWND hwnd = FindWindow(L"Shell_TrayWnd", NULL);
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);

    RM_UNIQUE_PROCESS out = { 0, { -1, -1 } };
    DWORD bytesReturned;
    WCHAR imageName[MAX_PATH]; // process image name buffer
    DWORD processIds[2048]; // max 2048 processes (more than enough)

    // enumerate all running processes (usually around 60-70)
    EnumProcesses(processIds, sizeof(processIds), &bytesReturned);
    int count = bytesReturned / sizeof(DWORD); // number of processIds returned

    for (int i = 0; i < count; ++i)
    {
        DWORD processId = processIds[i];
        HANDLE hProc;
        if (processId == pid && (hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId)))
        {
            GetProcessImageFileNameW(hProc, imageName, MAX_PATH);
            FILETIME ftStart, ftExit, ftKernel, ftUser;
            GetProcessTimes(hProc, &ftStart, &ftExit, &ftKernel, &ftUser);

            if (ftStart.dwLowDateTime < out.ProcessStartTime.dwLowDateTime)
            {
                out.dwProcessId = processId;
                out.ProcessStartTime = ftStart;
            }
            CloseHandle(hProc);
        }
    }
    return out; // return count in pResults
}

static DWORD RmSession = -1;
static wchar_t RmSessionKey[CCH_RM_SESSION_KEY + 1];

// shuts down the explorer and is ready for explorer restart
inline void BeginExplorerRestart()
{
    if (RmStartSession(&RmSession, 0, RmSessionKey) == ERROR_SUCCESS)
    {
        RM_UNIQUE_PROCESS rgApplications[] = { GetExplorerApplication() };
        RmRegisterResources(RmSession, 0, 0, 1, rgApplications, 0, 0);

        DWORD rebootReason;
        UINT nProcInfoNeeded, nProcInfo = 16;
        RM_PROCESS_INFO affectedApps[16];
        RmGetList(RmSession, &nProcInfoNeeded, &nProcInfo, affectedApps, &rebootReason);

        if (rebootReason == RmRebootReasonNone) // no need for reboot?
        {
            // shutdown explorer
            RmShutdown(RmSession, RmForceShutdown, 0);
        }
    }
}
// restarts the explorer
inline void FinishExplorerRestart()
{
    DWORD dwError;
    if (dwError = RmRestart(RmSession, 0, NULL))
        printf("\n RmRestart error: %d\n\n", dwError);

    RmEndSession(RmSession);
    RmSession = -1;
    RmSessionKey[0] = 0;
}

// https://stackoverflow.com/questions/5689904/gracefully-exit-explorer-programmatically
inline BOOL ExitExplorer()
{
    HWND hWndTray = FindWindowW(L"Shell_TrayWnd", NULL);
    return PostMessageW(hWndTray, 0x5B4, 0, 0);
}

inline void StartExplorerWithDelay(int delay, HANDLE userToken)
{
    WCHAR wszPath[MAX_PATH];
    ZeroMemory(wszPath, MAX_PATH * sizeof(WCHAR));
    GetWindowsDirectoryW(wszPath, MAX_PATH);
    wcscat_s(wszPath, MAX_PATH, L"\\explorer.exe");
    Sleep(delay);
    if (userToken != INVALID_HANDLE_VALUE)
    {
        HANDLE primaryUserToken = INVALID_HANDLE_VALUE;
        if (ImpersonateLoggedOnUser(userToken))
        {
            DuplicateTokenEx(userToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &primaryUserToken);
            RevertToSelf();
        }
        if (primaryUserToken != INVALID_HANDLE_VALUE)
        {
            PROCESS_INFORMATION processInfo;
            ZeroMemory(&processInfo, sizeof(processInfo));
            STARTUPINFOW startupInfo;
            ZeroMemory(&startupInfo, sizeof(startupInfo));
            startupInfo.cb = sizeof(startupInfo);
            BOOL processCreated = CreateProcessWithTokenW(
                primaryUserToken, LOGON_WITH_PROFILE, wszPath, NULL, 0, NULL, NULL, &startupInfo, &processInfo) != 0;
            CloseHandle(primaryUserToken);
            if (processInfo.hProcess != INVALID_HANDLE_VALUE)
            {
                CloseHandle(processInfo.hProcess);
            }
            if (processInfo.hThread != INVALID_HANDLE_VALUE)
            {
                CloseHandle(processInfo.hThread);
            }
            if (processCreated)
            {
                return;
            }
        }
    }
    ShellExecuteW(
        NULL,
        L"open",
        wszPath,
        NULL,
        NULL,
        SW_SHOWNORMAL
    );
}

inline void StartExplorer()
{

    /*PROCESSENTRY32 pe32 = {0};
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPPROCESS,
        0
    );
    if (Process32First(hSnapshot, &pe32) == TRUE)
    {
        do
        {
            if (!wcscmp(pe32.szExeFile, TEXT("explorer.exe")))
            {
                HANDLE hSihost = OpenProcess(
                    PROCESS_TERMINATE,
                    FALSE,
                    pe32.th32ProcessID
                );
                TerminateProcess(hSihost, 1);
                CloseHandle(hSihost);
            }
        } while (Process32Next(hSnapshot, &pe32) == TRUE);
    }
    CloseHandle(hSnapshot);
    */
    wchar_t wszPath[MAX_PATH];
    ZeroMemory(
        wszPath,
        (MAX_PATH) * sizeof(wchar_t)
    );
    GetWindowsDirectoryW(
        wszPath,
        MAX_PATH
    );
    wcscat_s(
        wszPath,
        MAX_PATH,
        L"\\explorer.exe"
    );
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    if (CreateProcessW(
        NULL,
        wszPath,
        NULL,
        NULL,
        TRUE,
        CREATE_UNICODE_ENVIRONMENT,
        NULL,
        NULL,
        &si,
        &pi
    ))
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

inline BOOL IncrementDLLReferenceCount(HINSTANCE hinst)
{
    HMODULE hMod;
    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        hinst,
        &hMod);
}

inline BOOL WINAPI PatchContextMenuOfNewMicrosoftIME(BOOL* bFound)
{
    // huge thanks to @Simplestas: https://github.com/valinet/ExplorerPatcher/issues/598
    if (bFound) *bFound = FALSE;
    DWORD patch_from, patch_to;
    if (IsWindows11Version22H2OrHigher())
    {
        // cmp byte ptr [rbp+40h+arg_0], r13b
        patch_from = 0x506D3844;
        patch_to = 0x546D3844;
    }
    else
    {
        // cmp byte ptr [rbp+50h], r12b
        patch_from = 0x50653844;
        patch_to = 0x54653844;
    }
    HMODULE hInputSwitch = NULL;
    if (!GetModuleHandleExW(0, L"InputSwitch.dll", &hInputSwitch))
    {
        return FALSE;
    }
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hInputSwitch;
    PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((DWORD_PTR)dosHeader + dosHeader->e_lfanew);
    PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)(pNTHeader + 1);
    char* mod = 0;
    int i;
    for (i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
    {
        //if (strcmp((char*)pSectionHeader[i].Name, ".text") == 0)
        if ((pSectionHeader[i].Characteristics & IMAGE_SCN_CNT_CODE) && pSectionHeader[i].SizeOfRawData)
        {
            mod = (char*)dosHeader + pSectionHeader[i].VirtualAddress;
            break;
        }
    }
    if (!mod)
    {
        return FALSE;
    }
    for (size_t off = 0; off < pSectionHeader[i].Misc.VirtualSize - sizeof(DWORD); ++off)
    {
        DWORD* ptr = (DWORD*)(mod + off);
        if (*ptr == patch_from)
        {
            if (bFound) *bFound = TRUE;
            DWORD prot;
            if (VirtualProtect(ptr, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &prot))
            {
                *ptr = patch_to;
                VirtualProtect(ptr, sizeof(DWORD), prot, &prot);
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

extern UINT PleaseWaitTimeout;
extern HHOOK PleaseWaitHook;
extern HWND PleaseWaitHWND;
extern void* PleaseWaitCallbackData;
extern BOOL (*PleaseWaitCallbackFunc)(void* data);
BOOL PleaseWait_UpdateTimeout(int timeout);
VOID CALLBACK PleaseWait_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);
LRESULT CALLBACK PleaseWait_HookProc(int code, WPARAM wParam, LPARAM lParam);

BOOL DownloadAndInstallWebView2Runtime();

BOOL DownloadFile(LPCWSTR wszURL, DWORD dwSize, LPCWSTR wszPath);

BOOL IsConnectedToInternet();

#define SCRATCH_QCM_FIRST 1
#define SCRATCH_QCM_LAST  0x7FFF

#define SPOP_OPENMENU            1
#define SPOP_INSERTMENU_ALL      0b1111110000
#define SPOP_INSERTMENU_OPEN     0b0000010000
#define SPOP_INSERTMENU_NEXTPIC  0b0000100000
#define SPOP_INSERTMENU_LIKE     0b0001000000
#define SPOP_INSERTMENU_DISLIKE  0b0010000000
#define SPOP_INSERTMENU_INFOTIP1 0b0100000000
#define SPOP_INSERTMENU_INFOTIP2 0b1000000000
#define SPOP_CLICKMENU_FIRST     40000
#define SPOP_CLICKMENU_OPEN      40000
#define SPOP_CLICKMENU_NEXTPIC   40001
#define SPOP_CLICKMENU_LIKE      40002
#define SPOP_CLICKMENU_DISLIKE   40003
#define SPOP_CLICKMENU_LAST      40003

BOOL DoesOSBuildSupportSpotlight();

BOOL IsSpotlightEnabled();

void SpotlightHelper(DWORD dwOp, HWND hWnd, HMENU hMenu, LPPOINT pPt);

typedef struct _MonitorOverrideData
{
    DWORD cbIndex;
    DWORD dwIndex;
    HMONITOR hMonitor;
} MonitorOverrideData;

BOOL ExtractMonitorByIndex(HMONITOR hMonitor, HDC hDC, LPRECT lpRect, MonitorOverrideData* mod);

inline BOOL MaskCompare(PVOID pBuffer, LPCSTR lpPattern, LPCSTR lpMask)
{
    for (PBYTE value = pBuffer; *lpMask; ++lpPattern, ++lpMask, ++value)
    {
        if (*lpMask == 'x' && *(LPCBYTE)lpPattern != *value)
            return FALSE;
    }

    return TRUE;
}

inline PVOID FindPattern(PVOID pBase, SIZE_T dwSize, LPCSTR lpPattern, LPCSTR lpMask)
{
    dwSize -= strlen(lpMask);

    for (SIZE_T index = 0; index < dwSize; ++index)
    {
        PBYTE pAddress = (PBYTE)pBase + index;

        if (MaskCompare(pAddress, lpPattern, lpMask))
            return pAddress;
    }

    return NULL;
}
#endif

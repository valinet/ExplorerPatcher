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
#include <Shlobj_core.h>
#include <restartmanager.h>
#pragma comment(lib, "Rstrtmgr.lib")
#define _LIBVALINET_INCLUDE_UNIVERSAL
#include <valinet/universal/toast/toast.h>
#include "queryversion.h"
#pragma comment(lib, "Psapi.lib")

#define APPID L"Microsoft.Windows.Explorer"
#define REGPATH "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ExplorerPatcher"
#define SPECIAL_FOLDER CSIDL_PROGRAM_FILES
#define SPECIAL_FOLDER_LEGACY CSIDL_APPDATA
#define PRODUCT_NAME "ExplorerPatcher"
#define PRODUCT_PUBLISHER "VALINET Solutions SRL"
#define APP_RELATIVE_PATH "\\" PRODUCT_NAME
#define EP_CLSID "{D17F1E1A-5919-4427-8F89-A1A8503CA3EB}"
#define DOSMODE_OFFSET 78
#define SETUP_UTILITY_NAME "ep_setup.exe"
#define TOAST_BUFSIZ 1024
#define SEH_REGPATH "Control Panel\\Quick Actions\\Control Center\\QuickActionsStateCapture\\ExplorerPatcher"
#define EP_SETUP_HELPER_SWITCH "/CreateExplorerShellUnelevatedAfterServicing"

#define WM_MSG_GUI_SECTION WM_USER + 1
#define WM_MSG_GUI_SECTION_GET 1

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

DEFINE_GUID(CLSID_ImmersiveShell,
    0xc2f03a33,
    0x21f5, 0x47fa, 0xb4, 0xbb,
    0x15, 0x63, 0x62, 0xa2, 0xf2, 0x39
);

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

#pragma region "Weird stuff"
INT64 STDMETHODCALLTYPE nimpl4_1(INT64 a1, DWORD* a2);
INT64 STDMETHODCALLTYPE nimpl4_0(INT64 a1, DWORD* a2);
__int64 STDMETHODCALLTYPE nimpl2(__int64 a1, uintptr_t* a2);
ULONG STDMETHODCALLTYPE nimpl3();
HRESULT STDMETHODCALLTYPE nimpl();
HRESULT STDMETHODCALLTYPE nimpl1(__int64 a1, uintptr_t* a2, uintptr_t* a3);
HRESULT STDMETHODCALLTYPE nimpl1_2(__int64 a1, uintptr_t* a2, uintptr_t* a3);
HRESULT STDMETHODCALLTYPE nimpl1_3(__int64 a1, uintptr_t* a2, uintptr_t* a3);
__int64 STDMETHODCALLTYPE nimpl4(__int64 a1, __int64 a2, __int64 a3, BYTE* a4);
typedef struct _IActivationFactoryAA
{
    CONST_VTBL struct IActivationFactoryVtbl* lpVtbl;
    struct IActivationFactoryVtbl* lpVtbl2;
    struct IActivationFactoryVtbl* lpVtbl3;
} IActivationFactoryAA;
extern const IActivationFactoryAA XamlExtensionsFactory;
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

static void(*GetThemeName)(void*, void*, void*);

static BOOL AppsShouldUseDarkMode() { return TRUE; }

void* ReadFromFile(wchar_t* wszFileName, DWORD* dwSize);

int ComputeFileHash(LPCWSTR filename, LPCSTR hash, DWORD dwHash);

void LaunchPropertiesGUI(HMODULE hModule);

BOOL SystemShutdown(BOOL reboot);

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

inline void StartExplorerWithDelay(int delay)
{
    WCHAR wszPath[MAX_PATH];
    ZeroMemory(wszPath, MAX_PATH * sizeof(WCHAR));
    GetWindowsDirectoryW(wszPath, MAX_PATH);
    wcscat_s(wszPath, MAX_PATH, L"\\explorer.exe");
    Sleep(delay);
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
#endif
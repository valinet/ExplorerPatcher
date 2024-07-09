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
#ifndef __cplusplus
#include <valinet/universal/toast/toast.h>
#endif
#include "osutility.h"
#include "queryversion.h"
#pragma comment(lib, "Psapi.lib")
#include <activscp.h>
#include <netlistmgr.h>
#include <Psapi.h>
#include <stdbool.h>
#include "Localization.h"

#include "def.h"

#define WM_MSG_GUI_SECTION WM_USER + 1
#define WM_MSG_GUI_SECTION_GET 1

#ifdef __cplusplus
extern "C" {
#endif

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

#pragma region "Enable old taskbar"
typedef interface ITrayUIHost ITrayUIHost;

typedef interface ITrayUI ITrayUI;

DEFINE_GUID(IID_ITrayUI,
    0x12b454e1,
    0x6e50, 0x42b8, 0xbc, 0x3e,
    0xae, 0x7f, 0x54, 0x91, 0x99, 0xd6
);

DEFINE_GUID(IID_ITrayUIComponent,
    0x27775f88,
    0x01d3, 0x46ec, 0xa1, 0xc1,
    0x64, 0xb4, 0xc0, 0x9b, 0x21, 0x1b
);

#ifdef __cplusplus
inline
#endif
HRESULT(*explorer_TrayUI_CreateInstanceFunc)(ITrayUIHost* host, REFIID riid, void** ppv);
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

__declspec(dllexport) int CALLBACK ZZTestBalloon(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

#ifdef _DEBUG
__declspec(dllexport) int CALLBACK ZZTestToast(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);
#endif

__declspec(dllexport) int CALLBACK ZZLaunchExplorer(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllexport) int CALLBACK ZZLaunchExplorerDelayed(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllexport) int CALLBACK ZZRestartExplorer(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

typedef LSTATUS(*t_SHRegGetValueFromHKCUHKLM)(
    PCWSTR pwszKey,
    PCWSTR pwszValue,
    int/*SRRF*/ srrfFlags,
    DWORD* pdwType,
    void* pvData,
    DWORD* pcbData
);
#ifdef __cplusplus
inline
#endif
t_SHRegGetValueFromHKCUHKLM SHRegGetValueFromHKCUHKLMFunc;

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
            (LPBYTE)pvData,
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
            (LPBYTE)pvData,
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

#ifdef __cplusplus
inline
#endif
HWND(WINAPI* CreateWindowInBand)(
    _In_ DWORD dwExStyle,
    _In_opt_ LPCWSTR lpClassName,
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

#ifdef __cplusplus
inline
#endif
BOOL(WINAPI* GetWindowBand)(HWND hWnd, PDWORD pdwBand);

#ifdef __cplusplus
inline
#endif
BOOL(WINAPI* SetWindowBand)(HWND hWnd, HWND hwndInsertAfter, DWORD dwBand);

#ifdef __cplusplus
inline
#endif
INT64(*SetWindowCompositionAttribute)(HWND, void*);

static void(*SetPreferredAppMode)(BOOL bAllowDark);

static void(*AllowDarkModeForWindow)(HWND hWnd, BOOL bAllowDark);

static bool(*ShouldAppsUseDarkMode)();

static bool(*ShouldSystemUseDarkMode)();

static void(*GetThemeName)(void*, void*, void*);

void* ReadFromFile(wchar_t* wszFileName, DWORD* dwSize);

int ComputeFileHash(LPCWSTR filename, LPSTR hash, DWORD dwHash);

int ComputeFileHash2(HMODULE hModule, LPCWSTR filename, LPSTR hash, DWORD dwHash);

void GetHardcodedHash(LPCWSTR wszPath, LPSTR hash, DWORD dwHash);

void LaunchPropertiesGUI(HMODULE hModule);

BOOL SystemShutdown(BOOL reboot);

LSTATUS RegisterDWMService(DWORD dwDesiredState, DWORD dwOverride);

char* StrReplaceAllA(const char* s, const char* oldW, const char* newW, int* dwNewSize);

WCHAR* StrReplaceAllW(const WCHAR* s, const WCHAR* oldW, const WCHAR* newW, int* dwNewSize);

HRESULT InputBox(BOOL bPassword, HWND hWnd, LPCWSTR wszPrompt, LPCWSTR wszTitle, LPCWSTR wszDefault, LPWSTR wszAnswer, DWORD cbAnswer, BOOL* bCancelled);

BOOL GetLogonSid(PSID* ppsid);

BOOL PrepareSecurityDescriptor(PSID pMainSid, DWORD dwMainPermissions, PSID pSecondarySid, DWORD dwSecondayPermissions, PSECURITY_DESCRIPTOR* ppSD);

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
inline DWORD WINAPI BeginExplorerRestart(LPVOID lpUnused)
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
    return 0;
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
        (LPCWSTR)hinst,
        &hMod);
    return TRUE;
}

#ifdef _WIN64
PVOID FindPattern(PVOID pBase, SIZE_T dwSize, LPCSTR lpPattern, LPCSTR lpMask);

inline BOOL WINAPI PatchContextMenuOfNewMicrosoftIME(BOOL* bFound)
{
    // huge thanks to @Simplestas: https://github.com/valinet/ExplorerPatcher/issues/598
    HMODULE hInputSwitch = NULL;
    if (!GetModuleHandleExW(0, L"InputSwitch.dll", &hInputSwitch))
        return FALSE;

    MODULEINFO mi;
    GetModuleInformation(GetCurrentProcess(), hInputSwitch, &mi, sizeof(mi));

    // 44 38 ?? ?? 74 ?? ?? 8B CE E8 ?? ?? ?? ?? 85 C0
    //             ^^ Change jz into jmp
    PBYTE match = (PBYTE)FindPattern(
        hInputSwitch,
        mi.SizeOfImage,
        "\x44\x38\x00\x00\x74\x00\x00\x8B\xCE\xE8\x00\x00\x00\x00\x85\xC0",
        "xx??x??xxx????xx"
    );
    if (!match)
        return FALSE;

    DWORD dwOldProtect;
    if (!VirtualProtect(match + 4, 1, PAGE_EXECUTE_READWRITE, &dwOldProtect))
        return FALSE;

    match[4] = 0xEB;

    VirtualProtect(match + 4, 1, dwOldProtect, &dwOldProtect);

    return TRUE;
}
#endif

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
HRESULT SHRegGetBOOLWithREGSAM(HKEY key, LPCWSTR subKey, LPCWSTR value, REGSAM regSam, BOOL* data);
HRESULT SHRegGetDWORD(HKEY hkey, const WCHAR* pwszSubKey, const WCHAR* pwszValue, DWORD* pdwData);

#ifdef _WIN64
PVOID FindPattern(PVOID pBase, SIZE_T dwSize, LPCSTR lpPattern, LPCSTR lpMask);
#endif

inline HMODULE LoadGuiModule()
{
    wchar_t epGuiPath[MAX_PATH];
    ZeroMemory(epGuiPath, sizeof(epGuiPath));
    SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, epGuiPath);
    wcscat_s(epGuiPath, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\ep_gui.dll");
    return LoadLibraryExW(epGuiPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
}

inline BOOL DoesWindows10StartMenuExist()
{
    if (!IsWindows11())
        return TRUE;

    wchar_t szPath[MAX_PATH];
    GetWindowsDirectoryW(szPath, MAX_PATH);
    wcscat_s(szPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\StartUI.dll");
    return FileExistsW(szPath);
}

inline BOOL IsStockWindows10TaskbarAvailable()
{
    return global_rovi.dwBuildNumber < 26002;
}

inline const WCHAR* PickTaskbarDll()
{
    DWORD b = global_rovi.dwBuildNumber;

    if ((b >= 22621 && b <= 22635)  // 22H2-23H2 Release, Release Preview, and Beta channels
     || (b >= 23403 && b <= 25197)) // Early pre-reboot Dev channel until post-reboot Dev channel
    {
        return L"ep_taskbar.2.dll";
    }

    if (b >= 25201 && b <= 25915) // Pre-reboot Dev channel until early Canary channel, nuked ITrayComponentHost methods related to classic search box
    {
        return L"ep_taskbar.3.dll";
    }

    if (b >= 25921 && b <= 26040) // Canary channel with nuked classic system tray
    {
        return L"ep_taskbar.4.dll";
    }

    if (b >= 26052) // Same as 4 but with 2 new methods in ITrayComponentHost between GetTrayUI and ProgrammableTaskbarReportClick
    {
        return L"ep_taskbar.5.dll";
    }

    return NULL;
}

inline BOOL DoesTaskbarDllExist()
{
    const wchar_t* pszTaskbarDll = PickTaskbarDll();
    if (!pszTaskbarDll)
        return FALSE;

    wchar_t szPath[MAX_PATH];
    ZeroMemory(szPath, sizeof(szPath));
    SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, szPath);
    wcscat_s(szPath, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\");
    wcscat_s(szPath, MAX_PATH, pszTaskbarDll);
    return FileExistsW(szPath);
}

inline void AdjustTaskbarStyleValue(DWORD* pdwValue)
{
    if (*pdwValue >= 2 && !DoesTaskbarDllExist())
    {
        *pdwValue = 1;
    }
    if (*pdwValue == 1 && !IsStockWindows10TaskbarAvailable())
    {
        *pdwValue = 0;
    }
}

#ifdef __cplusplus
}
#endif

#endif

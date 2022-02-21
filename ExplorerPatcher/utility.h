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
#include "queryversion.h"
#pragma comment(lib, "Psapi.lib")
#include <activscp.h>
#include <valinet/utility/osversion.h>

#include "def.h"

#define WM_MSG_GUI_SECTION WM_USER + 1
#define WM_MSG_GUI_SECTION_GET 1

// This allows compiling with older Windows SDKs as well
#ifndef NTDDI_WIN10_CO
#define DWMWA_USE_HOSTBACKDROPBRUSH 17            // [set] BOOL, Allows the use of host backdrop brushes for the window.
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20          // [set] BOOL, Allows a window to either use the accent color, or dark, according to the user Color Mode preferences.
#define DWMWA_WINDOW_CORNER_PREFERENCE 33         // [set] WINDOW_CORNER_PREFERENCE, Controls the policy that rounds top-level window corners
#define DWMWA_BORDER_COLOR 34                     // [set] COLORREF, The color of the thin border around a top-level window
#define DWMWA_CAPTION_COLOR 35                    // [set] COLORREF, The color of the caption
#define DWMWA_TEXT_COLOR 36                       // [set] COLORREF, The color of the caption text
#define DWMWA_VISIBLE_FRAME_BORDER_THICKNESS 37   // [get] UINT, width of the visible border around a thick frame window
#define DWMWCP_DEFAULT 0
#define DWMWCP_DONOTROUND 1
#define DWMWCP_ROUND 2
#define DWMWCP_ROUNDSMALL 3
#endif
#define DWMWA_MICA_EFFFECT 1029

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

inline BOOL WINAPI PatchContextMenuOfNewMicrosoftIME(BOOL* bFound)
{
    // huge thanks to @Simplestas: https://github.com/valinet/ExplorerPatcher/issues/598
    if (bFound) *bFound = FALSE;
    const DWORD patch_from = 0x50653844, patch_to = 0x54653844; // cmp byte ptr [rbp+50h], r12b
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
            }
            break;
        }
    }
    return TRUE;
}

extern UINT PleaseWaitTimeout;
extern HHOOK PleaseWaitHook;
extern HWND PleaseWaitHWND;
extern void* PleaseWaitCallbackData;
extern BOOL (*PleaseWaitCallbackFunc)(void* data);
BOOL PleaseWait_UpdateTimeout(int timeout);
VOID CALLBACK PleaseWait_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);
LRESULT CALLBACK PleaseWait_HookProc(int code, WPARAM wParam, LPARAM lParam);

inline BOOL IsWindows11()
{
    RTL_OSVERSIONINFOW rovi;
    if (VnGetOSVersion(&rovi) && rovi.dwBuildNumber >= 21996)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL DownloadAndInstallWebView2Runtime();
#endif

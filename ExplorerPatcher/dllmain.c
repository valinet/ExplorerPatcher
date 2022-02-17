#ifdef _WIN64
#include "hooking.h"
#endif
#include <initguid.h>
#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib") // required by funchook
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <windowsx.h>
#include <Uxtheme.h>
#pragma comment(lib, "UxTheme.lib")
#include <Shlobj_core.h>
#include <commctrl.h>
#pragma comment(lib, "Comctl32.lib")
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#include <roapi.h>
#include <ShellScalingApi.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
#include <tlhelp32.h>
#include <UIAutomationClient.h>
#ifdef _WIN64
#include <valinet/pdb/pdb.h>
#endif
#if defined(DEBUG) | defined(_DEBUG)
#define _LIBVALINET_DEBUG_HOOKING_IATPATCH
#endif
#include <valinet/hooking/iatpatch.h>
#include <valinet/utility/memmem.h>
#include "../ep_weather_host/ep_weather.h"
#ifdef _WIN64
#include "../ep_weather_host/ep_weather_host_h.h"
IEPWeather* epw = NULL;
SRWLOCK lock_epw = { .Ptr = SRWLOCK_INIT };
#endif

#define WINX_ADJUST_X 5
#define WINX_ADJUST_Y 5

#define CHECKFOREGROUNDELAPSED_TIMEOUT 300
#define POPUPMENU_SAFETOREMOVE_TIMEOUT 300
#define POPUPMENU_BLUETOOTH_TIMEOUT 700
#define POPUPMENU_PNIDUI_TIMEOUT 300
#define POPUPMENU_SNDVOLSSO_TIMEOUT 300
#define POPUPMENU_INPUTSWITCH_TIMEOUT 700
#define POPUPMENU_EX_ELAPSED 300

BOOL bIsExplorerProcess = FALSE;
BOOL bInstanced = FALSE;
HWND archivehWnd;
DWORD bOldTaskbar = TRUE;
DWORD bWasOldTaskbarSet = FALSE;
DWORD bAllocConsole = FALSE;
DWORD bHideExplorerSearchBar = FALSE;
DWORD bMicaEffectOnTitlebar = FALSE;
DWORD bHideControlCenterButton = FALSE;
DWORD bFlyoutMenus = TRUE;
DWORD bCenterMenus = TRUE;
DWORD bSkinMenus = TRUE;
DWORD bSkinIcons = TRUE;
DWORD bReplaceNetwork = FALSE;
DWORD dwExplorerReadyDelay = 0;
DWORD bEnableArchivePlugin = FALSE;
DWORD bMonitorOverride = TRUE;
DWORD bOpenAtLogon = FALSE;
DWORD bClockFlyoutOnWinC = FALSE;
DWORD bDisableImmersiveContextMenu = FALSE;
DWORD bClassicThemeMitigations = FALSE;
DWORD bWasClassicThemeMitigationsSet = FALSE;
DWORD bHookStartMenu = TRUE;
DWORD bPropertiesInWinX = FALSE;
DWORD bNoMenuAccelerator = FALSE;
DWORD dwIMEStyle = 0;
DWORD dwTaskbarAl = 1;
DWORD bShowUpdateToast = FALSE;
DWORD bToolbarSeparators = FALSE;
DWORD bTaskbarAutohideOnDoubleClick = FALSE;
DWORD dwOrbStyle = 0;
DWORD bEnableSymbolDownload = TRUE;
DWORD dwAltTabSettings = 0;
DWORD dwSnapAssistSettings = 0;
BOOL bDoNotRedirectSystemToSettingsApp = FALSE;
BOOL bDoNotRedirectProgramsAndFeaturesToSettingsApp = FALSE;
BOOL bDoNotRedirectDateAndTimeToSettingsApp = FALSE;
BOOL bDoNotRedirectNotificationIconsToSettingsApp = FALSE;
BOOL bDisableOfficeHotkeys = FALSE;
DWORD bNoPropertiesInContextMenu = FALSE;
#define TASKBARGLOMLEVEL_DEFAULT 2
#define MMTASKBARGLOMLEVEL_DEFAULT 2
DWORD dwTaskbarGlomLevel = TASKBARGLOMLEVEL_DEFAULT;
DWORD dwMMTaskbarGlomLevel = MMTASKBARGLOMLEVEL_DEFAULT;
HMODULE hModule = NULL;
HANDLE hDelayedInjectionThread = NULL;
HANDLE hIsWinXShown = NULL;
HANDLE hWinXThread = NULL;
HANDLE hSwsSettingsChanged = NULL;
HANDLE hSwsOpacityMaybeChanged = NULL;
HANDLE hWin11AltTabInitialized = NULL;
BYTE* lpShouldDisplayCCButton = NULL;
#define MAX_NUM_MONITORS 30
HMONITOR hMonitorList[MAX_NUM_MONITORS];
DWORD dwMonitorCount = 0;
HANDLE hCanStartSws = NULL;
DWORD dwWeatherViewMode = EP_WEATHER_VIEW_ICONTEXT;
DWORD dwWeatherTemperatureUnit = EP_WEATHER_TUNIT_CELSIUS;
DWORD dwWeatherUpdateSchedule = EP_WEATHER_UPDATE_NORMAL;
DWORD bWeatherFixedSize = FALSE;
DWORD dwWeatherTheme = 0;
DWORD dwWeatherGeolocationMode = 0;
DWORD dwWeatherWindowCornerPreference = DWMWCP_ROUND;
WCHAR* wszWeatherTerm = NULL;
WCHAR* wszWeatherLanguage = NULL;
WCHAR* wszEPWeatherKillswitch = NULL;
HANDLE hEPWeatherKillswitch = NULL;
DWORD bWasPinnedItemsActAsQuickLaunch = FALSE;
DWORD bPinnedItemsActAsQuickLaunch = FALSE;
DWORD bWasRemoveExtraGapAroundPinnedItems = FALSE;
DWORD bRemoveExtraGapAroundPinnedItems = FALSE;
int Code = 0;
HRESULT InjectStartFromExplorer();
void InvokeClockFlyout();
void WINAPI Explorer_RefreshUI(int unused);

#define ORB_STYLE_WINDOWS10 0
#define ORB_STYLE_WINDOWS11 1
#define ORB_STYLE_TRANSPARENT 2
typedef struct _OrbInfo
{
    HTHEME hTheme;
    UINT dpi;
} OrbInfo;

void* P_Icon_Light_Search = NULL;
DWORD S_Icon_Light_Search = 0;

void* P_Icon_Light_TaskView = NULL;
DWORD S_Icon_Light_TaskView = 0;

void* P_Icon_Light_Widgets = NULL;
DWORD S_Icon_Light_Widgets = 0;

void* P_Icon_Dark_Search = NULL;
DWORD S_Icon_Dark_Search = 0;

void* P_Icon_Dark_TaskView = NULL;
DWORD S_Icon_Dark_TaskView = 0;

void* P_Icon_Dark_Widgets = NULL;
DWORD S_Icon_Dark_Widgets = 0;



#include "utility.h"
#include "resource.h"
#ifdef USE_PRIVATE_INTERFACES
#include "ep_private.h"
#endif
#ifdef _WIN64
#include "symbols.h"
#include "dxgi_imp.h"
#include "ArchiveMenu.h"
#include "StartupSound.h"
#include "StartMenu.h"
#include "GUI.h"
#include "TaskbarCenter.h"
#include "../libs/sws/SimpleWindowSwitcher/sws_WindowSwitcher.h"
#endif
#include "SettingsMonitor.h"
#include "HideExplorerSearchBar.h"
#include "ImmersiveFlyouts.h"
#include "updates.h"
DWORD dwUpdatePolicy = UPDATE_POLICY_DEFAULT;

HRESULT WINAPI _DllRegisterServer();
HRESULT WINAPI _DllUnregisterServer();
HRESULT WINAPI _DllCanUnloadNow();
HRESULT WINAPI _DllGetClassObject(
    REFCLSID rclsid,
    REFIID   riid,
    LPVOID* ppv
);

#pragma region "Updates"
#ifdef _WIN64
DWORD CheckForUpdatesThread(LPVOID unused)
{
    HRESULT hr = S_OK;
    HSTRING_HEADER header_AppIdHString;
    HSTRING AppIdHString = NULL;
    HSTRING_HEADER header_ToastNotificationManagerHString;
    HSTRING ToastNotificationManagerHString = NULL;
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotificationManagerStatics* toastStatics = NULL;
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotifier* notifier = NULL;
    HSTRING_HEADER header_ToastNotificationHString;
    HSTRING ToastNotificationHString = NULL;
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotificationFactory* notifFactory = NULL;
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotification* toast = NULL;

    while (TRUE)
    {
        HWND hShell_TrayWnd = FindWindowExW(
            NULL,
            NULL,
            L"Shell_TrayWnd",
            NULL
        );
        if (hShell_TrayWnd)
        {
            Sleep(5000);
            break;
        }
        Sleep(100);
    }
    printf("[Updates] Starting daemon.\n");

    if (SUCCEEDED(hr))
    {
        hr = RoInitialize(RO_INIT_MULTITHREADED);
    }
    if (SUCCEEDED(hr))
    {
        hr = WindowsCreateStringReference(
            APPID,
            (UINT32)(sizeof(APPID) / sizeof(TCHAR) - 1),
            &header_AppIdHString,
            &AppIdHString
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = WindowsCreateStringReference(
            RuntimeClass_Windows_UI_Notifications_ToastNotificationManager,
            (UINT32)(sizeof(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager) / sizeof(wchar_t) - 1),
            &header_ToastNotificationManagerHString,
            &ToastNotificationManagerHString
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = RoGetActivationFactory(
            ToastNotificationManagerHString,
            &UIID_IToastNotificationManagerStatics,
            (LPVOID*)&toastStatics
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = toastStatics->lpVtbl->CreateToastNotifierWithId(
            toastStatics,
            AppIdHString,
            &notifier
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = WindowsCreateStringReference(
            RuntimeClass_Windows_UI_Notifications_ToastNotification,
            (UINT32)(sizeof(RuntimeClass_Windows_UI_Notifications_ToastNotification) / sizeof(wchar_t) - 1),
            &header_ToastNotificationHString,
            &ToastNotificationHString
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = RoGetActivationFactory(
            ToastNotificationHString,
            &UIID_IToastNotificationFactory,
            (LPVOID*)&notifFactory
        );
    }

    HANDLE hEvents[2];
    hEvents[0] = CreateEventW(NULL, FALSE, FALSE, L"EP_Ev_CheckForUpdates_" _T(EP_CLSID));
    hEvents[1] = CreateEventW(NULL, FALSE, FALSE, L"EP_Ev_InstallUpdates_" _T(EP_CLSID));
    if (hEvents[0] && hEvents[1])
    {
        if (bShowUpdateToast)
        {
            ShowUpdateSuccessNotification(hModule, notifier, notifFactory, &toast);

            HKEY hKey = NULL;

            RegCreateKeyExW(
                HKEY_CURRENT_USER,
                TEXT(REGPATH),
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_READ | KEY_WOW64_64KEY | KEY_WRITE,
                NULL,
                &hKey,
                NULL
            );
            if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
            {
                hKey = NULL;
            }
            if (hKey)
            {
                bShowUpdateToast = FALSE;
                RegSetValueExW(
                    hKey,
                    TEXT("IsUpdatePending"),
                    0,
                    REG_DWORD,
                    &bShowUpdateToast,
                    sizeof(DWORD)
                );
                RegCloseKey(hKey);
            }
        }
        if (dwUpdatePolicy != UPDATE_POLICY_MANUAL)
        {
            InstallUpdatesIfAvailable(hModule, notifier, notifFactory, &toast, UPDATES_OP_DEFAULT, bAllocConsole, dwUpdatePolicy);
        }
        DWORD dwRet = 0;
        while (TRUE)
        {
            switch (WaitForMultipleObjects(2, hEvents, FALSE, INFINITE))
            {
            case WAIT_OBJECT_0:
            {
                InstallUpdatesIfAvailable(hModule, notifier, notifFactory, &toast, UPDATES_OP_CHECK, bAllocConsole, dwUpdatePolicy);
                break;
            }
            case WAIT_OBJECT_0 + 1:
            {
                InstallUpdatesIfAvailable(hModule, notifier, notifFactory, &toast, UPDATES_OP_INSTALL, bAllocConsole, dwUpdatePolicy);
                break;
            }
            default:
            {
                break;
            }
            }
        }
        CloseHandle(hEvents[0]);
        CloseHandle(hEvents[1]);
    }

    if (toast)
    {
        toast->lpVtbl->Release(toast);
    }
    if (notifFactory)
    {
        notifFactory->lpVtbl->Release(notifFactory);
    }
    if (ToastNotificationHString)
    {
        WindowsDeleteString(ToastNotificationHString);
    }
    if (notifier)
    {
        notifier->lpVtbl->Release(notifier);
    }
    if (toastStatics)
    {
        toastStatics->lpVtbl->Release(toastStatics);
    }
    if (ToastNotificationManagerHString)
    {
        WindowsDeleteString(ToastNotificationManagerHString);
    }
    if (AppIdHString)
    {
        WindowsDeleteString(AppIdHString);
    }
}
#endif
#pragma endregion


#pragma region "Generics"
#ifdef _WIN64
HWND GetMonitorInfoFromPointForTaskbarFlyoutActivation(POINT ptCursor, DWORD dwFlags, LPMONITORINFO lpMi)
{
    HMONITOR hMonitor = MonitorFromPoint(ptCursor, dwFlags);
    HWND hWnd = NULL;
    do
    {
        hWnd = FindWindowEx(
            NULL,
            hWnd,
            L"Shell_SecondaryTrayWnd",
            NULL
        );
        if (MonitorFromWindow(hWnd, dwFlags) == hMonitor)
        {
            if (lpMi)
            {
                GetMonitorInfo(
                    MonitorFromPoint(
                        ptCursor,
                        dwFlags
                    ),
                    lpMi
                );
            }
            break;
        }
    } while (hWnd);
    if (!hWnd)
    {
        hWnd = FindWindowEx(
            NULL,
            NULL,
            L"Shell_TrayWnd",
            NULL
        );
        //ptCursor.x = 0;
        //ptCursor.y = 0;
        if (lpMi)
        {
            GetMonitorInfo(
                MonitorFromWindow(
                    hWnd,
                    dwFlags
                ),
                lpMi
            );
        }
    }
    return hWnd;
}

POINT GetDefaultWinXPosition(BOOL bUseRcWork, BOOL* lpBottom, BOOL* lpRight, BOOL bAdjust, BOOL bToRight)
{
    if (lpBottom) *lpBottom = FALSE;
    if (lpRight) *lpRight = FALSE;
    POINT point;
    point.x = 0;
    point.y = 0;
    POINT ptCursor;
    GetCursorPos(&ptCursor);
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    HWND hWnd = GetMonitorInfoFromPointForTaskbarFlyoutActivation(
        ptCursor,
        MONITOR_DEFAULTTOPRIMARY,
        &mi
    );
    if (hWnd)
    {
        RECT rc;
        GetWindowRect(hWnd, &rc);
        if (rc.left - mi.rcMonitor.left <= 0)
        {
            if (bUseRcWork)
            {
                point.x = mi.rcWork.left;
            }
            else
            {
                point.x = mi.rcMonitor.left;
            }
            if (bToRight)
            {
                point.x = mi.rcMonitor.right;
            }
            if (bAdjust)
            {
                point.x++;
            }
            if (rc.top - mi.rcMonitor.top <= 0)
            {
                if (bUseRcWork)
                {
                    point.y = mi.rcWork.top;
                }
                else
                {
                    point.y = mi.rcMonitor.top;
                }
                if (bAdjust)
                {
                    point.y++;
                }
            }
            else
            {
                if (lpBottom) *lpBottom = TRUE;
                if (bUseRcWork)
                {
                    point.y = mi.rcWork.bottom;
                }
                else
                {
                    point.y = mi.rcMonitor.bottom;
                }
                if (bAdjust)
                {
                    point.y--;
                }
            }
        }
        else
        {
            if (lpRight) *lpRight = TRUE;
            if (bUseRcWork)
            {
                point.x = mi.rcWork.right;
            }
            else
            {
                point.x = mi.rcMonitor.right;
            }
            if (bAdjust)
            {
                point.x--;
            }
            if (rc.top - mi.rcMonitor.top <= 0)
            {
                if (bUseRcWork)
                {
                    point.y = mi.rcWork.top;
                }
                else
                {
                    point.y = mi.rcMonitor.top;
                }
                if (bAdjust)
                {
                    point.y++;
                }
            }
            else
            {
                if (lpBottom) *lpBottom = TRUE;
                if (bUseRcWork)
                {
                    point.y = mi.rcWork.bottom;
                }
                else
                {
                    point.y = mi.rcMonitor.bottom;
                }
                if (bAdjust)
                {
                    point.y--;
                }
            }
        }
    }
    return point;
}

BOOL TerminateShellExperienceHost()
{
    BOOL bRet = FALSE;
    WCHAR wszKnownPath[MAX_PATH];
    GetWindowsDirectoryW(wszKnownPath, MAX_PATH);
    wcscat_s(wszKnownPath, MAX_PATH, L"\\SystemApps\\ShellExperienceHost_cw5n1h2txyewy\\ShellExperienceHost.exe");
    HANDLE hSnapshot = NULL;
    PROCESSENTRY32 pe32;
    ZeroMemory(&pe32, sizeof(PROCESSENTRY32));
    pe32.dwSize = sizeof(PROCESSENTRY32);
    hSnapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPPROCESS,
        0
    );
    if (Process32First(hSnapshot, &pe32) == TRUE)
    {
        do
        {
            if (!wcscmp(pe32.szExeFile, TEXT("ShellExperienceHost.exe")))
            {
                HANDLE hProcess = OpenProcess(
                    PROCESS_QUERY_LIMITED_INFORMATION |
                    PROCESS_TERMINATE,
                    FALSE,
                    pe32.th32ProcessID
                );
                if (hProcess)
                {
                    TCHAR wszProcessPath[MAX_PATH];
                    DWORD dwLength = MAX_PATH;
                    QueryFullProcessImageNameW(
                        hProcess,
                        0,
                        wszProcessPath,
                        &dwLength
                    );
                    if (!_wcsicmp(wszProcessPath, wszKnownPath))
                    {
                        TerminateProcess(hProcess, 0);
                        bRet = TRUE;
                    }
                    CloseHandle(hProcess);
                    hProcess = NULL;
                }
            }
        } while (Process32Next(hSnapshot, &pe32) == TRUE);
        if (hSnapshot)
        {
            CloseHandle(hSnapshot);
        }
    }
    return bRet;
}

long long elapsedCheckForeground = 0;
HANDLE hCheckForegroundThread = NULL;
DWORD CheckForegroundThread(DWORD dwMode)
{
    printf("Started \"Check foreground window\" thread.\n");
    UINT i = 0;
    while (TRUE)
    {
        wchar_t text[200];
        ZeroMemory(text, 200);
        GetClassNameW(GetForegroundWindow(), text, 200);
        if (!wcscmp(text, L"Windows.UI.Core.CoreWindow"))
        {
            break;
        }
        i++;
        if (i >= 15) break;
        Sleep(100);
    }
    while (TRUE)
    {
        wchar_t text[200];
        ZeroMemory(text, 200);
        GetClassNameW(GetForegroundWindow(), text, 200);
        if (wcscmp(text, L"Windows.UI.Core.CoreWindow"))
        {
            break;
        }
        Sleep(100);
    }
    elapsedCheckForeground = milliseconds_now();
    if (!dwMode)
    {
        RegDeleteKeyW(HKEY_CURRENT_USER, _T(SEH_REGPATH));
        TerminateShellExperienceHost();
        Sleep(100);
    }
    printf("Ended \"Check foreground window\" thread.\n");
    return 0;
}

void LaunchNetworkTargets(DWORD dwTarget)
{
    // very helpful: https://www.tenforums.com/tutorials/3123-clsid-key-guid-shortcuts-list-windows-10-a.html
    if (!dwTarget)
    {
        InvokeFlyout(INVOKE_FLYOUT_SHOW, INVOKE_FLYOUT_NETWORK);
    }
    else if (dwTarget == 5)
    {
        ShellExecuteW(
            NULL,
            L"open",
            L"ms-availablenetworks:",
            NULL,
            NULL,
            SW_SHOWNORMAL
        );
    }
    else if (dwTarget == 6)
    {
        InvokeActionCenter();
        // ShellExecuteW(
        //     NULL,
        //     L"open",
        //     L"ms-actioncenter:controlcenter/&showFooter=true",
        //     NULL,
        //     NULL,
        //     SW_SHOWNORMAL
        // );
    }
    else if (dwTarget == 1)
    {
        ShellExecuteW(
            NULL,
            L"open",
            L"ms-settings:network",
            NULL,
            NULL,
            SW_SHOWNORMAL
        );
    }
    else if (dwTarget == 2)
    {
        HMODULE hVan = LoadLibraryW(L"van.dll");
        if (hVan)
        {
            long(*ShowVAN)(BOOL, BOOL, void*) = GetProcAddress(hVan, "ShowVAN");
            if (ShowVAN)
            {
                ShowVAN(0, 0, 0);
            }
            FreeLibrary(hVan);
        }
    }
    else if (dwTarget == 3)
    {
        ShellExecuteW(
            NULL,
            L"open",
            L"shell:::{8E908FC9-BECC-40f6-915B-F4CA0E70D03D}",
            NULL,
            NULL,
            SW_SHOWNORMAL
        );
    }
    else if (dwTarget == 4)
    {
        ShellExecuteW(
            NULL,
            L"open",
            L"shell:::{7007ACC7-3202-11D1-AAD2-00805FC1270E}",
            NULL,
            NULL,
            SW_SHOWNORMAL
        );
    }
}
#endif
#pragma endregion


#pragma region "Service Window"
#ifdef _WIN64
#define EP_SERVICE_WINDOW_CLASS_NAME L"EP_Service_Window_" _T(EP_CLSID)
LRESULT CALLBACK EP_Service_Window_WndProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    if (uMsg == WM_HOTKEY && (wParam == 1 || wParam == 2))
    {
        InvokeClockFlyout();
        return 0;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
DWORD EP_ServiceWindowThread(DWORD unused)
{
    WNDCLASS wc = { 0 };
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = EP_Service_Window_WndProc;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = EP_SERVICE_WINDOW_CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hWnd = CreateWindowExW(
        0,
        EP_SERVICE_WINDOW_CLASS_NAME,
        0,
        WS_POPUP,
        0,
        0,
        0,
        0,
        0,
        0,
        GetModuleHandle(NULL),
        NULL
    );
    if (hWnd)
    {
        if (bClockFlyoutOnWinC)
        {
            RegisterHotKey(hWnd, 1, MOD_WIN | MOD_NOREPEAT, 'C');
        }
        RegisterHotKey(hWnd, 2, MOD_WIN | MOD_ALT, 'D');
        MSG msg;
        BOOL bRet;
        while ((bRet = GetMessageW(&msg, NULL, 0, 0)) != 0)
        {
            if (bRet == -1)
            {
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
        DestroyWindow(hWnd);
    }
    SetEvent(hCanStartSws);
}
#endif
#pragma endregion


#pragma region "Toggle shell features"
BOOL CALLBACK ToggleImmersiveCallback(HWND hWnd, LPARAM lParam)
{
    WORD ClassWord;

    ClassWord = GetClassWord(hWnd, GCW_ATOM);
    if (ClassWord == RegisterWindowMessageW(L"WorkerW"))
    {
        PostMessageW(hWnd, WM_HOTKEY, lParam, 0);
    }

    return TRUE;
}

void ToggleHelp()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 505, 0);
}

void ToggleRunDialog()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 502, MAKELPARAM(MOD_WIN, 0x52));
}

void ToggleSystemProperties()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 512, 0);
}

void FocusSystray()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 514, 0);
}

void TriggerAeroShake()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 515, 0);
}

void PeekDesktop()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 516, 0);
}

void ToggleEmojiPanel()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 579, 0);
}

void ShowDictationPanel()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 577, 0);
}

void ToggleClipboardViewer()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 578, 0);
}

void ToggleSearch()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 507, MAKELPARAM(MOD_WIN, 0x53));
}

void ToggleTaskView()
{
    EnumThreadWindows(GetWindowThreadProcessId(FindWindowExW(NULL, NULL, L"ApplicationManager_ImmersiveShellWindow", NULL), NULL), ToggleImmersiveCallback, 11);
}

void ToggleWidgetsPanel()
{
    EnumThreadWindows(GetWindowThreadProcessId(FindWindowExW(NULL, NULL, L"ApplicationManager_ImmersiveShellWindow", NULL), NULL), ToggleImmersiveCallback, 0x66);
}

void ToggleMainClockFlyout()
{
    EnumThreadWindows(GetWindowThreadProcessId(FindWindowExW(NULL, NULL, L"ApplicationManager_ImmersiveShellWindow", NULL), NULL), ToggleImmersiveCallback, 0x6B);
}

void ToggleNotificationsFlyout()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 591, 0);
}

void ToggleActionCenter()
{
    PostMessageW(FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL), WM_HOTKEY, 500, MAKELPARAM(MOD_WIN, 0x41));
}
#pragma endregion


#pragma region "twinui.pcshell.dll hooks"
#ifdef _WIN64
#define LAUNCHERTIP_CLASS_NAME L"LauncherTipWnd"
static INT64(*winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc)(
    void* _this,
    INT64 a2,
    INT a3
    ) = NULL;
static INT64(*CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc)(
    void* _this,
    POINT* pt
    ) = NULL;
static void(*CLauncherTipContextMenu_ExecuteCommandFunc)(
    void* _this,
    int a2
    ) = NULL;
static void(*CLauncherTipContextMenu_ExecuteShutdownCommandFunc)(
    void* _this,
    void* a2
    ) = NULL;
static INT64(*ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)(
    HMENU h1,
    HMENU h2,
    HWND a3,
    unsigned int a4,
    void* data
    ) = NULL;
static void(*ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)(
    HMENU _this,
    HMENU hWnd,
    HWND a3
    ) = NULL;
static INT64(*CLauncherTipContextMenu_GetMenuItemsAsyncFunc)(
    void* _this,
    void* rect,
    void** iunk
    ) = NULL;
static INT64(*CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc)(
    HWND hWnd,
    int a2,
    HWND a3,
    int a4,
    BOOL* a5
    ) = NULL;

LRESULT CALLBACK CLauncherTipContextMenu_WndProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    LRESULT result;

    if (hWnd == archivehWnd && !ArchiveMenuWndProc(
        hWnd, 
        uMsg, 
        wParam, 
        lParam,
        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc,
        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc
    ))
    {
        return 0;
    }

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCs = lParam;
        if (pCs->lpCreateParams)
        {
            *((HWND*)((char*)pCs->lpCreateParams + 0x78)) = hWnd;
            SetWindowLongPtr(
                hWnd, 
                GWLP_USERDATA,
                pCs->lpCreateParams
            );
            result = DefWindowProc(
                hWnd,
                uMsg,
                wParam,
                lParam
            );
        }
        else
        {
            result = DefWindowProc(
                hWnd,
                uMsg,
                wParam,
                lParam
            );
            //result = 0;
        }
    }
    else
    {
        void* _this = GetWindowLongPtr(hWnd, GWLP_USERDATA);
        BOOL v12 = FALSE;
        if ((uMsg == WM_DRAWITEM || uMsg == WM_MEASUREITEM) &&
            CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc &&
            CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc(
                hWnd,
                uMsg,
                wParam,
                lParam,
                &v12
            ))
        {
            result = 0;
        }
        else
        {
            result = DefWindowProc(
                hWnd,
                uMsg,
                wParam,
                lParam
            );
        }
        if (_this)
        {
            if (uMsg == WM_NCDESTROY)
            {
                SetWindowLongPtrW(
                    hWnd, 
                    GWLP_USERDATA,
                    0
                );
                *((HWND*)((char*)_this + 0x78)) = 0;
            }
        }
    }
    return result;
}

typedef struct
{
    void* _this;
    POINT point;
    IUnknown* iunk;
    BOOL bShouldCenterWinXHorizontally;
} ShowLauncherTipContextMenuParameters;
HWND hWinXWnd;
DWORD ShowLauncherTipContextMenu(
    ShowLauncherTipContextMenuParameters* params
)
{
    WNDCLASS wc = { 0 };
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = CLauncherTipContextMenu_WndProc;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = LAUNCHERTIP_CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClass(&wc);

    hWinXWnd = CreateWindowInBand(
        0,
        LAUNCHERTIP_CLASS_NAME,
        0,
        WS_POPUP,
        0,
        0,
        0,
        0,
        0,
        0,
        GetModuleHandle(NULL),
        (char*)params->_this - 0x58,
        7
    );
    // DO NOT USE ShowWindow here; it breaks the window order
    // and renders the desktop toggle unusable; but leave
    // SetForegroundWindow as is so that the menu gets dismissed
    // when the user clicks outside it
    // 
    // ShowWindow(hWinXWnd, SW_SHOW);
    SetForegroundWindow(hWinXWnd);

    while (!(*((HMENU*)((char*)params->_this + 0xe8))))
    {
        Sleep(1);
    }
    if (!(*((HMENU*)((char*)params->_this + 0xe8))))
    {
        goto finalize;
    }
   
    TCHAR buffer[260];
    LoadStringW(GetModuleHandleW(L"ExplorerFrame.dll"), 50222, buffer + (bNoMenuAccelerator ? 0 : 1), 260);
    if (!bNoMenuAccelerator)
    {
        buffer[0] = L'&';
    }
    wchar_t* p = wcschr(buffer, L'(');
    if (p)
    {
        p--;
        if (*p == L' ')
        {
            *p = 0;
        }
        else
        {
            p++;
            *p = 0;
        }
    }

    BOOL bCreatedMenu = FALSE;
    MENUITEMINFOW menuInfo;
    ZeroMemory(&menuInfo, sizeof(MENUITEMINFOW));
    menuInfo.cbSize = sizeof(MENUITEMINFOW);
    menuInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA;
    menuInfo.wID = 3999;
    menuInfo.dwItemData = 0;
    menuInfo.fType = MFT_STRING;
    menuInfo.dwTypeData = buffer;
    menuInfo.cch = wcslen(buffer);
    if (bPropertiesInWinX)
    {
        InsertMenuItemW(
            *((HMENU*)((char*)params->_this + 0xe8)),
            GetMenuItemCount(*((HMENU*)((char*)params->_this + 0xe8))) - 1,
            TRUE,
            &menuInfo
        );
        bCreatedMenu = TRUE;
    }

    INT64* unknown_array = NULL;
    if (bSkinMenus)
    {
        unknown_array = calloc(4, sizeof(INT64));
        if (ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)
        {
            ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                *((HMENU*)((char*)params->_this + 0xe8)),
                hWinXWnd,
                &(params->point),
                0xc,
                unknown_array
            );
        }
    }

    BOOL res = TrackPopupMenu(
        *((HMENU*)((char*)params->_this + 0xe8)),
        TPM_RETURNCMD | TPM_RIGHTBUTTON | (params->bShouldCenterWinXHorizontally ? TPM_CENTERALIGN : 0),
        params->point.x,
        params->point.y,
        0,
        hWinXWnd,
        0
    );

    if (bSkinMenus)
    {
        if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
        {
            ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                *((HMENU*)((char*)params->_this + 0xe8)),
                hWinXWnd,
                &(params->point)
            );
        }
        free(unknown_array);
    }

    if (bCreatedMenu)
    {
        RemoveMenu(
            *((HMENU*)((char*)params->_this + 0xe8)),
            3999,
            MF_BYCOMMAND
        );
    }

    if (res > 0)
    {
        if (bCreatedMenu && res == 3999)
        {
            LaunchPropertiesGUI(hModule);
        }
        else if (res < 4000)
        {
            INT64 info = *(INT64*)((char*)(*(INT64*)((char*)params->_this + 0xa8 - 0x58)) + (INT64)res * 8 - 8);
            if (CLauncherTipContextMenu_ExecuteCommandFunc)
            {
                CLauncherTipContextMenu_ExecuteCommandFunc(
                    (char*)params->_this - 0x58,
                    &info
                );
            }
        }
        else
        {
            INT64 info = *(INT64*)((char*)(*(INT64*)((char*)params->_this + 0xc8 - 0x58)) + ((INT64)res - 4000) * 8);
            if (CLauncherTipContextMenu_ExecuteShutdownCommandFunc)
            {
                CLauncherTipContextMenu_ExecuteShutdownCommandFunc(
                    (char*)params->_this - 0x58,
                    &info
                );
            }
        }
    }

finalize:
    params->iunk->lpVtbl->Release(params->iunk);
    SendMessage(
        hWinXWnd,
        WM_CLOSE,
        0,
        0
    );
    free(params);
    hIsWinXShown = NULL;
    return 0;
}

INT64 CLauncherTipContextMenu_ShowLauncherTipContextMenuHook(
    void* _this,
    POINT* pt
)
{
    if (hWinXThread)
    {
        WaitForSingleObject(hWinXThread, INFINITE);
        CloseHandle(hWinXThread);
        hWinXThread = NULL;
    }

    if (hIsWinXShown)
    {
        goto finalize;
    }

    BOOL bShouldCenterWinXHorizontally = FALSE;
    POINT point;
    if (pt)
    {
        point = *pt;
        BOOL bBottom, bRight;
        POINT dPt = GetDefaultWinXPosition(FALSE, &bBottom, &bRight, FALSE, FALSE);
        POINT posCursor;
        GetCursorPos(&posCursor);
        RECT rcHitZone;
        rcHitZone.left = pt->x - 5;
        rcHitZone.right = pt->x + 5;
        rcHitZone.top = pt->y - 5;
        rcHitZone.bottom = pt->y + 5;
        //printf("%d %d = %d %d %d %d\n", posCursor.x, posCursor.y, rcHitZone.left, rcHitZone.right, rcHitZone.top, rcHitZone.bottom);
        if (bBottom && IsThemeActive() && PtInRect(&rcHitZone, posCursor))
        {
            HMONITOR hMonitor = MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY);
            MONITORINFO mi;
            mi.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo(hMonitor, &mi);
            HWND hWndUnder = WindowFromPoint(*pt);
            TCHAR wszClassName[100];
            ZeroMemory(wszClassName, 100);
            GetClassNameW(hWndUnder, wszClassName, 100);
            if (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd"))
            {
                hWndUnder = FindWindowEx(
                    hWndUnder,
                    NULL,
                    L"Start",
                    NULL
                );
            }
            RECT rcUnder;
            GetWindowRect(hWndUnder, &rcUnder);
            if (mi.rcMonitor.left != rcUnder.left)
            {
                bShouldCenterWinXHorizontally = TRUE;
                point.x = rcUnder.left + (rcUnder.right - rcUnder.left) / 2;
                point.y = rcUnder.top;
            }
            else
            {
                UINT dpiX, dpiY;
                HRESULT hr = GetDpiForMonitor(
                    hMonitor,
                    MDT_DEFAULT,
                    &dpiX,
                    &dpiY
                );
                double dx = dpiX / 96.0, dy = dpiY / 96.0;
                BOOL xo = FALSE, yo = FALSE;
                if (point.x - WINX_ADJUST_X * dx < mi.rcMonitor.left)
                {
                    xo = TRUE;
                }
                if (point.y + WINX_ADJUST_Y * dy > mi.rcMonitor.bottom)
                {
                    yo = TRUE;
                }
                POINT ptCursor;
                GetCursorPos(&ptCursor);
                if (xo)
                {
                    ptCursor.x += (WINX_ADJUST_X * 2) * dx;
                }
                else
                {
                    point.x -= WINX_ADJUST_X * dx;
                }
                if (yo)
                {
                    ptCursor.y -= (WINX_ADJUST_Y * 2) * dy;
                }
                else
                {
                    point.y += WINX_ADJUST_Y * dy;
                }
                SetCursorPos(ptCursor.x, ptCursor.y);
            }
        }
    }
    else
    {
        point = GetDefaultWinXPosition(FALSE, NULL, NULL, TRUE, FALSE);
    }

    IUnknown* iunk = NULL;
    if (CLauncherTipContextMenu_GetMenuItemsAsyncFunc)
    {
        CLauncherTipContextMenu_GetMenuItemsAsyncFunc(
            _this,
            &point,
            &iunk
        );
    }
    if (iunk)
    {
        iunk->lpVtbl->AddRef(iunk);

        ShowLauncherTipContextMenuParameters* params = malloc(
            sizeof(ShowLauncherTipContextMenuParameters)
        );
        params->_this = _this;
        params->point = point;
        params->iunk = iunk;
        params->bShouldCenterWinXHorizontally = bShouldCenterWinXHorizontally;
        hIsWinXShown = CreateThread(
            0,
            0,
            ShowLauncherTipContextMenu,
            params,
            0,
            0
        );
        hWinXThread = hIsWinXShown;
    }
finalize:
    if (CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc)
    {
        return CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc(_this, pt);
    }
    return 0;
}
#endif
#pragma endregion


#ifdef _WIN64
#pragma region "Windows 10 Taskbar Hooks"
// credits: https://github.com/m417z/7-Taskbar-Tweaker

DEFINE_GUID(IID_ITaskGroup,
    0x3af85589, 0x678f, 0x4fb5, 0x89, 0x25, 0x5a, 0x13, 0x4e, 0xbf, 0x57, 0x2c);

typedef interface ITaskGroup ITaskGroup;

typedef struct ITaskGroupVtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        ITaskGroup* This,
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        ITaskGroup* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        ITaskGroup* This);

    HRESULT(STDMETHODCALLTYPE* Initialize)(
        ITaskGroup* This);

    HRESULT(STDMETHODCALLTYPE* AddTaskItem)(
        ITaskGroup* This);

    HRESULT(STDMETHODCALLTYPE* RemoveTaskItem)(
        ITaskGroup* This);

    HRESULT(STDMETHODCALLTYPE* EnumTaskItems)(
        ITaskGroup* This);

    HRESULT(STDMETHODCALLTYPE* DoesWindowMatch)(
        ITaskGroup* This,
        HWND hCompareWnd,
        ITEMIDLIST* pCompareItemIdList,
        WCHAR* pCompareAppId,
        int* pnMatch,
        LONG** p_task_item);
    // ...

    END_INTERFACE
} ITaskGroupVtbl;

interface ITaskGroup
{
    CONST_VTBL struct ITaskGroupVtbl* lpVtbl;
};

HRESULT(*CTaskGroup_DoesWindowMatchFunc)(LONG_PTR* task_group, HWND hCompareWnd, ITEMIDLIST* pCompareItemIdList,
    WCHAR* pCompareAppId, int* pnMatch, LONG_PTR** p_task_item) = NULL;
HRESULT __stdcall CTaskGroup_DoesWindowMatchHook(LONG_PTR* task_group, HWND hCompareWnd, ITEMIDLIST* pCompareItemIdList,
    WCHAR* pCompareAppId, int* pnMatch, LONG_PTR** p_task_item)
{
    HRESULT hr = CTaskGroup_DoesWindowMatchFunc(task_group, hCompareWnd, pCompareItemIdList, pCompareAppId, pnMatch, p_task_item);
    BOOL bDontGroup = FALSE;
    BOOL bPinned = FALSE;
    if (bPinnedItemsActAsQuickLaunch && SUCCEEDED(hr) && *pnMatch >= 1 && *pnMatch <= 3) // itemlist or appid match
    {
        bDontGroup = FALSE;
        bPinned = (!task_group[4] || (int)((LONG_PTR*)task_group[4])[0] == 0);
        if (bPinned)
        {
            bDontGroup = TRUE;
        }
        if (bDontGroup)
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

DEFINE_GUID(IID_ITaskBtnGroup,
    0x2e52265d, 0x1a3b, 0x4e46, 0x94, 0x17, 0x51, 0xa5, 0x9c, 0x47, 0xd6, 0x0b);

typedef interface ITaskBtnGroup ITaskBtnGroup;

typedef struct ITaskBtnGroupVtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        ITaskBtnGroup* This,
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        ITaskBtnGroup* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        ITaskBtnGroup* This);

    HRESULT(STDMETHODCALLTYPE* Shutdown)(
        ITaskBtnGroup* This);

    HRESULT(STDMETHODCALLTYPE* GetGroupType)(
        ITaskBtnGroup* This);

    HRESULT(STDMETHODCALLTYPE* UpdateGroupType)(
        ITaskBtnGroup* This);

    HRESULT(STDMETHODCALLTYPE* GetGroup)(
        ITaskBtnGroup* This);

    HRESULT(STDMETHODCALLTYPE* AddTaskItem)(
        ITaskBtnGroup* This);

    HRESULT(STDMETHODCALLTYPE* IndexOfTaskItem)(
        ITaskBtnGroup* This);

    HRESULT(STDMETHODCALLTYPE* RemoveTaskItem)(
        ITaskBtnGroup* This);

    HRESULT(STDMETHODCALLTYPE* RealityCheck)(
        ITaskBtnGroup* This);

    HRESULT(STDMETHODCALLTYPE* IsItemBeingRemoved)(
        ITaskBtnGroup* This);

    HRESULT(STDMETHODCALLTYPE* CancelRemoveItem)(
        ITaskBtnGroup* This);

    LONG_PTR(STDMETHODCALLTYPE* GetIdealSpan)(
        ITaskBtnGroup* This,
        LONG_PTR var2, 
        LONG_PTR var3,
        LONG_PTR var4, 
        LONG_PTR var5, 
        LONG_PTR var6);
    // ...

    END_INTERFACE
} ITaskBtnGroupVtbl;

interface ITaskBtnGroup
{
    CONST_VTBL struct ITaskBtnGroupVtbl* lpVtbl;
};

LONG_PTR (*CTaskBtnGroup_GetIdealSpanFunc)(ITaskBtnGroup* _this, LONG_PTR var2, LONG_PTR var3,
    LONG_PTR var4, LONG_PTR var5, LONG_PTR var6) = NULL;
LONG_PTR __stdcall CTaskBtnGroup_GetIdealSpanHook(ITaskBtnGroup* _this, LONG_PTR var2, LONG_PTR var3,
    LONG_PTR var4, LONG_PTR var5, LONG_PTR var6)
{
    LONG_PTR ret = NULL;
    BOOL bTypeModified = FALSE;
    int button_group_type = *(unsigned int*)((INT64)_this + 64);
    if (bRemoveExtraGapAroundPinnedItems && button_group_type == 2)
    {
        *(unsigned int*)((INT64)_this + 64) = 4;
        bTypeModified = TRUE;
    }
    ret = CTaskBtnGroup_GetIdealSpanFunc(_this, var2, var3, var4, var5, var6);
    if (bRemoveExtraGapAroundPinnedItems && bTypeModified)
    {
        *(unsigned int*)((INT64)_this + 64) = button_group_type;
    }
    return ret;
}

void explorer_QISearch(void* that, LPCQITAB pqit, REFIID riid, void** ppv)
{
    HRESULT hr = QISearch(that, pqit, riid, ppv);
    if (SUCCEEDED(hr) && IsEqualGUID(pqit[0].piid, &IID_ITaskGroup))
    {
        ITaskGroup* pTaskGroup = (char*)that + pqit[0].dwOffset;
        DWORD flOldProtect = 0;
        if (VirtualProtect(pTaskGroup->lpVtbl, sizeof(ITaskGroupVtbl), PAGE_EXECUTE_READWRITE, &flOldProtect))
        {
            if (!CTaskGroup_DoesWindowMatchFunc)
            {
                CTaskGroup_DoesWindowMatchFunc = pTaskGroup->lpVtbl->DoesWindowMatch;
            }
            pTaskGroup->lpVtbl->DoesWindowMatch = CTaskGroup_DoesWindowMatchHook;
            VirtualProtect(pTaskGroup->lpVtbl, sizeof(ITaskGroupVtbl), flOldProtect, &flOldProtect);
        }
    }
    else if (SUCCEEDED(hr) && IsEqualGUID(pqit[0].piid, &IID_ITaskBtnGroup))
    {
        ITaskBtnGroup* pTaskBtnGroup = (char*)that + pqit[0].dwOffset;
        DWORD flOldProtect = 0;
        if (VirtualProtect(pTaskBtnGroup->lpVtbl, sizeof(ITaskBtnGroupVtbl), PAGE_EXECUTE_READWRITE, &flOldProtect))
        {
            if (!CTaskBtnGroup_GetIdealSpanFunc)
            {
                CTaskBtnGroup_GetIdealSpanFunc = pTaskBtnGroup->lpVtbl->GetIdealSpan;
            }
            pTaskBtnGroup->lpVtbl->GetIdealSpan = CTaskBtnGroup_GetIdealSpanHook;
            VirtualProtect(pTaskBtnGroup->lpVtbl, sizeof(ITaskBtnGroupVtbl), flOldProtect, &flOldProtect);
        }
    }
    return hr;
}
#pragma endregion
#endif


#pragma region "Show Start in correct location according to TaskbarAl"
#ifdef _WIN64
void UpdateStartMenuPositioning(LPARAM loIsShouldInitializeArray_hiIsShouldRoInitialize)
{
    BOOL bShouldInitialize = LOWORD(loIsShouldInitializeArray_hiIsShouldRoInitialize);
    BOOL bShouldRoInitialize = HIWORD(loIsShouldInitializeArray_hiIsShouldRoInitialize);

    if (!bOldTaskbar)
    {
        return;
    }

    DWORD dwPosCurrent = GetStartMenuPosition(SHRegGetValueFromHKCUHKLMFunc);
    if (bShouldInitialize || InterlockedAdd(&dwTaskbarAl, 0) != dwPosCurrent)
    {
        HRESULT hr = S_OK;
        if (bShouldRoInitialize)
        {
            hr = RoInitialize(RO_INIT_MULTITHREADED);
        }
        if (SUCCEEDED(hr))
        {
            InterlockedExchange(&dwTaskbarAl, dwPosCurrent);
            StartMenuPositioningData spd;
            spd.pMonitorCount = &dwMonitorCount;
            spd.pMonitorList = hMonitorList;
            spd.location = dwPosCurrent;
            if (bShouldInitialize)
            {
                spd.operation = STARTMENU_POSITIONING_OPERATION_REMOVE;
                unsigned int k = InterlockedAdd(&dwMonitorCount, 0);
                for (unsigned int i = 0; i < k; ++i)
                {
                    NeedsRo_PositionStartMenuForMonitor(hMonitorList[i], NULL, NULL, &spd);
                }
                InterlockedExchange(&dwMonitorCount, 0);
                spd.operation = STARTMENU_POSITIONING_OPERATION_ADD;
            }
            else
            {
                spd.operation = STARTMENU_POSITIONING_OPERATION_CHANGE;
            }
            EnumDisplayMonitors(NULL, NULL, NeedsRo_PositionStartMenuForMonitor, &spd);
            if (bShouldRoInitialize)
            {
                RoUninitialize();
            }
        }
    }
}
#else
void UpdateStartMenuPositioning(LPARAM loIsShouldInitializeArray_hiIsShouldRoInitialize) {}
#endif
#pragma endregion


#pragma region "Fix Windows 11 taskbar not showing tray when auto hide is on"
#ifdef _WIN64
#define FIXTASKBARAUTOHIDE_CLASS_NAME L"FixTaskbarAutohide_" _T(EP_CLSID)
LRESULT CALLBACK FixTaskbarAutohide_WndProc(
    HWND hWnd,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam)
{
    static UINT s_uTaskbarRestart;

    switch (uMessage)
    {
    case WM_CREATE:
        s_uTaskbarRestart = RegisterWindowMessageW(TEXT("TaskbarCreated"));
        break;

    default:
        if (uMessage == s_uTaskbarRestart)
            PostQuitMessage(0);
        break;
    }

    return DefWindowProcW(hWnd, uMessage, wParam, lParam);
}
DWORD FixTaskbarAutohide(DWORD unused)
{
    WNDCLASS wc = { 0 };
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = FixTaskbarAutohide_WndProc;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = FIXTASKBARAUTOHIDE_CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hWnd = CreateWindowExW(
        0,
        FIXTASKBARAUTOHIDE_CLASS_NAME,
        0,
        WS_POPUP,
        0,
        0,
        0,
        0,
        0,
        0,
        GetModuleHandle(NULL),
        NULL
    );
    if (hWnd)
    {
        MSG msg;
        BOOL bRet;
        while ((bRet = GetMessageW(&msg, NULL, 0, 0)) != 0)
        {
            if (bRet == -1)
            {
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
        DestroyWindow(hWnd);

        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        if (SHAppBarMessage(ABM_GETSTATE, &abd) == ABS_AUTOHIDE)
        {
            abd.lParam = 0;
            SHAppBarMessage(ABM_SETSTATE, &abd);
            Sleep(1000);
            abd.lParam = ABS_AUTOHIDE;
            SHAppBarMessage(ABM_SETSTATE, &abd);
        }
    }
    SetEvent(hCanStartSws);
}
#endif
#pragma endregion


#pragma region "Shell_TrayWnd subclass"
#ifdef _WIN64
HMENU explorer_LoadMenuW(HINSTANCE hInstance, LPCWSTR lpMenuName)
{
    HMENU hMenu = LoadMenuW(hInstance, lpMenuName);
    if (hInstance == GetModuleHandle(NULL) && lpMenuName == MAKEINTRESOURCEW(205))
    {
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        if (hSubMenu)
        {
            TCHAR buffer[260];
            LoadStringW(GetModuleHandleW(L"ExplorerFrame.dll"), 50222, buffer + (bNoMenuAccelerator ? 0 : 1), 260);
            if (!bNoMenuAccelerator)
            {
                buffer[0] = L'&';
            }
            wchar_t* p = wcschr(buffer, L'(');
            if (p)
            {
                p--;
                if (*p == L' ')
                {
                    *p = 0;
                }
                else
                {
                    p++;
                    *p = 0;
                }
            }
            MENUITEMINFOW menuInfo;
            ZeroMemory(&menuInfo, sizeof(MENUITEMINFOW));
            menuInfo.cbSize = sizeof(MENUITEMINFOW);
            menuInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA;
            menuInfo.wID = 12100;
            menuInfo.dwItemData = CheckForUpdatesThread;
            menuInfo.fType = MFT_STRING;
            menuInfo.dwTypeData = buffer;
            menuInfo.cch = wcslen(buffer);
            if (!bNoPropertiesInContextMenu)
            {
                InsertMenuItemW(
                    hSubMenu,
                    GetMenuItemCount(hSubMenu) - 4,
                    TRUE,
                    &menuInfo
                );
            }
        }
    }
    return hMenu;
}

HHOOK Shell_TrayWndMouseHook = NULL;

BOOL IsPointOnEmptyAreaOfNewTaskbar(POINT pt)
{
    HRESULT hr = S_OK;
    IUIAutomation2* pIUIAutomation2 = NULL;
    IUIAutomationElement* pIUIAutomationElement = NULL;
    HWND hWnd = NULL;
    BOOL bRet = FALSE;
    
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(&CLSID_CUIAutomation8, NULL, CLSCTX_INPROC_SERVER, &IID_IUIAutomation2, &pIUIAutomation2);
    }
    if (SUCCEEDED(hr))
    {
        hr = pIUIAutomation2->lpVtbl->ElementFromPoint(pIUIAutomation2, pt, &pIUIAutomationElement);
    }
    if (SUCCEEDED(hr))
    {
        hr = pIUIAutomationElement->lpVtbl->get_CurrentNativeWindowHandle(pIUIAutomationElement, &hWnd);
    }
    if (SUCCEEDED(hr))
    {
        WCHAR wszClassName[200];
        GetClassNameW(hWnd, wszClassName, 200);
        if (IsWindow(hWnd))
        {
            if (!wcscmp(wszClassName, L"Windows.UI.Input.InputSite.WindowClass"))
            {
                HWND hAncestor = GetAncestor(hWnd, GA_ROOT);
                HWND hWindow = FindWindowExW(hAncestor, NULL, L"Windows.UI.Composition.DesktopWindowContentBridge", NULL);
                if (IsWindow(hWindow))
                {
                    hWindow = FindWindowExW(hWindow, NULL, L"Windows.UI.Input.InputSite.WindowClass", NULL);
                    if (IsWindow(hWindow))
                    {
                        if (hWindow == hWnd)
                        {
                            bRet = TRUE;
                        }
                    }
                }
            }
            else if (!wcscmp(wszClassName, L"MSTaskListWClass"))
            {
                IUIAutomationTreeWalker* pControlWalker = NULL;
                IUIAutomationElement* pTaskbarButton = NULL;
                IUIAutomationElement* pNextTaskbarButton = NULL;
                RECT rc;
                if (SUCCEEDED(hr))
                {
                    hr = pIUIAutomation2->lpVtbl->get_RawViewWalker(pIUIAutomation2, &pControlWalker);
                }
                if (SUCCEEDED(hr) && pControlWalker)
                {
                    hr = pControlWalker->lpVtbl->GetFirstChildElement(pControlWalker, pIUIAutomationElement, &pTaskbarButton);
                }
                BOOL bValid = TRUE, bFirst = TRUE;
                while (SUCCEEDED(hr) && pTaskbarButton)
                {
                    pControlWalker->lpVtbl->GetNextSiblingElement(pControlWalker, pTaskbarButton, &pNextTaskbarButton);
                    SetRect(&rc, 0, 0, 0, 0);
                    pTaskbarButton->lpVtbl->get_CurrentBoundingRectangle(pTaskbarButton, &rc);
                    if (bFirst)
                    {
                        // Account for Start button as well
                        rc.left -= (rc.right - rc.left);
                        bFirst = FALSE;
                    }
                    //printf("PT %d %d RECT %d %d %d %d\n", pt.x, pt.y, rc.left, rc.top, rc.right, rc.bottom);
                    if (pNextTaskbarButton && PtInRect(&rc, pt))
                    {
                        bValid = FALSE;
                    }
                    pTaskbarButton->lpVtbl->Release(pTaskbarButton);
                    pTaskbarButton = pNextTaskbarButton;
                }
                //printf("IS VALID %d\n", bValid);
                //printf("\n");
                if (pControlWalker)
                {
                    pControlWalker->lpVtbl->Release(pControlWalker);
                }
                if (bValid)
                {
                    HWND hAncestor = GetAncestor(hWnd, GA_ROOT);
                    HWND hWindow = FindWindowExW(hAncestor, NULL, L"WorkerW", NULL);
                    if (IsWindow(hWindow))
                    {
                        hWindow = FindWindowExW(hWindow, NULL, L"MSTaskListWClass", NULL);
                        if (IsWindow(hWindow))
                        {
                            if (hWindow == hWnd)
                            {
                                bRet = TRUE;
                            }
                        }
                    }
                }
            }
        }
    }
    if (pIUIAutomationElement)
    {
        pIUIAutomationElement->lpVtbl->Release(pIUIAutomationElement);
    }
    if (pIUIAutomation2)
    {
        pIUIAutomation2->lpVtbl->Release(pIUIAutomation2);
    }
    return bRet;
}

long long TaskbarLeftClickTime = 0;
BOOL bTaskbarLeftClickEven = FALSE;
LRESULT CALLBACK Shell_TrayWndMouseProc(
    _In_ int    nCode,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    if (!bOldTaskbar && 
        nCode == HC_ACTION && 
        wParam == WM_RBUTTONUP && 
        IsPointOnEmptyAreaOfNewTaskbar(((MOUSEHOOKSTRUCT*)lParam)->pt)
        )
    {
        PostMessageW(
            FindWindowW(L"Shell_TrayWnd", NULL),
            RegisterWindowMessageW(L"Windows11ContextMenu_" _T(EP_CLSID)),
            0,
            MAKELPARAM(((MOUSEHOOKSTRUCT*)lParam)->pt.x, ((MOUSEHOOKSTRUCT*)lParam)->pt.y)
        );
        return 1;
    }
    if (!bOldTaskbar &&
        bTaskbarAutohideOnDoubleClick && 
        nCode == HC_ACTION && 
        wParam == WM_LBUTTONUP &&
        IsPointOnEmptyAreaOfNewTaskbar(((MOUSEHOOKSTRUCT*)lParam)->pt)
        )
    {
        /*BOOL bShouldCheck = FALSE;
        if (bOldTaskbar)
        {
            WCHAR cn[200];
            GetClassNameW(((MOUSEHOOKSTRUCT*)lParam)->hwnd, cn, 200);
            wprintf(L"%s\n", cn);
            bShouldCheck = !wcscmp(cn, L"Shell_SecondaryTrayWnd"); // !wcscmp(cn, L"Shell_TrayWnd")
        }
        else
        {
            bShouldCheck = IsPointOnEmptyAreaOfNewTaskbar(((MOUSEHOOKSTRUCT*)lParam)->pt);
        }
        if (bShouldCheck)
        {*/
            if (bTaskbarLeftClickEven)
            {
                if (TaskbarLeftClickTime != 0)
                {
                    TaskbarLeftClickTime = milliseconds_now() - TaskbarLeftClickTime;
                }
                if (TaskbarLeftClickTime != 0 && TaskbarLeftClickTime < GetDoubleClickTime())
                {
                    TaskbarLeftClickTime = 0;
                    ToggleTaskbarAutohide();
                }
                else
                {
                    TaskbarLeftClickTime = milliseconds_now();
                }
            }
            bTaskbarLeftClickEven = !bTaskbarLeftClickEven;
        //}
    }
    return CallNextHookEx(Shell_TrayWndMouseHook, nCode, wParam, lParam);
}

INT64 Shell_TrayWndSubclassProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    UINT_PTR    uIdSubclass,
    DWORD_PTR   bIsPrimaryTaskbar
)
{
    if (uMsg == WM_NCDESTROY)
    {
        if (bIsPrimaryTaskbar)
        {
            UnhookWindowsHookEx(Shell_TrayWndMouseHook);
        }
        RemoveWindowSubclass(hWnd, Shell_TrayWndSubclassProc, Shell_TrayWndSubclassProc);
    }
    else if (!bIsPrimaryTaskbar && uMsg == WM_CONTEXTMENU)
    {
        // Received some times when right clicking a secondary taskbar button, and it would
        // show the classic taskbar context menu but containing only "Show desktop" instead
        // of ours or a button's jump list, so we cancel it and that seems to properly invoke
        // the right menu
        return 0;
    }
    else if (!bOldTaskbar && !bIsPrimaryTaskbar && uMsg == WM_SETCURSOR)
    {
        // Received when mouse is over taskbar edge and autohide is on
        PostMessageW(hWnd, WM_ACTIVATE, WA_ACTIVE, NULL);
    }
    else if (bOldTaskbar && uMsg == WM_LBUTTONDBLCLK && bTaskbarAutohideOnDoubleClick)
    {
        ToggleTaskbarAutohide();
        return 0;
    }
    else if (uMsg == WM_HOTKEY && wParam == 500 && lParam == MAKELPARAM(MOD_WIN, 0x41))
    {
        InvokeActionCenter();
        return 0;
        /*if (lpShouldDisplayCCButton)
        {
            *lpShouldDisplayCCButton = 1;
        }
        LRESULT lRes = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        if (lpShouldDisplayCCButton)
        {
            *lpShouldDisplayCCButton = bHideControlCenterButton;
        }
        return lRes;*/
    }
    else if (bIsPrimaryTaskbar && uMsg == WM_DISPLAYCHANGE)
    {
        UpdateStartMenuPositioning(MAKELPARAM(TRUE, FALSE));
    }
    //else if (!bOldTaskbar && uMsg == WM_PARENTNOTIFY && wParam == WM_RBUTTONDOWN && !Shell_TrayWndMouseHook) // && !IsUndockingDisabled
    //{
    //    DWORD dwThreadId = GetCurrentThreadId();
    //    Shell_TrayWndMouseHook = SetWindowsHookExW(WH_MOUSE, Shell_TrayWndMouseProc, NULL, dwThreadId);
    //}
    else if (uMsg == RegisterWindowMessageW(L"Windows11ContextMenu_" _T(EP_CLSID)))
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        HMENU hMenu = LoadMenuW(GetModuleHandle(NULL), MAKEINTRESOURCEW(205));
        if (hMenu)
        {
            HMENU hSubMenu = GetSubMenu(hMenu, 0);
            if (hSubMenu)
            {
                if (GetAsyncKeyState(VK_SHIFT) >= 0 || GetAsyncKeyState(VK_CONTROL) >= 0)
                {
                    DeleteMenu(hSubMenu, 518, MF_BYCOMMAND); // Exit Explorer
                }
                DeleteMenu(hSubMenu, 424, MF_BYCOMMAND); // Lock the taskbar
                DeleteMenu(hSubMenu, 425, MF_BYCOMMAND); // Lock all taskbars
                DeleteMenu(hSubMenu, 416, MF_BYCOMMAND); // Undo
                DeleteMenu(hSubMenu, 437, MF_BYCOMMAND); // Show Pen button
                DeleteMenu(hSubMenu, 438, MF_BYCOMMAND); // Show touchpad button
                DeleteMenu(hSubMenu, 435, MF_BYCOMMAND); // Show People on the taskbar
                DeleteMenu(hSubMenu, 430, MF_BYCOMMAND); // Show Task View button
                DeleteMenu(hSubMenu, 449, MF_BYCOMMAND); // Show Cortana button
                DeleteMenu(hSubMenu, 621, MF_BYCOMMAND); // News and interests
                DeleteMenu(hSubMenu, 445, MF_BYCOMMAND); // Cortana
                DeleteMenu(hSubMenu, 431, MF_BYCOMMAND); // Search
                DeleteMenu(hSubMenu, 421, MF_BYCOMMAND); // Customize notification icons
                DeleteMenu(hSubMenu, 408, MF_BYCOMMAND); // Adjust date/time
                DeleteMenu(hSubMenu, 436, MF_BYCOMMAND); // Show touch keyboard button
                DeleteMenu(hSubMenu, 0, MF_BYPOSITION); // Separator
                DeleteMenu(hSubMenu, 0, MF_BYPOSITION); // Separator

                TCHAR buffer[260];
                LoadStringW(GetModuleHandleW(L"ExplorerFrame.dll"), 50222, buffer + (bNoMenuAccelerator ? 0 : 1), 260);
                if (!bNoMenuAccelerator)
                {
                    buffer[0] = L'&';
                }
                wchar_t* p = wcschr(buffer, L'(');
                if (p)
                {
                    p--;
                    if (*p == L' ')
                    {
                        *p = 0;
                    }
                    else
                    {
                        p++;
                        *p = 0;
                    }
                }
                MENUITEMINFOW menuInfo;
                ZeroMemory(&menuInfo, sizeof(MENUITEMINFOW));
                menuInfo.cbSize = sizeof(MENUITEMINFOW);
                menuInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA | MIIM_STATE;
                menuInfo.wID = 3999;
                menuInfo.dwItemData = 0;
                menuInfo.fType = MFT_STRING;
                menuInfo.dwTypeData = buffer;
                menuInfo.cch = wcslen(buffer);
                if (!bNoPropertiesInContextMenu)
                {
                    InsertMenuItemW(
                        hSubMenu,
                        GetMenuItemCount(hSubMenu) - 1,
                        TRUE,
                        &menuInfo
                    );
                }

                INT64* unknown_array = NULL;
                if (bSkinMenus)
                {
                    unknown_array = calloc(4, sizeof(INT64));
                    if (ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)
                    {
                        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                            hSubMenu,
                            hWnd,
                            &pt,
                            0xc,
                            unknown_array
                        );
                    }
                }

                BOOL res = TrackPopupMenu(
                    hSubMenu,
                    TPM_RETURNCMD | TPM_RIGHTBUTTON,
                    pt.x,
                    pt.y,
                    0,
                    hWnd,
                    0
                );
                if (res == 3999)
                {
                    LaunchPropertiesGUI(hModule);
                }
                else if (res == 403)
                {
                    CascadeWindows(NULL, 0, NULL, 0, NULL);
                }
                else if (res == 404)
                {
                    TileWindows(NULL, 0, NULL, 0, NULL);
                }
                else if (res == 405)
                {
                    TileWindows(NULL, 1, NULL, 0, NULL);
                }
                else
                {
                    PostMessageW(hWnd, WM_COMMAND, res, 0);
                }

                if (bSkinMenus)
                {
                    if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
                    {
                        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                            hSubMenu,
                            hWnd,
                            &pt
                        );
                    }
                    free(unknown_array);
                }

                DestroyMenu(hSubMenu);
            }
            DestroyMenu(hMenu);
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
#endif
#pragma endregion


#pragma region "Allow legacy volume applet"
#ifdef _WIN64
LSTATUS sndvolsso_RegGetValueW(
    HKEY    hkey,
    LPCWSTR lpSubKey,
    LPCWSTR lpValue,
    DWORD   dwFlags,
    LPDWORD pdwType,
    PVOID   pvData,
    LPDWORD pcbData
)
{
    if (SHRegGetValueFromHKCUHKLMFunc &&
        hkey == HKEY_LOCAL_MACHINE &&
        !_wcsicmp(lpSubKey, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\MTCUVC") &&
        !_wcsicmp(lpValue, L"EnableMTCUVC"))
    {
        return SHRegGetValueFromHKCUHKLMFunc(
            lpSubKey,
            lpValue,
            SRRF_RT_REG_DWORD,
            pdwType,
            pvData,
            pcbData
        );
    }
    return RegGetValueW(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
}
#endif
#pragma endregion


#pragma region "Allow legacy date and time"
#ifdef _WIN64
DEFINE_GUID(GUID_Win32Clock,
    0x0A323554A,
    0x0FE1, 0x4E49, 0xae, 0xe1,
    0x67, 0x22, 0x46, 0x5d, 0x79, 0x9f
);
DEFINE_GUID(IID_Win32Clock,
    0x7A5FCA8A,
    0x76B1, 0x44C8, 0xa9, 0x7c,
    0xe7, 0x17, 0x3c, 0xca, 0x5f, 0x4f
);
typedef interface Win32Clock Win32Clock;

typedef struct Win32ClockVtbl
{
    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            Win32Clock* This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        Win32Clock* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        Win32Clock* This);

    HRESULT(STDMETHODCALLTYPE* ShowWin32Clock)(
        Win32Clock* This,
        /* [in] */ HWND hWnd,
        /* [in] */ LPRECT lpRect);

    END_INTERFACE
} Win32ClockVtbl;

interface Win32Clock
{
    CONST_VTBL struct Win32ClockVtbl* lpVtbl;
};
DWORD ShouldShowLegacyClockExperience()
{
    DWORD dwVal = 0, dwSize = sizeof(DWORD);
    if (SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ImmersiveShell"),
        TEXT("UseWin32TrayClockExperience"),
        SRRF_RT_REG_DWORD,
        NULL,
        &dwVal,
        (LPDWORD)(&dwSize)
    ) == ERROR_SUCCESS)
    {
        return dwVal;
    }
    return 0;
}
BOOL ShowLegacyClockExperience(HWND hWnd)
{
    if (!hWnd)
    {
        return FALSE;
    }
    HRESULT hr = S_OK;
    Win32Clock* pWin32Clock = NULL;
    hr = CoCreateInstance(
        &GUID_Win32Clock,
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
        &IID_Win32Clock,
        &pWin32Clock
    );
    if (SUCCEEDED(hr))
    {
        RECT rc;
        GetWindowRect(hWnd, &rc);
        pWin32Clock->lpVtbl->ShowWin32Clock(pWin32Clock, hWnd, &rc);
        pWin32Clock->lpVtbl->Release(pWin32Clock);
    }
    return TRUE;
}

INT64 ClockButtonSubclassProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    UINT_PTR    uIdSubclass,
    DWORD_PTR   dwRefData
)
{
    if (uMsg == WM_NCDESTROY)
    {
        RemoveWindowSubclass(hWnd, ClockButtonSubclassProc, ClockButtonSubclassProc);
    }
    else if (uMsg == WM_LBUTTONDOWN || (uMsg == WM_KEYDOWN && wParam == VK_RETURN))
    {
        if (ShouldShowLegacyClockExperience() == 1)
        {
            if (!FindWindowW(L"ClockFlyoutWindow", NULL))
            {
                return ShowLegacyClockExperience(hWnd);
            }
            else
            {
                return 1;
            }
        }
        else if (ShouldShowLegacyClockExperience() == 2)
        {
            if (FindWindowW(L"Windows.UI.Core.CoreWindow", NULL))
            {
                ToggleNotificationsFlyout();
            }
            return 1;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
#endif
#pragma endregion


#pragma region "Popup menu hooks"
BOOL IsImmersiveMenu = FALSE;
BOOL CheckIfImmersiveContextMenu(
    HWND unnamedParam1,
    LPCSTR unnamedParam2,
    HANDLE unnamedParam3
)
{
    if ((*((WORD*)&(unnamedParam2)+1)))
    {
        if (!strncmp(unnamedParam2, "ImmersiveContextMenuArray", 25))
        {
            IsImmersiveMenu = TRUE;
            return FALSE;
        }
    }
    return TRUE;
}
void RemoveOwnerDrawFromMenu(int level, HMENU hMenu)
{
    if (hMenu)
    {
        int k = GetMenuItemCount(hMenu);
        for (int i = 0; i < k; ++i)
        {
            MENUITEMINFO mii;
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_FTYPE | MIIM_SUBMENU;
            if (GetMenuItemInfoW(hMenu, i, TRUE, &mii) && (mii.fType & MFT_OWNERDRAW))
            {
                mii.fType &= ~MFT_OWNERDRAW;
                printf("[ROD]: Level %d Position %d/%d Status %d\n", level, i, k, SetMenuItemInfoW(hMenu, i, TRUE, &mii));
                RemoveOwnerDrawFromMenu(level + 1, mii.hSubMenu);
            }
        }
    }
}
BOOL CheckIfMenuContainsOwnPropertiesItem(HMENU hMenu)
{
#ifdef _WIN64
    if (hMenu)
    {
        int k = GetMenuItemCount(hMenu);
        for (int i = k - 1; i >= 0; i--)
        {
            MENUITEMINFO mii;
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_DATA | MIIM_ID;
            BOOL b = GetMenuItemInfoW(hMenu, i, TRUE, &mii);
            if (b && (mii.wID >= 12000 && mii.wID <= 12200) && mii.dwItemData == CheckForUpdatesThread)
            {
                return TRUE;
            }
        }
    }
#endif
    return FALSE;
}
BOOL TrackPopupMenuHookEx(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    IsImmersiveMenu = FALSE;

    wchar_t wszClassName[200];
    ZeroMemory(wszClassName, 200);
    GetClassNameW(hWnd, wszClassName, 200);

    BOOL bIsTaskbar = (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd")) ? !bSkinMenus : bDisableImmersiveContextMenu;
    //wprintf(L">> %s %d %d\n", wszClassName, bIsTaskbar, bIsExplorerProcess);

    BOOL bContainsOwn = FALSE;
    if (bIsExplorerProcess && (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd")))
    {
        bContainsOwn = CheckIfMenuContainsOwnPropertiesItem(hMenu);
    }

    if (bIsTaskbar && (bIsExplorerProcess ? 1 : (!wcscmp(wszClassName, L"SHELLDLL_DefView") || !wcscmp(wszClassName, L"SysTreeView32"))))
    {
        EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
        if (IsImmersiveMenu)
        {
            IsImmersiveMenu = FALSE;
#ifndef _WIN64
            if (bIsExplorerProcess)
            {
#else
            if (bIsExplorerProcess && ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
            {
                POINT pt;
                pt.x = x;
                pt.y = y;
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
#endif
            }
            else
            {
                RemoveOwnerDrawFromMenu(0, hMenu);
            }

            BOOL bRet = TrackPopupMenuEx(
                hMenu,
                uFlags,
                x,
                y,
                hWnd,
                lptpm
            );
#ifdef _WIN64
            if (bContainsOwn && (bRet >= 12000 && bRet <= 12200))
            {
                LaunchPropertiesGUI(hModule);
                return FALSE;
            }
#endif
            return bRet;
        }
        IsImmersiveMenu = FALSE;
    }
    BOOL b = TrackPopupMenuEx(
        hMenu,
        uFlags,
        x,
        y,
        hWnd,
        lptpm
    );
#ifdef _WIN64
    if (bContainsOwn && (b >= 12000 && b <= 12200))
    {
        LaunchPropertiesGUI(hModule);
        return FALSE;
    }
#endif
    return b;
}
BOOL TrackPopupMenuHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    int         nReserved,
    HWND        hWnd,
    const RECT* prcRect
)
{
    IsImmersiveMenu = FALSE;

    wchar_t wszClassName[200];
    ZeroMemory(wszClassName, 200);
    GetClassNameW(hWnd, wszClassName, 200);

    BOOL bIsTaskbar = (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd")) ? !bSkinMenus : bDisableImmersiveContextMenu;
    //wprintf(L">> %s %d %d\n", wszClassName, bIsTaskbar, bIsExplorerProcess);

    BOOL bContainsOwn = FALSE;
    if (bIsExplorerProcess && (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd")))
    {
        bContainsOwn = CheckIfMenuContainsOwnPropertiesItem(hMenu);
    }

    if (bIsTaskbar && (bIsExplorerProcess ? 1 : (!wcscmp(wszClassName, L"SHELLDLL_DefView") || !wcscmp(wszClassName, L"SysTreeView32"))))
    {
        EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
        if (IsImmersiveMenu)
        {
            IsImmersiveMenu = FALSE;

#ifndef _WIN64
            if (bIsExplorerProcess)
            {
#else
            if (bIsExplorerProcess && ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
            {
                POINT pt;
                pt.x = x;
                pt.y = y;
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
#endif
            }
            else
            {
                RemoveOwnerDrawFromMenu(0, hMenu);
            }

            BOOL bRet = TrackPopupMenu(
                hMenu,
                uFlags,
                x,
                y,
                0,
                hWnd,
                prcRect
            );
#ifdef _WIN64
            if (bContainsOwn && (bRet >= 12000 && bRet <= 12200))
            {
                LaunchPropertiesGUI(hModule);
                return FALSE;
            }
#endif
            return bRet;
        }
        IsImmersiveMenu = FALSE;
    }
    BOOL b = TrackPopupMenu(
        hMenu,
        uFlags,
        x,
        y,
        0,
        hWnd,
        prcRect
    );
#ifdef _WIN64
    if (bContainsOwn && (b >= 12000 && b <= 12200))
    {
        LaunchPropertiesGUI(hModule);
        return FALSE;
    }
#endif
    return b;
}
#ifdef _WIN64
#define TB_POS_NOWHERE 0
#define TB_POS_BOTTOM 1
#define TB_POS_TOP 2
#define TB_POS_LEFT 3
#define TB_POS_RIGHT 4
void PopupMenuAdjustCoordinatesAndFlags(int* x, int* y, UINT* uFlags)
{
    POINT pt;
    GetCursorPos(&pt);
    RECT rc;
    UINT tbPos = GetTaskbarLocationAndSize(pt, &rc);
    if (tbPos == TB_POS_BOTTOM)
    {
        *y = MIN(*y, rc.top);
        *uFlags |= TPM_CENTERALIGN | TPM_BOTTOMALIGN;
    }
    else if (tbPos == TB_POS_TOP)
    {
        *y = MAX(*y, rc.bottom);
        *uFlags |= TPM_CENTERALIGN | TPM_TOPALIGN;
    }
    else if (tbPos == TB_POS_LEFT)
    {
        *x = MAX(*x, rc.right);
        *uFlags |= TPM_VCENTERALIGN | TPM_LEFTALIGN;
    }
    if (tbPos == TB_POS_RIGHT)
    {
        *x = MIN(*x, rc.left);
        *uFlags |= TPM_VCENTERALIGN | TPM_RIGHTALIGN;
    }
}
UINT GetTaskbarLocationAndSize(POINT ptCursor, RECT* rc)
{
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    HWND hWnd = GetMonitorInfoFromPointForTaskbarFlyoutActivation(
        ptCursor,
        MONITOR_DEFAULTTOPRIMARY,
        &mi
    );
    if (hWnd)
    {
        GetWindowRect(hWnd, rc);
        RECT rcC = *rc;
        rcC.left -= mi.rcMonitor.left;
        rcC.right -= mi.rcMonitor.left;
        rcC.top -= mi.rcMonitor.top;
        rcC.bottom -= mi.rcMonitor.top;
        if (rcC.left < 5 && rcC.top > 5)
        {
            return TB_POS_BOTTOM;
        }
        else if (rcC.left < 5 && rcC.top < 5 && rcC.right > rcC.bottom)
        {
            return TB_POS_TOP;
        }
        else if (rcC.left < 5 && rcC.top < 5 && rcC.right < rcC.bottom)
        {
            return TB_POS_LEFT;
        }
        else if (rcC.left > 5 && rcC.top < 5)
        {
            return TB_POS_RIGHT;
        }
    }
    return TB_POS_NOWHERE;
}
INT64 OwnerDrawSubclassProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    UINT_PTR    uIdSubclass,
    DWORD_PTR   dwRefData
)
{
    BOOL v12 = FALSE;
    if ((uMsg == WM_DRAWITEM || uMsg == WM_MEASUREITEM) &&
        CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc &&
        CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc(
            hWnd,
            uMsg,
            wParam,
            lParam,
            &v12
        ))
    {
        return 0;
    }
    return DefSubclassProc(
        hWnd,
        uMsg,
        wParam,
        lParam
    );
}
long long explorer_TrackPopupMenuExElapsed = 0;
BOOL explorer_TrackPopupMenuExHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    long long elapsed = milliseconds_now() - explorer_TrackPopupMenuExElapsed;
    BOOL b = FALSE;

    wchar_t wszClassName[200];
    ZeroMemory(wszClassName, 200);
    GetClassNameW(hWnd, wszClassName, 200);
    BOOL bContainsOwn = FALSE;
    if (bIsExplorerProcess && (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd")))
    {
        bContainsOwn = CheckIfMenuContainsOwnPropertiesItem(hMenu);
    }
    
    wchar_t wszClassNameOfWindowUnderCursor[200];
    ZeroMemory(wszClassNameOfWindowUnderCursor, 200);
    POINT p; p.x = x; p.y = y;
    GetClassNameW(WindowFromPoint(p), wszClassNameOfWindowUnderCursor, 200);
    BOOL bIsSecondaryTaskbar = (!wcscmp(wszClassName, L"Shell_SecondaryTrayWnd") && !wcscmp(wszClassNameOfWindowUnderCursor, L"Shell_SecondaryTrayWnd"));

    if (elapsed > POPUPMENU_EX_ELAPSED || !bFlyoutMenus || bIsSecondaryTaskbar)
    {
        if (bCenterMenus && !bIsSecondaryTaskbar)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        IsImmersiveMenu = FALSE;
        if (!bSkinMenus)
        {
            EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
            if (IsImmersiveMenu)
            {
                if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
                {
                    POINT pt;
                    pt.x = x;
                    pt.y = y;
                    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                        hMenu,
                        hWnd,
                        &(pt)
                    );
                }
                else
                {
                    RemoveOwnerDrawFromMenu(0, hMenu);
                }
            }
            IsImmersiveMenu = FALSE;
        }
        b = TrackPopupMenuEx(
            hMenu,
            uFlags,
            x,
            y,
            hWnd,
            lptpm
        );
        if (bContainsOwn && (b >= 12000 && b <= 12200))
        {
            LaunchPropertiesGUI(hModule);
            return FALSE;
        }
        if (!bIsSecondaryTaskbar)
        {
            explorer_TrackPopupMenuExElapsed = milliseconds_now();
        }
    }
    return b;
}
long long pnidui_TrackPopupMenuElapsed = 0;
BOOL pnidui_TrackPopupMenuHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    int         nReserved,
    HWND        hWnd,
    const RECT* prcRect
)
{
    long long elapsed = milliseconds_now() - pnidui_TrackPopupMenuElapsed;
    BOOL b = FALSE;
    if (elapsed > POPUPMENU_PNIDUI_TIMEOUT || !bFlyoutMenus)
    {
        if (bCenterMenus)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        IsImmersiveMenu = FALSE;
        if (!bSkinMenus)
        {
            EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
            if (IsImmersiveMenu)
            {
                if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
                {
                    POINT pt;
                    pt.x = x;
                    pt.y = y;
                    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                        hMenu,
                        hWnd,
                        &(pt)
                    );
                }
                else
                {
                    RemoveOwnerDrawFromMenu(0, hMenu);
                }
            }
            IsImmersiveMenu = FALSE;
        }
        b = TrackPopupMenu(
            hMenu,
            uFlags | TPM_RIGHTBUTTON,
            x,
            y,
            0,
            hWnd,
            prcRect
        );
        if (bReplaceNetwork && b == 3109)
        {
            LaunchNetworkTargets(bReplaceNetwork + 2);
            b = 0;
        }
        pnidui_TrackPopupMenuElapsed = milliseconds_now();
    }
    return b;
}
long long sndvolsso_TrackPopupMenuExElapsed = 0;
BOOL sndvolsso_TrackPopupMenuExHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    long long elapsed = milliseconds_now() - sndvolsso_TrackPopupMenuExElapsed;
    BOOL b = FALSE;
    if (elapsed > POPUPMENU_SNDVOLSSO_TIMEOUT || !bFlyoutMenus)
    {
        if (bCenterMenus)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        IsImmersiveMenu = FALSE;
        if (!bSkinMenus)
        {
            EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
            if (IsImmersiveMenu)
            {
                if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
                {
                    POINT pt;
                    pt.x = x;
                    pt.y = y;
                    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                        hMenu,
                        hWnd,
                        &(pt)
                    );
                }
                else
                {
                    RemoveOwnerDrawFromMenu(0, hMenu);
                }
            }
            IsImmersiveMenu = FALSE;
        }

        /*MENUITEMINFOW menuInfo;
        ZeroMemory(&menuInfo, sizeof(MENUITEMINFOW));
        menuInfo.cbSize = sizeof(MENUITEMINFOW);
        menuInfo.fMask = MIIM_ID | MIIM_STRING;
        printf("GetMenuItemInfoW %d\n", GetMenuItemInfoW(
            hMenu,
            GetMenuItemCount(hMenu) - 1,
            TRUE,
            &menuInfo
        ));
        menuInfo.dwTypeData = malloc(menuInfo.cch + sizeof(wchar_t));
        menuInfo.cch++;
        printf("GetMenuItemInfoW %d\n", GetMenuItemInfoW(
            hMenu,
            GetMenuItemCount(hMenu) - 1,
            TRUE,
            &menuInfo
        ));
        wcscpy_s(menuInfo.dwTypeData, menuInfo.cch, L"test");
        menuInfo.fMask = MIIM_STRING;
        wprintf(L"SetMenuItemInfoW %s %d\n", menuInfo.dwTypeData, SetMenuItemInfoW(
            hMenu,
            GetMenuItemCount(hMenu) - 1,
            TRUE,
            &menuInfo
        ));
        wcscpy_s(menuInfo.dwTypeData, menuInfo.cch, L"");
        printf("GetMenuItemInfoW %d\n", GetMenuItemInfoW(
            hMenu,
            GetMenuItemCount(hMenu) - 1,
            TRUE,
            &menuInfo
        ));
        wprintf(L"%s\n", menuInfo.dwTypeData);
        free(menuInfo.dwTypeData);*/

        b = TrackPopupMenuEx(
            hMenu,
            uFlags | TPM_RIGHTBUTTON,
            x,
            y,
            hWnd,
            lptpm
        );
        sndvolsso_TrackPopupMenuExElapsed = milliseconds_now();
    }
    return b;
}
long long stobject_TrackPopupMenuExElapsed = 0;
BOOL stobject_TrackPopupMenuExHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    long long elapsed = milliseconds_now() - stobject_TrackPopupMenuExElapsed;
    BOOL b = FALSE;
    if (elapsed > POPUPMENU_SAFETOREMOVE_TIMEOUT || !bFlyoutMenus)
    {
        if (bCenterMenus)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        INT64* unknown_array = NULL;
        POINT pt;
        if (bSkinMenus)
        {
            unknown_array = calloc(4, sizeof(INT64));
            pt.x = x;
            pt.y = y;
            if (ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)
            {
                ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt),
                    0xc,
                    unknown_array
                );
            }
            SetWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc, 0);
        }
        b = TrackPopupMenuEx(
            hMenu,
            uFlags | TPM_RIGHTBUTTON,
            x,
            y,
            hWnd,
            lptpm
        );
        stobject_TrackPopupMenuExElapsed = milliseconds_now();
        if (bSkinMenus)
        {
            RemoveWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc);
            if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
            {
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
            }
            free(unknown_array);
        }
    }
    return b;
}
long long stobject_TrackPopupMenuElapsed = 0;
BOOL stobject_TrackPopupMenuHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    int         nReserved,
    HWND        hWnd,
    const RECT* prcRect
)
{
    long long elapsed = milliseconds_now() - stobject_TrackPopupMenuElapsed;
    BOOL b = FALSE;
    if (elapsed > POPUPMENU_SAFETOREMOVE_TIMEOUT || !bFlyoutMenus)
    {
        if (bCenterMenus)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        INT64* unknown_array = NULL;
        POINT pt;
        if (bSkinMenus)
        {
            unknown_array = calloc(4, sizeof(INT64));
            pt.x = x;
            pt.y = y;
            if (ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)
            {
                ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt),
                    0xc,
                    unknown_array
                );
            }
            SetWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc, 0);
        }
        b = TrackPopupMenu(
            hMenu,
            uFlags | TPM_RIGHTBUTTON,
            x,
            y,
            0,
            hWnd,
            prcRect
        );
        stobject_TrackPopupMenuElapsed = milliseconds_now();
        if (bSkinMenus)
        {
            RemoveWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc);
            if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
            {
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
            }
            free(unknown_array);
        }
    }
    return b;
}
long long bthprops_TrackPopupMenuExElapsed = 0;
BOOL bthprops_TrackPopupMenuExHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    long long elapsed = milliseconds_now() - bthprops_TrackPopupMenuExElapsed;
    BOOL b = FALSE;
    if (elapsed > POPUPMENU_BLUETOOTH_TIMEOUT || !bFlyoutMenus)
    {
        if (bCenterMenus)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        INT64* unknown_array = NULL;
        POINT pt;
        if (bSkinMenus)
        {
            unknown_array = calloc(4, sizeof(INT64));
            pt.x = x;
            pt.y = y;
            if (ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)
            {
                ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt),
                    0xc,
                    unknown_array
                );
            }
            SetWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc, 0);
        }
        b = TrackPopupMenuEx(
            hMenu,
            uFlags | TPM_RIGHTBUTTON,
            x,
            y,
            hWnd,
            lptpm
        );
        bthprops_TrackPopupMenuExElapsed = milliseconds_now();
        if (bSkinMenus)
        {
            RemoveWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc);
            if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
            {
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
            }
            free(unknown_array);
        }
    }
    return b;
}
long long inputswitch_TrackPopupMenuExElapsed = 0;
BOOL inputswitch_TrackPopupMenuExHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    long long elapsed = milliseconds_now() - inputswitch_TrackPopupMenuExElapsed;
    BOOL b = FALSE;
    if (elapsed > POPUPMENU_INPUTSWITCH_TIMEOUT || !bFlyoutMenus)
    {
        if (bCenterMenus)
        {
            PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
        }
        IsImmersiveMenu = FALSE;
        if (!bSkinMenus)
        {
            EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
            if (IsImmersiveMenu)
            {
                if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
                {
                    POINT pt;
                    pt.x = x;
                    pt.y = y;
                    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                        hMenu,
                        hWnd,
                        &(pt)
                    );
                }
                else
                {
                    RemoveOwnerDrawFromMenu(0, hMenu);
                }
            }
            IsImmersiveMenu = FALSE;
        }
        b = TrackPopupMenuEx(
            hMenu,
            uFlags | TPM_RIGHTBUTTON,
            x,
            y,
            hWnd,
            lptpm
        );
        inputswitch_TrackPopupMenuExElapsed = milliseconds_now();
    }
    return b;
}
#endif
#pragma endregion


#pragma region "Disable immersive menus"
BOOL WINAPI DisableImmersiveMenus_SystemParametersInfoW(
    UINT  uiAction,
    UINT  uiParam,
    PVOID pvParam,
    UINT  fWinIni
)
{
    if (bDisableImmersiveContextMenu && uiAction == SPI_GETSCREENREADER)
    {
        printf("SystemParametersInfoW\n");
        *(BOOL*)pvParam = TRUE;
        return TRUE;
    }
    return SystemParametersInfoW(uiAction, uiParam, pvParam, fWinIni);
}
#pragma endregion


#pragma region "Explorer: Hide search bar, Mica effect (private), hide navigation bar"
static HWND(__stdcall *explorerframe_SHCreateWorkerWindowFunc)(
    WNDPROC  	wndProc,
    HWND  	hWndParent,
    DWORD  	dwExStyle,
    DWORD  	dwStyle,
    HMENU  	hMenu,
    LONG_PTR  	wnd_extra
    );

HWND WINAPI explorerframe_SHCreateWorkerWindowHook(
    WNDPROC  	wndProc,
    HWND  	hWndParent,
    DWORD  	dwExStyle,
    DWORD  	dwStyle,
    HMENU  	hMenu,
    LONG_PTR  	wnd_extra
)
{
    HWND result;
    LSTATUS lRes = ERROR_FILE_NOT_FOUND;
    DWORD dwSize = 0;
    
    printf("%x %x\n", dwExStyle, dwStyle);

    if (SHRegGetValueFromHKCUHKLMWithOpt(
        TEXT("SOFTWARE\\Classes\\CLSID\\{056440FD-8568-48e7-A632-72157243B55B}\\InProcServer32"),
        TEXT(""),
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        (LPDWORD)(&dwSize)
    ) == ERROR_SUCCESS && (dwSize < 4) && dwExStyle == 0x10000 && dwStyle == 1174405120)
    {
        result = 0;
    }
    else
    {
        result = explorerframe_SHCreateWorkerWindowFunc(
            wndProc,
            hWndParent,
            dwExStyle,
            dwStyle,
            hMenu,
            wnd_extra
        );
    }
    if (dwExStyle == 0x10000 && dwStyle == 0x46000000)
    {
#ifdef USE_PRIVATE_INTERFACES
        if (bMicaEffectOnTitlebar && result)
        {
            BOOL value = TRUE;
            SetPropW(hWndParent, L"NavBarGlass", HANDLE_FLAG_INHERIT);
            DwmSetWindowAttribute(hWndParent, DWMWA_MICA_EFFFECT, &value, sizeof(BOOL));
            if (result) SetWindowSubclass(result, ExplorerMicaTitlebarSubclassProc, ExplorerMicaTitlebarSubclassProc, 0);
        }
#endif

        if (bHideExplorerSearchBar && result)
        {
            SetWindowSubclass(hWndParent, HideExplorerSearchBarSubClass, HideExplorerSearchBarSubClass, 0);
        }
    }
    return result;
}
#pragma endregion


#pragma region "Fix battery flyout"
#ifdef _WIN64
LSTATUS stobject_RegGetValueW(
    HKEY    hkey,
    LPCWSTR lpSubKey,
    LPCWSTR lpValue,
    DWORD   dwFlags,
    LPDWORD pdwType,
    PVOID   pvData,
    LPDWORD pcbData
)
{
    if (!lstrcmpW(lpValue, L"UseWin32BatteryFlyout"))
    {
        if (SHRegGetValueFromHKCUHKLMFunc)
        {
            return SHRegGetValueFromHKCUHKLMFunc(
                lpSubKey,
                lpValue,
                SRRF_RT_REG_DWORD,
                pdwType,
                pvData,
                pcbData
            );
        }
    }
    return RegGetValueW(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
}

HRESULT stobject_CoCreateInstanceHook(
    REFCLSID  rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD     dwClsContext,
    REFIID    riid,
    LPVOID* ppv
)
{
    DWORD dwVal = 0, dwSize = sizeof(DWORD);
    if (IsEqualGUID(rclsid, &CLSID_ImmersiveShell) &&
        IsEqualGUID(riid, &IID_IServiceProvider) &&
        SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ImmersiveShell"),
            TEXT("UseWin32BatteryFlyout"),
            SRRF_RT_REG_DWORD,
            NULL,
            &dwVal,
            (LPDWORD)(&dwSize)
        ) == ERROR_SUCCESS)
    {
        if (!dwVal)
        {
            if (hCheckForegroundThread)
            {
                if (WaitForSingleObject(hCheckForegroundThread, 0) == WAIT_TIMEOUT)
                {
                    return E_NOINTERFACE;
                }
                WaitForSingleObject(hCheckForegroundThread, INFINITE);
                CloseHandle(hCheckForegroundThread);
                hCheckForegroundThread = NULL;
            }
            HKEY hKey = NULL;
            if (RegCreateKeyExW(
                HKEY_CURRENT_USER,
                _T(SEH_REGPATH),
                0,
                NULL,
                REG_OPTION_VOLATILE,
                KEY_READ,
                NULL,
                &hKey,
                NULL
            ) == ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
            }
            TerminateShellExperienceHost();
            InvokeFlyout(0, INVOKE_FLYOUT_BATTERY);
            Sleep(100);
            hCheckForegroundThread = CreateThread(
                0,
                0,
                CheckForegroundThread,
                dwVal,
                0,
                0
            );
        }
    }
    return CoCreateInstance(
        rclsid,
        pUnkOuter,
        dwClsContext,
        riid,
        ppv
    );
}
#endif
#pragma endregion


#pragma region "Show WiFi networks on network icon click"
#ifdef _WIN64
HRESULT pnidui_CoCreateInstanceHook(
    REFCLSID  rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD     dwClsContext,
    REFIID    riid,
    LPVOID* ppv
)
{
    DWORD dwVal = 0, dwSize = sizeof(DWORD);
    if (IsEqualGUID(rclsid, &CLSID_ImmersiveShell) && 
        IsEqualGUID(riid, &IID_IServiceProvider) &&
        SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Settings\\Network"),
            TEXT("ReplaceVan"),
            SRRF_RT_REG_DWORD,
            NULL,
            &dwVal,
            (LPDWORD)(&dwSize)
        ) == ERROR_SUCCESS)
    {
        if (dwVal)
        {
            if (dwVal == 5 || dwVal == 6)
            {
                if (hCheckForegroundThread)
                {
                    WaitForSingleObject(hCheckForegroundThread, INFINITE);
                    CloseHandle(hCheckForegroundThread);
                    hCheckForegroundThread = NULL;
                }
                if (milliseconds_now() - elapsedCheckForeground > CHECKFOREGROUNDELAPSED_TIMEOUT)
                {
                    LaunchNetworkTargets(dwVal);
                    hCheckForegroundThread = CreateThread(
                        0,
                        0,
                        CheckForegroundThread,
                        dwVal,
                        0,
                        0
                    );
                }
            }
            else
            {
                LaunchNetworkTargets(dwVal);
            }
            return E_NOINTERFACE;
        }
        else
        {
            if (hCheckForegroundThread)
            {
                if (WaitForSingleObject(hCheckForegroundThread, 0) == WAIT_TIMEOUT)
                {
                    return E_NOINTERFACE;
                }
                WaitForSingleObject(hCheckForegroundThread, INFINITE);
                CloseHandle(hCheckForegroundThread);
                hCheckForegroundThread = NULL;
            }
            HKEY hKey = NULL;
            if (RegCreateKeyExW(
                HKEY_CURRENT_USER,
                _T(SEH_REGPATH),
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_READ,
                NULL,
                &hKey,
                NULL
            ) == ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
            }
            TerminateShellExperienceHost();
            InvokeFlyout(0, INVOKE_FLYOUT_NETWORK);
            Sleep(100);
            hCheckForegroundThread = CreateThread(
                0,
                0,
                CheckForegroundThread,
                dwVal,
                0,
                0
            );
        }
    }
    return CoCreateInstance(
        rclsid,
        pUnkOuter,
        dwClsContext,
        riid,
        ppv
    );
}
#endif
#pragma endregion


#pragma region "Clock flyout helper"
#ifdef _WIN64
typedef struct _ClockButton_ToggleFlyoutCallback_Params
{
    void* TrayUIInstance;
    unsigned int CLOCKBUTTON_OFFSET_IN_TRAYUI;
    void* oldClockButtonInstance;
} ClockButton_ToggleFlyoutCallback_Params;
void ClockButton_ToggleFlyoutCallback(
    HWND hWnd,
    UINT uMsg,
    ClockButton_ToggleFlyoutCallback_Params* params,
    LRESULT lRes
)
{
    *((INT64*)params->TrayUIInstance + params->CLOCKBUTTON_OFFSET_IN_TRAYUI) = params->oldClockButtonInstance;
    free(params);
}
void InvokeClockFlyout()
{
    POINT ptCursor;
    GetCursorPos(&ptCursor);
    HWND hWnd = GetMonitorInfoFromPointForTaskbarFlyoutActivation(
        ptCursor,
        MONITOR_DEFAULTTOPRIMARY,
        NULL
    );
    HWND prev_hWnd = hWnd;
    HWND hShellTray_Wnd = FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL);
    const unsigned int WM_TOGGLE_CLOCK_FLYOUT = 1486;
    if (hWnd == hShellTray_Wnd)
    {
        if (ShouldShowLegacyClockExperience() == 1)
        {
            if (!FindWindowW(L"ClockFlyoutWindow", NULL))
            {
                if (bOldTaskbar)
                {
                    return ShowLegacyClockExperience(FindWindowExW(FindWindowExW(hShellTray_Wnd, NULL, L"TrayNotifyWnd", NULL), NULL, L"TrayClockWClass", NULL));
                }
                else
                {
                    POINT pt;
                    pt.x = 0;
                    pt.y = 0;
                    GetCursorPos(&pt);
                    BOOL bBottom, bRight;
                    POINT dPt = GetDefaultWinXPosition(FALSE, NULL, NULL, FALSE, TRUE);
                    SetCursorPos(dPt.x - 1, dPt.y);
                    BOOL bRet = ShowLegacyClockExperience(hShellTray_Wnd);
                    SetCursorPos(pt.x, pt.y);
                    return bRet;
                }
            }
            else
            {
                return PostMessageW(FindWindowW(L"ClockFlyoutWindow", NULL), WM_CLOSE, 0, 0);
            }
        }
        else if (ShouldShowLegacyClockExperience() == 2)
        {
            ToggleNotificationsFlyout();
            return 0;
        }
        // On the main monitor, the TrayUI component of CTray handles this
        // message and basically does a `ClockButton::ToggleFlyout`; that's
        // the only place in code where that is used, otherwise, clicking and
        // dismissing the clock flyout probably involves 2 separate methods
        PostMessageW(hShellTray_Wnd, WM_TOGGLE_CLOCK_FLYOUT, 0, 0);
    }
    else
    {
        // Of course, on secondary monitors, the situation is much more
        // complicated; there is no simple way to do this, afaik; the way I do it
        // is to obtain a pointer to TrayUI from CTray (pointers to the classes
        // that created the windows are always available at location 0 in the hWnd)
        // and from there issue a "show clock flyout" message manually, taking care to temporarly
        // change the internal clock button pointer of the class to point
        // to the clock button on the secondary monitor.
        if (bOldTaskbar)
        {
            hWnd = FindWindowExW(hWnd, NULL, L"ClockButton", NULL);
        }
        if (hWnd)
        {
            if (ShouldShowLegacyClockExperience() == 1)
            {
                if (!FindWindowW(L"ClockFlyoutWindow", NULL))
                {
                    if (bOldTaskbar)
                    {
                        return ShowLegacyClockExperience(hWnd);
                    }
                    else
                    {
                        POINT pt;
                        pt.x = 0;
                        pt.y = 0;
                        GetCursorPos(&pt);
                        BOOL bBottom, bRight;
                        POINT dPt = GetDefaultWinXPosition(FALSE, NULL, NULL, FALSE, TRUE);
                        SetCursorPos(dPt.x, dPt.y);
                        BOOL bRet = ShowLegacyClockExperience(hWnd);
                        SetCursorPos(pt.x, pt.y);
                        return bRet;
                    }
                }
                else
                {
                    return PostMessageW(FindWindowW(L"ClockFlyoutWindow", NULL), WM_CLOSE, 0, 0);
                }
            }
            else if (ShouldShowLegacyClockExperience() == 2)
            {
                ToggleNotificationsFlyout();
                return 0;
            }
            if (bOldTaskbar)
            {
                INT64* CTrayInstance = (BYTE*)(GetWindowLongPtrW(hShellTray_Wnd, 0)); // -> CTray
                void* ClockButtonInstance = (BYTE*)(GetWindowLongPtrW(hWnd, 0)); // -> ClockButton

                // inspect CTray::v_WndProc, look for mentions of
                // CTray::_HandlePowerStatus or patterns like **((_QWORD **)this + 110) + 184i64
                const unsigned int TRAYUI_OFFSET_IN_CTRAY = 110;
                // simply inspect vtable of TrayUI
                const unsigned int TRAYUI_WNDPROC_POSITION_IN_VTABLE = 4;
                // inspect TrayUI::WndProc, specifically this section
                /*
                    {
                      if ( (_DWORD)a3 == 1486 )
                      {
                        v80 = (ClockButton *)*((_QWORD *)this + 100);
                        if ( v80 )
                          ClockButton::ToggleFlyout(v80);
                */
                const unsigned int CLOCKBUTTON_OFFSET_IN_TRAYUI = 100;
                void* TrayUIInstance = *((INT64*)CTrayInstance + TRAYUI_OFFSET_IN_CTRAY);
                void* oldClockButtonInstance = *((INT64*)TrayUIInstance + CLOCKBUTTON_OFFSET_IN_TRAYUI);
                ClockButton_ToggleFlyoutCallback_Params* params = malloc(sizeof(ClockButton_ToggleFlyoutCallback_Params));
                if (params)
                {
                    *((INT64*)TrayUIInstance + CLOCKBUTTON_OFFSET_IN_TRAYUI) = ClockButtonInstance;
                    params->TrayUIInstance = TrayUIInstance;
                    params->CLOCKBUTTON_OFFSET_IN_TRAYUI = CLOCKBUTTON_OFFSET_IN_TRAYUI;
                    params->oldClockButtonInstance = oldClockButtonInstance;
                    SendMessageCallbackW(hShellTray_Wnd, WM_TOGGLE_CLOCK_FLYOUT, 0, 0, ClockButton_ToggleFlyoutCallback, params);
                }
            }
            else
            {
                PostMessageW(hShellTray_Wnd, WM_TOGGLE_CLOCK_FLYOUT, 0, 0);
            }
        }
    }
}
INT64 winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageHook(
    void* _this,
    INT64 a2,
    INT a3
)
{
    if (!bClockFlyoutOnWinC)
    {
        if (winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc)
        {
            return winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc(_this, a2, a3);
        }
        return 0;
    }
    if (a2 == 786 && a3 == 107)
    {
        InvokeClockFlyout();
    }
    return 0;
}
#endif
#pragma endregion


#pragma region "Enable old taskbar"
#ifdef _WIN64
DEFINE_GUID(GUID_18C02F2E_2754_5A20_8BD5_0B34CE79DA2B,
    0x18C02F2E,
    0x2754, 0x5A20, 0x8b, 0xd5,
    0x0b, 0x34, 0xce, 0x79, 0xda, 0x2b
);
HRESULT explorer_RoGetActivationFactoryHook(HSTRING activatableClassId, GUID* iid, void** factory)
{
    PCWSTR StringRawBuffer = WindowsGetStringRawBuffer(activatableClassId, 0);
    if (!wcscmp(StringRawBuffer, L"WindowsUdk.ApplicationModel.AppExtensions.XamlExtensions") && IsEqualGUID(iid, &GUID_18C02F2E_2754_5A20_8BD5_0B34CE79DA2B))
    {
        *factory = &XamlExtensionsFactory;
        return S_OK;
    }
    return RoGetActivationFactory(activatableClassId, iid, factory);
}

FARPROC explorer_GetProcAddressHook(HMODULE hModule, const CHAR* lpProcName)
{
    if ((*((WORD*)&(lpProcName)+1)) && !strncmp(lpProcName, "RoGetActivationFactory", 22))
        return (FARPROC)explorer_RoGetActivationFactoryHook;
    else
        return GetProcAddress(hModule, lpProcName);
}
#endif
#pragma endregion


#pragma region "Open power user menu on Win+X"
#ifdef _WIN64
LRESULT explorer_SendMessageW(HWND hWndx, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == 0x579) // "Raise desktop" - basically shows desktop or the windows
                       // wParam = 3 => show desktop
                       // wParam = 2 => raise windows
    {
        
    }
    else if (uMsg == TB_GETTEXTROWS)
    {
        HWND hWnd = FindWindowEx(
            NULL,
            NULL,
            L"Shell_TrayWnd",
            NULL
        );
        if (hWnd)
        {
            hWnd = FindWindowEx(
                hWnd,
                NULL,
                L"RebarWindow32",
                NULL
            );
            if (hWnd)
            {
                hWnd = FindWindowEx(
                    hWnd,
                    NULL,
                    L"MSTaskSwWClass",
                    NULL
                );
                if (hWnd && hWnd == hWndx && wParam == -1)
                {
                    if (hIsWinXShown)
                    {
                        SendMessage(hWinXWnd, WM_CLOSE, 0, 0);
                    }
                    else
                    {
                        hWnd = FindWindowEx(
                            NULL,
                            NULL,
                            L"Shell_TrayWnd",
                            NULL
                        );
                        if (hWnd)
                        {
                            hWnd = FindWindowEx(
                                hWnd,
                                NULL,
                                L"Start",
                                NULL
                            );
                            if (hWnd)
                            {
                                POINT pt = GetDefaultWinXPosition(FALSE, NULL, NULL, TRUE, FALSE);
                                // Finally implemented a variation of
                                // https://github.com/valinet/ExplorerPatcher/issues/3
                                // inspired by how the real Start button activates this menu
                                // (CPearl::_GetLauncherTipContextMenu)
                                // This also works when auto hide taskbar is on (#63)
                                HRESULT hr = S_OK;
                                IUnknown* pImmersiveShell = NULL;
                                hr = CoCreateInstance(
                                    &CLSID_ImmersiveShell,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    &IID_IServiceProvider,
                                    &pImmersiveShell
                                );
                                if (SUCCEEDED(hr))
                                {
                                    IImmersiveMonitorService* pMonitorService = NULL;
                                    IUnknown_QueryService(
                                        pImmersiveShell,
                                        &SID_IImmersiveMonitorService,
                                        &IID_IImmersiveMonitorService,
                                        &pMonitorService
                                    );
                                    if (pMonitorService)
                                    {
                                        ILauncherTipContextMenu* pMenu = NULL;
                                        pMonitorService->lpVtbl->QueryServiceFromWindow(
                                            pMonitorService,
                                            hWnd,
                                            &IID_ILauncherTipContextMenu,
                                            &IID_ILauncherTipContextMenu,
                                            &pMenu
                                        );
                                        if (pMenu)
                                        {
                                            pMenu->lpVtbl->ShowLauncherTipContextMenu(
                                                pMenu,
                                                &pt
                                            );
                                            pMenu->lpVtbl->Release(pMenu);
                                        }
                                        pMonitorService->lpVtbl->Release(pMonitorService);
                                    }
                                    pImmersiveShell->lpVtbl->Release(pImmersiveShell);
                                }
                            }
                        }
                    }
                    return 0;
                }
            }
        }
    }
    return SendMessageW(hWndx, uMsg, wParam, lParam);
}
#endif
#pragma endregion


#pragma region "Set up taskbar button hooks, implement Weather widget"
#ifdef _WIN64

DWORD ShouldShowWidgetsInsteadOfCortana()
{
    DWORD dwVal = 0, dwSize = sizeof(DWORD);
    if (SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
        TEXT("TaskbarDa"),
        SRRF_RT_REG_DWORD,
        NULL,
        &dwVal,
        (LPDWORD)(&dwSize)
    ) == ERROR_SUCCESS)
    {
        return dwVal;
    }
    return 0;
}

__int64 (*Widgets_OnClickFunc)(__int64 a1, __int64 a2) = 0;
__int64 Widgets_OnClickHook(__int64 a1, __int64 a2)
{
    if (ShouldShowWidgetsInsteadOfCortana() == 1)
    {
        ToggleWidgetsPanel();
        return 0;
    }
    else
    {
        if (Widgets_OnClickFunc)
        {
            return Widgets_OnClickFunc(a1, a2);
        }
        return 0;
    }
}

HRESULT (*Widgets_GetTooltipTextFunc)(__int64 a1, __int64 a2, __int64 a3, WCHAR* a4, UINT a5) = 0;
HRESULT WINAPI Widgets_GetTooltipTextHook(__int64 a1, __int64 a2, __int64 a3, WCHAR* a4, UINT a5)
{
    if (ShouldShowWidgetsInsteadOfCortana() == 1)
    {
        return SHLoadIndirectString(
            L"@{windows?ms-resource://Windows.UI.SettingsAppThreshold/SystemSettings/Resources/SystemSettings_DesktopTaskbar_Da2/DisplayName}",
            a4,
            a5,
            0
        );
    }
    else
    {
        if (Widgets_GetTooltipTextFunc)
        {
            return Widgets_GetTooltipTextFunc(a1, a2, a3, a4, a5);
        }
        return 0;
    }
}

/*int WINAPI explorer_LoadStringWHook(HINSTANCE hInstance, UINT uID, WCHAR* lpBuffer, UINT cchBufferMax)
{
    WCHAR wszBuffer[MAX_PATH];
    if (hInstance == GetModuleHandle(NULL) && uID == 912)// && SUCCEEDED(epw->lpVtbl->GetTitle(epw, MAX_PATH, wszBuffer)))
    {
        //sws_error_PrintStackTrace();
        int rez = LoadStringW(hInstance, uID, lpBuffer, cchBufferMax);
        //wprintf(L"%s\n", lpBuffer);
        return rez;
    }
    else
    {
        return LoadStringW(hInstance, uID, lpBuffer, cchBufferMax);
    }
}*/

void stub1(void* i)
{
}

HWND PeopleButton_LastHWND = NULL;

BOOL explorer_DeleteMenu(HMENU hMenu, UINT uPosition, UINT uFlags)
{
    if (uPosition == 621 && uFlags == 0) // when removing News and interests
    {
        DeleteMenu(hMenu, 449, 0); // remove Cortana menu
        DeleteMenu(hMenu, 435, 0); // remove People menu
    }
    return DeleteMenu(hMenu, uPosition, uFlags);
}

HWND hWndWeatherFlyout;
void RecomputeWeatherFlyoutLocation(HWND hWnd)
{
    RECT rcButton;
    GetWindowRect(PeopleButton_LastHWND, &rcButton);
    POINT pButton;
    pButton.x = rcButton.left;
    pButton.y = rcButton.top;

    RECT rcWeatherFlyoutWindow;
    GetWindowRect(hWnd, &rcWeatherFlyoutWindow);

    POINT pNewWindow;

    RECT rc;
    UINT tbPos = GetTaskbarLocationAndSize(pButton, &rc);
    if (tbPos == TB_POS_BOTTOM)
    {
        pNewWindow.y = rcButton.top - (rcWeatherFlyoutWindow.bottom - rcWeatherFlyoutWindow.top);
    }
    else if (tbPos == TB_POS_TOP)
    {
        pNewWindow.y = rcButton.bottom;
    }
    else if (tbPos == TB_POS_LEFT)
    {
        pNewWindow.x = rcButton.right;
    }
    if (tbPos == TB_POS_RIGHT)
    {
        pNewWindow.x = rcButton.left - (rcWeatherFlyoutWindow.right - rcWeatherFlyoutWindow.left);
    }

    if (tbPos == TB_POS_BOTTOM || tbPos == TB_POS_TOP)
    {
        pNewWindow.x = rcButton.left + ((rcButton.right - rcButton.left) / 2) - ((rcWeatherFlyoutWindow.right - rcWeatherFlyoutWindow.left) / 2);

        HMONITOR hMonitor = MonitorFromPoint(pButton, MONITOR_DEFAULTTOPRIMARY);
        if (hMonitor)
        {
            MONITORINFO mi;
            mi.cbSize = sizeof(MONITORINFO);
            if (GetMonitorInfoW(hMonitor, &mi))
            {
                if (mi.rcWork.right < pNewWindow.x + (rcWeatherFlyoutWindow.right - rcWeatherFlyoutWindow.left))
                {
                    pNewWindow.x = mi.rcWork.right - (rcWeatherFlyoutWindow.right - rcWeatherFlyoutWindow.left);
                }
            }
        }
    }
    else if (tbPos == TB_POS_LEFT || tbPos == TB_POS_RIGHT)
    {
        pNewWindow.y = rcButton.top + ((rcButton.bottom - rcButton.top) / 2) - ((rcWeatherFlyoutWindow.bottom - rcWeatherFlyoutWindow.top) / 2);

        HMONITOR hMonitor = MonitorFromPoint(pButton, MONITOR_DEFAULTTOPRIMARY);
        if (hMonitor)
        {
            MONITORINFO mi;
            mi.cbSize = sizeof(MONITORINFO);
            if (GetMonitorInfoW(hMonitor, &mi))
            {
                if (mi.rcWork.bottom < pNewWindow.y + (rcWeatherFlyoutWindow.bottom - rcWeatherFlyoutWindow.top))
                {
                    pNewWindow.y = mi.rcWork.bottom - (rcWeatherFlyoutWindow.bottom - rcWeatherFlyoutWindow.top);
                }
            }
        }
    }

    SetWindowPos(hWnd, NULL, pNewWindow.x, pNewWindow.y, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
}

int prev_total_h = 0;
SIZE (*PeopleButton_CalculateMinimumSizeFunc)(void*, SIZE*);
SIZE WINAPI PeopleButton_CalculateMinimumSizeHook(void* _this, SIZE* pSz)
{
    SIZE ret = PeopleButton_CalculateMinimumSizeFunc(_this, pSz);
    AcquireSRWLockShared(&lock_epw);
    if (epw)
    {
        if (bWeatherFixedSize)
        {
            int mul = 1;
            switch (dwWeatherViewMode)
            {
            case EP_WEATHER_VIEW_ICONTEXT:
                mul = 4;
                break;
            case EP_WEATHER_VIEW_TEXTONLY:
                mul = 3;
                break;
            case EP_WEATHER_VIEW_ICONTEMP:
                mul = 2;
                break;
            case EP_WEATHER_VIEW_ICONONLY:
            case EP_WEATHER_VIEW_TEMPONLY:
                mul = 1;
                break;
            }
            pSz->cx = pSz->cx * mul;
        }
        else
        {
            if (!prev_total_h)
            {
                pSz->cx = 10000;
            }
            else
            {
                pSz->cx = prev_total_h;
            }
        }
    }
    //printf("[CalculateMinimumSize] %d %d\n", pSz->cx, pSz->cy);
    if (pSz->cy && epw)
    {
        BOOL bIsInitialized = TRUE;
        HRESULT hr = epw->lpVtbl->IsInitialized(epw, &bIsInitialized);
        if (SUCCEEDED(hr))
        {
            int rt = MulDiv(48, pSz->cy, 60);
            if (!bIsInitialized)
            {
                ReleaseSRWLockShared(&lock_epw);
                AcquireSRWLockExclusive(&lock_epw);
                epw->lpVtbl->SetTerm(epw, MAX_PATH * sizeof(WCHAR), wszWeatherTerm);
                epw->lpVtbl->SetLanguage(epw, MAX_PATH * sizeof(WCHAR), wszWeatherLanguage);
                UINT dpiX = 0, dpiY = 0;
                HMONITOR hMonitor = MonitorFromWindow(PeopleButton_LastHWND, MONITOR_DEFAULTTOPRIMARY);
                HRESULT hr = GetDpiForMonitor(hMonitor, MDT_DEFAULT, &dpiX, &dpiY);
                MONITORINFO mi;
                ZeroMemory(&mi, sizeof(MONITORINFO));
                mi.cbSize = sizeof(MONITORINFO);
                if (GetMonitorInfoW(hMonitor, &mi))
                {
                    DWORD dwTextScaleFactor = 0, dwSize = sizeof(DWORD);
                    if (SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
                        TEXT("SOFTWARE\\Microsoft\\Accessibility"),
                        TEXT("TextScaleFactor"),
                        SRRF_RT_REG_DWORD,
                        NULL,
                        &dwTextScaleFactor,
                        (LPDWORD)(&dwSize)
                    ) != ERROR_SUCCESS)
                    {
                        dwTextScaleFactor = 100;
                    }

                    RECT rcWeatherFlyoutWindow;
                    rcWeatherFlyoutWindow.left = mi.rcWork.left;
                    rcWeatherFlyoutWindow.top = mi.rcWork.top;
                    rcWeatherFlyoutWindow.right = rcWeatherFlyoutWindow.left + MulDiv(MulDiv(EP_WEATHER_WIDTH, dpiX, 96), dwTextScaleFactor, 100);
                    rcWeatherFlyoutWindow.bottom = rcWeatherFlyoutWindow.top + MulDiv(MulDiv(EP_WEATHER_HEIGHT, dpiX, 96), dwTextScaleFactor, 100);
                    if (FAILED(epw->lpVtbl->Initialize(epw, wszEPWeatherKillswitch, bAllocConsole, EP_WEATHER_PROVIDER_GOOGLE, rt, rt, dwWeatherTemperatureUnit, dwWeatherUpdateSchedule * 1000, rcWeatherFlyoutWindow, dwWeatherTheme, dwWeatherGeolocationMode, &hWndWeatherFlyout)))
                    {
                        epw->lpVtbl->Release(epw);
                        epw = NULL;
                        prev_total_h = 0;
                    }
                    else
                    {
                        epw->lpVtbl->SetWindowCornerPreference(epw, dwWeatherWindowCornerPreference);
                    }
                }
                ReleaseSRWLockExclusive(&lock_epw);
                AcquireSRWLockShared(&lock_epw);
            }
            else
            {
                epw->lpVtbl->SetIconSize(epw, rt, rt);
            }
        }
        else
        {
            if (hr == 0x800706ba) // RPC server is unavailable
            {
                ReleaseSRWLockShared(&lock_epw);
                AcquireSRWLockExclusive(&lock_epw);
                epw = NULL;
                prev_total_h = 0;
                InvalidateRect(PeopleButton_LastHWND, NULL, TRUE);
                ReleaseSRWLockExclusive(&lock_epw);
                AcquireSRWLockShared(&lock_epw);
            }
        }
    }
    ReleaseSRWLockShared(&lock_epw);
    return ret;
}

int PeopleBand_MulDivHook(int nNumber, int nNumerator, int nDenominator)
{
    //printf("[MulDivHook] %d %d %d\n", nNumber, nNumerator, nDenominator);
    AcquireSRWLockShared(&lock_epw);
    if (epw)
    {
        if (bWeatherFixedSize)
        {
            int mul = 1;
            switch (dwWeatherViewMode)
            {
            case EP_WEATHER_VIEW_ICONTEXT:
                mul = 4;
                break;
            case EP_WEATHER_VIEW_TEXTONLY:
                mul = 3;
                break;
            case EP_WEATHER_VIEW_ICONTEMP:
                mul = 2;
                break;
            case EP_WEATHER_VIEW_ICONONLY:
            case EP_WEATHER_VIEW_TEMPONLY:
                mul = 1;
                break;
            }
            ReleaseSRWLockShared(&lock_epw);
            return MulDiv(nNumber * mul, nNumerator, nDenominator);
        }
        else
        {
            if (prev_total_h)
            {
                ReleaseSRWLockShared(&lock_epw);
                return prev_total_h;
            }
            else
            {
                prev_total_h = MulDiv(nNumber, nNumerator, nDenominator);
                ReleaseSRWLockShared(&lock_epw);
                return prev_total_h;
            }
        }
    }
    ReleaseSRWLockShared(&lock_epw);
    return MulDiv(nNumber, nNumerator, nDenominator);
}

DWORD epw_cbTemperature = 0;
DWORD epw_cbUnit = 0;
DWORD epw_cbCondition = 0;
DWORD epw_cbImage = 0;
WCHAR* epw_wszTemperature = NULL;
WCHAR* epw_wszUnit = NULL;
WCHAR* epw_wszCondition = NULL;
char* epw_pImage = NULL;
__int64 (*PeopleBand_DrawTextWithGlowFunc)(
    HDC hdc,
    const unsigned __int16* a2,
    int a3,
    struct tagRECT* a4,
    unsigned int a5,
    unsigned int a6,
    unsigned int a7,
    unsigned int dy,
    unsigned int a9,
    int a10,
    int(__stdcall* a11)(HDC, unsigned __int16*, int, struct tagRECT*, unsigned int, __int64),
    __int64 a12);
__int64 __fastcall PeopleBand_DrawTextWithGlowHook(
    HDC hdc,
    const unsigned __int16* a2,
    int a3,
    struct tagRECT* a4,
    unsigned int a5,
    unsigned int a6,
    unsigned int a7,
    unsigned int dy,
    unsigned int a9,
    int a10,
    int(__stdcall* a11)(HDC, unsigned __int16*, int, struct tagRECT*, unsigned int, __int64),
    __int64 a12)
{
    AcquireSRWLockShared(&lock_epw);
    if (a5 == 0x21 && epw)
    {
        BOOL bUseCachedData = InSendMessage();
        BOOL bIsThemeActive = TRUE;
        if (!IsThemeActive() || IsHighContrast())
        {
            bIsThemeActive = FALSE;
        }
        HRESULT hr = S_OK;
        if (bUseCachedData ? TRUE : SUCCEEDED(hr = epw->lpVtbl->LockData(epw)))
        {
            UINT dpiX = 0, dpiY = 0;
            HRESULT hr = GetDpiForMonitor(MonitorFromWindow(PeopleButton_LastHWND, MONITOR_DEFAULTTOPRIMARY), MDT_DEFAULT, &dpiX, &dpiY);
            BOOL bShouldUnlockData = TRUE;
            DWORD cbTemperature = 0;
            DWORD cbUnit = 0;
            DWORD cbCondition = 0;
            DWORD cbImage = 0;
            BOOL bEmptyData = FALSE;
            if (bUseCachedData ? TRUE : SUCCEEDED(hr = epw->lpVtbl->GetDataSizes(epw, &cbTemperature, &cbUnit, &cbCondition, &cbImage)))
            {
                if (cbTemperature && cbUnit && cbCondition && cbImage)
                {
                    epw_cbTemperature = cbTemperature;
                    epw_cbUnit = cbUnit;
                    epw_cbCondition = cbCondition;
                    epw_cbImage = cbImage;
                }
                else
                {
                    if (!bUseCachedData)
                    {
                        bEmptyData = TRUE;
                        if (bShouldUnlockData)
                        {
                            epw->lpVtbl->UnlockData(epw);
                            bShouldUnlockData = FALSE;
                        }
                    }
                    else
                    {
                        bEmptyData = !epw_wszTemperature || !epw_wszUnit || !epw_wszCondition;
                    }
                    bUseCachedData = TRUE;
                }
                if (!bUseCachedData)
                {
                    if (epw_wszTemperature)
                    {
                        free(epw_wszTemperature);
                    }
                    epw_wszTemperature = calloc(1, epw_cbTemperature);
                    if (epw_wszUnit)
                    {
                        free(epw_wszUnit);
                    }
                    epw_wszUnit = calloc(1, epw_cbUnit);
                    if (epw_wszCondition)
                    {
                        free(epw_wszCondition);
                    }
                    epw_wszCondition = calloc(1, epw_cbCondition);
                    if (epw_pImage)
                    {
                        free(epw_pImage);
                    }
                    epw_pImage = calloc(1, epw_cbImage);
                }
                if (bUseCachedData ? TRUE : SUCCEEDED(hr = epw->lpVtbl->GetData(epw, epw_cbTemperature, epw_wszTemperature, epw_cbUnit, epw_wszUnit, epw_cbCondition, epw_wszCondition, epw_cbImage, epw_pImage)))
                {
                    if (!bUseCachedData)
                    {
                        WCHAR wszBuffer[MAX_PATH];
                        ZeroMemory(wszBuffer, sizeof(WCHAR) * MAX_PATH);
                        swprintf_s(wszBuffer, MAX_PATH, L"%s %s, %s, ", epw_wszTemperature, epw_wszUnit, epw_wszCondition);
                        int len = wcslen(wszBuffer);
                        epw->lpVtbl->GetTitle(epw, sizeof(WCHAR) * (MAX_PATH - len), wszBuffer + len, dwWeatherViewMode);
                        SetWindowTextW(PeopleButton_LastHWND, wszBuffer);

                        epw->lpVtbl->UnlockData(epw);
                        bShouldUnlockData = FALSE;
                    }

                    LOGFONTW logFont;
                    ZeroMemory(&logFont, sizeof(logFont));
                    NONCLIENTMETRICS ncm;
                    ZeroMemory(&ncm, sizeof(NONCLIENTMETRICS));
                    ncm.cbSize = sizeof(NONCLIENTMETRICS);
                    SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0, dpiX);
                    logFont = ncm.lfCaptionFont;
                    logFont.lfWeight = FW_NORMAL;
                    if (bEmptyData)
                    {
                        DWORD dwVal = 1, dwSize = sizeof(DWORD);
                        RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", L"TaskbarSmallIcons", RRF_RT_DWORD, NULL, &dwVal, &dwSize);
                        if (!dwVal)
                        {
                            logFont.lfHeight *= 1.6;
                        }
                    }
                    else
                    {
                        if (dwWeatherViewMode == EP_WEATHER_VIEW_ICONTEXT)
                        {
                            //logFont.lfHeight = -12 * (dpiX / 96.0);
                        }
                    }
                    HFONT hFont = CreateFontIndirectW(&logFont);
                    if (hFont)
                    {
                        HDC hDC = CreateCompatibleDC(0);
                        if (hDC)
                        {
                            COLORREF rgbColor = RGB(0, 0, 0);
                            if (bIsThemeActive)
                            {
                                if (ShouldSystemUseDarkMode && ShouldSystemUseDarkMode())
                                {
                                    rgbColor = RGB(255, 255, 255);
                                }
                            }
                            else
                            {
                                rgbColor = GetSysColor(COLOR_WINDOWTEXT);
                            }
                            HFONT hOldFont = SelectFont(hDC, hFont);
                            if (bEmptyData)
                            {
                                RECT rcText;
                                SetRect(&rcText, 0, 0, a4->right, a4->bottom);
                                SIZE size;
                                size.cx = rcText.right - rcText.left;
                                size.cy = rcText.bottom - rcText.top;
                                DWORD dwTextFlags = DT_SINGLELINE | DT_VCENTER | DT_HIDEPREFIX | DT_CENTER;
                                HBITMAP hBitmap = sws_WindowHelpers_CreateAlphaTextBitmap(L"\U0001f4f0", hFont, dwTextFlags, size, rgbColor);
                                if (hBitmap)
                                {
                                    HBITMAP hOldBMP = SelectBitmap(hDC, hBitmap);
                                    BITMAP BMInf;
                                    GetObjectW(hBitmap, sizeof(BITMAP), &BMInf);

                                    BLENDFUNCTION bf;
                                    bf.BlendOp = AC_SRC_OVER;
                                    bf.BlendFlags = 0;
                                    bf.SourceConstantAlpha = 0xFF;
                                    bf.AlphaFormat = AC_SRC_ALPHA;
                                    GdiAlphaBlend(hdc, 0, 0, BMInf.bmWidth, BMInf.bmHeight, hDC, 0, 0, BMInf.bmWidth, BMInf.bmHeight, bf);

                                    SelectBitmap(hDC, hOldBMP);
                                    DeleteBitmap(hBitmap);
                                }
                            }
                            else
                            {
                                DWORD dwTextFlags = DT_SINGLELINE | DT_VCENTER | DT_HIDEPREFIX;

                                WCHAR wszText1[MAX_PATH];
                                swprintf_s(wszText1, MAX_PATH, L"%s%s %s", bIsThemeActive ? L"" : L" ", epw_wszTemperature, dwWeatherTemperatureUnit == EP_WEATHER_TUNIT_FAHRENHEIT ? L"\u00B0F" : L"\u00B0C");// epw_wszUnit);
                                RECT rcText1;
                                SetRect(&rcText1, 0, 0, a4->right, a4->bottom);
                                DrawTextW(hDC, wszText1, -1, &rcText1, dwTextFlags | DT_CALCRECT);
                                rcText1.bottom = a4->bottom;
                                WCHAR wszText2[MAX_PATH];
                                swprintf_s(wszText2, MAX_PATH, L"%s%s", bIsThemeActive ? L"" : L" ", epw_wszCondition);
                                RECT rcText2;
                                SetRect(&rcText2, 0, 0, a4->right, a4->bottom);
                                DrawTextW(hDC, wszText2, -1, &rcText2, dwTextFlags | DT_CALCRECT);
                                rcText2.bottom = a4->bottom;

                                if (bWeatherFixedSize)
                                {
                                    dwTextFlags |= DT_END_ELLIPSIS;
                                }

                                int addend = 0;
                                //int rt = MulDiv(48, a4->bottom, 60);
                                int rt = sqrt(epw_cbImage / 4);
                                int p = 0;// MulDiv(rt, 4, 64);
                                int margin_h = MulDiv(12, dpiX, 144);

                                BOOL bIsIconMode = (
                                    dwWeatherViewMode == EP_WEATHER_VIEW_ICONTEMP ||
                                    dwWeatherViewMode == EP_WEATHER_VIEW_ICONTEXT ||
                                    dwWeatherViewMode == EP_WEATHER_VIEW_ICONONLY);
                                switch (dwWeatherViewMode)
                                {
                                case EP_WEATHER_VIEW_ICONTEXT:
                                case EP_WEATHER_VIEW_TEXTONLY:
                                    addend = (rcText1.right - rcText1.left) + margin_h + (rcText2.right - rcText2.left) + margin_h;
                                    break;
                                case EP_WEATHER_VIEW_ICONTEMP:
                                case EP_WEATHER_VIEW_TEMPONLY:
                                    addend = (rcText1.right - rcText1.left) + margin_h;
                                    break;
                                case EP_WEATHER_VIEW_ICONONLY:
                                    addend = 0;
                                    break;
                                }
                                int margin_v = (a4->bottom - rt) / 2;
                                int total_h = (bIsIconMode ? ((margin_h - p) + rt + (margin_h - p)) : margin_h) + addend;
                                if (bWeatherFixedSize)
                                {
                                    if (total_h > a4->right)
                                    {
                                        int diff = total_h - a4->right;
                                        rcText2.right -= diff - 2;
                                        switch (dwWeatherViewMode)
                                        {
                                        case EP_WEATHER_VIEW_ICONTEXT:
                                        case EP_WEATHER_VIEW_TEXTONLY:
                                            addend = (rcText1.right - rcText1.left) + margin_h + (rcText2.right - rcText2.left) + margin_h;
                                            break;
                                        case EP_WEATHER_VIEW_ICONTEMP:
                                        case EP_WEATHER_VIEW_TEMPONLY: // should be impossible
                                            addend = (rcText1.right - rcText1.left) + margin_h;
                                            break;
                                        case EP_WEATHER_VIEW_ICONONLY:
                                            addend = 0;
                                            break;
                                        }
                                        total_h = (margin_h - p) + rt + (margin_h - p) + addend;
                                    }
                                }
                                int start_x = 0; // prev_total_h - total_h;
                                if (bWeatherFixedSize)
                                {
                                    start_x = (a4->right - total_h) / 2;
                                }

                                HBITMAP hBitmap = NULL, hOldBitmap = NULL;
                                void* pvBits = NULL;
                                SIZE size;

                                if (bIsIconMode)
                                {
                                    BITMAPINFOHEADER BMIH;
                                    ZeroMemory(&BMIH, sizeof(BITMAPINFOHEADER));
                                    BMIH.biSize = sizeof(BITMAPINFOHEADER);
                                    BMIH.biWidth = rt;
                                    BMIH.biHeight = -rt;
                                    BMIH.biPlanes = 1;
                                    BMIH.biBitCount = 32;
                                    BMIH.biCompression = BI_RGB;
                                    hBitmap = CreateDIBSection(hDC, &BMIH, 0, &pvBits, NULL, 0);
                                    if (hBitmap)
                                    {
                                        memcpy(pvBits, epw_pImage, epw_cbImage);
                                        hOldBitmap = SelectBitmap(hDC, hBitmap);

                                        BLENDFUNCTION bf;
                                        bf.BlendOp = AC_SRC_OVER;
                                        bf.BlendFlags = 0;
                                        bf.SourceConstantAlpha = 0xFF;
                                        bf.AlphaFormat = AC_SRC_ALPHA;
                                        GdiAlphaBlend(hdc, start_x + (margin_h - p), margin_v, rt, rt, hDC, 0, 0, rt, rt, bf);

                                        SelectBitmap(hDC, hOldBitmap);
                                        DeleteBitmap(hBitmap);
                                    }
                                }

                                if (dwWeatherViewMode == EP_WEATHER_VIEW_ICONTEMP || 
                                    dwWeatherViewMode == EP_WEATHER_VIEW_ICONTEXT ||
                                    dwWeatherViewMode == EP_WEATHER_VIEW_TEMPONLY ||
                                    dwWeatherViewMode == EP_WEATHER_VIEW_TEXTONLY
                                    )
                                {
                                    size.cx = rcText1.right - rcText1.left;
                                    size.cy = rcText1.bottom - rcText1.top;
                                    hBitmap = sws_WindowHelpers_CreateAlphaTextBitmap(wszText1, hFont, dwTextFlags, size, rgbColor);
                                    if (hBitmap)
                                    {
                                        HBITMAP hOldBMP = SelectBitmap(hDC, hBitmap);
                                        BITMAP BMInf;
                                        GetObjectW(hBitmap, sizeof(BITMAP), &BMInf);

                                        BLENDFUNCTION bf;
                                        bf.BlendOp = AC_SRC_OVER;
                                        bf.BlendFlags = 0;
                                        bf.SourceConstantAlpha = 0xFF;
                                        bf.AlphaFormat = AC_SRC_ALPHA;
                                        GdiAlphaBlend(hdc, start_x + (bIsIconMode ? ((margin_h - p) + rt + (margin_h - p)) : margin_h), 0, BMInf.bmWidth, BMInf.bmHeight, hDC, 0, 0, BMInf.bmWidth, BMInf.bmHeight, bf);

                                        SelectBitmap(hDC, hOldBMP);
                                        DeleteBitmap(hBitmap);
                                    }
                                }

                                if (dwWeatherViewMode == EP_WEATHER_VIEW_ICONTEXT ||
                                    dwWeatherViewMode == EP_WEATHER_VIEW_TEXTONLY
                                    )
                                {
                                    size.cx = rcText2.right - rcText2.left;
                                    size.cy = rcText2.bottom - rcText2.top;
                                    hBitmap = sws_WindowHelpers_CreateAlphaTextBitmap(wszText2, hFont, dwTextFlags, size, rgbColor);
                                    if (hBitmap)
                                    {
                                        HBITMAP hOldBMP = SelectBitmap(hDC, hBitmap);
                                        BITMAP BMInf;
                                        GetObjectW(hBitmap, sizeof(BITMAP), &BMInf);

                                        BLENDFUNCTION bf;
                                        bf.BlendOp = AC_SRC_OVER;
                                        bf.BlendFlags = 0;
                                        bf.SourceConstantAlpha = 0xFF;
                                        bf.AlphaFormat = AC_SRC_ALPHA;
                                        GdiAlphaBlend(hdc, start_x + (bIsIconMode ? ((margin_h - p) + rt + (margin_h - p)) : margin_h) + (rcText1.right - rcText1.left) + margin_h, 0, BMInf.bmWidth, BMInf.bmHeight, hDC, 0, 0, BMInf.bmWidth, BMInf.bmHeight, bf);

                                        SelectBitmap(hDC, hOldBMP);
                                        DeleteBitmap(hBitmap);
                                    }
                                }

                                if (bWeatherFixedSize)
                                {

                                }
                                else
                                {
                                    if (total_h != prev_total_h)
                                    {
                                        prev_total_h = total_h;
                                        SendNotifyMessageW(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)L"TraySettings");
                                    }
                                }

                                /*
                                SetLastError(0);
                                LONG_PTR oldStyle = GetWindowLongPtrW(PeopleButton_LastHWND, GWL_EXSTYLE);
                                if (!GetLastError())
                                {
                                    LONG_PTR style;
                                    if (bIsThemeActive)
                                    {
                                        style = oldStyle & ~WS_EX_DLGMODALFRAME;
                                    }
                                    else
                                    {
                                        style = oldStyle | WS_EX_DLGMODALFRAME;
                                    }
                                    if (style != oldStyle)
                                    {
                                        SetWindowLongPtrW(PeopleButton_LastHWND, GWL_EXSTYLE, style);
                                    }
                                }
                                */
                            }
                            SelectFont(hDC, hOldFont);
                            DeleteDC(hDC);
                        }
                    }
                    if (IsWindowVisible(hWndWeatherFlyout))
                    {
                        RecomputeWeatherFlyoutLocation(hWndWeatherFlyout);
                    }
                }
                /*free(epw_pImage);
                free(epw_wszCondition);
                free(epw_wszUnit);
                free(epw_wszTemperature);*/
            }
            if (!bUseCachedData && bShouldUnlockData)
            {
                epw->lpVtbl->UnlockData(epw);
            }
        }
        else
        {
            //printf("444444444444 0x%x\n", hr);
            if (hr == 0x800706ba) // RPC server is unavailable
            {
                ReleaseSRWLockShared(&lock_epw);
                AcquireSRWLockExclusive(&lock_epw);
                epw = NULL;
                prev_total_h = 0;
                InvalidateRect(PeopleButton_LastHWND, NULL, TRUE); 
                ReleaseSRWLockExclusive(&lock_epw);
                AcquireSRWLockShared(&lock_epw);
            }
        }

        //printf("hr %x\n", hr);

        ReleaseSRWLockShared(&lock_epw);
        return S_OK;
    }
    else
    {
        ReleaseSRWLockShared(&lock_epw);
        return PeopleBand_DrawTextWithGlowFunc(hdc, a2, a3, a4, a5, a6, a7, dy, a9, a10, a11, a12);
    }
}

void(*PeopleButton_ShowTooltipFunc)(__int64 a1, unsigned __int8 bShow) = 0;
void WINAPI PeopleButton_ShowTooltipHook(__int64 _this, unsigned __int8 bShow)
{
    AcquireSRWLockShared(&lock_epw);
    if (epw)
    {
        if (bShow)
        {
            HRESULT hr = epw->lpVtbl->LockData(epw);
            if (SUCCEEDED(hr))
            {
                WCHAR wszBuffer[MAX_PATH];
                ZeroMemory(wszBuffer, sizeof(WCHAR) * MAX_PATH);
                epw->lpVtbl->GetTitle(epw, sizeof(WCHAR) * MAX_PATH, wszBuffer, dwWeatherViewMode);
                if (wcsstr(wszBuffer, L"(null)"))
                {
                    HMODULE hModule = GetModuleHandleW(L"pnidui.dll");
                    if (hModule)
                    {
                        LoadStringW(hModule, 35, wszBuffer, MAX_PATH);
                    }
                }
                TTTOOLINFOW ti;
                ZeroMemory(&ti, sizeof(TTTOOLINFOW));
                ti.cbSize = sizeof(TTTOOLINFOW);
                ti.hwnd = *((INT64*)_this + 1);
                ti.uId = *((INT64*)_this + 1);
                ti.lpszText = wszBuffer;
                SendMessageW((HWND) * ((INT64*)_this + 10), TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
                epw->lpVtbl->UnlockData(epw);
            }
        }
    }
    else
    {
        WCHAR wszBuffer[MAX_PATH];
        ZeroMemory(wszBuffer, sizeof(WCHAR) * MAX_PATH);
        LoadStringW(GetModuleHandle(NULL), 912, wszBuffer, MAX_PATH);
        if (wszBuffer[0])
        {
            TTTOOLINFOW ti;
            ZeroMemory(&ti, sizeof(TTTOOLINFOW));
            ti.cbSize = sizeof(TTTOOLINFOW);
            ti.hwnd = *((INT64*)_this + 1);
            ti.uId = *((INT64*)_this + 1);
            ti.lpszText = wszBuffer;
            SendMessageW((HWND) * ((INT64*)_this + 10), TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
        }
    }
    ReleaseSRWLockShared(&lock_epw);
    if (PeopleButton_ShowTooltipFunc)
    {
        return PeopleButton_ShowTooltipFunc(_this, bShow);
    }
    return 0;
}

__int64 (*PeopleButton_OnClickFunc)(__int64 a1, __int64 a2) = 0;
__int64 PeopleButton_OnClickHook(__int64 a1, __int64 a2)
{
    AcquireSRWLockShared(&lock_epw);
    if (epw)
    {
        if (!hWndWeatherFlyout)
        {
            epw->lpVtbl->GetWindowHandle(epw, &hWndWeatherFlyout);
        }
        if (hWndWeatherFlyout)
        {
            if (IsWindowVisible(hWndWeatherFlyout))
            {
                if (GetForegroundWindow() != hWndWeatherFlyout)
                {
                    SwitchToThisWindow(hWndWeatherFlyout, TRUE);
                }
                else
                {
                    epw->lpVtbl->Hide(epw);
                    //printf("HR %x\n", PostMessageW(hWnd, EP_WEATHER_WM_FETCH_DATA, 0, 0));
                }
            }
            else
            {
                RecomputeWeatherFlyoutLocation(hWndWeatherFlyout);

                epw->lpVtbl->Show(epw);

                SwitchToThisWindow(hWndWeatherFlyout, TRUE);
            }
        }
        ReleaseSRWLockShared(&lock_epw);
        return 0;
    }
    else
    {
        ReleaseSRWLockShared(&lock_epw);
        if (PeopleButton_OnClickFunc)
        {
            return PeopleButton_OnClickFunc(a1, a2);
        }
        return 0;
    }
}

INT64 PeopleButton_SubclassProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    UINT_PTR    uIdSubclass,
    DWORD_PTR   dwRefData
)
{
    if (uMsg == WM_NCDESTROY)
    {
        RemoveWindowSubclass(hWnd, PeopleButton_SubclassProc, PeopleButton_SubclassProc);
        AcquireSRWLockExclusive(&lock_epw);
        if (epw)
        {
            epw->lpVtbl->Release(epw);
            epw = NULL;
            PeopleButton_LastHWND = NULL;
            prev_total_h = 0;
        }
        ReleaseSRWLockExclusive(&lock_epw);
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


static BOOL(*SetChildWindowNoActivateFunc)(HWND);
BOOL explorer_SetChildWindowNoActivateHook(HWND hWnd)
{
    TCHAR className[100];
    ZeroMemory(className, 100);
    GetClassNameW(hWnd, className, 100);
    if (!wcscmp(className, L"ControlCenterButton"))
    {
        lpShouldDisplayCCButton = (BYTE*)(GetWindowLongPtrW(hWnd, 0) + 120);
        if (*lpShouldDisplayCCButton)
        {
            *lpShouldDisplayCCButton = !bHideControlCenterButton;
        }
    }
    // get a look at vtable by searching for v_IsEnabled
    if (!wcscmp(className, L"TrayButton"))
    {
        uintptr_t Instance = *(uintptr_t*)GetWindowLongPtrW(hWnd, 0);
        uintptr_t TrayButton_GetComponentName = *(INT_PTR(WINAPI**)())(Instance + 304);
        if (!IsBadCodePtr(TrayButton_GetComponentName))
        {
            wchar_t* wszComponentName = (const WCHAR*)(*(uintptr_t (**)(void))(Instance + 304))();
            if (!wcscmp(wszComponentName, L"CortanaButton"))
            {
                DWORD dwOldProtect;
                VirtualProtect(Instance + 160, sizeof(uintptr_t), PAGE_READWRITE, &dwOldProtect);
                if (!Widgets_OnClickFunc) Widgets_OnClickFunc = *(uintptr_t*)(Instance + 160);
                *(uintptr_t*)(Instance + 160) = Widgets_OnClickHook;    // OnClick
                VirtualProtect(Instance + 160, sizeof(uintptr_t), dwOldProtect, &dwOldProtect);
                VirtualProtect(Instance + 216, sizeof(uintptr_t), PAGE_READWRITE, &dwOldProtect);
                if (!Widgets_GetTooltipTextFunc) Widgets_GetTooltipTextFunc = *(uintptr_t*)(Instance + 216);
                *(uintptr_t*)(Instance + 216) = Widgets_GetTooltipTextHook; // OnTooltipShow
                VirtualProtect(Instance + 216, sizeof(uintptr_t), dwOldProtect, &dwOldProtect);
            }
            else if (!wcscmp(wszComponentName, L"MultitaskingButton"))
            {
                DWORD dwOldProtect;
                VirtualProtect(Instance + 160, sizeof(uintptr_t), PAGE_READWRITE, &dwOldProtect);
                *(uintptr_t*)(Instance + 160) = ToggleTaskView;    // OnClick
                VirtualProtect(Instance + 160, sizeof(uintptr_t), dwOldProtect, &dwOldProtect);
            }
            else if (!wcscmp(wszComponentName, L"PeopleButton"))
            {
                DWORD dwOldProtect;

                uintptr_t PeopleButton_Instance = *((uintptr_t*)GetWindowLongPtrW(hWnd, 0) + 17);

                VirtualProtect(PeopleButton_Instance + 32, sizeof(uintptr_t), PAGE_READWRITE, &dwOldProtect);
                if (!PeopleButton_CalculateMinimumSizeFunc) PeopleButton_CalculateMinimumSizeFunc = *(uintptr_t*)(PeopleButton_Instance + 32);
                *(uintptr_t*)(PeopleButton_Instance + 32) = PeopleButton_CalculateMinimumSizeHook; // CalculateMinimumSize
                VirtualProtect(PeopleButton_Instance + 32, sizeof(uintptr_t), dwOldProtect, &dwOldProtect);

                VirtualProtect(Instance + 224, sizeof(uintptr_t), PAGE_READWRITE, &dwOldProtect);
                if (!PeopleButton_ShowTooltipFunc) PeopleButton_ShowTooltipFunc = *(uintptr_t*)(Instance + 224);
                *(uintptr_t*)(Instance + 224) = PeopleButton_ShowTooltipHook; // OnTooltipShow
                VirtualProtect(Instance + 224, sizeof(uintptr_t), dwOldProtect, &dwOldProtect);

                VirtualProtect(Instance + 160, sizeof(uintptr_t), PAGE_READWRITE, &dwOldProtect);
                if (!PeopleButton_OnClickFunc) PeopleButton_OnClickFunc = *(uintptr_t*)(Instance + 160);
                *(uintptr_t*)(Instance + 160) = PeopleButton_OnClickHook;    // OnClick
                VirtualProtect(Instance + 160, sizeof(uintptr_t), dwOldProtect, &dwOldProtect);

                PeopleButton_LastHWND = hWnd;
                SetWindowSubclass(hWnd, PeopleButton_SubclassProc, PeopleButton_SubclassProc, 0);

                AcquireSRWLockExclusive(&lock_epw);
                if (!epw)
                {
                    if (SUCCEEDED(CoCreateInstance(&CLSID_EPWeather, NULL, CLSCTX_LOCAL_SERVER, &IID_IEPWeather, &epw)) && epw)
                    {
                        epw->lpVtbl->SetNotifyWindow(epw, hWnd);

                        WCHAR wszBuffer[MAX_PATH];
                        ZeroMemory(wszBuffer, sizeof(WCHAR) * MAX_PATH);
                        HMODULE hModule = GetModuleHandleW(L"pnidui.dll");
                        if (hModule)
                        {
                            LoadStringW(hModule, 35, wszBuffer, MAX_PATH);
                        }
                        SetWindowTextW(hWnd, wszBuffer);
                    }
                }
                ReleaseSRWLockExclusive(&lock_epw);
            }
        }
    }
    return SetChildWindowNoActivateFunc(hWnd);
}
#endif
#pragma endregion


#pragma region "Hide Show desktop button"
#ifdef _WIN64
INT64 ShowDesktopSubclassProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    UINT_PTR    uIdSubclass,
    DWORD_PTR   dwRefData
)
{
    if (uMsg == WM_NCDESTROY)
    {
        RemoveWindowSubclass(hWnd, ShowDesktopSubclassProc, ShowDesktopSubclassProc);
    }
    else if (uMsg == WM_USER + 100)
    {
        LRESULT lRes = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        if (lRes > 0)
        {
            DWORD dwVal = 0, dwSize = sizeof(DWORD);
            if (SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
                TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
                TEXT("TaskbarSd"),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwVal,
                (LPDWORD)(&dwSize)
            ) == ERROR_SUCCESS && !dwVal)
            {
                lRes = 0;
            }
        }
        return lRes;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
#endif
#pragma endregion


#pragma region "Notify shell ready (fixes delay at logon)"
#ifdef _WIN64
DWORD SignalShellReady(DWORD wait)
{
    printf("Started \"Signal shell ready\" thread.\n");
    //UpdateStartMenuPositioning(MAKELPARAM(TRUE, TRUE));

    while (!wait && TRUE)
    {
        HWND hShell_TrayWnd = FindWindowEx(
            NULL,
            NULL,
            L"Shell_TrayWnd",
            NULL
        );
        if (hShell_TrayWnd)
        {
            HWND hWnd = FindWindowEx(
                hShell_TrayWnd,
                NULL,
                L"Start",
                NULL
            );
            if (hWnd)
            {
                if (IsWindowVisible(hWnd))
                {
                    UpdateStartMenuPositioning(MAKELPARAM(TRUE, TRUE));
                    break;
                }
            }
        }
        Sleep(100);
    }

    if (!wait)
    {
        Sleep(600);
    }
    else
    {
        Sleep(wait);
    }

    HANDLE hEvent = CreateEventW(0, 0, 0, L"ShellDesktopSwitchEvent");
    if (hEvent)
    {
        printf(">>> Signal shell ready.\n");
        SetEvent(hEvent);
    }
    SetEvent(hCanStartSws);

    printf("Ended \"Signal shell ready\" thread.\n");
    return 0;
}
#endif
#pragma endregion


#pragma region "Window Switcher"
#ifdef _WIN64
DWORD sws_IsEnabled = FALSE;

void sws_ReadSettings(sws_WindowSwitcher* sws)
{
    HKEY hKey = NULL;
    DWORD dwSize = 0;

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        DWORD val = 0;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("AltTabSettings"),
            0,
            NULL,
            &val,
            &dwSize
        );
        sws_IsEnabled = (val == 2);
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH) L"\\sws",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        if (sws)
        {
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("IncludeWallpaper"),
                0,
                NULL,
                &(sws->bIncludeWallpaper),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("RowHeight"),
                0,
                NULL,
                &(sws->dwRowHeight),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("MaxWidth"),
                0,
                NULL,
                &(sws->dwMaxWP),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("MaxHeight"),
                0,
                NULL,
                &(sws->dwMaxHP),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("ColorScheme"),
                0,
                NULL,
                &(sws->dwColorScheme),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("Theme"),
                0,
                NULL,
                &(sws->dwTheme),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("CornerPreference"),
                0,
                NULL,
                &(sws->dwCornerPreference),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("ShowDelay"),
                0,
                NULL,
                &(sws->dwShowDelay),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("PrimaryOnly"),
                0,
                NULL,
                &(sws->bPrimaryOnly),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("PerMonitor"),
                0,
                NULL,
                &(sws->bPerMonitor),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("MaxWidthAbs"),
                0,
                NULL,
                &(sws->dwMaxAbsoluteWP),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("MaxHeightAbs"),
                0,
                NULL,
                &(sws->dwMaxAbsoluteHP),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("NoPerApplicationList"),
                0,
                NULL,
                &(sws->bNoPerApplicationList),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("MasterPadding"),
                0,
                NULL,
                &(sws->dwMasterPadding),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("SwitcherIsPerApplication"),
                0,
                NULL,
                &(sws->bSwitcherIsPerApplication),
                &dwSize
            );
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("AlwaysUseWindowTitleAndIcon"),
                0,
                NULL,
                &(sws->bAlwaysUseWindowTitleAndIcon),
                &dwSize
            );
            if (sws->bIsInitialized)
            {
                sws_WindowSwitcher_UnregisterHotkeys(sws);
                sws_WindowSwitcher_RegisterHotkeys(sws, NULL);
                sws_WindowSwitcher_RefreshTheme(sws);
            }
        }
        RegCloseKey(hKey);
    }
}

DWORD WindowSwitcher(DWORD unused)
{
    WaitForSingleObject(hCanStartSws, INFINITE);
    if (!bOldTaskbar)
    {
        WaitForSingleObject(hWin11AltTabInitialized, INFINITE);
    }
    Sleep(1000);

    while (TRUE)
    {
        //Sleep(5000);
        while (!FindWindowExW(
            NULL,
            NULL,
            L"Shell_TrayWnd",
            NULL
        ))
        {
            printf("[sws] Waiting for taskbar...\n");
            Sleep(100);
        }
        Sleep(100);
        sws_ReadSettings(NULL);
        if (sws_IsEnabled)
        {
            sws_error_t err;
            sws_WindowSwitcher* sws = calloc(1, sizeof(sws_WindowSwitcher));
            if (!sws)
            {
                return 0;
            }
            sws_WindowSwitcher_InitializeDefaultSettings(sws);
            sws->dwWallpaperSupport = SWS_WALLPAPERSUPPORT_EXPLORER;
            sws_ReadSettings(sws);
            err = sws_error_Report(sws_error_GetFromInternalError(sws_WindowSwitcher_Initialize(&sws, FALSE)), NULL);
            if (err == SWS_ERROR_SUCCESS)
            {
                sws_WindowSwitcher_RefreshTheme(sws);
                HANDLE hEvents[3];
                hEvents[0] = sws->hEvExit;
                hEvents[1] = hSwsSettingsChanged;
                hEvents[2] = hSwsOpacityMaybeChanged;
                while (TRUE)
                {
                    DWORD dwRes = MsgWaitForMultipleObjectsEx(
                        3,
                        hEvents,
                        INFINITE,
                        QS_ALLINPUT,
                        MWMO_INPUTAVAILABLE
                    );
                    if (dwRes == WAIT_OBJECT_0 + 0)
                    {
                        break;
                    }
                    if (dwRes == WAIT_OBJECT_0 + 1)
                    {
                        sws_ReadSettings(sws);
                        if (!sws_IsEnabled)
                        {
                            break;
                        }
                    }
                    else if (dwRes == WAIT_OBJECT_0 + 2)
                    {
                        sws_WindowSwitcher_RefreshTheme(sws);
                    }
                    else if (dwRes == WAIT_OBJECT_0 + 3)
                    {
                        MSG msg;
                        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                sws_WindowSwitcher_Clear(sws);
                free(sws);
            }
            else
            {
                return 0;
            }
        }
        else
        {
            WaitForSingleObject(
                hSwsSettingsChanged,
                INFINITE
            );
        }
    }
}
#endif
#pragma endregion


#pragma region "Load Settings from registry"
#define REFRESHUI_NONE    0b0000
#define REFRESHUI_GLOM    0b0001
#define REFRESHUI_ORB     0b0010
#define REFRESHUI_PEOPLE  0b0100
#define REFRESHUI_TASKBAR 0b1000
void WINAPI LoadSettings(LPARAM lParam)
{
    BOOL bIsExplorer = LOWORD(lParam);
    BOOL bIsRefreshAllowed = HIWORD(lParam);
    DWORD dwRefreshUIMask = REFRESHUI_NONE;

    HKEY hKey = NULL;
    DWORD dwSize = 0, dwTemp = 0;

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
#ifdef _WIN64
        dwSize = sizeof(DWORD);
        dwTemp = 0;
        RegQueryValueExW(
            hKey,
            TEXT("MigratedFromOldSettings"),
            0,
            NULL,
            &dwTemp,
            &dwSize
        );
        if (!dwTemp)
        {
            HKEY hOldKey = NULL;
            RegOpenKeyExW(
                HKEY_CURRENT_USER,
                TEXT(REGPATH_OLD),
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS | KEY_WOW64_64KEY,
                &hOldKey
            );
            if (hOldKey == NULL || hOldKey == INVALID_HANDLE_VALUE)
            {
                hOldKey = NULL;
            }
            if (hOldKey)
            {
                dwSize = sizeof(DWORD);
                DWORD dw1 = 0;
                RegQueryValueExW(
                    hKey,
                    TEXT("OpenPropertiesAtNextStart"),
                    0,
                    NULL,
                    &dw1,
                    &dwSize
                );
                dwSize = sizeof(DWORD);
                DWORD dw2 = 0;
                RegQueryValueExW(
                    hKey,
                    TEXT("IsUpdatePending"),
                    0,
                    NULL,
                    &dw2,
                    &dwSize
                );
                if (RegCopyTreeW(hOldKey, NULL, hKey) == ERROR_SUCCESS)
                {
                    RegSetValueExW(
                        hKey,
                        TEXT("OpenPropertiesAtNextStart"),
                        0,
                        REG_DWORD,
                        &dw1,
                        sizeof(DWORD)
                    );
                    RegSetValueExW(
                        hKey,
                        TEXT("IsUpdatePending"),
                        0,
                        REG_DWORD,
                        &dw2,
                        sizeof(DWORD)
                    );
                    RegDeleteKeyExW(hKey, TEXT(STARTDOCKED_SB_NAME), KEY_WOW64_64KEY, 0);
                    DWORD dwTaskbarGlomLevel = 0, dwMMTaskbarGlomLevel = 0;
                    dwSize = sizeof(DWORD);
                    RegGetValueW(
                        HKEY_CURRENT_USER,
                        L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
                        L"TaskbarGlomLevel",
                        REG_DWORD,
                        NULL,
                        &dwTaskbarGlomLevel,
                        &dwSize
                    );
                    RegSetValueExW(
                        hKey,
                        TEXT("TaskbarGlomLevel"),
                        0,
                        REG_DWORD,
                        &dwTaskbarGlomLevel,
                        sizeof(DWORD)
                    );
                    dwSize = sizeof(DWORD);
                    RegGetValueW(
                        HKEY_CURRENT_USER,
                        L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
                        L"MMTaskbarGlomLevel",
                        REG_DWORD,
                        NULL,
                        &dwMMTaskbarGlomLevel,
                        &dwSize
                    );
                    RegSetValueExW(
                        hKey,
                        TEXT("MMTaskbarGlomLevel"),
                        0,
                        REG_DWORD,
                        &dwMMTaskbarGlomLevel,
                        sizeof(DWORD)
                    );
                }
            }
            dwTemp = TRUE;
            RegSetValueExW(
                hKey,
                TEXT("MigratedFromOldSettings"),
                0,
                REG_DWORD,
                &dwTemp,
                sizeof(DWORD)
            );
        }
#endif
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("AllocConsole"),
            0,
            NULL,
            &bAllocConsole,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        dwTemp = 0;
        RegQueryValueExW(
            hKey,
            TEXT("Memcheck"),
            0,
            NULL,
            &dwTemp,
            &dwSize
        );
        if (dwTemp)
        {
#if defined(DEBUG) | defined(_DEBUG)
            printf("[Memcheck] Dumping memory leaks...\n");
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
            _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
            _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
            _CrtDumpMemoryLeaks();
            printf("[Memcheck] Memory leak dump complete.\n");
            printf(
                "[Memcheck] Objects in use:\nGDI\tGDIp\tUSER\tUSERp\n%d\t%d\t%d\t%d\n",
                GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS),
                GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS_PEAK),
                GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS),
                GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS_PEAK)
            );
#endif
            dwTemp = 0;
            RegSetValueExW(
                hKey,
                TEXT("Memcheck"),
                0,
                REG_DWORD,
                &dwTemp,
                sizeof(DWORD)
            );
        }
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("HideExplorerSearchBar"),
            0,
            NULL,
            &bHideExplorerSearchBar,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("DisableImmersiveContextMenu"),
            0,
            NULL,
            &bDisableImmersiveContextMenu,
            &dwSize
        );
        dwTemp = FALSE;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ClassicThemeMitigations"),
            0,
            NULL,
            &dwTemp,
            &dwSize
        );
        if (!bWasClassicThemeMitigationsSet)
        {
            bClassicThemeMitigations = dwTemp;
            bWasClassicThemeMitigationsSet = TRUE;
        }
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("SkinMenus"),
            0,
            NULL,
            &bSkinMenus,
            &dwSize
        );
        if (bIsExplorerProcess)
        {
            if (bAllocConsole)
            {
                FILE* conout;
                AllocConsole();
                freopen_s(
                    &conout,
                    "CONOUT$",
                    "w",
                    stdout
                );
            }
            else
            {
                FreeConsole();
            }
        }
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("DoNotRedirectSystemToSettingsApp"),
            0,
            NULL,
            &bDoNotRedirectSystemToSettingsApp,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("DoNotRedirectProgramsAndFeaturesToSettingsApp"),
            0,
            NULL,
            &bDoNotRedirectProgramsAndFeaturesToSettingsApp,
            &dwSize
        );
        if (!bIsExplorer)
        {
            RegCloseKey(hKey);
            return;
        }
        dwTemp = TRUE;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("OldTaskbar"),
            0,
            NULL,
            &dwTemp,
            &dwSize
        );
        if (!bWasOldTaskbarSet)
        {
            bOldTaskbar = dwTemp;
            bWasOldTaskbarSet = TRUE;
        }
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("MicaEffectOnTitlebar"),
            0,
            NULL,
            &bMicaEffectOnTitlebar,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("HideControlCenterButton"),
            0,
            NULL,
            &bHideControlCenterButton,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("FlyoutMenus"),
            0,
            NULL,
            &bFlyoutMenus,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("CenterMenus"),
            0,
            NULL,
            &bCenterMenus,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("SkinIcons"),
            0,
            NULL,
            &bSkinIcons,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ReplaceNetwork"),
            0,
            NULL,
            &bReplaceNetwork,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ExplorerReadyDelay"),
            0,
            NULL,
            &dwExplorerReadyDelay,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ArchiveMenu"),
            0,
            NULL,
            &bEnableArchivePlugin,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ClockFlyoutOnWinC"),
            0,
            NULL,
            &bClockFlyoutOnWinC,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("DisableImmersiveContextMenu"),
            0,
            NULL,
            &bDisableImmersiveContextMenu,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("HookStartMenu"),
            0,
            NULL,
            &bHookStartMenu,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("PropertiesInWinX"),
            0,
            NULL,
            &bPropertiesInWinX,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("NoPropertiesInContextMenu"),
            0,
            NULL,
            &bNoPropertiesInContextMenu,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("NoMenuAccelerator"),
            0,
            NULL,
            &bNoMenuAccelerator,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("IMEStyle"),
            0,
            NULL,
            &dwIMEStyle,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("UpdatePolicy"),
            0,
            NULL,
            &dwUpdatePolicy,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("IsUpdatePending"),
            0,
            NULL,
            &bShowUpdateToast,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ToolbarSeparators"),
            0,
            NULL,
            &bToolbarSeparators,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("TaskbarAutohideOnDoubleClick"),
            0,
            NULL,
            &bTaskbarAutohideOnDoubleClick,
            &dwSize
        );
        dwTemp = ORB_STYLE_WINDOWS10;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("OrbStyle"),
            0,
            NULL,
            &dwTemp,
            &dwSize
        );
        if (bOldTaskbar && (dwTemp != dwOrbStyle))
        {
            dwOrbStyle = dwTemp;
            dwRefreshUIMask |= REFRESHUI_ORB;
        }
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("EnableSymbolDownload"),
            0,
            NULL,
            &bEnableSymbolDownload,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        dwTemp = 0;
        RegQueryValueExW(
            hKey,
            TEXT("OpenPropertiesAtNextStart"),
            0,
            NULL,
            &dwTemp,
            &dwSize
        );
        if (!IsAppRunningAsAdminMode() && dwTemp)
        {
#ifdef _WIN64
            LaunchPropertiesGUI(hModule);
#endif
        }
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("SnapAssistSettings"),
            0,
            NULL,
            &dwSnapAssistSettings,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("DoNotRedirectDateAndTimeToSettingsApp"),
            0,
            NULL,
            &bDoNotRedirectDateAndTimeToSettingsApp,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("DoNotRedirectNotificationIconsToSettingsApp"),
            0,
            NULL,
            &bDoNotRedirectNotificationIconsToSettingsApp,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("DisableOfficeHotkeys"),
            0,
            NULL,
            &bDisableOfficeHotkeys,
            &dwSize
        );
        dwTemp = FALSE;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("PinnedItemsActAsQuickLaunch"),
            0,
            NULL,
            &dwTemp,
            &dwSize
        );
        if (!bWasPinnedItemsActAsQuickLaunch)
        {
            //if (dwTemp != bPinnedItemsActAsQuickLaunch)
            {
                bPinnedItemsActAsQuickLaunch = dwTemp;
                bWasPinnedItemsActAsQuickLaunch = TRUE;
                //dwRefreshUIMask |= REFRESHUI_TASKBAR;
            }
        }
        dwTemp = FALSE;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("RemoveExtraGapAroundPinnedItems"),
            0,
            NULL,
            &dwTemp,
            &dwSize
        );
        //if (!bWasRemoveExtraGapAroundPinnedItems)
        {
            if (dwTemp != bRemoveExtraGapAroundPinnedItems)
            {
                bRemoveExtraGapAroundPinnedItems = dwTemp;
                bWasRemoveExtraGapAroundPinnedItems = TRUE;
                dwRefreshUIMask |= REFRESHUI_TASKBAR;
            }
        }

#ifdef _WIN64
        AcquireSRWLockShared(&lock_epw);

        DWORD dwOldWeatherTemperatureUnit = dwWeatherTemperatureUnit;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("WeatherTemperatureUnit"),
            0,
            NULL,
            &dwWeatherTemperatureUnit,
            &dwSize
        );
        if (dwWeatherTemperatureUnit != dwOldWeatherTemperatureUnit && epw)
        {
            epw->lpVtbl->SetTemperatureUnit(epw, dwWeatherTemperatureUnit);
            HWND hWnd = NULL;
            if (SUCCEEDED(epw->lpVtbl->GetWindowHandle(epw, &hWnd)) && hWnd)
            {
                SendMessageW(hWnd, EP_WEATHER_WM_FETCH_DATA, 0, 0);
            }
        }

        DWORD dwOldWeatherViewMode = dwWeatherViewMode;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("WeatherViewMode"),
            0,
            NULL,
            &dwWeatherViewMode,
            &dwSize
        );
        if (dwWeatherViewMode != dwOldWeatherViewMode && PeopleButton_LastHWND)
        {
            dwRefreshUIMask |= REFRESHUI_PEOPLE;
        }

        DWORD dwOldUpdateSchedule = dwWeatherUpdateSchedule;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("WeatherContentUpdateMode"),
            0,
            NULL,
            &dwWeatherUpdateSchedule,
            &dwSize
        );
        if (dwWeatherUpdateSchedule != dwOldUpdateSchedule && epw)
        {
            epw->lpVtbl->SetUpdateSchedule(epw, dwWeatherUpdateSchedule * 1000);
        }

        dwSize = MAX_PATH * sizeof(WCHAR);
        if (RegQueryValueExW(
            hKey,
            TEXT("WeatherLocation"),
            0,
            NULL,
            wszWeatherTerm,
            &dwSize
        ))
        {
            wcscpy_s(wszWeatherTerm, MAX_PATH, L"");
        }        
        else
        {
            if (wszWeatherTerm[0] == 0)
            {
                wcscpy_s(wszWeatherTerm, MAX_PATH, L"");
            }
        }
        if (epw)
        {
            epw->lpVtbl->SetTerm(epw, MAX_PATH * sizeof(WCHAR), wszWeatherTerm);
        }

        dwSize = MAX_PATH * sizeof(WCHAR);
        if (RegQueryValueExW(
            hKey,
            TEXT("WeatherLanguage"),
            0,
            NULL,
            wszWeatherLanguage,
            &dwSize
        ))
        {
            BOOL bOk = FALSE;
            ULONG ulNumLanguages = 0;
            LPCWSTR wszLanguagesBuffer = NULL;
            ULONG cchLanguagesBuffer = 0;
            if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &ulNumLanguages, NULL, &cchLanguagesBuffer))
            {
                if (wszLanguagesBuffer = malloc(cchLanguagesBuffer * sizeof(WCHAR)))
                {
                    if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &ulNumLanguages, wszLanguagesBuffer, &cchLanguagesBuffer))
                    {
                        wcscpy_s(wszWeatherLanguage, MAX_PATH, wszLanguagesBuffer);
                        bOk = TRUE;
                    }
                    free(wszLanguagesBuffer);
                }
            }
            if (!bOk)
            {
                wcscpy_s(wszWeatherLanguage, MAX_PATH, L"en-US");
            }
        }
        else
        {
            if (wszWeatherLanguage[0] == 0)
            {
                BOOL bOk = FALSE;
                ULONG ulNumLanguages = 0;
                LPCWSTR wszLanguagesBuffer = NULL;
                ULONG cchLanguagesBuffer = 0;
                if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &ulNumLanguages, NULL, &cchLanguagesBuffer))
                {
                    if (wszLanguagesBuffer = malloc(cchLanguagesBuffer * sizeof(WCHAR)))
                    {
                        if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &ulNumLanguages, wszLanguagesBuffer, &cchLanguagesBuffer))
                        {
                            wcscpy_s(wszWeatherLanguage, MAX_PATH, wszLanguagesBuffer);
                            bOk = TRUE;
                        }
                        free(wszLanguagesBuffer);
                    }
                }
                if (!bOk)
                {
                    wcscpy_s(wszWeatherLanguage, MAX_PATH, L"en-US");
                }
            }
        }
        if (epw)
        {
            epw->lpVtbl->SetLanguage(epw, MAX_PATH * sizeof(WCHAR), wszWeatherLanguage);
        }

        DWORD bOldWeatherFixedSize = bWeatherFixedSize;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("WeatherFixedSize"),
            0,
            NULL,
            &bWeatherFixedSize,
            &dwSize
        );
        if (bWeatherFixedSize != bOldWeatherFixedSize && epw)
        {
            dwRefreshUIMask |= REFRESHUI_PEOPLE;
        }

        DWORD dwOldWeatherTheme = dwWeatherTheme;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("WeatherTheme"),
            0,
            NULL,
            &dwWeatherTheme,
            &dwSize
        );
        if (dwWeatherTheme != dwOldWeatherTheme && PeopleButton_LastHWND)
        {
            if (epw)
            {
                epw->lpVtbl->SetDarkMode(epw, (LONG64)dwWeatherTheme, TRUE);
            }
        }

        DWORD dwOldWeatherGeolocationMode = dwWeatherGeolocationMode;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("WeatherLocationType"),
            0,
            NULL,
            &dwWeatherGeolocationMode,
            &dwSize
        );
        if (dwWeatherGeolocationMode != dwOldWeatherGeolocationMode && PeopleButton_LastHWND)
        {
            if (epw)
            {
                epw->lpVtbl->SetGeolocationMode(epw, (LONG64)dwWeatherGeolocationMode);
            }
        }

        DWORD dwOldWeatherWindowCornerPreference = dwWeatherWindowCornerPreference;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("WeatherWindowCornerPreference"),
            0,
            NULL,
            &dwWeatherWindowCornerPreference,
            &dwSize
        );
        if (dwWeatherWindowCornerPreference != dwOldWeatherWindowCornerPreference && PeopleButton_LastHWND)
        {
            if (epw)
            {
                epw->lpVtbl->SetWindowCornerPreference(epw, (LONG64)dwWeatherWindowCornerPreference);
            }
        }

        ReleaseSRWLockShared(&lock_epw);
#endif

        dwTemp = TASKBARGLOMLEVEL_DEFAULT;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("TaskbarGlomLevel"),
            0,
            NULL,
            &dwTemp,
            &dwSize
        );
        if (bOldTaskbar && (dwTemp != dwTaskbarGlomLevel))
        {
            dwRefreshUIMask = REFRESHUI_GLOM;
        }
        dwTaskbarGlomLevel = dwTemp;
        dwTemp = MMTASKBARGLOMLEVEL_DEFAULT;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("MMTaskbarGlomLevel"),
            0,
            NULL,
            &dwTemp,
            &dwSize
        );
        if (bOldTaskbar && (dwTemp != dwMMTaskbarGlomLevel))
        {
            dwRefreshUIMask = REFRESHUI_GLOM;
        }
        dwMMTaskbarGlomLevel = dwTemp;
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("AltTabSettings"),
            0,
            NULL,
            &dwAltTabSettings,
            &dwSize
        );
        RegCloseKey(hKey);
    }


    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("MonitorOverride"),
            0,
            NULL,
            &bMonitorOverride,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("OpenAtLogon"),
            0,
            NULL,
            &bOpenAtLogon,
            &dwSize
        );
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH) L"\\sws",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MultitaskingView\\AltTabViewHost",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Search",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\People",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\TabletTip\\1.7",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    if (bIsRefreshAllowed && dwRefreshUIMask)
    {
        if (dwRefreshUIMask & REFRESHUI_GLOM)
        {
            Explorer_RefreshUI(0);
        }
        if ((dwRefreshUIMask & REFRESHUI_ORB) || (dwRefreshUIMask & REFRESHUI_PEOPLE))
        {
            SendNotifyMessageW(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)L"TraySettings");
            if (dwRefreshUIMask & REFRESHUI_ORB)
            {
                InvalidateRect(FindWindowW(L"ExplorerPatcher_GUI_" _T(EP_CLSID), NULL), NULL, FALSE);
            }
            if (dwRefreshUIMask & REFRESHUI_PEOPLE)
            {
#ifdef _WIN64
                InvalidateRect(PeopleButton_LastHWND, NULL, TRUE);
#endif
            }
        }
        if (dwRefreshUIMask & REFRESHUI_TASKBAR)
        {
            // this is mostly a hack...
            DWORD dwGlomLevel = 2, dwSize = sizeof(DWORD), dwNewGlomLevel;
            RegGetValueW(HKEY_CURRENT_USER, TEXT(REGPATH), L"TaskbarGlomLevel", RRF_RT_DWORD, NULL, &dwGlomLevel, &dwSize);
            Sleep(100);
            dwNewGlomLevel = 0;
            RegSetKeyValueW(HKEY_CURRENT_USER, TEXT(REGPATH), L"TaskbarGlomLevel", REG_DWORD, &dwNewGlomLevel, sizeof(DWORD));
            Explorer_RefreshUI(0);
            Sleep(100);
            dwNewGlomLevel = 2;
            RegSetKeyValueW(HKEY_CURRENT_USER, TEXT(REGPATH), L"TaskbarGlomLevel", REG_DWORD, &dwNewGlomLevel, sizeof(DWORD));
            Explorer_RefreshUI(0);
            Sleep(100);
            RegSetKeyValueW(HKEY_CURRENT_USER, TEXT(REGPATH), L"TaskbarGlomLevel", REG_DWORD, &dwGlomLevel, sizeof(DWORD));
            Explorer_RefreshUI(0);
        }
    }
}

void Explorer_RefreshClockHelper(HWND hClockButton)
{
    INT64* ClockButtonInstance = (BYTE*)(GetWindowLongPtrW(hClockButton, 0)); // -> ClockButton
    // we call v_Initialize because all it does is to query the
    // registry and update the internal state to display seconds or not
    // to get the offset, simply inspect the vtable of ClockButton
    ((void(*)(void*))(*(INT64*)((*(INT64*)ClockButtonInstance) + 6 * sizeof(uintptr_t))))(ClockButtonInstance); // v_Initialize
    // we need to refresh the button; for the text to actually change, we need to set this:
    // inspect ClockButton::v_OnTimer
    *((BYTE*)ClockButtonInstance + 547) = 1;
    // then, we simply invalidate the area
    InvalidateRect(hClockButton, NULL, TRUE);
}

void Explorer_RefreshClock(int unused)
{
    HWND hShellTray_Wnd = FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL);
    if (hShellTray_Wnd)
    {
        HWND hTrayNotifyWnd = FindWindowExW(hShellTray_Wnd, NULL, L"TrayNotifyWnd", NULL);
        if (hTrayNotifyWnd)
        {
            HWND hClockButton = FindWindowExW(hTrayNotifyWnd, NULL, L"TrayClockWClass", NULL);
            if (hClockButton)
            {
                Explorer_RefreshClockHelper(hClockButton);
            }
        }
    }

    HWND hWnd = NULL;
    do
    {
        hWnd = FindWindowExW(
            NULL,
            hWnd,
            L"Shell_SecondaryTrayWnd",
            NULL
        );
        if (hWnd)
        {
            HWND hClockButton = FindWindowExW(hWnd, NULL, L"ClockButton", NULL);
            if (hClockButton)
            {
                Explorer_RefreshClockHelper(hClockButton);
            }
        }
    } while (hWnd);
}

void WINAPI Explorer_RefreshUI(int unused)
{
    SendNotifyMessageW(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)L"TraySettings");
    Explorer_RefreshClock(0);
}

void Explorer_TogglePeopleButton(int unused)
{
    HWND hShellTray_Wnd = FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL);
    if (hShellTray_Wnd)
    {
        INT64* CTrayInstance = (BYTE*)(GetWindowLongPtrW(hShellTray_Wnd, 0)); // -> CTray
        const unsigned int TRAYUI_OFFSET_IN_CTRAY = 110;
        INT64* TrayUIInstance = *((INT64*)CTrayInstance + TRAYUI_OFFSET_IN_CTRAY);

        ((void(*)(void*))(*(INT64*)((*(INT64*)TrayUIInstance) + 57 * sizeof(uintptr_t))))(TrayUIInstance);
    }
}

void Explorer_ToggleTouchpad(int unused)
{
    HWND hShellTray_Wnd = FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL);
    if (hShellTray_Wnd)
    {
        INT64* CTrayInstance = (BYTE*)(GetWindowLongPtrW(hShellTray_Wnd, 0)); // -> CTray
        const unsigned int TRAYUI_OFFSET_IN_CTRAY = 110;
        INT64* TrayUIInstance = *((INT64*)CTrayInstance + TRAYUI_OFFSET_IN_CTRAY);

        ((void(*)(void*))(*(INT64*)((*(INT64*)TrayUIInstance) + 60 * sizeof(uintptr_t))))(TrayUIInstance);
    }
}
#pragma endregion


#pragma region "Fix taskbar for classic theme and set Explorer window hooks"
HWND(*CreateWindowExWFunc)(
    DWORD     dwExStyle,
    LPCWSTR   lpClassName,
    LPCWSTR   lpWindowName,
    DWORD     dwStyle,
    int       X,
    int       Y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
    );
HWND CreateWindowExWHook(
    DWORD     dwExStyle,
    LPCWSTR   lpClassName,
    LPCWSTR   lpWindowName,
    DWORD     dwStyle,
    int       X,
    int       Y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
)
{
    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"TrayNotifyWnd"))
    {
        dwExStyle |= WS_EX_STATICEDGE;
    }
    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"NotifyIconOverflowWindow"))
    {
        dwExStyle |= WS_EX_STATICEDGE;
    }
    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && (!wcscmp(lpClassName, L"SysListView32") || !wcscmp(lpClassName, L"SysTreeView32"))) // !wcscmp(lpClassName, L"FolderView")
    {
        wchar_t wszClassName[200];
        ZeroMemory(wszClassName, 200);
        GetClassNameW(GetAncestor(hWndParent, GA_ROOT), wszClassName, 200);
        if (!wcscmp(wszClassName, L"CabinetWClass"))
        {
            dwExStyle |= WS_EX_CLIENTEDGE;
        }
    }
    if (bIsExplorerProcess && bToolbarSeparators && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"ReBarWindow32"))
    {
        wchar_t wszClassName[200];
        ZeroMemory(wszClassName, 200);
        GetClassNameW(hWndParent, wszClassName, 200);
        if (!wcscmp(wszClassName, L"Shell_TrayWnd"))
        {
            dwStyle |= RBS_BANDBORDERS;
        }
    }
    HWND hWnd = CreateWindowExWFunc(
        dwExStyle,
        lpClassName,
        lpWindowName,
        dwStyle,
        X,
        Y,
        nWidth,
        nHeight,
        hWndParent,
        hMenu,
        hInstance,
        lpParam
    );
#ifdef _WIN64
    if (bIsExplorerProcess && (*((WORD*)&(lpClassName)+1)) && (!wcscmp(lpClassName, L"TrayClockWClass") || !wcscmp(lpClassName, L"ClockButton")))
    {
        SetWindowSubclass(hWnd, ClockButtonSubclassProc, ClockButtonSubclassProc, 0);
    }
    else if (bIsExplorerProcess && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"TrayShowDesktopButtonWClass"))
    {
        SetWindowSubclass(hWnd, ShowDesktopSubclassProc, ShowDesktopSubclassProc, 0);
    }
    else if (bIsExplorerProcess && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"Shell_TrayWnd"))
    {
        SetWindowSubclass(hWnd, Shell_TrayWndSubclassProc, Shell_TrayWndSubclassProc, TRUE);
        Shell_TrayWndMouseHook = SetWindowsHookExW(WH_MOUSE, Shell_TrayWndMouseProc, NULL, GetCurrentThreadId());
    }
    else if (bIsExplorerProcess && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"Shell_SecondaryTrayWnd"))
    {
        SetWindowSubclass(hWnd, Shell_TrayWndSubclassProc, Shell_TrayWndSubclassProc, FALSE);
    }
#endif
    /*
    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && (!wcscmp(lpClassName, L"FolderView")))
    {
        wchar_t wszClassName[200];
        GetClassNameW(GetAncestor(hWndParent, GA_ROOT), wszClassName, 200);
        if (!wcscmp(wszClassName, L"CabinetWClass"))
        {
            SendMessageW(hWnd, 0x108, 0, 0);
        }
    }
    */
    //SetWindowTheme(hWnd, L" ", L" ");
    return hWnd;
}

LONG_PTR(*SetWindowLongPtrWFunc)(
    HWND     hWnd,
    int      nIndex,
    LONG_PTR dwNewLong
    );
LONG_PTR SetWindowLongPtrWHook(
    HWND     hWnd,
    int      nIndex,
    LONG_PTR dwNewLong
)
{
    WCHAR lpClassName[200];
    ZeroMemory(lpClassName, 200);
    GetClassNameW(hWnd, lpClassName, 200);
    HWND hWndParent = GetParent(hWnd);

    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"TrayNotifyWnd"))
    {
        if (nIndex == GWL_EXSTYLE)
        {
            dwNewLong |= WS_EX_STATICEDGE;
        }
    }
    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"NotifyIconOverflowWindow"))
    {
        if (nIndex == GWL_EXSTYLE)
        {
            dwNewLong |= WS_EX_STATICEDGE;
        }
    }
    if (bClassicThemeMitigations && (*((WORD*)&(lpClassName)+1)) && (!wcscmp(lpClassName, L"SysListView32") || !wcscmp(lpClassName, L"SysTreeView32"))) // !wcscmp(lpClassName, L"FolderView")
    {
        wchar_t wszClassName[200];
        ZeroMemory(wszClassName, 200);
        GetClassNameW(GetAncestor(hWndParent, GA_ROOT), wszClassName, 200);
        if (!wcscmp(wszClassName, L"CabinetWClass"))
        {
            if (nIndex == GWL_EXSTYLE)
            {
                dwNewLong |= WS_EX_CLIENTEDGE;
            }
        }
    }
    if (bIsExplorerProcess && bToolbarSeparators && (*((WORD*)&(lpClassName)+1)) && !wcscmp(lpClassName, L"ReBarWindow32"))
    {
        wchar_t wszClassName[200];
        ZeroMemory(wszClassName, 200);
        GetClassNameW(hWndParent, wszClassName, 200);
        if (!wcscmp(wszClassName, L"Shell_TrayWnd"))
        {
            if (nIndex == GWL_STYLE)
            {
                dwNewLong |= RBS_BANDBORDERS;
            }
        }
    }
    return SetWindowLongPtrWFunc(hWnd, nIndex, dwNewLong);
}

#ifdef _WIN64
HRESULT (*explorer_SetWindowThemeFunc)(
    HWND    hwnd,
    LPCWSTR pszSubAppName,
    LPCWSTR pszSubIdList
);
HRESULT explorer_SetWindowThemeHook(
    HWND    hwnd,
    LPCWSTR pszSubAppName,
    LPCWSTR pszSubIdList
)
{
    if (bClassicThemeMitigations)
    {
        printf("SetWindowTheme\n");
        return explorer_SetWindowThemeFunc(hwnd, L" ", L" ");
    }
    return explorer_SetWindowThemeFunc(hwnd, pszSubAppName, pszSubIdList);
}

INT64 explorer_SetWindowCompositionAttribute(HWND hWnd, WINCOMPATTRDATA* data)
{
    if (bClassicThemeMitigations)
    {
        return TRUE;
    }
    return SetWindowCompositionAttribute(hWnd, data);
}

HDPA hOrbCollection = NULL;
HRESULT explorer_DrawThemeBackground(
    HTHEME  hTheme,
    HDC     hdc,
    int     iPartId,
    int     iStateId,
    LPCRECT pRect,
    LPCRECT pClipRect
)
{
    if (dwOrbStyle && hOrbCollection)
    {
        for (unsigned int i = 0; i < DPA_GetPtrCount(hOrbCollection); ++i)
        {
            OrbInfo* oi = DPA_FastGetPtr(hOrbCollection, i);
            if (oi->hTheme == hTheme)
            {
                BITMAPINFO bi;
                ZeroMemory(&bi, sizeof(BITMAPINFO));
                bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bi.bmiHeader.biWidth = 1;
                bi.bmiHeader.biHeight = 1;
                bi.bmiHeader.biPlanes = 1;
                bi.bmiHeader.biBitCount = 32;
                bi.bmiHeader.biCompression = BI_RGB;
                RGBQUAD transparent = { 0, 0, 0, 0 };
                RGBQUAD color = { 0xFF, 0xFF, 0xFF, 0xFF };

                if (dwOrbStyle == ORB_STYLE_WINDOWS11)
                {
                    UINT separator = oi->dpi / 96;
                    //printf(">>> SEPARATOR %p %d %d\n", oi->hTheme, oi->dpi, separator);

                    // Background
                    StretchDIBits(hdc,
                        pRect->left + (separator % 2),
                        pRect->top + (separator % 2),
                        pRect->right - pRect->left - (separator % 2),
                        pRect->bottom - pRect->top - (separator % 2),
                        0, 0, 1, 1, &color, &bi,
                        DIB_RGB_COLORS, SRCCOPY);
                    // Middle vertical line
                    StretchDIBits(hdc,
                        pRect->left + ((pRect->right - pRect->left) / 2) - (separator / 2),
                        pRect->top,
                        separator,
                        pRect->bottom - pRect->top,
                        0, 0, 1, 1, &transparent, &bi,
                        DIB_RGB_COLORS, SRCCOPY);
                    // Middle horizontal line
                    StretchDIBits(hdc,
                        pRect->left,
                        pRect->top + ((pRect->bottom - pRect->top) / 2) - (separator / 2),
                        pRect->right - pRect->left,
                        separator,
                        0, 0, 1, 1, &transparent, &bi,
                        DIB_RGB_COLORS, SRCCOPY);
                }
                else if (dwOrbStyle == ORB_STYLE_TRANSPARENT)
                {
                    StretchDIBits(hdc,
                        pRect->left,
                        pRect->top,
                        pRect->right - pRect->left,
                        pRect->bottom - pRect->top,
                        0, 0, 1, 1, &transparent, &bi,
                        DIB_RGB_COLORS, SRCCOPY);
                }
                return S_OK;
            }
        }
    }
    if (bClassicThemeMitigations)
    {
        if (iPartId == 4 && iStateId == 1)
        {
            COLORREF bc = GetBkColor(hdc);
            COLORREF fc = GetTextColor(hdc);
            int mode = SetBkMode(hdc, TRANSPARENT);

            SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

            NONCLIENTMETRICSW ncm;
            ncm.cbSize = sizeof(NONCLIENTMETRICSW);
            SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0);

            HFONT hFont = CreateFontIndirectW(&(ncm.lfCaptionFont));

            UINT dpiX, dpiY;
            HRESULT hr = GetDpiForMonitor(
                MonitorFromWindow(WindowFromDC(hdc), MONITOR_DEFAULTTOPRIMARY),
                MDT_DEFAULT,
                &dpiX,
                &dpiY
            );
            double dx = dpiX / 96.0, dy = dpiY / 96.0;

            HGDIOBJ hOldFont = SelectObject(hdc, hFont);
            DWORD dwTextFlags = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
            RECT rc = *pRect;
            rc.bottom -= 7 * dy;
            DrawTextW(
                hdc,
                L"\u2026",
                -1, 
                &rc,
                dwTextFlags
            );
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            SetBkColor(hdc, bc);
            SetTextColor(hdc, fc);
            SetBkMode(hdc, mode);
        }
        return S_OK;
    }
    return DrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

HRESULT explorer_CloseThemeData(HTHEME hTheme)
{
    HRESULT hr = CloseThemeData(hTheme);
    if (SUCCEEDED(hr) && hOrbCollection)
    {
        for (unsigned int i = 0; i < DPA_GetPtrCount(hOrbCollection); ++i)
        {
            OrbInfo* oi = DPA_FastGetPtr(hOrbCollection, i);
            if (oi->hTheme == hTheme)
            {
                //printf(">>> DELETE DPA %p %d\n", oi->hTheme, oi->dpi);
                DPA_DeletePtr(hOrbCollection, i);
                free(oi);
                break;
            }
        }
    }
    return hr;
}

HTHEME explorer_OpenThemeDataForDpi(
    HWND    hwnd,
    LPCWSTR pszClassList,
    UINT    dpi
)
{
    if ((*((WORD*)&(pszClassList)+1)) && !wcscmp(pszClassList, L"TaskbarPearl"))
    {
        if (!hOrbCollection)
        {
            hOrbCollection = DPA_Create(MAX_NUM_MONITORS);
        }

        HTHEME hTheme = OpenThemeDataForDpi(hwnd, pszClassList, dpi);
        if (hTheme && hOrbCollection)
        {
            OrbInfo* oi = malloc(sizeof(OrbInfo));
            if (oi)
            {
                oi->hTheme = hTheme;
                oi->dpi = dpi;
                //printf(">>> APPEND DPA %p %d\n", oi->hTheme, oi->dpi);
                DPA_AppendPtr(hOrbCollection, oi);
            }
        }
        return hTheme;
    }

    // task list - Taskband2 from CTaskListWnd::_HandleThemeChanged
    if (bClassicThemeMitigations && (*((WORD*)&(pszClassList)+1)) && !wcscmp(pszClassList, L"Taskband2"))
    {
        return 0xDeadBeef;
    }
    // system tray notification area more icons
    else if (bClassicThemeMitigations && (*((WORD*)&(pszClassList)+1)) && !wcscmp(pszClassList, L"TrayNotifyFlyout"))
    {
        return 0xABadBabe;
    }
    /*else if (bClassicThemeMitigations && (*((WORD*)&(pszClassList)+1)) && wcsstr(pszClassList, L"::Taskband2"))
    {
        wprintf(L"%s\n", pszClassList);
        return 0xB16B00B5;
    }*/
    return OpenThemeDataForDpi(hwnd, pszClassList, dpi);
}

HRESULT explorer_GetThemeMetric(
    HTHEME hTheme,
    HDC    hdc,
    int    iPartId,
    int    iStateId,
    int    iPropId,
    int* piVal
)
{
    if (!bClassicThemeMitigations || (hTheme != 0xABadBabe))
    {
        return GetThemeMetric(
            hTheme,
            hdc,
            iPartId,
            iStateId,
            iPropId,
            piVal
        );
    }
    const int TMT_WIDTH = 2416;
    const int TMT_HEIGHT = 2417;
    if (hTheme == 0xABadBabe && iPropId == TMT_WIDTH && iPartId == 3 && iStateId == 0)
    {
        *piVal = GetSystemMetrics(SM_CXICON);
    }
    else if (hTheme == 0xABadBabe && iPropId == TMT_HEIGHT && iPartId == 3 && iStateId == 0)
    {
        *piVal = GetSystemMetrics(SM_CYICON);
    }
    return S_OK;
}

HRESULT explorer_GetThemeMargins(
    HTHEME  hTheme,
    HDC     hdc,
    int     iPartId,
    int     iStateId,
    int     iPropId,
    LPCRECT prc,
    MARGINS* pMargins
)
{
    if (!bClassicThemeMitigations || (hTheme != 0xDeadBeef && hTheme != 0xABadBabe))
    {
        HRESULT hr = GetThemeMargins(
            hTheme,
            hdc,
            iPartId,
            iStateId,
            iPropId,
            prc,
            pMargins
        );
        return hr;
    }
    const int TMT_SIZINGMARGINS = 3601;
    const int TMT_CONTENTMARGINS = 3602;
    HRESULT hr = S_OK;
    if (hTheme)
    {
        hr = GetThemeMargins(
            hTheme,
            hdc,
            iPartId,
            iStateId,
            iPropId,
            prc,
            pMargins
        );
    }
    /*if (hTheme == 0xB16B00B5)
    {
        printf(
            "GetThemeMargins %d %d %d - %d %d %d %d\n", 
            iPartId, 
            iStateId, 
            iPropId, 
            pMargins->cxLeftWidth, 
            pMargins->cyTopHeight, 
            pMargins->cxRightWidth, 
            pMargins->cyBottomHeight
        );
    }*/
    if (hTheme == 0xDeadBeef && iPropId == TMT_CONTENTMARGINS && iPartId == 5 && iStateId == 1)
    {
        // task list button measurements
        pMargins->cxLeftWidth = 4;
        pMargins->cyTopHeight = 3;
        pMargins->cxRightWidth = 4;
        pMargins->cyBottomHeight = 3;
    }
    else if (hTheme == 0xDeadBeef && iPropId == TMT_CONTENTMARGINS && iPartId == 1 && iStateId == 0)
    {
        // task list measurements
        pMargins->cxLeftWidth = 0;
        pMargins->cyTopHeight = 0;
        pMargins->cxRightWidth = 4;
        pMargins->cyBottomHeight = 0;
    }
    else if (hTheme == 0xDeadBeef && iPropId == TMT_SIZINGMARGINS && iPartId == 5 && iStateId == 1)
    {
        pMargins->cxLeftWidth = 10;
        pMargins->cyTopHeight = 10;
        pMargins->cxRightWidth = 10;
        pMargins->cyBottomHeight = 10;
    }
    else if (hTheme = 0xABadBabe && iPropId == TMT_CONTENTMARGINS && iPartId == 3 && iStateId == 0)
    {
        pMargins->cxLeftWidth = 6;// GetSystemMetrics(SM_CXICONSPACING);
        pMargins->cyTopHeight = 6;// GetSystemMetrics(SM_CYICONSPACING);
        pMargins->cxRightWidth = 6;//GetSystemMetrics(SM_CXICONSPACING);
        pMargins->cyBottomHeight = 6;// GetSystemMetrics(SM_CYICONSPACING);
    }
    HWND hShell_TrayWnd = FindWindowEx(NULL, NULL, L"Shell_TrayWnd", NULL);
    if (hShell_TrayWnd)
    {
        LONG dwStyle = 0;
        dwStyle = GetWindowLongW(hShell_TrayWnd, GWL_STYLE);
        dwStyle |= WS_DLGFRAME;
        SetWindowLongW(hShell_TrayWnd, GWL_STYLE, dwStyle);
        dwStyle &= ~WS_DLGFRAME;
        SetWindowLongW(hShell_TrayWnd, GWL_STYLE, dwStyle);
    }
    HWND hWnd = NULL;
    do
    {
        hWnd = FindWindowEx(
            NULL,
            hWnd,
            L"Shell_SecondaryTrayWnd",
            NULL
        );
        if (hWnd)
        {
            LONG dwStyle = 0;
            dwStyle = GetWindowLongW(hWnd, GWL_STYLE);
            dwStyle |= WS_DLGFRAME;
            SetWindowLongW(hWnd, GWL_STYLE, dwStyle);
            dwStyle &= ~WS_DLGFRAME;
            SetWindowLongW(hWnd, GWL_STYLE, dwStyle);
        }
    } while (hWnd);
    return S_OK;
}

HRESULT explorer_DrawThemeTextEx(
    HTHEME        hTheme,
    HDC           hdc,
    int           iPartId,
    int           iStateId,
    LPCWSTR       pszText,
    int           cchText,
    DWORD         dwTextFlags,
    LPRECT        pRect,
    const DTTOPTS* pOptions
)
{
    if (!bClassicThemeMitigations)
    {
        return DrawThemeTextEx(
            hTheme,
            hdc,
            iPartId,
            iStateId,
            pszText,
            cchText,
            dwTextFlags,
            pRect,
            pOptions
        );
    }

    COLORREF bc = GetBkColor(hdc);
    COLORREF fc = GetTextColor(hdc);
    int mode = SetBkMode(hdc, TRANSPARENT);
    
    wchar_t text[200];
    GetWindowTextW(GetForegroundWindow(), text, 200);

    BOOL bIsActiveUnhovered = (iPartId == 5 && iStateId == 5);
    BOOL bIsInactiveUnhovered = (iPartId == 5 && iStateId == 1);
    BOOL bIsInactiveHovered = (iPartId == 5 && iStateId == 2);
    BOOL bIsActiveHovered = bIsInactiveHovered && !wcscmp(text, pszText);

    SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

    NONCLIENTMETRICSW ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICSW);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0);

    HFONT hFont = NULL;
    if (bIsActiveUnhovered)
    {
        hFont = CreateFontIndirectW(&(ncm.lfCaptionFont));
    }
    else if (bIsInactiveUnhovered)
    {
        hFont = CreateFontIndirectW(&(ncm.lfMenuFont));
    }
    else if (bIsActiveHovered)
    {
        hFont = CreateFontIndirectW(&(ncm.lfCaptionFont));
    }
    else if (bIsInactiveHovered)
    {
        hFont = CreateFontIndirectW(&(ncm.lfMenuFont));
    }
    else
    {
        hFont = CreateFontIndirectW(&(ncm.lfMenuFont));
        //wprintf(L"DrawThemeTextEx %d %d %s\n", iPartId, iStateId, pszText);
    }

    if (iPartId == 5 && iStateId == 0) // clock
    {
        pRect->top += 2;
    }

    HGDIOBJ hOldFont = SelectObject(hdc, hFont);
    DrawTextW(
        hdc,
        pszText,
        cchText, 
        pRect,
        dwTextFlags
    );
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    SetBkColor(hdc, bc);
    SetTextColor(hdc, fc);
    SetBkMode(hdc, mode);
    return S_OK;
}
#endif
#pragma endregion


#pragma region "Change links"
int ExplorerFrame_CompareStringOrdinal(const WCHAR* a1, int a2, const WCHAR* a3, int a4, BOOL bIgnoreCase)
{
    void* pRedirects[10] =
    {
        L"::{BB06C0E4-D293-4F75-8A90-CB05B6477EEE}", // System                     (default: redirected to Settings app)
        L"::{7B81BE6A-CE2B-4676-A29E-EB907A5126C5}", // Programs and Features      (default: not redirected)
        NULL,
        // The following are unused but available for the future
        L"::{D450A8A1-9568-45C7-9C0E-B4F9FB4537BD}", // Installed Updates          (default: not redirected)
        L"::{17CD9488-1228-4B2F-88CE-4298E93E0966}", // Default Programs           (default: not redirected)
        L"::{8E908FC9-BECC-40F6-915B-F4CA0E70D03D}", // Network and Sharing Center (default: not redirected)
        L"::{7007ACC7-3202-11D1-AAD2-00805FC1270E}", // Network Connections        (default: not redirected)
        L"Advanced",                                 // Network and Sharing Center -> Change advanced sharing options (default: not redirected)
        L"::{A8A91A66-3A7D-4424-8D24-04E180695C7A}", // Devices and Printers       (default: not redirected)
        NULL
    };
    int ret = CompareStringOrdinal(a1, a2, a3, a4, bIgnoreCase);
    if ((!bDoNotRedirectSystemToSettingsApp && !bDoNotRedirectProgramsAndFeaturesToSettingsApp) || ret != CSTR_EQUAL)
    {
        return ret;
    }

    int i = 0;
    while (CompareStringOrdinal(a3, -1, pRedirects[i], -1, FALSE) != CSTR_EQUAL)
    {
        i++;
        if (pRedirects[i] == NULL)
        {
            return ret;
        }
    }

    return CSTR_GREATER_THAN;
}

#ifdef _WIN64
DEFINE_GUID(IID_EnumExplorerCommand,
    0xA88826F8,
    0x186F, 0x4987, 0xAA, 0xDE,
    0xEA, 0x0C, 0xEF, 0x8F, 0xBF, 0xE8
);

typedef interface EnumExplorerCommand EnumExplorerCommand;

typedef struct EnumExplorerCommandVtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        EnumExplorerCommand* This,
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        EnumExplorerCommand* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        EnumExplorerCommand* This);

    HRESULT(STDMETHODCALLTYPE* Next)(
        EnumExplorerCommand* This,
        unsigned int a2,
        void** a3,
        void* a4);

    END_INTERFACE
} EnumExplorerCommandVtbl;

interface EnumExplorerCommand
{
    CONST_VTBL struct EnumExplorerCommandVtbl* lpVtbl;
};

typedef interface UICommand UICommand;

typedef struct UICommandVtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        UICommand* This,
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        UICommand* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        UICommand* This);

    HRESULT(STDMETHODCALLTYPE* GetTitle)(
        UICommand* This);

    HRESULT(STDMETHODCALLTYPE* GetIcon)(
        UICommand* This);

    HRESULT(STDMETHODCALLTYPE* GetTooltip)(
        UICommand* This);

    HRESULT(STDMETHODCALLTYPE* GetCanonicalName)(
        UICommand* This,
        GUID* guid);

    HRESULT(STDMETHODCALLTYPE* GetState)(
        UICommand* This);

    HRESULT(STDMETHODCALLTYPE* Invoke)(
        UICommand* This,
        void* a2,
        void* a3);

    HRESULT(STDMETHODCALLTYPE* GetFlags)(
        UICommand* This);

    HRESULT(STDMETHODCALLTYPE* EnumSubCommands)(
        UICommand* This);

    END_INTERFACE
} UICommandVtbl;

interface UICommand
{
    CONST_VTBL struct UICommandVtbl* lpVtbl;
};

DEFINE_GUID(GUID_UICommand_System,
    0x4C202CF0,
    0xC4DC, 0x4251, 0xA3, 0x71,
    0xB6, 0x22, 0xB4, 0x3D, 0x59, 0x2B
);
DEFINE_GUID(GUID_UICommand_ProgramsAndFeatures,
    0xA2E6D9CC,
    0xF866, 0x40B6, 0xA4, 0xB2,
    0xEE, 0x9E, 0x10, 0x04, 0xBD, 0xFC
);
HRESULT(*shell32_UICommand_InvokeFunc)(UICommand*, void*, void*);
HRESULT shell32_UICommand_InvokeHook(UICommand* _this, void* a2, void* a3)
{
    // Guid = {A2E6D9CC-F866-40B6-A4B2-EE9E1004BDFC} Programs and Features
    // Guid = {4C202CF0-C4DC-4251-A371-B622B43D592B} System
    GUID guid;
    ZeroMemory(&guid, sizeof(GUID));
    _this->lpVtbl->GetCanonicalName(_this, &guid);
    BOOL bIsSystem = bDoNotRedirectSystemToSettingsApp && IsEqualGUID(&guid, &GUID_UICommand_System);
    BOOL bIsProgramsAndFeatures = bDoNotRedirectProgramsAndFeaturesToSettingsApp && IsEqualGUID(&guid, &GUID_UICommand_ProgramsAndFeatures);
    if (bIsSystem || bIsProgramsAndFeatures)
    {
        IOpenControlPanel* pOpenControlPanel = NULL;
        CoCreateInstance(
            &CLSID_OpenControlPanel,
            NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
            &IID_OpenControlPanel,
            &pOpenControlPanel
        );
        if (pOpenControlPanel)
        {
            WCHAR* pszWhat = L"";
            if (bIsSystem)
            {
                pszWhat = L"Microsoft.System";
            }
            else if (bIsProgramsAndFeatures)
            {
                pszWhat = L"Microsoft.ProgramsAndFeatures";
            }
            pOpenControlPanel->lpVtbl->Open(pOpenControlPanel, pszWhat, NULL, NULL);
            pOpenControlPanel->lpVtbl->Release(pOpenControlPanel);
            return S_OK;
        }
    }
    return shell32_UICommand_InvokeFunc(_this, a2, a3);
}

BOOL explorer_ShellExecuteExW(SHELLEXECUTEINFOW* pExecInfo)
{
    if (bDoNotRedirectSystemToSettingsApp && pExecInfo && pExecInfo->lpFile && !wcscmp(pExecInfo->lpFile, L"ms-settings:about"))
    {
        IOpenControlPanel* pOpenControlPanel = NULL;
        CoCreateInstance(
            &CLSID_OpenControlPanel,
            NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
            &IID_OpenControlPanel,
            &pOpenControlPanel
        );
        if (pOpenControlPanel)
        {
            pOpenControlPanel->lpVtbl->Open(pOpenControlPanel, L"Microsoft.System", NULL, NULL);
            pOpenControlPanel->lpVtbl->Release(pOpenControlPanel);
            return 1;
        }
    }
    return ShellExecuteExW(pExecInfo);
}

HINSTANCE explorer_ShellExecuteW(
    HWND    hwnd,
    LPCWSTR lpOperation,
    LPCWSTR lpFile,
    LPCWSTR lpParameters,
    LPCWSTR lpDirectory,
    INT     nShowCmd
)
{
    if (bDoNotRedirectNotificationIconsToSettingsApp && !wcscmp(lpFile, L"ms-settings:notifications"))
    {
        return ShellExecuteW(
            hwnd, lpOperation,
            L"shell:::{05d7b0f4-2121-4eff-bf6b-ed3f69b894d9}",
            lpParameters, lpDirectory, nShowCmd
        );
    }
    else if (bDoNotRedirectDateAndTimeToSettingsApp && !wcscmp(lpFile, L"ms-settings:dateandtime"))
    {
        return ShellExecuteW(
            hwnd, lpOperation,
            L"shell:::{E2E7934B-DCE5-43C4-9576-7FE4F75E7480}",
            lpParameters, lpDirectory, nShowCmd
        );
    }
    /*else if (!wcscmp(lpFile, L"ms-settings:taskbar"))
    {
        LaunchPropertiesGUI(hModule);
        return 0;
    }*/
    return ShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
}
#endif
#pragma endregion


#pragma region "Change language UI style"
#ifdef _WIN64
DEFINE_GUID(CLSID_InputSwitchControl,
    0xB9BC2A50,
    0x43C3, 0x41AA, 0xa0, 0x86,
    0x5D, 0xB1, 0x4e, 0x18, 0x4b, 0xae
);

DEFINE_GUID(IID_InputSwitchControl,
    0xB9BC2A50,
    0x43C3, 0x41AA, 0xa0, 0x82,
    0x5D, 0xB1, 0x4e, 0x18, 0x4b, 0xae
);

#define LANGUAGEUI_STYLE_DESKTOP 0       // Windows 11 style
#define LANGUAGEUI_STYLE_TOUCHKEYBOARD 1 // Windows 10 style
#define LANGUAGEUI_STYLE_LOGONUI 2
#define LANGUAGEUI_STYLE_UAC 3
#define LANGUAGEUI_STYLE_SETTINGSPANE 4
#define LANGUAGEUI_STYLE_OOBE 5
#define LANGUAGEUI_STYLE_OTHER 100

char mov_edx_val[6] = { 0xBA, 0x00, 0x00, 0x00, 0x00, 0xC3 };
char* ep_pf = NULL;

typedef interface IInputSwitchControl IInputSwitchControl;

typedef struct IInputSwitchControlVtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        IInputSwitchControl* This,
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        IInputSwitchControl* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        IInputSwitchControl* This);

    HRESULT(STDMETHODCALLTYPE* Init)(
        IInputSwitchControl* This,
        /* [in] */ unsigned int clientType);

    HRESULT(STDMETHODCALLTYPE* SetCallback)(
        IInputSwitchControl* This,
        /* [in] */ void* pInputSwitchCallback);

    HRESULT(STDMETHODCALLTYPE* ShowInputSwitch)(
        IInputSwitchControl* This,
        /* [in] */ RECT* lpRect);

    HRESULT(STDMETHODCALLTYPE* GetProfileCount)(
        IInputSwitchControl* This,
        /* [in] */ unsigned int* pOutNumberOfProfiles,
        /* [in] */ int* a3);

    // ...

    END_INTERFACE
} IInputSwitchControlVtbl;

interface IInputSwitchControl
{
    CONST_VTBL struct IInputSwitchControlVtbl* lpVtbl;
};

HRESULT(*CInputSwitchControl_InitFunc)(IInputSwitchControl*, unsigned int);
HRESULT CInputSwitchControl_InitHook(IInputSwitchControl* _this, unsigned int dwOriginalIMEStyle)
{
    return CInputSwitchControl_InitFunc(_this, dwIMEStyle ? dwIMEStyle : dwOriginalIMEStyle);
}

HRESULT (*CInputSwitchControl_ShowInputSwitchFunc)(IInputSwitchControl*, RECT*);
HRESULT CInputSwitchControl_ShowInputSwitchHook(IInputSwitchControl* _this, RECT* lpRect)
{
    if (!dwIMEStyle) // impossible case (this is not called for the Windows 11 language switcher), but just in case
    {
        return CInputSwitchControl_ShowInputSwitchFunc(_this, lpRect);
    }

    unsigned int dwNumberOfProfiles = 0;
    int a3 = 0;
    _this->lpVtbl->GetProfileCount(_this, &dwNumberOfProfiles, &a3);

    HWND hWndTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);

    UINT dpiX = 96, dpiY = 96;
    HRESULT hr = GetDpiForMonitor(
        MonitorFromWindow(hWndTaskbar, MONITOR_DEFAULTTOPRIMARY),
        MDT_DEFAULT,
        &dpiX,
        &dpiY
    );
    double dpix = dpiX / 96.0;
    double dpiy = dpiY / 96.0;

    //printf("RECT %d %d %d %d - %d %d\n", lpRect->left, lpRect->right, lpRect->top, lpRect->bottom, dwNumberOfProfiles, a3);

    RECT rc;
    GetWindowRect(hWndTaskbar, &rc);
    POINT pt;
    pt.x = rc.left;
    pt.y = rc.top;
    UINT tbPos = GetTaskbarLocationAndSize(pt, &rc);
    if (tbPos == TB_POS_BOTTOM)
    {
    }
    else if (tbPos == TB_POS_TOP)
    {
        if (dwIMEStyle == 1) // Windows 10 (with Language preferences link)
        {
            lpRect->top = rc.top + (rc.bottom - rc.top) + (UINT)(((double)dwNumberOfProfiles * (60.0 * dpiy)) + (5.0 * dpiy * 4.0) + (dpiy) + (48.0 * dpiy));
        }
        else if (dwIMEStyle == 2 || dwIMEStyle == 3 || dwIMEStyle == 4 || dwIMEStyle == 5) // LOGONUI, UAC, Windows 10, OOBE
        {
            lpRect->top = rc.top + (rc.bottom - rc.top) + (UINT)(((double)dwNumberOfProfiles * (60.0 * dpiy)) + (5.0 * dpiy * 2.0));
        }
    }
    else if (tbPos == TB_POS_LEFT)
    {
        if (dwIMEStyle == 1 || dwIMEStyle == 2 || dwIMEStyle == 3 || dwIMEStyle == 4 || dwIMEStyle == 5)
        {
            lpRect->right = rc.left + (rc.right - rc.left) + (UINT)((double)(300.0 * dpix));
            lpRect->top += (lpRect->bottom - lpRect->top);
        }
    }
    if (tbPos == TB_POS_RIGHT)
    {
        if (dwIMEStyle == 1 || dwIMEStyle == 2 || dwIMEStyle == 3 || dwIMEStyle == 4 || dwIMEStyle == 5)
        {
            lpRect->right = lpRect->right - (rc.right - rc.left);
            lpRect->top += (lpRect->bottom - lpRect->top);
        }
    }

    if (dwIMEStyle == 4)
    {
        lpRect->right -= (UINT)((double)(300.0 * dpix)) - (lpRect->right - lpRect->left);
    }

    return CInputSwitchControl_ShowInputSwitchFunc(_this, lpRect);
}

HRESULT explorer_CoCreateInstanceHook(
    REFCLSID   rclsid,
    LPUNKNOWN  pUnkOuter,
    DWORD      dwClsContext,
    REFIID     riid,
    IUnknown** ppv
)
{
    if (IsEqualCLSID(rclsid, &CLSID_InputSwitchControl) && IsEqualIID(riid, &IID_InputSwitchControl))
    {
        HRESULT hr = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
        if (SUCCEEDED(hr))
        {
            // The commented method below is no longer required as I have now came to patching
            // the interface's vtable.
            // Also, make sure to read the explanation below as well, it's useful for understanding
            // how this worked.
            IInputSwitchControl* pInputSwitchControl = *ppv;
            DWORD flOldProtect = 0;
            if (VirtualProtect(pInputSwitchControl->lpVtbl, sizeof(IInputSwitchControlVtbl), PAGE_EXECUTE_READWRITE, &flOldProtect))
            {
                CInputSwitchControl_ShowInputSwitchFunc = pInputSwitchControl->lpVtbl->ShowInputSwitch;
                pInputSwitchControl->lpVtbl->ShowInputSwitch = CInputSwitchControl_ShowInputSwitchHook;
                CInputSwitchControl_InitFunc = pInputSwitchControl->lpVtbl->Init;
                pInputSwitchControl->lpVtbl->Init = CInputSwitchControl_InitHook;
                VirtualProtect(pInputSwitchControl->lpVtbl, sizeof(IInputSwitchControlVtbl), flOldProtect, &flOldProtect);
            }
            // Pff... how this works:
            // 
            // * This `CoCreateInstance` call will get a pointer to an IInputSwitchControl interface
            // (the call to here is made from `explorer!CTrayInputIndicator::_RegisterInputSwitch`);
            // the next call on this pointer will be on the `IInputSwitchControl::Init` function.
            // 
            // * `IInputSwitchControl::Init`'s second parameter is a number (x) which tells which
            // language switcher UI to prepare (check `IsUtil::MapClientTypeToString` in
            // `InputSwitch.dll`). "explorer" requests the "DESKTOP" UI (x = 0), which is the new
            // Windows 11 UI; if we replace that number with something else, some other UI will
            // be created
            // 
            // * ~~We cannot patch the vtable of the COM object because the executable is protected
            // by control flow guard and we would make a jump to an invalid site (maybe there is
            // some clever workaround fpr this as well, somehow telling the compiler to place a certain
            // canary before our trampoline, so it matches with what the runtime support for CFG expects,
            // but we'd have to keep that canary in sync with the one in explorer.exe, so not very
            // future proof).~~ Edit: Not true after all.
            // 
            // * Taking advantage of the fact that the call to `IInputSwitchControl::Init` is the thing
            // that happens right after we return from here, and looking on the disassembly, we see nothing
            // else changes `rdx` (which is the second argument to a function call), basically x, besides the
            // very `xor edx, edx` instruction before the call. Thus, we patch that out, and we also do
            // `mov edx, whatever` here; afterwards, we do NOTHING else, but just return and hope that
            // edx will stick
            // 
            // * Needless to say this is **HIGHLY** amd64
            /*
            char pattern[2] = {0x33, 0xD2};
            DWORD dwOldProtect;
            char* p_mov_edx_val = mov_edx_val;
            if (!ep_pf)
            {
                ep_pf = memmem(_ReturnAddress(), 200, pattern, 2);
                if (ep_pf)
                {
                    // Cancel out `xor edx, edx`
                    VirtualProtect(ep_pf, 2, PAGE_EXECUTE_READWRITE, &dwOldProtect);
                    memset(ep_pf, 0x90, 2);
                    VirtualProtect(ep_pf, 2, dwOldProtect, &dwOldProtect);
                }
                VirtualProtect(p_mov_edx_val, 6, PAGE_EXECUTE_READWRITE, &dwOldProtect);
            }
            if (ep_pf)
            {
                // Craft a "function" which does `mov edx, whatever; ret` and call it
                DWORD* pVal = mov_edx_val + 1;
                *pVal = dwIMEStyle;
                void(*pf_mov_edx_val)() = p_mov_edx_val;
                pf_mov_edx_val();
            }
            */
        }
        return hr;
    }
    return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}
#endif
#pragma endregion


#pragma region "Explorer Registry Hooks"
LSTATUS explorer_RegCreateKeyExW(HKEY a1, const WCHAR* a2,  DWORD a3, WCHAR* a4, DWORD a5, REGSAM a6, struct _SECURITY_ATTRIBUTES* a7, HKEY* a8, DWORD* a9)
{
    const wchar_t* v13; // rdx
    int v14; // eax

    if (lstrcmpW(a2, L"MMStuckRects3"))
    {
        v14 = lstrcmpW(a2, L"StuckRects3");
        v13 = L"StuckRectsLegacy";
        if (v14)
        {
            v13 = a2;
        }
    }
    else
    {
        v13 = L"MMStuckRectsLegacy";
    }

    return RegCreateKeyExW(a1, v13, a3, a4, a5, a6, a7, a8, a9);
}

LSTATUS explorer_SHGetValueW(HKEY a1, const WCHAR* a2, const WCHAR* a3, DWORD* a4, void* a5, DWORD* a6)
{
    const WCHAR* v10; // rdx
    int v11; // eax

    if (lstrcmpW(a2, L"MMStuckRects3"))
    {
        v11 = lstrcmpW(a2, L"StuckRects3");
        v10 = L"StuckRectsLegacy";
        if (v11)
            v10 = a2;
    }
    else
    {
        v10 = L"MMStuckRectsLegacy";
    }

    return SHGetValueW(a1, v10, a3, a4, a5, a6);
}

LSTATUS explorer_OpenRegStream(HKEY hkey, PCWSTR pszSubkey, PCWSTR pszValue, DWORD grfMode)
{
    DWORD flOldProtect[6];

    if (!lstrcmpiW(pszValue, L"TaskbarWinXP")
        && VirtualProtect(pszValue, 0xC8ui64, 0x40u, flOldProtect))
    {
        lstrcpyW(pszValue, L"TaskbarWinEP");
        VirtualProtect(pszValue, 0xC8ui64, flOldProtect[0], flOldProtect);
    }

    return OpenRegStream(hkey, pszSubkey, pszValue, grfMode);
}

LSTATUS explorer_RegOpenKeyExW(HKEY a1, WCHAR* a2, DWORD a3, REGSAM a4, HKEY* a5)
{
    DWORD flOldProtect[6];

    if (!lstrcmpiW(a2, L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\TrayNotify")
        && VirtualProtect(a2, 0xC8ui64, 0x40u, flOldProtect))
    {
        lstrcpyW(a2, L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\TrayNotSIB");
        VirtualProtect(a2, 0xC8ui64, flOldProtect[0], flOldProtect);
    }

    return RegOpenKeyExW(a1, a2, a3, a4, a5);
}

LSTATUS explorer_RegSetValueExW(
    HKEY       hKey,
    LPCWSTR    lpValueName,
    DWORD      Reserved,
    DWORD      dwType,
    const BYTE* lpData,
    DWORD      cbData
)
{
    if (!lstrcmpW(lpValueName, L"ShowCortanaButton"))
    {
        if (cbData == sizeof(DWORD) && *(DWORD*)lpData == 1)
        {
            DWORD dwData = 2;
            return RegSetValueExW(hKey, L"TaskbarDa", Reserved, dwType, &dwData, cbData);
        }
        return RegSetValueExW(hKey, L"TaskbarDa", Reserved, dwType, lpData, cbData);
    }

    return RegSetValueExW(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}

LSTATUS explorer_RegGetValueW(
    HKEY    hkey,
    LPCWSTR lpSubKey,
    LPCWSTR lpValue,
    DWORD   dwFlags,
    LPDWORD pdwType,
    PVOID   pvData,
    LPDWORD pcbData
)
{
    DWORD flOldProtect;
    BOOL bShowTaskViewButton = FALSE;
    LSTATUS lRes;

    if (!lstrcmpW(lpValue, L"ShowCortanaButton"))
    {
        lRes = RegGetValueW(hkey, lpSubKey, L"TaskbarDa", dwFlags, pdwType, pvData, pcbData);
        if (*(DWORD*)pvData == 2)
        {
            *(DWORD*)pvData = 1;
        }
    }
    else if (!lstrcmpW(lpValue, L"TaskbarGlomLevel") || !lstrcmpW(lpValue, L"MMTaskbarGlomLevel"))
    {
        lRes = RegGetValueW(HKEY_CURRENT_USER, _T(REGPATH), lpValue, dwFlags, pdwType, pvData, pcbData);
        if (lRes != ERROR_SUCCESS)
        {
            *(DWORD*)pvData = (lpValue[0] == L'T' ? TASKBARGLOMLEVEL_DEFAULT : MMTASKBARGLOMLEVEL_DEFAULT);
            *(DWORD*)pcbData = sizeof(DWORD32);
            lRes = ERROR_SUCCESS;
        }
    }
    /*else if (!lstrcmpW(lpValue, L"PeopleBand"))
    {
        lRes = RegGetValueW(hkey, lpSubKey, L"TaskbarMn", dwFlags, pdwType, pvData, pcbData);
    }*/
    else
    {
        lRes = RegGetValueW(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
    }

    if (!lstrcmpW(lpValue, L"SearchboxTaskbarMode"))
    {
        if (*(DWORD*)pvData)
        {
            *(DWORD*)pvData = 1;
        }

        lRes = ERROR_SUCCESS;
    }

    return lRes;
}

LSTATUS twinuipcshell_RegGetValueW(
    HKEY    hkey,
    LPCWSTR lpSubKey,
    LPCWSTR lpValue,
    DWORD   dwFlags,
    LPDWORD pdwType,
    PVOID   pvData,
    LPDWORD pcbData
)
{
    LSTATUS lRes = RegGetValueW(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);

    if (!lstrcmpW(lpValue, L"AltTabSettings"))
    {
        if (lRes == ERROR_SUCCESS && *(DWORD*)pvData)
        {
            if (*(DWORD*)pvData == 3)
            {
                *(DWORD*)pvData = 0;
            }
            else
            {
                *(DWORD*)pvData = 1;
            }
        }

        if (!bOldTaskbar && hWin11AltTabInitialized)
        {
            SetEvent(hWin11AltTabInitialized);
            CloseHandle(hWin11AltTabInitialized);
            hWin11AltTabInitialized = NULL;
        }

        lRes = ERROR_SUCCESS;
    }

    return lRes;
}

HRESULT (*explorer_SHCreateStreamOnModuleResourceWFunc)(
    HMODULE hModule,
    LPCWSTR pwszName,
    LPCWSTR pwszType,
    IStream** ppStream
);

HRESULT WINAPI explorer_SHCreateStreamOnModuleResourceWHook(
    HMODULE hModule,
    LPCWSTR pwszName,
    LPCWSTR pwszType,
    IStream** ppStream
)
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(hModule, path, MAX_PATH);
    if ((*((WORD*)&(pwszName)+1)))
    {
        wprintf(L"%s - %s %s\n", path, pwszName, pwszType);
    }
    else
    {
        wprintf(L"%s - %d %s\n", path, pwszName, pwszType);

        IStream* pStream = NULL;
        if (pwszName < 124)
        {
            if (S_Icon_Dark_TaskView)
            {
                pStream = SHCreateMemStream(P_Icon_Dark_TaskView, S_Icon_Dark_TaskView);
                if (pStream)
                {
                    *ppStream = pStream;
                    return S_OK;
                }
            }
        }
        else if (pwszName >= 151)
        {
            if (pwszName < 163)
            {
                if (S_Icon_Dark_Search)
                {
                    pStream = SHCreateMemStream(P_Icon_Dark_Search, S_Icon_Dark_Search);
                    if (pStream)
                    {
                        *ppStream = pStream;
                        return S_OK;
                    }
                }
            }

            if (pwszName < 201)
            {
                if (S_Icon_Light_Search)
                {
                    pStream = SHCreateMemStream(P_Icon_Light_Search, S_Icon_Light_Search);
                    if (pStream)
                    {
                        *ppStream = pStream;
                        return S_OK;
                    }
                }
            }

            if (pwszName < 213)
            {
                if (S_Icon_Dark_Widgets)
                {
                    printf(">>> %p %d\n", P_Icon_Dark_Widgets, S_Icon_Dark_Widgets);
                    pStream = SHCreateMemStream(P_Icon_Dark_Widgets, S_Icon_Dark_Widgets);
                    if (pStream)
                    {
                        *ppStream = pStream;
                        return S_OK;
                    }
                }
            }

            if (pwszName < 251)
            {
                if (S_Icon_Light_Widgets)
                {
                    pStream = SHCreateMemStream(P_Icon_Light_Widgets, S_Icon_Light_Widgets);
                    if (pStream)
                    {
                        *ppStream = pStream;
                        return S_OK;
                    }
                }
            }
        }
        else if (pwszName < 307)
        {
            if (S_Icon_Light_TaskView)
            {
                pStream = SHCreateMemStream(P_Icon_Light_TaskView, S_Icon_Light_TaskView);
                if (pStream)
                {
                    *ppStream = pStream;
                    return S_OK;
                }
            }
        }
    }
    return explorer_SHCreateStreamOnModuleResourceWFunc(hModule, pwszName, pwszType, ppStream);
}
#pragma endregion


#pragma region "Remember primary taskbar positioning"
BOOL bTaskbarFirstTimePositioning = FALSE;
BOOL bTaskbarSet = FALSE;

BOOL explorer_SetRect(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom)
{
    BOOL bIgnore = FALSE;
    if (bTaskbarFirstTimePositioning)
    {
        bIgnore = bTaskbarSet;
    }
    else
    {
        bTaskbarFirstTimePositioning = TRUE;
        bIgnore = (GetSystemMetrics(SM_CMONITORS) == 1);
        bTaskbarSet = bIgnore;
    }

    if (bIgnore)
    {
        return SetRect(lprc, xLeft, yTop, xRight, yBottom);
    }
    if (xLeft)
    {
        return SetRect(lprc, xLeft, yTop, xRight, yBottom);
    }
    if (yTop)
    {
        return SetRect(lprc, xLeft, yTop, xRight, yBottom);
    }
    if (xRight != GetSystemMetrics(SM_CXSCREEN))
    {
        return SetRect(lprc, xLeft, yTop, xRight, yBottom);
    }
    if (yBottom != GetSystemMetrics(SM_CYSCREEN))
    {
        return SetRect(lprc, xLeft, yTop, xRight, yBottom);
    }

    bTaskbarSet = TRUE;
    
    StuckRectsData srd;
    DWORD pcbData = sizeof(StuckRectsData);
    RegGetValueW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StuckRectsLegacy",
        L"Settings",
        REG_BINARY,
        NULL,
        &srd,
        &pcbData);

    if (pcbData != sizeof(StuckRectsData))
    {
        return SetRect(lprc, xLeft, yTop, xRight, yBottom);
    }

    if (srd.pvData[0] != sizeof(StuckRectsData))
    {
        return SetRect(lprc, xLeft, yTop, xRight, yBottom);
    }

    if (srd.pvData[1] != -2)
    {
        return SetRect(lprc, xLeft, yTop, xRight, yBottom);
    }

    HMONITOR hMonitor = MonitorFromRect(&(srd.rc), MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi;
    ZeroMemory(&mi, sizeof(MONITORINFO));
    mi.cbSize = sizeof(MONITORINFO);
    if (!GetMonitorInfoW(hMonitor, &mi))
    {
        return SetRect(lprc, xLeft, yTop, xRight, yBottom);
    }

    if (lprc)
    {
        *lprc = mi.rcMonitor;
        return TRUE;
    }

    return FALSE;
}
#pragma endregion


#pragma region "Disable Office Hotkeys"
const UINT office_hotkeys[10] = { 0x57, 0x54, 0x59, 0x4F, 0x50, 0x44, 0x4C, 0x58, 0x4E, 0x20 };
BOOL explorer_RegisterHotkeyHook(HWND hWnd, int id, UINT fsModifiers, UINT vk)
{
    if (fsModifiers == (MOD_ALT | MOD_CONTROL | MOD_SHIFT | MOD_WIN | MOD_NOREPEAT) && (
        vk == office_hotkeys[0] ||
        vk == office_hotkeys[1] ||
        vk == office_hotkeys[2] ||
        vk == office_hotkeys[3] ||
        vk == office_hotkeys[4] ||
        vk == office_hotkeys[5] ||
        vk == office_hotkeys[6] ||
        vk == office_hotkeys[7] ||
        vk == office_hotkeys[8] ||
        vk == office_hotkeys[9] ||
        !vk))
    {
        SetLastError(ERROR_HOTKEY_ALREADY_REGISTERED);
        return FALSE;
    }
    return RegisterHotKey(hWnd, id, fsModifiers, vk);
}
#pragma endregion


DWORD InjectBasicFunctions(BOOL bIsExplorer, BOOL bInstall)
{
    //Sleep(150);

    HMODULE hShlwapi = LoadLibraryW(L"Shlwapi.dll");
    if (bInstall)
    {
        SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(hShlwapi, "SHRegGetValueFromHKCUHKLM");
    }
    else
    {
        FreeLibrary(hShlwapi);
        FreeLibrary(hShlwapi);
    }

    HANDLE hShell32 = LoadLibraryW(L"shell32.dll");
    if (bInstall)
    {
        VnPatchIAT(hShell32, "user32.dll", "TrackPopupMenu", TrackPopupMenuHook);
        VnPatchIAT(hShell32, "user32.dll", "SystemParametersInfoW", DisableImmersiveMenus_SystemParametersInfoW);
        if (!bIsExplorer)
        {
            CreateWindowExWFunc = CreateWindowExW;
            VnPatchIAT(hShell32, "user32.dll", "CreateWindowExW", CreateWindowExWHook);
            SetWindowLongPtrWFunc = SetWindowLongPtrW;
            VnPatchIAT(hShell32, "user32.dll", "SetWindowLongPtrW", SetWindowLongPtrWHook);
        }
    }
    else
    {
        VnPatchIAT(hShell32, "user32.dll", "TrackPopupMenu", TrackPopupMenu);
        VnPatchIAT(hShell32, "user32.dll", "SystemParametersInfoW", SystemParametersInfoW);
        if (!bIsExplorer)
        {
            VnPatchIAT(hShell32, "user32.dll", "CreateWindowExW", CreateWindowExW);
            VnPatchIAT(hShell32, "user32.dll", "SetWindowLongPtrW", SetWindowLongPtrW);
        }
        FreeLibrary(hShell32);
        FreeLibrary(hShell32);
    }

    HANDLE hShcore = LoadLibraryW(L"shcore.dll");
    if (bInstall)
    {
        explorerframe_SHCreateWorkerWindowFunc = GetProcAddress(hShcore, (LPCSTR)188);
    }
    else
    {
        FreeLibrary(hShcore);
        FreeLibrary(hShcore);
    }

    HANDLE hExplorerFrame = LoadLibraryW(L"ExplorerFrame.dll");
    if (bInstall)
    {
        VnPatchIAT(hExplorerFrame, "user32.dll", "TrackPopupMenu", TrackPopupMenuHook);
        VnPatchIAT(hExplorerFrame, "user32.dll", "SystemParametersInfoW", DisableImmersiveMenus_SystemParametersInfoW);
        VnPatchIAT(hExplorerFrame, "shcore.dll", (LPCSTR)188, explorerframe_SHCreateWorkerWindowHook);  // <<<SAB>>>
        if (!bIsExplorer)
        {
            CreateWindowExWFunc = CreateWindowExW;
            VnPatchIAT(hExplorerFrame, "user32.dll", "CreateWindowExW", CreateWindowExWHook);
            SetWindowLongPtrWFunc = SetWindowLongPtrW;
            VnPatchIAT(hExplorerFrame, "user32.dll", "SetWindowLongPtrW", SetWindowLongPtrWHook);
        }
        VnPatchIAT(hExplorerFrame, "API-MS-WIN-CORE-STRING-L1-1-0.DLL", "CompareStringOrdinal", ExplorerFrame_CompareStringOrdinal);
    }
    else
    {
        VnPatchIAT(hExplorerFrame, "user32.dll", "TrackPopupMenu", TrackPopupMenu);
        VnPatchIAT(hExplorerFrame, "user32.dll", "SystemParametersInfoW", SystemParametersInfoW);
        VnPatchIAT(hExplorerFrame, "shcore.dll", (LPCSTR)188, explorerframe_SHCreateWorkerWindowFunc);
        if (!bIsExplorer)
        {
            VnPatchIAT(hExplorerFrame, "user32.dll", "CreateWindowExW", CreateWindowExW);
            VnPatchIAT(hExplorerFrame, "user32.dll", "SetWindowLongPtrW", SetWindowLongPtrW);
        }
        VnPatchIAT(hExplorerFrame, "API-MS-WIN-CORE-STRING-L1-1-0.DLL", "CompareStringOrdinal", CompareStringOrdinal);
        FreeLibrary(hExplorerFrame);
        FreeLibrary(hExplorerFrame);
    }

    HANDLE hWindowsUIFileExplorer = LoadLibraryW(L"Windows.UI.FileExplorer.dll");
    if (hWindowsUIFileExplorer)
    {
        VnPatchDelayIAT(hWindowsUIFileExplorer, "user32.dll", "TrackPopupMenu", TrackPopupMenuHook);
        VnPatchDelayIAT(hWindowsUIFileExplorer, "user32.dll", "SystemParametersInfoW", DisableImmersiveMenus_SystemParametersInfoW);
        if (!bIsExplorer)
        {
            CreateWindowExWFunc = CreateWindowExW;
            VnPatchIAT(hWindowsUIFileExplorer, "user32.dll", "CreateWindowExW", CreateWindowExWHook);
            SetWindowLongPtrWFunc = SetWindowLongPtrW;
            VnPatchIAT(hWindowsUIFileExplorer, "user32.dll", "SetWindowLongPtrW", SetWindowLongPtrWHook);
        }
    }
    else
    {
        VnPatchDelayIAT(hWindowsUIFileExplorer, "user32.dll", "TrackPopupMenu", TrackPopupMenu);
        VnPatchDelayIAT(hWindowsUIFileExplorer, "user32.dll", "SystemParametersInfoW", SystemParametersInfoW);
        if (!bIsExplorer)
        {
            VnPatchIAT(hWindowsUIFileExplorer, "user32.dll", "CreateWindowExW", CreateWindowExW);
            VnPatchIAT(hWindowsUIFileExplorer, "user32.dll", "SetWindowLongPtrW", SetWindowLongPtrW);
        }
        FreeLibrary(hWindowsUIFileExplorer);
        FreeLibrary(hWindowsUIFileExplorer);
    }
}

INT64(*twinui_pcshell_IsUndockedAssetAvailableFunc)(INT a1, INT64 a2, INT64 a3, const char* a4);
INT64 twinui_pcshell_IsUndockedAssetAvailableHook(INT a1, INT64 a2, INT64 a3, const char* a4)
{
    // if IsAltTab and AltTabSettings == Windows 10 or sws (Precision Touchpad gesture)
    if (a1 == 1 && (dwAltTabSettings == 3 || dwAltTabSettings == 2)) 
    {
        return 0;
    }
    // if IsSnapAssist and SnapAssistSettings == Windows 10
    else if (a1 == 4 && dwSnapAssistSettings == 3)
    {
        return 0;
    }
    // else, show Windows 11 style basically
    else
    {
        return twinui_pcshell_IsUndockedAssetAvailableFunc(a1, a2, a3, a4);
    }
}

BOOL IsDebuggerPresentHook()
{
    return FALSE;
}

DWORD Inject(BOOL bIsExplorer)
{
#if defined(DEBUG) | defined(_DEBUG)
    FILE* conout;
    AllocConsole();
    freopen_s(
        &conout, 
        "CONOUT$",
        "w", 
        stdout
    );
#endif

    int rv;

    if (bIsExplorer)
    {
        wszWeatherLanguage = malloc(sizeof(WCHAR) * MAX_PATH);
        wszWeatherTerm = malloc(sizeof(WCHAR) * MAX_PATH);
    }

    LoadSettings(MAKELPARAM(bIsExplorer, FALSE));

#ifdef _WIN64
    if (bIsExplorer)
    {
        funchook = funchook_create();
        printf("funchook create %d\n", funchook != 0);
    }
#endif

    if (bIsExplorer)
    {
        hSwsSettingsChanged = CreateEventW(NULL, FALSE, FALSE, NULL);
        hSwsOpacityMaybeChanged = CreateEventW(NULL, FALSE, FALSE, NULL);
    }

    unsigned int numSettings = bIsExplorer ? 12 : 2;
    Setting* settings = calloc(numSettings, sizeof(Setting));
    if (settings)
    {
        unsigned int cs = 0;

        if (cs < numSettings)
        {
            settings[cs].callback = NULL;
            settings[cs].data = NULL;
            settings[cs].hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
            settings[cs].hKey = NULL;
            ZeroMemory(settings[cs].name, MAX_PATH);
            settings[cs].origin = NULL;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = LoadSettings;
            settings[cs].data = MAKELPARAM(bIsExplorer, TRUE);
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, TEXT(REGPATH));
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = LoadSettings;
            settings[cs].data = MAKELPARAM(bIsExplorer, FALSE);
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = SetEvent;
            settings[cs].data = hSwsSettingsChanged;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, TEXT(REGPATH) L"\\sws");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = SetEvent;
            settings[cs].data = hSwsOpacityMaybeChanged;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MultitaskingView\\AltTabViewHost");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = Explorer_RefreshUI;
            settings[cs].data = NULL;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = Explorer_RefreshUI;
            settings[cs].data = NULL;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Search");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = Explorer_RefreshUI;
            settings[cs].data = NULL;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\People");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = Explorer_RefreshUI;
            settings[cs].data = NULL;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\TabletTip\\1.7");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = SetEvent;
            settings[cs].data = hSwsSettingsChanged;
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = UpdateStartMenuPositioning;
            settings[cs].data = MAKELPARAM(FALSE, TRUE);
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        if (cs < numSettings)
        {
            settings[cs].callback = LoadSettings;
            settings[cs].data = MAKELPARAM(bIsExplorer, FALSE);
            settings[cs].hEvent = NULL;
            settings[cs].hKey = NULL;
            wcscpy_s(settings[cs].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer");
            settings[cs].origin = HKEY_CURRENT_USER;
            cs++;
        }

        SettingsChangeParameters* settingsParams = calloc(1, sizeof(SettingsChangeParameters));
        if (settingsParams)
        {
            settingsParams->settings = settings;
            settingsParams->size = numSettings;
            settingsParams->hThread = CreateThread(
                0,
                0,
                MonitorSettings,
                settingsParams,
                0,
                0
            );
        }
        else
        {
            if (numSettings && settings[0].hEvent)
            {
                CloseHandle(settings[0].hEvent);
            }
            free(settings);
            settings = NULL;
        }
    }

    InjectBasicFunctions(bIsExplorer, TRUE);
    //if (!hDelayedInjectionThread)
    //{
    //    hDelayedInjectionThread = CreateThread(0, 0, InjectBasicFunctions, 0, 0, 0);
    //}

    if (!bIsExplorer)
    {
        return;
    }


    wszEPWeatherKillswitch = calloc(sizeof(WCHAR), MAX_PATH);
    rand_string(wszEPWeatherKillswitch, MAX_PATH / 2);
    wcscat_s(wszEPWeatherKillswitch, MAX_PATH, _T(EP_Weather_Killswitch));
    hEPWeatherKillswitch = CreateMutexW(NULL, TRUE, wszEPWeatherKillswitch);


#ifdef _WIN64
    hCanStartSws = CreateEventW(NULL, FALSE, FALSE, NULL);
    hWin11AltTabInitialized = CreateEventW(NULL, FALSE, FALSE, NULL);
    CreateThread(
        0,
        0,
        WindowSwitcher,
        0,
        0,
        0
    );


#ifdef USE_PRIVATE_INTERFACES
    P_Icon_Dark_Search = ReadFromFile(L"C:\\Users\\root\\Downloads\\pri\\resources\\Search_Dark\\png\\32.png", &S_Icon_Dark_Search);
    P_Icon_Light_Search = ReadFromFile(L"C:\\Users\\root\\Downloads\\pri\\resources\\Search_Light\\png\\32.png", &S_Icon_Light_Search);
    P_Icon_Dark_TaskView = ReadFromFile(L"C:\\Users\\root\\Downloads\\pri\\resources\\TaskView_Dark\\png\\32.png", &S_Icon_Dark_TaskView);
    P_Icon_Light_TaskView = ReadFromFile(L"C:\\Users\\root\\Downloads\\pri\\resources\\TaskView_Light\\png\\32.png", &S_Icon_Light_TaskView);
    P_Icon_Dark_Widgets = ReadFromFile(L"C:\\Users\\root\\Downloads\\pri\\resources\\Widgets_Dark\\png\\32.png", &S_Icon_Dark_Widgets);
    P_Icon_Light_Widgets = ReadFromFile(L"C:\\Users\\root\\Downloads\\pri\\resources\\Widgets_Light\\png\\32.png", &S_Icon_Dark_Widgets);
#endif


    symbols_addr symbols_PTRS;
    ZeroMemory(
        &symbols_PTRS,
        sizeof(symbols_addr)
    );
    if (LoadSymbols(&symbols_PTRS, hModule))
    {
        if (bEnableSymbolDownload)
        {
            printf("Attempting to download symbol data; for now, the program may have limited functionality.\n");
            DownloadSymbolsParams* params = malloc(sizeof(DownloadSymbolsParams));
            params->hModule = hModule;
            params->bVerbose = FALSE;
            CreateThread(0, 0, DownloadSymbols, params, 0, 0);
        }
    }
    else
    {
        printf("Loaded symbols\n");
    }


    HANDLE hUser32 = LoadLibraryW(L"user32.dll");
    CreateWindowInBand = GetProcAddress(hUser32, "CreateWindowInBand");
    GetWindowBand = GetProcAddress(hUser32, "GetWindowBand");
    SetWindowBand = GetProcAddress(hUser32, "SetWindowBand");
    SetWindowCompositionAttribute = GetProcAddress(hUser32, "SetWindowCompositionAttribute");
    printf("Setup user32 functions done\n");


    HANDLE hExplorer = GetModuleHandleW(NULL);
    SetChildWindowNoActivateFunc = GetProcAddress(GetModuleHandleW(L"user32.dll"), (LPCSTR)2005);
    if (bOldTaskbar)
    {
        VnPatchIAT(hExplorer, "user32.dll", (LPCSTR)2005, explorer_SetChildWindowNoActivateHook);
        VnPatchDelayIAT(hExplorer, "ext-ms-win-rtcore-ntuser-window-ext-l1-1-0.dll", "SendMessageW", explorer_SendMessageW);
        VnPatchIAT(hExplorer, "api-ms-win-core-libraryloader-l1-2-0.dll", "GetProcAddress", explorer_GetProcAddressHook);
        VnPatchIAT(hExplorer, "shell32.dll", "ShellExecuteW", explorer_ShellExecuteW);
        VnPatchIAT(hExplorer, "shell32.dll", "ShellExecuteExW", explorer_ShellExecuteExW);
        VnPatchIAT(hExplorer, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegGetValueW", explorer_RegGetValueW);
        VnPatchIAT(hExplorer, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegSetValueExW", explorer_RegSetValueExW);
        VnPatchIAT(hExplorer, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegCreateKeyExW", explorer_RegCreateKeyExW);
        VnPatchIAT(hExplorer, "API-MS-WIN-SHCORE-REGISTRY-L1-1-0.DLL", "SHGetValueW", explorer_SHGetValueW);
        VnPatchIAT(hExplorer, "user32.dll", "LoadMenuW", explorer_LoadMenuW);
        VnPatchIAT(hExplorer, "api-ms-win-core-shlwapi-obsolete-l1-1-0.dll", "QISearch", explorer_QISearch);
    }
    VnPatchIAT(hExplorer, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegOpenKeyExW", explorer_RegOpenKeyExW);
    VnPatchIAT(hExplorer, "shell32.dll", (LPCSTR)85, explorer_OpenRegStream);
    VnPatchIAT(hExplorer, "user32.dll", "TrackPopupMenuEx", explorer_TrackPopupMenuExHook);
    VnPatchIAT(hExplorer, "uxtheme.dll", "OpenThemeDataForDpi", explorer_OpenThemeDataForDpi);
    VnPatchIAT(hExplorer, "uxtheme.dll", "DrawThemeBackground", explorer_DrawThemeBackground);
    VnPatchIAT(hExplorer, "uxtheme.dll", "CloseThemeData", explorer_CloseThemeData);
    //VnPatchIAT(hExplorer, "api-ms-win-core-libraryloader-l1-2-0.dll", "LoadStringW", explorer_LoadStringWHook);
    if (bClassicThemeMitigations)
    {
        /*explorer_SetWindowThemeFunc = SetWindowTheme;
        rv = funchook_prepare(
            funchook,
            (void**)&explorer_SetWindowThemeFunc,
            explorer_SetWindowThemeHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }*/
        VnPatchIAT(hExplorer, "uxtheme.dll", "DrawThemeTextEx", explorer_DrawThemeTextEx);
        VnPatchIAT(hExplorer, "uxtheme.dll", "GetThemeMargins", explorer_GetThemeMargins);
        VnPatchIAT(hExplorer, "uxtheme.dll", "GetThemeMetric", explorer_GetThemeMetric);
        //VnPatchIAT(hExplorer, "uxtheme.dll", "OpenThemeDataForDpi", explorer_OpenThemeDataForDpi);
        //VnPatchIAT(hExplorer, "uxtheme.dll", "DrawThemeBackground", explorer_DrawThemeBackground);
        VnPatchIAT(hExplorer, "user32.dll", "SetWindowCompositionAttribute", explorer_SetWindowCompositionAttribute);
    }
    //VnPatchDelayIAT(hExplorer, "ext-ms-win-rtcore-ntuser-window-ext-l1-1-0.dll", "CreateWindowExW", explorer_CreateWindowExW);
    if (bOldTaskbar && dwIMEStyle)
    {
        VnPatchIAT(hExplorer, "api-ms-win-core-com-l1-1-0.dll", "CoCreateInstance", explorer_CoCreateInstanceHook);
    }
    if (bOldTaskbar)
    {
        VnPatchIAT(hExplorer, "API-MS-WIN-NTUSER-RECTANGLE-L1-1-0.DLL", "SetRect", explorer_SetRect);
    }
    if (bOldTaskbar)
    {
        VnPatchIAT(hExplorer, "USER32.DLL", "DeleteMenu", explorer_DeleteMenu);
    }


#ifdef USE_PRIVATE_INTERFACES
    HANDLE hShcore = LoadLibraryW(L"shcore.dll");
    explorer_SHCreateStreamOnModuleResourceWFunc = GetProcAddress(hShcore, (LPCSTR)109);
    VnPatchIAT(hExplorer, "shcore.dll", (LPCSTR)0x6D, explorer_SHCreateStreamOnModuleResourceWHook);
#endif

    printf("Setup explorer functions done\n");




    CreateWindowExWFunc = CreateWindowExW;
    rv = funchook_prepare(
        funchook,
        (void**)&CreateWindowExWFunc,
        CreateWindowExWHook
    );
    if (rv != 0)
    {
        FreeLibraryAndExitThread(hModule, rv);
        return rv;
    }
    SetWindowLongPtrWFunc = SetWindowLongPtrW;
    rv = funchook_prepare(
        funchook,
        (void**)&SetWindowLongPtrWFunc,
        SetWindowLongPtrWHook
    );
    if (rv != 0)
    {
        FreeLibraryAndExitThread(hModule, rv);
        return rv;
    }




    HANDLE hUxtheme = LoadLibraryW(L"uxtheme.dll");
    SetPreferredAppMode = GetProcAddress(hUxtheme, (LPCSTR)0x87);
    AllowDarkModeForWindow = GetProcAddress(hUxtheme, (LPCSTR)0x85);
    ShouldAppsUseDarkMode = GetProcAddress(hUxtheme, (LPCSTR)0x84);
    ShouldSystemUseDarkMode = GetProcAddress(hUxtheme, (LPCSTR)0x8A);
    GetThemeName = GetProcAddress(hUxtheme, (LPCSTR)0x4A);
    PeopleBand_DrawTextWithGlowFunc = GetProcAddress(hUxtheme, (LPCSTR)0x7E);
    if (bOldTaskbar)
    {
        VnPatchIAT(hExplorer, "uxtheme.dll", (LPCSTR)0x7E, PeopleBand_DrawTextWithGlowHook);
    }
    printf("Setup uxtheme functions done\n");


    HANDLE hTwinuiPcshell = LoadLibraryW(L"twinui.pcshell.dll");

    if (symbols_PTRS.twinui_pcshell_PTRS[0] && symbols_PTRS.twinui_pcshell_PTRS[0] != 0xFFFFFFFF)
    {
        CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc = (INT64(*)(HWND, int, HWND, int, BOOL*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[0]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[1] && symbols_PTRS.twinui_pcshell_PTRS[1] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_GetMenuItemsAsyncFunc = (INT64(*)(void*, void*, void**))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[1]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[2] && symbols_PTRS.twinui_pcshell_PTRS[2] != 0xFFFFFFFF)
    {
        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc = (INT64(*)(HMENU, HMENU, HWND, unsigned int, void*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[2]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[3] && symbols_PTRS.twinui_pcshell_PTRS[3] != 0xFFFFFFFF)
    {
        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc = (void(*)(HMENU, HMENU, HWND))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[3]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[4] && symbols_PTRS.twinui_pcshell_PTRS[4] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_ExecuteShutdownCommandFunc = (void(*)(void*, void*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[4]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[5] && symbols_PTRS.twinui_pcshell_PTRS[5] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_ExecuteCommandFunc = (void(*)(void*, int))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[5]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[6] && symbols_PTRS.twinui_pcshell_PTRS[6] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc = (INT64(*)(void*, POINT*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[6]);
        rv = funchook_prepare(
            funchook,
            (void**)&CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc,
            CLauncherTipContextMenu_ShowLauncherTipContextMenuHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[7] && symbols_PTRS.twinui_pcshell_PTRS[7] != 0xFFFFFFFF)
    {
        twinui_pcshell_IsUndockedAssetAvailableFunc = (INT64(*)(void*, POINT*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[7]);
        rv = funchook_prepare(
            funchook,
            (void**)&twinui_pcshell_IsUndockedAssetAvailableFunc,
            twinui_pcshell_IsUndockedAssetAvailableHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
    }

    /*if (symbols_PTRS.twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT - 1] && symbols_PTRS.twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT - 1] != 0xFFFFFFFF)
    {
        winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc = (INT64(*)(void*, POINT*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT - 1]);
        rv = funchook_prepare(
            funchook,
            (void**)&winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc,
            winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
    }*/
    VnPatchIAT(hTwinuiPcshell, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegGetValueW", twinuipcshell_RegGetValueW);
    //VnPatchIAT(hTwinuiPcshell, "api-ms-win-core-debug-l1-1-0.dll", "IsDebuggerPresent", IsDebuggerPresentHook);
    printf("Setup twinui.pcshell functions done\n");



    HANDLE hStobject = LoadLibraryW(L"stobject.dll");
    VnPatchIAT(hStobject, "api-ms-win-core-registry-l1-1-0.dll", "RegGetValueW", stobject_RegGetValueW);
    VnPatchIAT(hStobject, "api-ms-win-core-com-l1-1-0.dll", "CoCreateInstance", stobject_CoCreateInstanceHook);
    VnPatchDelayIAT(hStobject, "user32.dll", "TrackPopupMenu", stobject_TrackPopupMenuHook);
    VnPatchDelayIAT(hStobject, "user32.dll", "TrackPopupMenuEx", stobject_TrackPopupMenuExHook);
#ifdef USE_PRIVATE_INTERFACES
    if (bSkinIcons)
    {
        VnPatchDelayIAT(hStobject, "user32.dll", "LoadImageW", SystemTray_LoadImageWHook);
    }
#endif
    printf("Setup stobject functions done\n");



    HANDLE hBthprops = LoadLibraryW(L"bthprops.cpl");
    VnPatchIAT(hBthprops, "user32.dll", "TrackPopupMenuEx", bthprops_TrackPopupMenuExHook);
#ifdef USE_PRIVATE_INTERFACES
    if (bSkinIcons)
    {
        VnPatchIAT(hBthprops, "user32.dll", "LoadImageW", SystemTray_LoadImageWHook);
    }
#endif
    printf("Setup bthprops functions done\n");



    HANDLE hPnidui = LoadLibraryW(L"pnidui.dll");
    VnPatchIAT(hPnidui, "api-ms-win-core-com-l1-1-0.dll", "CoCreateInstance", pnidui_CoCreateInstanceHook);
    VnPatchIAT(hPnidui, "user32.dll", "TrackPopupMenu", pnidui_TrackPopupMenuHook);
#ifdef USE_PRIVATE_INTERFACES
    if (bSkinIcons)
    {
        VnPatchIAT(hPnidui, "user32.dll", "LoadImageW", SystemTray_LoadImageWHook);
    }
#endif
    printf("Setup pnidui functions done\n");



    HANDLE hSndvolsso = LoadLibraryW(L"sndvolsso.dll");
    VnPatchIAT(hSndvolsso, "user32.dll", "TrackPopupMenuEx", sndvolsso_TrackPopupMenuExHook);
    VnPatchIAT(hSndvolsso, "api-ms-win-core-registry-l1-1-0.dll", "RegGetValueW", sndvolsso_RegGetValueW);
#ifdef USE_PRIVATE_INTERFACES
    if (bSkinIcons)
    {
        VnPatchIAT(hSndvolsso, "user32.dll", "LoadImageW", SystemTray_LoadImageWHook);
    }
#endif
    printf("Setup sndvolsso functions done\n");



    HANDLE hShell32 = GetModuleHandleW(L"shell32.dll");
    if (hShell32)
    {
        HRESULT(*SHELL32_Create_IEnumUICommand)(IUnknown*, int*, int, IUnknown**) = GetProcAddress(hShell32, (LPCSTR)0x2E8);
        if (SHELL32_Create_IEnumUICommand)
        {
            char WVTASKITEM[80];
            ZeroMemory(WVTASKITEM, 80);
            IUnknown* pEnumUICommand = NULL;
            SHELL32_Create_IEnumUICommand(NULL, WVTASKITEM, 1, &pEnumUICommand);
            if (pEnumUICommand)
            {
                EnumExplorerCommand* pEnumExplorerCommand = NULL;
                pEnumUICommand->lpVtbl->QueryInterface(pEnumUICommand, &IID_EnumExplorerCommand, &pEnumExplorerCommand);
                pEnumUICommand->lpVtbl->Release(pEnumUICommand);
                if (pEnumExplorerCommand)
                {
                    UICommand* pUICommand = NULL;
                    pEnumExplorerCommand->lpVtbl->Next(pEnumExplorerCommand, 1, &pUICommand, NULL);
                    pEnumExplorerCommand->lpVtbl->Release(pEnumExplorerCommand);
                    if (pUICommand)
                    {
                        DWORD flOldProtect = 0;
                        if (VirtualProtect(pUICommand->lpVtbl, sizeof(UICommandVtbl), PAGE_EXECUTE_READWRITE, &flOldProtect))
                        {
                            shell32_UICommand_InvokeFunc = pUICommand->lpVtbl->Invoke;
                            pUICommand->lpVtbl->Invoke = shell32_UICommand_InvokeHook;
                            VirtualProtect(pUICommand->lpVtbl, sizeof(UICommandVtbl), flOldProtect, &flOldProtect);
                        }
                        pUICommand->lpVtbl->Release(pUICommand);
                    }
                }
            }
        }
    }
    printf("Setup shell32 functions done\n");



    HANDLE hInputSwitch = LoadLibraryW(L"InputSwitch.dll");
    printf("[IME] Context menu patch status: %d\n", PatchContextMenuOfNewMicrosoftIME(NULL));
    if (hInputSwitch)
    {
        VnPatchIAT(hInputSwitch, "user32.dll", "TrackPopupMenuEx", inputswitch_TrackPopupMenuExHook);
        printf("Setup inputswitch functions done\n");
    }



    HANDLE hPeopleBand = LoadLibraryW(L"PeopleBand.dll");
    if (hPeopleBand)
    {
        VnPatchIAT(hPeopleBand, "api-ms-win-core-largeinteger-l1-1-0.dll", "MulDiv", PeopleBand_MulDivHook);
        printf("Setup peopleband functions done\n");
    }




    rv = funchook_install(funchook, 0);
    if (rv != 0)
    {
        FreeLibraryAndExitThread(hModule, rv);
        return rv;
    }
    printf("Installed hooks.\n");




    /*HANDLE hEvent = CreateEventEx(
        0,
        L"ShellDesktopSwitchEvent",
        CREATE_EVENT_MANUAL_RESET,
        EVENT_ALL_ACCESS
    );
    if (GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("Created ShellDesktopSwitchEvent event.\n");
        ResetEvent(hEvent);
    }*/

    if (bOldTaskbar)
    {
        CreateThread(
            0,
            0,
            PlayStartupSound,
            0,
            0,
            0
        );
        printf("Play startup sound thread...\n");
    }


    if (bOldTaskbar)
    {
        CreateThread(
            0,
            0,
            SignalShellReady,
            dwExplorerReadyDelay,
            0,
            0
        );
        printf("Signal shell ready...\n");
    }
    else
    {
        CreateThread(
            0,
            0,
            FixTaskbarAutohide,
            0,
            0,
            0
        );
        RegDeleteKeyValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", L"TaskbarGlomLevel");
        RegDeleteKeyValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", L"MMTaskbarGlomLevel");
    }


    DWORD dwSize = 0;
    if (SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
        L"Control Panel\\Desktop\\WindowMetrics",
        L"MinWidth",
        SRRF_RT_REG_SZ,
        NULL,
        NULL,
        (LPDWORD)(&dwSize)
    ) != ERROR_SUCCESS)
    {
        RegSetKeyValueW(
            HKEY_CURRENT_USER, 
            L"Control Panel\\Desktop\\WindowMetrics",
            L"MinWidth",
            REG_SZ,
            L"38",
            sizeof(L"38")
        );
    }



    CreateThread(
        0,
        0,
        OpenStartOnCurentMonitorThread,
        0,
        0,
        0
    );
    printf("Open Start on monitor thread\n");



    CreateThread(
        0,
        0,
        EP_ServiceWindowThread,
        0,
        0,
        0
    );
    printf("EP Service Window thread\n");



    if (bDisableOfficeHotkeys)
    {
        VnPatchIAT(hExplorer, "user32.dll", "RegisterHotKey", explorer_RegisterHotkeyHook);
    }


    if (bEnableArchivePlugin)
    {
        ArchiveMenuThreadParams* params = calloc(1, sizeof(ArchiveMenuThreadParams));
        params->CreateWindowInBand = CreateWindowInBand;
        params->hWnd = &archivehWnd;
        params->wndProc = CLauncherTipContextMenu_WndProc;
        CreateThread(
            0,
            0,
            ArchiveMenuThread,
            params,
            0,
            0,
            0
        );
    }



    CreateThread(NULL, 0, CheckForUpdatesThread, 0, 0, NULL);



    WCHAR wszExtraLibPath[MAX_PATH];
    if (GetWindowsDirectoryW(wszExtraLibPath, MAX_PATH))
    {
        wcscat_s(wszExtraLibPath, MAX_PATH, L"\\ep_extra.dll");
        if (FileExistsW(wszExtraLibPath))
        {
            HMODULE hExtra = LoadLibraryW(wszExtraLibPath);
            if (hExtra)
            {
                printf("[Extra] Found library: %p.\n", hExtra);
                FARPROC ep_extra_entrypoint = GetProcAddress(hExtra, "ep_extra_EntryPoint");
                if (ep_extra_entrypoint)
                {
                    printf("[Extra] Running entry point...\n");
                    ep_extra_entrypoint();
                    printf("[Extra] Finished running entry point.\n");
                }
            }
            else
            {
                printf("[Extra] LoadLibraryW failed with 0x%x.", GetLastError());
            }
        }
    }



    /*if (bHookStartMenu)
    {
        HookStartMenuParams* params2 = calloc(1, sizeof(HookStartMenuParams));
        params2->dwTimeout = 1000;
        params2->hModule = hModule;
        params2->proc = InjectStartFromExplorer;
        GetModuleFileNameW(hModule, params2->wszModulePath, MAX_PATH);
        CreateThread(0, 0, HookStartMenu, params2, 0, 0);
    }*/



    // This notifies applications when the taskbar has recomputed its layout
    /*if (SUCCEEDED(TaskbarCenter_Initialize(hExplorer)))
    {
        printf("Initialized taskbar update notification.\n");
    }
    else
    {
        printf("Failed to register taskbar update notification.\n");
    }*/




    //CreateThread(0, 0, PositionStartMenuTimeout, 0, 0, 0);

    /*else
    {
        if (bIsExplorer)
        {
            // deinject all

            rv = funchook_uninstall(funchook, 0);
            if (rv != 0)
            {
                FreeLibraryAndExitThread(hModule, rv);
                return rv;
            }

            rv = funchook_destroy(funchook);
            if (rv != 0)
            {
                FreeLibraryAndExitThread(hModule, rv);
                return rv;
            }
        }

        //SetEvent(hExitSettingsMonitor);
        //WaitForSingleObject(hSettingsMonitorThread, INFINITE);
        //CloseHandle(hExitSettingsMonitor);
        //free(settingsParams);
        //free(settings);
        //InjectBasicFunctions(FALSE, FALSE);
        FreeLibraryAndExitThread(hModule, 0);
    }*/
#endif
    return 0;
}

#ifdef _WIN64
char VisibilityChangedEventArguments_GetVisible(__int64 a1)
{
    int v1;
    char v3[8];
    ZeroMemory(v3, 8);

    v1 = (*(__int64(__fastcall**)(__int64, char*))(*(INT64*)a1 + 48))(a1, v3);
    if (v1 < 0)
        return 0;

    return v3[0];
}

DWORD StartMenu_maximumFreqApps = 6;
DWORD StartMenu_ShowAllApps = 0;

void StartMenu_LoadSettings(BOOL bRestartIfChanged)
{
    HKEY hKey = NULL;
    DWORD dwSize, dwVal;

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("MakeAllAppsDefault"),
            0,
            NULL,
            &StartMenu_ShowAllApps,
            &dwSize
        );
        RegCloseKey(hKey);
    }
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH_STARTMENU),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
    if (hKey)
    {
        dwSize = sizeof(DWORD);
        dwVal = 6;
        RegQueryValueExW(
            hKey,
            TEXT("Start_MaximumFrequentApps"),
            0,
            NULL,
            &dwVal,
            &dwSize
        );
        if (bRestartIfChanged && dwVal != StartMenu_maximumFreqApps)
        {
            exit(0);
        }
        StartMenu_maximumFreqApps = dwVal;
        RegCloseKey(hKey);
    }
}

static INT64(*StartDocked_LauncherFrame_OnVisibilityChangedFunc)(void*, INT64, void*) = NULL;

static INT64(*StartDocked_LauncherFrame_ShowAllAppsFunc)(void* _this) = NULL;

INT64 StartDocked_LauncherFrame_OnVisibilityChangedHook(void* _this, INT64 a2, void* VisibilityChangedEventArguments)
{
    INT64 r = 0;
    if (StartDocked_LauncherFrame_OnVisibilityChangedFunc)
    {
        r = StartDocked_LauncherFrame_OnVisibilityChangedFunc(_this, a2, VisibilityChangedEventArguments);
    }
    if (StartMenu_ShowAllApps)
    {
        //if (VisibilityChangedEventArguments_GetVisible(VisibilityChangedEventArguments))
        {
            if (StartDocked_LauncherFrame_ShowAllAppsFunc)
            {
                StartDocked_LauncherFrame_ShowAllAppsFunc(_this);
            }
        }
    }
    return r;
}

INT64(*StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsFunc)(void*) = NULL;

INT64 StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsHook(void* _this)
{
    return StartMenu_maximumFreqApps;
}

INT64(*StartDocked_StartSizingFrame_StartSizingFrameFunc)(void* _this) = NULL;

INT64 StartDocked_StartSizingFrame_StartSizingFrameHook(void* _this)
{
    INT64 rv = StartDocked_StartSizingFrame_StartSizingFrameFunc(_this);
    HMODULE hModule = LoadLibraryW(L"Shlwapi.dll");
    if (hModule)
    {
        DWORD dwStatus = 0, dwSize = sizeof(DWORD);
        FARPROC SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(hModule, "SHRegGetValueFromHKCUHKLM");
        if (!SHRegGetValueFromHKCUHKLMFunc || SHRegGetValueFromHKCUHKLMFunc(
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
            TEXT("TaskbarAl"),
            SRRF_RT_REG_DWORD,
            NULL,
            &dwStatus,
            (LPDWORD)(&dwSize)
        ) != ERROR_SUCCESS)
        {
            dwStatus = 0;
        }
        FreeLibrary(hModule);
        *(((char*)_this + 387)) = dwStatus;
    }
    return rv;
}

int WINAPI SetupMessage(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
    return 0;
    LPCWSTR lpOldText = lpText;
    LPCWSTR lpOldCaption = lpCaption;
    wchar_t wszText[MAX_PATH];
    ZeroMemory(wszText, MAX_PATH * sizeof(wchar_t));
    wchar_t wszCaption[MAX_PATH];
    ZeroMemory(wszCaption, MAX_PATH * sizeof(wchar_t));
    LoadStringW(hModule, IDS_PRODUCTNAME, wszCaption, MAX_PATH);
    switch (Code)
    {
    case 1:
        LoadStringW(hModule, IDS_INSTALL_SUCCESS_TEXT, wszText, MAX_PATH);
        break;
    case -1:
        LoadStringW(hModule, IDS_INSTALL_ERROR_TEXT, wszText, MAX_PATH);
        break;
    case 2:
        LoadStringW(hModule, IDS_UNINSTALL_SUCCESS_TEXT, wszText, MAX_PATH);
        break;
    case -2:
        LoadStringW(hModule, IDS_UNINSTALL_ERROR_TEXT, wszText, MAX_PATH);
        break;
    default:
        LoadStringW(hModule, IDS_OPERATION_NONE, wszText, MAX_PATH);
        break;
    }
    int ret = MessageBoxW(hWnd, wszText, wszCaption, uType);
    lpText = lpOldText;
    lpOldCaption = lpOldCaption;
    return ret;
}

void Setup_Regsvr32(BOOL bInstall)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    if (!IsAppRunningAsAdminMode())
    {
        wchar_t wszPath[MAX_PATH];
        ZeroMemory(wszPath, ARRAYSIZE(wszPath));
        wchar_t wszCurrentDirectory[MAX_PATH];
        ZeroMemory(wszCurrentDirectory, ARRAYSIZE(wszCurrentDirectory));
        if (GetModuleFileNameW(NULL, wszPath, ARRAYSIZE(wszPath)) &&
            GetCurrentDirectoryW(ARRAYSIZE(wszCurrentDirectory), wszCurrentDirectory + (bInstall ? 1 : 4)))
        {
            wszCurrentDirectory[0] = L'"';
            if (!bInstall)
            {
                wszCurrentDirectory[0] = L'/';
                wszCurrentDirectory[1] = L'u';
                wszCurrentDirectory[2] = L' ';
                wszCurrentDirectory[3] = L'"';
            }
            wcscat_s(wszCurrentDirectory, ARRAYSIZE(wszCurrentDirectory), L"\\ExplorerPatcher.amd64.dll\"");
            SHELLEXECUTEINFOW sei;
            ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
            sei.cbSize = sizeof(sei);
            sei.lpVerb = L"runas";
            sei.lpFile = wszPath;
            sei.lpParameters = wszCurrentDirectory;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;
            if (!ShellExecuteExW(&sei))
            {
                DWORD dwError = GetLastError();
                if (dwError == ERROR_CANCELLED)
                {
                    wchar_t wszText[MAX_PATH];
                    ZeroMemory(wszText, MAX_PATH * sizeof(wchar_t));
                    wchar_t wszCaption[MAX_PATH];
                    ZeroMemory(wszCaption, MAX_PATH * sizeof(wchar_t));
                    LoadStringW(hModule, IDS_PRODUCTNAME, wszCaption, MAX_PATH);
                    LoadStringW(hModule, IDS_INSTALL_ERROR_TEXT, wszText, MAX_PATH);
                    MessageBoxW(0, wszText, wszCaption, MB_ICONINFORMATION);
                }
            }
            exit(0);
        }
    }

    VnPatchDelayIAT(GetModuleHandle(NULL), "ext-ms-win-ntuser-dialogbox-l1-1-0.dll", "MessageBoxW", SetupMessage);
}

#ifdef _WIN64
#pragma comment(linker, "/export:DllRegisterServer=_DllRegisterServer")
#endif
HRESULT WINAPI _DllRegisterServer()
{
    DWORD dwLastError = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    wchar_t wszFilename[MAX_PATH];
    wchar_t wszInstallPath[MAX_PATH];

    Setup_Regsvr32(TRUE);

    if (!dwLastError)
    {
        if (!GetModuleFileNameW(hModule, wszFilename, MAX_PATH))
        {
            dwLastError = GetLastError();
        }
    }
    if (!dwLastError)
    {
        dwLastError = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Classes\\CLSID\\" TEXT(EP_CLSID) L"\\InProcServer32",
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_WOW64_64KEY,
            NULL,
            &hKey,
            NULL
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegSetValueExW(
                hKey,
                NULL,
                0,
                REG_SZ,
                wszFilename,
                (wcslen(wszFilename) + 1) * sizeof(wchar_t)
            );
            dwLastError = RegSetValueExW(
                hKey,
                L"ThreadingModel",
                0,
                REG_SZ,
                L"Apartment",
                10 * sizeof(wchar_t)
            );
            RegCloseKey(hKey);
        }
    }
    if (!dwLastError)
    {
        PathRemoveExtensionW(wszFilename);
        PathRemoveExtensionW(wszFilename);
        wcscat_s(wszFilename, MAX_PATH, L".IA-32.dll");
    }
    if (!dwLastError)
    {
        dwLastError = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\WOW6432Node\\Classes\\CLSID\\" TEXT(EP_CLSID) L"\\InProcServer32",
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_WOW64_64KEY,
            NULL,
            &hKey,
            NULL
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegSetValueExW(
                hKey,
                NULL,
                0,
                REG_SZ,
                wszFilename,
                (wcslen(wszFilename) + 1) * sizeof(wchar_t)
            );
            dwLastError = RegSetValueExW(
                hKey,
                L"ThreadingModel",
                0,
                REG_SZ,
                L"Apartment",
                10 * sizeof(wchar_t)
            );
            RegCloseKey(hKey);
        }
    }
    if (!dwLastError)
    {
        dwLastError = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Classes\\Drive\\shellex\\FolderExtensions\\" TEXT(EP_CLSID),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_WOW64_64KEY,
            NULL,
            &hKey,
            NULL
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            DWORD dwDriveMask = 255;
            dwLastError = RegSetValueExW(
                hKey,
                L"DriveMask",
                0,
                REG_DWORD,
                &dwDriveMask,
                sizeof(DWORD)
            );
            RegCloseKey(hKey);
        }
    }
    /*if (!dwLastError)
    {
        dwLastError = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\" TEXT(EP_CLSID),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_WOW64_64KEY,
            NULL,
            &hKey,
            NULL
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            DWORD dwNoInternetExplorer = 1;
            dwLastError = RegSetValueExW(
                hKey,
                L"NoInternetExplorer",
                0,
                REG_DWORD,
                &dwNoInternetExplorer,
                sizeof(DWORD)
            );
            RegCloseKey(hKey);
        }
    }*/
    Code = 1;
    if (dwLastError) Code = -Code;

    //ZZRestartExplorer(0, 0, 0, 0);

    return dwLastError == 0 ? S_OK : HRESULT_FROM_WIN32(dwLastError);
}

#ifdef _WIN64
#pragma comment(linker, "/export:DllUnregisterServer=_DllUnregisterServer")
#endif
HRESULT WINAPI _DllUnregisterServer()
{
    DWORD dwLastError = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    wchar_t wszFilename[MAX_PATH];

    Setup_Regsvr32(FALSE);

    if (!dwLastError)
    {
        if (!GetModuleFileNameW(hModule, wszFilename, MAX_PATH))
        {
            dwLastError = GetLastError();
        }
    }
    if (!dwLastError)
    {
        dwLastError = RegOpenKeyW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Classes\\CLSID\\" TEXT(EP_CLSID),
            &hKey
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegDeleteTreeW(
                hKey,
                0
            );
            RegCloseKey(hKey);
            if (!dwLastError)
            {
                RegDeleteKeyW(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Classes\\CLSID\\" TEXT(EP_CLSID)
                );
            }
        }
    }
    if (!dwLastError)
    {
        PathRemoveExtensionW(wszFilename);
        PathRemoveExtensionW(wszFilename);
        wcscat_s(wszFilename, MAX_PATH, L".IA-32.dll");
    }
    if (!dwLastError)
    {
        dwLastError = RegOpenKeyW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\WOW6432Node\\Classes\\CLSID\\" TEXT(EP_CLSID),
            &hKey
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegDeleteTreeW(
                hKey,
                0
            );
            RegCloseKey(hKey);
            if (!dwLastError)
            {
                RegDeleteKeyW(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\WOW6432Node\\Classes\\CLSID\\" TEXT(EP_CLSID)
                );
            }
        }
    }
    if (!dwLastError)
    {
        dwLastError = RegOpenKeyW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Classes\\Drive\\shellex\\FolderExtensions\\" TEXT(EP_CLSID),
            &hKey
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegDeleteTreeW(
                hKey,
                0
            );
            RegCloseKey(hKey);
            if (!dwLastError)
            {
                RegDeleteKeyW(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Classes\\Drive\\shellex\\FolderExtensions\\" TEXT(EP_CLSID)
                );
            }
        }
    }
    /*if (!dwLastError)
    {
        dwLastError = RegOpenKeyW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\" TEXT(EP_CLSID),
            &hKey
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegDeleteTreeW(
                hKey,
                0
            );
            RegCloseKey(hKey);
            if (!dwLastError)
            {
                RegDeleteKeyW(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\" TEXT(EP_CLSID)
                );
            }
        }
    }*/
    Code = 2;
    if (dwLastError) Code = -Code;

    //ZZRestartExplorer(0, 0, 0, 0);

    return dwLastError == 0 ? S_OK : HRESULT_FROM_WIN32(dwLastError);
}
#endif

#ifdef _WIN64
#pragma comment(linker, "/export:DllCanUnloadNow=_DllCanUnloadNow")
#else
#pragma comment(linker, "/export:DllCanUnloadNow=__DllCanUnloadNow@0")
#endif
HRESULT WINAPI _DllCanUnloadNow()
{
    return S_FALSE;
}

void InjectStartMenu()
{
#ifdef _WIN64
    funchook = funchook_create();

    StartMenu_LoadSettings(FALSE);

    Setting* settings = calloc(3, sizeof(Setting));
    settings[0].callback = NULL;
    settings[0].data = NULL;
    settings[0].hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    settings[0].hKey = NULL;
    ZeroMemory(settings[0].name, MAX_PATH);
    settings[0].origin = NULL;
    settings[1].callback = StartMenu_LoadSettings;
    settings[1].data = FALSE;
    settings[1].hEvent = NULL;
    settings[1].hKey = NULL;
    wcscpy_s(settings[1].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage");
    settings[1].origin = HKEY_CURRENT_USER;
    settings[2].callback = StartMenu_LoadSettings;
    settings[2].data = TRUE;
    settings[2].hEvent = NULL;
    settings[2].hKey = NULL;
    wcscpy_s(settings[2].name, MAX_PATH, TEXT(REGPATH_STARTMENU));
    settings[2].origin = HKEY_CURRENT_USER;

    SettingsChangeParameters* params = calloc(1, sizeof(SettingsChangeParameters));
    params->settings = settings;
    params->size = 3;
    CreateThread(
        0,
        0,
        MonitorSettings,
        params,
        0,
        0
    );

    int rv;

    DWORD dwVal0 = 0x62254, dwVal1 = 0x188EBC, dwVal2 = 0x187120, dwVal3 = 0x3C10, dwVal4 = 0x160AEC;

    HMODULE hModule = LoadLibraryW(L"Shlwapi.dll");
    if (hModule)
    {
        DWORD dwStatus = 0, dwSize = sizeof(DWORD);
        FARPROC SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(hModule, "SHRegGetValueFromHKCUHKLM");

        if (SHRegGetValueFromHKCUHKLMFunc)
        {

            dwSize = sizeof(DWORD);
            SHRegGetValueFromHKCUHKLMFunc(
                TEXT(REGPATH_STARTMENU) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                TEXT(STARTDOCKED_SB_0),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwVal0,
                (LPDWORD)(&dwSize)
            );
            SHRegGetValueFromHKCUHKLMFunc(
                TEXT(REGPATH_STARTMENU) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                TEXT(STARTDOCKED_SB_1),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwVal1,
                (LPDWORD)(&dwSize)
            );
            SHRegGetValueFromHKCUHKLMFunc(
                TEXT(REGPATH_STARTMENU) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                TEXT(STARTDOCKED_SB_2),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwVal2,
                (LPDWORD)(&dwSize)
            );
            SHRegGetValueFromHKCUHKLMFunc(
                TEXT(REGPATH_STARTMENU) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                TEXT(STARTDOCKED_SB_3),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwVal3,
                (LPDWORD)(&dwSize)
            );
            SHRegGetValueFromHKCUHKLMFunc(
                TEXT(REGPATH_STARTMENU) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                TEXT(STARTDOCKED_SB_4),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwVal4,
                (LPDWORD)(&dwSize)
            );

        }
        FreeLibrary(hModule);
    }

    LoadLibraryW(L"StartDocked.dll");
    HANDLE hStartDocked = GetModuleHandle(L"StartDocked.dll");
    if (dwVal1 && dwVal1 != 0xFFFFFFFF)
    {
        StartDocked_LauncherFrame_ShowAllAppsFunc = (INT64(*)(void*))
            ((uintptr_t)hStartDocked + dwVal1);
    }
    if (dwVal2 && dwVal2 != 0xFFFFFFFF)
    {
        StartDocked_LauncherFrame_OnVisibilityChangedFunc = (INT64(*)(void*, INT64, void*))
            ((uintptr_t)hStartDocked + dwVal2);
        rv = funchook_prepare(
            funchook,
            (void**)&StartDocked_LauncherFrame_OnVisibilityChangedFunc,
            StartDocked_LauncherFrame_OnVisibilityChangedHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
    }
    if (dwVal3 && dwVal3 != 0xFFFFFFFF)
    {
        StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsFunc = (INT64(*)(void*, INT64, void*))
            ((uintptr_t)hStartDocked + dwVal3);
        rv = funchook_prepare(
            funchook,
            (void**)&StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsFunc,
            StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
    }
    if (dwVal4 && dwVal4 != 0xFFFFFFFF)
    {
        /*StartDocked_StartSizingFrame_StartSizingFrameFunc = (INT64(*)(void*, INT64, void*))
            ((uintptr_t)hStartDocked + dwVal4);
        rv = funchook_prepare(
            funchook,
            (void**)&StartDocked_StartSizingFrame_StartSizingFrameFunc,
            StartDocked_StartSizingFrame_StartSizingFrameHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }*/
    }

    rv = funchook_install(funchook, 0);
    if (rv != 0)
    {
        FreeLibraryAndExitThread(hModule, rv);
        return rv;
    }
#endif
}

void InjectShellExperienceHost()
{
#ifdef _WIN64
    HKEY hKey;
    if (RegOpenKeyW(HKEY_CURRENT_USER, _T(SEH_REGPATH), &hKey) != ERROR_SUCCESS)
    {
        return;
    }
    RegCloseKey(hKey);
    HMODULE hQA = LoadLibraryW(L"Windows.UI.QuickActions.dll");
    if (hQA)
    {
        PIMAGE_DOS_HEADER dosHeader = hQA;
        if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE)
        {
            PIMAGE_NT_HEADERS64 ntHeader = (PIMAGE_NT_HEADERS64)((u_char*)dosHeader + dosHeader->e_lfanew);
            if (ntHeader->Signature == IMAGE_NT_SIGNATURE)
            {
                char* pSEHPatchArea = NULL;
                char seh_pattern1[14] =
                {
                    // mov al, 1
                    0xB0, 0x01,
                    // jmp + 2
                    0xEB, 0x02,
                    // xor al, al
                    0x32, 0xC0,
                    // add rsp, 0x20
                    0x48, 0x83, 0xC4, 0x20,
                    // pop rdi
                    0x5F,
                    // pop rsi
                    0x5E,
                    // pop rbx
                    0x5B,
                    // ret
                    0xC3
                };
                char seh_off = 12;
                char seh_pattern2[5] =
                {
                    // mov r8b, 3
                    0x41, 0xB0, 0x03,
                    // mov dl, 1
                    0xB2, 0x01
                };
                BOOL bTwice = FALSE;
                PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeader);
                for (unsigned int i = 0; i < ntHeader->FileHeader.NumberOfSections; ++i)
                {
                    if (section->Characteristics & IMAGE_SCN_CNT_CODE)
                    {
                        if (section->SizeOfRawData && !bTwice)
                        {
                            DWORD dwOldProtect;
                            //VirtualProtect(hQA + section->VirtualAddress, section->SizeOfRawData, PAGE_EXECUTE_READWRITE, &dwOldProtect);
                            char* pCandidate = NULL;
                            while (TRUE)
                            {
                                pCandidate = memmem(
                                    !pCandidate ? hQA + section->VirtualAddress : pCandidate,
                                    !pCandidate ? section->SizeOfRawData : (uintptr_t)section->SizeOfRawData - (uintptr_t)(pCandidate - (hQA + section->VirtualAddress)),
                                    seh_pattern1,
                                    sizeof(seh_pattern1)
                                );
                                if (!pCandidate)
                                {
                                    break;
                                }
                                char* pCandidate2 = pCandidate - seh_off - sizeof(seh_pattern2);
                                if (pCandidate2 > section->VirtualAddress)
                                {
                                    if (memmem(pCandidate2, sizeof(seh_pattern2), seh_pattern2, sizeof(seh_pattern2)))
                                    {
                                        if (!pSEHPatchArea)
                                        {
                                            pSEHPatchArea = pCandidate;
                                        }
                                        else
                                        {
                                            bTwice = TRUE;
                                        }
                                    }
                                }
                                pCandidate += sizeof(seh_pattern1);
                            }
                            //VirtualProtect(hQA + section->VirtualAddress, section->SizeOfRawData, dwOldProtect, &dwOldProtect);
                        }
                    }
                    section++;
                }
                if (pSEHPatchArea && !bTwice)
                {
                    DWORD dwOldProtect;
                    VirtualProtect(pSEHPatchArea, sizeof(seh_pattern1), PAGE_EXECUTE_READWRITE, &dwOldProtect);
                    pSEHPatchArea[2] = 0x90;
                    pSEHPatchArea[3] = 0x90;
                    VirtualProtect(pSEHPatchArea, sizeof(seh_pattern1), dwOldProtect, &dwOldProtect);
                }
            }
        }
    }
#endif
}

#define DLL_INJECTION_METHOD_DXGI 0
#define DLL_INJECTION_METHOD_COM 1
#define DLL_INJECTION_METHOD_START_INJECTION 2
HRESULT EntryPoint(DWORD dwMethod)
{
    if (bInstanced)
    {
        return E_NOINTERFACE;
    }

    TCHAR exePath[MAX_PATH], dllName[MAX_PATH];
    GetModuleFileNameW(hModule, dllName, MAX_PATH);
    PathStripPathW(dllName);
    BOOL bIsDllNameDXGI = !_wcsicmp(dllName, L"dxgi.dll");
    if (dwMethod == DLL_INJECTION_METHOD_DXGI && !bIsDllNameDXGI)
    {
        return E_NOINTERFACE;
    }

    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION,
        FALSE,
        GetCurrentProcessId()
    );
    if (!hProcess)
    {
        return E_NOINTERFACE;
    }
    DWORD dwLength = MAX_PATH;
    QueryFullProcessImageNameW(
        hProcess,
        0,
        exePath,
        &dwLength
    );
    CloseHandle(hProcess);

    TCHAR wszSearchIndexerPath[MAX_PATH];
    GetSystemDirectoryW(wszSearchIndexerPath, MAX_PATH);
    wcscat_s(wszSearchIndexerPath, MAX_PATH, L"\\SearchIndexer.exe");
    if (!_wcsicmp(exePath, wszSearchIndexerPath))
    {
        return E_NOINTERFACE;
    }

    TCHAR wszExplorerExpectedPath[MAX_PATH];
    GetWindowsDirectoryW(wszExplorerExpectedPath, MAX_PATH);
    wcscat_s(wszExplorerExpectedPath, MAX_PATH, L"\\explorer.exe");
    BOOL bIsThisExplorer = !_wcsicmp(exePath, wszExplorerExpectedPath);

    TCHAR wszStartExpectedPath[MAX_PATH];
    GetWindowsDirectoryW(wszStartExpectedPath, MAX_PATH);
    wcscat_s(wszStartExpectedPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\StartMenuExperienceHost.exe");
    BOOL bIsThisStartMEH = !_wcsicmp(exePath, wszStartExpectedPath);

    TCHAR wszShellExpectedPath[MAX_PATH];
    GetWindowsDirectoryW(wszShellExpectedPath, MAX_PATH);
    wcscat_s(wszShellExpectedPath, MAX_PATH, L"\\SystemApps\\ShellExperienceHost_cw5n1h2txyewy\\ShellExperienceHost.exe");
    BOOL bIsThisShellEH = !_wcsicmp(exePath, wszShellExpectedPath);

    if (dwMethod == DLL_INJECTION_METHOD_DXGI)
    {
        if (!(bIsThisExplorer || bIsThisStartMEH || bIsThisShellEH))
        {
            return E_NOINTERFACE;
        }
        TCHAR wszRealDXGIPath[MAX_PATH];
        GetSystemDirectoryW(wszRealDXGIPath, MAX_PATH);
        wcscat_s(wszRealDXGIPath, MAX_PATH, L"\\dxgi.dll");
#ifdef _WIN64
        SetupDXGIImportFunctions(LoadLibraryW(wszRealDXGIPath));
#endif
    }
    if (dwMethod == DLL_INJECTION_METHOD_COM && (bIsThisExplorer || bIsThisStartMEH || bIsThisShellEH))
    {
        return E_NOINTERFACE;
    }
    if (dwMethod == DLL_INJECTION_METHOD_START_INJECTION && !bIsThisStartMEH)
    {
        return E_NOINTERFACE;
    }

    bIsExplorerProcess = bIsThisExplorer;
    if (bIsThisExplorer)
    {
        Inject(!IsDesktopWindowAlreadyPresent());
        IncrementDLLReferenceCount(hModule);
        bInstanced = TRUE;
    }
    else if (bIsThisStartMEH)
    {
        InjectStartMenu();
        IncrementDLLReferenceCount(hModule);
        bInstanced = TRUE;
    }
    else if (bIsThisShellEH)
    {
        InjectShellExperienceHost();
        IncrementDLLReferenceCount(hModule);
        bInstanced = TRUE;
    }
    else if (dwMethod == DLL_INJECTION_METHOD_COM)
    {
        Inject(FALSE);
        IncrementDLLReferenceCount(hModule);
        bInstanced = TRUE;
    }

    return E_NOINTERFACE;
}

#ifdef _WIN64
// for explorer.exe and ShellExperienceHost.exe
__declspec(dllexport) HRESULT DXGIDeclareAdapterRemovalSupport()
{
    EntryPoint(DLL_INJECTION_METHOD_DXGI);
    return DXGIDeclareAdapterRemovalSupportFunc();
}
// for StartMenuExperienceHost.exe via DXGI
__declspec(dllexport) HRESULT CreateDXGIFactory1(void* p1, void** p2)
{
    EntryPoint(DLL_INJECTION_METHOD_DXGI);
    return CreateDXGIFactory1Func(p1, p2);
}
// for StartMenuExperienceHost.exe via injection from explorer
HRESULT InjectStartFromExplorer()
{
    EntryPoint(DLL_INJECTION_METHOD_START_INJECTION);
    return HRESULT_FROM_WIN32(GetLastError());
}
#pragma comment(linker, "/export:DllGetClassObject=_DllGetClassObject")
#else
#pragma comment(linker, "/export:DllGetClassObject=__DllGetClassObject@12")
#endif
// for everything else
HRESULT WINAPI _DllGetClassObject(
    REFCLSID rclsid,
    REFIID   riid,
    LPVOID* ppv
)
{
    return EntryPoint(DLL_INJECTION_METHOD_COM);
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

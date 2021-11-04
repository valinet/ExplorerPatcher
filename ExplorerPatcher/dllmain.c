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
#ifdef _WIN64
#include <valinet/pdb/pdb.h>
#endif
#if defined(DEBUG) | defined(_DEBUG)
#define _LIBVALINET_DEBUG_HOOKING_IATPATCH
#endif
#include <valinet/hooking/iatpatch.h>

#define EP_CLSID "{D17F1E1A-5919-4427-8F89-A1A8503CA3EB}"

#define WINX_ADJUST_X 5
#define WINX_ADJUST_Y 5

#define CHECKFOREGROUNDELAPSED_TIMEOUT 100
#define POPUPMENU_SAFETOREMOVE_TIMEOUT 300
#define POPUPMENU_BLUETOOTH_TIMEOUT 700
#define POPUPMENU_PNIDUI_TIMEOUT 300
#define POPUPMENU_SNDVOLSSO_TIMEOUT 300
#define POPUPMENU_EX_ELAPSED 300

BOOL bIsExplorerProcess = FALSE;
BOOL bInstanced = FALSE;
HWND archivehWnd;
HMODULE hStartIsBack64 = 0;
DWORD bOldTaskbar = TRUE;
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
DWORD bHookStartMenu = TRUE;
DWORD bNoMenuAccelerator = FALSE;
DWORD bTaskbarMonitorOverride = 0;
HMODULE hModule = NULL;
HANDLE hSettingsMonitorThread = NULL;
HANDLE hDelayedInjectionThread = NULL;
HANDLE hIsWinXShown = NULL;
HANDLE hWinXThread = NULL;
HANDLE hExitSettingsMonitor = NULL;
HANDLE hSwsSettingsChanged = NULL;
HANDLE hSwsOpacityMaybeChanged = NULL;
BYTE* lpShouldDisplayCCButton = NULL;
int Code = 0;


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
Setting* settings = NULL;
SettingsChangeParameters* settingsParams = NULL;

HRESULT WINAPI _DllRegisterServer();
HRESULT WINAPI _DllUnregisterServer();
HRESULT WINAPI _DllCanUnloadNow();
HRESULT WINAPI _DllGetClassObject(
    REFCLSID rclsid,
    REFIID   riid,
    LPVOID* ppv
);

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

long long elapsedCheckForeground = 0;
HANDLE hCheckForegroundThread = NULL;
DWORD CheckForegroundThread(wchar_t* wszClassName)
{
    printf("Started \"Check foreground window\" thread.\n");
    UINT i = 0;
    while (TRUE)
    {
        wchar_t text[200];
        GetClassNameW(GetForegroundWindow(), text, 200);
        if (!wcscmp(text, wszClassName))
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
        GetClassNameW(GetForegroundWindow(), text, 200);
        if (wcscmp(text, wszClassName))
        {
            break;
        }
        Sleep(100);
    }
    elapsedCheckForeground = milliseconds_now();
    printf("Ended \"Check foreground window\" thread.\n");
    return 0;
}

void LaunchNetworkTargets(DWORD dwTarget)
{
    // very helpful: https://www.tenforums.com/tutorials/3123-clsid-key-guid-shortcuts-list-windows-10-a.html
    if (!dwTarget)
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
    );
static INT64(*CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc)(
    void* _this,
    POINT* pt
    );
static void(*CLauncherTipContextMenu_ExecuteCommandFunc)(
    void* _this,
    int a2
    );
static void(*CLauncherTipContextMenu_ExecuteShutdownCommandFunc)(
    void* _this,
    void* a2
    );
static INT64(*ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)(
    HMENU h1,
    HMENU h2,
    HWND a3,
    unsigned int a4,
    void* data
    );
static void(*ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)(
    HMENU _this,
    HMENU hWnd,
    HWND a3
    );
static INT64(*CLauncherTipContextMenu_GetMenuItemsAsyncFunc)(
    void* _this,
    void* rect,
    void** iunk
    );
static INT64(*CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc)(
    HWND hWnd,
    int a2,
    HWND a3,
    int a4,
    BOOL* a5
    );

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
        if (p == L' ')
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
    InsertMenuItemW(
        *((HMENU*)((char*)params->_this + 0xe8)),
        GetMenuItemCount(*((HMENU*)((char*)params->_this + 0xe8))) - 1,
        TRUE,
        &menuInfo
    );

    INT64* unknown_array = NULL;
    if (bSkinMenus)
    {
        unknown_array = calloc(4, sizeof(INT64));
        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
            *((HMENU*)((char*)params->_this + 0xe8)),
            hWinXWnd,
            &(params->point),
            0xc,
            unknown_array
        );
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
        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
            *((HMENU*)((char*)params->_this + 0xe8)),
            hWinXWnd,
            &(params->point)
        );
        free(unknown_array);
    }

    RemoveMenu(
        *((HMENU*)((char*)params->_this + 0xe8)),
        3999,
        MF_BYCOMMAND
    );

    if (res > 0)
    {
        if (res == 3999)
        {
            //CreateThread(0, 0, ZZGUI, 0, 0, 0);
            wchar_t wszPath[MAX_PATH * 2];
            ZeroMemory(
                wszPath,
                (MAX_PATH * 2) * sizeof(wchar_t)
            );
            wszPath[0] = '\"';
            GetSystemDirectoryW(
                wszPath + 1,
                MAX_PATH
            );
            wcscat_s(
                wszPath,
                MAX_PATH * 2,
                L"\\rundll32.exe\" \""
            );
            GetModuleFileNameW(
                hModule,
                wszPath + wcslen(wszPath),
                MAX_PATH
            );
            wcscat_s(
                wszPath,
                MAX_PATH * 2,
                L"\",ZZGUI"
            );
            wprintf(L"Launching : %s\n", wszPath);
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
        else if (res < 4000)
        {
            INT64 info = *(INT64*)((char*)(*(INT64*)((char*)params->_this + 0xa8 - 0x58)) + (INT64)res * 8 - 8);
            CLauncherTipContextMenu_ExecuteCommandFunc(
                (char*)params->_this - 0x58,
                &info
            );
        }
        else
        {
            INT64 info = *(INT64*)((char*)(*(INT64*)((char*)params->_this + 0xc8 - 0x58)) + ((INT64)res - 4000) * 8);
            CLauncherTipContextMenu_ExecuteShutdownCommandFunc(
                (char*)params->_this - 0x58,
                &info
            );
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
        POINT dPt = GetDefaultWinXPosition(FALSE, &bBottom, &bRight, FALSE);
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
        point = GetDefaultWinXPosition(FALSE, NULL, NULL, TRUE);
    }

    IUnknown* iunk;
    INT64 r = CLauncherTipContextMenu_GetMenuItemsAsyncFunc(
        _this,
        &point,
        &iunk
    );
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

finalize:
    return CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc(_this, pt);
}
#endif
#pragma endregion


#pragma region "Shell_TrayWnd subclass"
#ifdef _WIN64
INT64 Shell_TrayWndSubclassProc(
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
        RemoveWindowSubclass(hWnd, Shell_TrayWndSubclassProc, Shell_TrayWndSubclassProc);
    }
    else if (uMsg == WM_HOTKEY && wParam == 500 && lParam == MAKELPARAM(MOD_WIN, 0x41))
    {
        if (lpShouldDisplayCCButton)
        {
            *lpShouldDisplayCCButton = 1;
        }
        LRESULT lRes = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        if (lpShouldDisplayCCButton)
        {
            *lpShouldDisplayCCButton = bHideControlCenterButton;
        }
        return lRes;
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
BOOL ShowLegacyClockExpierience(HWND hWnd)
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
                return ShowLegacyClockExpierience(hWnd);
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
    GetClassNameW(hWnd, wszClassName, 200);

    BOOL bIsTaskbar = (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd")) ? !bSkinMenus : bDisableImmersiveContextMenu;
    //wprintf(L">> %s %d %d\n", wszClassName, bIsTaskbar, bIsExplorerProcess);

    if (bIsTaskbar && (bIsExplorerProcess ? 1 : (!wcscmp(wszClassName, L"SHELLDLL_DefView") || !wcscmp(wszClassName, L"SysTreeView32"))))
    {
        EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
        if (IsImmersiveMenu)
        {
            IsImmersiveMenu = FALSE;

            if (bIsExplorerProcess)
            {
#ifdef _WIN64
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

            return bRet;
        }
        IsImmersiveMenu = FALSE;
    }
    return TrackPopupMenuEx(
        hMenu,
        uFlags,
        x,
        y,
        hWnd,
        lptpm
    );
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
    GetClassNameW(hWnd, wszClassName, 200);

    BOOL bIsTaskbar = (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd")) ? !bSkinMenus : bDisableImmersiveContextMenu;
    //wprintf(L">> %s %d %d\n", wszClassName, bIsTaskbar, bIsExplorerProcess);

    if (bIsTaskbar && (bIsExplorerProcess ? 1 : (!wcscmp(wszClassName, L"SHELLDLL_DefView") || !wcscmp(wszClassName, L"SysTreeView32"))))
    {
        EnumPropsA(hWnd, CheckIfImmersiveContextMenu);
        if (IsImmersiveMenu)
        {
            IsImmersiveMenu = FALSE;

            if (bIsExplorerProcess)
            {
#ifdef _WIN64
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

            return bRet;
        }
        IsImmersiveMenu = FALSE;
    }
    return TrackPopupMenu(
        hMenu,
        uFlags,
        x,
        y,
        0,
        hWnd,
        prcRect
    );
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
    if (elapsed > POPUPMENU_EX_ELAPSED || !bFlyoutMenus)
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
                POINT pt;
                pt.x = x;
                pt.y = y;
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
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
        explorer_TrackPopupMenuExElapsed = milliseconds_now();
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
                POINT pt;
                pt.x = x;
                pt.y = y;
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
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
                POINT pt;
                pt.x = x;
                pt.y = y;
                ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                    hMenu,
                    hWnd,
                    &(pt)
                );
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
            ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                hMenu,
                hWnd,
                &(pt),
                0xc,
                unknown_array
            );
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
            ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                hMenu,
                hWnd,
                &(pt)
            );
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
            ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                hMenu,
                hWnd,
                &(pt),
                0xc,
                unknown_array
            );
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
            ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                hMenu,
                hWnd,
                &(pt)
            );
            free(unknown_array);
        }
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
        if (bMicaEffectOnTitlebar)
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


#pragma region "Show WiFi networks on network icon click"
#ifdef _WIN64
DEFINE_GUID(GUID_c2f03a33_21f5_47fa_b4bb_156362a2f239,
    0xc2f03a33,
    0x21f5, 0x47fa, 0xb4, 0xbb,
    0x15, 0x63, 0x62, 0xa2, 0xf2, 0x39
);
DEFINE_GUID(GUID_6d5140c1_7436_11ce_8034_00aa006009fa,
    0x6d5140c1,
    0x7436, 0x11ce, 0x80, 0x34,
    0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa
);
HRESULT pnidui_CoCreateInstanceHook(
    REFCLSID  rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD     dwClsContext,
    REFIID    riid,
    LPVOID* ppv
)
{
    DWORD dwVal = 0, dwSize = sizeof(DWORD);
    if (IsEqualGUID(rclsid, &GUID_c2f03a33_21f5_47fa_b4bb_156362a2f239) && 
        IsEqualGUID(riid, &GUID_6d5140c1_7436_11ce_8034_00aa006009fa) &&
        SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Settings\\Network"),
            TEXT("ReplaceVan"),
            SRRF_RT_REG_DWORD,
            NULL,
            &dwVal,
            (LPDWORD)(&dwSize)
        ) == ERROR_SUCCESS)
    {
        if (dwVal != 0)
        {
            LaunchNetworkTargets(dwVal);
            return E_NOINTERFACE;
        }
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
                L"Windows.UI.Core.CoreWindow",
                0, 
                0
            );
        }
        return E_NOINTERFACE;
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


#pragma region "Show Clock flyout on Win+C"
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
INT64 winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageHook(
    void* _this,
    INT64 a2,
    INT a3
)
{
    if (!bClockFlyoutOnWinC)
    {
        return winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc(_this, a2, a3);
    }
    if (a2 == 786 && a3 == 107)
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
                    return ShowLegacyClockExpierience(FindWindowExW(FindWindowExW(hShellTray_Wnd, NULL, L"TrayNotifyWnd", NULL), NULL, L"TrayClockWClass", NULL));
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
            hWnd = FindWindowExW(hWnd, NULL, L"ClockButton", NULL);
            if (hWnd)
            {
                if (ShouldShowLegacyClockExperience())
                {
                    if (!FindWindowW(L"ClockFlyoutWindow", NULL))
                    {
                        return ShowLegacyClockExpierience(hWnd);
                    }
                    else
                    {
                        return PostMessageW(FindWindowW(L"ClockFlyoutWindow", NULL), WM_CLOSE, 0, 0);
                    }
                }
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
        }
    }
    return 0;
}
#endif
#pragma endregion


#pragma region "Show Start in correct location according to TaskbarAl"
#ifdef _WIN64
DEFINE_GUID(GUID_18C02F2E_2754_5A20_8BD5_0B34CE79DA2B,
    0x18C02F2E,
    0x2754, 0x5A20, 0x8b, 0xd5,
    0x0b, 0x34, 0xce, 0x79, 0xda, 0x2b
);

DEFINE_GUID(_uuidof_WindowsUdk_UI_Shell_TaskbarLayout_Factory,
    0x4472FE8B,
    0xF3B1, 0x5CC9, 0x81, 0xc1,
    0x76, 0xf8, 0xc3, 0x38, 0x8a, 0xab
);
DEFINE_GUID(_uuidof_v13,
    0x4FB10D7C4,
    0x4F7F, 0x5DE5, 0xA5, 0x28,
    0x7e, 0xfe, 0xf4, 0x18, 0xaa, 0x48
);
BOOL PositionStartMenuForMonitor(
    HMONITOR hMonitor, 
    HDC unused1,
    LPRECT unused2,
    DWORD location)
{
    HRESULT hr = S_OK;
    HSTRING_HEADER hstringHeader;
    HSTRING string = NULL;
    void* factory = NULL;
    INT64 v12 = NULL;
    INT64* v13 = NULL;

    if (SUCCEEDED(hr))
    {
        hr = RoInitialize(RO_INIT_MULTITHREADED);
    }
    if (SUCCEEDED(hr))
    {
        hr = WindowsCreateStringReference(
            L"WindowsUdk.UI.Shell.TaskbarLayout",
            33,
            &hstringHeader,
            &string
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = RoGetActivationFactory(
            string,
            &_uuidof_WindowsUdk_UI_Shell_TaskbarLayout_Factory,
            &factory
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = (*(HRESULT(**)(INT64, INT64*))(*(INT64*)factory + 48))(factory, &v12);
    }
    if (SUCCEEDED(hr))
    {
        hr = (**(HRESULT(***)(INT64, GUID*, INT64*))v12)(v12, &_uuidof_v13, (INT64*)&v13); // QueryInterface
    }
    if (SUCCEEDED(hr))
    {
        void** p = malloc(41 * sizeof(void*));
        for (unsigned int i = 0; i < 41; ++i)
        {
            if (i == 1 || i == 2)
            {
                p[i] = nimpl3;
            }
            else if (i == 6 || i == 10)
            {
                if (location)
                {
                    p[i] = nimpl4_1;
                }
                else
                {
                    p[i] = nimpl4_0;
                }
            }
            else if (i == 14)
            {
                p[i] = nimpl4_1;
            }
            else
            {
                p[i] = nimpl;
            }
        }
        hr = (*(HRESULT(**)(INT64, HMONITOR, INT64(***)(), INT64))(*v13 + 48))(v13, hMonitor, &p, 0);
        free(p);
    }
    if (SUCCEEDED(hr))
    {
        (*(void(**)(INT64*))(*v13 + 16))(v13); // Release
        (*(void(**)(INT64))(*(INT64*)v12 + 16))(v12); // Release
        (*(void(**)(INT64))(*(INT64*)factory + 16))(factory); // Release
        WindowsDeleteString(string);
        RoUninitialize();
    }
    return TRUE;
}

void PositionStartMenu(INT64 unused, DWORD location)
{
    HWND hWnd = NULL;

    do
    {
        hWnd = FindWindowEx(
            NULL,
            hWnd,
            L"Shell_SecondaryTrayWnd",
            NULL
        );
        PositionStartMenuForMonitor(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), NULL, NULL, location);
    } while (hWnd);
    if (!hWnd)
    {
        hWnd = FindWindowEx(
            NULL,
            NULL,
            L"Shell_TrayWnd",
            NULL
        );
        PositionStartMenuForMonitor(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), NULL, NULL, location);
    }
}

DWORD PositionStartMenuTimeout(INT64 timeout)
{
    Sleep(timeout);
    printf("Started \"Position Start menu\" thread.\n");
    EnumDisplayMonitors(NULL, NULL, PositionStartMenuForMonitor, GetStartMenuPosition());
    printf("Ended \"Position Start menu\" thread.\n");
}

DWORD GetStartMenuPosition()
{
    DWORD dwSize = sizeof(DWORD);

    DWORD dwTaskbarAl = 0;
    if (!SHRegGetValueFromHKCUHKLMFunc || SHRegGetValueFromHKCUHKLMFunc(
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
        TEXT("TaskbarAl"),
        SRRF_RT_REG_DWORD,
        NULL,
        &dwTaskbarAl,
        (LPDWORD)(&dwSize)
    ) != ERROR_SUCCESS)
    {
        dwTaskbarAl = 0;
    }

    return dwTaskbarAl;
}

INT64 PositionStartMenuOnMonitorTopologyChangeSubclass(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    UINT_PTR    uIdSubclass,
    DWORD_PTR   dwRefData
)
{
    if (uMsg == WM_DISPLAYCHANGE)
    {
        CreateThread(0, 0, PositionStartMenuTimeout, 1000, 0, 0);
    }
    return DefSubclassProc(
        hWnd,
        uMsg,
        wParam,
        lParam
    );
}
#endif
#pragma endregion


#pragma region "Enable old taskbar"
#ifdef _WIN64
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
                                POINT pt = GetDefaultWinXPosition(FALSE, NULL, NULL, TRUE);
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


#pragma region "Set up taskbar button hooks"
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

void stub1(void* i)
{
}

static BOOL(*SetChildWindowNoActivateFunc)(HWND);
BOOL explorer_SetChildWindowNoActivateHook(HWND hWnd)
{
    TCHAR className[100];
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
            /*else if (!wcscmp(wszComponentName, L"PeopleButton"))
            {
                DWORD dwOldProtect;
                VirtualProtect(Instance + 160, sizeof(uintptr_t), PAGE_READWRITE, &dwOldProtect);
                *(uintptr_t*)(Instance + 160) = ToggleMainClockFlyout;    // OnClick
                VirtualProtect(Instance + 160, sizeof(uintptr_t), dwOldProtect, &dwOldProtect);
            }*/
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
                    /*
                    SetWindowSubclass(
                        hWnd,
                        PositionStartMenuOnMonitorTopologyChangeSubclass,
                        PositionStartMenuOnMonitorTopologyChangeSubclass,
                        0
                    );
                    SettingsChangeParameters* params = calloc(1, sizeof(SettingsChangeParameters));
                    params->isStartMenuExperienceHost = FALSE;
                    params->TaskbarAlChangedCallback = PositionStartMenu;
                    params->TaskbarAlChangedCallbackData = 0;
                    CreateThread(
                        0,
                        0,
                        MonitorSettingsChanges,
                        params,
                        0,
                        0
                    );
                    */
                    EnumDisplayMonitors(NULL, NULL, PositionStartMenuForMonitor, GetStartMenuPosition());
                    /*printf("hook show desktop\n");
                    void* ShellTrayWndProcFuncT = GetWindowLongPtrW(hWnd, GWLP_WNDPROC);
                    if (ShellTrayWndProcHook != ShellTrayWndProcFuncT)
                    {
                        ShellTrayWndProcFunc = ShellTrayWndProcFuncT;
                        SetWindowLongPtrW(hWnd, GWLP_WNDPROC, ShellTrayWndProcHook);
                    }*/
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

    HANDLE hEvent = CreateEvent(0, 0, 0, L"ShellDesktopSwitchEvent");
    if (hEvent)
    {
        printf(">>> Signal shell ready.\n");
        SetEvent(hEvent);
    }

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
            if (sws)
            {
                sws_WindowSwitcher_RefreshTheme(sws);
            }
        }
        RegCloseKey(hKey);
    }
}

DWORD WindowSwitcher(DWORD unused)
{
    while (TRUE)
    {
        sws_ReadSettings(NULL);
        if (sws_IsEnabled)
        {
            sws_error_t err;
            sws_WindowSwitcher* sws = NULL;
            err = sws_error_Report(sws_error_GetFromInternalError(sws_WindowSwitcher_Initialize(&sws, FALSE)), NULL);
            sws_ReadSettings(sws);
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
void WINAPI LoadSettings(BOOL bIsExplorer)
{
    HKEY hKey = NULL;
    DWORD dwSize = 0;

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH),
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
            TEXT("AllocConsole"),
            0,
            NULL,
            &bAllocConsole,
            &dwSize
        );
        DWORD bMemcheck = FALSE;
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("Memcheck"),
            0,
            NULL,
            &bMemcheck,
            &dwSize
        );
        if (bMemcheck)
        {
#if defined(DEBUG) | defined(_DEBUG)
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
            _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
            _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
            _CrtDumpMemoryLeaks();
#endif
            bMemcheck = FALSE;
            RegSetValueExW(
                hKey,
                TEXT("Memcheck"),
                0,
                REG_DWORD,
                &bMemcheck,
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
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("ClassicThemeMitigations"),
            0,
            NULL,
            &bClassicThemeMitigations,
            &dwSize
        );
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
        if (!bIsExplorer)
        {
            RegCloseKey(hKey);
            return;
        }
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("OldTaskbar"),
            0,
            NULL,
            &bOldTaskbar,
            &dwSize
        );
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
            TEXT("NoMenuAccelerator"),
            0,
            NULL,
            &bNoMenuAccelerator,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("TaskbarMonitorOverride"),
            0,
            NULL,
            &bTaskbarMonitorOverride,
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
        GetClassNameW(GetAncestor(hWndParent, GA_ROOT), wszClassName, 200);
        if (!wcscmp(wszClassName, L"CabinetWClass"))
        {
            dwExStyle |= WS_EX_CLIENTEDGE;
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
        SetWindowSubclass(hWnd, Shell_TrayWndSubclassProc, Shell_TrayWndSubclassProc, 0);
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

HRESULT explorer_DrawThemeBackground(
    HTHEME  hTheme,
    HDC     hdc,
    int     iPartId,
    int     iStateId,
    LPCRECT pRect,
    LPCRECT pClipRect
)
{
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

            HGDIOBJ hOldFont = SelectObject(hdc, hFont);
            DWORD dwTextFlags = DT_SINGLELINE | DT_CENTER;
            DrawTextW(
                hdc,
                L"\u2026",
                -1, 
                pRect,
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

INT64 explorer_SetWindowCompositionAttribute(HWND hWnd, WINCOMPATTRDATA* data)
{
    if (bClassicThemeMitigations)
    {
        return TRUE;
    }
    return SetWindowCompositionAttribute(hWnd, data);
}

HTHEME explorer_OpenThemeDataForDpi(
    HWND    hwnd,
    LPCWSTR pszClassList,
    UINT    dpi
)
{
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


HINSTANCE explorer_ShellExecuteW(
    HWND    hwnd,
    LPCWSTR lpOperation,
    LPCWSTR lpFile,
    LPCWSTR lpParameters,
    LPCWSTR lpDirectory,
    INT     nShowCmd
)
{
    if (!wcscmp(lpFile, L"ms-settings:notifications"))
    {
        return ShellExecuteW(
            hwnd, lpOperation,
            L"shell:::{05d7b0f4-2121-4eff-bf6b-ed3f69b894d9}",
            lpParameters, lpDirectory, nShowCmd
        );
    }
    else if (!wcscmp(lpFile, L"ms-settings:dateandtime"))
    {
        return ShellExecuteW(
            hwnd, lpOperation,
            L"shell:::{E2E7934B-DCE5-43C4-9576-7FE4F75E7480}",
            lpParameters, lpDirectory, nShowCmd
        );
    }
    return ShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
}


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
        }
    }
    else
    {
        VnPatchIAT(hShell32, "user32.dll", "TrackPopupMenu", TrackPopupMenu);
        VnPatchIAT(hShell32, "user32.dll", "SystemParametersInfoW", SystemParametersInfoW);
        if (!bIsExplorer)
        {
            VnPatchIAT(hShell32, "user32.dll", "CreateWindowExW", CreateWindowExW);
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
        }
    }
    else
    {
        VnPatchIAT(hExplorerFrame, "user32.dll", "TrackPopupMenu", TrackPopupMenu);
        VnPatchIAT(hExplorerFrame, "user32.dll", "SystemParametersInfoW", SystemParametersInfoW);
        VnPatchIAT(hExplorerFrame, "shcore.dll", (LPCSTR)188, explorerframe_SHCreateWorkerWindowFunc);
        if (!bIsExplorer)
        {
            VnPatchIAT(hExplorerFrame, "user32.dll", "CreateWindowExW", CreateWindowExW);
        }
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
        }
    }
    else
    {
        VnPatchDelayIAT(hWindowsUIFileExplorer, "user32.dll", "TrackPopupMenu", TrackPopupMenu);
        VnPatchDelayIAT(hWindowsUIFileExplorer, "user32.dll", "SystemParametersInfoW", SystemParametersInfoW);
        if (!bIsExplorer)
        {
            VnPatchIAT(hWindowsUIFileExplorer, "user32.dll", "CreateWindowExW", CreateWindowExW);
        }
        FreeLibrary(hWindowsUIFileExplorer);
        FreeLibrary(hWindowsUIFileExplorer);
    }

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

BOOL CALLBACK GetMonitorByIndex(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, RECT* rc)
{
    //printf(">> %d %d %d %d\n", lprcMonitor->left, lprcMonitor->top, lprcMonitor->right, lprcMonitor->bottom);
    if (--rc->left < 0)
    {
        *rc = *lprcMonitor;
        return FALSE;
    }
    return TRUE;
}

HMONITOR explorer_MonitorFromRect(LPCRECT lprc, DWORD dwFlags)
{
    /*printf("%d %d %d %d\n", lprc->left, lprc->top, lprc->right, lprc->bottom);

        return MonitorFromRect(lprc, dwFlags);
    //}*/
    if (bTaskbarMonitorOverride)
    {
        RECT rc;
        ZeroMemory(&rc, sizeof(RECT));
        rc.left = bTaskbarMonitorOverride - 1;
        EnumDisplayMonitors(
            NULL,
            NULL,
            GetMonitorByIndex,
            &rc
        );
        if (rc.top != rc.bottom)
        {
            return MonitorFromRect(&rc, dwFlags);
        }
    }
    return MonitorFromRect(lprc, dwFlags);
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

__declspec(dllexport) DWORD WINAPI main(
    _In_ LPVOID bIsExplorer
)
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

    LoadSettings(bIsExplorer);

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

    settings = calloc(9, sizeof(Setting));
    settings[0].callback = LoadSettings;
    settings[0].data = bIsExplorer;
    settings[0].hEvent = NULL;
    settings[0].hKey = NULL;
    wcscpy_s(settings[0].name, MAX_PATH, TEXT(REGPATH));
    settings[0].origin = HKEY_CURRENT_USER;

    settings[1].callback = LoadSettings;
    settings[1].data = bIsExplorer;
    settings[1].hEvent = NULL;
    settings[1].hKey = NULL;
    wcscpy_s(settings[1].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage");
    settings[1].origin = HKEY_CURRENT_USER;

    settings[2].callback = SetEvent;
    settings[2].data = hSwsSettingsChanged;
    settings[2].hEvent = NULL;
    settings[2].hKey = NULL;
    wcscpy_s(settings[2].name, MAX_PATH, TEXT(REGPATH) L"\\sws");
    settings[2].origin = HKEY_CURRENT_USER;

    settings[3].callback = SetEvent;
    settings[3].data = hSwsOpacityMaybeChanged;
    settings[3].hEvent = NULL;
    settings[3].hKey = NULL;
    wcscpy_s(settings[3].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MultitaskingView\\AltTabViewHost");
    settings[3].origin = HKEY_CURRENT_USER;

    settings[4].callback = Explorer_RefreshUI;
    settings[4].data = NULL;
    settings[4].hEvent = NULL;
    settings[4].hKey = NULL;
    wcscpy_s(settings[4].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced");
    settings[4].origin = HKEY_CURRENT_USER;

    settings[5].callback = Explorer_RefreshUI;
    settings[5].data = NULL;
    settings[5].hEvent = NULL;
    settings[5].hKey = NULL;
    wcscpy_s(settings[5].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Search");
    settings[5].origin = HKEY_CURRENT_USER;

    settings[6].callback = Explorer_RefreshUI;
    settings[6].data = NULL;
    settings[6].hEvent = NULL;
    settings[6].hKey = NULL;
    wcscpy_s(settings[6].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\People");
    settings[6].origin = HKEY_CURRENT_USER;

    settings[7].callback = Explorer_RefreshUI;
    settings[7].data = NULL;
    settings[7].hEvent = NULL;
    settings[7].hKey = NULL;
    wcscpy_s(settings[7].name, MAX_PATH, L"SOFTWARE\\Microsoft\\TabletTip\\1.7");
    settings[7].origin = HKEY_CURRENT_USER;

    settings[8].callback = SetEvent;
    settings[8].data = hSwsSettingsChanged;
    settings[8].hEvent = NULL;
    settings[8].hKey = NULL;
    wcscpy_s(settings[8].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer");
    settings[8].origin = HKEY_CURRENT_USER;

    settingsParams = calloc(1, sizeof(SettingsChangeParameters));
    settingsParams->settings = settings;
    settingsParams->size = bIsExplorer ? 9 : 1;
    hExitSettingsMonitor = CreateEventW(NULL, FALSE, FALSE, NULL);
    settingsParams->hExitEvent = hExitSettingsMonitor;
    if (!hSettingsMonitorThread)
    {
        hSettingsMonitorThread = CreateThread(
            0,
            0,
            MonitorSettings,
            settingsParams,
            0,
            0
        );
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

#ifdef _WIN64
    if (bIsExplorer)
    {
        CreateThread(
            0,
            0,
            WindowSwitcher,
            0,
            0,
            0
        );
    }


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
        printf("Symbols have to be (re)downloaded...\n");
        DownloadSymbolsParams* params = malloc(sizeof(DownloadSymbolsParams));
        params->hModule = hModule;
        CreateThread(0, 0, DownloadSymbols, params, 0, 0);
        return 0;
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
    VnPatchIAT(hExplorer, "user32.dll", (LPCSTR)2005, explorer_SetChildWindowNoActivateHook);
    VnPatchDelayIAT(hExplorer, "ext-ms-win-rtcore-ntuser-window-ext-l1-1-0.dll", "SendMessageW", explorer_SendMessageW);
    if (bOldTaskbar)
    {
        VnPatchIAT(hExplorer, "api-ms-win-core-libraryloader-l1-2-0.dll", "GetProcAddress", explorer_GetProcAddressHook);
        VnPatchIAT(hExplorer, "shell32.dll", "ShellExecuteW", explorer_ShellExecuteW);
        VnPatchIAT(hExplorer, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegGetValueW", explorer_RegGetValueW);
        VnPatchIAT(hExplorer, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegSetValueExW", explorer_RegSetValueExW);
        VnPatchIAT(hExplorer, "user32.dll", "MonitorFromRect", explorer_MonitorFromRect);
    }
    VnPatchIAT(hExplorer, "user32.dll", "TrackPopupMenuEx", explorer_TrackPopupMenuExHook);
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
        VnPatchIAT(hExplorer, "uxtheme.dll", "OpenThemeDataForDpi", explorer_OpenThemeDataForDpi);
        VnPatchIAT(hExplorer, "uxtheme.dll", "DrawThemeBackground", explorer_DrawThemeBackground);
        VnPatchIAT(hExplorer, "user32.dll", "SetWindowCompositionAttribute", explorer_SetWindowCompositionAttribute);
    }
    //VnPatchDelayIAT(hExplorer, "ext-ms-win-rtcore-ntuser-window-ext-l1-1-0.dll", "CreateWindowExW", explorer_CreateWindowExW);


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




    HANDLE hUxtheme = LoadLibraryW(L"uxtheme.dll");
    SetPreferredAppMode = GetProcAddress(hUxtheme, (LPCSTR)0x87);
    AllowDarkModeForWindow = GetProcAddress(hUxtheme, (LPCSTR)0x85);
    ShouldAppsUseDarkMode = GetProcAddress(hUxtheme, (LPCSTR)0x84);
    GetThemeName = GetProcAddress(hUxtheme, (LPCSTR)0x4A);
    printf("Setup uxtheme functions done\n");


    HANDLE hTwinuiPcshell = LoadLibraryW(L"twinui.pcshell.dll");

    if (symbols_PTRS.twinui_pcshell_PTRS[0] != 0xFFFFFFFF)
    {
        CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc = (INT64(*)(HWND, int, HWND, int, BOOL*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[0]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[1] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_GetMenuItemsAsyncFunc = (INT64(*)(void*, void*, void**))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[1]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[2] != 0xFFFFFFFF)
    {
        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc = (INT64(*)(HMENU, HMENU, HWND, unsigned int, void*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[2]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[3] != 0xFFFFFFFF)
    {
        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc = (void(*)(HMENU, HMENU, HWND))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[3]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[4] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_ExecuteShutdownCommandFunc = (void(*)(void*, void*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[4]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[5] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_ExecuteCommandFunc = (void(*)(void*, int))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[5]);
    }

    if (symbols_PTRS.twinui_pcshell_PTRS[6] != 0xFFFFFFFF)
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

    if (symbols_PTRS.twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT - 1] != 0xFFFFFFFF)
    {
        if (symbols_PTRS.twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT - 1])
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
        }
    }
    printf("Setup twinui.pcshell functions done\n");


    HANDLE hStobject = LoadLibraryW(L"stobject.dll");
    VnPatchDelayIAT(hStobject, "user32.dll", "TrackPopupMenu", stobject_TrackPopupMenuHook);
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


    CreateThread(
        0,
        0,
        PlayStartupSound,
        0,
        0,
        0
    );
    printf("Play startup sound thread...\n");


    CreateThread(
        0,
        0,
        SignalShellReady,
        dwExplorerReadyDelay,
        0,
        0
    );
    printf("Signal shell ready...\n");


    CreateThread(
        0,
        0,
        OpenStartOnCurentMonitorThread,
        0,
        0,
        0
    );
    printf("Open Start on monitor thread\n");


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


    if (bHookStartMenu)
    {
        HookStartMenuParams* params2 = calloc(1, sizeof(HookStartMenuParams));
        params2->dwTimeout = 1000;
        params2->hModule = hModule;
        params2->proc = _DllGetClassObject;
        GetModuleFileNameW(hModule, params2->wszModulePath, MAX_PATH);
        CreateThread(0, 0, HookStartMenu, params2, 0, 0);
    }

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
        TEXT(REGPATH),
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

static INT64(*StartDocked_LauncherFrame_OnVisibilityChangedFunc)(void*, INT64, void*);

static INT64(*StartDocked_LauncherFrame_ShowAllAppsFunc)(void* _this);

INT64 StartDocked_LauncherFrame_OnVisibilityChangedHook(void* _this, INT64 a2, void* VisibilityChangedEventArguments)
{
    INT64 r = StartDocked_LauncherFrame_OnVisibilityChangedFunc(_this, a2, VisibilityChangedEventArguments);
    if (StartMenu_ShowAllApps)
    {
        //if (VisibilityChangedEventArguments_GetVisible(VisibilityChangedEventArguments))
        {
            StartDocked_LauncherFrame_ShowAllAppsFunc(_this);
        }
    }
    return r;
}

INT64(*StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsFunc)(void*);

INT64 StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsHook(void* _this)
{
    return StartMenu_maximumFreqApps;
}

INT64(*StartDocked_StartSizingFrame_StartSizingFrameFunc)(void* _this);

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

    ZZRestartExplorer();

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

    ZZRestartExplorer();

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

#ifdef _WIN64
#pragma comment(linker, "/export:DllGetClassObject=_DllGetClassObject")
#else
#pragma comment(linker, "/export:DllGetClassObject=__DllGetClassObject@12")
#endif
HRESULT WINAPI _DllGetClassObject(
    REFCLSID rclsid,
    REFIID   riid,
    LPVOID* ppv
)
{
    if (bInstanced)
    {
        return E_NOINTERFACE;
    }
    TCHAR exeName[MAX_PATH + 1];
    GetProcessImageFileNameW(
        OpenProcess(
            PROCESS_QUERY_INFORMATION,
            FALSE,
            GetCurrentProcessId()
        ),
        exeName,
        MAX_PATH
    );
    PathStripPathW(exeName);
    TCHAR wszSystemPath[MAX_PATH + 1];
    GetSystemDirectory(wszSystemPath, MAX_PATH + 1);
    wcscat_s(wszSystemPath, MAX_PATH + 1, L"\\dxgi.dll");
    /*HMODULE hModule = LoadLibraryW(wszSystemPath);
    SetupDXGIImportFunctions(hModule);*/
    if (!wcscmp(exeName, L"explorer.exe") && FileExistsW(wszSystemPath))
    {
        bInstanced = TRUE;
        return E_NOINTERFACE;
    }
    bIsExplorerProcess = !wcscmp(exeName, L"explorer.exe");
    if (!wcscmp(exeName, L"explorer.exe"))
    {
        main(!IsDesktopWindowAlreadyPresent());
    }
    else if (!wcscmp(exeName, L"StartMenuExperienceHost.exe"))
    {
#ifdef _WIN64
        funchook = funchook_create();

        StartMenu_LoadSettings(FALSE);

        Setting* settings = calloc(2, sizeof(Setting));
        settings[0].callback = StartMenu_LoadSettings;
        settings[0].data = FALSE;
        settings[0].hEvent = NULL;
        settings[0].hKey = NULL;
        wcscpy_s(settings[0].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage");
        settings[0].origin = HKEY_CURRENT_USER;
        settings[1].callback = StartMenu_LoadSettings;
        settings[1].data = TRUE;
        settings[1].hEvent = NULL;
        settings[1].hKey = NULL;
        wcscpy_s(settings[1].name, MAX_PATH, TEXT(REGPATH));
        settings[1].origin = HKEY_CURRENT_USER;

        SettingsChangeParameters* params = calloc(1, sizeof(SettingsChangeParameters));
        params->settings = settings;
        params->size = 2;
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
                    TEXT(REGPATH) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                    TEXT(STARTDOCKED_SB_0),
                    SRRF_RT_REG_DWORD,
                    NULL,
                    &dwVal0,
                    (LPDWORD)(&dwSize)
                );
                SHRegGetValueFromHKCUHKLMFunc(
                    TEXT(REGPATH) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                    TEXT(STARTDOCKED_SB_1),
                    SRRF_RT_REG_DWORD,
                    NULL,
                    &dwVal1,
                    (LPDWORD)(&dwSize)
                );
                SHRegGetValueFromHKCUHKLMFunc(
                    TEXT(REGPATH) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                    TEXT(STARTDOCKED_SB_2),
                    SRRF_RT_REG_DWORD,
                    NULL,
                    &dwVal2,
                    (LPDWORD)(&dwSize)
                );
                SHRegGetValueFromHKCUHKLMFunc(
                    TEXT(REGPATH) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
                    TEXT(STARTDOCKED_SB_3),
                    SRRF_RT_REG_DWORD,
                    NULL,
                    &dwVal3,
                    (LPDWORD)(&dwSize)
                );
                SHRegGetValueFromHKCUHKLMFunc(
                    TEXT(REGPATH) TEXT("\\") TEXT(STARTDOCKED_SB_NAME),
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
        if (dwVal1 != 0xFFFFFFFF)
        {
            StartDocked_LauncherFrame_ShowAllAppsFunc = (INT64(*)(void*))
                ((uintptr_t)hStartDocked + dwVal1);
        }
        if (dwVal2 != 0xFFFFFFFF)
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
        if (dwVal3 != 0xFFFFFFFF)
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
        if (dwVal4 != 0xFFFFFFFF)
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
    else if (!wcscmp(exeName, L"regsvr32.exe"))
    {
    }
    else
    {
        main(FALSE);
    }
    bInstanced = TRUE;
    return E_NOINTERFACE;
}
#ifdef _WIN64
__declspec(dllexport) HRESULT DXGIDeclareAdapterRemovalSupport()
{
    TCHAR exeName[MAX_PATH], dllName[MAX_PATH];
    GetProcessImageFileNameW(
        OpenProcess(
            PROCESS_QUERY_INFORMATION,
            FALSE,
            GetCurrentProcessId()
        ),
        exeName,
        MAX_PATH
    );
    PathStripPathW(exeName);
    GetModuleFileNameW(hModule, dllName, MAX_PATH);
    PathStripPathW(dllName);
    TCHAR wszSystemPath[MAX_PATH];
    GetSystemDirectory(wszSystemPath, MAX_PATH);
    wcscat_s(wszSystemPath, MAX_PATH, L"\\dxgi.dll");
    HMODULE hModule = LoadLibraryW(wszSystemPath);
    SetupDXGIImportFunctions(hModule);
    bIsExplorerProcess = !wcscmp(exeName, L"explorer.exe");
    if (!wcscmp(exeName, L"explorer.exe") && !wcscmp(dllName, L"dxgi.dll"))
    {
        // CreateEventW(NULL, FALSE, FALSE, L"ExplorerPatcher_Guard_{D17F1E1A-5919-4427-8F89-A1A8503CA3EB}") && GetLastError() != ERROR_ALREADY_EXISTS
        main(!IsDesktopWindowAlreadyPresent()); //wcsstr(GetCommandLineW(), L"NoUACCheck") // !IsDesktopWindowAlreadyPresent()
        bInstanced = TRUE;
    }
    return DXGIDeclareAdapterRemovalSupportFunc();
}
#endif

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

#include "hooking.h"
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
#include <valinet/pdb/pdb.h>
#define _LIBVALINET_DEBUG_HOOKING_IATPATCH
#include <valinet/hooking/iatpatch.h>

#define WINX_ADJUST_X 5
#define WINX_ADJUST_Y 5

#define CHECKFOREGROUNDELAPSED_TIMEOUT 100
#define POPUPMENU_SAFETOREMOVE_TIMEOUT 300
#define POPUPMENU_BLUETOOTH_TIMEOUT 700
#define POPUPMENU_PNIDUI_TIMEOUT 300
#define POPUPMENU_SNDVOLSSO_TIMEOUT 300
#define POPUPMENU_EX_ELAPSED 300

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
HMODULE hModule = NULL;
HANDLE hIsWinXShown = NULL;
HANDLE hWinXThread = NULL;
HANDLE hSwsSettingsChanged = NULL;
HANDLE hSwsOpacityMaybeChanged = NULL;

#include "utility.h"
#ifdef USE_PRIVATE_INTERFACES
#include "ep_private.h"
#endif
#include "symbols.h"
#include "dxgi_imp.h"
#include "ArchiveMenu.h"
#include "StartupSound.h"
#include "SettingsMonitor.h"
#include "HideExplorerSearchBar.h"
#include "StartMenu.h"
#include "GUI.h"
#include "TaskbarCenter.h"
#include "../libs/sws/SimpleWindowSwitcher/sws_WindowSwitcher.h"

#pragma region "Generics"
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

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

static void(*SetPreferredAppMode)(INT64 bAllowDark);

static void(*AllowDarkModeForWindow)(HWND hWnd, INT64 bAllowDark);

static BOOL(*ShouldAppsUseDarkMode)();

static void(*GetThemeName)(void*, void*, void*);

static BOOL AppsShouldUseDarkMode() { return TRUE; }

long long milliseconds_now() {
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
        ptCursor.x = 0;
        ptCursor.y = 0;
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
#pragma endregion


#pragma region "twinui.pcshell.dll hooks"
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
    LoadStringW(GetModuleHandleW(L"ExplorerFrame.dll"), 50222, buffer + 1, 260);
    buffer[0] = L'&';
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
            GetWindowsDirectoryW(
                wszPath + wcslen(wszPath),
                MAX_PATH
            );
            wcscat_s(
                wszPath,
                MAX_PATH * 2,
                L"\\dxgi.dll\",ZZGUI"
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
        if (bBottom && IsThemeActive())
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
#pragma endregion


#pragma region "Popup menu hooks"
#define TB_POS_NOWHERE 0
#define TB_POS_BOTTOM 1
#define TB_POS_TOP 2
#define TB_POS_LEFT 3
#define TB_POS_RIGHT 4
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
BOOL shell32_TrackPopupMenuHook(
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
    if (bDisableImmersiveContextMenu)
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
    if (bSkinMenus)
    {
        //PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
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
    if (bCenterMenus)
    {
        //PopupMenuAdjustCoordinatesAndFlags(&x, &y, &uFlags);
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
long long TrackPopupMenuExElapsed = 0;
BOOL TrackPopupMenuExHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    long long elapsed = milliseconds_now() - TrackPopupMenuExElapsed;
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
        TrackPopupMenuExElapsed = milliseconds_now();
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
            // very helpful: https://www.tenforums.com/tutorials/3123-clsid-key-guid-shortcuts-list-windows-10-a.html
            if (bReplaceNetwork == 1)
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
            else if (bReplaceNetwork == 2)
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
#pragma endregion


#pragma region "Hide search bar in Explorer"
static HWND(*explorerframe_SHCreateWorkerWindowFunc)(
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
    HWND result = explorerframe_SHCreateWorkerWindowFunc(
        wndProc,
        hWndParent,
        dwExStyle,
        dwStyle,
        hMenu,
        wnd_extra
    );
    if (dwExStyle == 0x10000 && dwStyle == 1174405120)
    {
#ifdef USE_PRIVATE_INTERFACES
        if (bMicaEffectOnTitlebar)
        {
            BOOL value = TRUE;
            SetPropW(hWndParent, L"NavBarGlass", HANDLE_FLAG_INHERIT);
            DwmSetWindowAttribute(hWndParent, DWMWA_MICA_EFFFECT, &value, sizeof(BOOL));
            SetWindowSubclass(result, ExplorerMicaTitlebarSubclassProc, ExplorerMicaTitlebarSubclassProc, 0);
        }
#endif
        if (bHideExplorerSearchBar)
        {
            SetWindowSubclass(hWndParent, HideExplorerSearchBarSubClass, HideExplorerSearchBarSubClass, 0);
        }
    }
    return result;
}
#pragma endregion


#pragma region "Show WiFi networks on network icon click"
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
    if (IsEqualGUID(rclsid, &GUID_c2f03a33_21f5_47fa_b4bb_156362a2f239) && 
        IsEqualGUID(riid, &GUID_6d5140c1_7436_11ce_8034_00aa006009fa))
    {
        if (hCheckForegroundThread)
        {
            WaitForSingleObject(hCheckForegroundThread, INFINITE);
            CloseHandle(hCheckForegroundThread);
            hCheckForegroundThread = NULL;
        }
        if (milliseconds_now() - elapsedCheckForeground > CHECKFOREGROUNDELAPSED_TIMEOUT)
        {
            ShellExecute(
                NULL,
                L"open",
                L"ms-availablenetworks:",
                NULL,
                NULL,
                SW_SHOWNORMAL
            );
            hCheckForegroundThread = CreateThread(
                0, 
                0, 
                CheckForegroundThread,
                L"Windows.UI.Core.CoreWindow",
                0, 
                0
            );
        }
        /*PostMessageW(
            FindWindowEx(
                NULL,
                NULL,
                L"Shell_TrayWnd",
                NULL
            ), 
            WM_HOTKEY,
            500, 
            MAKELPARAM(MOD_WIN, 0x41)
        );*/
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
#pragma endregion


#pragma region "Show Clock flyout on Win+C"
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
#pragma endregion


#pragma region "Show Start in correct location according to TaskbarAl"
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
    HMODULE hShlwapi = LoadLibraryW(L"Shlwapi.dll");
    FARPROC SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(hShlwapi, "SHRegGetValueFromHKCUHKLM");
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

    if (hShlwapi)
    {
        FreeLibrary(hShlwapi);
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
#pragma endregion


#pragma region "Enable old taskbar"
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
#pragma endregion


#pragma region "Open power user menu on Win+X"
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
#pragma endregion


#pragma region "Hide Control Center button"
static BOOL(*SetChildWindowNoActivateFunc)(HWND);
BOOL explorer_SetChildWindowNoActivateHook(HWND hWnd)
{
    TCHAR className[100];
    GetClassNameW(hWnd, className, 100);
    if (!wcscmp(className, L"ControlCenterButton"))
    {
        BYTE* lpShouldDisplayCCButton = (BYTE*)(GetWindowLongPtrW(hWnd, 0) + 120);
        if (*lpShouldDisplayCCButton)
        {
            *lpShouldDisplayCCButton = !bHideControlCenterButton;
        }
    }
    return SetChildWindowNoActivateFunc(hWnd);
}
#pragma endregion


#pragma region "Hide Show desktop button"
LRESULT(*ShellTrayWndProcFunc)(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );
LRESULT ShellTrayWndProcHook(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg)
    {
    case WM_DESTROY:
    {
        break;
    }
    case WM_ERASEBKGND:
    {
        HWND v33 = FindWindowExW(hWnd, 0, L"TrayNotifyWnd", 0);
        HWND v34 = FindWindowExW(v33, 0, L"TrayShowDesktopButtonWClass", 0);
        if (v34)
        {
            /*BYTE* lpShouldDisplayCCButton = (BYTE*)(GetWindowLongPtrW(v34, 0) + 120);
            if (*lpShouldDisplayCCButton)
            {
                *lpShouldDisplayCCButton = FALSE;
            }*/

            //ShowWindow(v34, SW_HIDE);
        }
        break;
    }
    }
    return ShellTrayWndProcFunc(hWnd, uMsg, wParam, lParam);
}
#pragma endregion


#pragma region "Notify shell ready (fixes delay at logon)"
DWORD SignalShellReady(DWORD wait)
{
    printf("Started \"Signal shell ready\" thread.\n");

    while (!wait && TRUE)
    {
        HWND hWnd = FindWindowEx(
            NULL,
            NULL,
            L"Shell_TrayWnd",
            NULL
        );
        if (hWnd)
        {
            HWND shWnd = FindWindowEx(
                hWnd,
                NULL,
                L"Start",
                NULL
            );
            if (shWnd)
            {
                if (IsWindowVisible(shWnd))
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
#pragma endregion


#pragma region "Window Switcher"
DWORD sws_IsEnabled = FALSE;

void sws_ReadSettings(sws_WindowSwitcher* sws)
{
    HKEY hKey = NULL;
    DWORD dwSize = 0;

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
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("Enabled"),
            0,
            NULL,
            &sws_IsEnabled,
            &dwSize
        );
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
            sws_WindowSwitcher_RefreshTheme(sws);
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
#pragma endregion


#pragma region "Load Settings in Explorer"
void Explorer_LoadSettings(int unused)
{
    HKEY hKey = NULL;
    DWORD dwSize = 0;

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
        RegQueryValueExW(
            hKey,
            TEXT("AllocConsole"),
            0,
            NULL,
            &bAllocConsole,
            &dwSize
        );
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
            TEXT("HideExplorerSearchBar"),
            0,
            NULL,
            &bHideExplorerSearchBar,
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
            TEXT("SkinMenus"),
            0,
            NULL,
            &bSkinMenus,
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
            TEXT("ClassicThemeMitigations"),
            0,
            NULL,
            &bClassicThemeMitigations,
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
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MultitaskingView\\AltTabViewHost",
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
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
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
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Search",
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
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\People",
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
        RegCloseKey(hKey);
    }

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\TabletTip\\1.7",
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

void Explorer_RefreshUI(int unused)
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


#pragma region "Fix taskbar for classic theme"
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
    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
    SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
    NONCLIENTMETRICSW ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICSW);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0);
    HFONT hFont = CreateFontIndirectW(&(ncm.lfCaptionFont));
    HGDIOBJ hOldFont = SelectObject(hdc, hFont);
    DrawTextW(
        hdc,
        pszText,
        cchText, pRect,
        dwTextFlags
    );
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    SetBkColor(hdc, bc);
    SetTextColor(hdc, fc);
    return S_OK;
}
#pragma endregion


__declspec(dllexport) DWORD WINAPI main(
    _In_ LPVOID lpParameter
)
{
#ifdef DEBUG
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
    if (!funchook)
    {
        funchook = funchook_create();
        printf("funchook create %d\n", funchook != 0);

        Explorer_LoadSettings(0);

        hSwsSettingsChanged = CreateEventW(NULL, FALSE, FALSE, NULL);
        hSwsOpacityMaybeChanged = CreateEventW(NULL, FALSE, FALSE, NULL);

        Setting* settings = calloc(8, sizeof(Setting));
        settings[0].callback = Explorer_LoadSettings;
        settings[0].data = NULL;
        settings[0].hEvent = NULL;
        settings[0].hKey = NULL;
        wcscpy_s(settings[0].name, MAX_PATH, TEXT(REGPATH));
        settings[0].origin = HKEY_CURRENT_USER;

        settings[1].callback = Explorer_LoadSettings;
        settings[1].data = NULL;
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

        SettingsChangeParameters* params = calloc(1, sizeof(SettingsChangeParameters));
        params->settings = settings;
        params->size = 8;
        CreateThread(
            0,
            0,
            MonitorSettings,
            params,
            0,
            0
        );


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


        CreateThread(
            0,
            0,
            WindowSwitcher,
            0,
            0,
            0
        );


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


        HANDLE hExplorer = GetModuleHandleW(NULL);
        SetChildWindowNoActivateFunc = GetProcAddress(GetModuleHandleW(L"user32.dll"), (LPCSTR)2005);
        VnPatchIAT(hExplorer, "user32.dll", (LPCSTR)2005, explorer_SetChildWindowNoActivateHook);
        VnPatchDelayIAT(hExplorer, "ext-ms-win-rtcore-ntuser-window-ext-l1-1-0.dll", "SendMessageW", explorer_SendMessageW);
        if (bOldTaskbar)
        {
            VnPatchIAT(hExplorer, "api-ms-win-core-libraryloader-l1-2-0.dll", "GetProcAddress", explorer_GetProcAddressHook);
        }
        VnPatchIAT(hExplorer, "user32.dll", "TrackPopupMenuEx", TrackPopupMenuExHook);
        VnPatchIAT(hExplorer, "uxtheme.dll", "DrawThemeTextEx", explorer_DrawThemeTextEx);
        printf("Setup explorer functions done\n");


        HANDLE hUser32 = LoadLibraryW(L"user32.dll");
        CreateWindowInBand = GetProcAddress(hUser32, "CreateWindowInBand");
        GetWindowBand = GetProcAddress(hUser32, "GetWindowBand");
        SetWindowBand = GetProcAddress(hUser32, "SetWindowBand");
        printf("Setup user32 functions done\n");


        HANDLE hUxtheme = LoadLibraryW(L"uxtheme.dll");
        SetPreferredAppMode = GetProcAddress(hUxtheme, (LPCSTR)0x87);
        AllowDarkModeForWindow = GetProcAddress(hUxtheme, (LPCSTR)0x85);
        ShouldAppsUseDarkMode = GetProcAddress(hUxtheme, (LPCSTR)0x84);
        GetThemeName = GetProcAddress(hUxtheme, (LPCSTR)0x4A);
        printf("Setup uxtheme functions done\n");


        HANDLE hTwinuiPcshell = LoadLibraryW(L"twinui.pcshell.dll");

        CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc = (INT64(*)(HWND, int, HWND, int, BOOL*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[0]);

        CLauncherTipContextMenu_GetMenuItemsAsyncFunc = (INT64(*)(void*, void*, void**))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[1]);

        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc = (INT64(*)(HMENU, HMENU, HWND, unsigned int, void*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[2]);

        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc = (void(*)(HMENU, HMENU, HWND))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[3]);

        CLauncherTipContextMenu_ExecuteShutdownCommandFunc = (void(*)(void*, void*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[4]);

        CLauncherTipContextMenu_ExecuteCommandFunc = (void(*)(void*, int))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS.twinui_pcshell_PTRS[5]);

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
#ifdef USE_PRIVATE_INTERFACES
        if (bSkinIcons)
        {
            VnPatchIAT(hSndvolsso, "user32.dll", "LoadImageW", SystemTray_LoadImageWHook);
        }
#endif
        printf("Setup sndvolsso functions done\n");



        HANDLE hExplorerFrame = LoadLibraryW(L"ExplorerFrame.dll");
        explorerframe_SHCreateWorkerWindowFunc = GetProcAddress(LoadLibraryW(L"shcore.dll"), (LPCSTR)188);
        VnPatchIAT(hExplorerFrame, "shcore.dll", (LPCSTR)188, explorerframe_SHCreateWorkerWindowHook);
        VnPatchIAT(hExplorerFrame, "user32.dll", "TrackPopupMenu", TrackPopupMenuHook);
        printf("Setup ExplorerFrame functions done\n");



        HANDLE hShell32 = LoadLibraryW(L"shell32.dll");
        VnPatchIAT(hShell32, "user32.dll", "TrackPopupMenu", shell32_TrackPopupMenuHook);
        printf("Setup shell32 functions done\n");



        rv = funchook_install(funchook, 0);
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
        printf("Installed hooks.\n");


        HANDLE hEvent = CreateEventEx(
            0,
            L"ShellDesktopSwitchEvent",
            CREATE_EVENT_MANUAL_RESET,
            EVENT_ALL_ACCESS
        );
        ResetEvent(hEvent);
        printf("Created ShellDesktopSwitchEvent event.\n");


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


        HookStartMenuParams* params2 = calloc(1, sizeof(HookStartMenuParams));
        params2->dwTimeout = 1000;
        params2->hModule = hModule;
        GetModuleFileNameW(hModule, params2->wszModulePath, MAX_PATH);
        CreateThread(0, 0, HookStartMenu, params2, 0, 0);


        // This notifies applications when the taskbar has recomputed its layout
        if (SUCCEEDED(TaskbarCenter_Initialize(hExplorer)))
        {
            printf("Initialized taskbar update notification.\n");
        }
        else
        {
            printf("Failed to register taskbar update notification.\n");
        }




        //CreateThread(0, 0, PositionStartMenuTimeout, 0, 0, 0);
    }
    else
    {
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

        FreeLibraryAndExitThread(hModule, 0);
    }

    return 0;
}

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

void StartMenu_LoadSettings(int unused)
{
    HKEY hKey = NULL;
    DWORD dwSize;

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
        RegQueryValueExW(
            hKey,
            TEXT("Start_MaximumFrequentApps"),
            0,
            NULL,
            &StartMenu_maximumFreqApps,
            &dwSize
        );
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
        PathStripPath(exeName);
        TCHAR wszSystemPath[MAX_PATH + 1];
        GetSystemDirectory(wszSystemPath, MAX_PATH + 1);
        wcscat_s(wszSystemPath, MAX_PATH + 1, L"\\dxgi.dll");
        HMODULE hModule = LoadLibraryW(wszSystemPath);
#pragma warning(disable : 6387)
        SetupDXGIImportFunctions(hModule);
        if (!wcscmp(exeName, L"explorer.exe"))
        {
            main(0);
        }
        else if (!wcscmp(exeName, L"StartMenuExperienceHost.exe"))
        {
            funchook = funchook_create();

            StartMenu_LoadSettings(0);

            Setting* settings = calloc(2, sizeof(Setting));
            settings[0].callback = StartMenu_LoadSettings;
            settings[0].data = 0;
            settings[0].hEvent = NULL;
            settings[0].hKey = NULL;
            wcscpy_s(settings[0].name, MAX_PATH, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage");
            settings[0].origin = HKEY_CURRENT_USER;
            settings[1].callback = exit;
            settings[1].data = 0;
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
            StartDocked_LauncherFrame_ShowAllAppsFunc = (INT64(*)(void*))
                ((uintptr_t)hStartDocked + dwVal1);
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

            rv = funchook_install(funchook, 0);
            if (rv != 0)
            {
#pragma warning(disable : 6387)
                FreeLibraryAndExitThread(hModule, rv);
#pragma warning(default : 6387)
                return rv;
            }
        }
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

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
#include "utility.h"
#include "symbols.h"
#include "dxgi_imp.h"
#include "ArchiveMenu.h"
#include "StartupSound.h"
#include "SettingsMonitor.h"
#include "HideExplorerSearchBar.h"
#include "StartMenu.h"
#include "GUI.h"
#include "TaskbarCenter.h"

#define WINX_ADJUST_X 5
#define WINX_ADJUST_Y 5

#define SB_MICA_EFFECT_SUBCLASS_OFFSET 0x5BFC // 0x5C70
#define SB_INIT1 0x20054 // 0x26070
#define SB_INIT2 0x83A4 // Enable dark mode fixes
#define SB_TRACKPOPUPMENU_HOOK 0x1C774 // 0x21420
#define SB_TRACKPOPUPMENUEX_HOOK 0x1CB18 // 0x21920
#define SB_LOADIMAGEW_HOOK 0x3BEB0 // 0x4A6F0

HWND archivehWnd;
HMODULE hStartIsBack64 = 0;
BOOL bHideExplorerSearchBar = FALSE;
BOOL bMicaEffectOnTitlebar = FALSE;
BOOL bHideControlCenterButton = FALSE;
BOOL bSkinMenus = TRUE;
BOOL bSkinIcons = TRUE;
HMODULE hModule = NULL;
HANDLE hIsWinXShown = NULL;


#pragma region "Generics"
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

static void(*SetPreferredAppMode)(INT64 bAllowDark);

static void(*AllowDarkModeForWindow)(HWND hWnd, INT64 bAllowDark);

static BOOL(*ShouldAppsUseDarkMode)();

static void(*GetThemeName)(void*, void*, void*);

static BOOL AppsShouldUseDarkMode() { return TRUE; }
#pragma endregion


#pragma region "twinui.pcshell.dll hooks"
#define CLASS_NAME L"LauncherTipWnd"
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
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClass(&wc);

    hWinXWnd = CreateWindowInBand(
        0,
        CLASS_NAME,
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

    INT64* unknown_array = calloc(4, sizeof(INT64));
    ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
        *((HMENU*)((char*)params->_this + 0xe8)),
        hWinXWnd,
        &(params->point),
        0xc,
        unknown_array
    );

    BOOL res = TrackPopupMenu(
        *((HMENU*)((char*)params->_this + 0xe8)),
        TPM_RETURNCMD | TPM_RIGHTBUTTON | (params->bShouldCenterWinXHorizontally ? TPM_CENTERALIGN : 0),
        params->point.x,
        params->point.y,
        0,
        hWinXWnd,
        0
    );

    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
        *((HMENU*)((char*)params->_this + 0xe8)),
        hWinXWnd,
        &(params->point)
    );
    free(unknown_array);

    RemoveMenu(
        *((HMENU*)((char*)params->_this + 0xe8)),
        3999,
        MF_BYCOMMAND
    );

    if (res > 0)
    {
        if (res == 3999)
        {
            CreateThread(0, 0, ZZGUI, 0, 0, 0);
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
        if (bBottom)
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

finalize:
    return CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc(_this, pt);
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
#pragma endregion


#pragma region "Popup menu hooks"
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
    INT64* unknown_array = calloc(4, sizeof(INT64));
    POINT pt;
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
    BOOL b = TrackPopupMenu(
        hMenu,
        uFlags | TPM_RIGHTBUTTON,
        x,
        y,
        0,
        hWnd,
        prcRect
    );
    RemoveWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc);
    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
        hMenu,
        hWnd,
        &(pt)
    );
    free(unknown_array);
    return b;
}
BOOL TrackPopupMenuExHook(
    HMENU       hMenu,
    UINT        uFlags,
    int         x,
    int         y,
    HWND        hWnd,
    LPTPMPARAMS lptpm
)
{
    INT64* unknown_array = calloc(4, sizeof(INT64));
    POINT pt;
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
    BOOL b = TrackPopupMenuEx(
        hMenu,
        uFlags | TPM_RIGHTBUTTON,
        x,
        y,
        hWnd,
        lptpm
    );
    RemoveWindowSubclass(hWnd, OwnerDrawSubclassProc, OwnerDrawSubclassProc);
    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
        hMenu,
        hWnd,
        &(pt)
    );
    free(unknown_array);
    return b;
}
#pragma endregion


#pragma region "Mica effect for Explorer and remove search bar"
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
        if (hStartIsBack64 && bMicaEffectOnTitlebar)
        {
            BOOL value = TRUE;
            DwmSetWindowAttribute(hWndParent, DWMWA_MICA_EFFFECT, &value, sizeof(BOOL)); // Set Mica effect on title bar
            SetWindowSubclass(
                result, 
                (uintptr_t)hStartIsBack64 + SB_MICA_EFFECT_SUBCLASS_OFFSET, 
                (uintptr_t)hStartIsBack64 + SB_MICA_EFFECT_SUBCLASS_OFFSET, 
                0
            );
        }
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
        ShellExecute(
            NULL,
            L"open",
            L"ms-availablenetworks:",
            NULL,
            NULL,
            SW_SHOWNORMAL
        );
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

INT64 winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageHook(
    void* _this,
    INT64 a2,
    INT a3
)
{
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
        if (hWnd)
        {
            hWnd = FindWindowEx(hWnd, NULL, TEXT("TrayNotifyWnd"), NULL);
            if (hWnd)
            {
                hWnd = FindWindowEx(hWnd, NULL, TEXT("TrayClockWClass"), NULL);
            }
            if (!hWnd)
            {
                hWnd = FindWindowEx(prev_hWnd, NULL, TEXT("ClockButton"), NULL);
            }
            if (hWnd)
            {
                RECT rc;
                GetWindowRect(hWnd, &rc);
                HWND g_ProgWin = FindWindowEx(
                    NULL,
                    NULL,
                    L"Progman",
                    NULL
                );
                SetForegroundWindow(g_ProgWin);
                PostMessage(hWnd, WM_LBUTTONDOWN, 0, 0);
                PostMessage(hWnd, WM_LBUTTONUP, 0, 0);
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
void PositionStartMenuForMonitor(HMONITOR hMonitor, DWORD location)
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
        PositionStartMenuForMonitor(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), location);
    } while (hWnd);
    if (!hWnd)
    {
        hWnd = FindWindowEx(
            NULL,
            NULL,
            L"Shell_TrayWnd",
            NULL
        );
        PositionStartMenuForMonitor(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), location);
    }
}

DWORD PositionStartMenuTimeout(INT64 timeout)
{
    Sleep(timeout);
    PositionStartMenu(0, GetStartMenuPosition());
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
                                PostMessage(
                                    hWnd,
                                    WM_CONTEXTMENU,
                                    hWnd,
                                    MAKELPARAM(pt.x, pt.y)
                                );
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
            *lpShouldDisplayCCButton = FALSE;
        }
    }
    return SetChildWindowNoActivateFunc(hWnd);
}
#pragma endregion


#pragma region "Notify shell ready (fixes delay at logon)"
DWORD SignalShellReady(DWORD wait)
{
    if (wait)
    {
        Sleep(wait);
    }

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
                    PositionStartMenu(0, GetStartMenuPosition());
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

    HANDLE hEvent = CreateEvent(0, 0, 0, L"ShellDesktopSwitchEvent");
    if (hEvent)
    {
        SetEvent(hEvent);
    }

    printf("Ended \"Signal shell ready\" thread.\n");
    return 0;
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
        HKEY hKey;
        DWORD dwDisposition;
        DWORD dwSize = sizeof(DWORD);



        RegCreateKeyExW(
            HKEY_CURRENT_USER,
            TEXT(REGPATH),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ,
            NULL,
            &hKey,
            &dwDisposition
        );
        DWORD bAllocConsole = FALSE;
        RegQueryValueExW(
            hKey,
            TEXT("AllocConsole"),
            0,
            NULL,
            &bAllocConsole,
            &dwSize
        );
        printf("AllocConsole %d %d\n", bAllocConsole, hKey);
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
        HMODULE hSws = LoadLibraryW(L"SimpleWindowSwitcher.dll");
        if (hSws)
        {
            printf("Loaded Simple Window Switcher.\n");
        }

        symbols_addr symbols_PTRS;
        ZeroMemory(
            &symbols_PTRS,
            sizeof(symbols_addr)
        );

        if (LoadSymbols(&symbols_PTRS))
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


        RegQueryValueExW(
            hKey,
            TEXT("HideExplorerSearchBar"),
            0,
            NULL,
            &bHideExplorerSearchBar,
            &dwSize
        );
        RegQueryValueExW(
            hKey,
            TEXT("MicaEffectOnTitlebar"),
            0,
            NULL,
            &bMicaEffectOnTitlebar,
            &dwSize
        );
        RegQueryValueExW(
            hKey,
            TEXT("HideControlCenterButton"),
            0,
            NULL,
            &bHideControlCenterButton,
            &dwSize
        );
        RegQueryValueExW(
            hKey,
            TEXT("SkinMenus"),
            0,
            NULL,
            &bSkinMenus,
            &dwSize
        );
        RegQueryValueExW(
            hKey,
            TEXT("SkinIcons"),
            0,
            NULL,
            &bSkinIcons,
            &dwSize
        );
        


        TCHAR* wszSBPath = malloc((MAX_PATH + 1) * sizeof(TCHAR));
        if (!wszSBPath)
        {
            return 0;
        }
        ZeroMemory(
            wszSBPath,
            (MAX_PATH + 1) * sizeof(TCHAR)
        );
        SHGetFolderPathW(
            NULL,
            CSIDL_APPDATA,
            NULL,
            SHGFP_TYPE_CURRENT,
            wszSBPath
        );
        wcscat_s(
            wszSBPath,
            MAX_PATH,
            TEXT(APP_RELATIVE_PATH) L"\\StartAllBackX64.dll"
        );
        hStartIsBack64 = LoadLibraryW(wszSBPath);
        free(wszSBPath);
        if (hStartIsBack64)
        {
            ((void(*)())((uintptr_t)hStartIsBack64 + SB_INIT1))();

            ((void(*)())((uintptr_t)hStartIsBack64 + SB_INIT2))();

            printf("Loaded and initialized StartIsBack64 DLL\n");
        }


        
        HANDLE hExplorer = GetModuleHandle(NULL);
        SetChildWindowNoActivateFunc = GetProcAddress(GetModuleHandleW(L"user32.dll"), (LPCSTR)2005);
        if (bHideControlCenterButton)
        {
            VnPatchIAT(hExplorer, "user32.dll", (LPCSTR)2005, explorer_SetChildWindowNoActivateHook);
        }
        VnPatchDelayIAT(hExplorer, "ext-ms-win-rtcore-ntuser-window-ext-l1-1-0.dll", "SendMessageW", explorer_SendMessageW);
        VnPatchIAT(hExplorer, "api-ms-win-core-libraryloader-l1-2-0.dll", "GetProcAddress", explorer_GetProcAddressHook);
        printf("Setup explorer functions done\n");


        HANDLE hUser32 = LoadLibraryW(L"user32.dll");
        CreateWindowInBand = GetProcAddress(hUser32, "CreateWindowInBand");
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
        if (bSkinMenus)
        {
            if (1) // !hStartIsBack64
            {
                VnPatchDelayIAT(hStobject, "user32.dll", "TrackPopupMenu", TrackPopupMenuHook);
            }
            else
            {
                VnPatchDelayIAT(hStobject, "user32.dll", "TrackPopupMenu", (uintptr_t)hStartIsBack64 + SB_TRACKPOPUPMENU_HOOK);
            }
        }
        if (bSkinIcons && hStartIsBack64)
        {
            VnPatchDelayIAT(hStobject, "user32.dll", "LoadImageW", (uintptr_t)hStartIsBack64 + SB_LOADIMAGEW_HOOK);
        }
        printf("Setup stobject functions done\n");



        HANDLE hBthprops = LoadLibraryW(L"bthprops.cpl");
        if (bSkinMenus)
        {
            if (1) //!hStartIsBack64
            {
                VnPatchIAT(hBthprops, "user32.dll", "TrackPopupMenuEx", TrackPopupMenuExHook);
            }
            else
            {
                VnPatchIAT(hBthprops, "user32.dll", "TrackPopupMenuEx", (uintptr_t)hStartIsBack64 + SB_TRACKPOPUPMENUEX_HOOK);
            }
        }
        if (bSkinIcons && hStartIsBack64)
        {
            VnPatchIAT(hBthprops, "user32.dll", "LoadImageW", (uintptr_t)hStartIsBack64 + SB_LOADIMAGEW_HOOK);
        }
        printf("Setup bthprops functions done\n");



        HANDLE hPnidui = LoadLibraryW(L"pnidui.dll");
        VnPatchIAT(hPnidui, "api-ms-win-core-com-l1-1-0.dll", "CoCreateInstance", pnidui_CoCreateInstanceHook);
        if (bSkinIcons && hStartIsBack64)
        {
            VnPatchIAT(hPnidui, "user32.dll", "LoadImageW", (uintptr_t)hStartIsBack64 + SB_LOADIMAGEW_HOOK);
        }
        printf("Setup pnidui functions done\n");




        HANDLE hSndvolsso = LoadLibraryW(L"sndvolsso.dll");
        if (bSkinIcons && hStartIsBack64)
        {
            VnPatchIAT(hSndvolsso, "user32.dll", "LoadImageW", (uintptr_t)hStartIsBack64 + SB_LOADIMAGEW_HOOK);
        }
        printf("Setup sndvolsso functions done\n");


        
        HANDLE hExplorerFrame = LoadLibraryW(L"ExplorerFrame.dll");
        explorerframe_SHCreateWorkerWindowFunc = GetProcAddress(LoadLibraryW(L"shcore.dll"), (LPCSTR)188);
        VnPatchIAT(hExplorerFrame, "shcore.dll", (LPCSTR)188, explorerframe_SHCreateWorkerWindowHook);
        printf("Setup ExplorerFrame functions done\n");



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



        DWORD delay = 0;
        RegQueryValueExW(
            hKey,
            TEXT("ExplorerReadyDelay"),
            0,
            NULL,
            &delay,
            &dwSize
        );
        CreateThread(
            0,
            0,
            SignalShellReady,
            delay,
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





        DWORD bEnableArchivePlugin = 0;
        RegQueryValueExW(
            hKey,
            TEXT("ArchiveMenu"),
            0,
            NULL,
            &bEnableArchivePlugin,
            &dwSize
        );
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



        HookStartMenuParams* params = calloc(1, sizeof(HookStartMenuParams));
        params->dwTimeout = 1000;
        params->hModule = hModule;
        GetModuleFileNameW(hModule, params->wszModulePath, MAX_PATH);
        CreateThread(0, 0, HookStartMenu, params, 0, 0);



        // This notifies applications when the taskbar has recomputed its layout
        if (SUCCEEDED(TaskbarCenter_Initialize(hExplorer)))
        {
            printf("Initialized taskbar update notification.\n");
        }
        else
        {
            printf("Failed to register taskbar update notification.\n");
        }
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

static INT64(*StartDocked_LauncherFrame_OnVisibilityChangedFunc)(void*, INT64, void*);

static INT64(*StartDocked_LauncherFrame_ShowAllAppsFunc)(void* _this);

INT64 StartDocked_LauncherFrame_OnVisibilityChangedHook(void* _this, INT64 a2, void* VisibilityChangedEventArguments)
{
    INT64 r = StartDocked_LauncherFrame_OnVisibilityChangedFunc(_this, a2, VisibilityChangedEventArguments);
    HMODULE hModule = LoadLibraryW(L"Shlwapi.dll");
    if (hModule)
    {
        DWORD dwStatus = 0, dwSize = sizeof(DWORD);
        FARPROC SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(hModule, "SHRegGetValueFromHKCUHKLM");
        if (!SHRegGetValueFromHKCUHKLMFunc || SHRegGetValueFromHKCUHKLMFunc(
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage"),
            TEXT("MakeAllAppsDefault"),
            SRRF_RT_REG_DWORD,
            NULL,
            &dwStatus,
            (LPDWORD)(&dwSize)
        ) != ERROR_SUCCESS)
        {
            dwStatus = 0;
        }
        FreeLibrary(hModule);
        if (dwStatus)
        {
            StartDocked_LauncherFrame_ShowAllAppsFunc(_this);
        }
    }
    return r;
}

INT64 maximumFreqApps;

INT64(*StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsFunc)(void*);

INT64 StartDocked_SystemListPolicyProvider_GetMaximumFrequentAppsHook(void* _this)
{
    return maximumFreqApps;
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

            int rv;

            DWORD dwVal0 = 0x62254, dwVal1 = 0x188EBC, dwVal2 = 0x187120, dwVal3 = 0x3C10, dwVal4 = 0x160AEC;

            HMODULE hModule = LoadLibraryW(L"Shlwapi.dll");
            if (hModule)
            {
                DWORD dwStatus = 0, dwSize = sizeof(DWORD);
                FARPROC SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(hModule, "SHRegGetValueFromHKCUHKLM");
                if (!SHRegGetValueFromHKCUHKLMFunc || SHRegGetValueFromHKCUHKLMFunc(
                    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
                    TEXT("Start_MaximumFrequentApps"),
                    SRRF_RT_REG_DWORD,
                    NULL,
                    &dwStatus,
                    (LPDWORD)(&dwSize)
                ) != ERROR_SUCCESS)
                {
                    dwStatus = 6;
                }
                maximumFreqApps = dwStatus;

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

            SettingsChangeParameters* params = calloc(1, sizeof(SettingsChangeParameters));
            params->isStartMenuExperienceHost = TRUE;
            CreateThread(
                0,
                0,
                MonitorSettingsChanges,
                params,
                0,
                0
            );

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
            StartDocked_StartSizingFrame_StartSizingFrameFunc = (INT64(*)(void*, INT64, void*))
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
            }

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

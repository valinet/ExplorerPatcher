#include <initguid.h>
#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#include <funchook.h>
#pragma comment(lib, "Psapi.lib") // required by funchook
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <windowsx.h>
#include <Uxtheme.h>
#pragma comment(lib, "UxTheme.lib")
#include <valinet/ini/ini.h>
#include <valinet/pdb/pdb.h>
#define _LIBVALINET_INCLUDE_UNIVERSAL
#include <valinet/universal/toast/toast.h>
#include <Shlobj_core.h>
DEFINE_GUID(__uuidof_TaskbarList,
    0x56FDF344,
    0xFD6D, 0x11d0, 0x95, 0x8A,
    0x00, 0x60, 0x97, 0xC9, 0xA0, 0x90
);
DEFINE_GUID(__uuidof_ITaskbarList,
    0x56FDF342,
    0xFD6D, 0x11d0, 0x95, 0x8A,
    0x00, 0x60, 0x97, 0xC9, 0xA0, 0x90
);
DEFINE_GUID(__uuidof_AuthUILogonSound,
    0x0A0D018EE,
    0x1100, 0x4389, 0xAB, 0x44,
    0x46, 0x4F, 0xAF, 0x00, 0x12, 0x88
);
DEFINE_GUID(__uuidof_IAuthUILogonSound,
    0xc35243ea,
    0x4cfc, 0x435a, 0x91, 0xc2,
    0x9d, 0xbd, 0xec, 0xbf, 0xfc, 0x95
);
#define OPEN_NAME L"&Open archive"
#define EXTRACT_NAME L"&Extract to \"%s\\\""
#define OPEN_CMD L"\"C:\\Program Files\\7-Zip\\7zFM.exe\" %s"
#define EXTRACT_CMD L"\"C:\\Program Files\\7-Zip\\7zG.exe\" x -o\"%s\" -spe %s"

#define APPID L"Microsoft.Windows.Explorer"
#define SYMBOLS_RELATIVE_PATH "\\settings.ini"
#define EXPLORER_SB_NAME "explorer"
#define EXPLORER_SB_0 "CTray::_HandleGlobalHotkey"
#define EXPLORER_SB_1 "CTray::v_WndProc"
#define EXPLORER_SB_2 "CTray::_FireDesktopSwitchIfReady"
#define EXPLORER_SB_CNT 3
#define TWINUI_PCSHELL_SB_NAME "twinui.pcshell"
#define TWINUI_PCSHELL_SB_0 "CImmersiveContextMenuOwnerDrawHelper::s_ContextMenuWndProc"
#define TWINUI_PCSHELL_SB_1 "CLauncherTipContextMenu::GetMenuItemsAsync"
#define TWINUI_PCSHELL_SB_2 "ImmersiveContextMenuHelper::ApplyOwnerDrawToMenu"
#define TWINUI_PCSHELL_SB_3 "ImmersiveContextMenuHelper::RemoveOwnerDrawFromMenu"
#define TWINUI_PCSHELL_SB_4 "CLauncherTipContextMenu::_ExecuteShutdownCommand"
#define TWINUI_PCSHELL_SB_5 "CLauncherTipContextMenu::_ExecuteCommand"
#define TWINUI_PCSHELL_SB_6 "CLauncherTipContextMenu::ShowLauncherTipContextMenu"
#define TWINUI_PCSHELL_SB_CNT 7
#define TWINUI_SB_NAME "twinui"
#define TWINUI_SB_0 "CImmersiveHotkeyNotification::_GetMonitorForHotkeyNotification"
#define TWINUI_SB_1 "IsDesktopInputContext"
#define TWINUI_SB_2 "CImmersiveHotkeyNotification::OnMessage"
#define TWINUI_SB_CNT 3
#define STOBJECT_SB_NAME "stobject"
#define STOBJECT_SB_0 "SysTrayWndProc"
#define STOBJECT_SB_1 "HotPlugButtonClick"
#define STOBJECT_SB_CNT 2
const char* explorer_SN[EXPLORER_SB_CNT] = {
    EXPLORER_SB_0,
    EXPLORER_SB_1,
    EXPLORER_SB_2
};
const char* twinui_pcshell_SN[TWINUI_PCSHELL_SB_CNT] = {
    TWINUI_PCSHELL_SB_0,
    TWINUI_PCSHELL_SB_1,
    TWINUI_PCSHELL_SB_2,
    TWINUI_PCSHELL_SB_3,
    TWINUI_PCSHELL_SB_4,
    TWINUI_PCSHELL_SB_5,
    TWINUI_PCSHELL_SB_6
};
const char* twinui_SN[TWINUI_SB_CNT] = {
    TWINUI_SB_0,
    TWINUI_SB_1,
    TWINUI_SB_2
};
const char* stobject_SN[STOBJECT_SB_CNT] = {
    STOBJECT_SB_0,
    STOBJECT_SB_1
};
#pragma pack(push, 1)
typedef struct symbols_addr
{
    DWORD explorer_PTRS[EXPLORER_SB_CNT];
    DWORD twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT];
    DWORD twinui_PTRS[TWINUI_SB_CNT];
    DWORD stobject_PTRS[STOBJECT_SB_CNT];
} symbols_addr;
#pragma pack(pop)

wchar_t DownloadSymbolsXML[] =
L"<toast displayTimestamp=\"2021-08-29T00:00:00.000Z\" scenario=\"reminder\" "
L"activationType=\"protocol\" launch=\"https://github.com/valinet/ExplorerPatcher\" duration=\"short\">\r\n"
L"	<visual>\r\n"
L"		<binding template=\"ToastGeneric\">\r\n"
L"			<text><![CDATA[Unable to find symbols for OS version %s]]></text>\r\n"
L"			<text><![CDATA[Downloading and applying symbol information, please wait...]]></text>\r\n"
L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
L"		</binding>\r\n"
L"	</visual>\r\n"
L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
L"</toast>\r\n";

wchar_t DownloadOKXML[] =
L"<toast displayTimestamp=\"2021-08-29T01:00:00.000Z\" scenario=\"reminder\" "
L"activationType=\"protocol\" launch=\"https://github.com/valinet/ExplorerPatcher\" duration=\"long\">\r\n"
L"	<visual>\r\n"
L"		<binding template=\"ToastGeneric\">\r\n"
L"			<text><![CDATA[Symbols downloaded and applied successfully! You can now enjoy full application functionality.]]></text>\r\n"
L"			<text><![CDATA[This notification will not show again until the next OS build update.]]></text>\r\n"
L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
L"		</binding>\r\n"
L"	</visual>\r\n"
L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
L"</toast>\r\n";

#define DEBUG
#undef DEBUG

HWND archivehWnd;

funchook_t* funchook = NULL;
HMODULE hModule = NULL;
HWND messageWindow = NULL;
HANDLE hIsWinXShown = NULL;
INT64 lockEnsureWinXHotkeyOnlyOnce;

typedef LONG NTSTATUS, * PNTSTATUS;
#define STATUS_SUCCESS (0x00000000)

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

BOOL GetOSVersion(PRTL_OSVERSIONINFOW lpRovi)
{
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod != NULL)
    {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)GetProcAddress(
            hMod,
            "RtlGetVersion"
        );
        if (fxPtr != NULL)
        {
            lpRovi->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
            if (STATUS_SUCCESS == fxPtr(lpRovi))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
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

static INT64(*CTray_HandleGlobalHotkeyFunc)(
    void* _this,
    unsigned int a2,
    unsigned int a3
    );

static INT64(*CTray_v_WndProcFunc)(
    void* _this,
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    );

static INT64(*CTray__FireDesktopSwitchIfReadyFunc)(
    void* _this,
    unsigned int a2
    );

DEFINE_GUID(IID_ILauncherTipContextMenu,
    0xb8c1db5f,
    0xcbb3, 0x48bc, 0xaf, 0xd9,
    0xce, 0x6b, 0x88, 0x0c, 0x79, 0xed
);

#define CLASS_NAME L"LauncherTipWnd"



static HRESULT(*CImmersiveHotkeyNotification_OnMessageFunc)(
    void* _this,
    INT64 msg,
    INT wParam,
    INT64 lParam
    );

static INT64(*CImmersiveHotkeyNotification_GetMonitorForHotkeyNotificationFunc)(
    void* _this,
    void** a2,
    HWND* a3
    );

static BOOL(*IsDesktopInputContextFunc)(
    void* p1,
    void* p2
    );



static INT64(*SysTrayWndProcFunc)(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    );

static BOOL(*TrackPopupMenuFunc)(
    HMENU      hMenu,
    UINT       uFlags,
    int        x,
    int        y,
    int        nReserved,
    HWND       hWnd,
    const RECT* prcRect
);




DEFINE_GUID(CLSID_ImmersiveShell,
    0xc2f03a33,
    0x21f5, 0x47fa, 0xb4, 0xbb,
    0x15, 0x63, 0x62, 0xa2, 0xf2, 0x39
);

DEFINE_GUID(SID_IImmersiveMonitorService,
    0x47094e3a,
    0x0cf2, 0x430f, 0x80, 0x6f,
    0xcf, 0x9e, 0x4f, 0x0f, 0x12, 0xdd
);

DEFINE_GUID(IID_IImmersiveMonitorService,
    0x4d4c1e64,
    0xe410, 0x4faa, 0xba, 0xfa,
    0x59, 0xca, 0x06, 0x9b, 0xfe, 0xc2
);

DEFINE_GUID(SID_ImmersiveLauncher,
    0x6f86e01c,
    0xc649, 0x4d61, 0xbe, 0x23,
    0xf1, 0x32, 0x2d, 0xde, 0xca, 0x9d
);

DEFINE_GUID(IID_IImmersiveLauncher10RS,
    0xd8d60399,
    0xa0f1, 0xf987, 0x55, 0x51,
    0x32, 0x1f, 0xd1, 0xb4, 0x98, 0x64
);

typedef interface IImmersiveMonitorService IImmersiveMonitorService;

typedef struct IImmersiveMonitorServiceVtbl
{
    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            IImmersiveMonitorService* This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        IImmersiveMonitorService* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        IImmersiveMonitorService* This);

    HRESULT(STDMETHODCALLTYPE* method3)(
        IImmersiveMonitorService* This);

    HRESULT(STDMETHODCALLTYPE* method4)(
        IImmersiveMonitorService* This);

    HRESULT(STDMETHODCALLTYPE* method5)(
        IImmersiveMonitorService* This);

    HRESULT(STDMETHODCALLTYPE* GetFromHandle)(
        IImmersiveMonitorService* This,
        /* [in] */ HMONITOR hMonitor,
        _COM_Outptr_  IUnknown** ppvObject);

    END_INTERFACE
} IImmersiveMonitorServiceVtbl;

interface IImmersiveMonitorService
{
    CONST_VTBL struct IImmersiveMonitorServiceVtbl* lpVtbl;
};


typedef interface IImmersiveLauncher10RS IImmersiveLauncher10RS;

typedef struct IImmersiveLauncher10RSVtbl
{
    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            IImmersiveLauncher10RS* This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        IImmersiveLauncher10RS* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        IImmersiveLauncher10RS* This);

    HRESULT(STDMETHODCALLTYPE* ShowStartView)(
        IImmersiveLauncher10RS* This,
        /* [in] */ int method,
        /* [in] */ int flags);

    HRESULT(STDMETHODCALLTYPE* Dismiss)(
        IImmersiveLauncher10RS* This);

    HRESULT(STDMETHODCALLTYPE* method5)(
        IImmersiveLauncher10RS* This);

    HRESULT(STDMETHODCALLTYPE* method6)(
        IImmersiveLauncher10RS* This);

    HRESULT(STDMETHODCALLTYPE* IsVisible)(
        IImmersiveLauncher10RS* This,
        /* [in] */ BOOL* ret);

    HRESULT(STDMETHODCALLTYPE* method8)(
        IImmersiveLauncher10RS* This);

    HRESULT(STDMETHODCALLTYPE* method9)(
        IImmersiveLauncher10RS* This);

    HRESULT(STDMETHODCALLTYPE* ConnectToMonitor)(
        IImmersiveLauncher10RS* This,
        /* [in] */ IUnknown* monitor);

    HRESULT(STDMETHODCALLTYPE* GetMonitor)(
        IImmersiveLauncher10RS* This,
        /* [in] */ IUnknown** monitor);

    END_INTERFACE
} IImmersiveLauncher10RSVtbl;

interface IImmersiveLauncher10RS
{
    CONST_VTBL struct IImmersiveLauncher10RSVtbl* lpVtbl;
};






typedef interface AuthUILogonSound AuthUILogonSound;

typedef struct AuthUILogonSoundVtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        AuthUILogonSound* This,
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        AuthUILogonSound* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        AuthUILogonSound* This);

    HRESULT(STDMETHODCALLTYPE* PlayIfNecessary)(
        AuthUILogonSound* This,
        /* [in] */ INT64 a1);

    END_INTERFACE
} AuthUILogonSoundVtbl;

interface AuthUILogonSound
{
    CONST_VTBL struct AuthUILogonSoundVtbl* lpVtbl;
};






LRESULT CALLBACK CLauncherTipContextMenu_WndProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    LRESULT result;

    if (uMsg == WM_COPYDATA && hWnd == archivehWnd)
    {
        COPYDATASTRUCT* st = lParam;
        HWND srcWnd = wParam;

        POINT pt;
        GetCursorPos(&pt);

        SetForegroundWindow(hWnd);

        HMENU hMenu = CreatePopupMenu();

        TCHAR buffer[MAX_PATH + 100];
        TCHAR filename[MAX_PATH];
        ZeroMemory(filename, MAX_PATH * sizeof(TCHAR));
        memcpy(filename, st->lpData, wcslen(st->lpData) * sizeof(TCHAR));
        PathUnquoteSpacesW(filename);
        PathRemoveExtensionW(filename);
        PathStripPathW(filename);
        wsprintf(buffer, EXTRACT_NAME, filename);

        InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, 1, buffer);
        InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, 2, OPEN_NAME);

        INT64* unknown_array = calloc(4, sizeof(INT64));
        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
            hMenu,
            hWnd,
            &(pt),
            0xc,
            unknown_array
        );

        BOOL res = TrackPopupMenu(
            hMenu,
            TPM_RETURNCMD,
            pt.x - 15,
            pt.y - 15,
            0,
            hWnd,
            0
        );

        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
            hMenu,
            hWnd,
            &(pt)
        );
        free(unknown_array);

        if (res == 1 || res == 2)
        {
            ZeroMemory(buffer, (MAX_PATH + 100) * sizeof(TCHAR));
            if (res == 2)
            {
                wsprintf(buffer, OPEN_CMD, st->lpData);
                //wprintf(L"%s\n%s\n\n", st->lpData, buffer);
            }
            else if (res == 1)
            {
                TCHAR path[MAX_PATH], path_orig[MAX_PATH];
                ZeroMemory(path, MAX_PATH * sizeof(TCHAR));
                ZeroMemory(path_orig, MAX_PATH * sizeof(TCHAR));
                memcpy(path, st->lpData, wcslen(st->lpData) * sizeof(TCHAR));
                memcpy(path_orig, st->lpData, wcslen(st->lpData) * sizeof(TCHAR));
                PathUnquoteSpacesW(path_orig);
                PathRemoveExtensionW(path_orig);
                wsprintf(buffer, EXTRACT_CMD, path_orig, path);
                //wprintf(L"%s\n%s\n\n", st->lpData, buffer);
            }
            STARTUPINFO si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            BOOL b = CreateProcess(
                NULL,
                buffer,
                NULL,
                NULL,
                TRUE,
                CREATE_UNICODE_ENVIRONMENT,
                NULL,
                NULL,
                &si,
                &pi
            );
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        DestroyMenu(hMenu);
        ShowWindow(hWnd, SW_HIDE);
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    if (uMsg == WM_CLOSE && hWnd == archivehWnd)
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

HWND lastSysTrayhWnd;

BOOL TrackPopupMenuHook(
    HMENU      hMenu,
    UINT       uFlags,
    int        x,
    int        y,
    int        nReserved,
    HWND       hWnd,
    const RECT* prcRect
)
{
    if (hWnd != lastSysTrayhWnd)
    {
        return TrackPopupMenuFunc(
            hMenu,
            uFlags,
            x,
            y,
            nReserved,
            hWnd,
            prcRect
        );
    }
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
    BOOL b = TrackPopupMenuFunc(
        hMenu,
        uFlags,
        x,
        y,
        nReserved,
        hWnd,
        prcRect
    );
    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
        hMenu,
        hWnd,
        &(pt)
    );
    free(unknown_array);
    return b;
}

INT64 SysTrayWndProcHook(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    lastSysTrayhWnd = hWnd;
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
    return SysTrayWndProcFunc(
        hWnd,
        uMsg,
        wParam,
        lParam
    );
}

typedef struct
{
    void* _this;
    POINT point;
    IUnknown* iunk;
} ShowLauncherTipContextMenuParameters;

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

    HWND hWnd = CreateWindowInBand(
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
    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);

    while (!(*((HMENU*)((char*)params->_this + 0xe8))))
    {
        Sleep(1);
    }
    if (!(*((HMENU*)((char*)params->_this + 0xe8))))
    {
        goto finalize;
    }

    INT64* unknown_array = calloc(4, sizeof(INT64));
    ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
        *((HMENU*)((char*)params->_this + 0xe8)),
        hWnd,
        &(params->point),
        0xc,
        unknown_array
    );

    BOOL res = TrackPopupMenu(
        *((HMENU*)((char*)params->_this + 0xe8)),
        TPM_RETURNCMD,
        params->point.x,
        params->point.y,
        0,
        hWnd,
        0
    );

    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
        *((HMENU*)((char*)params->_this + 0xe8)),
        hWnd,
        &(params->point)
    );
    free(unknown_array);

    if (res > 0)
    {
        if (res < 4000)
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
        hWnd,
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

    POINT point;
    if (pt)
    {
        point = *pt;
    }
    else
    {
        POINT ptCursor;
        GetCursorPos(&ptCursor);
        MONITORINFO mi;
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(
            MonitorFromPoint(
                ptCursor, 
                MONITOR_DEFAULTTONEAREST
            ), 
            &mi
        );
        // https://stackoverflow.com/questions/44746234/programatically-get-windows-taskbar-info-autohidden-state-taskbar-coordinates
        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
        if (abd.rc.left < 5 && abd.rc.top > 5)
        {
            // TB_POS_BOTTOM
            point.x = mi.rcMonitor.left;
            point.y = mi.rcMonitor.bottom;
        }
        else if (abd.rc.left < 5 && abd.rc.top < 5 && abd.rc.right > abd.rc.bottom)
        {
            // TB_POS_TOP
            point.x = mi.rcMonitor.left;
            point.y = mi.rcMonitor.top;
        }
        else if (abd.rc.left < 5 && abd.rc.top < 5 && abd.rc.right < abd.rc.bottom)
        {
            // TB_POS_LEFT
            point.x = mi.rcMonitor.left;
            point.y = mi.rcMonitor.top;
        }
        else if (abd.rc.left > 5 && abd.rc.top < 5)
        {
            // TB_POS_RIGHT
            point.x = mi.rcMonitor.right;
            point.y = mi.rcMonitor.top;
        }
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

INT64 CTray_HandleGlobalHotkeyHook(
    void* _this,
    unsigned int a2,
    unsigned int a3
)
{
    if (a2 == 590 && IsDesktopInputContextFunc(_this, a2))
    {
        // this works just fine but is hacky because using 
        // the proper way does not work for some reason
        // see https://github.com/valinet/ExplorerPatcher/issues/3
        if (hIsWinXShown)
        {
            INPUT ip[2];
            ip[0].type = INPUT_KEYBOARD;
            ip[0].ki.wScan = 0;
            ip[0].ki.time = 0;
            ip[0].ki.dwExtraInfo = 0;
            ip[0].ki.wVk = VK_ESCAPE;
            ip[0].ki.dwFlags = 0;
            ip[1].type = INPUT_KEYBOARD;
            ip[1].ki.wScan = 0;
            ip[1].ki.time = 0;
            ip[1].ki.dwExtraInfo = 0;
            ip[1].ki.wVk = VK_ESCAPE;
            ip[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, ip, sizeof(INPUT));
            return 0;
        }

        HWND hWnd = GetForegroundWindow();
        HWND g_ProgWin = FindWindowEx(
            NULL,
            NULL,
            L"Progman",
            NULL
        );
        SetForegroundWindow(g_ProgWin);

        INPUT ip[4];
        ip[0].type = INPUT_KEYBOARD;
        ip[0].ki.wScan = 0;
        ip[0].ki.time = 0;
        ip[0].ki.dwExtraInfo = 0;
        ip[0].ki.wVk = VK_LWIN;
        ip[0].ki.dwFlags = 0;
        ip[1].type = INPUT_KEYBOARD;
        ip[1].ki.wScan = 0;
        ip[1].ki.time = 0;
        ip[1].ki.dwExtraInfo = 0;
        ip[1].ki.wVk = 0x51; // 0x46;
        ip[1].ki.dwFlags = 0;
        ip[2].type = INPUT_KEYBOARD;
        ip[2].ki.wScan = 0;
        ip[2].ki.time = 0;
        ip[2].ki.dwExtraInfo = 0;
        ip[2].ki.wVk = 0x51; // 0x46;
        ip[2].ki.dwFlags = KEYEVENTF_KEYUP;
        ip[3].type = INPUT_KEYBOARD;
        ip[3].ki.wScan = 0;
        ip[3].ki.time = 0;
        ip[3].ki.dwExtraInfo = 0;
        ip[3].ki.wVk = VK_LWIN;
        ip[3].ki.dwFlags = KEYEVENTF_KEYUP;
        InterlockedExchange64(&lockEnsureWinXHotkeyOnlyOnce, 1);
        SendInput(4, ip, sizeof(INPUT));
       
        SetForegroundWindow(hWnd);

        return 0;
    }
    return CTray_HandleGlobalHotkeyFunc(
        _this,
        a2,
        a3
    );
}

BOOL notFirstTimeCTray_v_WndProcHook = FALSE;
INT64 CTray_v_WndProcHook(
    void* _this,
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    if (!notFirstTimeCTray_v_WndProcHook)
    {
        notFirstTimeCTray_v_WndProcHook = TRUE;
        CTray__FireDesktopSwitchIfReadyFunc(_this, 2);
    }
    return CTray_v_WndProcFunc(
        _this,
        hWnd,
        uMsg,
        wParam,
        lParam
    );
}

HRESULT CImmersiveHotkeyNotification_OnMessageHook(
    void* _this,
    INT64 msg,
    INT wParam,
    INT64 lParam
)
{
    if (InterlockedExchange64(&lockEnsureWinXHotkeyOnlyOnce, 0) &&
        wParam == 30 && // 28, 15
        IsDesktopInputContextFunc(_this, msg)
    )
    {
        IUnknown* pMonitor = NULL;
        HRESULT hr = CImmersiveHotkeyNotification_GetMonitorForHotkeyNotificationFunc(
            (char*)_this - 0x68,
            &pMonitor,
            0
        );
        if (SUCCEEDED(hr))
        {
            IUnknown* pMenu = NULL;
            IUnknown_QueryService(
                pMonitor,
                &IID_ILauncherTipContextMenu,
                &IID_ILauncherTipContextMenu,
                &pMenu
            );
            if (pMenu)
            {
                CLauncherTipContextMenu_ShowLauncherTipContextMenuHook(
                    pMenu,
                    0
                );
                pMenu->lpVtbl->Release(pMenu);
            }
        }
        return 0;
    }

    return CImmersiveHotkeyNotification_OnMessageFunc(
        _this,
        msg,
        wParam,
        lParam
    );
}

// Slightly tweaked version of function available in Open Shell 
// (Open-Shell-Menu\Src\StartMenu\StartMenuHelper\StartMenuHelper.cpp)
LRESULT CALLBACK OpenStartOnCurentMonitorThreadHook(
    int code, 
    WPARAM wParam, 
    LPARAM lParam
)
{
    if (code == HC_ACTION && wParam)
    {
        MSG* msg = (MSG*)lParam;
        if (GetSystemMetrics(SM_CMONITORS) >= 2 && msg->message == WM_SYSCOMMAND && (msg->wParam & 0xFFF0) == SC_TASKLIST)
        {
            BOOL bShouldCheckHKLM = FALSE;
            HKEY hKey;
            if (RegOpenKeyEx(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage"),
                0,
                KEY_READ,
                &hKey
            ) != ERROR_SUCCESS)
            {
                bShouldCheckHKLM = TRUE;
            }
            DWORD dwStatus = 0;
            DWORD dwSize = sizeof(DWORD);
            if (RegGetValue(
                hKey,
                NULL,
                TEXT("MonitorOverride"),
                RRF_RT_REG_DWORD,
                NULL,
                &dwStatus,
                (LPDWORD)(&dwSize)
            ) != ERROR_SUCCESS)
            {
                bShouldCheckHKLM = TRUE;
            }
            RegCloseKey(hKey);
            if (bShouldCheckHKLM)
            {
                if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage"),
                    0,
                    KEY_READ,
                    &hKey
                ) != ERROR_SUCCESS)
                {
                    goto finish;
                }
                dwStatus = 0;
                dwSize = sizeof(DWORD);
                if (RegGetValue(
                    hKey,
                    NULL,
                    TEXT("MonitorOverride"),
                    RRF_RT_REG_DWORD,
                    NULL,
                    &dwStatus,
                    (LPDWORD)(&dwSize)
                ) != ERROR_SUCCESS)
                {
                    goto finish;
                }
                RegCloseKey(hKey);
            }
            if (dwStatus == 1)
            {
                goto finish;
            }

            DWORD pts = GetMessagePos();
            POINT pt;
            pt.x = GET_X_LPARAM(pts);
            pt.y = GET_Y_LPARAM(pts);
            HMONITOR monitor = MonitorFromPoint(
                pt, 
                MONITOR_DEFAULTTONULL
            );

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
                    IUnknown* pMonitor = NULL;
                    pMonitorService->lpVtbl->GetFromHandle(
                        pMonitorService, 
                        monitor, 
                        &pMonitor
                    );
                    IImmersiveLauncher10RS* pLauncher = NULL;
                    IUnknown_QueryService(
                        pImmersiveShell,
                        &SID_ImmersiveLauncher,
                        &IID_IImmersiveLauncher10RS,
                        &pLauncher
                    );
                    if (pLauncher)
                    {
                        BOOL bIsVisible = FALSE;
                        pLauncher->lpVtbl->IsVisible(pLauncher, &bIsVisible);
                        if (SUCCEEDED(hr))
                        {
                            if (!bIsVisible)
                            {
                                if (pMonitor)
                                {
                                    pLauncher->lpVtbl->ConnectToMonitor(pLauncher, pMonitor);
                                }
                                pLauncher->lpVtbl->ShowStartView(pLauncher, 11, 0);
                            }
                            else
                            {
                                pLauncher->lpVtbl->Dismiss(pLauncher);
                            }
                        }
                        pLauncher->lpVtbl->Release(pLauncher);
                    }
                    if (pMonitor)
                    {
                        pMonitor->lpVtbl->Release(pMonitor);
                    }
                    pMonitorService->lpVtbl->Release(pMonitorService);
                }
                pImmersiveShell->lpVtbl->Release(pImmersiveShell);
            }

            msg->message = WM_NULL;
        }
    }
    finish:
    return CallNextHookEx(NULL, code, wParam, lParam);
}

DWORD ArchiveMenuThread(LPVOID unused)
{
    Sleep(1000);

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        return 0;
    }

    WNDCLASS wc = { 0 };
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = CLauncherTipContextMenu_WndProc;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"ArchiveMenuWindowExplorer";
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClass(&wc);

    archivehWnd = CreateWindowInBand(
        0,
        L"ArchiveMenuWindowExplorer",
        0,
        WS_POPUP,
        0,
        0,
        0,
        0,
        0,
        0,
        GetModuleHandle(NULL),
        NULL,
        7
    );
    if (!archivehWnd)
    {
        return 0;
    }
    ITaskbarList* pTaskList = NULL;
    hr = CoCreateInstance(
        &__uuidof_TaskbarList,
        NULL,
        CLSCTX_ALL,
        &__uuidof_ITaskbarList,
        (void**)(&pTaskList)
    );
    if (FAILED(hr))
    {
        return 0;
    }
    hr = pTaskList->lpVtbl->HrInit(pTaskList);
    if (FAILED(hr))
    {
        return 0;
    }
    ShowWindow(archivehWnd, SW_SHOW);
    hr = pTaskList->lpVtbl->DeleteTab(pTaskList, archivehWnd);
    if (FAILED(hr))
    {
        return 0;
    }
    hr = pTaskList->lpVtbl->Release(pTaskList);
    if (FAILED(hr))
    {
        return 0;
    }

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

DWORD OpenStartOnCurentMonitorThread(LPVOID unused)
{
    HWND g_ProgWin = FindWindowEx(
        NULL, 
        NULL, 
        L"Progman",
        NULL
    );
    DWORD progThread = GetWindowThreadProcessId(
        g_ProgWin, 
        NULL
    );
    HHOOK g_ProgHook = SetWindowsHookEx(
        WH_GETMESSAGE, 
        OpenStartOnCurentMonitorThreadHook, 
        NULL, 
        progThread
    );
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

DWORD PlayStartupSound(DWORD x)
{
    Sleep(1000);

    HRESULT hr = CoInitialize(NULL);

    /*HKEY hKey2;
    LONG lRes2 = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Authentication\\LogonUI\\LogonSoundPlayed",
        0,
        KEY_READ,
        &hKey2
    );
    if (lRes2 == ERROR_SUCCESS)
    {
        DWORD val = 5;
        DWORD szval = 4;
        lRes2 = RegQueryValueExW(
            hKey2,
            L"SoundPlayed",
            0,
            NULL,
            &val,
            &szval
        );
        printf("SoundPlayed: %d %d\n", val, lRes2);
    }*/

    AuthUILogonSound* ppv;
    hr = CoCreateInstance(
        &__uuidof_AuthUILogonSound,
        0,
        1,
        &__uuidof_IAuthUILogonSound,
        &ppv
    );
    if (SUCCEEDED(hr))
    {
        ppv->lpVtbl->PlayIfNecessary(ppv, 1);
        ppv->lpVtbl->Release(ppv);
    }
    return 0;
}

DWORD SignalShellReady(DWORD x)
{
    Sleep(1000);

    HANDLE hEvent = CreateEvent(0, 1, 1, L"ShellDesktopSwitchEvent");
    if (hEvent)
    {
        SetEvent(hEvent);
    }
    return 0;
}

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
        
        CreateThread(
            0,
            0,
            PlayStartupSound,
            0,
            0,
            0
        );




        CreateThread(
            0,
            0,
            SignalShellReady,
            0,
            0,
            0
        );




        messageWindow = (HWND)lpParameter;



        funchook = funchook_create();




        DWORD dwRet = 0;
        char szSettingsPath[MAX_PATH];
        ZeroMemory(
            szSettingsPath,
            (MAX_PATH) * sizeof(char)
        );
        TCHAR wszSettingsPath[MAX_PATH];
        ZeroMemory(
            wszSettingsPath,
            (MAX_PATH) * sizeof(TCHAR)
        );
        GetModuleFileNameA(
            hModule,
            szSettingsPath,
            MAX_PATH
        );
        PathRemoveFileSpecA(szSettingsPath);
        strcat_s(
            szSettingsPath,
            MAX_PATH,
            SYMBOLS_RELATIVE_PATH
        );
        mbstowcs_s(
            &dwRet,
            wszSettingsPath,
            MAX_PATH,
            szSettingsPath,
            MAX_PATH
        );




        CreateThread(
            0,
            0,
            OpenStartOnCurentMonitorThread,
            0,
            0,
            0
        );




        symbols_addr symbols_PTRS;
        ZeroMemory(
            &symbols_PTRS,
            sizeof(symbols_addr)
        );
        symbols_PTRS.explorer_PTRS[0] = VnGetUInt(
            TEXT(EXPLORER_SB_NAME),
            TEXT(EXPLORER_SB_0),
            0,
            wszSettingsPath
        );
        symbols_PTRS.explorer_PTRS[1] = VnGetUInt(
            TEXT(EXPLORER_SB_NAME),
            TEXT(EXPLORER_SB_1),
            0,
            wszSettingsPath
        );
        symbols_PTRS.explorer_PTRS[2] = VnGetUInt(
            TEXT(EXPLORER_SB_NAME),
            TEXT(EXPLORER_SB_2),
            0,
            wszSettingsPath
        );

        symbols_PTRS.twinui_pcshell_PTRS[0] = VnGetUInt(
            TEXT(TWINUI_PCSHELL_SB_NAME),
            TEXT(TWINUI_PCSHELL_SB_0),
            0,
            wszSettingsPath
        );
        symbols_PTRS.twinui_pcshell_PTRS[1] = VnGetUInt(
            TEXT(TWINUI_PCSHELL_SB_NAME),
            TEXT(TWINUI_PCSHELL_SB_1),
            0,
            wszSettingsPath
        );
        symbols_PTRS.twinui_pcshell_PTRS[2] = VnGetUInt(
            TEXT(TWINUI_PCSHELL_SB_NAME),
            TEXT(TWINUI_PCSHELL_SB_2),
            0,
            wszSettingsPath
        );
        symbols_PTRS.twinui_pcshell_PTRS[3] = VnGetUInt(
            TEXT(TWINUI_PCSHELL_SB_NAME),
            TEXT(TWINUI_PCSHELL_SB_3),
            0,
            wszSettingsPath
        );
        symbols_PTRS.twinui_pcshell_PTRS[4] = VnGetUInt(
            TEXT(TWINUI_PCSHELL_SB_NAME),
            TEXT(TWINUI_PCSHELL_SB_4),
            0,
            wszSettingsPath
        );
        symbols_PTRS.twinui_pcshell_PTRS[5] = VnGetUInt(
            TEXT(TWINUI_PCSHELL_SB_NAME),
            TEXT(TWINUI_PCSHELL_SB_5),
            0,
            wszSettingsPath
        );
        symbols_PTRS.twinui_pcshell_PTRS[6] = VnGetUInt(
            TEXT(TWINUI_PCSHELL_SB_NAME),
            TEXT(TWINUI_PCSHELL_SB_6),
            0,
            wszSettingsPath
        );

        symbols_PTRS.twinui_PTRS[0] = VnGetUInt(
            TEXT(TWINUI_SB_NAME),
            TEXT(TWINUI_SB_0),
            0,
            wszSettingsPath
        );
        symbols_PTRS.twinui_PTRS[1] = VnGetUInt(
            TEXT(TWINUI_SB_NAME),
            TEXT(TWINUI_SB_1),
            0,
            wszSettingsPath
        );
        symbols_PTRS.twinui_PTRS[2] = VnGetUInt(
            TEXT(TWINUI_SB_NAME),
            TEXT(TWINUI_SB_2),
            0,
            wszSettingsPath
        );

        symbols_PTRS.stobject_PTRS[0] = VnGetUInt(
            TEXT(STOBJECT_SB_NAME),
            TEXT(STOBJECT_SB_0),
            0,
            wszSettingsPath
        );
        symbols_PTRS.stobject_PTRS[1] = VnGetUInt(
            TEXT(STOBJECT_SB_NAME),
            TEXT(STOBJECT_SB_1),
            0,
            wszSettingsPath
        );

        BOOL bNeedToDownload = FALSE;
        for (UINT i = 0; i < sizeof(symbols_addr) / sizeof(DWORD); ++i)
        {
            if (!((DWORD*)&symbols_PTRS)[i])
            {
                bNeedToDownload = TRUE;
            }
        }
        // https://stackoverflow.com/questions/36543301/detecting-windows-10-version/36543774#36543774
        RTL_OSVERSIONINFOW rovi;
        if (!GetOSVersion(&rovi))
        {
            FreeLibraryAndExitThread(
                hModule,
                1
            );
            return 1;
        }
        // https://stackoverflow.com/questions/47926094/detecting-windows-10-os-build-minor-version
        DWORD32 ubr = 0, ubr_size = sizeof(DWORD32);
        HKEY hKey;
        LONG lRes = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            wcschr(
                wcschr(
                    wcschr(
                        UNIFIEDBUILDREVISION_KEY,
                        '\\'
                    ) + 1,
                    '\\'
                ) + 1,
                '\\'
            ) + 1,
            0,
            KEY_READ,
            &hKey
        );
        if (lRes == ERROR_SUCCESS)
        {
            RegQueryValueExW(
                hKey,
                UNIFIEDBUILDREVISION_VALUE,
                0,
                NULL,
                &ubr,
                &ubr_size
            );
        }
        TCHAR szReportedVersion[MAX_PATH];
        ZeroMemory(
            szReportedVersion,
            (MAX_PATH) * sizeof(TCHAR)
        );
        TCHAR szStoredVersion[MAX_PATH];
        ZeroMemory(
            szStoredVersion,
            (MAX_PATH) * sizeof(TCHAR)
        );
        wsprintf(
            szReportedVersion,
            L"%d.%d.%d.%d",
            rovi.dwMajorVersion,
            rovi.dwMinorVersion,
            rovi.dwBuildNumber,
            ubr
        );
        VnGetString(
            TEXT("OS"),
            TEXT("Build"),
            szStoredVersion,
            MAX_PATH,
            MAX_PATH,
            NULL,
            wszSettingsPath
        );
        if (!bNeedToDownload)
        {
            bNeedToDownload = wcscmp(szReportedVersion, szStoredVersion);
        }

        if (bNeedToDownload)
        {
            TCHAR buffer[sizeof(DownloadSymbolsXML) / sizeof(wchar_t) + 30];
            ZeroMemory(
                buffer,
                (sizeof(DownloadSymbolsXML) / sizeof(wchar_t) + 30) * sizeof(TCHAR)
            );
            wsprintf(
                buffer,
                DownloadSymbolsXML,
                szReportedVersion
            );
            HRESULT hr = S_OK;
            __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
            hr = String2IXMLDocument(
                buffer,
                wcslen(buffer),
                &inputXml,
#ifdef DEBUG
                stdout
#else
                NULL
#endif
            );
            hr = ShowToastMessage(
                inputXml,
                APPID,
                sizeof(APPID) / sizeof(TCHAR) - 1,
#ifdef DEBUG
                stdout
#else
                NULL
#endif
            );
            char explorer_sb_exe[MAX_PATH];
            ZeroMemory(
                explorer_sb_exe,
                (MAX_PATH) * sizeof(char)
            );
            GetWindowsDirectoryA(
                explorer_sb_exe,
                MAX_PATH
            );
            strcat_s(
                explorer_sb_exe,
                MAX_PATH,
                "\\"
            );
            strcat_s(
                explorer_sb_exe,
                MAX_PATH,
                EXPLORER_SB_NAME
            );
            strcat_s(
                explorer_sb_exe,
                MAX_PATH,
                ".exe"
            );
            printf("Downloading symbols for %s.\n", explorer_sb_exe);
            if (VnDownloadSymbols(
                NULL,
                explorer_sb_exe,
                szSettingsPath,
                MAX_PATH
            ))
            {
                FreeLibraryAndExitThread(
                    hModule,
                    2
                );
                return 2;
            }
            printf("Reading symbols.\n");
            if (VnGetSymbols(
                szSettingsPath,
                symbols_PTRS.explorer_PTRS,
                explorer_SN,
                EXPLORER_SB_CNT
            ))
            {
                FreeLibraryAndExitThread(
                    hModule,
                    3
                );
                return 3;
            }
            VnWriteUInt(
                TEXT(EXPLORER_SB_NAME),
                TEXT(EXPLORER_SB_0),
                symbols_PTRS.explorer_PTRS[0],
                wszSettingsPath
            );
            VnWriteUInt(
                TEXT(EXPLORER_SB_NAME),
                TEXT(EXPLORER_SB_1),
                symbols_PTRS.explorer_PTRS[1],
                wszSettingsPath
            );
            VnWriteUInt(
                TEXT(EXPLORER_SB_NAME),
                TEXT(EXPLORER_SB_2),
                symbols_PTRS.explorer_PTRS[2],
                wszSettingsPath
            );

            char twinui_pcshell_sb_dll[MAX_PATH];
            ZeroMemory(
                twinui_pcshell_sb_dll,
                (MAX_PATH) * sizeof(char)
            );
            GetSystemDirectoryA(
                twinui_pcshell_sb_dll,
                MAX_PATH
            );
            strcat_s(
                twinui_pcshell_sb_dll,
                MAX_PATH,
                "\\"
            );
            strcat_s(
                twinui_pcshell_sb_dll,
                MAX_PATH,
                TWINUI_PCSHELL_SB_NAME
            );
            strcat_s(
                twinui_pcshell_sb_dll,
                MAX_PATH,
                ".dll"
            );
            printf("Downloading symbols for %s.\n", twinui_pcshell_sb_dll);
            if (VnDownloadSymbols(
                NULL,
                twinui_pcshell_sb_dll,
                szSettingsPath,
                MAX_PATH
            ))
            {
                FreeLibraryAndExitThread(
                    hModule,
                    4
                );
                return 4;
            }
            printf("Reading symbols.\n");
            if (VnGetSymbols(
                szSettingsPath,
                symbols_PTRS.twinui_pcshell_PTRS,
                twinui_pcshell_SN,
                TWINUI_PCSHELL_SB_CNT
            ))
            {
                FreeLibraryAndExitThread(
                    hModule,
                    5
                );
                return 5;
            }
            VnWriteUInt(
                TEXT(TWINUI_PCSHELL_SB_NAME),
                TEXT(TWINUI_PCSHELL_SB_0),
                symbols_PTRS.twinui_pcshell_PTRS[0],
                wszSettingsPath
            );
            VnWriteUInt(
                TEXT(TWINUI_PCSHELL_SB_NAME),
                TEXT(TWINUI_PCSHELL_SB_1),
                symbols_PTRS.twinui_pcshell_PTRS[1],
                wszSettingsPath
            );
            VnWriteUInt(
                TEXT(TWINUI_PCSHELL_SB_NAME),
                TEXT(TWINUI_PCSHELL_SB_2),
                symbols_PTRS.twinui_pcshell_PTRS[2],
                wszSettingsPath
            );
            VnWriteUInt(
                TEXT(TWINUI_PCSHELL_SB_NAME),
                TEXT(TWINUI_PCSHELL_SB_3),
                symbols_PTRS.twinui_pcshell_PTRS[3],
                wszSettingsPath
            );
            VnWriteUInt(
                TEXT(TWINUI_PCSHELL_SB_NAME),
                TEXT(TWINUI_PCSHELL_SB_4),
                symbols_PTRS.twinui_pcshell_PTRS[4],
                wszSettingsPath
            );
            VnWriteUInt(
                TEXT(TWINUI_PCSHELL_SB_NAME),
                TEXT(TWINUI_PCSHELL_SB_5),
                symbols_PTRS.twinui_pcshell_PTRS[5],
                wszSettingsPath
            );
            VnWriteUInt(
                TEXT(TWINUI_PCSHELL_SB_NAME),
                TEXT(TWINUI_PCSHELL_SB_6),
                symbols_PTRS.twinui_pcshell_PTRS[6],
                wszSettingsPath
            );

            char twinui_sb_dll[MAX_PATH];
            ZeroMemory(
                twinui_sb_dll,
                (MAX_PATH) * sizeof(char)
            );
            GetSystemDirectoryA(
                twinui_sb_dll,
                MAX_PATH
            );
            strcat_s(
                twinui_sb_dll,
                MAX_PATH,
                "\\"
            );
            strcat_s(
                twinui_sb_dll,
                MAX_PATH,
                TWINUI_SB_NAME
            );
            strcat_s(
                twinui_sb_dll,
                MAX_PATH,
                ".dll"
            );
            printf("Downloading symbols for %s.\n", twinui_sb_dll);
            if (VnDownloadSymbols(
                NULL,
                twinui_sb_dll,
                szSettingsPath,
                MAX_PATH
            ))
            {
                FreeLibraryAndExitThread(
                    hModule,
                    6
                );
                return 6;
            }
            printf("Reading symbols.\n");
            if (VnGetSymbols(
                szSettingsPath,
                symbols_PTRS.twinui_PTRS,
                twinui_SN,
                TWINUI_SB_CNT
            ))
            {
                FreeLibraryAndExitThread(
                    hModule,
                    7
                );
                return 7;
            }
            VnWriteUInt(
                TEXT(TWINUI_SB_NAME),
                TEXT(TWINUI_SB_0),
                symbols_PTRS.twinui_PTRS[0],
                wszSettingsPath
            );
            VnWriteUInt(
                TEXT(TWINUI_SB_NAME),
                TEXT(TWINUI_SB_1),
                symbols_PTRS.twinui_PTRS[1],
                wszSettingsPath
            );
            VnWriteUInt(
                TEXT(TWINUI_SB_NAME),
                TEXT(TWINUI_SB_2),
                symbols_PTRS.twinui_PTRS[2],
                wszSettingsPath
            );

            char stobject_sb_dll[MAX_PATH];
            ZeroMemory(
                stobject_sb_dll,
                (MAX_PATH) * sizeof(char)
            );
            GetSystemDirectoryA(
                stobject_sb_dll,
                MAX_PATH
            );
            strcat_s(
                stobject_sb_dll,
                MAX_PATH,
                "\\"
            );
            strcat_s(
                stobject_sb_dll,
                MAX_PATH,
                STOBJECT_SB_NAME
            );
            strcat_s(
                stobject_sb_dll,
                MAX_PATH,
                ".dll"
            );
            printf("Downloading symbols for %s.\n", stobject_sb_dll);
            if (VnDownloadSymbols(
                NULL,
                stobject_sb_dll,
                szSettingsPath,
                MAX_PATH
            ))
            {
                FreeLibraryAndExitThread(
                    hModule,
                    6
                );
                return 6;
            }
            printf("Reading symbols.\n");
            if (VnGetSymbols(
                szSettingsPath,
                symbols_PTRS.stobject_PTRS,
                stobject_SN,
                STOBJECT_SB_CNT
            ))
            {
                FreeLibraryAndExitThread(
                    hModule,
                    7
                );
                return 7;
            }
            VnWriteUInt(
                TEXT(STOBJECT_SB_NAME),
                TEXT(STOBJECT_SB_0),
                symbols_PTRS.stobject_PTRS[0],
                wszSettingsPath
            );
            VnWriteUInt(
                TEXT(STOBJECT_SB_NAME),
                TEXT(STOBJECT_SB_1),
                symbols_PTRS.stobject_PTRS[1],
                wszSettingsPath
            );

            VnWriteString(
                TEXT("OS"),
                TEXT("Build"),
                szReportedVersion,
                wszSettingsPath
            );

            __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml2 = NULL;
            hr = String2IXMLDocument(
                DownloadOKXML,
                wcslen(DownloadOKXML),
                &inputXml2,
#ifdef DEBUG
                stdout
#else
                NULL
#endif
            );
            hr = ShowToastMessage(
                inputXml2,
                APPID,
                sizeof(APPID) / sizeof(TCHAR) - 1,
#ifdef DEBUG
                stdout
#else
                NULL
#endif
            );
        }


        
        
        HANDLE hExplorer = GetModuleHandle(NULL);
        CTray_HandleGlobalHotkeyFunc = (INT64(*)(void*, unsigned int, unsigned int))
            ((uintptr_t)hExplorer + symbols_PTRS.explorer_PTRS[0]);
        rv = funchook_prepare(
            funchook,
            (void**)&CTray_HandleGlobalHotkeyFunc,
            CTray_HandleGlobalHotkeyHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
        CTray_v_WndProcFunc = (INT64(*)(void*, HWND, UINT, WPARAM, LPARAM))
            ((uintptr_t)hExplorer + symbols_PTRS.explorer_PTRS[1]);
        /*rv = funchook_prepare(
            funchook,
            (void**)&CTray_v_WndProcFunc,
            CTray_v_WndProcHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }*/
        CTray__FireDesktopSwitchIfReadyFunc = (INT64(*)(HWND, int))
            ((uintptr_t)hExplorer + symbols_PTRS.explorer_PTRS[2]);


        HANDLE hUser32 = GetModuleHandle(L"user32.dll");

        if (hUser32) CreateWindowInBand = GetProcAddress(hUser32, "CreateWindowInBand");


        LoadLibrary("twinui.pcshell.dll");
        HANDLE hTwinuiPcshell = GetModuleHandle(L"twinui.pcshell.dll");

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


        LoadLibrary(L"twinui.dll");
        HANDLE hTwinui = GetModuleHandle(L"twinui.dll");

        CImmersiveHotkeyNotification_GetMonitorForHotkeyNotificationFunc = (INT64(*)(void*, void**, HWND*))
            ((uintptr_t)hTwinui + symbols_PTRS.twinui_PTRS[0]);

        IsDesktopInputContextFunc = (BOOL(*)(void*, void*))
            ((uintptr_t)hTwinui + symbols_PTRS.twinui_PTRS[1]);

        CImmersiveHotkeyNotification_OnMessageFunc = (HRESULT(*)(void*, INT64, INT, INT64))
            ((uintptr_t)hTwinui + symbols_PTRS.twinui_PTRS[2]);
        rv = funchook_prepare(
            funchook,
            (void**)&CImmersiveHotkeyNotification_OnMessageFunc,
            CImmersiveHotkeyNotification_OnMessageHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }



        LoadLibrary(L"stobject.dll");
        HANDLE hStobject = GetModuleHandle(L"stobject.dll");
        SysTrayWndProcFunc = (INT64(*)(HWND, UINT, WPARAM, LPARAM))
            ((uintptr_t)hStobject + symbols_PTRS.stobject_PTRS[0]);
        rv = funchook_prepare(
            funchook,
            (void**)&SysTrayWndProcFunc,
            SysTrayWndProcHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
        TrackPopupMenuFunc = TrackPopupMenu;
        rv = funchook_prepare(
            funchook,
            (void**)&TrackPopupMenuFunc,
            TrackPopupMenuHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }


        rv = funchook_install(funchook, 0);
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }




        UINT archive_plugin = VnGetUInt(
            L"ArchiveMenu",
            L"Enabled",
            0,
            wszSettingsPath
        );
        if (archive_plugin)
        {
            CreateThread(
                0,
                0,
                ArchiveMenuThread,
                0,
                0,
                0
            );
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

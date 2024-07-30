#include <Windows.h>
#include <Shlwapi.h>
#include <ShellScalingApi.h>
#include <initguid.h>

#include <wrl/client.h>
#include <wil/resource.h>
#include <wil/result_macros.h>

#include <valinet/utility/memmem.h>
#include <wrl/wrappers/corewrappers.h>

#include "ArchiveMenu.h"
#include "utility.h"
#include "hooking.h"
#include "symbols.h"

using namespace Microsoft::WRL;


#pragma region "Types and utilities"

DEFINE_GUID(SID_EdgeUi, 0x0D189B30, 0xF12B, 0x4B13, 0x94, 0xCF, 0x53, 0xCB, 0x0E, 0x0E, 0x24, 0x0D); // 0d189b30-f12b-4b13-94cf-53cb0e0e240d

interface IImmersiveApplication;
interface IEdgeUiInvocationProvider;

enum EDGEUI_COMPONENT
{
    EUICMP_UNKNOWN = -1,
    EUICMP_SWITCHER = 0,
    EUICMP_CHARMSBAR,
    EUICMP_APPBAR,
    EUICMP_TASKBAR,
    EUICMP_TITLEBAR,
    EUICMP_TABLETMODEVIEWMANAGER,
    EUICMP_ACTIONCENTER,
    EUICMP_TOTALCOUNT,
};

enum DISMISSED_UI_FLAGS
{
    DUF_NONE = 0x0,
    DUF_FORCEOBSERVATIONOFF = 0x1,
};

enum EDGEUI_TRAYSTUCKPLACE
{
    EUITSP_UNKNOWN = -1,
    EUITSP_LEFT = 0,
    EUITSP_TOP,
    EUITSP_RIGHT,
    EUITSP_BOTTOM,
};

MIDL_INTERFACE("6e6c3c52-5a5e-4b4b-a0f8-7fe12621a93e")
IEdgeUiManager : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetTargetApplicationFromPoint(POINT, int, IImmersiveApplication**) = 0;
    virtual HRESULT STDMETHODCALLTYPE DismissedUI(EDGEUI_COMPONENT, DISMISSED_UI_FLAGS) = 0;
    virtual HRESULT STDMETHODCALLTYPE HandleEdgeGesturePrefChanged(HWND) = 0;
    virtual HRESULT STDMETHODCALLTYPE DiscreteInvokeForApp(EDGEUI_COMPONENT, IImmersiveApplication*) = 0;
    virtual HRESULT STDMETHODCALLTYPE BeginInputObservation(EDGEUI_COMPONENT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetRegionForCornerOrEdge(EDGEUI_COMPONENT, HRGN*) = 0;
    virtual HRESULT STDMETHODCALLTYPE NotifyTrayStuckPlaceChanged(EDGEUI_TRAYSTUCKPLACE) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTrayStuckPlace(EDGEUI_TRAYSTUCKPLACE*) = 0;
    virtual HRESULT STDMETHODCALLTYPE NotifyTraySearchBoxVisibilityChanged(BOOL) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTraySearchBoxVisibility(BOOL*) = 0;
    virtual HRESULT STDMETHODCALLTYPE NotifyPearlRectChanged(RECT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPearlRect(RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE UpdateEdgeWindowZorder() = 0;
    virtual HRESULT STDMETHODCALLTYPE ShowStandardSystemOverlays(IImmersiveApplication*) = 0;
    virtual HRESULT STDMETHODCALLTYPE OverrideInvocation(IEdgeUiInvocationProvider*) = 0;
    virtual HRESULT STDMETHODCALLTYPE NotifyAutohideImmuneWorkAreaMayHaveChanged(RECT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAutohideImmuneWorkArea(RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE TaskbarRaised() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTrayRect(RECT*) = 0;
};

enum IMMERSIVE_MONITOR_FILTER_FLAGS
{
    IMMERSIVE_MONITOR_FILTER_FLAGS_NONE = 0x0,
    IMMERSIVE_MONITOR_FILTER_FLAGS_DISABLE_TRAY = 0x1,
};

DEFINE_ENUM_FLAG_OPERATORS(IMMERSIVE_MONITOR_FILTER_FLAGS);

MIDL_INTERFACE("880b26f8-9197-43d0-8045-8702d0d72000")
IImmersiveMonitor : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetIdentity(DWORD*) = 0;
    virtual HRESULT STDMETHODCALLTYPE ConnectObject(IUnknown*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetHandle(HMONITOR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsConnected(BOOL*) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsPrimary(BOOL*) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsImmersiveDisplayDevice(BOOL*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDisplayRect(RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetOrientation(DWORD*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetWorkArea(RECT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsEqual(IImmersiveMonitor*, BOOL*) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsImmersiveCapable(BOOL*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetEffectiveDpi(UINT*, UINT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFilterFlags(IMMERSIVE_MONITOR_FILTER_FLAGS*) = 0;
};

DEFINE_GUID(SID_IImmersiveMonitorService, 0x47094E3A, 0x0CF2, 0x430F, 0x80, 0x6F, 0xCF, 0x9E, 0x4F, 0x0F, 0x12, 0xDD); // 47094e3a-0cf2-430f-806f-cf9e4f0f12dd

enum IMMERSIVE_MONITOR_MOVE_DIRECTION
{
    IMMD_PREVIOUS,
    IMMD_NEXT,
};

interface IImmersiveMonitorFilter;

MIDL_INTERFACE("4d4c1e64-e410-4faa-bafa-59ca069bfec2")
IImmersiveMonitorManager : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetCount(UINT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetConnectedCount(UINT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAt(UINT, IImmersiveMonitor**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFromHandle(HMONITOR, IImmersiveMonitor**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFromIdentity(DWORD, IImmersiveMonitor**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetImmersiveProxyMonitor(IImmersiveMonitor**) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryService(HMONITOR, REFGUID, REFGUID, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryServiceByIdentity(DWORD, REFGUID, REFGUID, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryServiceFromWindow(HWND, REFGUID, REFGUID, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryServiceFromPoint(const POINT*, REFGUID, REFGUID, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetNextImmersiveMonitor(IMMERSIVE_MONITOR_MOVE_DIRECTION, IImmersiveMonitor*, IImmersiveMonitor**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMonitorArray(IObjectArray**) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetFilter(IImmersiveMonitorFilter*) = 0;
};

DEFINE_GUID(SID_ImmersiveLauncher, 0x6F86E01C, 0xC649, 0x4D61, 0xBE, 0x23, 0xF1, 0x32, 0x2D, 0xDE, 0xCA, 0x9D); // 6f86e01c-c649-4d61-be23-f1322ddeca9d

enum IMMERSIVELAUNCHERSHOWMETHOD
{
    ILSM_INVALID = 0,
    ILSM_HSHELLTASKMAN = 1,
    ILSM_IMMERSIVEBACKGROUND = 4,
    ILSM_APPCLOSED = 6,
    ILSM_STARTBUTTON = 11,
    ILSM_RETAILDEMO_EDUCATIONAPP = 12,
    ILSM_BACK = 13,
    ILSM_SESSIONONUNLOCK = 14,
};

enum IMMERSIVELAUNCHERSHOWFLAGS
{
    ILSF_NONE = 0x0,
    ILSF_IGNORE_SET_FOREGROUND_ERROR = 0x4,
};

DEFINE_ENUM_FLAG_OPERATORS(IMMERSIVELAUNCHERSHOWFLAGS);

enum IMMERSIVELAUNCHERDISMISSMETHOD
{
    ILDM_INVALID = 0,
    ILDM_HSHELLTASKMAN = 1,
    ILDM_STARTCHARM = 2,
    ILDM_BACKGESTURE = 3,
    ILDM_ESCAPEKEY = 4,
    ILDM_SHOWDESKTOP = 5,
    ILDM_STARTTIP = 6,
    ILDM_GENERIC_NONANIMATING = 7,
    ILDM_SEARCH_OPENING = 8,
    ILDM_DRAG = 9,
};

MIDL_INTERFACE("d8d60399-a0f1-f987-5551-321fd1b49864")
IImmersiveLauncher : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE ShowStartView(IMMERSIVELAUNCHERSHOWMETHOD, IMMERSIVELAUNCHERSHOWFLAGS) = 0;
    virtual HRESULT STDMETHODCALLTYPE Dismiss(IMMERSIVELAUNCHERDISMISSMETHOD) = 0;
    virtual HRESULT STDMETHODCALLTYPE DismissToLastDesktopApplication(IMMERSIVELAUNCHERDISMISSMETHOD) = 0;
    virtual HRESULT STDMETHODCALLTYPE DismissSynchronouslyWithoutTransition() = 0;
    virtual HRESULT STDMETHODCALLTYPE IsVisible(BOOL*) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnStartButtonPressed(IMMERSIVELAUNCHERSHOWMETHOD, IMMERSIVELAUNCHERDISMISSMETHOD) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetForeground() = 0;
    virtual HRESULT STDMETHODCALLTYPE ConnectToMonitor(IImmersiveMonitor*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMonitor(IImmersiveMonitor**) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnFirstSignAnimationFinished() = 0;
    virtual HRESULT STDMETHODCALLTYPE Prelaunch() = 0;
};

MIDL_INTERFACE("b8c1db5f-cbb3-48bc-afd9-ce6b880c79ed")
ILauncherTipContextMenu : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE ShowLauncherTipContextMenu(POINT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMenuItemsAsync(RECT, IUnknown**) = 0;
};

inline BOOL IsBiDiLocale(LCID locale)
{
    int info;
    int charsRead = GetLocaleInfoW(
        locale,
        LOCALE_IREADINGLAYOUT | LOCALE_RETURN_NUMBER,
        (LPWSTR)&info,
        sizeof(info) / sizeof(WCHAR)
    );
    return charsRead > 0 ? info == 1 : false;
}

BOOL Mirror_IsThreadRTL()
{
    return IsBiDiLocale(GetThreadUILanguage());
}

enum ZBID : int;
enum ACCENT_STATE : int;

class CSingleViewShellExperience;

class SingleViewShellExperiencePersonality;

class CSingleViewShellExperience
{
public:
    enum class Border
    {
        None = 0,
        Left = 1,
        Top = 2,
        Right = 4,
        Bottom = 8
    };

    HRESULT SetPosition(const RECT* rect);

    Wrappers::HString _args;
    Wrappers::HString _aumid;
    Wrappers::HString _experience;
    void* _viewWrapper;
    ComPtr<ABI::Windows::Foundation::Collections::IPropertySet> _propertySet;
    int _viewState;
    ABI::Windows::Foundation::Size _desiredSize;
    BOOLEAN _fullScreen;
    bool _isSessionIdle;
    DWORD _pid;
    ZBID _zbidDefault;
    int _pendingViewAction;
    int _pendingViewShowFlags;
    int _navLevelOverrideHelper[2];
    wistd::unique_ptr<SingleViewShellExperiencePersonality> m_personality;
    // ...
};

class SingleViewShellExperiencePersonality
{
public:
    virtual ~SingleViewShellExperiencePersonality() = 0;
    virtual bool IsPersonality(void*) = 0;
    virtual HRESULT Initialize(IServiceProvider*) = 0;
    virtual HRESULT EnableSessionIdleNotifications(IServiceProvider*) = 0;
    virtual HRESULT OnViewWrapperChanged() = 0;
    virtual HRESULT ShowView() = 0;
    virtual HRESULT HideView() = 0;
    virtual HRESULT IsViewVisible(bool*) = 0;
    virtual HRESULT SetWindowBand(ZBID) = 0;
    virtual HRESULT BringToForeground() = 0;
    virtual HRESULT BringToFocus() = 0;
    virtual HRESULT ShowBorder(CSingleViewShellExperience::Border, ACCENT_STATE, DWORD, const RECT*) = 0;
    virtual HRESULT SetPosition(const RECT*) = 0;
};

HRESULT CSingleViewShellExperience::SetPosition(const RECT* rect)
{
    RETURN_HR(m_personality->SetPosition(rect));
}

namespace ExperienceManagerUtils
{
    void ScaleByDPI(const ABI::Windows::Foundation::Size* size, int dpi, int* outWidth, int* outHeight)
    {
        *outWidth = MulDiv((int)size->Width, dpi, 96);
        *outHeight = MulDiv((int)size->Height, dpi, 96);
    }
}

#pragma endregion


#pragma region "Stuff from dllmain"

extern "C"
{

extern HMODULE hModule;
extern HWND archivehWnd;
extern DWORD bOldTaskbar;
extern DWORD bSkinMenus;
extern DWORD bClockFlyoutOnWinC;
extern DWORD bPropertiesInWinX;
extern DWORD bNoMenuAccelerator;
extern DWORD dwAltTabSettings;
extern DWORD dwSnapAssistSettings;
extern DWORD dwStartShowClassicMode;
extern HANDLE hWin11AltTabInitialized;

typedef HRESULT(*ImmersiveContextMenuHelper_ApplyOwnerDrawToMenu_t)(HMENU hmenu, HWND hWnd, POINT* pptOrigin, unsigned int icmoFlags, void* srgRenderingData);
extern ImmersiveContextMenuHelper_ApplyOwnerDrawToMenu_t ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc;
typedef void(*ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenu_t)(HMENU hmenu, HWND hwnd);
extern ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenu_t ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc;
typedef LRESULT(*CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc_t)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc_t CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc;

BOOL VnPatchIAT_NonInline(HMODULE hMod, const char* libName, const char* funcName, uintptr_t hookAddr);
POINT GetDefaultWinXPosition(BOOL bUseRcWork, BOOL* lpBottom, BOOL* lpRight, BOOL bAdjust, BOOL bToRight);
BOOL InvokeClockFlyout();
void ReportSuccessfulAnimationPatching();
BOOL IsCrashCounterEnabled();

} // extern "C"

#pragma endregion


#pragma region "twinui.pcshell.dll hooks"

#define LAUNCHERTIP_CLASS_NAME L"LauncherTipWnd"
#define WINX_ADJUST_X 5
#define WINX_ADJUST_Y 5

static INT64(*winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc)(void* _this, INT64 a2, INT a3 ) = nullptr;
static INT64(*CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc)(void* _this, POINT* pt) = nullptr;
static void(*CLauncherTipContextMenu_ExecuteCommandFunc)(void* _this, void* a2) = nullptr;
static void(*CLauncherTipContextMenu_ExecuteShutdownCommandFunc)(void* _this, void* a2) = nullptr;
static INT64(*CLauncherTipContextMenu_GetMenuItemsAsyncFunc)(void* _this, RECT rect, IUnknown** iunk) = nullptr;

HWND hWinXWnd;
HANDLE hIsWinXShown;
HANDLE hWinXThread;

extern "C" LRESULT CALLBACK CLauncherTipContextMenu_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result;

    if (hWnd == archivehWnd && !ArchiveMenuWndProc(
        hWnd, uMsg, wParam, lParam,
        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc,
        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc
    ))
    {
        return 0;
    }

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCs = (CREATESTRUCT*)lParam;
        if (pCs->lpCreateParams)
        {
            *((HWND*)((char*)pCs->lpCreateParams + 0x78)) = hWnd;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCs->lpCreateParams);
            result = DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }
        else
        {
            result = DefWindowProcW(hWnd, uMsg, wParam, lParam);
            //result = 0;
        }
    }
    else
    {
        void* _this = (void*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
        if ((uMsg == WM_DRAWITEM || uMsg == WM_MEASUREITEM) &&
            CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc &&
            CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc(hWnd, uMsg, wParam, lParam))
        {
            result = 0;
        }
        else
        {
            result = DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }
        if (_this)
        {
            if (uMsg == WM_NCDESTROY)
            {
                SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
                *((HWND*)((char*)_this + 0x78)) = nullptr;
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

DWORD ShowLauncherTipContextMenu(LPVOID lpParams)
{
    ShowLauncherTipContextMenuParameters* params = (ShowLauncherTipContextMenuParameters*)lpParams;

    // Adjust this based on info from: CLauncherTipContextMenu::SetSite
    // and CLauncherTipContextMenu::CLauncherTipContextMenu
    // 22000.739: 0xe8
    // 22000.778: 0xf0
    // What has happened, between .739 and .778 is that the launcher tip
    // context menu object now implements a new interface, ILauncherTipContextMenuMigration;
    // thus, members have shifted 8 bytes (one 64-bit value which will hold the
    // address of the vtable for this intf at runtime) to the right;
    // all this intf seems to do, as of now, is to remove some "obsolete" links
    // from the menu (check out "CLauncherTipContextMenu::RunMigrationTasks"); it
    // seems you can disable this by setting a DWORD "WinXMigrationLevel" = 1 in
    // HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced
    int offset_in_class = 0;
    if (global_rovi.dwBuildNumber >= 22621 || (global_rovi.dwBuildNumber == 22000 && global_ubr >= 778))
    {
        offset_in_class = 8;
    }

    static ATOM windowRegistrationAtom = 0;
    if (windowRegistrationAtom == 0)
    {
        WNDCLASS wc = {
            .style = CS_DBLCLKS,
            .lpfnWndProc = CLauncherTipContextMenu_WndProc,
            .hInstance = GetModuleHandleW(nullptr),
            .hCursor = LoadCursorW(nullptr, IDC_ARROW),
            .hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH),
            .lpszClassName = LAUNCHERTIP_CLASS_NAME
        };
        ATOM atom = RegisterClassW(&wc);
        if (atom)
            windowRegistrationAtom = atom;
    }

    hWinXWnd = CreateWindowInBand(
        0,
        MAKEINTATOM(windowRegistrationAtom),
        nullptr,
        WS_POPUP,
        0, 0, 0, 0,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        (char*)params->_this - 0x58,
        7 // ZBID_IMMERSIVE_EDGY
    );
    // DO NOT USE ShowWindow here; it breaks the window order
    // and renders the desktop toggle unusable; but leave
    // SetForegroundWindow as is so that the menu gets dismissed
    // when the user clicks outside it
    //
    // ShowWindow(hWinXWnd, SW_SHOW);
    SetForegroundWindow(hWinXWnd);

    HMENU* phMenu = ((HMENU*)((char*)params->_this + 0xe8 + offset_in_class));
    while (!*phMenu)
    {
        Sleep(1);
    }
    auto finalize = wil::scope_exit([&]() -> void
    {
        params->iunk->Release();
        SendMessageW(hWinXWnd, WM_CLOSE, 0, 0);
        free(params);
        hIsWinXShown = nullptr;
    });
    if (!*phMenu)
    {
        return 0;
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
    menuInfo.cch = (UINT)wcslen(buffer);
    if (bPropertiesInWinX)
    {
        InsertMenuItemW(
            *phMenu,
            GetMenuItemCount(*phMenu) - 1,
            TRUE,
            &menuInfo
        );
        bCreatedMenu = TRUE;
    }

    INT64* unknown_array = nullptr;
    if (bSkinMenus)
    {
        unknown_array = (INT64*)calloc(4, sizeof(INT64));
        if (ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)
        {
            ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
                *phMenu,
                hWinXWnd,
                &(params->point),
                0xc,
                unknown_array
            );
        }
    }

    BOOL res = TrackPopupMenu(
        *phMenu,
        TPM_RETURNCMD | TPM_RIGHTBUTTON | (params->bShouldCenterWinXHorizontally ? TPM_CENTERALIGN : 0),
        params->point.x,
        params->point.y,
        0,
        hWinXWnd,
        nullptr
    );

    if (bSkinMenus)
    {
        if (ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)
        {
            ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
                *phMenu,
                hWinXWnd
            );
        }
        free(unknown_array);
    }

    if (bCreatedMenu)
    {
        RemoveMenu(
            *phMenu,
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
            INT64 info = *(INT64*)((char*)(*(INT64*)((char*)params->_this + 0xa8 + offset_in_class - 0x58)) + (INT64)res * 8 - 8);
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
            INT64 info = *(INT64*)((char*)(*(INT64*)((char*)params->_this + 0xc8 + offset_in_class - 0x58)) + ((INT64)res - 4000) * 8);
            if (CLauncherTipContextMenu_ExecuteShutdownCommandFunc)
            {
                CLauncherTipContextMenu_ExecuteShutdownCommandFunc(
                    (char*)params->_this - 0x58,
                    &info
                );
            }
        }
    }

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
        hWinXThread = nullptr;
    }

    if (!hIsWinXShown)
    {
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
            if (bBottom && IsThemeActive() && PtInRect(&rcHitZone, posCursor) && GetClassWord(WindowFromPoint(point), GCW_ATOM) == RegisterWindowMessageW(L"Start"))
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
                        nullptr,
                        L"Start",
                        nullptr
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
                    if ((int)(point.x - WINX_ADJUST_X * dx) < mi.rcMonitor.left)
                    {
                        xo = TRUE;
                    }
                    if ((int)(point.y + WINX_ADJUST_Y * dy) > mi.rcMonitor.bottom)
                    {
                        yo = TRUE;
                    }
                    POINT ptCursor;
                    GetCursorPos(&ptCursor);
                    if (xo)
                    {
                        ptCursor.x += (int)((WINX_ADJUST_X * 2) * dx);
                    }
                    else
                    {
                        point.x -= (int)(WINX_ADJUST_X * dx);
                    }
                    if (yo)
                    {
                        ptCursor.y -= (int)((WINX_ADJUST_Y * 2) * dy);
                    }
                    else
                    {
                        point.y += (int)(WINX_ADJUST_Y * dy);
                    }
                    SetCursorPos(ptCursor.x, ptCursor.y);
                }
            }
        }
        else
        {
            point = GetDefaultWinXPosition(FALSE, nullptr, nullptr, TRUE, FALSE);
        }

        IUnknown* iunk = nullptr;
        if (CLauncherTipContextMenu_GetMenuItemsAsyncFunc)
        {
            RECT rc = { 0 };
            CLauncherTipContextMenu_GetMenuItemsAsyncFunc(_this, rc, &iunk);
        }
        if (iunk)
        {
            iunk->AddRef();

            ShowLauncherTipContextMenuParameters* params = (ShowLauncherTipContextMenuParameters*)malloc(sizeof(ShowLauncherTipContextMenuParameters));
            params->_this = _this;
            params->point = point;
            params->iunk = iunk;
            params->bShouldCenterWinXHorizontally = bShouldCenterWinXHorizontally;
            hIsWinXShown = CreateThread(nullptr, 0, ShowLauncherTipContextMenu, params, 0, nullptr);
            hWinXThread = hIsWinXShown;
        }
    }
    if (CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc)
    {
        return CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc(_this, pt);
    }
    return 0;
}

extern "C" void ToggleLauncherTipContextMenu()
{
    if (hIsWinXShown)
    {
        SendMessageW(hWinXWnd, WM_CLOSE, 0, 0);
        return;
    }

    HWND hWnd = FindWindowExW(nullptr, nullptr, L"Shell_TrayWnd", nullptr);
    if (!hWnd)
        return;

    hWnd = FindWindowExW(hWnd, nullptr, L"Start", nullptr);
    if (!hWnd)
        return;

    POINT pt = GetDefaultWinXPosition(FALSE, nullptr, nullptr, TRUE, FALSE);
    // Finally implemented a variation of
    // https://github.com/valinet/ExplorerPatcher/issues/3
    // inspired by how the real Start button activates this menu
    // (CPearl::_GetLauncherTipContextMenu)
    // This also works when auto hide taskbar is on (#63)
    ComPtr<IServiceProvider> pImmersiveShell;
    if (SUCCEEDED(CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pImmersiveShell))))
    {
        ComPtr<IImmersiveMonitorManager> pMonitorService;
        if (SUCCEEDED(pImmersiveShell->QueryService(SID_IImmersiveMonitorService, IID_PPV_ARGS(&pMonitorService))))
        {
            ComPtr<ILauncherTipContextMenu> pMenu;
            if (SUCCEEDED(pMonitorService->QueryServiceFromWindow(hWnd, __uuidof(ILauncherTipContextMenu), IID_PPV_ARGS(&pMenu))))
            {
                pMenu->ShowLauncherTipContextMenu(&pt);
            }
        }
    }
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
            hWin11AltTabInitialized = nullptr;
        }

        lRes = ERROR_SUCCESS;
    }

    return lRes;
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

#pragma endregion


#pragma region "Enable old Alt+Tab"

INT64(*twinui_pcshell_IsUndockedAssetAvailableFunc)(INT a1, INT64 a2, INT64 a3, const char* a4);
INT64 twinui_pcshell_IsUndockedAssetAvailableHook(INT a1, INT64 a2, INT64 a3, const char* a4)
{
    // if IsAltTab and AltTabSettings == Windows 10 or sws (Precision Touchpad gesture)
    if (a1 == 1 && (dwAltTabSettings == 3 || dwAltTabSettings == 2))
    {
        return 0;
    }
    // if IsSnapAssist and SnapAssistSettings == Windows 10
    else if (a1 == 4 && dwSnapAssistSettings == 3 && !IsWindows11Version22H2OrHigher())
    {
        return 0;
    }
    // else, show Windows 11 style basically
    else
    {
        if (twinui_pcshell_IsUndockedAssetAvailableFunc)
            return twinui_pcshell_IsUndockedAssetAvailableFunc(a1, a2, a3, a4);
        return 1;
    }
}

INT64(*twinui_pcshell_CMultitaskingViewManager__CreateDCompMTVHostFunc)(INT64 _this, unsigned int a2, INT64 a3, INT64 a4, INT64* a5);
INT64(*twinui_pcshell_CMultitaskingViewManager__CreateXamlMTVHostFunc)(INT64 _this, unsigned int a2, INT64 a3, INT64 a4, INT64* a5);
INT64 twinui_pcshell_CMultitaskingViewManager__CreateXamlMTVHostHook(INT64 _this, unsigned int a2, INT64 a3, INT64 a4, INT64* a5)
{
    if (!twinui_pcshell_IsUndockedAssetAvailableHook(a2, 0, 0, nullptr))
        return twinui_pcshell_CMultitaskingViewManager__CreateDCompMTVHostFunc(_this, a2, a3, a4, a5);
    return twinui_pcshell_CMultitaskingViewManager__CreateXamlMTVHostFunc(_this, a2, a3, a4, a5);
}

#pragma endregion


#pragma region "Fixes related to the removal of STTest feature flag (22621.2134+)"

HRESULT(*twinui_pcshell_PenMenuSystemTrayManager__GetDynamicSystemTrayHeightForMonitorFunc)(IInspectable* _this, HMONITOR hMonitor, float* outHeight);
HRESULT twinui_pcshell_PenMenuSystemTrayManager__GetDynamicSystemTrayHeightForMonitorHook(IInspectable* _this, HMONITOR hMonitor, float* outHeight)
{
    if (bOldTaskbar)
    {
        MONITORINFO mi;
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfoW(hMonitor, &mi);
        *outHeight = (float)(mi.rcMonitor.bottom - mi.rcWork.bottom);
        return S_OK;
    }
    return twinui_pcshell_PenMenuSystemTrayManager__GetDynamicSystemTrayHeightForMonitorFunc(_this, hMonitor, outHeight);
}

static struct
{
    int coroInstance_rcOut; // 22621.1992: 0x10
    int coroInstance_pHardwareConfirmatorHost; // 22621.1992: 0xFD
    int hardwareConfirmatorHost_bIsInLockScreen; // 22621.1992: 0xEC
} g_Moment2PatchOffsets;

#if defined(_M_X64)
inline PBYTE GetTargetOfJzBeforeMe(PBYTE anchor)
{
    // Check big jz
    if (*(anchor - 6) == 0x0F && *(anchor - 5) == 0x84)
        return anchor + *(int*)(anchor - 4);
    // Check small jz
    if (*(anchor - 2) == 0x74)
        return anchor + *(char*)(anchor - 1);
    return nullptr;
}
#endif

// CActionCenterExperienceManager::GetViewPosition() patcher
BOOL Moment2PatchActionCenter(LPMODULEINFO mi)
{
#if defined(_M_X64)
    // Step 1:
    // Scan within the DLL for `*a2 = mi.rcMonitor`.
    // ```0F 10 45 ?? F3 0F 7F ?? 80 ?? ?? ?? 00 00 00 // movups - movdqu - cmp```
    // 22621.1992: 7E2F0
    // 22621.2283: 140D5
    PBYTE rcMonitorAssignment = (PBYTE)FindPattern(mi->lpBaseOfDll, mi->SizeOfImage, "\x0F\x10\x45\x00\xF3\x0F\x7F\x00\x80\x00\x00\x00\x00\x00\x00", "xxx?xxx?x???xxx");
    if (!rcMonitorAssignment) return FALSE;
    printf("[AC] rcMonitorAssignment = %llX\n", rcMonitorAssignment - (PBYTE)mi->lpBaseOfDll);

    // 22621.1992 has a different compiled code structure than 22621.2283 therefore we have to use a different approach:
    // Short circuiting the `if (26008830 is enabled)`.
    // 22621.1992: 7E313
    if (!IsWindows11Version22H2Build2134OrHigher()) // We're on 1413-1992
    {
#if USE_MOMENT_3_FIXES_ON_MOMENT_2
        PBYTE featureCheckJz = rcMonitorAssignment + 35;
        if (*featureCheckJz != 0x0F && *(featureCheckJz + 1) != 0x84) return FALSE;

        DWORD dwOldProtect = 0;
        PBYTE jzAddr = featureCheckJz + 6 + *(DWORD*)(featureCheckJz + 2);
        if (!VirtualProtect(featureCheckJz, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect)) return FALSE;
        featureCheckJz[0] = 0xE9;
        *(DWORD*)(featureCheckJz + 1) = (DWORD)(jzAddr - featureCheckJz - 5);
        VirtualProtect(featureCheckJz, 5, dwOldProtect, &dwOldProtect);
        goto done;
#else
        return FALSE;
#endif
    }

    // Step 2:
    // Copy `*a2 = mi.rcMonitor` into right after the first jz starting from step 1.
    // Find within couple bytes from step 1:
    // ```48 8D // lea```
    // Then offset the first ?? so that it points to mi.rcWork which is 16 bytes after mi.rcMonitor.
    // 22621.2283: 140E6
    PBYTE blockBegin = (PBYTE)FindPattern(rcMonitorAssignment + 1, 32, "\x48\x8D", "xx");
    if (!blockBegin) return FALSE;
    printf("[AC] blockBegin = %llX\n", blockBegin - (PBYTE)mi->lpBaseOfDll);

    // Step 3:
    // Exit the block by writing a long jmp into the address referenced by the jz right before step 3, into right after
    // the 8 bytes `rcMonitor = mi.rcWork` we've written.
    PBYTE blockEnd = GetTargetOfJzBeforeMe(blockBegin);
    if (!blockEnd) return FALSE;
    printf("[AC] blockEnd = %llX\n", blockEnd - (PBYTE)mi->lpBaseOfDll);

    // Execution
    DWORD dwOldProtect = 0;
    if (!VirtualProtect(blockBegin, 8 /**a2 = mi.rcWork*/ + 5 /*jmp*/, PAGE_EXECUTE_READWRITE, &dwOldProtect)) return FALSE;

    // Step 2
    memcpy(blockBegin, rcMonitorAssignment, 8);
    blockBegin[3] += offsetof(MONITORINFO, rcWork) - offsetof(MONITORINFO, rcMonitor);

    // Step 3
    PBYTE jmpToEnd = blockBegin + 8;
    jmpToEnd[0] = 0xE9;
    *(DWORD*)(jmpToEnd + 1) = (DWORD)(blockEnd - jmpToEnd - 5);

    VirtualProtect(blockBegin, 8 + 5, dwOldProtect, &dwOldProtect);
    goto done;

done:
    printf("[AC] Patched!\n");
    return TRUE;
#else
    return FALSE;
#endif
}

// CControlCenterExperienceManager::PositionView() patcher
BOOL Moment2PatchControlCenter(LPMODULEINFO mi)
{
#if defined(_M_X64)
    // Step 1:
    // Scan within the DLL for `rcMonitor = mi.rcMonitor`.
    // ```0F 10 44 24 ?? F3 0F 7F 44 24 ?? 80 // movups - movdqu - cmp```
    // 22621.1992: 4B35B
    // 22621.2283: 65C5C
    PBYTE rcMonitorAssignment = (PBYTE)FindPattern(mi->lpBaseOfDll, mi->SizeOfImage, "\x0F\x10\x44\x24\x00\xF3\x0F\x7F\x44\x24\x00\x80", "xxxx?xxxxx?x");
    if (!rcMonitorAssignment) return FALSE;
    printf("[CC] rcMonitorAssignment = %llX\n", rcMonitorAssignment - (PBYTE)mi->lpBaseOfDll);

    // Step 2:
    // Scan within the function for the 10 bytes long `rcMonitor = mi.rcWork`.
    // This pattern applies to both ControlCenter and ToastCenter.
    // ```0F 10 45 ?? F3 0F 7F 44 24 ?? 48 // movups - movdqu - test```
    // 22621.1992: 4B3FD and 4B418 (The second one is compiled out in later builds)
    // 22621.2283: 65CE6
    PBYTE rcWorkAssignment = (PBYTE)FindPattern(rcMonitorAssignment + 1, 256, "\x0F\x10\x45\x00\xF3\x0F\x7F\x44\x24\x00\x48", "xxx?xxxxx?x");
    if (!rcWorkAssignment) return FALSE;
    printf("[CC] rcWorkAssignment = %llX\n", rcWorkAssignment - (PBYTE)mi->lpBaseOfDll);

    // Step 3:
    // Copy the `rcMonitor = mi.rcWork` into right after the first jz starting from step 1.
    // Find within couple bytes from step 1:
    // ```48 8D // lea```
    // 22621.1992: 4B373
    // 22621.2283: 65C74
    PBYTE blockBegin = (PBYTE)FindPattern(rcMonitorAssignment + 1, 32, "\x48\x8D", "xx");
    if (!blockBegin) return FALSE;
    printf("[CC] blockBegin = %llX\n", blockBegin - (PBYTE)mi->lpBaseOfDll);

    // Step 4:
    // Exit the block by writing a long jmp into the address referenced by the jz right before step 3, into right after
    // the 10 bytes `rcMonitor = mi.rcWork` we've written.
    PBYTE blockEnd = GetTargetOfJzBeforeMe(blockBegin);
    if (!blockEnd) return FALSE;
    printf("[CC] blockEnd = %llX\n", blockEnd - (PBYTE)mi->lpBaseOfDll);

    // Execution
    DWORD dwOldProtect = 0;
    if (!VirtualProtect(blockBegin, 10 /*rcMonitor = mi.rcWork*/ + 5 /*jmp*/, PAGE_EXECUTE_READWRITE, &dwOldProtect)) return FALSE;

    // Step 2
    memcpy(blockBegin, rcWorkAssignment, 10);

    // Step 3
    PBYTE jmpToEnd = blockBegin + 10;
    jmpToEnd[0] = 0xE9;
    *(DWORD*)(jmpToEnd + 1) = (DWORD)(blockEnd - jmpToEnd - 5);

    VirtualProtect(blockBegin, 10 + 5, dwOldProtect, &dwOldProtect);

    printf("[CC] Patched!\n");
    return TRUE;
#else
    return FALSE;
#endif
}

// CToastCenterExperienceManager::PositionView() patcher
BOOL Moment2PatchToastCenter(LPMODULEINFO mi)
{
#if defined(_M_X64)
    // Step 1:
    // Scan within the DLL for `rcMonitor = mi.rcMonitor`.
    //
    // Pattern 1:
    // Will have a match if CToastCenterExperienceManager::ShouldShowWithinWorkArea() is present.
    // ```0F 10 45 ?? ?? 0F 7F 44 24 ?? 48 8B CF // movups - movdqu - mov```
    // 22621.1992: 40CE8
    // 22621.2283: 501DB
    //
    // Pattern 2:
    // Will have a match if CToastCenterExperienceManager::ShouldShowWithinWorkArea() is inlined.
    // ```0F 10 45 ?? ?? 0F 7F 44 24 ?? 44 38 // movups - movdqu - cmp```
    // 25951.1000: 36B2C4
    //
    // Pattern 3:
    // Same as pattern 1, but different length of the movdqu instruction.
    // ```0F 10 45 ?? ?? 0F 7F 45 ?? 48 8B CF // movups - movdqu - mov```
    // 22621.3066: 3DC340
    //
    // Pattern 4:
    // Same as pattern 2, but different length of the movdqu instruction.
    // ```0F 10 45 ?? ?? 0F 7F 45 ?? 44 38 // movups - movdqu - cmp```
    // No matches yet, just in case.
    int assignmentSize = 10;
    PBYTE rcMonitorAssignment = (PBYTE)FindPattern(mi->lpBaseOfDll, mi->SizeOfImage, "\x0F\x10\x45\x00\x00\x0F\x7F\x44\x24\x00\x48\x8B\xCF", "xxx??xxxx?xxx");
    if (!rcMonitorAssignment)
    {
        rcMonitorAssignment = (PBYTE)FindPattern(mi->lpBaseOfDll, mi->SizeOfImage, "\x0F\x10\x45\x00\x00\x0F\x7F\x44\x24\x00\x44\x38", "xxx??xxxx?xx");
        if (!rcMonitorAssignment)
        {
            assignmentSize = 9;
            rcMonitorAssignment = (PBYTE)FindPattern(mi->lpBaseOfDll, mi->SizeOfImage, "\x0F\x10\x45\x00\x00\x0F\x7F\x45\x00\x48\x8B\xCF", "xxx??xxx?xxx");
            if (!rcMonitorAssignment)
            {
                rcMonitorAssignment = (PBYTE)FindPattern(mi->lpBaseOfDll, mi->SizeOfImage, "\x0F\x10\x45\x00\x00\x0F\x7F\x45\x00\x44\x38", "xxx??xxx?xx");
                if (!rcMonitorAssignment) return FALSE;
            }
        }
    }
    printf("[TC] rcMonitorAssignment = %llX\n", rcMonitorAssignment - (PBYTE)mi->lpBaseOfDll);

    // Step 2:
    // Copy the `rcMonitor = mi.rcMonitor` into right after the first jz starting from step 1.
    // Find within couple bytes from step 1:
    // ```48 8D // lea```
    // Then offset the first ?? so that it points to mi.rcWork which is 16 bytes after mi.rcMonitor.
    // 22621.1992: 40D02
    // 22621.2283: 501F5
    PBYTE blockBegin = (PBYTE)FindPattern(rcMonitorAssignment + 1, 32, "\x48\x8D", "xx");
    if (!blockBegin) return FALSE;
    printf("[TC] blockBegin = %llX\n", blockBegin - (PBYTE)mi->lpBaseOfDll);

    // Step 3:
    // Exit the block by writing a long jmp into the address referenced by the jz right before step 3, into right after
    // the <assignmentSize> bytes `rcMonitor = mi.rcWork` we've written.
    //
    // Note: We are skipping EdgeUI calls here.
    PBYTE blockEnd = GetTargetOfJzBeforeMe(blockBegin);
    if (!blockEnd) return FALSE;
    printf("[TC] blockEnd = %llX\n", blockEnd - (PBYTE)mi->lpBaseOfDll);

    // Execution
    DWORD dwOldProtect = 0;
    if (!VirtualProtect(blockBegin, assignmentSize /*rcMonitor = mi.rcWork*/ + 5 /*jmp*/, PAGE_EXECUTE_READWRITE, &dwOldProtect)) return FALSE;

    // Step 2
    memcpy(blockBegin, rcMonitorAssignment, assignmentSize);
    blockBegin[3] += offsetof(MONITORINFO, rcWork) - offsetof(MONITORINFO, rcMonitor);

    // Step 3
    PBYTE jmpToEnd = blockBegin + assignmentSize;
    jmpToEnd[0] = 0xE9;
    *(DWORD*)(jmpToEnd + 1) = (DWORD)(blockEnd - jmpToEnd - 5);

    VirtualProtect(blockBegin, assignmentSize + 5, dwOldProtect, &dwOldProtect);

    printf("[TC] Patched!\n");
    return TRUE;
#else
    return FALSE;
#endif
}

// TaskViewFrame::RuntimeClassInitialize() patcher
BOOL Moment2PatchTaskView(LPMODULEINFO mi)
{
#if defined(_M_X64)
    /***
    If we're using the old taskbar, it'll be stuck in an infinite loading since it's waiting for the new one to respond.
    Let's safely skip those by NOPing the `TaskViewFrame::UpdateWorkAreaAsync()` and `WaitForCompletion()` calls, and
    turning off the COM object cleanup.

    Step 1:
    Scan within the DLL to find the beginning, which is the preparation of the 1st call.
    It should be 4C 8B or 4D 8B (mov r8, ...).
    For the patterns, they're +1 from the result since it can be either of those.

    Pattern 1:
    ```8B ?? 48 8D 55 ??    48 8B ?? E8 ?? ?? ?? ?? 48 8B 08 E8```
    22621.1992: 7463C
    22621.2134: 3B29C

    Pattern 2:
    ```8B ?? 48 8D 54 24 ?? 48 8B ?? E8 ?? ?? ?? ?? 48 8B 08 E8```
    22621.2283: 24A1D2

    Step 2:
    In place of the 1st call's call op (E8), overwrite it with a code to set the value of the com_ptr passed into the
    2nd argument (rdx) to 0. This is to skip the cleanup that happens right after the 2nd call.
    ```48 C7 02 00 00 00 00 mov qword ptr [rdx], 0```
    Start from -13 of the byte after 2nd call's end.
    22621.1992: 74646
    22621.2134: 3B2A6
    22621.2283: 24A1DD

    Step 3:
    NOP the rest of the 2nd call.

    Summary:
    ```
       48 8B ?? 48 8D 55 ??    48 8B ?? E8 ?? ?? ?? ?? 48 8B 08 E8 ?? ?? ?? ?? // Pattern 1
       48 8B ?? 48 8D 54 24 ?? 48 8B ?? E8 ?? ?? ?? ?? 48 8B 08 E8 ?? ?? ?? ?? // Pattern 2
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ^^^^^^^^^^^^^^^^^^^^^^^
       1st: TaskViewFrame::UpdateWorkAreaAsync()       2nd: WaitForCompletion()
       48 8B ?? 48 8D 54 24 ?? 48 8B ?? 48 C7 02 00 00 00 00 90 90 90 90 90 90 // Result according to Pattern 2
       -------------------------------- xxxxxxxxxxxxxxxxxxxx xxxxxxxxxxxxxxxxx
       We need rdx                      Step 2               Step 3
    ```

    Notes:
    - In 22621.1992 and 22621.2134, `~AsyncOperationCompletedHandler()` is inlined, while it is not in 22621.2283. We
      can see `unconditional_release_ref()` calls right in `RuntimeClassInitialize()` of 1992 and 2134.
    - In 22621.2134, there is `33 FF xor edi, edi` before the jz for the inlined cleanup. The value of edi is used in
      two more cleanup calls after our area of interest (those covered by twoCallsLength), therefore we can't just NOP
      everything. And I think detecting such things is too much work.
    ***/

    int twoCallsLength = 1 + 18 + 4; // 4C/4D + pattern length + 4 bytes for the 2nd call's call address
    PBYTE firstCallPrep = (PBYTE)FindPattern(mi->lpBaseOfDll, mi->SizeOfImage, "\x8B\x00\x48\x8D\x55\x00\x48\x8B\x00\xE8\x00\x00\x00\x00\x48\x8B\x08\xE8", "x?xxx?xx?x????xxxx");
    if (!firstCallPrep)
    {
        twoCallsLength += 1; // Add 1 to the pattern length
        firstCallPrep = (PBYTE)FindPattern(mi->lpBaseOfDll, mi->SizeOfImage, "\x8B\x00\x48\x8D\x54\x24\x00\x48\x8B\x00\xE8\x00\x00\x00\x00\x48\x8B\x08\xE8", "x?xxxx?xx?x????xxxx");
        if (!firstCallPrep) return FALSE;
    }
    firstCallPrep -= 1; // Point to the 4C/4D
    printf("[TV] firstCallPrep = %llX\n", firstCallPrep - (PBYTE)mi->lpBaseOfDll);

    PBYTE firstCallCall = firstCallPrep + twoCallsLength - 13;
    printf("[TV] firstCallCall = %llX\n", firstCallCall - (PBYTE)mi->lpBaseOfDll);

    PBYTE nopBegin = firstCallCall + 7;

    // Execution
    DWORD dwOldProtect = 0;
    if (!VirtualProtect(firstCallPrep, twoCallsLength, PAGE_EXECUTE_READWRITE, &dwOldProtect)) return FALSE;
    const BYTE step2Payload[] = { 0x48, 0xC7, 0x02, 0x00, 0x00, 0x00, 0x00 };
    memcpy(firstCallCall, step2Payload, sizeof(step2Payload));
    memset(nopBegin, 0x90, twoCallsLength - (nopBegin - firstCallPrep));
    VirtualProtect(firstCallPrep, twoCallsLength, dwOldProtect, &dwOldProtect);

    printf("[TV] Patched!\n");
    return TRUE;
#else
    return FALSE;
#endif
}

// Reimplementation of HardwareConfirmatorHost::GetDisplayRect()
void WINAPI HardwareConfirmatorShellcode(PBYTE pCoroInstance)
{
    PBYTE pHardwareConfirmatorHost = *(PBYTE*)(pCoroInstance + g_Moment2PatchOffsets.coroInstance_pHardwareConfirmatorHost);

    RECT rc;
    HMONITOR hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTOPRIMARY);

    ComPtr<IServiceProvider> pImmersiveShell;
    if (SUCCEEDED(CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pImmersiveShell))))
    {
        ComPtr<IImmersiveMonitorManager> pMonitorService;
        if (SUCCEEDED(pImmersiveShell->QueryService(SID_IImmersiveMonitorService, IID_PPV_ARGS(&pMonitorService))))
        {
            ComPtr<IEdgeUiManager> pEdgeUiManager;
            if (SUCCEEDED(pMonitorService->QueryService(hMonitor, SID_EdgeUi, IID_PPV_ARGS(&pEdgeUiManager))))
            {
                if (*(pHardwareConfirmatorHost + g_Moment2PatchOffsets.hardwareConfirmatorHost_bIsInLockScreen))
                {
                    // Lock screen
                    MONITORINFO mi;
                    mi.cbSize = sizeof(MONITORINFO);
                    if (GetMonitorInfoW(hMonitor, &mi))
                        rc = mi.rcMonitor;
                }
                else
                {
                    // Desktop
                    LOG_IF_FAILED(pEdgeUiManager->GetAutohideImmuneWorkArea(&rc));
                }

                ABI::Windows::Foundation::Rect* out = (ABI::Windows::Foundation::Rect*)(pCoroInstance + g_Moment2PatchOffsets.coroInstance_rcOut);
                out->X = (float)rc.left;
                out->Y = (float)rc.top;
                out->Width = (float)(rc.right - rc.left);
                out->Height = (float)(rc.bottom - rc.top);
            }
        }
    }
}

// [HardwareConfirmatorHost::GetDisplayRectAsync$_ResumeCoro$1() patcher
BOOL Moment2PatchHardwareConfirmator(LPMODULEINFO mi)
{
#if defined(_M_X64)
    // Find required offsets

    // pHardwareConfirmatorHost and bIsInLockScreen:
    // Find in GetDisplayRectAsync$_ResumeCoro$1, inside `case 4:`
    //
    // 48 8B 83 ED 00 00 00     mov     rax, [rbx+0EDh]
    //          ^^^^^^^^^^^ pHardwareConfirmatorHost
    // 8A 80 EC 00 00 00        mov     al, [rax+0ECh]
    //       ^^^^^^^^^^^ bIsInLockScreen
    //
    // if ( ADJ(this)->pHardwareConfirmatorHost->bIsInLockScreen )
    // if ( *(_BYTE *)(*(_QWORD *)(this + 237) + 236i64) ) // 22621.2283
    //                                    ^ HCH  ^ bIsInLockScreen
    //
    // 22621.2134: 1D55D
    PBYTE match1 = (PBYTE)FindPattern(mi->lpBaseOfDll, mi->SizeOfImage, "\x48\x8B\x83\x00\x00\x00\x00\x8A\x80", "xxx????xx");
    printf("[HC] match1 = %llX\n", match1 - (PBYTE)mi->lpBaseOfDll);
    if (!match1) return FALSE;
    g_Moment2PatchOffsets.coroInstance_pHardwareConfirmatorHost = *(int*)(match1 + 3);
    g_Moment2PatchOffsets.hardwareConfirmatorHost_bIsInLockScreen = *(int*)(match1 + 9);

    // coroInstance_rcOut:
    // Also in GetDisplayRectAsync$_ResumeCoro$1, through `case 4:`
    // We also use this as the point to jump to, which is the code to set the rect and finish the coroutine.
    //
    // v27 = *(_OWORD *)(this + 16);
    // *(_OWORD *)(this - 16) = v27;
    // if ( winrt_suspend_handler ) ...
    //
    // 0F 10 43 10              movups  xmm0, xmmword ptr [rbx+10h]
    //          ^^ coroInstance_rcOut
    // 0F 11 84 24 D0 00 00 00  movups  [rsp+158h+var_88], xmm0
    //
    // 22621.2134: 1D624
    PBYTE match2 = (PBYTE)FindPattern(mi->lpBaseOfDll, mi->SizeOfImage, "\x0F\x10\x43\x00\x0F\x11\x84\x24", "xxx?xxxx");
    printf("[HC] match2 = %llX\n", match2 - (PBYTE)mi->lpBaseOfDll);
    if (!match2) return FALSE;
    g_Moment2PatchOffsets.coroInstance_rcOut = *(match2 + 3);

    // Find where to put the shellcode
    // We'll overwrite from this position:
    //
    // *(_OWORD *)(this + 32) = 0i64;
    // *(_QWORD *)(this + 48) = MonitorFromRect((LPCRECT)(this + 32), 1u);
    //
    // 22621.2134: 1D21E
    PBYTE writeAt = (PBYTE)FindPattern(mi->lpBaseOfDll, mi->SizeOfImage, "\x48\x8D\x4B\x00\x0F", "xxx?x");
    if (!writeAt) return FALSE;
    printf("[HC] writeAt = %llX\n", writeAt - (PBYTE)mi->lpBaseOfDll);

    // In 22621.2134+, after our jump location there is a cleanup for something we skipped. NOP them.
    // From match2, bytes +17 until +37, which is 21 bytes to be NOP'd.
    // 22621.2134: 1D635-1D64A
    PBYTE cleanupBegin = nullptr, cleanupEnd = nullptr;
    if (IsWindows11Version22H2Build2134OrHigher())
    {
        cleanupBegin = match2 + 17;
        cleanupEnd = match2 + 38; // Exclusive
        printf("[HC] cleanup = %llX-%llX\n", cleanupBegin - (PBYTE)mi->lpBaseOfDll, cleanupEnd - (PBYTE)mi->lpBaseOfDll);
        if (*cleanupBegin != 0x49 || *cleanupEnd != 0x90 /*Already NOP here*/) return FALSE;
    }

    // Craft the shellcode
    BYTE shellcode[] = {
        // lea rcx, [rbx+0] ; rbx is the `this` which is the instance of the coro, we pass it to our function
        0x48, 0x8D, 0x0B,
        // mov rax, 1111111111111111h ; placeholder for the address of HardwareConfirmatorShellcode
        0x48, 0xB8, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        // call rax
        0xFF, 0xD0
    };

    uintptr_t pattern = 0x1111111111111111;
    *(uintptr_t*)(memmem(shellcode, sizeof(shellcode), &pattern, sizeof(uintptr_t))) = (uintptr_t)HardwareConfirmatorShellcode;

    // Execution
    DWORD dwOldProtect = 0;
    SIZE_T totalSize = sizeof(shellcode) + 5;
    if (!VirtualProtect(writeAt, totalSize, PAGE_EXECUTE_READWRITE, &dwOldProtect)) return FALSE;
    memcpy(writeAt, shellcode, sizeof(shellcode));
    PBYTE jmpLoc = writeAt + sizeof(shellcode);
    jmpLoc[0] = 0xE9;
    *(DWORD*)(jmpLoc + 1) = (DWORD)(match2 - jmpLoc - 5);
    VirtualProtect(writeAt, totalSize, dwOldProtect, &dwOldProtect);

    if (cleanupBegin)
    {
        dwOldProtect = 0;
        if (!VirtualProtect(cleanupBegin, cleanupEnd - cleanupBegin, PAGE_EXECUTE_READWRITE, &dwOldProtect)) return FALSE;
        memset(cleanupBegin, 0x90, cleanupEnd - cleanupBegin);
        VirtualProtect(cleanupBegin, cleanupEnd - cleanupBegin, dwOldProtect, &dwOldProtect);
    }

    printf("[HC] Patched!\n");
    return TRUE;
#else
    return FALSE;
#endif
}

#pragma endregion


#pragma region "Fix broken Windows 10 start menu positioning issues caused by 44656322"

// Reverts 44656322's effects on the start menu
extern "C" HRESULT CStartExperienceManager_GetMonitorInformationHook(void* _this, CSingleViewShellExperience* experience, RECT* rcOutWorkArea, EDGEUI_TRAYSTUCKPLACE* outTrayStuckPlace, bool* bOutRtl, HMONITOR* hOutMonitor)
{
    *rcOutWorkArea = {};
    *outTrayStuckPlace = EUITSP_BOTTOM;
    *bOutRtl = false;
    if (hOutMonitor)
        *hOutMonitor = nullptr;

    ComPtr<IServiceProvider> spImmersiveShellServiceProvider;
    RETURN_IF_FAILED(CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&spImmersiveShellServiceProvider)));

    ComPtr<IImmersiveLauncher> spImmersiveLauncher;
    RETURN_IF_FAILED(spImmersiveShellServiceProvider->QueryService(SID_ImmersiveLauncher, IID_PPV_ARGS(&spImmersiveLauncher)));

    ComPtr<IImmersiveMonitor> spImmersiveMonitor;
    HRESULT hr = spImmersiveLauncher->GetMonitor(&spImmersiveMonitor);
    if (FAILED(hr))
        return hr;

    HMONITOR hMonitor = nullptr;
    if (hOutMonitor)
        hr = spImmersiveMonitor->GetHandle(&hMonitor);

    if (FAILED(hr))
        return hr;

    ComPtr<IEdgeUiManager> spEdgeUiManager;
    hr = IUnknown_QueryService(spImmersiveMonitor.Get(), SID_EdgeUi, IID_PPV_ARGS(&spEdgeUiManager));
    if (FAILED(hr))
        return hr;

    EDGEUI_TRAYSTUCKPLACE trayStuckPlace;
    RETURN_IF_FAILED(spEdgeUiManager->GetTrayStuckPlace(&trayStuckPlace));

    HWND hwndTray = FindWindowW(L"Shell_TrayWnd", nullptr);

    RECT rcWork;
    if (hwndTray && GetPropW(hwndTray, L"IsAutoHideEnabled"))
    {
        RETURN_IF_FAILED(spEdgeUiManager->GetAutohideImmuneWorkArea(&rcWork));
    }
    else
    {
        RETURN_IF_FAILED(spImmersiveMonitor->GetWorkArea(&rcWork));
    }

    *rcOutWorkArea = rcWork;
    *outTrayStuckPlace = trayStuckPlace;
    *bOutRtl = Mirror_IsThreadRTL() != FALSE;
    if (hOutMonitor)
        *hOutMonitor = hMonitor;

    return S_OK;
}

#pragma endregion


#pragma region "Fix Windows 10 start menu animation on 22000.65+"

static struct
{
    int startExperienceManager_IStartExperienceManager;
    int startExperienceManager_SingleViewShellExperienceEventHandler;
    int startExperienceManager_singleViewShellExperience;
    int startExperienceManager_openingAnimation;
    int startExperienceManager_closingAnimation;
    int startExperienceManager_bTransitioningToCortana;
} g_SMAnimationPatchOffsets;

// Names taken from Windows.UI.Xaml.pdb, only defining the used ones
enum DWMTRANSITION_TARGET
{
    DWMTARGET_LAUNCHERFLYOUTTOLEFT = 0x4D,
    DWMTARGET_LAUNCHERFLYOUTTORIGHT = 0x4E,
    DWMTARGET_LAUNCHERFLYOUTTOTOP = 0x4F,
    DWMTARGET_LAUNCHERFLYOUTTOBOTTOM = 0x50,
    DWMTARGET_LAUNCHERFLYOUT = 0x51,
    DWMTARGET_LAUNCHERFULLSCREEN = 0x52,
};

DEFINE_ENUM_FLAG_OPERATORS(DWMTRANSITION_TARGET);

extern HRESULT CStartExperienceManager_GetMonitorInformationHook(void* _this, CSingleViewShellExperience* experience, RECT* rcOutWorkArea, EDGEUI_TRAYSTUCKPLACE* outTrayStuckPlace, bool* bOutRtl, HMONITOR* hOutMonitor);
HRESULT(*CStartExperienceManager_GetMonitorInformationFunc)(void* _this, void* experience, RECT* rcOutWorkArea, EDGEUI_TRAYSTUCKPLACE* outTrayStuckPlace, bool* bOutRtl, HMONITOR* hOutMonitor);
HRESULT(*CExperienceManagerAnimationHelper_BeginFunc)(void* _this, void* pViewWrapper, DWMTRANSITION_TARGET target, const RECT* prcBeginSource, const RECT* prcBeginDestination, const RECT* prcEndSource, const RECT* prcEndDestination, const RECT* prcClip);
HRESULT(*CExperienceManagerAnimationHelper_EndFunc)(void* _this);

HRESULT(*CStartExperienceManager_OnViewUncloakingFunc)(void* eventHandler, CSingleViewShellExperience* pSender);
HRESULT CStartExperienceManager_OnViewUncloakingHook(void* eventHandler, CSingleViewShellExperience* pSender)
{
    PBYTE _this = (PBYTE)eventHandler - g_SMAnimationPatchOffsets.startExperienceManager_SingleViewShellExperienceEventHandler;

    RECT rcWorkArea;
    EDGEUI_TRAYSTUCKPLACE tsp;
    bool bRtl;
    if (SUCCEEDED(CStartExperienceManager_GetMonitorInformationHook(_this, pSender, &rcWorkArea, &tsp, &bRtl, NULL)) && dwStartShowClassicMode)
    {
        DWMTRANSITION_TARGET target = DWMTARGET_LAUNCHERFLYOUT;
        if (pSender->_fullScreen)
            target = DWMTARGET_LAUNCHERFULLSCREEN;
        else if (tsp == EUITSP_LEFT)
            target = DWMTARGET_LAUNCHERFLYOUTTORIGHT;
        else if (tsp == EUITSP_TOP)
            target = DWMTARGET_LAUNCHERFLYOUTTOBOTTOM;
        else if (tsp == EUITSP_RIGHT)
            target = DWMTARGET_LAUNCHERFLYOUTTOLEFT;
        else if (tsp == EUITSP_BOTTOM)
            target = DWMTARGET_LAUNCHERFLYOUTTOTOP;

        CExperienceManagerAnimationHelper_BeginFunc(
            _this + g_SMAnimationPatchOffsets.startExperienceManager_openingAnimation,
            pSender->_viewWrapper,
            (DWMTRANSITION_TARGET)(target | 0x200000), nullptr, nullptr, nullptr, nullptr, &rcWorkArea);
    }

    if (global_rovi.dwBuildNumber >= 25169)
    {
        // Patch hardcoded EUITSP_BOTTOM present in the original function with the correct value
#if defined(_M_X64)
        static int* rgpConstants[2];
        if (!rgpConstants[0] && rgpConstants[0] != (int*)-1)
        {
            // 03 00 00 00
            PBYTE match = (PBYTE)FindPattern(CStartExperienceManager_OnViewUncloakingFunc, 80, "\x03\x00\x00\x00", "xxxx");
            rgpConstants[0] = match ? (int*)match : (int*)-1;
            if (match)
            {
                match = (PBYTE)FindPattern(match + 4, 40, "\x03\x00\x00\x00", "xxxx");
                rgpConstants[1] = match ? (int*)match : (int*)-1;
            }
        }
        for (int i = 0; i < ARRAYSIZE(rgpConstants); i++)
        {
            if (rgpConstants[i] && rgpConstants[i] != (int*)-1)
            {
                DWORD dwOldProtect;
                if (VirtualProtect(rgpConstants[i], 4, PAGE_EXECUTE_READWRITE, &dwOldProtect))
                {
                    *rgpConstants[i] = tsp;
                    VirtualProtect(rgpConstants[i], 4, dwOldProtect, &dwOldProtect);
                }
            }
        }
#elif defined(_M_ARM64)
        static DWORD* rgpInsnMovs[2];
        if (!rgpInsnMovs[0] && rgpInsnMovs[0] != (DWORD*)-1)
        {
            // 68 00 80 52
            PBYTE match = (PBYTE)FindPattern(CStartExperienceManager_OnViewUncloakingFunc, 160, "\x68\x00\x80\x52", "xxxx");
            rgpInsnMovs[0] = match ? (DWORD*)match : (DWORD*)-1;
            if (match)
            {
                match = (PBYTE)FindPattern(match + 4, 40, "\x68\x00\x80\x52", "xxxx");
                rgpInsnMovs[1] = match ? (DWORD*)match : (DWORD*)-1;
            }
        }
        for (int i = 0; i < ARRAYSIZE(rgpInsnMovs); i++)
        {
            if (rgpInsnMovs[i] && rgpInsnMovs[i] != (DWORD*)-1)
            {
                DWORD dwOldProtect;
                if (VirtualProtect(rgpInsnMovs[i], 4, PAGE_EXECUTE_READWRITE, &dwOldProtect))
                {
                    if (ARM64_IsInRange(tsp, 16))
                    {
                        DWORD insn = *rgpInsnMovs[i];
                        int imm16Mask = ((1 << 16) - 1) << 5;
                        insn &= ~imm16Mask; // clear imm16
                        insn |= (tsp << 5) & imm16Mask; // set imm16
                        *rgpInsnMovs[i] = insn;
                    }
                    VirtualProtect(rgpInsnMovs[i], 4, dwOldProtect, &dwOldProtect);
                }
            }
        }
#endif
    }

    return CStartExperienceManager_OnViewUncloakingFunc(eventHandler, pSender);
}

HRESULT(*CStartExperienceManager_OnViewUncloakedFunc)(void* eventHandler, CSingleViewShellExperience* pSender);
HRESULT CStartExperienceManager_OnViewUncloakedHook(void* eventHandler, CSingleViewShellExperience* pSender)
{
    PBYTE _this = (PBYTE)eventHandler - g_SMAnimationPatchOffsets.startExperienceManager_SingleViewShellExperienceEventHandler;

    if (dwStartShowClassicMode)
    {
        CExperienceManagerAnimationHelper_EndFunc(_this + g_SMAnimationPatchOffsets.startExperienceManager_openingAnimation);
    }

    return CStartExperienceManager_OnViewUncloakedFunc(eventHandler, pSender);
}

HRESULT(*CStartExperienceManager_OnViewCloakingFunc)(void* eventHandler, CSingleViewShellExperience* pSender);
HRESULT CStartExperienceManager_OnViewCloakingHook(void* eventHandler, CSingleViewShellExperience* pSender)
{
    PBYTE _this = (PBYTE)eventHandler - g_SMAnimationPatchOffsets.startExperienceManager_SingleViewShellExperienceEventHandler;

    bool bTransitioningToCortana = *(_this + g_SMAnimationPatchOffsets.startExperienceManager_bTransitioningToCortana);
    if (!bTransitioningToCortana && dwStartShowClassicMode)
    {
        RECT rcWorkArea;
        EDGEUI_TRAYSTUCKPLACE tsp;
        bool bRtl;
        HMONITOR hMonitor;
        if (SUCCEEDED(CStartExperienceManager_GetMonitorInformationHook(_this, pSender, &rcWorkArea, &tsp, &bRtl, &hMonitor)))
        {
            DWMTRANSITION_TARGET target = DWMTARGET_LAUNCHERFLYOUT;
            if (pSender->_fullScreen)
                target = DWMTARGET_LAUNCHERFULLSCREEN;
            else if (tsp == EUITSP_LEFT)
                target = DWMTARGET_LAUNCHERFLYOUTTOLEFT;
            else if (tsp == EUITSP_TOP)
                target = DWMTARGET_LAUNCHERFLYOUTTOTOP;
            else if (tsp == EUITSP_RIGHT)
                target = DWMTARGET_LAUNCHERFLYOUTTORIGHT;
            else if (tsp == EUITSP_BOTTOM)
                target = DWMTARGET_LAUNCHERFLYOUTTOBOTTOM;

            CExperienceManagerAnimationHelper_BeginFunc(
                _this + g_SMAnimationPatchOffsets.startExperienceManager_closingAnimation,
                pSender->_viewWrapper,
                (DWMTRANSITION_TARGET)(target | 0x200000), nullptr, nullptr, nullptr, nullptr, &rcWorkArea);
        }
    }

    return CStartExperienceManager_OnViewCloakingFunc(eventHandler, pSender);
}

HRESULT(*CStartExperienceManager_OnViewHiddenFunc)(void* eventHandler, CSingleViewShellExperience* pSender);
HRESULT CStartExperienceManager_OnViewHiddenHook(void* eventHandler, CSingleViewShellExperience* pSender)
{
    PBYTE _this = (PBYTE)eventHandler - g_SMAnimationPatchOffsets.startExperienceManager_SingleViewShellExperienceEventHandler;

    bool bTransitioningToCortana = *(_this + g_SMAnimationPatchOffsets.startExperienceManager_bTransitioningToCortana);
    if (!bTransitioningToCortana && dwStartShowClassicMode)
    {
        CExperienceManagerAnimationHelper_EndFunc(_this + g_SMAnimationPatchOffsets.startExperienceManager_closingAnimation);
    }

    return CStartExperienceManager_OnViewHiddenFunc(eventHandler, pSender);
}

BOOL FixStartMenuAnimation(LPMODULEINFO mi)
{
    // The idea here is to re-add the code that got removed in 22000.65+. We can see that "STest03" is the feature flag
    // that experiments with the new start menu. So, because in 22000.51 one can enable the old start menu with proper
    // behavior by setting the Start_ShowClassicMode registry value to 1, and there is a convenient function called
    // `StartDocked::ShouldUseStartDocked()`, we crosscheck the removed code and piece together a patch for proper
    // animations on 22000.65+.

    g_SMAnimationPatchOffsets.startExperienceManager_IStartExperienceManager = 0x28;
    g_SMAnimationPatchOffsets.startExperienceManager_SingleViewShellExperienceEventHandler = 0x60;

    // ### CStartExperienceManager::`vftable'{for `SingleViewShellExperienceEventHandler'}
#if defined(_M_X64)
    // ```
    // 48 89 46 48 48 8D 05 ?? ?? ?? ?? 48 89 46 60 48 8D 4E 68 E8
    //                      ^^^^^^^^^^^
    // ```
    // Ref: CStartExperienceManager::CStartExperienceManager()
    PBYTE matchVtable = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x48\x89\x46\x48\x48\x8D\x05\x00\x00\x00\x00\x48\x89\x46\x60\x48\x8D\x4E\x68\xE8",
        "xxxxxxx????xxxxxxxxx"
    );
    if (matchVtable)
    {
        matchVtable += 4;
        matchVtable += 7 + *(int*)(matchVtable + 3);
    }
#elif defined(_M_ARM64)
    // ```
    // 69 22 04 A9 ?? ?? 00 ?? 08 81 ?? 91 60 A2 01 91 68 32 00 F9
    //             ^^^^^^^^^^^+^^^^^^^^^^^
    PBYTE matchVtable = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x69\x22\x04\xA9\x00\x00\x00\x00\x08\x81\x00\x91\x60\xA2\x01\x91\x68\x32\x00\xF9",
        "xxxx??x?xx?xxxxxxxxx"
    );
    if (matchVtable)
    {
        matchVtable += 4;
        matchVtable = (PBYTE)ARM64_DecodeADRL((UINT_PTR)matchVtable, *(DWORD*)matchVtable, *(DWORD*)(matchVtable + 4));
    }
#endif
    if (matchVtable)
    {
        printf("[SMA] matchVtable = %llX\n", matchVtable - (PBYTE)mi->lpBaseOfDll);
    }

    // ### Offset of SingleViewShellExperience instance and its event handler
#if defined(_M_X64)
    // ```
    // 48 8D 8E ?? ?? ?? ?? 44 8D 45 41 48 8D 56 60 E8
    //          ^^^^^^^^^^^ SVSE                 ^^ SVSEEH (hardcoded to 0x60, included in pattern for sanity check)
    // ```
    // Ref: CStartExperienceManager::CStartExperienceManager()
    PBYTE matchSingleViewShellExperienceFields = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x48\x8D\x8E\x00\x00\x00\x00\x44\x8D\x45\x41\x48\x8D\x56\x60\xE8",
        "xxx????xxxxxxxxx"
    );
    if (matchSingleViewShellExperienceFields)
    {
        g_SMAnimationPatchOffsets.startExperienceManager_singleViewShellExperience = *(int*)(matchSingleViewShellExperienceFields + 3);
    }
#elif defined(_M_ARM64)
    // ```
    // 22 08 80 52 61 82 01 91 60 ?? ?? 91 ?? ?? ?? ?? 1F 20 03 D5
    //             ^^^SVSEEH^^ ^^^^^^^^^^^ SVSE
    // ```
    // Ref: CStartExperienceManager::CStartExperienceManager()
    PBYTE matchSingleViewShellExperienceFields = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x22\x08\x80\x52\x61\x82\x01\x91\x60\x00\x00\x91\x00\x00\x00\x00\x1F\x20\x03\xD5",
        "xxxxxxxxx??x????xxxx"
    );
    if (matchSingleViewShellExperienceFields)
    {
        g_SMAnimationPatchOffsets.startExperienceManager_singleViewShellExperience = (int)ARM64_DecodeADD(*(DWORD*)(matchSingleViewShellExperienceFields + 8));
    }
#endif
    if (matchSingleViewShellExperienceFields)
    {
        printf("[SMA] matchSingleViewShellExperienceFields = %llX\n", matchSingleViewShellExperienceFields - (PBYTE)mi->lpBaseOfDll);
    }

    // ### Offsets of Animation Helpers
    PBYTE matchAnimationHelperFields = nullptr;
#if defined(_M_X64)
    // ```
    // 40 88 AE ?? ?? ?? ?? C7 86 ?? ?? ?? ?? 38 00 00 00
    //          ^^^^^^^^^^^ AH1
    // ```
    // Ref: CStartExperienceManager::CStartExperienceManager()
    // AH2 is located right after AH1. AH is 32 bytes
    if (matchSingleViewShellExperienceFields)
    {
        matchAnimationHelperFields = (PBYTE)FindPattern(
           matchSingleViewShellExperienceFields + 16,
           128,
           "\x40\x88\xAE\x00\x00\x00\x00\xC7\x86\x00\x00\x00\x00\x38\x00\x00\x00",
           "xxx????xx????xxxx"
       );
    }
    if (matchAnimationHelperFields)
    {
        g_SMAnimationPatchOffsets.startExperienceManager_openingAnimation = *(int*)(matchAnimationHelperFields + 3);
        g_SMAnimationPatchOffsets.startExperienceManager_closingAnimation = g_SMAnimationPatchOffsets.startExperienceManager_openingAnimation + 32;
    }
#elif defined(_M_ARM64)
    // ```
    // 08 07 80 52 7F ?? ?? 39 68 ?? ?? B9
    //             ^^^^^^^^^^^ AH1
    // ```
    // Ref: CStartExperienceManager::CStartExperienceManager()
    // AH2 is located right after AH1. AH is 32 bytes
    if (matchSingleViewShellExperienceFields)
    {
        matchAnimationHelperFields = (PBYTE)FindPattern(
            matchSingleViewShellExperienceFields + 20,
            128,
            "\x08\x07\x80\x52\x7F\x00\x00\x39\x68\x00\x00\xB9",
            "xxxxx??xx??x"
        );
    }
    if (matchAnimationHelperFields)
    {
        int openingAnimation = (int)ARM64_DecodeSTRBIMM(*(DWORD*)(matchAnimationHelperFields + 4));
        if (openingAnimation != -1)
        {
            g_SMAnimationPatchOffsets.startExperienceManager_openingAnimation = openingAnimation;
            g_SMAnimationPatchOffsets.startExperienceManager_closingAnimation = g_SMAnimationPatchOffsets.startExperienceManager_openingAnimation + 32;
        }
        else
        {
            matchAnimationHelperFields = nullptr;
        }
    }
#endif
    if (matchAnimationHelperFields)
    {
        printf(
            "[SMA] matchAnimationHelperFields = %llX, +0x%X, +0x%X\n",
            matchAnimationHelperFields - (PBYTE)mi->lpBaseOfDll,
            g_SMAnimationPatchOffsets.startExperienceManager_openingAnimation,
            g_SMAnimationPatchOffsets.startExperienceManager_closingAnimation
        );
    }

    // ### Offset of bTransitioningToCortana
#if defined(_M_X64)
    // ```
    // 80 B9 ?? ?? ?? ?? 00 75 ?? 48 83 C1 D8
    //       ^^^^^^^^^^^ bTransitioningToCortana
    // ```
    // Ref: CStartExperienceManager::DimStart()
    PBYTE matchTransitioningToCortanaField = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x80\xB9\x00\x00\x00\x00\x00\x75\x00\x48\x83\xC1\xD8",
        "xx????xx?xxxx"
        );
    if (matchTransitioningToCortanaField)
    {
        g_SMAnimationPatchOffsets.startExperienceManager_bTransitioningToCortana = g_SMAnimationPatchOffsets.startExperienceManager_IStartExperienceManager + *(int*)(matchTransitioningToCortanaField + 2);
    }
#elif defined(_M_ARM64)
    // ```
    // ?? ?? ?? 39 E8 00 00 35 ?? ?? ?? ?? 01 ?? ?? 91 22 00 80 52
    // ^^^^^^^^^^^ bTransitioningToCortana
    // ```
    // Ref: CStartExperienceManager::DimStart()
    PBYTE matchTransitioningToCortanaField = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x39\xE8\x00\x00\x35\x00\x00\x00\x00\x01\x00\x00\x91\x22\x00\x80\x52",
        "xxxxx????x??xxxxx"
    );
    if (matchTransitioningToCortanaField)
    {
        int off = (int)ARM64_DecodeLDRBIMM(*(DWORD*)(matchTransitioningToCortanaField - 3));
        if (off != -1)
        {
            g_SMAnimationPatchOffsets.startExperienceManager_bTransitioningToCortana = g_SMAnimationPatchOffsets.startExperienceManager_IStartExperienceManager + off;
        }
        else
        {
            matchTransitioningToCortanaField = nullptr;
        }
    }
#endif
    if (matchTransitioningToCortanaField)
    {
        printf("[SMA] matchTransitioningToCortanaField = %llX, +0x%X\n", matchTransitioningToCortanaField - (PBYTE)mi->lpBaseOfDll, g_SMAnimationPatchOffsets.startExperienceManager_bTransitioningToCortana);
    }

    // ### Offset of CStartExperienceManager::GetMonitorInformation()
#if defined(_M_X64)
    // ```
    // 48 8B ?? E8 ?? ?? ?? ?? 8B ?? 85 C0 0F 88 ?? ?? ?? ?? C6 44 24 ?? 01
    //             ^^^^^^^^^^^
    // ```
    // Ref: CStartExperienceManager::PositionMenu()
    PBYTE matchGetMonitorInformation = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x48\x8B\x00\xE8\x00\x00\x00\x00\x8B\x00\x85\xC0\x0F\x88\x00\x00\x00\x00\xC6\x44\x24\x00\x01",
        "xx?x????x?xxxx????xxx?x"
    );
    if (matchGetMonitorInformation)
    {
        matchGetMonitorInformation += 3;
        matchGetMonitorInformation += 5 + *(int*)(matchGetMonitorInformation + 1);
    }
#elif defined(_M_ARM64)
    // * Pattern for 261xx:
    //   ```
    //   E2 82 00 91 E1 03 13 AA E0 03 14 AA ?? ?? ?? ??
    //                                       ^^^^^^^^^^^
    //   ```
    // * Different patterns needed for 226xx and 262xx+
    // Ref: CStartExperienceManager::PositionMenu()
    PBYTE matchGetMonitorInformation = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\xE2\x82\x00\x91\xE1\x03\x13\xAA\xE0\x03\x14\xAA",
        "xxxxxxxxxxxx"
    );
    if (matchGetMonitorInformation)
    {
        matchGetMonitorInformation += 12;
        matchGetMonitorInformation = (PBYTE)ARM64_FollowBL((DWORD*)matchGetMonitorInformation);
    }
#endif
    if (matchGetMonitorInformation)
    {
        CStartExperienceManager_GetMonitorInformationFunc = (decltype(CStartExperienceManager_GetMonitorInformationFunc))matchGetMonitorInformation;
        printf("[SMA] CStartExperienceManager::GetMonitorInformation() = %llX\n", matchGetMonitorInformation - (PBYTE)mi->lpBaseOfDll);
    }

    // ### Offset of CExperienceManagerAnimationHelper::Begin()
#if defined(_M_X64)
    // * Pattern 1, used when all arguments are available:
    //   ```
    //   44 8B C7                      E8 ?? ?? ?? ?? 85 C0 79 19
    //                                    ^^^^^^^^^^^
    //   ```
    // * Pattern 2, used when a4, a5, and a6 are optimized out (e.g. 26020, 26058):
    //   ```
    //   44 8B C7 48 8D 8B ?? ?? ?? ?? E8 ?? ?? ?? ?? 85 C0 79 19
    //                                    ^^^^^^^^^^^
    //   ```
    // Ref: CJumpViewExperienceManager::OnViewUncloaking()
    PBYTE matchAnimationBegin = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x44\x8B\xC7\xE8\x00\x00\x00\x00\x85\xC0\x79\x19",
        "xxxx????xxxx"
    );
    if (matchAnimationBegin)
    {
        matchAnimationBegin += 3;
        matchAnimationBegin += 5 + *(int*)(matchAnimationBegin + 1);
    }
    else
    {
        matchAnimationBegin = (PBYTE)FindPattern(
            mi->lpBaseOfDll,
            mi->SizeOfImage,
            "\x44\x8B\xC7\x48\x8D\x8B\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x85\xC0\x79\x19",
            "xxxxxx????x????xxxx"
        );
        if (matchAnimationBegin)
        {
            matchAnimationBegin += 10;
            matchAnimationBegin += 5 + *(int*)(matchAnimationBegin + 1);
        }
    }
#elif defined(_M_ARM64)
    // * Pattern 1, used when all arguments are available:
    //   ```
    //   Not implemented
    //
    //   ```
    // * Pattern 2, used when a4, a5, and a6 are optimized out (e.g. 26020, 26058):
    //   ```
    //   82 02 0B 32 67 ?? ?? 91 60 ?? ?? 91 ?? ?? ?? ?? E3 03 00 2A
    //                                       ^^^^^^^^^^^
    //   ```
    // Ref: CJumpViewExperienceManager::OnViewUncloaking()
    PBYTE matchAnimationBegin = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x82\x02\x0B\x32\x67\x00\x00\x91\x60\x00\x00\x91\x00\x00\x00\x00\xE3\x03\x00\x2A",
        "xxxxx??xx??x????xxxx"
    );
    if (matchAnimationBegin)
    {
        matchAnimationBegin += 12;
        matchAnimationBegin = (PBYTE)ARM64_FollowBL((DWORD*)matchAnimationBegin);
    }
#endif
    if (matchAnimationBegin)
    {
        CExperienceManagerAnimationHelper_BeginFunc = (decltype(CExperienceManagerAnimationHelper_BeginFunc))matchAnimationBegin;
        printf("[SMA] CExperienceManagerAnimationHelper::Begin() = %llX\n", matchAnimationBegin - (PBYTE)mi->lpBaseOfDll);
    }

    // ### Offset of CExperienceManagerAnimationHelper::End()
#if defined(_M_X64)
    // ```
    // 40 53 48 83 EC 20 80 39 00 74
    // ```
    PBYTE matchAnimationEnd = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x40\x53\x48\x83\xEC\x20\x80\x39\x00\x74",
        "xxxxxxxxxx"
    );
#elif defined(_M_ARM64)
    // ```
    // 7F 23 03 D5 F3 0F 1F F8 FD 7B BF A9 FD 03 00 91 08 00 40 39
    // ----------- PACIBSP, don't scan for this because it's everywhere
    // ```
    PBYTE matchAnimationEnd = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\xF3\x0F\x1F\xF8\xFD\x7B\xBF\xA9\xFD\x03\x00\x91\x08\x00\x40\x39",
        "xxxxxxxxxxxxxxxx"
    );
    if (matchAnimationEnd)
    {
        matchAnimationEnd -= 4;
    }
#endif
    if (matchAnimationEnd)
    {
        CExperienceManagerAnimationHelper_EndFunc = (decltype(CExperienceManagerAnimationHelper_EndFunc))matchAnimationEnd;
        printf("[SMA] CExperienceManagerAnimationHelper::End() = %llX\n", matchAnimationEnd - (PBYTE)mi->lpBaseOfDll);
    }

    // ### CStartExperienceManager::Hide()
#if defined(_M_X64)
    // * Pattern 1, mov [rbx+2A3h], r12b:
    //   ```
    //   74 ?? ?? 03 00 00 00 44 88
    //   ^^ Turn jz into jmp
    //   ```
    // * Pattern 2, mov byte ptr [rbx+2A3h], 1:
    //   ```
    //   74 ?? ?? 03 00 00 00 C6 83
    //   ^^ Turn jz into jmp
    //   ```
    // Perform on exactly two matches
    PBYTE matchHideA = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x74\x00\x00\x03\x00\x00\x00\x44\x88",
        "x??xxxxxx"
    );
    PBYTE matchHideB = nullptr;
    if (matchHideA)
    {
        printf("[SMA] matchHideA in CStartExperienceManager::Hide() = %llX\n", matchHideA - (PBYTE)mi->lpBaseOfDll);
        matchHideB = (PBYTE)FindPattern(
            matchHideA + 14,
            mi->SizeOfImage - (matchHideA + 14 - (PBYTE)mi->lpBaseOfDll),
            "\x74\x00\x00\x03\x00\x00\x00\x44\x88",
            "x??xxxxxx"
        );
        if (matchHideB)
        {
            printf("[SMA] matchHideB in CStartExperienceManager::Hide() = %llX\n", matchHideB - (PBYTE)mi->lpBaseOfDll);
        }
    }

    if (!matchHideA || !matchHideB)
    {
        matchHideA = (PBYTE)FindPattern(
            mi->lpBaseOfDll,
            mi->SizeOfImage,
            "\x74\x00\x00\x03\x00\x00\x00\xC6\x83",
            "x??xxxxxx"
        );
        matchHideB = nullptr;
        if (matchHideA)
        {
            printf("[SMA] matchHideA in CStartExperienceManager::Hide() = %llX\n", matchHideA - (PBYTE)mi->lpBaseOfDll);
            matchHideB = (PBYTE)FindPattern(
                matchHideA + 14,
                mi->SizeOfImage - (matchHideA + 14 - (PBYTE)mi->lpBaseOfDll),
                "\x74\x00\x00\x03\x00\x00\x00\xC6\x83",
                "x??xxxxxx"
            );
            if (matchHideB)
            {
                printf("[SMA] matchHideB in CStartExperienceManager::Hide() = %llX\n", matchHideB - (PBYTE)mi->lpBaseOfDll);
            }
        }
    }
#elif defined(_M_ARM64)
    // ```
    // ?? ?? ?? 34 ?? 00 80 52 ?? 8E 0A 39
    // ^^^^^^^^^^^ Turn CBZ into B
    // ```
    // Perform on exactly two matches
    PBYTE matchHideA = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x34\x00\x00\x80\x52\x00\x8E\x0A\x39",
        "x?xxx?xxx"
    );
    PBYTE matchHideB = nullptr;
    if (matchHideA)
    {
        matchHideA -= 3;
        printf("[SMA] matchHideA in CStartExperienceManager::Hide() = %llX\n", matchHideA - (PBYTE)mi->lpBaseOfDll);
        matchHideB = (PBYTE)FindPattern(
            matchHideA + 12,
            mi->SizeOfImage - (matchHideA + 12 - (PBYTE)mi->lpBaseOfDll),
            "\x34\x00\x00\x80\x52\x00\x8E\x0A\x39",
            "x?xxx?xxx"
        );
        if (matchHideB)
        {
            matchHideB -= 3;
            printf("[SMA] matchHideB in CStartExperienceManager::Hide() = %llX\n", matchHideB - (PBYTE)mi->lpBaseOfDll);
        }
    }
#endif

    if (!matchVtable
        || !matchSingleViewShellExperienceFields
        || !matchAnimationHelperFields
        || !matchTransitioningToCortanaField
        || !matchGetMonitorInformation
        || !matchAnimationBegin
        || !matchAnimationEnd
        || !matchHideA
        || !matchHideB)
    {
        printf("[SMA] Not all offsets were found, cannot perform patch\n");
        return FALSE;
    }

    DWORD dwOldProtect = 0;

    void** vtable = (void**)matchVtable;
    void** p_OnViewUncloaking = &vtable[4];
    void** p_OnViewUncloaked = &vtable[5];
    void** p_OnViewCloaking = &vtable[6];
    void** p_OnViewHidden = &vtable[10];

    // OnViewUncloaking
    if (*p_OnViewUncloaking != CStartExperienceManager_OnViewUncloakingHook)
    {
        CStartExperienceManager_OnViewUncloakingFunc = (decltype(CStartExperienceManager_OnViewUncloakingFunc))*p_OnViewUncloaking;
        if (VirtualProtect(p_OnViewUncloaking, sizeof(void*), PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            *p_OnViewUncloaking = CStartExperienceManager_OnViewUncloakingHook;
            VirtualProtect(p_OnViewUncloaking, sizeof(void*), dwOldProtect, &dwOldProtect);
        }
    }

    // OnViewUncloaked
    if (*p_OnViewUncloaked != CStartExperienceManager_OnViewUncloakedHook)
    {
        CStartExperienceManager_OnViewUncloakedFunc = (decltype(CStartExperienceManager_OnViewUncloakedFunc))*p_OnViewUncloaked;
        if (VirtualProtect(p_OnViewUncloaked, sizeof(void*), PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            *p_OnViewUncloaked = CStartExperienceManager_OnViewUncloakedHook;
            VirtualProtect(p_OnViewUncloaked, sizeof(void*), dwOldProtect, &dwOldProtect);
        }
    }

    // OnViewCloaking
    if (*p_OnViewCloaking != CStartExperienceManager_OnViewCloakingHook)
    {
        CStartExperienceManager_OnViewCloakingFunc = (decltype(CStartExperienceManager_OnViewCloakingFunc))*p_OnViewCloaking;
        if (VirtualProtect(p_OnViewCloaking, sizeof(void*), PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            *p_OnViewCloaking = CStartExperienceManager_OnViewCloakingHook;
            VirtualProtect(p_OnViewCloaking, sizeof(void*), dwOldProtect, &dwOldProtect);
        }
    }

    // OnViewHidden
    if (*p_OnViewHidden != CStartExperienceManager_OnViewHiddenHook)
    {
        CStartExperienceManager_OnViewHiddenFunc = (decltype(CStartExperienceManager_OnViewHiddenFunc))*p_OnViewHidden;
        if (VirtualProtect(p_OnViewHidden, sizeof(void*), PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            *p_OnViewHidden = CStartExperienceManager_OnViewHiddenHook;
            VirtualProtect(p_OnViewHidden, sizeof(void*), dwOldProtect, &dwOldProtect);
        }
    }

    if (dwStartShowClassicMode)
    {
#if defined(_M_X64)
        if (VirtualProtect(matchHideA, 1, PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            matchHideA[0] = 0xEB;
            VirtualProtect(matchHideA, 1, dwOldProtect, &dwOldProtect);

            dwOldProtect = 0;
            if (VirtualProtect(matchHideB, 1, PAGE_EXECUTE_READWRITE, &dwOldProtect))
            {
                matchHideB[0] = 0xEB;
                VirtualProtect(matchHideB, 1, dwOldProtect, &dwOldProtect);
            }
        }
#elif defined(_M_ARM64)
        if (VirtualProtect(matchHideA, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            DWORD newInsn = ARM64_CBZWToB(*(DWORD*)matchHideA);
            if (newInsn)
                *(DWORD*)matchHideA = newInsn;
            VirtualProtect(matchHideA, 4, dwOldProtect, &dwOldProtect);

            dwOldProtect = 0;
            if (VirtualProtect(matchHideB, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect))
            {
                newInsn = ARM64_CBZWToB(*(DWORD*)matchHideB);
                if (newInsn)
                    *(DWORD*)matchHideB = newInsn;
                VirtualProtect(matchHideB, 4, dwOldProtect, &dwOldProtect);
            }
        }
#endif
    }

    int rv = -1;
    if (CStartExperienceManager_GetMonitorInformationFunc)
    {
        rv = funchook_prepare(
            funchook,
            (void**)&CStartExperienceManager_GetMonitorInformationFunc,
            CStartExperienceManager_GetMonitorInformationHook
        );
    }
    if (rv != 0)
    {
        printf("Failed to hook CStartExperienceManager::GetMonitorInformation(). rv = %d\n", rv);
    }

    return TRUE;
}

#pragma endregion


#pragma region "Fix broken taskbar jump list positioning caused by 40874676"

class RoVariant
{
    enum States
    {
        StateIsNull = 0,
        StateIsObjNoRef = 1,
        StateIsObj = 3,
        StateIsPV = 7
    };

    static bool StateHasRefcount(HRESULT hrState)
    {
        return hrState == StateIsPV || hrState == StateIsObj;
    }

    class Accessor
    {
        IInspectable* _pI;
        HRESULT _hrState;

        ABI::Windows::Foundation::IPropertyValue* _PV() const
        {
            return (ABI::Windows::Foundation::IPropertyValue*)_pI;
        }

        HRESULT VerifyPV() const
        {
            if (_hrState == StateIsPV)
                return S_OK;
            if (SUCCEEDED(_hrState))
                return TYPE_E_TYPEMISMATCH;
            return _hrState;
        }

    public:
        Accessor(IInspectable* pI, HRESULT hr) : _pI(pI), _hrState(hr) {}
        Accessor* operator->() { return this; }
        HRESULT get_Type(ABI::Windows::Foundation::PropertyType* type) const;

        #define GETTER(name, type) HRESULT Get##name(type* value) const \
        { \
            if (!value) \
                return E_POINTER; \
            *value = {}; \
            HRESULT hr = VerifyPV(); \
            if (SUCCEEDED(hr)) \
                hr = _PV()->Get##name(value); \
            return hr; \
        }

        GETTER(UInt8, UINT8);
        GETTER(Int16, INT16);
        GETTER(UInt16, UINT16);
        GETTER(Int32, INT32);
        GETTER(UInt32, UINT32);
        GETTER(Int64, INT64);
        GETTER(UInt64, UINT64);
        GETTER(Single, float);
        GETTER(Double, double);
        GETTER(Char16, WCHAR);
        GETTER(Boolean, BOOLEAN);
        GETTER(String, HSTRING);
        GETTER(Guid, GUID);
        GETTER(DateTime, ABI::Windows::Foundation::DateTime);
        GETTER(TimeSpan, ABI::Windows::Foundation::TimeSpan);
        GETTER(Point, ABI::Windows::Foundation::Point);
        GETTER(Size, ABI::Windows::Foundation::Size);
        GETTER(Rect, ABI::Windows::Foundation::Rect);

        #undef GETTER

        HRESULT GetInspectable(IInspectable** value) const
        {
            if (!value)
                return E_POINTER;
            *value = nullptr;
            HRESULT hr = _hrState;
            if (SUCCEEDED(hr))
            {
                if (hr != StateIsNull && (hr == StateIsObj || hr == StateIsObjNoRef))
                {
                    *value = _pI;
                    _pI->AddRef();
                    hr = S_OK;
                }
                else
                {
                    hr = TYPE_E_TYPEMISMATCH;
                }
            }
            return hr;
        }

        #define GETTER_ARRAY(name, type) HRESULT Get##name##Array(UINT* length, type** value) const \
        { \
            if (!length || !value) \
                return E_POINTER; \
            *length = 0; \
            *value = nullptr; \
            HRESULT hr = VerifyPV(); \
            if (SUCCEEDED(hr)) \
                hr = _PV()->Get##name##Array(length, value); \
            return hr; \
        }

        GETTER_ARRAY(UInt8, UINT8);
        GETTER_ARRAY(Int16, INT16);
        GETTER_ARRAY(UInt16, UINT16);
        GETTER_ARRAY(Int32, INT32);
        GETTER_ARRAY(UInt32, UINT32);
        GETTER_ARRAY(Int64, INT64);
        GETTER_ARRAY(UInt64, UINT64);
        GETTER_ARRAY(Single, float);
        GETTER_ARRAY(Double, double);
        GETTER_ARRAY(Char16, WCHAR);
        GETTER_ARRAY(Boolean, BOOLEAN);
        GETTER_ARRAY(String, HSTRING);
        GETTER_ARRAY(Inspectable, IInspectable*);
        GETTER_ARRAY(Guid, GUID);
        GETTER_ARRAY(DateTime, ABI::Windows::Foundation::DateTime);
        GETTER_ARRAY(TimeSpan, ABI::Windows::Foundation::TimeSpan);
        GETTER_ARRAY(Point, ABI::Windows::Foundation::Point);
        GETTER_ARRAY(Size, ABI::Windows::Foundation::Size);
        GETTER_ARRAY(Rect, ABI::Windows::Foundation::Rect);

        #undef GETTER_ARRAY
    };

    class OutRef
    {
        RoVariant* _pOwner;
        IInspectable* _pI;

    public:
        OutRef(RoVariant* pOwner) : _pOwner(pOwner), _pI(nullptr) {}
        operator ABI::Windows::Foundation::IPropertyValue**() { return (ABI::Windows::Foundation::IPropertyValue**)&_pI; }
        operator IInspectable**() { return &_pI; }
        ~OutRef() { _pOwner->Attach(_pI); }
    };

    IInspectable* _pI = nullptr;
    HRESULT _hrState = StateIsNull;

public:
    RoVariant() = default;
    RoVariant(RoVariant*);
    RoVariant(RoVariant&);
    RoVariant(ABI::Windows::Foundation::IPropertyValue*, bool);

private:
    RoVariant(IInspectable* pI, bool fAddRefInspectable, bool attach)
    {
        if (pI)
        {
            ABI::Windows::Foundation::IPropertyValue* pPV;
            HRESULT hr = pI->QueryInterface(IID_PPV_ARGS(&pPV));
            if (SUCCEEDED(hr))
            {
                _pI = pPV;
                if (attach)
                    pI->Release();
                _hrState = StateIsPV;
            }
            else if (hr != E_NOINTERFACE)
            {
                _pI = nullptr;
                _hrState = hr;
                if (attach)
                    pI->Release();
            }
            else
            {
                _pI = pI;
                if (fAddRefInspectable && !attach)
                    _pI->AddRef();
                _hrState = fAddRefInspectable ? StateIsObj : StateIsObjNoRef;
            }
        }
        else
        {
            _pI = nullptr;
            _hrState = StateIsNull;
        }
    }

public:
    RoVariant(IInspectable* pI, bool attach)
    {
        RoVariant tmp(pI, true, attach);
        Swap(tmp);
    }

    RoVariant(void*);

    ~RoVariant()
    {
        if (_pI && StateHasRefcount(_hrState))
            _pI->Release();
    }

    RoVariant& operator=(RoVariant other)
    {
        Swap(other);
        return *this;
    }

    void Swap(RoVariant& other)
    {
        IInspectable* pI = _pI;
        _pI = other._pI;
        other._pI = pI;

        HRESULT hrState = _hrState;
        _hrState = other._hrState;
        other._hrState = hrState;
    }

    operator IInspectable*() const { return Get(); }
    IInspectable* Get() const { return _pI; }
    IInspectable* Detach();
    void Attach(ABI::Windows::Foundation::IPropertyValue*);

    void Attach(IInspectable* pI)
    {
        RoVariant tmp = RoVariant(pI, true, true);
        Swap(tmp);
    }

    Accessor operator*() const { return Accessor(_pI, _hrState); }
    Accessor operator->() const { return Accessor(_pI, _hrState); }
    OutRef operator&() { return ReleaseAndGetAddressOf(); }

    struct USE_INSTEAD_ReleaseAndGetAddressOf
    {
    };

    USE_INSTEAD_ReleaseAndGetAddressOf GetAddressOf() { return USE_INSTEAD_ReleaseAndGetAddressOf(); }
    OutRef ReleaseAndGetAddressOf() { return OutRef(this); }
    bool operator!();
    static RoVariant Wrap(ABI::Windows::Foundation::IPropertyValue*);
    static RoVariant Wrap(IInspectable* pI) { return RoVariant(pI, false, false); }
    HRESULT CopyTo(ABI::Windows::Foundation::IPropertyValue**);
    HRESULT CopyTo(IInspectable**);
};

namespace ABI::Windows::UI::Xaml
{
    enum HorizontalAlignment
    {
        HorizontalAlignment_Left = 0,
        HorizontalAlignment_Center = 1,
        HorizontalAlignment_Right = 2,
        HorizontalAlignment_Stretch = 3,
    };

    enum VerticalAlignment
    {
        VerticalAlignment_Top = 0,
        VerticalAlignment_Center = 1,
        VerticalAlignment_Bottom = 2,
        VerticalAlignment_Stretch = 3,
    };
}

HRESULT CJumpViewExperienceManager_CalcWindowPosition(
    RECT rcWork,
    POINT ptAnchor,
    int width,
    int height,
    ABI::Windows::UI::Xaml::HorizontalAlignment hAlign,
    ABI::Windows::UI::Xaml::VerticalAlignment vAlign,
    RECT& result)
{
    using namespace ABI::Windows::UI::Xaml;

    if (false) // Feature_40874676
    {
        result.bottom = max(min(ptAnchor.y, rcWork.bottom), rcWork.top);
        int desiredTop = result.bottom - height;
        result.top = max(desiredTop, rcWork.top);
        int desiredLeft = ptAnchor.x - (width / 2);
        result.left = min(max(desiredLeft, rcWork.left), rcWork.right);
        result.right = min(result.left + width, rcWork.right);
        return S_OK;
    }

    switch (vAlign)
    {
        case VerticalAlignment_Center:
        {
            int desiredTopPre = (height / -2) + ptAnchor.y;
            result.bottom = min(height + max(desiredTopPre, rcWork.top), rcWork.bottom);
            int desiredTop = result.bottom - height;
            result.top = max(desiredTop, rcWork.top);
            break;
        }
        case VerticalAlignment_Top:
        {
            int desiredTopPre = ptAnchor.y;
            result.bottom = min(height + max(desiredTopPre, rcWork.top), rcWork.bottom);
            int desiredTop = result.bottom - height;
            result.top = max(desiredTop, rcWork.top);
            break;
        }
        case VerticalAlignment_Bottom:
        {
            int top = max(min(ptAnchor.y, rcWork.bottom) - height, rcWork.top);
            result.bottom = min(top + height, rcWork.bottom);
            result.top = top;
            break;
        }
        default:
        {
            RETURN_HR(E_NOTIMPL);
        }
    }

    switch (hAlign)
    {
        case HorizontalAlignment_Center:
        {
            int desiredLeftPre = (width / -2) + ptAnchor.x;
            result.right = min(width + max(desiredLeftPre, rcWork.left), rcWork.right);
            int desiredLeft = result.right - width;
            result.left = max(desiredLeft, rcWork.left);
            break;
        }
        case HorizontalAlignment_Left:
        {
            int desiredLeftPre = ptAnchor.x;
            result.right = min(width + max(desiredLeftPre, rcWork.left), rcWork.right);
            int desiredLeft = result.right - width;
            result.left = max(desiredLeft, rcWork.left);
            break;
        }
        case HorizontalAlignment_Right:
        {
            result.left = max(min(ptAnchor.x, rcWork.right) - width, rcWork.left);
            result.right = min(result.left + width, rcWork.right);
            break;
        }
        default:
        {
            RETURN_HR(E_NOTIMPL);
        }
    }

    return S_OK;
}

HRESULT CJumpViewExperienceManager_GetMonitorInformation(void* _this, POINT ptAnchor, RECT* prcOutWorkArea, UINT* outDpi, EDGEUI_TRAYSTUCKPLACE* outStuckPlace)
{
    HMONITOR hMonitor = MonitorFromPoint(ptAnchor, MONITOR_DEFAULTTONEAREST);
    RETURN_LAST_ERROR_IF(hMonitor == INVALID_HANDLE_VALUE);

    MONITORINFO mi = { sizeof(mi) };
    RETURN_IF_WIN32_BOOL_FALSE(GetMonitorInfoW(hMonitor, &mi));
    *prcOutWorkArea = mi.rcWork;

    UINT dpiY;
    RETURN_IF_FAILED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, outDpi, &dpiY)); // 884

    ComPtr<IServiceProvider> spImmersiveShellServiceProvider;
    RETURN_IF_FAILED(CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&spImmersiveShellServiceProvider)));

    ComPtr<IImmersiveMonitorManager> spImmersiveMonitorManager;
    RETURN_IF_FAILED(spImmersiveShellServiceProvider->QueryService(SID_IImmersiveMonitorService, IID_PPV_ARGS(&spImmersiveMonitorManager))); // 886

    ComPtr<IEdgeUiManager> spEdgeUiManager;
    RETURN_IF_FAILED(spImmersiveMonitorManager->QueryService(hMonitor, SID_EdgeUi, IID_PPV_ARGS(&spEdgeUiManager))); // 887
    RETURN_IF_FAILED(spEdgeUiManager->GetTrayStuckPlace(outStuckPlace)); // 888

    return S_OK;
}

HRESULT(*CJumpViewExperienceManager_EnsureWindowPositionFunc)(void* _this, CSingleViewShellExperience* experience);
HRESULT CJumpViewExperienceManager_EnsureWindowPositionHook(void* _this, CSingleViewShellExperience* experience)
{
    if (!experience->_viewWrapper)
        return S_OK;

    ComPtr<ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable*>> properties;
    RETURN_IF_FAILED(experience->_propertySet.As(&properties)); // 813

    POINT ptAnchor;
    {
        RoVariant variant;
        RETURN_IF_FAILED(properties->Lookup(Wrappers::HStringReference(L"Position").Get(), &variant)); // 816

        ABI::Windows::Foundation::Point value;
        RETURN_IF_FAILED(variant->GetPoint(&value)); // 819

        ptAnchor.x = (int)value.X;
        ptAnchor.y = (int)value.Y;
    }

    ABI::Windows::UI::Xaml::HorizontalAlignment hAlign;
    {
        RoVariant variant;
        RETURN_IF_FAILED(properties->Lookup(Wrappers::HStringReference(L"HorizontalAlign").Get(), &variant)); // 828

        int value;
        RETURN_IF_FAILED(variant->GetInt32(&value)); // 831
        hAlign = (ABI::Windows::UI::Xaml::HorizontalAlignment)value;
    }

    ABI::Windows::UI::Xaml::VerticalAlignment vAlign;
    {
        RoVariant variant;
        RETURN_IF_FAILED(properties->Lookup(Wrappers::HStringReference(L"VerticalAlign").Get(), &variant)); // 838

        int value;
        RETURN_IF_FAILED(variant->GetInt32(&value)); // 841
        vAlign = (ABI::Windows::UI::Xaml::VerticalAlignment)value;
    }

    RECT rcWorkArea;
    UINT dpi;
    RETURN_IF_FAILED(CJumpViewExperienceManager_GetMonitorInformation(
        _this, ptAnchor, &rcWorkArea, &dpi,
        (EDGEUI_TRAYSTUCKPLACE*)((PBYTE)_this + 0x1F0))); // 850
    *((RECT*)((PBYTE)_this + 0x200)) = rcWorkArea;

    int width, height;
    ExperienceManagerUtils::ScaleByDPI(&experience->_desiredSize, dpi, &width, &height);
    RETURN_HR_IF(E_INVALIDARG, width <= 0 || height <= 0); // 860

    RECT rcPosition;
    RETURN_IF_FAILED(CJumpViewExperienceManager_CalcWindowPosition(rcWorkArea, ptAnchor, width, height, hAlign, vAlign, rcPosition));
    RETURN_IF_FAILED(experience->SetPosition(&rcPosition));

    return S_OK;
}

BOOL FixJumpViewPositioning(MODULEINFO* mi)
{
    // Offset sanity checks

    // EDGEUI_TRAYSTUCKPLACE CJumpViewExperienceManager::m_trayStuckPlace
#if defined(_M_X64)
    // 8B 8B B0 01 00 00 BF 5C 00 00 00 85 C9
    //       ^^^^^^^^^^^
    // Ref: CJumpViewExperienceManager::OnViewUncloaking()
    PBYTE matchOffsetTrayStuckPlace = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x8B\x8B\xB0\x01\x00\x00\xBF\x5C\x00\x00\x00\x85\xC9",
        "xxxxxxxxxxxxx"
    );
#elif defined(_M_ARM64)
    // 08 B0 41 B9 89 0B 80 52
    // ^^^^^^^^^^^
    // Ref: CJumpViewExperienceManager::OnViewCloaking()
    PBYTE matchOffsetTrayStuckPlace = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x08\xB0\x41\xB9\x89\x0B\x80\x52",
        "xxxxxxxx"
    );
#endif
    if (matchOffsetTrayStuckPlace)
    {
        printf("[JVP] matchOffsetTrayStuckPlace = %llX\n", matchOffsetTrayStuckPlace - (PBYTE)mi->lpBaseOfDll);
    }

    // RECT CJumpViewExperienceManager::m_rcWorkArea
    PBYTE matchOffsetRcWorkArea = nullptr;
#if defined(_M_X64)
    // 48 8B 53 70 48 8D 83 C0 01 00 00
    //          --          ^^^^^^^^^^^
    // Ref: CJumpViewExperienceManager::OnViewUncloaking()
    if (matchOffsetTrayStuckPlace)
    {
        matchOffsetRcWorkArea = (PBYTE)FindPattern(
           matchOffsetTrayStuckPlace + 13,
           256,
           "\x48\x8B\x53\x70\x48\x8D\x83\xC0\x01\x00\x00",
           "xxxxxxxxxxx"
        );
    }
#elif defined(_M_ARM64)
    // 01 38 40 F9 07 00 07 91
    // ----------- ^^^^^^^^^^^
    // Ref: CJumpViewExperienceManager::OnViewCloaking()
    if (matchOffsetTrayStuckPlace)
    {
        matchOffsetRcWorkArea = (PBYTE)FindPattern(
            matchOffsetTrayStuckPlace + 8,
            128,
            "\x01\x38\x40\xF9\x07\x00\x07\x91",
            "xxxxxxxx"
        );
    }
#endif
    if (matchOffsetRcWorkArea)
    {
        printf("[JVP] matchOffsetRcWorkArea = %llX\n", matchOffsetRcWorkArea - (PBYTE)mi->lpBaseOfDll);
    }

    // CJumpViewExperienceManager::EnsureWindowPosition()
#if defined(_M_X64)
    // 8D 4E C0 48 8B ?? E8 ?? ?? ?? ?? 8B
    //                      ^^^^^^^^^^^
    // Ref: CJumpViewExperienceManager::OnViewPropertiesChanging()
    PBYTE matchEnsureWindowPosition = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\x8D\x4E\xC0\x48\x8B\x00\xE8\x00\x00\x00\x00\x8B",
        "xxxxx?x????x"
    );
    if (matchEnsureWindowPosition)
    {
        matchEnsureWindowPosition += 6;
        matchEnsureWindowPosition += 5 + *(int*)(matchEnsureWindowPosition + 1);
    }
#elif defined(_M_ARM64)
    // E1 03 15 AA 80 02 01 D1 ?? ?? ?? ?? F3 03 00 2A
    //                         ^^^^^^^^^^^
    // Ref: CJumpViewExperienceManager::OnViewPropertiesChanging()
    PBYTE matchEnsureWindowPosition = (PBYTE)FindPattern(
        mi->lpBaseOfDll,
        mi->SizeOfImage,
        "\xE1\x03\x15\xAA\x80\x02\x01\xD1\x00\x00\x00\x00\xF3\x03\x00\x2A",
        "xxxxxxxx????xxxx"
    );
    if (matchEnsureWindowPosition)
    {
        matchEnsureWindowPosition += 8;
        matchEnsureWindowPosition = (PBYTE)ARM64_FollowBL((DWORD*)matchEnsureWindowPosition);
    }
#endif
    if (matchEnsureWindowPosition)
    {
        printf("[JVP] matchEnsureWindowPosition = %llX\n", matchEnsureWindowPosition - (PBYTE)mi->lpBaseOfDll);
    }

    if (!matchOffsetTrayStuckPlace
        || !matchOffsetRcWorkArea
        || !matchEnsureWindowPosition)
    {
        printf("[JVP] Not all offsets were found, cannot perform patch\n");
        return FALSE;
    }

    CJumpViewExperienceManager_EnsureWindowPositionFunc = (decltype(CJumpViewExperienceManager_EnsureWindowPositionFunc))matchEnsureWindowPosition;
    funchook_prepare(
        funchook,
        (void**)&CJumpViewExperienceManager_EnsureWindowPositionFunc,
        CJumpViewExperienceManager_EnsureWindowPositionHook
    );

    return TRUE;
}

#pragma endregion


void TryToFindTwinuiPCShellOffsets(DWORD* pOffsets)
{
    // We read from the file instead of from memory because other tweak software might've modified the functions we're looking for
    WCHAR wszPath[MAX_PATH];
    GetSystemDirectoryW(wszPath, MAX_PATH);
    wcscat_s(wszPath, MAX_PATH, L"\\twinui.pcshell.dll");
    HANDLE hFile = CreateFileW(wszPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open twinui.pcshell.dll\n");
        return;
    }

    DWORD dwSize = GetFileSize(hFile, nullptr);
    PBYTE pFile = (PBYTE)malloc(dwSize);
    DWORD dwRead = 0;
    if (!ReadFile(hFile, pFile, dwSize, &dwRead, nullptr) || dwRead != dwSize)
    {
        printf("Failed to read twinui.pcshell.dll\n");
        goto cleanup;
    }

    if (IsWindows11())
    {
        // All patterns here have been tested to work on:
        // - 22621.1, 22621.1992, 22621.2134, 22621.2283, 22621.2359 (RP)
        // - 23545.1000
        // - 25951.1000

        if (!pOffsets[0] || pOffsets[0] == 0xFFFFFFFF)
        {
#if defined(_M_X64)
            // 48 8B 49 08 E8 ?? ?? ?? ?? E9 ?? ?? ?? ?? 48 8B 89
            //                ^^^^^^^^^^^
            // Ref: CMultitaskingViewFrame::v_WndProc()
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\x48\x8B\x49\x08\xE8\x00\x00\x00\x00\xE9\x00\x00\x00\x00\x48\x8B\x89",
                "xxxxx????x????xxx"
            );
            if (match)
            {
                match += 4;
                pOffsets[0] = (DWORD)(match + 5 + *(int*)(match + 1) - pFile);
            }
#elif defined(_M_ARM64)
            // ?? AE 00 71 ?? ?? 00 54 ?? 06 40 F9 E3 03 ?? AA E2 03 ?? AA E1 03 ?? 2A ?? ?? ?? ??
            //                                                                         ^^^^^^^^^^^
            // Ref: CMultitaskingViewFrame::v_WndProc()
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\xAE\x00\x71\x00\x00\x00\x54\x00\x06\x40\xF9\xE3\x03\x00\xAA\xE2\x03\x00\xAA\xE1\x03\x00\x2A",
                "xxx??xx?xxxxx?xxx?xxx?x"
            );
            if (match)
            {
                match += 23;
                pOffsets[0] = (DWORD)FileOffsetToRVA(pFile, (PBYTE)ARM64_FollowBL((DWORD*)match) - pFile);
            }
#endif
            if (pOffsets[0] && pOffsets[0] != 0xFFFFFFFF)
            {
                printf("CImmersiveContextMenuOwnerDrawHelper::s_ContextMenuWndProc() = %lX\n", pOffsets[0]);
            }
        }
        if ((!pOffsets[1] || pOffsets[1] == 0xFFFFFFFF) || (!pOffsets[6] || pOffsets[6] == 0xFFFFFFFF))
        {
            UINT_PTR* vtable = nullptr;
#if defined(_M_X64)
            // 48 8D 05 ?? ?? ?? ?? 48 8B D9 48 89 01 48 8D 05 ?? ?? ?? ?? 48 89 41 18 48 8D 05 ?? ?? ?? ?? 48 89 41 20 48 8D 05 ?? ?? ?? ?? 48 89 41 58 48 8D 05 ?? ?? ?? ?? 48 89 41 60
            //                                                                                                                   ^^^^^^^^^^^
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\x48\x8D\x05\x00\x00\x00\x00\x48\x8B\xD9\x48\x89\x01\x48\x8D\x05\x00\x00\x00\x00\x48\x89\x41\x18\x48\x8D\x05\x00\x00\x00\x00\x48\x89\x41\x20\x48\x8D\x05\x00\x00\x00\x00\x48\x89\x41\x58\x48\x8D\x05\x00\x00\x00\x00\x48\x89\x41\x60",
                "xxx????xxxxxxxxx????xxxxxxx????xxxxxxx????xxxxxxx????xxxx"
            );
            if (match)
            {
                match += 35; // Point to 48
                vtable = (UINT_PTR*)(match + 7 + *(int*)(match + 3));
            }
#elif defined(_M_ARM64)
            // * Pattern 1 (for 24H2):
            //   69 A2 01 A9 ?? ?? 00 ?? 09 ?? ?? 91 ?? ?? 00 ?? 08 ?? ?? 91 69 A2 05 A9 ?? ?? 00 ?? 08 ?? ?? 91 68 36 00 F9 ?? ?? 00 ?? 08 ?? ?? 91 68 3E 00 F9
            //               ^^^^^^^^^^^+^^^^^^^^^^^
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\x69\xA2\x01\xA9\x00\x00\x00\x00\x09\x00\x00\x91\x00\x00\x00\x00\x08\x00\x00\x91\x69\xA2\x05\xA9\x00\x00\x00\x00\x08\x00\x00\x91\x68\x36\x00\xF9\x00\x00\x00\x00\x08\x00\x00\x91\x68\x3E\x00\xF9",
                "xxxx??x?x??x??x?x??xxxxx??x?x??xxxxx??x?x??xxxxx"
            );
            // Patterns for 226xx are not implemented
            if (match)
            {
                match += 4; // Point to ADRP
                UINT_PTR vtableRVA = ARM64_DecodeADRL(FileOffsetToRVA(pFile, match - pFile), *(DWORD*)match, *(DWORD*)(match + 4));
                vtable = (UINT_PTR*)((UINT_PTR)pFile + RVAToFileOffset(pFile, vtableRVA));
            }
#endif
            if (vtable)
            {
                if (!pOffsets[6] || pOffsets[6] == 0xFFFFFFFF)
                {
                    pOffsets[6] = (DWORD)(vtable[3] - 0x180000000);
                }
                if (!pOffsets[1] || pOffsets[1] == 0xFFFFFFFF)
                {
                    pOffsets[1] = (DWORD)(vtable[4] - 0x180000000);
                }
            }
            if (pOffsets[6] && pOffsets[6] != 0xFFFFFFFF)
            {
                printf("CLauncherTipContextMenu::ShowLauncherTipContextMenu() = %lX\n", pOffsets[6]);
            }
            if (pOffsets[1] && pOffsets[1] != 0xFFFFFFFF)
            {
                printf("CLauncherTipContextMenu::GetMenuItemsAsync() = %lX\n", pOffsets[1]);
            }
        }
        if (!pOffsets[2] || pOffsets[2] == 0xFFFFFFFF)
        {
#if defined(_M_X64)
            // Don't worry if this is too long, this works on 17763 and 25951
            // 40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 8B B5 ? ? ? ? 41 8B C1
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\x40\x55\x53\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\xAC\x24\x00\x00\x00\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC4\x48\x89\x85\x00\x00\x00\x00\x4C\x8B\xB5\x00\x00\x00\x00\x41\x8B\xC1",
                "xxxxxxxxxxxxxxxxx????xxx????xxx????xxxxxx????xxx????xxx"
            );
            if (match)
            {
                pOffsets[2] = (DWORD)(match - pFile);
            }
#elif defined(_M_ARM64)
            // 40 F9 43 03 1C 32 E4 03 15 AA ?? ?? FF 97
            //                               ^^^^^^^^^^^
            // Ref: ImmersiveContextMenuHelper::ApplyOwnerDrawToMenu()
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\x40\xF9\x43\x03\x1C\x32\xE4\x03\x15\xAA\x00\x00\xFF\x97",
                "xxxxxxxxxx??xx"
            );
            if (match)
            {
                match += 10;
                pOffsets[2] = (DWORD)FileOffsetToRVA(pFile, (PBYTE)ARM64_FollowBL((DWORD*)match) - pFile);
            }
#endif
            if (pOffsets[2] && pOffsets[2] != 0xFFFFFFFF)
            {
                printf("ImmersiveContextMenuHelper::ApplyOwnerDrawToMenu() = %lX\n", pOffsets[2]);
            }
        }
        if (!pOffsets[3] || pOffsets[3] == 0xFFFFFFFF)
        {
#if defined(_M_X64)
            // 48 89 5C 24 ? 48 89 7C 24 ? 55 48 8B EC 48 83 EC 60 48 8B FA 48 8B D9 E8
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\x48\x89\x5C\x24\x00\x48\x89\x7C\x24\x00\x55\x48\x8B\xEC\x48\x83\xEC\x60\x48\x8B\xFA\x48\x8B\xD9\xE8",
                "xxxx?xxxx?xxxxxxxxxxxxxxx"
            );
            if (match)
            {
                pOffsets[3] = (DWORD)(match - pFile);
            }
#elif defined(_M_ARM64)
            // 7F 23 03 D5 F3 53 BF A9 FD 7B BB A9 FD 03 00 91 F3 03 00 AA F4 03 01 AA ?? ?? ?? ?? FF ?? 03 A9
            // ----------- PACIBSP, don't scan for this because it's everywhere
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\xF3\x53\xBF\xA9\xFD\x7B\xBB\xA9\xFD\x03\x00\x91\xF3\x03\x00\xAA\xF4\x03\x01\xAA\x00\x00\x00\x00\xFF\x00\x03\xA9",
                "xxxxxxxxxxxxxxxxxxxx????x?xx"
            );
            if (match)
            {
                match -= 4;
                pOffsets[3] = (DWORD)FileOffsetToRVA(pFile, match - pFile);
            }
#endif
            if (pOffsets[3] && pOffsets[3] != 0xFFFFFFFF)
            {
                printf("ImmersiveContextMenuHelper::RemoveOwnerDrawFromMenu() = %lX\n", pOffsets[3]);
            }
        }
        if (!pOffsets[4] || pOffsets[4] == 0xFFFFFFFF)
        {
#if defined(_M_X64)
            // 48 8B ? E8 ? ? ? ? 4C 8B ? 48 8B ? 48 8B CE E8 ? ? ? ? 90
            //                                                ^^^^^^^
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\x48\x8B\x00\xE8\x00\x00\x00\x00\x4C\x8B\x00\x48\x8B\x00\x48\x8B\xCE\xE8\x00\x00\x00\x00\x90",
                "xx?x????xx?xx?xxxx????x"
            );
            if (match)
            {
                match += 17;
                pOffsets[4] = (DWORD)(match + 5 + *(int*)(match + 1) - pFile);
            }
#elif defined(_M_ARM64)
            // 82 62 00 91 ?? A2 00 91 E0 03 ?? AA ?? ?? ?? ?? 1F 20 03 D5
            //                                     ^^^^^^^^^^^
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\x82\x62\x00\x91\x00\xA2\x00\x91\xE0\x03\x00\xAA\x00\x00\x00\x00\x1F\x20\x03\xD5",
                "xxxx?xxxxx?x????xxxx"
            );
            if (match)
            {
                match += 12;
                pOffsets[4] = (DWORD)FileOffsetToRVA(pFile, (PBYTE)ARM64_FollowBL((DWORD*)match) - pFile);
            }
#endif
            if (pOffsets[4] && pOffsets[4] != 0xFFFFFFFF)
            {
                printf("CLauncherTipContextMenu::_ExecuteShutdownCommand() = %lX\n", pOffsets[4]);
            }
        }
        if (!pOffsets[5] || pOffsets[5] == 0xFFFFFFFF)
        {
#if defined(_M_X64)
            // 48 8B ? E8 ? ? ? ? 48 8B D3 48 8B CF E8 ? ? ? ? 90 48 8D 56 ? 48 8B CE
            //                                         ^^^^^^^    ------------------- Non-inlined ~::final_suspend()
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\x48\x8B\x00\xE8\x00\x00\x00\x00\x48\x8B\xD3\x48\x8B\xCF\xE8\x00\x00\x00\x00\x90\x48\x8D\x56\x00\x48\x8B\xCE",
                "xx?x????xxxxxxx????xxxx?xxx"
            );
            if (match)
            {
                match += 14;
                pOffsets[5] = (DWORD)(match + 5 + *(int*)(match + 1) - pFile);
            }
            else
            {
                // 48 8B ? E8 ? ? ? ? 48 8B D3 48 8B CF E8 ? ? ? ? 90 48 8B 05 ? ? ? ? 48
                //                                         ^^^^^^^    ------------------- Inlined ~::final_suspend()
                match = (PBYTE)FindPattern(
                    pFile, dwSize,
                    "\x48\x8B\x00\xE8\x00\x00\x00\x00\x48\x8B\xD3\x48\x8B\xCF\xE8\x00\x00\x00\x00\x90\x48\x8B\x05\x00\x00\x00\x00\x48",
                    "xx?x????xxxxxxx????xxxx????x"
                );
                if (match)
                {
                    match += 14;
                    pOffsets[5] = (DWORD)(match + 5 + *(int*)(match + 1) - pFile);
                }
            }
#elif defined(_M_ARM64)
            // 08 09 40 F9 ?? 16 00 F9 ?? ?? ?? ?? ?? A2 00 91 E0 03 ?? AA ?? ?? ?? ?? 1F 20 03 D5
            //                                                             ^^^^^^^^^^^
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\x08\x09\x40\xF9\x00\x16\x00\xF9\x00\x00\x00\x00\x00\xA2\x00\x91\xE0\x03\x00\xAA\x00\x00\x00\x00\x1F\x20\x03\xD5",
                "xxxx?xxx?????xxxxx?x????xxxx"
            );
            if (match)
            {
                match += 20;
                pOffsets[5] = (DWORD)FileOffsetToRVA(pFile, (PBYTE)ARM64_FollowBL((DWORD*)match) - pFile);
            }
#endif
            if (pOffsets[5] && pOffsets[5] != 0xFFFFFFFF)
            {
                printf("CLauncherTipContextMenu::_ExecuteCommand() = %lX\n", pOffsets[5]);
            }
        }
        if (!pOffsets[7] || pOffsets[7] == 0xFFFFFFFF)
        {
#if defined(_M_X64)
            // Ref: CMultitaskingViewManager::_CreateMTVHost()
            // Inlined GetMTVHostKind()
            // 4C 89 74 24 ?? ?? 8B ?? ?? 8B ?? 8B D7 48 8B CE E8 ?? ?? ?? ?? 8B
            //                                                    ^^^^^^^^^^^
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\x4C\x89\x74\x24\x00\x00\x8B\x00\x00\x8B\x00\x8B\xD7\x48\x8B\xCE\xE8\x00\x00\x00\x00\x8B",
                "xxxx??x??x?xxxxxx????x"
            );
            if (match)
            {
                match += 16;
                pOffsets[7] = (DWORD)(match + 5 + *(int*)(match + 1) - pFile);
            }
            else
            {
                // Non-inlined GetMTVHostKind()
                // 8B CF E8 ?? ?? ?? ?? ?? 89 ?? 24 ?? ?? 8B ?? ?? 8B ?? 8B D7 48 8B CE 83 F8 01 <jnz>
                match = (PBYTE)FindPattern(
                    pFile, dwSize,
                    "\x8B\xCF\xE8\x00\x00\x00\x00\x00\x89\x00\x24\x00\x00\x8B\x00\x00\x8B\x00\x8B\xD7\x48\x8B\xCE\x83\xF8\x01",
                    "xxx?????x?x??x??x?xxxxxxxx"
                );
                if (match)
                {
                    PBYTE target = nullptr;
                    DWORD jnzSize = 0;
                    if (FollowJnz(match + 26, &target, &jnzSize))
                    {
                        match += 26 + jnzSize;
                        if (match[0] == 0xE8)
                        {
                            pOffsets[7] = (DWORD)(match + 5 + *(int*)(match + 1) - pFile);
                        }
                    }
                }
            }
#elif defined(_M_ARM64)
            // F3 53 BE A9  F5 5B 01 A9  FD 7B ?? A9  FD 03 00 91  30 00 80 92  F5 03 04 AA  B0 ?? 00 F9  F3 03 00 AA  BF 02 00 F9  68 2E 40 F9  F6 03 03 AA  B3 23 02 A9  ?? ?? 00 B5
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\xF3\x53\xBE\xA9\xF5\x5B\x01\xA9\xFD\x7B\x00\xA9\xFD\x03\x00\x91\x30\x00\x80\x92\xF5\x03\x04\xAA\xB0\x00\x00\xF9\xF3\x03\x00\xAA\xBF\x02\x00\xF9\x68\x2E\x40\xF9\xF6\x03\x03\xAA\xB3\x23\x02\xA9\x00\x00\x00\xB5",
                "xxxxxxxxxx?xxxxxxxxxxxxxx?xxxxxxxxxxxxxxxxxxxxxx??xx"
            );
            if (match)
            {
                pOffsets[7] = (DWORD)FileOffsetToRVA(pFile, match - 4 - pFile);
            }
#endif
            if (pOffsets[7] && pOffsets[7] != 0xFFFFFFFF)
            {
                printf("CMultitaskingViewManager::_CreateXamlMTVHost() = %lX\n", pOffsets[7]);
            }
        }
        if (!pOffsets[8] || pOffsets[8] == 0xFFFFFFFF)
        {
#if defined(_M_X64)
            // Ref: CMultitaskingViewManager::_CreateMTVHost()
            // Inlined GetMTVHostKind()
            // 4C 89 74 24 ?? ?? 8B ?? ?? 8B ?? 8B D7 48 8B CE E8 ?? ?? ?? ?? 90
            //                                                    ^^^^^^^^^^^
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\x4C\x89\x74\x24\x00\x00\x8B\x00\x00\x8B\x00\x8B\xD7\x48\x8B\xCE\xE8\x00\x00\x00\x00\x90",
                "xxxx??x??x?xxxxxx????x"
            );
            if (match)
            {
                match += 16;
                pOffsets[8] = (DWORD)(match + 5 + *(int*)(match + 1) - pFile);
            }
            else
            {
                // Non-inlined GetMTVHostKind()
                // 8B CF E8 ?? ?? ?? ?? ?? 89 ?? 24 ?? ?? 8B ?? ?? 8B ?? 8B D7 48 8B CE 83 F8 01 <jnz>
                match = (PBYTE)FindPattern(
                    pFile, dwSize,
                    "\x8B\xCF\xE8\x00\x00\x00\x00\x00\x89\x00\x24\x00\x00\x8B\x00\x00\x8B\x00\x8B\xD7\x48\x8B\xCE\x83\xF8\x01",
                    "xxx?????x?x??x??x?xxxxxxxx"
                );
                if (match)
                {
                    PBYTE target = nullptr;
                    DWORD jnzSize = 0;
                    if (FollowJnz(match + 26, &target, &jnzSize) && target[0] == 0xE8)
                    {
                        pOffsets[8] = (DWORD)(target + 5 + *(int*)(target + 1) - pFile);
                    }
                }
            }
#elif defined(_M_ARM64)
            // F3 53 BC A9  F5 5B 01 A9  F7 13 00 F9  F9 17 00 F9  FB 1B 00 F9  FD 7B BC A9  FD 03 00 91  FF ?? 00 D1  30 00 80 92  FB 03 04 AA
            PBYTE match = (PBYTE)FindPattern(
                pFile, dwSize,
                "\xF3\x53\xBC\xA9\xF5\x5B\x01\xA9\xF7\x13\x00\xF9\xF9\x17\x00\xF9\xFB\x1B\x00\xF9\xFD\x7B\xBC\xA9\xFD\x03\x00\x91\xFF\x00\x00\xD1\x30\x00\x80\x92\xFB\x03\x04\xAA",
                "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx?xxxxxxxxxx"
            );
            if (match)
            {
                pOffsets[8] = (DWORD)FileOffsetToRVA(pFile, match - 4 - pFile);
            }
#endif
            if (pOffsets[8] && pOffsets[8] != 0xFFFFFFFF)
            {
                printf("CMultitaskingViewManager::_CreateDCompMTVHost() = %lX\n", pOffsets[8]);
            }
        }
    }

cleanup:
    free(pFile);
    CloseHandle(hFile);
}

extern "C" void RunTwinUIPCShellPatches(symbols_addr* symbols_PTRS)
{
    HMODULE hTwinuiPcshell = LoadLibraryW(L"twinui.pcshell.dll");
    MODULEINFO miTwinuiPcshell;
    GetModuleInformation(GetCurrentProcess(), hTwinuiPcshell, &miTwinuiPcshell, sizeof(MODULEINFO));

    // ZeroMemory(symbols_PTRS->twinui_pcshell_PTRS, sizeof(symbols_PTRS->twinui_pcshell_PTRS)); // Uncomment for testing
    TryToFindTwinuiPCShellOffsets(symbols_PTRS->twinui_pcshell_PTRS);

    if (symbols_PTRS->twinui_pcshell_PTRS[0] && symbols_PTRS->twinui_pcshell_PTRS[0] != 0xFFFFFFFF)
    {
        CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc = (decltype(CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProcFunc))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS->twinui_pcshell_PTRS[0]);
    }

    if (symbols_PTRS->twinui_pcshell_PTRS[1] && symbols_PTRS->twinui_pcshell_PTRS[1] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_GetMenuItemsAsyncFunc = (decltype(CLauncherTipContextMenu_GetMenuItemsAsyncFunc))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS->twinui_pcshell_PTRS[1]);
    }

    if (symbols_PTRS->twinui_pcshell_PTRS[2] && symbols_PTRS->twinui_pcshell_PTRS[2] != 0xFFFFFFFF)
    {
        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc = (decltype(ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS->twinui_pcshell_PTRS[2]);
    }

    if (symbols_PTRS->twinui_pcshell_PTRS[3] && symbols_PTRS->twinui_pcshell_PTRS[3] != 0xFFFFFFFF)
    {
        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc = (decltype(ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS->twinui_pcshell_PTRS[3]);
    }

    if (symbols_PTRS->twinui_pcshell_PTRS[4] && symbols_PTRS->twinui_pcshell_PTRS[4] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_ExecuteShutdownCommandFunc = (decltype(CLauncherTipContextMenu_ExecuteShutdownCommandFunc))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS->twinui_pcshell_PTRS[4]);
    }

    if (symbols_PTRS->twinui_pcshell_PTRS[5] && symbols_PTRS->twinui_pcshell_PTRS[5] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_ExecuteCommandFunc = (decltype(CLauncherTipContextMenu_ExecuteCommandFunc))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS->twinui_pcshell_PTRS[5]);
    }

    int rv = -1;
    if (symbols_PTRS->twinui_pcshell_PTRS[6] && symbols_PTRS->twinui_pcshell_PTRS[6] != 0xFFFFFFFF)
    {
        CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc = (decltype(CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS->twinui_pcshell_PTRS[6]);
        rv = funchook_prepare(
            funchook,
            (void**)&CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc,
            CLauncherTipContextMenu_ShowLauncherTipContextMenuHook
        );
    }
    if (rv != 0)
    {
        printf("Failed to hook CLauncherTipContextMenu::ShowLauncherTipContextMenu(). rv = %d\n", rv);
    }

    if (IsWindows11())
    {
        rv = -1;
        if (symbols_PTRS->twinui_pcshell_PTRS[7] && symbols_PTRS->twinui_pcshell_PTRS[7] != 0xFFFFFFFF)
        {
            twinui_pcshell_CMultitaskingViewManager__CreateDCompMTVHostFunc = (decltype(twinui_pcshell_CMultitaskingViewManager__CreateDCompMTVHostFunc))
                ((uintptr_t)hTwinuiPcshell + symbols_PTRS->twinui_pcshell_PTRS[8]);
            twinui_pcshell_CMultitaskingViewManager__CreateXamlMTVHostFunc = (decltype(twinui_pcshell_CMultitaskingViewManager__CreateXamlMTVHostFunc))
                ((uintptr_t)hTwinuiPcshell + symbols_PTRS->twinui_pcshell_PTRS[7]);
            rv = funchook_prepare(
                funchook,
                (void**)&twinui_pcshell_CMultitaskingViewManager__CreateXamlMTVHostFunc,
                twinui_pcshell_CMultitaskingViewManager__CreateXamlMTVHostHook
            );
        }
        if (rv != 0)
        {
            printf("Failed to hook CMultitaskingViewManager::_CreateXamlMTVHost(). rv = %d\n", rv);
        }
    }

    /*rv = -1;
    if (symbols_PTRS->twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT - 1] && symbols_PTRS->twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT - 1] != 0xFFFFFFFF)
    {
        winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc = (INT64(*)(void*, POINT*))
            ((uintptr_t)hTwinuiPcshell + symbols_PTRS->twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT - 1]);
        rv = funchook_prepare(
            funchook,
            (void**)&winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageFunc,
            winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessageHook
        );
    }
    if (rv != 0)
    {
        printf("Failed to hook winrt_Windows_Internal_Shell_implementation_MeetAndChatManager_OnMessage(). rv = %d\n", rv);
    }*/

#if USE_MOMENT_3_FIXES_ON_MOMENT_2
    // Use this only for testing, since the RtlQueryFeatureConfiguration() hook is perfect.
    // Only tested on 22621.1992.
    BOOL bPerformMoment2Patches = IsWindows11Version22H2Build1413OrHigher();
#else
    // This is the only way to fix stuff since the flag "26008830" and the code when it's not enabled are gone.
    // Tested on:
    // - 22621.2134, 22621.2283, 22621.2359 (RP)
    // - 23545.1000
    BOOL bPerformMoment2Patches = IsWindows11Version22H2Build2134OrHigher();
#endif
    if (bOldTaskbar != 1)
    {
        bPerformMoment2Patches = FALSE;
    }
    if (bPerformMoment2Patches)
    {
        // Fix flyout placement: Our goal with these patches is to get `mi.rcWork` assigned
        Moment2PatchActionCenter(&miTwinuiPcshell);
        Moment2PatchControlCenter(&miTwinuiPcshell);
        Moment2PatchToastCenter(&miTwinuiPcshell);

        // Fix task view
        Moment2PatchTaskView(&miTwinuiPcshell);

        // Fix volume and brightness popups
        HMODULE hHardwareConfirmator = LoadLibraryW(L"Windows.Internal.HardwareConfirmator.dll");
        MODULEINFO miHardwareConfirmator;
        GetModuleInformation(GetCurrentProcess(), hHardwareConfirmator, &miHardwareConfirmator, sizeof(MODULEINFO));
        Moment2PatchHardwareConfirmator(&miHardwareConfirmator);

        // Fix pen menu
#if defined(_M_X64)
        // 48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 50 49 8B ? 48 81 C1
        PBYTE match = (PBYTE)FindPattern(
            hTwinuiPcshell,
            miTwinuiPcshell.SizeOfImage,
            "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x50\x49\x8B\x00\x48\x81\xC1",
            "xxxx?xxxx?xxxxxxx?xxx"
        );
#elif defined(_M_ARM64)
        PBYTE match = nullptr;
#endif
        rv = -1;
        if (match)
        {
            twinui_pcshell_PenMenuSystemTrayManager__GetDynamicSystemTrayHeightForMonitorFunc = (decltype(twinui_pcshell_PenMenuSystemTrayManager__GetDynamicSystemTrayHeightForMonitorFunc))match;
            printf("PenMenuSystemTrayManager::GetDynamicSystemTrayHeightForMonitor() = %llX\n", match - (PBYTE)hTwinuiPcshell);
            rv = funchook_prepare(
               funchook,
               (void**)&twinui_pcshell_PenMenuSystemTrayManager__GetDynamicSystemTrayHeightForMonitorFunc,
               twinui_pcshell_PenMenuSystemTrayManager__GetDynamicSystemTrayHeightForMonitorHook
           );
        }
        if (rv != 0)
        {
            printf("Failed to hook PenMenuSystemTrayManager::GetDynamicSystemTrayHeightForMonitor(). rv = %d\n", rv);
        }
    }

    if ((global_rovi.dwBuildNumber > 22000 || global_rovi.dwBuildNumber == 22000 && global_ubr >= 65) // Allow on 22000.65+
        && (bOldTaskbar || dwStartShowClassicMode))
    {
        // Make sure crash counter is enabled. If one of the patches make Explorer crash while the start menu is open,
        // we don't want to softlock the user. The system reopens the start menu if Explorer terminates while it's open.
        if (IsCrashCounterEnabled())
        {
            if (FixStartMenuAnimation(&miTwinuiPcshell))
            {
                ReportSuccessfulAnimationPatching();
            }
        }
    }

    if (IsWindows11Version22H2OrHigher() && bOldTaskbar)
    {
        // Fix broken taskbar jump list positioning caused by 40874676
        FixJumpViewPositioning(&miTwinuiPcshell);
    }

    VnPatchIAT_NonInline(hTwinuiPcshell, "API-MS-WIN-CORE-REGISTRY-L1-1-0.DLL", "RegGetValueW", (uintptr_t)twinuipcshell_RegGetValueW);
    printf("Setup twinui.pcshell functions done\n");
}

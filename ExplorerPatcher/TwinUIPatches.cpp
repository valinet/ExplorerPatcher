#include <Windows.h>
#include <Shlwapi.h>
#include <initguid.h>

#include <wrl/client.h>
#include <wil/result_macros.h>

#include "utility.h"

using namespace Microsoft::WRL;

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

class CSingleViewShellExperience;

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

extern "C" HRESULT CStartExperienceManager_GetMonitorInformationHook(void* _this, CSingleViewShellExperience* experience, RECT* rcOutWorkArea, EDGEUI_TRAYSTUCKPLACE* outTrayStuckPlace, bool* bOutRtl, HMONITOR* hOutMonitor)
{
    *rcOutWorkArea = {};
    *outTrayStuckPlace = EUITSP_BOTTOM;
    *bOutRtl = false;
    if (hOutMonitor)
        *hOutMonitor = nullptr;

    ComPtr<IServiceProvider> spImmersiveShellServiceProvider;
    RETURN_IF_FAILED(CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER, IID_IServiceProvider, &spImmersiveShellServiceProvider));

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

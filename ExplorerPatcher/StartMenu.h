#ifndef _H_STARTMENU_H_
#define _H_STARTMENU_H_
#include <initguid.h>
#include <Windows.h>
#include <windowsx.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <TlHelp32.h>
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")
#include <roapi.h>
#include <winstring.h>
#include "utility.h"

#pragma comment(lib, "ntdll.lib")
EXTERN_C NTSYSAPI PIMAGE_NT_HEADERS NTAPI RtlImageNtHeader(PVOID);

extern DWORD bMonitorOverride;
extern DWORD bOpenAtLogon;

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

DEFINE_GUID(IID_WindowsUdk_UI_Shell_ITaskbarSettings6,
    0x5CBF9899,
    0x3E66, 0x5556, 0xA1, 0x31,
    0x1E, 0x3E, 0xE8, 0x14, 0x85, 0x90
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

    HRESULT(STDMETHODCALLTYPE* GetCount)(
        IImmersiveMonitorService* This);

    HRESULT(STDMETHODCALLTYPE* GetConnectedCount)(
        IImmersiveMonitorService* This);

    HRESULT(STDMETHODCALLTYPE* GetAt)(
        IImmersiveMonitorService* This);

    HRESULT(STDMETHODCALLTYPE* GetFromHandle)(
        IImmersiveMonitorService* This,
        /* [in] */ HMONITOR hMonitor,
        _COM_Outptr_  IUnknown** ppvObject);

    HRESULT(STDMETHODCALLTYPE* GetFromIdentity)(
        IImmersiveMonitorService* This);

    HRESULT(STDMETHODCALLTYPE* GetImmersiveProxyMonitor)(
        IImmersiveMonitorService* This);

    HRESULT(STDMETHODCALLTYPE* QueryService)(
        IImmersiveMonitorService* This,
        HMONITOR hMonitor,
        GUID*,
        GUID*,
        void** ppvObject
        );

    HRESULT(STDMETHODCALLTYPE* QueryServiceByIdentity)(
        IImmersiveMonitorService* This);

    HRESULT(STDMETHODCALLTYPE* QueryServiceFromWindow)(
        IImmersiveMonitorService* This,
        HWND hWnd,
        GUID* a3,
        GUID* a4,
        void** ppvObject
        );

    HRESULT(STDMETHODCALLTYPE* QueryServiceFromPoint)(
        IImmersiveMonitorService* This,
        POINT pt,
        GUID* a3,
        GUID* a4,
        void** ppvObject
        );

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

void OpenStartOnMonitor(HMONITOR monitor);

// Slightly tweaked version of function available in Open Shell 
// (Open-Shell-Menu\Src\StartMenu\StartMenuHelper\StartMenuHelper.cpp)
LRESULT CALLBACK OpenStartOnCurentMonitorThreadHook(
    int code,
    WPARAM wParam,
    LPARAM lParam
);

typedef DWORD OpenStartOnCurentMonitorThreadParams;
DWORD OpenStartOnCurentMonitorThread(OpenStartOnCurentMonitorThreadParams* unused);

typedef DWORD OpenStartAtLogonThreadParams;
DWORD OpenStartAtLogonThread(OpenStartAtLogonThreadParams* unused);

typedef struct _HookStartMenuParams
{
    HMODULE hModule;
    DWORD dwTimeout;
    wchar_t wszModulePath[MAX_PATH];
    FARPROC proc;
} HookStartMenuParams;
DWORD WINAPI HookStartMenu(HookStartMenuParams* params);

typedef interface WindowsUdk_UI_Shell_TaskbarLayoutStatics WindowsUdk_UI_Shell_TaskbarLayoutStatics;
typedef interface WindowsUdk_UI_Shell_TaskbarLayoutManager WindowsUdk_UI_Shell_TaskbarLayoutManager;

DEFINE_GUID(IID_WindowsUdk_UI_Shell_TaskbarLayoutStatics,
    0x4472FE8B,
    0xF3B1, 0x5CC9, 0x81, 0xc1,
    0x76, 0xf8, 0xc3, 0x38, 0x8a, 0xab
);

typedef struct WindowsUdk_UI_Shell_TaskbarLayoutStaticsVtbl // : IInspectableVtbl
{
    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutStatics* This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutStatics* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutStatics* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutStatics* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutStatics* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutStatics* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* get_Current)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutStatics* This,
        /* [out] */ __RPC__out void** _instanceof_winrt_WindowsUdk_UI_Shell_implementation_TaskbarLayout);

    END_INTERFACE
} WindowsUdk_UI_Shell_TaskbarLayoutStaticsVtbl;

interface WindowsUdk_UI_Shell_TaskbarLayoutStatics // : IInspectable
{
    CONST_VTBL struct WindowsUdk_UI_Shell_TaskbarLayoutStaticsVtbl* lpVtbl;
};

DEFINE_GUID(IID_WindowsUdk_UI_Shell_ITaskbarLayoutManager,
    0xFB10D7C4,
    0x4F7F, 0x5DE5, 0xA5, 0x28,
    0x7E, 0xFE, 0xF4, 0x18, 0xAA, 0x48
);

// Used in 23545+ (or maybe couple lower builds too). Still named ITaskbarLayoutManager but has different ReportMonitorAdded signature.
DEFINE_GUID(IID_WindowsUdk_UI_Shell_ITaskbarLayoutManager2,
    0x98F82ED2,
    0x4791, 0x58A0, 0x8D, 0x2F,
    0xDA, 0xBD, 0x7A, 0x2F, 0x18, 0x9F
);

typedef struct WindowsUdk_UI_Shell_TaskbarLayoutManagerVtbl // : IInspectableVtbl
{
    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutManager* This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutManager* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutManager* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutManager* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutManager* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutManager* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    union
    {
        HRESULT(STDMETHODCALLTYPE* ReportMonitorAdded)(
            __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutManager* This,
            __RPC__in unsigned __int64 hMonitor,
            __RPC__in void* _instance_of_winrt_WindowsUdk_UI_Shell_ITaskbarSettings,
            __RPC__in void* _unknown_shellViewToRectMap);

        HRESULT(STDMETHODCALLTYPE* ReportMonitorAdded2)(
            __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutManager* This,
            __RPC__in unsigned __int64 hMonitor,
            __RPC__in void* _instance_of_winrt_WindowsUdk_UI_Shell_ITaskbarSettings,
            __RPC__in void* _unknown_shellViewToRectMap,
            /* [out] */ __RPC__out unsigned __int64* result);
    };

    HRESULT(STDMETHODCALLTYPE* ReportMonitorRemoved)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutManager* This,
        __RPC__in unsigned __int64 hMonitor);

    HRESULT(STDMETHODCALLTYPE* ReportMonitorChanged)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutManager* This,
        __RPC__in unsigned __int64 hMonitor,
        __RPC__in LPRECT _unknown_lpGeometry);

    HRESULT(STDMETHODCALLTYPE* ReportSettingsForMonitor)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutManager* This,
        __RPC__in unsigned __int64 hMonitor,
        __RPC__in void* _instance_of_winrt_WindowsUdk_UI_Shell_ITaskbarSettings);

    HRESULT(STDMETHODCALLTYPE* ReportShellViewButtonBounds)(
        __RPC__in WindowsUdk_UI_Shell_TaskbarLayoutManager* This,
        __RPC__in unsigned __int64 hMonitor,
        __RPC__in void* _instanceof_winrt_WindowsUdk_UI_Shell_Bamo_ShellViewButtonBounds);

    END_INTERFACE
} WindowsUdk_UI_Shell_TaskbarLayoutManagerVtbl;

interface WindowsUdk_UI_Shell_TaskbarLayoutManager // : IInspectable
{
    CONST_VTBL struct WindowsUdk_UI_Shell_TaskbarLayoutManagerVtbl* lpVtbl;
};

typedef struct _MonitorListEntry
{
    HMONITOR hMonitor;
    unsigned __int64 token;
} MonitorListEntry;

typedef struct _StartMenuPositioningData
{
    DWORD location;
    DWORD operation;
    DWORD* pMonitorCount;
    MonitorListEntry* pMonitorList;
    DWORD i;
} StartMenuPositioningData;

#define STARTMENU_POSITIONING_OPERATION_ADD 0
#define STARTMENU_POSITIONING_OPERATION_REMOVE 1
#define STARTMENU_POSITIONING_OPERATION_CHANGE 3

BOOL NeedsRo_PositionStartMenuForMonitor(
    HMONITOR hMonitor,
    HDC unused1,
    LPRECT unused2,
    StartMenuPositioningData* data
);

DWORD GetStartMenuPosition(FARPROC SHRegGetValueFromHKCUHKLMFunc);
#endif

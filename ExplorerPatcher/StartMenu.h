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

extern DWORD bMonitorOverride;
extern DWORD bOpenAtLogon;

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

DEFINE_GUID(IID_ILauncherTipContextMenu,
    0xb8c1db5f,
    0xcbb3, 0x48bc, 0xaf, 0xd9,
    0xce, 0x6b, 0x88, 0x0c, 0x79, 0xed
);

typedef interface ILauncherTipContextMenu ILauncherTipContextMenu;

typedef struct ILauncherTipContextMenuVtbl
{
    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            ILauncherTipContextMenu* This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        ILauncherTipContextMenu* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        ILauncherTipContextMenu* This);

    HRESULT(STDMETHODCALLTYPE* ShowLauncherTipContextMenu)(
        ILauncherTipContextMenu* This,
        /* [in] */ POINT* pt);

    END_INTERFACE
} ILauncherTipContextMenuVtbl;

interface ILauncherTipContextMenu
{
    CONST_VTBL struct ILauncherTipContextMenuVtbl* lpVtbl;
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
#endif

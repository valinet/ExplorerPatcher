#ifndef _H_IMMERSIVEFLYOUTS_H_
#define _H_IMMERSIVEFLYOUTS_H_
#include <Windows.h>
#include <roapi.h>
#include "utility.h"

DEFINE_GUID(IID_TrayBatteryFlyoutExperienceManager,
    0x0a73aedc,
    0x1c68, 0x410d, 0x8d, 0x53,
    0x63, 0xaf, 0x80, 0x95, 0x1e, 0x8f
);
DEFINE_GUID(IID_TrayClockFlyoutExperienceManager,
    0xb1604325,
    0x6b59, 0x427b, 0xbf, 0x1b,
    0x80, 0xa2, 0xdb, 0x02, 0xd3, 0xd8
);
DEFINE_GUID(IID_TrayMtcUvcFlyoutExperienceManager,
    0x7154c95d,
    0xc519, 0x49bd, 0xa9, 0x7e,
    0x64, 0x5b, 0xbf, 0xab, 0xE1, 0x11
);
DEFINE_GUID(IID_NetworkFlyoutExperienceManager,
    0xC9DDC674,
    0xB44B, 0x4C67, 0x9D, 0x79,
    0x2B, 0x23, 0x7D, 0x9B, 0xE0, 0x5A
);
typedef interface IExperienceManager IExperienceManager;

typedef struct IExperienceManagerVtbl // : IInspectable
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        IExperienceManager* This,
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        IExperienceManager* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        IExperienceManager* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        IExperienceManager* This,
        ULONG* iidCount,
        IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        IExperienceManager* This,
        HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        IExperienceManager* This,
        TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* ShowFlyout)(
        IExperienceManager* This,
        /* [in] */ __x_ABI_CWindows_CFoundation_CRect* rect);

    HRESULT(STDMETHODCALLTYPE* HideFlyout)(
        IExperienceManager* This);

    END_INTERFACE
} IExperienceManagerVtbl;

interface IExperienceManager
{
    CONST_VTBL struct IExperienceManagerVtbl* lpVtbl;
};

DEFINE_GUID(CLSID_ShellExperienceManagerFactory,
    0x2E8FCB18,
    0xA0EE, 0x41AD, 0x8E, 0xF8,
    0x77, 0xFB, 0x3A, 0x37, 0x0C, 0xA5
);
typedef interface IShellExperienceManagerFactory IShellExperienceManagerFactory;

typedef struct IShellExperienceManagerFactoryVtbl // : IInspectable
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        IShellExperienceManagerFactory* This,
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        IShellExperienceManagerFactory* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        IShellExperienceManagerFactory* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        IShellExperienceManagerFactory* This,
        ULONG* iidCount,
        IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        IShellExperienceManagerFactory* This,
        HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        IShellExperienceManagerFactory* This,
        TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* GetExperienceManager)(
        IShellExperienceManagerFactory* This,
        /* [in] */ HSTRING* experience,
        _COM_Outptr_  IInspectable** ppvObject);

    END_INTERFACE
} IShellExperienceManagerFactoryVtbl;

interface IShellExperienceManagerFactory
{
    CONST_VTBL struct IShellExperienceManagerFactoryVtbl* lpVtbl;
};

DEFINE_GUID(IID_ActionCenterExperienceManager,
    0xdec04b18,
    0x357e, 0x41d8, 0x9b, 0x71,
    0xb9, 0x91, 0x24, 0x3b, 0xea, 0x34
);
DEFINE_GUID(IID_ControlCenterExperienceManager,
    0xd669a58e,
    0x6b18, 0x4d1d, 0x90, 0x04,
    0xa8, 0x86, 0x2a, 0xdb, 0x0a, 0x20
);
typedef interface IActionCenterOrControlCenterExperienceManager IActionCenterOrControlCenterExperienceManager;

typedef struct IActionCenterOrControlCenterExperienceManagerVtbl // : IInspectable
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        IActionCenterOrControlCenterExperienceManager* This,
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        IActionCenterOrControlCenterExperienceManager* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        IActionCenterOrControlCenterExperienceManager* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        IActionCenterOrControlCenterExperienceManager* This,
        ULONG* iidCount,
        IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        IActionCenterOrControlCenterExperienceManager* This,
        HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        IActionCenterOrControlCenterExperienceManager* This,
        TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* HotKeyInvoked)(
        IActionCenterOrControlCenterExperienceManager* This,
        /* [in] */ void* kind);

    HRESULT(STDMETHODCALLTYPE* Show)( // only in control center
        IActionCenterOrControlCenterExperienceManager* This,
        HSTRING hstringUnknown,
        void* bSupressAnimations,
        void* dwUnknown_ShouldBeOne);

    HRESULT(STDMETHODCALLTYPE* Hide)( // only in control center
        IActionCenterOrControlCenterExperienceManager* This,
        HSTRING hstringUnknown,
        void* bSupressAnimations);

    END_INTERFACE
} IActionCenterOrControlCenterExperienceManagerVtbl;

interface IActionCenterOrControlCenterExperienceManager
{
    CONST_VTBL struct IActionCenterOrControlCenterExperienceManagerVtbl* lpVtbl;
};

void InvokeActionCenter();

#define INVOKE_FLYOUT_SHOW 1
#define INVOKE_FLYOUT_HIDE 2
#define INVOKE_FLYOUT_NETWORK 1
#define INVOKE_FLYOUT_CLOCK 2
#define INVOKE_FLYOUT_BATTERY 3
#define INVOKE_FLYOUT_SOUND 4

HRESULT InvokeFlyoutRect(BOOL bAction, DWORD dwWhich, __x_ABI_CWindows_CFoundation_CRect* pRc);

inline HRESULT InvokeFlyout(BOOL bAction, DWORD dwWhich)
{
    __x_ABI_CWindows_CFoundation_CRect rc;
    ZeroMemory(&rc, sizeof(rc));
    return InvokeFlyoutRect(bAction, dwWhich, &rc);
}

#endif
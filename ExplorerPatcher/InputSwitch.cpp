#include "InputSwitch.h"

#include <shellscalingapi.h>
#include <wrl/implements.h>
#include <wil/result_macros.h>

#define TB_POS_NOWHERE 0
#define TB_POS_BOTTOM 1
#define TB_POS_TOP 2
#define TB_POS_LEFT 3
#define TB_POS_RIGHT 4
extern "C" UINT GetTaskbarLocationAndSize(POINT ptCursor, RECT* rc);

extern "C" __MIDL___MIDL_itf_inputswitchserver_0000_0000_0001 dwIMEStyle;
extern "C" HRESULT CInputSwitchControl_ModifyAnchor(UINT dwNumberOfProfiles, RECT* lpRect);

HRESULT CInputSwitchControl_ModifyAnchor(UINT dwNumberOfProfiles, RECT* lpRect)
{
    if (!dwIMEStyle) // impossible case (this is not called for the Windows 11 language switcher), but just in case
    {
        return S_FALSE;
    }

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

    return S_OK;
}

class CInputSwitchControlProxy : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IInputSwitchControl>
{
public:
    CInputSwitchControlProxy()
        : m_type((__MIDL___MIDL_itf_inputswitchserver_0000_0000_0001)-1)
    {
    }

    HRESULT RuntimeClassInitialize(IInputSwitchControl* original)
    {
        m_original = original;
        return S_OK;
    }

    STDMETHODIMP Init(__MIDL___MIDL_itf_inputswitchserver_0000_0000_0001 type) override
    {
        m_type = type;
        return m_original->Init(type == ISCT_IDL_DESKTOP && dwIMEStyle != ISCT_IDL_DESKTOP ? dwIMEStyle : type);
    }

    STDMETHODIMP ShowInputSwitch(const RECT* rect) override
    {
        RECT myRect = *rect;
        if (m_type == ISCT_IDL_DESKTOP)
        {
            UINT dwNumberOfProfiles = 0;
            BOOL bImePresent = FALSE;
            m_original->GetProfileCount(&dwNumberOfProfiles, &bImePresent);
            CInputSwitchControl_ModifyAnchor(dwNumberOfProfiles, &myRect);
        }
        return m_original->ShowInputSwitch(&myRect);
    }

    STDMETHODIMP SetCallback(IInputSwitchCallback* callback) override { return m_original->SetCallback(callback); }
    STDMETHODIMP GetProfileCount(UINT* count, BOOL* bOutImePresent) override { return m_original->GetProfileCount(count, bOutImePresent); }
    STDMETHODIMP GetCurrentProfile(__MIDL___MIDL_itf_inputswitchserver_0000_0000_0002* data) override { return m_original->GetCurrentProfile(data); }
    STDMETHODIMP RegisterHotkeys() override { return m_original->RegisterHotkeys(); }
    STDMETHODIMP ClickImeModeItem(__MIDL___MIDL_itf_inputswitchserver_0000_0000_0004 type, POINT point, const RECT* rect) override { return m_original->ClickImeModeItem(type, point, rect); }
    STDMETHODIMP ForceHide() override { return m_original->ForceHide(); }
    STDMETHODIMP ShowTouchKeyboardInputSwitch(const RECT* rect, __MIDL___MIDL_itf_inputswitchserver_0000_0000_0006 align, int a3, DWORD a4, __MIDL___MIDL_itf_inputswitchserver_0000_0000_0005 a5) override { return m_original->ShowTouchKeyboardInputSwitch(rect, align, a3, a4, a5); }
    STDMETHODIMP GetContextFlags(DWORD* flags) override { return m_original->GetContextFlags(flags); }
    STDMETHODIMP SetContextOverrideMode(__MIDL___MIDL_itf_inputswitchserver_0000_0000_0008 mode) override { return m_original->SetContextOverrideMode(mode); }
    STDMETHODIMP GetCurrentImeModeItem(__MIDL___MIDL_itf_inputswitchserver_0000_0000_0003* data) override { return m_original->GetCurrentImeModeItem(data); }
    STDMETHODIMP ActivateInputProfile(const WCHAR* profile) override { return m_original->ActivateInputProfile(profile); }
    STDMETHODIMP SetUserSid(const WCHAR* sid) override { return m_original->SetUserSid(sid); }

private:
    Microsoft::WRL::ComPtr<IInputSwitchControl> m_original;
    __MIDL___MIDL_itf_inputswitchserver_0000_0000_0001 m_type;
};

HRESULT CInputSwitchControlProxy_CreateInstance(IInputSwitchControl* original, REFIID riid, void** ppvObject)
{
    Microsoft::WRL::ComPtr<CInputSwitchControlProxy> proxy;
    RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CInputSwitchControlProxy>(&proxy, original));
    RETURN_HR(proxy.CopyTo(riid, ppvObject));
}

class CInputSwitchControlProxySV2 : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IInputSwitchControlSV2>
{
public:
    CInputSwitchControlProxySV2()
        : m_type((__MIDL___MIDL_itf_inputswitchserver_0000_0000_0001)-1)
    {
    }

    HRESULT RuntimeClassInitialize(IInputSwitchControlSV2* original)
    {
        m_original = original;
        return S_OK;
    }

    STDMETHODIMP Init(__MIDL___MIDL_itf_inputswitchserver_0000_0000_0001 type) override
    {
        m_type = type;
        return m_original->Init(type == ISCT_IDL_DESKTOP && dwIMEStyle != ISCT_IDL_DESKTOP ? dwIMEStyle : type);
    }

    STDMETHODIMP ShowInputSwitch(const RECT* rect) override
    {
        RECT myRect = *rect;
        if (m_type == ISCT_IDL_DESKTOP)
        {
            UINT dwNumberOfProfiles = 0;
            BOOL bImePresent = FALSE;
            m_original->GetProfileCount(&dwNumberOfProfiles, &bImePresent);
            CInputSwitchControl_ModifyAnchor(dwNumberOfProfiles, &myRect);
        }
        return m_original->ShowInputSwitch(&myRect);
    }

    STDMETHODIMP SetCallback(IInputSwitchCallback* callback) override { return m_original->SetCallback(callback); }
    STDMETHODIMP GetProfileCount(UINT* count, BOOL* bOutImePresent) override { return m_original->GetProfileCount(count, bOutImePresent); }
    STDMETHODIMP GetCurrentProfile(__MIDL___MIDL_itf_inputswitchserver_0000_0000_0002* data) override { return m_original->GetCurrentProfile(data); }
    STDMETHODIMP RegisterHotkeys() override { return m_original->RegisterHotkeys(); }
    STDMETHODIMP ClickImeModeItem(__MIDL___MIDL_itf_inputswitchserver_0000_0000_0004 type, POINT point, const RECT* rect) override { return m_original->ClickImeModeItem(type, point, rect); }
    STDMETHODIMP ClickImeModeItemWithAnchor(__MIDL___MIDL_itf_inputswitchserver_0000_0000_0004 type, IUnknown* anchor) override { return m_original->ClickImeModeItemWithAnchor(type, anchor); }
    STDMETHODIMP ForceHide() override { return m_original->ForceHide(); }
    STDMETHODIMP ShowTouchKeyboardInputSwitch(const RECT* rect, __MIDL___MIDL_itf_inputswitchserver_0000_0000_0006 align, int a3, DWORD a4, __MIDL___MIDL_itf_inputswitchserver_0000_0000_0005 a5) override { return m_original->ShowTouchKeyboardInputSwitch(rect, align, a3, a4, a5); }
    STDMETHODIMP GetContextFlags(DWORD* flags) override { return m_original->GetContextFlags(flags); }
    STDMETHODIMP SetContextOverrideMode(__MIDL___MIDL_itf_inputswitchserver_0000_0000_0008 mode) override { return m_original->SetContextOverrideMode(mode); }
    STDMETHODIMP GetCurrentImeModeItem(__MIDL___MIDL_itf_inputswitchserver_0000_0000_0003* data) override { return m_original->GetCurrentImeModeItem(data); }
    STDMETHODIMP ActivateInputProfile(const WCHAR* profile) override { return m_original->ActivateInputProfile(profile); }
    STDMETHODIMP SetUserSid(const WCHAR* sid) override { return m_original->SetUserSid(sid); }

private:
    Microsoft::WRL::ComPtr<IInputSwitchControlSV2> m_original;
    __MIDL___MIDL_itf_inputswitchserver_0000_0000_0001 m_type;
};

HRESULT CInputSwitchControlProxySV2_CreateInstance(IInputSwitchControlSV2* original, REFIID riid, void** ppvObject)
{
    Microsoft::WRL::ComPtr<CInputSwitchControlProxySV2> proxy;
    RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CInputSwitchControlProxySV2>(&proxy, original));
    RETURN_HR(proxy.CopyTo(riid, ppvObject));
}

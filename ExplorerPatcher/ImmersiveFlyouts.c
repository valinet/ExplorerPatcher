#include "ImmersiveFlyouts.h"

void InvokeActionCenter()
{
    HRESULT hr = S_OK;
    IUnknown* pImmersiveShell = NULL;
    hr = CoCreateInstance(
        &CLSID_ImmersiveShell,
        NULL,
        CLSCTX_NO_CODE_DOWNLOAD | CLSCTX_LOCAL_SERVER,
        &IID_IServiceProvider,
        &pImmersiveShell
    );
    if (SUCCEEDED(hr))
    {
        IShellExperienceManagerFactory* pShellExperienceManagerFactory = NULL;
        IUnknown_QueryService(
            pImmersiveShell,
            &CLSID_ShellExperienceManagerFactory,
            &CLSID_ShellExperienceManagerFactory,
            &pShellExperienceManagerFactory
        );
        if (pShellExperienceManagerFactory)
        {
            HSTRING_HEADER hstringHeader;
            HSTRING hstring = NULL;
            hr = WindowsCreateStringReference(
                L"Windows.Internal.ShellExperience.ControlCenter",
                (UINT32)(sizeof(L"Windows.Internal.ShellExperience.ControlCenter") / sizeof(wchar_t) - 1),
                &hstringHeader,
                &hstring
            );
            if (hstring)
            {
                IUnknown* pIntf = NULL;
                pShellExperienceManagerFactory->lpVtbl->GetExperienceManager(
                    pShellExperienceManagerFactory,
                    hstring,
                    &pIntf
                );
                if (pIntf)
                {
                    IActionCenterOrControlCenterExperienceManager* pControlCenterExperienceManager = NULL;
                    pIntf->lpVtbl->QueryInterface(pIntf, &IID_ControlCenterExperienceManager, &pControlCenterExperienceManager);
                    if (pControlCenterExperienceManager)
                    {
                        pControlCenterExperienceManager->lpVtbl->HotKeyInvoked(pControlCenterExperienceManager, 0);
                        pControlCenterExperienceManager->lpVtbl->Release(pControlCenterExperienceManager);
                    }
                }
                WindowsDeleteString(hstring);
            }
            pShellExperienceManagerFactory->lpVtbl->Release(pShellExperienceManagerFactory);
        }
        pImmersiveShell->lpVtbl->Release(pImmersiveShell);
    }
}

HRESULT InvokeFlyoutRect(BOOL bAction, DWORD dwWhich, __x_ABI_CWindows_CFoundation_CRect* pRc)
{
    HRESULT hr = S_OK;
    IUnknown* pImmersiveShell = NULL;
    hr = CoCreateInstance(
        &CLSID_ImmersiveShell,
        NULL,
        CLSCTX_NO_CODE_DOWNLOAD | CLSCTX_LOCAL_SERVER,
        &IID_IServiceProvider,
        &pImmersiveShell
    );
    if (SUCCEEDED(hr))
    {
        IShellExperienceManagerFactory* pShellExperienceManagerFactory = NULL;
        hr = IUnknown_QueryService(
            pImmersiveShell,
            &CLSID_ShellExperienceManagerFactory,
            &CLSID_ShellExperienceManagerFactory,
            &pShellExperienceManagerFactory
        );
        if (SUCCEEDED(hr))
        {
            HSTRING_HEADER hstringHeader;
            HSTRING hstring = NULL;
            WCHAR* pwszStr = NULL;
            switch (dwWhich)
            {
            case INVOKE_FLYOUT_NETWORK:
                pwszStr = L"Windows.Internal.ShellExperience.NetworkFlyout";
                break;
            case INVOKE_FLYOUT_CLOCK:
                pwszStr = L"Windows.Internal.ShellExperience.TrayClockFlyout";
                break;
            case INVOKE_FLYOUT_BATTERY:
                pwszStr = L"Windows.Internal.ShellExperience.TrayBatteryFlyout";
                break;
            case INVOKE_FLYOUT_SOUND:
                pwszStr = L"Windows.Internal.ShellExperience.MtcUvc";
                break;
            }
            hr = WindowsCreateStringReference(
                pwszStr,
                pwszStr ? wcslen(pwszStr) : 0,
                &hstringHeader,
                &hstring
            );
            if (SUCCEEDED(hr))
            {
                IUnknown* pIntf = NULL;
                hr = pShellExperienceManagerFactory->lpVtbl->GetExperienceManager(
                    pShellExperienceManagerFactory,
                    hstring,
                    &pIntf
                );
                if (SUCCEEDED(hr))
                {
                    IExperienceManager* pExperienceManager = NULL;
                    hr = pIntf->lpVtbl->QueryInterface(
                        pIntf,
                        dwWhich == INVOKE_FLYOUT_NETWORK ? &IID_NetworkFlyoutExperienceManager :
                        (dwWhich == INVOKE_FLYOUT_CLOCK ? &IID_TrayClockFlyoutExperienceManager :
                            (dwWhich == INVOKE_FLYOUT_BATTERY ? &IID_TrayBatteryFlyoutExperienceManager :
                                (dwWhich == INVOKE_FLYOUT_SOUND ? &IID_TrayMtcUvcFlyoutExperienceManager : &IID_IUnknown))),
                        &pExperienceManager
                    );
                    if (SUCCEEDED(hr))
                    {
                        if (bAction == INVOKE_FLYOUT_SHOW)
                        {
                            hr = pExperienceManager->lpVtbl->ShowFlyout(pExperienceManager, pRc);
                        }
                        else if (bAction == INVOKE_FLYOUT_HIDE)
                        {
                            hr = pExperienceManager->lpVtbl->HideFlyout(pExperienceManager);
                        }
                        pExperienceManager->lpVtbl->Release(pExperienceManager);
                    }

                }
                WindowsDeleteString(hstring);
            }
            pShellExperienceManagerFactory->lpVtbl->Release(pShellExperienceManagerFactory);
        }
        pImmersiveShell->lpVtbl->Release(pImmersiveShell);
    }
    return hr;
}

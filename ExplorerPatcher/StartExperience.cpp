#include "StartExperience.h"

#include <wrl/implements.h>
#include <wil/wrl.h>

using namespace Microsoft::WRL;

namespace wis = ABI::Windows::Internal::Shell;
namespace wf = ABI::Windows::Foundation;

using namespace ABI;

class CStartExperienceWrapper
    : public RuntimeClass<RuntimeClassFlags<WinRtClassicComMix>
        , wis::StartUI::IStartExperience
    >
{
    InspectableClass(L"", BaseTrust);

public:
    CStartExperienceWrapper() = default;
    HRESULT RuntimeClassInitialize(IStartExperience* pInner);

    //~ Begin wis::StartUI::IStartExperience Interface
    STDMETHODIMP ProcessBackgroundImage(IInspectable* a1, int a2, INT64* a3) override;
    STDMETHODIMP get_WaitableExplorerProcessHandle(UINT64* a1) override;
    STDMETHODIMP PromoteAppsToTopOfFrequentList(wf::Collections::IVector<HSTRING>* a1) override;
    STDMETHODIMP add_GlobalAnimationRequested(
        wf::ITypedEventHandler<
            wis::StartUI::StartExperience*, wis::StartUI::StartExperienceAnimationRequestedEventArgs*>* a1,
        EventRegistrationToken* token) override;
    STDMETHODIMP remove_GlobalAnimationRequested(EventRegistrationToken token) override;
    STDMETHODIMP add_ShellModeChanged(
        wf::ITypedEventHandler<wis::StartUI::StartExperience*, wis::StartUI::ShellModeChangedEventArgs*>* a1,
        EventRegistrationToken* token) override;
    STDMETHODIMP remove_ShellModeChanged(EventRegistrationToken token) override;
    //~ End wis::StartUI::IStartExperience Interface

private:
    ComPtr<IStartExperience> _spInner;
};

HRESULT CStartExperienceWrapper::RuntimeClassInitialize(IStartExperience* pInner)
{
    _spInner = pInner;
    return S_OK;
}

HRESULT CStartExperienceWrapper::ProcessBackgroundImage(IInspectable* a1, int a2, INT64* a3)
{
    return _spInner->ProcessBackgroundImage(a1, a2, a3);
}

HRESULT CStartExperienceWrapper::get_WaitableExplorerProcessHandle(UINT64* a1)
{
    return _spInner->get_WaitableExplorerProcessHandle(a1);
}

HRESULT CStartExperienceWrapper::PromoteAppsToTopOfFrequentList(wf::Collections::IVector<HSTRING>* a1)
{
    return _spInner->PromoteAppsToTopOfFrequentList(a1);
}

HRESULT CStartExperienceWrapper::add_GlobalAnimationRequested(
    wf::ITypedEventHandler<
        wis::StartUI::StartExperience*, wis::StartUI::StartExperienceAnimationRequestedEventArgs*>* a1,
        EventRegistrationToken* token)
{
    return _spInner->add_GlobalAnimationRequested(a1, token);
}

HRESULT CStartExperienceWrapper::remove_GlobalAnimationRequested(EventRegistrationToken token)
{
    return _spInner->remove_GlobalAnimationRequested(token);
}

HRESULT CStartExperienceWrapper::add_ShellModeChanged(
    wf::ITypedEventHandler<
        wis::StartUI::StartExperience*, wis::StartUI::ShellModeChangedEventArgs*>* a1, EventRegistrationToken* token)
{
    *token = {};
    return S_OK;
}

HRESULT CStartExperienceWrapper::remove_ShellModeChanged(EventRegistrationToken token)
{
    return S_OK;
}

class CStartExperienceStaticsWrapper
    : public RuntimeClass<RuntimeClassFlags<WinRtClassicComMix>
        , wis::StartUI::IStartExperienceStatics
    >
{
    InspectableClass(L"", BaseTrust);

public:
    CStartExperienceStaticsWrapper() = default;
    HRESULT RuntimeClassInitialize(IStartExperienceStatics* pInner);

    //~ Begin wis::StartUI::IStartExperienceStatics Interface
    STDMETHODIMP GetForCurrentView(wis::StartUI::IStartExperience** ppStartExperience) override;
    STDMETHODIMP GetIfNoCurrentView(wis::StartUI::IStartExperience** ppStartExperience) override;
    //~ End wis::StartUI::IStartExperienceStatics Interface

private:
    ComPtr<IStartExperienceStatics> _spInner;
};

HRESULT CStartExperienceStaticsWrapper::RuntimeClassInitialize(IStartExperienceStatics* pInner)
{
    _spInner = pInner;
    return S_OK;
}

HRESULT CStartExperienceStaticsWrapper::GetForCurrentView(wis::StartUI::IStartExperience** ppStartExperience)
{
    ComPtr<wis::StartUI::IStartExperience> spStartExperience;
    RETURN_IF_FAILED(_spInner->GetForCurrentView(&spStartExperience));
    RETURN_HR(MakeAndInitialize<CStartExperienceWrapper>(ppStartExperience, spStartExperience.Get()));
}

HRESULT CStartExperienceStaticsWrapper::GetIfNoCurrentView(wis::StartUI::IStartExperience** ppStartExperience)
{
    ComPtr<wis::StartUI::IStartExperience> spStartExperience;
    RETURN_IF_FAILED(_spInner->GetIfNoCurrentView(&spStartExperience));
    RETURN_HR(MakeAndInitialize<CStartExperienceWrapper>(ppStartExperience, spStartExperience.Get()));
}

EXTERN_C HRESULT StartExperienceWrapper_Wrap(REFIID riid, void** ppv)
{
    ComPtr<wis::StartUI::IStartExperienceStatics> spInner;
    spInner.Attach(*reinterpret_cast<wis::StartUI::IStartExperienceStatics**>(ppv));
    *ppv = nullptr;

    ComPtr<CStartExperienceStaticsWrapper> spInstance;
    RETURN_IF_FAILED(MakeAndInitialize<CStartExperienceStaticsWrapper>(&spInstance, spInner.Get()));
    RETURN_HR(spInstance.CopyTo(riid, ppv));
}

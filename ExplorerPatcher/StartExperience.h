#pragma once

#include <Windows.h>
#include <Windows.Foundation.h>
#include <Windows.Foundation.Collections.h>

#include <initguid.h>
DEFINE_GUID(IID_IStartExperience, 0x4C4D0C66, 0x5BD5, 0xFD8C, 0x61, 0x7F, 0x3C, 0x77, 0x78, 0xF4, 0x6B, 0xB6);
DEFINE_GUID(IID_IStartExperienceStatics, 0xFB2E3E59, 0xB442, 0x4B5B, 0x91, 0x28, 0x23, 0x19, 0xBF, 0x8D, 0xE3, 0xB0);

namespace ABI::Windows::Internal::Shell::StartUI
{

struct StartExperience;

class StartExperienceAnimationRequestedEventArgs;
class ShellModeChangedEventArgs;

MIDL_INTERFACE("4c4d0c66-5bd5-fd8c-617f-3c7778f46bb6")
IStartExperience : IInspectable
{
    virtual HRESULT STDMETHODCALLTYPE ProcessBackgroundImage(IInspectable*, int, INT64*) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_WaitableExplorerProcessHandle(UINT64*) = 0;
    virtual HRESULT STDMETHODCALLTYPE PromoteAppsToTopOfFrequentList(
        ABI::Windows::Foundation::Collections::IVector<HSTRING>*) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_GlobalAnimationRequested(
        ABI::Windows::Foundation::ITypedEventHandler<
            StartExperience*, StartExperienceAnimationRequestedEventArgs*>*, EventRegistrationToken*) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_GlobalAnimationRequested(EventRegistrationToken) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_ShellModeChanged(
        ABI::Windows::Foundation::ITypedEventHandler<
            StartExperience*, ShellModeChangedEventArgs*>*, EventRegistrationToken*) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_ShellModeChanged(EventRegistrationToken) = 0;
};

MIDL_INTERFACE("fb2e3e59-b442-4b5b-9128-2319bf8de3b0")
IStartExperienceStatics : IInspectable
{
    virtual HRESULT STDMETHODCALLTYPE GetForCurrentView(IStartExperience**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetIfNoCurrentView(IStartExperience**) = 0;
};

} // ABI::Windows::Internal::Shell::StartUI

EXTERN_C HRESULT StartExperienceWrapper_Wrap(REFIID riid, void** ppv);

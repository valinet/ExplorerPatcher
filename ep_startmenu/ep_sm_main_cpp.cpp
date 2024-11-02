#include <Windows.h>
#include <ShlObj_core.h>
#include <windows.foundation.h>
#include <windows.applicationmodel.resources.core.h>

#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

namespace ABI::Windows::ApplicationModel::Resources::Core::Internal
{
    MIDL_INTERFACE("4a8eac58-b652-459d-8de1-239471e8b22b")
    IResourceManagerStaticInternal : IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE GetResourceManagerForSystemProfile(IResourceManager** result) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCurrentResourceManagerForSystemProfile(IResourceManager** result) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCurrentResourceManagerState(DWORD* result) = 0;
    };

    MIDL_INTERFACE("c408a1f1-3ede-41e9-9a38-c203678c2df7")
    ISystemResourceManagerExtensions : IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE GetDefaultResourceContextForCurrentThread(IResourceContext**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetMrtResourceManagerForResourceManager(IInspectable**) = 0;
    };

    MIDL_INTERFACE("8c25e859-1042-4da0-9232-bf2aa8ff3726")
    ISystemResourceManagerExtensions2 : IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE LoadPriFileForSystemUse(const WCHAR* path) = 0;
    };
}

using namespace Microsoft::WRL;

extern "C" HRESULT LoadOurShellCommonPri()
{
    using namespace ABI::Windows::Foundation;

    ComPtr<IInspectable> pInspectable;
    HRESULT hr = Windows::Foundation::GetActivationFactory(
        Wrappers::HStringReference(L"Windows.ApplicationModel.Resources.Core.ResourceManager").Get(),
        pInspectable.GetAddressOf()
    );
    if (FAILED(hr))
        return hr;

    ComPtr<ABI::Windows::ApplicationModel::Resources::Core::Internal::IResourceManagerStaticInternal> pResourceManagerStaticInternal;
    hr = pInspectable.As(&pResourceManagerStaticInternal);
    if (FAILED(hr))
        return hr;

    ComPtr<ABI::Windows::ApplicationModel::Resources::Core::IResourceManager> pResourceManager;
    hr = pResourceManagerStaticInternal->GetCurrentResourceManagerForSystemProfile(&pResourceManager);
    if (FAILED(hr))
        return hr;

    ComPtr<ABI::Windows::ApplicationModel::Resources::Core::Internal::ISystemResourceManagerExtensions2> pSystemResourceManagerExtensions2;
    pResourceManager.As(&pSystemResourceManagerExtensions2);

    WCHAR wszPath[MAX_PATH] = {};
    hr = SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES, nullptr, SHGFP_TYPE_CURRENT, wszPath);
    if (FAILED(hr))
        return hr;

    wcscat_s(wszPath, MAX_PATH, L"\\ExplorerPatcher\\Windows.UI.ShellCommon.pri");
    hr = pSystemResourceManagerExtensions2->LoadPriFileForSystemUse(wszPath);

    return hr;
}

extern "C" WCHAR g_szStartUIName[MAX_PATH];

extern "C" HRESULT GetActivationFactoryByPCWSTR_InStartUI(PCWSTR activatableClassId, REFIID riid, void** ppv)
{
    typedef HRESULT (WINAPI* DllGetActivationFactory_t)(HSTRING, IActivationFactory**);
    static DllGetActivationFactory_t pfnGetActivationFactory;
    if (!pfnGetActivationFactory)
    {
        HMODULE hModule = GetModuleHandleW(g_szStartUIName);
        if (hModule)
        {
            pfnGetActivationFactory = (DllGetActivationFactory_t)GetProcAddress(hModule, "DllGetActivationFactory");
        }
    }

    if (!pfnGetActivationFactory)
        return E_FAIL;

    ComPtr<IActivationFactory> activationFactory;
    HRESULT hr = pfnGetActivationFactory(Wrappers::HStringReference(activatableClassId).Get(), &activationFactory);
    if (FAILED(hr))
        return hr;

    return activationFactory.CopyTo(riid, ppv);
}

extern "C" HRESULT GetActivationFactoryByPCWSTR_InJumpViewUI(PCWSTR activatableClassId, REFIID riid, void** ppv)
{
    typedef HRESULT (WINAPI* DllGetActivationFactory_t)(HSTRING, IActivationFactory**);
    static DllGetActivationFactory_t pfnGetActivationFactory;
    if (!pfnGetActivationFactory)
    {
        HMODULE hModule = GetModuleHandleW(L"JumpViewUI_.dll");
        if (hModule)
        {
            pfnGetActivationFactory = (DllGetActivationFactory_t)GetProcAddress(hModule, "DllGetActivationFactory");
        }
    }

    if (!pfnGetActivationFactory)
        return E_FAIL;

    ComPtr<IActivationFactory> activationFactory;
    HRESULT hr = pfnGetActivationFactory(Wrappers::HStringReference(activatableClassId).Get(), &activationFactory);
    if (FAILED(hr))
        return hr;

    return activationFactory.CopyTo(riid, ppv);
}

#include <windows.h>
#include <windows.system.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.system.h>
#include <roapi.h>

static std::vector<winrt::guid> GlobalStartData_GetPlacesFromRegistry()
{
    std::vector<winrt::guid> places;

    DWORD dwSize;
    HRESULT hr = RegGetValueW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Start",
        L"VisiblePlaces",
        RRF_RT_REG_BINARY,
        nullptr,
        nullptr,
        &dwSize
    );
    if (FAILED(hr) || dwSize == 0)
        return places;

    places.resize(dwSize / sizeof(winrt::guid));
    hr = RegGetValueW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Start",
        L"VisiblePlaces",
        RRF_RT_REG_BINARY,
        nullptr,
        places.data(),
        &dwSize
    );
    if (FAILED(hr))
        places.clear();

    return places;
}

namespace ABI::WindowsInternal::Shell::CDSProperties
{
    interface IStartGlobalProperties;

    MIDL_INTERFACE("2c670963-f8a9-4bbb-9adf-683a3a89537e")
    IStartGlobalPropertiesFactory : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE Create(Windows::System::IUser* user, IStartGlobalProperties** result) = 0;
    };

    MIDL_INTERFACE("ee807266-a2db-4c9a-a1b4-970d33f99c91")
    IStartGlobalProperties : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_FullScreenMode(unsigned char*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_FullScreenMode(unsigned char) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_HideAppList(unsigned char*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_HideAppList(unsigned char) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_HideRecentList(unsigned char*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_HideRecentList(unsigned char) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_HideFrequentList(unsigned char*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_HideFrequentList(unsigned char) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_StartMenuRelativeHeightPixels(unsigned int*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_StartMenuRelativeHeightPixels(unsigned int) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_PlacesInitialized(unsigned char*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_PlacesInitialized(unsigned char) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_PlacesInitializedVersion(unsigned int*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_PlacesInitializedVersion(unsigned int) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetVisiblePlaces(Windows::Foundation::Collections::IVectorView<GUID>**) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetVisiblePlaces(Windows::Foundation::Collections::IVectorView<GUID>*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_StartViewRestoring(unsigned char*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_StartViewRestoring(unsigned char) = 0;
        virtual HRESULT STDMETHODCALLTYPE add_PropertiesChanged(
            /*Windows::Foundation::ITypedEventHandler<
                StartGlobalProperties*,
                StartGlobalPropertiesChangedArgs*
            >*,
            EventRegistrationToken**/
        ) = 0;
        virtual HRESULT STDMETHODCALLTYPE remove_PropertiesChanged(EventRegistrationToken) = 0;
    };
}

extern "C" BOOL NeedsRo_SyncSettingsFromRegToCDS()
{
    winrt::com_ptr<ABI::WindowsInternal::Shell::CDSProperties::IStartGlobalPropertiesFactory> global_properties_factory;
    winrt::param::hstring hstr = L"WindowsInternal.Shell.CDSProperties.StartGlobalProperties";
    HRESULT hr = RoGetActivationFactory(
        *(HSTRING*)&hstr,
        __uuidof(ABI::WindowsInternal::Shell::CDSProperties::IStartGlobalPropertiesFactory),
        global_properties_factory.put_void()
    );
    if (FAILED(hr))
    {
        return FALSE;
    }

    winrt::Windows::System::User user = winrt::Windows::System::User::FindAllAsync().get().GetAt(0);
    winrt::com_ptr<ABI::WindowsInternal::Shell::CDSProperties::IStartGlobalProperties> start_global_properties;
    hr = global_properties_factory->Create(user.as<ABI::Windows::System::IUser>().get(), start_global_properties.put());
    if (FAILED(hr))
    {
        return FALSE;
    }

    DWORD dwValue, dwSize;

    // ShowFrequentList
    dwValue = 0; // Default off
    dwSize = sizeof(DWORD);
    RegGetValueW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Start",
        L"ShowFrequentList",
        RRF_RT_REG_DWORD,
        nullptr,
        &dwValue,
        &dwSize
    );
    start_global_properties->put_HideFrequentList(!dwValue);

    // ShowRecentList
    dwValue = 1; // Default on
    dwSize = sizeof(DWORD);
    RegGetValueW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Start",
        L"ShowRecentList",
        RRF_RT_REG_DWORD,
        nullptr,
        &dwValue,
        &dwSize
    );
    start_global_properties->put_HideRecentList(!dwValue);

    // VisiblePlaces
    auto places_view = single_threaded_vector<winrt::guid>(GlobalStartData_GetPlacesFromRegistry()).GetView();
    start_global_properties->SetVisiblePlaces(places_view.as<ABI::Windows::Foundation::Collections::IVectorView<GUID>>().get());

    return TRUE;
}

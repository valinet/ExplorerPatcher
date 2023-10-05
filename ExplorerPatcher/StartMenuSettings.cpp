#include <windows.h>
#include <windows.system.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.system.h>
#include <wil/winrt.h>

extern "C" extern DWORD dwStartShowClassicMode;

static void EPWilLogCallback(wil::FailureInfo const &failure) noexcept
{
    wchar_t message[2048];
    HRESULT hr = GetFailureLogString(message, ARRAYSIZE(message), failure);
    if (SUCCEEDED(hr))
    {
        wprintf(L"%s", message); // message includes newline
    }
}

extern "C" void InitializeWilLogCallback()
{
    SetResultLoggingCallback(EPWilLogCallback);
}

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

namespace ABI::WindowsInternal::Shell::UnifiedTile
{
    MIDL_INTERFACE("d3653510-4fff-4bfa-905b-ea038b142fa5")
    IUnifiedTileIdentifier : public IInspectable
    {
    };

    MIDL_INTERFACE("0e7735be-a965-44a6-a75f-54b8bcd67bec")
    IWin32UnifiedTileIdentifierFactory : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE Create(HSTRING, IUnifiedTileIdentifier**) = 0;
    };
}

namespace ABI::WindowsInternal::Shell::UnifiedTile::Private
{
    MIDL_INTERFACE("0083831c-82d6-4e8f-bcc2-a8ac2691be49")
    IUnifiedTileUserPinHelperStatics : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE CreateUserPinnedShortcutTile(IUnifiedTileIdentifier*) = 0;
    };
}

namespace ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
    enum class CollectionAttributes {};
    enum class PackageStatusChangeType {};
    enum class StartCollectionCustomizationRestrictionType {};
    enum class TilePinSize {};

    namespace DataStoreCache::CuratedTileCollectionTransformer
    {
        class CuratedTile;
    }

    MIDL_INTERFACE("ffffffff-ffff-ffff-ffff-ffffffffffff")
    ICuratedTileGroup : public IInspectable
    {
    };

    MIDL_INTERFACE("ffffffff-ffff-ffff-ffff-ffffffffffff")
    ICuratedTile : public IInspectable
    {
    };

    MIDL_INTERFACE("51a07090-3a1f-49ef-9932-a971b8154790")
    ICuratedTileCollection : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_CollectionName(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Attributes(CollectionAttributes*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_Attributes(CollectionAttributes) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Version(unsigned int*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_Version(unsigned int) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetGroups(Windows::Foundation::Collections::IMapView<GUID, ICuratedTileGroup*>**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetTiles(Windows::Foundation::Collections::IMapView<GUID, ICuratedTile*>**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetAllTilesInCollection(Windows::Foundation::Collections::IMapView<GUID, ICuratedTile*>**) = 0;
        virtual HRESULT STDMETHODCALLTYPE DoesCollectionContainTile(IUnifiedTileIdentifier*, ICuratedTile**, unsigned char*) = 0;
        virtual HRESULT STDMETHODCALLTYPE FindTileAndParentGroup(IUnifiedTileIdentifier*, ICuratedTile**, ICuratedTileGroup**, unsigned char*) = 0;
        virtual HRESULT STDMETHODCALLTYPE MoveExistingGroupToNewParent(ICuratedTileGroup*, ICuratedTileGroup*) = 0;
        virtual HRESULT STDMETHODCALLTYPE CreateNewGroup(ICuratedTileGroup**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetGroup(GUID, ICuratedTileGroup**) = 0;
        virtual HRESULT STDMETHODCALLTYPE DeleteGroup(GUID) = 0;
        virtual HRESULT STDMETHODCALLTYPE RemoveGroup(GUID) = 0;
        virtual HRESULT STDMETHODCALLTYPE MoveExistingTileToNewParent(ICuratedTile*, ICuratedTileGroup*) = 0;
        virtual HRESULT STDMETHODCALLTYPE AddTile(IUnifiedTileIdentifier*, ICuratedTile**) = 0;
        virtual HRESULT STDMETHODCALLTYPE AddTileWithId(IUnifiedTileIdentifier*, GUID, ICuratedTile**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetTile(GUID, ICuratedTile**) = 0;
        virtual HRESULT STDMETHODCALLTYPE DeleteTile(GUID) = 0;
        virtual HRESULT STDMETHODCALLTYPE RemoveTile(GUID) = 0;
        virtual HRESULT STDMETHODCALLTYPE Commit() = 0;
        virtual HRESULT STDMETHODCALLTYPE CommitAsync(Windows::Foundation::IAsyncAction**) = 0;
        virtual HRESULT STDMETHODCALLTYPE CommitAsyncWithTimerBypass(Windows::Foundation::IAsyncAction**) = 0;
        virtual HRESULT STDMETHODCALLTYPE ResetToDefault() = 0;
        virtual HRESULT STDMETHODCALLTYPE ResetToDefaultAsync(Windows::Foundation::IAsyncAction**) = 0;
        virtual HRESULT STDMETHODCALLTYPE CheckForUpdate() = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCustomProperty(const HSTRING, HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE HasCustomProperty(const HSTRING, unsigned char*) = 0;
        virtual HRESULT STDMETHODCALLTYPE RemoveCustomProperty(const HSTRING) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetCustomProperty(const HSTRING, HSTRING) = 0;
        virtual HRESULT STDMETHODCALLTYPE EnsureTileRegistration() = 0;
        virtual HRESULT STDMETHODCALLTYPE ResurrectTile(std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedTile>, const GUID&) = 0;
        virtual HRESULT STDMETHODCALLTYPE OnTileAddedWithinCollection(IUnifiedTileIdentifier*) = 0;
        virtual HRESULT STDMETHODCALLTYPE OnTileRemovedWithinCollection(IUnifiedTileIdentifier*) = 0;
    };

    MIDL_INTERFACE("adbf8965-6056-4126-ab26-6660af4661ce")
    IStartTileCollection : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE PinToStart(IUnifiedTileIdentifier*, TilePinSize) = 0;
        virtual HRESULT STDMETHODCALLTYPE PinToStartAtLocation(IUnifiedTileIdentifier*, ICuratedTileGroup*, Windows::Foundation::Point, Windows::Foundation::Size) = 0;
        virtual HRESULT STDMETHODCALLTYPE UnpinFromStart(IUnifiedTileIdentifier*) = 0;
        virtual HRESULT STDMETHODCALLTYPE ReplaceTinyOrMediumTile(IUnifiedTileIdentifier*, IUnifiedTileIdentifier*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_LastGroupId(GUID*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_LastGroupId(GUID) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_CustomizationRestriction(StartCollectionCustomizationRestrictionType*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_CustomizationRestriction(StartCollectionCustomizationRestrictionType) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_GroupCellWidth(unsigned int*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_GroupCellWidth(unsigned int) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_PreferredColumnCount(unsigned int*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_PreferredColumnCount(unsigned int) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_CurrentColumnCount(unsigned int*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_CurrentColumnCount(unsigned int) = 0;
    };

    MIDL_INTERFACE("ffffffff-ffff-ffff-ffff-ffffffffffff")
    ICuratedTileCollectionOptions : public IInspectable
    {
    };

    MIDL_INTERFACE("ffffffff-ffff-ffff-ffff-ffffffffffff")
    ICuratedTileCollectionManager : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE NotifyPackageStatusChanged(HSTRING, PackageStatusChangeType) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCollection(HSTRING, ICuratedTileCollection**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCollectionWithOptions(HSTRING, ICuratedTileCollectionOptions*, ICuratedTileCollection**) = 0;
        virtual HRESULT STDMETHODCALLTYPE DeleteCollection(HSTRING) = 0;
        virtual HRESULT STDMETHODCALLTYPE CollectionExists(HSTRING, unsigned char*) = 0;
        virtual HRESULT STDMETHODCALLTYPE InitializeCollection(HSTRING) = 0;
    };

    MIDL_INTERFACE("15f254ac-49b3-4e6e-9c62-806ffaf554f9")
    ICuratedTileCollectionManagerStatics : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE CreateWithUser(Windows::System::IUser*, ICuratedTileCollectionManager**) = 0;
    };
}

struct CCacheShortcut
{
public:
    const wchar_t* GetAppID(const void* a2) const
    {
        DWORD dwOffset = *((DWORD*)this + 11); // Same offset in Windows 10 and 11
        return dwOffset != -1 ? (wchar_t*)((char*)a2 + dwOffset) : nullptr;
    }
};

extern "C"
{
HRESULT(*AppResolver_CAppResolverCacheBuilder__AddUserPinnedShortcutToStartFunc)(const CCacheShortcut* a2, const void* a3);
HRESULT AppResolver_CAppResolverCacheBuilder__AddUserPinnedShortcutToStart(const CCacheShortcut* a2, const void* a3)
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;
    using namespace ABI::WindowsInternal::Shell::UnifiedTile::Private;
    using namespace ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections;

    if (!dwStartShowClassicMode)
    {
        return AppResolver_CAppResolverCacheBuilder__AddUserPinnedShortcutToStartFunc(a2, a3);
    }

    Microsoft::WRL::ComPtr<IWin32UnifiedTileIdentifierFactory> pTileIdentifierFactory;
    RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
        Microsoft::WRL::Wrappers::HStringReference(L"WindowsInternal.Shell.UnifiedTile.UnifiedTileIdentifier").Get(),
        pTileIdentifierFactory.GetAddressOf()
    ));

    Microsoft::WRL::ComPtr<IUnifiedTileIdentifier> pTileIdentifier;
    const wchar_t* pwszAppId = a2->GetAppID(a3);
    RETURN_IF_FAILED(pTileIdentifierFactory->Create(
        Microsoft::WRL::Wrappers::HStringReference(pwszAppId).Get(),
        pTileIdentifier.GetAddressOf()
    ));

    Microsoft::WRL::ComPtr<IUnifiedTileUserPinHelperStatics> pTileUserPinHelper;
    RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
        Microsoft::WRL::Wrappers::HStringReference(L"WindowsInternal.Shell.UnifiedTile.Private.UnifiedTileUserPinHelper").Get(),
        pTileUserPinHelper.GetAddressOf()
    ));

    RETURN_IF_FAILED(pTileUserPinHelper->CreateUserPinnedShortcutTile(
        pTileIdentifier.Get()
    ));

    // At this point, on Windows 11 the Windows 10 code doesn't exist anymore, so we'll add them here
    Microsoft::WRL::ComPtr<ICuratedTileCollectionManager> pTileCollectionManager;
    RETURN_IF_FAILED(RoActivateInstance(
        Microsoft::WRL::Wrappers::HStringReference(L"WindowsInternal.Shell.UnifiedTile.CuratedTileCollections.CuratedTileCollectionManager").Get(),
        (IInspectable**)pTileCollectionManager.GetAddressOf()
    ));

    Microsoft::WRL::ComPtr<ICuratedTileCollection> pTileCollection;
    RETURN_IF_FAILED(pTileCollectionManager->GetCollection(
        Microsoft::WRL::Wrappers::HStringReference(L"Start.TileGrid").Get(),
        pTileCollection.GetAddressOf()
    ));

    Microsoft::WRL::ComPtr<IStartTileCollection> pStartTileCollection;
    RETURN_IF_FAILED(pTileCollection.As(&pStartTileCollection));

    RETURN_IF_FAILED(pStartTileCollection->PinToStart(
        pTileIdentifier.Get(),
        static_cast<TilePinSize>(0)
    ));

    RETURN_IF_FAILED(pTileCollection->Commit());

    return S_OK;
}
}

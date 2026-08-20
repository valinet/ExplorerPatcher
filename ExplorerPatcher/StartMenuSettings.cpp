#include "utility.h"
#include "hooking.h"

#include <windows.h>
#include <windows.system.h>
#include <windows.ui.shell.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.system.h>
#include <wil/winrt.h>

using namespace Microsoft::WRL;

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
    LSTATUS lRes = RegGetValueW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Start",
        L"VisiblePlaces",
        RRF_RT_REG_BINARY,
        nullptr,
        nullptr,
        &dwSize
    );
    if (lRes != ERROR_SUCCESS || dwSize == 0)
        return places;

    places.resize(dwSize / sizeof(winrt::guid));
    lRes = RegGetValueW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Start",
        L"VisiblePlaces",
        RRF_RT_REG_BINARY,
        nullptr,
        places.data(),
        &dwSize
    );
    if (lRes != ERROR_SUCCESS)
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
        virtual HRESULT STDMETHODCALLTYPE get_FullScreenMode(BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_FullScreenMode(BOOLEAN) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_HideAppList(BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_HideAppList(BOOLEAN) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_HideRecentList(BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_HideRecentList(BOOLEAN) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_HideFrequentList(BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_HideFrequentList(BOOLEAN) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_StartMenuRelativeHeightPixels(UINT*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_StartMenuRelativeHeightPixels(UINT) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_PlacesInitialized(BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_PlacesInitialized(BOOLEAN) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_PlacesInitializedVersion(UINT*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_PlacesInitializedVersion(UINT) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetVisiblePlaces(Windows::Foundation::Collections::IVectorView<GUID>**) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetVisiblePlaces(Windows::Foundation::Collections::IVectorView<GUID>*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_StartViewRestoring(BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_StartViewRestoring(BOOLEAN) = 0;
        virtual HRESULT STDMETHODCALLTYPE add_PropertiesChanged(
            /*Windows::Foundation::ITypedEventHandler<
                StartGlobalProperties*,
                StartGlobalPropertiesChangedArgs*
            >*,
            EventRegistrationToken**/
        ) = 0;
        virtual HRESULT STDMETHODCALLTYPE remove_PropertiesChanged(EventRegistrationToken) = 0;
    };

    enum ExtendedReconciliationRequirements
    {
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

	// HideAppList
	start_global_properties->put_HideAppList(FALSE);

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

namespace ABI::WindowsUdk::ApplicationModel::AppExtensions
{
    enum AppExtensionOptions {};

    MIDL_INTERFACE("836da1ed-5be8-5365-8452-6af327aa427b")
    IExtensionFactoryStatics : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE IsExtensionAvailable(HSTRING, HSTRING, bool*) = 0;
        virtual HRESULT STDMETHODCALLTYPE IsExtensionAvailableWithOptions(HSTRING, HSTRING, AppExtensionOptions, bool*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetInstance(HSTRING, HSTRING, IInspectable**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetInstanceWithOptions(HSTRING, HSTRING, AppExtensionOptions, IInspectable**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetFactory(HSTRING, HSTRING, IInspectable**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetFactoryWithOptions(HSTRING, HSTRING, AppExtensionOptions, IInspectable**) = 0;
    };
}

class DummyExtensionFactory : ABI::WindowsUdk::ApplicationModel::AppExtensions::IExtensionFactoryStatics
{
public:
    HRESULT QueryInterface(REFIID riid, void** ppvObject) override { return E_NOINTERFACE; }
    ULONG AddRef() override { return 1; }
    ULONG Release() override { return 1; }
    HRESULT GetIids(ULONG* iidCount, IID** iids) override { return E_NOTIMPL; }
    HRESULT GetRuntimeClassName(HSTRING* className) override { return E_NOTIMPL; }
    HRESULT GetTrustLevel(TrustLevel* trustLevel) override { return E_NOTIMPL; }

    // Keep the value of result as zero (set by the caller) and return S_OK to make the Windows 10 code run
    HRESULT IsExtensionAvailable(HSTRING, HSTRING, bool*) override { return S_OK; }
    HRESULT IsExtensionAvailableWithOptions(HSTRING, HSTRING, ABI::WindowsUdk::ApplicationModel::AppExtensions::AppExtensionOptions, bool*) override { return S_OK; }
    HRESULT GetInstance(HSTRING, HSTRING, IInspectable**) override { return S_OK; }
    HRESULT GetInstanceWithOptions(HSTRING, HSTRING, ABI::WindowsUdk::ApplicationModel::AppExtensions::AppExtensionOptions, IInspectable**) override { return S_OK; }
    HRESULT GetFactory(HSTRING, HSTRING, IInspectable**) override { return S_OK; }
    HRESULT GetFactoryWithOptions(HSTRING, HSTRING, ABI::WindowsUdk::ApplicationModel::AppExtensions::AppExtensionOptions, IInspectable**) override { return S_OK; }
};

static const DummyExtensionFactory instanceof_WindowsUdk_ApplicationModel_AppExtensions_IExtensionFactoryStatics;

extern "C" HRESULT AppResolver_StartTileData_RoGetActivationFactory(HSTRING activatableClassId, REFIID iid, void** factory)
{
    if (dwStartShowClassicMode && IsEqualGUID(iid, __uuidof(ABI::WindowsUdk::ApplicationModel::AppExtensions::IExtensionFactoryStatics)))
    {
        *factory = const_cast<DummyExtensionFactory*>(&instanceof_WindowsUdk_ApplicationModel_AppExtensions_IExtensionFactoryStatics);
        return S_OK;
    }
    return RoGetActivationFactory(activatableClassId, iid, factory);
}

namespace ABI::Windows::Internal::ApplicationModel
{
    namespace WindowManagement
    {
        struct WindowId
        {
            UINT Value;
        };
    }

    MIDL_INTERFACE("1223aea2-547b-4d03-9e6c-388a4307a07e")
    IPinnableSurface : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE CanPinTile(HSTRING, ABI::Windows::UI::StartScreen::ISecondaryTile*, BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE RequestPinTile(HSTRING, HSTRING, Windows::UI::StartScreen::ISecondaryTile*, ABI::Windows::Internal::ApplicationModel::WindowManagement::WindowId, BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE PinTile(HSTRING, ABI::Windows::UI::StartScreen::ISecondaryTile*) = 0;
        virtual HRESULT STDMETHODCALLTYPE IsTilePinned(HSTRING, HSTRING, BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE CanUnpinTile(HSTRING, HSTRING, BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE UnpinTile(HSTRING, HSTRING) = 0;
    };

    MIDL_INTERFACE("f27684e4-e634-4807-be9a-4838381fcbfc")
    IPinnableSurfaceFactory : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE GetCurrent(IPinnableSurface**);
    };
}

namespace ABI::Windows::Internal::UI::StartScreen
{
    enum SecondaryTileCreationBehaviors
    {
        SecondaryTileCreationBehaviors_None = 0x0,
        SecondaryTileCreationBehaviors_SuppressPinToStart = 0x1,
        SecondaryTileCreationBehaviors_SuppressBackgroundPolicyCheck = 0x2,
        SecondaryTileCreationBehaviors_SuppressPinRequest = 0x4,
    };

    DEFINE_ENUM_FLAG_OPERATORS(SecondaryTileCreationBehaviors);

    MIDL_INTERFACE("2d7f0d3b-ec36-463b-9f69-d7238d77c122")
    ISecondaryTilePrivate : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE PopulateIdentity(HSTRING) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetTileCreationBehaviors(SecondaryTileCreationBehaviors) = 0;
        virtual HRESULT STDMETHODCALLTYPE RefreshPropertiesFromPrimaryTile() = 0;
        virtual HRESULT STDMETHODCALLTYPE GetUniqueId(GUID*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetDefaultTileSize(ABI::Windows::UI::StartScreen::TileSize*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetAppUserModelId(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE ValidatePropertiesForCreate() = 0;
        virtual HRESULT STDMETHODCALLTYPE CreateInTileStore() = 0;
    };
}

namespace ABI::WindowsInternal::Shell::UnifiedTile
{
    enum UnifiedTileIdentifierKind
    {
        UnifiedTileIdentifierKind_Unknown = 0x0,
        UnifiedTileIdentifierKind_Packaged = 0x1,
        UnifiedTileIdentifierKind_Win32 = 0x2,
        UnifiedTileIdentifierKind_TargetedContent = 0x3,
    };

    MIDL_INTERFACE("d3653510-4fff-4bfa-905b-ea038b142fa5")
    IUnifiedTileIdentifier : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_Kind(UnifiedTileIdentifierKind*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_SerializedIdentifier(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_NotificationId(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_TelemetryId(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE IsEqual(IUnifiedTileIdentifier*, BOOLEAN*) = 0;
    };

    MIDL_INTERFACE("ec3e7864-aaab-4367-9c63-94d289545500")
    IPackagedUnifiedTileIdentifierFactory : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE Create(HSTRING, IUnifiedTileIdentifier**) = 0;
        virtual HRESULT STDMETHODCALLTYPE CreateWithTileId(HSTRING, HSTRING, IUnifiedTileIdentifier**) = 0;
    };

    MIDL_INTERFACE("87a52467-266a-4b20-a2c8-e316bfbaf64a")
    IUnifiedTileIdentifierStatics : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE DeserializeIdentifier(HSTRING, IUnifiedTileIdentifier**) = 0;
    };

    MIDL_INTERFACE("0e7735be-a965-44a6-a75f-54b8bcd67bec")
    IWin32UnifiedTileIdentifierFactory : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE Create(HSTRING, IUnifiedTileIdentifier**) = 0;
    };

    interface IVisualTileInfo;
    interface ILargeFormatVisualTileInfo;
    interface IEnterpriseDataProtectionTileInfo;
    interface IAppUsageInfo;
    interface IAppLifecycleInfo;

    MIDL_INTERFACE("861778d6-ac6c-456f-bd3c-32ab601245e1")
    IWin32ShortcutInfo : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_IsTargetFolder(BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_TargetPath(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_ShortcutArguments(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_IsUserPinnedShortcut(BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_IsManifested(BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_PreventPinning(BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_SuiteName(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_SuiteSortName(HSTRING*) = 0;
    };

    interface IPackagedAppTileInfo;
    interface ISuggestionTileInfo;
    interface IMixedRealityTileInfo;
    interface ITileActivationContext;
    interface IVerbSource;

    MIDL_INTERFACE("65b4e03e-a32e-40cf-8bab-b2d9c5287307")
    IUnifiedTile : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_Id(IUnifiedTileIdentifier**) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_VisualTileInfo(IVisualTileInfo**) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_LargeFormatVisualTileInfo(ILargeFormatVisualTileInfo**) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_EnterpriseDataProtectionTileInfo(IEnterpriseDataProtectionTileInfo**) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_AppUsageInfo(IAppUsageInfo**) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_AppLifecycleInfo(IAppLifecycleInfo**) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Win32ShortcutInfo(IWin32ShortcutInfo**) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_PackagedAppTileInfo(IPackagedAppTileInfo**) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_SuggestionTileInfo(ISuggestionTileInfo**) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_MixedRealityTileInfo(IMixedRealityTileInfo**) = 0;
        virtual HRESULT STDMETHODCALLTYPE ActivateAsync(ABI::Windows::Foundation::IAsyncAction**) = 0;
        virtual HRESULT STDMETHODCALLTYPE ActivateWithContextAsync(ITileActivationContext*, ABI::Windows::Foundation::IAsyncAction**) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Verbs(IVerbSource**) = 0;
        virtual HRESULT STDMETHODCALLTYPE add_TileChanged(void*, EventRegistrationToken*) = 0;
        virtual HRESULT STDMETHODCALLTYPE remove_TileChanged(EventRegistrationToken) = 0;
    };

    namespace CuratedTileCollections
    {
        interface ICuratedTileCollection;
        interface ICuratedTileGroup;
    }

    MIDL_INTERFACE("abaabd17-2d4e-43f0-b43c-60e699a32341")
    IUnifiedTileCollection : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_CollectionId(HSTRING*);
        virtual HRESULT STDMETHODCALLTYPE GetAllTilesRecursive(void**);
        virtual HRESULT STDMETHODCALLTYPE get_CuratedCollectionInfo(CuratedTileCollections::ICuratedTileCollection**);
    };

    interface ICuratedCollectionBatch;

    MIDL_INTERFACE("6fe03efc-a8af-4faf-9c23-bc0cb5bf29d3")
    ITileCollectionContainer : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_Id(GUID*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetContainers(void**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetTiles(void**) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_DisplayName(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_SortName(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_ParentContainer(ITileCollectionContainer**) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_CuratedGroupInfo(CuratedTileCollections::ICuratedTileGroup**) = 0;
        virtual HRESULT STDMETHODCALLTYPE CreateCuratedCollectionBatch(ICuratedCollectionBatch**) = 0;
        virtual HRESULT STDMETHODCALLTYPE add_ContentsChanged(void*, EventRegistrationToken*) = 0;
        virtual HRESULT STDMETHODCALLTYPE remove_ContentsChanged(EventRegistrationToken) = 0;
        virtual HRESULT STDMETHODCALLTYPE add_PropertyChanged(void*, EventRegistrationToken*) = 0;
        virtual HRESULT STDMETHODCALLTYPE remove_PropertyChanged(EventRegistrationToken) = 0;
    };

    enum CollectionProvider
    {
        CollectionProvider_AllTilesCollection = 0,
        CollectionProvider_AppsListCollection = 1,
        CollectionProvider_CuratedTileCollection = 2,
        CollectionProvider_FlatAppsListCollection = 3,
        CollectionProvider_FlatAppsListWithSecondaryAndUserPinnedTilesCollection = 4,
    };

    enum CollectionOptions
    {
        CollectionOptions_None = 0x0,
        CollectionOptions_IncludeTombstones = 0x1,
        CollectionOptions_UpdatedItemsOnly = 0x2,
    };

    // typedef ABI::Windows::Internal::Storage::Cloud::CollectionOptions CollectionOptions;

    MIDL_INTERFACE("1048dc30-f4f7-4ff4-970e-5058ca17cc26")
    IUnifiedTileManager : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE GetTransformer(GUID, IInspectable**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCollection(CollectionProvider, HSTRING, IUnifiedTileCollection**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCollectionWithOptions(CollectionProvider, HSTRING, CollectionOptions, IUnifiedTileCollection**) = 0;
        virtual HRESULT STDMETHODCALLTYPE FindTile(IUnifiedTileIdentifier*, IUnifiedTile**) = 0;
        virtual HRESULT STDMETHODCALLTYPE WaitForDataStoreReconciliationCompleteAsync(ABI::Windows::Foundation::IAsyncAction**) = 0;
        virtual HRESULT STDMETHODCALLTYPE WaitForSecondaryDataReconciliationCompleteAsync(ABI::Windows::Foundation::IAsyncAction**) = 0;
    };

    interface ICollectionTile;
    interface IContentsChangedEventArgs;

    enum TileVerbFlags
    {
        TileVerbFlags_None = 0x0,
        TileVerbFlags_CanExecute = 0x1,
        TileVerbFlags_IsMetadata = 0x2,
        TileVerbFlags_IsGroup = 0x4,
        TileVerbFlags_IsSeparator = 0x8,
        TileVerbFlags_IsDefault = 0x10,
    };

    DEFINE_ENUM_FLAG_OPERATORS(TileVerbFlags);

    MIDL_INTERFACE("e98fc955-cda1-4ae8-ae08-292531bc6bb2")
    IVerbExecutionArgs : IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_Position(void**) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_Position(void*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_KeyModifiers(ABI::Windows::System::VirtualKeyModifiers*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_KeyModifiers(ABI::Windows::System::VirtualKeyModifiers) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetCallerWindow(ABI::Windows::UI::Core::ICoreWindow*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_CallerWindowId(unsigned int*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_CallerWindowId(unsigned int) = 0;
    };

    MIDL_INTERFACE("f9ad7985-244a-4e61-8ba2-55a3f5e1c665")
    ITileVerb : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_VerbProviderId(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_GroupPath(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_CanonicalName(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_DisplayName(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Glyph(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_GlyphFontFamily(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_AccessKey(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_ShortcutText(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Flags(TileVerbFlags*) = 0;
        virtual HRESULT STDMETHODCALLTYPE Execute(IVerbExecutionArgs*) = 0;
        virtual HRESULT STDMETHODCALLTYPE ExecuteAsync(IVerbExecutionArgs*, ABI::Windows::Foundation::IAsyncOperation<bool>**) = 0;
    };

    enum VerbEnumerationOptions
    {
        VerbEnumerationOptions_None = 0x0,
        VerbEnumerationOptions_ExcludeNonExecutable = 0x1,
        VerbEnumerationOptions_ExcludeResources = 0x2,
        VerbEnumerationOptions_IncludeExtendedVerbs = 0x4,
    };

    MIDL_INTERFACE("a9d9c0b6-c84b-4010-8202-7c23b17dc148")
    IVerbEnumerationArgs : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_Options(VerbEnumerationOptions*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_Options(VerbEnumerationOptions) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_VerbProviderId(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_VerbProviderId(HSTRING) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_GroupPathPrefix(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_GroupPathPrefix(HSTRING) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_VerbCanonicalName(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_VerbCanonicalName(HSTRING) = 0;
    };
}

namespace ABI::WindowsInternal::Shell::UnifiedTile::Private
{
    MIDL_INTERFACE("0083831c-82d6-4e8f-bcc2-a8ac2691be49")
    IUnifiedTileUserPinHelperStatics : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE CreateUserPinnedShortcutTile(IUnifiedTileIdentifier*) = 0;
    };

    MIDL_INTERFACE("7813d04d-61d5-40e7-8d6d-781c5603a891")
    ITileContainerPrivate : public IInspectable
    {
        virtual void* STDMETHODCALLTYPE GetGroups(void* retstr) = 0;
        virtual void* STDMETHODCALLTYPE GetTiles(void* retstr) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetContainer(const GUID*, ITileCollectionContainer**) = 0;
        virtual HRESULT STDMETHODCALLTYPE AddContainer(ITileCollectionContainer*) = 0;
        virtual HRESULT STDMETHODCALLTYPE RemoveContainer(const GUID*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetTile(const GUID*, ICollectionTile**) = 0;
        virtual HRESULT STDMETHODCALLTYPE AddTile(ICollectionTile*) = 0;
        virtual HRESULT STDMETHODCALLTYPE RemoveTile(const GUID*) = 0;
        virtual bool STDMETHODCALLTYPE TryFindTileByUnifiedTileIdRecursive(IUnifiedTileIdentifier*, ICollectionTile**, ITileCollectionContainer**) = 0;
        virtual HRESULT STDMETHODCALLTYPE InvokeContentsChangedEventSource(IContentsChangedEventArgs*) = 0;
        virtual HRESULT STDMETHODCALLTYPE InvokePropertiesChangedEventSource(IInspectable*) = 0;
    };

    enum UnifiedTileKind
    {
    };

    MIDL_INTERFACE("de10b7d8-bebd-4599-925d-759462d1c1b1")
    IUnifiedTilePrivate : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_IsVisibleInAppList(BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Kind(UnifiedTileKind*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_ExtendedReconciliationRequirements(ABI::WindowsInternal::Shell::CDSProperties::ExtendedReconciliationRequirements*) = 0;
    };

    MIDL_INTERFACE("3b8c9be7-fc8c-42e2-a6b5-7005aa719c35")
    IVerbEnumerationArgsPrivate : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE IsMatchingVerbProviderId(HSTRING, BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE IsMatchingGroupPath(HSTRING, BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE IsMatchingVerbCanonicalName(HSTRING, BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_User(ABI::Windows::System::IUser**) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_User(ABI::Windows::System::IUser*) = 0;
    };
}

namespace ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections
{
    enum CollectionAttributes {};
    enum PackageStatusChangeType {};
    enum StartCollectionCustomizationRestrictionType {};

    enum TilePinSize
    {
        TilePinSize_Tile2x2 = 0,
        TilePinSize_Tile4x2 = 1,
    };

    MIDL_INTERFACE("354cba6d-19ab-490c-97b6-8d4d9862e052")
    ICuratedTileGroup : public IInspectable
    {
    };

    MIDL_INTERFACE("bb4b31ed-0705-432e-bf3d-24bf54bee10d")
    ICuratedTile : public IInspectable
    {
    };

    MIDL_INTERFACE("51a07090-3a1f-49ef-9932-a971b8154790")
    ICuratedTileCollection : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_CollectionName(HSTRING*) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Attributes(CollectionAttributes*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_Attributes(CollectionAttributes) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Version(UINT*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_Version(UINT) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetGroups(Windows::Foundation::Collections::IMapView<GUID, ICuratedTileGroup*>**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetTiles(Windows::Foundation::Collections::IMapView<GUID, ICuratedTile*>**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetAllTilesInCollection(Windows::Foundation::Collections::IMapView<GUID, ICuratedTile*>**) = 0;
        virtual HRESULT STDMETHODCALLTYPE DoesCollectionContainTile(IUnifiedTileIdentifier*, ICuratedTile**, BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE FindTileAndParentGroup(IUnifiedTileIdentifier*, ICuratedTile**, ICuratedTileGroup**, BOOLEAN*) = 0;
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
        virtual HRESULT STDMETHODCALLTYPE HasCustomProperty(const HSTRING, BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE RemoveCustomProperty(const HSTRING) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetCustomProperty(const HSTRING, HSTRING) = 0;
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
        virtual HRESULT STDMETHODCALLTYPE get_GroupCellWidth(UINT*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_GroupCellWidth(UINT) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_PreferredColumnCount(UINT*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_PreferredColumnCount(UINT) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_CurrentColumnCount(UINT*) = 0;
        virtual HRESULT STDMETHODCALLTYPE put_CurrentColumnCount(UINT) = 0;
    };

    MIDL_INTERFACE("a680369c-0862-41a0-b7cd-bb35e3c497eb")
    ICuratedTileCollectionOptions : public IInspectable
    {
    };

    MIDL_INTERFACE("899ee71b-5c01-438f-b12e-61d49f6b4083")
    ICuratedTileCollectionManager : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE NotifyPackageStatusChanged(HSTRING, PackageStatusChangeType) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCollection(HSTRING, ICuratedTileCollection**) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCollectionWithOptions(HSTRING, ICuratedTileCollectionOptions*, ICuratedTileCollection**) = 0;
        virtual HRESULT STDMETHODCALLTYPE DeleteCollection(HSTRING) = 0;
        virtual HRESULT STDMETHODCALLTYPE CollectionExists(HSTRING, BOOLEAN*) = 0;
        virtual HRESULT STDMETHODCALLTYPE InitializeCollection(HSTRING) = 0;
    };

    MIDL_INTERFACE("15f254ac-49b3-4e6e-9c62-806ffaf554f9")
    ICuratedTileCollectionManagerStatics : public IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE CreateWithUser(Windows::System::IUser*, ICuratedTileCollectionManager**) = 0;
    };
}

#if 0
HRESULT StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__GetUnifiedIdentifierForAumid(
    void* _this,
    const WCHAR* pszAumid,
    const WCHAR* pszTileId,
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier** out)
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;

    ComPtr<IUnifiedTileIdentifierStatics> pUnifiedTileIdentifierStatics;
    RETURN_IF_FAILED(RoGetActivationFactory(
        Wrappers::HStringReference(L"WindowsInternal.Shell.UnifiedTile.UnifiedTileIdentifier").Get(),
        IID_PPV_ARGS(&pUnifiedTileIdentifierStatics)
    ));

    ComPtr<IPackagedUnifiedTileIdentifierFactory> pPackagedUnifiedTileIdentifierFactory;
    RETURN_IF_FAILED(pUnifiedTileIdentifierStatics.As(&pPackagedUnifiedTileIdentifierFactory));

    ComPtr<IUnifiedTileIdentifier> pUnifiedTileIdentifier;
    RETURN_IF_FAILED(pPackagedUnifiedTileIdentifierFactory->CreateWithTileId(
        Wrappers::HStringReference(pszAumid).Get(),
        Wrappers::HStringReference(pszTileId).Get(),
        &pUnifiedTileIdentifier
    ));

    *out = pUnifiedTileIdentifier.Detach();
    return S_OK;
}

HRESULT (*StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__PinTileFunc)(
    void* _this,
    HSTRING hstrAumid,
    ABI::Windows::UI::StartScreen::ISecondaryTile* tile);

HRESULT StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__PinTile(
    void* _this,
    HSTRING hstrAumid,
    ABI::Windows::UI::StartScreen::ISecondaryTile* tile)
{
    using namespace ABI::Windows::UI::StartScreen;
    using namespace ABI::Windows::Internal::UI::StartScreen;
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;
    using namespace ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections;

    if (!dwStartShowClassicMode)
        return StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__PinTileFunc(_this, hstrAumid, tile);

    /*if (RtlIsMultiUsersInSessionSku())
        return S_OK;*/

    Wrappers::HString hstrTileId;
    if (tile)
    {
        RETURN_IF_FAILED(tile->get_TileId(hstrTileId.ReleaseAndGetAddressOf()));
    }

    ComPtr<IUnifiedTileIdentifier> pUnifiedTileIdentifier;
    RETURN_IF_FAILED(StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__GetUnifiedIdentifierForAumid(
        _this,
        WindowsGetStringRawBuffer(hstrAumid, nullptr),
        hstrTileId.GetRawBuffer(nullptr),
        &pUnifiedTileIdentifier
    ));

    // Windows 10 start menu-specific code begins here
    ComPtr<ICuratedTileCollectionManager> pTileCollectionManager;
    RETURN_IF_FAILED(RoActivateInstance(
        Wrappers::HStringReference(L"WindowsInternal.Shell.UnifiedTile.CuratedTileCollections.CuratedTileCollectionManager").Get(),
        &pTileCollectionManager
    ));

    ComPtr<ICuratedTileCollection> pTileCollection;
    RETURN_IF_FAILED(pTileCollectionManager->GetCollection(
        Wrappers::HStringReference(L"Start.TileGrid").Get(),
        &pTileCollection
    ));

    ComPtr<IStartTileCollection> pStartTileCollection;
    RETURN_IF_FAILED(pTileCollection.As(&pStartTileCollection));

    TileSize defaultTileSize = TileSize_Square150x150;
    if (tile)
    {
        ComPtr<ISecondaryTilePrivate> pSecondaryTilePrivate;
        RETURN_IF_FAILED(tile->QueryInterface(IID_PPV_ARGS(&pSecondaryTilePrivate)));
        RETURN_IF_FAILED(pSecondaryTilePrivate->GetDefaultTileSize(&defaultTileSize));
    }

    RETURN_IF_FAILED(pStartTileCollection->PinToStart(
        pUnifiedTileIdentifier.Get(),
        defaultTileSize == TileSize_Wide310x150 ? TilePinSize_Tile4x2 : TilePinSize_Tile2x2
    ));

    return S_OK;
}

HRESULT (*StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__IsTilePinnedFunc)(
    void* _this,
    HSTRING hstrAumid,
    HSTRING hstrTileId,
    BOOLEAN* out);

HRESULT StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__IsTilePinned(
    void* _this,
    HSTRING hstrAumid,
    HSTRING hstrTileId,
    BOOLEAN* out)
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;
    using namespace ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections;

    if (!dwStartShowClassicMode)
        return StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__IsTilePinnedFunc(_this, hstrAumid, hstrTileId, out);

    *out = FALSE;

    /*if (RtlIsMultiUsersInSessionSku())
        return S_OK;*/

    // Windows 10 start menu-specific code begins here
    ComPtr<ICuratedTileCollectionManager> pTileCollectionManager;
    RETURN_IF_FAILED(RoActivateInstance(
        Wrappers::HStringReference(L"WindowsInternal.Shell.UnifiedTile.CuratedTileCollections.CuratedTileCollectionManager").Get(),
        &pTileCollectionManager
    ));

    ComPtr<IUnifiedTileIdentifier> pUnifiedTileIdentifier;
    RETURN_IF_FAILED(StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__GetUnifiedIdentifierForAumid(
        _this,
        WindowsGetStringRawBuffer(hstrAumid, nullptr),
        WindowsGetStringRawBuffer(hstrTileId, nullptr),
        &pUnifiedTileIdentifier
    ));

    ComPtr<ICuratedTileCollection> pTileCollection;
    RETURN_IF_FAILED(pTileCollectionManager->GetCollection(
        Wrappers::HStringReference(L"Start.TileGrid").Get(),
        &pTileCollection
    ));

    RETURN_IF_FAILED(pTileCollection->DoesCollectionContainTile(pUnifiedTileIdentifier.Get(), nullptr, out));

    return S_OK;
}

HRESULT (*StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__UnpinTileFunc)(
    void* _this,
    HSTRING hstrAumid,
    HSTRING hstrTileId);

HRESULT StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__UnpinTile(
    void* _this,
    HSTRING hstrAumid,
    HSTRING hstrTileId)
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;
    using namespace ABI::WindowsInternal::Shell::UnifiedTile::Private;
    using namespace ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections;

    if (!dwStartShowClassicMode)
        return StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__UnpinTileFunc(_this, hstrAumid, hstrTileId);

    /*if (RtlIsMultiUsersInSessionSku())
        return S_OK;*/

    ComPtr<IUnifiedTileIdentifier> pUnifiedTileIdentifier;
    RETURN_IF_FAILED(StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__GetUnifiedIdentifierForAumid(
        _this,
        WindowsGetStringRawBuffer(hstrAumid, nullptr),
        WindowsGetStringRawBuffer(hstrTileId, nullptr),
        &pUnifiedTileIdentifier
    ));

    // Windows 10 start menu-specific code begins here
    ComPtr<ICuratedTileCollectionManager> pTileCollectionManager;
    RETURN_IF_FAILED(RoActivateInstance(
        Wrappers::HStringReference(L"WindowsInternal.Shell.UnifiedTile.CuratedTileCollections.CuratedTileCollectionManager").Get(),
        &pTileCollectionManager
    ));

    ComPtr<ICuratedTileCollection> pTileCollection;
    RETURN_IF_FAILED(pTileCollectionManager->GetCollection(
        Wrappers::HStringReference(L"Start.TileGrid").Get(),
        &pTileCollection
    ));

    ComPtr<IStartTileCollection> pStartTileCollection;
    RETURN_IF_FAILED(pTileCollection.As(&pStartTileCollection));

    RETURN_IF_FAILED(pStartTileCollection->UnpinFromStart(pUnifiedTileIdentifier.Get()));

    return S_OK;
}

HRESULT PatchStartPinnableSurface(HMODULE hModule, ABI::Windows::Internal::ApplicationModel::IPinnableSurfaceFactory** outPinnableSurfaceFactory)
{
    using namespace ABI::Windows::Internal::ApplicationModel;

    if (outPinnableSurfaceFactory)
        *outPinnableSurfaceFactory = nullptr;

    typedef HRESULT (WINAPI* DllGetActivationFactory_t)(HSTRING, IActivationFactory**);
    DllGetActivationFactory_t pfnGetActivationFactory = (DllGetActivationFactory_t)GetProcAddress(hModule, "DllGetActivationFactory");
    RETURN_HR_IF_NULL(E_FAIL, pfnGetActivationFactory);

    ComPtr<IActivationFactory> activationFactory;
    RETURN_IF_FAILED(pfnGetActivationFactory(
        Wrappers::HStringReference(L"Windows.Internal.ApplicationModel.StartPinnableSurface").Get(),
        activationFactory.ReleaseAndGetAddressOf())
    );

    ComPtr<IPinnableSurfaceFactory> pinnableSurfaceFactory;
    RETURN_IF_FAILED(activationFactory.As(&pinnableSurfaceFactory));

    if (outPinnableSurfaceFactory)
        pinnableSurfaceFactory.CopyTo(outPinnableSurfaceFactory);

    ComPtr<IPinnableSurface> pinnableSurface;
    RETURN_IF_FAILED(pinnableSurfaceFactory->GetCurrent(pinnableSurface.ReleaseAndGetAddressOf()));

    DWORD dwOldProtect = 0;

    void** vtable = *(void***)pinnableSurface.Get();
    void** p_PinTile = &vtable[8];
    void** p_IsTilePinned = &vtable[9];
    void** p_UnpinTile = &vtable[11];

    // PinTile
    if (*p_PinTile != StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__PinTile)
    {
        StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__PinTileFunc =
            (decltype(StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__PinTileFunc))*p_PinTile;
        if (VirtualProtect(p_PinTile, sizeof(void*), PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            *p_PinTile = StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__PinTile;
            VirtualProtect(p_PinTile, sizeof(void*), dwOldProtect, &dwOldProtect);
        }
    }

    // IsTilePinned
    if (*p_IsTilePinned != StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__IsTilePinned)
    {
        StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__IsTilePinnedFunc =
            (decltype(StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__IsTilePinnedFunc))*p_IsTilePinned;
        if (VirtualProtect(p_IsTilePinned, sizeof(void*), PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            *p_IsTilePinned = StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__IsTilePinned;
            VirtualProtect(p_IsTilePinned, sizeof(void*), dwOldProtect, &dwOldProtect);
        }
    }

    // UnpinTile
    if (*p_UnpinTile != StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__UnpinTile)
    {
        StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__UnpinTileFunc =
            (decltype(StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__UnpinTileFunc))*p_UnpinTile;
        if (VirtualProtect(p_UnpinTile, sizeof(void*), PAGE_EXECUTE_READWRITE, &dwOldProtect))
        {
            *p_UnpinTile = StartTileData_Windows__Internal__ApplicationModel__StartPinnableSurface__UnpinTile;
            VirtualProtect(p_UnpinTile, sizeof(void*), dwOldProtect, &dwOldProtect);
        }
    }

    return S_OK;
}
#endif

namespace VerbGlyphs::SegoeMDL2Assets
{
    const WCHAR* const Pin   = L"\uE718";
    const WCHAR* const Unpin = L"\uE77A";
}

class EPStartPinUnpinTileVerb : public RuntimeClass<RuntimeClassFlags<WinRt>, ABI::WindowsInternal::Shell::UnifiedTile::ITileVerb, FtmBase>
{
public:
    EPStartPinUnpinTileVerb() :
        m_flags(ABI::WindowsInternal::Shell::UnifiedTile::TileVerbFlags_None)
    {
    }

    HRESULT RuntimeClassInitialize(
        ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier* unifiedTileIdentifier,
        bool bShowPin,
        ABI::WindowsInternal::Shell::UnifiedTile::IVerbEnumerationArgs* verbEnumerationArgs,
        HSTRING verbProviderId,
        HSTRING groupPath)
    {
        using namespace ABI::WindowsInternal::Shell::UnifiedTile;
        using namespace ABI::WindowsInternal::Shell::UnifiedTile::Private;

        m_unifiedTileIdentifier = unifiedTileIdentifier;

        VerbEnumerationOptions options;
        RETURN_IF_FAILED(verbEnumerationArgs->get_Options(&options));

        ComPtr<IVerbEnumerationArgsPrivate> pVerbEnumerationArgsPrivate;
        RETURN_IF_FAILED(verbEnumerationArgs->QueryInterface(IID_PPV_ARGS(&pVerbEnumerationArgsPrivate)));
        RETURN_IF_FAILED(pVerbEnumerationArgsPrivate->get_User(&m_user));

        if (bShowPin)
        {
            RETURN_IF_FAILED(m_canonicalName.Set(L"StartPin"));
            if ((options & VerbEnumerationOptions_ExcludeResources) == 0)
            {
                RETURN_IF_FAILED(m_glyph.Set(VerbGlyphs::SegoeMDL2Assets::Pin));
            }
        }
        else
        {
            RETURN_IF_FAILED(m_canonicalName.Set(L"StartUnpin"));
            if ((options & VerbEnumerationOptions_ExcludeResources) == 0)
            {
                RETURN_IF_FAILED(m_glyph.Set(VerbGlyphs::SegoeMDL2Assets::Unpin));
            }
        }

        if ((options & VerbEnumerationOptions_ExcludeResources) == 0)
        {
            WCHAR szDisplayName[260];
            int written = LoadStringW(GetModuleHandleW(L"StartTileData.dll"), bShowPin ? 1007 : 1008, szDisplayName, ARRAYSIZE(szDisplayName));
            if (written > 0 && written < ARRAYSIZE(szDisplayName))
            {
                RETURN_IF_FAILED(m_displayName.Set(szDisplayName));
            }
            else
            {
                RETURN_IF_FAILED(m_displayName.Set(m_canonicalName));
            }
            RETURN_IF_FAILED(m_glyphFontFamily.Set(L"Segoe Fluent Icons"));
        }

        RETURN_IF_FAILED(m_verbProviderId.Set(verbProviderId));
        RETURN_IF_FAILED(m_groupPath.Set(groupPath));

        m_flags |= TileVerbFlags_CanExecute;

        return S_OK;
    }

    STDMETHODIMP get_VerbProviderId(HSTRING* out) override { RETURN_HR(m_verbProviderId.CopyTo(out)); }
    STDMETHODIMP get_GroupPath(HSTRING* out) override { RETURN_HR(m_groupPath.CopyTo(out)); }
    STDMETHODIMP get_CanonicalName(HSTRING* out) override { RETURN_HR(m_canonicalName.CopyTo(out)); }
    STDMETHODIMP get_DisplayName(HSTRING* out) override { RETURN_HR(m_displayName.CopyTo(out)); }
    STDMETHODIMP get_Glyph(HSTRING* out) override { RETURN_HR(m_glyph.CopyTo(out)); }
    STDMETHODIMP get_GlyphFontFamily(HSTRING* out) override { RETURN_HR(m_glyphFontFamily.CopyTo(out)); }
    STDMETHODIMP get_AccessKey(HSTRING* out) override { RETURN_HR(m_accessKey.CopyTo(out)); }
    STDMETHODIMP get_ShortcutText(HSTRING* out) override { RETURN_HR(m_shortcutText.CopyTo(out)); }
    STDMETHODIMP get_Flags(ABI::WindowsInternal::Shell::UnifiedTile::TileVerbFlags* out) override { *out = m_flags; return S_OK; }

    STDMETHODIMP Execute(ABI::WindowsInternal::Shell::UnifiedTile::IVerbExecutionArgs* verbExecutionArgs) override
    {
        using namespace ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections;

        ComPtr<ICuratedTileCollectionManagerStatics> pCuratedTileCollectionManagerStatics;
        RETURN_IF_FAILED(RoGetActivationFactory(
            Wrappers::HStringReference(L"WindowsInternal.Shell.UnifiedTile.CuratedTileCollections.CuratedTileCollectionManager").Get(),
            IID_PPV_ARGS(&pCuratedTileCollectionManagerStatics)
        ));

        ComPtr<ICuratedTileCollectionManager> pCuratedTileCollectionManager;
        RETURN_IF_FAILED(pCuratedTileCollectionManagerStatics->CreateWithUser(m_user.Get(), &pCuratedTileCollectionManager));

        ComPtr<ICuratedTileCollection> pTileCollection;
        RETURN_IF_FAILED(pCuratedTileCollectionManager->GetCollection(
            Wrappers::HStringReference(L"Start.TileGrid").Get(),
            &pTileCollection
        ));

        BOOLEAN bCollectionContainsTile;
        RETURN_IF_FAILED(pTileCollection->DoesCollectionContainTile(m_unifiedTileIdentifier.Get(), nullptr, &bCollectionContainsTile));
        bool bPinned = bCollectionContainsTile != FALSE;

        bool bPin = m_canonicalName == Wrappers::HStringReference(L"StartPin").Get();
        if (bPin != bPinned)
        {
            ComPtr<IStartTileCollection> pStartTileCollection;
            RETURN_IF_FAILED(pTileCollection.As(&pStartTileCollection));

            if (bPin)
            {
                RETURN_IF_FAILED(pStartTileCollection->PinToStart(m_unifiedTileIdentifier.Get(), TilePinSize_Tile2x2));
            }
            else
            {
                RETURN_IF_FAILED(pStartTileCollection->UnpinFromStart(m_unifiedTileIdentifier.Get()));
            }
        }

        return S_OK;
    }

    STDMETHODIMP ExecuteAsync(ABI::WindowsInternal::Shell::UnifiedTile::IVerbExecutionArgs* verbExecutionArgs, ABI::Windows::Foundation::IAsyncOperation<bool>** out) override
    {
        winrt::Windows::Foundation::IAsyncOperation<bool> asyncOp = InternalExecuteAsync(verbExecutionArgs);
        *out = (ABI::Windows::Foundation::IAsyncOperation<bool>*)winrt::detach_abi(asyncOp);
        return S_OK;
    }

    winrt::Windows::Foundation::IAsyncOperation<bool> InternalExecuteAsync(ABI::WindowsInternal::Shell::UnifiedTile::IVerbExecutionArgs* verbExecutionArgs)
    {
        co_await winrt::resume_background();
        co_return SUCCEEDED(Execute(verbExecutionArgs));
    }

    Wrappers::HString m_verbProviderId;
    Wrappers::HString m_groupPath;
    Wrappers::HString m_canonicalName;
    Wrappers::HString m_displayName;
    Wrappers::HString m_glyph;
    Wrappers::HString m_glyphFontFamily;
    Wrappers::HString m_accessKey;
    Wrappers::HString m_shortcutText;
    ComPtr<ABI::Windows::System::IUser> m_user;
    ABI::WindowsInternal::Shell::UnifiedTile::TileVerbFlags m_flags;

    ComPtr<ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileIdentifier> m_unifiedTileIdentifier;
};

namespace ABI::Windows::Foundation::Collections
{
    template <>
    struct __declspec(uuid("22e86da4-c5d3-50e2-b649-dd5a9e58fd26"))
    IVector<ABI::WindowsInternal::Shell::UnifiedTile::ITileVerb*> : IVector_impl<ABI::WindowsInternal::Shell::UnifiedTile::ITileVerb*>
    {
    };
}

HRESULT (*WindowsInternal__Shell__UnifiedTile__Private__UnifiedTilePinUnpinVerbProvider__AddStartPinUnpinVerbIfApplicableFunc)(
    void* _this,
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTile* tile,
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileManager* manager,
    ABI::WindowsInternal::Shell::UnifiedTile::IVerbEnumerationArgs* verbEnumerationArgs,
    ABI::Windows::Foundation::Collections::IVector<ABI::WindowsInternal::Shell::UnifiedTile::ITileVerb*>* verbs);

HRESULT WindowsInternal__Shell__UnifiedTile__Private__UnifiedTilePinUnpinVerbProvider__AddStartPinUnpinVerbIfApplicable(
    void* _this,
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTile* tile,
    ABI::WindowsInternal::Shell::UnifiedTile::IUnifiedTileManager* manager,
    ABI::WindowsInternal::Shell::UnifiedTile::IVerbEnumerationArgs* verbEnumerationArgs,
    ABI::Windows::Foundation::Collections::IVector<ABI::WindowsInternal::Shell::UnifiedTile::ITileVerb*>* verbs)
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;
    using namespace ABI::WindowsInternal::Shell::UnifiedTile::Private;

    if (!dwStartShowClassicMode)
        return WindowsInternal__Shell__UnifiedTile__Private__UnifiedTilePinUnpinVerbProvider__AddStartPinUnpinVerbIfApplicableFunc(_this, tile, manager, verbEnumerationArgs, verbs);

    ComPtr<IUnifiedTileIdentifier> pUnifiedTileIdentifier;
    THROW_IF_FAILED(tile->get_Id(&pUnifiedTileIdentifier));

    UnifiedTileIdentifierKind kind;
    THROW_IF_FAILED(pUnifiedTileIdentifier->get_Kind(&kind));

    ComPtr<IVerbEnumerationArgsPrivate> pVerbEnumerationArgsPrivate;
    THROW_IF_FAILED(verbEnumerationArgs->QueryInterface(IID_PPV_ARGS(&pVerbEnumerationArgsPrivate)));

    bool bCanProceed = false;

    BOOLEAN bIsStartPin;
    if (SUCCEEDED_LOG(pVerbEnumerationArgsPrivate->IsMatchingVerbCanonicalName(Wrappers::HStringReference(L"StartPin").Get(), &bIsStartPin)) && bIsStartPin)
    {
        bCanProceed = kind != UnifiedTileIdentifierKind_Unknown;
    }
    else
    {
        BOOLEAN bIsStartUnpin;
        if (SUCCEEDED_LOG(pVerbEnumerationArgsPrivate->IsMatchingVerbCanonicalName(Wrappers::HStringReference(L"StartUnpin").Get(), &bIsStartUnpin)) && bIsStartUnpin)
        {
            bCanProceed = kind != UnifiedTileIdentifierKind_Unknown;
        }
    }

    if (!bCanProceed)
        return S_OK;

    // Windows 10 start menu-specific code begins here
    ComPtr<IUnifiedTileCollection> pTileCollection;
    THROW_IF_FAILED(manager->GetCollection(CollectionProvider_CuratedTileCollection, Wrappers::HStringReference(L"Start.TileGrid").Get(), &pTileCollection)); // @MOD Not accessing field_58

    ComPtr<ITileContainerPrivate> pTileContainerPrivate;
    THROW_IF_FAILED(pTileCollection.As(&pTileContainerPrivate));

    bool bPreventPinning;

    ComPtr<ITileCollectionContainer> pTileCollectionContainer;
    bool bCollectionFound = pTileContainerPrivate->TryFindTileByUnifiedTileIdRecursive(pUnifiedTileIdentifier.Get(), nullptr, &pTileCollectionContainer);

    if (bCollectionFound)
    {
        bPreventPinning = false;
    }
    else if (kind == UnifiedTileIdentifierKind_TargetedContent)
    {
        bPreventPinning = true;
    }
    else
    {
        ComPtr<IUnifiedTilePrivate> pUnifiedTilePrivate;
        THROW_IF_FAILED(tile->QueryInterface(IID_PPV_ARGS(&pUnifiedTilePrivate)));

        BOOLEAN bVisibleInAppList;
        THROW_IF_FAILED(pUnifiedTilePrivate->get_IsVisibleInAppList(&bVisibleInAppList));

        if (bVisibleInAppList)
        {
            bPreventPinning = false;
            if (kind == UnifiedTileIdentifierKind_Win32)
            {
                ComPtr<IWin32ShortcutInfo> pWin32ShortcutInfo;
                THROW_IF_FAILED(tile->get_Win32ShortcutInfo(&pWin32ShortcutInfo));

                if (pWin32ShortcutInfo.Get())
                {
                    BOOLEAN bLocalPreventPinning;
                    if (SUCCEEDED_LOG(pWin32ShortcutInfo->get_PreventPinning(&bLocalPreventPinning)))
                        bPreventPinning = bLocalPreventPinning != FALSE;
                }
            }
        }
        else
        {
            bPreventPinning = true;
        }
    }

    if (!bPreventPinning)
    {
        // We are not playing around with policies for now
    }

    ComPtr<ITileVerb> pTileVerb;
    THROW_IF_FAILED(MakeAndInitialize<EPStartPinUnpinTileVerb>(&pTileVerb, pUnifiedTileIdentifier.Get(), !bCollectionFound, verbEnumerationArgs, *(HSTRING*)((PBYTE)_this + 0x40), nullptr));
    THROW_IF_FAILED(verbs->Append(pTileVerb.Get()));

    return S_OK;
}

HRESULT PatchUnifiedTilePinUnpinProvider(HMODULE hModule)
{
    PBYTE pText;
    DWORD cbText;
    RETURN_HR_IF(E_NOT_SET, !TextSectionBeginAndSize(hModule, &pText, &cbText));

#if defined(_M_X64)
    PBYTE match;
    SIZE_T offset = (SIZE_T)pText;
    while (true)
    {
        // 48 89 ?? 24 ?? 4C 8B ?? 4C 8B 44 24 ?? 49 8B ?? ?? 8B ?? E8 ?? ?? ?? ??
        //                                                             ^^^^^^^^^^^
        match = (PBYTE)FindPattern(
            (PVOID)offset,
            cbText - (DWORD)(offset - (SIZE_T)pText),
            "\x48\x89\x00\x24\x00\x4C\x8B\x00\x4C\x8B\x44\x24\x00\x49\x8B\x00\x00\x8B\x00\xE8",
            "xx?x?xx?xxxx?xx??x?x"
        );
        if (!match)
        {
            // We tried our best, but we found nothing...
            break;
        }

        // Possible match, prepare the start offset for the next search
        offset += ((SIZE_T)match - offset) + 24 /*first pattern size*/;

        // Check the referred function's preamble to see if this is what we're looking for
        match += 19;
        match += 5 + *(int*)(match + 1);

        // 41 54 41 55 41 56 41 57 48
        PBYTE matchPreambleTest = (PBYTE)FindPattern(
            match,
            9 /*second pattern size*/ + 8 /*should start within these first bytes*/,
            "\x41\x54\x41\x55\x41\x56\x41\x57\x48",
            "xxxxxxxxx"
        );

        if (matchPreambleTest)
        {
            // Got it!
            break;
        }
    }
#elif defined(_M_ARM64)
    // ?? ?? 40 F9 E3 03 15 AA ?? ?? 40 F9 E1 03 ?? AA E0 03 ?? AA ?? ?? ?? ?? E3 03 00 2A // NI, GE
    //                                                             ^^^^^^^^^^^
    // Ref: WindowsInternal::Shell::UnifiedTile::Private::UnifiedTilePinUnpinVerbProvider::GetVerbs()
    PBYTE match = (PBYTE)FindPattern_4_(
        pText + 2,
        cbText - 2,
        "\x40\xF9\xE3\x03\x15\xAA\x00\x00\x40\xF9\xE1\x03\x00\xAA\xE0\x03\x00\xAA\x00\x00\x00\x00\xE3\x03\x00\x2A",
        "xxxxxx??xxxx?xxx?x????xxxx"
    );
    if (match)
    {
        match += 18;
        match = (PBYTE)ARM64_FollowBL((DWORD*)match);
    }
    else
    {
        // E4 8A 40 A9 E3 03 ?? AA E1 03 ?? AA E0 03 ?? AA ?? ?? ?? ?? ?? ?? ?? F9 E3 03 00 2A // BR
        //                                                 ^^^^^^^^^^^
        // Ref: WindowsInternal::Shell::UnifiedTile::Private::UnifiedTilePinUnpinVerbProvider::GetVerbs()
        match = (PBYTE)FindPattern_4_(
            pText,
            cbText,
            "\xE4\x8A\x40\xA9\xE3\x03\x00\xAA\xE1\x03\x00\xAA\xE0\x03\x00\xAA\x00\x00\x00\x00\x00\x00\x00\xF9\xE3\x03\x00\x2A",
            "xxxxxx?xxx?xxx?x???????xxxxx"
        );
        if (match)
        {
            match += 16;
            match = (PBYTE)ARM64_FollowBL((DWORD*)match);
        }
    }
#endif

    int rv = -1;
    if (match)
    {
        WindowsInternal__Shell__UnifiedTile__Private__UnifiedTilePinUnpinVerbProvider__AddStartPinUnpinVerbIfApplicableFunc =
            (decltype(WindowsInternal__Shell__UnifiedTile__Private__UnifiedTilePinUnpinVerbProvider__AddStartPinUnpinVerbIfApplicableFunc))match;
        rv = funchook_prepare(
            funchook,
            (void**)&WindowsInternal__Shell__UnifiedTile__Private__UnifiedTilePinUnpinVerbProvider__AddStartPinUnpinVerbIfApplicableFunc,
            WindowsInternal__Shell__UnifiedTile__Private__UnifiedTilePinUnpinVerbProvider__AddStartPinUnpinVerbIfApplicable
        );
    }
    if (rv != 0)
    {
        printf("Failed to hook UnifiedTilePinUnpinVerbProvider::GetVerbs(). rv = %d\n", rv);
    }

    return S_OK;
}

#define FIND_PATTERN_WITH_GAP(pFile, dwSize, pat1, msk1, siz1, gap1and2, pat2, msk2, siz2, postprocess, dest) \
	do \
	{ \
		PBYTE pCurrent = (pFile); \
		while (pCurrent + (siz1) + (gap1and2) + (siz2) < (pFile) + (dwSize)) \
		{ \
			PBYTE matchLocal = (PBYTE)FindPattern( \
				pCurrent, \
				(SIZE_T)(dwSize) - (SIZE_T)(pCurrent - (SIZE_T)(pFile)), \
				(pat1), \
				(msk1) \
			); \
			if (!matchLocal) \
			{ \
				break; /* We tried our best, but we found nothing... */ \
			} \
 			\
			/* Possible match, shift to continuation search start */ \
			pCurrent = matchLocal + (siz1); \
 			\
			if (!(pCurrent + (gap1and2) + (siz2) < (pFile) + (dwSize))) \
			{ \
				break; /* Not enough space for continuation */ \
			} \
 			\
			/* Check continuation */ \
			PBYTE matchContinuationTest = (PBYTE)FindPattern( \
				pCurrent, \
				(gap1and2) + (siz2), \
				(pat2), \
				(msk2) \
			); \
			if (!matchContinuationTest) \
			{ \
				continue; /* Not this one, continue at first pattern + pattern size */ \
			} \
 			\
			matchLocal = (postprocess)(matchLocal); \
			if (!matchLocal) \
			{ \
				continue; /* Not this one, continue at first pattern + pattern size */ \
			} \
 			\
			*(dest) = matchLocal; \
			break; /* Got it! */ \
		} \
	} \
	while (false)

// ALL pointers and sizes must be multiples of 4
#define FIND_PATTERN_WITH_GAP_ARM(pFile, dwSize, pat1, msk1, siz1, gap1and2, pat2, msk2, siz2, postprocess, dest) \
	do \
	{ \
		PBYTE pCurrent = (pFile); \
		while (pCurrent + (siz1) + (gap1and2) + (siz2) < (pFile) + (dwSize)) \
		{ \
			PBYTE matchLocal = (PBYTE)FindPatternBitMask_4_( \
				pCurrent, \
				(SIZE_T)(dwSize) - (SIZE_T)(pCurrent - (SIZE_T)(pFile)), \
				(pat1), \
				(msk1), \
				(siz1) \
			); \
			if (!matchLocal) \
			{ \
				break; /* We tried our best, but we found nothing... */ \
			} \
 			\
			/* Possible match, shift to continuation search start */ \
			pCurrent = matchLocal + (siz1); \
 			\
			if (!(pCurrent + (gap1and2) + (siz2) < (pFile) + (dwSize))) \
			{ \
				break; /* Not enough space for continuation */ \
			} \
 			\
			/* Check continuation */ \
			PBYTE matchContinuationTest = (PBYTE)FindPatternBitMask_4_( \
				pCurrent, \
				(gap1and2) + (siz2), \
				(pat2), \
				(msk2), \
				(siz2) \
			); \
			if (!matchContinuationTest) \
			{ \
				continue; /* Not this one, continue at first pattern + pattern size */ \
			} \
 			\
			matchLocal = (postprocess)(matchLocal); \
			if (!matchLocal) \
			{ \
				continue; /* Not this one, continue at first pattern + pattern size */ \
			} \
 			\
			*(dest) = matchLocal; \
			break; /* Got it! */ \
		} \
	} \
	while (false)

void* g_ctc_MakeAndInitialize_CDSStartCollectionWriter_Func;
void* g_ctc_CreateLayoutInitializationLayoutRoot_Func;
void* g_ctc_MakeAndInitialize_CDSLayoutProvider_Func;
void* g_ctc_MakeShared_LayoutRootInternal_Func;

void* (__thiscall *g_ctc_LayoutRootInternal_CtorWithTransformerRoot_Func)(void* _this, void* context, void* transformerRoot);
void* (__thiscall *g_pfnLayoutRootInternal_CtorWithTransformerRoot)(void* _this, void* context, void* transformerRoot);
void* g_LayoutRootInternal_CtorWithTransformerRoot_expectedReturnAddress;

void* __thiscall ctc_LayoutRootInternal_CtorWithTransformerRoot_Hook(void* _this, void* context, void* transformerRoot)
{
    if (_ReturnAddress() == g_LayoutRootInternal_CtorWithTransformerRoot_expectedReturnAddress)
    {
        return g_pfnLayoutRootInternal_CtorWithTransformerRoot(_this, context, transformerRoot);
    }
    else
    {
        return g_ctc_LayoutRootInternal_CtorWithTransformerRoot_Func(_this, context, transformerRoot);
    }
}

// void* g_ctc_MakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor_Func;
void* g_ctc_LogAllTilesActivity_Dtor_Func;
void* g_ctc_Create_StartTileGridCollection;

HRESULT BringBackSharedStartLayoutInCuratedTileCollections(HMODULE hModule)
{
    PBYTE pText;
    DWORD cbText;
    RETURN_HR_IF(E_NOT_SET, !TextSectionBeginAndSize(hModule, &pText, &cbText));

    // Microsoft::WRL::Details::MakeAndInitialize<ctc::CDSStartCollectionWriter,ctc::ICollectionWriter,std::wstring &,bool,std::shared_ptr<ctc::CollectionContext> const &>()
    PBYTE matchMakeAndInitialize_CDSStartCollectionWriter = nullptr;
#if defined(_M_X64)
    // - GetCDSStartCollectionWriter() non-inlined (16299, 26100 ~)
    // 48 8B D7 E8 ?? ?? ?? ?? 8B D8 48 8B CF E8 ?? ?? ?? ?? 8B C3
    //             ^^^^^^^^^^^
    // Ref: ctc::GetCDSStartCollectionWriter()
    matchMakeAndInitialize_CDSStartCollectionWriter = (PBYTE)FindPattern(
        pText, cbText,
        "\x48\x8B\xD7\xE8\x00\x00\x00\x00\x8B\xD8\x48\x8B\xCF\xE8\x00\x00\x00\x00\x8B\xC3",
        "xxxx????xxxxxx????xx"
    );
    if (matchMakeAndInitialize_CDSStartCollectionWriter)
    {
        matchMakeAndInitialize_CDSStartCollectionWriter += 3;
        matchMakeAndInitialize_CDSStartCollectionWriter += 5 + *(int*)(matchMakeAndInitialize_CDSStartCollectionWriter + 1);
    }
    else
    {
        // - GetCDSStartCollectionWriter() inlined (17134 ~ 22621)
        // E8 ?? ?? ?? ?? 8B F0 49 8B CE E8 ?? ?? ?? ?? 48 8B 4D 5F
        //    ^^^^^^^^^^^ . Pattern begins here
        // Ref: ctc::StartTileGridCollectionInitializer::CreateStartCollectionPipeline()
        matchMakeAndInitialize_CDSStartCollectionWriter = (PBYTE)FindPattern(
            pText, cbText,
            "\x8B\xF0\x49\x8B\xCE\xE8\x00\x00\x00\x00\x48\x8B\x4D\x5F",
            "xxxxxx????xxxx"
        );
        if (matchMakeAndInitialize_CDSStartCollectionWriter)
        {
            matchMakeAndInitialize_CDSStartCollectionWriter -= 5;
            if (matchMakeAndInitialize_CDSStartCollectionWriter >= pText && *matchMakeAndInitialize_CDSStartCollectionWriter == 0xE8)
            {
                matchMakeAndInitialize_CDSStartCollectionWriter += 5 + *(int*)(matchMakeAndInitialize_CDSStartCollectionWriter + 1);
            }
            else
            {
                matchMakeAndInitialize_CDSStartCollectionWriter = nullptr;
            }
        }
    }
#elif defined(_M_ARM64)
    // - GetCDSStartCollectionWriter() non-inlined (26100 ~)
    // E1 03 13 AA ?? ?? ?? ?? F4 03 00 2A E0 03 13 AA
    //             ^^^^^^^^^^^
    // Ref: ctc::GetCDSStartCollectionWriter()
    matchMakeAndInitialize_CDSStartCollectionWriter = (PBYTE)FindPattern_4_(
        pText, cbText,
        "\xE1\x03\x13\xAA\x00\x00\x00\x00\xF4\x03\x00\x2A\xE0\x03\x13\xAA",
        "xxxx????xxxxxxxx"
    );
    if (matchMakeAndInitialize_CDSStartCollectionWriter)
    {
        matchMakeAndInitialize_CDSStartCollectionWriter += 4;
        matchMakeAndInitialize_CDSStartCollectionWriter = (PBYTE)ARM64_FollowBL((DWORD*)matchMakeAndInitialize_CDSStartCollectionWriter);
    }
    else
    {
        // - GetCDSStartCollectionWriter() inlined (20348 ~ 25398)
        // ?? 42 00 91 ?? ?? ?? ?? ?? 03 00 2A ... ?? 02 00 F9 28 00 80 52
        //             ^^^^^^^^^^^
        // ADD X0, X??, #0x10
        //   P: 0b10010001_00_000000010000_?????_00000 = 91004000 = 00 40 00 91
        //   M: 0b11111111_11_111111111111_00000_11111 = FFFFFC1F = 1F FC FF FF
        // ORR W??, WZR, W0 (MOV W??, W0)
        //   P: 0b00101010_00_0_00000_000000_11111_????? = 2A0003E0 = E0 03 00 2A
        //   M: 0b11111111_11_1_11111_111111_11111_00000 = FFFFFFE0 = E0 FF FF FF
        // STR XZR, [X??]
        //   P: 0b1111100100_000000000000_?????_11111 = F900001F = 1F 00 00 F9
        //   M: 0b1111111111_111111111111_00000_11111 = FFFFFC1F = 1F FC FF FF
        // Ref: ctc::StartTileGridCollectionInitializer::CreateStartCollectionPipeline()
        FIND_PATTERN_WITH_GAP_ARM(
            pText, cbText,

            "\x00\x40\x00\x91\x00\x00\x00\x94\xE0\x03\x00\x2A",
            "\x1F\xFC\xFF\xFF\x00\x00\x00\xFC\xE0\xFF\xFF\xFF",
            12,

            44,

            "\x1F\x00\x00\xF9\x28\x00\x80\x52",
            "\x1F\xFC\xFF\xFF\xFF\xFF\xFF\xFF",
            8,

            [&](PBYTE matchCandidate) -> PBYTE
            {
                matchCandidate += 4;
                return (PBYTE)ARM64_FollowBL((DWORD*)matchCandidate);
            },

            &matchMakeAndInitialize_CDSStartCollectionWriter
        );
    }
#endif

    // ctc::Internal::LayoutRoot::CreateLayoutInitializationLayoutRoot()
    PBYTE matchCreateLayoutInitializationLayoutRoot = nullptr;
#if defined(_M_X64)
    // 16299 ~
    // 48 8D 4D ?? E8 ?? ?? ?? ?? 90 48 8D 4D ?? E8 ?? ?? ?? ?? 48 8B ?? 48 89 ?? ?? 48
    //                ^^^^^^^^^^^
    // Ref: ctc::DefaultLayoutParser::ParseStartLayouts()
    matchCreateLayoutInitializationLayoutRoot = (PBYTE)FindPattern(
        pText, cbText,
        "\x48\x8D\x4D\x00\xE8\x00\x00\x00\x00\x90\x48\x8D\x4D\x00\xE8\x00\x00\x00\x00\x48\x8B\x00\x48\x89\x00\x00\x48",
        "xxx?x????xxxx?x????xx?xx??x"
    );
    if (matchCreateLayoutInitializationLayoutRoot)
    {
        matchCreateLayoutInitializationLayoutRoot += 4;
        matchCreateLayoutInitializationLayoutRoot += 5 + *(int*)(matchCreateLayoutInitializationLayoutRoot + 1);
    }
#elif defined(_M_ARM64)
    // ?? 42 03 91 ?? E3 00 91 ?? ?? ?? ?? 1F 20 03 D5 A0 63 00 91
    //                         ^^^^^^^^^^^
    // Ref: ctc::DefaultLayoutParser::ParseStartLayouts()
    matchCreateLayoutInitializationLayoutRoot = (PBYTE)FindPattern_4_(
        pText + 1, cbText - 1,
        "\x42\x03\x91\x0\xE3\x00\x91\x0\x0\x0\x0\x1F\x20\x03\xD5\xA0\x63\x00\x91",
        "xxx?xxx????xxxxxxxx"
    );
    if (matchCreateLayoutInitializationLayoutRoot)
    {
        matchCreateLayoutInitializationLayoutRoot += 7;
        matchCreateLayoutInitializationLayoutRoot = (PBYTE)ARM64_FollowBL((DWORD*)matchCreateLayoutInitializationLayoutRoot);
    }
#endif

    // Microsoft::WRL::Details::MakeAndInitialize<ctc::CDSLayoutProvider,ctc::IInitialCollectionProvider,unsigned short const (&)[15],std::shared_ptr<ctc::CollectionContext> const &>
    PBYTE matchMakeAndInitialize_CDSLayoutProvider = nullptr;
#if defined(_M_X64)
    // 16299 ~
    // 4C 8D 41 08 48 8B CA E8 ?? ?? ?? ?? 85 C0 79 1A
    //                         ^^^^^^^^^^^
    // Ref: ctc::AppendWin8UpgradeTilesPolicy::GetCustomProvider()
    // Warning: a2 is optimized out to be always L"Start.TileGrid"
    matchMakeAndInitialize_CDSLayoutProvider = (PBYTE)FindPattern(
        pText, cbText,
        "\x4C\x8D\x41\x08\x48\x8B\xCA\xE8\x00\x00\x00\x00\x85\xC0\x79\x1A",
        "xxxxxxxx????xxxx"
    );
    if (matchMakeAndInitialize_CDSLayoutProvider)
    {
        matchMakeAndInitialize_CDSLayoutProvider += 7;
        matchMakeAndInitialize_CDSLayoutProvider += 5 + *(int*)(matchMakeAndInitialize_CDSLayoutProvider + 1);
    }
#elif defined(_M_ARM64)
    // 02 20 00 91 E0 03 13 AA ?? ?? ?? ?? E3 03 00 2A
    //                         ^^^^^^^^^^^
    // Ref: ctc::AppendWin8UpgradeTilesPolicy::GetCustomProvider()
    // Warning: a2 is optimized out to be always L"Start.TileGrid"
    matchMakeAndInitialize_CDSLayoutProvider = (PBYTE)FindPattern_4_(
        pText, cbText,
        "\x02\x20\x00\x91\xE0\x03\x13\xAA\x00\x00\x00\x00\xE3\x03\x00\x2A",
        "xxxxxxxx????xxxx"
    );
    if (matchMakeAndInitialize_CDSLayoutProvider)
    {
        matchMakeAndInitialize_CDSLayoutProvider += 8;
        matchMakeAndInitialize_CDSLayoutProvider = (PBYTE)ARM64_FollowBL((DWORD*)matchMakeAndInitialize_CDSLayoutProvider);
    }
#endif

    // std::make_shared<ctc::Internal::LayoutRootInternal,std::shared_ptr<ctc::CollectionContext> &,std::shared_ptr<DataStoreCache::CuratedTileCollectionTransformer::CuratedRoot> >()
    PBYTE matchMakeShared_LayoutRootInternal = nullptr;
    bool bIsInlined_MakeShared_LayoutRootInternal = false;
    PBYTE matchLayoutRootInternal_CtorWithTransformerRoot = nullptr;
#if defined(_M_X64)
    // 16299 ~
    // E8 ?? ?? ?? ?? 48 8B D0 ?? 8D ?? 10 E8 ?? ?? ?? ?? 48 8B ?? 24
    //    ^^^^^^^^^^^ . Pattern begins here
    // The 48 8B ?? 24 can be:
    // - 48 8B 4C 24 70          <continuation 1 byte after first pattern + pattern size>
    // - 48 8B 8C 24 80 00 00 00 <continuation 4 bytes ...>
    //               . First pattern + pattern size
    //               ----------- 4 bytes max gap
    // Continued by:
    // 48 85 C9 74 06 E8 ?? ?? ?? ?? 90 48 8B ?? 24
    // Ref: ctc::PreserveLayoutPostProcessor::RuntimeClassInitialize()
    FIND_PATTERN_WITH_GAP(
        pText, cbText,

        "\x48\x8B\xD0\x00\x8D\x00\x10\xE8\x00\x00\x00\x00\x48\x8B\x00\x24",
        "xxx?x?xx????xx?x",
        16,

        4,

        "\x48\x85\xC9\x74\x06\xE8\x00\x00\x00\x00\x90\x48\x8B\x00\x24",
        "xxxxxx????xxx?x",
        15,

        [&](PBYTE matchCandidate) -> PBYTE
        {
            matchCandidate -= 5;
            if (matchCandidate >= pText && *matchCandidate == 0xE8)
            {
                return matchCandidate + 5 + *(int*)(matchCandidate + 1);
            }
            else
            {
                return nullptr;
            }
        },

        &matchMakeShared_LayoutRootInternal
    );
#elif defined(_M_ARM64)
    // ?? 82 00 91 ?? ?? ?? ?? E1 03 00 AA ?? 42 00 91
    //             ^^^^^^^^^^^
    // Ref: ctc::PreserveLayoutPostProcessor::RuntimeClassInitialize()
    matchMakeShared_LayoutRootInternal = (PBYTE)FindPattern_4_(
        pText + 1, cbText - 1,
        "\x82\x00\x91\x00\x00\x00\x00\xE1\x03\x00\xAA\x00\x42\x00\x91",
        "xxx????xxxx?xxx"
    );
    if (matchMakeShared_LayoutRootInternal)
    {
        matchMakeShared_LayoutRootInternal += 3;
        matchMakeShared_LayoutRootInternal = (PBYTE)ARM64_FollowBL((DWORD*)matchMakeShared_LayoutRootInternal);
    }
    else
    {
        // Note: 26100+ make_shared is inlined here
        // 00 2E 80 D2 ?? ?? ?? ?? F3 03 00 AA B3 0B 00 F9 ?? ?? ?? ?? ?? ?? ?? ?? E9 03 00 B2 A2 83 01 91 68 26 00 A9
        //                                                 ^^^^^^^^^^^^^^^^^^^^^^^ std::_Ref_count_obj2 vtbl
        // A7 ?? C0 ?? ?? 82 00 91 60 42 00 91 BF 7F ?? A9 A7 1B 80 3D ?? ?? ?? ?? 1F 20 03 D5 68 42 00 91 A1 43 00 91
        //                                                             ^^^^^^^^^^^ Ctor
        // A8 4F 01 A9 ?? 42 00 91 ?? ?? ?? ?? A0 0F 40 F9
        //
        matchMakeShared_LayoutRootInternal = (PBYTE)FindPattern_4_(
            pText, cbText,
            "\x00\x2E\x80\xD2\x00\x00\x00\x00\xF3\x03\x00\xAA\xB3\x0B\x00\xF9\x00\x00\x00\x00\x00\x00\x00\x00\xE9\x03\x00\xB2\xA2\x83\x01\x91\x68\x26\x00\xA9\xA7\x00\xC0\x00\x00\x82\x00\x91\x60\x42\x00\x91\xBF\x7F\x00\xA9\xA7\x1B\x80\x3D\x00\x00\x00\x00\x1F\x20\x03\xD5\x68\x42\x00\x91\xA1\x43\x00\x91\xA8\x4F\x01\xA9\x00\x42\x00\x91\x00\x00\x00\x00\xA0\x0F\x40\xF9",
            "xxxx????xxxxxxxx????????xxxxxxxxxxxxx?x??xxxxxxxxx?xxxxx????xxxxxxxxxxxxxxxx?xxx????xxxx"
        );
        if (matchMakeShared_LayoutRootInternal)
        {
            bIsInlined_MakeShared_LayoutRootInternal = true;

            matchLayoutRootInternal_CtorWithTransformerRoot = matchMakeShared_LayoutRootInternal + 56;
            matchLayoutRootInternal_CtorWithTransformerRoot = (PBYTE)ARM64_FollowBL((DWORD*)matchLayoutRootInternal_CtorWithTransformerRoot);
            if (!matchLayoutRootInternal_CtorWithTransformerRoot)
            {
                matchMakeShared_LayoutRootInternal = nullptr;
            }

            g_LayoutRootInternal_CtorWithTransformerRoot_expectedReturnAddress = matchMakeShared_LayoutRootInternal + 60;
        }
    }
#endif

#if 0
    // wil::MakeAndInitializeOrThrow<ctc::Win8LayoutMigrationPostProcessor,HSTRING__ * &,std::shared_ptr<ctc::CollectionContext> const &>()
    PBYTE matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor = nullptr;
#if defined(_M_X64)
    // - ctc::CreateWin8LayoutMigrationPostProcessor() inlined (17134 ~)
    // 4C 8D 41 08 48 8D 55 28 48 8D 4D 30 E8 ?? ?? ?? ?? 48 8B 08 48 ?? ?? 00
    //                                        ^^^^^^^^^^^
    // 48 ?? ?? 00 ... can be:
    // - 48 83 20 00          <continuation 0 bytes after first pattern + pattern size>
    // - 48 C7 00 00 00 00 00 <continuation 3 bytes ...>
    //               . First pattern + pattern size
    //               -------- 3 bytes max gap
    // Continued by:
    // 48 89 4D 18 C7 45 D8 02 00 00 00
    // Ref: ctc::AppendWin8UpgradeTilesPolicy::GetPostProcessors()
    /*FIND_PATTERN_WITH_GAP(
        pText, cbText,

        "\x4C\x8D\x41\x08\x48\x8D\x55\x28\x48\x8D\x4D\x30\xE8\x00\x00\x00\x00\x48\x8B\x08\x48\x00\x00\x00",
        "xxxxxxxxxxxxx????xxxx??x",
        24,

        3,

        "\x48\x89\x4D\x18\xC7\x45\xD8\x02\x00\x00\x00",
        "xxxxxxxxxxx",
        11,

        [&](PBYTE matchCandidate) -> PBYTE
        {
            matchCandidate += 12;
            return matchCandidate + 5 + *(int*)(matchCandidate + 1);
        },

        &matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor,
    );*/ // This complex 2-step method is not needed
    matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor = (PBYTE)FindPattern(
        pText, cbText,
        "\x4C\x8D\x41\x08\x48\x8D\x55\x28\x48\x8D\x4D\x30\xE8\x00\x00\x00\x00\x48\x8B\x08\x48\x00\x00\x00",
        "xxxxxxxxxxxxx????xxxx??x"
    );
    if (matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor)
    {
        matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor += 12;
        matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor += 5 + *(int*)(matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor + 1);
    }
    else
    {
        // !!! (Please comment this outside the suite, we're not patching 16299 ctc) !!!

        // - ctc::CreateWin8LayoutMigrationPostProcessor() non-inlined (16299)
        // Instead look for Microsoft::WRL::Details::MakeAndInitialize<ctc::Win8LayoutMigrationPostProcessor,ctc::Win8LayoutMigrationPostProcessor,HSTRING__ * &,std::shared_ptr<ctc::CollectionContext> const &>()
        // 4C 8B C3 48 8D 54 24 48 48 8D 4C 24 58 E8 ?? ?? ?? ?? 48 8B 4C 24 38 85 C0
        //                                           ^^^^^^^^^^^
        // Ref: ctc::CreateWin8LayoutMigrationPostProcessor()
        matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor = (PBYTE)FindPattern(
            pText, cbText,
            "\x4C\x8B\xC3\x48\x8D\x54\x24\x48\x48\x8D\x4C\x24\x58\xE8\x0\x0\x0\x0\x48\x8B\x4C\x24\x38\x85\xC0",
            "xxxxxxxxxxxxxx????xxxxxxx"
        );
        if (matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor)
        {
            matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor += 13;
            matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor += 5 + *(int*)(matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor + 1);
        }
    }
#elif defined(_M_ARM64)
    // A1 83 00 91 A0 A3 00 91 A8 13 00 F9 ?? ?? ?? ?? ... (max 16, 21 including masks) ?? 00 80 52
    //                                     ^^^^^^^^^^^
    // Ref: ctc::PreserveLayoutPostProcessor::RuntimeClassInitialize()
    FIND_PATTERN_WITH_GAP(
        pText, cbText,

        "\xA1\x83\x00\x91\xA0\xA3\x00\x91\xA8\x13\x00\xF9",
        "xxxxxxxxxxxx",
        12,

        21,

        "\x00\x80\x52",
        "xxx",
        3,

        [&](PBYTE matchCandidate) -> PBYTE
        {
            matchCandidate += 12;
            return (PBYTE)ARM64_FollowBL((DWORD*)matchCandidate);
        },

        &matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor,
    );
    /*matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor = (PBYTE)FindPatternBitMask_4_(
        pText, cbText,
        "\xA1\x83\x00\x91\xA0\xA3\x00\x91\xA8\x13\x00\xF9\x00\x00\x00\x94",
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\xFC",
        16,
    );
    if (matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor)
    {
        matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor += 12;
        matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor = (PBYTE)ARM64_FollowBL((DWORD*)matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor);
    }*/
#endif
#endif

    // CommonStartTelemetry::LogAllTilesActivity::~LogAllTilesActivity()
    PBYTE matchLogAllTilesActivity_Dtor = nullptr;
#if defined(_M_X64)
    // 16299~
    // 48 85 C9 74 06 E8 ?? ?? ?? ?? 90 49 8B ?? E8 ?? ?? ?? ?? 33 C0 48 8B 4D ?? 48 33 CC
    //                                              ^^^^^^^^^^^
    // Ref: ctc::GenericCollectionWriter::WriteCollection()
    matchLogAllTilesActivity_Dtor = (PBYTE)FindPattern(
        pText, cbText,
        "\x48\x85\xC9\x74\x06\xE8\x00\x00\x00\x00\x90\x49\x8B\x00\xE8\x00\x00\x00\x00\x33\xC0\x48\x8B\x4D\x00\x48\x33\xCC",
        "xxxxxx????xxx?x????xxxxx?xxx"
    );
    if (matchLogAllTilesActivity_Dtor)
    {
        matchLogAllTilesActivity_Dtor += 14;
        matchLogAllTilesActivity_Dtor += 5 + *(int*)(matchLogAllTilesActivity_Dtor + 1);
    }
#elif defined(_M_ARM64)
    // ?? ?? 40 F9 60 00 00 B4 ?? ?? ?? ?? 1F 20 03 D5 E0 03 ?? AA ?? ?? ?? ?? 00 00 80 52 FF 43 01 91
    //                                                             ^^^^^^^^^^^
    // Ref: ctc::GenericCollectionWriter::WriteCollection()
    matchLogAllTilesActivity_Dtor = (PBYTE)FindPattern_4_(
        pText + 2, cbText - 2,
        "\x40\xF9\x60\x00\x00\xB4\x00\x00\x00\x00\x1F\x20\x03\xD5\xE0\x03\x00\xAA\x00\x00\x00\x00\x00\x00\x80\x52\xFF\x43\x01\x91",
        "xxxxxx????xxxxxx?x????xxxxxxxx"
    );
    if (matchLogAllTilesActivity_Dtor)
    {
        matchLogAllTilesActivity_Dtor += 18;
        matchLogAllTilesActivity_Dtor = (PBYTE)ARM64_FollowBL((DWORD*)matchLogAllTilesActivity_Dtor);
    }
#endif

    // ctc::FindCollectionTypesEntryForCollection()
    // Call with L"Start.TileGrid" to get ctc::Create_StartTileGridCollectionInitializer() & ctc::Create_StartTileGridCollection()
    PBYTE matchFindCollectionTypesEntryForCollection = nullptr;
#if defined(_M_X64)
    // 49 8B ?? 48 8D 4D ?? E8 ?? ?? ?? ?? 48 8B C8 E8 ?? ?? ?? ?? 48 85 C0
    //                                                 ^^^^^^^^^^^
    // Ref: ctc::CuratedTileCollectionManager::GetCollectionForCollectionName()
    matchFindCollectionTypesEntryForCollection = (PBYTE)FindPattern(
        pText, cbText,
        "\x49\x8B\x00\x48\x8D\x4D\x00\xE8\x00\x00\x00\x00\x48\x8B\xC8\xE8\x00\x00\x00\x00\x48\x85\xC0",
        "xx?xxx?x????xxxx????xxx"
    );
    if (matchFindCollectionTypesEntryForCollection)
    {
        matchFindCollectionTypesEntryForCollection += 15;
        matchFindCollectionTypesEntryForCollection += 5 + *(int*)(matchFindCollectionTypesEntryForCollection + 1);
    }
#elif defined(_M_ARM64)
    // FD 7B ?? A9 FD 03 00 91 ?? 03 00 AA ?? 03 01 AA E1 03 02 AA E0 ?? 00 91 ?? ?? ?? ?? ?? ?? ?? ??
    //                                                                                     ^^^^^^^^^^^
    // Ref: ctc::CuratedTileCollectionManager::GetInitializerForCollectionName()
    matchFindCollectionTypesEntryForCollection = (PBYTE)FindPatternBitMask_4_(
        pText, cbText,
        "\xFD\x7B\x00\xA9\xFD\x03\x00\x91\x00\x03\x00\xAA\x00\x03\x01\xAA\xE1\x03\x02\xAA\xE0\x00\x00\x91\x00\x00\x00\x94\x00\x00\x00\x94",
        "\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\xFF\xFF\x00\x00\x00\xFC\x00\x00\x00\xFC",
        32
    );
    if (matchFindCollectionTypesEntryForCollection)
    {
        matchFindCollectionTypesEntryForCollection += 28;
        matchFindCollectionTypesEntryForCollection = (PBYTE)ARM64_FollowBL((DWORD*)matchFindCollectionTypesEntryForCollection);
    }
#endif

    // Concurrency::details::_Task_impl_base::_Wait()
    PBYTE matchConcurrencyDefaultTaskHelperNoCallOnDefaultTaskErrorImpl = nullptr;
    PBYTE matchConcurrencyTaskImplWait = nullptr;
#if defined(_M_X64)
    // 16299~
    // 45 85 C0 78 ?? 48 8B 49 08 48 85 C9 75 06 E8 ?? ?? ?? ?? CC E8 ?? ?? ?? ??
    //                                                                ^^^^^^^^^^^
    // Ref: ctc::TransformerHelpers::BatchAction()
    //      -> MakeAsyncAction() lambda
    PBYTE matchConcurrencyTaskWait2x = (PBYTE)FindPattern(
        pText, cbText,
        "\x45\x85\xC0\x78\x00\x48\x8B\x49\x08\x48\x85\xC9\x75\x06\xE8\x00\x00\x00\x00\xCC\xE8",
        "xxxx?xxxxxxxxxx????xx"
    );
    if (matchConcurrencyTaskWait2x)
    {
        matchConcurrencyDefaultTaskHelperNoCallOnDefaultTaskErrorImpl = matchConcurrencyTaskWait2x + 14;
        matchConcurrencyDefaultTaskHelperNoCallOnDefaultTaskErrorImpl += 5 + *(int*)(matchConcurrencyDefaultTaskHelperNoCallOnDefaultTaskErrorImpl + 1);

        matchConcurrencyTaskImplWait = matchConcurrencyTaskWait2x + 20;
        matchConcurrencyTaskImplWait += 5 + *(int*)(matchConcurrencyTaskImplWait + 1);
    }
#elif defined(_M_ARM64)
    // C2 00 F8 37 00 04 40 F9 40 00 00 B5 ?? ?? ?? ?? ?? ?? ?? ?? 02 00 80 52
    //                                                 ^^^^^^^^^^^
    // Ref: ctc::TransformerHelpers::BatchAction()
    //      -> MakeAsyncAction() lambda
    PBYTE matchConcurrencyTaskWait2x = (PBYTE)FindPattern_4_(
        pText, cbText,
        "\xC2\x00\xF8\x37\x00\x04\x40\xF9\x40\x00\x00\xB5\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x80\x52",
        "xxxxxxxxxxxx????????xxxx"
    );
    if (matchConcurrencyTaskWait2x)
    {
        matchConcurrencyDefaultTaskHelperNoCallOnDefaultTaskErrorImpl = matchConcurrencyTaskWait2x + 12;
        matchConcurrencyDefaultTaskHelperNoCallOnDefaultTaskErrorImpl = (PBYTE)ARM64_FollowBL((DWORD*)matchConcurrencyDefaultTaskHelperNoCallOnDefaultTaskErrorImpl);

        matchConcurrencyTaskImplWait = matchConcurrencyTaskWait2x + 16;
        matchConcurrencyTaskImplWait = (PBYTE)ARM64_FollowBL((DWORD*)matchConcurrencyTaskImplWait);
    }
#endif

    RETURN_HR_IF_NULL(E_NOT_SET, matchMakeAndInitialize_CDSStartCollectionWriter);
    RETURN_HR_IF_NULL(E_NOT_SET, matchCreateLayoutInitializationLayoutRoot);
    RETURN_HR_IF_NULL(E_NOT_SET, matchMakeAndInitialize_CDSLayoutProvider);
    RETURN_HR_IF_NULL(E_NOT_SET, matchMakeShared_LayoutRootInternal);
    // RETURN_HR_IF_NULL(E_NOT_SET, matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor);
    RETURN_HR_IF_NULL(E_NOT_SET, matchLogAllTilesActivity_Dtor);
    RETURN_HR_IF_NULL(E_NOT_SET, matchFindCollectionTypesEntryForCollection);
    RETURN_HR_IF_NULL(E_NOT_SET, matchConcurrencyDefaultTaskHelperNoCallOnDefaultTaskErrorImpl);
    RETURN_HR_IF_NULL(E_NOT_SET, matchConcurrencyTaskImplWait);

    WCHAR szPath[MAX_PATH];
    RETURN_IF_FAILED(SHGetFolderPathW(nullptr, SPECIAL_FOLDER, nullptr, SHGFP_TYPE_CURRENT, szPath));
    RETURN_IF_FAILED(StringCchCatW(szPath, ARRAYSIZE(szPath), _T(APP_RELATIVE_PATH) L"\\ep_starttiledata.dll"));

    wil::unique_hmodule hMyStartTileData(LoadLibraryW(szPath));
    RETURN_LAST_ERROR_IF_NULL(hMyStartTileData);

    g_ctc_MakeAndInitialize_CDSStartCollectionWriter_Func = matchMakeAndInitialize_CDSStartCollectionWriter;
    void* pfnMakeAndInitialize_CDSStartCollectionWriter = GetProcAddress(hMyStartTileData.get(), "??$MakeAndInitialize@VCDSStartCollectionWriter@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@UICollectionWriter@2345@AEAV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@_NAEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@8@@Details@WRL@Microsoft@@YAJPEAPEAUICollectionWriter@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@AEAV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@$$QEA_NAEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@9@@Z");
    RETURN_LAST_ERROR_IF_NULL(pfnMakeAndInitialize_CDSStartCollectionWriter);

    g_ctc_CreateLayoutInitializationLayoutRoot_Func = matchCreateLayoutInitializationLayoutRoot;
    void* pfnCreateLayoutInitializationLayoutRoot = GetProcAddress(hMyStartTileData.get(), "?CreateLayoutInitializationLayoutRoot@LayoutRoot@Internal@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@SA?AV?$shared_ptr@ULayoutRoot@Internal@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@std@@AEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@8@@Z");
    RETURN_LAST_ERROR_IF_NULL(pfnCreateLayoutInitializationLayoutRoot);

    g_ctc_MakeAndInitialize_CDSLayoutProvider_Func = matchMakeAndInitialize_CDSLayoutProvider;
    void* pfnMakeAndInitialize_CDSLayoutProvider = GetProcAddress(hMyStartTileData.get(), "??$MakeAndInitialize@VCDSLayoutProvider@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@UIInitialCollectionProvider@2345@AEAY0P@$$CBGAEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@std@@@Details@WRL@Microsoft@@YAJPEAPEAUIInitialCollectionProvider@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@AEAY0P@$$CBGAEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@std@@@Z");
    RETURN_LAST_ERROR_IF_NULL(pfnMakeAndInitialize_CDSLayoutProvider);

    void* pfnMakeShared_LayoutRootInternal = nullptr;
    void* pvtblRefCountObj2LayoutRootInternal = nullptr;
    // void* pfnLayoutRootInternal_CtorWithTransformerRoot = nullptr;
    if (!bIsInlined_MakeShared_LayoutRootInternal)
    {
        g_ctc_MakeShared_LayoutRootInternal_Func = matchMakeShared_LayoutRootInternal;
        pfnMakeShared_LayoutRootInternal = GetProcAddress(hMyStartTileData.get(), "??$make_shared@VLayoutRootInternal@Internal@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@AEAV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@std@@V?$shared_ptr@VCuratedRoot@CuratedTileCollectionTransformer@DataStoreCache@@@8@@std@@YA?AV?$shared_ptr@VLayoutRootInternal@Internal@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@0@AEAV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@0@$$QEAV?$shared_ptr@VCuratedRoot@CuratedTileCollectionTransformer@DataStoreCache@@@0@@Z");
        RETURN_LAST_ERROR_IF_NULL(pfnMakeShared_LayoutRootInternal);
    }
    else
    {
        pvtblRefCountObj2LayoutRootInternal = GetProcAddress(hMyStartTileData.get(), "??_7?$_Ref_count_obj2@VLayoutRootInternal@Internal@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@std@@6B@");
        RETURN_LAST_ERROR_IF_NULL(pvtblRefCountObj2LayoutRootInternal);

        g_ctc_LayoutRootInternal_CtorWithTransformerRoot_Func = (decltype(g_ctc_LayoutRootInternal_CtorWithTransformerRoot_Func))matchLayoutRootInternal_CtorWithTransformerRoot;
        g_pfnLayoutRootInternal_CtorWithTransformerRoot = (decltype(g_pfnLayoutRootInternal_CtorWithTransformerRoot))GetProcAddress(hMyStartTileData.get(), "??0LayoutRootInternal@Internal@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@QEAA@AEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@std@@V?$shared_ptr@VCuratedRoot@CuratedTileCollectionTransformer@DataStoreCache@@@7@@Z");
        RETURN_LAST_ERROR_IF_NULL(g_pfnLayoutRootInternal_CtorWithTransformerRoot);
    }

    // g_ctc_MakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor_Func = matchMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor;
    // void* pfnMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor = GetProcAddress(hMyStartTileData.get(), "??$MakeAndInitializeOrThrow@VWin8LayoutMigrationPostProcessor@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@AEAPEAUHSTRING__@@AEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@std@@@wil@@YA?AV?$ComPtr@VWin8LayoutMigrationPostProcessor@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@WRL@Microsoft@@AEAPEAUHSTRING__@@AEBV?$shared_ptr@UCollectionContext@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@@std@@@Z");
    // RETURN_LAST_ERROR_IF_NULL(pfnMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor);

    void* pfnCreate_StartTileGridCollection = GetProcAddress(hMyStartTileData.get(), "?Create_StartTileGridCollection@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@@YA?AV?$com_ptr_t@UICuratedTileCollection@CuratedTileCollections@UnifiedTile@Shell@WindowsInternal@ABI@@Uerr_exception_policy@wil@@@wil@@W4CuratedTileCollectionOptionsInternal@1234@PEAUIUser@System@Windows@ABI@@@Z");
    RETURN_LAST_ERROR_IF_NULL(pfnCreate_StartTileGridCollection);

    void** ppfnLogAllTilesActivity_Dtor = (void**)GetProcAddress(hMyStartTileData.get(), "g_pfnLogAllTilesActivity_Dtor");
    RETURN_LAST_ERROR_IF_NULL(ppfnLogAllTilesActivity_Dtor);

    void** ppfnCreate_StartTileGridCollectionInitializer = (void**)GetProcAddress(hMyStartTileData.get(), "g_pfnCreate_StartTileGridCollectionInitializer");
    RETURN_LAST_ERROR_IF_NULL(ppfnCreate_StartTileGridCollectionInitializer);

    void** ppfnConcurrency__details___DefaultTaskHelper___NoCallOnDefaultTask_ErrorImpl = (void**)GetProcAddress(hMyStartTileData.get(), "g_pfnConcurrency__details___DefaultTaskHelper___NoCallOnDefaultTask_ErrorImpl");
    RETURN_LAST_ERROR_IF_NULL(ppfnConcurrency__details___DefaultTaskHelper___NoCallOnDefaultTask_ErrorImpl);

    void** ppfnConcurrency__details___Task_impl_base___Wait = (void**)GetProcAddress(hMyStartTileData.get(), "g_pfnConcurrency__details___Task_impl_base___Wait");
    RETURN_LAST_ERROR_IF_NULL(ppfnConcurrency__details___Task_impl_base___Wait);

    struct CollectionAssociatedTypes
    {
        void* pfnCreateInitializer;
        void* pfnCreate;
    };
    using FindCollectionTypesEntryForCollection_t = const CollectionAssociatedTypes* (*)(std::wstring);
    auto pfnFindCollectionTypesEntryForCollection = reinterpret_cast<FindCollectionTypesEntryForCollection_t>(matchFindCollectionTypesEntryForCollection);
    const CollectionAssociatedTypes* typesStartTileGrid;
    try
    {
        typesStartTileGrid = pfnFindCollectionTypesEntryForCollection(L"Start.TileGrid");
    } CATCH_RETURN()
    RETURN_HR_IF_NULL(E_NOT_SET, typesStartTileGrid);
    RETURN_HR_IF_NULL(E_NOT_SET, typesStartTileGrid->pfnCreateInitializer);
    RETURN_HR_IF_NULL(E_NOT_SET, typesStartTileGrid->pfnCreate);

    g_ctc_Create_StartTileGridCollection = typesStartTileGrid->pfnCreate;

    BYTE rdADRP_MakeShared_LayoutRootInternal = 0;
    BYTE rdADD_MakeShared_LayoutRootInternal = 0;
    BYTE rnADD_MakeShared_LayoutRootInternal = 0;
    DWORD insnADRPNew_MakeShared_LayoutRootInternal = 0;
    DWORD insnADDNew_MakeShared_LayoutRootInternal = 0;
    if (bIsInlined_MakeShared_LayoutRootInternal)
    {
#if defined(_M_X64)
        return E_UNEXPECTED;
#elif defined(_M_ARM64)
        RETURN_HR_IF(HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW), ARM64_DecodeADRLEx(
            (UINT_PTR)(matchMakeShared_LayoutRootInternal + 16), *(DWORD*)(matchMakeShared_LayoutRootInternal + 16),
            *(DWORD*)(matchMakeShared_LayoutRootInternal + 20), &rdADRP_MakeShared_LayoutRootInternal,
            &rdADD_MakeShared_LayoutRootInternal, &rnADD_MakeShared_LayoutRootInternal) == 0);
        RETURN_HR_IF(E_UNEXPECTED, rdADRP_MakeShared_LayoutRootInternal != rnADD_MakeShared_LayoutRootInternal);
        RETURN_HR_IF(HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW), !ARM64_EncodeADRL(
            (UINT_PTR)(matchMakeShared_LayoutRootInternal + 16), (UINT_PTR)pvtblRefCountObj2LayoutRootInternal,
            rdADRP_MakeShared_LayoutRootInternal, rdADD_MakeShared_LayoutRootInternal,
            &insnADRPNew_MakeShared_LayoutRootInternal, &insnADDNew_MakeShared_LayoutRootInternal));
#endif
    }

    RETURN_IF_FAILED(SlimDetoursTransactionBegin());
    auto abortOnFailure = wil::scope_exit([] { LOG_IF_FAILED(SlimDetoursTransactionAbort()); });

    RETURN_IF_FAILED(SlimDetoursAttach(&g_ctc_MakeAndInitialize_CDSStartCollectionWriter_Func, pfnMakeAndInitialize_CDSStartCollectionWriter));
    RETURN_IF_FAILED(SlimDetoursAttach(&g_ctc_CreateLayoutInitializationLayoutRoot_Func, pfnCreateLayoutInitializationLayoutRoot));
    RETURN_IF_FAILED(SlimDetoursAttach(&g_ctc_MakeAndInitialize_CDSLayoutProvider_Func, pfnMakeAndInitialize_CDSLayoutProvider));
    // RETURN_IF_FAILED(SlimDetoursAttach(&g_ctc_MakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor_Func, pfnMakeAndInitializeOrThrow_Win8LayoutMigrationPostProcessor));
    if (!bIsInlined_MakeShared_LayoutRootInternal)
    {
        RETURN_IF_FAILED(SlimDetoursAttach(&g_ctc_MakeShared_LayoutRootInternal_Func, pfnMakeShared_LayoutRootInternal));
    }
    else
    {
        RETURN_IF_FAILED(SlimDetoursAttach((void**)&g_ctc_LayoutRootInternal_CtorWithTransformerRoot_Func, ctc_LayoutRootInternal_CtorWithTransformerRoot_Hook));
    }
    RETURN_IF_FAILED(SlimDetoursAttach(&g_ctc_Create_StartTileGridCollection, pfnCreate_StartTileGridCollection));
    *ppfnLogAllTilesActivity_Dtor = matchLogAllTilesActivity_Dtor;
    *ppfnCreate_StartTileGridCollectionInitializer = typesStartTileGrid->pfnCreateInitializer;
    *ppfnConcurrency__details___DefaultTaskHelper___NoCallOnDefaultTask_ErrorImpl = matchConcurrencyDefaultTaskHelperNoCallOnDefaultTaskErrorImpl;
    *ppfnConcurrency__details___Task_impl_base___Wait = matchConcurrencyTaskImplWait;

    RETURN_IF_FAILED(SlimDetoursTransactionCommit());

    abortOnFailure.release();
    hMyStartTileData.release();
    return S_OK;
}

extern "C" {

void PatchStartTileDataFurther(HMODULE hModule, BOOL bSMEH)
{
    // ComPtr<ABI::Windows::Internal::ApplicationModel::IPinnableSurfaceFactory> pPinnableSurfaceFactory;
    // PatchStartPinnableSurface(hModule, nullptr /*&pPinnableSurfaceFactory*/);

    // if (bSMEH)
        // pPinnableSurfaceFactory->AddRef(); // Pin in memory so that StartTileData.dll doesn't get unloaded

    if (bSMEH
        && ((global_rovi.dwBuildNumber >= 22621 && global_rovi.dwBuildNumber <= 22635) && global_ubr >= 3420
            || global_rovi.dwBuildNumber >= 25169))
    {
        PatchUnifiedTilePinUnpinProvider(hModule);
    }

    // Although this patch works with 11 Start, allow this to be disabled when 10 Start is not in use,
    // in case it crashloops explorer. But Explorer needs to be restarted to take full effect.
    if (dwStartShowClassicMode)
    {
        BringBackSharedStartLayoutInCuratedTileCollections(hModule);
    }
}

} // extern "C"

class StartPinUnpinContextMenuWrapper final
    : public RuntimeClass<RuntimeClassFlags<ClassicCom>
        , IContextMenu
        , IShellExtInit
        , IObjectWithSite
    >
{
public:
    HRESULT RuntimeClassInitialize(IContextMenu* pStock, IContextMenu* pCustom);

    //~ Begin IContextMenu Interface
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags) override;
    STDMETHODIMP InvokeCommand(CMINVOKECOMMANDINFO* pici) override;
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax) override;
    //~ End IContextMenu Interface

    //~ Begin IShellExtInit Interface
    STDMETHODIMP Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject* pDataObj, HKEY hKeyProgID) override;
    //~ End IShellExtInit Interface

    //~ Begin IObjectWithSite Interface
    STDMETHODIMP SetSite(IUnknown* punkSite) override;
    STDMETHODIMP GetSite(REFIID riid, void** ppvSite) override;
    //~ End IObjectWithSite Interface

    ComPtr<IContextMenu> _spStock;
    ComPtr<IContextMenu> _spCustom;
};

HRESULT StartPinUnpinContextMenuWrapper::RuntimeClassInitialize(IContextMenu* pStock, IContextMenu* pCustom)
{
    _spStock = pStock;
    _spCustom = pCustom;
    return S_OK;
}

HRESULT StartPinUnpinContextMenuWrapper::QueryContextMenu(
    HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    return (dwStartShowClassicMode ? _spCustom : _spStock)->QueryContextMenu(
        hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags);
}

HRESULT StartPinUnpinContextMenuWrapper::InvokeCommand(CMINVOKECOMMANDINFO* pici)
{
    return (dwStartShowClassicMode ? _spCustom : _spStock)->InvokeCommand(pici);
}

HRESULT StartPinUnpinContextMenuWrapper::GetCommandString(
    UINT_PTR idCmd, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
    return (dwStartShowClassicMode ? _spCustom : _spStock)->GetCommandString(
        idCmd, uFlags, pwReserved, pszName, cchMax);
}

HRESULT StartPinUnpinContextMenuWrapper::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pDataObj, HKEY hKeyProgID)
{
    {
        ComPtr<IShellExtInit> spStockInit;
        RETURN_IF_FAILED(_spStock.As(&spStockInit));
        RETURN_IF_FAILED(spStockInit->Initialize(pidlFolder, pDataObj, hKeyProgID));
    }
    {
        ComPtr<IShellExtInit> spCustomInit;
        RETURN_IF_FAILED(_spCustom.As(&spCustomInit));
        RETURN_IF_FAILED(spCustomInit->Initialize(pidlFolder, pDataObj, hKeyProgID));
    }
    return S_OK;
}

HRESULT StartPinUnpinContextMenuWrapper::GetSite(const IID& riid, void** ppvSite)
{
    ComPtr<IObjectWithSite> spStockSite;
    RETURN_IF_FAILED(_spStock.As(&spStockSite));
    return spStockSite->GetSite(riid, ppvSite);
}

HRESULT StartPinUnpinContextMenuWrapper::SetSite(IUnknown* punkSite)
{
    {
        ComPtr<IObjectWithSite> spStockSite;
        RETURN_IF_FAILED(_spStock.As(&spStockSite));
        RETURN_IF_FAILED(spStockSite->SetSite(punkSite));
    }
    {
        ComPtr<IObjectWithSite> spCustomSite;
        RETURN_IF_FAILED(_spCustom.As(&spCustomSite));
        RETURN_IF_FAILED(spCustomSite->SetSite(punkSite));
    }
    return S_OK;
}

struct CCacheShortcut
{
    const wchar_t* GetAppID(const void* a2) const
    {
        DWORD dwOffset = *(DWORD*)((PBYTE)this + 44); // Same offset in Windows 10 and 11
        return dwOffset != -1 ? (wchar_t*)((char*)a2 + dwOffset) : nullptr;
    }
};

extern "C"
{

class DECLSPEC_UUID("470c0ebd-5d73-4d58-9ced-e91e22e23282")
StartPinUnpinContextMenu;

HRESULT (STDMETHODCALLTYPE *StartPinUnpinContextMenu_CreateInstance_IClassFactory_Func)(
    IClassFactory* This, IUnknown* pUnkOuter, REFIID riid, void** ppvObject);
HRESULT STDMETHODCALLTYPE StartPinUnpinContextMenu_CreateInstance_IClassFactory_Hook(
    IClassFactory* This, IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
    WCHAR szPath[MAX_PATH];
    RETURN_IF_FAILED(SHGetFolderPathW(nullptr, SPECIAL_FOLDER, nullptr, SHGFP_TYPE_CURRENT, szPath));
    RETURN_IF_FAILED(StringCchCatW(szPath, ARRAYSIZE(szPath), _T(APP_RELATIVE_PATH) L"\\ep_starttiledata.dll"));

    wil::unique_hmodule hMyStartTileData(LoadLibraryW(szPath));
    RETURN_LAST_ERROR_IF_NULL(hMyStartTileData);

    auto pfnDllGetClassObject = (decltype(&DllGetClassObject))GetProcAddress(hMyStartTileData.get(), "DllGetClassObject");
    RETURN_LAST_ERROR_IF_NULL(pfnDllGetClassObject);

    ComPtr<IClassFactory> spFactory;
    RETURN_IF_FAILED(pfnDllGetClassObject(__uuidof(StartPinUnpinContextMenu), IID_PPV_ARGS(&spFactory)));

    ComPtr<IContextMenu> spCustomCM;
    RETURN_IF_FAILED(spFactory->CreateInstance(pUnkOuter, IID_PPV_ARGS(&spCustomCM)));

    ComPtr<IContextMenu> spStockCM;
    RETURN_IF_FAILED(StartPinUnpinContextMenu_CreateInstance_IClassFactory_Func(This, pUnkOuter, IID_PPV_ARGS(&spStockCM)));

    ComPtr<StartPinUnpinContextMenuWrapper> spWrapper;
    RETURN_IF_FAILED(MakeAndInitialize<StartPinUnpinContextMenuWrapper>(&spWrapper, spStockCM.Get(), spCustomCM.Get()));
    RETURN_IF_FAILED(spWrapper.CopyTo(riid, ppvObject));

    hMyStartTileData.release();
    return S_OK;
}

extern HRESULT PatchAppResolver_PatchStartPinUnpinContextMenu(HMODULE hAppResolver)
{
    auto pfnDllGetClassObject = (decltype(&DllGetClassObject))GetProcAddress(hAppResolver, "DllGetClassObject");
    RETURN_LAST_ERROR_IF_NULL(pfnDllGetClassObject);

    ComPtr<IClassFactory> spFactory;
    RETURN_IF_FAILED(pfnDllGetClassObject(__uuidof(StartPinUnpinContextMenu), IID_PPV_ARGS(&spFactory)));

    void** vtable = *(void***)spFactory.Get();
    REPLACE_VTABLE_ENTRY(vtable, 3, StartPinUnpinContextMenu_CreateInstance_IClassFactory_);

    return S_OK;
}

HRESULT (__thiscall *AppResolver_CAppResolverCacheBuilder__AddUserPinnedShortcutToStartFunc)(void* _this, const CCacheShortcut* a2, const void* a3);
HRESULT __thiscall AppResolver_CAppResolverCacheBuilder__AddUserPinnedShortcutToStart(void* _this, const CCacheShortcut* a2, const void* a3)
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;
    using namespace ABI::WindowsInternal::Shell::UnifiedTile::Private;
    using namespace ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections;

    if (!dwStartShowClassicMode)
        return AppResolver_CAppResolverCacheBuilder__AddUserPinnedShortcutToStartFunc(_this, a2, a3);

    // UnifiedTileIdentifier^ tileIdentifier = UnifiedTileIdentifier::Create(a2->GetAppID(a3));
    // StartTilePinnedShortcutsManager::CreateUserPinnedShortcutTile(tileIdentifier);
    // CuratedTileCollectionManager^ tileCollectionManager = ref new CuratedTileCollectionManager();
    // CuratedTileCollection^ tileCollection = tileCollectionManager->GetCollection(L"Start.TileGrid");
    // tileCollection->PinToStart(tileIdentifier, TilePinSize::Tile2x2);
    // tileCollection->Commit();

    ComPtr<IWin32UnifiedTileIdentifierFactory> pTileIdentifierFactory;
    RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
        Wrappers::HStringReference(L"WindowsInternal.Shell.UnifiedTile.UnifiedTileIdentifier").Get(),
        &pTileIdentifierFactory
    ));

    ComPtr<IUnifiedTileIdentifier> pTileIdentifier;
    const wchar_t* pwszAppId = a2->GetAppID(a3);
    RETURN_IF_FAILED(pTileIdentifierFactory->Create(
        Wrappers::HStringReference(pwszAppId).Get(),
        &pTileIdentifier
    ));

    ComPtr<IUnifiedTileUserPinHelperStatics> pTileUserPinHelper;
    RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
        Wrappers::HStringReference(L"WindowsInternal.Shell.UnifiedTile.Private.UnifiedTileUserPinHelper").Get(),
        &pTileUserPinHelper
    ));

    RETURN_IF_FAILED(pTileUserPinHelper->CreateUserPinnedShortcutTile(pTileIdentifier.Get()));

    // Windows 10 start menu-specific code begins here
    ComPtr<ICuratedTileCollectionManager> pTileCollectionManager;
    RETURN_IF_FAILED(RoActivateInstance(
        Wrappers::HStringReference(L"WindowsInternal.Shell.UnifiedTile.CuratedTileCollections.CuratedTileCollectionManager").Get(),
        &pTileCollectionManager
    ));

    ComPtr<ICuratedTileCollection> pTileCollection;
    RETURN_IF_FAILED(pTileCollectionManager->GetCollection(
        Wrappers::HStringReference(L"Start.TileGrid").Get(),
        &pTileCollection
    ));

    ComPtr<IStartTileCollection> pStartTileCollection;
    RETURN_IF_FAILED(pTileCollection.As(&pStartTileCollection));

    RETURN_IF_FAILED(pStartTileCollection->PinToStart(pTileIdentifier.Get(), TilePinSize_Tile2x2));

    RETURN_IF_FAILED(pTileCollection->Commit());

    return S_OK;
}

} // extern "C"

#include "utility.h"
#include "hooking.h"

#include <windows.h>
#include <windows.system.h>
#include <windows.ui.shell.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.system.h>
#include <wil/winrt.h>
#pragma comment(lib, "Psapi.lib")

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

    namespace DataStoreCache::CuratedTileCollectionTransformer
    {
        class CuratedTile;
    }

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
    MODULEINFO mi;
    RETURN_IF_WIN32_BOOL_FALSE(GetModuleInformation(GetCurrentProcess(), hModule, &mi, sizeof(mi)));

#if defined(_M_X64)
    // 40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ?? 48 81 EC ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 45 ?? 49 8B ?? 4D 8B ?? 48 8B ?? 4C 8B ?? 4C 89 4D
    PBYTE match = (PBYTE)FindPattern(
        hModule,
        mi.SizeOfImage,
        "\x40\x55\x53\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\x6C\x24\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC4\x48\x89\x45\x00\x49\x8B\x00\x4D\x8B\x00\x48\x8B\x00\x4C\x8B\x00\x4C\x89\x4D",
        "xxxxxxxxxxxxxxxxx?xxx????xxx????xxxxxx?xx?xx?xx?xx?xxx"
    );
#elif defined(_M_ARM64)
    // E4 06 40 F9 E3 03 15 AA E2 0E 40 F9 E1 03 19 AA E0 03 16 AA ?? ?? ?? ?? E3 03 00 2A
    //                                                             ^^^^^^^^^^^
    // Ref: WindowsInternal::Shell::UnifiedTile::Private::UnifiedTilePinUnpinVerbProvider::GetVerbs()
    PBYTE match = (PBYTE)FindPattern(
        hModule,
        mi.SizeOfImage,
        "\xE4\x06\x40\xF9\xE3\x03\x15\xAA\xE2\x0E\x40\xF9\xE1\x03\x19\xAA\xE0\x03\x16\xAA\x00\x00\x00\x00\xE3\x03\x00\x2A",
        "xxxxxxxxxxxxxxxxxxxx????xxxx"
    );
    if (match)
    {
        match += 20;
        match = (PBYTE)ARM64_FollowBL((DWORD*)match);
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

extern "C" {

void PatchStartTileDataFurther(HMODULE hModule, BOOL bSMEH)
{
    // ComPtr<ABI::Windows::Internal::ApplicationModel::IPinnableSurfaceFactory> pPinnableSurfaceFactory;
    // PatchStartPinnableSurface(hModule, nullptr /*&pPinnableSurfaceFactory*/);

    // if (bSMEH)
        // pPinnableSurfaceFactory->AddRef(); // Pin in memory so that StartTileData.dll doesn't get unloaded

    PatchUnifiedTilePinUnpinProvider(hModule);
}

} // extern "C"

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

HRESULT(*AppResolver_CAppResolverCacheBuilder__AddUserPinnedShortcutToStartFunc)(void* _this, const CCacheShortcut* a2, const void* a3);
HRESULT AppResolver_CAppResolverCacheBuilder__AddUserPinnedShortcutToStart(void* _this, const CCacheShortcut* a2, const void* a3)
{
    using namespace ABI::WindowsInternal::Shell::UnifiedTile;
    using namespace ABI::WindowsInternal::Shell::UnifiedTile::Private;
    using namespace ABI::WindowsInternal::Shell::UnifiedTile::CuratedTileCollections;

    if (!dwStartShowClassicMode)
        return AppResolver_CAppResolverCacheBuilder__AddUserPinnedShortcutToStartFunc(_this, a2, a3);

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

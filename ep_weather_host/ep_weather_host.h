#ifndef _H_AS_SERVICE_P_H_
#define _H_AS_SERVICE_P_H_
#include "ep_weather.h"
#include "ep_weather_utility.h"
#include "ep_weather_host_h.h"
#include "../ExplorerPatcher/def.h"
#include <windowsx.h>
#include <ShlObj.h>
#include <Shobjidl.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#include <netlistmgr.h>
#include <Iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#include "WebView2.h"
#pragma comment(lib, "uxtheme.lib")
#include <ShellScalingApi.h>
#include <shlwapi.h>

DEFINE_GUID(IID_ITaskbarList,
    0x56FDF342, 0xFD6D, 0x11d0, 0x95, 0x8A, 0x00, 0x60, 0x97, 0xC9, 0xA0, 0x90);

#define EP_WEATHER_NUM_SIGNALS 4

#define EP_WEATHER_TIMER_REQUEST_REPAINT 1
#define EP_WEATHER_TIMER_REQUEST_REPAINT_DELAY 1000
#define EP_WEATHER_TIMER_REQUEST_REFRESH 10
#define EP_WEATHER_TIMER_REQUEST_REFRESH_DELAY 2000
#define EP_WEATHER_TIMER_SCHEDULE_REFRESH 11
#define EP_WEATHER_TIMER_RESIZE_WINDOW 15
#define EP_WEATHER_TIMER_RESIZE_WINDOW_DELAY 150
#define EP_WEATHER_TIMER_EXECUTEDATASCRIPT 20
#define EP_WEATHER_TIMER_EXECUTEDATASCRIPT_DELAY 500

typedef struct _GenericObjectWithThis GenericObjectWithThis;

/* EPWeather */
typedef interface EPWeather
{
    CONST_VTBL IEPWeatherVtbl* lpVtbl;
    unsigned int cbCount;
    HRESULT hrLastError;
    /**/HANDLE hMainThread;//
    /**/HANDLE hInitializeEvent;//
    /*//*/HWND hWnd;//

    INT64 bBrowserBusy; // interlocked
    HWND hNotifyWnd; // interlocked
    LONG64 dwTemperatureUnit; // interlocked
    LONG64 dwUpdateSchedule; // interlocked
    WCHAR wszTerm[MAX_PATH];
    WCHAR wszLanguage[MAX_PATH];
    LONG64 cbx; // interlocked
    LONG64 cby; // interlocked
    LONG64 dwProvider; // interlocked
    LONG64 bIsNavigatingToError; // interlocked
    LONG64 g_darkModeEnabled; // interlocked
    LONG64 dwGeolocationMode;
    LONG64 dwWindowCornerPreference;
    LONG64 dwDevMode;
    LONG64 dwTextDir;
    LONG64 dwIconPack;
    LONG64 dwZoomFactor;

    /**/HANDLE hMutexData;// // protects the following:
    DWORD cbTemperature;
    /*//*/LPCWSTR wszTemperature;//
    DWORD cbUnit;
    /*//*/LPCWSTR wszUnit;//
    DWORD cbCondition;
    /*//*/LPCWSTR wszCondition;//
    DWORD cbImage;
    /*//*/char* pImage;//
    DWORD cbLocation;
    /*//*/LPCWSTR wszLocation;//
    LONG64 dwTextScaleFactor; // interlocked
    /**/HMODULE hUxtheme;//
    /**/HMODULE hShlwapi;//
    /**/HKEY hKCUAccessibility;//
    /**/HKEY hKLMAccessibility;//
    DWORD cntResizeWindow;

    RECT rcBorderThickness; // local variables:
    /*//*/ITaskbarList* pTaskList;//
    /*//*/ICoreWebView2Controller* pCoreWebView2Controller;//
    /*//*/ICoreWebView2* pCoreWebView2;//
    /*//*/GenericObjectWithThis* pCoreWebView2NavigationStartingEventHandler;//
    EventRegistrationToken tkOnNavigationStarting;
    /*//*/GenericObjectWithThis* pCoreWebView2NavigationCompletedEventHandler;//
    EventRegistrationToken tkOnNavigationCompleted;
    /*//*/GenericObjectWithThis* pCoreWebView2PermissionRequestedEventHandler;//
    EventRegistrationToken tkOnPermissionRequested;
    RECT rc;
    LONG64 dpiXInitial;
    LONG64 dpiYInitial;
    FARPROC SHRegGetValueFromHKCUHKLMFunc;
    LONG64 cbGenericObject;

    /**/HANDLE hSignalExitMainThread;//
    /**/HANDLE hSignalKillSwitch;//
    /**/HANDLE hSignalOnAccessibilitySettingsChangedFromHKCU;//
    /**/HANDLE hSignalOnAccessibilitySettingsChangedFromHKLM;//
} EPWeather;

ULONG   STDMETHODCALLTYPE epw_Weather_AddRef(EPWeather* _this);
ULONG   STDMETHODCALLTYPE epw_Weather_Release(EPWeather* _this);
HRESULT STDMETHODCALLTYPE epw_Weather_QueryInterface(EPWeather* _this, REFIID riid, void** ppv);
HRESULT STDMETHODCALLTYPE epw_Weather_About(EPWeather* _this, HWND hWnd);

HRESULT STDMETHODCALLTYPE epw_Weather_Initialize(EPWeather* _this, WCHAR wszName[MAX_PATH], BOOL bAllocConsole, LONG64 dwProvider, LONG64 cbx, LONG64 cby, LONG64 dwTemperatureUnit, LONG64 dwUpdateSchedule, RECT rc, LONG64 dwDarkMode, LONG64 dwGeolocationMode, HWND* hWnd, LONG64 dwZoomFactor, LONG64 dpiXInitial, LONG64 dpiYInitial);

HRESULT STDMETHODCALLTYPE epw_Weather_Show(EPWeather* _this);
HRESULT STDMETHODCALLTYPE epw_Weather_Hide(EPWeather* _this);
HRESULT STDMETHODCALLTYPE epw_Weather_GetWindowHandle(EPWeather* _this, HWND* phWnd);
HRESULT STDMETHODCALLTYPE epw_Weather_IsInitialized(EPWeather* _this, BOOL* bIsInitialized);

HRESULT STDMETHODCALLTYPE epw_Weather_LockData(EPWeather* _this);
HRESULT STDMETHODCALLTYPE epw_Weather_GetDataSizes(EPWeather* _this, LPDWORD pcbTemperature, LPDWORD pcbUnit, LPDWORD pcbCondition, LPDWORD pcbImage);
HRESULT STDMETHODCALLTYPE epw_Weather_GetData(EPWeather* _this, DWORD cbTemperature, LPCWSTR wszTemperature, DWORD cbUnit, LPCWSTR wszUnit, DWORD cbCondition, LPCWSTR wszCondition, DWORD cbImage, char* pImage);
HRESULT STDMETHODCALLTYPE epw_Weather_GetTitle(EPWeather* _this, DWORD cbTitle, LPCWSTR wszTitle, DWORD dwType);
HRESULT STDMETHODCALLTYPE epw_Weather_UnlockData(EPWeather* _this);

HRESULT STDMETHODCALLTYPE epw_Weather_SetNotifyWindow(EPWeather* _this, HWND hWndNotify);
HRESULT STDMETHODCALLTYPE epw_Weather_SetTemperatureUnit(EPWeather* _this, LONG64 dwTemperatureUnit);
HRESULT STDMETHODCALLTYPE epw_Weather_SetUpdateSchedule(EPWeather* _this, LONG64 dwUpdateSchedule);
HRESULT STDMETHODCALLTYPE epw_Weather_SetTerm(EPWeather* _this, DWORD cbTerm, LPCWSTR wszTerm);
HRESULT STDMETHODCALLTYPE epw_Weather_SetLanguage(EPWeather* _this, DWORD cbLanguage, LPCWSTR wszLanguage);
HRESULT STDMETHODCALLTYPE epw_Weather_SetIconSize(EPWeather* _this, LONG64 cbx, LONG64 cby);
HRESULT STDMETHODCALLTYPE epw_Weather_GetIconSize(EPWeather* _this, LONG64* cbx, LONG64* cby);
HRESULT STDMETHODCALLTYPE epw_Weather_SetDarkMode(EPWeather* _this, LONG64 dwDarkMode, LONG64 bRefresh);
HRESULT STDMETHODCALLTYPE epw_Weather_IsDarkMode(EPWeather* _this, LONG64 dwDarkMode, LONG64* bEnabled);
HRESULT STDMETHODCALLTYPE epw_Weather_SetGeolocationMode(EPWeather* _this, LONG64 dwGeolocationMode);
HRESULT STDMETHODCALLTYPE epw_Weather_SetWindowCornerPreference(EPWeather* _this, LONG64 dwWindowCornerPreference);
HRESULT STDMETHODCALLTYPE epw_Weather_SetDevMode(EPWeather* _this, LONG64 dwDevMode, LONG64 bRefresh);
HRESULT STDMETHODCALLTYPE epw_Weather_SetIconPack(EPWeather* _this, LONG64 dwIconPack, LONG64 bRefresh);
HRESULT STDMETHODCALLTYPE epw_Weather_SetZoomFactor(EPWeather* _this, LONG64 dwZoomFactor);
HRESULT STDMETHODCALLTYPE epw_Weather_GetLastUpdateTime(EPWeather* _this, LPSYSTEMTIME lpLastUpdateTime);

static const IEPWeatherVtbl IEPWeather_Vtbl = {
    .QueryInterface = epw_Weather_QueryInterface,
    .AddRef = epw_Weather_AddRef,
    .Release = epw_Weather_Release,
    .About = epw_Weather_About,
    .Initialize = epw_Weather_Initialize,
    .Show = epw_Weather_Show,
    .Hide = epw_Weather_Hide,
    .GetWindowHandle = epw_Weather_GetWindowHandle,
    .LockData = epw_Weather_LockData,
    .GetDataSizes = epw_Weather_GetDataSizes,
    .GetData = epw_Weather_GetData,
    .UnlockData = epw_Weather_UnlockData,
    .SetNotifyWindow = epw_Weather_SetNotifyWindow,
    .IsInitialized = epw_Weather_IsInitialized,
    .GetTitle = epw_Weather_GetTitle,
    .SetTemperatureUnit = epw_Weather_SetTemperatureUnit,
    .SetTerm = epw_Weather_SetTerm,
    .SetLanguage = epw_Weather_SetLanguage,
    .SetIconSize = epw_Weather_SetIconSize,
    .GetIconSize = epw_Weather_GetIconSize,
    .SetUpdateSchedule = epw_Weather_SetUpdateSchedule,
    .SetDarkMode = epw_Weather_SetDarkMode,
    .SetGeolocationMode = epw_Weather_SetGeolocationMode,
    .SetWindowCornerPreference = epw_Weather_SetWindowCornerPreference,
    .SetDevMode = epw_Weather_SetDevMode,
    .SetIconPack = epw_Weather_SetIconPack,
    .SetZoomFactor = epw_Weather_SetZoomFactor,
    .GetLastUpdateTime = epw_Weather_GetLastUpdateTime,
};

static inline DWORD epw_Weather_GetTextScaleFactor(EPWeather* _this) { return InterlockedAdd64(&_this->dwTextScaleFactor, 0); }
static inline DWORD epw_Weather_GetZoomFactor(EPWeather* _this) { return InterlockedAdd64(&_this->dwZoomFactor, 0); }
static inline DWORD epw_Weather_GetStyle(EPWeather* _this) { SetLastError(0); return GetWindowLongW(_this->hWnd, GWL_STYLE); }
static inline DWORD epw_Weather_HasMenuBar(EPWeather* _this) { return 0; }
static inline DWORD epw_Weather_GetExtendedStyle(EPWeather* _this) { SetLastError(0); return GetWindowLongW(_this->hWnd, GWL_EXSTYLE); }
static void epw_Weather_SetTextScaleFactorFromRegistry(EPWeather* _this, HKEY hKey, BOOL bRefresh);

HRESULT STDMETHODCALLTYPE epw_Weather_static_Stub(void* _this);
ULONG   STDMETHODCALLTYPE epw_Weather_static_AddRefRelease(EPWeather* _this);

/* ICoreWebView2EnvironmentOptions */
HRESULT STDMETHODCALLTYPE ICoreWebView2_get_AdditionalBrowserArguments(ICoreWebView2EnvironmentOptions* _this, LPWSTR* value);
HRESULT STDMETHODCALLTYPE ICoreWebView2_get_Language(ICoreWebView2EnvironmentOptions* _this, LPWSTR* value);
HRESULT STDMETHODCALLTYPE ICoreWebView2_get_TargetCompatibleBrowserVersion(ICoreWebView2EnvironmentOptions* _this, LPWSTR* value);
HRESULT STDMETHODCALLTYPE ICoreWebView2_get_AllowSingleSignOnUsingOSPrimaryAccount(ICoreWebView2EnvironmentOptions* _this, BOOL* allow);
HRESULT STDMETHODCALLTYPE ICoreWebView2EnvironmentOptions_QueryInterface(IUnknown* _this, REFIID riid, void** ppv);
static const ICoreWebView2EnvironmentOptionsVtbl EPWeather_ICoreWebView2EnvironmentOptionsVtbl = {
    .QueryInterface = ICoreWebView2EnvironmentOptions_QueryInterface,
    .AddRef = epw_Weather_static_AddRefRelease,
    .Release = epw_Weather_static_AddRefRelease,
    .get_AdditionalBrowserArguments = ICoreWebView2_get_AdditionalBrowserArguments,
    .put_AdditionalBrowserArguments = epw_Weather_static_Stub,
    .get_Language = ICoreWebView2_get_Language,
    .put_Language = epw_Weather_static_Stub,
    .get_TargetCompatibleBrowserVersion = ICoreWebView2_get_TargetCompatibleBrowserVersion,
    .put_TargetCompatibleBrowserVersion = epw_Weather_static_Stub,
    .get_AllowSingleSignOnUsingOSPrimaryAccount = ICoreWebView2_get_AllowSingleSignOnUsingOSPrimaryAccount,
    .put_AllowSingleSignOnUsingOSPrimaryAccount = epw_Weather_static_Stub,
};
static const ICoreWebView2EnvironmentOptions EPWeather_ICoreWebView2EnvironmentOptions = {
    .lpVtbl = &EPWeather_ICoreWebView2EnvironmentOptionsVtbl
};


/* GenericObjectWithThis */
typedef struct _GenericObjectWithThis {
    IUnknownVtbl* lpVtbl;
    void* pInstance;
    LONG64 cbCount;
    EPWeather* _this;
    LPWSTR pName;
} GenericObjectWithThis;
GenericObjectWithThis* GenericObjectWithThis_MakeAndInitialize(IUnknownVtbl* vtbl, EPWeather* _this, const LPWSTR pName);
ULONG   STDMETHODCALLTYPE GenericObjectWithThis_AddRef(GenericObjectWithThis* _this);
ULONG   STDMETHODCALLTYPE GenericObjectWithThis_Release(GenericObjectWithThis* _this);


/* INetworkListManagerEvents */
HRESULT STDMETHODCALLTYPE INetworkListManagerEvents_QueryInterface(GenericObjectWithThis* _this, REFIID riid, void** ppv);
HRESULT STDMETHODCALLTYPE INetworkListManagerEvents_ConnectivityChanged(GenericObjectWithThis* _this2, NLM_CONNECTIVITY newConnectivity);
static const INetworkListManagerEventsVtbl INetworkListManagerEvents_Vtbl = {
    .QueryInterface = INetworkListManagerEvents_QueryInterface,
    .AddRef = GenericObjectWithThis_AddRef,
    .Release = GenericObjectWithThis_Release,
    .ConnectivityChanged = INetworkListManagerEvents_ConnectivityChanged,
};
/* */


/* ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler */
HRESULT STDMETHODCALLTYPE ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler_QueryInterface(GenericObjectWithThis* _this, REFIID riid, void** ppv);
HRESULT STDMETHODCALLTYPE ICoreWebView2_CreateCoreWebView2EnvironmentCompleted(GenericObjectWithThis* _this, HRESULT errorCode, ICoreWebView2Environment* pCoreWebView2Environment);
static const ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl EPWeather_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl = {
    .QueryInterface = ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler_QueryInterface,
    .AddRef = GenericObjectWithThis_AddRef,
    .Release = GenericObjectWithThis_Release,
    .Invoke = ICoreWebView2_CreateCoreWebView2EnvironmentCompleted,
};
/* */


/* ICoreWebView2CreateCoreWebView2ControllerCompletedHandler */
HRESULT STDMETHODCALLTYPE ICoreWebView2CreateCoreWebView2ControllerCompletedHandler_QueryInterface(GenericObjectWithThis* _this, REFIID riid, void** ppv);
HRESULT STDMETHODCALLTYPE ICoreWebView2_CreateCoreWebView2ControllerCompleted(GenericObjectWithThis* _this, HRESULT hr, ICoreWebView2Controller* pCoreWebView2Controller);
static const ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl EPWeather_ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl = {
    .QueryInterface = ICoreWebView2CreateCoreWebView2ControllerCompletedHandler_QueryInterface,
    .AddRef = GenericObjectWithThis_AddRef,
    .Release = GenericObjectWithThis_Release,
    .Invoke = ICoreWebView2_CreateCoreWebView2ControllerCompleted,
};
/* */


/* ICoreWebView2NavigationStartingEventHandler */
HRESULT STDMETHODCALLTYPE ICoreWebView2NavigationStartingEventHandler_QueryInterface(GenericObjectWithThis* _this, REFIID riid, void** ppv);
HRESULT STDMETHODCALLTYPE ICoreWebView2_NavigationStarting(GenericObjectWithThis* _this, ICoreWebView2* pCoreWebView2, ICoreWebView2NavigationStartingEventArgs* pCoreWebView2NavigationStartingEventArgs);
static const ICoreWebView2NavigationStartingEventHandlerVtbl EPWeather_ICoreWebView2NavigationStartingEventHandlerVtbl = {
    .QueryInterface = ICoreWebView2NavigationStartingEventHandler_QueryInterface,
    .AddRef = GenericObjectWithThis_AddRef,
    .Release = GenericObjectWithThis_Release,
    .Invoke = ICoreWebView2_NavigationStarting,
};


/* ICoreWebView2NavigationCompletedEventHandler */
HRESULT STDMETHODCALLTYPE ICoreWebView2NavigationCompletedEventHandler_QueryInterface(GenericObjectWithThis* _this, REFIID riid, void** ppv);
HRESULT STDMETHODCALLTYPE ICoreWebView2_NavigationCompleted(GenericObjectWithThis* _this, ICoreWebView2* pCoreWebView2, ICoreWebView2NavigationCompletedEventArgs* pCoreWebView2NavigationCompletedEventArgs);
static const ICoreWebView2NavigationCompletedEventHandlerVtbl EPWeather_ICoreWebView2NavigationCompletedEventHandlerVtbl = {
    .QueryInterface = ICoreWebView2NavigationCompletedEventHandler_QueryInterface,
    .AddRef = GenericObjectWithThis_AddRef,
    .Release = GenericObjectWithThis_Release,
    .Invoke = ICoreWebView2_NavigationCompleted,
};


/* ICoreWebView2PermissionRequestedEventHandler */
HRESULT STDMETHODCALLTYPE ICoreWebView2PermissionRequestedEventHandler_QueryInterface(GenericObjectWithThis* _this, REFIID riid, void** ppv);
HRESULT STDMETHODCALLTYPE ICoreWebView2_PermissionRequested(GenericObjectWithThis* _this, ICoreWebView2* pCoreWebView2, ICoreWebView2PermissionRequestedEventArgs* pCoreWebView2PermissionRequestedEventArgs);
static const ICoreWebView2PermissionRequestedEventHandlerVtbl EPWeather_ICoreWebView2PermissionRequestedEventHandlerVtbl = {
    .QueryInterface = ICoreWebView2PermissionRequestedEventHandler_QueryInterface,
    .AddRef = GenericObjectWithThis_AddRef,
    .Release = GenericObjectWithThis_Release,
    .Invoke = ICoreWebView2_PermissionRequested,
};


/* ICoreWebView2CallDevToolsProtocolMethodCompletedHandler */
HRESULT STDMETHODCALLTYPE ICoreWebView2CallDevToolsProtocolMethodCompletedHandler_QueryInterface(GenericObjectWithThis* _this, REFIID riid, void** ppv);
HRESULT STDMETHODCALLTYPE ICoreWebView2_CallDevToolsProtocolMethodCompleted(GenericObjectWithThis* _this, HRESULT errorCode, LPCWSTR returnObjectAsJson);
static ICoreWebView2CallDevToolsProtocolMethodCompletedHandlerVtbl EPWeather_ICoreWebView2CallDevToolsProtocolMethodCompletedHandlerVtbl = {
    .QueryInterface = ICoreWebView2CallDevToolsProtocolMethodCompletedHandler_QueryInterface,
    .AddRef = GenericObjectWithThis_AddRef,
    .Release = GenericObjectWithThis_Release,
    .Invoke = ICoreWebView2_CallDevToolsProtocolMethodCompleted
};


/* ICoreWebView2ExecuteScriptCompletedHandler */
HRESULT STDMETHODCALLTYPE ICoreWebView2ExecuteScriptCompletedHandler_QueryInterface(GenericObjectWithThis* _this, REFIID riid, void** ppv);
HRESULT STDMETHODCALLTYPE ICoreWebView2_ExecuteScriptCompleted(GenericObjectWithThis* _this, HRESULT hr, LPCWSTR pResultObjectAsJson);
static const ICoreWebView2ExecuteScriptCompletedHandlerVtbl EPWeather_ICoreWebView2ExecuteScriptCompletedHandlerVtbl = {
    .QueryInterface = ICoreWebView2ExecuteScriptCompletedHandler_QueryInterface,
    .AddRef = GenericObjectWithThis_AddRef,
    .Release = GenericObjectWithThis_Release,
    .Invoke = ICoreWebView2_ExecuteScriptCompleted,
};
#endif

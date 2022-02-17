#ifndef _H_AS_SERVICE_P_H_
#define _H_AS_SERVICE_P_H_
#include "ep_weather.h"
#include "ep_weather_utility.h"
#include "../ep_weather_host_stub/ep_weather_host_h.h"
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

typedef interface EPWeather
{
    CONST_VTBL IEPWeatherVtbl* lpVtbl;
    unsigned int cbCount;
    HRESULT hrLastError;
    HANDLE hMainThread;
    HANDLE hInitializeEvent;
    HWND hWnd;

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

    HANDLE hMutexData; // protects the following:
    DWORD cbTemperature;
    LPCWSTR wszTemperature;
    DWORD cbUnit;
    LPCWSTR wszUnit;
    DWORD cbCondition;
    LPCWSTR wszCondition;
    DWORD cbImage;
    char* pImage;
    DWORD cbLocation;
    LPCWSTR wszLocation;
    LONG64 dwTextScaleFactor; // interlocked
    HMODULE hUxtheme;
    HMODULE hShlwapi;
    HKEY hKCUAccessibility;
    HKEY hKLMAccessibility;
    DWORD cntResizeWindow;

    RECT rcBorderThickness; // local variables:
    ITaskbarList* pTaskList;
    ICoreWebView2Controller* pCoreWebView2Controller;
    ICoreWebView2* pCoreWebView2;
    EventRegistrationToken* tkOnNavigationCompleted;
    EventRegistrationToken* tkOnPermissionRequested;
    RECT rc;

    HANDLE hSignalExitMainThread;
    HANDLE hSignalKillSwitch;
    HANDLE hSignalOnAccessibilitySettingsChangedFromHKCU;
    HANDLE hSignalOnAccessibilitySettingsChangedFromHKLM;
} EPWeather;

ULONG   STDMETHODCALLTYPE epw_Weather_AddRef(EPWeather* _this);
ULONG   STDMETHODCALLTYPE epw_Weather_Release(EPWeather* _this);
HRESULT STDMETHODCALLTYPE epw_Weather_QueryInterface(EPWeather* _this, REFIID riid, void** ppv);
HRESULT STDMETHODCALLTYPE epw_Weather_About(EPWeather* _this, HWND hWnd);

HRESULT STDMETHODCALLTYPE epw_Weather_Initialize(EPWeather* _this, WCHAR wszName[MAX_PATH], BOOL bAllocConsole, LONG64 dwProvider, LONG64 cbx, LONG64 cby, LONG64 dwTemperatureUnit, LONG64 dwUpdateSchedule, RECT rc, LONG64 dwDarkMode, LONG64 dwGeolocationMode, HWND* hWnd);

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
};

static inline DWORD epw_Weather_GetTextScaleFactor(EPWeather* _this) { return InterlockedAdd64(&_this->dwTextScaleFactor, 0); }
static void epw_Weather_SetTextScaleFactorFromRegistry(EPWeather* _this, HKEY hKey, BOOL bRefresh);

HRESULT STDMETHODCALLTYPE epw_Weather_static_Stub(void* _this);
ULONG   STDMETHODCALLTYPE epw_Weather_static_AddRefRelease(EPWeather* _this);
HRESULT STDMETHODCALLTYPE epw_Weather_static_QueryInterface(EPWeather* _this, REFIID riid, void** ppv);

HRESULT STDMETHODCALLTYPE ICoreWebView2_CreateCoreWebView2EnvironmentCompleted(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* _this, HRESULT errorCode, ICoreWebView2Environment* pCoreWebView2Environment);
HRESULT STDMETHODCALLTYPE ICoreWebView2_CreateCoreWebView2ControllerCompleted(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* _this, HRESULT hr, ICoreWebView2Controller* pCoreWebView2Controller);
HRESULT STDMETHODCALLTYPE ICoreWebView2_NavigationCompleted(ICoreWebView2NavigationCompletedEventHandler* _this, ICoreWebView2* pCoreWebView2, ICoreWebView2NavigationCompletedEventArgs* pCoreWebView2NavigationCompletedEventArgs);
HRESULT STDMETHODCALLTYPE ICoreWebView2_ExecuteScriptCompleted(ICoreWebView2ExecuteScriptCompletedHandler* _this, HRESULT hr, LPCWSTR pResultObjectAsJson);
HRESULT STDMETHODCALLTYPE ICoreWebView2_get_AdditionalBrowserArguments(ICoreWebView2EnvironmentOptions* _this, LPWSTR* value);
HRESULT STDMETHODCALLTYPE ICoreWebView2_get_Language(ICoreWebView2EnvironmentOptions* _this, LPWSTR* value);
HRESULT STDMETHODCALLTYPE ICoreWebView2_get_TargetCompatibleBrowserVersion(ICoreWebView2EnvironmentOptions* _this, LPWSTR* value);
HRESULT STDMETHODCALLTYPE ICoreWebView2_get_AllowSingleSignOnUsingOSPrimaryAccount(ICoreWebView2EnvironmentOptions* _this, BOOL* allow);

HRESULT STDMETHODCALLTYPE ICoreWebView2_CallDevToolsProtocolMethodCompleted(ICoreWebView2CallDevToolsProtocolMethodCompletedHandler* _this, HRESULT errorCode, LPCWSTR returnObjectAsJson);

HRESULT STDMETHODCALLTYPE ICoreWebView2_PermissionRequested(ICoreWebView2PermissionRequestedEventHandler* _this2, ICoreWebView2* pCoreWebView2, ICoreWebView2PermissionRequestedEventArgs* pCoreWebView2PermissionRequestedEventArgs);

static const ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl EPWeather_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl = {
    .QueryInterface = epw_Weather_static_QueryInterface,
    .AddRef = epw_Weather_static_AddRefRelease,
    .Release = epw_Weather_static_AddRefRelease,
    .Invoke = ICoreWebView2_CreateCoreWebView2EnvironmentCompleted,
};

static const ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler EPWeather_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler = {
    .lpVtbl = &EPWeather_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl
};

static const ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl EPWeather_ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl = {
    .QueryInterface = epw_Weather_static_QueryInterface,
    .AddRef = epw_Weather_static_AddRefRelease,
    .Release = epw_Weather_static_AddRefRelease,
    .Invoke = ICoreWebView2_CreateCoreWebView2ControllerCompleted,
};

static const ICoreWebView2CreateCoreWebView2ControllerCompletedHandler EPWeather_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler = {
    .lpVtbl = &EPWeather_ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl
};

static const ICoreWebView2NavigationCompletedEventHandlerVtbl EPWeather_ICoreWebView2NavigationCompletedEventHandlerVtbl = {
    .QueryInterface = epw_Weather_static_QueryInterface,
    .AddRef = epw_Weather_static_AddRefRelease,
    .Release = epw_Weather_static_AddRefRelease,
    .Invoke = ICoreWebView2_NavigationCompleted,
};

static const ICoreWebView2NavigationCompletedEventHandler EPWeather_ICoreWebView2NavigationCompletedEventHandler = {
    .lpVtbl = &EPWeather_ICoreWebView2NavigationCompletedEventHandlerVtbl
};

static const ICoreWebView2ExecuteScriptCompletedHandlerVtbl EPWeather_ICoreWebView2ExecuteScriptCompletedHandlerVtbl = {
    .QueryInterface = epw_Weather_static_QueryInterface,
    .AddRef = epw_Weather_static_AddRefRelease,
    .Release = epw_Weather_static_AddRefRelease,
    .Invoke = ICoreWebView2_ExecuteScriptCompleted,
};

static const ICoreWebView2ExecuteScriptCompletedHandler EPWeather_ICoreWebView2ExecuteScriptCompletedHandler = {
    .lpVtbl = &EPWeather_ICoreWebView2ExecuteScriptCompletedHandlerVtbl
};

static const ICoreWebView2EnvironmentOptionsVtbl EPWeather_ICoreWebView2EnvironmentOptionsVtbl = {
    .QueryInterface = epw_Weather_static_QueryInterface,
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

static ICoreWebView2CallDevToolsProtocolMethodCompletedHandlerVtbl EPWeather_ICoreWebView2CallDevToolsProtocolMethodCompletedHandlerVtbl = {
    .QueryInterface = epw_Weather_static_QueryInterface,
    .AddRef = epw_Weather_static_AddRefRelease,
    .Release = epw_Weather_static_AddRefRelease,
    .Invoke = ICoreWebView2_CallDevToolsProtocolMethodCompleted
};

static const ICoreWebView2CallDevToolsProtocolMethodCompletedHandler EPWeather_ICoreWebView2CallDevToolsProtocolMethodCompletedHandler = {
    .lpVtbl = &EPWeather_ICoreWebView2CallDevToolsProtocolMethodCompletedHandlerVtbl
};

static const ICoreWebView2PermissionRequestedEventHandlerVtbl EPWeather_ICoreWebView2PermissionRequestedEventHandlerVtbl = {
    .QueryInterface = epw_Weather_static_QueryInterface,
    .AddRef = epw_Weather_static_AddRefRelease,
    .Release = epw_Weather_static_AddRefRelease,
    .Invoke = ICoreWebView2_PermissionRequested,
};

static const ICoreWebView2PermissionRequestedEventHandler EPWeather_ICoreWebView2PermissionRequestedEventHandler = {
    .lpVtbl = &EPWeather_ICoreWebView2PermissionRequestedEventHandlerVtbl
};

HRESULT STDMETHODCALLTYPE INetworkListManagerEvents_QueryInterface(void* _this, REFIID riid, void** ppv);
ULONG   STDMETHODCALLTYPE INetworkListManagerEvents_AddRefRelease(void* _this);
HRESULT STDMETHODCALLTYPE INetworkListManagerEvents_ConnectivityChanged(void* _this, NLM_CONNECTIVITY newConnectivity);

static const INetworkListManagerEventsVtbl INetworkListManagerEvents_Vtbl = {
    .QueryInterface = INetworkListManagerEvents_QueryInterface,
    .AddRef = INetworkListManagerEvents_AddRefRelease,
    .Release = INetworkListManagerEvents_AddRefRelease,
    .ConnectivityChanged = INetworkListManagerEvents_ConnectivityChanged,
};

static const INetworkListManagerEvents INetworkListManagerEvents_Instance = {
    .lpVtbl = &INetworkListManagerEvents_Vtbl,
};
#endif

#include "utility.h"

#pragma region "Enable old taskbar"
/***
Our target is in `CTray::Init()`. It constructs either the Windows 11 or the Windows 10 taskbar based on the result of
`winrt::WindowsUdk::ApplicationModel::AppExtensions::XamlExtensions::IsExtensionAvailable()`. We can to make the last
argument of that function be set to false, so that we'll get the Windows 10 taskbar instead of the Windows 11 one that
gets constructed through `CTray::InitializeTrayUIComponent()`.

Alternatively, we can modify the behavior of `CTray::InitializeTrayUIComponent`. It contains the code to call
`TrayUI_CreateInstance()` that resides in `Taskbar.dll` (checked through HKLM\SOFTWARE\Classes\CLSID\<the CLSID>) which
is a copy of the Windows 10 taskbar code but modified over the time to support the Windows 11 taskbar. We see that it
calls `CoCreateInstance` to get an `ITrayUIComponent` interface to an instance of `TrayUIComponent`. We hook that
function to make it return our own custom `ITrayUIComponent` instance. Our `ITrayUIComponent::InitializeWithTray()`
function calls `TrayUI_CreateInstance()` of `explorer.exe` that is also called when the last argument of
`IsExtensionAvailable()` after the call is false.

This way, we can get the Windows 10 taskbar which resides in explorer.exe without hooking LoadLibraryExW() in order to
perform our initial method which has been known to be inconsistent on some systems. (Thanks feature flags!)
***/

static ULONG STDMETHODCALLTYPE nimplAddRefRelease(IUnknown* This)
{
    return 1;
}

static HRESULT STDMETHODCALLTYPE ITrayUIComponent_QueryInterface(ITrayUIComponent* This, REFIID riid, void** ppvObject)
{
    // Should never be called
    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE ITrayUIComponent_InitializeWithTray(ITrayUIComponent* This, ITrayUIHost* host, ITrayUI** result)
{
    return explorer_TrayUI_CreateInstanceFunc(host, &IID_ITrayUI, (void**)result);
}

static const ITrayUIComponentVtbl instanceof_ITrayUIComponentVtbl = {
    .QueryInterface = ITrayUIComponent_QueryInterface,
    .AddRef = nimplAddRefRelease,
    .Release = nimplAddRefRelease,
    .InitializeWithTray = ITrayUIComponent_InitializeWithTray
};
const ITrayUIComponent instanceof_ITrayUIComponent = { &instanceof_ITrayUIComponentVtbl };
#pragma endregion

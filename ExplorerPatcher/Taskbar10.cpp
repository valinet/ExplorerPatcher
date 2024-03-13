#include "utility.h"

#include <dcomptypes.h>

#include <wrl/implements.h>
#include <wrl/wrappers/corewrappers.h>
#include <wil/result_macros.h>

extern "C" DWORD (*CImmersiveColor_GetColorFunc)(int colorType);

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

MIDL_INTERFACE("27775f88-01d3-46ec-a1c1-64b4c09b211b")
ITrayUIComponent : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE InitializeWithTray(ITrayUIHost* host, ITrayUI** result) = 0;
};

class EPTrayUIComponent : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, ITrayUIComponent>
{
public:
    STDMETHODIMP InitializeWithTray(ITrayUIHost* host, ITrayUI** result) override
    {
        RETURN_IF_FAILED(explorer_TrayUI_CreateInstanceFunc(host, IID_ITrayUI, (void**)result));
        return S_OK;
    }
};

extern "C" HRESULT EPTrayUIComponent_CreateInstance(REFIID riid, void** ppvObject)
{
    Microsoft::WRL::ComPtr<EPTrayUIComponent> instance;
    RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<EPTrayUIComponent>(&instance));
    RETURN_HR(instance.CopyTo(riid, ppvObject));
}
#pragma endregion

#pragma region "Restore acrylic background"
typedef enum WINDOWCOMPOSITIONATTRIB
{
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_HOLOGRAPHIC = 23,
    WCA_EXCLUDED_FROM_DDA = 24,
    WCA_PASSIVEUPDATEMODE = 25,
    WCA_USEDARKMODECOLORS = 26,
    WCA_CORNER_STYLE = 27,
    WCA_PART_COLOR = 28,
    WCA_DISABLE_MOVESIZE_FEEDBACK = 29,
    WCA_SYSTEMBACKDROP_TYPE = 30,
    WCA_SET_TAGGED_WINDOW_RECT = 31,
    WCA_CLEAR_TAGGED_WINDOW_RECT = 32,
    WCA_LAST = 33,
} WINDOWCOMPOSITIONATTRIB;

typedef struct tagWINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    void* pvData;
    unsigned int cbData;
} WINDOWCOMPOSITIONATTRIBDATA;

typedef enum ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5,
    ACCENT_INVALID_STATE = 6,
} ACCENT_STATE;

typedef struct ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    unsigned int AccentFlags;
    unsigned long GradientColor;
    long AnimationId;
} ACCENT_POLICY;

namespace ABI::WindowsUdk::UI::Themes
{
    enum class VisualTheme
    {
        Dark = 0,
        Light = 1,
        HighContrastBlack = 2,
        HighContrastWhite = 3,
    };

    MIDL_INTERFACE("8f0a6c35-72ca-5f4a-a5fb-1a731ec8b514")
    ISystemVisualThemeStatics : IInspectable
    {
        virtual HRESULT STDMETHODCALLTYPE get_Current(VisualTheme* value) = 0;
        virtual HRESULT STDMETHODCALLTYPE add_Changed(void* handler, EventRegistrationToken* token) = 0;
        virtual HRESULT STDMETHODCALLTYPE remove_Changed(EventRegistrationToken token) = 0;
    };
}

struct TaskbarTheme
{
    bool bColorPrevalence;
    bool bEnableTransparency;
    ABI::WindowsUdk::UI::Themes::VisualTheme visualTheme;

    bool IsHighContrast() const
    {
        using namespace ABI::WindowsUdk::UI::Themes;
        return visualTheme == VisualTheme::HighContrastBlack || visualTheme == VisualTheme::HighContrastWhite;
    }

    bool IsDark() const
    {
        using namespace ABI::WindowsUdk::UI::Themes;
        return visualTheme == VisualTheme::Dark || visualTheme == VisualTheme::HighContrastBlack;
    }
};

struct struct_b
{
    int a;
    int b;
    int c;
    int d;
};

typedef HRESULT (*NtDCompositionGetFrameStatistics_t)(DCOMPOSITION_FRAME_STATISTICS*, struct_b*);

inline HRESULT NtDCompositionGetFrameStatistics(DCOMPOSITION_FRAME_STATISTICS* a, struct_b* b)
{
    static NtDCompositionGetFrameStatistics_t f = nullptr;
    if (!f)
    {
        HMODULE h = GetModuleHandleW(L"dcomp.dll");
        if (h)
            f = (NtDCompositionGetFrameStatistics_t)GetProcAddress(h, MAKEINTRESOURCEA(1046));
    }
    return f ? f(a, b) : E_NOTIMPL;
}

bool ShouldApplyBlur()
{
    DCOMPOSITION_FRAME_STATISTICS v7;
    struct_b v6;
    return SUCCEEDED(NtDCompositionGetFrameStatistics(&v7, &v6)) && v6.d && !v6.c;
}

TaskbarTheme GetTaskbarTheme()
{
    TaskbarTheme rv;
    // rv.visualTheme = winrt::WindowsUdk::UI::Themes::SystemVisualTheme::Current();

    rv.visualTheme = ABI::WindowsUdk::UI::Themes::VisualTheme::Light;
    Microsoft::WRL::ComPtr<ABI::WindowsUdk::UI::Themes::ISystemVisualThemeStatics> systemVisualTheme;
    HRESULT hr = RoGetActivationFactory(
        Microsoft::WRL::Wrappers::HStringReference(L"WindowsUdk.UI.Themes.SystemVisualTheme").Get(),
        IID_PPV_ARGS(&systemVisualTheme)
    );
    if (SUCCEEDED_LOG(hr))
    {
        ABI::WindowsUdk::UI::Themes::VisualTheme theme;
        if (SUCCEEDED_LOG(systemVisualTheme->get_Current(&theme)))
        {
            rv.visualTheme = theme;
        }
    }

    DWORD bColorPrevalence = 0;
    rv.bColorPrevalence =
        SUCCEEDED(SHRegGetDWORD(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            L"ColorPrevalence",
            &bColorPrevalence
        )) && bColorPrevalence;

    bool bApplyBlur = ShouldApplyBlur();
    DWORD bEnableTransparency;
    rv.bEnableTransparency = !rv.IsHighContrast() && bApplyBlur
        && SUCCEEDED(SHRegGetDWORD(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            L"EnableTransparency",
            &bEnableTransparency
        )) && bEnableTransparency;

    return rv;
}

DWORD GetTaskbarColor()
{
    TaskbarTheme tt = GetTaskbarTheme();

    if (tt.IsHighContrast())
        return GetSysColor(COLOR_WINDOW);

    if (tt.bColorPrevalence && CImmersiveColor_GetColorFunc)
    {
        DWORD result = CImmersiveColor_GetColorFunc(tt.IsDark() ? 6 /*IMCLR_SystemAccentDark2*/ : 2 /*IMCLR_SystemAccentLight2*/);
        if (tt.bEnableTransparency)
            return (result & 0xFFFFFF) | 0xCC000000;
        return result;
    }

    if (tt.IsDark())
        return tt.bEnableTransparency ? 0x80202020 : 0xFF202020;

    return tt.bEnableTransparency ? 0xF3F3F3 : 0xFFF3F3F3;
}

extern "C" void UpdateWindowAccentProperties_PatchAttribData(WINDOWCOMPOSITIONATTRIBDATA* pAttrData)
{
    ACCENT_POLICY* pAccentPolicy = (ACCENT_POLICY*)pAttrData->pvData;
    if (false) // STTest makes it like this:
    {
        pAccentPolicy->AccentState = ACCENT_ENABLE_TRANSPARENTGRADIENT;
        pAccentPolicy->GradientColor = 0;
        pAccentPolicy->AnimationId = 0;
    }
    else
    {
        pAccentPolicy->AccentState = GetTaskbarTheme().bEnableTransparency ? ACCENT_ENABLE_ACRYLICBLURBEHIND : ACCENT_ENABLE_GRADIENT;
        pAccentPolicy->GradientColor = GetTaskbarColor();
        pAccentPolicy->AnimationId = 0;
    }

    pAccentPolicy->AccentFlags = 0x1 | 0x2 | 0x10;
}
#pragma endregion

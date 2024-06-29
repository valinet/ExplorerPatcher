#pragma once

#include <Windows.h>

enum IMMERSIVE_COLOR_TYPE
{
    // Defining only used ones
    IMCLR_SystemAccentLight2 = 2,
    IMCLR_SystemAccentDark2 = 6
};

struct IMMERSIVE_COLOR_PREFERENCE
{
    DWORD crStartColor;
    DWORD crAccentColor;
};

enum IMMERSIVE_HC_CACHE_MODE
{
    IHCM_USE_CACHED_VALUE = 0,
    IHCM_REFRESH = 1
};

typedef bool (*RefreshImmersiveColorPolicyState_t)(); // 104
inline bool RefreshImmersiveColorPolicyState()
{
    static RefreshImmersiveColorPolicyState_t fn;
    if (!fn)
    {
        HMODULE h = GetModuleHandleW(L"uxtheme.dll");
        if (h)
            fn = (RefreshImmersiveColorPolicyState_t)GetProcAddress(h, MAKEINTRESOURCEA(104));
    }
    return fn ? fn() : false;
}

typedef bool (*GetIsImmersiveColorUsingHighContrast_t)(IMMERSIVE_HC_CACHE_MODE); // 106
inline bool GetIsImmersiveColorUsingHighContrast(IMMERSIVE_HC_CACHE_MODE mode)
{
    static GetIsImmersiveColorUsingHighContrast_t fn;
    if (!fn)
    {
        HMODULE h = GetModuleHandleW(L"uxtheme.dll");
        if (h)
            fn = (GetIsImmersiveColorUsingHighContrast_t)GetProcAddress(h, MAKEINTRESOURCEA(106));
    }
    return fn ? fn(mode) : false;
}

typedef HRESULT (*GetUserColorPreference_t)(IMMERSIVE_COLOR_PREFERENCE*, bool); // 120
inline HRESULT GetUserColorPreference(IMMERSIVE_COLOR_PREFERENCE* pcpColorPreference, bool fForceReload)
{
    static GetUserColorPreference_t fn;
    if (!fn)
    {
        HMODULE h = GetModuleHandleW(L"uxtheme.dll");
        if (h)
            fn = (GetUserColorPreference_t)GetProcAddress(h, MAKEINTRESOURCEA(120));
    }
    return fn ? fn(pcpColorPreference, fForceReload) : E_FAIL;
}

typedef DWORD (*GetColorFromPreference_t)(const IMMERSIVE_COLOR_PREFERENCE*, IMMERSIVE_COLOR_TYPE, bool, IMMERSIVE_HC_CACHE_MODE); // 121
inline DWORD GetColorFromPreference(const IMMERSIVE_COLOR_PREFERENCE* cpcpPreference, IMMERSIVE_COLOR_TYPE colorType, bool fNoHighContrast, IMMERSIVE_HC_CACHE_MODE mode)
{
    static GetColorFromPreference_t fn;
    if (!fn)
    {
        HMODULE h = GetModuleHandleW(L"uxtheme.dll");
        if (h)
            fn = (GetColorFromPreference_t)GetProcAddress(h, MAKEINTRESOURCEA(121));
    }
    return fn ? fn(cpcpPreference, colorType, fNoHighContrast, mode) : 0;
}

class CImmersiveColor
{
public:
    static DWORD GetColor(IMMERSIVE_COLOR_TYPE colorType)
    {
        IMMERSIVE_COLOR_PREFERENCE icp;
        icp.crStartColor = 0;
        icp.crAccentColor = 0;
        GetUserColorPreference(&icp, false/*, true*/);
        return GetColorFromPreference(&icp, colorType, false, IHCM_REFRESH);
    }

    static bool IsColorSchemeChangeMessage(UINT uMsg, LPARAM lParam)
    {
        bool bRet = false;
        if (uMsg == WM_SETTINGCHANGE && lParam && CompareStringOrdinal((WCHAR*)lParam, -1, L"ImmersiveColorSet", -1, TRUE) == CSTR_EQUAL)
        {
            RefreshImmersiveColorPolicyState();
            bRet = true;
        }
        GetIsImmersiveColorUsingHighContrast(IHCM_REFRESH);
        return bRet;
    }
};

class CImmersiveColorImpl
{
public:
    static HRESULT GetColorPreferenceImpl(IMMERSIVE_COLOR_PREFERENCE* pcpPreference, bool fForceReload, bool fUpdateCached)
    {
        return GetUserColorPreference(pcpPreference, fForceReload);
    }
};

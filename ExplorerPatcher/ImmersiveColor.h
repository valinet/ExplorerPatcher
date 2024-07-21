#pragma once

#include <Windows.h>

#include "utility.h"

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

#ifndef _H_EP_WEATHER_UTILITY_H_
#define _H_EP_WEATHER_UTILITY_H_
#include <Windows.h>
#include <stdint.h>
#include "../ExplorerPatcher/queryversion.h"
#include "../ExplorerPatcher/osutility.h"

extern void(*RefreshImmersiveColorPolicyState)();
extern void(*SetPreferredAppMode)(INT64 bAllowDark);
extern void(*AllowDarkModeForWindow)(HWND hWnd, INT64 bAllowDark);
extern BOOL(*ShouldAppsUseDarkMode)();
extern BOOL(*ShouldSystemUseDarkMode)();

inline BOOL IsColorSchemeChangeMessage(LPARAM lParam)
{
    BOOL is = FALSE;
    if (lParam && CompareStringOrdinal(lParam, -1, L"ImmersiveColorSet", -1, TRUE) == CSTR_EQUAL)
    {
        is = TRUE;
    }
    return is;
}

inline BOOL IsHighContrast()
{
    HIGHCONTRASTW highContrast;
    ZeroMemory(&highContrast, sizeof(HIGHCONTRASTW));
    highContrast.cbSize = sizeof(highContrast);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, FALSE))
        return highContrast.dwFlags & HCF_HIGHCONTRASTON;
    return FALSE;
}
#endif
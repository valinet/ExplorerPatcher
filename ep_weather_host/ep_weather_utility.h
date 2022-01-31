#ifndef _H_EP_WEATHER_UTILITY_H_
#define _H_EP_WEATHER_UTILITY_H_
#include <Windows.h>
#include <stdint.h>

#ifndef NTDDI_WIN10_CO
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
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
inline void QueryVersionInfo(HMODULE hModule, WORD Resource, DWORD* dwLeftMost, DWORD* dwSecondLeft, DWORD* dwSecondRight, DWORD* dwRightMost)
{
    HRSRC hResInfo;
    DWORD dwSize;
    HGLOBAL hResData;
    LPVOID pRes, pResCopy;
    UINT uLen;
    VS_FIXEDFILEINFO* lpFfi;

    hResInfo = FindResource(hModule, MAKEINTRESOURCE(Resource), RT_VERSION);
    dwSize = SizeofResource(hModule, hResInfo);
    hResData = LoadResource(hModule, hResInfo);
    pRes = LockResource(hResData);
    pResCopy = LocalAlloc(LMEM_FIXED, dwSize);
    CopyMemory(pResCopy, pRes, dwSize);
    FreeResource(hResData);

    VerQueryValue(pResCopy, TEXT("\\"), (LPVOID*)&lpFfi, &uLen);

    DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
    DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;

    *dwLeftMost = HIWORD(dwFileVersionMS);
    *dwSecondLeft = LOWORD(dwFileVersionMS);
    *dwSecondRight = HIWORD(dwFileVersionLS);
    *dwRightMost = LOWORD(dwFileVersionLS);

    LocalFree(pResCopy);
}
#endif
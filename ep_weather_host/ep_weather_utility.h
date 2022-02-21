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

#define OSVERSION_INVALID 0xffffffff

typedef LONG NTSTATUS, * PNTSTATUS;
#define STATUS_SUCCESS (0x00000000)

typedef NTSTATUS(WINAPI* VnRtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

// https://stackoverflow.com/questions/36543301/detecting-windows-10-version/36543774#36543774
inline BOOL GetOSVersion(PRTL_OSVERSIONINFOW lpRovi)
{
	HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
	if (hMod != NULL)
	{
		VnRtlGetVersionPtr fxPtr = (VnRtlGetVersionPtr)GetProcAddress(
			hMod,
			"RtlGetVersion"
		);
		if (fxPtr != NULL)
		{
			lpRovi->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
			if (STATUS_SUCCESS == fxPtr(lpRovi))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

// https://stackoverflow.com/questions/47926094/detecting-windows-10-os-build-minor-version
inline DWORD32 GetUBR()
{
	DWORD32 ubr = 0, ubr_size = sizeof(DWORD32);
	HKEY hKey;
	LONG lRes = RegOpenKeyExW(
		HKEY_LOCAL_MACHINE,
		wcschr(
			wcschr(
				wcschr(
					UNIFIEDBUILDREVISION_KEY,
					'\\'
				) + 1,
				'\\'
			) + 1,
			'\\'
		) + 1,
		0,
		KEY_READ,
		&hKey
	);
	if (lRes == ERROR_SUCCESS)
	{
		RegQueryValueExW(
			hKey,
			UNIFIEDBUILDREVISION_VALUE,
			0,
			NULL,
			&ubr,
			&ubr_size
		);
	}
}

inline DWORD32 GetOSVersionAndUBR(PRTL_OSVERSIONINFOW lpRovi)
{
	if (!GetOSVersion(lpRovi))
	{
		return OSVERSION_INVALID;
	}
	return GetUBR();
}

inline BOOL IsWindows11()
{
	RTL_OSVERSIONINFOW rovi;
	if (GetOSVersion(&rovi) && rovi.dwBuildNumber >= 21996)
	{
		return TRUE;
	}
	return FALSE;
}
#endif
#include "ep_weather.h"
#include "ep_weather_factory.h"
#include "ep_weather_host.h"

HMODULE epw_hModule;
DWORD epw_OutstandingObjects = 0;
DWORD epw_LockCount = 0;

void(*RefreshImmersiveColorPolicyState)();
void(*SetPreferredAppMode)(INT64 bAllowDark);
void(*AllowDarkModeForWindow)(HWND hWnd, INT64 bAllowDark);
BOOL(*ShouldAppsUseDarkMode)();
BOOL(*ShouldSystemUseDarkMode)();

#ifdef _WIN64
#pragma comment(linker, "/export:DllRegisterServer=_DllRegisterServer")
#else
#pragma comment(linker, "/export:DllRegisterServer=__DllRegisterServer@0")
#endif
HRESULT WINAPI _DllRegisterServer()
{
    DWORD dwLastError = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    wchar_t wszFilename[MAX_PATH];
    wchar_t wszInstallPath[MAX_PATH];

    if (!dwLastError)
    {
        if (!GetModuleFileNameW(epw_hModule, wszFilename, MAX_PATH))
        {
            dwLastError = GetLastError();
        }
    }
    if (!dwLastError)
    {
        dwLastError = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            _T("SOFTWARE\\Classes\\CLSID\\") _T(CLSID_EPWeather_TEXT),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_WOW64_64KEY,
            NULL,
            &hKey,
            NULL
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegSetValueExW(
                hKey,
                NULL,
                0,
                REG_SZ,
                _T(CLSID_EPWeather_Name),
                29 * sizeof(wchar_t)
            );
            dwLastError = RegSetValueExW(
                hKey,
                L"AppID",
                0,
                REG_SZ,
                _T(CLSID_EPWeather_TEXT),
                39 * sizeof(wchar_t)
            );
            RegCloseKey(hKey);
        }
        dwLastError = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            _T("SOFTWARE\\Classes\\CLSID\\") _T(CLSID_EPWeather_TEXT) _T("\\InProcServer32"),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_WOW64_64KEY,
            NULL,
            &hKey,
            NULL
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegSetValueExW(
                hKey,
                NULL,
                0,
                REG_SZ,
                wszFilename,
                (wcslen(wszFilename) + 1) * sizeof(wchar_t)
            );
            dwLastError = RegSetValueExW(
                hKey,
                L"ThreadingModel",
                0,
                REG_SZ,
                L"Apartment",
                10 * sizeof(wchar_t)
            );
            RegCloseKey(hKey);
        }
        dwLastError = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            _T("SOFTWARE\\Classes\\AppID\\") _T(CLSID_EPWeather_TEXT),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_WOW64_64KEY,
            NULL,
            &hKey,
            NULL
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegSetValueExW(
                hKey,
                NULL,
                0,
                REG_SZ,
                _T(CLSID_EPWeather_Name),
                29 * sizeof(wchar_t)
            );
            dwLastError = RegSetValueExW(
                hKey,
                L"DllSurrogate",
                0,
                REG_SZ,
                L"",
                1 * sizeof(wchar_t)
            );
            RegCloseKey(hKey);
        }
        dwLastError = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            _T("SOFTWARE\\Classes\\AppID\\") _T(CLSID_EPWeather_TEXT),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_WOW64_64KEY,
            NULL,
            &hKey,
            NULL
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegSetValueExW(
                hKey,
                NULL,
                0,
                REG_SZ,
                _T(CLSID_EPWeather_Name),
                29 * sizeof(wchar_t)
            );
            dwLastError = RegSetValueExW(
                hKey,
                L"DllSurrogate",
                0,
                REG_SZ,
                L"",
                1 * sizeof(wchar_t)
            );
            RegCloseKey(hKey);
        }
    }

    return dwLastError == 0 ? (NOERROR) : (HRESULT_FROM_WIN32(dwLastError));
}

#ifdef _WIN64
#pragma comment(linker, "/export:DllUnregisterServer=_DllUnregisterServer")
#else
#pragma comment(linker, "/export:DllUnregisterServer=__DllUnregisterServer@0")
#endif
HRESULT WINAPI _DllUnregisterServer()
{
    DWORD dwLastError = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    wchar_t wszFilename[MAX_PATH];

    if (!dwLastError)
    {
        if (!GetModuleFileNameW(epw_hModule, wszFilename, MAX_PATH))
        {
            dwLastError = GetLastError();
        }
    }
    if (!dwLastError)
    {
        dwLastError = RegOpenKeyW(
            HKEY_LOCAL_MACHINE,
            _T("SOFTWARE\\Classes\\CLSID\\") _T(CLSID_EPWeather_TEXT),
            &hKey
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegDeleteTreeW(
                hKey,
                0
            );
            RegCloseKey(hKey);
            if (!dwLastError)
            {
                RegDeleteTreeW(
                    HKEY_LOCAL_MACHINE,
                    _T("SOFTWARE\\Classes\\CLSID\\") _T(CLSID_EPWeather_TEXT)
                );
            }
        }
        dwLastError = RegOpenKeyW(
            HKEY_LOCAL_MACHINE,
            _T("SOFTWARE\\Classes\\AppID\\") _T(CLSID_EPWeather_TEXT),
            &hKey
        );
        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
        {
            hKey = NULL;
        }
        if (hKey)
        {
            dwLastError = RegDeleteTreeW(
                hKey,
                0
            );
            RegCloseKey(hKey);
            if (!dwLastError)
            {
                RegDeleteTreeW(
                    HKEY_LOCAL_MACHINE,
                    _T("SOFTWARE\\Classes\\AppID\\") _T(CLSID_EPWeather_TEXT)
                );
            }
        }
    }

    return dwLastError == 0 ? (NOERROR) : (HRESULT_FROM_WIN32(dwLastError));
}

#ifdef _WIN64
#pragma comment(linker, "/export:DllCanUnloadNow=_DllCanUnloadNow")
#else
#pragma comment(linker, "/export:DllCanUnloadNow=__DllCanUnloadNow@0")
#endif
HRESULT WINAPI _DllCanUnloadNow()
{
    return((epw_OutstandingObjects | epw_LockCount) ? S_FALSE : S_OK);
}

#ifdef _WIN64
#pragma comment(linker, "/export:DllGetClassObject=_DllGetClassObject")
#else
#pragma comment(linker, "/export:DllGetClassObject=__DllGetClassObject@12")
#endif
HRESULT WINAPI _DllGetClassObject(
    REFCLSID objGuid,
    REFIID   factoryGuid,
    LPVOID* factoryHandle
)
{
    HRESULT  hr;
    if (IsEqualCLSID(objGuid, &CLSID_EPWeather))
    {
        hr = ClassFactory->lpVtbl->QueryInterface(
            ClassFactory,
            factoryGuid,
            factoryHandle
        );
    }
    else
    {
        *factoryHandle = 0;
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }

    return(hr);
}

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        epw_hModule = hinstDLL;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

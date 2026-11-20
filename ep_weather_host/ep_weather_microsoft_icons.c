#include "ep_weather_microsoft_icons.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wincrypt.h>
#include <winhttp.h>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "winhttp.lib")

#define EP_WEATHER_ICON_MAX_BYTES (32 * 1024)
#define EP_WEATHER_ICON_WAIT_MS 5000
#define EP_WEATHER_ICON_RETRY_MS 60000

// The same public replacement artwork used by the Microsoft pack.
static const struct
{
    LPCWSTR name;
    LPCWSTR path;
} epWeatherIconSources[] = {
    {L"clear-night", L"/6503598/156949451-269d02a3-08cb-4237-9789-f1e60fdc723d.png"},
    {L"cloudy", L"/6503598/156949452-f347fe27-5005-48f2-9c9a-899bb7b8825e.png"},
    {L"fog", L"/6503598/156949454-81d5d47d-1f33-4859-a112-5a64ceb549a1.png"},
    {L"heavy-snow", L"/6503598/156949456-04a4bdbd-ff3b-4484-bb30-8909baff8aa8.png"},
    {L"light-rain", L"/6503598/156949458-dc66775d-8bb9-4d04-838e-7f550d305c26.png"},
    {L"mostly-cloudy-day", L"/6503598/156949462-f50c21dd-85dd-4d9c-a4eb-516e6cddfb1f.png"},
    {L"mostly-cloudy-night", L"/6503598/156949463-a427edfb-3d7f-4167-bd6f-f5019c482ea1.png"},
    {L"mostly-sunny-day", L"/6503598/156949438-4e0c0e0d-67bc-4c76-b75e-e0ffcead3f48.png"},
    {L"mostly-sunny-night", L"/6503598/156949442-63f14d44-ec0e-40b2-aa1b-8e4a27ec10f5.png"},
    {L"partly-cloudy-day", L"/6503598/156949465-54dd31c6-7e3a-464a-8e64-8b54b6fb7a65.png"},
    {L"partly-cloudy-night", L"/6503598/156949461-1f058cf3-6fdd-4aeb-80b7-68fa27b02845.png"},
    {L"rain", L"/6503598/156949460-7c132d89-efb7-457f-8810-9bf235f5737f.png"},
    {L"rain-and-snow", L"/6503598/156949445-60d12efa-a21d-40e0-b9a8-1b7a84e58944.png"},
    {L"snow", L"/6503598/156949459-dfe70eba-6c2c-4b1c-b51b-27c13ce7c08c.png"},
    {L"snow-and-rain", L"/6503598/156950233-ccaadb4a-2e9a-4934-b41c-acd36a7f0d9c.png"},
    {L"snow-showers-day", L"/6503598/156949447-a6658710-567e-4977-9316-a80007df3076.png"},
    {L"snow-showers-night", L"/6503598/156949448-cd1b69af-4028-4153-8e40-288526577b58.png"},
    {L"sun-showers-day", L"/6503598/156949443-062a0fa9-88c1-4e07-b6b1-8e52ff64f4f3.png"},
    {L"sun-showers-night", L"/6503598/156949444-d3aea936-4c22-4f17-a201-02155396684d.png"},
    {L"sunny", L"/6503598/156949449-9320c6f5-15ef-4c17-9e72-740708f4828c.png"},
    {L"thunderstorms", L"/6503598/156949450-7e03a3f5-580e-4414-aaeb-3a0898afd1da.png"},
};

static SRWLOCK epWeatherIconsLock = SRWLOCK_INIT;
static ULONGLONG epWeatherIconsStarted;
static BOOL epWeatherIconsLoading;
static BOOL epWeatherIconsHaveBatch;
static size_t epWeatherIconsCached;
static LPWSTR epWeatherIconPngs[ARRAYSIZE(epWeatherIconSources)];
static size_t epWeatherNextIcon;

static DWORD epw_Weather_ReadIcon(HINTERNET connection, LPCWSTR path, BYTE* buffer, DWORD capacity, ULONGLONG deadline)
{
    HINTERNET request = WinHttpOpenRequest(connection, L"GET", path, NULL,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!request) return 0;

    DWORD disabled = WINHTTP_DISABLE_COOKIES | WINHTTP_DISABLE_AUTHENTICATION | WINHTTP_DISABLE_REDIRECTS;
    DWORD status = 0, statusBytes = sizeof(status);
    DWORD length = 0;
    if (WinHttpSetOption(request, WINHTTP_OPTION_DISABLE_FEATURE, &disabled, sizeof(disabled)) &&
        WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
        WinHttpReceiveResponse(request, NULL) &&
        WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusBytes, WINHTTP_NO_HEADER_INDEX) &&
        status == HTTP_STATUS_OK)
    {
        while (length < capacity && GetTickCount64() < deadline)
        {
            DWORD received = 0;
            if (!WinHttpReadData(request, buffer + length, capacity - length, &received)) break;
            if (!received)
            {
                static const BYTE pngSignature[] = {0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a};
                if (length > sizeof(pngSignature) && !memcmp(buffer, pngSignature, sizeof(pngSignature)))
                {
                    WinHttpCloseHandle(request);
                    return length;
                }
                break;
            }
            length += received;
        }
    }
    WinHttpCloseHandle(request);
    return 0;
}

static void epw_Weather_DownloadMissingMicrosoftIcons(void)
{
    BYTE png[EP_WEATHER_ICON_MAX_BYTES + 1];
    WCHAR base64[((EP_WEATHER_ICON_MAX_BYTES + 2) / 3) * 4 + 1];

    HINTERNET session = WinHttpOpen(L"ExplorerPatcher Weather", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    HINTERNET connection = NULL;
    if (session && WinHttpSetTimeouts(session, 1200, 1500, 1500, 1500))
    {
        connection = WinHttpConnect(session, L"user-images.githubusercontent.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    }

    ULONGLONG deadline = epWeatherIconsStarted + EP_WEATHER_ICON_WAIT_MS;
    for (size_t attempt = 0; connection && attempt < ARRAYSIZE(epWeatherIconSources) && GetTickCount64() < deadline; ++attempt)
    {
        size_t i = epWeatherNextIcon;
        epWeatherNextIcon = (i + 1) % ARRAYSIZE(epWeatherIconSources);
        // Only the worker writes the cache. The renderer reads it under the lock.
        if (epWeatherIconPngs[i]) continue;
        DWORD size = epw_Weather_ReadIcon(connection, epWeatherIconSources[i].path, png, sizeof(png), deadline);
        DWORD cchBase64 = ARRAYSIZE(base64);
        if (size && CryptBinaryToStringW(png, size, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, base64, &cchBase64))
        {
            LPWSTR encoded = _wcsdup(base64);
            if (encoded)
            {
                AcquireSRWLockExclusive(&epWeatherIconsLock);
                epWeatherIconPngs[i] = encoded;
                ++epWeatherIconsCached;
                ReleaseSRWLockExclusive(&epWeatherIconsLock);
            }
        }
    }
    if (connection) WinHttpCloseHandle(connection);
    if (session) WinHttpCloseHandle(session);
}

// The caller holds a shared lock so all entries belong to one cache snapshot.
static LPWSTR epw_Weather_BuildIconCacheScript(void)
{
    if (!epWeatherIconsCached) return NULL;
    const WCHAR prefix[] = L"const ep_weather_microsoft_icons = {\n";
    const WCHAR suffix[] = L"};\n";
    size_t capacity = ARRAYSIZE(prefix) - 1 + ARRAYSIZE(suffix);
    for (size_t i = 0; i < ARRAYSIZE(epWeatherIconSources); ++i)
    {
        if (!epWeatherIconPngs[i]) continue;
        capacity += wcslen(epWeatherIconSources[i].name) + wcslen(epWeatherIconPngs[i]) + ARRAYSIZE(L"'': '',\n") - 1;
    }
    LPWSTR script = malloc(capacity * sizeof(WCHAR));
    if (!script) return NULL;

    size_t used = ARRAYSIZE(prefix) - 1;
    memcpy(script, prefix, used * sizeof(WCHAR));
    for (size_t i = 0; i < ARRAYSIZE(epWeatherIconSources); ++i)
    {
        if (!epWeatherIconPngs[i]) continue;
        int count = swprintf_s(script + used, capacity - used, L"'%ls': '%ls',\n", epWeatherIconSources[i].name, epWeatherIconPngs[i]);
        if (count < 0)
        {
            free(script);
            return NULL;
        }
        used += count;
    }
    memcpy(script + used, suffix, sizeof(suffix));
    return script;
}

static DWORD WINAPI epw_Weather_DownloadMicrosoftIconsThread(LPVOID module)
{
    epw_Weather_DownloadMissingMicrosoftIcons();
    AcquireSRWLockExclusive(&epWeatherIconsLock);
    epWeatherIconsHaveBatch = epWeatherIconsCached != 0;
    epWeatherIconsLoading = FALSE;
    ReleaseSRWLockExclusive(&epWeatherIconsLock);
    FreeLibraryAndExitThread(module, 0);
}

void epw_Weather_PrefetchMicrosoftIcons(void)
{
    AcquireSRWLockExclusive(&epWeatherIconsLock);
    ULONGLONG now = GetTickCount64();
    if (epWeatherIconsCached < ARRAYSIZE(epWeatherIconSources) && !epWeatherIconsLoading &&
        (!epWeatherIconsStarted || now - epWeatherIconsStarted >= EP_WEATHER_ICON_RETRY_MS))
    {
        epWeatherIconsStarted = now;

        // Keep the DLL loaded while the network worker is running.
        HMODULE module = NULL;
        if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)&epWeatherIconsLock, &module))
        {
            HANDLE thread = CreateThread(NULL, 0, epw_Weather_DownloadMicrosoftIconsThread, module, 0, NULL);
            epWeatherIconsLoading = thread != NULL;
            if (thread) CloseHandle(thread);
            else FreeLibrary(module);
        }
    }
    ReleaseSRWLockExclusive(&epWeatherIconsLock);
}

BOOL epw_Weather_GetMicrosoftIconScript(LPWSTR* script)
{
    epw_Weather_PrefetchMicrosoftIcons();
    AcquireSRWLockShared(&epWeatherIconsLock);
    BOOL ready = epWeatherIconsHaveBatch || !epWeatherIconsLoading || GetTickCount64() - epWeatherIconsStarted >= EP_WEATHER_ICON_WAIT_MS;
    *script = ready ? epw_Weather_BuildIconCacheScript() : NULL;
    ReleaseSRWLockShared(&epWeatherIconsLock);
    return ready;
}

void epw_Weather_FreeMicrosoftIconScript(void)
{
    for (size_t i = 0; i < ARRAYSIZE(epWeatherIconPngs); ++i) free(epWeatherIconPngs[i]);
}

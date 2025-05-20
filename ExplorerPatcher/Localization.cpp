#include "Localization.h"

#include <algorithm>
#include <vector>

#include "def.h"

extern "C"
{

EP_L10N_Language LangIDToEPLanguage(LANGID wLanguage)
{
    EP_L10N_Language language = {};
    language.id = wLanguage;
    GetLocaleInfoW(wLanguage, LOCALE_SNAME, language.wszId, ARRAYSIZE(language.wszId));
    GetLocaleInfoW(wLanguage, LOCALE_SLOCALIZEDDISPLAYNAME, language.wszDisplayName, ARRAYSIZE(language.wszDisplayName));
    return language;
}

BOOL EP_L10N_ApplyPreferredLanguageForCurrentThread()
{
    BOOL rv = FALSE;
    HKEY hKey = nullptr;
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH),
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        nullptr,
        &hKey,
        nullptr
    );
    if (hKey == nullptr || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = nullptr;
    }
    if (hKey)
    {
        DWORD dwPreferredLanguage = 0;
        DWORD dwSize = sizeof(dwPreferredLanguage);
        LSTATUS lres = RegQueryValueExW(
            hKey,
            TEXT("Language"),
            nullptr,
            nullptr,
            (LPBYTE)&dwPreferredLanguage,
            &dwSize
        );
        if (lres == ERROR_SUCCESS && dwPreferredLanguage != 0)
        {
            EP_L10N_Language language = LangIDToEPLanguage((LANGID)dwPreferredLanguage);
            rv = SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, language.wszId, nullptr);
        }
        else
        {
            rv = SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, nullptr, nullptr);
        }
        RegCloseKey(hKey);
    }
    return rv;
}

BOOL EP_L10N_GetCurrentUserLanguage(wchar_t* wszLanguage, int cch)
{
    BOOL bOk = FALSE;
    ULONG ulNumLanguages = 0;
    ULONG cchLanguagesBuffer = 0;
    if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &ulNumLanguages, nullptr, &cchLanguagesBuffer))
    {
        wchar_t* wszLanguagesBuffer = (wchar_t*)malloc(cchLanguagesBuffer * sizeof(wchar_t));
        if (wszLanguagesBuffer)
        {
            if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &ulNumLanguages, wszLanguagesBuffer, &cchLanguagesBuffer))
            {
                wcscpy_s(wszLanguage, cch, wszLanguagesBuffer);
                bOk = TRUE;
            }
            free(wszLanguagesBuffer);
        }
    }
    if (!bOk)
    {
        wcscpy_s(wszLanguage, cch, L"en-US");
    }
    return TRUE;
}

BOOL EP_L10N_GetCurrentThreadLanguage(wchar_t* wszLanguage, int cch)
{
    BOOL bOk = FALSE;
    ULONG ulNumLanguages = 0;
    ULONG cchLanguagesBuffer = 0;
    if (GetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, &ulNumLanguages, nullptr, &cchLanguagesBuffer))
    {
        wchar_t* wszLanguagesBuffer = (wchar_t*)malloc(cchLanguagesBuffer * sizeof(wchar_t));
        if (wszLanguagesBuffer)
        {
            if (GetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, &ulNumLanguages, wszLanguagesBuffer, &cchLanguagesBuffer))
            {
                wcscpy_s(wszLanguage, cch, wszLanguagesBuffer);
                bOk = TRUE;
            }
            free(wszLanguagesBuffer);
        }
    }
    if (!bOk)
    {
        wcscpy_s(wszLanguage, cch, L"en-US");
    }
    return TRUE;
}

void EP_L10N_EnumerateLanguages(HMODULE hModule, LPCWSTR lpType, LPCWSTR lpName, EP_L10N_EnumerateLanguagesProc_t pfnProc, void* data)
{
    std::vector<EP_L10N_Language> languages;

    // English (US) is our primary language
    languages.push_back(LangIDToEPLanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)));

    // Add the rest below it
    EnumResourceLanguagesW(hModule, lpType, lpName, [](HMODULE, LPCWSTR, LPCWSTR, WORD wLanguage, LONG_PTR lParam) -> BOOL
    {
        if (wLanguage != MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
        {
            EP_L10N_Language language = LangIDToEPLanguage(wLanguage);
            ((std::vector<EP_L10N_Language>*)lParam)->push_back(language);
        }
        return TRUE;
    }, (LONG_PTR)&languages);

    // Sort the non-primary languages by localized display name
    std::sort(languages.begin() + 1, languages.end(), [](const EP_L10N_Language& a, const EP_L10N_Language& b) -> bool
    {
        return wcscmp(a.wszDisplayName, b.wszDisplayName) < 0;
    });

    // Call the callback for each language
    for (const EP_L10N_Language& language : languages)
    {
        pfnProc(&language, data);
    }
}

}

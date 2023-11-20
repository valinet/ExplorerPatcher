#pragma once

#include <Windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct EP_L10N_Language
{
    LANGID id;
    wchar_t wszId[LOCALE_NAME_MAX_LENGTH];
    wchar_t wszDisplayName[LOCALE_NAME_MAX_LENGTH];
} EP_L10N_Language;

typedef void(*EP_L10N_EnumerateLanguagesProc_t)(const EP_L10N_Language* language, void* data);

BOOL EP_L10N_ApplyPreferredLanguageForCurrentThread();
BOOL EP_L10N_GetCurrentUserLanguage(wchar_t* wszLanguage, int cch);
BOOL EP_L10N_GetCurrentThreadLanguage(wchar_t* wszLanguage, int cch);
void EP_L10N_EnumerateLanguages(HMODULE hModule, LPCWSTR lpType, LPCWSTR lpName, EP_L10N_EnumerateLanguagesProc_t pfnProc, void* data);

#ifdef __cplusplus
}
#endif

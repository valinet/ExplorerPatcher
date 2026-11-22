#include <Windows.h>
#include <wctype.h>
#include <string.h>
#include <wchar.h>
#include "appverifier.h"

static const WCHAR marker[] = L"ExplorerPatcherVerifierFlagOwned";
static const WCHAR createdFlagMarker[] = L"ExplorerPatcherVerifierCreatedGlobalFlag";

typedef struct
{
    DWORD value;
    BOOL present;
} OPTIONAL_DWORD;

typedef struct
{
    WCHAR providers[32768];
    BOOL hadProviders;
    OPTIONAL_DWORD flags, owned, createdFlag;
} VERIFIER_SNAPSHOT;

static LSTATUS DeleteValueIfPresent(HKEY key, LPCWSTR name)
{
    LSTATUS status = RegDeleteValueW(key, name);
    return status == ERROR_FILE_NOT_FOUND ? ERROR_SUCCESS : status;
}

static LSTATUS ReadOptionalDword(HKEY key, LPCWSTR name, OPTIONAL_DWORD* original)
{
    DWORD type = 0, bytes = sizeof(original->value);
    LSTATUS status = RegQueryValueExW(key, name, NULL, &type, (BYTE*)&original->value, &bytes);
    original->present = status == ERROR_SUCCESS;
    if (status == ERROR_FILE_NOT_FOUND)
    {
        original->value = 0;
        return ERROR_SUCCESS;
    }
    if (status == ERROR_SUCCESS && (type != REG_DWORD || bytes != sizeof(original->value)))
    {
        return ERROR_INVALID_DATA;
    }
    return status;
}

static void RestoreOptionalDword(HKEY key, LPCWSTR name, const OPTIONAL_DWORD* original)
{
    if (original->present)
    {
        RegSetValueExW(key, name, 0, REG_DWORD, (const BYTE*)&original->value, sizeof(original->value));
    }
    else
    {
        RegDeleteValueW(key, name);
    }
}

static void RestoreVerifierValues(HKEY key, const VERIFIER_SNAPSHOT* original)
{
    if (original->hadProviders)
    {
        RegSetValueExW(key, L"VerifierDlls", 0, REG_SZ, (const BYTE*)original->providers,
            (DWORD)((wcslen(original->providers) + 1) * sizeof(WCHAR)));
    }
    else
    {
        RegDeleteValueW(key, L"VerifierDlls");
    }
    RestoreOptionalDword(key, L"GlobalFlag", &original->flags);
    RestoreOptionalDword(key, marker, &original->owned);
    RestoreOptionalDword(key, createdFlagMarker, &original->createdFlag);
}

static WCHAR* FindProvider(WCHAR* providers, LPCWSTR verifierDll, size_t length)
{
    for (WCHAR* cursor = providers; *cursor;)
    {
        while (*cursor && iswspace(*cursor))
        {
            ++cursor;
        }
        WCHAR* begin = cursor;
        while (*cursor && !iswspace(*cursor))
        {
            ++cursor;
        }
        if ((size_t)(cursor - begin) == length && !_wcsnicmp(begin, verifierDll, length))
        {
            return begin;
        }
    }
    return NULL;
}

// Preserve other verifier providers and unrelated IFEO values.
BOOL ConfigureAppVerifier(BOOL install, LPCWSTR targetImage, LPCWSTR verifierDll)
{
    static const WCHAR keyPrefix[] = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\";
    WCHAR keyName[MAX_PATH];
    if (wcscpy_s(keyName, ARRAYSIZE(keyName), keyPrefix) || wcscat_s(keyName, ARRAYSIZE(keyName), targetImage))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    HKEY key = NULL;
    LSTATUS status = install ? RegCreateKeyExW(HKEY_LOCAL_MACHINE, keyName, 0, NULL, 0, KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &key, NULL)
                             : RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyName, 0, KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_WOW64_64KEY, &key);
    if (!install && status == ERROR_FILE_NOT_FOUND)
    {
        return TRUE;
    }
    if (status != ERROR_SUCCESS)
    {
        SetLastError(status);
        return FALSE;
    }

    VERIFIER_SNAPSHOT original = {0};
    WCHAR providers[ARRAYSIZE(original.providers)] = {0};
    DWORD type = 0, bytes = sizeof(providers);
    status = RegQueryValueExW(key, L"VerifierDlls", NULL, &type, (BYTE*)providers, &bytes);
    original.hadProviders = status == ERROR_SUCCESS;
    if (status == ERROR_FILE_NOT_FOUND)
    {
        status = ERROR_SUCCESS;
        providers[0] = 0;
    }
    else if (status == ERROR_SUCCESS && (type != REG_SZ || bytes < sizeof(WCHAR) || bytes > sizeof(providers) || providers[bytes / sizeof(WCHAR) - 1] != 0))
    {
        status = ERROR_INVALID_DATA;
    }
    if (status == ERROR_SUCCESS)
    {
        status = ReadOptionalDword(key, L"GlobalFlag", &original.flags);
    }
    if (status == ERROR_SUCCESS)
    {
        status = ReadOptionalDword(key, marker, &original.owned);
    }
    if (status == ERROR_SUCCESS)
    {
        status = ReadOptionalDword(key, createdFlagMarker, &original.createdFlag);
    }
    if (status == ERROR_SUCCESS)
    {
        wcscpy_s(original.providers, ARRAYSIZE(original.providers), providers);
    }
    DWORD flags = original.flags.value;
    size_t verifierLength = wcslen(verifierDll);
    BOOL writeAttempted = FALSE;
    WCHAR* found = NULL;
    if (status == ERROR_SUCCESS)
    {
        found = FindProvider(providers, verifierDll, verifierLength);
        if (install && !found)
        {
            size_t len = wcslen(providers), count = verifierLength;
            if (len + (len ? 1 : 0) + count + 1 > ARRAYSIZE(providers))
            {
                status = ERROR_INSUFFICIENT_BUFFER;
            }
            else
            {
                if (len)
                {
                    providers[len++] = L' ';
                }
                memcpy(providers + len, verifierDll, (count + 1) * sizeof(WCHAR));
            }
        }
        else if (!install && found)
        {
            WCHAR* end = found + verifierLength;
            while (*end && iswspace(*end))
            {
                ++end;
            }
            memmove(found, end, (wcslen(end) + 1) * sizeof(WCHAR));
            size_t len = wcslen(providers);
            while (len && iswspace(providers[len - 1]))
            {
                providers[--len] = 0;
            }
        }
    }
    if (status == ERROR_SUCCESS && (install || found))
    {
        writeAttempted = TRUE;
        if (providers[0])
        {
            status = RegSetValueExW(key, L"VerifierDlls", 0, REG_SZ, (BYTE*)providers, (DWORD)((wcslen(providers) + 1) * sizeof(WCHAR)));
        }
        else
        {
            status = DeleteValueIfPresent(key, L"VerifierDlls");
        }
        if (status == ERROR_SUCCESS && install && !(flags & 0x100))
        {
            DWORD one = 1;
            status = RegSetValueExW(key, marker, 0, REG_DWORD, (BYTE*)&one, sizeof(one));
            if (status == ERROR_SUCCESS)
            {
                if (!original.flags.present)
                {
                    status = RegSetValueExW(key, createdFlagMarker, 0, REG_DWORD, (BYTE*)&one, sizeof(one));
                }
                else
                {
                    status = DeleteValueIfPresent(key, createdFlagMarker);
                }
            }
            if (status == ERROR_SUCCESS)
            {
                flags |= 0x100;
                status = RegSetValueExW(key, L"GlobalFlag", 0, REG_DWORD, (BYTE*)&flags, sizeof(flags));
            }
        }
        else if (status == ERROR_SUCCESS && !install && original.owned.value)
        {
            // Another verifier configuration may use the bit even without a
            // custom provider list. Do not disable it when VerifierFlags exists.
            DWORD verifierFlagsSize = 0;
            LSTATUS verifierFlagsStatus = RegQueryValueExW(key, L"VerifierFlags", NULL, NULL, NULL, &verifierFlagsSize);
            if (verifierFlagsStatus != ERROR_SUCCESS && verifierFlagsStatus != ERROR_FILE_NOT_FOUND)
            {
                status = verifierFlagsStatus;
            }
            if (status == ERROR_SUCCESS && !providers[0] && verifierFlagsStatus == ERROR_FILE_NOT_FOUND)
            {
                flags &= ~0x100;
                if (original.createdFlag.value && flags == 0)
                {
                    status = DeleteValueIfPresent(key, L"GlobalFlag");
                }
                else
                {
                    status = RegSetValueExW(key, L"GlobalFlag", 0, REG_DWORD, (BYTE*)&flags, sizeof(flags));
                }
            }
            if (status == ERROR_SUCCESS)
            {
                status = DeleteValueIfPresent(key, marker);
            }
            if (status == ERROR_SUCCESS && original.createdFlag.value)
            {
                status = DeleteValueIfPresent(key, createdFlagMarker);
            }
        }
    }
    if (status != ERROR_SUCCESS && writeAttempted)
    {
        // Restore the exact values captured before this attempt. Keep the
        // original error code so the setup can report failure.
        RestoreVerifierValues(key, &original);
    }
    RegCloseKey(key);
    if (status != ERROR_SUCCESS)
    {
        SetLastError(status);
    }
    return status == ERROR_SUCCESS;
}

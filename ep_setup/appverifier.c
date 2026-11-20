#include <Windows.h>
#include <wctype.h>
#include "../ExplorerPatcher/utility.h"
#include "appverifier.h"
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

    static const WCHAR marker[] = L"ExplorerPatcherVerifierFlagOwned";
    static const WCHAR createdFlagMarker[] = L"ExplorerPatcherVerifierCreatedGlobalFlag";
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

    WCHAR providers[32768] = {0};
    DWORD type = 0, bytes = sizeof(providers);
    status = RegQueryValueExW(key, L"VerifierDlls", NULL, &type, (BYTE*)providers, &bytes);
    BOOL hadProviders = status == ERROR_SUCCESS;
    if (status == ERROR_FILE_NOT_FOUND)
    {
        status = ERROR_SUCCESS;
        providers[0] = 0;
    }
    else if (status == ERROR_SUCCESS && (type != REG_SZ || bytes < sizeof(WCHAR) || bytes > sizeof(providers) || providers[bytes / sizeof(WCHAR) - 1] != 0))
    {
        status = ERROR_INVALID_DATA;
    }
    DWORD flags = 0, flagType = 0, flagSize = sizeof(flags);
    BOOL hadFlags = FALSE;
    if (status == ERROR_SUCCESS)
    {
        LSTATUS flagStatus = RegQueryValueExW(key, L"GlobalFlag", NULL, &flagType, (BYTE*)&flags, &flagSize);
        hadFlags = flagStatus == ERROR_SUCCESS;
        if (flagStatus == ERROR_FILE_NOT_FOUND)
        {
            flags = 0;
        }
        else if (flagStatus != ERROR_SUCCESS || flagType != REG_DWORD || flagSize != sizeof(flags))
        {
            status = flagStatus == ERROR_SUCCESS ? ERROR_INVALID_DATA : flagStatus;
        }
    }
    DWORD owned = 0, ownedSize = sizeof(owned), ownedType = 0;
    BOOL hadOwner = FALSE;
    if (status == ERROR_SUCCESS)
    {
        LSTATUS ownerStatus = RegQueryValueExW(key, marker, NULL, &ownedType, (BYTE*)&owned, &ownedSize);
        hadOwner = ownerStatus == ERROR_SUCCESS;
        if (ownerStatus == ERROR_FILE_NOT_FOUND)
        {
            owned = 0;
        }
        else if (ownerStatus != ERROR_SUCCESS || ownedType != REG_DWORD || ownedSize != sizeof(owned))
        {
            status = ownerStatus == ERROR_SUCCESS ? ERROR_INVALID_DATA : ownerStatus;
        }
    }
    DWORD createdFlag = 0, createdFlagSize = sizeof(createdFlag), createdFlagType = 0;
    BOOL hadCreatedFlagMarker = FALSE;
    if (status == ERROR_SUCCESS)
    {
        LSTATUS createdStatus = RegQueryValueExW(key, createdFlagMarker, NULL, &createdFlagType, (BYTE*)&createdFlag, &createdFlagSize);
        hadCreatedFlagMarker = createdStatus == ERROR_SUCCESS;
        if (createdStatus == ERROR_FILE_NOT_FOUND)
        {
            createdFlag = 0;
        }
        else if (createdStatus != ERROR_SUCCESS || createdFlagType != REG_DWORD || createdFlagSize != sizeof(createdFlag))
        {
            status = createdStatus == ERROR_SUCCESS ? ERROR_INVALID_DATA : createdStatus;
        }
    }

    WCHAR originalProviders[ARRAYSIZE(providers)];
    if (status == ERROR_SUCCESS)
    {
        wcscpy_s(originalProviders, ARRAYSIZE(originalProviders), providers);
    }
    DWORD originalFlags = flags;
    BOOL writeAttempted = FALSE;
    WCHAR* found = NULL;
    if (status == ERROR_SUCCESS)
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
            if ((size_t)(cursor - begin) == wcslen(verifierDll) && !_wcsnicmp(begin, verifierDll, wcslen(verifierDll)))
            {
                found = begin;
                break;
            }
        }
        if (install && !found)
        {
            size_t len = wcslen(providers), count = wcslen(verifierDll);
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
            WCHAR* end = found + wcslen(verifierDll);
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
            status = RegDeleteValueW(key, L"VerifierDlls");
            if (status == ERROR_FILE_NOT_FOUND)
            {
                status = ERROR_SUCCESS;
            }
        }
        if (status == ERROR_SUCCESS && install && !(flags & 0x100))
        {
            DWORD one = 1;
            status = RegSetValueExW(key, marker, 0, REG_DWORD, (BYTE*)&one, sizeof(one));
            if (status == ERROR_SUCCESS)
            {
                if (!hadFlags)
                {
                    status = RegSetValueExW(key, createdFlagMarker, 0, REG_DWORD, (BYTE*)&one, sizeof(one));
                }
                else
                {
                    status = RegDeleteValueW(key, createdFlagMarker);
                    if (status == ERROR_FILE_NOT_FOUND)
                    {
                        status = ERROR_SUCCESS;
                    }
                }
            }
            if (status == ERROR_SUCCESS)
            {
                flags |= 0x100;
                status = RegSetValueExW(key, L"GlobalFlag", 0, REG_DWORD, (BYTE*)&flags, sizeof(flags));
            }
        }
        else if (status == ERROR_SUCCESS && !install && owned)
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
                if (createdFlag && flags == 0)
                {
                    status = RegDeleteValueW(key, L"GlobalFlag");
                    if (status == ERROR_FILE_NOT_FOUND)
                    {
                        status = ERROR_SUCCESS;
                    }
                }
                else
                {
                    status = RegSetValueExW(key, L"GlobalFlag", 0, REG_DWORD, (BYTE*)&flags, sizeof(flags));
                }
            }
            if (status == ERROR_SUCCESS)
            {
                LSTATUS s = RegDeleteValueW(key, marker);
                if (s != ERROR_SUCCESS && s != ERROR_FILE_NOT_FOUND)
                {
                    status = s;
                }
            }
            if (status == ERROR_SUCCESS && createdFlag)
            {
                LSTATUS s = RegDeleteValueW(key, createdFlagMarker);
                if (s != ERROR_SUCCESS && s != ERROR_FILE_NOT_FOUND)
                {
                    status = s;
                }
            }
        }
    }
    if (status != ERROR_SUCCESS && writeAttempted)
    {
        // Restore the exact values captured before this attempt. Keep the
        // original error code so the setup can report failure.
        if (hadProviders)
        {
            RegSetValueExW(key, L"VerifierDlls", 0, REG_SZ, (BYTE*)originalProviders, (DWORD)((wcslen(originalProviders) + 1) * sizeof(WCHAR)));
        }
        else
        {
            RegDeleteValueW(key, L"VerifierDlls");
        }
        if (hadFlags)
        {
            RegSetValueExW(key, L"GlobalFlag", 0, REG_DWORD, (BYTE*)&originalFlags, sizeof(originalFlags));
        }
        else
        {
            RegDeleteValueW(key, L"GlobalFlag");
        }
        if (hadOwner)
        {
            RegSetValueExW(key, marker, 0, REG_DWORD, (BYTE*)&owned, sizeof(owned));
        }
        else
        {
            RegDeleteValueW(key, marker);
        }
        if (hadCreatedFlagMarker)
        {
            RegSetValueExW(key, createdFlagMarker, 0, REG_DWORD, (BYTE*)&createdFlag, sizeof(createdFlag));
        }
        else
        {
            RegDeleteValueW(key, createdFlagMarker);
        }
    }
    RegCloseKey(key);
    if (status != ERROR_SUCCESS)
    {
        SetLastError(status);
    }
    return status == ERROR_SUCCESS;
}

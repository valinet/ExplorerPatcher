#include "updates.h"

BOOL IsUpdatePolicy(LPCWSTR wszDataStore, DWORD dwUpdatePolicy)
{
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    DWORD dwQueriedPolicy = 0;
    BOOL bIsPolicyMatch = (dwUpdatePolicy == UPDATE_POLICY_AUTO);

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        wszDataStore,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
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
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("UpdatePolicy"),
            0,
            NULL,
            &dwQueriedPolicy,
            &dwSize
        );
        RegCloseKey(hKey);
        bIsPolicyMatch = (dwQueriedPolicy == dwUpdatePolicy);
    }
    return bIsPolicyMatch;
}

void IsUpdateAvailableHelperCallback(
    HINTERNET hInternet,
    struct IsUpdateAvailableParameters* params,
    DWORD dwInternetStatus,
    INTERNET_ASYNC_RESULT* lpvStatusInformation,
    DWORD dwStatusInformationLength
)
{
    if (dwInternetStatus == INTERNET_STATUS_REQUEST_COMPLETE)
    {
        params->hInternet = lpvStatusInformation->dwResult;
        SetEvent(params->hEvent);
    }
}

BOOL IsUpdateAvailableHelper(
    char* url,
    char* szCheckAgainst,
    DWORD dwUpdateTimeout,
    BOOL* lpFail,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotifier* notifier,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotificationFactory* notifFactory,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotification** toast,
    BOOL bUpdatePreferStaging,
    WCHAR* wszInfoURL,
    DWORD dwInfoURLLen,
    HMODULE hModule,
    DWORD* pLeftMost, DWORD* pSecondLeft, DWORD* pSecondRight, DWORD* pRightMost
)
{
    BOOL bIsUpdateAvailable = FALSE;

    struct IsUpdateAvailableParameters params;
    params.hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (!params.hEvent)
    {
        return bIsUpdateAvailable;
    }

    char* staging_buffer = NULL;
    HINTERNET hInternet = NULL;

    if (bUpdatePreferStaging)
    {
        if (hInternet = InternetOpenA(
            UPDATES_USER_AGENT,
            INTERNET_OPEN_TYPE_PRECONFIG,
            NULL,
            NULL,
            0
        ))
        {
            HINTERNET hConnect = InternetOpenUrlA(
                hInternet,
                url,
                NULL,
                0,
                INTERNET_FLAG_RAW_DATA |
                INTERNET_FLAG_RELOAD |
                INTERNET_FLAG_RESYNCHRONIZE |
                INTERNET_FLAG_NO_COOKIES |
                INTERNET_FLAG_NO_UI |
                INTERNET_FLAG_NO_CACHE_WRITE |
                INTERNET_FLAG_DONT_CACHE,
                &params
            );
            if (hConnect)
            {
                DWORD dwSize = 5000;
                DWORD dwRead = dwSize;
                staging_buffer = calloc(dwSize, sizeof(char));
                if (staging_buffer)
                {
                    BOOL bRet = FALSE;
                    if (bRet = InternetReadFile(
                        hConnect,
                        staging_buffer,
                        dwSize - 1,
                        &dwRead
                    ))
                    {
                        char* a1 = strstr(staging_buffer, "\"browser_download_url\"");
                        if (a1)
                        {
                            char* a2 = strchr(a1 + 24, '"');
                            if (a2)
                            {
                                a2[0] = 0;
                                printf("[Updates] Prerelease update URL: \"%s\"\n", a1 + 24);
                                url = a1 + 24;
                                bUpdatePreferStaging = FALSE;
                                if (wszInfoURL)
                                {
                                    char* a3 = strstr(staging_buffer, "\"html_url\"");
                                    if (a3)
                                    {
                                        char* a4 = strchr(a3 + 12, '"');
                                        if (a4)
                                        {
                                            a4[0] = 0;
                                            printf("[Updates] Release notes URL: \"%s\"\n", a3 + 12);
                                            MultiByteToWideChar(
                                                CP_UTF8,
                                                MB_PRECOMPOSED,
                                                a3 + 12,
                                                -1,
                                                wszInfoURL,
                                                dwInfoURLLen
                                            );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                InternetCloseHandle(hConnect);
            }
            InternetCloseHandle(hInternet);
        }
    }

    if (!bUpdatePreferStaging && (hInternet = InternetOpenA(
        UPDATES_USER_AGENT,
        INTERNET_OPEN_TYPE_PRECONFIG,
        NULL,
        NULL,
        0
    )))
    {
        HINTERNET hConnect = InternetOpenUrlA(
            hInternet,
            url,
            NULL,
            0,
            INTERNET_FLAG_RAW_DATA |
            INTERNET_FLAG_RELOAD |
            INTERNET_FLAG_RESYNCHRONIZE |
            INTERNET_FLAG_NO_COOKIES |
            INTERNET_FLAG_NO_UI |
            INTERNET_FLAG_NO_CACHE_WRITE |
            INTERNET_FLAG_DONT_CACHE,
            &params
        );
        if (hConnect)
        {
            if (szCheckAgainst)
            {
                BOOL bRet = FALSE;
                DWORD dwRead = 0;
                char hash[DOSMODE_OFFSET + UPDATES_HASH_SIZE + 1];
                ZeroMemory(hash, DOSMODE_OFFSET + UPDATES_HASH_SIZE + 1);
                if (bRet = InternetReadFile(
                    hConnect,
                    hash,
                    DOSMODE_OFFSET + UPDATES_HASH_SIZE,
                    &dwRead
                ) && dwRead == DOSMODE_OFFSET + UPDATES_HASH_SIZE)
                {
#ifdef UPDATES_VERBOSE_OUTPUT
                    printf("[Updates] Hash of remote file is \"%s\" (%s).\n", DOSMODE_OFFSET + hash, (hash[0] == 0x4D && hash[1] == 0x5A) ? "valid" : "invalid");
#endif
                    BOOL bOldType = TRUE;
                    char *szLeftMost = NULL, *szSecondLeft = NULL, *szSecondRight = NULL, *szRightMost = NULL, *szRealHash = NULL;
                    if (hModule)
                    {
                        if (hash[0] == 0x4D && hash[1] == 0x5A)
                        {
                            if (strchr(DOSMODE_OFFSET + hash, '.'))
                            {
                                szLeftMost = DOSMODE_OFFSET + hash;
                                if (szSecondLeft = strchr(szLeftMost, '.'))
                                {
                                    *szSecondLeft = 0;
                                    szSecondLeft++;
                                    if (szSecondRight = strchr(szSecondLeft, '.'))
                                    {
                                        *szSecondRight = 0;
                                        szSecondRight++;
                                        if (szRightMost = strchr(szSecondRight, '.'))
                                        {
                                            *szRightMost = 0;
                                            szRightMost++;
                                            if (szRealHash = strchr(szRightMost, '.'))
                                            {
                                                bOldType = FALSE;

                                                *szRealHash = 0;
                                                szRealHash++;
                                                DWORD dwRemoteLeftMost = atoi(szLeftMost);
                                                if (pLeftMost) *pLeftMost = dwRemoteLeftMost - ((dwRemoteLeftMost == 22622 && szRealHash[0] != '!') ? 1 : 0);
                                                DWORD dwRemoteSecondLeft = atoi(szSecondLeft);
                                                if (pSecondLeft) *pSecondLeft = dwRemoteSecondLeft;
                                                DWORD dwRemoteSecondRight = atoi(szSecondRight);
                                                if (pSecondRight) *pSecondRight = dwRemoteSecondRight;
                                                DWORD dwRemoteRightMost = atoi(szRightMost);
                                                if (pRightMost) *pRightMost = dwRemoteRightMost;
                                                DWORD dwLocalLeftMost = 0;
                                                DWORD dwLocalSecondLeft = 0;
                                                DWORD dwLocalSecondRight = 0;
                                                DWORD dwLocalRightMost = 0;
                                                BOOL bExtractedFromHash = FALSE;
                                                CHAR hashCopy[100];
                                                strcpy_s(hashCopy, 100, szCheckAgainst);
                                                char* szLocalLeftMost = NULL, *szLocalSecondLeft = NULL, *szLocalSecondRight = NULL, *szLocalRightMost = NULL, *szLocalRealHash = NULL;
                                                if (strchr(hashCopy, '.')) 
                                                {
                                                    szLocalLeftMost = hashCopy;
                                                    if (szLocalSecondLeft = strchr(szLocalLeftMost, '.'))
                                                    {
                                                        *szLocalSecondLeft = 0;
                                                        szLocalSecondLeft++;
                                                        if (szLocalSecondRight = strchr(szLocalSecondLeft, '.'))
                                                        {
                                                            *szLocalSecondRight = 0;
                                                            szLocalSecondRight++;
                                                            if (szLocalRightMost = strchr(szLocalSecondRight, '.'))
                                                            {
                                                                *szLocalRightMost = 0;
                                                                szLocalRightMost++;
                                                                if (szLocalRealHash = strchr(szLocalRightMost, '.'))
                                                                {
                                                                    *szLocalRealHash = 0;
                                                                    szLocalRealHash++;

                                                                    bExtractedFromHash = TRUE;
                                                                    dwLocalLeftMost = atoi(szLocalLeftMost);
                                                                    dwLocalSecondLeft = atoi(szLocalSecondLeft);
                                                                    dwLocalSecondRight = atoi(szLocalSecondRight);
                                                                    dwLocalRightMost = atoi(szLocalRightMost);
#ifdef UPDATES_VERBOSE_OUTPUT
                                                                    printf("[Updates] Local version obtained from hash is %d.%d.%d.%d.\n", dwLocalLeftMost, dwLocalSecondLeft, dwLocalSecondRight, dwLocalRightMost);
#endif
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                if (!bExtractedFromHash) 
                                                {
                                                    QueryVersionInfo(hModule, VS_VERSION_INFO, &dwLocalLeftMost, &dwLocalSecondLeft, &dwLocalSecondRight, &dwLocalRightMost);
                                                }

                                                int res = 0;
                                                if (!res)
                                                {
                                                    if (dwLocalLeftMost < dwRemoteLeftMost)
                                                    {
                                                        res = -1;
                                                    }
                                                    if (dwLocalLeftMost > dwRemoteLeftMost)
                                                    {
                                                        res = 1;
                                                    }
                                                    if (!res)
                                                    {
                                                        if (dwLocalSecondLeft < dwRemoteSecondLeft)
                                                        {
                                                            res = -1;
                                                        }
                                                        if (dwLocalSecondLeft > dwRemoteSecondLeft)
                                                        {
                                                            res = 1;
                                                        }
                                                        if (!res)
                                                        {
                                                            if (dwLocalSecondRight < dwRemoteSecondRight)
                                                            {
                                                                res = -1;
                                                            }
                                                            if (dwLocalSecondRight > dwRemoteSecondRight)
                                                            {
                                                                res = 1;
                                                            }
                                                            if (!res)
                                                            {
                                                                if (dwLocalRightMost < dwRemoteRightMost)
                                                                {
                                                                    res = -1;
                                                                }
                                                                if (dwLocalRightMost > dwRemoteRightMost)
                                                                {
                                                                    res = 1;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                DWORD dwAllowDowngrades = FALSE, dwSize = sizeof(DWORD);
                                                RegGetValueW(HKEY_CURRENT_USER, _T(REGPATH), L"UpdateAllowDowngrades", RRF_RT_DWORD, NULL, &dwAllowDowngrades, &dwSize);
                                                if ((res == 1 && dwAllowDowngrades) || res == -1)
                                                {
                                                    bIsUpdateAvailable = TRUE;
                                                }
                                                else if (res == 0)
                                                {
                                                    *(szSecondLeft - 1) = '.';
                                                    *(szSecondRight - 1) = '.';
                                                    *(szRightMost - 1) = '.';
                                                    *(szRealHash - 1) = '.';
                                                    if (_stricmp(DOSMODE_OFFSET + hash, szCheckAgainst))
                                                    {
                                                        bIsUpdateAvailable = TRUE;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    /*if (bOldType)
                    {
                        if (hash[0] == 0x4D && hash[1] == 0x5A && _stricmp(DOSMODE_OFFSET + hash, szCheckAgainst))
                        {
                            bIsUpdateAvailable = TRUE;
                        }
                    }*/
                }
                else
                {
#ifdef UPDATES_VERBOSE_OUTPUT
                    printf("[Updates] Failed. Read %d bytes.\n", dwRead);
#endif
                    if (lpFail) *lpFail = TRUE;
                }
            }
            else
            {
                WCHAR wszPath[MAX_PATH];
                ZeroMemory(wszPath, MAX_PATH * sizeof(WCHAR));
                SHGetFolderPathW(NULL, SPECIAL_FOLDER_LEGACY, NULL, SHGFP_TYPE_CURRENT, wszPath);
                wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH));
                BOOL bRet = CreateDirectoryW(wszPath, NULL);
                if (bRet || (!bRet && GetLastError() == ERROR_ALREADY_EXISTS))
                {
                    wcscat_s(wszPath, MAX_PATH, L"\\Update for " _T(PRODUCT_NAME) L" from ");
                    WCHAR wszURL[MAX_PATH];
                    ZeroMemory(wszURL, MAX_PATH * sizeof(WCHAR));
                    MultiByteToWideChar(
                        CP_UTF8,
                        MB_PRECOMPOSED,
                        url,
                        -1,
                        wszURL,
                        MAX_PATH
                    );
                    if (wszURL[97])
                    {
                        wszURL[96] = L'.';
                        wszURL[97] = L'.';
                        wszURL[98] = L'.';
                        wszURL[99] = L'e';
                        wszURL[100] = L'x';
                        wszURL[101] = L'e';
                        wszURL[102] = 0;
                    }
                    for (unsigned int i = 0; i < wszURL; ++i)
                    {
                        if (!wszURL[i])
                        {
                            break;
                        }
                        if (wszURL[i] == L'/')
                        {
                            wszURL[i] = L'\u2215';
                        }
                        else if (wszURL[i] == L':')
                        {
                            wszURL[i] = L'\ua789';
                        }
                    }
                    wcscat_s(wszPath, MAX_PATH, wszURL);
#ifdef UPDATES_VERBOSE_OUTPUT
                    wprintf(L"[Updates] Download path is \"%s\".\n", wszPath);
#endif

                    BOOL bRet = DeleteFileW(wszPath);
                    if (bRet || (!bRet && GetLastError() == ERROR_FILE_NOT_FOUND))
                    {
                        DWORD bIsUsingEpMake = 0, dwSize = sizeof(DWORD);
                        RegGetValueW(HKEY_CURRENT_USER, TEXT(REGPATH), L"UpdateUseLocal", RRF_RT_DWORD, NULL, &bIsUsingEpMake, &dwSize);
                        if (bIsUsingEpMake) {
                            GetSystemDirectoryW(wszPath, MAX_PATH);
                            wcscat_s(wszPath, MAX_PATH, L"\\WindowsPowerShell\\v1.0\\powershell.exe");
                        }

                        FILE* f = NULL;
                        if (!bIsUsingEpMake && !_wfopen_s(
                            &f,
                            wszPath,
                            L"wb"
                        ) && f)
                        {
                            BYTE* buffer = (BYTE*)malloc(UPDATES_BUFSIZ);
                            if (buffer != NULL)
                            {
                                DWORD dwRead = 0;
                                bRet = FALSE;
                                while (bRet = InternetReadFile(
                                    hConnect,
                                    buffer,
                                    UPDATES_BUFSIZ,
                                    &dwRead
                                ))
                                {
                                    if (dwRead == 0)
                                    {
                                        bIsUpdateAvailable = TRUE;
#ifdef UPDATES_VERBOSE_OUTPUT
                                        printf("[Updates] Downloaded finished.\n");
#endif
                                        break;
                                    }
#ifdef UPDATES_VERBOSE_OUTPUT
                                    printf("[Updates] Downloaded %d bytes.\n", dwRead);
#endif
                                    fwrite(
                                        buffer,
                                        sizeof(BYTE),
                                        dwRead,
                                        f
                                    );
                                    dwRead = 0;
                                }
                                free(buffer);
                            }
                            fclose(f);
                        }
                        if (bIsUsingEpMake || bIsUpdateAvailable)
                        {
                            bIsUpdateAvailable = FALSE;
#ifdef UPDATES_VERBOSE_OUTPUT
                            printf(
                                "[Updates] In order to install this update for the product \""
                                PRODUCT_NAME
                                "\", please allow the request.\n"
                            );
#endif

                            if (*toast)
                            {
                                if (notifier)
                                {
                                    notifier->lpVtbl->Hide(notifier, *toast);
                                }
                                (*toast)->lpVtbl->Release((*toast));
                                (*toast) = NULL;
                            }

                            BOOL bHasErrored = FALSE;
                            BOOL bIsUACEnabled = FALSE;
                            DWORD(*CheckElevationEnabled)(BOOL*) = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CheckElevationEnabled");
                            if (CheckElevationEnabled) CheckElevationEnabled(&bIsUACEnabled);
                            DWORD dwData = FALSE, dwSize = sizeof(DWORD);
                            RegGetValueW(
                                HKEY_LOCAL_MACHINE,
                                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                                L"ConsentPromptBehaviorAdmin",
                                RRF_RT_DWORD,
                                NULL,
                                &dwData,
                                &dwSize
                            );
                            LPWSTR wszSID = NULL;
                            HANDLE hMyToken = NULL;
                            if (OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hMyToken))
                            {
                                PTOKEN_USER ptu = NULL;
                                DWORD dwSize = 0;
                                if (!GetTokenInformation(hMyToken, TokenUser, NULL, 0, &dwSize)
                                    && ERROR_INSUFFICIENT_BUFFER == GetLastError())
                                {
                                    if (NULL != (ptu = (PTOKEN_USER)LocalAlloc(LPTR, dwSize)))
                                    {
                                        if (!GetTokenInformation(hMyToken, TokenUser, ptu, dwSize, &dwSize))
                                        {
                                            LocalFree((HLOCAL)ptu);
                                            return FALSE;
                                        }
                                        ConvertSidToStringSidW(ptu->User.Sid, &wszSID);
                                        LocalFree((HLOCAL)ptu);
                                    }
                                }
                                CloseHandle(hMyToken);
                            }
                            size_t lnSID = (wszSID ? wcslen(wszSID) : 0);
                            DWORD bIsBuiltInAdministratorInApprovalMode = FALSE;
                            BOOL bIsBuiltInAdministratorAccount =
                                IsUserAnAdmin() &&
                                lnSID && !_wcsnicmp(wszSID, L"S-1-5-", 6) && wszSID[lnSID - 4] == L'-' && wszSID[lnSID - 3] == L'5' && wszSID[lnSID - 2] == L'0' && wszSID[lnSID - 1] == L'0';
                            if (bIsBuiltInAdministratorAccount)
                            {
                                DWORD dwSSize = sizeof(DWORD);
                                RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", L"FilterAdministratorToken", RRF_RT_DWORD, NULL, &bIsBuiltInAdministratorInApprovalMode, &dwSSize);
                            }
                            LocalFree(wszSID);
                            if (!bIsUACEnabled || !dwData || (bIsBuiltInAdministratorAccount && !bIsBuiltInAdministratorInApprovalMode))
                            {
                                WCHAR wszURL2[MAX_PATH];
                                ZeroMemory(wszURL2, MAX_PATH * sizeof(WCHAR));
                                MultiByteToWideChar(
                                    CP_UTF8,
                                    MB_PRECOMPOSED,
                                    url,
                                    -1,
                                    wszURL2,
                                    MAX_PATH
                                );

                                WCHAR wszMsg[500];
                                wszMsg[0] = 0;

                                WCHAR wszMsgFormat[500];
                                EP_L10N_ApplyPreferredLanguageForCurrentThread();
                                HMODULE hEPGui = LoadGuiModule();
                                if (LoadStringW(hEPGui, IDS_UPDATES_PROMPT, wszMsgFormat, ARRAYSIZE(wszMsgFormat)))
                                {
                                    swprintf_s(wszMsg, ARRAYSIZE(wszMsg), wszMsgFormat, wszURL2);
                                }
                                FreeLibrary(hEPGui);

                                if (MessageBoxW(
                                    FindWindowW(L"ExplorerPatcher_GUI_" _T(EP_CLSID), NULL),
                                    wszMsg,
                                    _T(PRODUCT_NAME),
                                    MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION
                                ) == IDNO)
                                {
                                    bHasErrored = TRUE;
                                    SetLastError(ERROR_CANCELLED);
                                }
                            }

                            SHELLEXECUTEINFO ShExecInfo = { 0 };
                            ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
                            ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
                            ShExecInfo.hwnd = NULL;
                            ShExecInfo.lpVerb = bIsUsingEpMake ? L"open" : L"runas";
                            ShExecInfo.lpFile = wszPath;
                            ShExecInfo.lpParameters = bIsUsingEpMake ? L"iex (irm 'https://raw.githubusercontent.com/valinet/ep_make/master/ep_make_safe.ps1')" : L"/update_silent";
                            ShExecInfo.lpDirectory = NULL;
                            ShExecInfo.nShow = SW_SHOW;
                            ShExecInfo.hInstApp = NULL;
                            if (!bHasErrored && ShellExecuteExW(&ShExecInfo) && ShExecInfo.hProcess)
                            {
                                WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
                                DWORD dwExitCode = 0;
                                if (GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode) && !dwExitCode)
                                {
                                    bIsUpdateAvailable = TRUE;
#ifdef UPDATES_VERBOSE_OUTPUT
                                    printf("[Updates] Update successful, File Explorer will probably restart momentarily.\n");
#endif
                                }
                                else
                                {
                                    SetLastError(dwExitCode);
#ifdef UPDATES_VERBOSE_OUTPUT
                                    printf("[Updates] Update failed because the following error has occured: %d.\n", dwExitCode);
#endif
                                }
                                CloseHandle(ShExecInfo.hProcess);
                            }
                            else
                            {
                                DWORD dwError = GetLastError();
                                if (dwError == ERROR_CANCELLED)
                                {
#ifdef UPDATES_VERBOSE_OUTPUT
                                    printf("[Updates] Update failed because the request was denied.\n");
#endif
                                }
                                else
                                {
#ifdef UPDATES_VERBOSE_OUTPUT
                                    printf("[Updates] Update failed because the following error has occured: %d.\n", GetLastError());
#endif
                                }
                            }
                        }
                    }
                }
            }
            InternetCloseHandle(hConnect);
        }
        else
        {
            if (lpFail) *lpFail = TRUE;
        }
        InternetCloseHandle(hInternet);
    }

    CloseHandle(params.hEvent);

    if (staging_buffer)
    {
        free(staging_buffer);
        staging_buffer = NULL;
    }

    return bIsUpdateAvailable;
}

BOOL IsUpdateAvailable(LPCWSTR wszDataStore, char* szCheckAgainst, BOOL* lpFail, WCHAR* wszInfoURL, DWORD dwInfoURLLen, HMODULE hModule,
    DWORD* pLeftMost, DWORD* pSecondLeft, DWORD* pSecondRight, DWORD* pRightMost
)
{
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    DWORD dwQueriedPolicy = 0;
    BOOL bIsPolicyMatch = FALSE;
    CHAR szUpdateURL[MAX_PATH];
    ZeroMemory(szUpdateURL, MAX_PATH * sizeof(CHAR));
    strcat_s(szUpdateURL, MAX_PATH, UPDATES_RELEASE_INFO_URL_STABLE);
#ifdef UPDATES_VERBOSE_OUTPUT
    printf("[Updates] Checking against hash \"%s\"\n", szCheckAgainst);
#endif
    DWORD dwUpdateTimeout = UPDATES_DEFAULT_TIMEOUT;
    DWORD bUpdatePreferStaging = FALSE;

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        wszDataStore,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
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
        dwSize = MAX_PATH;
        RegQueryValueExA(
            hKey,
            "UpdateURL",
            0,
            NULL,
            szUpdateURL,
            &dwSize
        );
        if (dwSize == 1 && szUpdateURL[0] == 0)
        {
            strcat_s(szUpdateURL, MAX_PATH, UPDATES_RELEASE_INFO_URL_STABLE);
        }
        strcat_s(szUpdateURL, MAX_PATH, "/download/");
        strcat_s(szUpdateURL, MAX_PATH, SETUP_UTILITY_NAME);
        if (wszInfoURL)
        {
            dwSize = dwInfoURLLen;
            RegQueryValueExW(
                hKey,
                L"UpdateURL",
                0,
                NULL,
                wszInfoURL,
                &dwSize
            );
            if (dwSize == 1 && wszInfoURL[0] == 0)
            {
                wcscat_s(wszInfoURL, dwInfoURLLen, _T(UPDATES_RELEASE_INFO_URL_STABLE));
            }
        }
        dwSize = sizeof(DWORD);
        RegQueryValueExA(
            hKey,
            "UpdateTimeout",
            0,
            NULL,
            &dwUpdateTimeout,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExA(
            hKey,
            "UpdatePreferStaging",
            0,
            NULL,
            &bUpdatePreferStaging,
            &dwSize
        );
        if (bUpdatePreferStaging)
        {
            ZeroMemory(szUpdateURL, MAX_PATH * sizeof(CHAR));
            strcat_s(szUpdateURL, MAX_PATH, UPDATES_RELEASE_INFO_URL_STAGING);
            dwSize = MAX_PATH;
            RegQueryValueExA(
                hKey,
                "UpdateURLStaging",
                0,
                NULL,
                szUpdateURL,
                &dwSize
            );
            if (dwSize == 1 && szUpdateURL[0] == 0)
            {
                strcat_s(szUpdateURL, MAX_PATH, UPDATES_RELEASE_INFO_URL_STAGING);
            }
        }
        RegCloseKey(hKey);
    }
#ifdef UPDATES_VERBOSE_OUTPUT
    printf("[Updates] Update URL: %s\n", szUpdateURL);
#endif
    return IsUpdateAvailableHelper(
        szUpdateURL, 
        szCheckAgainst, 
        dwUpdateTimeout, 
        lpFail,
        NULL, NULL, NULL, 
        bUpdatePreferStaging,
        wszInfoURL,
        dwInfoURLLen,
        hModule,
        pLeftMost, pSecondLeft, pSecondRight, pRightMost
    );
}

BOOL UpdateProduct(
    LPCWSTR wszDataStore,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotifier* notifier,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotificationFactory* notifFactory,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotification** toast,
    HMODULE hModule
)
{
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    DWORD dwQueriedPolicy = 0;
    BOOL bIsPolicyMatch = FALSE;
    CHAR szUpdateURL[MAX_PATH];
    ZeroMemory(szUpdateURL, MAX_PATH * sizeof(CHAR));
    strcat_s(szUpdateURL, MAX_PATH, UPDATES_RELEASE_INFO_URL_STABLE);

    DWORD dwUpdateTimeout = UPDATES_DEFAULT_TIMEOUT;
    DWORD bUpdatePreferStaging = FALSE;

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        wszDataStore,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
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
        dwSize = MAX_PATH;
        RegQueryValueExA(
            hKey,
            "UpdateURL",
            0,
            NULL,
            szUpdateURL,
            &dwSize
        );
        if (dwSize == 1 && szUpdateURL[0] == 0)
        {
            strcat_s(szUpdateURL, MAX_PATH, UPDATES_RELEASE_INFO_URL_STABLE);
        }
        strcat_s(szUpdateURL, MAX_PATH, "/download/");
        strcat_s(szUpdateURL, MAX_PATH, SETUP_UTILITY_NAME);
        dwSize = sizeof(DWORD);
        RegQueryValueExA(
            hKey,
            "UpdateTimeout",
            0,
            NULL,
            &dwUpdateTimeout,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExA(
            hKey,
            "UpdatePreferStaging",
            0,
            NULL,
            &bUpdatePreferStaging,
            &dwSize
        );
        if (bUpdatePreferStaging)
        {
            ZeroMemory(szUpdateURL, MAX_PATH * sizeof(CHAR));
            strcat_s(szUpdateURL, MAX_PATH, UPDATES_RELEASE_INFO_URL_STAGING);
            dwSize = MAX_PATH;
            RegQueryValueExA(
                hKey,
                "UpdateURLStaging",
                0,
                NULL,
                szUpdateURL,
                &dwSize
            );
            if (dwSize == 1 && szUpdateURL[0] == 0)
            {
                strcat_s(szUpdateURL, MAX_PATH, UPDATES_RELEASE_INFO_URL_STAGING);
            }
        }
        RegCloseKey(hKey);
    }
#ifdef UPDATES_VERBOSE_OUTPUT
    printf("[Updates] Update URL: %s\n", szUpdateURL);
#endif
    return IsUpdateAvailableHelper(
        szUpdateURL, 
        NULL, 
        dwUpdateTimeout, 
        NULL,
        notifier,
        notifFactory,
        toast,
        bUpdatePreferStaging,
        NULL, 0,
        hModule,
        NULL, NULL, NULL, NULL
    );
}

BOOL ShowUpdateSuccessNotification(
    HMODULE hModule,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotifier* notifier,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotificationFactory* notifFactory,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotification** toast
)
{
    EP_L10N_ApplyPreferredLanguageForCurrentThread();
    HMODULE hEPGui = LoadGuiModule();

    wchar_t buf[TOAST_BUFSIZ];
    DWORD dwLeftMost = 0;
    DWORD dwSecondLeft = 0;
    DWORD dwSecondRight = 0;
    DWORD dwRightMost = 0;
    QueryVersionInfo(hModule, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

    const wchar_t text[] =
        L"<toast scenario=\"reminder\" activationType=\"protocol\" launch=\"%s\" duration=\"%s\">\r\n"
        L"	<visual>\r\n"
        L"		<binding template=\"ToastGeneric\">\r\n"
        L"			<text><![CDATA[%s]]></text>\r\n"
        L"			<text><![CDATA[%s]]></text>\r\n"
        L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
        L"		</binding>\r\n"
        L"	</visual>\r\n"
        L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
        L"</toast>\r\n";
    wchar_t title[100];
    wchar_t body[200];
    title[0] = 0; body[0] = 0;

    LoadStringW(hEPGui, IDS_UPDATES_SUCCESS_T, title, ARRAYSIZE(title));

    wchar_t bodyFormat[200];
    ZeroMemory(bodyFormat, sizeof(bodyFormat));
    if (LoadStringW(hEPGui, IDS_UPDATES_INSTALLEDVER, bodyFormat, ARRAYSIZE(bodyFormat)))
    {
        swprintf_s(body, ARRAYSIZE(body), bodyFormat, dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
    }

    swprintf_s(buf, TOAST_BUFSIZ, text, _T(UPDATES_RELEASE_INFO_URL), L"short", title, body);
    __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
    String2IXMLDocument(
        buf,
        wcslen(buf),
        &inputXml,
        NULL
    );
    if (*toast)
    {
        if (notifier)
        {
            notifier->lpVtbl->Hide(notifier, *toast);
        }
        (*toast)->lpVtbl->Release((*toast));
        (*toast) = NULL;
    }
    if (notifFactory)
    {
        notifFactory->lpVtbl->CreateToastNotification(notifFactory, inputXml, toast);
    }
    if ((*toast) && notifier)
    {
        notifier->lpVtbl->Show(notifier, *toast);
    }
    if (inputXml)
    {
        inputXml->lpVtbl->Release(inputXml);
    }
    if (hEPGui)
    {
        FreeLibrary(hEPGui);
    }
    SwitchToThread();

    return TRUE;
}

BOOL InstallUpdatesIfAvailable(
    HMODULE hModule,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotifier* notifier,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotificationFactory* notifFactory,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotification** toast,
    DWORD dwOperation, 
    DWORD bAllocConsole, 
    DWORD dwUpdatePolicy
)
{
    EP_L10N_ApplyPreferredLanguageForCurrentThread();
    HMODULE hEPGui = LoadGuiModule();

    wchar_t wszInfoURL[MAX_PATH];
    ZeroMemory(wszInfoURL, MAX_PATH * sizeof(wchar_t));
    wcscat_s(wszInfoURL, MAX_PATH, _T(UPDATES_RELEASE_INFO_URL_STABLE));
    wchar_t buf[TOAST_BUFSIZ];
    DWORD dwLeftMost = 0;
    DWORD dwSecondLeft = 0;
    DWORD dwSecondRight = 0;
    DWORD dwRightMost = 0;
    QueryVersionInfo(hModule, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

    if (bAllocConsole)
    {
        switch (dwUpdatePolicy)
        {
        default:
        case UPDATE_POLICY_AUTO:
        {
            dwUpdatePolicy = UPDATE_POLICY_AUTO;
            printf("[Updates] Configured update policy on this system: \"Install updates automatically\".\n");
            break;
        }
        case UPDATE_POLICY_NOTIFY:
        {
            printf("[Updates] Configured update policy on this system: \"Check for updates but let me choose whether to download and install them\".\n");
            break;
        }
        case UPDATE_POLICY_MANUAL:
        {
            printf("[Updates] Configured update policy on this system: \"Manually check for updates\".\n");
            break;
        }
        }
    }

    const wchar_t text[] =
        L"<toast scenario=\"reminder\" activationType=\"protocol\" launch=\"%s\" duration=\"%s\">\r\n"
        L"	<visual>\r\n"
        L"		<binding template=\"ToastGeneric\">\r\n"
        L"			<text><![CDATA[%s]]></text>\r\n"
        L"			<text><![CDATA[%s]]></text>\r\n"
        L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
        L"		</binding>\r\n"
        L"	</visual>\r\n"
        L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
        L"</toast>\r\n";
    wchar_t title[100];
    wchar_t body[200];
    title[0] = 0; body[0] = 0;

    if (dwOperation == UPDATES_OP_INSTALL)
    {
        LoadStringW(hEPGui, IDS_UPDATES_DOWNLOADING_T, title, ARRAYSIZE(title));
    }
    else if (dwOperation == UPDATES_OP_CHECK)
    {
        LoadStringW(hEPGui, IDS_UPDATES_CHECKING_T, title, ARRAYSIZE(title));
    }

    wchar_t bodyFormat[200];
    ZeroMemory(bodyFormat, sizeof(bodyFormat));
    if (LoadStringW(hEPGui, IDS_UPDATES_INSTALLEDVER, bodyFormat, ARRAYSIZE(bodyFormat)))
    {
        swprintf_s(body, ARRAYSIZE(body), bodyFormat, dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
    }

    swprintf_s(buf, TOAST_BUFSIZ, text, _T(UPDATES_RELEASE_INFO_URL), L"long", title, body);

    __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
    String2IXMLDocument(
        buf,
        wcslen(buf),
        &inputXml,
        NULL
    );

    if (dwOperation == UPDATES_OP_CHECK || dwOperation == UPDATES_OP_INSTALL)
    {
        if (*toast)
        {
            if (notifier)
            {
                notifier->lpVtbl->Hide(notifier, *toast);
            }
            (*toast)->lpVtbl->Release((*toast));
            (*toast) = NULL;
        }
        if (notifFactory)
        {
            notifFactory->lpVtbl->CreateToastNotification(notifFactory, inputXml, toast);
        }
        if ((*toast) && notifier)
        {
            notifier->lpVtbl->Show(notifier, *toast);
        }
        if (inputXml)
        {
            inputXml->lpVtbl->Release(inputXml);
        }
    }

    WCHAR dllName[MAX_PATH];
    GetModuleFileNameW(hModule, dllName, MAX_PATH);
    wprintf(L"[Updates] Path to module: %s\n", dllName);

    CHAR hash[100];
    ZeroMemory(hash, 100 * sizeof(CHAR));
    GetHardcodedHash(dllName, hash, 100);
    if (!strcmp(hash, "This")) ComputeFileHash2(hModule, dllName, hash, 100);
    else printf("[Updates] Using hardcoded hash.\n");

    BOOL bFail = FALSE, bReturnValue = FALSE;
    dwLeftMost = 0; dwSecondLeft = 0; dwSecondRight = 0; dwRightMost = 0;
    if (IsUpdateAvailable(_T(REGPATH), hash, &bFail, wszInfoURL, MAX_PATH, hModule, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost))
    {
        printf("[Updates] An update is available.\n");
        if ((dwOperation == UPDATES_OP_DEFAULT && dwUpdatePolicy == UPDATE_POLICY_AUTO) || (dwOperation == UPDATES_OP_INSTALL))
        {
            __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
            BOOL bOk = UpdateProduct(_T(REGPATH), notifier, notifFactory, toast, hModule);
            if (!bOk)
            {
                if (dwOperation == UPDATES_OP_INSTALL)
                {
                    title[0] = 0; body[0] = 0;
                    LoadStringW(hEPGui, IDS_UPDATES_DLFAILED_T, title, ARRAYSIZE(title));
                    LoadStringW(hEPGui, IDS_UPDATES_DLFAILED_B, body, ARRAYSIZE(body));

                    swprintf_s(buf, TOAST_BUFSIZ, text, _T(UPDATES_RELEASE_INFO_URL), L"short", title, body);

                    String2IXMLDocument(
                        buf,
                        wcslen(buf),
                        &inputXml,
                        NULL
                    );
                }
            }
            if (bOk || (!bOk && (dwOperation == UPDATES_OP_INSTALL)))
            {
                if (*toast)
                {
                    if (notifier)
                    {
                        notifier->lpVtbl->Hide(notifier, *toast);
                    }
                    (*toast)->lpVtbl->Release((*toast));
                    (*toast) = NULL;
                }
                if (notifFactory)
                {
                    notifFactory->lpVtbl->CreateToastNotification(notifFactory, inputXml, toast);
                }
                if ((*toast) && notifier)
                {
                    notifier->lpVtbl->Show(notifier, *toast);
                }
                if (inputXml)
                {
                    inputXml->lpVtbl->Release(inputXml);
                }
            }
        }
        else if ((dwOperation == UPDATES_OP_DEFAULT && dwUpdatePolicy == UPDATE_POLICY_NOTIFY) || (dwOperation == UPDATES_OP_CHECK))
        {
            title[0] = 0; body[0] = 0;

            if (!dwLeftMost)
            {
                LoadStringW(hEPGui, IDS_UPDATES_AVAILABLE_T_U, title, ARRAYSIZE(title));
            }
            else
            {
                WCHAR titleFormat[100];
                if (LoadStringW(hEPGui, IDS_UPDATES_AVAILABLE_T, titleFormat, ARRAYSIZE(titleFormat)))
                {
                    swprintf_s(title, ARRAYSIZE(title), titleFormat, dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
                }
            }

            LoadStringW(hEPGui, IDS_UPDATES_AVAILABLE_B, body, ARRAYSIZE(body));

            swprintf_s(buf, TOAST_BUFSIZ, text, wszInfoURL, L"long", title, body);

            __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
            String2IXMLDocument(
                buf,
                wcslen(buf),
                &inputXml,
                NULL
            );
            if (*toast)
            {
                if (notifier)
                {
                    notifier->lpVtbl->Hide(notifier, *toast);
                }
                (*toast)->lpVtbl->Release((*toast));
                (*toast) = NULL;
            }
            if (notifFactory)
            {
                notifFactory->lpVtbl->CreateToastNotification(notifFactory, inputXml, toast);
            }
            if ((*toast) && notifier)
            {
                notifier->lpVtbl->Show(notifier, *toast);
            }
            if (inputXml)
            {
                inputXml->lpVtbl->Release(inputXml);
            }
        }
        bReturnValue = TRUE;
    }
    else
    {
        if (bFail)
        {
            printf("[Updates] Unable to check for updates because the remote server is unavailable.\n");
        }
        else
        {
            printf("[Updates] No updates are available.\n");
        }
        if (dwOperation == UPDATES_OP_CHECK || dwOperation == UPDATES_OP_INSTALL)
        {
            title[0] = 0; body[0] = 0;
            if (bFail)
            {
                LoadStringW(hEPGui, IDS_UPDATES_CHECKFAILED_T, title, ARRAYSIZE(title));
                LoadStringW(hEPGui, IDS_UPDATES_CHECKFAILED_B, body, ARRAYSIZE(body));
            }
            else
            {
                LoadStringW(hEPGui, IDS_UPDATES_ISLATEST_T, title, ARRAYSIZE(title));
                LoadStringW(hEPGui, IDS_UPDATES_ISLATEST_B, body, ARRAYSIZE(body));
            }

            swprintf_s(buf, TOAST_BUFSIZ, text, _T(UPDATES_RELEASE_INFO_URL), L"short", title, body);

            __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
            String2IXMLDocument(
                buf,
                wcslen(buf),
                &inputXml,
                NULL
            );
            if (*toast)
            {
                if (notifier)
                {
                    notifier->lpVtbl->Hide(notifier, *toast);
                }
                (*toast)->lpVtbl->Release((*toast));
                (*toast) = NULL;
            }
            if (notifFactory)
            {
                notifFactory->lpVtbl->CreateToastNotification(notifFactory, inputXml, toast);
            }
            if ((*toast) && notifier)
            {
                notifier->lpVtbl->Show(notifier, *toast);
            }
            if (inputXml)
            {
                inputXml->lpVtbl->Release(inputXml);
            }
        }
        bReturnValue = FALSE;
    }

    if (hEPGui)
    {
        FreeLibrary(hEPGui);
    }

    return bReturnValue;
}
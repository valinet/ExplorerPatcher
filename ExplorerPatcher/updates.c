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
    DWORD dwInfoURLLen
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
                    if (hash[0] == 0x4D && hash[1] == 0x5A && _stricmp(DOSMODE_OFFSET + hash, szCheckAgainst))
                    {
                        bIsUpdateAvailable = TRUE;
                    }
                }
                else
                {
#ifdef UPDATES_VERBOSE_OUTPUT
                    printf("[Updates] Failed. Read %d bytes.\n");
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
                        FILE* f = NULL;
                        if (!_wfopen_s(
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
                        if (bIsUpdateAvailable)
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
                            if (!bIsUACEnabled || !dwData)
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
                                swprintf_s(wszMsg, 500, L"Would you like to install an update for " _T(PRODUCT_NAME) L"?\n\nDownloaded from:\n%s", wszURL2);
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
                            ShExecInfo.lpVerb = L"runas";
                            ShExecInfo.lpFile = wszPath;
                            ShExecInfo.lpParameters = L"/update_silent";
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
                                    printf("[Updates] Update successful, File Explorer will probably restart momentarly.\n");
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

BOOL IsUpdateAvailable(LPCWSTR wszDataStore, char* szCheckAgainst, BOOL* lpFail, WCHAR* wszInfoURL, DWORD dwInfoURLLen)
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
        dwInfoURLLen
    );
}

BOOL UpdateProduct(
    LPCWSTR wszDataStore,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotifier* notifier,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotificationFactory* notifFactory,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotification** toast
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
        NULL, 0
    );
}

BOOL ShowUpdateSuccessNotification(
    HMODULE hModule,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotifier* notifier,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotificationFactory* notifFactory,
    __x_ABI_CWindows_CUI_CNotifications_CIToastNotification** toast
)
{
    wchar_t buf[TOAST_BUFSIZ];
    DWORD dwLeftMost = 0;
    DWORD dwSecondLeft = 0;
    DWORD dwSecondRight = 0;
    DWORD dwRightMost = 0;
    QueryVersionInfo(hModule, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

    __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
    const wchar_t text[] =
        L"<toast scenario=\"reminder\" "
        L"activationType=\"protocol\" launch=\"" _T(UPDATES_RELEASE_INFO_URL) L"\" duration=\"short\">\r\n"
        L"	<visual>\r\n"
        L"		<binding template=\"ToastGeneric\">\r\n"
        L"			<text><![CDATA[Update successful]]></text>\r\n"
        L"			<text><![CDATA[Installed version: %d.%d.%d.%d]]></text>\r\n"
        L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
        L"		</binding>\r\n"
        L"	</visual>\r\n"
        L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
        L"</toast>\r\n";
    swprintf_s(buf, TOAST_BUFSIZ, text, dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
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

    SwitchToThread();
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

    __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
    if (dwOperation == UPDATES_OP_INSTALL)
    {
        const wchar_t text[] =
            L"<toast scenario=\"reminder\" "
            L"activationType=\"protocol\" launch=\"" _T(UPDATES_RELEASE_INFO_URL) L"\" duration=\"long\">\r\n"
            L"	<visual>\r\n"
            L"		<binding template=\"ToastGeneric\">\r\n"
            L"			<text><![CDATA[Downloading and installing updates]]></text>\r\n"
            L"			<text><![CDATA[Installed version: %d.%d.%d.%d]]></text>\r\n"
            L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
            L"		</binding>\r\n"
            L"	</visual>\r\n"
            L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
            L"</toast>\r\n";
        swprintf_s(buf, TOAST_BUFSIZ, text, dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
        String2IXMLDocument(
            buf,
            wcslen(buf),
            &inputXml,
            NULL
        );
    }
    else if (dwOperation == UPDATES_OP_CHECK)
    {
        const wchar_t text[] =
            L"<toast scenario=\"reminder\" "
            L"activationType=\"protocol\" launch=\"" _T(UPDATES_RELEASE_INFO_URL) L"\" duration=\"long\">\r\n"
            L"	<visual>\r\n"
            L"		<binding template=\"ToastGeneric\">\r\n"
            L"			<text><![CDATA[Checking for updates]]></text>\r\n"
            L"			<text><![CDATA[Installed version: %d.%d.%d.%d]]></text>\r\n"
            L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
            L"		</binding>\r\n"
            L"	</visual>\r\n"
            L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
            L"</toast>\r\n";
        swprintf_s(buf, TOAST_BUFSIZ, text, dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
        String2IXMLDocument(
            buf,
            wcslen(buf),
            &inputXml,
            NULL
        );
    }

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
    ComputeFileHash(dllName, hash, 100);

    BOOL bFail = FALSE;
    if (IsUpdateAvailable(_T(REGPATH), hash, &bFail, wszInfoURL, MAX_PATH))
    {
        printf("[Updates] An update is available.\n");
        if ((dwOperation == UPDATES_OP_DEFAULT && dwUpdatePolicy == UPDATE_POLICY_AUTO) || (dwOperation == UPDATES_OP_INSTALL))
        {
            __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
            BOOL bOk = UpdateProduct(_T(REGPATH), notifier, notifFactory, toast);
            if (!bOk)
            {
                if (dwOperation == UPDATES_OP_INSTALL)
                {
                    const wchar_t text[] =
                        L"<toast scenario=\"reminder\" "
                        L"activationType=\"protocol\" launch=\"" _T(UPDATES_RELEASE_INFO_URL) L"\" duration=\"short\">\r\n"
                        L"	<visual>\r\n"
                        L"		<binding template=\"ToastGeneric\">\r\n"
                        L"			<text><![CDATA[Update failed]]></text>\r\n"
                        L"			<text><![CDATA[The request was declined or an error has occured when attempting to install this update.]]></text>\r\n"
                        L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
                        L"		</binding>\r\n"
                        L"	</visual>\r\n"
                        L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
                        L"</toast>\r\n";
                    String2IXMLDocument(
                        text,
                        wcslen(text),
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
            const wchar_t text[] =
                L"<toast scenario=\"reminder\" "
                L"activationType=\"protocol\" launch=\"%s\" duration=\"long\">\r\n"
                L"	<visual>\r\n"
                L"		<binding template=\"ToastGeneric\">\r\n"
                L"			<text><![CDATA[New version available]]></text>\r\n"
                L"			<text><![CDATA[You can update by right clicking the taskbar, choosing \"Properties\", then \"Updates\". Click here to learn more about this update.]]></text>\r\n"
                L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
                L"		</binding>\r\n"
                L"	</visual>\r\n"
                L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
                L"</toast>\r\n";
            swprintf_s(buf, TOAST_BUFSIZ, text, wszInfoURL);
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

        return TRUE;
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
            const wchar_t text[] =
                L"<toast scenario=\"reminder\" "
                L"activationType=\"protocol\" launch=\"" _T(UPDATES_RELEASE_INFO_URL) L"\" duration=\"short\">\r\n"
                L"	<visual>\r\n"
                L"		<binding template=\"ToastGeneric\">\r\n"
                L"			<text><![CDATA[No updates are available]]></text>\r\n"
                L"			<text><![CDATA[Please check back later.]]></text>\r\n"
                L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
                L"		</binding>\r\n"
                L"	</visual>\r\n"
                L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
                L"</toast>\r\n";
            const wchar_t text2[] =
                L"<toast scenario=\"reminder\" "
                L"activationType=\"protocol\" launch=\"" _T(UPDATES_RELEASE_INFO_URL) L"\" duration=\"short\">\r\n"
                L"	<visual>\r\n"
                L"		<binding template=\"ToastGeneric\">\r\n"
                L"			<text><![CDATA[Unable to check for updates]]></text>\r\n"
                L"			<text><![CDATA[Make sure that you are connected to the Internet and that the remote server is online.]]></text>\r\n"
                L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
                L"		</binding>\r\n"
                L"	</visual>\r\n"
                L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
                L"</toast>\r\n";
            __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
            String2IXMLDocument(
                bFail ? text2 : text,
                wcslen(bFail ? text2 : text),
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
        return FALSE;
    }
}
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

BOOL IsUpdateAvailableHelper(char* url, char* szCheckAgainst, DWORD dwUpdateTimeout, BOOL* lpFail)
{
    BOOL bIsUpdateAvailable = FALSE;

    struct IsUpdateAvailableParameters params;
    params.hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (!params.hEvent)
    {
        return bIsUpdateAvailable;
    }

    HINTERNET hInternet = NULL;
    if (hInternet = InternetOpenA(
        UPDATES_USER_AGENT,
        INTERNET_OPEN_TYPE_PRECONFIG,
        NULL,
        NULL,
        INTERNET_FLAG_ASYNC
    ))
    {
        InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &dwUpdateTimeout, sizeof(DWORD));
        if (InternetSetStatusCallbackA(hInternet, IsUpdateAvailableHelperCallback) != INTERNET_INVALID_STATUS_CALLBACK)
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
                INTERNET_FLAG_NO_CACHE_WRITE,
                &params
            );
            if (!hConnect && GetLastError() == ERROR_IO_PENDING)
            {
                if (WaitForSingleObject(params.hEvent, dwUpdateTimeout) == WAIT_OBJECT_0)
                {
                    hConnect = params.hInternet;
                }
            }
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
                        if (lpFail) *lpFail = TRUE;
                    }
                }
                else
                {
                    WCHAR wszPath[MAX_PATH];
                    ZeroMemory(wszPath, MAX_PATH * sizeof(WCHAR));
                    SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
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
                        if (wszURL[95])
                        {
                            wszURL[94] = L'.';
                            wszURL[95] = L'.';
                            wszURL[96] = L'.';
                            wszURL[97] = L'e';
                            wszURL[98] = L'x';
                            wszURL[99] = L'e';
                            wszURL[100] = 0;
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
                                    "\", please allow the elevation request.\n"
                                );
#endif
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
                                if (ShellExecuteExW(&ShExecInfo) && ShExecInfo.hProcess)
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
                                        printf("[Updates] Update failed because the elevation request was denied.\n");
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
        }
        InternetCloseHandle(hInternet);
    }

    CloseHandle(params.hEvent);

    return bIsUpdateAvailable;
}

BOOL IsUpdateAvailable(LPCWSTR wszDataStore, char* szCheckAgainst, BOOL* lpFail)
{
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    DWORD dwQueriedPolicy = 0;
    BOOL bIsPolicyMatch = FALSE;
    CHAR szUpdateURL[MAX_PATH];
    ZeroMemory(szUpdateURL, MAX_PATH * sizeof(CHAR));
    strcat_s(szUpdateURL, MAX_PATH, "https://github.com/valinet/ExplorerPatcher/releases/latest/download/");
#ifdef UPDATES_VERBOSE_OUTPUT
    printf("[Updates] Checking against hash \"%s\"\n", szCheckAgainst);
#endif
    DWORD dwUpdateTimeout = UPDATES_DEFAULT_TIMEOUT;

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
        RegCloseKey(hKey);
    }
#ifdef UPDATES_VERBOSE_OUTPUT
    printf("[Updates] Update URL: %s\n", szUpdateURL);
#endif
    return IsUpdateAvailableHelper(szUpdateURL, szCheckAgainst, dwUpdateTimeout, lpFail);
}

BOOL UpdateProduct(LPCWSTR wszDataStore)
{
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    DWORD dwQueriedPolicy = 0;
    BOOL bIsPolicyMatch = FALSE;
    CHAR szUpdateURL[MAX_PATH];
    ZeroMemory(szUpdateURL, MAX_PATH * sizeof(CHAR));
    strcat_s(szUpdateURL, MAX_PATH, "https://github.com/valinet/ExplorerPatcher/releases/latest/download/");

    DWORD dwUpdateTimeout = UPDATES_DEFAULT_TIMEOUT;

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
        RegCloseKey(hKey);
    }
#ifdef UPDATES_VERBOSE_OUTPUT
    printf("[Updates] Update URL: %s\n", szUpdateURL);
#endif
    return IsUpdateAvailableHelper(szUpdateURL, NULL, dwUpdateTimeout, NULL);
}

BOOL InstallUpdatesIfAvailable(DWORD dwOperation, DWORD bAllocConsole, DWORD dwUpdatePolicy)
{
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

    if (dwOperation == UPDATES_OP_INSTALL)
    {
        const wchar_t UpdateAvailableXML[] =
            L"<toast displayTimestamp=\"2021-08-29T00:00:00.000Z\" scenario=\"reminder\" "
            L"activationType=\"protocol\" launch=\"https://github.com/valinet/ExplorerPatcher/releases/latest\" duration=\"short\">\r\n"
            L"	<visual>\r\n"
            L"		<binding template=\"ToastGeneric\">\r\n"
            L"			<text><![CDATA[Downloading and installing updates]]></text>\r\n"
            L"			<text><![CDATA[An installation screen will show as soon as the download completes.]]></text>\r\n"
            L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
            L"		</binding>\r\n"
            L"	</visual>\r\n"
            L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
            L"</toast>\r\n";
        HRESULT hr = S_OK;
        __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
        hr = String2IXMLDocument(
            UpdateAvailableXML,
            wcslen(UpdateAvailableXML),
            &inputXml,
#ifdef DEBUG
            stdout
#else
            NULL
#endif
        );
        hr = ShowToastMessage(
            inputXml,
            APPID,
            sizeof(APPID) / sizeof(TCHAR) - 1,
#ifdef DEBUG
            stdout
#else
            NULL
#endif
        );
    }

    WCHAR dllName[MAX_PATH];
    GetModuleFileNameW(hModule, dllName, MAX_PATH);
    wprintf(L"[Updates] Path to module: %s\n", dllName);

    CHAR hash[100];
    ZeroMemory(hash, 100 * sizeof(CHAR));
    ComputeFileHash(dllName, hash, 100);

    BOOL bFail = FALSE;
    if (IsUpdateAvailable(_T(REGPATH), hash, &bFail))
    {
        printf("[Updates] An update is available.\n");
        if ((dwOperation == UPDATES_OP_DEFAULT && dwUpdatePolicy == UPDATE_POLICY_AUTO) || (dwOperation == UPDATES_OP_INSTALL))
        {
            UpdateProduct(_T(REGPATH));
        }
        else if ((dwOperation == UPDATES_OP_DEFAULT && dwUpdatePolicy == UPDATE_POLICY_NOTIFY) || (dwOperation == UPDATES_OP_CHECK))
        {
            const wchar_t UpdateAvailableXML[] =
                L"<toast displayTimestamp=\"2021-08-29T00:00:00.000Z\" scenario=\"reminder\" "
                L"activationType=\"protocol\" launch=\"https://github.com/valinet/ExplorerPatcher/releases/latest\" duration=\"long\">\r\n"
                L"	<visual>\r\n"
                L"		<binding template=\"ToastGeneric\">\r\n"
                L"			<text><![CDATA[An update is available]]></text>\r\n"
                L"			<text><![CDATA[Right click the taskbar, choose \"Properties\", then \"Updates\" - \"Install latest version\". Click here to learn more about this update.]]></text>\r\n"
                L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
                L"		</binding>\r\n"
                L"	</visual>\r\n"
                L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
                L"</toast>\r\n";
            HRESULT hr = S_OK;
            __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
            hr = String2IXMLDocument(
                UpdateAvailableXML,
                wcslen(UpdateAvailableXML),
                &inputXml,
#ifdef DEBUG
                stdout
#else
                NULL
#endif
            );
            hr = ShowToastMessage(
                inputXml,
                APPID,
                sizeof(APPID) / sizeof(TCHAR) - 1,
#ifdef DEBUG
                stdout
#else
                NULL
#endif
            );
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
            const wchar_t UpdateAvailableXML[] =
                L"<toast displayTimestamp=\"2021-08-29T00:00:00.000Z\" scenario=\"reminder\" "
                L"activationType=\"protocol\" launch=\"https://github.com/valinet/ExplorerPatcher/releases/latest\" duration=\"short\">\r\n"
                L"	<visual>\r\n"
                L"		<binding template=\"ToastGeneric\">\r\n"
                L"			<text><![CDATA[No updates are available]]></text>\r\n"
                L"			<text><![CDATA[Please check back later.]]></text>\r\n"
                L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
                L"		</binding>\r\n"
                L"	</visual>\r\n"
                L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
                L"</toast>\r\n";
            const wchar_t UpdateAvailableXML2[] =
                L"<toast displayTimestamp=\"2021-08-29T00:00:00.000Z\" scenario=\"reminder\" "
                L"activationType=\"protocol\" launch=\"https://github.com/valinet/ExplorerPatcher/releases/latest\" duration=\"short\">\r\n"
                L"	<visual>\r\n"
                L"		<binding template=\"ToastGeneric\">\r\n"
                L"			<text><![CDATA[Unable to check for updates]]></text>\r\n"
                L"			<text><![CDATA[Make sure that you are connected to the Internet and that the remote server is online.]]></text>\r\n"
                L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
                L"		</binding>\r\n"
                L"	</visual>\r\n"
                L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
                L"</toast>\r\n";
            HRESULT hr = S_OK;
            __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
            hr = String2IXMLDocument(
                bFail ? UpdateAvailableXML2 : UpdateAvailableXML,
                wcslen(bFail ? UpdateAvailableXML2 : UpdateAvailableXML),
                &inputXml,
#ifdef DEBUG
                stdout
#else
                NULL
#endif
            );
            hr = ShowToastMessage(
                inputXml,
                APPID,
                sizeof(APPID) / sizeof(TCHAR) - 1,
#ifdef DEBUG
                stdout
#else
                NULL
#endif
            );
        }
        return FALSE;
    }
}
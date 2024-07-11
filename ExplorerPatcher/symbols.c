#include "symbols.h"

const char* explorer_SN[EXPLORER_SB_CNT] = {
    EXPLORER_SB_0,
    EXPLORER_SB_1,
    EXPLORER_SB_2,
    EXPLORER_SB_3,
    EXPLORER_SB_4,
    EXPLORER_SB_5
};
const char* explorer_SN_26244[1] = {
    EXPLORER_SB_4,
};
const char* twinui_pcshell_SN[TWINUI_PCSHELL_SB_CNT] = {
    TWINUI_PCSHELL_SB_0,
    TWINUI_PCSHELL_SB_1,
    TWINUI_PCSHELL_SB_2,
    TWINUI_PCSHELL_SB_3,
    TWINUI_PCSHELL_SB_4,
    TWINUI_PCSHELL_SB_5,
    TWINUI_PCSHELL_SB_6,
    TWINUI_PCSHELL_SB_7,
    TWINUI_PCSHELL_SB_8
};
const char* startdocked_SN[STARTDOCKED_SB_CNT] = {
    STARTDOCKED_SB_0,
    STARTDOCKED_SB_1,
    STARTDOCKED_SB_2,
    STARTDOCKED_SB_3,
    STARTDOCKED_SB_4
};
const char* startui_SN[STARTUI_SB_CNT] = {
    STARTUI_SB_0
};

const wchar_t DownloadNotificationXML[] =
    L"<toast scenario=\"reminder\" "
    L"activationType=\"protocol\" launch=\"%s\" duration=\"%s\">\r\n"
    L"	<visual>\r\n"
    L"		<binding template=\"ToastGeneric\">\r\n"
    L"			<text><![CDATA[%s]]></text>\r\n"
    L"			<text><![CDATA[%s]]></text>\r\n"
    L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
    L"		</binding>\r\n"
    L"	</visual>\r\n"
    L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
    L"</toast>\r\n";

extern INT VnDownloadSymbols(HMODULE hModule, char* dllName, char* szLibPath, UINT sizeLibPath);
extern INT VnGetSymbols(const char* pdb_file, DWORD* addresses, char** symbols, DWORD numOfSymbols);

BOOL CheckVersion(HKEY hKey, DWORD dwVersion)
{
    DWORD dwSize = sizeof(DWORD);
    DWORD dwStoredVersion = 0;
    if (RegQueryValueExW(hKey, TEXT("Version"), 0, NULL, &dwStoredVersion, &dwSize) == ERROR_SUCCESS)
    {
        return dwStoredVersion == dwVersion;
    }
    return FALSE;
}

void SaveVersion(HKEY hKey, DWORD dwVersion)
{
    RegSetValueExW(hKey, TEXT("Version"), 0, REG_DWORD, &dwVersion, sizeof(DWORD));
}

static BOOL ProcessExplorerSymbols(const char* pszSettingsPath, DWORD* pOffsets)
{
    HKEY hKey = NULL;
    DWORD dwDisposition;
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH) L"\\" TEXT(EXPLORER_SB_NAME),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        &dwDisposition
    );
    if (!hKey || hKey == INVALID_HANDLE_VALUE)
    {
        printf("[Symbols] Unable to create registry key.\n");
        return FALSE;
    }

    CHAR szHash[100];
    WCHAR wszPath[MAX_PATH];

    ZeroMemory(szHash, sizeof(szHash));
    ZeroMemory(wszPath, sizeof(wszPath));

    char explorer_sb_dll[MAX_PATH];
    ZeroMemory(explorer_sb_dll, sizeof(explorer_sb_dll));
    GetWindowsDirectoryA(explorer_sb_dll, MAX_PATH);
    strcat_s(explorer_sb_dll, MAX_PATH, "\\" EXPLORER_SB_NAME ".exe");

    GetWindowsDirectoryW(wszPath, MAX_PATH);
    wcscat_s(wszPath, MAX_PATH, L"\\" _T(EXPLORER_SB_NAME) L".exe");
    ComputeFileHash(wszPath, szHash, ARRAYSIZE(szHash));

    printf("[Symbols] Downloading symbols for \"%s\" (\"%s\")...\n", explorer_sb_dll, szHash);
    if (VnDownloadSymbols(
        NULL,
        explorer_sb_dll,
        pszSettingsPath,
        MAX_PATH
    ))
    {
        printf("[Symbols] Symbols for \"%s\" are not available - unable to download.\n", explorer_sb_dll);
        printf("[Symbols] Please refer to \"https://github.com/valinet/ExplorerPatcher/wiki/Symbols\" for more information.\n");
        if (hKey) RegCloseKey(hKey);
        return FALSE;
    }

    printf("[Symbols] Reading symbols...\n");
    if (VnGetSymbols(pszSettingsPath, pOffsets, explorer_SN, ARRAYSIZE(explorer_SN)) != 0)
    {
        DWORD offsets26244[ARRAYSIZE(explorer_SN_26244)];
        if (VnGetSymbols(pszSettingsPath, offsets26244, explorer_SN_26244, ARRAYSIZE(explorer_SN_26244)) == 0)
        {
            pOffsets[4] = offsets26244[0];
        }
        else
        {
            printf("[Symbols] Failure in reading symbols for \"%s\".\n", explorer_sb_dll);
            if (hKey) RegCloseKey(hKey);
            return FALSE;
        }
    }

    RegSetValueExW(hKey, TEXT(EXPLORER_SB_0), 0, REG_DWORD, &pOffsets[0], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(EXPLORER_SB_1), 0, REG_DWORD, &pOffsets[1], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(EXPLORER_SB_2), 0, REG_DWORD, &pOffsets[2], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(EXPLORER_SB_3), 0, REG_DWORD, &pOffsets[3], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(EXPLORER_SB_4), 0, REG_DWORD, &pOffsets[4], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(EXPLORER_SB_5), 0, REG_DWORD, &pOffsets[5], sizeof(DWORD));

    RegSetValueExA(hKey, "Hash", 0, REG_SZ, szHash, strlen(szHash) + 1);
    SaveVersion(hKey, EXPLORER_SB_VERSION);

    if (hKey) RegCloseKey(hKey);
    return TRUE;
}

static BOOL ProcessTwinuiPcshellSymbols(const char* pszSettingsPath, DWORD* pOffsets)
{
    HKEY hKey = NULL;
    DWORD dwDisposition;
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH) L"\\" TEXT(TWINUI_PCSHELL_SB_NAME),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        &dwDisposition
    );
    if (!hKey || hKey == INVALID_HANDLE_VALUE)
    {
        printf("[Symbols] Unable to create registry key.\n");
        return FALSE;
    }

    CHAR szHash[100];
    WCHAR wszPath[MAX_PATH];

    ZeroMemory(szHash, sizeof(szHash));
    ZeroMemory(wszPath, sizeof(wszPath));

    char twinui_pcshell_sb_dll[MAX_PATH];
    ZeroMemory(twinui_pcshell_sb_dll, sizeof(twinui_pcshell_sb_dll));
    GetSystemDirectoryA(twinui_pcshell_sb_dll, MAX_PATH);
    strcat_s(twinui_pcshell_sb_dll, MAX_PATH, "\\");
    strcat_s(twinui_pcshell_sb_dll, MAX_PATH, TWINUI_PCSHELL_SB_NAME);
    strcat_s(twinui_pcshell_sb_dll, MAX_PATH, ".dll");

    GetSystemDirectoryW(wszPath, MAX_PATH);
    wcscat_s(wszPath, MAX_PATH, L"\\" _T(TWINUI_PCSHELL_SB_NAME) L".dll");
    ComputeFileHash(wszPath, szHash, ARRAYSIZE(szHash));

    printf("[Symbols] Downloading symbols for \"%s\" (\"%s\")...\n", twinui_pcshell_sb_dll, szHash);
    if (VnDownloadSymbols(
        NULL,
        twinui_pcshell_sb_dll,
        pszSettingsPath,
        MAX_PATH
    ))
    {
        printf("[Symbols] Symbols for \"%s\" are not available - unable to download.\n", twinui_pcshell_sb_dll);
        printf("[Symbols] Please refer to \"https://github.com/valinet/ExplorerPatcher/wiki/Symbols\" for more information.\n");
        if (hKey) RegCloseKey(hKey);
        return FALSE;
    }

    printf("[Symbols] Reading symbols...\n");
    if (!IsWindows11())
    {
        DWORD flOldProtect = 0;
        if (VirtualProtect(twinui_pcshell_SN, sizeof(twinui_pcshell_SN), PAGE_EXECUTE_READWRITE, &flOldProtect))
        {
            twinui_pcshell_SN[1] = twinui_pcshell_SN[0];
            VirtualProtect(twinui_pcshell_SN, sizeof(twinui_pcshell_SN), flOldProtect, &flOldProtect);
        }
    }
    if (VnGetSymbols(
        pszSettingsPath,
        pOffsets,
        twinui_pcshell_SN,
        IsWindows11() ? TWINUI_PCSHELL_SB_CNT : 4
    ))
    {
        printf("[Symbols] Failure in reading symbols for \"%s\".\n", twinui_pcshell_sb_dll);
        if (hKey) RegCloseKey(hKey);
        return FALSE;
    }

    if (!IsWindows11())
    {
        pOffsets[1] = 0;
    }
    RegSetValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_0), 0, REG_DWORD, &pOffsets[0], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_1), 0, REG_DWORD, &pOffsets[1], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_2), 0, REG_DWORD, &pOffsets[2], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_3), 0, REG_DWORD, &pOffsets[3], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_4), 0, REG_DWORD, &pOffsets[4], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_5), 0, REG_DWORD, &pOffsets[5], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_6), 0, REG_DWORD, &pOffsets[6], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_7), 0, REG_DWORD, &pOffsets[7], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_8), 0, REG_DWORD, &pOffsets[8], sizeof(DWORD));

    RegSetValueExA(hKey, "Hash", 0, REG_SZ, szHash, strlen(szHash) + 1);
    SaveVersion(hKey, TWINUI_PCSHELL_SB_VERSION);

    if (hKey) RegCloseKey(hKey);
    return TRUE;
}

static BOOL ProcessStartDockedSymbols(const char* pszSettingsPath, DWORD* pOffsets)
{
    HKEY hKey = NULL;
    DWORD dwDisposition;
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH_STARTMENU) L"\\" TEXT(STARTDOCKED_SB_NAME),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        &dwDisposition
    );
    if (!hKey || hKey == INVALID_HANDLE_VALUE)
    {
        printf("[Symbols] Unable to create registry key.\n");
        return FALSE;
    }

    CHAR szHash[100];
    WCHAR wszPath[MAX_PATH];

    ZeroMemory(szHash, sizeof(szHash));
    ZeroMemory(wszPath, sizeof(wszPath));

    char startdocked_sb_dll[MAX_PATH];
    ZeroMemory(startdocked_sb_dll, sizeof(startdocked_sb_dll));
    GetWindowsDirectoryA(startdocked_sb_dll, MAX_PATH);
    strcat_s(startdocked_sb_dll, MAX_PATH, "\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\");
    strcat_s(startdocked_sb_dll, MAX_PATH, STARTDOCKED_SB_NAME);
    strcat_s(startdocked_sb_dll, MAX_PATH, ".dll");

    GetWindowsDirectoryW(wszPath, MAX_PATH);
    wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\" _T(STARTDOCKED_SB_NAME) L".dll");
    ComputeFileHash(wszPath, szHash, ARRAYSIZE(szHash));

    printf("[Symbols] Downloading symbols for \"%s\" (\"%s\")...\n", startdocked_sb_dll, szHash);
    if (VnDownloadSymbols(
        NULL,
        startdocked_sb_dll,
        pszSettingsPath,
        MAX_PATH
    ))
    {
        printf("[Symbols] Symbols for \"%s\" are not available - unable to download.\n", startdocked_sb_dll);
        printf("[Symbols] Please refer to \"https://github.com/valinet/ExplorerPatcher/wiki/Symbols\" for more information.\n");
        if (hKey) RegCloseKey(hKey);
        return FALSE;
    }

    printf("[Symbols] Reading symbols...\n");
    if (VnGetSymbols(
        pszSettingsPath,
        pOffsets,
        startdocked_SN,
        STARTDOCKED_SB_CNT
    ))
    {
        printf("[Symbols] Failure in reading symbols for \"%s\".\n", startdocked_sb_dll);
        if (hKey) RegCloseKey(hKey);
        return FALSE;
    }

    RegSetValueExW(hKey, TEXT(STARTDOCKED_SB_0), 0, REG_DWORD, &pOffsets[0], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(STARTDOCKED_SB_1), 0, REG_DWORD, &pOffsets[1], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(STARTDOCKED_SB_2), 0, REG_DWORD, &pOffsets[2], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(STARTDOCKED_SB_3), 0, REG_DWORD, &pOffsets[3], sizeof(DWORD));
    RegSetValueExW(hKey, TEXT(STARTDOCKED_SB_4), 0, REG_DWORD, &pOffsets[4], sizeof(DWORD));

    RegSetValueExA(hKey, "Hash", 0, REG_SZ, szHash, strlen(szHash) + 1);
    SaveVersion(hKey, STARTDOCKED_SB_VERSION);

    if (hKey) RegCloseKey(hKey);
    return TRUE;
}

static BOOL ProcessStartUISymbols(const char* pszSettingsPath, DWORD* pOffsets)
{
    HKEY hKey = NULL;
    DWORD dwDisposition;
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH_STARTMENU) L"\\" TEXT(STARTUI_SB_NAME),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        &dwDisposition
    );
    if (!hKey || hKey == INVALID_HANDLE_VALUE)
    {
        printf("[Symbols] Unable to create registry key.\n");
        return FALSE;
    }

    CHAR szHash[100];
    WCHAR wszPath[MAX_PATH];

    ZeroMemory(szHash, sizeof(szHash));
    ZeroMemory(wszPath, sizeof(wszPath));

    char startui_sb_dll[MAX_PATH];
    ZeroMemory(startui_sb_dll, sizeof(startui_sb_dll));
    GetWindowsDirectoryA(startui_sb_dll, MAX_PATH);
    strcat_s(startui_sb_dll, MAX_PATH, "\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\");
    strcat_s(startui_sb_dll, MAX_PATH, STARTUI_SB_NAME);
    strcat_s(startui_sb_dll, MAX_PATH, ".dll");

    GetWindowsDirectoryW(wszPath, MAX_PATH);
    wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\" _T(STARTUI_SB_NAME) L".dll");
    ComputeFileHash(wszPath, szHash, ARRAYSIZE(szHash));

    printf("[Symbols] Downloading symbols for \"%s\" (\"%s\")...\n", startui_sb_dll, szHash);
    if (VnDownloadSymbols(
        NULL,
        startui_sb_dll,
        pszSettingsPath,
        MAX_PATH
    ))
    {
        printf("[Symbols] Symbols for \"%s\" are not available - unable to download.\n", startui_sb_dll);
        printf("[Symbols] Please refer to \"https://github.com/valinet/ExplorerPatcher/wiki/Symbols\" for more information.\n");
        if (hKey) RegCloseKey(hKey);
        return FALSE;
    }

    printf("[Symbols] Reading symbols...\n");
    if (VnGetSymbols(
        pszSettingsPath,
        pOffsets,
        startui_SN,
        STARTUI_SB_CNT
    ))
    {
        printf("[Symbols] Failure in reading symbols for \"%s\".\n", startui_sb_dll);
        if (hKey) RegCloseKey(hKey);
        return FALSE;
    }

    RegSetValueExW(hKey, TEXT(STARTUI_SB_0), 0, REG_DWORD, &pOffsets[0], sizeof(DWORD));

    RegSetValueExA(hKey, "Hash", 0, REG_SZ, szHash, strlen(szHash) + 1);
    SaveVersion(hKey, STARTUI_SB_VERSION);

    if (hKey) RegCloseKey(hKey);
    return TRUE;
}

DWORD DownloadSymbols(DownloadSymbolsParams* params)
{
    Sleep(6000);

    printf("[Symbols] Started \"Download symbols\" thread.\n");

    EP_L10N_ApplyPreferredLanguageForCurrentThread();
    HMODULE hEPGui = LoadGuiModule();

    RTL_OSVERSIONINFOW rovi;
    DWORD32 ubr = VnGetOSVersionAndUBR(&rovi);
    wchar_t szReportedVersion[32];
    swprintf_s(
        szReportedVersion,
        ARRAYSIZE(szReportedVersion),
        L"%d.%d.%d.%d",
        rovi.dwMajorVersion,
        rovi.dwMinorVersion,
        rovi.dwBuildNumber,
        ubr
    );

    wchar_t title[160];
    wchar_t body[200];
    wchar_t titleFormat[160];
    wchar_t buffer[1000];
    title[0] = 0; body[0] = 0; titleFormat[0] = 0; buffer[0] = 0;

    // Don't annoy the user with "Downloading symbols" notification if the symbols aren't available in MS' servers
    HKEY hKey = NULL;
    DWORD dwDisposition;
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WRITE,
        NULL,
        &hKey,
        &dwDisposition
    );
    wchar_t szLastNotifiedBuild[32];
    szLastNotifiedBuild[0] = 0;
    DWORD dwSize = sizeof(szLastNotifiedBuild);
    RegQueryValueExW(
        hKey,
        TEXT("SymbolsLastNotifiedOSBuild"),
        0,
        NULL,
        szLastNotifiedBuild,
        &dwSize
    );

    BOOL bNewBuild = wcscmp(szLastNotifiedBuild, szReportedVersion) != 0;
    if (bNewBuild)
    {
        if (LoadStringW(hEPGui, IDS_SYM_DL_T, titleFormat, ARRAYSIZE(titleFormat)))
        {
            swprintf_s(title, ARRAYSIZE(title), titleFormat, szReportedVersion);
        }

        LoadStringW(hEPGui, IDS_SYM_DL_B, body, ARRAYSIZE(body));

        swprintf_s(buffer, ARRAYSIZE(buffer), DownloadNotificationXML, L"https://github.com/valinet/ExplorerPatcher/wiki/Symbols", L"short", title, body);

        HRESULT hr = S_OK;
        __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
        hr = String2IXMLDocument(
            buffer,
            wcslen(buffer),
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
            sizeof(APPID) / sizeof(wchar_t) - 1,
#ifdef DEBUG
            stdout
#else
            NULL
#endif
        );

        RegSetValueExW(
            hKey,
            TEXT("SymbolsLastNotifiedOSBuild"),
            0,
            REG_SZ,
            szReportedVersion,
            wcslen(szReportedVersion) * sizeof(wchar_t)
        );
    }

    RegCloseKey(hKey);

    wprintf(
        L"[Symbols] "
        L"Attempting to download symbols for OS version %s.\n",
        szReportedVersion
    );

    char szSettingsPath[MAX_PATH];
    ZeroMemory(
        szSettingsPath,
        sizeof(szSettingsPath)
    );
    SHGetFolderPathA(
        NULL,
        SPECIAL_FOLDER_LEGACY,
        NULL,
        SHGFP_TYPE_CURRENT,
        szSettingsPath
    );
    strcat_s(
        szSettingsPath,
        MAX_PATH,
        APP_RELATIVE_PATH
    );
    CreateDirectoryA(szSettingsPath, NULL);
    strcat_s(
        szSettingsPath,
        MAX_PATH,
        "\\"
    );
    printf("[Symbols] Downloading to \"%s\".\n", szSettingsPath);

    symbols_addr symbols_PTRS;
    ZeroMemory(&symbols_PTRS, sizeof(symbols_addr));
    BOOL bAnySuccess = FALSE, bAllSuccess = TRUE;
    if (params->loadResult.bNeedToDownloadExplorerSymbols && IsWindows11Version22H2OrHigher())
    {
        BOOL bSuccess = ProcessExplorerSymbols(szSettingsPath, symbols_PTRS.explorer_PTRS);
        bAnySuccess |= bSuccess;
        bAllSuccess &= bSuccess;
    }
    if (params->loadResult.bNeedToDownloadTwinuiPcshellSymbols)
    {
        BOOL bSuccess = ProcessTwinuiPcshellSymbols(szSettingsPath, symbols_PTRS.twinui_pcshell_PTRS);
        bAnySuccess |= bSuccess;
        bAllSuccess &= bSuccess;
    }
    if (params->loadResult.bNeedToDownloadStartDockedSymbols && IsWindows11())
    {
        BOOL bSuccess = ProcessStartDockedSymbols(szSettingsPath, symbols_PTRS.startdocked_PTRS);
        bAnySuccess |= bSuccess;
        bAllSuccess &= bSuccess;
    }
    if (params->loadResult.bNeedToDownloadStartUISymbols && rovi.dwBuildNumber >= 18362)
    {
        BOOL bSuccess = ProcessStartUISymbols(szSettingsPath, symbols_PTRS.startui_PTRS);
        bAnySuccess |= bSuccess;
        bAllSuccess &= bSuccess;
    }

    printf("[Symbols] Finished gathering symbol data.\n");

    title[0] = 0; body[0] = 0;
    BOOL bNotify = TRUE;
    if (bAllSuccess)
    {
        if (LoadStringW(hEPGui, IDS_SYM_SUCCESS_T, titleFormat, ARRAYSIZE(titleFormat)))
        {
            swprintf_s(title, ARRAYSIZE(title), titleFormat, szReportedVersion);
        }
        LoadStringW(hEPGui, IDS_SYM_SUCCESS_B, body, ARRAYSIZE(body));
        swprintf_s(buffer, ARRAYSIZE(buffer), DownloadNotificationXML, L"https://github.com/valinet/ExplorerPatcher/wiki/Symbols", L"long", title, body);
    }
    else if (bAnySuccess)
    {
        if (LoadStringW(hEPGui, IDS_SYM_FAILEDSOME_T, titleFormat, ARRAYSIZE(titleFormat)))
        {
            swprintf_s(title, ARRAYSIZE(title), titleFormat, szReportedVersion);
        }
        LoadStringW(hEPGui, IDS_SYM_FAILEDSOME_B, body, ARRAYSIZE(body));
        swprintf_s(buffer, ARRAYSIZE(buffer), DownloadNotificationXML, L"https://github.com/valinet/ExplorerPatcher/wiki/Symbols", L"short", title, body);
    }
    else
    {
        if (LoadStringW(hEPGui, IDS_SYM_FAILEDALL_T, titleFormat, ARRAYSIZE(titleFormat)))
        {
            swprintf_s(title, ARRAYSIZE(title), titleFormat, szReportedVersion);
        }
        LoadStringW(hEPGui, IDS_SYM_FAILEDALL_B, body, ARRAYSIZE(body));
        swprintf_s(buffer, ARRAYSIZE(buffer), DownloadNotificationXML, L"https://github.com/valinet/ExplorerPatcher/wiki/Symbols", L"short", title, body);
        bNotify = bNewBuild;
    }

    if (bNotify)
    {
        __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml2 = NULL;
        HRESULT hr = String2IXMLDocument(
            buffer,
            wcslen(buffer),
            &inputXml2,
#ifdef DEBUG
            stdout
#else
            NULL
#endif
        );
        hr = ShowToastMessage(
            inputXml2,
            APPID,
            sizeof(APPID) / sizeof(wchar_t) - 1,
#ifdef DEBUG
            stdout
#else
            NULL
#endif
        );
    }

    FreeLibrary(hEPGui);

    printf("[Symbols] Finished \"Download symbols\" thread.\n");
    return 0;
}

LoadSymbolsResult LoadSymbols(symbols_addr* symbols_PTRS)
{
    LoadSymbolsResult result;
    ZeroMemory(&result, sizeof(LoadSymbolsResult));

    HKEY hKey = NULL;
    DWORD dwDisposition;
    DWORD dwSize;

    RTL_OSVERSIONINFOW rovi;
    DWORD32 ubr = VnGetOSVersionAndUBR(&rovi);

    CHAR szHash[33];
    CHAR szStoredHash[33];
    ZeroMemory(szHash, sizeof(szHash));
    ZeroMemory(szStoredHash, sizeof(szStoredHash));
    wchar_t wszPath[MAX_PATH];
    BOOL bOffsetsValid;

    // Load explorer.exe offsets
    if (IsWindows11Version22H2OrHigher())
    {
        bOffsetsValid = FALSE;
        RegCreateKeyExW(
            HKEY_CURRENT_USER,
            TEXT(REGPATH) L"\\" TEXT(EXPLORER_SB_NAME),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ,
            NULL,
            &hKey,
            &dwDisposition
        );
        if (!hKey || hKey == INVALID_HANDLE_VALUE)
        {
            result.bSuccess = FALSE;
            return result;
        }

        GetWindowsDirectoryW(wszPath, MAX_PATH);
        wcscat_s(wszPath, MAX_PATH, L"\\" TEXT(EXPLORER_SB_NAME) L".exe");
        if (ComputeFileHash(wszPath, szHash, ARRAYSIZE(szHash)) == ERROR_SUCCESS)
        {
            szStoredHash[0] = 0;
            dwSize = sizeof(szStoredHash);
            if (RegQueryValueExA(hKey, "Hash", 0, NULL, szStoredHash, &dwSize) == ERROR_SUCCESS
                && !_stricmp(szHash, szStoredHash) && CheckVersion(hKey, EXPLORER_SB_VERSION))
            {
                dwSize = sizeof(DWORD);
                RegQueryValueExW(hKey, TEXT(EXPLORER_SB_0), 0, NULL, &symbols_PTRS->explorer_PTRS[0], &dwSize);
                RegQueryValueExW(hKey, TEXT(EXPLORER_SB_1), 0, NULL, &symbols_PTRS->explorer_PTRS[1], &dwSize);
                RegQueryValueExW(hKey, TEXT(EXPLORER_SB_2), 0, NULL, &symbols_PTRS->explorer_PTRS[2], &dwSize);
                RegQueryValueExW(hKey, TEXT(EXPLORER_SB_3), 0, NULL, &symbols_PTRS->explorer_PTRS[3], &dwSize);
                RegQueryValueExW(hKey, TEXT(EXPLORER_SB_4), 0, NULL, &symbols_PTRS->explorer_PTRS[4], &dwSize);
                RegQueryValueExW(hKey, TEXT(EXPLORER_SB_5), 0, NULL, &symbols_PTRS->explorer_PTRS[5], &dwSize);
                bOffsetsValid = TRUE;
            }
            else
            {
                printf("[Symbols] Symbols for \"%s\" are not available.\n", EXPLORER_SB_NAME);
                result.bNeedToDownloadExplorerSymbols = TRUE;
            }
        }
        if (hKey) RegCloseKey(hKey);
        if (!bOffsetsValid)
        {
            RegDeleteTreeW(
                HKEY_CURRENT_USER,
                TEXT(REGPATH) L"\\" TEXT(EXPLORER_SB_NAME)
            );
        }
    }

    // Load twinui.pcshell.dll offsets
    bOffsetsValid = FALSE;
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH) L"\\" TEXT(TWINUI_PCSHELL_SB_NAME),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        &dwDisposition
    );
    if (!hKey || hKey == INVALID_HANDLE_VALUE)
    {
        result.bSuccess = FALSE;
        return result;
    }

    GetSystemDirectoryW(wszPath, MAX_PATH);
    wcscat_s(wszPath, MAX_PATH, L"\\" TEXT(TWINUI_PCSHELL_SB_NAME) L".dll");
    if (ComputeFileHash(wszPath, szHash, ARRAYSIZE(szHash)) == ERROR_SUCCESS)
    {
        szStoredHash[0] = 0;
        dwSize = sizeof(szStoredHash);
        if (RegQueryValueExA(hKey, "Hash", 0, NULL, szStoredHash, &dwSize) == ERROR_SUCCESS
            && !_stricmp(szHash, szStoredHash) && CheckVersion(hKey, TWINUI_PCSHELL_SB_VERSION))
        {
            dwSize = sizeof(DWORD);
            RegQueryValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_0), 0, NULL, &symbols_PTRS->twinui_pcshell_PTRS[0], &dwSize);
            RegQueryValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_1), 0, NULL, &symbols_PTRS->twinui_pcshell_PTRS[1], &dwSize);
            RegQueryValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_2), 0, NULL, &symbols_PTRS->twinui_pcshell_PTRS[2], &dwSize);
            RegQueryValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_3), 0, NULL, &symbols_PTRS->twinui_pcshell_PTRS[3], &dwSize);
            RegQueryValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_4), 0, NULL, &symbols_PTRS->twinui_pcshell_PTRS[4], &dwSize);
            RegQueryValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_5), 0, NULL, &symbols_PTRS->twinui_pcshell_PTRS[5], &dwSize);
            RegQueryValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_6), 0, NULL, &symbols_PTRS->twinui_pcshell_PTRS[6], &dwSize);
            RegQueryValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_7), 0, NULL, &symbols_PTRS->twinui_pcshell_PTRS[7], &dwSize);
            RegQueryValueExW(hKey, TEXT(TWINUI_PCSHELL_SB_8), 0, NULL, &symbols_PTRS->twinui_pcshell_PTRS[8], &dwSize);
            bOffsetsValid = TRUE;
        }
        else
        {
            printf("[Symbols] Symbols for \"%s\" are not available.\n", TWINUI_PCSHELL_SB_NAME);
#ifdef _M_X64 // TODO Add support for ARM64
            result.bNeedToDownloadTwinuiPcshellSymbols = TRUE;
#endif
        }
    }

    RegCloseKey(hKey);
    if (!bOffsetsValid)
    {
        RegDeleteTreeW(
            HKEY_CURRENT_USER,
            TEXT(REGPATH) L"\\" TEXT(TWINUI_PCSHELL_SB_NAME)
        );
    }

    if (IsWindows11())
    {
        // Load StartDocked.dll offsets
        bOffsetsValid = FALSE;
        RegCreateKeyExW(
            HKEY_CURRENT_USER,
            TEXT(REGPATH_STARTMENU) L"\\" TEXT(STARTDOCKED_SB_NAME),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ,
            NULL,
            &hKey,
            &dwDisposition
        );

        GetWindowsDirectoryW(wszPath, MAX_PATH);
        wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\" TEXT(STARTDOCKED_SB_NAME) L".dll");
        if (ComputeFileHash(wszPath, szHash, ARRAYSIZE(szHash)) == ERROR_SUCCESS)
        {
            szStoredHash[0] = 0;
            dwSize = sizeof(szStoredHash);
            if (RegQueryValueExA(hKey, "Hash", 0, NULL, szStoredHash, &dwSize) == ERROR_SUCCESS
                && !_stricmp(szHash, szStoredHash) && CheckVersion(hKey, STARTDOCKED_SB_VERSION))
            {
                dwSize = sizeof(DWORD);
                RegQueryValueExW(hKey, TEXT(STARTDOCKED_SB_0), 0, NULL, &symbols_PTRS->startdocked_PTRS[0], &dwSize);
                RegQueryValueExW(hKey, TEXT(STARTDOCKED_SB_1), 0, NULL, &symbols_PTRS->startdocked_PTRS[1], &dwSize);
                RegQueryValueExW(hKey, TEXT(STARTDOCKED_SB_2), 0, NULL, &symbols_PTRS->startdocked_PTRS[2], &dwSize);
                RegQueryValueExW(hKey, TEXT(STARTDOCKED_SB_3), 0, NULL, &symbols_PTRS->startdocked_PTRS[3], &dwSize);
                RegQueryValueExW(hKey, TEXT(STARTDOCKED_SB_4), 0, NULL, &symbols_PTRS->startdocked_PTRS[4], &dwSize);
                bOffsetsValid = TRUE;
            }
            else
            {
                printf("[Symbols] Symbols for \"%s\" are not available.\n", STARTDOCKED_SB_NAME);
                result.bNeedToDownloadStartDockedSymbols = TRUE;
            }
        }
        if (hKey) RegCloseKey(hKey);
        if (!bOffsetsValid)
        {
            RegDeleteTreeW(
                HKEY_CURRENT_USER,
                TEXT(REGPATH_STARTMENU) L"\\" TEXT(STARTDOCKED_SB_NAME)
            );
        }
    }

    if (rovi.dwBuildNumber >= 18362)
    {
        // Load StartUI.dll offsets
        bOffsetsValid = FALSE;
        RegCreateKeyExW(
            HKEY_CURRENT_USER,
            TEXT(REGPATH_STARTMENU) L"\\" TEXT(STARTUI_SB_NAME),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ,
            NULL,
            &hKey,
            &dwDisposition
        );

        GetWindowsDirectoryW(wszPath, MAX_PATH);
        wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\" TEXT(STARTUI_SB_NAME) L".dll");
        if (ComputeFileHash(wszPath, szHash, ARRAYSIZE(szHash)) == ERROR_SUCCESS)
        {
            szStoredHash[0] = 0;
            dwSize = sizeof(szStoredHash);
            if (RegQueryValueExA(hKey, "Hash", 0, NULL, szStoredHash, &dwSize) == ERROR_SUCCESS
                && !_stricmp(szHash, szStoredHash) && CheckVersion(hKey, STARTUI_SB_VERSION))
            {
                dwSize = sizeof(DWORD);
                RegQueryValueExW(hKey, TEXT(STARTUI_SB_0), 0, NULL, &symbols_PTRS->startui_PTRS[0], &dwSize);
                bOffsetsValid = TRUE;
            }
            else
            {
                printf("[Symbols] Symbols for \"%s\" are not available.\n", STARTUI_SB_NAME);
                result.bNeedToDownloadStartUISymbols = TRUE;
            }
        }
        if (hKey) RegCloseKey(hKey);
        if (!bOffsetsValid)
        {
            RegDeleteTreeW(
               HKEY_CURRENT_USER,
               TEXT(REGPATH_STARTMENU) L"\\" TEXT(STARTUI_SB_NAME)
           );
        }
    }

    // Delete "OSBuild" value from previous versions of EP
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        &dwDisposition
    );
    if (hKey && hKey != INVALID_HANDLE_VALUE)
    {
        RegDeleteValueW(hKey, TEXT("OSBuild"));
        RegCloseKey(hKey);
    }

    result.bSuccess = TRUE;
    return result;
}

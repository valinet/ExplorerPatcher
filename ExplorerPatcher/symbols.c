#include "symbols.h"

const char* twinui_pcshell_SN[TWINUI_PCSHELL_SB_CNT] = {
    TWINUI_PCSHELL_SB_0,
    TWINUI_PCSHELL_SB_1,
    TWINUI_PCSHELL_SB_2,
    TWINUI_PCSHELL_SB_3,
    TWINUI_PCSHELL_SB_4,
    TWINUI_PCSHELL_SB_5,
    TWINUI_PCSHELL_SB_6,
    TWINUI_PCSHELL_SB_7
};
const char* startdocked_SN[STARTDOCKED_SB_CNT] = {
    STARTDOCKED_SB_0,
    STARTDOCKED_SB_1,
    STARTDOCKED_SB_2,
    STARTDOCKED_SB_3,
    STARTDOCKED_SB_4
};

const wchar_t DownloadSymbolsXML[] =
L"<toast displayTimestamp=\"2021-08-29T00:00:00.000Z\" scenario=\"reminder\" "
L"activationType=\"protocol\" launch=\"https://github.com/valinet/ExplorerPatcher\" duration=\"short\">\r\n"
L"	<visual>\r\n"
L"		<binding template=\"ToastGeneric\">\r\n"
L"			<text><![CDATA[Unable to find symbols for OS version %s]]></text>\r\n"
L"			<text><![CDATA[Downloading and applying symbol information, please wait...]]></text>\r\n"
L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
L"		</binding>\r\n"
L"	</visual>\r\n"
L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
L"</toast>\r\n";

const wchar_t DownloadOKXML[] =
L"<toast displayTimestamp=\"2021-08-29T01:00:00.000Z\" scenario=\"reminder\" "
L"activationType=\"protocol\" launch=\"https://github.com/valinet/ExplorerPatcher\" duration=\"short\">\r\n"
L"	<visual>\r\n"
L"		<binding template=\"ToastGeneric\">\r\n"
L"			<text><![CDATA[Symbols downloaded and applied successfully!]]></text>\r\n"
L"			<text><![CDATA[Now, please wait while dynamic Explorer patching is done...]]></text>\r\n"
L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
L"		</binding>\r\n"
L"	</visual>\r\n"
L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
L"</toast>\r\n";

const wchar_t InstallOK[] =
L"<toast displayTimestamp=\"2021-08-29T01:00:00.000Z\" scenario=\"reminder\" "
L"activationType=\"protocol\" launch=\"https://github.com/valinet/ExplorerPatcher\" duration=\"long\">\r\n"
L"	<visual>\r\n"
L"		<binding template=\"ToastGeneric\">\r\n"
L"			<text><![CDATA[Installation succeeded!]]></text>\r\n"
L"			<text><![CDATA[This notification will not show again until the next OS build update.]]></text>\r\n"
L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
L"		</binding>\r\n"
L"	</visual>\r\n"
L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
L"</toast>\r\n";

DWORD DownloadSymbols(DownloadSymbolsParams* params)
{
    HKEY hKey = NULL;
    DWORD dwDisposition;

    HMODULE hModule = params->hModule;

    Sleep(3000);

    printf("Started \"Download symbols\" thread.\n");

    RTL_OSVERSIONINFOW rovi;
    DWORD32 ubr = VnGetOSVersionAndUBR(&rovi);
    TCHAR szReportedVersion[MAX_PATH + 1];
    ZeroMemory(
        szReportedVersion,
        (MAX_PATH + 1) * sizeof(TCHAR)
    );
    wsprintf(
        szReportedVersion,
        L"%d.%d.%d.%d",
        rovi.dwMajorVersion,
        rovi.dwMinorVersion,
        rovi.dwBuildNumber,
        ubr
    );

    TCHAR buffer[sizeof(DownloadSymbolsXML) / sizeof(wchar_t) + 30];
    ZeroMemory(
        buffer,
        (sizeof(DownloadSymbolsXML) / sizeof(wchar_t) + 30) * sizeof(TCHAR)
    );
    wsprintf(
        buffer,
        DownloadSymbolsXML,
        szReportedVersion
    );
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
        sizeof(APPID) / sizeof(TCHAR) - 1,
#ifdef DEBUG
        stdout
#else
        NULL
#endif
    );


    char szSettingsPath[MAX_PATH];
    ZeroMemory(
        szSettingsPath,
        MAX_PATH * sizeof(char)
    );
    SHGetFolderPathA(
        NULL,
        SPECIAL_FOLDER,
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
    printf("Downloading to \"%s\".\n", szSettingsPath);

    symbols_addr symbols_PTRS;
    ZeroMemory(
        &symbols_PTRS,
        sizeof(symbols_addr)
    );




    char twinui_pcshell_sb_dll[MAX_PATH];
    ZeroMemory(
        twinui_pcshell_sb_dll,
        (MAX_PATH) * sizeof(char)
    );
    GetSystemDirectoryA(
        twinui_pcshell_sb_dll,
        MAX_PATH
    );
    strcat_s(
        twinui_pcshell_sb_dll,
        MAX_PATH,
        "\\"
    );
    strcat_s(
        twinui_pcshell_sb_dll,
        MAX_PATH,
        TWINUI_PCSHELL_SB_NAME
    );
    strcat_s(
        twinui_pcshell_sb_dll,
        MAX_PATH,
        ".dll"
    );
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
        FreeLibraryAndExitThread(
            hModule,
            9
        );
        return 9;
    }
    printf("Downloading symbols for \"%s\"...\n", twinui_pcshell_sb_dll);
    if (VnDownloadSymbols(
        NULL,
        twinui_pcshell_sb_dll,
        szSettingsPath,
        MAX_PATH
    ))
    {
        FreeLibraryAndExitThread(
            hModule,
            4
        );
        return 4;
    }
    printf("Reading symbols...\n");
    if (VnGetSymbols(
        szSettingsPath,
        symbols_PTRS.twinui_pcshell_PTRS,
        twinui_pcshell_SN,
        TWINUI_PCSHELL_SB_CNT
    ))
    {
        printf("Hooking Win+C is not available for this build.\n");
        DWORD dwZero = 0;
        RegSetValueExW(
            hKey,
            TEXT(TWINUI_PCSHELL_SB_7),
            0,
            REG_DWORD,
            &dwZero,
            sizeof(DWORD)
        );
        if (VnGetSymbols(
            szSettingsPath,
            symbols_PTRS.twinui_pcshell_PTRS,
            twinui_pcshell_SN,
            TWINUI_PCSHELL_SB_CNT - 1
        ))
        {
            FreeLibraryAndExitThread(
                hModule,
                5
            );
            return 5;
        }
    }
    RegSetValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_0),
        0,
        REG_DWORD,
        &(symbols_PTRS.twinui_pcshell_PTRS[0]),
        sizeof(DWORD)
    );
    RegSetValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_1),
        0,
        REG_DWORD,
        &(symbols_PTRS.twinui_pcshell_PTRS[1]),
        sizeof(DWORD)
    );
    RegSetValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_2),
        0,
        REG_DWORD,
        &(symbols_PTRS.twinui_pcshell_PTRS[2]),
        sizeof(DWORD)
    );
    RegSetValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_3),
        0,
        REG_DWORD,
        &(symbols_PTRS.twinui_pcshell_PTRS[3]),
        sizeof(DWORD)
    );
    RegSetValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_4),
        0,
        REG_DWORD,
        &(symbols_PTRS.twinui_pcshell_PTRS[4]),
        sizeof(DWORD)
    );
    RegSetValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_5),
        0,
        REG_DWORD,
        &(symbols_PTRS.twinui_pcshell_PTRS[5]),
        sizeof(DWORD)
    );
    RegSetValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_6),
        0,
        REG_DWORD,
        &(symbols_PTRS.twinui_pcshell_PTRS[6]),
        sizeof(DWORD)
    );
    RegSetValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_7),
        0,
        REG_DWORD,
        &(symbols_PTRS.twinui_pcshell_PTRS[7]),
        sizeof(DWORD)
    );
    if (hKey) RegCloseKey(hKey);



    char startdocked_sb_dll[MAX_PATH];
    ZeroMemory(
        startdocked_sb_dll,
        (MAX_PATH) * sizeof(char)
    );
    GetWindowsDirectoryA(
        startdocked_sb_dll,
        MAX_PATH
    );
    strcat_s(
        startdocked_sb_dll,
        MAX_PATH,
        "\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\"
    );
    strcat_s(
        startdocked_sb_dll,
        MAX_PATH,
        STARTDOCKED_SB_NAME
    );
    strcat_s(
        startdocked_sb_dll,
        MAX_PATH,
        ".dll"
    );
    printf("Downloading symbols for \"%s\"...\n", startdocked_sb_dll);
    if (VnDownloadSymbols(
        NULL,
        startdocked_sb_dll,
        szSettingsPath,
        MAX_PATH
    ))
    {
        FreeLibraryAndExitThread(
            hModule,
            6
        );
        return 6;
    }
    printf("Reading symbols...\n");
    if (VnGetSymbols(
        szSettingsPath,
        symbols_PTRS.startdocked_PTRS,
        startdocked_SN,
        STARTDOCKED_SB_CNT
    ))
    {
        printf("error...\n");
        FreeLibraryAndExitThread(
            hModule,
            7
        );
        return 7;
    }
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH) L"\\" TEXT(STARTDOCKED_SB_NAME),
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
        FreeLibraryAndExitThread(
            hModule,
            8
        );
        return 8;
    }
    RegSetValueExW(
        hKey,
        TEXT(STARTDOCKED_SB_0),
        0,
        REG_DWORD,
        &(symbols_PTRS.startdocked_PTRS[0]),
        sizeof(DWORD)
    );
    RegSetValueExW(
        hKey,
        TEXT(STARTDOCKED_SB_1),
        0,
        REG_DWORD,
        &(symbols_PTRS.startdocked_PTRS[1]),
        sizeof(DWORD)
    );
    RegSetValueExW(
        hKey,
        TEXT(STARTDOCKED_SB_2),
        0,
        REG_DWORD,
        &(symbols_PTRS.startdocked_PTRS[2]),
        sizeof(DWORD)
    );
    RegSetValueExW(
        hKey,
        TEXT(STARTDOCKED_SB_3),
        0,
        REG_DWORD,
        &(symbols_PTRS.startdocked_PTRS[3]),
        sizeof(DWORD)
    );
    RegSetValueExW(
        hKey,
        TEXT(STARTDOCKED_SB_4),
        0,
        REG_DWORD,
        &(symbols_PTRS.startdocked_PTRS[4]),
        sizeof(DWORD)
    );
    if (hKey) RegCloseKey(hKey);






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
    if (!hKey || hKey == INVALID_HANDLE_VALUE)
    {
        FreeLibraryAndExitThread(
            hModule,
            10
        );
        return 10;
    }
    RegSetValueExW(
        hKey,
        TEXT("OSBuild"),
        0,
        REG_SZ,
        szReportedVersion,
        wcslen(szReportedVersion) * sizeof(TCHAR)
    );
    if (hKey) RegCloseKey(hKey);


    if (symbols_PTRS.twinui_pcshell_PTRS[0])
    {
        __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml = NULL;
        HRESULT hr = String2IXMLDocument(
            InstallOK,
            wcslen(InstallOK),
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
    else
    {
        __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml2 = NULL;
        hr = String2IXMLDocument(
            DownloadOKXML,
            wcslen(DownloadOKXML),
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
            sizeof(APPID) / sizeof(TCHAR) - 1,
#ifdef DEBUG
            stdout
#else
            NULL
#endif
        );
    }

    Sleep(4000);

    exit(0);
}

BOOL LoadSymbols(symbols_addr* symbols_PTRS, HMODULE hModule)
{
    HKEY hKey = NULL;
    DWORD dwDisposition;
    DWORD dwSize = sizeof(DWORD);

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
        FreeLibraryAndExitThread(
            hModule,
            1
        );
        return 1;
    }
    RegQueryValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_0),
        0,
        NULL,
        &(symbols_PTRS->twinui_pcshell_PTRS[0]),
        &dwSize
    );
    RegQueryValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_1),
        0,
        NULL,
        &(symbols_PTRS->twinui_pcshell_PTRS[1]),
        &dwSize
    );
    RegQueryValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_2),
        0,
        NULL,
        &(symbols_PTRS->twinui_pcshell_PTRS[2]),
        &dwSize
    );
    RegQueryValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_3),
        0,
        NULL,
        &(symbols_PTRS->twinui_pcshell_PTRS[3]),
        &dwSize
    );
    RegQueryValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_4),
        0,
        NULL,
        &(symbols_PTRS->twinui_pcshell_PTRS[4]),
        &dwSize
    );
    RegQueryValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_5),
        0,
        NULL,
        &(symbols_PTRS->twinui_pcshell_PTRS[5]),
        &dwSize
    );
    RegQueryValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_6),
        0,
        NULL,
        &(symbols_PTRS->twinui_pcshell_PTRS[6]),
        &dwSize
    );
    RegQueryValueExW(
        hKey,
        TEXT(TWINUI_PCSHELL_SB_7),
        0,
        NULL,
        &(symbols_PTRS->twinui_pcshell_PTRS[7]),
        &dwSize
    );
    RegCloseKey(hKey);

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH) L"\\" TEXT(STARTDOCKED_SB_NAME),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        &dwDisposition
    );
    RegQueryValueExW(
        hKey,
        TEXT(STARTDOCKED_SB_0),
        0,
        NULL,
        &(symbols_PTRS->startdocked_PTRS[0]),
        &dwSize
    );
    RegQueryValueExW(
        hKey,
        TEXT(STARTDOCKED_SB_1),
        0,
        NULL,
        &(symbols_PTRS->startdocked_PTRS[1]),
        &dwSize
    );
    RegQueryValueExW(
        hKey,
        TEXT(STARTDOCKED_SB_2),
        0,
        NULL,
        &(symbols_PTRS->startdocked_PTRS[2]),
        &dwSize
    );
    RegQueryValueExW(
        hKey,
        TEXT(STARTDOCKED_SB_3),
        0,
        NULL,
        &(symbols_PTRS->startdocked_PTRS[3]),
        &dwSize
    );
    RegQueryValueExW(
        hKey,
        TEXT(STARTDOCKED_SB_4),
        0,
        NULL,
        &(symbols_PTRS->startdocked_PTRS[4]),
        &dwSize
    );
    if (hKey) RegCloseKey(hKey);

    BOOL bNeedToDownload = FALSE;
    for (UINT i = 0; i < sizeof(symbols_addr) / sizeof(DWORD); ++i)
    {
        if (!((DWORD*)symbols_PTRS)[i] &&
            (((DWORD*)symbols_PTRS) + i) != symbols_PTRS->twinui_pcshell_PTRS + TWINUI_PCSHELL_SB_CNT - 1
            )
        {
            bNeedToDownload = TRUE;
        }
    }
    RTL_OSVERSIONINFOW rovi;
    DWORD32 ubr = VnGetOSVersionAndUBR(&rovi);
    TCHAR szReportedVersion[MAX_PATH + 1];
    ZeroMemory(
        szReportedVersion,
        (MAX_PATH + 1) * sizeof(TCHAR)
    );
    TCHAR szStoredVersion[MAX_PATH + 1];
    ZeroMemory(
        szStoredVersion,
        (MAX_PATH + 1) * sizeof(TCHAR)
    );
    wsprintf(
        szReportedVersion,
        L"%d.%d.%d.%d",
        rovi.dwMajorVersion,
        rovi.dwMinorVersion,
        rovi.dwBuildNumber,
        ubr
    );
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKey,
        &dwDisposition
    );
    dwSize = MAX_PATH;
    RegQueryValueExW(
        hKey,
        TEXT("OSBuild"),
        0,
        NULL,
        szStoredVersion,
        &dwSize
    );
    RegCloseKey(hKey);
    if (!bNeedToDownload)
    {
        bNeedToDownload = wcscmp(szReportedVersion, szStoredVersion);
    }
    return bNeedToDownload;
}
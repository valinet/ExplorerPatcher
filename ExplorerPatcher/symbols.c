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
    TCHAR* wszSettingsPath = params->wszSettingsPath;
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

    DWORD dwRet = 0;
    char szSettingsPath[MAX_PATH + 1];
    ZeroMemory(
        szSettingsPath,
        (MAX_PATH + 1) * sizeof(char)
    );
    wcstombs_s(
        &dwRet,
        szSettingsPath,
        MAX_PATH + 1,
        wszSettingsPath,
        MAX_PATH + 1
    );
    PathRemoveFileSpecA(szSettingsPath);
    CreateDirectoryA(szSettingsPath, NULL);
    strcat_s(
        szSettingsPath,
        MAX_PATH + 1,
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
    VnWriteUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_0),
        symbols_PTRS.twinui_pcshell_PTRS[0],
        wszSettingsPath
    );
    VnWriteUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_1),
        symbols_PTRS.twinui_pcshell_PTRS[1],
        wszSettingsPath
    );
    VnWriteUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_2),
        symbols_PTRS.twinui_pcshell_PTRS[2],
        wszSettingsPath
    );
    VnWriteUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_3),
        symbols_PTRS.twinui_pcshell_PTRS[3],
        wszSettingsPath
    );
    VnWriteUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_4),
        symbols_PTRS.twinui_pcshell_PTRS[4],
        wszSettingsPath
    );
    VnWriteUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_5),
        symbols_PTRS.twinui_pcshell_PTRS[5],
        wszSettingsPath
    );
    VnWriteUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_6),
        symbols_PTRS.twinui_pcshell_PTRS[6],
        wszSettingsPath
    );
    VnWriteUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_7),
        symbols_PTRS.twinui_pcshell_PTRS[7],
        wszSettingsPath
    );



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
    VnWriteUInt(
        TEXT(STARTDOCKED_SB_NAME),
        TEXT(STARTDOCKED_SB_0),
        symbols_PTRS.startdocked_PTRS[0],
        wszSettingsPath
    );
    VnWriteUInt(
        TEXT(STARTDOCKED_SB_NAME),
        TEXT(STARTDOCKED_SB_1),
        symbols_PTRS.startdocked_PTRS[1],
        wszSettingsPath
    );
    VnWriteUInt(
        TEXT(STARTDOCKED_SB_NAME),
        TEXT(STARTDOCKED_SB_2),
        symbols_PTRS.startdocked_PTRS[2],
        wszSettingsPath
    );
    VnWriteUInt(
        TEXT(STARTDOCKED_SB_NAME),
        TEXT(STARTDOCKED_SB_3),
        symbols_PTRS.startdocked_PTRS[3],
        wszSettingsPath
    );
    VnWriteUInt(
        TEXT(STARTDOCKED_SB_NAME),
        TEXT(STARTDOCKED_SB_4),
        symbols_PTRS.startdocked_PTRS[4],
        wszSettingsPath
    );







    VnWriteString(
        TEXT("OS"),
        TEXT("Build"),
        szReportedVersion,
        wszSettingsPath
    );

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

    TCHAR wszExplorerPath[MAX_PATH + 1];
    wszExplorerPath[0] = L'\"';
    GetSystemDirectory(wszExplorerPath + 1, MAX_PATH);
    wcscat_s(wszExplorerPath, MAX_PATH + 1, L"\\rundll32.exe\" \"");
    GetModuleFileName(hModule, wszExplorerPath + wcslen(wszExplorerPath), MAX_PATH - wcslen(wszExplorerPath));
    wcscat_s(wszExplorerPath, MAX_PATH, L"\",ZZLaunchExplorer");
    wprintf(L"Command to launch: \" %s \"\n.", wszExplorerPath);
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    BOOL b = CreateProcess(
        NULL,
        wszExplorerPath,
        NULL,
        NULL,
        TRUE,
        CREATE_UNICODE_ENVIRONMENT,
        NULL,
        NULL,
        &si,
        &pi
    );

    FreeConsole();
    TerminateProcess(
        OpenProcess(
            PROCESS_TERMINATE,
            FALSE,
            GetCurrentProcessId()
        ),
        EXIT_CODE_EXPLORER
    );
}

BOOL LoadSymbols(symbols_addr* symbols_PTRS, TCHAR* wszSettingsPath)
{
    symbols_PTRS->twinui_pcshell_PTRS[0] = VnGetUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_0),
        0,
        wszSettingsPath
    );
    symbols_PTRS->twinui_pcshell_PTRS[1] = VnGetUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_1),
        0,
        wszSettingsPath
    );
    symbols_PTRS->twinui_pcshell_PTRS[2] = VnGetUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_2),
        0,
        wszSettingsPath
    );
    symbols_PTRS->twinui_pcshell_PTRS[3] = VnGetUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_3),
        0,
        wszSettingsPath
    );
    symbols_PTRS->twinui_pcshell_PTRS[4] = VnGetUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_4),
        0,
        wszSettingsPath
    );
    symbols_PTRS->twinui_pcshell_PTRS[5] = VnGetUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_5),
        0,
        wszSettingsPath
    );
    symbols_PTRS->twinui_pcshell_PTRS[6] = VnGetUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_6),
        0,
        wszSettingsPath
    );
    symbols_PTRS->twinui_pcshell_PTRS[7] = VnGetUInt(
        TEXT(TWINUI_PCSHELL_SB_NAME),
        TEXT(TWINUI_PCSHELL_SB_7),
        0,
        wszSettingsPath
    );
    HKEY hKeySettings;
    DWORD dwDisposition;

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH_OTHERS) L"\\" TEXT(STARTDOCKED_SB_NAME),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKeySettings,
        &dwDisposition
    );
    symbols_PTRS->startdocked_PTRS[0] = VnGetUInt(
        TEXT(STARTDOCKED_SB_NAME),
        TEXT(STARTDOCKED_SB_0),
        0,
        wszSettingsPath
    );
    if (hKeySettings)
    {
        RegSetValueExW(
            hKeySettings,
            TEXT(STARTDOCKED_SB_0),
            0,
            REG_DWORD,
            &(symbols_PTRS->startdocked_PTRS[0]),
            sizeof(DWORD)
        );
    }
    symbols_PTRS->startdocked_PTRS[1] = VnGetUInt(
        TEXT(STARTDOCKED_SB_NAME),
        TEXT(STARTDOCKED_SB_1),
        0,
        wszSettingsPath
    );
    if (hKeySettings)
    {
        RegSetValueExW(
            hKeySettings,
            TEXT(STARTDOCKED_SB_1),
            0,
            REG_DWORD,
            &(symbols_PTRS->startdocked_PTRS[1]),
            sizeof(DWORD)
        );
    }
    symbols_PTRS->startdocked_PTRS[2] = VnGetUInt(
        TEXT(STARTDOCKED_SB_NAME),
        TEXT(STARTDOCKED_SB_2),
        0,
        wszSettingsPath
    );
    if (hKeySettings)
    {
        RegSetValueExW(
            hKeySettings,
            TEXT(STARTDOCKED_SB_2),
            0,
            REG_DWORD,
            &(symbols_PTRS->startdocked_PTRS[2]),
            sizeof(DWORD)
        );
    }
    symbols_PTRS->startdocked_PTRS[3] = VnGetUInt(
        TEXT(STARTDOCKED_SB_NAME),
        TEXT(STARTDOCKED_SB_3),
        0,
        wszSettingsPath
    );
    if (hKeySettings)
    {
        RegSetValueExW(
            hKeySettings,
            TEXT(STARTDOCKED_SB_3),
            0,
            REG_DWORD,
            &(symbols_PTRS->startdocked_PTRS[3]),
            sizeof(DWORD)
        );
    }
    symbols_PTRS->startdocked_PTRS[4] = VnGetUInt(
        TEXT(STARTDOCKED_SB_NAME),
        TEXT(STARTDOCKED_SB_4),
        0,
        wszSettingsPath
    );
    if (hKeySettings)
    {
        RegSetValueExW(
            hKeySettings,
            TEXT(STARTDOCKED_SB_4),
            0,
            REG_DWORD,
            &(symbols_PTRS->startdocked_PTRS[4]),
            sizeof(DWORD)
        );
    }
    if (hKeySettings)
    {
        RegCloseKey(hKeySettings);
    }

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
    VnGetString(
        TEXT("OS"),
        TEXT("Build"),
        szStoredVersion,
        MAX_PATH,
        MAX_PATH,
        NULL,
        wszSettingsPath
    );
    if (!bNeedToDownload)
    {
        bNeedToDownload = wcscmp(szReportedVersion, szStoredVersion);
    }
    return bNeedToDownload;
}
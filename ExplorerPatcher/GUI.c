#include "GUI.h"

static HRESULT GUI_AboutProc(
    HWND hwnd,
    UINT uNotification,
    WPARAM wParam,
    LPARAM lParam,
    LONG_PTR lpRefData
)
{
    switch (uNotification)
    {
    case TDN_BUTTON_CLICKED:
    {
        if (wParam == IDOK || wParam == IDCANCEL)
        {
            return S_OK;
        }
        else if (wParam == IDS_VISITGITHUB)
        {
            ShellExecuteA(
                NULL,
                "open",
                "https://github.com/valinet/ExplorerPatcher",
                NULL,
                NULL,
                SW_SHOWNORMAL
            );
        }
        else if (wParam == IDS_VISITWEBSITE)
        {
            ShellExecuteA(
                NULL,
                "open",
                "https://www.valinet.ro",
                NULL,
                NULL,
                SW_SHOWNORMAL
            );
        }
        else if (wParam == IDS_LICENSEINFO)
        {
            ShellExecuteA(
                NULL,
                "open",
                "mailto:valentingabrielradu@gmail.com",
                NULL,
                NULL,
                SW_SHOWNORMAL
            );
        }
    }
    }
    return S_OK;
}

static BOOL GUI_Build(HDC hDC, HWND hwnd, POINT pt)
{
    GUI* _this;
    LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    _this = (int*)(ptr);
    double dx = _this->dpi.x / 96.0, dy = _this->dpi.y / 96.0;
    _this->padding.left = GUI_PADDING_LEFT * dx;
    _this->padding.right = GUI_PADDING_RIGHT * dx;
    _this->padding.top = GUI_PADDING_TOP * dy;
    _this->padding.bottom = GUI_PADDING_BOTTOM * dy;

    RECT rc;
    GetClientRect(hwnd, &rc);
    HRSRC hRscr = FindResourceA(
        hModule,
        MAKEINTRESOURCEA(IDR_REGISTRY1),
        RT_RCDATA
    );
    if (!hRscr)
    {
        return FALSE;
    }
    HGLOBAL hgRscr = LoadResource(
        hModule,
        hRscr
    );
    if (!hgRscr)
    {
        return FALSE;
    }
    PVOID pRscr = LockResource(hgRscr);
    DWORD cbRscr = SizeofResource(
        hModule,
        hRscr
    );

    LOGFONT logFont;
    memset(&logFont, 0, sizeof(logFont));
    logFont.lfHeight = GUI_CAPTION_FONT_SIZE * dy;
    logFont.lfWeight = FW_BOLD;
    wcscpy_s(logFont.lfFaceName, 32, L"Segoe UI");
    HFONT hFontCaption = CreateFontIndirect(&logFont);
    logFont.lfHeight = GUI_TITLE_FONT_SIZE * dy;
    HFONT hFontTitle = CreateFontIndirect(&logFont);
    logFont.lfWeight = FW_REGULAR;
    logFont.lfUnderline = 1;
    HFONT hFontUnderline = CreateFontIndirect(&logFont);
    logFont.lfWeight = FW_REGULAR;
    logFont.lfUnderline = 0;
    HFONT hFontRegular = CreateFontIndirect(&logFont);
    HFONT hOldFont = NULL;

    DTTOPTS DttOpts;
    DttOpts.dwSize = sizeof(DTTOPTS);
    DttOpts.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR;
    DttOpts.crText = GUI_TEXTCOLOR;
    DWORD dwTextFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
    RECT rcText;
    DWORD dwCL = 0;
    BOOL bTabOrderHit = FALSE;

    HDC hdcPaint = NULL;
    BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
    params.dwFlags = BPPF_ERASE;
    HPAINTBUFFER hBufferedPaint = BeginBufferedPaint(hDC, &rc, BPBF_TOPDOWNDIB, &params, &hdcPaint);

    if (!hDC || (hDC && hdcPaint))
    {
        FILE* f = fmemopen(pRscr, cbRscr, "r");
        char* line = malloc(MAX_LINE_LENGTH * sizeof(char));
        wchar_t* text = malloc(MAX_LINE_LENGTH * sizeof(wchar_t)); 
        wchar_t* name = malloc(MAX_LINE_LENGTH * sizeof(wchar_t));
        wchar_t* section = malloc(MAX_LINE_LENGTH * sizeof(wchar_t));
        size_t bufsiz = 0, numChRd = 0, tabOrder = 1;
        while ((numChRd = getline(&line, &bufsiz, f)) != -1)
        {
            if (strcmp(line, "Windows Registry Editor Version 5.00\r\n") && strcmp(line, "\r\n"))
            {
                if (!strncmp(line, "[", 1))
                {
                    ZeroMemory(section, MAX_LINE_LENGTH * sizeof(wchar_t));
                    MultiByteToWideChar(
                        CP_UTF8,
                        MB_PRECOMPOSED,
                        line[1] == '-' ? line + 2 : line + 1,
                        numChRd - (line[1] == '-' ? 5 : 4),
                        section,
                        MAX_LINE_LENGTH
                    );
                    //wprintf(L"%s\n", section);
                }

                DWORD dwLineHeight = GUI_LINE_HEIGHT;

                rcText.left = _this->padding.left;
                rcText.top = _this->padding.top + dwCL;
                rcText.right = (rc.right - rc.left) - _this->padding.right;
                rcText.bottom = dwCL + dwLineHeight * dy - _this->padding.bottom;

                if (!strncmp(line, ";T ", 3))
                {
                    hOldFont = SelectObject(hDC ? hdcPaint : GetDC(hwnd), hFontTitle);
                }
                else if (!strncmp(line, ";M ", 3))
                {
                    hOldFont = SelectObject(hDC ? hdcPaint : GetDC(hwnd), hFontCaption);
                }
                else if (!strncmp(line, ";u ", 3))
                {
                    hOldFont = SelectObject(hDC ? hdcPaint : GetDC(hwnd), hFontUnderline);
                }
                else
                {
                    hOldFont = SelectObject(hDC ? hdcPaint : GetDC(hwnd), hFontRegular);
                }

                if (!strncmp(line, ";T ", 3) || !strncmp(line, ";t ", 3) || !strncmp(line, ";u ", 3) || !strncmp(line, ";M ", 3))
                {
                    ZeroMemory(text, MAX_LINE_LENGTH * sizeof(wchar_t));
                    MultiByteToWideChar(
                        CP_UTF8,
                        MB_PRECOMPOSED,
                        line + 3,
                        numChRd - 3,
                        text,
                        MAX_LINE_LENGTH
                    );
                    if (!strncmp(line, ";M ", 3))
                    {
                        TCHAR exeName[MAX_PATH + 1];
                        GetProcessImageFileNameW(
                            OpenProcess(
                                PROCESS_QUERY_INFORMATION,
                                FALSE,
                                GetCurrentProcessId()
                            ),
                            exeName,
                            MAX_PATH
                        );
                        PathStripPath(exeName);
                        if (wcscmp(exeName, L"explorer.exe"))
                        {
                            LoadStringW(hModule, IDS_PRODUCTNAME, text, MAX_LINE_LENGTH);
                        }
                        else
                        {
                            LoadStringW(GetModuleHandleW(L"ExplorerFrame.dll"), 50222, text, 260);
                            wchar_t* p = wcschr(text, L'(');
                            if (p)
                            {
                                p--;
                                *p = 0;
                            }
                        }
                        rcText.bottom += GUI_CAPTION_LINE_HEIGHT - dwLineHeight;
                        dwLineHeight = GUI_CAPTION_LINE_HEIGHT;
                        _this->extent.cyTopHeight = rcText.bottom;
                    }
                    if (hDC)
                    {
                        if (!strncmp(line, ";u ", 3) && tabOrder == _this->tabOrder)
                        {
                            bTabOrderHit = TRUE;
                            DttOpts.crText = GUI_TEXTCOLOR_SELECTED;
                        }
                        DrawThemeTextEx(
                            _this->hTheme,
                            hdcPaint,
                            hOldFont ? 0 : 8,
                            0,
                            text,
                            -1,
                            dwTextFlags,
                            &rcText,
                            &DttOpts
                        );
                        if (!strncmp(line, ";u ", 3) && tabOrder == _this->tabOrder)
                        {
                            DttOpts.crText = GUI_TEXTCOLOR;
                        }
                    }
                    else
                    {
                        RECT rcTemp;
                        rcTemp = rcText;
                        DrawText(
                            GetDC(hwnd),
                            text,
                            -1,
                            &rcTemp,
                            DT_CALCRECT
                        );
                        rcTemp.bottom = rcText.bottom;
                        if (!strncmp(line, ";u ", 3) && (PtInRect(&rcTemp, pt) || (pt.x == 0 && pt.y == 0 && tabOrder == _this->tabOrder)))
                        {
                            numChRd = getline(&line, &bufsiz, f);
                            char* p = strchr(line, '\r');
                            if (p) *p = 0;
                            p = strchr(line, '\n');
                            if (p) *p = 0;
                            if (!strncmp(line + 1, "restart", 7))
                            {
                                PROCESSENTRY32 pe32 = { 0 };
                                pe32.dwSize = sizeof(PROCESSENTRY32);
                                HANDLE hSnapshot = CreateToolhelp32Snapshot(
                                    TH32CS_SNAPPROCESS,
                                    0
                                );
                                if (Process32First(hSnapshot, &pe32) == TRUE)
                                {
                                    do
                                    {
                                        if (!wcscmp(pe32.szExeFile, TEXT("sihost.exe")))
                                        {
                                            HANDLE hSihost = OpenProcess(
                                                PROCESS_TERMINATE,
                                                FALSE,
                                                pe32.th32ProcessID
                                            );
                                            TerminateProcess(hSihost, 0);
                                            CloseHandle(hSihost);
                                            return TRUE;
                                        }
                                    } while (Process32Next(hSnapshot, &pe32) == TRUE);
                                }
                                CloseHandle(hSnapshot);
                            }
                            else if (!strncmp(line + 1, "reset", 5))
                            {
                                wchar_t wszPath[MAX_PATH];
                                ZeroMemory(
                                    wszPath,
                                    MAX_PATH * sizeof(wchar_t)
                                );
                                SHGetFolderPathW(
                                    NULL,
                                    SPECIAL_FOLDER,
                                    NULL,
                                    SHGFP_TYPE_CURRENT,
                                    wszPath
                                );
                                wcscat_s(
                                    wszPath,
                                    MAX_PATH,
                                    TEXT(APP_RELATIVE_PATH)
                                );
                                CreateDirectoryW(wszPath, NULL);
                                wcscat_s(
                                    wszPath,
                                    MAX_PATH,
                                    L"\\settings.reg"
                                );
                                wprintf(L"%s\n", wszPath);
                                HANDLE hFile = CreateFileW(
                                    wszPath,
                                    GENERIC_WRITE,
                                    0,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                );
                                if (hFile)
                                {
                                    DWORD dwNumberOfBytesWritten = 0;
                                    if (WriteFile(
                                        hFile,
                                        pRscr,
                                        cbRscr,
                                        &dwNumberOfBytesWritten,
                                        NULL
                                    ))
                                    {
                                        CloseHandle(hFile);

                                        SHELLEXECUTEINFO ShExecInfo = { 0 };
                                        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
                                        ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
                                        ShExecInfo.hwnd = NULL;
                                        ShExecInfo.lpVerb = NULL;
                                        ShExecInfo.lpFile = wszPath;
                                        ShExecInfo.lpParameters = L"";
                                        ShExecInfo.lpDirectory = NULL;
                                        ShExecInfo.nShow = SW_SHOW;
                                        ShExecInfo.hInstApp = NULL;
                                        ShellExecuteEx(&ShExecInfo);
                                        WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
                                        DWORD dwExitCode = 0;
                                        GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
                                        _this->tabOrder = 0;
                                        InvalidateRect(hwnd, NULL, FALSE);
                                        CloseHandle(ShExecInfo.hProcess);
                                        DeleteFileW(wszPath);
                                    }
                                }
                            }
                            else if (!strncmp(line + 1, "about", 5))
                            {
                                DWORD dwLeftMost = 0;
                                DWORD dwSecondLeft = 0;
                                DWORD dwSecondRight = 0;
                                DWORD dwRightMost = 0;

                                QueryVersionInfo(hModule, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

                                TCHAR wszIDS_VISITGITHUB[100];
                                LoadString(hModule, IDS_VISITGITHUB, wszIDS_VISITGITHUB, 100);
                                TCHAR wszIDS_VISITWEBSITE[100];
                                LoadString(hModule, IDS_VISITWEBSITE, wszIDS_VISITWEBSITE, 100);
                                TCHAR wszIDS_LICENSEINFO[100];
                                LoadString(hModule, IDS_LICENSEINFO, wszIDS_LICENSEINFO, 100);
                                TCHAR wszIDS_PRODUCTNAME[100];
                                LoadString(hModule, IDS_PRODUCTNAME, wszIDS_PRODUCTNAME, 100);
                                TCHAR wszIDS_VERSION[100];
                                LoadString(hModule, IDS_VERSION, wszIDS_VERSION, 100);
                                TCHAR wszIDS_PRODUCTTAG[406];
                                wsprintf(wszIDS_PRODUCTTAG, wszIDS_VERSION, dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
                                wcscat_s(
                                    wszIDS_PRODUCTTAG,
                                    406,
                                    L"\r\n"
                                );
                                LoadString(hModule, IDS_COPYRIGHT, wszIDS_PRODUCTTAG + wcslen(wszIDS_PRODUCTTAG), 100);
                                wcscat_s(
                                    wszIDS_PRODUCTTAG,
                                    406,
                                    L"\r\n\r\n"
                                );
                                LoadString(hModule, IDS_PRODUCTTAG, wszIDS_PRODUCTTAG + wcslen(wszIDS_PRODUCTTAG), 200);

                                TASKDIALOG_BUTTON buttons[3];
                                buttons[0].nButtonID = IDS_VISITGITHUB;
                                buttons[0].pszButtonText = wszIDS_VISITGITHUB;
                                buttons[1].nButtonID = IDS_VISITWEBSITE;
                                buttons[1].pszButtonText = wszIDS_VISITWEBSITE;
                                buttons[2].nButtonID = IDS_LICENSEINFO;
                                buttons[2].pszButtonText = wszIDS_LICENSEINFO;

                                TASKDIALOGCONFIG td;
                                ZeroMemory(&td, sizeof(TASKDIALOGCONFIG));
                                td.cbSize = sizeof(TASKDIALOGCONFIG);
                                td.hwndParent = hwnd;
                                td.hInstance = hModule;
                                td.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_SIZE_TO_CONTENT | TDF_USE_COMMAND_LINKS;
                                td.dwCommonButtons = TDCBF_OK_BUTTON;
                                td.pszWindowTitle = L" ";
                                td.pszMainIcon = TD_INFORMATION_ICON;
                                td.pszMainInstruction = wszIDS_PRODUCTNAME;
                                td.pszContent = wszIDS_PRODUCTTAG;
                                td.cButtons = sizeof(buttons) / sizeof(buttons[0]);
                                td.pButtons = buttons;
                                td.nDefaultButton = IDOK;
                                td.cRadioButtons = 0;
                                td.pRadioButtons = NULL;
                                td.cxWidth = 0;
                                td.pszFooter = L"";
                                td.pfCallback = GUI_AboutProc;
                                td.lpCallbackData = 0;
                                int ret;

                                // If used directly, StartMenuExperienceHost.exe crashes badly and is unable to start; guess how I know...
                                (HRESULT(*)(const TASKDIALOGCONFIG*, int*, int*, BOOL*))(GetProcAddress(GetModuleHandleA("Comctl32.dll"), "TaskDialogIndirect"))(
                                    &td,
                                    &ret,
                                    NULL,
                                    NULL
                                );
                            }
                        }
                    }
                    dwCL += dwLineHeight * dy;
                    if (!strncmp(line, ";u ", 3))
                    {
                        tabOrder++;
                    }
                }
                else if (!strncmp(line, ";l ", 3) || !strncmp(line, ";c ", 3) || !strncmp(line, ";b ", 3) || !strncmp(line, ";i ", 3) || !strncmp(line, ";d ", 3) || !strncmp(line, ";v ", 3))
                {
                    ZeroMemory(text, MAX_LINE_LENGTH * sizeof(wchar_t));
                    text[0] = L'\u2795';
                    text[1] = L' ';
                    text[2] = L' ';
                    MultiByteToWideChar(
                        CP_UTF8,
                        MB_PRECOMPOSED,
                        !strncmp(line, ";c ", 3) ? strchr(line + 3, ' ') + 1 : line + 3,
                        numChRd - 3,
                        text + 3,
                        MAX_LINE_LENGTH
                    );
                    wchar_t* x = wcschr(text, L'\n');
                    if (x) *x = 0;
                    x = wcschr(text, L'\r');
                    if (x) *x = 0;
                    if (!strncmp(line, ";c ", 3) || !strncmp(line, ";b ", 3) || !strncmp(line, ";i ", 3) || !strncmp(line, ";d ", 3) || !strncmp(line, ";v ", 3))
                    {
                        HMENU hMenu = NULL;
                        BOOL bChoice = !strncmp(line, ";c ", 3);
                        BOOL bInvert = !strncmp(line, ";i ", 3);
                        BOOL bJustCheck = !strncmp(line, ";d ", 3);
                        BOOL bBool = !strncmp(line, ";b ", 3);
                        BOOL bValue = !strncmp(line, ";v ", 3);
                        DWORD numChoices = 0;
                        if (bChoice)
                        {
                            char* p = strchr(line + 3, ' ');
                            if (p) *p = 0;
                            numChoices = atoi(line + 3);
                            hMenu = CreatePopupMenu();
                            for (unsigned int i = 0; i < numChoices; ++i)
                            {
                                char* l = malloc(MAX_LINE_LENGTH * sizeof(char));
                                numChRd = getline(&l, &bufsiz, f);
                                if (strncmp(l, ";x ", 3))
                                {
                                    i--;
                                    continue;
                                }
                                char* p = strchr(l + 3, ' ');
                                if (p) *p = 0;
                                char* ln = p + 1;
                                p = strchr(p + 1, '\r');
                                if (p) *p = 0;
                                p = strchr(p + 1, '\n');
                                if (p) *p = 0;

                                MENUITEMINFOA menuInfo;
                                ZeroMemory(&menuInfo, sizeof(MENUITEMINFOA));
                                menuInfo.cbSize = sizeof(MENUITEMINFOA);
                                menuInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA | MIIM_STATE;
                                menuInfo.wID = atoi(l + 3) + 1;
                                menuInfo.dwItemData = l;
                                menuInfo.fType = MFT_STRING;
                                menuInfo.dwTypeData = ln;
                                menuInfo.cch = strlen(ln);
                                InsertMenuItemA(
                                    hMenu,
                                    i,
                                    TRUE,
                                    &menuInfo
                                );
                            }
                        }
                        numChRd = getline(&line, &bufsiz, f);
                        ZeroMemory(name, MAX_LINE_LENGTH * sizeof(wchar_t));
                        MultiByteToWideChar(
                            CP_UTF8,
                            MB_PRECOMPOSED,
                            line[0] == '"' ? line + 1 : line,
                            numChRd,
                            name,
                            MAX_LINE_LENGTH
                        );
                        wchar_t* d = wcschr(name, L'=');
                        if (d) *d = 0;
                        wchar_t* p = wcschr(name, L'"');
                        if (p) *p = 0;
                        HKEY hKey;
                        DWORD dwDisposition;
                        DWORD dwSize = sizeof(DWORD);
                        DWORD value = FALSE;

                        //wprintf(L"%s %s %s\n", section, name, d + 1);
                        if (!wcsncmp(d + 1, L"dword:", 6))
                        {
                            wchar_t* x = wcschr(d + 1, L':');
                            x++;
                            value = _wtoi(x);
                        }

                        if (!bJustCheck)
                        {
                            RegCreateKeyExW(
                                HKEY_CURRENT_USER,
                                wcschr(section, L'\\') + 1,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ | (hDC ? 0 : KEY_WRITE),
                                NULL,
                                &hKey,
                                &dwDisposition
                            );
                            RegQueryValueExW(
                                hKey,
                                name,
                                0,
                                NULL,
                                &value,
                                &dwSize
                            );
                            if (hDC && bInvert)
                            {
                                value = !value;
                            }
                        }
                        else
                        {
                            RegOpenKeyExW(
                                HKEY_CURRENT_USER,
                                wcschr(section, L'\\') + 1,
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ | (hDC ? 0 : KEY_WRITE),
                                &hKey
                            );
                            value = hKey;
                        }
                        if (bInvert || bBool || bJustCheck)
                        {
                            if (value)
                            {
                                text[0] = L'\u2714';
                            }
                            else
                            {
                                text[0] = L'\u274C';
                            }
                            text[1] = L' ';
                            text[2] = L' ';
                        }
                        else if (bValue)
                        {
                            wcscat_s(
                                text,
                                MAX_LINE_LENGTH,
                                L" : "
                            );
                            wchar_t buf[100];
                            _itow_s(value, buf, 100, 10);
                            wcscat_s(
                                text,
                                MAX_LINE_LENGTH,
                                buf
                            );
                        }
                        else if (bChoice)
                        {
                            wcscat_s(
                                text,
                                MAX_LINE_LENGTH,
                                L" : "
                            );
                            MENUITEMINFOA menuInfo;
                            ZeroMemory(&menuInfo, sizeof(MENUITEMINFOA));
                            menuInfo.cbSize = sizeof(MENUITEMINFOA);
                            menuInfo.fMask = MIIM_STRING;
                            GetMenuItemInfoA(hMenu, value + 1, FALSE, &menuInfo);
                            char* buf = malloc(sizeof(char) * (menuInfo.cch + 1));
                            menuInfo.dwTypeData = buf;
                            menuInfo.cch += 1;
                            GetMenuItemInfoA(hMenu, value + 1, FALSE, &menuInfo);
                            MultiByteToWideChar(
                                CP_UTF8,
                                MB_PRECOMPOSED,
                                buf,
                                menuInfo.cch,
                                text + wcslen(text),
                                MAX_LINE_LENGTH
                            );
                        }
                        RECT rcTemp;
                        rcTemp = rcText;
                        DrawText(
                            GetDC(hwnd),
                            text,
                            -1,
                            &rcTemp,
                            DT_CALCRECT
                        );
                        rcTemp.bottom = rcText.bottom;
                        if (!hDC && (PtInRect(&rcTemp, pt) || (pt.x == 0 && pt.y == 0 && tabOrder == _this->tabOrder)))
                        {
                            if (bJustCheck)
                            {
                                if (hKey)
                                {
                                    RegCloseKey(hKey);
                                    RegDeleteKeyExW(
                                        HKEY_CURRENT_USER,
                                        wcschr(section, L'\\') + 1,
                                        REG_OPTION_NON_VOLATILE,
                                        0
                                    );
                                }
                                else
                                {
                                    RegCreateKeyExW(
                                        HKEY_CURRENT_USER,
                                        wcschr(section, L'\\') + 1,
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_WRITE,
                                        NULL,
                                        &hKey,
                                        &dwDisposition
                                    );
                                    if (d[1] == '"')
                                    {
                                        wchar_t* p = wcschr(d + 2, L'"');
                                        if (p) *p = 0;
                                        RegSetValueExW(
                                            hKey,
                                            !wcsncmp(name, L"@", 1) ? NULL : name,
                                            0,
                                            REG_SZ,
                                            d + 2,
                                            wcslen(d + 2) * sizeof(wchar_t)
                                        );
                                    }
                                }
                            }
                            else
                            {
                                if (bChoice)
                                {
                                    POINT p;
                                    p.x = rcText.left;
                                    p.y = rcText.bottom;
                                    ClientToScreen(
                                        hwnd,
                                        &p
                                    );
                                    DWORD val = TrackPopupMenu(
                                        hMenu, 
                                        TPM_RETURNCMD | TPM_RIGHTBUTTON,
                                        p.x,
                                        p.y,
                                        0,
                                        hwnd,
                                        0
                                    );
                                    if (val > 0) value = val - 1;
                                }
                                else if (bValue)
                                {

                                }
                                else
                                {
                                    value = !value;
                                }
                                RegSetValueExW(
                                    hKey,
                                    name,
                                    0,
                                    REG_DWORD,
                                    &value,
                                    sizeof(DWORD)
                                );
                            }
                            InvalidateRect(hwnd, NULL, FALSE);
                        }
                        RegCloseKey(hKey);
                        if (bChoice)
                        {
                            for (unsigned int i = 0; i < numChoices; ++i)
                            {
                                MENUITEMINFOA menuInfo;
                                ZeroMemory(&menuInfo, sizeof(MENUITEMINFOA));
                                menuInfo.cbSize = sizeof(MENUITEMINFOA);
                                menuInfo.fMask = MIIM_DATA;
                                GetMenuItemInfoA(hMenu, i, TRUE, &menuInfo);
                                if (menuInfo.dwItemData)
                                {
                                    free(menuInfo.dwItemData);
                                }
                            }
                            DestroyMenu(hMenu);
                        }
                    }
                    if (!hDC && !strncmp(line, ";l ", 3))
                    {
                        RECT rcTemp;
                        rcTemp = rcText;
                        DrawText(
                            GetDC(hwnd),
                            text,
                            -1,
                            &rcTemp,
                            DT_CALCRECT
                        );
                        rcTemp.bottom = rcText.bottom;
                        //printf("%d %d %d %d %d %d %d %d\n", rcText.left, rcText.top, rcText.right, rcText.bottom, rcTemp.left, rcTemp.top, rcTemp.right, rcTemp.bottom);
                        if (PtInRect(&rcTemp, pt) || (pt.x == 0 && pt.y == 0 && tabOrder == _this->tabOrder))
                        {
                            numChRd = getline(&line, &bufsiz, f);
                            char* p = strchr(line, '\r');
                            if (p) *p = 0;
                            p = strchr(line, '\n');
                            if (p) *p = 0;
                            ShellExecuteA(
                                NULL,
                                "open",
                                line + 1,
                                NULL,
                                NULL,
                                SW_SHOWNORMAL
                            );
                        }
                    }
                    if (hDC)
                    {
                        if (tabOrder == _this->tabOrder)
                        {
                            bTabOrderHit = TRUE;
                            DttOpts.crText = GUI_TEXTCOLOR_SELECTED;
                        }
                        DrawThemeTextEx(
                            _this->hTheme,
                            hdcPaint,
                            0,
                            0,
                            text,
                            -1,
                            dwTextFlags,
                            &rcText,
                            &DttOpts
                        );
                        if (tabOrder == _this->tabOrder)
                        {
                            DttOpts.crText = GUI_TEXTCOLOR;
                        }
                    }
                    dwCL += dwLineHeight * dy;
                    tabOrder++;
                }
            }
        }
        fclose(f);

        if (hDC)
        {
            SelectObject(hdcPaint, hOldFont);
            DeleteObject(hFontRegular);
            DeleteObject(hFontTitle);
            DeleteObject(hFontUnderline);
            DeleteObject(hFontCaption);
            if (_this->tabOrder == GUI_MAX_TABORDER)
            {
                _this->tabOrder = tabOrder;
            }
            else if (!bTabOrderHit)
            {
                _this->tabOrder = 0;
            }
        }
        EndBufferedPaint(hBufferedPaint, TRUE);
    }
    return TRUE;
}

static LRESULT CALLBACK GUI_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    GUI* _this;
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
        _this = (int*)(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)_this);
        UINT dpiX, dpiY, dpiXP, dpiYP;
        POINT ptCursor, ptZero;
        ptZero.x = 0;
        ptZero.y = 0;
        GetCursorPos(&ptCursor);
        HMONITOR hMonitor = MonitorFromPoint(ptCursor, MONITOR_DEFAULTTOPRIMARY);
        HMONITOR hPrimaryMonitor = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
        HRESULT hr = GetDpiForMonitor(
            hMonitor,
            MDT_DEFAULT,
            &dpiX,
            &dpiY
        );
        hr = GetDpiForMonitor(
            hPrimaryMonitor,
            MDT_DEFAULT,
            &dpiXP,
            &dpiYP
        );
        MONITORINFO mi;
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hMonitor, &mi);
        double dx = dpiX / 96.0, dy = dpiY / 96.0, dxp = dpiXP / 96.0, dyp = dpiYP / 96.0;
        _this->dpi.x = dpiX;
        _this->dpi.y = dpiY;
        SetWindowPos(
            hWnd, 
            hWnd, 
            mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left) / 2 - (_this->size.cx * dx) / 2),
            mi.rcWork.top + ((mi.rcWork.bottom - mi.rcWork.top) / 2 - (_this->size.cy * dy) / 2),
            _this->size.cx * dxp, 
            _this->size.cy * dyp,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
    }
    else
    {
        LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
        _this = (int*)(ptr);
    }
    if (uMsg == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }
    else if (uMsg == WM_KEYDOWN)
    {
        if (wParam == VK_ESCAPE)
        {
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            return 0;
        }
        else if (wParam == VK_TAB)
        {
            if (GetKeyState(VK_SHIFT) & 0x8000)
            {
                _this->tabOrder--;
                if (_this->tabOrder == 0)
                {
                    _this->tabOrder = GUI_MAX_TABORDER;
                }
            }
            else
            {
                _this->tabOrder++;
            }
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;
        }
        else if (wParam == VK_SPACE)
        {
            POINT pt;
            pt.x = 0;
            pt.y = 0;
            GUI_Build(0, hWnd, pt);
            return 0;
        }
    }
    else if (uMsg == WM_NCHITTEST)
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ScreenToClient(hWnd, &pt);
        if (pt.y < _this->extent.cyTopHeight)
        {
            return HTCAPTION;
        }
    }
    else if (uMsg == WM_LBUTTONDOWN)
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        GUI_Build(0, hWnd, pt);
        _this->tabOrder = 0;
        InvalidateRect(hWnd, NULL, FALSE);
    }
    else if (uMsg == WM_DPICHANGED)
    {
        _this->dpi.x = LOWORD(wParam);
        _this->dpi.y = HIWORD(wParam);
        RECT* rc = lParam;
        SetWindowPos(
            hWnd,
            hWnd,
            rc->left,
            rc->top,
            rc->right - rc->left,
            rc->bottom - rc->top,
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS
        );
        return 0;
    }
    else if (uMsg == WM_PAINT)
    {
        MARGINS marGlassInset = { -1, -1, -1, -1 }; // -1 means the whole window
        DwmExtendFrameIntoClientArea(hWnd, &marGlassInset);

        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hWnd, &ps);

        RECT rc;
        GetClientRect(hWnd, &rc);

        POINT pt;
        pt.x = 0;
        pt.y = 0;
        GUI_Build(hDC, hWnd, pt);

        EndPaint(hWnd, &ps);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

__declspec(dllexport) int ZZGUI(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    HKEY hKey;
    DWORD dwDisposition;
    DWORD dwSize = sizeof(DWORD);
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
    DWORD bAllocConsole = FALSE;
    RegQueryValueExW(
        hKey,
        TEXT("AllocConsole"),
        0,
        NULL,
        &bAllocConsole,
        &dwSize
    );
    if (bAllocConsole)
    {
        FILE* conout;
        AllocConsole();
        freopen_s(
            &conout,
            "CONOUT$",
            "w",
            stdout
        );
    }

    printf("Started \"GUI\" thread.\n");

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    GUI _this;
    _this.hBackgroundBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
    _this.location.x = GUI_POSITION_X;
    _this.location.y = GUI_POSITION_Y;
    _this.size.cx = GUI_POSITION_WIDTH;
    _this.size.cy = GUI_POSITION_HEIGHT;
    _this.padding.left = GUI_PADDING_LEFT;
    _this.padding.right = GUI_PADDING_RIGHT;
    _this.padding.top = GUI_PADDING_TOP;
    _this.padding.bottom = GUI_PADDING_BOTTOM;
    _this.hTheme = OpenThemeData(NULL, TEXT(GUI_WINDOWSWITCHER_THEME_CLASS));
    _this.tabOrder = 0;

    WNDCLASS wc = { 0 };
    ZeroMemory(&wc, sizeof(WNDCLASSW));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = GUI_WindowProc;
    wc.hbrBackground = _this.hBackgroundBrush;
    wc.hInstance = hModule;
    wc.lpszClassName = L"ExplorerPatcherGUI";
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    TCHAR title[260];
    LoadStringW(GetModuleHandleW(L"ExplorerFrame.dll"), 726, title, 260);
    if (title[0] == 0)
    {
        LoadStringW(hModule, IDS_PRODUCTNAME, title, 260);
    }

    HWND hwnd = CreateWindowEx(
        NULL,
        L"ExplorerPatcherGUI",
        title,
        WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
        0,
        0,
        0,
        0,
        NULL, NULL, hModule, &_this
    );
    if (!hwnd)
    {
        return 1;
    }
    BOOL value = 1;
    DwmSetWindowAttribute(hwnd, 1029, &value, sizeof(BOOL));
    WTA_OPTIONS ops;
    ops.dwFlags = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
    ops.dwMask = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
    SetWindowThemeAttribute(
        hwnd,
        WTA_NONCLIENT,
        &ops,
        sizeof(WTA_OPTIONS)
    );
    ShowWindow(hwnd, SW_SHOW);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    printf("Ended \"GUI\" thread.\n");
}
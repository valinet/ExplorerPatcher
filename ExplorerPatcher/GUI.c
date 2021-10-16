#include "GUI.h"

void* GUI_FileMapping = NULL;
DWORD GUI_FileSize = 0;
BOOL g_darkModeEnabled = FALSE;
static void(*RefreshImmersiveColorPolicyState)() = NULL;
static int(*SetPreferredAppMode)(int bAllowDark) = NULL;
static BOOL(*AllowDarkModeForWindow)(HWND hWnd, BOOL bAllowDark) = NULL;
static BOOL(*ShouldAppsUseDarkMode)() = NULL;
BOOL IsHighContrast()
{
    HIGHCONTRASTW highContrast;
    ZeroMemory(&highContrast, sizeof(HIGHCONTRASTW));
    highContrast.cbSize = sizeof(highContrast);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, FALSE))
        return highContrast.dwFlags & HCF_HIGHCONTRASTON;
    return FALSE;
}
BOOL IsColorSchemeChangeMessage(LPARAM lParam)
{
    BOOL is = FALSE;
    if (lParam && CompareStringOrdinal(lParam, -1, L"ImmersiveColorSet", -1, TRUE) == CSTR_EQUAL)
    {
        RefreshImmersiveColorPolicyState();
        is = TRUE;
    }
    return is;
}

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
    _this->sidebarWidth = GUI_SIDEBAR_WIDTH * dx;

    RECT rc;
    GetClientRect(hwnd, &rc);

    PVOID pRscr = NULL;
    DWORD cbRscr = 0;
    if (GUI_FileMapping && GUI_FileSize)
    {
        pRscr = GUI_FileMapping;
        cbRscr = GUI_FileSize;
    }
    else
    {
        HRSRC hRscr = FindResource(
            hModule,
            MAKEINTRESOURCE(IDR_REGISTRY1),
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
        pRscr = LockResource(hgRscr);
        cbRscr = SizeofResource(
            hModule,
            hRscr
        );
    }

    LOGFONT logFont;
    memset(&logFont, 0, sizeof(logFont));
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfoW(
        SPI_GETNONCLIENTMETRICS,
        sizeof(NONCLIENTMETRICS),
        &ncm,
        0
    );
    logFont = ncm.lfCaptionFont;
    logFont.lfHeight = GUI_CAPTION_FONT_SIZE * dy;
    logFont.lfWeight = FW_BOLD;
    HFONT hFontCaption = CreateFontIndirect(&logFont);
    logFont.lfHeight = GUI_TITLE_FONT_SIZE * dy;
    HFONT hFontTitle = CreateFontIndirect(&logFont);
    logFont.lfWeight = FW_REGULAR;
    logFont.lfUnderline = 1;
    HFONT hFontUnderline = CreateFontIndirect(&logFont);
    logFont.lfWeight = FW_REGULAR;
    logFont.lfUnderline = 0;
    HFONT hFontRegular = CreateFontIndirect(&logFont);
    logFont.lfWeight = FW_DEMIBOLD;
    logFont.lfHeight = GUI_SECTION_FONT_SIZE * dy;
    HFONT hFontSection = CreateFontIndirect(&logFont);
    logFont.lfUnderline = 1;
    HFONT hFontSectionSel = CreateFontIndirect(&logFont);
    HFONT hOldFont = NULL;

    DTTOPTS DttOpts;
    DttOpts.dwSize = sizeof(DTTOPTS);
    DttOpts.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR;
    //DttOpts.crText = GetSysColor(COLOR_WINDOWTEXT);
    DttOpts.crText = g_darkModeEnabled ? GUI_TEXTCOLOR_DARK : GUI_TEXTCOLOR;
    DWORD dwTextFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
    RECT rcText;
    DWORD dwMaxHeight = 0, dwMaxWidth = 0;
    BOOL bTabOrderHit = FALSE;
    DWORD dwLeftPad = _this->padding.left + _this->sidebarWidth + _this->padding.right;
    DWORD dwInitialLeftPad = dwLeftPad;

    HDC hdcPaint = NULL;
    BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
    params.dwFlags = BPPF_ERASE;
    HPAINTBUFFER hBufferedPaint = BeginBufferedPaint(hDC, &rc, BPBF_TOPDOWNDIB, &params, &hdcPaint);

    if (!hDC || (hDC && hdcPaint))
    {
        if (!hDC)
        {
            hdcPaint = GetDC(hwnd);
        }

        if (!IsThemeActive() && hDC)
        {
            COLORREF oldcr = SetBkColor(hdcPaint, GetSysColor(COLOR_WINDOW));
            ExtTextOutW(hdcPaint, 0, 0, ETO_OPAQUE, &rc, L"", 0, 0);
            SetBkColor(hdcPaint, oldcr);
            SetTextColor(hdcPaint, GetSysColor(COLOR_WINDOWTEXT));
        }

        FILE* f = fmemopen(pRscr, cbRscr, "r");
        char* line = malloc(MAX_LINE_LENGTH * sizeof(char));
        wchar_t* text = malloc((MAX_LINE_LENGTH + 3) * sizeof(wchar_t)); 
        wchar_t* name = malloc(MAX_LINE_LENGTH * sizeof(wchar_t));
        wchar_t* section = malloc(MAX_LINE_LENGTH * sizeof(wchar_t));
        size_t bufsiz = MAX_LINE_LENGTH, numChRd = 0, tabOrder = 1, currentSection = -1, topAdj = 0;
        while ((numChRd = getline(&line, &bufsiz, f)) != -1)
        {
            if (strcmp(line, "Windows Registry Editor Version 5.00\r\n") && strcmp(line, "\r\n") && (currentSection == -1 || currentSection == _this->section || !strncmp(line, ";T ", 3) || !strncmp(line, ";f", 2)))
            {
#ifndef USE_PRIVATE_INTERFACES
                if (!strncmp(line, ";p ", 3))
                {
                    int num = atoi(line + 3);
                    for (int i = 0; i < num; ++i)
                    {
                        getline(&line, &bufsiz, f);
                    }
                }
#endif
                if (!strncmp(line, ";f", 2))
                {
                    //if (topAdj + ((currentSection + 2) * GUI_SECTION_HEIGHT * dy) > dwMaxHeight)
                    //{
                    //    dwMaxHeight = topAdj + ((currentSection + 2) * GUI_SECTION_HEIGHT * dy);
                    //}
                    if (_this->dwStatusbarY == 0)
                    {
                        dwMaxHeight += GUI_STATUS_PADDING * dy;
                        _this->dwStatusbarY = dwMaxHeight / dy;
                    }
                    else
                    {
                        dwMaxHeight = _this->dwStatusbarY * dy;
                    }
                    currentSection = -1;
                    dwLeftPad = 0;
                    continue;
                }

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
                DWORD dwBottom = _this->padding.bottom;
                DWORD dwTop = _this->padding.top;
                if (!strncmp(line, ";a ", 3) || !strncmp(line, ";e ", 3))
                {
                    dwBottom = 0;
                    dwLineHeight -= 0.2 * dwLineHeight;
                }

                rcText.left = dwLeftPad + _this->padding.left;
                rcText.top = dwTop + dwMaxHeight;
                rcText.right = (rc.right - rc.left) - _this->padding.right;
                rcText.bottom = dwMaxHeight + dwLineHeight * dy - dwBottom;

                if (!strncmp(line, ";T ", 3))
                {
                    if (currentSection + 1 == _this->section)
                    {
                        hOldFont = SelectObject(hdcPaint, hFontSectionSel);
                    }
                    else
                    {
                        hOldFont = SelectObject(hdcPaint, hFontSection);
                    }
                    rcText.left = _this->padding.left;
                    rcText.right = _this->padding.left + _this->sidebarWidth;
                    rcText.top = topAdj + ((currentSection + 1) * GUI_SECTION_HEIGHT * dy);
                    rcText.bottom = topAdj + ((currentSection + 2) * GUI_SECTION_HEIGHT * dy);
                    ZeroMemory(text, (MAX_LINE_LENGTH + 3) * sizeof(wchar_t));
                    MultiByteToWideChar(
                        CP_UTF8,
                        MB_PRECOMPOSED,
                        line + 3,
                        numChRd - 3,
                        text,
                        MAX_LINE_LENGTH
                    );
                    if (hDC)
                    {
                        if (IsThemeActive())
                        {
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
                        }
                        else
                        {
                            DrawTextW(
                                hdcPaint,
                                text,
                                -1,
                                &rcText,
                                dwTextFlags
                            );
                        }
                    }
                    else
                    {
                        RECT rcTemp;
                        rcTemp = rcText;
                        DrawTextW(
                            hdcPaint,
                            text,
                            -1,
                            &rcTemp,
                            DT_CALCRECT
                        );
                        rcTemp.bottom = rcText.bottom;
                        if (PtInRect(&rcTemp, pt))
                        {
                            _this->section = currentSection + 1;
                            InvalidateRect(hwnd, NULL, FALSE);
                        }
                    }
                    currentSection++;
                    continue;
                }
                else if (!strncmp(line, ";M ", 3))
                {
                    rcText.left = _this->padding.left;
                    topAdj = dwMaxHeight + GUI_CAPTION_LINE_HEIGHT * dy;
                    hOldFont = SelectObject(hdcPaint, hFontCaption);
                }
                else if (!strncmp(line, ";u ", 3))
                {
                    hOldFont = SelectObject(hdcPaint, hFontUnderline);
                }
                else
                {
                    hOldFont = SelectObject(hdcPaint, hFontRegular);
                }

                if (!strncmp(line, ";e ", 3) || !strncmp(line, ";a ", 3) || !strncmp(line, ";T ", 3) || !strncmp(line, ";t ", 3) || !strncmp(line, ";u ", 3) || !strncmp(line, ";M ", 3))
                {
                    if (!strncmp(line, ";t ", 3) || !strncmp(line, ";e ", 3) || !strncmp(line, ";a ", 3))
                    {
                        char* p = strstr(line, "%VERSIONINFO%");
                        if (p)
                        {
                            DWORD dwLeftMost = 0;
                            DWORD dwSecondLeft = 0;
                            DWORD dwSecondRight = 0;
                            DWORD dwRightMost = 0;

                            QueryVersionInfo(hModule, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

                            sprintf_s(p, MAX_PATH, "%d.%d.%d.%d", dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
                        }
                    }
                    ZeroMemory(text, (MAX_LINE_LENGTH + 3) * sizeof(wchar_t));
                    MultiByteToWideChar(
                        CP_UTF8,
                        MB_PRECOMPOSED,
                        line + 3,
                        numChRd - 3,
                        text,
                        MAX_LINE_LENGTH
                    );
                    if (!strncmp(line, ";a ", 3))
                    {
                        RECT rcTemp;
                        rcTemp = rcText;
                        DrawTextW(
                            hdcPaint,
                            L"\u2795  ",
                            3,
                            &rcTemp,
                            DT_CALCRECT
                        );
                        rcText.left += rcTemp.right - rcTemp.left;
                        rcText.right += rcTemp.right - rcTemp.left;
                    }
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
                        //if (wcscmp(exeName, L"explorer.exe"))
                        //{
                        //    LoadStringW(hModule, IDS_PRODUCTNAME, text, MAX_LINE_LENGTH);
                        //}
                        //else
                        //{
                            HMODULE hExplorerFrame = LoadLibraryExW(L"ExplorerFrame.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
                            LoadStringW(hExplorerFrame, 50222, text, 260);
                            FreeLibrary(hExplorerFrame);
                            wchar_t* p = wcschr(text, L'(');
                            if (p)
                            {
                                p--;
                                if (p == L' ')
                                {
                                    *p = 0;
                                }
                                else
                                {
                                    p++;
                                    *p = 0;
                                }
                            }
                        //}
                        rcText.bottom += GUI_CAPTION_LINE_HEIGHT - dwLineHeight;
                        dwLineHeight = GUI_CAPTION_LINE_HEIGHT;
                        _this->extent.cyTopHeight = rcText.bottom;
                    }
                    if (hDC)
                    {
                        COLORREF cr;
                        if (!strncmp(line, ";u ", 3) && tabOrder == _this->tabOrder)
                        {
                            bTabOrderHit = TRUE;
                            if (!IsThemeActive())
                            {
                                cr = SetTextColor(hdcPaint, GetSysColor(COLOR_HIGHLIGHT));
                            }
                            else
                            {
                                DttOpts.crText = g_darkModeEnabled ? GUI_TEXTCOLOR_SELECTED_DARK : GUI_TEXTCOLOR_SELECTED;
                                //DttOpts.crText = GetSysColor(COLOR_HIGHLIGHT);
                            }
                        }
                        RECT rcNew = rcText;
                        DrawTextW(
                            hdcPaint,
                            text,
                            -1,
                            &rcNew,
                            DT_CALCRECT
                        );
                        if (rcNew.right - rcNew.left > dwMaxWidth)
                        {
                            dwMaxWidth = rcNew.right - rcNew.left + 50 * dx;
                        }
                        if (IsThemeActive())
                        {
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
                        }
                        else
                        {
                            DrawTextW(
                                hdcPaint,
                                text,
                                -1,
                                &rcText,
                                dwTextFlags
                            );
                        }
                        if (!strncmp(line, ";u ", 3) && tabOrder == _this->tabOrder)
                        {
                            if (!IsThemeActive())
                            {
                                SetTextColor(hdcPaint, cr);
                            }
                            else
                            {
                                DttOpts.crText = g_darkModeEnabled ? GUI_TEXTCOLOR_DARK : GUI_TEXTCOLOR;
                                //DttOpts.crText = GetSysColor(COLOR_WINDOWTEXT);
                            }
                        }
                    }
                    else
                    {
                        RECT rcTemp;
                        rcTemp = rcText;
                        DrawTextW(
                            hdcPaint,
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
                    dwMaxHeight += dwLineHeight * dy;
                    if (!strncmp(line, ";u ", 3))
                    {
                        tabOrder++;
                    }
                }
                else if (!strncmp(line, ";l ", 3) || !strncmp(line, ";c ", 3) || !strncmp(line, ";b ", 3) || !strncmp(line, ";i ", 3) || !strncmp(line, ";d ", 3) || !strncmp(line, ";v ", 3))
                {
                    ZeroMemory(text, (MAX_LINE_LENGTH + 3) * sizeof(wchar_t));
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
                        HKEY hKey = NULL;
                        DWORD dwDisposition;
                        DWORD dwSize = sizeof(DWORD);
                        DWORD value = FALSE;

                        //wprintf(L"%s %s %s\n", section, name, d + 1);
                        if (!wcsncmp(d + 1, L"dword:", 6))
                        {
                            wchar_t* x = wcschr(d + 1, L':');
                            x++;
                            value = wcstol(x, NULL, 16);
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
                            if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
                            {
                                hKey = NULL;
                            }
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
                            if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
                            {
                                hKey = NULL;
                            }
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
                            MENUITEMINFOW menuInfo;
                            ZeroMemory(&menuInfo, sizeof(MENUITEMINFOW));
                            menuInfo.cbSize = sizeof(MENUITEMINFOW);
                            menuInfo.fMask = MIIM_STRING;
                            GetMenuItemInfoW(hMenu, value + 1, FALSE, &menuInfo);
                            menuInfo.cch += 1;
                            menuInfo.dwTypeData = text + wcslen(text);
                            GetMenuItemInfoW(hMenu, value + 1, FALSE, &menuInfo);
                            ZeroMemory(&menuInfo, sizeof(MENUITEMINFOW));
                            menuInfo.cbSize = sizeof(MENUITEMINFOW);
                            menuInfo.fMask = MIIM_STATE;
                            menuInfo.fState = MFS_CHECKED;
                            SetMenuItemInfo(hMenu, value + 1, FALSE, &menuInfo);
                        }
                        if (hDC && !bInvert && !bBool && !bJustCheck)
                        {
                            RECT rcTemp;
                            rcTemp = rcText;
                            DrawTextW(
                                hdcPaint,
                                text,
                                3,
                                &rcTemp,
                                DT_CALCRECT
                            );
                            rcText.left += rcTemp.right - rcTemp.left;
                            for (unsigned int i = 0; i < wcslen(text); ++i)
                            {
                                text[i] = text[i + 3];
                            }
                        }
                        RECT rcTemp;
                        rcTemp = rcText;
                        DrawTextW(
                            hdcPaint,
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
                                    hKey = NULL;
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
                                    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
                                    {
                                        hKey = NULL;
                                    }
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
                                    RECT rcTemp;
                                    rcTemp = rcText;
                                    DrawTextW(
                                        hdcPaint,
                                        text,
                                        3,
                                        &rcTemp,
                                        DT_CALCRECT
                                    );
                                    POINT p;
                                    p.x = rcText.left + rcTemp.right - rcTemp.left;
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
                        if (hKey)
                        {
                            RegCloseKey(hKey);
                        }
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
                        DrawTextW(
                            hdcPaint,
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
                            if (line[1] != 0)
                            {
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
                    }
                    if (hDC)
                    {
                        COLORREF cr;
                        if (tabOrder == _this->tabOrder)
                        {
                            bTabOrderHit = TRUE;
                            if (!IsThemeActive())
                            {
                                cr = SetTextColor(hdcPaint, GetSysColor(COLOR_HIGHLIGHT));
                            }
                            else
                            {
                                DttOpts.crText = g_darkModeEnabled ? GUI_TEXTCOLOR_SELECTED_DARK : GUI_TEXTCOLOR_SELECTED;
                                //DttOpts.crText = GetSysColor(COLOR_HIGHLIGHT);
                            }
                        }
                        RECT rcNew = rcText;
                        DrawTextW(
                            hdcPaint,
                            text,
                            -1,
                            &rcNew,
                            DT_CALCRECT
                        );
                        if (rcNew.right - rcNew.left > dwMaxWidth)
                        {
                            dwMaxWidth = rcNew.right - rcNew.left + 50 * dx;
                        }
                        if (IsThemeActive())
                        {
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
                        }
                        else
                        {
                            DrawTextW(
                                hdcPaint,
                                text,
                                -1,
                                &rcText,
                                dwTextFlags
                            );
                        }
                        if (tabOrder == _this->tabOrder)
                        {
                            if (!IsThemeActive())
                            {
                                SetTextColor(hdcPaint, cr);
                            }
                            else
                            {
                                DttOpts.crText = g_darkModeEnabled ? GUI_TEXTCOLOR_DARK : GUI_TEXTCOLOR;
                                //DttOpts.crText = GetSysColor(COLOR_WINDOWTEXT);
                            }
                        }
                    }
                    dwMaxHeight += dwLineHeight * dy;
                    tabOrder++;
                }
            }
        }
        fclose(f);
        free(section);
        free(name);
        free(text);
        free(line);

        SelectObject(hdcPaint, hOldFont);
        if (!hDC)
        {
            ReleaseDC(hwnd, hdcPaint);
        }
        DeleteObject(hFontSectionSel);
        DeleteObject(hFontSection);
        DeleteObject(hFontRegular);
        DeleteObject(hFontTitle);
        DeleteObject(hFontUnderline);
        DeleteObject(hFontCaption);
        if (hDC)
        {
            if (_this->tabOrder == GUI_MAX_TABORDER)
            {
                _this->tabOrder = tabOrder;
            }
            else if (!bTabOrderHit)
            {
                _this->tabOrder = 0;
            }
        }
    }
    if (_this->bCalcExtent)
    {
        RECT rcWin;
        GetWindowRect(hwnd, &rcWin);
        printf("%d %d - %d %d\n", rcWin.right - rcWin.left, rcWin.bottom - rcWin.top, dwMaxWidth, dwMaxHeight);

        dwMaxWidth += dwInitialLeftPad + _this->padding.left + _this->padding.right;
        dwMaxHeight += GUI_LINE_HEIGHT * dy + 20 * dy;

        HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi;
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hMonitor, &mi);
        SetWindowPos(
            hwnd,
            hwnd,
            mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left) / 2 - (dwMaxWidth) / 2),
            mi.rcWork.top + ((mi.rcWork.bottom - mi.rcWork.top) / 2 - (dwMaxHeight) / 2),
            dwMaxWidth,
            dwMaxHeight,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
        _this->bCalcExtent = FALSE;
        InvalidateRect(hwnd, NULL, FALSE);

    }

    EndBufferedPaint(hBufferedPaint, TRUE);
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
        if (IsThemeActive() && ShouldAppsUseDarkMode)
        {
            AllowDarkModeForWindow(hWnd, g_darkModeEnabled);
            BOOL value = g_darkModeEnabled;
            DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(BOOL));
        }
        if (!IsThemeActive())
        {
            int extendedStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
            SetWindowLong(hWnd, GWL_EXSTYLE, extendedStyle | WS_EX_DLGMODALFRAME);
        }
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
    else if (uMsg == WM_GETICON)
    {
        return _this->hIcon;
    }
    else if (uMsg == WM_SETTINGCHANGE)
    {
        if (IsColorSchemeChangeMessage(lParam))
        {
            if (IsThemeActive() && ShouldAppsUseDarkMode)
            {
                BOOL bIsCompositionEnabled = TRUE;
                DwmIsCompositionEnabled(&bIsCompositionEnabled);
                BOOL bDarkModeEnabled = IsThemeActive() && bIsCompositionEnabled && ShouldAppsUseDarkMode() && !IsHighContrast();
                if (bDarkModeEnabled != g_darkModeEnabled)
                {
                    g_darkModeEnabled = bDarkModeEnabled;
                    AllowDarkModeForWindow(hWnd, g_darkModeEnabled);
                    BOOL value = g_darkModeEnabled;
                    DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(BOOL));
                    InvalidateRect(hWnd, NULL, FALSE);
                }
            }
            else
            {
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
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
        // this should be determined from the file, but for now it works
        else if (wParam >= 0x30 + 1 && wParam <= 0x30 + 7) 
        {
            _this->tabOrder = 0;
            _this->section = wParam - 0x30 - 1;
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;
        }
    }
    else if (uMsg == WM_NCHITTEST && IsThemeActive())
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
        if (IsThemeActive())
        {
            BOOL bIsCompositionEnabled = TRUE;
            DwmIsCompositionEnabled(&bIsCompositionEnabled);
            if (bIsCompositionEnabled)
            {
                MARGINS marGlassInset = { -1, -1, -1, -1 }; // -1 means the whole window
                DwmExtendFrameIntoClientArea(hWnd, &marGlassInset);
            }
        }
    
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
    HKEY hKey = NULL;
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
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = NULL;
    }
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
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    wchar_t wszPath[MAX_PATH];
    ZeroMemory(
        wszPath,
        (MAX_PATH) * sizeof(char)
    );
    GetModuleFileNameW(hModule, wszPath, MAX_PATH);
    PathRemoveFileSpecW(wszPath);
    wcscat_s(
        wszPath,
        MAX_PATH,
        L"\\settings.reg"
    );
    wprintf(L"%s\n", wszPath);
    if (FileExistsW(wszPath))
    {
        HANDLE hFile = CreateFileW(
            wszPath,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            0
        );
        if (hFile)
        {
            HANDLE hFileMapping = CreateFileMappingW(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
            if (hFileMapping)
            {
                GUI_FileMapping = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
                GUI_FileSize = GetFileSize(hFile, NULL);
            }
        }
    }

    printf("Started \"GUI\" thread.\n");

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    GUI _this;
    _this.hBackgroundBrush = (HBRUSH)(CreateSolidBrush(RGB(255, 255, 255)));// (HBRUSH)GetStockObject(BLACK_BRUSH);
    _this.location.x = GUI_POSITION_X;
    _this.location.y = GUI_POSITION_Y;
    _this.size.cx = GUI_POSITION_WIDTH;
    _this.size.cy = GUI_POSITION_HEIGHT;
    _this.padding.left = GUI_PADDING_LEFT;
    _this.padding.right = GUI_PADDING_RIGHT;
    _this.padding.top = GUI_PADDING_TOP;
    _this.padding.bottom = GUI_PADDING_BOTTOM;
    _this.sidebarWidth = GUI_SIDEBAR_WIDTH;
    _this.hTheme = OpenThemeData(NULL, TEXT(GUI_WINDOWSWITCHER_THEME_CLASS));
    _this.tabOrder = 0;
    _this.bCalcExtent = TRUE;
    _this.section = 0;
    _this.dwStatusbarY = 0;
    _this.hIcon = NULL;

    ZeroMemory(
        wszPath,
        (MAX_PATH) * sizeof(wchar_t)
    );
    GetWindowsDirectoryW(
        wszPath,
        MAX_PATH
    );
    wcscat_s(
        wszPath,
        MAX_PATH,
        L"\\explorer.exe"
    );

    WNDCLASS wc = { 0 };
    ZeroMemory(&wc, sizeof(WNDCLASSW));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = GUI_WindowProc;
    wc.hbrBackground = _this.hBackgroundBrush;
    wc.hInstance = hModule;
    wc.lpszClassName = L"ExplorerPatcherGUI";
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    HMODULE hExplorer = LoadLibraryExW(wszPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hExplorer)
    {
        _this.hIcon = LoadIconW(hExplorer, L"ICO_MYCOMPUTER");
        wc.hIcon = _this.hIcon;
    }
    RegisterClassW(&wc);

    TCHAR title[260];
    HMODULE hExplorerFrame = LoadLibraryExW(L"ExplorerFrame.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
    LoadStringW(hExplorerFrame, 726, title, 260);
    FreeLibrary(hExplorerFrame);
    if (title[0] == 0)
    {
        LoadStringW(hModule, IDS_PRODUCTNAME, title, 260);
    }
    HANDLE hUxtheme = NULL;
    BOOL bHasLoadedUxtheme = FALSE;
    BOOL bIsCompositionEnabled = TRUE;
    DwmIsCompositionEnabled(&bIsCompositionEnabled);
    if (IsThemeActive() && bIsCompositionEnabled)
    {
        bHasLoadedUxtheme = TRUE;
        hUxtheme = LoadLibraryW(L"uxtheme.dll");
        if (hUxtheme)
        {
            RefreshImmersiveColorPolicyState = GetProcAddress(hUxtheme, (LPCSTR)104);
            SetPreferredAppMode = GetProcAddress(hUxtheme, (LPCSTR)135);
            AllowDarkModeForWindow = GetProcAddress(hUxtheme, (LPCSTR)133);
            ShouldAppsUseDarkMode = GetProcAddress(hUxtheme, (LPCSTR)132);
            if (ShouldAppsUseDarkMode &&
                SetPreferredAppMode &&
                AllowDarkModeForWindow &&
                RefreshImmersiveColorPolicyState
                )
            {
                SetPreferredAppMode(TRUE);
                RefreshImmersiveColorPolicyState();
                g_darkModeEnabled = IsThemeActive() && bIsCompositionEnabled && ShouldAppsUseDarkMode() && !IsHighContrast();
            }
        }
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
    if (IsThemeActive())
    {
        if (bIsCompositionEnabled)
        {
            BOOL value = 1;
            DwmSetWindowAttribute(hwnd, DWMWA_MICA_EFFFECT, &value, sizeof(BOOL));
            WTA_OPTIONS ops;
            ops.dwFlags = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
            ops.dwMask = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
            SetWindowThemeAttribute(
                hwnd,
                WTA_NONCLIENT,
                &ops,
                sizeof(WTA_OPTIONS)
            );
        }
    }
    ShowWindow(hwnd, SW_SHOW);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hExplorer)
    {
        CloseHandle(_this.hIcon);
        FreeLibrary(hExplorer);
    }

    if (bHasLoadedUxtheme && hUxtheme)
    {
        FreeLibrary(hUxtheme);
    }

    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
    _CrtDumpMemoryLeaks();
#ifdef _DEBUG
    _getch();
#endif

    printf("Ended \"GUI\" thread.\n");
}

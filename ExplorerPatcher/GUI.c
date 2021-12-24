#include "GUI.h"

LANGID locale;
void* GUI_FileMapping = NULL;
DWORD GUI_FileSize = 0;
BOOL g_darkModeEnabled = FALSE;
static void(*RefreshImmersiveColorPolicyState)() = NULL;
static BOOL(*ShouldAppsUseDarkMode)() = NULL;
DWORD dwTaskbarPosition = 3;
BOOL gui_bOldTaskbar = TRUE;

NTSTATUS NTAPI hookRtlQueryElevationFlags(DWORD* pFlags)
{
    *pFlags = 0;
    return 0;
}

PVOID pvRtlQueryElevationFlags;

LONG NTAPI OnVex(PEXCEPTION_POINTERS ExceptionInfo)
{
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP &&
        ExceptionInfo->ExceptionRecord->ExceptionAddress == pvRtlQueryElevationFlags)
    {
        ExceptionInfo->ContextRecord->
#if defined(_X86_)
            Eip
#elif defined (_AMD64_)
            Rip
#else
#error not implemented
#endif
            = (ULONG_PTR)hookRtlQueryElevationFlags;

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

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
        is = TRUE;
    }
    return is;
}

LSTATUS GUI_RegSetValueExW(
    HKEY       hKey,
    LPCWSTR    lpValueName,
    DWORD      Reserved,
    DWORD      dwType,
    const BYTE* lpData,
    DWORD      cbData
)
{
    if (!lpValueName || wcsncmp(lpValueName, L"Virtualized_" _T(EP_CLSID), 50))
    {
        return RegSetValueExW(hKey, lpValueName, 0, dwType, lpData, cbData);
    }
    if (!wcscmp(lpValueName, L"Virtualized_" _T(EP_CLSID) L"_TaskbarPosition"))
    {
        StuckRectsData srd;
        DWORD pcbData = sizeof(StuckRectsData);
        RegGetValueW(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StuckRectsLegacy",
            L"Settings",
            REG_BINARY,
            NULL,
            &srd,
            &pcbData);
        if (pcbData == sizeof(StuckRectsData) && srd.pvData[0] == sizeof(StuckRectsData) && srd.pvData[1] == -2)
        {
            srd.pvData[3] = *(DWORD*)lpData;
            dwTaskbarPosition = *(DWORD*)lpData;
            RegSetKeyValueW(
                HKEY_CURRENT_USER,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StuckRectsLegacy",
                L"Settings",
                REG_BINARY,
                &srd,
                sizeof(StuckRectsData)
            );
        }
        pcbData = sizeof(StuckRectsData);
        RegGetValueW(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StuckRects3",
            L"Settings",
            REG_BINARY,
            NULL,
            &srd,
            &pcbData);
        if (pcbData == sizeof(StuckRectsData) && srd.pvData[0] == sizeof(StuckRectsData) && srd.pvData[1] == -2)
        {
            srd.pvData[3] = *(DWORD*)lpData;
            if (srd.pvData[3] != 1 && srd.pvData[3] != 3) // Disallow left/right settings for Windows 11 taskbar, as this breaks it
            {
                srd.pvData[3] = 3;
            }
            RegSetKeyValueW(
                HKEY_CURRENT_USER,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StuckRects3",
                L"Settings",
                REG_BINARY,
                &srd,
                sizeof(StuckRectsData)
            );
        }
        return ERROR_SUCCESS;
    }
    else if (!wcscmp(lpValueName, L"Virtualized_" _T(EP_CLSID) L"_MMTaskbarPosition"))
    {
        HKEY hKey = NULL;
        RegOpenKeyExW(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MMStuckRectsLegacy",
            REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE,
            &hKey
        );
        if (hKey)
        {
            DWORD cValues = 0;
            RegQueryInfoKeyW(
                hKey,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                &cValues,
                NULL,
                NULL,
                NULL,
                NULL
            );
            WCHAR name[60];
            DWORD szName = 60;
            StuckRectsData srd;
            DWORD pcbData = sizeof(StuckRectsData);
            for (int i = 0; i < cValues; ++i)
            {
                RegEnumValueW(
                    hKey,
                    i,
                    name,
                    &szName,
                    0,
                    NULL,
                    &srd,
                    &pcbData
                );
                szName = 60;
                srd.pvData[3] = *(DWORD*)lpData;
                pcbData = sizeof(StuckRectsData);
                RegSetKeyValueW(
                    HKEY_CURRENT_USER,
                    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MMStuckRectsLegacy",
                    name,
                    REG_BINARY,
                    &srd,
                    sizeof(StuckRectsData)
                );
            }
            RegCloseKey(hKey);
            SendNotifyMessageW(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)L"TraySettings");
        }
        RegOpenKeyExW(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MMStuckRects3",
            REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE,
            &hKey
        );
        if (hKey)
        {
            DWORD cValues = 0;
            RegQueryInfoKeyW(
                hKey,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                &cValues,
                NULL,
                NULL,
                NULL,
                NULL
            );
            WCHAR name[60];
            DWORD szName = 60;
            StuckRectsData srd;
            DWORD pcbData = sizeof(StuckRectsData);
            for (int i = 0; i < cValues; ++i)
            {
                RegEnumValueW(
                    hKey,
                    i,
                    name,
                    &szName,
                    0,
                    NULL,
                    &srd,
                    &pcbData
                );
                szName = 60;
                srd.pvData[3] = *(DWORD*)lpData;
                if (srd.pvData[3] != 1 && srd.pvData[3] != 3) // Disallow left/right settings for Windows 11 taskbar, as this breaks it
                {
                    srd.pvData[3] = 3;
                }
                pcbData = sizeof(StuckRectsData);
                RegSetKeyValueW(
                    HKEY_CURRENT_USER,
                    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MMStuckRects3",
                    name,
                    REG_BINARY,
                    &srd,
                    sizeof(StuckRectsData)
                );
            }
            RegCloseKey(hKey);
        }
        return ERROR_SUCCESS;
    }
    else if (!wcscmp(lpValueName, L"Virtualized_" _T(EP_CLSID) L"_AutoHideTaskbar"))
    {
        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        abd.lParam = *(DWORD*)lpData;
        SHAppBarMessage(ABM_SETSTATE, &abd);
        return ERROR_SUCCESS;
    }
    else if (!wcscmp(lpValueName, L"Virtualized_" _T(EP_CLSID) L"_PeopleBand"))
    {
        PostMessageW(FindWindowW(L"Shell_TrayWnd", NULL), WM_COMMAND, 435, 0);
        return ERROR_SUCCESS;
    }
    else if (!wcscmp(lpValueName, L"Virtualized_" _T(EP_CLSID) L"_Start_MaximumFrequentApps"))
    {
        RegSetKeyValueW(
            HKEY_CURRENT_USER,
            TEXT(REGPATH),
            L"Start_MaximumFrequentApps",
            dwType,
            lpData,
            cbData
        );
        return RegSetValueExW(hKey, L"Start_MaximumFrequentApps", 0, dwType, lpData, cbData);
    }
}

LSTATUS GUI_RegQueryValueExW(
    HKEY    hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
)
{
    if (!lpValueName || wcsncmp(lpValueName, L"Virtualized_" _T(EP_CLSID), 50))
    {
        return RegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
    }
    if (!wcscmp(lpValueName, L"Virtualized_" _T(EP_CLSID) L"_TaskbarPosition"))
    {
        StuckRectsData srd;
        DWORD pcbData = sizeof(StuckRectsData);
        RegGetValueW(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StuckRectsLegacy",
            L"Settings",
            REG_BINARY,
            NULL,
            &srd,
            &pcbData);
        if (pcbData == sizeof(StuckRectsData) && srd.pvData[0] == sizeof(StuckRectsData) && srd.pvData[1] == -2)
        {
            dwTaskbarPosition = srd.pvData[3];
            if (!gui_bOldTaskbar)
            {
                if (srd.pvData[3] != 1 && srd.pvData[3] != 3) // Disallow left/right settings for Windows 11 taskbar, as this breaks it
                {
                    srd.pvData[3] = 3;
                }
            }
            *(DWORD*)lpData = srd.pvData[3];
            return ERROR_SUCCESS;
        }
        return ERROR_ACCESS_DENIED;
    }
    else if (!wcscmp(lpValueName, L"Virtualized_" _T(EP_CLSID) L"_MMTaskbarPosition"))
    {
        HKEY hKey = NULL;
        RegOpenKeyExW(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MMStuckRectsLegacy",
            REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE,
            &hKey
        );
        if (hKey)
        {
            WCHAR name[60];
            DWORD szName = 60;
            StuckRectsData srd;
            DWORD pcbData = sizeof(StuckRectsData);
            RegEnumValueW(
                hKey,
                0,
                name,
                &szName,
                0,
                NULL,
                &srd,
                &pcbData
            );
            if (pcbData == sizeof(StuckRectsData) && srd.pvData[0] == sizeof(StuckRectsData) && srd.pvData[1] == -2)
            {
                if (!gui_bOldTaskbar)
                {
                    if (srd.pvData[3] != 1 && srd.pvData[3] != 3) // Disallow left/right settings for Windows 11 taskbar, as this breaks it
                    {
                        srd.pvData[3] = 3;
                    }
                }
                *(DWORD*)lpData = srd.pvData[3];
                RegCloseKey(hKey);
                return ERROR_SUCCESS;
            }
            RegCloseKey(hKey);
        }
        return ERROR_ACCESS_DENIED;
    }
    else if (!wcscmp(lpValueName, L"Virtualized_" _T(EP_CLSID) L"_AutoHideTaskbar"))
    {
        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        *(DWORD*)lpData = (SHAppBarMessage(ABM_GETSTATE, &abd) == ABS_AUTOHIDE);
        return ERROR_SUCCESS;
    }
    else if (!wcscmp(lpValueName, L"Virtualized_" _T(EP_CLSID) L"_PeopleBand"))
    {
        return RegQueryValueExW(hKey, L"PeopleBand", lpReserved, lpType, lpData, lpcbData);
    }
    else if (!wcscmp(lpValueName, L"Virtualized_" _T(EP_CLSID) L"_Start_MaximumFrequentApps"))
    {
        return RegQueryValueExW(hKey, L"Start_MaximumFrequentApps", lpReserved, lpType, lpData, lpcbData);
    }
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

static void GUI_SetSection(GUI* _this, BOOL bCheckEnablement, int dwSection)
{
    _this->section = dwSection;

    HKEY hKey = NULL;
    DWORD dwSize = sizeof(DWORD);
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY | KEY_WRITE,
        NULL,
        &hKey,
        NULL
    );
    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
    {
        return;
    }

    BOOL bEnabled = FALSE;
    if (bCheckEnablement)
    {
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            TEXT("LastSectionInProperties"),
            0,
            NULL,
            &bEnabled,
            &dwSize
        );
        dwSection++;
    }
    else
    {
        bEnabled = TRUE;
    }

    if (bEnabled)
    {
        RegSetValueExW(
            hKey,
            TEXT("LastSectionInProperties"),
            0,
            REG_DWORD,
            &dwSection,
            sizeof(DWORD)
        );
    }

    RegCloseKey(hKey);
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
    //logFont.lfWeight = FW_BOLD;
    HFONT hFontCaption = CreateFontIndirect(&logFont);
    logFont = ncm.lfMenuFont;
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
            COLORREF oldcr = SetBkColor(hdcPaint, GetSysColor(COLOR_MENU));
            ExtTextOutW(hdcPaint, 0, 0, ETO_OPAQUE, &rc, L"", 0, 0);
            SetBkColor(hdcPaint, oldcr);
            SetTextColor(hdcPaint, GetSysColor(COLOR_WINDOWTEXT));
            SetBkMode(hdcPaint, TRANSPARENT);
        }

        BOOL bWasSpecifiedSectionValid = FALSE;
        FILE* f = fmemopen(pRscr, cbRscr, "r");
        char* line = malloc(MAX_LINE_LENGTH * sizeof(char));
        wchar_t* text = malloc((MAX_LINE_LENGTH + 3) * sizeof(wchar_t)); 
        wchar_t* name = malloc(MAX_LINE_LENGTH * sizeof(wchar_t));
        wchar_t* section = malloc(MAX_LINE_LENGTH * sizeof(wchar_t));
        size_t bufsiz = MAX_LINE_LENGTH, numChRd = 0, tabOrder = 1, currentSection = -1, topAdj = 0;
        while ((numChRd = getline(&line, &bufsiz, f)) != -1)
        {
            if (currentSection == _this->section)
            {
                bWasSpecifiedSectionValid = TRUE;
            }
            if (strcmp(line, "Windows Registry Editor Version 5.00\r\n") && 
                strcmp(line, "\r\n") && 
                (currentSection == -1 || currentSection == _this->section || !strncmp(line, ";T ", 3) || !strncmp(line, ";f", 2)) &&
                !(!IsThemeActive() && !strncmp(line, ";M ", 3))
                )
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
                        //dwMaxHeight += GUI_STATUS_PADDING * dy;
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

                DWORD dwLineHeight = !strncmp(line, ";M ", 3) ? _this->GUI_CAPTION_LINE_HEIGHT : GUI_LINE_HEIGHT;
                DWORD dwBottom = _this->padding.bottom;
                DWORD dwTop = _this->padding.top;
                if (!strncmp(line, ";a ", 3) || !strncmp(line, ";e ", 3))
                {
                    dwBottom = 0;
                    dwLineHeight -= 0.2 * dwLineHeight;
                }

                rcText.left = dwLeftPad + _this->padding.left;
                rcText.top = !strncmp(line, ";M ", 3) ? 0 : (dwTop + dwMaxHeight);
                rcText.right = (rc.right - rc.left) - _this->padding.right;
                rcText.bottom = !strncmp(line, ";M ", 3) ? _this->GUI_CAPTION_LINE_HEIGHT * dy : (dwMaxHeight + dwLineHeight * dy - dwBottom);

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
                            GUI_SetSection(_this, TRUE, currentSection + 1);
                            InvalidateRect(hwnd, NULL, FALSE);
                        }
                    }
                    currentSection++;
                    continue;
                }
                else if (!strncmp(line, ";M ", 3))
                {
                    UINT diff = (((_this->GUI_CAPTION_LINE_HEIGHT - 16) * dx) / 2.0);
                    rcText.left = diff + (int)(16.0 * dx) + diff / 2;
                    topAdj = dwMaxHeight + _this->GUI_CAPTION_LINE_HEIGHT * dy;
                    hOldFont = SelectObject(hdcPaint, hFontCaption);
                }
                else if (!strncmp(line, ";u ", 3) || (!strncmp(line, ";y ", 3) && !strstr(line, "\xF0\x9F")))
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
                        char* p = strstr(line, "%VERSIONINFORMATIONSTRING%");
                        if (p)
                        {
                            DWORD dwLeftMost = 0;
                            DWORD dwSecondLeft = 0;
                            DWORD dwSecondRight = 0;
                            DWORD dwRightMost = 0;

                            QueryVersionInfo(hModule, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

                            sprintf_s(p, MAX_PATH, "%d.%d.%d.%d%s", dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost, 
#if defined(DEBUG) | defined(_DEBUG)
                                " (Debug)"
#else
                                ""
#endif
                                );
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
                        if (hDC)
                        {
                            UINT diff = (int)(((_this->GUI_CAPTION_LINE_HEIGHT - 16) * dx) / 2.0);
                            //printf("!!! %d %d\n", (int)(16.0 * dx), diff);
                            DrawIconEx(
                                hdcPaint,
                                diff,
                                diff,
                                _this->hIcon,
                                (int)(16.0 * dx),
                                (int)(16.0 * dy),
                                0,
                                NULL,
                                DI_NORMAL
                            );
                        }

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
                        //rcText.bottom += _this->GUI_CAPTION_LINE_HEIGHT - dwLineHeight;
                        dwLineHeight = _this->GUI_CAPTION_LINE_HEIGHT;
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
                                cr = SetTextColor(hdcPaint, GetSysColor(COLOR_HIGHLIGHTTEXT));
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
                                HWND hShellTrayWnd = FindWindowW(L"Shell_TrayWnd", NULL);
                                if (hShellTrayWnd)
                                {
                                    WCHAR wszPath[MAX_PATH];
                                    ZeroMemory(wszPath, MAX_PATH * sizeof(WCHAR));
                                    PDWORD_PTR res = -1;
                                    if (!SendMessageTimeoutW(hShellTrayWnd, 1460, 0, 0, SMTO_ABORTIFHUNG, 2000, &res) && res)
                                    {
                                        HANDLE hExplorerRestartThread = CreateThread(NULL, 0, BeginExplorerRestart, NULL, 0, NULL);
                                        if (hExplorerRestartThread)
                                        {
                                            WaitForSingleObject(hExplorerRestartThread, 2000);
                                            CloseHandle(hExplorerRestartThread);
                                            hExplorerRestartThread = NULL;
                                        }
                                        else
                                        {
                                            BeginExplorerRestart();
                                        }
                                    }
                                    Sleep(100);
                                    GetSystemDirectoryW(wszPath, MAX_PATH);
                                    wcscat_s(wszPath, MAX_PATH, L"\\taskkill.exe");
                                    SHELLEXECUTEINFOW sei;
                                    ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
                                    sei.cbSize = sizeof(sei);
                                    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
                                    sei.hwnd = NULL;
                                    sei.hInstApp = NULL;
                                    sei.lpVerb = NULL;
                                    sei.lpFile = wszPath;
                                    sei.lpParameters = L"/f /im explorer.exe";
                                    sei.hwnd = NULL;
                                    sei.nShow = SW_SHOWMINIMIZED;
                                    if (ShellExecuteExW(&sei) && sei.hProcess)
                                    {
                                        WaitForSingleObject(sei.hProcess, INFINITE);
                                        CloseHandle(sei.hProcess);
                                    }
                                    GetWindowsDirectoryW(wszPath, MAX_PATH);
                                    wcscat_s(wszPath, MAX_PATH, L"\\explorer.exe");
                                    Sleep(1000);
                                    GUI_RegSetValueExW(NULL, L"Virtualized_" _T(EP_CLSID) L"_TaskbarPosition", NULL, NULL, &dwTaskbarPosition, NULL);
                                    ShellExecuteW(
                                        NULL,
                                        L"open",
                                        wszPath,
                                        NULL,
                                        NULL,
                                        SW_SHOWNORMAL
                                    );
                                }
                                else
                                {
                                    StartExplorer();
                                }
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
                                    SPECIAL_FOLDER_LEGACY,
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
                                    void* buffer = NULL;
                                    HKEY hKey = NULL;
                                    RegOpenKeyExW(
                                        HKEY_LOCAL_MACHINE,
                                        L"Software\\Classes\\CLSID\\" _T(EP_CLSID) L"\\InprocServer32",
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ | KEY_WOW64_64KEY,
                                        &hKey
                                    );
                                    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
                                    {
                                        buffer = malloc(cbRscr);
                                        if (buffer)
                                        {
                                            memcpy(buffer, pRscr, cbRscr);
                                            char* p1 = strstr(buffer, "[-HKEY_LOCAL_MACHINE\\Software\\Classes\\CLSID\\" EP_CLSID "\\InprocServer32]");
                                            if (p1) p1[0] = ';';
                                            char* p2 = strstr(buffer, ";d Register as shell extension");
                                            if (p2) memcpy(p2, ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;", 70);
                                        }
                                        else
                                        {
                                            RegCloseKey(hKey);
                                            hKey = NULL;
                                        }
                                    }
                                    if (!buffer)
                                    {
                                        buffer = pRscr;
                                    }
                                    DWORD dwNumberOfBytesWritten = 0;
                                    if (WriteFile(
                                        hFile,
                                        buffer,
                                        cbRscr,
                                        &dwNumberOfBytesWritten,
                                        NULL
                                    ))
                                    {
                                        CloseHandle(hFile);

                                        DWORD dwError = 1;
                                        if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
                                        {
                                            dwError = 0;
                                            // https://stackoverflow.com/questions/50298722/win32-launching-a-highestavailable-child-process-as-a-normal-user-process
                                            if (pvRtlQueryElevationFlags = GetProcAddress(GetModuleHandleW(L"ntdll"), "RtlQueryElevationFlags"))
                                            {
                                                PVOID pv;
                                                if (pv = AddVectoredExceptionHandler(TRUE, OnVex))
                                                {
                                                    CONTEXT ctx;
                                                    ZeroMemory(&ctx, sizeof(CONTEXT));
                                                    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
                                                    ctx.Dr7 = 0x404;
                                                    ctx.Dr1 = (ULONG_PTR)pvRtlQueryElevationFlags;

                                                    if (SetThreadContext(GetCurrentThread(), &ctx))
                                                    {
                                                        WCHAR wszExec[MAX_PATH * 2];
                                                        ZeroMemory(wszExec, MAX_PATH * 2 * sizeof(WCHAR));
                                                        wszExec[0] = L'"';
                                                        GetWindowsDirectoryW(wszExec + 1, MAX_PATH);
                                                        wcscat_s(wszExec, MAX_PATH * 2, L"\\regedit.exe\" \"");
                                                        wcscat_s(wszExec, MAX_PATH * 2, wszPath);
                                                        wcscat_s(wszExec, MAX_PATH * 2, L"\"");
                                                        STARTUPINFO si;
                                                        ZeroMemory(&si, sizeof(STARTUPINFO));
                                                        si.cb = sizeof(STARTUPINFO);
                                                        PROCESS_INFORMATION pi;
                                                        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
                                                        wprintf(L"%s\n", wszExec);
                                                        if (CreateProcessW(NULL, wszExec, 0, 0, 0, 0, 0, 0, &si, &pi))
                                                        {
                                                            CloseHandle(pi.hThread);
                                                            //CloseHandle(pi.hProcess);
                                                        }
                                                        else
                                                        {
                                                            dwError = GetLastError();
                                                        }

                                                        ctx.Dr7 = 0x400;
                                                        ctx.Dr1 = 0;
                                                        SetThreadContext(GetCurrentThread(), &ctx);

                                                        if (pi.hProcess)
                                                        {
                                                            WaitForSingleObject(pi.hProcess, INFINITE);
                                                            DWORD dwExitCode = 0;
                                                            GetExitCodeProcess(pi.hProcess, &dwExitCode);
                                                            CloseHandle(pi.hProcess);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        dwError = GetLastError();
                                                    }
                                                    RemoveVectoredExceptionHandler(pv);
                                                }
                                                else
                                                {
                                                    dwError = GetLastError();
                                                }
                                            }
                                            else
                                            {
                                                dwError = GetLastError();
                                            }
                                        }
                                        if (dwError)
                                        {
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
                                            ShellExecuteExW(&ShExecInfo);
                                            WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
                                            DWORD dwExitCode = 0;
                                            GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
                                            CloseHandle(ShExecInfo.hProcess);
                                        }
                                        _this->tabOrder = 0;
                                        InvalidateRect(hwnd, NULL, FALSE);
                                        DeleteFileW(wszPath);
                                    }
                                    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
                                    {
                                        RegCloseKey(hKey);
                                        free(buffer);
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
                else if (!strncmp(line, ";l ", 3) || !strncmp(line, ";y ", 3) || !strncmp(line, ";c ", 3) || !strncmp(line, ";z ", 3) || !strncmp(line, ";b ", 3) || !strncmp(line, ";i ", 3) || !strncmp(line, ";d ", 3) || !strncmp(line, ";v ", 3))
                {
                    ZeroMemory(text, (MAX_LINE_LENGTH + 3) * sizeof(wchar_t));
                    text[0] = L'\u2795';
                    text[1] = L' ';
                    text[2] = L' ';
                    MultiByteToWideChar(
                        CP_UTF8,
                        MB_PRECOMPOSED,
                        !strncmp(line, ";c ", 3) || !strncmp(line, ";z ", 3) ? strchr(line + 3, ' ') + 1 : line + 3,
                        numChRd - 3,
                        text + 3,
                        MAX_LINE_LENGTH
                    );

                    wchar_t* x = wcschr(text, L'\n');
                    if (x) *x = 0;
                    x = wcschr(text, L'\r');
                    if (x) *x = 0;
                    if (!strncmp(line, ";c ", 3) || !strncmp(line, ";z ", 3) || !strncmp(line, ";b ", 3) || !strncmp(line, ";i ", 3) || !strncmp(line, ";d ", 3) || !strncmp(line, ";v ", 3))
                    {
                        HMENU hMenu = NULL;
                        BOOL bChoice = !strncmp(line, ";c ", 3);
                        BOOL bChoiceLefted = !strncmp(line, ";z ", 3);
                        BOOL bInvert = !strncmp(line, ";i ", 3);
                        BOOL bJustCheck = !strncmp(line, ";d ", 3);
                        BOOL bBool = !strncmp(line, ";b ", 3);
                        BOOL bValue = !strncmp(line, ";v ", 3);
                        DWORD numChoices = 0;
                        if (bChoice || bChoiceLefted)
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
                                    free(l);
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

                                wchar_t* miText = malloc((strlen(ln) + 1) * sizeof(wchar_t));
                                MultiByteToWideChar(
                                    CP_UTF8,
                                    MB_PRECOMPOSED,
                                    ln,
                                    strlen(ln) + 1,
                                    miText,
                                    strlen(ln) + 1
                                );

                                MENUITEMINFOW menuInfo;
                                ZeroMemory(&menuInfo, sizeof(MENUITEMINFOW));
                                menuInfo.cbSize = sizeof(MENUITEMINFOW);
                                menuInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA | MIIM_STATE;
                                menuInfo.wID = atoi(l + 3) + 1;
                                menuInfo.dwItemData = l;
                                menuInfo.fType = MFT_STRING;
                                menuInfo.dwTypeData = miText;
                                menuInfo.cch = strlen(ln);
                                InsertMenuItemW(
                                    hMenu,
                                    i,
                                    TRUE,
                                    &menuInfo
                                );

                                free(miText);
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
                        BOOL bShouldAlterTaskbarDa = FALSE;
                        if (!wcscmp(name, L"TaskbarDa"))
                        {
                            if (!gui_bOldTaskbar)
                            {
                                MENUITEMINFOA menuInfo;
                                ZeroMemory(&menuInfo, sizeof(MENUITEMINFOA));
                                menuInfo.cbSize = sizeof(MENUITEMINFOA);
                                menuInfo.fMask = MIIM_DATA;
                                GetMenuItemInfoA(hMenu, 3, FALSE, &menuInfo);
                                if (menuInfo.dwItemData)
                                {
                                    free(menuInfo.dwItemData);
                                }
                                RemoveMenu(hMenu, 3, MF_BYCOMMAND);
                                bShouldAlterTaskbarDa = TRUE;
                            }
                        }
                        if (!wcscmp(name, L"Virtualized_" _T(EP_CLSID) L"_TaskbarPosition") || !wcscmp(name, L"Virtualized_" _T(EP_CLSID) L"_MMTaskbarPosition"))
                        {
                            if (!gui_bOldTaskbar)
                            {
                                MENUITEMINFOA menuInfo;
                                ZeroMemory(&menuInfo, sizeof(MENUITEMINFOA));
                                menuInfo.cbSize = sizeof(MENUITEMINFOA);
                                menuInfo.fMask = MIIM_DATA;
                                GetMenuItemInfoA(hMenu, 1, FALSE, &menuInfo);
                                if (menuInfo.dwItemData)
                                {
                                    free(menuInfo.dwItemData);
                                }
                                RemoveMenu(hMenu, 1, MF_BYCOMMAND);
                                ZeroMemory(&menuInfo, sizeof(MENUITEMINFOA));
                                menuInfo.cbSize = sizeof(MENUITEMINFOA);
                                menuInfo.fMask = MIIM_DATA;
                                GetMenuItemInfoA(hMenu, 3, FALSE, &menuInfo);
                                if (menuInfo.dwItemData)
                                {
                                    free(menuInfo.dwItemData);
                                }
                                RemoveMenu(hMenu, 3, MF_BYCOMMAND);
                            }
                        }
                        HKEY hKey = NULL;
                        wchar_t* bIsHKLM = wcsstr(section, L"HKEY_LOCAL_MACHINE");
                        bIsHKLM = !bIsHKLM ? NULL : ((bIsHKLM - section) < 3);
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
                                bIsHKLM ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                wcschr(section, L'\\') + 1,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ | (hDC ? 0 : (!bIsHKLM ? KEY_WRITE : 0)),
                                NULL,
                                &hKey,
                                &dwDisposition
                            );
                            if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
                            {
                                hKey = NULL;
                            }
                            GUI_RegQueryValueExW(
                                hKey,
                                name,
                                0,
                                NULL,
                                &value,
                                &dwSize
                            );
                            if (!wcscmp(name, L"OldTaskbar"))
                            {
                                gui_bOldTaskbar = value;
                            }
                            if (hDC && bInvert)
                            {
                                value = !value;
                            }
                        }
                        else
                        {
                            RegOpenKeyExW(
                                bIsHKLM ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                wcschr(section, L'\\') + 1,
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ | (hDC ? 0 : (!bIsHKLM ? KEY_WRITE : 0)),
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
                        else if (bChoice || bChoiceLefted)
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
                            int vvv = value + 1;
                            if (bShouldAlterTaskbarDa && vvv == 3) vvv = 2;
                            GetMenuItemInfoW(hMenu, vvv, FALSE, &menuInfo);
                            menuInfo.cch += 1;
                            menuInfo.dwTypeData = text + wcslen(text);
                            GetMenuItemInfoW(hMenu, vvv, FALSE, &menuInfo);
                            ZeroMemory(&menuInfo, sizeof(MENUITEMINFOW));
                            menuInfo.cbSize = sizeof(MENUITEMINFOW);
                            menuInfo.fMask = MIIM_STATE;
                            menuInfo.fState = MFS_CHECKED;
                            SetMenuItemInfo(hMenu, vvv, FALSE, &menuInfo);
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
                            rcText.left += (!bChoiceLefted ? (rcTemp.right - rcTemp.left) : 0);
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
                                if (bIsHKLM && wcsstr(section, L"Software\\Classes\\CLSID\\" _T(EP_CLSID) L"\\InprocServer32"))
                                {
                                    WCHAR wszArgs[MAX_PATH];
                                    if (!hKey)
                                    {
                                        wszArgs[0] = L'\"';
                                        SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszArgs + 1);
                                        wcscat_s(wszArgs, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\" _T(PRODUCT_NAME) L".amd64.dll\"");
                                    }
                                    else
                                    {
                                        wszArgs[0] = L'/';
                                        wszArgs[1] = L'u';
                                        wszArgs[2] = L' ';
                                        wszArgs[3] = L'"';
                                        SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszArgs + 4);
                                        wcscat_s(wszArgs, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\" _T(PRODUCT_NAME) L".amd64.dll\"");
                                    }
                                    wprintf(L"%s\n", wszArgs);
                                    WCHAR wszApp[MAX_PATH * 2];
                                    GetSystemDirectoryW(wszApp, MAX_PATH * 2);
                                    wcscat_s(wszApp, MAX_PATH * 2, L"\\regsvr32.exe");
                                    wprintf(L"%s\n", wszApp);
                                    SHELLEXECUTEINFOW sei;
                                    ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
                                    sei.cbSize = sizeof(sei);
                                    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
                                    sei.hwnd = NULL;
                                    sei.hInstApp = NULL;
                                    sei.lpVerb = L"runas";
                                    sei.lpFile = wszApp;
                                    sei.lpParameters = wszArgs;
                                    sei.hwnd = NULL;
                                    sei.nShow = SW_NORMAL;
                                    if (ShellExecuteExW(&sei) && sei.hProcess)
                                    {
                                        WaitForSingleObject(sei.hProcess, INFINITE);
                                        DWORD dwExitCode = 0;
                                        if (GetExitCodeProcess(sei.hProcess, &dwExitCode) && !dwExitCode)
                                        {

                                        }
                                        else
                                        {

                                        }
                                        CloseHandle(sei.hProcess);
                                    }
                                    else
                                    {
                                        DWORD dwError = GetLastError();
                                        if (dwError == ERROR_CANCELLED)
                                        {
                                        }
                                    }
                                }
                                else
                                {
                                    if (hKey)
                                    {
                                        RegCloseKey(hKey);
                                        hKey = NULL;
                                        RegDeleteKeyExW(
                                            bIsHKLM ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                            wcschr(section, L'\\') + 1,
                                            REG_OPTION_NON_VOLATILE,
                                            0
                                        );
                                    }
                                    else
                                    {
                                        RegCreateKeyExW(
                                            bIsHKLM ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
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
                                            GUI_RegSetValueExW(
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
                            }
                            else
                            {
                                DWORD val = 0;
                                if (bChoice || bChoiceLefted)
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
                                    p.x = rcText.left + (bChoiceLefted ? 0 : (rcTemp.right - rcTemp.left));
                                    p.y = rcText.bottom;
                                    ClientToScreen(
                                        hwnd,
                                        &p
                                    );
                                    val = TrackPopupMenu(
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
                                if (!wcscmp(name, L"LastSectionInProperties") && wcsstr(section, _T(REGPATH)) && value)
                                {
                                    value = _this->section + 1;
                                }
                                if (!(bChoice || bChoiceLefted) || ((bChoice || bChoiceLefted) && val))
                                {
                                    GUI_RegSetValueExW(
                                        hKey,
                                        name,
                                        0,
                                        REG_DWORD,
                                        &value,
                                        sizeof(DWORD)
                                    );
                                }
                            }
                            InvalidateRect(hwnd, NULL, FALSE);
                        }
                        if (hKey)
                        {
                            RegCloseKey(hKey);
                        }
                        if (bChoice || bChoiceLefted)
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
                    if (hDC && (!strncmp(line, ";l ", 3) || !strncmp(line, ";y ", 3)))
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
                        rcText.left += (!strncmp(line, ";l ", 3) ? (rcTemp.right - rcTemp.left) : 0);
                        for (unsigned int i = 0; i < wcslen(text); ++i)
                        {
                            text[i] = text[i + 3];
                        }
                    }
                    if (!hDC && (!strncmp(line, ";l ", 3) || !strncmp(line, ";y ", 3)))
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
                                if (line[1] == ';')
                                {
                                    if (!strcmp(line + 2, ";EP_CHECK_FOR_UPDATES"))
                                    {
                                        HANDLE hEvent = CreateEventW(NULL, FALSE, FALSE, L"EP_Ev_CheckForUpdates_" _T(EP_CLSID));
                                        if (hEvent)
                                        {
                                            if (GetLastError() != ERROR_ALREADY_EXISTS)
                                            {
                                                CloseHandle(hEvent);
                                            }
                                            else
                                            {
                                                SetEvent(hEvent);
                                                CloseHandle(hEvent);
                                            }
                                        }
                                    }
                                    else if(!strcmp(line + 2, ";EP_INSTALL_UPDATES"))
                                    {
                                        HANDLE hEvent = CreateEventW(NULL, FALSE, FALSE, L"EP_Ev_InstallUpdates_" _T(EP_CLSID));
                                        if (hEvent)
                                        {
                                            if (GetLastError() != ERROR_ALREADY_EXISTS)
                                            {
                                                CloseHandle(hEvent);
                                            }
                                            else
                                            {
                                                SetEvent(hEvent);
                                                CloseHandle(hEvent);
                                            }
                                        }
                                    }
                                }
                                else
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
                        if (!wcsncmp(text + 3, L"%PLACEHOLDER_0001%", 18))
                        {
                            WCHAR key = 0;
                            BYTE kb[256];
                            ZeroMemory(kb, 256);
                            ToUnicode(
                                MapVirtualKeyW(0x29, MAPVK_VSC_TO_VK_EX),
                                0x29,
                                kb,
                                &key,
                                1,
                                0
                            );
                            swprintf(text + 3, MAX_LINE_LENGTH, L"Disable per-application window list ( Alt + %c )", key);
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
        if (!bWasSpecifiedSectionValid)
        {
            GUI_SetSection(_this, FALSE, 0);
            InvalidateRect(hwnd, NULL, FALSE);
        }

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
        if (!IsThemeActive())
        {
            dwMaxHeight += GUI_LINE_HEIGHT * dy + 20 * dy;
        }
        else
        {
            dwMaxHeight += GUI_PADDING * 2 * dy;
        }

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

        DWORD dwReadSection = 0;

        HKEY hKey = NULL;
        DWORD dwSize = sizeof(DWORD);
        RegCreateKeyExW(
            HKEY_CURRENT_USER,
            TEXT(REGPATH),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WOW64_64KEY | KEY_WRITE,
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
            dwReadSection = 0;
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("LastSectionInProperties"),
                0,
                NULL,
                &dwReadSection,
                &dwSize
            );
            if (dwReadSection)
            {
                _this->section = dwReadSection - 1;
            }
            dwReadSection = 0;
            dwSize = sizeof(DWORD);
            RegQueryValueExW(
                hKey,
                TEXT("OpenPropertiesAtNextStart"),
                0,
                NULL,
                &dwReadSection,
                &dwSize
            );
            if (dwReadSection)
            {
                _this->section = dwReadSection - 1;
                dwReadSection = 0;
                RegSetValueExW(
                    hKey,
                    TEXT("OpenPropertiesAtNextStart"),
                    0,
                    REG_DWORD,
                    &dwReadSection,
                    sizeof(DWORD)
                );
            }
            RegCloseKey(hKey);
        }

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
        SetRect(&_this->border_thickness, 2, 2, 2, 2);
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
        SetWindowPos(
            hWnd, 
            hWnd, 
            mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left) / 2 - (_this->size.cx * dx) / 2),
            mi.rcWork.top + ((mi.rcWork.bottom - mi.rcWork.top) / 2 - (_this->size.cy * dy) / 2),
            _this->size.cx * dxp, 
            _this->size.cy * dyp,
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
        );
        if (IsThemeActive())
        {
            RECT rcTitle;
            DwmGetWindowAttribute(hWnd, DWMWA_CAPTION_BUTTON_BOUNDS, &rcTitle, sizeof(RECT));
            _this->GUI_CAPTION_LINE_HEIGHT = rcTitle.bottom - rcTitle.top;
        }
        else
        {
            _this->GUI_CAPTION_LINE_HEIGHT = GUI_CAPTION_LINE_HEIGHT_DEFAULT;
        }
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
                RefreshImmersiveColorPolicyState();
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
        else if (wParam >= '1' && wParam <= '9') 
        {
            _this->tabOrder = 0;
            GUI_SetSection(_this, TRUE, wParam - '1');
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;
        }
    }
    else if (uMsg == WM_NCMOUSELEAVE && IsThemeActive())
    {
        LRESULT lRes = 0;
        if (DwmDefWindowProc(hWnd, uMsg, wParam, lParam, &lRes))
        {
            return lRes;
        }
    }
    else if (uMsg == WM_NCRBUTTONUP && IsThemeActive())
    {
        HMENU pSysMenu = GetSystemMenu(hWnd, FALSE);
        if (pSysMenu != NULL)
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            TrackPopupMenu(pSysMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, xPos, yPos, NULL, hWnd, 0);
        }
        return 0;
    }
    else if ((uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONUP) && IsThemeActive())
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        double dx = _this->dpi.x / 96.0, dy = _this->dpi.y / 96.0;
        UINT diff = (int)(((_this->GUI_CAPTION_LINE_HEIGHT - 16) * dx) / 2.0);
        RECT rc;
        SetRect(&rc, diff, diff, diff + (int)(16.0 * dx), diff + (int)(16.0 * dy));
        if (PtInRect(&rc, pt))
        {
            if (uMsg == WM_LBUTTONUP && _this->LeftClickTime != 0)
            {
                _this->LeftClickTime = milliseconds_now() - _this->LeftClickTime;
            }
            if (uMsg == WM_LBUTTONUP && _this->LeftClickTime != 0 && _this->LeftClickTime < GetDoubleClickTime())
            {
                _this->LeftClickTime = 0;
                PostQuitMessage(0);
            }
            else
            {
                if (uMsg == WM_LBUTTONUP)
                {
                    _this->LeftClickTime = milliseconds_now();
                }
                if (uMsg == WM_RBUTTONUP || !_this->LastClickTime || milliseconds_now() - _this->LastClickTime > 500)
                {
                    HMENU pSysMenu = GetSystemMenu(hWnd, FALSE);
                    if (pSysMenu != NULL)
                    {
                        if (uMsg == WM_LBUTTONUP)
                        {
                            pt.x = 0;
                            pt.y = _this->GUI_CAPTION_LINE_HEIGHT * dy;
                        }
                        ClientToScreen(hWnd, &pt);
                        TrackPopupMenu(pSysMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, NULL, hWnd, 0);
                        if (uMsg == WM_LBUTTONUP)
                        {
                            _this->LastClickTime = milliseconds_now();
                        }
                    }
                }
            }
            return 0;
        }
    }
    else if (uMsg == WM_NCHITTEST && IsThemeActive())
    {
        LRESULT lRes = 0;
        if (DwmDefWindowProc(hWnd, uMsg, wParam, lParam, &lRes))
        {
            return lRes;
        }

        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ScreenToClient(hWnd, &pt);

        double dx = _this->dpi.x / 96.0, dy = _this->dpi.y / 96.0;
        UINT diff = (int)(((_this->GUI_CAPTION_LINE_HEIGHT - 16) * dx) / 2.0);
        RECT rc;
        SetRect(&rc, diff, diff, diff + (int)(16.0 * dx), diff + (int)(16.0 * dy));
        if (PtInRect(&rc, pt))
        {
            return HTCLIENT;
        }

        if (pt.y < _this->extent.cyTopHeight)
        {
            return HTCAPTION;
        }
    }
    else if (uMsg == WM_NCCALCSIZE && wParam == TRUE && IsThemeActive())
    {
        NCCALCSIZE_PARAMS* sz = (NCCALCSIZE_PARAMS*)(lParam);
        sz->rgrc[0].left += _this->border_thickness.left;
        sz->rgrc[0].right -= _this->border_thickness.right;
        sz->rgrc[0].bottom -= _this->border_thickness.bottom;
        return 0;
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
        RECT rcTitle;
        DwmGetWindowAttribute(hWnd, DWMWA_CAPTION_BUTTON_BOUNDS, &rcTitle, sizeof(RECT));
        _this->GUI_CAPTION_LINE_HEIGHT = (rcTitle.bottom - rcTitle.top) * (96.0 / _this->dpi.y);
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
    else if (uMsg == WM_INPUTLANGCHANGE)
    {
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }
    else if (uMsg == WM_MSG_GUI_SECTION && wParam == WM_MSG_GUI_SECTION_GET)
    {
        return _this->section + 1;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

__declspec(dllexport) int ZZGUI(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    HWND hOther = NULL;
    if (hOther = FindWindowW(L"ExplorerPatcher_GUI_" _T(EP_CLSID), NULL))
    {
        SwitchToThisWindow(hOther, TRUE);
        return 0;
    }

    HKEY hKey = NULL;
    DWORD dwSize = sizeof(DWORD);
    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        TEXT(REGPATH),
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
    DWORD bAllocConsole = FALSE;
    if (hKey)
    {
        dwSize = sizeof(DWORD);
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
    }
    locale = GetUserDefaultUILanguage();
    dwSize = LOCALE_NAME_MAX_LENGTH;
    if (hKey)
    {
        RegQueryValueExW(
            hKey,
            TEXT("Language"),
            0,
            NULL,
            &locale,
            &dwSize
        );
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
    ZeroMemory(&_this, sizeof(GUI));
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
    GetSystemDirectoryW(
        wszPath,
        MAX_PATH
    );
    wcscat_s(
        wszPath,
        MAX_PATH,
        L"\\shell32.dll"
    );

    WNDCLASS wc = { 0 };
    ZeroMemory(&wc, sizeof(WNDCLASSW));
    wc.style = 0;// CS_DBLCLKS;
    wc.lpfnWndProc = GUI_WindowProc;
    wc.hbrBackground = _this.hBackgroundBrush;
    wc.hInstance = hModule;
    wc.lpszClassName = L"ExplorerPatcher_GUI_" _T(EP_CLSID);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    HMODULE hShell32 = LoadLibraryExW(wszPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hShell32)
    {
        _this.hIcon = LoadIconW(hShell32, MAKEINTRESOURCEW(40)); //40
        wc.hIcon = _this.hIcon;
    }
    RegisterClassW(&wc);

    TCHAR title[260];
    HMODULE hExplorerFrame = LoadLibraryExW(L"ExplorerFrame.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
    LoadStringW(hExplorerFrame, 50222, title, 260); // 726 = File Explorer
    FreeLibrary(hExplorerFrame);
    wchar_t* p = wcschr(title, L'(');
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
    GUI_RegQueryValueExW(NULL, L"Virtualized_" _T(EP_CLSID) L"_TaskbarPosition", NULL, NULL, &dwTaskbarPosition, NULL);
    HWND hwnd = CreateWindowEx(
        NULL,
        L"ExplorerPatcher_GUI_" _T(EP_CLSID),
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
            /*WTA_OPTIONS ops;
            ops.dwFlags = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
            ops.dwMask = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
            SetWindowThemeAttribute(
                hwnd,
                WTA_NONCLIENT,
                &ops,
                sizeof(WTA_OPTIONS)
            );*/
        }
    }
    ShowWindow(hwnd, SW_SHOW);
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hShell32)
    {
        CloseHandle(_this.hIcon);
        FreeLibrary(hShell32);
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

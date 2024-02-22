#include <Windows.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <stdio.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include "resources/resource.h"
#include "../ExplorerPatcher/utility.h"

BOOL SetupShortcut(BOOL bInstall, WCHAR* wszPath, WCHAR* wszArguments)
{
    WCHAR wszTitle[MAX_PATH];
    ZeroMemory(wszTitle, MAX_PATH);
    WCHAR wszExplorerPath[MAX_PATH];
    ZeroMemory(wszExplorerPath, MAX_PATH);
    GetSystemDirectoryW(wszExplorerPath, MAX_PATH);
    wcscat_s(wszExplorerPath, MAX_PATH, L"\\ExplorerFrame.dll");
    if (bInstall)
    {
        HMODULE hExplorerFrame = LoadLibraryExW(wszExplorerPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (hExplorerFrame)
        {
            LoadStringW(hExplorerFrame, 50222, wszTitle, 260); // 726 = File Explorer
            wchar_t* p = wcschr(wszTitle, L'(');
            if (p)
            {
                p--;
                if (*p == L' ')
                {
                    *p = 0;
                }
                else
                {
                    p++;
                    *p = 0;
                }
            }
            if (wszTitle[0] == 0)
            {
                wcscat_s(wszTitle, MAX_PATH, _T(PRODUCT_NAME));
            }
        }
        else
        {
            wcscat_s(wszTitle, MAX_PATH, _T(PRODUCT_NAME));
        }
    }
    BOOL bOk = FALSE;
    WCHAR wszStartPrograms[MAX_PATH + 1];
    ZeroMemory(wszStartPrograms, MAX_PATH + 1);
    SHGetFolderPathW(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, wszStartPrograms);
    wcscat_s(wszStartPrograms, MAX_PATH + 1, L"\\" _T(PRODUCT_NAME));
    wszStartPrograms[wcslen(wszStartPrograms) + 1] = 0;
    SHFILEOPSTRUCTW op;
    ZeroMemory(&op, sizeof(SHFILEOPSTRUCTW));
    op.wFunc = FO_DELETE;
    op.pFrom = wszStartPrograms;
    op.fFlags = FOF_NO_UI;
    bOk = SHFileOperationW(&op);
    bOk = !bOk;
    if (bInstall)
    {
        if (!CreateDirectoryW(wszStartPrograms, NULL))
        {
            return FALSE;
        }
    }
    else
    {
        return bOk;
    }
    wcscat_s(wszStartPrograms, MAX_PATH, L"\\");
    wcscat_s(wszStartPrograms, MAX_PATH, wszTitle);
    wcscat_s(wszStartPrograms, MAX_PATH, L" (");
    wcscat_s(wszStartPrograms, MAX_PATH, _T(PRODUCT_NAME) L").lnk");
    ZeroMemory(wszExplorerPath, MAX_PATH);
    GetSystemDirectoryW(wszExplorerPath, MAX_PATH);
    wcscat_s(wszExplorerPath, MAX_PATH, L"\\shell32.dll");
    if (bInstall)
    {
        if (SUCCEEDED(CoInitialize(0)))
        {
            IShellLinkW* pShellLink = NULL;
            if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC, &IID_IShellLinkW, &pShellLink)))
            {
                pShellLink->lpVtbl->SetPath(pShellLink, wszPath);
                pShellLink->lpVtbl->SetArguments(pShellLink, wszArguments);
                pShellLink->lpVtbl->SetIconLocation(pShellLink, wszExplorerPath, 40 - 1);
                PathRemoveFileSpecW(wszExplorerPath);
                pShellLink->lpVtbl->SetWorkingDirectory(pShellLink, wszExplorerPath);
                pShellLink->lpVtbl->SetDescription(pShellLink, _T(PRODUCT_NAME));

                IPersistFile* pPersistFile = NULL;
                if (SUCCEEDED(pShellLink->lpVtbl->QueryInterface(pShellLink, &IID_IPersistFile, &pPersistFile)))
                {
                    if (SUCCEEDED(pPersistFile->lpVtbl->Save(pPersistFile, wszStartPrograms, TRUE)))
                    {
                        bOk = TRUE;
                    }
                    pPersistFile->lpVtbl->Release(pPersistFile);
                }
                pShellLink->lpVtbl->Release(pShellLink);
            }
            CoUninitialize();
        }
    }
    return bOk;
}

BOOL SetupUninstallEntry(BOOL bInstall, WCHAR* wszPath)
{
    DWORD dwLastError = ERROR_SUCCESS;
    HKEY hKey = NULL;

    if (bInstall)
    {

        if (!dwLastError)
        {
            dwLastError = RegCreateKeyExW(
                HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" _T(EP_CLSID) L"_" _T(PRODUCT_NAME),
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE | KEY_WOW64_64KEY,
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
                if (!dwLastError)
                {
                    dwLastError = RegSetValueExW(
                        hKey,
                        L"UninstallString",
                        0,
                        REG_SZ,
                        wszPath,
                        (wcslen(wszPath) + 1) * sizeof(wchar_t)
                    );
                }
                if (!dwLastError)
                {
                    dwLastError = RegSetValueExW(
                        hKey,
                        L"DisplayName",
                        0,
                        REG_SZ,
                        _T(PRODUCT_NAME),
                        (wcslen(_T(PRODUCT_NAME)) + 1) * sizeof(wchar_t)
                    );
                }
                if (!dwLastError)
                {
                    dwLastError = RegSetValueExW(
                        hKey,
                        L"Publisher",
                        0,
                        REG_SZ,
                        _T(PRODUCT_PUBLISHER),
                        (wcslen(_T(PRODUCT_PUBLISHER)) + 1) * sizeof(wchar_t)
                    );
                }
                if (!dwLastError)
                {
                    DWORD dw1 = TRUE;
                    dwLastError = RegSetValueExW(
                        hKey,
                        L"NoModify",
                        0,
                        REG_DWORD,
                        &dw1,
                        sizeof(DWORD)
                    );
                }
                if (!dwLastError)
                {
                    DWORD dw1 = TRUE;
                    dwLastError = RegSetValueExW(
                        hKey,
                        L"NoRepair",
                        0,
                        REG_DWORD,
                        &dw1,
                        sizeof(DWORD)
                    );
                }
                if (!dwLastError)
                {
                    PathRemoveFileSpecW(wszPath + 1);
                    wcscat_s(wszPath + 1, MAX_PATH - 2, L"\\" _T(PRODUCT_NAME) L".amd64.dll");
                    HMODULE hEP = LoadLibraryExW(wszPath + 1, NULL, LOAD_LIBRARY_AS_DATAFILE);
                    if (hEP)
                    {
                        DWORD dwLeftMost = 0;
                        DWORD dwSecondLeft = 0;
                        DWORD dwSecondRight = 0;
                        DWORD dwRightMost = 0;

                        QueryVersionInfo(hEP, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

                        WCHAR wszBuf[30];
                        swprintf_s(wszBuf, 30, L"%d.%d.%d.%d", dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);

                        if (!dwLastError)
                        {
                            dwLastError = RegSetValueExW(
                                hKey,
                                L"DisplayVersion",
                                0,
                                REG_SZ,
                                wszBuf,
                                (wcslen(wszBuf) + 1) * sizeof(wchar_t)
                            );
                            if (!dwLastError)
                            {
                                dwLastError = RegSetValueExW(
                                    hKey,
                                    L"VersionMajor",
                                    0,
                                    REG_DWORD,
                                    &dwSecondRight,
                                    sizeof(DWORD)
                                );
                                if (!dwLastError)
                                {
                                    dwLastError = RegSetValueExW(
                                        hKey,
                                        L"VersionMinor",
                                        0,
                                        REG_DWORD,
                                        &dwRightMost,
                                        sizeof(DWORD)
                                    );
                                }
                            }
                        }

                        FreeLibrary(hEP);
                    }
                }
                if (!dwLastError)
                {
                    GetWindowsDirectoryW(wszPath, MAX_PATH);
                    wcscat_s(wszPath, MAX_PATH, L"\\explorer.exe");
                    dwLastError = RegSetValueExW(
                        hKey,
                        L"DisplayIcon",
                        0,
                        REG_SZ,
                        wszPath,
                        (wcslen(wszPath) + 1) * sizeof(wchar_t)
                    );
                }
                RegCloseKey(hKey);
            }
        }
    }
    else
    {
        if (!dwLastError)
        {
            dwLastError = RegOpenKeyW(
                HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" _T(EP_CLSID) L"_" _T(PRODUCT_NAME),
                &hKey
            );
            if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
            {
                hKey = NULL;
            }
            if (hKey)
            {
                dwLastError = RegDeleteTreeW(hKey, NULL);
                RegCloseKey(hKey);
            }
        }
     }
     return !dwLastError;
}

BOOL InstallResourceHelper(BOOL bInstall, HMODULE hModule, HRSRC hRscr, WCHAR* wszPath)
{
    WCHAR wszReplace[MAX_PATH];
    wcscpy_s(wszReplace, MAX_PATH, wszPath);
    PathRemoveExtensionW(wszReplace);
    wcscat_s(wszReplace, MAX_PATH, L".prev");
    BOOL bFileExists = PathFileExistsW(wszPath);
    BOOL bPrevExists = PathFileExistsW(wszReplace);
    if (bFileExists || bPrevExists)
    {
        BOOL bRet = !bPrevExists || DeleteFileW(wszReplace);
        if (bRet || (!bRet && GetLastError() == ERROR_FILE_NOT_FOUND))
        {
            if (bFileExists && !DeleteFileW(wszPath) && !MoveFileW(wszPath, wszReplace))
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    if (!hRscr)
    {
        if (bInstall)
        {
            wchar_t path[MAX_PATH];
            GetModuleFileNameW(hModule, path, MAX_PATH);
            return CopyFileW(path, wszPath, FALSE);
        }
        return TRUE;
    }
    else
    {
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
        void* pRscr = LockResource(hgRscr);
        DWORD cbRscr = SizeofResource(
            hModule,
            hRscr
        );
        if (bInstall)
        {
            HANDLE hFile = CreateFileW(
                wszPath,
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );
            if (!hFile)
            {
                return FALSE;
            }
            DWORD dwNumberOfBytesWritten = 0;
            if (!WriteFile(
                hFile,
                pRscr,
                cbRscr,
                &dwNumberOfBytesWritten,
                NULL
            ))
            {
                return FALSE;
            }
            CloseHandle(hFile);
        }
        return TRUE;
    }
}

BOOL InstallResource(BOOL bInstall, HMODULE hInstance, int res, LPCWSTR pwszDirectory, LPCWSTR pwszFileName)
{
    BOOL bOk = TRUE;
    HRSRC hRscr = NULL;
    if (res == 0 || ((hRscr = FindResourceW(hInstance, MAKEINTRESOURCE(res), RT_RCDATA))))
    {
        WCHAR wszPath[MAX_PATH];
        wcscpy_s(wszPath, MAX_PATH, pwszDirectory);
        wcscat_s(wszPath, MAX_PATH, L"\\");
        wcscat_s(wszPath, MAX_PATH, pwszFileName);
        bOk = InstallResourceHelper(bInstall, hInstance, hRscr, wszPath);
    }
    return bOk;
}

BOOL DeleteResource(LPCWSTR pwszDirectory, LPCWSTR pwszFileName)
{
    WCHAR wszPath[MAX_PATH];
    wcscpy_s(wszPath, MAX_PATH, pwszDirectory);
    wcscat_s(wszPath, MAX_PATH, L"\\");
    wcscat_s(wszPath, MAX_PATH, pwszFileName);
    return InstallResourceHelper(FALSE, NULL, NULL, wszPath);
}

BOOL ShouldDownloadOrDelete(BOOL bInstall, WCHAR* wszPath, LPCSTR chash)
{
    if (FileExistsW(wszPath))
    {
        char hash[100];
        ZeroMemory(hash, sizeof(char) * 100);
        ComputeFileHash(wszPath, hash, 100);
        if (_stricmp(hash, chash) != 0)
        {
            if (bInstall)
            {
                return TRUE;
            }
        }
        else
        {
            if (!bInstall)
            {
                return InstallResourceHelper(FALSE, NULL, NULL, wszPath); // Delete
            }
        }
    }
    else
    {
        if (bInstall)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL DownloadResource(BOOL bInstall, LPCWSTR pwszURL, DWORD dwSize, LPCSTR chash, LPCWSTR pwszDirectory, LPCWSTR pwszFileName)
{
    BOOL bOk = TRUE;
    WCHAR wszPath[MAX_PATH];
    wcscpy_s(wszPath, MAX_PATH, pwszDirectory);
    wcscat_s(wszPath, MAX_PATH, L"\\");
    wcscat_s(wszPath, MAX_PATH, pwszFileName);
    if (ShouldDownloadOrDelete(bInstall, wszPath, chash) && IsConnectedToInternet() == TRUE)
    {
        bOk = DownloadFile(pwszURL, dwSize, wszPath);
    }
    return bOk;
}

void ProcessTaskbarDlls(BOOL* bInOutOk, BOOL bInstall, BOOL bExtractMode, HINSTANCE hInstance, WCHAR wszPath[260])
{
#if WITH_ALT_TASKBAR_IMPL
    LPCWSTR pwszTaskbarDllName = bExtractMode ? NULL : PickTaskbarDll();
    if (*bInOutOk) *bInOutOk = InstallResource(bInstall && (bExtractMode || pwszTaskbarDllName && !wcscmp(pwszTaskbarDllName, L"ep_taskbar.2.dll")), hInstance, IDR_EP_TASKBAR_2, wszPath, L"ep_taskbar.2.dll");
    if (*bInOutOk) *bInOutOk = InstallResource(bInstall && (bExtractMode || pwszTaskbarDllName && !wcscmp(pwszTaskbarDllName, L"ep_taskbar.3.dll")), hInstance, IDR_EP_TASKBAR_3, wszPath, L"ep_taskbar.3.dll");
    if (*bInOutOk) *bInOutOk = InstallResource(bInstall && (bExtractMode || pwszTaskbarDllName && !wcscmp(pwszTaskbarDllName, L"ep_taskbar.4.dll")), hInstance, IDR_EP_TASKBAR_4, wszPath, L"ep_taskbar.4.dll");
    if (*bInOutOk) *bInOutOk = InstallResource(bInstall && (bExtractMode || pwszTaskbarDllName && !wcscmp(pwszTaskbarDllName, L"ep_taskbar.5.dll")), hInstance, IDR_EP_TASKBAR_5, wszPath, L"ep_taskbar.5.dll");
#endif
}

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    global_ubr = VnGetOSVersionAndUBR(&global_rovi);

    BOOL bOk = TRUE, bInstall = TRUE, bWasShellExt = FALSE, bIsUpdate = FALSE, bForcePromptForUninstall = FALSE;

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    int argc = 0;
    LPWSTR* wargv = CommandLineToArgvW(
        lpCmdLine,
        &argc
    );

    WCHAR wszPath[MAX_PATH];
    ZeroMemory(wszPath, MAX_PATH * sizeof(WCHAR));

    if (argc >= 1 && !_wcsicmp(wargv[0], L"/extract"))
    {
        if (argc >= 2)
        {
            wcsncpy_s(wszPath, MAX_PATH, wargv[1], MAX_PATH);
            CreateDirectoryW(wargv[1], NULL);
        }
        else
        {
            GetCurrentDirectoryW(MAX_PATH, wszPath);
        }
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_IA32, wszPath, _T(PRODUCT_NAME) L".IA-32.dll");
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_AMD64, wszPath, _T(PRODUCT_NAME) L".amd64.dll");
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_DWM, wszPath, L"ep_dwm.exe");
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_WEATHER, wszPath, L"ep_weather_host.dll");
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_WEATHER_STUB, wszPath, L"ep_weather_host_stub.dll");
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_MS_WEBVIEW2_LOADER, wszPath, L"WebView2Loader.dll");
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_STARTMENU, wszPath, L"wincorlib.dll");
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_GUI, wszPath, L"ep_gui.dll");
        ProcessTaskbarDlls(&bOk, bInstall, TRUE, hInstance, wszPath);
        return !bOk;
    }

    WCHAR wszOwnPath[MAX_PATH];
    ZeroMemory(wszOwnPath, ARRAYSIZE(wszOwnPath));
    if (!GetModuleFileNameW(NULL, wszOwnPath, ARRAYSIZE(wszOwnPath)))
    {
        exit(0);
    }

    bInstall = !(argc >= 1 && (!_wcsicmp(wargv[0], L"/uninstall") || !_wcsicmp(wargv[0], L"/uninstall_silent")));
    PathStripPathW(wszOwnPath);
    if (!_wcsicmp(wszOwnPath, L"ep_uninstall.exe"))
    {
        bInstall = FALSE;
        bForcePromptForUninstall = _wcsicmp(wargv[0], L"/uninstall_silent");
    }
    if (!GetModuleFileNameW(NULL, wszOwnPath, ARRAYSIZE(wszOwnPath)))
    {
        exit(0);
    }
    bIsUpdate = (argc >= 1 && !_wcsicmp(wargv[0], L"/update_silent"));
    if (!bInstall && (!_wcsicmp(wargv[0], L"/uninstall") || bForcePromptForUninstall))
    {
        HWND hwndExistingMb = FindWindowExW(NULL, NULL, L"#32770", _T(PRODUCT_NAME));
        if (hwndExistingMb)
        {
            SwitchToThisWindow(hwndExistingMb, TRUE);
            exit(0);
        }

        wchar_t mbText[256];
        mbText[0] = 0;
        LoadStringW(hInstance, IDS_SETUP_UNINSTALL_PROMPT, mbText, ARRAYSIZE(mbText));
        if (MessageBoxW(NULL, mbText, _T(PRODUCT_NAME), MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDNO)
        {
            exit(0);
        }
    }

    if (!IsAppRunningAsAdminMode())
    {
        SHELLEXECUTEINFOW sei;
        ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
        sei.cbSize = sizeof(sei);
        sei.lpVerb = L"runas";
        sei.lpFile = wszOwnPath;
        sei.lpParameters = !bInstall ? L"/uninstall_silent" : lpCmdLine;
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;
        if (!ShellExecuteExW(&sei))
        {
            DWORD dwError = GetLastError();
            if (dwError == ERROR_CANCELLED)
            {
            }
        }
        exit(0);
    }

    DWORD bIsUndockingDisabled = FALSE, dwSize = sizeof(DWORD);
    RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell\\Update\\Packages", L"UndockingDisabled", RRF_RT_DWORD, NULL, &bIsUndockingDisabled, &dwSize);
    if (bIsUndockingDisabled)
    {
        wchar_t mbText[256];
        mbText[0] = 0;
        LoadStringW(hInstance, bInstall ? IDS_SETUP_INSTALL_LOGOFF : IDS_SETUP_UNINSTALL_LOGOFF, mbText, ARRAYSIZE(mbText));
        if (MessageBoxW(NULL, mbText, _T(PRODUCT_NAME), MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION) == IDYES)
        {
            RegDeleteKeyValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell\\Update\\Packages", L"UndockingDisabled");
        }
        else
        {
            exit(0);
        }
    }

    CreateEventW(NULL, FALSE, FALSE, _T(EP_SETUP_EVENTNAME));

    SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
    wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH));
    bOk = CreateDirectoryW(wszPath, NULL);
    if (bOk || (!bOk && GetLastError() == ERROR_ALREADY_EXISTS))
    {
        bOk = TRUE;
        HANDLE userToken = INVALID_HANDLE_VALUE;

        HWND hShellTrayWnd = FindWindowW(L"Shell_TrayWnd", NULL);
        if (hShellTrayWnd)
        {
            DWORD explorerProcessId = 0;
            GetWindowThreadProcessId(hShellTrayWnd, &explorerProcessId);
            if (explorerProcessId != 0)
            {
                HANDLE explorerProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, explorerProcessId);
                if (explorerProcess != NULL) 
                {
                    OpenProcessToken(explorerProcess, TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, &userToken);
                    CloseHandle(explorerProcess);
                }
                if (userToken) 
                {
                    HANDLE myToken = INVALID_HANDLE_VALUE;
                    OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, &myToken);
                    if (myToken != INVALID_HANDLE_VALUE) 
                    {
                        DWORD cbSizeNeeded = 0;
                        SetLastError(0);
                        if (!GetTokenInformation(userToken, TokenUser, NULL, 0, &cbSizeNeeded) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                        {
                            TOKEN_USER* userTokenInfo = malloc(cbSizeNeeded);
                            if (userTokenInfo) 
                            {
                                if (GetTokenInformation(userToken, TokenUser, userTokenInfo, cbSizeNeeded, &cbSizeNeeded))
                                {
                                    cbSizeNeeded = 0;
                                    SetLastError(0);
                                    if (!GetTokenInformation(myToken, TokenUser, NULL, 0, &cbSizeNeeded) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                                    {
                                        TOKEN_USER* myTokenInfo = malloc(cbSizeNeeded);
                                        if (myTokenInfo)
                                        {
                                            if (GetTokenInformation(myToken, TokenUser, myTokenInfo, cbSizeNeeded, &cbSizeNeeded))
                                            {
                                                if (EqualSid(userTokenInfo->User.Sid, myTokenInfo->User.Sid))
                                                {
                                                    CloseHandle(userToken);
                                                    userToken = INVALID_HANDLE_VALUE;
                                                }
                                            }
                                            free(myTokenInfo);
                                        }
                                    }
                                }
                                free(userTokenInfo);
                            }
                        }
                        CloseHandle(myToken);
                    }
                }
            }
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
                    BeginExplorerRestart(NULL);
                }
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

        Sleep(500);

        BOOL bAreRoundedCornersDisabled = FALSE;
        HANDLE h_exists = CreateEventW(NULL, FALSE, FALSE, L"Global\\ep_dwm_" _T(EP_CLSID));
        if (h_exists)
        {
            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                bAreRoundedCornersDisabled = TRUE;
            }
            else
            {
                bAreRoundedCornersDisabled = FALSE;
            }
            CloseHandle(h_exists);
        }
        else
        {
            if (GetLastError() == ERROR_ACCESS_DENIED)
            {
                bAreRoundedCornersDisabled = TRUE;
            }
            else
            {
                bAreRoundedCornersDisabled = FALSE;
            }
        }
        if (bAreRoundedCornersDisabled)
        {
            RegisterDWMService(0, 1);
            RegisterDWMService(0, 3);
        }

        WCHAR wszSCPath[MAX_PATH];
        GetSystemDirectoryW(wszSCPath, MAX_PATH);
        wcscat_s(wszSCPath, MAX_PATH, L"\\sc.exe");
        SHELLEXECUTEINFO ShExecInfo = { 0 };
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = L"runas";
        ShExecInfo.lpFile = wszSCPath;
        ShExecInfo.lpParameters = L"stop " _T(EP_DWM_SERVICENAME);
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_HIDE;
        ShExecInfo.hInstApp = NULL;
        if (ShellExecuteExW(&ShExecInfo) && ShExecInfo.hProcess)
        {
            WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
            DWORD dwExitCode = 0;
            GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
            CloseHandle(ShExecInfo.hProcess);
        }

        HWND hWnd = FindWindowW(L"ExplorerPatcher_GUI_" _T(EP_CLSID), NULL);
        if (hWnd)
        {
            DWORD dwGUIPid = 0;
            GetWindowThreadProcessId(hWnd, &dwGUIPid);
            if (dwGUIPid)
            {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwGUIPid);
                if (hProcess)
                {
                    DWORD dwSection = SendMessageW(hWnd, WM_MSG_GUI_SECTION, WM_MSG_GUI_SECTION_GET, 0);

                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);

                    HKEY hKey = NULL;

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
                        RegSetValueExW(
                            hKey,
                            TEXT("OpenPropertiesAtNextStart"),
                            0,
                            REG_DWORD,
                            &dwSection,
                            sizeof(DWORD)
                        );
                        RegCloseKey(hKey);
                    }
                }
            }
        }

        Sleep(1000);

        // --------------------------------------------------------------------------------

        // C:\Program Files\ExplorerPatcher
        SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
        wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH));
        if (bOk && bInstall) bOk = InstallResource(bInstall, hInstance, 0, wszPath, _T(SETUP_UTILITY_NAME));
        if (bOk)
        {
            if (!bInstall)
            {
                HKEY hKey;
                RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Classes\\CLSID\\" TEXT(EP_CLSID) L"\\InProcServer32",
                    REG_OPTION_NON_VOLATILE,
                    KEY_READ,
                    &hKey
                );
                if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
                {
                    hKey = NULL;
                }
                if (hKey)
                {
                    bWasShellExt = TRUE;
                    RegCloseKey(hKey);
                }
                if (bWasShellExt)
                {
                    WCHAR wszArgs[MAX_PATH];
                    wszArgs[0] = L'/';
                    wszArgs[1] = L'u';
                    wszArgs[2] = L' ';
                    wszArgs[3] = L'"';
                    SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszArgs + 4);
                    wcscat_s(wszArgs, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\" _T(PRODUCT_NAME) L".amd64.dll\"");
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
                    sei.lpVerb = NULL;
                    sei.lpFile = wszApp;
                    sei.lpParameters = wszArgs;
                    sei.hwnd = NULL;
                    sei.nShow = SW_NORMAL;
                    if (ShellExecuteExW(&sei) && sei.hProcess)
                    {
                        WaitForSingleObject(sei.hProcess, INFINITE);
                        DWORD dwExitCode = 0;
                        GetExitCodeProcess(sei.hProcess, &dwExitCode);
                        SetLastError(dwExitCode);
                        CloseHandle(sei.hProcess);
                    }
                }
            }
        }
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_IA32, wszPath, _T(PRODUCT_NAME) L".IA-32.dll");
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_AMD64, wszPath, _T(PRODUCT_NAME) L".amd64.dll");
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_GUI, wszPath, L"ep_gui.dll");
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_DWM, wszPath, L"ep_dwm.exe");
        if (bInstall)
        {
            if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_WEATHER, wszPath, L"ep_weather_host.dll");
            if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_WEATHER_STUB, wszPath, L"ep_weather_host_stub.dll");
            if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_MS_WEBVIEW2_LOADER, wszPath, L"WebView2Loader.dll");
        }
        ProcessTaskbarDlls(&bOk, bInstall, FALSE, hInstance, wszPath);

        // --------------------------------------------------------------------------------

        // C:\Windows
        // + dxgi.dll
        if (bOk) GetWindowsDirectoryW(wszPath, MAX_PATH);
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_AMD64, wszPath, L"dxgi.dll");

        // --------------------------------------------------------------------------------

        // C:\Windows\SystemApps\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy
        // + dxgi.dll
        // + wincorlib.dll
        // + wincorlib_orig.dll (symlink)
        // - AppResolverLegacy.dll
        // - StartTileDataLegacy.dll
        // - Windows.UI.ShellCommon.pri
        // - en-US\StartTileDataLegacy.dll.mui
        // - pris2\Windows.UI.ShellCommon.en-US.pri
        if (bOk) GetWindowsDirectoryW(wszPath, MAX_PATH);
        if (bOk) wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy");
        if (bOk) bOk = InstallResource(bInstall, hInstance, IDR_EP_AMD64, wszPath, L"dxgi.dll");
        if (bOk) bOk = InstallResource(bInstall && IsWindows11(), hInstance, IDR_EP_STARTMENU, wszPath, L"wincorlib.dll");
        if (bOk) bOk = DeleteResource(wszPath, L"wincorlib_orig.dll");
        if (bOk && IsWindows11() && bInstall)
        {
            // Symlink wincorlib_orig.dll to wincorlib.dll in System32
            WCHAR wszOrigPath[MAX_PATH];
            GetSystemDirectoryW(wszOrigPath, MAX_PATH);
            wcscat_s(wszOrigPath, MAX_PATH, L"\\wincorlib.dll");

            WCHAR wszSymLinkPath[MAX_PATH];
            wcscpy_s(wszSymLinkPath, MAX_PATH, wszPath);
            wcscat_s(wszSymLinkPath, MAX_PATH, L"\\wincorlib_orig.dll");
            bOk = CreateSymbolicLinkW(wszSymLinkPath, wszOrigPath, 0);
        }

        // Delete remnants from earlier versions
        if (bOk) bOk = DeleteResource(wszPath, L"AppResolverLegacy.dll");
        if (bOk) bOk = DeleteResource(wszPath, L"StartTileDataLegacy.dll");
        if (bOk && IsWindows11()) bOk = DeleteResource(wszPath, L"Windows.UI.ShellCommon.pri");

        // .\en-US
        if (bOk && IsWindows11())
        {
            WCHAR wszSubPath[MAX_PATH];
            wcscpy_s(wszSubPath, MAX_PATH, wszPath);
            wcscat_s(wszSubPath, MAX_PATH, L"\\en-US");
            if (FileExistsW(wszSubPath))
            {
                bOk = DeleteResource(wszSubPath, L"StartTileDataLegacy.dll.mui");
                if (bOk) bOk = RemoveDirectoryW(wszSubPath);
            }
        }

        // .\pris2
        if (bOk && IsWindows11())
        {
            WCHAR wszSubPath[MAX_PATH];
            wcscpy_s(wszSubPath, MAX_PATH, wszPath);
            wcscat_s(wszSubPath, MAX_PATH, L"\\pris2");
            if (FileExistsW(wszSubPath))
            {
                bOk = DeleteResource(wszSubPath, L"Windows.UI.ShellCommon.en-US.pri");
                if (bOk) bOk = RemoveDirectoryW(wszSubPath);
            }
        }

        // End remnant deletion

        // --------------------------------------------------------------------------------

        // C:\Windows\SystemApps\ShellExperienceHost_cw5n1h2txyewy
        // + dxgi.dll
        if (bOk) GetWindowsDirectoryW(wszPath, MAX_PATH);
        if (bOk) wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\ShellExperienceHost_cw5n1h2txyewy");
        if (bOk && IsWindows11()) bOk = InstallResource(bInstall, hInstance, IDR_EP_AMD64, wszPath, L"dxgi.dll");

        // --------------------------------------------------------------------------------

        if (bOk)
        {
            GetSystemDirectoryW(wszPath, MAX_PATH);
            WCHAR* pArgs = NULL;
            DWORD dwLen = wcslen(wszPath);
            wcscat_s(wszPath, MAX_PATH - dwLen, L"\\rundll32.exe \"");
            dwLen = wcslen(wszPath);
            pArgs = wszPath + dwLen - 2;
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath + dwLen);
            wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\ep_gui.dll\",ZZGUI");
            pArgs[0] = 0;
            bOk = SetupShortcut(bInstall, wszPath, pArgs + 1);
            ZeroMemory(wszPath, MAX_PATH);
        }
        if (bOk)
        {
            wszPath[0] = L'"';
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath + 1);
            wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\" _T(SETUP_UTILITY_NAME) L"\" /uninstall");
            bOk = SetupUninstallEntry(bInstall, wszPath);
        }
        ShExecInfo.lpParameters = bInstall ? L"start " _T(EP_DWM_SERVICENAME) : L"delete " _T(EP_DWM_SERVICENAME);
        if (ShellExecuteExW(&ShExecInfo) && ShExecInfo.hProcess)
        {
            WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
            DWORD dwExitCode = 0;
            GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
            CloseHandle(ShExecInfo.hProcess);
        }
        if (bOk)
        {
            WCHAR wszArgs[MAX_PATH];
            wszArgs[0] = L'/';
            wszArgs[1] = L's';
            wszArgs[2] = L' ';
            wszArgs[3] = L'"';
            if (!bInstall)
            {
                wszArgs[3] = L'/';
                wszArgs[4] = L'u';
                wszArgs[5] = L' ';
                wszArgs[6] = L'"';
            }
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszArgs + 4 + (bInstall ? 0 : 3));
            wcscat_s(wszArgs, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\ep_weather_host.dll\"");
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
            sei.lpVerb = NULL;
            sei.lpFile = wszApp;
            sei.lpParameters = wszArgs;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;
            if (ShellExecuteExW(&sei) && sei.hProcess)
            {
                WaitForSingleObject(sei.hProcess, INFINITE);
                DWORD dwExitCode = 0;
                GetExitCodeProcess(sei.hProcess, &dwExitCode);
                SetLastError(dwExitCode);
                CloseHandle(sei.hProcess);
            }
        }
        if (bOk)
        {
            WCHAR wszArgs[MAX_PATH];
            wszArgs[0] = L'/';
            wszArgs[1] = L's';
            wszArgs[2] = L' ';
            wszArgs[3] = L'"';
            if (!bInstall)
            {
                wszArgs[3] = L'/';
                wszArgs[4] = L'u';
                wszArgs[5] = L' ';
                wszArgs[6] = L'"';
            }
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszArgs + 4 + (bInstall ? 0 : 3));
            wcscat_s(wszArgs, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\ep_weather_host_stub.dll\"");
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
            sei.lpVerb = NULL;
            sei.lpFile = wszApp;
            sei.lpParameters = wszArgs;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;
            if (ShellExecuteExW(&sei) && sei.hProcess)
            {
                WaitForSingleObject(sei.hProcess, INFINITE);
                DWORD dwExitCode = 0;
                GetExitCodeProcess(sei.hProcess, &dwExitCode);
                SetLastError(dwExitCode);
                CloseHandle(sei.hProcess);
            }
        }
        if (bOk && bInstall)
        {
            HKEY hKey = NULL;
            RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Policies\\Microsoft\\Windows\\Explorer", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hKey, NULL);
            if (hKey && hKey != INVALID_HANDLE_VALUE)
            {
                RegCloseKey(hKey);
            }
            RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hKey, NULL);
            if (hKey && hKey != INVALID_HANDLE_VALUE)
            {
                RegCloseKey(hKey);
            }
        }
        if (!bInstall)
        {
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
            wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH));
            if (bOk) bOk = DeleteResource(wszPath, L"ep_weather_host.dll");
            if (bOk) bOk = DeleteResource(wszPath, L"ep_weather_host_stub.dll");
            if (bOk) bOk = DeleteResource(wszPath, L"WebView2Loader.dll");
        }

        if (bOk)
        {
            if (!bInstall)
            {
                WCHAR wszTempPath[MAX_PATH];
                GetTempPathW(MAX_PATH, wszTempPath);
                wcscat_s(wszTempPath, MAX_PATH, _T(SETUP_UTILITY_NAME));
                if (MoveFileExW(wszOwnPath, wszTempPath, MOVEFILE_REPLACE_EXISTING))
                {
                    HKEY hKey = NULL;
                    RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL);
                    if (hKey && hKey != INVALID_HANDLE_VALUE)
                    {
                        WCHAR wszCommand[MAX_PATH];
                        wcscpy_s(wszCommand, MAX_PATH, L"cmd /c del /f /q \"");
                        wcscat_s(wszCommand, MAX_PATH, wszTempPath);
                        wcscat_s(wszCommand, MAX_PATH, L"\"");
                        RegSetValueExW(hKey, L"ExplorerPatcherCleanup", 0, REG_SZ, (BYTE*)wszCommand, (DWORD)((wcslen(wszCommand) + 1) * sizeof(WCHAR)));
                        RegCloseKey(hKey);
                    }
                }

                SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
                wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH));
                RemoveDirectoryW(wszPath);

                wchar_t mbText[256];
                mbText[0] = 0;
                if (bWasShellExt)
                {
                    LoadStringW(hInstance, IDS_SETUP_UNINSTALL_RESTART, mbText, ARRAYSIZE(mbText));
                    if (MessageBoxW(NULL, mbText, _T(PRODUCT_NAME), MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION) == IDYES)
                    {
                        SystemShutdown(TRUE);
                    }
                }
                else
                {
                    LoadStringW(hInstance, IDS_SETUP_UNINSTALL_FINISH, mbText, ARRAYSIZE(mbText));
                    MessageBoxW(NULL, mbText, _T(PRODUCT_NAME), MB_ICONASTERISK | MB_OK | MB_DEFBUTTON1);
                }
            }
            else
            {
                if (bIsUpdate)
                {
                    HKEY hKey = NULL;
                    DWORD dwSize = 0;

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
                        dwSize = TRUE;
                        RegSetValueExW(
                            hKey,
                            TEXT("IsUpdatePending"),
                            0,
                            REG_DWORD,
                            &dwSize,
                            sizeof(DWORD)
                        );
                        RegCloseKey(hKey);
                    }
                }
                //ZZRestartExplorer(0, 0, 0, 0);
            }
        }
        if (!bOk) //  && !(argc >= 1 && !_wcsicmp(wargv[0], L"/update_silent"))
        {
            wchar_t mbText[1024];
            mbText[0] = 0;
            LoadStringW(hInstance, IDS_SETUP_FAILED, mbText, ARRAYSIZE(mbText));
            MessageBoxW(NULL, mbText, _T(PRODUCT_NAME), MB_ICONERROR | MB_OK | MB_DEFBUTTON1);
        }
        if (bOk && bIsUndockingDisabled)
        {
            ExitWindowsEx(EWX_LOGOFF, SHTDN_REASON_FLAG_PLANNED);
            exit(0);
        }

        StartExplorerWithDelay(1000, userToken);
        if (userToken != INVALID_HANDLE_VALUE) CloseHandle(userToken);
    }

	return GetLastError();
}

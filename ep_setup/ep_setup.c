#include <Windows.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <stdio.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include "resource.h"
#include "../ExplorerPatcher/utility.h"

BOOL ShouldDownloadOrDelete(BOOL bInstall, HINSTANCE hInstance, LPCWSTR wszPath, LPCSTR chash)
{
    if (FileExistsW(wszPath))
    {
        WCHAR hash[100];
        ZeroMemory(hash, sizeof(WCHAR) * 100);
        ComputeFileHash(wszPath, hash, 100);
        if (_stricmp(hash, chash))
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
                return InstallResource(FALSE, hInstance, 0, wszPath); // Delete
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

BOOL InstallResource(BOOL bInstall, HMODULE hModule, int res, WCHAR* wszPath)
{
    if (PathFileExistsW(wszPath))
    {
        WCHAR wszReplace[MAX_PATH];
        wcscpy_s(wszReplace, MAX_PATH, wszPath);
        PathRemoveExtensionW(wszReplace);
        wcscat_s(wszReplace, MAX_PATH, L".prev");
        BOOL bRet = DeleteFileW(wszReplace);
        if (bRet || (!bRet && GetLastError() == ERROR_FILE_NOT_FOUND))
        {
            if (!MoveFileW(wszPath, wszReplace))
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    if (res == 0)
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
        HRSRC hRscr = FindResource(
            hModule,
            MAKEINTRESOURCE(res),
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

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    RTL_OSVERSIONINFOW rovi;
    DWORD32 ubr = VnGetOSVersionAndUBR(&rovi);

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
        if (bOk)
        {
            wcscat_s(wszPath, MAX_PATH, L"\\" _T(PRODUCT_NAME) L".IA-32.dll");
            bOk = InstallResource(TRUE, hInstance, IDR_EP_IA32, wszPath);
        }
        if (argc >= 2)
        {
            wcsncpy_s(wszPath, MAX_PATH, wargv[1], MAX_PATH);
        }
        else
        {
            GetCurrentDirectoryW(MAX_PATH, wszPath);
        }
        if (bOk)
        {
            wcscat_s(wszPath, MAX_PATH, L"\\" _T(PRODUCT_NAME) L".amd64.dll");
            bOk = InstallResource(TRUE, hInstance, IDR_EP_AMD64, wszPath);
        }
        if (argc >= 2)
        {
            wcsncpy_s(wszPath, MAX_PATH, wargv[1], MAX_PATH);
        }
        else
        {
            GetCurrentDirectoryW(MAX_PATH, wszPath);
        }
        if (bOk)
        {
            wcscat_s(wszPath, MAX_PATH, L"\\ep_dwm.exe");
            bOk = InstallResource(TRUE, hInstance, IDR_EP_DWM, wszPath);
        }
        if (argc >= 2)
        {
            wcsncpy_s(wszPath, MAX_PATH, wargv[1], MAX_PATH);
        }
        else
        {
            GetCurrentDirectoryW(MAX_PATH, wszPath);
        }
        if (bOk)
        {
            wcscat_s(wszPath, MAX_PATH, L"\\ep_weather_host.dll");
            bOk = InstallResource(TRUE, hInstance, IDR_EP_WEATHER, wszPath);
        }
        if (argc >= 2)
        {
            wcsncpy_s(wszPath, MAX_PATH, wargv[1], MAX_PATH);
        }
        else
        {
            GetCurrentDirectoryW(MAX_PATH, wszPath);
        }
        if (bOk)
        {
            wcscat_s(wszPath, MAX_PATH, L"\\ep_weather_host_stub.dll");
            bOk = InstallResource(TRUE, hInstance, IDR_EP_WEATHER_STUB, wszPath);
        }
        if (argc >= 2)
        {
            wcsncpy_s(wszPath, MAX_PATH, wargv[1], MAX_PATH);
        }
        else
        {
            GetCurrentDirectoryW(MAX_PATH, wszPath);
        }
        if (bOk)
        {
            wcscat_s(wszPath, MAX_PATH, L"\\WebView2Loader.dll");
            bOk = InstallResource(TRUE, hInstance, IDR_MS_WEBVIEW2_LOADER, wszPath);
        }
        if (argc >= 2)
        {
            wcsncpy_s(wszPath, MAX_PATH, wargv[1], MAX_PATH);
        }
        else
        {
            GetCurrentDirectoryW(MAX_PATH, wszPath);
        }
        if (bOk)
        {
            wcscat_s(wszPath, MAX_PATH, L"\\wincorlib.dll");
            bOk = InstallResource(TRUE, hInstance, IDR_EP_STARTMENU, wszPath);
        }
        return 0;
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
        if (MessageBoxW(
            NULL,
            L"Are you sure you want to remove " _T(PRODUCT_NAME) L" from your computer?",
            _T(PRODUCT_NAME),
            MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION
        ) == IDNO)
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
        if (MessageBoxW(
            NULL,
            bInstall ? L"In order to install, you will be automatically signed out of Windows. The software will be ready for use when you sign back in.\n\nDo you want to continue?"
                     : L"To complete the uninstallation, you will be automatically signed out of Windows.\n\nDo you want to continue?",
            _T(PRODUCT_NAME),
            MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION
        ) == IDYES)
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
                    BeginExplorerRestart();
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

        if (bOk)
        {
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
            wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\" _T(SETUP_UTILITY_NAME));
            bOk = InstallResource(bInstall, hInstance, 0, wszPath);
        }
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
        if (bOk)
        {
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
            wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\" _T(PRODUCT_NAME) L".IA-32.dll");
            bOk = InstallResource(bInstall, hInstance, IDR_EP_IA32, wszPath);
        }
        if (bOk)
        {
            PathRemoveFileSpecW(wszPath);
            wcscat_s(wszPath, MAX_PATH, L"\\" _T(PRODUCT_NAME) L".amd64.dll");
            bOk = InstallResource(bInstall, hInstance, IDR_EP_AMD64, wszPath);
        }
        if (bOk)
        {
            PathRemoveFileSpecW(wszPath);
            wcscat_s(wszPath, MAX_PATH, L"\\ep_dwm.exe");
            bOk = InstallResource(bInstall, hInstance, IDR_EP_DWM, wszPath);
        }
        if (bInstall)
        {
            if (bOk)
            {
                PathRemoveFileSpecW(wszPath);
                wcscat_s(wszPath, MAX_PATH, L"\\ep_weather_host.dll");
                bOk = InstallResource(bInstall, hInstance, IDR_EP_WEATHER, wszPath);
            }
            if (bOk)
            {
                PathRemoveFileSpecW(wszPath);
                wcscat_s(wszPath, MAX_PATH, L"\\ep_weather_host_stub.dll");
                bOk = InstallResource(bInstall, hInstance, IDR_EP_WEATHER_STUB, wszPath);
            }
            if (bOk)
            {
                PathRemoveFileSpecW(wszPath);
                wcscat_s(wszPath, MAX_PATH, L"\\WebView2Loader.dll");
                bOk = InstallResource(bInstall, hInstance, IDR_MS_WEBVIEW2_LOADER, wszPath);
            }
        }
        if (bOk)
        {
            bOk = GetWindowsDirectoryW(wszPath, MAX_PATH);
        }
        if (bOk)
        {
            wcscat_s(wszPath, MAX_PATH, L"\\dxgi.dll");
            bOk = InstallResource(bInstall, hInstance, IDR_EP_AMD64, wszPath);
        }
        if (bOk)
        {
            bOk = GetWindowsDirectoryW(wszPath, MAX_PATH);
        }
        if (bOk && rovi.dwBuildNumber >= 18362)
        {
            wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\dxgi.dll");
            bOk = InstallResource(bInstall, hInstance, IDR_EP_AMD64, wszPath);
        }
        if (bOk)
        {
            bOk = GetWindowsDirectoryW(wszPath, MAX_PATH);
        }
        if (bOk && IsWindows11())
        {
            wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\wincorlib.dll");
            bOk = InstallResource(bInstall, hInstance, IDR_EP_STARTMENU, wszPath);
        }
        if (bOk)
        {
            bOk = GetWindowsDirectoryW(wszPath, MAX_PATH);
        }
        if (bOk && IsWindows11())
        {
            wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\wincorlib_orig.dll");
            bOk = InstallResource(FALSE, hInstance, 0, wszPath); // Delete
        }
        if (bOk && IsWindows11())
        {
            if (bInstall)
            {
                WCHAR wszOrigPath[MAX_PATH];
                GetSystemDirectoryW(wszOrigPath, MAX_PATH);
                wcscat_s(wszOrigPath, MAX_PATH, L"\\wincorlib.dll");
                bOk = CreateSymbolicLinkW(wszPath, wszOrigPath, 0);
            }
        }
        if (bOk && IsWindows11())
        {
            GetWindowsDirectoryW(wszPath, MAX_PATH);
            wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\AppResolverLegacy.dll");
            if (FileExistsW(wszPath))
            {
                bOk = DeleteFileW(wszPath);
            }
        }
        if (bOk && IsWindows11())
        {
            GetWindowsDirectoryW(wszPath, MAX_PATH);
            wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\StartTileDataLegacy.dll");
            if (FileExistsW(wszPath))
            {
                bOk = DeleteFileW(wszPath);
            }
        }
        if (bOk && IsWindows11())
        {
            GetWindowsDirectoryW(wszPath, MAX_PATH);
            wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\Windows.UI.ShellCommon.pri");
            if (ShouldDownloadOrDelete(bInstall, hInstance, wszPath, "95b41e1a2661501036198d8225aaa605") && IsConnectedToInternet() == TRUE)
            {
                DownloadFile(L"https://github.com/valinet/ExplorerPatcher/files/8136442/Windows.UI.ShellCommon.pri.txt", 10 * 1024 * 1024, wszPath);
            }
        }
        if (bOk && IsWindows11())
        {
            GetWindowsDirectoryW(wszPath, MAX_PATH);
            wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\en-US\\StartTileDataLegacy.dll.mui");
            if (FileExistsW(wszPath))
            {
                bOk = DeleteFileW(wszPath);
                if (bOk)
                {
                    GetWindowsDirectoryW(wszPath, MAX_PATH);
                    wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\en-US");
                    bOk = RemoveDirectoryW(wszPath);
                }
            }
        }
        if (bOk && IsWindows11())
        {
            GetWindowsDirectoryW(wszPath, MAX_PATH);
            wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\pris2");
            CreateDirectoryW(wszPath, NULL);
            wcscat_s(wszPath, MAX_PATH, L"\\Windows.UI.ShellCommon.en-US.pri");
            if (ShouldDownloadOrDelete(bInstall, hInstance, wszPath, "12d7b85cd1b995698b23e5d41fab60ec") && IsConnectedToInternet() == TRUE)
            {
                DownloadFile(L"https://github.com/valinet/ExplorerPatcher/files/8136451/Windows.UI.ShellCommon.en-US.pri.txt", 10 * 1024 * 1024, wszPath);
            }
        }
        if (bOk)
        {
            bOk = GetWindowsDirectoryW(wszPath, MAX_PATH);
        }
        if (bOk && IsWindows11())
        {
            wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\ShellExperienceHost_cw5n1h2txyewy\\dxgi.dll");
            bOk = InstallResource(bInstall, hInstance, IDR_EP_AMD64, wszPath);
        }
        if (bOk)
        {
            GetSystemDirectoryW(wszPath, MAX_PATH);
            WCHAR* pArgs = NULL;
            DWORD dwLen = wcslen(wszPath);
            wcscat_s(wszPath, MAX_PATH - dwLen, L"\\rundll32.exe \"");
            dwLen = wcslen(wszPath);
            pArgs = wszPath + dwLen - 2;
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath + dwLen);
            wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\" _T(PRODUCT_NAME) L".amd64.dll\",ZZGUI");
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
            if (bOk)
            {
                SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
                wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\ep_weather_host.dll");
                bOk = InstallResource(bInstall, hInstance, IDR_EP_WEATHER, wszPath);
            }
            if (bOk)
            {
                PathRemoveFileSpecW(wszPath);
                wcscat_s(wszPath, MAX_PATH, L"\\ep_weather_host_stub.dll");
                bOk = InstallResource(bInstall, hInstance, IDR_EP_WEATHER_STUB, wszPath);
            }
            if (bOk)
            {
                PathRemoveFileSpecW(wszPath);
                wcscat_s(wszPath, MAX_PATH, L"\\WebView2Loader.dll");
                bOk = InstallResource(bInstall, hInstance, IDR_MS_WEBVIEW2_LOADER, wszPath);
            }
        }

        if (bOk)
        {
            if (!bInstall)
            {
                if (bWasShellExt)
                {
                    if (MessageBoxW(
                        NULL,
                        L"Please reboot the computer to complete the uninstall.\n\nDo you want to reboot now?",
                        _T(PRODUCT_NAME),
                        MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION
                    ) == IDYES)
                    {
                        SystemShutdown(TRUE);
                    }
                }
                else
                {
                    MessageBoxW(
                        NULL,
                        L"Uninstall completed. Thank you for using " _T(PRODUCT_NAME) L".",
                        _T(PRODUCT_NAME),
                        MB_ICONASTERISK | MB_OK | MB_DEFBUTTON1
                    );
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
            MessageBoxW(
                NULL,
                L"An error has occurred while servicing this product.\n"
                L"This is most likely caused by one or more of the backup files from a previous update still being in use. "
                L"Unlocking the files should fix this issue.\n\n"
                L"Troubleshooting steps:\n"
                L"* Close and reopen the \"Properties\" dialog if it is currently open.\n"
                L"* Kill and restart all \"explorer.exe\" processes.\n"
                L"* If you have registered this application as a shell extension, then restarting the computer will probably fix this.\n"
                L"* Lastly, reboot the computer and try again.",
                _T(PRODUCT_NAME),
                MB_ICONERROR | MB_OK | MB_DEFBUTTON1
            );
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

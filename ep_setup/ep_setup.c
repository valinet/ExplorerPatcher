#include <Windows.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <stdio.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include "resource.h"
#include "../ExplorerPatcher/utility.h"

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
                    PathRemoveFileSpecW(wszPath);
                    wcscat_s(wszPath, MAX_PATH, L"\\" _T(PRODUCT_NAME) L".amd64.dll");
                    HMODULE hEP = LoadLibraryExW(wszPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
                    if (hEP)
                    {
                        DWORD dwLeftMost = 0;
                        DWORD dwSecondLeft = 0;
                        DWORD dwSecondRight = 0;
                        DWORD dwRightMost = 0;

                        QueryVersionInfo(hEP, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

                        WCHAR wszBuf[20];
                        swprintf_s(wszBuf, 20, L"%d.%d.%d.%d", dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);

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
                dwLastError = RegDeleteTreeW(
                    hKey,
                    0
                );
                RegCloseKey(hKey);
                if (!dwLastError)
                {
                    RegDeleteKeyW(
                        HKEY_LOCAL_MACHINE,
                        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" _T(EP_CLSID) L"_" _T(PRODUCT_NAME)
                    );
                }
                return TRUE;
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
    BOOL bOk = TRUE, bInstall = TRUE, bWasShellExt = FALSE, bIsUpdate = FALSE;

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    if (!IsAppRunningAsAdminMode())
    {
        WCHAR wszPath[MAX_PATH];
        ZeroMemory(wszPath, ARRAYSIZE(wszPath));
        if (GetModuleFileNameW(NULL, wszPath, ARRAYSIZE(wszPath)))
        {
            SHELLEXECUTEINFOW sei;
            ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
            sei.cbSize = sizeof(sei);
            sei.lpVerb = L"runas";
            sei.lpFile = wszPath;
            sei.lpParameters = lpCmdLine;
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
    }

    int argc = 0;
    LPWSTR* wargv = CommandLineToArgvW(
        lpCmdLine,
        &argc
    );

    bIsUpdate = (argc >= 1 && !_wcsicmp(wargv[0], L"/update_silent"));

    WCHAR wszPath[MAX_PATH];
    ZeroMemory(wszPath, MAX_PATH * sizeof(WCHAR));
    if (bOk)
    {
        bOk = GetWindowsDirectoryW(wszPath, MAX_PATH);
    }
    if (bOk)
    {
        wcscat_s(wszPath, MAX_PATH, L"\\dxgi.dll");
        bInstall = !FileExistsW(wszPath) || bIsUpdate;
    }
    if (!bInstall)
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

    SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
    wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH));
    bOk = CreateDirectoryW(wszPath, NULL);
    if (bOk || (!bOk && GetLastError() == ERROR_ALREADY_EXISTS))
    {
        bOk = TRUE;

        BeginExplorerRestart();
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
        if (bOk)
        {
            wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\dxgi.dll");
            bOk = InstallResource(bInstall, hInstance, IDR_EP_AMD64, wszPath);
        }
        if (bOk)
        {
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
            wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\" _T(SETUP_UTILITY_NAME));
            bOk = SetupUninstallEntry(bInstall, wszPath);
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

        GetWindowsDirectoryW(wszPath, MAX_PATH);
        wcscat_s(wszPath, MAX_PATH, L"\\explorer.exe");
        Sleep(1000);
        ShellExecuteW(
            NULL,
            L"open",
            wszPath,
            NULL,
            NULL,
            SW_SHOWNORMAL
        );
    }

	return GetLastError();
}
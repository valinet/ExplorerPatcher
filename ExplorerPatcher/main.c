#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <valinet/hooking/exeinject.h>
#include <Windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#define DEBUG
#undef DEBUG

#define CLASS_NAME TEXT("ExplorerPatcher")
#define APP_NAME TEXT("Windows Explorer")
#define NOP 0x90
#define PATCH_OFFSET 0x8cb33
#define DELAY 5000

HANDLE hProcess = NULL;
HMODULE hMod = NULL;
LPVOID hInjection = NULL;
HWND hWnd = NULL;

DWORD KillAfter(INT64 timeout)
{
    Sleep(timeout);
    TerminateProcess(GetCurrentProcess(), 0);
    return 0;
}

LRESULT CALLBACK WindowProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg)
    {
    case WM_APP_CRASHED:
    {
        TerminateProcess(GetCurrentProcess(), 0);
    }
    }
    return VnWindowProc(
        hWnd,
        uMsg,
        wParam,
        lParam
    );
}

// https://stackoverflow.com/questions/8046097/how-to-check-if-a-process-has-the-administrative-rights
BOOL IsElevated() {
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_QUERY,
        &hToken
    ))
    {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(
            hToken,
            TokenElevation,
            &Elevation,
            sizeof(Elevation),
            &cbSize
        )) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    return fRet;
}

int install_uninstall()
{
    TCHAR buffer[200], szFileName[MAX_PATH], szReadName[MAX_PATH] = { 0 };
    HKEY hKey;
    DWORD dwReadBytes;

    if (IsElevated())
    {
        dwReadBytes = MAX_PATH;
        GetModuleFileName(NULL, szFileName, MAX_PATH);
        if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
            0,
            KEY_READ | KEY_SET_VALUE,
            &hKey
        ) != ERROR_SUCCESS)
        {
            goto error_setup;
        }
        RegGetValue(
            hKey,
            NULL,
            TEXT("Taskman"),
            RRF_RT_REG_SZ,
            NULL,
            szReadName,
            (LPDWORD)(&dwReadBytes)
        );
        if (!wcscmp(szFileName, szReadName))
        {
            if (RegDeleteValue(
                hKey,
                TEXT("Taskman")
            ) != ERROR_SUCCESS)
            {
                goto error_setup;
            }
            MessageBox(
                0,
                TEXT("Uninstall successful."),
                APP_NAME,
                MB_ICONINFORMATION
            );
        }
        else
        {
            if (RegSetValueEx(
                hKey,
                TEXT("Taskman"),
                0,
                REG_SZ,
                (const BYTE*)szFileName, (DWORD)(
#ifdef UNICODE
                    wcslen(szFileName)
#else
                    strlen(szFileName)
#endif
                    * sizeof(TCHAR))
            ) != ERROR_SUCCESS)
            {
                goto error_setup;
            }
            MessageBox(
                0,
                TEXT("Successfully installed Taskman registry key."),
                APP_NAME,
                MB_ICONINFORMATION
            );
        }
        return 1;
    error_setup:
#ifdef UNICODE
        swprintf(buffer, 200,
#else
        sprintf(buffer,
#endif
            TEXT("An error occured when servicing the product (%d)."), GetLastError());
        MessageBox(
            0,
            buffer,
            APP_NAME,
            MB_ICONERROR
        );
        return -1;
    }
    else
    {
        return 0;
    }
}

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    HANDLE hExplorer, hSnapshot;
    DWORD dwExplorerPID = 0, dwOldValue;
    SIZE_T dwNumberOfBytes;
    uintptr_t dwInjectedAddr = 0;
    const char szPayload[6] = { NOP, NOP, NOP, NOP, NOP, NOP };
    PROCESSENTRY32 pe32 = { 0 };
    MODULEENTRY32 me32 = { 0 };
    THREADENTRY32 th32 = { 0 };
    TCHAR szExplorerPath[MAX_PATH];
    FILE* conout;
    TCHAR szLibPath[MAX_PATH];

#ifdef DEBUG
    if (!AllocConsole());
    if (freopen_s(
        &conout,
        "CONOUT$",
        "w",
        stdout
    ));
#endif

    if (install_uninstall())
    {
        return 0;
    }

    while (TRUE)
    {
        pe32.dwSize = sizeof(PROCESSENTRY32);
        hSnapshot = CreateToolhelp32Snapshot(
            TH32CS_SNAPPROCESS,
            0
        );
        if (Process32First(hSnapshot, &pe32) == TRUE)
        {
            do
            {
                if (!wcscmp(pe32.szExeFile, TEXT("explorer.exe")))
                {
                    dwExplorerPID = pe32.th32ProcessID;
                    DebugActiveProcess(dwExplorerPID);
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32) == TRUE);
        }
        CloseHandle(hSnapshot);
        if (!dwExplorerPID)
        {
            break;
        }

        if ((hExplorer = OpenProcess(
            PROCESS_VM_READ |
            PROCESS_VM_WRITE |
            PROCESS_QUERY_INFORMATION |
            PROCESS_VM_OPERATION |
            SYNCHRONIZE,
            FALSE,
            dwExplorerPID
        )) != NULL && GetModuleFileNameEx(
            hExplorer,
            NULL,
            szExplorerPath,
            sizeof(szExplorerPath)
        ))
        {
            CharLower(szExplorerPath);
            me32.dwSize = sizeof(MODULEENTRY32);
            hSnapshot = CreateToolhelp32Snapshot(
                TH32CS_SNAPMODULE,
                dwExplorerPID
            );
            if (Module32First(hSnapshot, &me32) == TRUE)
            {
                do
                {
                    if (!wcscmp(CharLower(me32.szExePath), szExplorerPath))
                    {
                        dwInjectedAddr = (uintptr_t)me32.modBaseAddr + PATCH_OFFSET;
                        break;
                    }
                } while (Module32Next(hSnapshot, &me32) == TRUE);
            }
            CloseHandle(hSnapshot);

            if (dwInjectedAddr)
            {
                VirtualProtectEx(
                    hExplorer,
                    (LPVOID)dwInjectedAddr,
                    sizeof(szPayload),
                    PAGE_EXECUTE_READWRITE,
                    &dwOldValue
                );
                WriteProcessMemory(
                    hExplorer,
                    (LPVOID)dwInjectedAddr,
                    szPayload,
                    sizeof(szPayload),
                    &dwNumberOfBytes
                );
                VirtualProtectEx(
                    hExplorer,
                    (LPVOID)dwInjectedAddr,
                    sizeof(szPayload),
                    dwOldValue,
                    (PDWORD)(&dwNumberOfBytes)
                );
                DebugActiveProcessStop(dwExplorerPID);
                /*
                WaitForSingleObject(
                    hExplorer,
                    INFINITE
                );
                */
            }
            CloseHandle(hExplorer);
        }
        else
        {
            DebugActiveProcessStop(dwExplorerPID);
        }

        GetModuleFileName(
            GetModuleHandle(NULL),
            szLibPath,
            MAX_PATH
        );
        PathRemoveFileSpec(szLibPath);
        lstrcat(
            szLibPath,
            L"\\ExplorerPatcherLibrary.dll"
        );
        Sleep(DELAY);
        CreateThread(
            0,
            0,
            KillAfter,
            5000,
            0,
            0
        );
        return VnInjectAndMonitorProcess(
            szLibPath,
            MAX_PATH,
            "main",
            TEXT("explorer.exe"),
            CLASS_NAME,
            NULL,
            hInstance,
            stdout,
            0,
            WindowProc,
            TRUE,
            0,
            0,
            NULL,
            &hProcess,
            &hMod,
            &hInjection,
            NULL,
            0,
            &hWnd,
            &hWnd
        );
    }
    return 0;
}
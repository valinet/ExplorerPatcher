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

#include <valinet/ini/ini.h>
#include <valinet/pdb/pdb.h>
#define _LIBVALINET_INCLUDE_UNIVERSAL
#include <valinet/universal/toast/toast.h>

#include <distorm.h>
#define BYTES_TO_DISASSEMBLE 1000
#define APPID L"Microsoft.Windows.Explorer"
#define SYMBOLS_RELATIVE_PATH "\\settings.ini"
#define EXPLORER_SB_NAME "explorer"
#define EXPLORER_SB_0 "CTray::Init"
#define EXPLORER_SB_CNT 1
#define EXPLORER_PATCH_OFFSET "Offset"
#define EXPLORER_PATCH_OFFSET_OK "OffsetOK"
#define EXPLORER_PATCH_OFFSET_STRAT "OffsetStrat"
const char* explorer_SN[EXPLORER_SB_CNT] = {
    EXPLORER_SB_0
};
#pragma pack(push, 1)
typedef struct symbols_addr
{
    DWORD explorer_PTRS[EXPLORER_SB_CNT];
} symbols_addr;
#pragma pack(pop)

wchar_t InstallOK[] =
L"<toast displayTimestamp=\"2021-08-29T00:00:00.000Z\" scenario=\"reminder\" "
L"activationType=\"protocol\" launch=\"https://github.com/valinet/ExplorerPatcher\" duration=\"short\">\r\n"
L"	<visual>\r\n"
L"		<binding template=\"ToastGeneric\">\r\n"
L"			<text><![CDATA[Installation was successful]]></text>\r\n"
L"			<text><![CDATA[Explorer will restart several times; the cycle will end when the old taskbar will show, please be patient.]]></text>\r\n"
L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
L"		</binding>\r\n"
L"	</visual>\r\n"
L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
L"</toast>\r\n";

wchar_t UninstallOK[] =
L"<toast displayTimestamp=\"2021-08-29T00:00:00.000Z\" scenario=\"reminder\" "
L"activationType=\"protocol\" launch=\"https://github.com/valinet/ExplorerPatcher\" duration=\"short\">\r\n"
L"	<visual>\r\n"
L"		<binding template=\"ToastGeneric\">\r\n"
L"			<text><![CDATA[Uninstallation was successful]]></text>\r\n"
L"			<text><![CDATA[Thanks for using this application.]]></text>\r\n"
L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
L"		</binding>\r\n"
L"	</visual>\r\n"
L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
L"</toast>\r\n";

typedef LONG NTSTATUS, * PNTSTATUS;
#define STATUS_SUCCESS (0x00000000)

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

BOOL GetOSVersion(PRTL_OSVERSIONINFOW lpRovi)
{
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod != NULL)
    {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)GetProcAddress(
            hMod,
            "RtlGetVersion"
        );
        if (fxPtr != NULL)
        {
            lpRovi->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
            if (STATUS_SUCCESS == fxPtr(lpRovi))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

#define DEBUG
#undef DEBUG

#define CLASS_NAME TEXT("ExplorerPatcher")
#define APP_NAME TEXT("Windows Explorer")
#define NOP 0x90
#define PATCH_OFFSET 0 //0x8cb33
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
            BOOL canShowToast = FALSE;
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
                    if (!wcscmp(pe32.szExeFile, TEXT("explorer.exe")))
                    {
                        canShowToast = TRUE;
                        break;
                    }
                } while (Process32Next(hSnapshot, &pe32) == TRUE);
            }
            CloseHandle(hSnapshot);
            if (!canShowToast)
            {
                MessageBox(
                    0,
                    TEXT("Uninstall successful."),
                    APP_NAME,
                    MB_ICONINFORMATION
                );
            }
            else
            {
                __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml2 = NULL;
                HRESULT hr = String2IXMLDocument(
                    UninstallOK,
                    wcslen(UninstallOK),
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
            BOOL canShowToast = FALSE;
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
                    if (!wcscmp(pe32.szExeFile, TEXT("explorer.exe")))
                    {
                        canShowToast = TRUE;
                        break;
                    }
                } while (Process32Next(hSnapshot, &pe32) == TRUE);
            }
            CloseHandle(hSnapshot);
            if (!canShowToast)
            {
                MessageBox(
                    0,
                    TEXT("Installation was successful. Please sign out or restart")
                    TEXT("the computer for the changes to take effect."),
                    APP_NAME,
                    MB_ICONINFORMATION
                );
            }
            else
            {
                __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml2 = NULL;
                HRESULT hr = String2IXMLDocument(
                    InstallOK,
                    wcslen(InstallOK),
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
                Sleep(10000);
            }
            hSnapshot = CreateToolhelp32Snapshot(
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
                    }
                } while (Process32Next(hSnapshot, &pe32) == TRUE);
            }
            CloseHandle(hSnapshot);
            TerminateProcess(
                OpenProcess(
                    PROCESS_TERMINATE, 
                    FALSE, 
                    GetCurrentProcessId()
                ), 
                0
            );
        }
        RegCloseKey(hKey);
        return 1;
    error_setup:
        RegCloseKey(hKey);
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
    const char szPayload0[6] = { NOP, NOP, NOP, NOP, NOP, NOP };
    const char szPayload1[2] = { NOP, 0xE9 };
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
                if (pe32.th32ProcessID != GetCurrentProcessId() && 
                    !wcscmp(pe32.szExeFile, TEXT("ExplorerPatcher.exe")))
                {
                    HANDLE hOwn = OpenProcess(
                        SYNCHRONIZE,
                        FALSE,
                        pe32.th32ProcessID
                    );
                    WaitForSingleObject(
                        hOwn,
                        INFINITE
                    );
                }
            } while (Process32Next(hSnapshot, &pe32) == TRUE);
        }
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

        DWORD dwRet = 0;
        char szSettingsPath[MAX_PATH];
        ZeroMemory(
            szSettingsPath,
            (MAX_PATH) * sizeof(char)
        );
        TCHAR wszSettingsPath[MAX_PATH];
        ZeroMemory(
            wszSettingsPath,
            (MAX_PATH) * sizeof(TCHAR)
        );
        GetModuleFileNameA(
            hInstance,
            szSettingsPath,
            MAX_PATH
        );
        PathRemoveFileSpecA(szSettingsPath);
        strcat_s(
            szSettingsPath,
            MAX_PATH,
            SYMBOLS_RELATIVE_PATH
        );
        mbstowcs_s(
            &dwRet,
            wszSettingsPath,
            MAX_PATH,
            szSettingsPath,
            MAX_PATH
        );

        symbols_addr symbols_PTRS;
        ZeroMemory(
            &symbols_PTRS,
            sizeof(symbols_addr)
        );
        symbols_PTRS.explorer_PTRS[0] = VnGetUInt(
            TEXT(EXPLORER_SB_NAME),
            TEXT(EXPLORER_SB_0),
            0,
            wszSettingsPath
        );

        BOOL bNeedToDownload = FALSE;
        for (UINT i = 0; i < sizeof(symbols_addr) / sizeof(DWORD); ++i)
        {
            if (!((DWORD*)&symbols_PTRS)[i])
            {
                bNeedToDownload = TRUE;
            }
        }
        // https://stackoverflow.com/questions/36543301/detecting-windows-10-version/36543774#36543774
        RTL_OSVERSIONINFOW rovi;
        if (!GetOSVersion(&rovi))
        {
            DebugActiveProcessStop(dwExplorerPID);
            return 1;
        }
        // https://stackoverflow.com/questions/47926094/detecting-windows-10-os-build-minor-version
        DWORD32 ubr = 0, ubr_size = sizeof(DWORD32);
        HKEY hKey;
        LONG lRes = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            wcschr(
                wcschr(
                    wcschr(
                        UNIFIEDBUILDREVISION_KEY,
                        '\\'
                    ) + 1,
                    '\\'
                ) + 1,
                '\\'
            ) + 1,
            0,
            KEY_READ,
            &hKey
        );
        if (lRes == ERROR_SUCCESS)
        {
            RegQueryValueExW(
                hKey,
                UNIFIEDBUILDREVISION_VALUE,
                0,
                NULL,
                &ubr,
                &ubr_size
            );
        }
        TCHAR szReportedVersion[MAX_PATH];
        ZeroMemory(
            szReportedVersion,
            (MAX_PATH) * sizeof(TCHAR)
        );
        TCHAR szStoredVersion[MAX_PATH];
        ZeroMemory(
            szStoredVersion,
            (MAX_PATH) * sizeof(TCHAR)
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

        if (bNeedToDownload)
        {
            DeleteFile(wszSettingsPath);

            char explorer_sb_dll[MAX_PATH];
            ZeroMemory(
                explorer_sb_dll,
                (MAX_PATH) * sizeof(char)
            );
            GetWindowsDirectoryA(
                explorer_sb_dll,
                MAX_PATH
            );
            strcat_s(
                explorer_sb_dll,
                MAX_PATH,
                "\\"
            );
            strcat_s(
                explorer_sb_dll,
                MAX_PATH,
                EXPLORER_SB_NAME
            );
            strcat_s(
                explorer_sb_dll,
                MAX_PATH,
                ".exe"
            );
            if (VnDownloadSymbols(
                NULL,
                explorer_sb_dll,
                szSettingsPath,
                MAX_PATH
            ))
            {
                DebugActiveProcessStop(dwExplorerPID);
                return 2;
            }
            if (VnGetSymbols(
                szSettingsPath,
                symbols_PTRS.explorer_PTRS,
                explorer_SN,
                EXPLORER_SB_CNT
            ))
            {
                DebugActiveProcessStop(dwExplorerPID);
                return 3;
            }
            VnWriteUInt(
                TEXT(EXPLORER_SB_NAME),
                TEXT(EXPLORER_SB_0),
                symbols_PTRS.explorer_PTRS[0],
                wszSettingsPath
            );

            VnWriteString(
                TEXT("OS"),
                TEXT("Build"),
                szReportedVersion,
                wszSettingsPath
            );
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

            uintptr_t start = VnGetUInt(
                TEXT(EXPLORER_SB_NAME),
                TEXT(EXPLORER_PATCH_OFFSET),
                0,
                wszSettingsPath
            );
            uintptr_t ok = VnGetUInt(
                TEXT(EXPLORER_SB_NAME),
                TEXT(EXPLORER_PATCH_OFFSET_OK),
                0,
                wszSettingsPath
            );
            uintptr_t strat = VnGetUInt(
                TEXT(EXPLORER_SB_NAME),
                TEXT(EXPLORER_PATCH_OFFSET_STRAT),
                0,
                wszSettingsPath
            );
            uintptr_t end = 0;
            if (!ok)
            {
                uintptr_t CTray_Init = dwInjectedAddr + (uintptr_t)symbols_PTRS.explorer_PTRS[0];
                char m[BYTES_TO_DISASSEMBLE];
                VirtualProtectEx(
                    hExplorer,
                    (LPVOID)CTray_Init,
                    BYTES_TO_DISASSEMBLE,
                    PAGE_EXECUTE_READWRITE,
                    &dwOldValue
                );
                ReadProcessMemory(
                    hExplorer,
                    (LPVOID)CTray_Init,
                    m,
                    BYTES_TO_DISASSEMBLE,
                    &dwNumberOfBytes
                );
                VirtualProtectEx(
                    hExplorer,
                    (LPVOID)CTray_Init,
                    BYTES_TO_DISASSEMBLE,
                    dwOldValue,
                    (PDWORD)(&dwNumberOfBytes)
                );
                _DecodedInst decodedInstructions[1000];
                UINT decodedInstructionsCount = 0;
                _DecodeResult res = distorm_decode(
                    0,
                    (const unsigned char*)m,
                    BYTES_TO_DISASSEMBLE,
                    Decode64Bits,
                    decodedInstructions,
                    1000,
                    &decodedInstructionsCount
                );
                BOOL found = FALSE;
                for (UINT i = 0; i < decodedInstructionsCount; ++i)
                {
                    if ((!strcmp(decodedInstructions[i].mnemonic.p, "JZ") ||
                        !strcmp(decodedInstructions[i].mnemonic.p, "JNZ")) &&
                        decodedInstructions[i].offset > start)
                    {
                        found = TRUE;
                        start = decodedInstructions[i].offset;
                        if (strat == 0)
                        {
                            memcpy(
                                m + start, 
                                szPayload0, 
                                sizeof(szPayload0)
                            );
                        }
                        else if (strat == 1)
                        {
                            memcpy(
                                m + start, 
                                szPayload1, 
                                sizeof(szPayload1)
                            );
                        }
                        break;
                    }
                }
                if (!found)
                {
                    start = 0;
                    strat++;
                }
#ifdef DEBUG
                res = distorm_decode(
                    0,
                    (const unsigned char*)m,
                    BYTES_TO_DISASSEMBLE,
                    Decode64Bits,
                    decodedInstructions,
                    1000,
                    &decodedInstructionsCount
                );
                for (UINT i = 0; i < decodedInstructionsCount; ++i)
                {
                    printf(
                        "0x%p\t%s\t%s\n",
                        decodedInstructions[i].offset,
                        decodedInstructions[i].mnemonic.p,
                        decodedInstructions[i].instructionHex.p
                    );
                }
#endif
                VirtualProtectEx(
                    hExplorer,
                    (LPVOID)CTray_Init,
                    BYTES_TO_DISASSEMBLE,
                    PAGE_EXECUTE_READWRITE,
                    &dwOldValue
                );
                WriteProcessMemory(
                    hExplorer,
                    (LPVOID)CTray_Init,
                    m,
                    BYTES_TO_DISASSEMBLE,
                    &dwNumberOfBytes
                );
                VirtualProtectEx(
                    hExplorer,
                    (LPVOID)CTray_Init,
                    BYTES_TO_DISASSEMBLE,
                    dwOldValue,
                    (PDWORD)(&dwNumberOfBytes)
                );
                DebugActiveProcessStop(dwExplorerPID);
                Sleep(3000);
                HWND hWnd = FindWindowEx(
                    NULL,
                    NULL,
                    L"Shell_TrayWnd",
                    NULL
                );
                if (hWnd)
                {
                    hWnd = FindWindowEx(
                        hWnd,
                        NULL,
                        L"Start",
                        NULL
                    );
                    if (hWnd)
                    {
                        if (IsWindowVisible(hWnd))
                        {
                            ok = 1;
                        }
                    }
                }
                VnWriteUInt(
                    TEXT(EXPLORER_SB_NAME),
                    TEXT(EXPLORER_PATCH_OFFSET),
                    start,
                    wszSettingsPath
                );
                VnWriteUInt(
                    TEXT(EXPLORER_SB_NAME),
                    TEXT(EXPLORER_PATCH_OFFSET_OK),
                    ok,
                    wszSettingsPath
                );
                VnWriteUInt(
                    TEXT(EXPLORER_SB_NAME),
                    TEXT(EXPLORER_PATCH_OFFSET_STRAT),
                    strat,
                    wszSettingsPath
                );
#ifdef DEBUG
                printf("start: %d ok %d\n", start, ok);
#endif
                if (!ok)
                {
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
                                HANDLE hExpl = OpenProcess(
                                    PROCESS_TERMINATE,
                                    FALSE,
                                    pe32.th32ProcessID
                                );
                                TerminateProcess(hExpl, 1);
                                CloseHandle(hExpl);
                            }
                        } while (Process32Next(hSnapshot, &pe32) == TRUE);
                    }
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
                                TerminateProcess(hSihost, 1);
                                CloseHandle(hSihost);
                                Sleep(500);
                                STARTUPINFO info = {sizeof(info)};
                                PROCESS_INFORMATION processInfo;
                                BOOL b = CreateProcess(
                                    NULL,
                                    pe32.szExeFile,
                                    NULL,
                                    NULL,
                                    TRUE,
                                    CREATE_UNICODE_ENVIRONMENT,
                                    NULL,
                                    NULL,
                                    &info,
                                    &processInfo
                                );
                                break;
                            }
                        } while (Process32Next(hSnapshot, &pe32) == TRUE);
                    }
                    CloseHandle(hSnapshot);
                    TerminateProcess(
                        OpenProcess(
                            PROCESS_TERMINATE, 
                            FALSE, 
                            GetCurrentProcessId()
                        ), 
                        0
                    );
                }
            }
            else
            {
                dwInjectedAddr += (uintptr_t)symbols_PTRS.explorer_PTRS[0] + start;
                if (strat == 0)
                {
                    VirtualProtectEx(
                        hExplorer,
                        (LPVOID)dwInjectedAddr,
                        sizeof(szPayload0),
                        PAGE_EXECUTE_READWRITE,
                        &dwOldValue
                    );
                    WriteProcessMemory(
                        hExplorer,
                        (LPVOID)dwInjectedAddr,
                        szPayload0,
                        sizeof(szPayload0),
                        &dwNumberOfBytes
                    );
                    VirtualProtectEx(
                        hExplorer,
                        (LPVOID)dwInjectedAddr,
                        sizeof(szPayload0),
                        dwOldValue,
                        (PDWORD)(&dwNumberOfBytes)
                    );
                }
                else if (strat == 1)
                {
                    VirtualProtectEx(
                        hExplorer,
                        (LPVOID)dwInjectedAddr,
                        sizeof(szPayload1),
                        PAGE_EXECUTE_READWRITE,
                        &dwOldValue
                    );
                    WriteProcessMemory(
                        hExplorer,
                        (LPVOID)dwInjectedAddr,
                        szPayload1,
                        sizeof(szPayload1),
                        &dwNumberOfBytes
                    );
                    VirtualProtectEx(
                        hExplorer,
                        (LPVOID)dwInjectedAddr,
                        sizeof(szPayload1),
                        dwOldValue,
                        (PDWORD)(&dwNumberOfBytes)
                    );
                }
                DebugActiveProcessStop(dwExplorerPID);
                // WaitForSingleObject(
                //     hExplorer,
                //     INFINITE
                // );
            }
            CloseHandle(hExplorer);
        }
        else
        {
            DebugActiveProcessStop(dwExplorerPID);
            TerminateProcess(
                OpenProcess(
                    PROCESS_TERMINATE,
                    FALSE,
                    GetCurrentProcessId()
                ),
                0
            );
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
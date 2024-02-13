#include "utility.h"
#include <Wininet.h>
#pragma comment(lib, "Wininet.lib")
#include <TlHelp32.h>

RTL_OSVERSIONINFOW global_rovi;
DWORD32 global_ubr;

void printf_guid(GUID guid) 
{
    printf("Guid = {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\n",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

LRESULT CALLBACK BalloonWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    if (msg == WM_CREATE)
    {
        LPCREATESTRUCT lpCs = lParam;

        NOTIFYICONDATA ni = { 0 };
        ni.cbSize = sizeof(ni);
        ni.hWnd = hWnd;
        ni.uID = 1;
        ni.uFlags = NIF_INFO;
        ni.dwInfoFlags = NIIF_INFO;
        ni.uTimeout = 2000;
        _tcscpy_s(ni.szInfo, _countof(ni.szInfo), lpCs->lpCreateParams);
        _tcscpy_s(ni.szInfoTitle, _countof(ni.szInfoTitle), _T("ExplorerPatcher"));

        Shell_NotifyIcon(NIM_ADD, &ni);

        free(lpCs->lpCreateParams);

        exit(0);
    }
    else
    {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

__declspec(dllexport) int CALLBACK ZZTestBalloon(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    TCHAR* lpwszCmdLine = calloc((strlen(lpszCmdLine) + 1), sizeof(TCHAR));
    if (!lpwszCmdLine) exit(0);
    size_t numChConv = 0;
    mbstowcs_s(&numChConv, lpwszCmdLine, strlen(lpszCmdLine) + 1, lpszCmdLine, strlen(lpszCmdLine) + 1);

    WNDCLASSEX wc;
    HWND hwnd;
    MSG msg;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = BalloonWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"ExplorerPatcherBalloon";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        return 0;
    }

    hwnd = CreateWindowEx(0, L"ExplorerPatcherBalloon", L"",
        0, 0, 0, 0, 0,
        HWND_MESSAGE, NULL, hInstance, lpwszCmdLine);

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

#ifdef _DEBUG
const wchar_t TestToastXML[] =
L"<toast scenario=\"reminder\" "
L"activationType=\"protocol\" launch=\"https://github.com/valinet/ExplorerPatcher\" duration=\"%s\">\r\n"
L"	<visual>\r\n"
L"		<binding template=\"ToastGeneric\">\r\n"
L"			<text><![CDATA[%s]]></text>\r\n"
L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n"
L"		</binding>\r\n"
L"	</visual>\r\n"
L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n"
L"</toast>\r\n";
__declspec(dllexport) int CALLBACK ZZTestToast(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    TCHAR* lpwszCmdLine = calloc((strlen(lpszCmdLine) + 1), sizeof(TCHAR));
    if (!lpwszCmdLine) exit(0);
    size_t numChConv = 0;
    mbstowcs_s(&numChConv, lpwszCmdLine, strlen(lpszCmdLine) + 1, lpszCmdLine, strlen(lpszCmdLine) + 1);
    TCHAR* buffer = calloc((sizeof(TestToastXML) / sizeof(wchar_t) + strlen(lpszCmdLine) + 10), sizeof(TCHAR));
    if (buffer)
    {
        wsprintf(
            buffer,
            TestToastXML,
            L"short",
            lpwszCmdLine
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
        free(buffer);
    }
    free(lpwszCmdLine);
    return 0;
}
#endif

__declspec(dllexport) int CALLBACK ZZLaunchExplorer(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    Sleep(100);
    TCHAR wszExplorerPath[MAX_PATH + 1];
    GetWindowsDirectory(wszExplorerPath, MAX_PATH + 1);
    wcscat_s(wszExplorerPath, MAX_PATH + 1, L"\\explorer.exe");
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
        0
    );
    return 0;
}

__declspec(dllexport) int CALLBACK ZZLaunchExplorerDelayed(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    Sleep(2000);
    ZZLaunchExplorer(hWnd, hInstance, lpszCmdLine, nCmdShow);
    return 0;
}

__declspec(dllexport) int CALLBACK ZZRestartExplorer(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    BeginExplorerRestart(NULL);
    FinishExplorerRestart();
    return 0;
}

void* ReadFromFile(wchar_t* wszFileName, DWORD* dwSize)
{
    void* ok = NULL;
    HANDLE hImage = CreateFileW(wszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hImage)
    {
        LARGE_INTEGER dwFileSize;
        GetFileSizeEx(hImage, &dwFileSize);
        if (dwFileSize.LowPart)
        {
            void* pImage = malloc(dwFileSize.LowPart);
            if (pImage)
            {
                DWORD dwNumberOfBytesRead = 0;
                ReadFile(hImage, pImage, dwFileSize.LowPart, &dwNumberOfBytesRead, NULL);
                if (dwFileSize.LowPart == dwNumberOfBytesRead)
                {
                    ok = pImage;
                    *dwSize = dwNumberOfBytesRead;
                }
            }
        }
        CloseHandle(hImage);
    }
    return ok;
}

int ComputeFileHash(LPCWSTR filename, LPSTR hash, DWORD dwHash)
{
    DWORD dwStatus = 0;
    BOOL bResult = FALSE;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    HANDLE hFile = NULL;
    BYTE* rgbFile;
    DWORD cbRead = 0;
    BYTE rgbHash[16];
    DWORD cbHash = 0;
    WCHAR rgbDigits[] = L"0123456789abcdef";
    // Logic to check usage goes here.

    hFile = CreateFile(filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwStatus = GetLastError();
        return dwStatus;
    }

    LARGE_INTEGER dwFileSize;
    GetFileSizeEx(hFile, &dwFileSize);
    if (!dwFileSize.LowPart)
    {
        dwStatus = GetLastError();
        CloseHandle(hFile);
        return dwStatus;
    }

    rgbFile = malloc(dwFileSize.LowPart);
    if (!rgbFile)
    {
        dwStatus = E_OUTOFMEMORY;
        CloseHandle(hFile);
        return dwStatus;
    }

    // Get handle to the crypto provider
    if (!CryptAcquireContext(&hProv,
        NULL,
        NULL,
        PROV_RSA_FULL,
        CRYPT_VERIFYCONTEXT))
    {
        dwStatus = GetLastError();
        CloseHandle(hFile);
        return dwStatus;
    }

    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
        dwStatus = GetLastError();
        CloseHandle(hFile);
        CryptReleaseContext(hProv, 0);
        return dwStatus;
    }

    while (bResult = ReadFile(hFile, rgbFile, dwFileSize.LowPart, &cbRead, NULL))
    {
        if (0 == cbRead)
        {
            break;
        }

        if (!CryptHashData(hHash, rgbFile, cbRead, 0))
        {
            dwStatus = GetLastError();
            CryptReleaseContext(hProv, 0);
            CryptDestroyHash(hHash);
            CloseHandle(hFile);
            return dwStatus;
        }
    }

    if (!bResult)
    {
        dwStatus = GetLastError();
        CryptReleaseContext(hProv, 0);
        CryptDestroyHash(hHash);
        CloseHandle(hFile);
        return dwStatus;
    }

    cbHash = 16;
    if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
    {
        for (DWORD i = 0; i < cbHash; i++)
        {
            sprintf_s(hash + (i * 2), 3, "%c%c", rgbDigits[rgbHash[i] >> 4], rgbDigits[rgbHash[i] & 0xf]);
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);
    free(rgbFile);

    return dwStatus;
}

int ComputeFileHash2(HMODULE hModule, LPCWSTR filename, LPSTR hash, DWORD dwHash)
{
    if (dwHash < 33)
    {
        return ERROR_BUFFER_OVERFLOW;
    }
    if (!hModule)
    {
        return ERROR_INVALID_ADDRESS;
    }

    DWORD dwLeftMost = 0;
    DWORD dwSecondLeft = 0;
    DWORD dwSecondRight = 0;
    DWORD dwRightMost = 0;
    QueryVersionInfo(hModule, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

    sprintf_s(hash, 33, "%d.%d.%d.%d.", dwLeftMost == 22621 ? 22622 : dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);

    char real_hash[33];
    ComputeFileHash(filename, real_hash, 33);
    strncpy_s(hash + strlen(hash), dwHash - strlen(hash), real_hash, 32 - strlen(hash));
    if (dwLeftMost == 22622) *(strchr(strchr(strchr(strchr(hash, '.') + 1, '.') + 1, '.') + 1, '.') + 1) = '!';
    hash[33] = 0;

    return ERROR_SUCCESS;
}

void LaunchPropertiesGUI(HMODULE hModule)
{
    //CreateThread(0, 0, ZZGUI, 0, 0, 0);
    wchar_t wszPath[MAX_PATH * 2];
    ZeroMemory(wszPath, ARRAYSIZE(wszPath));
    wszPath[0] = '\"';
    GetSystemDirectoryW(wszPath + 1, MAX_PATH);
    wcscat_s(wszPath, ARRAYSIZE(wszPath), L"\\rundll32.exe\" \"");
    SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath + wcslen(wszPath));
    wcscat_s(wszPath, ARRAYSIZE(wszPath), _T(APP_RELATIVE_PATH) L"\\ep_gui.dll");
    wcscat_s(wszPath, ARRAYSIZE(wszPath), L"\",ZZGUI");

    wprintf(L"Launching : %s\n", wszPath);
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    if (CreateProcessW(
        NULL,
        wszPath,
        NULL,
        NULL,
        FALSE,
        CREATE_UNICODE_ENVIRONMENT,
        NULL,
        NULL,
        &si,
        &pi
    ))
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}


BOOL SystemShutdown(BOOL reboot)
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    // Get a token for this process. 

    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return(FALSE);

    // Get the LUID for the shutdown privilege. 

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
        &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;  // one privilege to set    
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get the shutdown privilege for this process. 

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
        (PTOKEN_PRIVILEGES)NULL, 0);

    if (GetLastError() != ERROR_SUCCESS)
        return FALSE;

    // Shut down the system and force all applications to close. 

    if (!ExitWindowsEx((reboot ? EWX_REBOOT : EWX_SHUTDOWN) | EWX_FORCE,
        SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
        SHTDN_REASON_MINOR_UPGRADE |
        SHTDN_REASON_FLAG_PLANNED))
        return FALSE;

    //shutdown was successful
    return TRUE;
}

HRESULT FindDesktopFolderView(REFIID riid, void** ppv)
{
    HRESULT hr = E_FAIL;
    IShellWindows* spShellWindows = NULL;
    hr = CoCreateInstance(
        &CLSID_ShellWindows,
        NULL,
        CLSCTX_ALL,
        &IID_IShellWindows,
        &spShellWindows
    );
    if (spShellWindows)
    {
        VARIANT vtEmpty;
        ZeroMemory(&vtEmpty, sizeof(VARIANT));
        VARIANT vtLoc;
        ZeroMemory(&vtLoc, sizeof(VARIANT));
        vtLoc.vt = VT_INT;
        vtLoc.intVal = CSIDL_DESKTOP;
        long lhwnd = 0;
        IDispatch* spdisp = NULL;
        hr = spShellWindows->lpVtbl->FindWindowSW(
            spShellWindows,
            &vtLoc,
            &vtEmpty,
            SWC_DESKTOP,
            &lhwnd,
            SWFO_NEEDDISPATCH,
            &spdisp
        );
        if (spdisp)
        {
            IServiceProvider* spdisp2 = NULL;
            hr = spdisp->lpVtbl->QueryInterface(spdisp, &IID_IServiceProvider, &spdisp2);
            if (spdisp2)
            {
                IShellBrowser* spBrowser = NULL;
                hr = spdisp2->lpVtbl->QueryService(spdisp2, &SID_STopLevelBrowser, &IID_IShellBrowser, &spBrowser);
                if (spBrowser)
                {
                    IShellView* spView = NULL;
                    hr = spBrowser->lpVtbl->QueryActiveShellView(spBrowser, &spView);
                    if (spView)
                    {
                        hr = spView->lpVtbl->QueryInterface(spView, riid, ppv);
                        spView->lpVtbl->Release(spView);
                    }
                    spBrowser->lpVtbl->Release(spBrowser);
                }
                spdisp2->lpVtbl->Release(spdisp2);
            }
            spdisp->lpVtbl->Release(spdisp);
        }
        spShellWindows->lpVtbl->Release(spShellWindows);
    }
    return hr;
}

HRESULT GetDesktopAutomationObject(REFIID riid, void** ppv)
{
    HRESULT hr = E_FAIL;
    IShellView* spsv = NULL;
    hr = FindDesktopFolderView(&IID_IShellView, &spsv);
    if (spsv)
    {
        IDispatch* spdispView = NULL;
        hr = spsv->lpVtbl->GetItemObject(spsv, SVGIO_BACKGROUND, &IID_IDispatch, &spdispView);
        if (spdispView)
        {
            hr = spdispView->lpVtbl->QueryInterface(spdispView, riid, ppv);
            spdispView->lpVtbl->Release(spdispView);
        }
        spsv->lpVtbl->Release(spsv);
    }
    return hr;
}

HRESULT ShellExecuteFromExplorer(
    PCWSTR pszFile,
    PCWSTR pszParameters,
    PCWSTR pszDirectory,
    PCWSTR pszOperation,
    int nShowCmd
)
{
    HRESULT hr = E_FAIL;
    IShellFolderViewDual* spFolderView = NULL;
    GetDesktopAutomationObject(&IID_IShellFolderViewDual, &spFolderView);
    if (spFolderView)
    {
        IDispatch* spdispShell = NULL;
        spFolderView->lpVtbl->get_Application(spFolderView, &spdispShell);
        if (spdispShell)
        {
            IShellDispatch2* spdispShell2 = NULL;
            spdispShell->lpVtbl->QueryInterface(spdispShell, &IID_IShellDispatch2, &spdispShell2);
            if (spdispShell2)
            {
                BSTR a_pszFile = pszFile ? SysAllocString(pszFile): SysAllocString(L"");
                VARIANT a_pszParameters, a_pszDirectory, a_pszOperation, a_nShowCmd;
                ZeroMemory(&a_pszParameters, sizeof(VARIANT));
                ZeroMemory(&a_pszDirectory, sizeof(VARIANT));
                ZeroMemory(&a_pszOperation, sizeof(VARIANT));
                ZeroMemory(&a_nShowCmd, sizeof(VARIANT));
                a_pszParameters.vt = VT_BSTR;
                a_pszParameters.bstrVal = pszParameters ? SysAllocString(pszParameters) : SysAllocString(L"");
                a_pszDirectory.vt = VT_BSTR;
                a_pszDirectory.bstrVal = pszDirectory ? SysAllocString(pszDirectory) : SysAllocString(L"");
                a_pszOperation.vt = VT_BSTR;
                a_pszOperation.bstrVal = pszOperation ? SysAllocString(pszOperation) : SysAllocString(L"");
                a_nShowCmd.vt = VT_INT;
                a_nShowCmd.intVal = nShowCmd;
                hr = spdispShell2->lpVtbl->ShellExecuteW(spdispShell2, a_pszFile, a_pszParameters, a_pszDirectory, a_pszOperation, a_nShowCmd);
                if (a_pszOperation.bstrVal)
                {
                    SysFreeString(a_pszOperation.bstrVal);
                }
                if (a_pszDirectory.bstrVal)
                {
                    SysFreeString(a_pszDirectory.bstrVal);
                }
                if (a_pszParameters.bstrVal)
                {
                    SysFreeString(a_pszParameters.bstrVal);
                }
                if (a_pszFile)
                {
                    SysFreeString(a_pszFile);
                }
                spdispShell2->lpVtbl->Release(spdispShell2);
            }
            spdispShell->lpVtbl->Release(spdispShell);
        }
        spFolderView->lpVtbl->Release(spFolderView);
    }
    return hr;
}

void ToggleTaskbarAutohide()
{
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    if (SHAppBarMessage(ABM_GETSTATE, &abd) == ABS_AUTOHIDE)
    {
        abd.lParam = 0;
        SHAppBarMessage(ABM_SETSTATE, &abd);
    }
    else
    {
        abd.lParam = ABS_AUTOHIDE;
        SHAppBarMessage(ABM_SETSTATE, &abd);
    }
}

LSTATUS RegisterDWMService(DWORD dwDesiredState, DWORD dwOverride)
{
    WCHAR wszPath[MAX_PATH];
    GetSystemDirectoryW(wszPath, MAX_PATH);
    wcscat_s(wszPath, MAX_PATH, L"\\cmd.exe");

    WCHAR wszSCPath[MAX_PATH];
    GetSystemDirectoryW(wszSCPath, MAX_PATH);
    wcscat_s(wszSCPath, MAX_PATH, L"\\sc.exe");

    WCHAR wszRundll32[MAX_PATH];
    SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszRundll32);
    wcscat_s(wszRundll32, MAX_PATH, _T(APP_RELATIVE_PATH));
    wcscat_s(wszRundll32, MAX_PATH, L"\\ep_dwm.exe");

    WCHAR wszEP[MAX_PATH];
    GetWindowsDirectoryW(wszEP, MAX_PATH);
    wcscat_s(wszEP, MAX_PATH, L"\\dxgi.dll");

    WCHAR wszTaskkill[MAX_PATH];
    GetSystemDirectoryW(wszTaskkill, MAX_PATH);
    wcscat_s(wszTaskkill, MAX_PATH, L"\\taskkill.exe");

    WCHAR wszArgumentsRegister[MAX_PATH * 10];
    swprintf_s(
        wszArgumentsRegister,
        MAX_PATH * 10,
        L"/c \""
        L"\"%s\" create " _T(EP_DWM_SERVICENAME) L" binPath= \"\\\"%s\\\" %s\" DisplayName= \"ExplorerPatcher Desktop Window Manager Service\" start= auto & "
        L"\"%s\" description " _T(EP_DWM_SERVICENAME) L" \"Service for managing aspects related to the Desktop Window Manager.\" & "
        L"\"%s\" %s " _T(EP_DWM_SERVICENAME)
        L"\"",
        wszSCPath,
        wszRundll32,
        _T(EP_DWM_SERVICENAME) L" " _T(EP_DWM_EVENTNAME),
        wszSCPath,
        wszSCPath,
        (!dwOverride || dwOverride == 3) ? L"start" : L"query"
    );
    WCHAR wszArgumentsUnRegister[MAX_PATH * 10];
    swprintf_s(
        wszArgumentsUnRegister,
        MAX_PATH * 10,
        L"/c \""
        L"\"%s\" stop " _T(EP_DWM_SERVICENAME) L" & "
        L"\"%s\" delete " _T(EP_DWM_SERVICENAME) L" & "
        L"\"",
        wszSCPath,
        wszSCPath
    );
    wprintf(L"%s\n", wszArgumentsRegister);

    BOOL bAreRoundedCornersDisabled = FALSE;
    if (dwOverride)
    {
        bAreRoundedCornersDisabled = !(dwOverride - 1);
    }
    else
    {
        HANDLE h_exists = CreateEventW(NULL, FALSE, FALSE, _T(EP_DWM_EVENTNAME));
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
        if ((bAreRoundedCornersDisabled && dwDesiredState) || (!bAreRoundedCornersDisabled && !dwDesiredState))
        {
            return FALSE;
        }
    }
    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = L"runas";
    ShExecInfo.lpFile = wszPath;
    ShExecInfo.lpParameters = !bAreRoundedCornersDisabled ? wszArgumentsRegister : wszArgumentsUnRegister;
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
    return TRUE;
}

char* StrReplaceAllA(const char* s, const char* oldW, const char* newW, int* dwNewSize)
{
    char* result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);

    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], oldW) == &s[i]) {
            cnt++;
            i += oldWlen - 1;
        }
    }
    result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1);
    i = 0;
    while (*s) {
        if (strstr(s, oldW) == s) {
            strcpy_s(&result[i], strlen(newW) + 1, newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }

    result[i] = '\0';
    if (dwNewSize) *dwNewSize = i;
    return result;
}

WCHAR* StrReplaceAllW(const WCHAR* s, const WCHAR* oldW, const WCHAR* newW, int* dwNewSize)
{
    WCHAR* result;
    int i, cnt = 0;
    int newWlen = wcslen(newW);
    int oldWlen = wcslen(oldW);

    for (i = 0; s[i] != L'\0'; i++) {
        if (wcsstr(&s[i], oldW) == &s[i]) {
            cnt++;
            i += oldWlen - 1;
        }
    }
    result = (WCHAR*)malloc((i + cnt * (newWlen - oldWlen) + 1) * sizeof(WCHAR));
    i = 0;
    while (*s) {
        if (wcsstr(s, oldW) == s) {
            wcscpy_s(&result[i], newWlen + 1, newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }
    result[i] = L'\0';
    if (dwNewSize) *dwNewSize = i;
    return result;
}

HWND InputBox_HWND;

HRESULT getEngineGuid(LPCTSTR extension, GUID* guidBuffer)
{
    wchar_t   buffer[100];
    HKEY      hk;
    DWORD     size;
    HKEY      subKey;
    DWORD     type;

    // See if this file extension is associated
    // with an ActiveX script engine
    if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, extension, 0,
        KEY_QUERY_VALUE | KEY_READ, &hk))
    {
        type = REG_SZ;
        size = sizeof(buffer);
        size = RegQueryValueEx(hk, 0, 0, &type,
            (LPBYTE)&buffer[0], &size);
        RegCloseKey(hk);
        if (!size)
        {
            // The engine set an association.
            // We got the Language string in buffer[]. Now
            // we can use it to look up the engine's GUID

            // Open HKEY_CLASSES_ROOT\{LanguageName}
        again:   size = sizeof(buffer);
            if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, (LPCTSTR)&buffer[0], 0,
                KEY_QUERY_VALUE | KEY_READ, &hk))
            {
                // Read the GUID (in string format)
                // into buffer[] by querying the value of CLSID
                if (!RegOpenKeyEx(hk, L"CLSID", 0,
                    KEY_QUERY_VALUE | KEY_READ, &subKey))
                {
                    size = RegQueryValueExW(subKey, 0, 0, &type,
                        (LPBYTE)&buffer[0], &size);
                    RegCloseKey(subKey);
                }
                else if (extension)
                {
                    // If an error, see if we have a "ScriptEngine"
                    // key under here that contains
                    // the real language name
                    if (!RegOpenKeyEx(hk, L"ScriptEngine", 0,
                        KEY_QUERY_VALUE | KEY_READ, &subKey))
                    {
                        size = RegQueryValueEx(subKey, 0, 0, &type,
                            (LPBYTE)&buffer[0], &size);
                        RegCloseKey(subKey);
                        if (!size)
                        {
                            RegCloseKey(hk);
                            extension = 0;
                            goto again;
                        }
                    }
                }
            }

            RegCloseKey(hk);

            if (!size)
            {
                // Convert the GUID string to a GUID
                // and put it in caller's guidBuffer
                if ((size = CLSIDFromString(&buffer[0], guidBuffer)))
                {
                    return(E_FAIL);
                }
                return(size);
            }
        }
    }

    return(E_FAIL);
}

ULONG STDMETHODCALLTYPE ep_static_AddRefRelease(void* _this)
{
    return 1;
}

HRESULT STDMETHODCALLTYPE IActiveScriptSite_QueryInterface(void* _this, REFIID riid, void** ppv)
{
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IActiveScriptSite))
        *ppv = _this;
    else if (IsEqualIID(riid, &IID_IActiveScriptSiteWindow))
        *ppv = ((unsigned char*)_this + 8);
    else
    {
        *ppv = 0;
        return(E_NOINTERFACE);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE IActiveScriptSiteWindow_QueryInterface(void* _this, REFIID riid, void** ppv)
{
    return IActiveScriptSite_QueryInterface((char*)_this - 8, riid, ppv);
}

HRESULT STDMETHODCALLTYPE IActiveScriptSite_GetLCID(void* _this, LCID* plcid)
{
    *plcid = LOCALE_USER_DEFAULT;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE IActiveScriptSite_GetItemInfo(void* _this, LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown** ppiunkItem, ITypeInfo** ppti)
{
    return TYPE_E_ELEMENTNOTFOUND;
}

HRESULT STDMETHODCALLTYPE IActiveScriptSite_GetDocVersionString(void* _this, BSTR* pbstrVersion)
{
    *pbstrVersion = 0;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE IActiveScriptSite_OnScriptTerminate(void* _this, const void* pvarResult, const EXCEPINFO* pexcepinfo)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE IActiveScriptSite_OnStateChange(void* _this, SCRIPTSTATE ssScriptState)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE IActiveScriptSite_OnScriptError(void* _this, IActiveScriptError* scriptError)
{
    ULONG        lineNumber;
    BSTR         desc;
    EXCEPINFO    ei;
    OLECHAR      wszOutput[1024];

    // Call GetSourcePosition() to retrieve the line # where
    // the error occurred in the script
    scriptError->lpVtbl->GetSourcePosition(scriptError, 0, &lineNumber, 0);

    // Call GetSourceLineText() to retrieve the line in the script that
    // has an error.
    desc = 0;
    scriptError->lpVtbl->GetSourceLineText(scriptError, &desc);

    // Call GetExceptionInfo() to fill in our EXCEPINFO struct with more
    // information.
    ZeroMemory(&ei, sizeof(EXCEPINFO));
    scriptError->lpVtbl->GetExceptionInfo(scriptError, &ei);

    // Format the message we'll display to the user
    wsprintfW(&wszOutput[0], L"%s\nLine %u: %s\n%s", ei.bstrSource,
        lineNumber + 1, ei.bstrDescription, desc ? desc : "");

    // Free what we got from the IActiveScriptError functions
    SysFreeString(desc);
    SysFreeString(ei.bstrSource);
    SysFreeString(ei.bstrDescription);
    SysFreeString(ei.bstrHelpFile);

    // Display the message
    MessageBoxW(0, &wszOutput[0], L"Error",
        MB_SETFOREGROUND | MB_OK | MB_ICONEXCLAMATION);

    return(S_OK);
}

HRESULT STDMETHODCALLTYPE IActiveScriptSite_OnEnterScript(void* _this)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE IActiveScriptSite_OnLeaveScript(void* _this)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE IActiveScriptSiteWindow_GetWindow(void* _this, HWND* phWnd)
{
    *phWnd = InputBox_HWND;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE IActiveScriptSiteWindow_EnableModeless(void* _this, BOOL fEnable)
{
    return S_OK;
}

static const IActiveScriptSiteVtbl IActiveScriptSite_Vtbl = {
    .QueryInterface = IActiveScriptSite_QueryInterface,
    .AddRef = ep_static_AddRefRelease,
    .Release = ep_static_AddRefRelease,
    .GetLCID = IActiveScriptSite_GetLCID,
    .GetItemInfo = IActiveScriptSite_GetItemInfo,
    .GetDocVersionString = IActiveScriptSite_GetDocVersionString,
    .OnScriptTerminate = IActiveScriptSite_OnScriptTerminate,
    .OnStateChange = IActiveScriptSite_OnStateChange,
    .OnScriptError = IActiveScriptSite_OnScriptError,
    .OnEnterScript = IActiveScriptSite_OnEnterScript,
    .OnLeaveScript = IActiveScriptSite_OnLeaveScript,
};

static const IActiveScriptSiteWindowVtbl IActiveScriptSiteWindow_Vtbl = {
    .QueryInterface = IActiveScriptSiteWindow_QueryInterface,
    .AddRef = ep_static_AddRefRelease,
    .Release = ep_static_AddRefRelease,
    .GetWindow = IActiveScriptSiteWindow_GetWindow,
    .EnableModeless = IActiveScriptSiteWindow_EnableModeless,
};

typedef struct _CSimpleScriptSite
{
    IActiveScriptSiteVtbl* lpVtbl;
    IActiveScriptSiteWindowVtbl* lpVtbl1;
} CSimpleScriptSite;

static const CSimpleScriptSite CSimpleScriptSite_Instance = {
    .lpVtbl = &IActiveScriptSite_Vtbl,
    .lpVtbl1 = &IActiveScriptSiteWindow_Vtbl
};

static BOOL HideInput = FALSE;
static LRESULT CALLBACK InputBoxProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < HC_ACTION)
        return CallNextHookEx(0, nCode, wParam, lParam);
    if (nCode = HCBT_ACTIVATE) {
        if (HideInput == TRUE) {
            HWND TextBox = FindWindowExA((HWND)wParam, NULL, "Edit", NULL);
            SendDlgItemMessageW((HWND)wParam, GetDlgCtrlID(TextBox), EM_SETPASSWORDCHAR, L'\x25cf', 0);
        }
    }
    if (nCode = HCBT_CREATEWND) {
        if (!(GetWindowLongPtr((HWND)wParam, GWL_STYLE) & WS_CHILD))
            SetWindowLongPtr((HWND)wParam, GWL_EXSTYLE, GetWindowLongPtr((HWND)wParam, GWL_EXSTYLE) | WS_EX_DLGMODALFRAME);
    }
    return CallNextHookEx(0, nCode, wParam, lParam);
}

HRESULT InputBox(BOOL bPassword, HWND hWnd, LPCWSTR wszPrompt, LPCWSTR wszTitle, LPCWSTR wszDefault, LPWSTR wszAnswer, DWORD cbAnswer, BOOL* bCancelled)
{
    HRESULT hr = S_OK;

    if (!wszPrompt || !wszTitle || !wszDefault || !wszAnswer || !cbAnswer || !bCancelled)
    {
        return E_FAIL;
    }

    GUID guidBuffer;
    hr = getEngineGuid(L".vbs", &guidBuffer);

    DWORD cchPromptSafe = 0, cchTitleSafe = 0, cchDefaultSafe = 0;
    LPWSTR wszPromptSafe = StrReplaceAllW(wszPrompt, L"\"", L"\"\"", &cchPromptSafe);
    LPWSTR wszTitleSafe = StrReplaceAllW(wszTitle, L"\"", L"\"\"", &cchTitleSafe);
    LPWSTR wszDefaultSafe = StrReplaceAllW(wszDefault, L"\"", L"\"\"", &cchDefaultSafe);
    if (!wszPromptSafe || !wszTitleSafe || !wszDefaultSafe)
    {
        if (wszPromptSafe)
        {
            free(wszPromptSafe);
        }
        if (wszTitleSafe)
        {
            free(wszTitleSafe);
        }
        if (wszDefaultSafe)
        {
            free(wszDefaultSafe);
        }
        return E_OUTOFMEMORY;
    }

    IActiveScript* pActiveScript = NULL;
    hr = CoCreateInstance(FAILED(hr) ? &CLSID_VBScript : &guidBuffer, 0, CLSCTX_ALL,
        &IID_IActiveScript,
        (void**)&pActiveScript);
    if (SUCCEEDED(hr) && pActiveScript)
    {
        hr = pActiveScript->lpVtbl->SetScriptSite(pActiveScript, &CSimpleScriptSite_Instance);
        if (SUCCEEDED(hr))
        {
            IActiveScriptParse* pActiveScriptParse = NULL;
            hr = pActiveScript->lpVtbl->QueryInterface(pActiveScript, &IID_IActiveScriptParse, &pActiveScriptParse);
            if (SUCCEEDED(hr) && pActiveScriptParse)
            {
                hr = pActiveScriptParse->lpVtbl->InitNew(pActiveScriptParse);
                if (SUCCEEDED(hr))
                {
                    LPWSTR wszEvaluation = malloc(sizeof(WCHAR) * (cchPromptSafe + cchTitleSafe + cchDefaultSafe + 100));
                    if (wszEvaluation)
                    {
                        swprintf_s(wszEvaluation, cchPromptSafe + cchTitleSafe + cchDefaultSafe + 100, L"InputBox(\"%s\", \"%s\", \"%s\")", wszPromptSafe, wszTitleSafe, wszDefaultSafe);
                        DWORD cchEvaluation2 = 0;
                        LPWSTR wszEvaluation2 = StrReplaceAllW(wszEvaluation, L"\n", L"\" + vbNewLine + \"", &cchEvaluation2);
                        if (wszEvaluation2)
                        {
                            EXCEPINFO ei;
                            ZeroMemory(&ei, sizeof(EXCEPINFO));
                            DWORD dwThreadId = GetCurrentThreadId();
                            HINSTANCE hInstance = GetModuleHandle(NULL);

                            if (!hWnd)
                            {
                                InputBox_HWND = GetAncestor(GetActiveWindow(), GA_ROOTOWNER);
                            }
                            else
                            {
                                InputBox_HWND = hWnd;
                            }

                            HHOOK hHook = SetWindowsHookExW(WH_CBT, &InputBoxProc, hInstance, dwThreadId);

                            VARIANT result;
                            VariantInit(&result);

                            HideInput = bPassword;
                            hr = pActiveScriptParse->lpVtbl->ParseScriptText(pActiveScriptParse, wszEvaluation2, NULL, NULL, NULL, 0, 0, SCRIPTTEXT_ISEXPRESSION, &result, &ei);

                            *bCancelled = (result.vt == VT_EMPTY);

                            UnhookWindowsHookEx(hHook);

                            free(wszEvaluation2);

                            if (result.bstrVal)
                            {
                                memcpy(wszAnswer, result.bstrVal, cbAnswer * sizeof(WCHAR));
                            }
                            else
                            {
                                if (result.vt != VT_EMPTY)
                                {
                                    wszAnswer[0] = 0;
                                }
                            }

                            VariantClear(&result);
                        }
                        free(wszEvaluation);
                    }
                }
                pActiveScriptParse->lpVtbl->Release(pActiveScriptParse);
            }
            pActiveScript->lpVtbl->Release(pActiveScript);
        }
    }

    if (wszPromptSafe)
    {
        free(wszPromptSafe);
    }
    if (wszTitleSafe)
    {
        free(wszTitleSafe);
    }
    if (wszDefaultSafe)
    {
        free(wszDefaultSafe);
    }

    return hr;
}

UINT PleaseWaitTimeout = 0;
HHOOK PleaseWaitHook = NULL;
HWND PleaseWaitHWND = NULL;
void* PleaseWaitCallbackData = NULL;
BOOL (*PleaseWaitCallbackFunc)(void* data) = NULL;
BOOL PleaseWait_UpdateTimeout(int timeout)
{
    if (PleaseWaitHWND)
    {
        KillTimer(PleaseWaitHWND, 'EPPW');
        PleaseWaitTimeout = timeout;
        return SetTimer(PleaseWaitHWND, 'EPPW', PleaseWaitTimeout, PleaseWait_TimerProc);
    }
    return FALSE;
}

VOID CALLBACK PleaseWait_TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    if (idEvent == 'EPPW')
    {
        if (PleaseWaitCallbackFunc)
        {
            if (PleaseWaitCallbackFunc(PleaseWaitCallbackData))
            {
                return;
            }
            PleaseWaitCallbackData = NULL;
            PleaseWaitCallbackFunc = NULL;
        }
        KillTimer(hWnd, 'EPPW');
        SetTimer(hWnd, 'EPPW', 0, NULL); // <- this closes the message box
        PleaseWaitHWND = NULL;
        PleaseWaitTimeout = 0;
    }
}

LRESULT CALLBACK PleaseWait_HookProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code < 0)
    {
        return CallNextHookEx(NULL, code, wParam, lParam);
    }

    CWPSTRUCT* msg = (CWPSTRUCT*)lParam;
    /*if (msg->message == WM_CREATE)
    {
        CREATESTRUCT* pCS = (CREATESTRUCT*)msg->lParam;
        if (pCS->lpszClass == RegisterWindowMessageW(L"Button"))
        {
        }
    }*/
    LRESULT result = CallNextHookEx(NULL, code, wParam, lParam);

    if (msg->message == WM_INITDIALOG)
    {
        PleaseWaitHWND = msg->hwnd;
        EnableWindow(PleaseWaitHWND, FALSE);
        LONG_PTR style = GetWindowLongPtrW(PleaseWaitHWND, GWL_STYLE);
        SetWindowLongPtrW(PleaseWaitHWND, GWL_STYLE, style & ~WS_SYSMENU);
        RECT rc;
        GetWindowRect(PleaseWaitHWND, &rc);
        SetWindowPos(PleaseWaitHWND, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top - MulDiv(50, GetDpiForWindow(PleaseWaitHWND), 96), SWP_NOMOVE | SWP_FRAMECHANGED);
        SetTimer(PleaseWaitHWND, 'EPPW', PleaseWaitTimeout, PleaseWait_TimerProc);
        UnhookWindowsHookEx(PleaseWaitHook);
        PleaseWaitHook = NULL;
    }
    return result;
}

BOOL DownloadAndInstallWebView2Runtime()
{
    BOOL bOK = FALSE;
    HINTERNET hInternet = NULL;
    if (hInternet = InternetOpenA(
        "ExplorerPatcher",
        INTERNET_OPEN_TYPE_PRECONFIG,
        NULL,
        NULL,
        0
    ))
    {
        HINTERNET hConnect = InternetOpenUrlA(
            hInternet,
            "https://go.microsoft.com/fwlink/p/?LinkId=2124703",
            NULL,
            0,
            INTERNET_FLAG_RAW_DATA |
            INTERNET_FLAG_RELOAD |
            INTERNET_FLAG_RESYNCHRONIZE |
            INTERNET_FLAG_NO_COOKIES |
            INTERNET_FLAG_NO_UI |
            INTERNET_FLAG_NO_CACHE_WRITE |
            INTERNET_FLAG_DONT_CACHE,
            NULL
        );
        if (hConnect)
        {
            char* exe_buffer = NULL;
            DWORD dwSize = 2 * 1024 * 1024;
            DWORD dwRead = dwSize;
            exe_buffer = calloc(dwSize, sizeof(char));
            if (exe_buffer)
            {
                BOOL bRet = FALSE;
                if (bRet = InternetReadFile(
                    hConnect,
                    exe_buffer,
                    dwSize - 1,
                    &dwRead
                ))
                {
                    WCHAR wszPath[MAX_PATH];
                    ZeroMemory(wszPath, MAX_PATH * sizeof(WCHAR));
                    SHGetFolderPathW(NULL, SPECIAL_FOLDER_LEGACY, NULL, SHGFP_TYPE_CURRENT, wszPath);
                    wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH));
                    BOOL bRet = CreateDirectoryW(wszPath, NULL);
                    if (bRet || (!bRet && GetLastError() == ERROR_ALREADY_EXISTS))
                    {
                        wcscat_s(wszPath, MAX_PATH, L"\\MicrosoftEdgeWebview2Setup.exe");
                        FILE* f = NULL;
                        _wfopen_s(&f, wszPath, L"wb");
                        if (f)
                        {
                            fwrite(exe_buffer, 1, dwRead, f);
                            fclose(f);
                            SHELLEXECUTEINFOW sei;
                            ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
                            sei.cbSize = sizeof(sei);
                            sei.fMask = SEE_MASK_NOCLOSEPROCESS;
                            sei.hwnd = NULL;
                            sei.hInstApp = NULL;
                            sei.lpVerb = NULL;
                            sei.lpFile = wszPath;
                            sei.lpParameters = L"";
                            sei.hwnd = NULL;
                            sei.nShow = SW_SHOWMINIMIZED;
                            if (ShellExecuteExW(&sei) && sei.hProcess)
                            {
                                WaitForSingleObject(sei.hProcess, INFINITE);
                                CloseHandle(sei.hProcess);
                                Sleep(100);
                                DeleteFileW(wszPath);
                                bOK = TRUE;
                            }
                        }
                    }
                }
                free(exe_buffer);
            }
            InternetCloseHandle(hConnect);
        }
        InternetCloseHandle(hInternet);
    }
    return bOK;
}

BOOL DownloadFile(LPCWSTR wszURL, DWORD dwSize, LPCWSTR wszPath)
{
    BOOL bOK = FALSE;
    HINTERNET hInternet = NULL;
    if (hInternet = InternetOpenW(
        L"ExplorerPatcher",
        INTERNET_OPEN_TYPE_PRECONFIG,
        NULL,
        NULL,
        0
    ))
    {
        HINTERNET hConnect = InternetOpenUrlW(
            hInternet,
            wszURL,
            NULL,
            0,
            INTERNET_FLAG_RAW_DATA |
            INTERNET_FLAG_RELOAD |
            INTERNET_FLAG_RESYNCHRONIZE |
            INTERNET_FLAG_NO_COOKIES |
            INTERNET_FLAG_NO_UI |
            INTERNET_FLAG_NO_CACHE_WRITE |
            INTERNET_FLAG_DONT_CACHE,
            NULL
        );
        if (hConnect)
        {
            char* exe_buffer = NULL;
            DWORD dwRead = dwSize;
            exe_buffer = calloc(dwSize, sizeof(char));
            if (exe_buffer)
            {
                BOOL bRet = FALSE;
                if (bRet = InternetReadFile(
                    hConnect,
                    exe_buffer,
                    dwSize - 1,
                    &dwRead
                ))
                {
                    FILE* f = NULL;
                    _wfopen_s(&f, wszPath, L"wb");
                    if (f)
                    {
                        fwrite(exe_buffer, 1, dwRead, f);
                        fclose(f);
                    }
                }
                free(exe_buffer);
            }
            InternetCloseHandle(hConnect);
        }
        InternetCloseHandle(hInternet);
    }
    return bOK;
}

BOOL IsConnectedToInternet()
{
    BOOL connectedStatus = FALSE;
    HRESULT hr = S_FALSE;

    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        INetworkListManager* pNetworkListManager;
        hr = CoCreateInstance(&CLSID_NetworkListManager, NULL, CLSCTX_ALL, &IID_NetworkListManager, (LPVOID*)&pNetworkListManager);
        if (SUCCEEDED(hr))
        {
            NLM_CONNECTIVITY nlmConnectivity = NLM_CONNECTIVITY_DISCONNECTED;
            VARIANT_BOOL isConnected = VARIANT_FALSE;
            hr = pNetworkListManager->lpVtbl->get_IsConnectedToInternet(pNetworkListManager, &isConnected);
            if (SUCCEEDED(hr))
            {
                if (isConnected == VARIANT_TRUE)
                    connectedStatus = TRUE;
                else
                    connectedStatus = FALSE;
            }
            if (isConnected == VARIANT_FALSE && SUCCEEDED(pNetworkListManager->lpVtbl->GetConnectivity(pNetworkListManager, &nlmConnectivity)))
            {
                if (nlmConnectivity & (NLM_CONNECTIVITY_IPV4_LOCALNETWORK | NLM_CONNECTIVITY_IPV4_SUBNET | NLM_CONNECTIVITY_IPV6_LOCALNETWORK | NLM_CONNECTIVITY_IPV6_SUBNET))
                {
                    connectedStatus = 2;
                }
            }
            pNetworkListManager->lpVtbl->Release(pNetworkListManager);
        }
        CoUninitialize();
    }
    return connectedStatus;
}

BOOL DoesOSBuildSupportSpotlight()
{
    return (global_rovi.dwBuildNumber == 22000 && global_ubr >= 706) || (global_rovi.dwBuildNumber >= 22598);
}

BOOL IsSpotlightEnabled()
{
    HKEY hKey = NULL;
    BOOL bRet = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\{2CC5CA98-6485-489A-920E-B3E88A6CCCE3}", 0, KEY_READ, &hKey) == ERROR_SUCCESS;
    if (bRet) RegCloseKey(hKey);
    return bRet;
}

const int spop_insertmenu_ops[] = { SPOP_INSERTMENU_OPEN, SPOP_INSERTMENU_NEXTPIC, 0, SPOP_INSERTMENU_LIKE, SPOP_INSERTMENU_DISLIKE };
void SpotlightHelper(DWORD dwOp, HWND hWnd, HMENU hMenu, LPPOINT pPt)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidl = NULL;
    SFGAOF sfgao = 0;
    if (SUCCEEDED(hr = SHParseDisplayName(L"::{2CC5CA98-6485-489A-920E-B3E88A6CCCE3}", NULL, &pidl, 0, &sfgao)))
    {
        IShellFolder* psf = NULL;
        LPCITEMIDLIST pidlChild;
        if (SUCCEEDED(hr = SHBindToParent(pidl, &IID_IShellFolder, (void**)&psf, &pidlChild)))
        {
            IContextMenu* pcm = NULL;
            if (SUCCEEDED(hr = psf->lpVtbl->GetUIObjectOf(psf, hWnd, 1, &pidlChild, &IID_IContextMenu, NULL, &pcm)))
            {
                HMENU hMenu2 = CreatePopupMenu();
                if (hMenu2)
                {
                    if (SUCCEEDED(hr = pcm->lpVtbl->QueryContextMenu(pcm, hMenu2, 0, SCRATCH_QCM_FIRST, SCRATCH_QCM_LAST, CMF_NORMAL)))
                    {
                        if (dwOp == SPOP_OPENMENU)
                        {
                            int iCmd = TrackPopupMenuEx(hMenu2, TPM_RETURNCMD, pPt->x, pPt->y, hWnd, NULL);
                            if (iCmd > 0)
                            {
                                CMINVOKECOMMANDINFOEX info = { 0 };
                                info.cbSize = sizeof(info);
                                info.fMask = CMIC_MASK_UNICODE | CMIC_MASK_PTINVOKE;
                                info.hwnd = hWnd;
                                info.lpVerb = MAKEINTRESOURCEA(iCmd - SCRATCH_QCM_FIRST);
                                info.lpVerbW = MAKEINTRESOURCEW(iCmd - SCRATCH_QCM_FIRST);
                                info.nShow = SW_SHOWNORMAL;
                                info.ptInvoke = *pPt;
                                pcm->lpVtbl->InvokeCommand(pcm, &info);
                            }
                        }
                        else if (!(dwOp & ~SPOP_INSERTMENU_ALL))
                        {
                            MENUITEMINFOW mii;
                            int i = ARRAYSIZE(spop_insertmenu_ops) - 1;
                            while (1)
                            {
                                if (i == -1 ? ((dwOp & SPOP_INSERTMENU_INFOTIP1) || (dwOp & SPOP_INSERTMENU_INFOTIP2)) : (dwOp & spop_insertmenu_ops[i]))
                                {
                                    mii.cbSize = sizeof(MENUITEMINFOW);
                                    mii.fMask = MIIM_FTYPE | MIIM_STRING;
                                    mii.cch = 0;
                                    mii.dwTypeData = NULL;
                                    if (i <= 0 ?
                                        (i == 0 ?
                                            !RegQueryValueW(HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\{2cc5ca98-6485-489a-920e-b3e88a6ccce3}", NULL, &mii.cch) :
                                            !RegGetValueW(HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\{2cc5ca98-6485-489a-920e-b3e88a6ccce3}", L"InfoTip", RRF_RT_REG_SZ, NULL, NULL, &mii.cch)
                                            ) :
                                        GetMenuItemInfoW(hMenu2, i, TRUE, &mii))
                                    {
                                        WCHAR* buf = malloc(++mii.cch * sizeof(WCHAR));
                                        if (buf)
                                        {
                                            mii.dwTypeData = buf;
                                            if (i <= 0 ?
                                                (i == 0 ?
                                                    !RegQueryValueW(HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\{2cc5ca98-6485-489a-920e-b3e88a6ccce3}", mii.dwTypeData, &mii.cch) :
                                                    !RegGetValueW(HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\{2cc5ca98-6485-489a-920e-b3e88a6ccce3}", L"InfoTip", RRF_RT_REG_SZ, NULL, mii.dwTypeData, &mii.cch)
                                                    ) :
                                                GetMenuItemInfoW(hMenu2, i, TRUE, &mii))
                                            {
                                                if (i == -1)
                                                {
                                                    WCHAR* pCInit = mii.dwTypeData;
                                                    WCHAR* pC = wcschr(mii.dwTypeData, L'\r');
                                                    if (pC)
                                                    {
                                                        pC[0] = 0;

                                                        pC++;
                                                        WCHAR* pC2 = wcschr(pC, L'\r');
                                                        if (pC2)
                                                        {
                                                            pC2[0] = 0;
                                                        }
                                                        mii.dwTypeData = pC;

                                                        mii.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA | MIIM_STATE;
                                                        mii.wID = 3999 + i - 1;
                                                        mii.dwItemData = SPOP_CLICKMENU_FIRST + i - 1;
                                                        mii.fType = MFT_STRING;
                                                        mii.fState = MFS_DISABLED;
                                                        if (dwOp & SPOP_INSERTMENU_INFOTIP2)
                                                        {
                                                            InsertMenuItemW(hMenu, 3, TRUE, &mii);
                                                        }

                                                        mii.dwTypeData = pCInit;
                                                    }
                                                }
                                                mii.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA | (i == -1 ? MIIM_STATE : 0);
                                                mii.wID = 3999 + i;
                                                mii.dwItemData = SPOP_CLICKMENU_FIRST + i;
                                                mii.fType = MFT_STRING;
                                                if (i == -1) mii.fState = MFS_DISABLED;
                                                if (i != -1 || (i == -1 && (dwOp & SPOP_INSERTMENU_INFOTIP1)))
                                                {
                                                    InsertMenuItemW(hMenu, 3, TRUE, &mii);
                                                }
                                            }
                                            free(buf);
                                        }
                                    }
                                }
                                i--;
                                if (i < -1) break;
                            }
                            mii.fMask = MIIM_FTYPE | MIIM_DATA;
                            mii.dwItemData = 0;
                            mii.fType = MFT_SEPARATOR;
                            InsertMenuItemW(hMenu, 3, TRUE, &mii);
                        }
                        else if (dwOp >= SPOP_CLICKMENU_FIRST && dwOp <= SPOP_CLICKMENU_LAST)
                        {
                            MENUITEMINFOW mii;
                            mii.cbSize = sizeof(MENUITEMINFOW);
                            mii.fMask = MIIM_ID;
                            if (GetMenuItemInfoW(hMenu2, dwOp - SPOP_CLICKMENU_FIRST, TRUE, &mii))
                            {
                                CMINVOKECOMMANDINFOEX info = { 0 };
                                info.cbSize = sizeof(info);
                                info.fMask = CMIC_MASK_UNICODE;
                                info.hwnd = hWnd;
                                info.lpVerb = MAKEINTRESOURCEA(mii.wID - SCRATCH_QCM_FIRST);
                                info.lpVerbW = MAKEINTRESOURCEW(mii.wID - SCRATCH_QCM_FIRST);
                                info.nShow = SW_SHOWNORMAL;
                                pcm->lpVtbl->InvokeCommand(pcm, &info);
                            }
                        }
                    }
                    DestroyMenu(hMenu2);
                }
                pcm->lpVtbl->Release(pcm);
            }
            psf->lpVtbl->Release(psf);
        }
        CoTaskMemFree(pidl);
    }
}

BOOL ExtractMonitorByIndex(HMONITOR hMonitor, HDC hDC, LPRECT lpRect, MonitorOverrideData* mod)
{
    POINT pt; pt.x = 0; pt.y = 0;
    if (MonitorFromPoint(pt, MONITOR_DEFAULTTONULL) == hMonitor)
    {
        return TRUE;
    }
    if (mod->cbIndex == mod->dwIndex)
    {
        mod->hMonitor = hMonitor;
        return FALSE;
    }
    mod->cbIndex++;
    return TRUE;
}

DWORD GetProcessIdByExeName(LPCWSTR wszProcessName)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);
        BOOL bRet = Process32FirstW(hSnap, &pe);
        while (bRet)
        {
            if (!_wcsicmp(pe.szExeFile, wszProcessName))
            {
                CloseHandle(hSnap);
                return pe.th32ProcessID;
            }
            bRet = Process32NextW(hSnap, &pe);
        }
        CloseHandle(hSnap);
    }
    return 0;
}

void KillProcess(LPCWSTR wszProcessName)
{
    DWORD dwProcessId = GetProcessIdByExeName(wszProcessName);
    if (!dwProcessId)
        return;

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
    if (hProcess)
    {
        TerminateProcess(hProcess, 1);
        CloseHandle(hProcess);
    }
}

#ifdef _WIN64
inline BOOL MaskCompare(PVOID pBuffer, LPCSTR lpPattern, LPCSTR lpMask)
{
    for (PBYTE value = (PBYTE)pBuffer; *lpMask; ++lpPattern, ++lpMask, ++value)
    {
        if (*lpMask == 'x' && *(LPCBYTE)lpPattern != *value)
            return FALSE;
    }

    return TRUE;
}

PVOID FindPattern(PVOID pBase, SIZE_T dwSize, LPCSTR lpPattern, LPCSTR lpMask)
{
    dwSize -= strlen(lpMask);

    for (SIZE_T index = 0; index < dwSize; ++index)
    {
        PBYTE pAddress = (PBYTE)pBase + index;

        if (MaskCompare(pAddress, lpPattern, lpMask))
            return pAddress;
    }

    return NULL;
}
#endif

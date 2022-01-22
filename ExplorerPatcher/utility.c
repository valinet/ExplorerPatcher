#include "utility.h"

#pragma region "Weird stuff"
INT64 STDMETHODCALLTYPE nimpl4_1(INT64 a1, DWORD* a2)
{
    *a2 = 1;
    return 0;
}
INT64 STDMETHODCALLTYPE nimpl4_0(INT64 a1, DWORD* a2)
{
    *a2 = 0;
    return 0;
}
__int64 STDMETHODCALLTYPE nimpl2(__int64 a1, uintptr_t* a2)
{
    __int64 v2; // rax

    v2 = a1 + 8;
    if (!a1)
        v2 = 0i64;

    *a2 = v2;
    return 0i64;
}
ULONG STDMETHODCALLTYPE nimpl3()
{
    return 1;
}
HRESULT STDMETHODCALLTYPE nimpl()
{
    return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE nimpl1(__int64 a1, uintptr_t* a2, uintptr_t* a3)
{
    __int64 v4 = a1; // rcx

    if (*a2 != 0x5FADCA5C34A95314i64 || a2[1] != 0xC1661118901A7CAEui64)
        return E_NOTIMPL;

    *a3 = v4;
    return S_OK;
}
HRESULT STDMETHODCALLTYPE nimpl1_2(__int64 a1, uintptr_t* a2, uintptr_t* a3)
{
    __int64 v4 = a1 - sizeof(__int64); // rcx

    if (*a2 != 0x5FADCA5C34A95314i64 || a2[1] != 0xC1661118901A7CAEui64)
        return E_NOTIMPL;

    *a3 = v4;
    return S_OK;
}
HRESULT STDMETHODCALLTYPE nimpl1_3(__int64 a1, uintptr_t* a2, uintptr_t* a3)
{
    __int64 v4 = a1 - 2 * sizeof(__int64); // rcx

    if (*a2 != 0x5FADCA5C34A95314i64 || a2[1] != 0xC1661118901A7CAEui64)
        return E_NOTIMPL;

    *a3 = v4;
    return S_OK;
}
__int64 STDMETHODCALLTYPE nimpl4(__int64 a1, __int64 a2, __int64 a3, BYTE* a4)
{
    *a4 = 0;
    return 0i64;
}
const IActivationFactoryVtbl _IActivationFactoryVtbl = {
    .QueryInterface = nimpl1,
    .AddRef = nimpl3,
    .Release = nimpl3,
    .GetIids = nimpl,
    .GetRuntimeClassName = nimpl,
    .GetTrustLevel = nimpl,
    .ActivateInstance = nimpl2
};
const IActivationFactoryVtbl _IActivationFactoryVtbl2 = {
    .QueryInterface = nimpl1_2,
    .AddRef = nimpl3,
    .Release = nimpl3,
    .GetIids = nimpl,
    .GetRuntimeClassName = nimpl,
    .GetTrustLevel = nimpl,
    .ActivateInstance = nimpl
};
const IActivationFactoryVtbl _IActivationFactoryVtbl3 = {
    .QueryInterface = nimpl1_3,
    .AddRef = nimpl3,
    .Release = nimpl3,
    .GetIids = nimpl,
    .GetRuntimeClassName = nimpl,
    .GetTrustLevel = nimpl,
    .ActivateInstance = nimpl4
};
const IActivationFactoryAA XamlExtensionsFactory = {
    .lpVtbl = &_IActivationFactoryVtbl,
    .lpVtbl2 = &_IActivationFactoryVtbl2,
    .lpVtbl3 = &_IActivationFactoryVtbl3
};
#pragma endregion

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

__declspec(dllexport) CALLBACK ZZTestBalloon(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
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
}

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
__declspec(dllexport) CALLBACK ZZTestToast(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
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
}

__declspec(dllexport) CALLBACK ZZLaunchExplorer(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
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
}

__declspec(dllexport) CALLBACK ZZLaunchExplorerDelayed(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    Sleep(2000);
    ZZLaunchExplorer(hWnd, hInstance, lpszCmdLine, nCmdShow);
}

__declspec(dllexport) CALLBACK ZZRestartExplorer(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    BeginExplorerRestart();
    FinishExplorerRestart();
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

    sprintf_s(hash, 33, "%d.%d.%d.%d.", dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);

    char real_hash[33];
    ComputeFileHash(filename, real_hash, 33);
    strncpy_s(hash + strlen(hash), dwHash, real_hash, 32 - strlen(hash));
    hash[33] = 0;

    return ERROR_SUCCESS;
}

void LaunchPropertiesGUI(HMODULE hModule)
{
    //CreateThread(0, 0, ZZGUI, 0, 0, 0);
    wchar_t wszPath[MAX_PATH * 2];
    ZeroMemory(
        wszPath,
        (MAX_PATH * 2) * sizeof(wchar_t)
    );
    wszPath[0] = '\"';
    GetSystemDirectoryW(
        wszPath + 1,
        MAX_PATH
    );
    wcscat_s(
        wszPath,
        MAX_PATH * 2,
        L"\\rundll32.exe\" \""
    );
    GetModuleFileNameW(
        hModule,
        wszPath + wcslen(wszPath),
        MAX_PATH
    );
    wcscat_s(
        wszPath,
        MAX_PATH * 2,
        L"\",ZZGUI"
    );
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

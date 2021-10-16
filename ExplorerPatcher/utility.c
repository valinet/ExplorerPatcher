#include "utility.h"

#pragma region "Weird stuff"
INT64 nimpl4_1(INT64 a1, DWORD* a2)
{
    *a2 = 1;
    return 0;
}
INT64 nimpl4_0(INT64 a1, DWORD* a2)
{
    *a2 = 0;
    return 0;
}
__int64 __fastcall nimpl2(__int64 a1, uintptr_t* a2)
{
    __int64 v2; // rax

    v2 = a1 + 8;
    if (!a1)
        v2 = 0i64;

    *a2 = v2;
    return 0i64;
}
ULONG nimpl3()
{
    return 1;
}
HRESULT nimpl()
{
    return E_NOTIMPL;
}
HRESULT nimpl1(__int64 a1, uintptr_t* a2, uintptr_t* a3)
{
    __int64 v4 = a1; // rcx

    if (*a2 != 0x5FADCA5C34A95314i64 || a2[1] != 0xC1661118901A7CAEui64)
        return E_NOTIMPL;

    *a3 = v4;
    return S_OK;
}
HRESULT nimpl1_2(__int64 a1, uintptr_t* a2, uintptr_t* a3)
{
    __int64 v4 = a1 - sizeof(__int64); // rcx

    if (*a2 != 0x5FADCA5C34A95314i64 || a2[1] != 0xC1661118901A7CAEui64)
        return E_NOTIMPL;

    *a3 = v4;
    return S_OK;
}
HRESULT nimpl1_3(__int64 a1, uintptr_t* a2, uintptr_t* a3)
{
    __int64 v4 = a1 - 2 * sizeof(__int64); // rcx

    if (*a2 != 0x5FADCA5C34A95314i64 || a2[1] != 0xC1661118901A7CAEui64)
        return E_NOTIMPL;

    *a3 = v4;
    return S_OK;
}
__int64 nimpl4(__int64 a1, __int64 a2, __int64 a3, BYTE* a4)
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

int FileExistsW(wchar_t* file)
{
    WIN32_FIND_DATAW FindFileData;
    HANDLE handle = FindFirstFileW(file, &FindFileData);
    int found = handle != INVALID_HANDLE_VALUE;
    if (found)
    {
        FindClose(handle);
    }
    return found;
}

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
L"<toast displayTimestamp=\"2021-08-29T00:00:00.000Z\" scenario=\"reminder\" "
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

POINT GetDefaultWinXPosition(BOOL bUseRcWork, BOOL* lpBottom, BOOL* lpRight, BOOL bAdjust)
{
    if (lpBottom) *lpBottom = FALSE;
    if (lpRight) *lpRight = FALSE;
    POINT point;
    point.x = 0;
    point.y = 0;
    POINT ptCursor;
    GetCursorPos(&ptCursor);
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    HWND hWnd = GetMonitorInfoFromPointForTaskbarFlyoutActivation(
        ptCursor,
        MONITOR_DEFAULTTOPRIMARY,
        &mi
    );
    if (hWnd)
    {
        RECT rc;
        GetWindowRect(hWnd, &rc);
        if (rc.left - mi.rcMonitor.left <= 0)
        {
            if (bUseRcWork)
            {
                point.x = mi.rcWork.left;
            }
            else
            {
                point.x = mi.rcMonitor.left;
            }
            if (bAdjust)
            {
                point.x++;
            }
            if (rc.top - mi.rcMonitor.top <= 0)
            {
                if (bUseRcWork)
                {
                    point.y = mi.rcWork.top;
                }
                else
                {
                    point.y = mi.rcMonitor.top;
                }
                if (bAdjust)
                {
                    point.y++;
                }
            }
            else
            {
                if (lpBottom) *lpBottom = TRUE;
                if (bUseRcWork)
                {
                    point.y = mi.rcWork.bottom;
                }
                else
                {
                    point.y = mi.rcMonitor.bottom;
                }
                if (bAdjust)
                {
                    point.y--;
                }
            }
        }
        else
        {
            if (lpRight) *lpRight = TRUE;
            if (bUseRcWork)
            {
                point.x = mi.rcWork.right;
            }
            else
            {
                point.x = mi.rcMonitor.right;
            }
            if (bAdjust)
            {
                point.x--;
            }
            if (rc.top - mi.rcMonitor.top <= 0)
            {
                if (bUseRcWork)
                {
                    point.y = mi.rcWork.top;
                }
                else
                {
                    point.y = mi.rcMonitor.top;
                }
                if (bAdjust)
                {
                    point.y++;
                }
            }
            else
            {
                if (lpBottom) *lpBottom = TRUE;
                if (bUseRcWork)
                {
                    point.y = mi.rcWork.bottom;
                }
                else
                {
                    point.y = mi.rcMonitor.bottom;
                }
                if (bAdjust)
                {
                    point.y--;
                }
            }
        }
    }
    return point;
}

void QueryVersionInfo(HMODULE hModule, WORD Resource, DWORD* dwLeftMost, DWORD* dwSecondLeft, DWORD* dwSecondRight, DWORD* dwRightMost)
{
    HRSRC hResInfo;
    DWORD dwSize;
    HGLOBAL hResData;
    LPVOID pRes, pResCopy;
    UINT uLen;
    VS_FIXEDFILEINFO* lpFfi;

    hResInfo = FindResource(hModule, MAKEINTRESOURCE(Resource), RT_VERSION);
    dwSize = SizeofResource(hModule, hResInfo);
    hResData = LoadResource(hModule, hResInfo);
    pRes = LockResource(hResData);
    pResCopy = LocalAlloc(LMEM_FIXED, dwSize);
    CopyMemory(pResCopy, pRes, dwSize);
    FreeResource(hResData);

    VerQueryValue(pResCopy, TEXT("\\"), (LPVOID*)&lpFfi, &uLen);

    DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
    DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;

    *dwLeftMost = HIWORD(dwFileVersionMS);
    *dwSecondLeft = LOWORD(dwFileVersionMS);
    *dwSecondRight = HIWORD(dwFileVersionLS);
    *dwRightMost = LOWORD(dwFileVersionLS);

    LocalFree(pResCopy);
}
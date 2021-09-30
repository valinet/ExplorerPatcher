#include "StartMenu.h"

void OpenStartOnMonitor(HMONITOR monitor)
{
    HRESULT hr = S_OK;
    IUnknown* pImmersiveShell = NULL;
    hr = CoCreateInstance(
        &CLSID_ImmersiveShell,
        NULL,
        CLSCTX_NO_CODE_DOWNLOAD | CLSCTX_LOCAL_SERVER,
        &IID_IServiceProvider,
        &pImmersiveShell
    );
    if (SUCCEEDED(hr))
    {
        IImmersiveMonitorService* pMonitorService = NULL;
        IUnknown_QueryService(
            pImmersiveShell,
            &SID_IImmersiveMonitorService,
            &IID_IImmersiveMonitorService,
            &pMonitorService
        );
        if (pMonitorService)
        {
            IUnknown* pMonitor = NULL;
            pMonitorService->lpVtbl->GetFromHandle(
                pMonitorService,
                monitor,
                &pMonitor
            );
            IImmersiveLauncher10RS* pLauncher = NULL;
            IUnknown_QueryService(
                pImmersiveShell,
                &SID_ImmersiveLauncher,
                &IID_IImmersiveLauncher10RS,
                &pLauncher
            );
            if (pLauncher)
            {
                BOOL bIsVisible = FALSE;
                pLauncher->lpVtbl->IsVisible(pLauncher, &bIsVisible);
                if (SUCCEEDED(hr))
                {
                    if (!bIsVisible)
                    {
                        if (pMonitor)
                        {
                            pLauncher->lpVtbl->ConnectToMonitor(pLauncher, pMonitor);
                        }
                        pLauncher->lpVtbl->ShowStartView(pLauncher, 11, 0);
                    }
                    else
                    {
                        pLauncher->lpVtbl->Dismiss(pLauncher);
                    }
                }
                pLauncher->lpVtbl->Release(pLauncher);
            }
            if (pMonitor)
            {
                pMonitor->lpVtbl->Release(pMonitor);
            }
            pMonitorService->lpVtbl->Release(pMonitorService);
        }
        pImmersiveShell->lpVtbl->Release(pImmersiveShell);
    }
}

LRESULT CALLBACK OpenStartOnCurentMonitorThreadHook(
    int code,
    WPARAM wParam,
    LPARAM lParam
)
{
    if (code == HC_ACTION && wParam)
    {
        MSG* msg = (MSG*)lParam;
        if (GetSystemMetrics(SM_CMONITORS) >= 2 && msg->message == WM_SYSCOMMAND && (msg->wParam & 0xFFF0) == SC_TASKLIST)
        {
            DWORD dwStatus = 0;
            DWORD dwSize = sizeof(DWORD);
            HMODULE hModule = GetModuleHandle(TEXT("Shlwapi.dll"));
            FARPROC SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(hModule, "SHRegGetValueFromHKCUHKLM");
            if (!SHRegGetValueFromHKCUHKLMFunc || SHRegGetValueFromHKCUHKLMFunc(
                TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage"),
                TEXT("MonitorOverride"),
                SRRF_RT_REG_DWORD,
                NULL,
                &dwStatus,
                (LPDWORD)(&dwSize)
            ) != ERROR_SUCCESS || dwStatus == 1)
            {
                goto finish;
            }

            DWORD pts = GetMessagePos();
            POINT pt;
            pt.x = GET_X_LPARAM(pts);
            pt.y = GET_Y_LPARAM(pts);
            HMONITOR monitor = MonitorFromPoint(
                pt,
                MONITOR_DEFAULTTONULL
            );
            OpenStartOnMonitor(monitor);

            msg->message = WM_NULL;
        }
    }
finish:
    return CallNextHookEx(NULL, code, wParam, lParam);
}

DWORD OpenStartOnCurentMonitorThread(OpenStartOnCurentMonitorThreadParams* unused)
{
    HANDLE hEvent = CreateEvent(0, 0, 0, L"ShellDesktopSwitchEvent");
    if (!hEvent)
    {
        printf("Failed to start \"Open Start on current monitor\" thread.\n");
        return 0;
    }
    WaitForSingleObject(
        hEvent,
        INFINITE
    );
    printf("Started \"Open Start on current monitor\" thread.\n");
    HWND g_ProgWin = FindWindowEx(
        NULL,
        NULL,
        L"Progman",
        NULL
    );
    DWORD progThread = GetWindowThreadProcessId(
        g_ProgWin,
        NULL
    );
    HHOOK g_ProgHook = SetWindowsHookEx(
        WH_GETMESSAGE,
        OpenStartOnCurentMonitorThreadHook,
        NULL,
        progThread
    );
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    printf("Ended \"Open Start on current monitor\" thread.\n");
}

DWORD OpenStartAtLogonThread(OpenStartAtLogonThreadParams* unused)
{
    HANDLE hEvent = CreateEvent(0, 0, 0, L"ShellDesktopSwitchEvent");
    if (!hEvent)
    {
        printf("Failed to start \"Open Start at Logon\" thread.\n");
        return 0;
    }
    WaitForSingleObject(
        hEvent,
        INFINITE
    );
    printf("Started \"Open Start at Logon\" thread.\n");

    DWORD dwStatus = 0;
    DWORD dwSize = sizeof(DWORD);
    HMODULE hModule = GetModuleHandle(TEXT("Shlwapi"));
    FARPROC SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(hModule, "SHRegGetValueFromHKCUHKLM");
    if (!SHRegGetValueFromHKCUHKLMFunc || SHRegGetValueFromHKCUHKLMFunc(
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage"),
        TEXT("OpenAtLogon"),
        SRRF_RT_REG_DWORD,
        NULL,
        &dwStatus,
        (LPDWORD)(&dwSize)
    ) != ERROR_SUCCESS || dwStatus == 0)
    {
        return 0;
    }

    POINT pt;
    pt.x = 0;
    pt.y = 0;
    HMONITOR monitor = MonitorFromPoint(
        pt,
        MONITOR_DEFAULTTOPRIMARY
    );
    OpenStartOnMonitor(monitor);

    printf("Ended \"Open Start at Logon\" thread.\n");
}

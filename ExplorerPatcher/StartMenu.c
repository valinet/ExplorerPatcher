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
            if (bMonitorOverride)
            {
                goto finish;
            }

            /*DWORD dwStatus = 0;
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
            }*/

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
    HANDLE hEvent = CreateEventW(0, 0, 0, L"ShellDesktopSwitchEvent");
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
    HWND g_ProgWin = NULL;
    while (!g_ProgWin)
    {
        g_ProgWin = FindWindowEx(
            NULL,
            NULL,
            L"Progman",
            NULL
        );
        if (!g_ProgWin)
        {
            Sleep(100);
        }
    }
    printf("Progman: %d\n", g_ProgWin);
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
    printf("Progman hook: %d\n", g_ProgHook);
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

    if (!bOpenAtLogon)
    {
        return 0;
    }

    /*DWORD dwStatus = 0;
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
    }*/

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

DWORD WINAPI HookStartMenu(HookStartMenuParams* params)
{
    printf("Started \"Hook Start Menu\" thread.\n");

    TCHAR wszKnownPath[MAX_PATH];
    GetWindowsDirectoryW(wszKnownPath, MAX_PATH);
    wcscat_s(wszKnownPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy\\StartMenuExperienceHost.exe");

    while (TRUE)
    {
        unsigned int retry = 0;
        HANDLE hProcess, hSnapshot;
        PROCESSENTRY32 pe32;
        while (TRUE)
        {
            hProcess = NULL;
            hSnapshot = NULL;
            ZeroMemory(&pe32, sizeof(PROCESSENTRY32));
            pe32.dwSize = sizeof(PROCESSENTRY32);
            hSnapshot = CreateToolhelp32Snapshot(
                TH32CS_SNAPPROCESS,
                0
            );
            if (Process32First(hSnapshot, &pe32) == TRUE)
            {
                do
                {
                    if (!wcscmp(pe32.szExeFile, TEXT("StartMenuExperienceHost.exe")))
                    {
                        hProcess = OpenProcess(
                            PROCESS_QUERY_LIMITED_INFORMATION |
                            PROCESS_VM_OPERATION |
                            PROCESS_VM_READ |
                            PROCESS_VM_WRITE |
                            PROCESS_CREATE_THREAD |
                            SYNCHRONIZE,
                            FALSE,
                            pe32.th32ProcessID
                        );
                        if (!hProcess)
                        {
                            printf("Unable to open handle to StartMenuExperienceHost.exe.\n");
                            Sleep(params->dwTimeout);
                            continue;
                        }
                        TCHAR wszProcessPath[MAX_PATH];
                        DWORD dwLength = MAX_PATH;
                        QueryFullProcessImageNameW(
                            hProcess,
                            0,
                            wszProcessPath,
                            &dwLength
                        );
                        if (!_wcsicmp(wszProcessPath, wszKnownPath))
                        {
                            break;
                        }
                        else
                        {
                            CloseHandle(hProcess);
                            hProcess = NULL;
                        }
                    }
                } while (Process32Next(hSnapshot, &pe32) == TRUE);
            }
            CloseHandle(hSnapshot);
            if (hProcess)
            {
                break;
            }
            else
            {
                retry++;
                if (retry > 5) return 0;
                Sleep(params->dwTimeout);
            }
        }
        printf("[StartMenu] Process found.\n");
        LPVOID lpRemotePath = VirtualAllocEx(
            hProcess,
            NULL,
            MAX_PATH,
            MEM_COMMIT,
            PAGE_READWRITE
        );
        if (!lpRemotePath)
        {
            printf("[StartMenu] Unable to allocate path memory.\n");
            Sleep(1000);
            continue;
        }
        printf("[StartMenu] Allocated path memory.\n");
        if (!WriteProcessMemory(
            hProcess,
            lpRemotePath,
            (void*)params->wszModulePath,
            MAX_PATH,
            NULL
        ))
        {
            printf("[StartMenu] Unable to write path.\n");
            Sleep(params->dwTimeout);
            continue;
        }
        wprintf(L"[StartMenu] Wrote path: %s.\n", params->wszModulePath);
        HANDLE hThread = CreateRemoteThread(
            hProcess,
            NULL,
            0,
            LoadLibraryW,
            lpRemotePath,
            0,
            NULL
        );
        if (!hThread)
        {
            printf("[StartMenu] Unable to inject DLL.\n");
            Sleep(params->dwTimeout);
            continue;
        }
        printf("[StartMenu] Injected DLL.\n");
        if (WaitForSingleObject(hThread, INFINITE) != WAIT_OBJECT_0)
        {
            printf("[StartMenu] Unable to determine LoadLibrary outcome.\n");
            Sleep(params->dwTimeout);
            continue;
        }
        DWORD dwExitCode = 10;
        GetExitCodeThread(hThread, &dwExitCode);
        CloseHandle(hThread);
        printf("[StartMenu] Library loaded: 0x%x.\n", dwExitCode);
        DWORD cbNeeded = 0;
        EnumProcessModules(
            hProcess,
            NULL,
            0,
            &cbNeeded
        );
        if (!cbNeeded)
        {
            printf("[StartMenu] Unable to determine number of modules in process.\n");
            Sleep(params->dwTimeout);
            continue;
        }
        HMODULE* hMods = malloc(cbNeeded);
        if (!hMods)
        {
            printf("[StartMenu] Out of memory.\n");
            Sleep(params->dwTimeout);
            continue;
        }
        if (!EnumProcessModulesEx(
            hProcess,
            hMods,
            cbNeeded,
            &cbNeeded,
            LIST_MODULES_ALL
        ))
        {
            printf("[StartMenu] Unable to enumerate modules of process.\n");
            Sleep(params->dwTimeout);
            continue;
        }
        printf("[StartMenu] Remote module enumeration succeeded.\n");
        BOOL bFound = FALSE;
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i)
        {
            TCHAR szModName[MAX_PATH];
            if (GetModuleFileNameExW(hProcess, hMods[i], szModName,
                sizeof(szModName) / sizeof(TCHAR)))
            {
                if (!wcscmp(szModName, params->wszModulePath))
                {
                    printf("[StartMenu] Found module in process memory space.\n");
                    HANDLE hTh = CreateRemoteThread(
                        hProcess,
                        NULL,
                        0,
                        (uintptr_t)(hMods[i]) + ((uintptr_t)params->proc - (uintptr_t)params->hModule),
                        0,
                        0,
                        NULL
                    );
                    if (hTh)
                    {
                        printf("[StartMenu] Waiting for remote initialization.\n");
                        WaitForSingleObject(hTh, INFINITE);
                        DWORD dwExitCode = 0;
                        GetExitCodeThread(hTh, &dwExitCode);
                        printf("[StartMenu] Initialization exited with code 0x%x.\n", dwExitCode);
                        CloseHandle(hTh);
                        printf("[StartMenu] HOOKED START MENU\n");
                    }
                    bFound = TRUE;
                    break;
                }
            }
        }
        free(hMods);
        if (!bFound)
        {
            printf("[StartMenu] Remote module not found.\n");
            
        }
        WaitForSingleObject(
            hProcess,
            INFINITE
        );
        CloseHandle(hProcess);
    }
}
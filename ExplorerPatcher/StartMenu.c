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
            printf("Position Start\n");
            if (bMonitorOverride == 1)
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

            HMONITOR monitor = NULL;
            if (!bMonitorOverride)
            {
                DWORD pts = GetMessagePos();
                POINT pt;
                pt.x = GET_X_LPARAM(pts);
                pt.y = GET_Y_LPARAM(pts);
                printf("!! %d %d\n", pt.x, pt.y);
                monitor = MonitorFromPoint(
                    pt,
                    MONITOR_DEFAULTTONULL
                );
            }
            else
            {
                MonitorOverrideData mod;
                mod.cbIndex = 2;
                mod.dwIndex = bMonitorOverride;
                mod.hMonitor = NULL;
                EnumDisplayMonitors(NULL, NULL, ExtractMonitorByIndex, &mod);
                if (mod.hMonitor == NULL)
                {
                    POINT pt; pt.x = 0; pt.y = 0;
                    monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
                }
                else
                {
                    monitor = mod.hMonitor;
                }
            }
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
        g_ProgWin = GetShellWindow();
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
                            CloseHandle(hSnapshot);
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
            if (hSnapshot)
            {
                CloseHandle(hSnapshot);
            }
            if (hProcess)
            {
                break;
            }
            else
            {
                retry++;
                if (retry > 20) return 0;
                Sleep(params->dwTimeout);
            }
        }
        printf("[StartMenu] Process found.\n");
        LPVOID lpRemotePath = VirtualAllocEx(
            hProcess,
            NULL,
            MAX_PATH,
            MEM_COMMIT | MEM_RESERVE,
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
        //Sleep(8000);

        BYTE shellcode[] = {
            // sub rsp, 28h
            //// 0x48, 0x83, 0xec, 0x28,
            // mov [rsp + 18h], rax
            //// 0x48, 0x89, 0x44, 0x24, 0x18,
            // mov [rsp + 10h], rcx
            //// 0x48, 0x89, 0x4c, 0x24, 0x10,
            // int 3
            //0xcc,
            
            // sub rsp, 28h
            0x48, 0x83, 0xec, 0x28,
            // mov rcx, 1111111111111111h; placeholder for DLL path
            0x48, 0xb9, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
            // mov rax, 2222222222222222h; placeholder for "LoadLibraryW" address
            0x48, 0xb8, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
            // call rax
            0xff, 0xd0,
            // cmp rax, 0
            0x48, 0x83, 0xF8, 0x00,
            // jz; skip if LoadLibraryW failed
            0x74, 0x14,
            // mov rcx, 4444444444444444h; placeholder for entry point
            0x48, 0xb9, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
            // add rax, rcx
            0x48, 0x01, 0xc8,
            // call rax
            0xff, 0xd0,
            // add rsp, 28h
            0x48, 0x83, 0xc4, 0x28,
            // ret
            0xc3,
            // mov rax, 5555555555555555h; placeholder for "GetLastError" address
            0x48, 0xb8, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
            // call rax
            0xff, 0xd0,
            // add rsp, 28h
            0x48, 0x83, 0xc4, 0x28,
            // ret
            0xc3,

            // mov rcx, [rsp + 10h]
            //// 0x48, 0x8b, 0x4c, 0x24, 0x10,
            // mov rax, [rsp + 18h]
            //// 0x48, 0x8b, 0x44, 0x24, 0x18,
            // add rsp, 28h
            //// 0x48, 0x83, 0xc4, 0x28,
            // mov r11, 33333333333333333h; placeholder for the original RIP
            0x49, 0xbb, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
            // jmp r11
            0x41, 0xff, 0xe3
        };
        uintptr_t pattern = 0;
        pattern = 0x1111111111111111;
        *(LPVOID*)(memmem(shellcode, sizeof(shellcode), &pattern, sizeof(uintptr_t))) = lpRemotePath;
        pattern = 0x2222222222222222;
        *(LPVOID*)(memmem(shellcode, sizeof(shellcode), &pattern, sizeof(uintptr_t))) = LoadLibraryW;
        pattern = 0x4444444444444444;
        *(LPVOID*)(memmem(shellcode, sizeof(shellcode), &pattern, sizeof(uintptr_t))) = ((uintptr_t)params->proc - (uintptr_t)params->hModule);
        pattern = 0x5555555555555555;
        *(LPVOID*)(memmem(shellcode, sizeof(shellcode), &pattern, sizeof(uintptr_t))) = GetLastError;

        LPVOID lpRemoteCode = VirtualAllocEx(
            hProcess,
            NULL,
            sizeof(shellcode),
            MEM_COMMIT | MEM_RESERVE,
            PAGE_EXECUTE_READWRITE
        );
        if (!lpRemoteCode)
        {
            printf("[StartMenu] Unable to allocate shellcode memory.\n");
            Sleep(1000);
            continue;
        }
        printf("[StartMenu] Allocated shellcode memory %p.\n", lpRemoteCode);
        if (!WriteProcessMemory(
            hProcess,
            lpRemoteCode,
            shellcode,
            sizeof(shellcode),
            NULL
        ))
        {
            printf("[StartMenu] Unable to write shellcode.\n");
            Sleep(params->dwTimeout);
            continue;
        }
        wprintf(L"[StartMenu] Wrote shellcode.\n");

        wprintf(L"[StartMenu] Size of image: %d\n", RtlImageNtHeader(params->hModule)->OptionalHeader.SizeOfImage);

        HANDLE hThread = CreateRemoteThread(
            hProcess,
            NULL,
            0,
            lpRemoteCode,
            0,
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
        printf("[StartMenu] Library initialization returned: 0x%x.\n", dwExitCode);

        WaitForSingleObject(
            hProcess,
            INFINITE
        );
        CloseHandle(hProcess);
    }
}

static HRESULT STDMETHODCALLTYPE WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented(void* instanceof_WindowsUdk_UI_Shell_ITaskbarSettings)
{
    return E_NOTIMPL;
}

static ULONG STDMETHODCALLTYPE WindowsUdk_UI_Shell_ITaskbarSettings_AddRefRelease(void* instanceof_WindowsUdk_UI_Shell_ITaskbarSettings)
{
    return 1;
}

static HRESULT STDMETHODCALLTYPE WindowsUdk_UI_Shell_ITaskbarSettings6_GetEffectiveSearchMode(
    void* instanceof_WindowsUdk_UI_Shell_ITaskbarSettings6,
    DWORD* pEffectiveSearchMode
)
{
    *pEffectiveSearchMode = 1;
    return 0;
}

static void* instanceof_WindowsUdk_UI_Shell_ITaskbarSettings6Vtbl[41] = { // : IInspectableVtbl
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_AddRefRelease,
    WindowsUdk_UI_Shell_ITaskbarSettings_AddRefRelease,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings6_GetEffectiveSearchMode,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented
};
typedef struct instanceof_WindowsUdk_UI_Shell_ITaskbarSettings6 // : IInspectable
{
    void* lpVtbl;
} WindowsUdk_UI_Shell_ITaskbarSettings6;
static const WindowsUdk_UI_Shell_ITaskbarSettings6 instanceof_WindowsUdk_UI_Shell_ITaskbarSettings6 = { instanceof_WindowsUdk_UI_Shell_ITaskbarSettings6Vtbl };

static HRESULT STDMETHODCALLTYPE WindowsUdk_UI_Shell_ITaskbarSettings_QueryInterface(void* instanceof_WindowsUdk_UI_Shell_ITaskbarSettings, REFIID riid, void** ppv)
{
    if (IsEqualIID(riid, &IID_WindowsUdk_UI_Shell_ITaskbarSettings6)) {
        *ppv = &instanceof_WindowsUdk_UI_Shell_ITaskbarSettings6;
        return S_OK;
    }
    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE WindowsUdk_UI_Shell_ITaskbarSettings_GetAlignment_Left(
    void* instanceof_WindowsUdk_UI_Shell_ITaskbarSettings,
    DWORD* pAlignment
)
{
    *pAlignment = 0;
    return 0;
}

static HRESULT STDMETHODCALLTYPE WindowsUdk_UI_Shell_ITaskbarSettings_GetAlignment_Center(
    void* instanceof_WindowsUdk_UI_Shell_ITaskbarSettings,
    DWORD* pAlignment
)
{
    *pAlignment = 1;
    return 0;
}

static HRESULT STDMETHODCALLTYPE WindowsUdk_UI_Shell_ITaskbarSettings_GetLocation_Left(
    void* instanceof_WindowsUdk_UI_Shell_ITaskbarSettings,
    DWORD* pLocation
)
{
    *pLocation = 0;
    return 0;
}

static HRESULT STDMETHODCALLTYPE WindowsUdk_UI_Shell_ITaskbarSettings_GetLocation_Center(
    void* instanceof_WindowsUdk_UI_Shell_ITaskbarSettings,
    DWORD* pLocation
)
{
    *pLocation = 1;
    return 0;
}

static HRESULT STDMETHODCALLTYPE WindowsUdk_UI_Shell_ITaskbarSettings_GetSearchMode(
    void* instanceof_WindowsUdk_UI_Shell_ITaskbarSettings,
    DWORD* pSearchMode
)
{
    *pSearchMode = 1;
    return 0;
}

static void* instanceof_WindowsUdk_UI_Shell_ITaskbarSettingsVtbl[41] = { // : IInspectableVtbl
    WindowsUdk_UI_Shell_ITaskbarSettings_QueryInterface,
    WindowsUdk_UI_Shell_ITaskbarSettings_AddRefRelease,
    WindowsUdk_UI_Shell_ITaskbarSettings_AddRefRelease,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_GetAlignment_Left,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_GetLocation_Left,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_GetSearchMode,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented,
    WindowsUdk_UI_Shell_ITaskbarSettings_NotImplemented
};
typedef struct instanceof_WindowsUdk_UI_Shell_ITaskbarSettings // : IInspectable
{
    void* lpVtbl;
} WindowsUdk_UI_Shell_ITaskbarSettings;
static const WindowsUdk_UI_Shell_ITaskbarSettings instanceof_WindowsUdk_UI_Shell_ITaskbarSettings = { instanceof_WindowsUdk_UI_Shell_ITaskbarSettingsVtbl };

BOOL NeedsRo_PositionStartMenuForMonitor(
    HMONITOR hMonitor,
    HDC unused1,
    LPRECT unused2,
    StartMenuPositioningData* data
)
{
    // For this to have any chance to succeed, the Windows Runtime has to be
    // already initialized on the calling thread, concurrency model RO_INIT_MULTITHREADED

    HRESULT hr = S_OK;
    HSTRING_HEADER hstringHeader_of_WindowsUdk_UI_Shell_TaskbarLayout;
    HSTRING hstring_of_WindowsUdk_UI_Shell_TaskbarLayout = NULL;
    WindowsUdk_UI_Shell_TaskbarLayoutStatics* pTaskbarLayoutFactory = NULL;
    IInspectable* pTaskbarLayout = NULL;
    WindowsUdk_UI_Shell_TaskbarLayoutManager* pTaskbarLayoutManager = NULL;

    if (SUCCEEDED(hr))
    {
        hr = WindowsCreateStringReference(
            L"WindowsUdk.UI.Shell.TaskbarLayout",
            33,
            &hstringHeader_of_WindowsUdk_UI_Shell_TaskbarLayout,
            &hstring_of_WindowsUdk_UI_Shell_TaskbarLayout
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = RoGetActivationFactory(
            hstring_of_WindowsUdk_UI_Shell_TaskbarLayout,
            &IID_WindowsUdk_UI_Shell_TaskbarLayoutStatics,
            &pTaskbarLayoutFactory
        );
    }
    if (SUCCEEDED(hr))
    {
        //hr = (*(HRESULT(**)(INT64, INT64*))(*(INT64*)pTaskbarLayoutFactory + 48))(pTaskbarLayoutFactory, &v12);
        hr = pTaskbarLayoutFactory->lpVtbl->get_Current(pTaskbarLayoutFactory, &pTaskbarLayout);
    }
    int interfaceVersion = 1;
    if (SUCCEEDED(hr))
    {
        /*hr = (**(HRESULT(***)(INT64, GUID*, INT64*))v12)(
            v12,
            &IID_WindowsUdk_UI_Shell_ITaskbarLayoutManager,
            (INT64*)&v13
            );*/
        hr = pTaskbarLayout->lpVtbl->QueryInterface(
            pTaskbarLayout,
            &IID_WindowsUdk_UI_Shell_ITaskbarLayoutManager,
            &pTaskbarLayoutManager
        );
        if (hr == E_NOINTERFACE)
        {
            interfaceVersion = 2;
            hr = pTaskbarLayout->lpVtbl->QueryInterface(
                pTaskbarLayout,
                &IID_WindowsUdk_UI_Shell_ITaskbarLayoutManager2,
                &pTaskbarLayoutManager
            );
        }
    }
    if (SUCCEEDED(hr))
    {
        //hr = (*(HRESULT(**)(INT64, HMONITOR, INT64(***)(), INT64))(*v13 + 48 + (sizeof(uintptr_t) * data->operation)))(v13, hMonitor, &p, 0);

        // Filling a vtable for a winrt::WindowsUdk::UI::Shell::ITaskbarSettings interface
        // some info about it can be found in method:
        // > WindowsUdk::UI::Shell::Bamo::TaskbarLayoutBamoPrincipal::SetSettings
        // from windowsudk.shellcommon.dll
        // useful refrences: https://blog.magnusmontin.net/2017/12/30/minimal-uwp-wrl-xaml-app/
        // registry: HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\WindowsRuntime\ActivatableClassId\WindowsUdk.UI.Shell.TaskbarLayout
        if (data->location)
        {
            instanceof_WindowsUdk_UI_Shell_ITaskbarSettingsVtbl[6] = WindowsUdk_UI_Shell_ITaskbarSettings_GetAlignment_Center;
            instanceof_WindowsUdk_UI_Shell_ITaskbarSettingsVtbl[10] = WindowsUdk_UI_Shell_ITaskbarSettings_GetLocation_Center;
        }
        else
        {
            instanceof_WindowsUdk_UI_Shell_ITaskbarSettingsVtbl[6] = WindowsUdk_UI_Shell_ITaskbarSettings_GetAlignment_Left;
            instanceof_WindowsUdk_UI_Shell_ITaskbarSettingsVtbl[10] = WindowsUdk_UI_Shell_ITaskbarSettings_GetLocation_Left;
        }
        instanceof_WindowsUdk_UI_Shell_ITaskbarSettingsVtbl[14] = WindowsUdk_UI_Shell_ITaskbarSettings_GetSearchMode;

        if (data->operation == STARTMENU_POSITIONING_OPERATION_ADD)
        {
            if (interfaceVersion == 1)
            {
                hr = pTaskbarLayoutManager->lpVtbl->ReportMonitorAdded(
                    pTaskbarLayoutManager,
                    hMonitor,
                    &instanceof_WindowsUdk_UI_Shell_ITaskbarSettings,
                    NULL
                );
            }
            else
            {
                unsigned __int64 result = 0;
                hr = pTaskbarLayoutManager->lpVtbl->ReportMonitorAdded2(
                    pTaskbarLayoutManager,
                    hMonitor,
                    &instanceof_WindowsUdk_UI_Shell_ITaskbarSettings,
                    NULL,
                    &result
                );
            }
            data->pMonitorList[InterlockedIncrement(data->pMonitorCount) - 1] = hMonitor;
            printf("[Positioning] Added settings for monitor %p : %d\n", hMonitor, data->location);
        }
        else if (data->operation == STARTMENU_POSITIONING_OPERATION_CHANGE)
        {
            hr = pTaskbarLayoutManager->lpVtbl->ReportSettingsForMonitor(
                pTaskbarLayoutManager,
                hMonitor,
                &instanceof_WindowsUdk_UI_Shell_ITaskbarSettings
            ); // TODO Doesn't work when the 2nd interface is used. Needs further investigation
            printf("[Positioning] Changed settings for monitor: %p : %d\n", hMonitor, data->location);
        }
        else if (data->operation == STARTMENU_POSITIONING_OPERATION_REMOVE)
        {
            hr = pTaskbarLayoutManager->lpVtbl->ReportMonitorRemoved(
                pTaskbarLayoutManager,
                hMonitor
            );
            printf("[Positioning] Removed settings for monitor: %p\n", hMonitor);
        }
    }
    if (pTaskbarLayoutManager)
    {
        pTaskbarLayoutManager->lpVtbl->Release(pTaskbarLayoutManager);
        pTaskbarLayoutManager = NULL;
    }
    if (pTaskbarLayout)
    {
        pTaskbarLayout->lpVtbl->Release(pTaskbarLayout);
        pTaskbarLayout = NULL;
    }
    if (pTaskbarLayoutFactory)
    {
        pTaskbarLayoutFactory->lpVtbl->Release(pTaskbarLayoutFactory);
        pTaskbarLayoutFactory = NULL;
    }
    if (hstring_of_WindowsUdk_UI_Shell_TaskbarLayout)
    {
        WindowsDeleteString(hstring_of_WindowsUdk_UI_Shell_TaskbarLayout);
        hstring_of_WindowsUdk_UI_Shell_TaskbarLayout = NULL;
    }

    return TRUE;
}

DWORD GetStartMenuPosition(t_SHRegGetValueFromHKCUHKLM SHRegGetValueFromHKCUHKLMFunc)
{
    DWORD dwSize = sizeof(DWORD);

    DWORD dwTaskbarAl = 1;
    if (!SHRegGetValueFromHKCUHKLMFunc || SHRegGetValueFromHKCUHKLMFunc(
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
        TEXT("TaskbarAl"),
        SRRF_RT_REG_DWORD,
        NULL,
        &dwTaskbarAl,
        &dwSize
    ) != ERROR_SUCCESS)
    {
        dwTaskbarAl = 1;
    }

    return dwTaskbarAl;
}
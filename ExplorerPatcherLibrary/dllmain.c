#include <initguid.h>
#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#include <funchook.h>
#pragma comment(lib, "Psapi.lib") // required by funchook
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

#define DEBUG
#undef DEBUG

funchook_t* funchook = NULL;
HMODULE hModule = NULL;
HWND messageWindow = NULL;



static HWND(WINAPI* CreateWindowInBand)(
    _In_ DWORD dwExStyle, 
    _In_opt_ ATOM atom, 
    _In_opt_ LPCWSTR lpWindowName, 
    _In_ DWORD dwStyle, 
    _In_ int X, 
    _In_ int Y,
    _In_ int nWidth,
    _In_ int nHeight,
    _In_opt_ HWND hWndParent,
    _In_opt_ HMENU hMenu,
    _In_opt_ HINSTANCE hInstance,
    _In_opt_ LPVOID lpParam,
    DWORD band
    );



static INT64(*CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc)(
    void* _this,
    POINT* pt
    );

static void(*CLauncherTipContextMenu_ExecuteCommand)(
    void* _this,
    int a2
    );

static void(*CLauncherTipContextMenu_ExecuteShutdownCommand)(
    void* _this,
    void* a2
    );

static INT64(*InternalAddRef)(
    void* a1,
    INT64 a2
    );

static INT64(*ImmersiveContextMenuHelper_ApplyOwnerDrawToMenu)(
    HMENU h1,
    HMENU h2,
    HWND a3,
    unsigned int a4,
    void* data
    );

static void(*ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenu)(
    HMENU _this,
    HMENU hWnd,
    HWND a3
    );

static INT64(*CLauncherTipContextMenu_GetMenuItemsAsync)(
    void* _this,
    void* rect,
    void** iunk
    );

static INT64(*CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProc)(
    HWND hWnd,
    int a2,
    HWND a3,
    int a4,
    BOOL* a5
    );

DEFINE_GUID(IID_ILauncherTipContextMenu,
    0xb8c1db5f,
    0xcbb3, 0x48bc, 0xaf, 0xd9,
    0xce, 0x6b, 0x88, 0x0c, 0x79, 0xed
);

#define CLASS_NAME L"LauncherTipWnd"



static HRESULT(*CImmersiveHotkeyNotification_OnMessageFunc)(
    void* _this,
    INT64 msg,
    INT wParam,
    INT64 lParam
    );

static INT64(*CImmersiveHotkeyNotification_GetMonitorForHotkeyNotificationFunc)(
    void* _this,
    void** a2,
    HWND* a3
    );

static BOOL(*IsDesktopInputContextFunc)(
    void* p1,
    void* p2
    );






HANDLE hThread;

LRESULT CALLBACK CLauncherTipContextMenu_WndProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    LRESULT result;

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCs = lParam;
        if (pCs->lpCreateParams)
        {
            *((HWND*)((char*)pCs->lpCreateParams + 0x78)) = hWnd;
            SetWindowLongPtr(
                hWnd, 
                GWLP_USERDATA,
                pCs->lpCreateParams
            );
            result = DefWindowProc(
                hWnd,
                uMsg,
                wParam,
                lParam
            );
        }
        else
        {
            result = 0;
        }
    }
    else
    {
        void* _this = GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (_this)
        {
            BOOL v12 = FALSE;
            if ((uMsg == WM_DRAWITEM || uMsg == WM_MEASUREITEM) &&
                CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProc(
                    hWnd,
                    uMsg,
                    wParam,
                    lParam,
                    &v12
                ))
            {
                result = 0;
            }
            else
            {
                result = DefWindowProc(
                    hWnd,
                    uMsg,
                    wParam,
                    lParam
                );
            }
            if (uMsg == WM_NCDESTROY)
            {
                SetWindowLongPtrW(
                    hWnd, 
                    GWLP_USERDATA,
                    0
                );
                *((HWND*)((char*)_this + 0x78)) = 0;
            }
        }
        else
        {
            result = DefWindowProc(
                hWnd,
                uMsg,
                wParam,
                lParam
            );
        }
    }
    return result;
}

typedef struct
{
    void* _this;
    POINT point;
    IUnknown* iunk;
} ShowLauncherTipContextMenuParameters;

DWORD ShowLauncherTipContextMenu(
    ShowLauncherTipContextMenuParameters* params
)
{
    WNDCLASS wc = { 0 };
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = CLauncherTipContextMenu_WndProc;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClass(&wc);

    HWND hWnd = CreateWindowInBand(
        0,
        CLASS_NAME,
        0,
        WS_POPUP,
        0,
        0,
        0,
        0,
        0,
        0,
        GetModuleHandle(NULL),
        (char*)params->_this - 0x58,
        7
    );
    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);

    while (!(*((HMENU*)((char*)params->_this + 0xe8))))
    {
        Sleep(1);
    }
    if (!(*((HMENU*)((char*)params->_this + 0xe8))))
    {
        goto finalize;
    }

    INT64* unknown_array = calloc(4, sizeof(INT64));
    ImmersiveContextMenuHelper_ApplyOwnerDrawToMenu(
        *((HMENU*)((char*)params->_this + 0xe8)),
        hWnd,
        &(params->point),
        0xc,
        unknown_array
    );

    BOOL res = TrackPopupMenu(
        *((HMENU*)((char*)params->_this + 0xe8)),
        TPM_RETURNCMD,
        params->point.x,
        params->point.y,
        0,
        hWnd,
        0
    );

    ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenu(
        *((HMENU*)((char*)params->_this + 0xe8)),
        hWnd,
        &(params->point)
    );
    free(unknown_array);

    if (res > 0)
    {
        if (res < 4000)
        {
            INT64 info = *(INT64*)((char*)(*(INT64*)((char*)params->_this + 0xa8 - 0x58)) + (INT64)res * 8 - 8);
            CLauncherTipContextMenu_ExecuteCommand(
                (char*)params->_this - 0x58,
                &info
            );
        }
        else
        {
            INT64 info = *(INT64*)((char*)(*(INT64*)((char*)params->_this + 0xc8 - 0x58)) + ((INT64)res - 4000) * 8);
            CLauncherTipContextMenu_ExecuteShutdownCommand(
                (char*)params->_this - 0x58,
                &info
            );
        }
    }

    finalize:
    params->iunk->lpVtbl->Release(params->iunk);
    SendMessage(
        hWnd,
        WM_CLOSE,
        0,
        0
    );
    free(params);
    hThread = NULL;
    return 0;
}

INT64 CLauncherTipContextMenu_ShowLauncherTipContextMenuHook(
    void* _this,
    POINT* pt
)
{
    if (hThread)
    {
        goto finalize;
    }

    POINT point;
    if (pt)
    {
        point = *pt;
    }
    else
    {
        POINT ptCursor;
        GetCursorPos(&ptCursor);
        MONITORINFO mi;
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(
            MonitorFromPoint(
                ptCursor, 
                MONITOR_DEFAULTTONEAREST
            ), 
            &mi
        );
        // https://stackoverflow.com/questions/44746234/programatically-get-windows-taskbar-info-autohidden-state-taskbar-coordinates
        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
        if (abd.rc.left < 5 && abd.rc.top > 5)
        {
            // TB_POS_BOTTOM
            point.x = mi.rcMonitor.left;
            point.y = mi.rcMonitor.bottom;
        }
        else if (abd.rc.left < 5 && abd.rc.top < 5 && abd.rc.right > abd.rc.bottom)
        {
            // TB_POS_TOP
            point.x = mi.rcMonitor.left;
            point.y = mi.rcMonitor.top;
        }
        else if (abd.rc.left < 5 && abd.rc.top < 5 && abd.rc.right < abd.rc.bottom)
        {
            // TB_POS_LEFT
            point.x = mi.rcMonitor.left;
            point.y = mi.rcMonitor.top;
        }
        else if (abd.rc.left > 5 && abd.rc.top < 5)
        {
            // TB_POS_RIGHT
            point.x = mi.rcMonitor.right;
            point.y = mi.rcMonitor.top;
        }
    }

    IUnknown* iunk;
    INT64 r = CLauncherTipContextMenu_GetMenuItemsAsync(
        _this,
        &point,
        &iunk
    );
    iunk->lpVtbl->AddRef(iunk);
    
    ShowLauncherTipContextMenuParameters* params = malloc(
        sizeof(ShowLauncherTipContextMenuParameters)
    );
    params->_this = _this;
    params->point = point;
    params->iunk = iunk;
    hThread = CreateThread(
        0,
        0,
        ShowLauncherTipContextMenu,
        params,
        0,
        0
    );

    finalize:
    return CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc(_this, pt);
}

HRESULT CImmersiveHotkeyNotification_OnMessageHook(
    void* _this,
    INT64 msg,
    INT wParam,
    INT64 lParam
)
{
    if (wParam == 28 && IsDesktopInputContextFunc(_this, msg)) // 15
    {
        IUnknown* pMonitor;
        HRESULT hr = CImmersiveHotkeyNotification_GetMonitorForHotkeyNotificationFunc(
            (char*)_this - 0x68,
            &pMonitor,
            0
        );
        if (SUCCEEDED(hr))
        {
            IUnknown* pMenu;
            IUnknown_QueryService(
                pMonitor,
                &IID_ILauncherTipContextMenu,
                &IID_ILauncherTipContextMenu,
                &pMenu
            );
            if (pMenu)
            {
                CLauncherTipContextMenu_ShowLauncherTipContextMenuHook(
                    pMenu,
                    0
                );
                pMenu->lpVtbl->Release(pMenu);
            }
        }
        return 0;
    }

    return CImmersiveHotkeyNotification_OnMessageFunc(
        _this,
        msg,
        wParam,
        lParam
    );
}

__declspec(dllexport) DWORD WINAPI main(
    _In_ LPVOID lpParameter
)
{
#ifdef DEBUG
    FILE* conout;
    AllocConsole();
    freopen_s(
        &conout, 
        "CONOUT$",
        "w", 
        stdout
    );
#endif

    int rv;
    if (!funchook)
    {
        messageWindow = (HWND)lpParameter;


        funchook = funchook_create();



        HANDLE hUser32 = GetModuleHandle(L"user32.dll");

        if (hUser32) CreateWindowInBand = GetProcAddress(hUser32, "CreateWindowInBand");



        HANDLE hTwinuiPcshell = GetModuleHandle(L"twinui.pcshell.dll");

        CImmersiveContextMenuOwnerDrawHelper_s_ContextMenuWndProc = (INT64(*)(HWND, int, HWND, int, BOOL*))
            ((uintptr_t)hTwinuiPcshell + 0xB0E12);

        InternalAddRef = (INT64(*)(void*, INT64))
            ((uintptr_t)hTwinuiPcshell + 0x46650);

        CLauncherTipContextMenu_GetMenuItemsAsync = (INT64(*)(void*, void*, void**))
            ((uintptr_t)hTwinuiPcshell + 0x5051F0);
        
        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenu = (INT64(*)(HMENU, HMENU, HWND, unsigned int, void*))
            ((uintptr_t)hTwinuiPcshell + 0x535AF8);

        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenu = (void(*)(HMENU, HMENU, HWND))
            ((uintptr_t)hTwinuiPcshell + 0x536300);
        
        CLauncherTipContextMenu_ExecuteShutdownCommand = (void(*)(void*, void*))
            ((uintptr_t)hTwinuiPcshell + 0x514714);
        
        CLauncherTipContextMenu_ExecuteCommand = (void(*)(void*, int))
            ((uintptr_t)hTwinuiPcshell + 0x5143D0);
        
        CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc = (INT64(*)(void*, POINT*))
            ((uintptr_t)hTwinuiPcshell + 0x506EE0);
        rv = funchook_prepare(
            funchook,
            (void**)&CLauncherTipContextMenu_ShowLauncherTipContextMenuFunc,
            CLauncherTipContextMenu_ShowLauncherTipContextMenuHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }



        HANDLE hTwinui = GetModuleHandle(L"twinui.dll");

        CImmersiveHotkeyNotification_GetMonitorForHotkeyNotificationFunc = (INT64(*)(void*, void**, HWND*))
            ((uintptr_t)hTwinui + 0x24B4A8);

        IsDesktopInputContextFunc = (BOOL(*)(void*, void*))
            ((uintptr_t)hTwinui + 0x24A5C4);

        CImmersiveHotkeyNotification_OnMessageFunc = (HRESULT(*)(void*, INT64, INT, INT64))
            ((uintptr_t)hTwinui + 0xB2A70);
        rv = funchook_prepare(
            funchook,
            (void**)&CImmersiveHotkeyNotification_OnMessageFunc,
            CImmersiveHotkeyNotification_OnMessageHook
        );
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }



        rv = funchook_install(funchook, 0);
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }
    }
    else
    {
        rv = funchook_uninstall(funchook, 0);
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }

        rv = funchook_destroy(funchook);
        if (rv != 0)
        {
            FreeLibraryAndExitThread(hModule, rv);
            return rv;
        }

        FreeLibraryAndExitThread(hModule, 0);
    }

    return 0;
}

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        hModule = hinstDLL;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
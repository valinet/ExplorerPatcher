#include "ArchiveMenu.h"

DWORD ArchiveMenuThread(ArchiveMenuThreadParams* params)
{
    Sleep(1000);
    printf("Started \"Archive menu\" thread.\n");

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        return 0;
    }

    WNDCLASS wc = { 0 };
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = params->wndProc;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"ArchiveMenuWindowExplorer";
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClass(&wc);

    *(params->hWnd) = params->CreateWindowInBand(
        0,
        L"ArchiveMenuWindowExplorer",
        0,
        WS_POPUP,
        0,
        0,
        0,
        0,
        0,
        0,
        GetModuleHandle(NULL),
        NULL,
        7
    );
    if (!*(params->hWnd))
    {
        return 0;
    }
    ITaskbarList* pTaskList = NULL;
    hr = CoCreateInstance(
        &__uuidof_TaskbarList,
        NULL,
        CLSCTX_ALL,
        &__uuidof_ITaskbarList,
        (void**)(&pTaskList)
    );
    if (FAILED(hr))
    {
        return 0;
    }
    hr = pTaskList->lpVtbl->HrInit(pTaskList);
    if (FAILED(hr))
    {
        return 0;
    }
    ShowWindow(*(params->hWnd), SW_SHOW);
    hr = pTaskList->lpVtbl->DeleteTab(pTaskList, *(params->hWnd));
    if (FAILED(hr))
    {
        return 0;
    }
    hr = pTaskList->lpVtbl->Release(pTaskList);
    if (FAILED(hr))
    {
        return 0;
    }

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    printf("Ended \"Archive menu\" thread.\n");
}

LRESULT CALLBACK ArchiveMenuWndProc(
    _In_ HWND   hWnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    HRESULT(*ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc)(HMENU hMenu, HWND hWnd, POINT* pPt, unsigned int options, void* data),
    void(*ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc)(HMENU hMenu, HWND hWnd)
)
{
    LRESULT result;

    if (uMsg == WM_COPYDATA)
    {
        COPYDATASTRUCT* st = lParam;
        HWND srcWnd = wParam;

        POINT pt;
        GetCursorPos(&pt);

        HWND prevhWnd = GetForegroundWindow();
        SetForegroundWindow(hWnd);

        HMENU hMenu = CreatePopupMenu();

        TCHAR buffer[MAX_PATH + 100];
        TCHAR filename[MAX_PATH];
        ZeroMemory(filename, MAX_PATH * sizeof(TCHAR));
        memcpy(filename, st->lpData, wcslen(st->lpData) * sizeof(TCHAR));
        PathUnquoteSpacesW(filename);
        PathRemoveExtensionW(filename);
        PathStripPathW(filename);
        wsprintf(buffer, EXTRACT_NAME, filename);

        InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, 1, buffer);
        InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, 2, OPEN_NAME);

        INT64* unknown_array = calloc(4, sizeof(INT64));
        ImmersiveContextMenuHelper_ApplyOwnerDrawToMenuFunc(
            hMenu,
            hWnd,
            &(pt),
            0xc,
            unknown_array
        );

        BOOL res = TrackPopupMenu(
            hMenu,
            TPM_RETURNCMD,
            pt.x - 15,
            pt.y - 15,
            0,
            hWnd,
            0
        );

        ImmersiveContextMenuHelper_RemoveOwnerDrawFromMenuFunc(
            hMenu,
            hWnd
        );
        free(unknown_array);
        SetForegroundWindow(prevhWnd);

        if (res == 1 || res == 2)
        {
            ZeroMemory(buffer, (MAX_PATH + 100) * sizeof(TCHAR));
            if (res == 2)
            {
                wsprintf(buffer, OPEN_CMD, st->lpData);
                //wprintf(L"%s\n%s\n\n", st->lpData, buffer);
            }
            else if (res == 1)
            {
                TCHAR path[MAX_PATH + 1], path_orig[MAX_PATH + 1];
                ZeroMemory(path, (MAX_PATH + 1) * sizeof(TCHAR));
                ZeroMemory(path_orig, (MAX_PATH + 1) * sizeof(TCHAR));
                memcpy(path, st->lpData, wcslen(st->lpData) * sizeof(TCHAR));
                memcpy(path_orig, st->lpData, wcslen(st->lpData) * sizeof(TCHAR));
                PathUnquoteSpacesW(path_orig);
                PathRemoveExtensionW(path_orig);
                wsprintf(buffer, EXTRACT_CMD, path_orig, path);
                //wprintf(L"%s\n%s\n\n", st->lpData, buffer);
            }
            STARTUPINFO si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            BOOL b = CreateProcess(
                NULL,
                buffer,
                NULL,
                NULL,
                TRUE,
                CREATE_UNICODE_ENVIRONMENT,
                NULL,
                NULL,
                &si,
                &pi
            );
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        DestroyMenu(hMenu);
        ShowWindow(hWnd, SW_HIDE);
        return 0;
    }
    else if (uMsg == WM_CLOSE)
    {
        return 0;
    }
    return 1;
}

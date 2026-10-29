#include "StartSearchFix.h"
#include <tlhelp32.h>
#include <stdio.h>

#define MAX_BUFFERED_KEYS 64

typedef struct
{
    DWORD vk;
    DWORD scan;
} BufferedKey;

static HHOOK g_hHook = NULL;
static HANDLE g_hHookThread = NULL;
static DWORD g_dwHookThreadId = 0;

static volatile BOOL g_isTransitioning = FALSE;
static volatile BOOL g_searchActive = FALSE;
static DWORD g_transitionStartTick = 0;

static CRITICAL_SECTION g_csBuffer;
static BufferedKey g_keyBuffer[MAX_BUFFERED_KEYS];
static int g_keyBufferCount = 0;

// Process name cache
static DWORD g_lastCheckedPid = 0;
static WCHAR g_lastCheckedPName[MAX_PATH] = { 0 };
static CRITICAL_SECTION g_csPidCache;

static void BufferKey(DWORD vk, DWORD scan)
{
    EnterCriticalSection(&g_csBuffer);
    if (g_keyBufferCount < MAX_BUFFERED_KEYS)
    {
        g_keyBuffer[g_keyBufferCount].vk = vk;
        g_keyBuffer[g_keyBufferCount].scan = scan;
        g_keyBufferCount++;
    }
    LeaveCriticalSection(&g_csBuffer);
}

static int CopyAndClearBuffer(BufferedKey* dest, int maxCount)
{
    int count = 0;
    EnterCriticalSection(&g_csBuffer);
    count = (g_keyBufferCount < maxCount) ? g_keyBufferCount : maxCount;
    for (int i = 0; i < count; i++)
    {
        dest[i] = g_keyBuffer[i];
    }
    g_keyBufferCount = 0;
    LeaveCriticalSection(&g_csBuffer);
    return count;
}

static void GetCachedProcessName(DWORD pid, WCHAR* outName, DWORD cchMax)
{
    if (pid == 0 || !outName || cchMax == 0)
    {
        if (outName && cchMax > 0) outName[0] = L'\0';
        return;
    }

    EnterCriticalSection(&g_csPidCache);
    if (pid == g_lastCheckedPid && g_lastCheckedPName[0] != L'\0')
    {
        wcsncpy_s(outName, cchMax, g_lastCheckedPName, _TRUNCATE);
        LeaveCriticalSection(&g_csPidCache);
        return;
    }

    g_lastCheckedPid = pid;
    g_lastCheckedPName[0] = L'\0';

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(PROCESSENTRY32W);
        if (Process32FirstW(hSnapshot, &pe))
        {
            do
            {
                if (pe.th32ProcessID == pid)
                {
                    wcsncpy_s(g_lastCheckedPName, MAX_PATH, pe.szExeFile, _TRUNCATE);
                    break;
                }
            } while (Process32NextW(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
    }

    wcsncpy_s(outName, cchMax, g_lastCheckedPName, _TRUNCATE);
    LeaveCriticalSection(&g_csPidCache);
}

static BOOL IsTypingKey(DWORD vk)
{
    // A-Z
    if (vk >= 0x41 && vk <= 0x5A) return TRUE;
    // 0-9
    if (vk >= 0x30 && vk <= 0x39) return TRUE;
    // Numpad 0-9
    if (vk >= 0x60 && vk <= 0x69) return TRUE;
    // Space, Backspace
    if (vk == VK_SPACE || vk == VK_BACK) return TRUE;
    // OEM keys for international/Turkish layouts
    if (vk >= 0xBA && vk <= 0xC0) return TRUE;
    if (vk >= 0xDB && vk <= 0xDF) return TRUE;
    if (vk == 0xE2) return TRUE;

    return FALSE;
}

static DWORD WINAPI TransitionWorker(LPVOID lpParam)
{
    UNREFERENCED_PARAMETER(lpParam);

    // Step 1: Dismiss Start Menu cleanly via ESC
    keybd_event(VK_ESCAPE, 0x01, 0, 0);
    Sleep(20);
    keybd_event(VK_ESCAPE, 0x01, KEYEVENTF_KEYUP, 0);

    // Wait 180ms for StartMenuExperienceHost dismissal animation to finish completely
    Sleep(180);

    // Ensure Win key is released (Never touch Alt or Ctrl to prevent Menu Bar mode)
    keybd_event(VK_LWIN, 0x5B, KEYEVENTF_KEYUP, 0);
    Sleep(10);

    // Step 2: Send Win + S via clean hardware scan codes
    keybd_event(VK_LWIN, 0x5B, 0, 0);              // Win Down (scan 0x5B)
    Sleep(30);
    keybd_event(0x53, 0x1F, 0, 0);                 // S Down (scan 0x1F)
    Sleep(30);
    keybd_event(0x53, 0x1F, KEYEVENTF_KEYUP, 0);   // S Up
    Sleep(30);
    keybd_event(VK_LWIN, 0x5B, KEYEVENTF_KEYUP, 0);// Win Up
    Sleep(20);

    g_searchActive = TRUE;

    // Step 3: Wait 400ms for Search flyout animation and cursor focus
    Sleep(400);

    // Step 4: Replay buffered keys with hardware scan codes
    BufferedKey replayKeys[MAX_BUFFERED_KEYS];
    int count = CopyAndClearBuffer(replayKeys, MAX_BUFFERED_KEYS);

    for (int i = 0; i < count; i++)
    {
        BYTE scan = (replayKeys[i].scan != 0) ? (BYTE)replayKeys[i].scan : (BYTE)MapVirtualKeyW(replayKeys[i].vk, MAPVK_VK_TO_VSC);
        keybd_event((BYTE)replayKeys[i].vk, scan, 0, 0);
        Sleep(20);
        keybd_event((BYTE)replayKeys[i].vk, scan, KEYEVENTF_KEYUP, 0);
        Sleep(20);
    }

    g_isTransitioning = FALSE;
    return 0;
}

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        KBDLLHOOKSTRUCT* pKbd = (KBDLLHOOKSTRUCT*)lParam;

        // Injected keystrokes pass through untouched
        if (pKbd->flags & LLKHF_INJECTED)
        {
            return CallNextHookEx(g_hHook, nCode, wParam, lParam);
        }

        DWORD vk = pKbd->vkCode;
        BOOL isKeyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);

        // Track Windows Key
        if (vk == VK_LWIN || vk == VK_RWIN)
        {
            if (isKeyDown)
            {
                g_searchActive = FALSE;
                g_isTransitioning = FALSE;
            }
            return CallNextHookEx(g_hHook, nCode, wParam, lParam);
        }

        // Track ESC
        if (vk == VK_ESCAPE && isKeyDown)
        {
            g_searchActive = FALSE;
            g_isTransitioning = FALSE;
            return CallNextHookEx(g_hHook, nCode, wParam, lParam);
        }

        // Track Win + S (opened search directly)
        if (isKeyDown && vk == 0x53 && ((GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000)))
        {
            g_searchActive = TRUE;
            return CallNextHookEx(g_hHook, nCode, wParam, lParam);
        }

        // Modifiers (Ctrl or Alt) held down -> pass through (preserves Alt+Tab, Ctrl+C, etc.)
        if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000))
        {
            return CallNextHookEx(g_hHook, nCode, wParam, lParam);
        }

        // Typing keys
        if (isKeyDown && IsTypingKey(vk))
        {
            DWORD now = GetTickCount();

            // Safety watchdog: reset if transition takes longer than 1500ms
            if (g_isTransitioning && (now - g_transitionStartTick > 1500))
            {
                g_isTransitioning = FALSE;
            }

            // Buffer keys during transition
            if (g_isTransitioning)
            {
                BufferKey(vk, pKbd->scanCode);
                return 1; // Swallow
            }

            // Check foreground process
            HWND hwndFg = GetForegroundWindow();
            DWORD pid = 0;
            if (hwndFg) GetWindowThreadProcessId(hwndFg, &pid);

            WCHAR pName[MAX_PATH] = { 0 };
            GetCachedProcessName(pid, pName, MAX_PATH);

            BOOL isTarget = FALSE;
            if (_wcsicmp(pName, L"SearchHost.exe") == 0 ||
                _wcsicmp(pName, L"StartMenuExperienceHost.exe") == 0 ||
                _wcsicmp(pName, L"ShellExperienceHost.exe") == 0)
            {
                isTarget = TRUE;
            }
            else if (_wcsicmp(pName, L"explorer.exe") == 0)
            {
                WCHAR clsName[64] = { 0 };
                GetClassNameW(hwndFg, clsName, 64);
                isTarget = (_wcsicmp(clsName, L"ApplicationFrameWindow") == 0 ||
                            _wcsicmp(clsName, L"Windows.UI.Core.CoreWindow") == 0);
            }

            if (!isTarget)
            {
                g_searchActive = FALSE;
                return CallNextHookEx(g_hHook, nCode, wParam, lParam);
            }

            if (g_searchActive)
            {
                return CallNextHookEx(g_hHook, nCode, wParam, lParam);
            }

            // Begin transition
            g_isTransitioning = TRUE;
            g_transitionStartTick = now;

            EnterCriticalSection(&g_csBuffer);
            g_keyBufferCount = 0;
            g_keyBuffer[0].vk = vk;
            g_keyBuffer[0].scan = pKbd->scanCode;
            g_keyBufferCount = 1;
            LeaveCriticalSection(&g_csBuffer);

            HANDLE hThread = CreateThread(NULL, 0, TransitionWorker, NULL, 0, NULL);
            if (hThread) CloseHandle(hThread);

            return 1; // Swallow initial trigger key
        }
    }

    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

static DWORD WINAPI HookThreadProc(LPVOID lpParam)
{
    UNREFERENCED_PARAMETER(lpParam);

    g_hHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandleW(NULL), 0);
    if (!g_hHook)
    {
        return 1;
    }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_hHook)
    {
        UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
    }

    return 0;
}

void StartSearchFix_Init(void)
{
    InitializeCriticalSection(&g_csBuffer);
    InitializeCriticalSection(&g_csPidCache);

    g_hHookThread = CreateThread(NULL, 0, HookThreadProc, NULL, 0, &g_dwHookThreadId);
}

void StartSearchFix_Uninit(void)
{
    if (g_dwHookThreadId != 0)
    {
        PostThreadMessageW(g_dwHookThreadId, WM_QUIT, 0, 0);
    }

    if (g_hHookThread)
    {
        WaitForSingleObject(g_hHookThread, 2000);
        CloseHandle(g_hHookThread);
        g_hHookThread = NULL;
    }

    DeleteCriticalSection(&g_csBuffer);
    DeleteCriticalSection(&g_csPidCache);
}

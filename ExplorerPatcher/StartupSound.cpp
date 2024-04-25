#include "StartupSound.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "Winmm.lib")
#include <Wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")
#include <tchar.h>
#include <wrl/client.h>
#include <wil/result_macros.h>

#include "def.h"

BOOL AreLogonLogoffShutdownSoundsEnabled()
{
#if 0
    DWORD dwValue = 0;
    DWORD dwSize = sizeof(dwValue);
    RegGetValueW(HKEY_CURRENT_USER, _T(REGPATH), L"LogonLogoffShutdownSounds", RRF_RT_DWORD, nullptr, &dwValue, &dwSize);
    return dwValue != 0;
#else
    return FALSE;
#endif
}

DWORD GetLastErrorError()
{
    DWORD result = GetLastError();
    return result == ERROR_SUCCESS ? 1 : result;
}

HRESULT HRESULTFromLastErrorError()
{
    DWORD error = GetLastError();
    if (error != ERROR_SUCCESS && (int)error <= 0)
        return (HRESULT)GetLastErrorError();
    else
        return (HRESULT)((GetLastErrorError() & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000);
}

DWORD PlaySoundFileThreadProc(LPVOID pvData)
{
    PlaySoundW((LPCWSTR)pvData, nullptr, SND_NODEFAULT | SND_MEMORY | SND_SYSTEM);
    LocalFree(pvData);
    return 0;
}

HRESULT PlaySoundFile(HANDLE* phThread, const WCHAR* pszPath)
{
    HRESULT hr;

    void* pvData = nullptr;
    HANDLE hFile = CreateFileW(
        pszPath,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        nullptr
    );
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwSize = GetFileSize(hFile, nullptr);
        hr = E_OUTOFMEMORY;
        if (dwSize != (DWORD)-1 && dwSize)
        {
            if (dwSize < 0x400000)
            {
                pvData = LocalAlloc(0, dwSize);
                if (pvData)
                {
                    DWORD dwRead;
                    if (ReadFile(hFile, pvData, dwSize, &dwRead, nullptr))
                        hr = dwSize == dwRead ? S_OK : HRESULT_FROM_WIN32(ERROR_IO_PENDING);
                    else
                        hr = HRESULTFromLastErrorError();
                }
            }
        }
        else
        {
            hr = HRESULTFromLastErrorError();
        }
        CloseHandle(hFile);
    }
    else
    {
        hr = HRESULTFromLastErrorError();
    }
    if (SUCCEEDED(hr))
    {
        HANDLE hThread = CreateThread(nullptr, 0, PlaySoundFileThreadProc, pvData, 0, nullptr);
        if (hThread)
        {
            if (phThread)
                *phThread = hThread;
            else
                CloseHandle(hThread);
            return hr;
        }
        hr = HRESULTFromLastErrorError();
    }
    if (pvData)
        LocalFree(pvData);
    return hr;
}

typedef enum LOGONOFFSOUNDTYPE
{
    LOGONOFFSOUNDTYPE_LOGON,
    LOGONOFFSOUNDTYPE_LOGOFF,
    LOGONOFFSOUNDTYPE_EXIT,
} LOGONOFFSOUNDTYPE;

HRESULT PlayLogonLogoffSound(HANDLE* phThread, LOGONOFFSOUNDTYPE type)
{
    const WCHAR* szEventName;
    switch (type)
    {
        case LOGONOFFSOUNDTYPE_LOGON:
            szEventName = L"WindowsLogon";
            break;
        case LOGONOFFSOUNDTYPE_LOGOFF:
            szEventName = L"WindowsLogoff";
            break;
        default:
            szEventName = L"SystemExit";
            break;
    }

    WCHAR szSubKey[MAX_PATH];
    HRESULT hr = StringCchPrintfW(szSubKey, ARRAYSIZE(szSubKey), L"AppEvents\\Schemes\\Apps\\.Default\\%ws\\.Current", szEventName);
    if (FAILED(hr))
        return hr;

    WCHAR szPath[MAX_PATH];
    DWORD cbData = sizeof(szPath);
    LSTATUS lStat = RegGetValueW(HKEY_CURRENT_USER, szSubKey, nullptr, REG_EXPAND_SZ, nullptr, szPath, &cbData);
    if (lStat != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lStat);

    return PlaySoundFile(phThread, szPath);
}

// https://stackoverflow.com/a/59810748
bool IsSessionLocked()
{
    WTSINFOEXW* pInfo = NULL;
    WTS_INFO_CLASS wtsic = WTSSessionInfoEx;
    LPTSTR ppBuffer = NULL;
    DWORD dwBytesReturned = 0;
    LONG sessionFlags = WTS_SESSIONSTATE_UNKNOWN;

    DWORD dwSessionID = WTSGetActiveConsoleSessionId();

    if (WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, dwSessionID, wtsic, &ppBuffer, &dwBytesReturned))
    {
        if (dwBytesReturned > 0)
        {
            pInfo = (WTSINFOEXW*)ppBuffer;
            if (pInfo->Level == 1)
            {
                sessionFlags = pInfo->Data.WTSInfoExLevel1.SessionFlags;
            }
        }
        WTSFreeMemory(ppBuffer);
        ppBuffer = NULL;
    }

    return (sessionFlags == WTS_SESSIONSTATE_LOCK);
}

HRESULT (*CLogonSound_PlayIfNecessaryFunc)(void* _this, LOGON_SOUND_CLIENT client);
HRESULT CLogonSound_PlayIfNecessaryHook(void* _this, LOGON_SOUND_CLIENT client)
{
    HRESULT hr = CLogonSound_PlayIfNecessaryFunc(_this, client);
    if (hr != S_OK && client == LSC_EXPLORER)
    {
        if (!IsSessionLocked())
            PlayLogonLogoffSound(nullptr, LOGONOFFSOUNDTYPE_LOGON);
    }
    return hr;
}

HRESULT HookLogonSound()
{
    RETURN_IF_FAILED(CoInitialize(nullptr));

    Microsoft::WRL::ComPtr<IAuthUILogonSound> logonSound;
    RETURN_IF_FAILED(CoCreateInstance(__uuidof_AuthUILogonSound, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&logonSound)));

    void** vtable = *(void***)logonSound.Get();
    DWORD flOldProtect;
    RETURN_HR_IF(E_FAIL, !VirtualProtect(&vtable[3], sizeof(void*), PAGE_EXECUTE_READWRITE, &flOldProtect));

    CLogonSound_PlayIfNecessaryFunc = (decltype(CLogonSound_PlayIfNecessaryFunc))vtable[3];
    vtable[3] = (void*)CLogonSound_PlayIfNecessaryHook;
    VirtualProtect(&vtable[3], sizeof(void*), flOldProtect, &flOldProtect);

    return S_OK;
}

LRESULT SHDefWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (IsWindowUnicode(hwnd))
    {
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
}

HWND g_hwndSound;

class CSoundWnd
{
public:
    CSoundWnd();

    BOOL Init();
    DWORD Release();

protected:
    static LRESULT CALLBACK s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    static DWORD s_CreateWindow(void* pvParam);
    static DWORD s_ThreadProc(void* pvParam);

    LONG m_refCount;
    HWND m_hwnd;
    HANDLE m_thread;
};

CSoundWnd::CSoundWnd()
    : m_refCount(1)
    , m_hwnd(nullptr)
    , m_thread(nullptr)
{
}

BOOL CSoundWnd::Init()
{
    SHCreateThread(s_ThreadProc, this, CTF_THREAD_REF | CTF_COINIT_STA | CTF_REF_COUNTED | CTF_NOADDREFLIB, s_CreateWindow);
    g_hwndSound = m_hwnd;
    return m_hwnd != nullptr;
}

DWORD CSoundWnd::Release()
{
    LONG refCount = InterlockedDecrement(&m_refCount);
    if (refCount == 0 && this)
        operator delete(this);
    return refCount;
}

LRESULT CSoundWnd::s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CSoundWnd* pThis = (CSoundWnd*)GetWindowLongPtrW(hwnd, 0);
    if (pThis)
        return pThis->v_WndProc(hwnd, uMsg, wParam, lParam);
    else
        return SHDefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CSoundWnd::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_QUERYENDSESSION:
        {
            if ((lParam & ENDSESSION_CRITICAL) == 0)
            {
                WCHAR sz[256];
                LoadStringW(GetModuleHandleW(nullptr), 731, sz, ARRAYSIZE(sz)); // Playing logoff sound...
                ShutdownBlockReasonCreate(m_hwnd, sz);
                PlayLogonLogoffSound(&m_thread, (lParam & ENDSESSION_LOGOFF) != 0 ? LOGONOFFSOUNDTYPE_LOGOFF : LOGONOFFSOUNDTYPE_EXIT);
                if (m_thread)
                {
                    WaitForSingleObject(m_thread, INFINITE); // @MOD
                    CloseHandle(m_thread); // @MOD
                }
            }
            return 1;
        }
        case WM_ENDSESSION:
        {
            /*if (wParam && (lParam & ENDSESSION_CRITICAL) == 0 && m_thread) // @MOD This doesn't work
            {
                WaitForSingleObject(m_thread, INFINITE);
                CloseHandle(m_thread);
            }*/
            DestroyWindow(m_hwnd);
            break;
        }
        case WM_NCDESTROY:
        {
            SetWindowLongW(hwnd, 0, 0);
            g_hwndSound = nullptr;
            m_hwnd = nullptr;
            PostQuitMessage(0);
            return 0;
        }
    }
    return SHDefWindowProc(hwnd, uMsg, wParam, lParam);
}

extern "C" HWND (__stdcall *explorerframe_SHCreateWorkerWindowFunc)(
    WNDPROC wndProc,
    HWND hWndParent,
    DWORD dwExStyle,
    DWORD dwStyle,
    HMENU hMenu,
    LONG_PTR wnd_extra
);

DWORD CSoundWnd::s_CreateWindow(void* pvParam)
{
    CSoundWnd* pThis = (CSoundWnd*)pvParam;
    InterlockedIncrement(&pThis->m_refCount);
    pThis->m_hwnd = explorerframe_SHCreateWorkerWindowFunc(s_WndProc, nullptr, 0, 0, nullptr, (LONG_PTR)pThis);
    return 0;
}

DWORD CSoundWnd::s_ThreadProc(void* pvParam)
{
    CSoundWnd* pThis = (CSoundWnd*)pvParam;
    if (pThis->m_hwnd)
    {
        MSG Msg;
        while (GetMessageW(&Msg, nullptr, 0, 0))
        {
            TranslateMessage(&Msg);
            DispatchMessageW(&Msg);
        }
    }
    pThis->Release();
    return 0;
}

BOOL InitSoundWindow()
{
    BOOL bSuccess = FALSE;
    CSoundWnd* soundWnd = new CSoundWnd();
    if (soundWnd)
    {
        bSuccess = soundWnd->Init();
        soundWnd->Release();
    }
    return bSuccess;
}

void TermSoundWindow()
{
    if (g_hwndSound)
    {
        PostMessageW(g_hwndSound, WM_CLOSE, 0, 0);
        g_hwndSound = nullptr;
    }
}

HRESULT SHPlaySound(LPCWSTR pszSound, DWORD dwFlags)
{
    HRESULT hr;
    BOOL bDefault = (dwFlags & 1) != 0;
    BOOL bSecondAttempt = FALSE;
    while (true)
    {
        WCHAR szKey[MAX_PATH];
        hr = StringCchPrintfW(szKey, MAX_PATH, L"AppEvents\\Schemes\\Apps\\%s\\%s\\.current", bDefault ? L".Default" : L"Explorer", pszSound);
        if (SUCCEEDED(hr))
        {
            WCHAR pvData[MAX_PATH];
            DWORD cbData = sizeof(pvData);
            if (SHGetValueW(HKEY_CURRENT_USER, szKey, nullptr, nullptr, pvData, &cbData) == ERROR_SUCCESS && cbData && pvData[0])
                hr = PlaySoundW(pszSound, nullptr, (!bDefault ? 0x400000 : 0) | (SND_ASYNC | SND_NODEFAULT | SND_NOSTOP | SND_NOWAIT | SND_ALIAS | SND_SENTRY | SND_SYSTEM)) ? S_OK : S_FALSE;
        }
        if (hr == S_OK || (dwFlags & 2) == 0 || bSecondAttempt)
            break;
        bDefault = !bDefault;
        bSecondAttempt = TRUE;
    }
    return hr;
}

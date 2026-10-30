#include "utility.h"

#include <string>
#include <commctrl.h>
#include <uxtheme.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "comctl32.lib")

extern "C" HMODULE hModule;

#pragma region "Type in the Windows 10 taskbar search box"
/***
The Windows 11 search app (SearchHost.exe) still has a layout for when the search box is on the taskbar, like Windows 10
had: it does not show a search box at the top of its window, its window extends over the taskbar instead, and its text
box is placed over the search box on the taskbar. It picks this layout when WindowsUdk.UI.Shell.TaskbarLayout reports
TaskbarSearchMode::SearchBox and search box (ShellView::Search) bounds of at least 200x32, which we report for the
Windows 10 search box along with the Start menu positioning (see GetTaskbarSearchMode() in StartMenu.c).

In that layout, the search app draws a Windows 11 styled text box over the search box on the taskbar. To keep the
Windows 10 look, we load ourselves into the UI thread of the search app with a WH_GETMESSAGE hook whenever it comes to
the foreground, and make its text box invisible (it keeps the keyboard focus, so typing, keyboard navigation and IMEs
work as usual). Whenever its text or selection changes, the search app sends them to us, and the placeholder text
control of the Windows 10 search box draws them, along with a caret, instead of the placeholder text.
***/

#define EP_TASKBAR10SEARCHBOX_SYNC_CLASS L"EP_Taskbar10SearchBox"
#define EP_TASKBAR10SEARCHBOX_STATE_MAGIC 0x45505342 // 'EPSB'
// Posted to the sync window to apply the last received state to the taskbar
#define WM_EP_APPLYSEARCHBOXSTATE (WM_APP + 0x10)

// Sent by the search app to the sync window through WM_COPYDATA
struct SearchBoxState
{
    UINT64 hSearchBox; // TrayDummySearchControl that shows the text, 0 when the search app is not shown over it
    INT32 iSelectionStart;
    INT32 iSelectionEnd;
    WCHAR wszText[256];
};

namespace SearchApp
{
    using namespace winrt::Windows::UI::Xaml;
    using namespace winrt::Windows::UI::Xaml::Controls;
    using namespace winrt::Windows::UI::Xaml::Media;

    static FrameworkElement FindByName(DependencyObject const& obj, std::wstring_view name)
    {
        if (auto fe = obj.try_as<FrameworkElement>(); fe && fe.Name() == name)
        {
            return fe;
        }
        int count = VisualTreeHelper::GetChildrenCount(obj);
        for (int i = 0; i < count; i++)
        {
            if (auto result = FindByName(VisualTreeHelper::GetChild(obj, i), name))
            {
                return result;
            }
        }
        return nullptr;
    }

    // Finds the visible Windows 10 taskbar search box on the given monitor
    static HWND FindTaskbarSearchBox(HMONITOR hMonitor)
    {
        HWND hTray = FindWindowW(L"Shell_TrayWnd", nullptr);
        while (hTray)
        {
            HWND hBox = FindWindowExW(hTray, nullptr, L"TrayDummySearchControl", nullptr);
            if (hBox && IsWindowVisible(hBox) && MonitorFromWindow(hBox, MONITOR_DEFAULTTONULL) == hMonitor)
            {
                return hBox;
            }
            hTray = FindWindowExW(nullptr, hTray, L"Shell_SecondaryTrayWnd", nullptr);
        }
        return nullptr;
    }

    // Intentionally leaked, so that no XAML object is released while the search app is shutting down
    alignas(FrameworkElement) static unsigned char g_pageStorage[sizeof(FrameworkElement)];
    static FrameworkElement* g_pPage;
    alignas(RichEditBox) static unsigned char g_textBoxStorage[sizeof(RichEditBox)];
    static RichEditBox* g_pTextBox;
    static HWND g_hSearchBox; // The Windows 10 search box that shows our text, if any
    static bool g_bStartLayout; // Our search box was at the top, as when Start is open

    static void SendState()
    {
        HWND hSync = FindWindowW(EP_TASKBAR10SEARCHBOX_SYNC_CLASS, nullptr);
        if (!hSync)
        {
            return;
        }
        SearchBoxState state = {};
        state.hSearchBox = (UINT64)g_hSearchBox;
        if (g_hSearchBox)
        {
            auto document = g_pTextBox->Document();
            winrt::hstring text;
            document.GetText(winrt::Windows::UI::Text::TextGetOptions::None, text);
            std::wstring_view view = text;
            while (!view.empty() && (view.back() == L'\r' || view.back() == L'\n'))
            {
                view.remove_suffix(1);
            }
            wcsncpy_s(state.wszText, view.data(), min(view.size(), ARRAYSIZE(state.wszText) - 1));
            auto selection = document.Selection();
            state.iSelectionStart = min(selection.StartPosition(), (INT32)wcslen(state.wszText));
            state.iSelectionEnd = min(selection.EndPosition(), (INT32)wcslen(state.wszText));
            // Like on Windows 10, typing in Start goes to the search box on the taskbar, which is left alone until then
            if (g_bStartLayout && !state.wszText[0])
            {
                state.hSearchBox = 0;
            }
        }
        COPYDATASTRUCT cds;
        cds.dwData = EP_TASKBAR10SEARCHBOX_STATE_MAGIC;
        cds.cbData = sizeof(state);
        cds.lpData = &state;
        SendMessageTimeoutW(hSync, WM_COPYDATA, 0, (LPARAM)&cds, SMTO_ABORTIFHUNG, 200, nullptr);
    }

    static void TranslateTo(FrameworkElement const& element, double x, double y)
    {
        element.ClearValue(UIElement::RenderTransformProperty());
        element.UpdateLayout();
        auto origin = element.TransformToVisual(*g_pPage).TransformPoint({ 0, 0 });
        TranslateTransform transform;
        transform.X(x - origin.X);
        transform.Y(y - origin.Y);
        element.RenderTransform(transform);
    }

    static void Revert()
    {
        if (!g_hSearchBox)
        {
            return;
        }
        g_hSearchBox = nullptr;
        g_bStartLayout = false;
        if (auto background = FindByName(*g_pPage, L"TaskbarSearchBackground"))
        {
            background.ClearValue(UIElement::OpacityProperty());
            background.ClearValue(FrameworkElement::HeightProperty());
        }
        if (auto control = FindByName(*g_pPage, L"SearchBoxControl"))
        {
            control.ClearValue(UIElement::OpacityProperty());
            control.ClearValue(FrameworkElement::WidthProperty());
            control.ClearValue(FrameworkElement::HeightProperty());
            control.ClearValue(UIElement::RenderTransformProperty());
        }
    }

    static void Apply()
    {
        Revert();

        auto control = FindByName(*g_pPage, L"SearchBoxControl");
        if (!control)
        {
            return;
        }

        // In the layout where the search box is on the taskbar, our search box is at the bottom of the page; in the one for
        // Start (shown with Start, and when typing in it), it is at the top
        g_pPage->UpdateLayout();
        auto controlOrigin = control.TransformToVisual(*g_pPage).TransformPoint({ 0, 0 });
        bool bStartLayout = controlOrigin.Y < g_pPage->ActualHeight() / 2;

        auto bounds = Window::Current().Bounds();
        double scale = winrt::Windows::Graphics::Display::DisplayInformation::GetForCurrentView().RawPixelsPerViewPixel();
        RECT rcWindow = { (LONG)(bounds.X * scale), (LONG)(bounds.Y * scale), (LONG)((bounds.X + bounds.Width) * scale), (LONG)((bounds.Y + bounds.Height) * scale) };
        HWND hSearchBox = FindTaskbarSearchBox(MonitorFromRect(&rcWindow, MONITOR_DEFAULTTONEAREST));
        RECT rcSearchBox;
        if (!hSearchBox || !GetWindowRect(hSearchBox, &rcSearchBox))
        {
            return;
        }
        g_hSearchBox = hSearchBox;
        g_bStartLayout = bStartLayout;

        auto background = FindByName(*g_pPage, L"TaskbarSearchBackground");
        if (background)
        {
            background.Opacity(0);
        }
        control.Opacity(0);

        if (bStartLayout)
        {
            // Windows 10 had no search box in Start; collapse ours (it keeps the keyboard focus) so the rest moves up
            if (background)
            {
                background.Height(0);
            }
            control.Height(0);
            return;
        }

        // Keep the invisible text box over the Windows 10 search box, so clicking it places the caret, and IME windows
        // show up next to it
        control.Width((rcSearchBox.right - rcSearchBox.left) / scale);
        control.Height((rcSearchBox.bottom - rcSearchBox.top) / scale);
        TranslateTo(control, rcSearchBox.left / scale - bounds.X, rcSearchBox.top / scale - bounds.Y);
    }

    template <typename F>
    static void Safely(F&& f)
    {
        try
        {
            f();
        }
        catch (...)
        {
        }
    }

    static void Initialize()
    {
        auto window = Window::Current();
        if (!window || !window.Content() || winrt::get_class_name(window.Content()) != L"Cortana.UI.Views.TaskbarSearchPage")
        {
            return;
        }
        auto page = window.Content().as<FrameworkElement>();
        auto textBox = FindByName(page, L"SearchTextBox").try_as<RichEditBox>();
        if (!textBox)
        {
            return;
        }
        g_pPage = ::new (g_pageStorage) FrameworkElement(page);
        g_pTextBox = ::new (g_textBoxStorage) RichEditBox(textBox);

        // Our event handlers must outlive the hook, which goes away when explorer.exe exits
        IncrementDLLReferenceCount(hModule);

        textBox.TextChanged([](auto&&, auto&&) { Safely(SendState); });
        textBox.SelectionChanged([](auto&&, auto&&) { Safely(SendState); });
        window.VisibilityChanged([](auto&&, winrt::Windows::UI::Core::VisibilityChangedEventArgs const& args)
        {
            Safely([&]
            {
                if (args.Visible())
                {
                    Apply();
                }
                else
                {
                    Revert();
                }
                SendState();
            });
        });
        // The layout also changes while the window is shown, e.g. when going from Start to the search box on the taskbar
        if (auto rootGrid = FindByName(page, L"RootGrid"))
        {
            for (auto&& group : VisualStateManager::GetVisualStateGroups(rootGrid))
            {
                if (group.Name() == L"SearchBoxLocationStates")
                {
                    group.CurrentStateChanged([](auto&&, auto&&)
                    {
                        Safely([]
                        {
                            Apply();
                            SendState();
                        });
                    });
                }
            }
        }
        Apply();
        SendState();
    }
}

extern "C" LRESULT CALLBACK Taskbar10SearchBox_SearchAppGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    static thread_local bool t_bInitialized = false;
    if (nCode >= 0 && !t_bInitialized)
    {
        t_bInitialized = true;
        SearchApp::Safely(SearchApp::Initialize);
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

// The state last received from the search app; written by the sync window, read when painting the taskbar
static SRWLOCK g_stateLock = SRWLOCK_INIT;
static SearchBoxState g_state;
static HWND g_hShownSearchBox; // The search box whose placeholder control currently draws the state

#define CARET_TIMER_ID 0x45505342

static bool GetStateForPlaceholder(HWND hPlaceholder, SearchBoxState* pState)
{
    AcquireSRWLockShared(&g_stateLock);
    bool bResult = g_state.hSearchBox && (HWND)g_state.hSearchBox == GetParent(hPlaceholder);
    if (bResult)
    {
        *pState = g_state;
    }
    ReleaseSRWLockShared(&g_stateLock);
    return bResult;
}

static COLORREF GetSearchBoxTextColor()
{
    DWORD dwSystemUsesLightTheme = 0, dwSize = sizeof(DWORD);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"SystemUsesLightTheme", RRF_RT_REG_DWORD, nullptr, &dwSystemUsesLightTheme, &dwSize);
    return dwSystemUsesLightTheme ? RGB(0, 0, 0) : RGB(255, 255, 255);
}

static void DrawSearchBoxCaret(HDC hdc, int x, const RECT* prc, COLORREF color)
{
    TEXTMETRICW tm;
    GetTextMetricsW(hdc, &tm);
    RECT rcCaret = { x, prc->top + (prc->bottom - prc->top - tm.tmHeight) / 2, x + max(1, GetSystemMetrics(SM_CXBORDER)), 0 };
    rcCaret.bottom = rcCaret.top + tm.tmHeight;
    SetBkColor(hdc, color);
    ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rcCaret, nullptr, 0, nullptr);
}

static void PaintSearchBoxText(HWND hWnd, HDC hdc, const SearchBoxState* pState, bool bCaretVisible)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    DrawThemeParentBackground(hWnd, hdc, &rc);

    HFONT hOldFont = (HFONT)SelectObject(hdc, (HFONT)SendMessageW(hWnd, WM_GETFONT, 0, 0));
    COLORREF crText = GetSearchBoxTextColor();
    int cchText = (int)wcslen(pState->wszText);
    int iSelStart = max(0, min(pState->iSelectionStart, pState->iSelectionEnd));
    int iSelEnd = min(cchText, max(pState->iSelectionStart, pState->iSelectionEnd));

    // Keep the caret in view by scrolling the text to the left when it does not fit
    SIZE size = {};
    GetTextExtentPoint32W(hdc, pState->wszText, iSelEnd, &size);
    int xOffset = max(0, (int)size.cx - (int)(rc.right - rc.left) + 2);

    TEXTMETRICW tm;
    GetTextMetricsW(hdc, &tm);
    int y = rc.top + (rc.bottom - rc.top - tm.tmHeight) / 2;
    int x = rc.left - xOffset;
    SetBkMode(hdc, TRANSPARENT);
    for (int iPart = 0; iPart < 3; iPart++)
    {
        int iStart = iPart == 0 ? 0 : iPart == 1 ? iSelStart : iSelEnd;
        int iEnd = iPart == 0 ? iSelStart : iPart == 1 ? iSelEnd : cchText;
        if (iEnd <= iStart)
        {
            continue;
        }
        GetTextExtentPoint32W(hdc, pState->wszText + iStart, iEnd - iStart, &size);
        if (iPart == 1)
        {
            RECT rcSelection = { x, y, x + size.cx, y + tm.tmHeight };
            SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
            ExtTextOutW(hdc, 0, 0, ETO_OPAQUE | ETO_CLIPPED, &rcSelection, nullptr, 0, nullptr);
            SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
        }
        else
        {
            SetTextColor(hdc, crText);
        }
        ExtTextOutW(hdc, x, y, ETO_CLIPPED, &rc, pState->wszText + iStart, iEnd - iStart, nullptr);
        x += size.cx;
    }

    if (bCaretVisible && iSelStart == iSelEnd)
    {
        GetTextExtentPoint32W(hdc, pState->wszText, iSelEnd, &size);
        DrawSearchBoxCaret(hdc, rc.left - xOffset + size.cx, &rc, crText);
    }
    SelectObject(hdc, hOldFont);
}

static LRESULT CALLBACK PlaceholderSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    static bool s_bCaretVisible;
    static bool s_bCaretTimer;
    switch (uMsg)
    {
        case WM_PAINT:
        {
            SearchBoxState state;
            if (!GetStateForPlaceholder(hWnd, &state))
            {
                if (s_bCaretTimer)
                {
                    KillTimer(hWnd, CARET_TIMER_ID);
                    s_bCaretTimer = false;
                }
                break;
            }
            if (!s_bCaretTimer)
            {
                SetTimer(hWnd, CARET_TIMER_ID, GetCaretBlinkTime(), nullptr);
                s_bCaretTimer = true;
                s_bCaretVisible = true;
            }
            if (!state.wszText[0])
            {
                // Nothing typed yet: the usual placeholder text, with the caret before it
                LRESULT lRes = DefSubclassProc(hWnd, uMsg, wParam, lParam);
                if (s_bCaretVisible)
                {
                    HDC hdc = GetDC(hWnd);
                    RECT rc;
                    GetClientRect(hWnd, &rc);
                    HFONT hOldFont = (HFONT)SelectObject(hdc, (HFONT)SendMessageW(hWnd, WM_GETFONT, 0, 0));
                    DrawSearchBoxCaret(hdc, rc.left, &rc, GetSearchBoxTextColor());
                    SelectObject(hdc, hOldFont);
                    ReleaseDC(hWnd, hdc);
                }
                return lRes;
            }
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            HDC hdcBuffer = nullptr;
            HPAINTBUFFER hBuffer = BeginBufferedPaint(hdc, &ps.rcPaint, BPBF_TOPDOWNDIB, nullptr, &hdcBuffer);
            PaintSearchBoxText(hWnd, hBuffer ? hdcBuffer : hdc, &state, s_bCaretVisible);
            if (hBuffer)
            {
                EndBufferedPaint(hBuffer, TRUE);
            }
            EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_TIMER:
        {
            if (wParam == CARET_TIMER_ID)
            {
                s_bCaretVisible = !s_bCaretVisible;
                InvalidateRect(hWnd, nullptr, TRUE);
                return 0;
            }
            break;
        }
        case WM_NCDESTROY:
        {
            RemoveWindowSubclass(hWnd, PlaceholderSubclassProc, uIdSubclass);
            break;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// Subclassing has to happen on the thread of the window, which is the thread of the taskbar
static HWND g_hPlaceholderToSubclass;

static LRESULT CALLBACK SubclassPlaceholderCallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    const CWPSTRUCT* pcwp = (const CWPSTRUCT*)lParam;
    if (nCode >= 0 && pcwp->hwnd == g_hPlaceholderToSubclass && pcwp->message == WM_NULL)
    {
        if (SetWindowSubclass(pcwp->hwnd, PlaceholderSubclassProc, (UINT_PTR)PlaceholderSubclassProc, 0))
        {
            SetPropW(pcwp->hwnd, EP_TASKBAR10SEARCHBOX_SYNC_CLASS, (HANDLE)1);
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

static HWND GetSearchBoxPlaceholder(HWND hSearchBox)
{
    HWND hPlaceholder = hSearchBox && IsWindow(hSearchBox) ? FindWindowExW(hSearchBox, nullptr, L"Static", nullptr) : nullptr;
    if (hPlaceholder && !GetPropW(hPlaceholder, EP_TASKBAR10SEARCHBOX_SYNC_CLASS))
    {
        HHOOK hHook = SetWindowsHookExW(WH_CALLWNDPROC, SubclassPlaceholderCallWndProc, nullptr, GetWindowThreadProcessId(hPlaceholder, nullptr));
        if (hHook)
        {
            g_hPlaceholderToSubclass = hPlaceholder;
            SendMessageTimeoutW(hPlaceholder, WM_NULL, 0, 0, SMTO_BLOCK, 1000, nullptr);
            g_hPlaceholderToSubclass = nullptr;
            UnhookWindowsHookEx(hHook);
        }
    }
    return hPlaceholder;
}

// Repaints the placeholder controls whose content changed with the last received state
static void ApplySearchBoxState()
{
    AcquireSRWLockShared(&g_stateLock);
    HWND hSearchBox = (HWND)g_state.hSearchBox;
    ReleaseSRWLockShared(&g_stateLock);

    if (g_hShownSearchBox && g_hShownSearchBox != hSearchBox)
    {
        if (HWND hPlaceholder = GetSearchBoxPlaceholder(g_hShownSearchBox))
        {
            InvalidateRect(hPlaceholder, nullptr, TRUE);
        }
    }
    g_hShownSearchBox = hSearchBox;
    if (HWND hPlaceholder = GetSearchBoxPlaceholder(hSearchBox))
    {
        InvalidateRect(hPlaceholder, nullptr, TRUE);
    }
}

static DWORD g_rgdwHookedSearchAppThreads[8];

static bool IsSearchAppWindow(HWND hWnd)
{
    WCHAR wszClassName[64];
    if (!GetClassNameW(hWnd, wszClassName, ARRAYSIZE(wszClassName)) || wcscmp(wszClassName, L"Windows.UI.Core.CoreWindow"))
    {
        return false;
    }
    DWORD dwProcessId = 0;
    GetWindowThreadProcessId(hWnd, &dwProcessId);
    bool bResult = false;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    if (hProcess)
    {
        WCHAR wszPath[MAX_PATH];
        DWORD dwSize = ARRAYSIZE(wszPath);
        if (QueryFullProcessImageNameW(hProcess, 0, wszPath, &dwSize))
        {
            const WCHAR* pwszFileName = wcsrchr(wszPath, L'\\');
            bResult = pwszFileName && !_wcsicmp(pwszFileName + 1, L"SearchHost.exe");
        }
        CloseHandle(hProcess);
    }
    return bResult;
}

static HWND g_hSync;

static void CALLBACK SearchAppForegroundProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    if (!IsSearchAppWindow(hWnd))
    {
        // In case the search app went away without telling us
        AcquireSRWLockExclusive(&g_stateLock);
        g_state.hSearchBox = 0;
        ReleaseSRWLockExclusive(&g_stateLock);
        ApplySearchBoxState();
        return;
    }
    DWORD dwThreadId = GetWindowThreadProcessId(hWnd, nullptr);
    for (DWORD dwHookedThreadId : g_rgdwHookedSearchAppThreads)
    {
        if (dwHookedThreadId == dwThreadId)
        {
            return;
        }
    }
    if (SetWindowsHookExW(WH_GETMESSAGE, Taskbar10SearchBox_SearchAppGetMsgProc, hModule, dwThreadId))
    {
        memmove(g_rgdwHookedSearchAppThreads + 1, g_rgdwHookedSearchAppThreads, sizeof(g_rgdwHookedSearchAppThreads) - sizeof(DWORD));
        g_rgdwHookedSearchAppThreads[0] = dwThreadId;
        PostMessageW(hWnd, WM_NULL, 0, 0);
    }
}

static LRESULT CALLBACK SyncWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_COPYDATA)
    {
        const COPYDATASTRUCT* pcds = (const COPYDATASTRUCT*)lParam;
        if (pcds->dwData == EP_TASKBAR10SEARCHBOX_STATE_MAGIC && pcds->cbData == sizeof(SearchBoxState))
        {
            AcquireSRWLockExclusive(&g_stateLock);
            g_state = *(const SearchBoxState*)pcds->lpData;
            g_state.wszText[ARRAYSIZE(g_state.wszText) - 1] = 0;
            ReleaseSRWLockExclusive(&g_stateLock);
            // Don't make the search app wait for the taskbar
            PostMessageW(hWnd, WM_EP_APPLYSEARCHBOXSTATE, 0, 0);
            return TRUE;
        }
        return FALSE;
    }
    if (uMsg == WM_EP_APPLYSEARCHBOXSTATE)
    {
        ApplySearchBoxState();
        return 0;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static DWORD WINAPI SearchAppWatcherThread(LPVOID lpParameter)
{
    WNDCLASSW wc = {};
    wc.lpfnWndProc = SyncWindowProc;
    wc.hInstance = hModule;
    wc.lpszClassName = EP_TASKBAR10SEARCHBOX_SYNC_CLASS;
    RegisterClassW(&wc);
    g_hSync = CreateWindowExW(0, EP_TASKBAR10SEARCHBOX_SYNC_CLASS, nullptr, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, hModule, nullptr);
    if (!g_hSync)
    {
        return 0;
    }
    // The search app runs in an app container
    ChangeWindowMessageFilterEx(g_hSync, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
    SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, SearchAppForegroundProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

extern "C" void Taskbar10SearchBox_Initialize()
{
    HANDLE hThread = CreateThread(nullptr, 0, SearchAppWatcherThread, nullptr, 0, nullptr);
    if (hThread)
    {
        CloseHandle(hThread);
    }
}
#pragma endregion

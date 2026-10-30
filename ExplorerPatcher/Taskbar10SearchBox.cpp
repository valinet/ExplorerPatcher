#include "utility.h"

#include <string>
#include <commctrl.h>
#include <corewindow.h>
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

#pragma region "Type in the search box of the Windows 10 (ExplorerPatcher) taskbar"
/***
On Windows 10, you typed in the search box on the taskbar, and the search results showed up in a panel attached right
above it. The Windows 11 search app (SearchHost.exe) shows its own search box at the top of a floating panel instead.

It still has a layout for when the search box is on the taskbar, though: no search box at the top of the panel, and its
text box placed over the search box on the taskbar. It uses that layout when it is told that the taskbar shows a search
box (see GetTaskbarSearchMode() in StartMenu.c).

That text box is styled like the one of the Windows 11 taskbar, so whenever the search app comes to the foreground, we
load ourselves into it (with a WH_GETMESSAGE hook on its UI thread) and:
- make its search box invisible; it keeps the keyboard focus, so typing, keyboard navigation and IMEs work as usual,
- attach its panel to the taskbar, like on Windows 10, instead of letting it float above it,
- send whatever is typed to explorer.exe, where the search box on the taskbar draws it instead of its placeholder text.

The search app also shows its search box at the top of Start, and keeps it there when you type in Start. As Windows 10
had no search box in Start, we collapse it as well, and what is typed in Start shows up in the search box on the taskbar.
***/

// A hidden window in explorer.exe that receives the typed text from the search app
#define TYPED_TEXT_RECEIVER_CLASS L"EP_TaskbarSearchBoxTypedTextReceiver"
// Identifies our WM_COPYDATA messages ('EPSB')
#define TYPED_TEXT_COPYDATA_ID 0x45505342
// Posted to the receiver window to redraw the search box on the taskbar with the latest typed text
#define WM_EP_REDRAW_TASKBAR_SEARCH_BOX (WM_APP + 0x10)

// What the search app sends to explorer.exe whenever its text or selection changes
struct TypedText
{
    UINT64 hTaskbarSearchBox; // The search box on the taskbar that shows the text; 0 to show its placeholder text again
    INT32 selectionStart;
    INT32 selectionEnd;
    WCHAR text[256];
};

#pragma region "Inside the search app"
namespace InsideSearchApp
{
    using namespace winrt::Windows::UI::Xaml;
    using namespace winrt::Windows::UI::Xaml::Controls;
    using namespace winrt::Windows::UI::Xaml::Media;

    // Names of the elements of the search app's page (Cortana.UI.Views.TaskbarSearchPage) that we change
    static const wchar_t* const c_rgPanelElements[] = { L"OuterBorderGrid", L"QueryFormulationRoot", L"HCOuterBorderGrid" };
    static const wchar_t* const c_rgPanelBorders[] = { L"AppBorder", L"LayerBorder", L"HCBorder", L"AccentAppBorder", L"AccentLayerBorder" };

    // Intentionally never released, so that no XAML object is released while the search app is shutting down
    alignas(FrameworkElement) static unsigned char g_searchPageStorage[sizeof(FrameworkElement)];
    static FrameworkElement* g_pSearchPage;
    alignas(RichEditBox) static unsigned char g_searchTextBoxStorage[sizeof(RichEditBox)];
    static RichEditBox* g_pSearchTextBox;
    // The visible part of the panel is AppBorder, which has a margin around it inside the window
    alignas(FrameworkElement) static unsigned char g_panelStorage[sizeof(FrameworkElement)];
    static FrameworkElement* g_pPanel;

    static HWND g_hTaskbarSearchBox; // The search box on the taskbar that shows what is typed, if any
    static bool g_bOpenedWithStart; // The search app is shown with Start, with its search box at the top
    static bool g_bChangingPage; // Our changes to the page raise some of the events we handle; we ignore those

    static FrameworkElement FindElement(DependencyObject const& parent, std::wstring_view name)
    {
        if (auto element = parent.try_as<FrameworkElement>(); element && element.Name() == name)
        {
            return element;
        }
        int count = VisualTreeHelper::GetChildrenCount(parent);
        for (int i = 0; i < count; i++)
        {
            if (auto result = FindElement(VisualTreeHelper::GetChild(parent, i), name))
            {
                return result;
            }
        }
        return nullptr;
    }

    // The visible search box of the Windows 10 taskbar on the given monitor, if any
    static HWND FindTaskbarSearchBox(HMONITOR hMonitor)
    {
        HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
        while (hTaskbar)
        {
            HWND hSearchBox = FindWindowExW(hTaskbar, nullptr, L"TrayDummySearchControl", nullptr);
            if (hSearchBox && IsWindowVisible(hSearchBox) && MonitorFromWindow(hSearchBox, MONITOR_DEFAULTTONULL) == hMonitor)
            {
                return hSearchBox;
            }
            hTaskbar = FindWindowExW(nullptr, hTaskbar, L"Shell_SecondaryTrayWnd", nullptr);
        }
        return nullptr;
    }

    static HWND GetSearchAppWindow()
    {
        HWND hWindow = nullptr;
        winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread().as<ICoreWindowInterop>()->get_WindowHandle(&hWindow);
        return hWindow;
    }

    // Converts screen pixels into the units of the page
    static double GetPixelsPerPageUnit()
    {
        return winrt::Windows::Graphics::Display::DisplayInformation::GetForCurrentView().RawPixelsPerViewPixel();
    }

    // Exceptions must not leave our XAML event handlers
    template <typename F>
    static void IgnoringErrors(F&& f)
    {
        try
        {
            f();
        }
        catch (...)
        {
        }
    }

    template <typename F>
    static void ChangePage(F&& f)
    {
        if (g_bChangingPage)
        {
            return;
        }
        g_bChangingPage = true;
        IgnoringErrors(f);
        g_bChangingPage = false;
    }

    static void SendTypedTextToExplorer()
    {
        HWND hReceiver = FindWindowW(TYPED_TEXT_RECEIVER_CLASS, nullptr);
        if (!hReceiver)
        {
            return;
        }
        TypedText typedText = {};
        typedText.hTaskbarSearchBox = (UINT64)g_hTaskbarSearchBox;
        if (g_hTaskbarSearchBox)
        {
            auto document = g_pSearchTextBox->Document();
            winrt::hstring text;
            document.GetText(winrt::Windows::UI::Text::TextGetOptions::None, text);
            std::wstring_view textView = text;
            while (!textView.empty() && (textView.back() == L'\r' || textView.back() == L'\n'))
            {
                textView.remove_suffix(1);
            }
            wcsncpy_s(typedText.text, textView.data(), min(textView.size(), ARRAYSIZE(typedText.text) - 1));
            INT32 textLength = (INT32)wcslen(typedText.text);
            auto selection = document.Selection();
            typedText.selectionStart = min(selection.StartPosition(), textLength);
            typedText.selectionEnd = min(selection.EndPosition(), textLength);
            // Like on Windows 10, Start leaves the search box on the taskbar alone until you type something
            if (g_bOpenedWithStart && !typedText.text[0])
            {
                typedText.hTaskbarSearchBox = 0;
            }
        }
        COPYDATASTRUCT cds;
        cds.dwData = TYPED_TEXT_COPYDATA_ID;
        cds.cbData = sizeof(typedText);
        cds.lpData = &typedText;
        SendMessageTimeoutW(hReceiver, WM_COPYDATA, 0, (LPARAM)&cds, SMTO_ABORTIFHUNG, 200, nullptr);
    }

    // Moves an element to the given position on the page, without changing its layout
    static void MoveElementTo(FrameworkElement const& element, double x, double y)
    {
        element.ClearValue(UIElement::RenderTransformProperty());
        element.UpdateLayout();
        auto position = element.TransformToVisual(*g_pSearchPage).TransformPoint({ 0, 0 });
        TranslateTransform offset;
        offset.X(x - position.X);
        offset.Y(y - position.Y);
        element.RenderTransform(offset);
    }

    static void UndoChanges()
    {
        if (!g_hTaskbarSearchBox)
        {
            return;
        }
        g_hTaskbarSearchBox = nullptr;
        g_bOpenedWithStart = false;
        if (auto background = FindElement(*g_pSearchPage, L"TaskbarSearchBackground"))
        {
            background.ClearValue(UIElement::OpacityProperty());
            background.ClearValue(FrameworkElement::HeightProperty());
        }
        if (auto searchBox = FindElement(*g_pSearchPage, L"SearchBoxControl"))
        {
            searchBox.ClearValue(UIElement::OpacityProperty());
            searchBox.ClearValue(FrameworkElement::WidthProperty());
            searchBox.ClearValue(FrameworkElement::HeightProperty());
            searchBox.ClearValue(UIElement::RenderTransformProperty());
        }
        for (const wchar_t* name : c_rgPanelElements)
        {
            if (auto element = FindElement(*g_pSearchPage, name))
            {
                element.ClearValue(UIElement::RenderTransformProperty());
            }
        }
        for (const wchar_t* name : c_rgPanelBorders)
        {
            if (auto border = FindElement(*g_pSearchPage, name))
            {
                border.ClearValue(Border::CornerRadiusProperty());
            }
        }
        if (auto shadow = FindElement(*g_pSearchPage, L"dropshadow"))
        {
            shadow.ClearValue(UIElement::OpacityProperty());
        }
    }

    // Where the bottom left corner of the panel goes on the page: right on top of the taskbar, lined up with the left edge of
    // the search box on the taskbar
    static bool GetPanelBottomLeftOnTaskbar(winrt::Windows::Foundation::Point& bottomLeft)
    {
        RECT rcWindow, rcTaskbar, rcTaskbarSearchBox;
        if (!g_pPanel || !g_hTaskbarSearchBox || !GetWindowRect(GetSearchAppWindow(), &rcWindow) || !GetWindowRect(GetAncestor(g_hTaskbarSearchBox, GA_ROOT), &rcTaskbar) || !GetWindowRect(g_hTaskbarSearchBox, &rcTaskbarSearchBox))
        {
            return false;
        }
        double scale = GetPixelsPerPageUnit();
        bottomLeft = { (float)((rcTaskbarSearchBox.left - rcWindow.left) / scale), (float)((rcTaskbar.top - rcWindow.top) / scale) };
        return true;
    }

    // While the search app is still laying itself out, the panel has no size yet; we come back when it does
    static bool HasPanelBeenLaidOut()
    {
        return g_pPanel->ActualHeight() >= 100;
    }

    // The search app moves the panel around as it shows up, without always resizing it
    static bool HasPanelMovedOffTaskbar()
    {
        winrt::Windows::Foundation::Point bottomLeft;
        if (!GetPanelBottomLeftOnTaskbar(bottomLeft) || !HasPanelBeenLaidOut())
        {
            return false;
        }
        auto position = g_pPanel->TransformToVisual(*g_pSearchPage).TransformPoint({ 0, (float)g_pPanel->ActualHeight() });
        return fabs(position.X - bottomLeft.X) > 0.5 || fabs(position.Y - bottomLeft.Y) > 0.5;
    }

    // Moves the panel right on top of the taskbar, lined up with the left edge of the search box on the taskbar, with square
    // corners and no shadow
    static void AttachPanelToTaskbar()
    {
        winrt::Windows::Foundation::Point bottomLeft;
        if (!GetPanelBottomLeftOnTaskbar(bottomLeft))
        {
            return;
        }
        g_pSearchPage->UpdateLayout();
        if (!HasPanelBeenLaidOut())
        {
            return;
        }
        for (const wchar_t* name : c_rgPanelElements)
        {
            if (auto element = FindElement(*g_pSearchPage, name))
            {
                element.ClearValue(UIElement::RenderTransformProperty());
            }
        }
        g_pSearchPage->UpdateLayout();
        auto panelPosition = g_pPanel->TransformToVisual(*g_pSearchPage).TransformPoint({ 0, 0 });
        TranslateTransform offset;
        offset.X(bottomLeft.X - panelPosition.X);
        offset.Y(bottomLeft.Y - (panelPosition.Y + g_pPanel->ActualHeight()));
        for (const wchar_t* name : c_rgPanelElements)
        {
            if (auto element = FindElement(*g_pSearchPage, name))
            {
                element.RenderTransform(offset);
            }
        }

        for (const wchar_t* name : c_rgPanelBorders)
        {
            if (auto border = FindElement(*g_pSearchPage, name).try_as<Border>())
            {
                border.CornerRadius({ 0, 0, 0, 0 });
            }
        }
        if (auto shadow = FindElement(*g_pSearchPage, L"dropshadow"))
        {
            shadow.Opacity(0);
        }
    }

    // As it shows up, the search app moves the panel without even laying it out again, so for a moment we check where the
    // panel is before every frame
    static void KeepPanelAttachedForAMoment()
    {
        static winrt::event_token s_renderingToken;
        static ULONGLONG s_stopTime;
        s_stopTime = GetTickCount64() + 1000;
        if (s_renderingToken)
        {
            return;
        }
        s_renderingToken = CompositionTarget::Rendering([](auto&&, auto&&)
        {
            ChangePage([]
            {
                if (HasPanelMovedOffTaskbar())
                {
                    AttachPanelToTaskbar();
                }
            });
            if (GetTickCount64() >= s_stopTime || !g_hTaskbarSearchBox)
            {
                IgnoringErrors([] { CompositionTarget::Rendering(s_renderingToken); });
                s_renderingToken = {};
            }
        });
    }

    static void ApplyChanges()
    {
        UndoChanges();

        auto searchBox = FindElement(*g_pSearchPage, L"SearchBoxControl");
        if (!searchBox)
        {
            return;
        }

        // Only while the Windows 10 taskbar shows a search box on the monitor of the search app
        HWND hWindow = GetSearchAppWindow();
        HMONITOR hMonitor = MonitorFromWindow(hWindow, MONITOR_DEFAULTTONEAREST);
        HWND hTaskbarSearchBox = FindTaskbarSearchBox(hMonitor);
        RECT rcTaskbarSearchBox;
        if (!hTaskbarSearchBox || !GetWindowRect(hTaskbarSearchBox, &rcTaskbarSearchBox))
        {
            return;
        }
        g_hTaskbarSearchBox = hTaskbarSearchBox;

        // When the search app is shown with Start, its search box is at the top of the page; when it is shown for the
        // search box on the taskbar, it is at the bottom
        g_pSearchPage->UpdateLayout();
        g_bOpenedWithStart = searchBox.TransformToVisual(*g_pSearchPage).TransformPoint({ 0, 0 }).Y < g_pSearchPage->ActualHeight() / 2;

        auto background = FindElement(*g_pSearchPage, L"TaskbarSearchBackground");
        if (background)
        {
            background.Opacity(0);
        }
        searchBox.Opacity(0);
        if (g_bOpenedWithStart)
        {
            // Collapse it, so that the rest of the panel moves up
            if (background)
            {
                background.Height(0);
            }
            searchBox.Height(0);
        }
        else
        {
            // Keep the invisible search box over the one on the taskbar, so that clicking it places the caret, and IME
            // windows show up next to it
            RECT rcWindow;
            GetWindowRect(hWindow, &rcWindow);
            double scale = GetPixelsPerPageUnit();
            searchBox.Width((rcTaskbarSearchBox.right - rcTaskbarSearchBox.left) / scale);
            searchBox.Height((rcTaskbarSearchBox.bottom - rcTaskbarSearchBox.top) / scale);
            MoveElementTo(searchBox, (rcTaskbarSearchBox.left - rcWindow.left) / scale, (rcTaskbarSearchBox.top - rcWindow.top) / scale);
        }

        // Last, as resizing the search box changes the height of the panel
        AttachPanelToTaskbar();
        KeepPanelAttachedForAMoment();
    }

    // Our changes are placed relative to the search app's window, so we redo them after it moves
    static void ApplyChangesAgainSoon()
    {
        static bool s_bPending;
        if (s_bPending)
        {
            return;
        }
        winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread().Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::High, []
        {
            s_bPending = false;
            ChangePage(ApplyChanges);
            IgnoringErrors(SendTypedTextToExplorer);
        });
        s_bPending = true;
    }

    // The search app places its window at the left edge of the screen when it is shown with Start, and a little to the left
    // of the search box on the taskbar otherwise. We move it to the left edge of the search box, so that the panel lines up
    // with it without being cut off.
    // Moving it while the search app is still showing it makes it lose the keyboard focus, so we wait until it is in the
    // foreground.
    static void LineUpWindowWithTaskbarSearchBoxSoon()
    {
        static bool s_bWaiting;
        static int s_checksLeft;
        s_checksLeft = 20;
        if (s_bWaiting)
        {
            return;
        }
        DispatcherTimer timer;
        timer.Interval(std::chrono::milliseconds(50));
        timer.Tick([](auto&& sender, auto&&)
        {
            IgnoringErrors([&]
            {
                HWND hWindow = GetSearchAppWindow();
                bool bInForeground = GetForegroundWindow() == hWindow;
                if (bInForeground || --s_checksLeft <= 0)
                {
                    sender.as<DispatcherTimer>().Stop();
                    s_bWaiting = false;
                }
                RECT rcWindow, rcTaskbarSearchBox;
                if (bInForeground && g_hTaskbarSearchBox && GetWindowRect(hWindow, &rcWindow) && GetWindowRect(g_hTaskbarSearchBox, &rcTaskbarSearchBox) && rcWindow.left != rcTaskbarSearchBox.left)
                {
                    SetWindowPos(hWindow, nullptr, rcTaskbarSearchBox.left, rcWindow.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
                }
            });
        });
        timer.Start();
        s_bWaiting = true;
    }

    // The search app's window is moved and resized from another process, without WM_WINDOWPOSCHANGING, so we can only
    // follow up once it has moved
    static LRESULT CALLBACK SearchAppWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
    {
        if (uMsg == WM_WINDOWPOSCHANGED)
        {
            auto pPosition = (const WINDOWPOS*)lParam;
            if (!(pPosition->flags & SWP_NOMOVE))
            {
                IgnoringErrors(LineUpWindowWithTaskbarSearchBoxSoon);
            }
            if (g_hTaskbarSearchBox && (pPosition->flags & (SWP_NOMOVE | SWP_NOSIZE)) != (SWP_NOMOVE | SWP_NOSIZE))
            {
                IgnoringErrors(ApplyChangesAgainSoon);
            }
        }
        else if (uMsg == WM_NCDESTROY)
        {
            RemoveWindowSubclass(hWnd, SearchAppWindowSubclassProc, uIdSubclass);
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    static void Initialize()
    {
        auto window = Window::Current();
        if (!window || !window.Content() || winrt::get_class_name(window.Content()) != L"Cortana.UI.Views.TaskbarSearchPage")
        {
            return;
        }
        auto searchPage = window.Content().as<FrameworkElement>();
        auto searchTextBox = FindElement(searchPage, L"SearchTextBox").try_as<RichEditBox>();
        auto panel = FindElement(searchPage, L"AppBorder");
        if (!searchTextBox || !panel)
        {
            return;
        }
        g_pSearchPage = ::new (g_searchPageStorage) FrameworkElement(searchPage);
        g_pSearchTextBox = ::new (g_searchTextBoxStorage) RichEditBox(searchTextBox);
        g_pPanel = ::new (g_panelStorage) FrameworkElement(panel);

        // Our event handlers must outlive the hook, which goes away when explorer.exe exits
        IncrementDLLReferenceCount(hModule);

        searchTextBox.TextChanged([](auto&&, auto&&)
        {
            ChangePage([]
            {
                // In case we missed the search app showing up
                if (!g_hTaskbarSearchBox && GetForegroundWindow() == GetSearchAppWindow())
                {
                    ApplyChanges();
                }
            });
            IgnoringErrors(SendTypedTextToExplorer);
        });
        searchTextBox.SelectionChanged([](auto&&, auto&&) { IgnoringErrors(SendTypedTextToExplorer); });
        window.VisibilityChanged([](auto&&, winrt::Windows::UI::Core::VisibilityChangedEventArgs const& args)
        {
            if (args.Visible())
            {
                ChangePage(ApplyChanges);
            }
            else
            {
                ChangePage(UndoChanges);
            }
            IgnoringErrors(SendTypedTextToExplorer);
        });
        // The search app does not always raise VisibilityChanged when it shows up again
        window.Activated([](auto&&, winrt::Windows::UI::Core::WindowActivatedEventArgs const& args)
        {
            if (args.WindowActivationState() != winrt::Windows::UI::Core::CoreWindowActivationState::Deactivated)
            {
                ChangePage(ApplyChanges);
                IgnoringErrors(SendTypedTextToExplorer);
            }
        });
        // The layout also changes while the search app is shown, e.g. when clicking the search box with Start open
        if (auto rootGrid = FindElement(searchPage, L"RootGrid"))
        {
            for (auto&& group : VisualStateManager::GetVisualStateGroups(rootGrid))
            {
                if (group.Name() == L"SearchBoxLocationStates")
                {
                    group.CurrentStateChanged([](auto&&, auto&&)
                    {
                        ChangePage(ApplyChanges);
                        IgnoringErrors(SendTypedTextToExplorer);
                    });
                }
            }
        }
        // The search app lays out and moves the panel some time after it shows up
        searchPage.LayoutUpdated([](auto&&, auto&&)
        {
            ChangePage([]
            {
                if (HasPanelMovedOffTaskbar())
                {
                    AttachPanelToTaskbar();
                }
            });
        });
        SetWindowSubclass(GetSearchAppWindow(), SearchAppWindowSubclassProc, 0, 0);
        ChangePage(ApplyChanges);
        IgnoringErrors(SendTypedTextToExplorer);
        LineUpWindowWithTaskbarSearchBoxSoon();
    }
}

// Runs on the UI thread of the search app, where explorer.exe installed it
extern "C" LRESULT CALLBACK Taskbar10SearchBox_SearchAppThreadHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    static thread_local bool t_bInitialized = false;
    if (nCode >= 0 && !t_bInitialized)
    {
        t_bInitialized = true;
        InsideSearchApp::IgnoringErrors(InsideSearchApp::Initialize);
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
#pragma endregion

#pragma region "Inside explorer.exe"
// The latest text received from the search app; written by the receiver window, read when drawing the taskbar
static SRWLOCK g_typedTextLock = SRWLOCK_INIT;
static TypedText g_typedText;
static HWND g_hTaskbarSearchBoxShowingText;

#define CARET_BLINK_TIMER_ID 0x45505342

// Gets the typed text if it is to be shown in the search box that contains this text control
static bool GetTypedTextFor(HWND hTextControl, TypedText* pTypedText)
{
    AcquireSRWLockShared(&g_typedTextLock);
    bool bShowTypedText = g_typedText.hTaskbarSearchBox && (HWND)g_typedText.hTaskbarSearchBox == GetParent(hTextControl);
    if (bShowTypedText)
    {
        *pTypedText = g_typedText;
    }
    ReleaseSRWLockShared(&g_typedTextLock);
    return bShowTypedText;
}

static COLORREF GetTypedTextColor()
{
    DWORD dwSystemUsesLightTheme = 0, dwSize = sizeof(DWORD);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"SystemUsesLightTheme", RRF_RT_REG_DWORD, nullptr, &dwSystemUsesLightTheme, &dwSize);
    return dwSystemUsesLightTheme ? RGB(0, 0, 0) : RGB(255, 255, 255);
}

static void DrawCaret(HDC hdc, int x, const RECT* prcTextControl, COLORREF color)
{
    TEXTMETRICW textMetrics;
    GetTextMetricsW(hdc, &textMetrics);
    RECT rcCaret;
    rcCaret.left = x;
    rcCaret.right = x + max(1, GetSystemMetrics(SM_CXBORDER));
    rcCaret.top = prcTextControl->top + (prcTextControl->bottom - prcTextControl->top - textMetrics.tmHeight) / 2;
    rcCaret.bottom = rcCaret.top + textMetrics.tmHeight;
    SetBkColor(hdc, color);
    ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rcCaret, nullptr, 0, nullptr);
}

// Draws the typed text with its selection, and the caret, in the font of the placeholder text
static void DrawTypedText(HWND hTextControl, HDC hdc, const TypedText* pTypedText, bool bCaretVisible)
{
    RECT rc;
    GetClientRect(hTextControl, &rc);
    DrawThemeParentBackground(hTextControl, hdc, &rc);

    HFONT hOldFont = (HFONT)SelectObject(hdc, (HFONT)SendMessageW(hTextControl, WM_GETFONT, 0, 0));
    COLORREF textColor = GetTypedTextColor();
    int textLength = (int)wcslen(pTypedText->text);
    int selectionStart = max(0, min(pTypedText->selectionStart, pTypedText->selectionEnd));
    int selectionEnd = min(textLength, max(pTypedText->selectionStart, pTypedText->selectionEnd));

    // Scroll the text to the left when the caret would not be visible
    SIZE size = {};
    GetTextExtentPoint32W(hdc, pTypedText->text, selectionEnd, &size);
    int scrollOffset = max(0, (int)size.cx - (int)(rc.right - rc.left) + 2);

    TEXTMETRICW textMetrics;
    GetTextMetricsW(hdc, &textMetrics);
    int y = rc.top + (rc.bottom - rc.top - textMetrics.tmHeight) / 2;
    int x = rc.left - scrollOffset;
    SetBkMode(hdc, TRANSPARENT);
    // Before the selection, the selection, after the selection
    const int partStarts[] = { 0, selectionStart, selectionEnd };
    const int partEnds[] = { selectionStart, selectionEnd, textLength };
    for (int part = 0; part < 3; part++)
    {
        int start = partStarts[part], end = partEnds[part];
        if (end <= start)
        {
            continue;
        }
        GetTextExtentPoint32W(hdc, pTypedText->text + start, end - start, &size);
        bool bSelected = part == 1;
        if (bSelected)
        {
            RECT rcSelection = { x, y, x + size.cx, y + textMetrics.tmHeight };
            SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
            ExtTextOutW(hdc, 0, 0, ETO_OPAQUE | ETO_CLIPPED, &rcSelection, nullptr, 0, nullptr);
        }
        SetTextColor(hdc, bSelected ? GetSysColor(COLOR_HIGHLIGHTTEXT) : textColor);
        ExtTextOutW(hdc, x, y, ETO_CLIPPED, &rc, pTypedText->text + start, end - start, nullptr);
        x += size.cx;
    }

    if (bCaretVisible && selectionStart == selectionEnd)
    {
        GetTextExtentPoint32W(hdc, pTypedText->text, selectionEnd, &size);
        DrawCaret(hdc, rc.left - scrollOffset + size.cx, &rc, textColor);
    }
    SelectObject(hdc, hOldFont);
}

// Subclass of the control that shows the placeholder text in the search box on the taskbar
static LRESULT CALLBACK TaskbarSearchBoxTextSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    static bool s_bCaretVisible;
    static bool s_bCaretBlinking;
    switch (uMsg)
    {
        case WM_PAINT:
        {
            TypedText typedText;
            if (!GetTypedTextFor(hWnd, &typedText))
            {
                if (s_bCaretBlinking)
                {
                    KillTimer(hWnd, CARET_BLINK_TIMER_ID);
                    s_bCaretBlinking = false;
                }
                break; // Usual placeholder text
            }
            if (!s_bCaretBlinking)
            {
                SetTimer(hWnd, CARET_BLINK_TIMER_ID, GetCaretBlinkTime(), nullptr);
                s_bCaretBlinking = true;
                s_bCaretVisible = true;
            }
            if (!typedText.text[0])
            {
                // Nothing typed yet: the usual placeholder text, with the caret before it
                LRESULT lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);
                if (s_bCaretVisible)
                {
                    HDC hdc = GetDC(hWnd);
                    RECT rc;
                    GetClientRect(hWnd, &rc);
                    HFONT hOldFont = (HFONT)SelectObject(hdc, (HFONT)SendMessageW(hWnd, WM_GETFONT, 0, 0));
                    DrawCaret(hdc, rc.left, &rc, GetTypedTextColor());
                    SelectObject(hdc, hOldFont);
                    ReleaseDC(hWnd, hdc);
                }
                return lResult;
            }
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            HDC hdcBuffer = nullptr;
            HPAINTBUFFER hBuffer = BeginBufferedPaint(hdc, &ps.rcPaint, BPBF_TOPDOWNDIB, nullptr, &hdcBuffer);
            DrawTypedText(hWnd, hBuffer ? hdcBuffer : hdc, &typedText, s_bCaretVisible);
            if (hBuffer)
            {
                EndBufferedPaint(hBuffer, TRUE);
            }
            EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_TIMER:
        {
            if (wParam == CARET_BLINK_TIMER_ID)
            {
                s_bCaretVisible = !s_bCaretVisible;
                InvalidateRect(hWnd, nullptr, TRUE);
                return 0;
            }
            break;
        }
        case WM_NCDESTROY:
        {
            RemoveWindowSubclass(hWnd, TaskbarSearchBoxTextSubclassProc, uIdSubclass);
            break;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// Windows can only be subclassed from their own thread, which is the thread of the taskbar
static HWND g_hTextControlToSubclass;

static LRESULT CALLBACK SubclassOnTaskbarThreadProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    const CWPSTRUCT* pMessage = (const CWPSTRUCT*)lParam;
    if (nCode >= 0 && pMessage->hwnd == g_hTextControlToSubclass && pMessage->message == WM_NULL)
    {
        if (SetWindowSubclass(pMessage->hwnd, TaskbarSearchBoxTextSubclassProc, (UINT_PTR)TaskbarSearchBoxTextSubclassProc, 0))
        {
            SetPropW(pMessage->hwnd, TYPED_TEXT_RECEIVER_CLASS, (HANDLE)1);
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

// The control that shows the placeholder text in a search box on the taskbar, subclassed so it can show typed text
static HWND GetTaskbarSearchBoxTextControl(HWND hTaskbarSearchBox)
{
    HWND hTextControl = hTaskbarSearchBox && IsWindow(hTaskbarSearchBox) ? FindWindowExW(hTaskbarSearchBox, nullptr, L"Static", nullptr) : nullptr;
    if (hTextControl && !GetPropW(hTextControl, TYPED_TEXT_RECEIVER_CLASS))
    {
        HHOOK hHook = SetWindowsHookExW(WH_CALLWNDPROC, SubclassOnTaskbarThreadProc, nullptr, GetWindowThreadProcessId(hTextControl, nullptr));
        if (hHook)
        {
            g_hTextControlToSubclass = hTextControl;
            SendMessageTimeoutW(hTextControl, WM_NULL, 0, 0, SMTO_BLOCK, 1000, nullptr);
            g_hTextControlToSubclass = nullptr;
            UnhookWindowsHookEx(hHook);
        }
    }
    return hTextControl;
}

// Redraws the search boxes on the taskbar whose content changed with the latest typed text
static void RedrawTaskbarSearchBoxes()
{
    AcquireSRWLockShared(&g_typedTextLock);
    HWND hTaskbarSearchBox = (HWND)g_typedText.hTaskbarSearchBox;
    ReleaseSRWLockShared(&g_typedTextLock);

    if (g_hTaskbarSearchBoxShowingText && g_hTaskbarSearchBoxShowingText != hTaskbarSearchBox)
    {
        if (HWND hTextControl = GetTaskbarSearchBoxTextControl(g_hTaskbarSearchBoxShowingText))
        {
            InvalidateRect(hTextControl, nullptr, TRUE);
        }
    }
    g_hTaskbarSearchBoxShowingText = hTaskbarSearchBox;
    if (HWND hTextControl = GetTaskbarSearchBoxTextControl(hTaskbarSearchBox))
    {
        InvalidateRect(hTextControl, nullptr, TRUE);
    }
}

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

// UI threads of the search app that already have our hook (the search app restarts now and then)
static DWORD g_rgdwHookedSearchAppThreads[8];

static void CALLBACK OnForegroundWindowChanged(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    if (!IsSearchAppWindow(hWnd))
    {
        // In case the search app went away without telling us
        AcquireSRWLockExclusive(&g_typedTextLock);
        g_typedText.hTaskbarSearchBox = 0;
        ReleaseSRWLockExclusive(&g_typedTextLock);
        RedrawTaskbarSearchBoxes();
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
    if (SetWindowsHookExW(WH_GETMESSAGE, Taskbar10SearchBox_SearchAppThreadHook, hModule, dwThreadId))
    {
        memmove(g_rgdwHookedSearchAppThreads + 1, g_rgdwHookedSearchAppThreads, sizeof(g_rgdwHookedSearchAppThreads) - sizeof(DWORD));
        g_rgdwHookedSearchAppThreads[0] = dwThreadId;
        PostMessageW(hWnd, WM_NULL, 0, 0); // So that the hook runs right away
    }
}

static LRESULT CALLBACK TypedTextReceiverWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_COPYDATA)
    {
        const COPYDATASTRUCT* pcds = (const COPYDATASTRUCT*)lParam;
        if (pcds->dwData == TYPED_TEXT_COPYDATA_ID && pcds->cbData == sizeof(TypedText))
        {
            AcquireSRWLockExclusive(&g_typedTextLock);
            g_typedText = *(const TypedText*)pcds->lpData;
            g_typedText.text[ARRAYSIZE(g_typedText.text) - 1] = 0;
            ReleaseSRWLockExclusive(&g_typedTextLock);
            // Don't make the search app wait for the taskbar
            PostMessageW(hWnd, WM_EP_REDRAW_TASKBAR_SEARCH_BOX, 0, 0);
            return TRUE;
        }
        return FALSE;
    }
    if (uMsg == WM_EP_REDRAW_TASKBAR_SEARCH_BOX)
    {
        RedrawTaskbarSearchBoxes();
        return 0;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

static DWORD WINAPI SearchAppWatcherThread(LPVOID lpParameter)
{
    WNDCLASSW wc = {};
    wc.lpfnWndProc = TypedTextReceiverWindowProc;
    wc.hInstance = hModule;
    wc.lpszClassName = TYPED_TEXT_RECEIVER_CLASS;
    RegisterClassW(&wc);
    HWND hReceiver = CreateWindowExW(0, TYPED_TEXT_RECEIVER_CLASS, nullptr, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, hModule, nullptr);
    if (!hReceiver)
    {
        return 0;
    }
    // The search app runs in an app container, which may otherwise not send messages to us
    ChangeWindowMessageFilterEx(hReceiver, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
    SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, OnForegroundWindowChanged, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

// Only for the Windows 10 (ExplorerPatcher) taskbar on Windows 11, see dllmain.c
extern "C" void Taskbar10SearchBox_Initialize()
{
    HANDLE hThread = CreateThread(nullptr, 0, SearchAppWatcherThread, nullptr, 0, nullptr);
    if (hThread)
    {
        CloseHandle(hThread);
    }
}
#pragma endregion
#pragma endregion

#include "lvt.h"

Windows_UI_Xaml_IDependencyObject* LVT_FindChildByClassName(Windows_UI_Xaml_IDependencyObject* pRootDependencyObject, Windows_UI_Xaml_IVisualTreeHelperStatics* pVisualTreeHelperStatics, LPCWSTR pwszRefName, INT* prevIndex)
{
    if (!pRootDependencyObject)
    {
        return NULL;
    }

    //WCHAR wszDebug[MAX_PATH];
    HRESULT hr = S_OK;
    INT32 Count = -1;
    hr = pVisualTreeHelperStatics->lpVtbl->GetChildrenCount(pVisualTreeHelperStatics, pRootDependencyObject, &Count);
    if (SUCCEEDED(hr))
    {
        for (INT32 Index = (prevIndex ? *prevIndex : 0); Index < Count; ++Index)
        {
            Windows_UI_Xaml_IDependencyObject* pChild = NULL;
            hr = pVisualTreeHelperStatics->lpVtbl->GetChild(pVisualTreeHelperStatics, pRootDependencyObject, Index, &pChild);
            if (SUCCEEDED(hr))
            {
                HSTRING hsChild = NULL;
                pChild->lpVtbl->GetRuntimeClassName(pChild, &hsChild);
                if (SUCCEEDED(hr))
                {
                    PCWSTR pwszName = WindowsGetStringRawBuffer(hsChild, 0);
                    //swprintf_s(wszDebug, MAX_PATH, L"Name: %s\n", pwszName);
                    //OutputDebugStringW(wszDebug);
                    if (!_wcsicmp(pwszName, pwszRefName))
                    {
                        WindowsDeleteString(hsChild);
                        if (prevIndex) *prevIndex = Index + 1;
                        return pChild;
                    }
                    WindowsDeleteString(hsChild);
                }
                pChild->lpVtbl->Release(pChild);
            }
        }
    }

    if (prevIndex) *prevIndex = Count;
    return NULL;
}

Windows_UI_Xaml_IDependencyObject* LVT_FindChildByName(Windows_UI_Xaml_IDependencyObject* pRootDependencyObject, Windows_UI_Xaml_IVisualTreeHelperStatics* pVisualTreeHelperStatics, LPCWSTR pwszRefName)
{
    if (!pRootDependencyObject)
    {
        return NULL;
    }

    //WCHAR wszDebug[MAX_PATH];
    HRESULT hr = S_OK;
    INT32 Count = -1;
    hr = pVisualTreeHelperStatics->lpVtbl->GetChildrenCount(pVisualTreeHelperStatics, pRootDependencyObject, &Count);
    if (SUCCEEDED(hr))
    {
        for (INT32 Index = 0; Index < Count; ++Index)
        {
            Windows_UI_Xaml_IDependencyObject* pChild = NULL;
            hr = pVisualTreeHelperStatics->lpVtbl->GetChild(pVisualTreeHelperStatics, pRootDependencyObject, Index, &pChild);
            if (SUCCEEDED(hr))
            {
                Windows_UI_Xaml_IFrameworkElement* pFrameworkElement = NULL;
                hr = pChild->lpVtbl->QueryInterface(pChild, &IID_Windows_UI_Xaml_IFrameworkElement, &pFrameworkElement);
                if (SUCCEEDED(hr))
                {
                    HSTRING hsChild = NULL;
                    pFrameworkElement->lpVtbl->get_Name(pFrameworkElement, &hsChild);
                    if (SUCCEEDED(hr))
                    {
                        PCWSTR pwszName = WindowsGetStringRawBuffer(hsChild, 0);
                        //swprintf_s(wszDebug, MAX_PATH, L"Name: %s\n", pwszName);
                        //OutputDebugStringW(wszDebug);
                        if (!_wcsicmp(pwszName, pwszRefName))
                        {
                            WindowsDeleteString(hsChild);
                            pFrameworkElement->lpVtbl->Release(pFrameworkElement);
                            return pChild;
                        }
                        WindowsDeleteString(hsChild);
                    }
                    pFrameworkElement->lpVtbl->Release(pFrameworkElement);
                }
                pChild->lpVtbl->Release(pChild);
            }
        }
    }

    return NULL;
}

// Referenece: https://www.reddit.com/r/Windows10/comments/nvcrie/windows_11_start_menu_how_to_temporary_make_your/
void LVT_StartUI_EnableRoundedCorners(HWND hWnd, DWORD dwReceipe, DWORD dwPos, HWND hWndTaskbar, RECT* rect)
{
    WCHAR wszDebug[MAX_PATH];
    HRESULT hr = S_OK;

    Windows_UI_Xaml_IDependencyObject* pRootDependencyObject = NULL;
    Windows_UI_Xaml_Controls_ICanvasStatics* pCanvasStatics = NULL;

    if (SUCCEEDED(hr))
    {
        HSTRING_HEADER hshControlsCanvasStatics;
        HSTRING hsControlsCanvasStatics = NULL;
        hr = WindowsCreateStringReference(L"Windows.UI.Xaml.Controls.Canvas", 31, &hshControlsCanvasStatics, &hsControlsCanvasStatics);
        if (SUCCEEDED(hr) && hsControlsCanvasStatics)
        {
            hr = RoGetActivationFactory(hsControlsCanvasStatics, &IID_Windows_UI_Xaml_Controls_ICanvasStatics, &pCanvasStatics);
            WindowsDeleteString(hsControlsCanvasStatics);
        }
    }

    if (SUCCEEDED(hr))
    {
        HSTRING_HEADER hshWindowStatics;
        HSTRING hsWindowStatics = NULL;
        hr = WindowsCreateStringReference(L"Windows.UI.Xaml.Window", 22, &hshWindowStatics, &hsWindowStatics);
        if (SUCCEEDED(hr) && hsWindowStatics)
        {
            Windows_UI_Xaml_IWindowStatics* pWindowStatics = NULL;
            hr = RoGetActivationFactory(hsWindowStatics, &IID_Windows_UI_Xaml_IWindowStatics, &pWindowStatics);
            if (SUCCEEDED(hr))
            {
                Windows_UI_Xaml_IWindow* pWindow = NULL;
                hr = pWindowStatics->lpVtbl->get_Current(pWindowStatics, &pWindow);
                if (SUCCEEDED(hr))
                {
                    IInspectable* pUIElement = NULL;
                    hr = pWindow->lpVtbl->get_Content(pWindow, &pUIElement);
                    if (SUCCEEDED(hr))
                    {
                        hr = pUIElement->lpVtbl->QueryInterface(pUIElement, &IID_Windows_UI_Xaml_IDependencyObject, &pRootDependencyObject);

                        pUIElement->lpVtbl->Release(pUIElement);
                    }
                    pWindow->lpVtbl->Release(pWindow);
                }
                pWindowStatics->lpVtbl->Release(pWindowStatics);
            }
            WindowsDeleteString(hsWindowStatics);
        }
    }

    if (pRootDependencyObject)
    {
        HSTRING_HEADER hshVisualTreeHelperStatics;
        HSTRING hsVisualTreeHelperStatics = NULL;
        hr = WindowsCreateStringReference(L"Windows.UI.Xaml.Media.VisualTreeHelper", 38, &hshVisualTreeHelperStatics, &hsVisualTreeHelperStatics);
        if (SUCCEEDED(hr) && hsVisualTreeHelperStatics)
        {
            Windows_UI_Xaml_IVisualTreeHelperStatics* pVisualTreeHelperStatics = NULL;
            hr = RoGetActivationFactory(hsVisualTreeHelperStatics, &IID_Windows_UI_Xaml_IVisualTreeHelperStatics, &pVisualTreeHelperStatics);
            if (SUCCEEDED(hr))
            {
                Windows_UI_Xaml_IDependencyObject* pStartSizingFrame = LVT_FindChildByClassName(pRootDependencyObject, pVisualTreeHelperStatics, L"StartUI.StartSizingFrame", NULL);
                if (pStartSizingFrame)
                {
                    int location = LVT_LOC_NONE;

                    Windows_UI_Xaml_Thickness drc;
                    drc.Left = 0.0; drc.Right = 0.0; drc.Top = 0.0; drc.Bottom = 0.0;
                    Windows_UI_Xaml_IUIElement* pIUIElement = NULL;
                    Windows_UI_Xaml_IFrameworkElement* pFrameworkElement = NULL;
                    pStartSizingFrame->lpVtbl->QueryInterface(pStartSizingFrame, &IID_Windows_UI_Xaml_IUIElement, &pIUIElement);
                    if (pIUIElement)
                    {
                        pCanvasStatics->lpVtbl->GetLeft(pCanvasStatics, pIUIElement, &(drc.Left));
                        pCanvasStatics->lpVtbl->GetTop(pCanvasStatics, pIUIElement, &(drc.Top));
                    }
                    pStartSizingFrame->lpVtbl->QueryInterface(pStartSizingFrame, &IID_Windows_UI_Xaml_IFrameworkElement, &pFrameworkElement);
                    if (pFrameworkElement)
                    {
                        pFrameworkElement->lpVtbl->get_ActualWidth(pFrameworkElement, &(drc.Right));
                        pFrameworkElement->lpVtbl->get_ActualHeight(pFrameworkElement, &(drc.Bottom));
                    }
                    UINT dpi = GetDpiForWindow(hWnd);
                    RECT rc;
                    SetRect(&rc, drc.Left, drc.Top, drc.Right, drc.Bottom);
                    SetRect(&rc, MulDiv(rc.left, dpi, 96), MulDiv(rc.top, dpi, 96), MulDiv(rc.right, dpi, 96), MulDiv(rc.bottom, dpi, 96));
                    *rect = rc;
                    HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
                    MONITORINFO mi;
                    ZeroMemory(&mi, sizeof(MONITORINFO));
                    mi.cbSize = sizeof(MONITORINFO);
                    GetMonitorInfoW(hMonitor, &mi);
                    //swprintf(wszDebug, MAX_PATH, L"RECT %d %d %d %d - %d %d %d %d\n", rc.left, rc.top, rc.right, rc.bottom, 0, 0, mi.rcWork.right - mi.rcWork.left, mi.rcWork.bottom - mi.rcWork.top);
                    //OutputDebugStringW(wszDebug);
                    if (!(rc.left == 0 && rc.top == 0 && abs(mi.rcWork.right - mi.rcWork.left - rc.right) < 5 && abs(mi.rcWork.bottom - mi.rcWork.top - rc.bottom) < 5))
                    {
                        if (rc.left == 0)
                        {
                            if (rc.top == 0)
                            {
                                location = LVT_LOC_TOPLEFT;
                            }
                            else
                            {
                                location = LVT_LOC_BOTTOMLEFT;
                            }
                        }
                        else
                        {
                            location = LVT_LOC_TOPRIGHT;
                        }
                    }
                    //swprintf_s(wszDebug, MAX_PATH, L"Location: %d\n", location);
                    //OutputDebugStringW(wszDebug);
                    if (pFrameworkElement)
                    {
                        pFrameworkElement->lpVtbl->Release(pFrameworkElement);
                    }
                    if (pIUIElement)
                    {
                        pIUIElement->lpVtbl->Release(pIUIElement);
                    }
                    Windows_UI_Xaml_IDependencyObject* pStartSizingFramePanel = LVT_FindChildByClassName(pStartSizingFrame, pVisualTreeHelperStatics, L"StartUI.StartSizingFramePanel", NULL);
                    if (pStartSizingFramePanel)
                    {
                        // Drop shadow
                        Windows_UI_Xaml_IDependencyObject* pDropShadow = LVT_FindChildByClassName(pStartSizingFramePanel, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.Image", NULL);
                        if (pDropShadow)
                        {
                            Windows_UI_Xaml_IUIElement* pIUIElement = NULL;
                            pDropShadow->lpVtbl->QueryInterface(pDropShadow, &IID_Windows_UI_Xaml_IUIElement, &pIUIElement);
                            if (pIUIElement)
                            {
                                if (dwReceipe)
                                {
                                    pIUIElement->lpVtbl->put_Visibility(pIUIElement, Windows_UI_Xaml_Visibility_Collapsed);
                                }
                                else
                                {
                                    pIUIElement->lpVtbl->put_Visibility(pIUIElement, Windows_UI_Xaml_Visibility_Visible);
                                }
                                pIUIElement->lpVtbl->Release(pIUIElement);
                            }
                            pDropShadow->lpVtbl->Release(pDropShadow);
                        }
                        Windows_UI_Xaml_IDependencyObject* pContentPresenter = LVT_FindChildByClassName(pStartSizingFramePanel, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.ContentPresenter", NULL);
                        if (pContentPresenter)
                        {
                            Windows_UI_Xaml_IDependencyObject* pFrame = LVT_FindChildByClassName(pContentPresenter, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.Frame", NULL);
                            if (pFrame)
                            {
                                // Main menu padding
                                Windows_UI_Xaml_Controls_IControl* pIBorder = NULL;
                                pFrame->lpVtbl->QueryInterface(pFrame, &IID_Windows_UI_Xaml_Controls_IControl, &pIBorder);
                                if (pIBorder)
                                {
                                    double pad = 12.0;
                                    Windows_UI_Xaml_Thickness th;
                                    if (location && dwReceipe == 1)
                                    {
                                        if (location == LVT_LOC_BOTTOMLEFT)
                                        {
                                            th.Left = dwReceipe ? pad : 0.0;
                                            th.Bottom = dwReceipe ? pad : 0.0;
                                            th.Right = hWndTaskbar ? (dwReceipe ? pad : 0.0) : 0.0;
                                            th.Top = hWndTaskbar ? (dwReceipe ? pad : 0.0) : 0.0;
                                        }
                                        else if (location == LVT_LOC_TOPLEFT)
                                        {
                                            th.Left = dwReceipe ? pad : 0.0;
                                            th.Bottom = hWndTaskbar ? (dwReceipe ? pad : 0.0) : 0.0;
                                            th.Right = hWndTaskbar ? (dwReceipe ? pad : 0.0) : 0.0;
                                            th.Top = dwReceipe ? pad : 0.0;
                                        }
                                        else if (location == LVT_LOC_TOPRIGHT)
                                        {
                                            th.Left = hWndTaskbar ? (dwReceipe ? pad : 0.0) : 0.0;
                                            th.Bottom = hWndTaskbar ? (dwReceipe ? pad : 0.0) : 0.0;
                                            th.Right = dwReceipe ? pad : 0.0;
                                            th.Top = dwReceipe ? pad : 0.0;
                                        }
                                    }
                                    else
                                    {
                                        th.Left = 0.0;
                                        th.Bottom = 0.0;
                                        th.Right = 0.0;
                                        th.Top = 0.0;
                                    }
                                    pIBorder->lpVtbl->put_Padding(pIBorder, th);
                                    pIBorder->lpVtbl->Release(pIBorder);
                                }
                                Windows_UI_Xaml_IDependencyObject* pContentPresenter2 = LVT_FindChildByClassName(pFrame, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.ContentPresenter", NULL);
                                if (pContentPresenter2)
                                {
                                    Windows_UI_Xaml_IDependencyObject* pSplitViewFrame = LVT_FindChildByClassName(pContentPresenter2, pVisualTreeHelperStatics, L"StartUI.SplitViewFrame", NULL);
                                    if (pSplitViewFrame)
                                    {
                                        Windows_UI_Xaml_IDependencyObject* pRootGrid = LVT_FindChildByName(pSplitViewFrame, pVisualTreeHelperStatics, L"RootGrid");
                                        if (pRootGrid)
                                        {
                                            // Main menu corners
                                            Windows_UI_Xaml_IDependencyObject* pAcrylicBorder = LVT_FindChildByName(pRootGrid, pVisualTreeHelperStatics, L"AcrylicBorder");
                                            if (pAcrylicBorder)
                                            {
                                                Windows_UI_Xaml_Controls_IBorder* pIBorder = NULL;
                                                pAcrylicBorder->lpVtbl->QueryInterface(pAcrylicBorder, &IID_Windows_UI_Xaml_Controls_IBorder, &pIBorder);
                                                if (pIBorder)
                                                {
                                                    double pad = 10.0;
                                                    Windows_UI_Xaml_CornerRadius cr;
                                                    if (location && dwReceipe == 2)
                                                    {
                                                        if (location == LVT_LOC_BOTTOMLEFT)
                                                        {
                                                            cr.BottomLeft = 0.0;
                                                            cr.BottomRight = 0.0;
                                                            cr.TopLeft = hWndTaskbar ? (dwReceipe ? pad : 0.0) : 0.0;
                                                            cr.TopRight = dwReceipe ? pad : 0.0;
                                                        }
                                                        else if (location == LVT_LOC_TOPLEFT)
                                                        {
                                                            cr.BottomLeft = (hWndTaskbar && (dwPos == 2)) ? (dwReceipe ? pad : 0.0) : 0.0;
                                                            cr.BottomRight = dwReceipe ? pad : 0.0;
                                                            cr.TopLeft = 0.0;
                                                            cr.TopRight = (hWndTaskbar && (dwPos != 2)) ? (dwReceipe ? pad : 0.0) : 0.0;
                                                        }
                                                        else if (location == LVT_LOC_TOPRIGHT)
                                                        {
                                                            cr.BottomLeft = dwReceipe ? pad : 0.0;
                                                            cr.BottomRight = 0.0;
                                                            cr.TopLeft = hWndTaskbar ? (dwReceipe ? pad : 0.0) : 0.0;
                                                            cr.TopRight = 0.0;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        cr.BottomLeft = (dwReceipe ? 10.0 : 0.0);
                                                        cr.BottomRight = cr.BottomLeft;
                                                        cr.TopLeft = cr.BottomLeft;
                                                        cr.TopRight = cr.BottomLeft;
                                                    }
                                                    pIBorder->lpVtbl->put_CornerRadius(pIBorder, cr);
                                                    pIBorder->lpVtbl->Release(pIBorder);
                                                }
                                                pAcrylicBorder->lpVtbl->Release(pAcrylicBorder);
                                            }
                                            // Live tiles corners
                                            Windows_UI_Xaml_IDependencyObject* pRootContent = LVT_FindChildByName(pRootGrid, pVisualTreeHelperStatics, L"RootContent");
                                            if (pRootContent)
                                            {
                                                Windows_UI_Xaml_IDependencyObject* pGrid = LVT_FindChildByClassName(pRootContent, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.Grid", NULL);
                                                if (pGrid)
                                                {
                                                    Windows_UI_Xaml_IDependencyObject* pContentRoot = LVT_FindChildByName(pGrid, pVisualTreeHelperStatics, L"ContentRoot");
                                                    if (pContentRoot)
                                                    {
                                                        Windows_UI_Xaml_IDependencyObject* pBorder = LVT_FindChildByClassName(pContentRoot, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.Border", NULL);
                                                        if (pBorder)
                                                        {
                                                            Windows_UI_Xaml_IDependencyObject* pContentPaneGrid = LVT_FindChildByName(pBorder, pVisualTreeHelperStatics, L"ContentPaneGrid");
                                                            if (pContentPaneGrid)
                                                            {
                                                                Windows_UI_Xaml_IDependencyObject* pGridPane = LVT_FindChildByName(pContentPaneGrid, pVisualTreeHelperStatics, L"GridPane");
                                                                if (pGridPane)
                                                                {
                                                                    Windows_UI_Xaml_IDependencyObject* ppage = LVT_FindChildByName(pGridPane, pVisualTreeHelperStatics, L"page");
                                                                    if (ppage)
                                                                    {
                                                                        Windows_UI_Xaml_IDependencyObject* pgridRoot = LVT_FindChildByName(ppage, pVisualTreeHelperStatics, L"gridRoot");
                                                                        if (pgridRoot)
                                                                        {
                                                                            Windows_UI_Xaml_IDependencyObject* pgroupItems = LVT_FindChildByName(pgridRoot, pVisualTreeHelperStatics, L"groupItems");
                                                                            if (pgroupItems)
                                                                            {
                                                                                Windows_UI_Xaml_IDependencyObject* pBorder2 = LVT_FindChildByClassName(pgroupItems, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.Border", NULL);
                                                                                if (pBorder2)
                                                                                {
                                                                                    Windows_UI_Xaml_IDependencyObject* pScrollViewer = LVT_FindChildByName(pBorder2, pVisualTreeHelperStatics, L"ScrollViewer");
                                                                                    if (pScrollViewer)
                                                                                    {
                                                                                        Windows_UI_Xaml_IDependencyObject* pBorder3 = LVT_FindChildByClassName(pScrollViewer, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.Border", NULL);
                                                                                        if (pBorder3)
                                                                                        {
                                                                                            Windows_UI_Xaml_IDependencyObject* pGrid2 = LVT_FindChildByClassName(pBorder3, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.Grid", NULL);
                                                                                            if (pGrid2)
                                                                                            {
                                                                                                Windows_UI_Xaml_IDependencyObject* pScrollContentPresenter = LVT_FindChildByName(pGrid2, pVisualTreeHelperStatics, L"ScrollContentPresenter");
                                                                                                if (pScrollContentPresenter)
                                                                                                {
                                                                                                    Windows_UI_Xaml_IDependencyObject* pItemsPresenter = LVT_FindChildByClassName(pScrollContentPresenter, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.ItemsPresenter", NULL);
                                                                                                    if (pItemsPresenter)
                                                                                                    {
                                                                                                        Windows_UI_Xaml_IDependencyObject* pTileGrid = LVT_FindChildByClassName(pItemsPresenter, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.TileGrid", NULL);
                                                                                                        if (pTileGrid)
                                                                                                        {
                                                                                                            INT iIndex = 0;
                                                                                                            BOOL bSkipFirst = TRUE;
                                                                                                            while (TRUE)
                                                                                                            {
                                                                                                                Windows_UI_Xaml_IDependencyObject* pCurrentGroup = LVT_FindChildByClassName(pTileGrid, pVisualTreeHelperStatics, L"StartUI.TileListViewItem", &iIndex);
                                                                                                                if (!pCurrentGroup)
                                                                                                                {
                                                                                                                    break;
                                                                                                                }
                                                                                                                if (bSkipFirst)
                                                                                                                {
                                                                                                                    bSkipFirst = FALSE;
                                                                                                                    pCurrentGroup->lpVtbl->Release(pCurrentGroup);
                                                                                                                    continue;
                                                                                                                }
                                                                                                                Windows_UI_Xaml_IDependencyObject* pcontentPresenter = LVT_FindChildByName(pCurrentGroup, pVisualTreeHelperStatics, L"contentPresenter");
                                                                                                                if (pcontentPresenter)
                                                                                                                {
                                                                                                                    Windows_UI_Xaml_IDependencyObject* pTileGroupViewControl = LVT_FindChildByClassName(pcontentPresenter, pVisualTreeHelperStatics, L"StartUI.TileGroupViewControl", NULL);
                                                                                                                    if (pTileGroupViewControl)
                                                                                                                    {
                                                                                                                        Windows_UI_Xaml_IDependencyObject* pGrid3 = LVT_FindChildByClassName(pTileGroupViewControl, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.Grid", NULL);
                                                                                                                        if (pGrid3)
                                                                                                                        {
                                                                                                                            Windows_UI_Xaml_IDependencyObject* pNestedPanel = LVT_FindChildByName(pGrid3, pVisualTreeHelperStatics, L"NestedPanel");
                                                                                                                            if (pNestedPanel)
                                                                                                                            {
                                                                                                                                INT jIndex = 0;
                                                                                                                                while (TRUE)
                                                                                                                                {
                                                                                                                                    Windows_UI_Xaml_IDependencyObject* pCurrentTile = LVT_FindChildByClassName(pNestedPanel, pVisualTreeHelperStatics, L"StartUI.TileListViewItem", &jIndex);
                                                                                                                                    if (!pCurrentTile)
                                                                                                                                    {
                                                                                                                                        break;
                                                                                                                                    }
                                                                                                                                    Windows_UI_Xaml_IDependencyObject* pcontentPresenter2 = LVT_FindChildByName(pCurrentTile, pVisualTreeHelperStatics, L"contentPresenter");
                                                                                                                                    if (pcontentPresenter2)
                                                                                                                                    {
                                                                                                                                        Windows_UI_Xaml_IDependencyObject* pTileViewControl = LVT_FindChildByClassName(pcontentPresenter2, pVisualTreeHelperStatics, L"StartUI.TileViewControl", NULL);
                                                                                                                                        if (pTileViewControl)
                                                                                                                                        {
                                                                                                                                            Windows_UI_Xaml_Controls_IGrid2* pIGrid2 = NULL;
                                                                                                                                            pTileViewControl->lpVtbl->QueryInterface(pTileViewControl, &IID_Windows_UI_Xaml_Controls_IGrid2, &pIGrid2);
                                                                                                                                            if (pIGrid2)
                                                                                                                                            {
                                                                                                                                                Windows_UI_Xaml_CornerRadius cr;
                                                                                                                                                cr.BottomLeft = (dwReceipe ? 5.0 : 0.0);
                                                                                                                                                cr.BottomRight = cr.BottomLeft;
                                                                                                                                                cr.TopLeft = cr.BottomLeft;
                                                                                                                                                cr.TopRight = cr.BottomLeft;
                                                                                                                                                pIGrid2->lpVtbl->put_CornerRadius(pIGrid2, cr);
                                                                                                                                                pIGrid2->lpVtbl->Release(pIGrid2);
                                                                                                                                            }
                                                                                                                                            pTileViewControl->lpVtbl->Release(pTileViewControl);
                                                                                                                                        }
                                                                                                                                        pcontentPresenter2->lpVtbl->Release(pcontentPresenter2);
                                                                                                                                    }
                                                                                                                                    pCurrentTile->lpVtbl->Release(pCurrentTile);
                                                                                                                                }
                                                                                                                                pNestedPanel->lpVtbl->Release(pNestedPanel);
                                                                                                                            }
                                                                                                                            pGrid3->lpVtbl->Release(pGrid3);
                                                                                                                        }
                                                                                                                        pTileGroupViewControl->lpVtbl->Release(pTileGroupViewControl);
                                                                                                                    }
                                                                                                                    pcontentPresenter->lpVtbl->Release(pcontentPresenter);
                                                                                                                }
                                                                                                                pCurrentGroup->lpVtbl->Release(pCurrentGroup);
                                                                                                            }
                                                                                                            pTileGrid->lpVtbl->Release(pTileGrid);
                                                                                                        }
                                                                                                        pItemsPresenter->lpVtbl->Release(pItemsPresenter);
                                                                                                    }
                                                                                                    pScrollContentPresenter->lpVtbl->Release(pScrollContentPresenter);
                                                                                                }
                                                                                                pGrid2->lpVtbl->Release(pGrid2);
                                                                                            }
                                                                                            pBorder3->lpVtbl->Release(pBorder3);
                                                                                        }
                                                                                        pScrollViewer->lpVtbl->Release(pScrollViewer);
                                                                                    }
                                                                                    pBorder2->lpVtbl->Release(pBorder2);
                                                                                }
                                                                                pgroupItems->lpVtbl->Release(pgroupItems);
                                                                            }
                                                                            pgridRoot->lpVtbl->Release(pgridRoot);
                                                                        }
                                                                        ppage->lpVtbl->Release(ppage);
                                                                    }
                                                                    pGridPane->lpVtbl->Release(pGridPane);
                                                                }
                                                                pContentPaneGrid->lpVtbl->Release(pContentPaneGrid);
                                                            }
                                                            pBorder->lpVtbl->Release(pBorder);
                                                        }
                                                        pContentRoot->lpVtbl->Release(pContentRoot);
                                                    }
                                                    pGrid->lpVtbl->Release(pGrid);
                                                }
                                                pRootContent->lpVtbl->Release(pRootContent);
                                            }
                                            pRootGrid->lpVtbl->Release(pRootGrid);
                                        }
                                        pSplitViewFrame->lpVtbl->Release(pSplitViewFrame);
                                    }
                                    pContentPresenter2->lpVtbl->Release(pContentPresenter2);
                                }
                                pFrame->lpVtbl->Release(pFrame);
                            }
                            pContentPresenter->lpVtbl->Release(pContentPresenter);
                        }
                        pStartSizingFramePanel->lpVtbl->Release(pStartSizingFramePanel);
                    }
                    pStartSizingFrame->lpVtbl->Release(pStartSizingFrame);
                }
                pVisualTreeHelperStatics->lpVtbl->Release(pVisualTreeHelperStatics);
            }
            WindowsDeleteString(hsVisualTreeHelperStatics);
        }
        pRootDependencyObject->lpVtbl->Release(pRootDependencyObject);
    }

    if (pCanvasStatics)
    {
        pCanvasStatics->lpVtbl->Release(pCanvasStatics);
    }
}

void LVT_StartDocked_120DPIHack(int maxHeight)
{
    HRESULT hr = S_OK;
    Windows_UI_Xaml_IDependencyObject* pRootDependencyObject = NULL;
    if (SUCCEEDED(hr))
    {
        HSTRING_HEADER hshWindowStatics;
        HSTRING hsWindowStatics = NULL;
        hr = WindowsCreateStringReference(L"Windows.UI.Xaml.Window", 22, &hshWindowStatics, &hsWindowStatics);
        if (SUCCEEDED(hr) && hsWindowStatics)
        {
            Windows_UI_Xaml_IWindowStatics* pWindowStatics = NULL;
            hr = RoGetActivationFactory(hsWindowStatics, &IID_Windows_UI_Xaml_IWindowStatics, &pWindowStatics);
            if (SUCCEEDED(hr))
            {
                Windows_UI_Xaml_IWindow* pWindow = NULL;
                hr = pWindowStatics->lpVtbl->get_Current(pWindowStatics, &pWindow);
                if (SUCCEEDED(hr))
                {
                    IInspectable* pUIElement = NULL;
                    hr = pWindow->lpVtbl->get_Content(pWindow, &pUIElement);
                    if (SUCCEEDED(hr))
                    {
                        hr = pUIElement->lpVtbl->QueryInterface(pUIElement, &IID_Windows_UI_Xaml_IDependencyObject, &pRootDependencyObject);

                        pUIElement->lpVtbl->Release(pUIElement);
                    }
                    pWindow->lpVtbl->Release(pWindow);
                }
                pWindowStatics->lpVtbl->Release(pWindowStatics);
            }
            WindowsDeleteString(hsWindowStatics);
        }
    }
    if (pRootDependencyObject)
    {
        HSTRING_HEADER hshVisualTreeHelperStatics;
        HSTRING hsVisualTreeHelperStatics = NULL;
        hr = WindowsCreateStringReference(L"Windows.UI.Xaml.Media.VisualTreeHelper", 38, &hshVisualTreeHelperStatics, &hsVisualTreeHelperStatics);
        if (SUCCEEDED(hr) && hsVisualTreeHelperStatics)
        {
            Windows_UI_Xaml_IVisualTreeHelperStatics* pVisualTreeHelperStatics = NULL;
            hr = RoGetActivationFactory(hsVisualTreeHelperStatics, &IID_Windows_UI_Xaml_IVisualTreeHelperStatics, &pVisualTreeHelperStatics);
            if (SUCCEEDED(hr))
            {
                Windows_UI_Xaml_IDependencyObject* pStartSizingFrame = LVT_FindChildByClassName(pRootDependencyObject, pVisualTreeHelperStatics, L"StartDocked.StartSizingFrame", NULL);
                if (pStartSizingFrame)
                {
                    Windows_UI_Xaml_IUIElement* pIUIElement = NULL;
                    pStartSizingFrame->lpVtbl->QueryInterface(pStartSizingFrame, &IID_Windows_UI_Xaml_IUIElement, &pIUIElement);
                    if (pIUIElement)
                    {
                        Windows_UI_Xaml_IFrameworkElement* pFrameworkElement = NULL;
                        pStartSizingFrame->lpVtbl->QueryInterface(pStartSizingFrame, &IID_Windows_UI_Xaml_IFrameworkElement, &pFrameworkElement);
                        if (pFrameworkElement)
                        {
                            if (!IsWindows11Version22H2Build1413OrHigher()) pIUIElement->lpVtbl->put_Visibility(pIUIElement, Windows_UI_Xaml_Visibility_Collapsed);
                            pFrameworkElement->lpVtbl->put_MaxHeight(pFrameworkElement, maxHeight);
                            if (!IsWindows11Version22H2Build1413OrHigher()) pIUIElement->lpVtbl->put_Visibility(pIUIElement, Windows_UI_Xaml_Visibility_Visible);
                            pFrameworkElement->lpVtbl->Release(pFrameworkElement);
                        }
                        pIUIElement->lpVtbl->Release(pIUIElement);
                    }
                    pStartSizingFrame->lpVtbl->Release(pStartSizingFrame);
                }
                pVisualTreeHelperStatics->lpVtbl->Release(pVisualTreeHelperStatics);
            }
            WindowsDeleteString(hsVisualTreeHelperStatics);
        }
        pRootDependencyObject->lpVtbl->Release(pRootDependencyObject);
    }
}

// Reference: https://www.reddit.com/r/Windows11/comments/p1ksou/this_is_not_a_concept_microsoft_in_windows_11/
void LVT_StartDocked_DisableRecommendedSection(HWND hWnd, BOOL bApply, RECT* rect)
{
    WCHAR wszDebug[MAX_PATH];
    HRESULT hr = S_OK;

    Windows_UI_Xaml_IDependencyObject* pRootDependencyObject = NULL;
    Windows_UI_Xaml_Controls_ICanvasStatics* pCanvasStatics = NULL;

    if (SUCCEEDED(hr))
    {
        HSTRING_HEADER hshControlsCanvasStatics;
        HSTRING hsControlsCanvasStatics = NULL;
        hr = WindowsCreateStringReference(L"Windows.UI.Xaml.Controls.Canvas", 31, &hshControlsCanvasStatics, &hsControlsCanvasStatics);
        if (SUCCEEDED(hr) && hsControlsCanvasStatics)
        {
            hr = RoGetActivationFactory(hsControlsCanvasStatics, &IID_Windows_UI_Xaml_Controls_ICanvasStatics, &pCanvasStatics);
            WindowsDeleteString(hsControlsCanvasStatics);
        }
    }

    if (SUCCEEDED(hr))
    {
        HSTRING_HEADER hshWindowStatics;
        HSTRING hsWindowStatics = NULL;
        hr = WindowsCreateStringReference(L"Windows.UI.Xaml.Window", 22, &hshWindowStatics, &hsWindowStatics);
        if (SUCCEEDED(hr) && hsWindowStatics)
        {
            Windows_UI_Xaml_IWindowStatics* pWindowStatics = NULL;
            hr = RoGetActivationFactory(hsWindowStatics, &IID_Windows_UI_Xaml_IWindowStatics, &pWindowStatics);
            if (SUCCEEDED(hr))
            {
                Windows_UI_Xaml_IWindow* pWindow = NULL;
                hr = pWindowStatics->lpVtbl->get_Current(pWindowStatics, &pWindow);
                if (SUCCEEDED(hr))
                {
                    IInspectable* pUIElement = NULL;
                    hr = pWindow->lpVtbl->get_Content(pWindow, &pUIElement);
                    if (SUCCEEDED(hr))
                    {
                        hr = pUIElement->lpVtbl->QueryInterface(pUIElement, &IID_Windows_UI_Xaml_IDependencyObject, &pRootDependencyObject);

                        pUIElement->lpVtbl->Release(pUIElement);
                    }
                    pWindow->lpVtbl->Release(pWindow);
                }
                pWindowStatics->lpVtbl->Release(pWindowStatics);
            }
            WindowsDeleteString(hsWindowStatics);
        }
    }

    if (pRootDependencyObject)
    {
        HSTRING_HEADER hshVisualTreeHelperStatics;
        HSTRING hsVisualTreeHelperStatics = NULL;
        hr = WindowsCreateStringReference(L"Windows.UI.Xaml.Media.VisualTreeHelper", 38, &hshVisualTreeHelperStatics, &hsVisualTreeHelperStatics);
        if (SUCCEEDED(hr) && hsVisualTreeHelperStatics)
        {
            Windows_UI_Xaml_IVisualTreeHelperStatics* pVisualTreeHelperStatics = NULL;
            hr = RoGetActivationFactory(hsVisualTreeHelperStatics, &IID_Windows_UI_Xaml_IVisualTreeHelperStatics, &pVisualTreeHelperStatics);
            if (SUCCEEDED(hr))
            {
                Windows_UI_Xaml_IDependencyObject* pStartSizingFrame = LVT_FindChildByClassName(pRootDependencyObject, pVisualTreeHelperStatics, L"StartDocked.StartSizingFrame", NULL);
                if (pStartSizingFrame)
                {
                    Windows_UI_Xaml_Thickness drc;
                    drc.Left = 0.0; drc.Right = 0.0; drc.Top = 0.0; drc.Bottom = 0.0;
                    Windows_UI_Xaml_IUIElement* pIUIElement = NULL;
                    Windows_UI_Xaml_IFrameworkElement* pFrameworkElement = NULL;
                    pStartSizingFrame->lpVtbl->QueryInterface(pStartSizingFrame, &IID_Windows_UI_Xaml_IUIElement, &pIUIElement);
                    if (pIUIElement)
                    {
                        pCanvasStatics->lpVtbl->GetLeft(pCanvasStatics, pIUIElement, &(drc.Left));
                        pCanvasStatics->lpVtbl->GetTop(pCanvasStatics, pIUIElement, &(drc.Top));
                    }
                    pStartSizingFrame->lpVtbl->QueryInterface(pStartSizingFrame, &IID_Windows_UI_Xaml_IFrameworkElement, &pFrameworkElement);
                    if (pFrameworkElement)
                    {
                        pFrameworkElement->lpVtbl->get_ActualWidth(pFrameworkElement, &(drc.Right));
                        pFrameworkElement->lpVtbl->get_ActualHeight(pFrameworkElement, &(drc.Bottom));
                    }
                    UINT dpi = GetDpiForWindow(hWnd);
                    RECT rc;
                    SetRect(&rc, drc.Left, drc.Top, drc.Right, drc.Bottom);
                    SetRect(&rc, MulDiv(rc.left, dpi, 96), MulDiv(rc.top, dpi, 96), MulDiv(rc.right, dpi, 96), MulDiv(rc.bottom, dpi, 96));
                    *rect = rc;
                    if (bApply && dpi == 120)
                    {
                        HANDLE hRealThreadHandle = NULL;
                        DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hRealThreadHandle, THREAD_SET_CONTEXT, FALSE, 0);
                        if (hRealThreadHandle)
                        {
                            QueueUserAPC(LVT_StartDocked_120DPIHack, hRealThreadHandle, 826);
                            CloseHandle(hRealThreadHandle);
                        }
                    }
                    else if (pFrameworkElement)
                    {
                        pFrameworkElement->lpVtbl->put_MaxHeight(pFrameworkElement, 726.0);
                    }
                    if (pFrameworkElement)
                    {
                        pFrameworkElement->lpVtbl->Release(pFrameworkElement);
                    }
                    if (pIUIElement)
                    {
                        pIUIElement->lpVtbl->Release(pIUIElement);
                    }
                    Windows_UI_Xaml_IDependencyObject* pStartSizingFramePanel = LVT_FindChildByClassName(pStartSizingFrame, pVisualTreeHelperStatics, L"StartDocked.StartSizingFramePanel", NULL);
                    if (pStartSizingFramePanel)
                    {
                        Windows_UI_Xaml_IDependencyObject* pContentPresenter = LVT_FindChildByClassName(pStartSizingFramePanel, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.ContentPresenter", NULL);
                        if (pContentPresenter)
                        {
                            Windows_UI_Xaml_IDependencyObject* pFrame = LVT_FindChildByClassName(pContentPresenter, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.Frame", NULL);
                            if (pFrame)
                            {
                                Windows_UI_Xaml_IDependencyObject* pContentPresenter2 = LVT_FindChildByClassName(pFrame, pVisualTreeHelperStatics, L"Windows.UI.Xaml.Controls.ContentPresenter", NULL);
                                if (pContentPresenter2)
                                {
                                    Windows_UI_Xaml_IDependencyObject* pLauncherFrame = LVT_FindChildByClassName(pContentPresenter2, pVisualTreeHelperStatics, L"StartDocked.LauncherFrame", NULL);
                                    if (pLauncherFrame)
                                    {
                                        Windows_UI_Xaml_IDependencyObject* pRootPanel = LVT_FindChildByName(pLauncherFrame, pVisualTreeHelperStatics, L"RootPanel");
                                        Windows_UI_Xaml_IDependencyObject* pRootGridParent = pRootPanel;
                                        if (!pRootGridParent)
                                        {
                                            pRootGridParent = pLauncherFrame;
                                        }
                                        Windows_UI_Xaml_IDependencyObject* pRootGrid = LVT_FindChildByName(pRootGridParent, pVisualTreeHelperStatics, L"RootGrid");
                                        if (pRootGrid)
                                        {
                                            Windows_UI_Xaml_IDependencyObject* pRootContent = LVT_FindChildByName(pRootGrid, pVisualTreeHelperStatics, L"RootContent");
                                            if (pRootContent)
                                            {
                                                Windows_UI_Xaml_IDependencyObject* pMainContent = LVT_FindChildByName(pRootContent, pVisualTreeHelperStatics, L"MainContent");
                                                if (pMainContent)
                                                {
                                                    Windows_UI_Xaml_IDependencyObject* pInnerContent = NULL;
                                                    Windows_UI_Xaml_IDependencyObject* pUndockedRoot = LVT_FindChildByName(pMainContent, pVisualTreeHelperStatics, L"UndockedRoot");
                                                    if (!pUndockedRoot)
                                                    {
                                                        pInnerContent = LVT_FindChildByName(pMainContent, pVisualTreeHelperStatics, L"InnerContent");
                                                        if (pInnerContent)
                                                        {
                                                            pUndockedRoot = LVT_FindChildByName(pInnerContent, pVisualTreeHelperStatics, L"UndockedRoot");
                                                        }
                                                    }
                                                    if (pUndockedRoot)
                                                    {
                                                        Windows_UI_Xaml_IDependencyObject* pStartInnerFrame = LVT_FindChildByClassName(pUndockedRoot, pVisualTreeHelperStatics, L"StartMenu.StartInnerFrame", NULL);
                                                        if (pStartInnerFrame)
                                                        {
                                                            Windows_UI_Xaml_IDependencyObject* pFrameRoot = LVT_FindChildByName(pStartInnerFrame, pVisualTreeHelperStatics, L"FrameRoot");
                                                            if (pFrameRoot)
                                                            {
                                                                Windows_UI_Xaml_IDependencyObject* pTopLevelRoot = LVT_FindChildByName(pFrameRoot, pVisualTreeHelperStatics, L"TopLevelRoot");
                                                                if (pTopLevelRoot)
                                                                {
                                                                    Windows_UI_Xaml_IDependencyObject* pStartMenuPinnedList = LVT_FindChildByName(pTopLevelRoot, pVisualTreeHelperStatics, L"StartMenuPinnedList");
                                                                    if (pStartMenuPinnedList)
                                                                    {
                                                                        Windows_UI_Xaml_IFrameworkElement* pFrameworkElement = NULL;
                                                                        pStartMenuPinnedList->lpVtbl->QueryInterface(pStartMenuPinnedList, &IID_Windows_UI_Xaml_IFrameworkElement, &pFrameworkElement);
                                                                        if (pFrameworkElement)
                                                                        {
                                                                            static double StartMenuPinnedList_Height = 252.0;
                                                                            double tempStartMenuPinnedList_Height = 0.0;
                                                                            if (SUCCEEDED(pFrameworkElement->lpVtbl->get_Height(pFrameworkElement, &tempStartMenuPinnedList_Height)) && tempStartMenuPinnedList_Height != 510.0) StartMenuPinnedList_Height = tempStartMenuPinnedList_Height;

                                                                            if (bApply)
                                                                            {
                                                                                pFrameworkElement->lpVtbl->put_Height(pFrameworkElement, 510.0);
                                                                            }
                                                                            else
                                                                            {
                                                                                pFrameworkElement->lpVtbl->put_Height(pFrameworkElement, StartMenuPinnedList_Height);
                                                                            }
                                                                            pFrameworkElement->lpVtbl->Release(pFrameworkElement);
                                                                        }
                                                                        Windows_UI_Xaml_IDependencyObject* pRoot = LVT_FindChildByName(pStartMenuPinnedList, pVisualTreeHelperStatics, L"Root");
                                                                        if (pRoot)
                                                                        {
                                                                            Windows_UI_Xaml_IDependencyObject* pPinnedListPipsPager = LVT_FindChildByName(pRoot, pVisualTreeHelperStatics, L"PinnedListPipsPager");
                                                                            if (pPinnedListPipsPager)
                                                                            {
                                                                                Windows_UI_Xaml_IUIElement* pIUIElement = NULL;
                                                                                pPinnedListPipsPager->lpVtbl->QueryInterface(pPinnedListPipsPager, &IID_Windows_UI_Xaml_IUIElement, &pIUIElement);
                                                                                if (pIUIElement)
                                                                                {
                                                                                    if (bApply)
                                                                                    {
                                                                                        pIUIElement->lpVtbl->put_Visibility(pIUIElement, Windows_UI_Xaml_Visibility_Collapsed);
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        pIUIElement->lpVtbl->put_Visibility(pIUIElement, Windows_UI_Xaml_Visibility_Visible);
                                                                                    }
                                                                                    pIUIElement->lpVtbl->Release(pIUIElement);
                                                                                }
                                                                                pPinnedListPipsPager->lpVtbl->Release(pPinnedListPipsPager);
                                                                            }
                                                                            pRoot->lpVtbl->Release(pRoot);
                                                                        }
                                                                        pStartMenuPinnedList->lpVtbl->Release(pStartMenuPinnedList);
                                                                    }
                                                                    pTopLevelRoot->lpVtbl->Release(pTopLevelRoot);
                                                                }
                                                                pFrameRoot->lpVtbl->Release(pFrameRoot);
                                                            }
                                                            pStartInnerFrame->lpVtbl->Release(pStartInnerFrame);
                                                        }
                                                        pUndockedRoot->lpVtbl->Release(pUndockedRoot);
                                                    }
                                                    if (pInnerContent)
                                                    {
                                                        pInnerContent->lpVtbl->Release(pInnerContent);
                                                    }
                                                    pMainContent->lpVtbl->Release(pMainContent);
                                                }
                                                pRootContent->lpVtbl->Release(pRootContent);
                                            }
                                            pRootGrid->lpVtbl->Release(pRootGrid);
                                        }
                                        if (pRootPanel)
                                        {
                                            pRootPanel->lpVtbl->Release(pRootPanel);
                                        }
                                        pLauncherFrame->lpVtbl->Release(pLauncherFrame);
                                    }
                                    pContentPresenter2->lpVtbl->Release(pContentPresenter2);
                                }
                                pFrame->lpVtbl->Release(pFrame);
                            }
                            pContentPresenter->lpVtbl->Release(pContentPresenter);
                        }
                        pStartSizingFramePanel->lpVtbl->Release(pStartSizingFramePanel);
                    }
                    pStartSizingFrame->lpVtbl->Release(pStartSizingFrame);
                }
                pVisualTreeHelperStatics->lpVtbl->Release(pVisualTreeHelperStatics);
            }
            WindowsDeleteString(hsVisualTreeHelperStatics);
        }
        pRootDependencyObject->lpVtbl->Release(pRootDependencyObject);
    }

    if (pCanvasStatics)
    {
        pCanvasStatics->lpVtbl->Release(pCanvasStatics);
    }
}

HRESULT IsThreadCoreWindowVisible(BOOL* bIsVisible)
{
    HRESULT hr = S_OK;
    if (SUCCEEDED(hr))
    {
        HSTRING_HEADER hshWindowStatics;
        HSTRING hsWindowStatics = NULL;
        hr = WindowsCreateStringReference(L"Windows.UI.Xaml.Window", 22, &hshWindowStatics, &hsWindowStatics);
        if (SUCCEEDED(hr) && hsWindowStatics)
        {
            Windows_UI_Xaml_IWindowStatics* pWindowStatics = NULL;
            hr = RoGetActivationFactory(hsWindowStatics, &IID_Windows_UI_Xaml_IWindowStatics, &pWindowStatics);
            if (SUCCEEDED(hr))
            {
                Windows_UI_Xaml_IWindow* pWindow = NULL;
                hr = pWindowStatics->lpVtbl->get_Current(pWindowStatics, &pWindow);
                if (SUCCEEDED(hr))
                {
                    Windows_UI_Xaml_Core_ICoreWindow* pCoreWindow = NULL;
                    hr = pWindow->lpVtbl->get_CoreWindow(pWindow, &pCoreWindow);
                    if (SUCCEEDED(hr))
                    {
                        hr = pCoreWindow->lpVtbl->get_Visible(pCoreWindow, bIsVisible);
                        pCoreWindow->lpVtbl->Release(pCoreWindow);
                    }
                    pWindow->lpVtbl->Release(pWindow);
                }
                pWindowStatics->lpVtbl->Release(pWindowStatics);
            }
            WindowsDeleteString(hsWindowStatics);
        }
    }
    return hr;
}

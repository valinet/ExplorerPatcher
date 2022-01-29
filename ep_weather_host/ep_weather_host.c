#include "ep_weather_host.h"
#include "ep_weather_provider_google_html.h"
#include "ep_weather_provider_google_script.h"
#include "ep_weather_error_html.h"

LPCWSTR EP_Weather_Script_Provider_Google = L"<!DOCTYPE html>\n";

HRESULT STDMETHODCALLTYPE INetworkListManagerEvents_QueryInterface(void* _this, REFIID riid, void** ppv)
{
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_INetworkListManagerEvents))
        *ppv = _this;
    else
    {
        *ppv = 0;
        return(E_NOINTERFACE);
    }
    return S_OK;
}

ULONG STDMETHODCALLTYPE INetworkListManagerEvents_AddRefRelease(void* _this)
{
    return 1;
}

HRESULT STDMETHODCALLTYPE INetworkListManagerEvents_ConnectivityChanged(void* _this2, NLM_CONNECTIVITY newConnectivity)
{
    EPWeather* _this = GetWindowLongPtrW(FindWindowW(_T(EPW_WEATHER_CLASSNAME), NULL), GWLP_USERDATA);
    if (_this)
    {
        if ((newConnectivity & (NLM_CONNECTIVITY_IPV4_INTERNET | NLM_CONNECTIVITY_IPV6_INTERNET)) != 0)
        {
            printf("[Network Events] Internet connection status is: Available.\n");
            LONG64 dwUpdateSchedule = InterlockedAdd64(&_this->dwUpdateSchedule, 0);
            PostMessageW(_this->hWnd, EP_WEATHER_WM_FETCH_DATA, 0, 0);
            SetTimer(_this->hWnd, EP_WEATHER_TIMER_SCHEDULE_REFRESH, dwUpdateSchedule, NULL);
            printf("[Network Events] Reinstalled refresh timer.\n");
        }
        else
        {
            printf("[Network Events] Internet connection status is: Offline.\n");
            KillTimer(_this->hWnd, EP_WEATHER_TIMER_SCHEDULE_REFRESH);
            printf("[Network Events] Killed refresh timer.\n");
        }
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_static_Stub(void* _this)
{
    return S_OK;
}

ULONG STDMETHODCALLTYPE epw_Weather_static_AddRefRelease(void* _this)
{
    return 1;
}

HRESULT STDMETHODCALLTYPE epw_Weather_static_QueryInterface(void* _this, REFIID riid, void** ppv)
{
    *ppv = _this;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_get_AdditionalBrowserArguments(ICoreWebView2EnvironmentOptions* _this, LPWSTR* value)
{
    *value = CoTaskMemAlloc(82 * sizeof(WCHAR));
    if (*value)
    {
        wcscpy_s(*value, 82, L"--disable-site-isolation-trials --disable-web-security --allow-insecure-localhost");
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_get_Language(ICoreWebView2EnvironmentOptions* _this, LPWSTR* value)
{
    *value = CoTaskMemAlloc(6 * sizeof(WCHAR));
    if (*value)
    {
        wcscpy_s(*value, 6, L"en-US");
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_get_TargetCompatibleBrowserVersion(ICoreWebView2EnvironmentOptions* _this, LPWSTR* value)
{
    *value = CoTaskMemAlloc(13 * sizeof(WCHAR));
    if (*value)
    {
        wcscpy_s(*value, 13, L"97.0.1072.69");
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_get_AllowSingleSignOnUsingOSPrimaryAccount(ICoreWebView2EnvironmentOptions* _this, BOOL* allow)
{
    *allow = TRUE;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_CreateCoreWebView2EnvironmentCompleted(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* _this, HRESULT hr, ICoreWebView2Environment* pCoreWebView2Environemnt)
{
    pCoreWebView2Environemnt->lpVtbl->CreateCoreWebView2Controller(pCoreWebView2Environemnt, FindWindowW(_T(EPW_WEATHER_CLASSNAME), NULL), &EPWeather_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE _epw_Weather_NavigateToError(EPWeather* _this)
{
    InterlockedExchange64(&_this->bIsNavigatingToError, TRUE);
    return _this->pCoreWebView2->lpVtbl->NavigateToString(_this->pCoreWebView2, ep_weather_error_html);
}

HRESULT STDMETHODCALLTYPE _epw_Weather_NavigateToProvider(EPWeather* _this)
{
    HRESULT hr = S_OK;
    LONG64 dwProvider = InterlockedAdd64(&_this->dwProvider, 0);
    if (dwProvider == EP_WEATHER_PROVIDER_TEST)
    {
    }
    else if (dwProvider == EP_WEATHER_PROVIDER_GOOGLE)
    {
        //hr = _this->pCoreWebView2->lpVtbl->Navigate(_this->pCoreWebView2, L"https://google.com");
        _this->wszScriptData = malloc(sizeof(WCHAR) * EP_WEATHER_PROVIDER_GOOGLE_HTML_LEN);
        if (_this->wszScriptData)
        {
            swprintf_s(_this->wszScriptData, EP_WEATHER_PROVIDER_GOOGLE_HTML_LEN, ep_weather_provider_google_html, _this->wszLanguage, _this->wszTerm[0] ? L" " : L"", _this->wszTerm);
            hr = _this->pCoreWebView2->lpVtbl->NavigateToString(_this->pCoreWebView2, _this->wszScriptData);
            if (FAILED(hr))
            {
                InterlockedExchange64(&_this->bBrowserBusy, FALSE);
                free(_this->wszScriptData);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE _epw_Weather_ExecuteDataScript(EPWeather* _this)
{
    HRESULT hr = S_OK;
    LONG64 dwProvider = InterlockedAdd64(&_this->dwProvider, 0);
    if (dwProvider == EP_WEATHER_PROVIDER_TEST)
    {
    }
    else if (dwProvider == EP_WEATHER_PROVIDER_GOOGLE)
    {
        _this->wszScriptData = malloc(sizeof(WCHAR) * EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_LEN);
        if (_this->wszScriptData)
        {
            LONG64 dwTemperatureUnit = InterlockedAdd64(&_this->dwTemperatureUnit, 0);
            LONG64 cbx = InterlockedAdd64(&_this->cbx, 0);
            swprintf_s(_this->wszScriptData, EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_LEN, ep_weather_provider_google_script, dwTemperatureUnit == EP_WEATHER_TUNIT_FAHRENHEIT ? L'F' : L'C', cbx, cbx);
            //wprintf(L"%s\n", _this->wszScriptData);
            hr = _this->pCoreWebView2->lpVtbl->ExecuteScript(_this->pCoreWebView2, _this->wszScriptData, &EPWeather_ICoreWebView2ExecuteScriptCompletedHandler);
            if (FAILED(hr))
            {
                InterlockedExchange64(&_this->bBrowserBusy, FALSE);
                free(_this->wszScriptData);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_CreateCoreWebView2ControllerCompleted(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* _this2, HRESULT hr, ICoreWebView2Controller* pCoreWebView2Controller)
{
    EPWeather* _this = GetWindowLongPtrW(FindWindowW(_T(EPW_WEATHER_CLASSNAME), NULL), GWLP_USERDATA);
    if (!_this->pCoreWebView2Controller)
    {
        _this->pCoreWebView2Controller = pCoreWebView2Controller;
        _this->pCoreWebView2Controller->lpVtbl->get_CoreWebView2(_this->pCoreWebView2Controller, &_this->pCoreWebView2);
        _this->pCoreWebView2Controller->lpVtbl->AddRef(_this->pCoreWebView2Controller);
    }

    RECT bounds;
    GetClientRect(_this->hWnd, &bounds);
    _this->pCoreWebView2Controller->lpVtbl->put_Bounds(_this->pCoreWebView2Controller, bounds);

    ICoreWebView2Controller2* pCoreWebView2Controller2 = NULL;
    _this->pCoreWebView2Controller->lpVtbl->QueryInterface(_this->pCoreWebView2Controller, &IID_ICoreWebView2Controller2, &pCoreWebView2Controller2);
    if (pCoreWebView2Controller2)
    {
        COREWEBVIEW2_COLOR transparent;
        transparent.A = 0;
        transparent.R = 0;
        transparent.G = 0;
        transparent.B = 0;
        pCoreWebView2Controller2->lpVtbl->put_DefaultBackgroundColor(pCoreWebView2Controller2, transparent);
        pCoreWebView2Controller2->lpVtbl->Release(pCoreWebView2Controller2);
    }

    ICoreWebView2Settings* pCoreWebView2Settings = NULL;
    _this->pCoreWebView2->lpVtbl->get_Settings(_this->pCoreWebView2, &pCoreWebView2Settings);
    if (pCoreWebView2Settings)
    {
        ICoreWebView2Settings6* pCoreWebView2Settings6 = NULL;
        pCoreWebView2Settings->lpVtbl->QueryInterface(pCoreWebView2Settings, &IID_ICoreWebView2Settings6, &pCoreWebView2Settings6);
        if (pCoreWebView2Settings6)
        {
            pCoreWebView2Settings6->lpVtbl->put_AreDevToolsEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_IsStatusBarEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_IsZoomControlEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_IsGeneralAutofillEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_IsPasswordAutosaveEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_IsPinchZoomEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_IsSwipeNavigationEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_AreBrowserAcceleratorKeysEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_AreDefaultContextMenusEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_AreDefaultScriptDialogsEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->Release(pCoreWebView2Settings6);
        }
        pCoreWebView2Settings->lpVtbl->Release(pCoreWebView2Settings);
    }

    _this->pCoreWebView2->lpVtbl->add_NavigationCompleted(_this->pCoreWebView2, &EPWeather_ICoreWebView2NavigationCompletedEventHandler, &_this->tkOnNavigationCompleted);

    _epw_Weather_NavigateToProvider(_this);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_NavigationCompleted(ICoreWebView2NavigationCompletedEventHandler* _this2, ICoreWebView2* pCoreWebView2, ICoreWebView2NavigationCompletedEventArgs* pCoreWebView2NavigationCompletedEventArgs)
{
    EPWeather* _this = GetWindowLongPtrW(FindWindowW(_T(EPW_WEATHER_CLASSNAME), NULL), GWLP_USERDATA);
    if (_this->wszScriptData)
    {
        free(_this->wszScriptData);
    }
    BOOL bIsSuccess = FALSE;
    pCoreWebView2NavigationCompletedEventArgs->lpVtbl->get_IsSuccess(pCoreWebView2NavigationCompletedEventArgs, &bIsSuccess);
    if (bIsSuccess)
    {
        BOOL bIsNavigatingToError = InterlockedAdd64(&_this->bIsNavigatingToError, 0);
        if (bIsNavigatingToError)
        {
            InterlockedExchange64(&_this->bIsNavigatingToError, FALSE);
            InterlockedExchange64(&_this->bBrowserBusy, FALSE);
        }
        else
        {
            _epw_Weather_ExecuteDataScript(_this);
        }
    }
    else
    {
        _epw_Weather_NavigateToError(_this);
    }
    _this->pCoreWebView2Controller->lpVtbl->put_IsVisible(_this->pCoreWebView2Controller, FALSE);
    _this->pCoreWebView2Controller->lpVtbl->put_IsVisible(_this->pCoreWebView2Controller, TRUE);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_ExecuteScriptCompleted(ICoreWebView2ExecuteScriptCompletedHandler* _this2, HRESULT hr, LPCWSTR pResultObjectAsJson)
{
    EPWeather* _this = GetWindowLongPtrW(FindWindowW(_T(EPW_WEATHER_CLASSNAME), NULL), GWLP_USERDATA);
    if (_this)
    {
        BOOL bOk = FALSE;
        LONG64 dwProvider = InterlockedAdd64(&_this->dwProvider, 0);
        if (dwProvider == EP_WEATHER_PROVIDER_GOOGLE)
        {
            if (!_wcsicmp(pResultObjectAsJson, L"\"run_part_2\""))
            {
                //_this->pCoreWebView2->lpVtbl->OpenDevToolsWindow(_this->pCoreWebView2);

                //printf("running part 2\n");
                _this->pCoreWebView2->lpVtbl->ExecuteScript(_this->pCoreWebView2, ep_weather_provider_google_script2, &EPWeather_ICoreWebView2ExecuteScriptCompletedHandler);
                bOk = TRUE;
            }
            else
            {
                free(_this->wszScriptData);
                _this->wszScriptData = NULL;

                //wprintf(L"%s\n", pResultObjectAsJson);

                epw_Weather_LockData(_this);

                WCHAR* wszTemperature = pResultObjectAsJson + 1;
                if (wszTemperature)
                {
                    WCHAR* wszUnit = wcschr(wszTemperature, L'#');
                    if (wszUnit)
                    {
                        wszUnit[0] = 0;
                        wszUnit++;
                        WCHAR* wszCondition = wcschr(wszUnit, L'#');
                        if (wszCondition)
                        {
                            wszCondition[0] = 0;
                            wszCondition++;
                            WCHAR* wszLocation = wcschr(wszCondition, L'#');
                            if (wszLocation)
                            {
                                wszLocation[0] = 0;
                                wszLocation++;
                                WCHAR* pImage = wcschr(wszLocation, L'#');
                                if (pImage)
                                {
                                    pImage[0] = 0;
                                    pImage++;
                                    WCHAR* pTerm = wcschr(pImage, L'"');
                                    if (pTerm)
                                    {
                                        pTerm[0] = 0;
                                        if (_this->wszTemperature)
                                        {
                                            free(_this->wszTemperature);
                                        }
                                        if (_this->wszUnit)
                                        {
                                            free(_this->wszUnit);
                                        }
                                        if (_this->wszCondition)
                                        {
                                            free(_this->wszCondition);
                                        }
                                        if (_this->pImage)
                                        {
                                            free(_this->pImage);
                                        }
                                        if (_this->wszLocation)
                                        {
                                            free(_this->wszLocation);
                                        }
                                        _this->cbTemperature = (wcslen(wszTemperature) + 1) * sizeof(WCHAR);
                                        _this->wszTemperature = malloc(_this->cbTemperature);
                                        _this->cbUnit = (wcslen(wszUnit) + 1) * sizeof(WCHAR);
                                        _this->wszUnit = malloc(_this->cbUnit);
                                        _this->cbCondition = (wcslen(wszCondition) + 1) * sizeof(WCHAR);
                                        _this->wszCondition = malloc(_this->cbCondition);
                                        _this->cbImage = wcslen(pImage) / 2;
                                        _this->pImage = malloc(_this->cbImage);
                                        _this->cbLocation = (wcslen(wszLocation) + 1) * sizeof(WCHAR);
                                        _this->wszLocation = malloc(_this->cbLocation);
                                        if (_this->wszTemperature && _this->wszUnit && _this->wszCondition && _this->pImage && _this->wszLocation)
                                        {
                                            wcscpy_s(_this->wszTemperature, _this->cbTemperature / 2, wszTemperature);
                                            wcscpy_s(_this->wszUnit, _this->cbUnit / 2, wszUnit);
                                            wcscpy_s(_this->wszCondition, _this->cbCondition / 2, wszCondition);
                                            wcscpy_s(_this->wszLocation, _this->cbLocation / 2, wszLocation);

                                            for (unsigned int i = 0; i < _this->cbImage * 2; i = i + 2)
                                            {
                                                WCHAR tmp[3];
                                                tmp[0] = pImage[i];
                                                tmp[1] = pImage[i + 1];
                                                tmp[2] = 0;
                                                _this->pImage[i / 2] = wcstol(tmp, NULL, 16);
                                            }

                                            bOk = TRUE;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                epw_Weather_UnlockData(_this);
            }
        }

        if (!bOk)
        {
            printf("[General] Navigating to error page.\n");
            _epw_Weather_NavigateToError(_this);
        }
        else
        {
            InterlockedExchange64(&_this->bBrowserBusy, FALSE);
            printf("[General] Fetched data, requesting redraw.\n");
            SetTimer(_this->hWnd, EP_WEATHER_TIMER_REQUEST_REPAINT, EP_WEATHER_TIMER_REQUEST_REPAINT_DELAY, NULL);
        }
    }
    return S_OK;
}

ULONG STDMETHODCALLTYPE epw_Weather_AddRef(EPWeather* _this)
{
    return InterlockedIncrement64(&(_this->cbCount));
}

ULONG STDMETHODCALLTYPE epw_Weather_Release(EPWeather* _this)
{
    ULONG value = InterlockedDecrement64(&(_this->cbCount));
    if (value == 0)
    {
        if (_this->hMainThread)
        {
            SetEvent(_this->hSignalExitMainThread);
            WaitForSingleObject(_this->hMainThread, INFINITE);
            CloseHandle(_this->hMainThread);
        }
        if (_this->hInitializeEvent)
        {
            CloseHandle(_this->hInitializeEvent);
        }

        if (_this->hMutexData)
        {
            CloseHandle(_this->hMutexData);
        }

        FREE(_this);
        LONG dwOutstandingObjects = InterlockedDecrement(&epw_OutstandingObjects);
        LONG dwOutstandingLocks = InterlockedAdd(&epw_LockCount, 0);
        if (!dwOutstandingObjects && !dwOutstandingLocks)
        {
        }

        return(0);
    }
    return value;
}

HRESULT STDMETHODCALLTYPE epw_Weather_QueryInterface(EPWeather* _this, REFIID riid, void** ppv)
{
    if (!IsEqualIID(riid, &IID_IEPWeather) &&
        !IsEqualIID(riid, &IID_IUnknown))
    {
        *ppv = 0;
        return(E_NOINTERFACE);
    }
    *ppv = _this;
    _this->lpVtbl->AddRef(_this);
    return(NOERROR);
}

HRESULT STDMETHODCALLTYPE epw_Weather_About(EPWeather* _this, HWND hWnd)
{
    HRESULT hr = NOERROR;

    if (SUCCEEDED(hr))
    {
        hr = !_this ? (E_NOINTERFACE) : hr;
    }
    if (SUCCEEDED(hr))
    {
        wchar_t text[MAX_PATH];

        DWORD dwLeftMost = 0;
        DWORD dwSecondLeft = 0;
        DWORD dwSecondRight = 0;
        DWORD dwRightMost = 0;

        QueryVersionInfo(epw_hModule, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

        swprintf_s(text, MAX_PATH, L"ExplorerPatcher Weather Host\r\n\r\nVersion %d.%d.%d.%d", dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);

        MessageBoxW(hWnd, text, _T("ExplorerPatcher Weather Host"), MB_ICONINFORMATION);
    }

    return hr;
}

LRESULT CALLBACK epw_Weather_WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    EPWeather* _this = NULL;
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
        _this = (int*)(pCreate->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)_this);
    }
    else
    {
        LONG_PTR ptr = GetWindowLongPtrW(hWnd, GWLP_USERDATA);
        _this = (EPWeather*)(ptr);
    }
    if (!_this)
    {
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    if (uMsg == WM_TIMER && wParam == EP_WEATHER_TIMER_REQUEST_REPAINT)
    {
        HWND hNotifyWnd = InterlockedAdd64(&_this->hNotifyWnd, 0);
        printf("[Timer Repaint] Request posted to window %x.\n", hNotifyWnd);
        if (hNotifyWnd)
        {
            InvalidateRect(hNotifyWnd, NULL, TRUE);
            //Sleep(100);
            //InvalidateRect(hNotifyWnd, NULL, TRUE);
        }
        KillTimer(_this->hWnd, EP_WEATHER_TIMER_REQUEST_REPAINT);
        return 0;
    }
    else if (uMsg == WM_TIMER && wParam == EP_WEATHER_TIMER_SCHEDULE_REFRESH)
    {
        if (SendMessageW(_this->hWnd, EP_WEATHER_WM_FETCH_DATA, 0, 0))
        {
            printf("[Timer Scheduled Refresh] Browser is busy, waiting a minute and retrying...\n");
            KillTimer(_this->hWnd, EP_WEATHER_TIMER_SCHEDULE_REFRESH);
            SetTimer(_this->hWnd, EP_WEATHER_TIMER_SCHEDULE_REFRESH, 1000 * 60, NULL);
        }
        else
        {
            KillTimer(_this->hWnd, EP_WEATHER_TIMER_SCHEDULE_REFRESH);
            LONG64 dwUpdateSchedule = InterlockedAdd64(&_this->dwUpdateSchedule, 0);
            printf("[Timer Scheduled Refresh] Fetching data, sleeping for %lld more ms.\n", dwUpdateSchedule);
            SetTimer(_this->hWnd, EP_WEATHER_TIMER_SCHEDULE_REFRESH, dwUpdateSchedule, NULL);
        }
        return 0;
    }
    else if (uMsg == WM_TIMER && wParam == EP_WEATHER_TIMER_REBOUND_BROWSER)
    {
        RECT bounds;
        GetClientRect(_this->hWnd, &bounds);
        _this->pCoreWebView2Controller->lpVtbl->put_Bounds(_this->pCoreWebView2Controller, bounds);
        KillTimer(_this->hWnd, EP_WEATHER_TIMER_REBOUND_BROWSER);
    }
    else if (uMsg == EP_WEATHER_WM_FETCH_DATA)
    {
        INT64 bWasBrowserBusy = InterlockedCompareExchange64(&_this->bBrowserBusy, TRUE, FALSE);
        if (!bWasBrowserBusy)
        {
            return _epw_Weather_NavigateToProvider(_this);
        }
        return HRESULT_FROM_WIN32(ERROR_BUSY);
    }
    else if (uMsg == WM_CLOSE || (uMsg == WM_KEYUP && wParam == VK_ESCAPE) || (uMsg == WM_ACTIVATEAPP && wParam == FALSE && GetAncestor(GetForegroundWindow(), GA_ROOT) != _this->hWnd))
    {
        epw_Weather_Hide(_this);
        return 0;
    }
    else if (uMsg == WM_NCHITTEST)
    {
        LRESULT lRes = DefWindowProcW(hWnd, uMsg, wParam, lParam);
        if (lRes == HTCAPTION)
        {
            return HTCLIENT;
        }
    }
    else if (uMsg == WM_WINDOWPOSCHANGING)
    {
        if (IsWindowVisible(hWnd))
        {
            WINDOWPOS* pwp = (WINDOWPOS*)lParam;
            pwp->flags |= SWP_NOMOVE | SWP_NOSIZE;
        }
        return 0;
    }

    BOOL bIsRunningWithoutVisualStyle = !IsThemeActive() || IsHighContrast();
    if (uMsg == WM_CREATE)
    {
        if (bIsRunningWithoutVisualStyle)
        {
            SetRectEmpty(&_this->rcBorderThickness);
            if (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_THICKFRAME)
            {
                AdjustWindowRectEx(&_this->rcBorderThickness, GetWindowLongPtr(hWnd, GWL_STYLE) & ~WS_CAPTION, FALSE, NULL);
                _this->rcBorderThickness.left *= -1;
                _this->rcBorderThickness.top *= -1;
            }
            else if (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_BORDER)
            {
                SetRect(&_this->rcBorderThickness, 1, 1, 1, 1);
            }
            SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        }
    }
    else if (uMsg == WM_NCCALCSIZE)
    {
        if (bIsRunningWithoutVisualStyle)
        {
            if (lParam)
            {
                NCCALCSIZE_PARAMS* sz = (NCCALCSIZE_PARAMS*)lParam;
                sz->rgrc[0].left += _this->rcBorderThickness.left;
                sz->rgrc[0].right -= _this->rcBorderThickness.right;
                sz->rgrc[0].bottom -= _this->rcBorderThickness.bottom;
                return 0;
            }
        }
    }
    else if (uMsg == WM_NCHITTEST)
    {
        if (bIsRunningWithoutVisualStyle)
        {
            LRESULT lRes = DefWindowProcW(hWnd, uMsg, wParam, lParam);
            if (lRes == HTCLIENT)
            {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                ScreenToClient(hWnd, &pt);
                if (pt.y < _this->rcBorderThickness.top)
                {
                    return HTTOP;
                }
                else
                {
                    return HTCAPTION;
                }
            }
            else
            {
                return lRes;
            }
        }
    }
    else if (uMsg == WM_NCACTIVATE)
    {
        if (bIsRunningWithoutVisualStyle)
        {
            return 0;
        }
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI epw_Weather_MainThread(EPWeather* _this)
{
    HRESULT hr = S_OK;

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    _this->hrLastError = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(_this->hrLastError))
    {
        goto cleanup;
    }

    _this->hrLastError = CoCreateInstance(
        &CLSID_TaskbarList,
        NULL,
        CLSCTX_INPROC,
        &IID_ITaskbarList,
        (LPVOID*)&_this->pTaskList
    );
    if (FAILED(_this->hrLastError))
    {
        goto cleanup;
    }

    WNDCLASSW wc;
    ZeroMemory(&wc, sizeof(WNDCLASSW));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = epw_Weather_WindowProc;
    wc.hInstance = epw_hModule;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = _T(EPW_WEATHER_CLASSNAME);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    if (!RegisterClassW(&wc))
    {
        _this->hrLastError = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    _this->hWnd = CreateWindowExW(0, _T(EPW_WEATHER_CLASSNAME), L"", WS_OVERLAPPED | WS_CAPTION, 100, 100, 690 * _this->dpi, 425 * _this->dpi, NULL, NULL, epw_hModule, _this); // 1030, 630
    if (!_this->hWnd)
    {
        _this->hrLastError = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    SetPropW(_this->hWnd, L"valinet.ExplorerPatcher.ShellManagedWindow", TRUE);

    _this->hrLastError = _this->pTaskList->lpVtbl->DeleteTab(_this->pTaskList, _this->hWnd);
    if (FAILED(_this->hrLastError))
    {
        goto cleanup;
    }

    WCHAR wszWorkFolder[MAX_PATH];
    ZeroMemory(wszWorkFolder, MAX_PATH * sizeof(WCHAR));
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wszWorkFolder);
    wcscat_s(wszWorkFolder, MAX_PATH, L"\\ExplorerPatcher\\ep_weather_host");
    BOOL bRet = CreateDirectoryW(wszWorkFolder, NULL);
    if (!(bRet || (!bRet && GetLastError() == ERROR_ALREADY_EXISTS)))
    {
        _this->hrLastError = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
        goto cleanup;
    }

    MARGINS marGlassInset = { -1, -1, -1, -1 }; // -1 means the whole window
    DwmExtendFrameIntoClientArea(_this->hWnd, &marGlassInset);
    BOOL value = 1;
    DwmSetWindowAttribute(_this->hWnd, 1029, &value, sizeof(BOOL));

    InterlockedExchange64(&_this->bBrowserBusy, TRUE);

    _this->hrLastError = CreateCoreWebView2EnvironmentWithOptions(NULL, wszWorkFolder, &EPWeather_ICoreWebView2EnvironmentOptions, &EPWeather_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler);
    if (FAILED(_this->hrLastError))
    {
        goto cleanup;
    }

    INetworkListManager* spManager = NULL;
    IConnectionPointContainer* spConnectionPoints = NULL;
    IConnectionPoint* spConnectionPoint = NULL;
    IUnknown* spSink = NULL;
    DWORD dwCookie = 0;

    if (SUCCEEDED(hr = CoCreateInstance(&CLSID_NetworkListManager, NULL, CLSCTX_ALL, &IID_INetworkListManager, &spManager) && spManager))
    {
        if (SUCCEEDED(hr = spManager->lpVtbl->QueryInterface(spManager, &IID_IConnectionPointContainer, &spConnectionPoints)))
        {
            if (SUCCEEDED(hr = spConnectionPoints->lpVtbl->FindConnectionPoint(spConnectionPoints, &IID_INetworkListManagerEvents, &spConnectionPoint)))
            {
                if (SUCCEEDED(hr = INetworkListManagerEvents_Instance.lpVtbl->QueryInterface(&INetworkListManagerEvents_Instance, &IID_IUnknown, &spSink)))
                {
                    if (SUCCEEDED(hr = spConnectionPoint->lpVtbl->Advise(spConnectionPoint, spSink, &dwCookie)))
                    {
                    }
                }
            }
        }
    }

    LONG64 dwUpdateSchedule = InterlockedAdd64(&_this->dwUpdateSchedule, 0);
    SetTimer(_this->hWnd, EP_WEATHER_TIMER_SCHEDULE_REFRESH, dwUpdateSchedule, NULL);

    SetEvent(_this->hInitializeEvent);

    MSG msg;
    while (TRUE)
    {
        DWORD dwRes = MsgWaitForMultipleObjects(EP_WEATHER_NUM_SIGNALS, &_this->hSignalExitMainThread, FALSE, INFINITE, QS_ALLINPUT);
        if (dwRes == WAIT_OBJECT_0)
        {
            break;
        }
        else if (dwRes == WAIT_OBJECT_0 + EP_WEATHER_NUM_SIGNALS)
        {
            BOOL bRet = 0, bQuit = FALSE;
            while (bRet = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (bRet == 0 || bRet == -1)
                {
                    bQuit = TRUE;
                    break;
                }
                else
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            if (bQuit)
            {
                break;
            }
        }
        else if (dwRes == WAIT_ABANDONED_0 + 1 || dwRes == WAIT_OBJECT_0 + 1)
        {
            CloseHandle(_this->hSignalKillSwitch);
            TerminateProcess(GetCurrentProcess(), 0);
        }
    }

    if (SUCCEEDED(hr))
    {
        spConnectionPoint->lpVtbl->Unadvise(spConnectionPoint, dwCookie);
    }
    if (spSink)
    {
        spSink->lpVtbl->Release(spSink);
    }
    if (spConnectionPoint)
    {
        spConnectionPoint->lpVtbl->Release(spConnectionPoint);
    }
    if (spConnectionPoints)
    {
        spConnectionPoints->lpVtbl->Release(spConnectionPoints);
    }
    if (spManager)
    {
        spManager->lpVtbl->Release(spManager);
    }

    cleanup:

    if (_this->pCoreWebView2)
    {
        _this->pCoreWebView2->lpVtbl->Release(_this->pCoreWebView2);
    }
    if (_this->pCoreWebView2Controller)
    {
        _this->pCoreWebView2Controller->lpVtbl->Release(_this->pCoreWebView2Controller);
    }
    if (_this->cbTemperature && _this->wszTemperature)
    {
        free(_this->wszTemperature);
    }
    if (_this->cbUnit && _this->wszUnit)
    {
        free(_this->wszUnit);
    }
    if (_this->cbCondition && _this->wszCondition)
    {
        free(_this->wszCondition);
    }
    if (_this->cbImage && _this->pImage)
    {
        free(_this->pImage);
    }
    if (_this->cbLocation && _this->wszLocation)
    {
        free(_this->wszLocation);
    }
    if (_this->hWnd)
    {
        DestroyWindow(_this->hWnd);
    }
    UnregisterClassW(_T(EPW_WEATHER_CLASSNAME), epw_hModule);
    if (_this->pTaskList)
    {
        _this->pTaskList->lpVtbl->Release(_this->pTaskList);
    }
    CoUninitialize();
    SetEvent(_this->hInitializeEvent);

    return 0;
}

HRESULT STDMETHODCALLTYPE epw_Weather_Initialize(EPWeather* _this, WCHAR wszName[MAX_PATH], BOOL bAllocConsole, LONG64 dwProvider, LONG64 cbx, LONG64 cby, LONG64 dwTemperatureUnit, LONG64 dwUpdateSchedule, double dpi)
{
    if (bAllocConsole)
    {
        FILE* conout;
        AllocConsole();
        freopen_s(
            &conout,
            "CONOUT$",
            "w",
            stdout
        );
    }

    _this->dpi = dpi;

    if (dwUpdateSchedule < 0)
    {
        return E_INVALIDARG;
    }
    InterlockedExchange64(&_this->dwUpdateSchedule, dwUpdateSchedule);

    if (dwTemperatureUnit < 0 || dwTemperatureUnit > EP_WEATHER_NUM_TUNITS)
    {
        return E_INVALIDARG;
    }
    InterlockedExchange64(&_this->dwTemperatureUnit, dwTemperatureUnit);

    if (dwProvider < 0 || dwProvider > EP_WEATHER_NUM_PROVIDERS)
    {
        return E_INVALIDARG;
    }
    InterlockedExchange64(&_this->dwProvider, dwProvider);

    if (!cbx || !cby)
    {
        return E_INVALIDARG;
    }
    InterlockedExchange64(&_this->cbx, cbx);
    InterlockedExchange64(&_this->cby, cby);

    _this->hSignalKillSwitch = CreateMutexW(NULL, FALSE, wszName);
    if (!_this->hSignalKillSwitch || GetLastError() != ERROR_ALREADY_EXISTS)
    {
        return E_INVALIDARG;
    }

    _this->hMutexData = CreateMutexW(NULL, FALSE, NULL);
    if (!_this->hMutexData)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    _this->hInitializeEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (!_this->hInitializeEvent)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    _this->hSignalExitMainThread = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (!_this->hSignalExitMainThread)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    _this->hMainThread = CreateThread(NULL, 0, epw_Weather_MainThread, _this, 0, NULL);
    if (!_this->hMainThread)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    WaitForSingleObject(_this->hInitializeEvent, INFINITE);
    if (FAILED(_this->hrLastError))
    {
        return _this->hrLastError;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_Show(EPWeather* _this)
{
    SetTimer(_this->hWnd, EP_WEATHER_TIMER_REBOUND_BROWSER, EP_WEATHER_TIMER_REBOUND_BROWSER_DELAY, NULL);
    ShowWindow(_this->hWnd, SW_SHOW);
    _this->pTaskList->lpVtbl->DeleteTab(_this->pTaskList, _this->hWnd);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_Hide(EPWeather* _this)
{
    ShowWindow(_this->hWnd, SW_HIDE);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_GetWindowHandle(EPWeather* _this, HWND* phWnd)
{
    *phWnd = _this->hWnd;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_LockData(EPWeather* _this)
{
    DWORD dwRes = WaitForSingleObject(_this->hMutexData, INFINITE);
    if (dwRes == WAIT_ABANDONED)
    {
        return HRESULT_FROM_WIN32(ERROR_ABANDONED_WAIT_0);
    }
    else if (dwRes == WAIT_FAILED)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_GetDataSizes(EPWeather* _this, LPDWORD pcbTemperature, LPDWORD pcbUnit, LPDWORD pcbCondition, LPDWORD pcbImage)
{
    *pcbTemperature = _this->cbTemperature;
    *pcbUnit = _this->cbUnit;
    *pcbCondition = _this->cbCondition;
    *pcbImage = _this->cbImage;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_GetData(EPWeather* _this, DWORD cbTemperature, LPCWSTR wszTemperature, DWORD cbUnit, LPCWSTR wszUnit, DWORD cbCondition, LPCWSTR wszCondition, DWORD cbImage, char* pImage)
{
    if (cbTemperature)
    {
        memcpy_s(wszTemperature, cbTemperature, _this->wszTemperature, _this->cbTemperature);
    }
    if (cbUnit)
    {
        memcpy_s(wszUnit, cbUnit, _this->wszUnit, _this->cbUnit);
    }
    if (cbCondition)
    {
        memcpy_s(wszCondition, cbCondition, _this->wszCondition, _this->cbCondition);
    }
    if (cbImage)
    {
        memcpy_s(pImage, cbImage, _this->pImage, _this->cbImage);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_GetTitle(EPWeather* _this, DWORD cbTitle, LPCWSTR wszTitle, DWORD dwType)
{
    WCHAR wszBuffer[MAX_PATH];
    if (cbTitle)
    {
        switch (dwType)
        {
        case EP_WEATHER_VIEW_ICONTEXT:
        case EP_WEATHER_VIEW_TEXTONLY:
            swprintf_s(wszBuffer, MAX_PATH, L"%s", _this->wszLocation);
            break;
        case EP_WEATHER_VIEW_ICONTEMP:
        case EP_WEATHER_VIEW_TEMPONLY:
            swprintf_s(wszBuffer, MAX_PATH, L"%s - %s", _this->wszLocation, _this->wszCondition);
            break;
        case EP_WEATHER_VIEW_ICONONLY:
            swprintf_s(wszBuffer, MAX_PATH, L"%s %s | %s - %s", _this->wszTemperature, _this->wszUnit, _this->wszLocation, _this->wszCondition);
            break;
        }
        memcpy_s(wszTitle, cbTitle, wszBuffer, MAX_PATH);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_UnlockData(EPWeather* _this)
{
    if (!ReleaseMutex(_this->hMutexData))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_IsInitialized(EPWeather* _this, BOOL* bIsInitialized)
{
    *bIsInitialized = _this->hInitializeEvent;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_SetNotifyWindow(EPWeather* _this, HWND hWndNotify)
{
    InterlockedExchange64(&_this->hNotifyWnd, hWndNotify);
}

HRESULT STDMETHODCALLTYPE epw_Weather_SetTemperatureUnit(EPWeather* _this, LONG64 dwTemperatureUnit)
{
    if (dwTemperatureUnit < 0 || dwTemperatureUnit > EP_WEATHER_NUM_TUNITS)
    {
        return E_INVALIDARG;
    }
    InterlockedExchange64(&_this->dwTemperatureUnit, dwTemperatureUnit);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_SetUpdateSchedule(EPWeather* _this, LONG64 dwUpdateSchedule)
{
    if (dwUpdateSchedule < 0)
    {
        return E_INVALIDARG;
    }
    LONG64 dwOldUpdateSchedule = InterlockedExchange64(&_this->dwUpdateSchedule, dwUpdateSchedule);
    if (dwOldUpdateSchedule != dwUpdateSchedule)
    {
        KillTimer(_this->hWnd, EP_WEATHER_TIMER_SCHEDULE_REFRESH);
        SetTimer(_this->hWnd, EP_WEATHER_TIMER_SCHEDULE_REFRESH, dwUpdateSchedule, NULL);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_SetTerm(EPWeather* _this, DWORD cbTerm, LPCWSTR wszTerm)
{
    if (cbTerm)
    {
        memcpy_s(_this->wszTerm, sizeof(WCHAR) * MAX_PATH, wszTerm, cbTerm);
    }
    else
    {
        ZeroMemory(&_this->wszTerm, sizeof(WCHAR) * MAX_PATH);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_SetLanguage(EPWeather* _this, DWORD cbLanguage, LPCWSTR wszLanguage)
{
    if (cbLanguage)
    {
        memcpy_s(_this->wszLanguage, sizeof(WCHAR) * MAX_PATH, wszLanguage, cbLanguage);
    }
    else
    {
        ZeroMemory(&_this->wszLanguage, sizeof(WCHAR) * MAX_PATH);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_SetIconSize(EPWeather* _this, LONG64 cbx, LONG64 cby)
{
    DWORD dwOldX = InterlockedAdd64(&_this->cbx, 0);
    DWORD dwOldY = InterlockedAdd64(&_this->cby, 0);
    if (dwOldX != cbx)
    {
        InterlockedExchange64(&_this->cbx, cbx);
        InterlockedExchange64(&_this->cby, cby);
        PostMessageW(_this->hWnd, EP_WEATHER_WM_FETCH_DATA, 0, 0);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_GetIconSize(EPWeather* _this, LONG64* cbx, LONG64* cby)
{
    if (cbx) *cbx = InterlockedAdd64(&_this->cbx, 0);
    if (cby) *cby = InterlockedAdd64(&_this->cby, 0);
    return S_OK;
}
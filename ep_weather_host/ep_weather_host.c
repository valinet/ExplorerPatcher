#include "ep_weather_host.h"
#include "ep_weather_provider_google_html.h"
#include "ep_weather_provider_google_script.h"
#include "ep_weather_error_html.h"

EPWeather* EPWeather_Instance = NULL;
SRWLOCK Lock_EPWeather_Instance = { .Ptr = SRWLOCK_INIT };
FARPROC SHRegGetValueFromHKCUHKLMFunc;

static DWORD epw_Weather_ReleaseBecauseClientDiedThread(EPWeather* _this)
{
    Sleep(5000);
    while (_this->lpVtbl->Release(_this));
    return 0;
}

static void epw_Weather_SetTextScaleFactorFromRegistry(EPWeather* _this, HKEY hKey, BOOL bRefresh)
{
    DWORD dwTextScaleFactor = 100, dwSize = sizeof(DWORD);
    if (SHRegGetValueFromHKCUHKLMFunc && SHRegGetValueFromHKCUHKLMFunc(L"SOFTWARE\\Microsoft\\Accessibility", L"TextScaleFactor", SRRF_RT_REG_DWORD, NULL, &dwTextScaleFactor, (LPDWORD)(&dwSize)) != ERROR_SUCCESS)
    {
        dwTextScaleFactor = 100;
    }
    if (InterlockedExchange64(&_this->dwTextScaleFactor, dwTextScaleFactor) == dwTextScaleFactor)
    {
        bRefresh = FALSE;
    }
    if (hKey == HKEY_CURRENT_USER)
    {
        if (RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Accessibility", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WOW64_64KEY | KEY_WRITE, NULL, &_this->hKCUAccessibility, NULL) == ERROR_SUCCESS)
        {
            RegNotifyChangeKeyValue(_this->hKCUAccessibility, FALSE, REG_NOTIFY_CHANGE_LAST_SET, _this->hSignalOnAccessibilitySettingsChangedFromHKCU, TRUE);
        }
    }
    else if (hKey == HKEY_LOCAL_MACHINE)
    {
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Accessibility", 0, KEY_READ | KEY_WOW64_64KEY | KEY_WRITE, NULL, &_this->hKLMAccessibility))
        {
            RegNotifyChangeKeyValue(_this->hKLMAccessibility, FALSE, REG_NOTIFY_CHANGE_LAST_SET, _this->hSignalOnAccessibilitySettingsChangedFromHKLM, TRUE);
        }
    }
    if (bRefresh)
    {
        _this->cntResizeWindow = 0;
        SetTimer(_this->hWnd, EP_WEATHER_TIMER_RESIZE_WINDOW, EP_WEATHER_TIMER_RESIZE_WINDOW_DELAY, NULL);
    }
}

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
    AcquireSRWLockShared(&Lock_EPWeather_Instance);
    EPWeather* _this = EPWeather_Instance; // GetWindowLongPtrW(FindWindowW(_T(EPW_WEATHER_CLASSNAME), NULL), GWLP_USERDATA);
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
    ReleaseSRWLockShared(&Lock_EPWeather_Instance);
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
    AcquireSRWLockShared(&Lock_EPWeather_Instance);
    pCoreWebView2Environemnt->lpVtbl->CreateCoreWebView2Controller(pCoreWebView2Environemnt, EPWeather_Instance->hWnd /* FindWindowW(_T(EPW_WEATHER_CLASSNAME), NULL) */, &EPWeather_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler);
    ReleaseSRWLockShared(&Lock_EPWeather_Instance);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE _epw_Weather_NavigateToError(EPWeather* _this)
{
    _ep_weather_ReboundBrowser(_this, TRUE);
    InterlockedExchange64(&_this->bIsNavigatingToError, TRUE);
    UINT dpi = GetDpiForWindow(_this->hWnd);
    int ch = MulDiv(MulDiv(305, dpi, 96), epw_Weather_GetTextScaleFactor(_this), 100);
    RECT rc;
    GetWindowRect(_this->hWnd, &rc);
    int w = MulDiv(MulDiv(EP_WEATHER_WIDTH, GetDpiForWindow(_this->hWnd), 96), epw_Weather_GetTextScaleFactor(_this), 100);
    if ((rc.bottom - rc.top != ch) || (rc.right - rc.left != w))
    {
        SetWindowPos(_this->hWnd, NULL, 0, 0, w, ch, SWP_NOMOVE | SWP_NOSENDCHANGING);
        HWND hNotifyWnd = InterlockedAdd64(&_this->hNotifyWnd, 0);
        if (hNotifyWnd)
        {
            InvalidateRect(hNotifyWnd, NULL, TRUE);
        }
    }
    if (_this->pCoreWebView2)
    {
        return _this->pCoreWebView2->lpVtbl->NavigateToString(_this->pCoreWebView2, ep_weather_error_html);
    }
    else
    {
        return E_FAIL;
    }
}

HRESULT STDMETHODCALLTYPE _epw_Weather_NavigateToProvider(EPWeather* _this)
{
    _ep_weather_ReboundBrowser(_this, FALSE);
    HRESULT hr = S_OK;
    LONG64 dwProvider = InterlockedAdd64(&_this->dwProvider, 0);
    if (dwProvider == EP_WEATHER_PROVIDER_TEST)
    {
    }
    else if (dwProvider == EP_WEATHER_PROVIDER_GOOGLE)
    {
        //hr = _this->pCoreWebView2->lpVtbl->Navigate(_this->pCoreWebView2, L"https://google.com");
        LPWSTR wszScriptData = malloc(sizeof(WCHAR) * EP_WEATHER_PROVIDER_GOOGLE_HTML_LEN);
        if (wszScriptData)
        {
            swprintf_s(wszScriptData, EP_WEATHER_PROVIDER_GOOGLE_HTML_LEN, L"https://www.google.com/search?hl=%s&q=weather%s%s", _this->wszLanguage, _this->wszTerm[0] ? L" " : L"", _this->wszTerm);
            if (_this->pCoreWebView2)
            {
                hr = _this->pCoreWebView2->lpVtbl->Navigate(_this->pCoreWebView2, wszScriptData);
            }
            else
            {
                hr = E_FAIL;
            }
            if (FAILED(hr))
            {
                InterlockedExchange64(&_this->bBrowserBusy, FALSE);
            }
            free(wszScriptData);
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
        LPWSTR wszScriptData = malloc(sizeof(WCHAR) * EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_LEN);
        if (wszScriptData)
        {
            LONG64 dwTemperatureUnit = InterlockedAdd64(&_this->dwTemperatureUnit, 0);
            LONG64 cbx = InterlockedAdd64(&_this->cbx, 0);
            swprintf_s(wszScriptData, EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_LEN, ep_weather_provider_google_script, dwTemperatureUnit == EP_WEATHER_TUNIT_FAHRENHEIT ? L'F' : L'C', cbx, cbx);
            //wprintf(L"%s\n", _this->wszScriptData);
            if (_this->pCoreWebView2)
            {
                hr = _this->pCoreWebView2->lpVtbl->ExecuteScript(_this->pCoreWebView2, wszScriptData, &EPWeather_ICoreWebView2ExecuteScriptCompletedHandler);
            }
            else
            {
                hr = E_FAIL;
            }
            if (FAILED(hr))
            {
                InterlockedExchange64(&_this->bBrowserBusy, FALSE);
            }
            free(wszScriptData);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE _ep_weather_ReboundBrowser(EPWeather* _this, LONG64 dwType)
{
    UINT dpi = GetDpiForWindow(_this->hWnd);
    RECT bounds;
    DWORD dwDevMode = InterlockedAdd64(&_this->dwDevMode, 0);
    if (dwType || dwDevMode)
    {
        GetClientRect(_this->hWnd, &bounds);
    }
    else
    {
        DWORD dwTextScaleFactor = epw_Weather_GetTextScaleFactor(_this);
        bounds.left = 0 - MulDiv(MulDiv(167, dpi, 96), dwTextScaleFactor, 100);
        bounds.top = 0 - MulDiv(MulDiv(178, dpi, 96), dwTextScaleFactor, 100);
        bounds.right = MulDiv(MulDiv(1333, dpi, 96), dwTextScaleFactor, 100);// 5560;
        bounds.bottom = MulDiv(MulDiv(600, dpi, 96), dwTextScaleFactor, 100);// 15600;
    }
    if (_this->pCoreWebView2Controller)
    {
        _this->pCoreWebView2Controller->lpVtbl->put_Bounds(_this->pCoreWebView2Controller, bounds);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_CreateCoreWebView2ControllerCompleted(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* _this2, HRESULT hr, ICoreWebView2Controller* pCoreWebView2Controller)
{
    AcquireSRWLockShared(&Lock_EPWeather_Instance);

    EPWeather* _this = EPWeather_Instance; // GetWindowLongPtrW(FindWindowW(_T(EPW_WEATHER_CLASSNAME), NULL), GWLP_USERDATA);
    if (!_this->pCoreWebView2Controller)
    {
        _this->pCoreWebView2Controller = pCoreWebView2Controller;
        _this->pCoreWebView2Controller->lpVtbl->get_CoreWebView2(_this->pCoreWebView2Controller, &_this->pCoreWebView2);
        _this->pCoreWebView2Controller->lpVtbl->AddRef(_this->pCoreWebView2Controller);
    }

    _ep_weather_ReboundBrowser(_this, FALSE);

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
            DWORD dwDevMode = InterlockedAdd64(&_this->dwDevMode, 0);
            pCoreWebView2Settings6->lpVtbl->put_AreDevToolsEnabled(pCoreWebView2Settings6, dwDevMode);
            pCoreWebView2Settings6->lpVtbl->put_AreDefaultContextMenusEnabled(pCoreWebView2Settings6, dwDevMode);
            pCoreWebView2Settings6->lpVtbl->put_IsStatusBarEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_IsZoomControlEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_IsGeneralAutofillEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_IsPasswordAutosaveEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_IsPinchZoomEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_IsSwipeNavigationEnabled(pCoreWebView2Settings6, FALSE);
            pCoreWebView2Settings6->lpVtbl->put_AreBrowserAcceleratorKeysEnabled(pCoreWebView2Settings6, dwDevMode);
            pCoreWebView2Settings6->lpVtbl->put_AreDefaultScriptDialogsEnabled(pCoreWebView2Settings6, dwDevMode);
            pCoreWebView2Settings6->lpVtbl->Release(pCoreWebView2Settings6);
        }
        pCoreWebView2Settings->lpVtbl->Release(pCoreWebView2Settings);
    }

    LONG64 dwDarkMode = InterlockedAdd64(&_this->g_darkModeEnabled, 0);
    epw_Weather_SetDarkMode(_this, dwDarkMode, FALSE);

    _this->pCoreWebView2->lpVtbl->add_PermissionRequested(_this->pCoreWebView2, &EPWeather_ICoreWebView2PermissionRequestedEventHandler, &_this->tkOnPermissionRequested);

    _this->pCoreWebView2->lpVtbl->add_NavigationCompleted(_this->pCoreWebView2, &EPWeather_ICoreWebView2NavigationCompletedEventHandler, &_this->tkOnNavigationCompleted);

    _epw_Weather_NavigateToProvider(_this);

    ReleaseSRWLockShared(&Lock_EPWeather_Instance);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_CallDevToolsProtocolMethodCompleted(ICoreWebView2CallDevToolsProtocolMethodCompletedHandler* _this, HRESULT errorCode, LPCWSTR returnObjectAsJson)
{
    AcquireSRWLockShared(&Lock_EPWeather_Instance);
    if (EPWeather_Instance)
    {
        wprintf(L"[CallDevToolsProtocolMethodCompleted] 0x%x [[ %s ]]\n", errorCode, returnObjectAsJson);
        PostMessageW(EPWeather_Instance->hWnd, EP_WEATHER_WM_FETCH_DATA, 0, 0);
        LPWSTR uri = NULL;
        if (EPWeather_Instance->pCoreWebView2)
        {
            EPWeather_Instance->pCoreWebView2->lpVtbl->get_Source(EPWeather_Instance->pCoreWebView2, &uri);
        }
        if (wcscmp(L"about:blank", uri ? uri : L""))
        {
            SetTimer(EPWeather_Instance->hWnd, EP_WEATHER_TIMER_REQUEST_REFRESH, EP_WEATHER_TIMER_REQUEST_REFRESH_DELAY, NULL);
        }
        CoTaskMemFree(uri);
    }
    ReleaseSRWLockShared(&Lock_EPWeather_Instance);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_NavigationCompleted(ICoreWebView2NavigationCompletedEventHandler* _this2, ICoreWebView2* pCoreWebView2, ICoreWebView2NavigationCompletedEventArgs* pCoreWebView2NavigationCompletedEventArgs)
{
    AcquireSRWLockShared(&Lock_EPWeather_Instance);
    EPWeather* _this = EPWeather_Instance; // GetWindowLongPtrW(FindWindowW(_T(EPW_WEATHER_CLASSNAME), NULL), GWLP_USERDATA);
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
    ReleaseSRWLockShared(&Lock_EPWeather_Instance);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_ExecuteScriptCompleted(ICoreWebView2ExecuteScriptCompletedHandler* _this2, HRESULT hr, LPCWSTR pResultObjectAsJson)
{
    AcquireSRWLockShared(&Lock_EPWeather_Instance);
    EPWeather* _this = EPWeather_Instance; // GetWindowLongPtrW(FindWindowW(_T(EPW_WEATHER_CLASSNAME), NULL), GWLP_USERDATA);
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
                //LONG64 bEnabled, dwDarkMode;
                //dwDarkMode = InterlockedAdd64(&_this->g_darkModeEnabled, 0);
                //epw_Weather_IsDarkMode(_this, dwDarkMode, &bEnabled);
                //swprintf_s(_this->wszScriptData, EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_LEN, ep_weather_provider_google_script2, bEnabled ? 1 : 0);
                _this->pCoreWebView2->lpVtbl->ExecuteScript(_this->pCoreWebView2, ep_weather_provider_google_script2, &EPWeather_ICoreWebView2ExecuteScriptCompletedHandler);
                bOk = TRUE;
            }
            else if (!_wcsicmp(pResultObjectAsJson, L"\"run_part_1\""))
            {
                printf("consent granted\n");
                PostMessageW(EPWeather_Instance->hWnd, EP_WEATHER_WM_FETCH_DATA, 0, 0);
                SetTimer(EPWeather_Instance->hWnd, EP_WEATHER_TIMER_REQUEST_REFRESH, EP_WEATHER_TIMER_REQUEST_REFRESH_DELAY * 5, NULL);
                ReleaseSRWLockShared(&Lock_EPWeather_Instance);
                return S_OK;
            }
            else
            {
                //wprintf(L"%s\n", pResultObjectAsJson);

                epw_Weather_LockData(_this);

                WCHAR* wszHeight = pResultObjectAsJson + 1;
                if (wszHeight)
                {
                    WCHAR* wszTemperature = wcschr(wszHeight, L'#');
                    if (wszTemperature)
                    {
                        wszTemperature[0] = 0;
                        wszTemperature++;
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
                                            int h = _wtoi(wszHeight);
                                            int ch = MulDiv(h, EP_WEATHER_HEIGHT, 367);
                                            UINT dpi = GetDpiForWindow(_this->hWnd);
                                            ch = MulDiv(MulDiv(ch, dpi, 96), epw_Weather_GetTextScaleFactor(_this), 100);
                                            RECT rc;
                                            GetWindowRect(_this->hWnd, &rc);
                                            int w = MulDiv(MulDiv(EP_WEATHER_WIDTH, GetDpiForWindow(_this->hWnd), 96), epw_Weather_GetTextScaleFactor(_this), 100);
                                            if ((rc.bottom - rc.top != ch) || (rc.right - rc.left != w))
                                            {
                                                SetWindowPos(_this->hWnd, NULL, 0, 0, w, ch, SWP_NOMOVE | SWP_NOSENDCHANGING);
                                                _ep_weather_ReboundBrowser(_this, FALSE);
                                                HWND hNotifyWnd = InterlockedAdd64(&_this->hNotifyWnd, 0);
                                                if (hNotifyWnd)
                                                {
                                                    InvalidateRect(hNotifyWnd, NULL, TRUE);
                                                }
                                            }
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
    ReleaseSRWLockShared(&Lock_EPWeather_Instance);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ICoreWebView2_PermissionRequested(ICoreWebView2PermissionRequestedEventHandler* _this2, ICoreWebView2* pCoreWebView2, ICoreWebView2PermissionRequestedEventArgs* pCoreWebView2PermissionRequestedEventArgs)
{
    AcquireSRWLockShared(&Lock_EPWeather_Instance);
    COREWEBVIEW2_PERMISSION_KIND kind;
    pCoreWebView2PermissionRequestedEventArgs->lpVtbl->get_PermissionKind(pCoreWebView2PermissionRequestedEventArgs, &kind);
    if (kind == COREWEBVIEW2_PERMISSION_KIND_GEOLOCATION)
    {
        DWORD r = InterlockedAdd64(&EPWeather_Instance->dwGeolocationMode, 0);
        printf("[Permissions] Geolocation permission request: %d\n", r);
        pCoreWebView2PermissionRequestedEventArgs->lpVtbl->put_State(pCoreWebView2PermissionRequestedEventArgs, r ? COREWEBVIEW2_PERMISSION_STATE_ALLOW : COREWEBVIEW2_PERMISSION_STATE_DENY);
    }
    ReleaseSRWLockShared(&Lock_EPWeather_Instance);
    return S_OK;
}

ULONG STDMETHODCALLTYPE epw_Weather_AddRef(EPWeather* _this)
{
    ULONG value = InterlockedIncrement64(&(_this->cbCount));
    printf("[General] AddRef: %d\n", value);
    return value;
}

ULONG STDMETHODCALLTYPE epw_Weather_Release(EPWeather* _this)
{
    ULONG value = InterlockedDecrement64(&(_this->cbCount));
    printf("[General] Release: %d\n", value);

    if (value == 0)
    {
        if (_this->hMainThread)
        {
            if (_this->hSignalExitMainThread)
            {
                SetEvent(_this->hSignalExitMainThread);
                WaitForSingleObject(_this->hMainThread, INFINITE);
            }
            CloseHandle(_this->hMainThread);
            if (_this->hSignalExitMainThread)
            {
                CloseHandle(_this->hSignalExitMainThread);
            }
        }
        if (_this->hInitializeEvent)
        {
            CloseHandle(_this->hInitializeEvent);
        }

        if (_this->hMutexData)
        {
            CloseHandle(_this->hMutexData);
        }

        if (_this->hUxtheme)
        {
            FreeLibrary(_this->hUxtheme);
        }
        if (_this->hShlwapi)
        {
            FreeLibrary(_this->hShlwapi);
        }
        if (_this->hKCUAccessibility)
        {
            RegCloseKey(_this->hKCUAccessibility);
        }
        if (_this->hKLMAccessibility)
        {
            RegCloseKey(_this->hKLMAccessibility);
        }
        if (_this->hSignalOnAccessibilitySettingsChangedFromHKCU)
        {
            CloseHandle(_this->hSignalOnAccessibilitySettingsChangedFromHKCU);
        }
        if (_this->hSignalOnAccessibilitySettingsChangedFromHKLM)
        {
            CloseHandle(_this->hSignalOnAccessibilitySettingsChangedFromHKLM);
        }
        if (_this->hSignalKillSwitch)
        {
            CloseHandle(_this->hSignalKillSwitch);
        }

        FREE(_this);
        LONG dwOutstandingObjects = InterlockedDecrement(&epw_OutstandingObjects);
        LONG dwOutstandingLocks = InterlockedAdd(&epw_LockCount, 0);
        if (!dwOutstandingObjects && !dwOutstandingLocks)
        {
        }
        printf("[General] Outstanding objects: %d, outstanding locks: %d\n", dwOutstandingObjects, dwOutstandingLocks);

#if defined(DEBUG) | defined(_DEBUG)
        printf("\nDumping memory leaks:\n");
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
        _CrtDumpMemoryLeaks();
        printf("Memory dump complete.\n\n");
#endif

        //TerminateProcess(GetCurrentProcess(), 0);

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
    else if (uMsg == WM_TIMER && wParam == EP_WEATHER_TIMER_REQUEST_REFRESH)
    {
        KillTimer(_this->hWnd, EP_WEATHER_TIMER_REQUEST_REFRESH);
        return SendMessageW(_this->hWnd, EP_WEATHER_WM_FETCH_DATA, 0, 0);
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
    else if (uMsg == WM_TIMER && wParam == EP_WEATHER_TIMER_RESIZE_WINDOW)
    {
        DWORD dwTextScaleFactor = epw_Weather_GetTextScaleFactor(_this);
        UINT dpi = GetDpiForWindow(_this->hWnd);
        SetWindowPos(_this->hWnd, NULL, 0, 0, MulDiv(MulDiv(EP_WEATHER_WIDTH, dpi, 96), dwTextScaleFactor, 100), MulDiv(MulDiv(EP_WEATHER_HEIGHT, dpi, 96), dwTextScaleFactor, 100), SWP_NOMOVE | SWP_NOSENDCHANGING);
        if (_this->cntResizeWindow == 7)
        {
            _this->cntResizeWindow = 0;
            KillTimer(_this->hWnd, EP_WEATHER_TIMER_RESIZE_WINDOW);
        }
        else
        {
            _this->cntResizeWindow++;
        }
        return 0;
    }
    else if (uMsg == EP_WEATHER_WM_REBOUND_BROWSER)
    {
        LPWSTR uri = NULL;
        if (_this->pCoreWebView2)
        {
            _this->pCoreWebView2->lpVtbl->get_Source(_this->pCoreWebView2, &uri);
        }
        _ep_weather_ReboundBrowser(_this, !wcscmp(L"about:blank", uri ? uri : L""));
        CoTaskMemFree(uri);
        return 0;
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
    else if (uMsg == EP_WEATHER_WM_SET_BROWSER_THEME)
    {
        if (_this->pCoreWebView2)
        {
            if (lParam)
            {
                EPWeather_ICoreWebView2CallDevToolsProtocolMethodCompletedHandler.lpVtbl->Invoke = ICoreWebView2_CallDevToolsProtocolMethodCompleted;
            }
            else
            {
                EPWeather_ICoreWebView2CallDevToolsProtocolMethodCompletedHandler.lpVtbl->Invoke = epw_Weather_static_Stub;
            }
            if (wParam)
            {
                printf("[SetDarkMode] 1\n");
                _this->pCoreWebView2->lpVtbl->CallDevToolsProtocolMethod(_this->pCoreWebView2, L"Emulation.setEmulatedMedia", L"{\"features\": [ { \"name\": \"prefers-color-scheme\", \"value\": \"dark\" }]}", &EPWeather_ICoreWebView2CallDevToolsProtocolMethodCompletedHandler);
                //_this->pCoreWebView2->lpVtbl->CallDevToolsProtocolMethod(_this->pCoreWebView2, L"Emulation.setAutoDarkModeOverride", L"{\"enabled\": true}", &EPWeather_ICoreWebView2CallDevToolsProtocolMethodCompletedHandler);
            }
            else
            {
                printf("[SetDarkMode] 0\n");
                _this->pCoreWebView2->lpVtbl->CallDevToolsProtocolMethod(_this->pCoreWebView2, L"Emulation.setEmulatedMedia", L"{\"features\": [ { \"name\": \"prefers-color-scheme\", \"value\": \"light\" }]}", &EPWeather_ICoreWebView2CallDevToolsProtocolMethodCompletedHandler);
                //_this->pCoreWebView2->lpVtbl->CallDevToolsProtocolMethod(_this->pCoreWebView2, L"Emulation.setAutoDarkModeOverride", L"{\"enabled\": false}", &EPWeather_ICoreWebView2CallDevToolsProtocolMethodCompletedHandler);
            }
            return S_OK;
        }
    }
    else if (uMsg == EP_WEATHER_WM_SETDEVMODE)
    {
        if (_this->pCoreWebView2)
        {
            ICoreWebView2Settings* pCoreWebView2Settings = NULL;
            _this->pCoreWebView2->lpVtbl->get_Settings(_this->pCoreWebView2, &pCoreWebView2Settings);
            if (pCoreWebView2Settings)
            {
                ICoreWebView2Settings6* pCoreWebView2Settings6 = NULL;
                pCoreWebView2Settings->lpVtbl->QueryInterface(pCoreWebView2Settings, &IID_ICoreWebView2Settings6, &pCoreWebView2Settings6);
                if (pCoreWebView2Settings6)
                {
                    pCoreWebView2Settings6->lpVtbl->put_AreDevToolsEnabled(pCoreWebView2Settings6, wParam);
                    pCoreWebView2Settings6->lpVtbl->put_AreDefaultContextMenusEnabled(pCoreWebView2Settings6, wParam);
                    pCoreWebView2Settings6->lpVtbl->put_AreBrowserAcceleratorKeysEnabled(pCoreWebView2Settings6, wParam);
                    pCoreWebView2Settings6->lpVtbl->put_AreDefaultScriptDialogsEnabled(pCoreWebView2Settings6, wParam);
                    pCoreWebView2Settings6->lpVtbl->Release(pCoreWebView2Settings6);
                    SetLastError(0);
                    LONG dwStyle = GetWindowLongW(_this->hWnd, GWL_STYLE);
                    if (!GetLastError())
                    {
                        if (wParam) dwStyle |= WS_SIZEBOX;
                        else dwStyle &= ~WS_SIZEBOX;
                        SetWindowLong(_this->hWnd, GWL_STYLE, dwStyle);
                    }
                    PostMessageW(_this->hWnd, EP_WEATHER_WM_FETCH_DATA, 0, 0);
                }
                pCoreWebView2Settings->lpVtbl->Release(pCoreWebView2Settings);
            }
        }
    }
    else if (uMsg == WM_CLOSE || (uMsg == WM_KEYUP && wParam == VK_ESCAPE) || (uMsg == WM_ACTIVATEAPP && wParam == FALSE && GetAncestor(GetForegroundWindow(), GA_ROOT) != _this->hWnd))
    {
        epw_Weather_Hide(_this);
        return 0;
    }
    else if (uMsg == WM_WINDOWPOSCHANGING)
    {
        if (IsWindowVisible(hWnd))
        {
            LONG64 dwDevMode = InterlockedAdd64(&_this->dwDevMode, 0);
            WINDOWPOS* pwp = (WINDOWPOS*)lParam;
            pwp->flags |= (!dwDevMode ? (SWP_NOMOVE | SWP_NOSIZE) : 0);
            if (dwDevMode)
            {
                _ep_weather_ReboundBrowser(_this, TRUE);
            }
        }
        return 0;
    }
    else if (uMsg == WM_SETTINGCHANGE)
    {
        if (IsColorSchemeChangeMessage(lParam))
        {
            MARGINS marGlassInset;
            if (!IsHighContrast())
            {
                marGlassInset.cxLeftWidth = -1; // -1 means the whole window
                marGlassInset.cxRightWidth = -1;
                marGlassInset.cyBottomHeight = -1;
                marGlassInset.cyTopHeight = -1;
            }
            else
            {
                marGlassInset.cxLeftWidth = 0;
                marGlassInset.cxRightWidth = 0;
                marGlassInset.cyBottomHeight = 0;
                marGlassInset.cyTopHeight = 0;
            }
            LONG64 dwDarkMode = InterlockedAdd64(&_this->g_darkModeEnabled, 0);
            if (IsWindows11())
            {
                DwmExtendFrameIntoClientArea(_this->hWnd, &marGlassInset);
                BOOL value = (IsThemeActive() && !IsHighContrast()) ? 1 : 0;
                DwmSetWindowAttribute(_this->hWnd, 1029, &value, sizeof(BOOL));
            }
            else
            {
                RTL_OSVERSIONINFOW rovi;
                DWORD32 ubr = GetOSVersionAndUBR(&rovi);
                int s = 0;
                if (rovi.dwBuildNumber < 18985)
                {
                    s = -1;
                }
                DwmSetWindowAttribute(_this->hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE + s, &dwDarkMode, sizeof(LONG64));
            }
            if (!dwDarkMode)
            {
                epw_Weather_SetDarkMode(_this, dwDarkMode, TRUE);
            }
            return 0;
        }
    }
    else if (uMsg == WM_DPICHANGED)
    {
        UINT dpiX = LOWORD(wParam);
        int w = MulDiv(MulDiv(EP_WEATHER_WIDTH, dpiX, 96), epw_Weather_GetTextScaleFactor(_this), 100);
        RECT* rc = lParam;
        SetWindowPos(_this->hWnd, NULL, rc->left, rc->top, w, rc->bottom - rc->top, 0);
        return 0;
    }
    else if (uMsg == WM_PAINT && !IsWindows11())
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (ps.fErase)
        {
            LONG64 bEnabled, dwDarkMode;
            dwDarkMode = InterlockedAdd64(&_this->g_darkModeEnabled, 0);
            epw_Weather_IsDarkMode(_this, dwDarkMode, &bEnabled);
            COLORREF oldcr = SetBkColor(hdc, bEnabled ? RGB(0, 0, 0) : RGB(255, 255, 255));
            ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, L"", 0, 0);
            SetBkColor(hdc, oldcr);
        }
        EndPaint(hWnd, &ps);
        return 0;
    }
    /*BOOL bIsRunningWithoutVisualStyle = !IsThemeActive() || IsHighContrast();
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
    }*/
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

HRESULT STDMETHODCALLTYPE epw_Weather_IsDarkMode(EPWeather* _this, LONG64 dwDarkMode, LONG64* bEnabled)
{
    BOOL bIsCompositionEnabled = TRUE;
    DwmIsCompositionEnabled(&bIsCompositionEnabled);
    if (!dwDarkMode)
    {
        RTL_OSVERSIONINFOW rovi;
        *bEnabled = bIsCompositionEnabled && ((GetOSVersion(&rovi) && rovi.dwBuildNumber < 18985) ? TRUE : (ShouldSystemUseDarkMode ? ShouldSystemUseDarkMode() : FALSE)) && !IsHighContrast();
    }
    else
    {
        *bEnabled = dwDarkMode - 1;
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_SetDarkMode(EPWeather* _this, LONG64 dwDarkMode, LONG64 bRefresh)
{
    LONG64 bEnabled;
    epw_Weather_IsDarkMode(_this, dwDarkMode, &bEnabled);
    InterlockedExchange64(&_this->g_darkModeEnabled, dwDarkMode);
    if ((dwDarkMode == 2 && bEnabled) || (dwDarkMode == 1 && !bEnabled) || !dwDarkMode)
    {
        RefreshImmersiveColorPolicyState();
        if (_this->hWnd)
        {
            AllowDarkModeForWindow(_this->hWnd, bEnabled);
            RTL_OSVERSIONINFOW rovi;
            DWORD32 ubr = GetOSVersionAndUBR(&rovi);
            int s = 0;
            if (rovi.dwBuildNumber < 18985)
            {
                s = -1;
            }
            DwmSetWindowAttribute(_this->hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE + s, &bEnabled, sizeof(BOOL));
            //InvalidateRect(_this->hWnd, NULL, FALSE);
            PostMessageW(_this->hWnd, EP_WEATHER_WM_SET_BROWSER_THEME, bEnabled, bRefresh);
        }
        return S_OK;
    }
    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE epw_Weather_SetGeolocationMode(EPWeather* _this, LONG64 dwGeolocationMode)
{
    InterlockedExchange64(&_this->dwGeolocationMode, dwGeolocationMode);
    PostMessageW(_this->hWnd, EP_WEATHER_WM_FETCH_DATA, 0, 0);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_SetWindowCornerPreference(EPWeather* _this, LONG64 dwWindowCornerPreference)
{
    InterlockedExchange64(&_this->dwWindowCornerPreference, dwWindowCornerPreference);
    INT preference = dwWindowCornerPreference;
    if (_this->hWnd)
    {
        DwmSetWindowAttribute(_this->hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_SetDevMode(EPWeather* _this, LONG64 dwDevMode, LONG64 bRefresh)
{
    InterlockedExchange64(&_this->dwDevMode, dwDevMode);
    if (bRefresh)
    {
        PostMessageW(_this->hWnd, EP_WEATHER_WM_SETDEVMODE, dwDevMode, 0);
    }
    return S_OK;
}

DWORD WINAPI epw_Weather_MainThread(EPWeather* _this)
{
    HRESULT hr = S_OK;
    BOOL bShouldReleaseBecauseClientDied = FALSE;

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
    wc.hbrBackground = IsWindows11() ? (HBRUSH)GetStockObject(BLACK_BRUSH) : NULL;
    wc.lpszClassName = _T(EPW_WEATHER_CLASSNAME);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    if (!RegisterClassW(&wc))
    {
        //_this->hrLastError = HRESULT_FROM_WIN32(GetLastError());
        //goto cleanup;
    }

    DWORD dwDevMode = InterlockedAdd64(&_this->dwDevMode, 0);
    _this->hWnd = CreateWindowExW(0, _T(EPW_WEATHER_CLASSNAME), L"", WS_OVERLAPPED | WS_CAPTION | (dwDevMode ? WS_SIZEBOX : 0), _this->rc.left, _this->rc.top, _this->rc.right - _this->rc.left, _this->rc.bottom - _this->rc.top, NULL, NULL, epw_hModule, _this); // 1030, 630
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

    LONG64 dwDarkMode = InterlockedAdd64(&_this->g_darkModeEnabled, 0);
    if (IsWindows11())
    {
        if (!IsHighContrast())
        {
            MARGINS marGlassInset = { -1, -1, -1, -1 }; // -1 means the whole window
            DwmExtendFrameIntoClientArea(_this->hWnd, &marGlassInset);
            BOOL value = 1;
            DwmSetWindowAttribute(_this->hWnd, 1029, &value, sizeof(BOOL));
        }
    }
    else
    {
        RTL_OSVERSIONINFOW rovi;
        DWORD32 ubr = GetOSVersionAndUBR(&rovi);
        int s = 0;
        if (rovi.dwBuildNumber < 18985)
        {
            s = -1;
        }
        DwmSetWindowAttribute(_this->hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE + s, &dwDarkMode, sizeof(LONG64));
    }
    epw_Weather_SetDarkMode(_this, dwDarkMode, FALSE);

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
        else if (dwRes == WAIT_ABANDONED_0 + 1)// || dwRes == WAIT_OBJECT_0 + 1)
        {
            printf("[General] Client has died.\n");

            if (OpenEventW(READ_CONTROL, FALSE, _T(EP_SETUP_EVENTNAME)))
            {
                printf("[General] Servicing is in progress, terminating...\n");
                TerminateProcess(GetCurrentProcess(), 0);
            }

            //if (dwRes == WAIT_OBJECT_0 + 1) ReleaseMutex(_this->hSignalKillSwitch);
            CloseHandle(_this->hSignalKillSwitch);
            //TerminateProcess(GetCurrentProcess(), 0);
            _this->hSignalKillSwitch = NULL;
            bShouldReleaseBecauseClientDied = TRUE;
            break;
        }
        else if (dwRes == WAIT_OBJECT_0 + 2)
        {
            epw_Weather_SetTextScaleFactorFromRegistry(_this, HKEY_CURRENT_USER, TRUE);
        }
        else if (dwRes == WAIT_OBJECT_0 + 3)
        {
            epw_Weather_SetTextScaleFactorFromRegistry(_this, HKEY_LOCAL_MACHINE, TRUE);
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
    if (_this->hWnd)
    {
        DestroyWindow(_this->hWnd);
    }
    //UnregisterClassW(_T(EPW_WEATHER_CLASSNAME), epw_hModule);
    if (_this->pTaskList)
    {
        _this->pTaskList->lpVtbl->Release(_this->pTaskList);
    }
    CoUninitialize();
    SetEvent(_this->hInitializeEvent);
    if (bShouldReleaseBecauseClientDied)
    {
        //SHCreateThread(epw_Weather_ReleaseBecauseClientDiedThread, _this, CTF_NOADDREFLIB, NULL);
    }

    return 0;
}

HRESULT STDMETHODCALLTYPE epw_Weather_Initialize(EPWeather* _this, WCHAR wszName[MAX_PATH], BOOL bAllocConsole, LONG64 dwProvider, LONG64 cbx, LONG64 cby, LONG64 dwTemperatureUnit, LONG64 dwUpdateSchedule, RECT rc, LONG64 dwDarkMode, LONG64 dwGeolocationMode, HWND* hWnd)
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

    /*if (EPWeather_Instance)
    {
        return E_FAIL;
    }*/
    AcquireSRWLockExclusive(&Lock_EPWeather_Instance);
    EPWeather_Instance = _this;
    ReleaseSRWLockExclusive(&Lock_EPWeather_Instance);

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

    InterlockedExchange64(&_this->dwGeolocationMode, dwGeolocationMode);

    _this->hUxtheme = LoadLibraryW(L"uxtheme.dll");
    if (_this->hUxtheme)
    {
        RefreshImmersiveColorPolicyState = GetProcAddress(_this->hUxtheme, (LPCSTR)104);
        SetPreferredAppMode = GetProcAddress(_this->hUxtheme, (LPCSTR)135);
        AllowDarkModeForWindow = GetProcAddress(_this->hUxtheme, (LPCSTR)133);
        ShouldAppsUseDarkMode = GetProcAddress(_this->hUxtheme, (LPCSTR)132);
        ShouldSystemUseDarkMode = GetProcAddress(_this->hUxtheme, (LPCSTR)138);
        if (ShouldAppsUseDarkMode &&
            ShouldSystemUseDarkMode &&
            SetPreferredAppMode &&
            AllowDarkModeForWindow &&
            RefreshImmersiveColorPolicyState
            )
        {
            SetPreferredAppMode(TRUE);
            epw_Weather_SetDarkMode(_this, dwDarkMode, FALSE);
        }
    }

    _this->hShlwapi = LoadLibraryW(L"Shlwapi.dll");
    if (_this->hShlwapi)
    {
        SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(_this->hShlwapi, "SHRegGetValueFromHKCUHKLM");
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

    _this->hSignalOnAccessibilitySettingsChangedFromHKCU = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (!_this->hSignalOnAccessibilitySettingsChangedFromHKCU)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    _this->hSignalOnAccessibilitySettingsChangedFromHKLM = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (!_this->hSignalOnAccessibilitySettingsChangedFromHKLM)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    epw_Weather_SetTextScaleFactorFromRegistry(_this, HKEY_CURRENT_USER, FALSE);
    epw_Weather_SetTextScaleFactorFromRegistry(_this, HKEY_LOCAL_MACHINE, FALSE);

    _this->rc = rc;

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

    *hWnd = _this->hWnd;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE epw_Weather_Show(EPWeather* _this)
{
    INT preference = InterlockedAdd64(&_this->dwWindowCornerPreference, 0);
    DwmSetWindowAttribute(_this->hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
    PostMessageW(_this->hWnd, EP_WEATHER_WM_REBOUND_BROWSER, 0, 0);
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
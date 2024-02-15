#include "TaskbarCenter.h"

#include "../ep_weather_host/ep_weather_host_h.h"

DEFINE_GUID(POLID_TurnOffSPIAnimations, 0xD7AF00A, 0xB468, 0x4A39, 0xB0, 0x16, 0x33, 0x3E, 0x22, 0x77, 0xAB, 0xED);
extern int(*SHWindowsPolicy)(REFIID);
extern HWND PeopleButton_LastHWND;
extern DWORD dwWeatherToLeft;
extern DWORD dwOldTaskbarAl;
extern DWORD dwMMOldTaskbarAl;
extern DWORD dwSearchboxTaskbarMode;
extern wchar_t* EP_TASKBAR_LENGTH_PROP_NAME;
extern IEPWeather* epw;
#define EP_TASKBAR_LENGTH_TOO_SMALL 20
BOOL bTaskbarCenterHasPatchedSHWindowsPolicy = FALSE;
UINT atomPeopleBand = 0;
UINT atomMSTaskListWClass = 0;
UINT atomMSTaskSwWClass = 0;

HRESULT TaskbarCenter_Center(HWND hWnd, HWND hWndTaskbar, RECT rc, BOOL bIsTaskbarHorizontal)
{
	HRESULT hr = S_OK;
	VARIANT vtChild[10];
	VARIANT vt;
	long k = 0, kk = 0;

	IAccessible* pAccessible = NULL;
	AccessibleObjectFromWindow(hWnd, 0, &IID_IAccessible, &pAccessible);
	if (pAccessible)
	{
		pAccessible->lpVtbl->get_accChildCount(pAccessible, &kk);
		if (kk <= 10)
		{
			AccessibleChildren(pAccessible, 0, kk, vtChild, &k);
			for (int i = 0; i < k; ++i)
			{
				if (vtChild[i].vt == VT_DISPATCH)
				{
					IDispatch* pDisp = vtChild[i].ppdispVal;
					IAccessible* pChild = NULL;
					pDisp->lpVtbl->QueryInterface(pDisp, &IID_IAccessible, &pChild);
					if (pChild)
					{
						vt.vt = VT_I4;
						vt.lVal = CHILDID_SELF;
						pChild->lpVtbl->get_accRole(pChild, vt, &vt);
						if (vt.lVal == ROLE_SYSTEM_TOOLBAR)
						{
							IAccessible* pLast = NULL;
							kk = 0;
							pChild->lpVtbl->get_accChildCount(pChild, &kk);
							if (kk <= 1)
							{
								SetPropW(hWnd, EP_TASKBAR_LENGTH_PROP_NAME, -1);
							}
							else if (kk >= 2)
							{
								vt.vt = VT_I4;
								vt.lVal = kk - 1;
								long x = 0, y = 0, w = 0, h = 0, d = 0;
								pChild->lpVtbl->accLocation(pChild, &x, &y, &w, &h, vt);
								if (bIsTaskbarHorizontal ? (x == -1 || w < EP_TASKBAR_LENGTH_TOO_SMALL) : (y == -1 || h < EP_TASKBAR_LENGTH_TOO_SMALL))
								{
									hr = E_FAIL;
								}
								else
								{
									if (kk >= 3)
									{
										d = (bIsTaskbarHorizontal ? ((x - rc.left) + w) : ((y - rc.top) + h));
										vt.vt = VT_I4;
										vt.lVal = 1;
										x = 0, y = 0, w = 0, h = 0;
										pChild->lpVtbl->accLocation(pChild, &x, &y, &w, &h, vt);
										if (bIsTaskbarHorizontal ? w == 0 : h == 0)
										{
											vt.vt = VT_I4;
											vt.lVal = 2;
											x = 0, y = 0, w = 0, h = 0;
											pChild->lpVtbl->accLocation(pChild, &x, &y, &w, &h, vt);
										}
										if (bIsTaskbarHorizontal ? (x == -1 || w < EP_TASKBAR_LENGTH_TOO_SMALL) : (y == -1 || h < EP_TASKBAR_LENGTH_TOO_SMALL))
										{
											hr == E_FAIL;
										}
										else
										{
											if (!((GetKeyState(VK_LBUTTON) < 0) && (GetForegroundWindow() == hWndTaskbar)))
											{
												SetPropW(hWnd, EP_TASKBAR_LENGTH_PROP_NAME, (bIsTaskbarHorizontal ? (d - (x - rc.left)) : (d - (y - rc.top))));
											}
										}
									}
									else
									{
										if (!((GetKeyState(VK_LBUTTON) < 0) && (GetForegroundWindow() == hWndTaskbar)))
										{
											SetPropW(hWnd, EP_TASKBAR_LENGTH_PROP_NAME, bIsTaskbarHorizontal ? w : h);
										}
									}
								}
							}
						}
						pChild->lpVtbl->Release(pChild);
					}
					pDisp->lpVtbl->Release(pDisp);
				}
			}
		}
		pAccessible->lpVtbl->Release(pAccessible);
	}
	return hr;
}

BOOL TaskbarCenter_GetClientRectHook(HWND hWnd, LPRECT lpRect)
{
	BOOL bWasCalled = FALSE;
	if (!atomMSTaskListWClass) atomMSTaskListWClass = RegisterWindowMessageW(L"MSTaskListWClass");
	if (GetClassWord(hWnd, GCW_ATOM) == atomMSTaskListWClass)
	{
		if (!atomMSTaskSwWClass) atomMSTaskSwWClass = RegisterWindowMessageW(L"MSTaskSwWClass");
		HWND hwndParent = GetParent(hWnd);
		BOOL bIsPrimaryTaskbar = (GetClassWord(hwndParent, GCW_ATOM) == atomMSTaskSwWClass);
		DWORD dwSetting = (bIsPrimaryTaskbar ? dwOldTaskbarAl : dwMMOldTaskbarAl);
		BOOL bCenteringEnabled = TaskbarCenter_ShouldCenter(dwSetting);
		if (!bCenteringEnabled && GetPropW(hWnd, EP_TASKBAR_LENGTH_PROP_NAME))
		{
			RemovePropW(hWnd, EP_TASKBAR_LENGTH_PROP_NAME);
		}
		if (!bCenteringEnabled && !epw)
		{
			return GetClientRect(hWnd, lpRect); // Early out
		}
		HWND hWndStart = NULL;
		RECT rcStart = { 0, 0, 0, 0 };
		HWND hWndTaskbar = NULL;
		if (bIsPrimaryTaskbar)
		{
			hWndTaskbar = GetParent(GetParent(hwndParent));
		}
		else
		{
			hWndTaskbar = GetParent(hwndParent);
		}
		hWndStart = FindWindowExW(hWndTaskbar, NULL, L"Start", NULL);
		BOOL bIsTaskbarHorizontal = TaskbarCenter_IsTaskbarHorizontal(hWnd);
		HWND hReBarWindow32 = NULL;
		if (bIsPrimaryTaskbar) hReBarWindow32 = FindWindowExW(hWndTaskbar, NULL, L"ReBarWindow32", NULL);
		HWND hPeopleBand = NULL;
		if (bIsPrimaryTaskbar) hPeopleBand = FindWindowExW(hReBarWindow32, NULL, L"PeopleBand", NULL);
		BOOL bIsWeatherAvailable = hPeopleBand && dwWeatherToLeft;
		BOOL bWasLeftAlignedDueToSpaceConstraints = FALSE;
		if (bCenteringEnabled)
		{
			if (hWndStart)
			{
				GetClientRect(hWndStart, &rcStart);
				HWND hTrayButton = NULL;
				wchar_t* pCn = L"TrayButton";
				if (/*!IsWindows11() &&*/ dwSearchboxTaskbarMode == 2) pCn = L"TrayDummySearchControl";
				while (hTrayButton = FindWindowExW(hWndTaskbar, hTrayButton, pCn, NULL))
				{
					if (pCn == L"TrayButton" && !IsWindowVisible(hTrayButton)) continue;
					RECT rcTrayButton;
					GetClientRect(hTrayButton, &rcTrayButton);
					if (bIsTaskbarHorizontal)
					{
						rcStart.right += (rcTrayButton.right - rcTrayButton.left);
					}
					else
					{
						rcStart.bottom += (rcTrayButton.bottom - rcTrayButton.top);
					}
					if (pCn == L"TrayDummySearchControl") {
						pCn = L"TrayButton";
						hTrayButton = NULL;
					}
				}
			}
			RECT rc;
			GetWindowRect(hWnd, &rc);
			MONITORINFO mi;
			ZeroMemory(&mi, sizeof(MONITORINFO));
			mi.cbSize = sizeof(MONITORINFO);
			GetMonitorInfoW(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);
			DWORD dwLength = 0;
			TaskbarCenter_Center(hWnd, hWndTaskbar, mi.rcMonitor, bIsTaskbarHorizontal);
			if (dwLength = GetPropW(hWnd, EP_TASKBAR_LENGTH_PROP_NAME))
			{
				if (dwLength == -1)
				{
					if (TaskbarCenter_ShouldStartBeCentered(dwSetting) && hWndStart)
					{
						if (bIsTaskbarHorizontal)
						{
							SetWindowPos(hWndStart, NULL, ((mi.rcMonitor.right - mi.rcMonitor.left) - (rcStart.right - rcStart.left)) / 2, rcStart.top, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS);
							RECT rcTrayButton;
							GetClientRect(hWndStart, &rcTrayButton);
							DWORD dwDim = (rcTrayButton.right - rcTrayButton.left);
							HWND hTrayButton = NULL;
							wchar_t* pCn = L"TrayButton";
							if (/*!IsWindows11() &&*/ dwSearchboxTaskbarMode == 2) pCn = L"TrayDummySearchControl";
							while (hTrayButton = FindWindowExW(hWndTaskbar, hTrayButton, pCn, NULL))
							{
								if (pCn == L"TrayButton" && !IsWindowVisible(hTrayButton)) continue;
								GetClientRect(hTrayButton, &rcTrayButton);
								MoveWindow(hTrayButton, ((mi.rcMonitor.right - mi.rcMonitor.left) - (rcStart.right - rcStart.left)) / 2 + dwDim, rcStart.top, rcTrayButton.right, rcTrayButton.bottom, TRUE);
								if (!bIsPrimaryTaskbar) InvalidateRect(hTrayButton, NULL, TRUE);
								dwDim += (rcTrayButton.right - rcTrayButton.left);
								if (pCn == L"TrayDummySearchControl") {
									pCn = L"TrayButton";
									hTrayButton = NULL;
								}
							}
						}
						else
						{
							SetWindowPos(hWndStart, NULL, rcStart.left, ((mi.rcMonitor.bottom - mi.rcMonitor.top) - (rcStart.bottom - rcStart.top)) / 2, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS);
							RECT rcTrayButton;
							GetClientRect(hWndStart, &rcTrayButton);
							DWORD dwDim = (rcTrayButton.bottom - rcTrayButton.top);
							HWND hTrayButton = NULL;
							wchar_t* pCn = L"TrayButton";
							if (/*!IsWindows11() &&*/ dwSearchboxTaskbarMode == 2) pCn = L"TrayDummySearchControl";
							while (hTrayButton = FindWindowExW(hWndTaskbar, hTrayButton, pCn, NULL))
							{
								if (pCn == L"TrayButton" && !IsWindowVisible(hTrayButton)) continue;
								GetClientRect(hTrayButton, &rcTrayButton);
								MoveWindow(hTrayButton, rcStart.left, ((mi.rcMonitor.bottom - mi.rcMonitor.top) - (rcStart.bottom - rcStart.top)) / 2 + dwDim, rcTrayButton.right, rcTrayButton.bottom, TRUE);
								InvalidateRect(hTrayButton, NULL, TRUE);
								dwDim += (rcTrayButton.bottom - rcTrayButton.top);
								if (pCn == L"TrayDummySearchControl") {
									pCn = L"TrayButton";
									hTrayButton = NULL;
								}
							}
						}
						if (!bIsPrimaryTaskbar) InvalidateRect(hWndStart, NULL, TRUE);
					}
				}
				else
				{
					RECT rcPeopleBand = { 0, 0, 0, 0 };
					RECT rcReBarWindow32 = { 0, 0, 0, 0 };
					if (hPeopleBand)
					{
						GetClientRect(hPeopleBand, &rcPeopleBand);
					}
					if (hReBarWindow32)
					{
						GetClientRect(hReBarWindow32, &rcReBarWindow32);
					}
					RECT rc;
					GetWindowRect(hWnd, &rc);
					//MARGINS mBand;
					//mBand.cxLeftWidth = 0; mBand.cxRightWidth = 0; mBand.cyBottomHeight = 0; mBand.cyTopHeight = 0;
					//if (bIsPrimaryTaskbar) SendMessageW(hReBarWindow32, RB_GETBANDMARGINS, 0, &mBand);
					//if (TaskbarCenter_ShouldStartBeCentered(dwSetting))
					//{
					//	rc.left -= mBand.cxLeftWidth;
					//}
					//else
					//{
					//	rc.left += mBand.cxLeftWidth;
					//}

					DWORD dwAdd = 0;
					if (TaskbarCenter_ShouldStartBeCentered(dwSetting) && hWndStart)
					{
						dwAdd += (bIsTaskbarHorizontal ? (rcStart.right - rcStart.left) : (rcStart.bottom - rcStart.top));
					}
					bWasCalled = GetClientRect(hWnd, lpRect);
					long res = 0;
					if (bIsTaskbarHorizontal)
					{
						res = ((mi.rcMonitor.right - mi.rcMonitor.left) - dwLength - dwAdd) / 2 - (rc.left - mi.rcMonitor.left);
						if (res < 0)
						{
							dwLength -= abs(res) * 2;
							res = ((mi.rcMonitor.right - mi.rcMonitor.left) - dwLength - dwAdd) / 2 - (rc.left - mi.rcMonitor.left);
						}
						if (TaskbarCenter_ShouldStartBeCentered(dwSetting))
						{
							res += (rcStart.right - rcStart.left);
						}
					}
					else
					{
						res = ((mi.rcMonitor.bottom - mi.rcMonitor.top) - dwLength - dwAdd) / 2 - (rc.top - mi.rcMonitor.top);
						if (res < 0)
						{
							dwLength -= abs(res) * 2;
							res = ((mi.rcMonitor.bottom - mi.rcMonitor.top) - dwLength - dwAdd) / 2 - (rc.top - mi.rcMonitor.top);
						}
						if (TaskbarCenter_ShouldStartBeCentered(dwSetting))
						{
							res += (rcStart.bottom - rcStart.top);
						}
					}
					if ((res + dwLength + 50 >= (bIsTaskbarHorizontal ? lpRect->right : lpRect->bottom)))
					{
						if (TaskbarCenter_ShouldLeftAlignWhenSpaceConstrained(dwSetting) || !bIsTaskbarHorizontal)
						{
							bWasLeftAlignedDueToSpaceConstraints = TRUE;
							res = 0;
							if (bIsTaskbarHorizontal)
							{
								if (TaskbarCenter_ShouldStartBeCentered(dwSetting))
								{
									res += (rcStart.right - rcStart.left);
								}
							}
							else
							{
								if (TaskbarCenter_ShouldStartBeCentered(dwSetting))
								{
									res += (rcStart.bottom - rcStart.top);
								}
							}
						}
					}
					if (bIsTaskbarHorizontal)
					{
						lpRect->left = res;
						//lpRect->right = MIN(MAX(lpRect->right, MIN_DIM), MAX(res + dwLength + 100, MIN_DIM));
					}
					else
					{
						lpRect->top = res;
						//lpRect->bottom = MIN(MAX(lpRect->bottom, MIN_DIM), MAX(res + dwLength + 100, MIN_DIM));
					}
					if (TaskbarCenter_ShouldStartBeCentered(dwSetting) && hWndStart)
					{
						if (bIsTaskbarHorizontal)
						{
							SetWindowPos(hWndStart, NULL, (rc.left - mi.rcMonitor.left) + lpRect->left - (rcStart.right - rcStart.left), rcStart.top, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS);
							RECT rcTrayButton;
							GetClientRect(hWndStart, &rcTrayButton);
							DWORD dwDim = (rcTrayButton.right - rcTrayButton.left);
							HWND hTrayButton = NULL;
							wchar_t* pCn = L"TrayButton";
							if (/*!IsWindows11() &&*/ dwSearchboxTaskbarMode == 2) pCn = L"TrayDummySearchControl";
							while (hTrayButton = FindWindowExW(hWndTaskbar, hTrayButton, pCn, NULL))
							{
								if (pCn == L"TrayButton" && !IsWindowVisible(hTrayButton)) continue;
								GetClientRect(hTrayButton, &rcTrayButton);
								MoveWindow(hTrayButton, (rc.left - mi.rcMonitor.left) + lpRect->left - (rcStart.right - rcStart.left) + dwDim, rcStart.top, rcTrayButton.right, rcTrayButton.bottom, TRUE);
								if (!bIsPrimaryTaskbar) InvalidateRect(hTrayButton, NULL, TRUE);
								dwDim += (rcTrayButton.right - rcTrayButton.left);
								if (pCn == L"TrayDummySearchControl") {
									pCn = L"TrayButton";
									hTrayButton = NULL;
								}
							}
						}
						else
						{
							SetWindowPos(hWndStart, NULL, rcStart.left, (rc.top - mi.rcMonitor.top) + lpRect->top - (rcStart.bottom - rcStart.top), 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS);
							RECT rcTrayButton;
							GetClientRect(hWndStart, &rcTrayButton);
							DWORD dwDim = (rcTrayButton.bottom - rcTrayButton.top);
							HWND hTrayButton = NULL;
							wchar_t* pCn = L"TrayButton";
							if (/*!IsWindows11() &&*/ dwSearchboxTaskbarMode == 2) pCn = L"TrayDummySearchControl";
							while (hTrayButton = FindWindowExW(hWndTaskbar, hTrayButton, pCn, NULL))
							{
								if (pCn == L"TrayButton" && !IsWindowVisible(hTrayButton)) continue;
								GetClientRect(hTrayButton, &rcTrayButton);
								MoveWindow(hTrayButton, rcStart.left, (rc.top - mi.rcMonitor.top) + lpRect->top - (rcStart.bottom - rcStart.top) + dwDim, rcTrayButton.right, rcTrayButton.bottom, TRUE);
								InvalidateRect(hTrayButton, NULL, TRUE);
								dwDim += (rcTrayButton.bottom - rcTrayButton.top);
								if (pCn == L"TrayDummySearchControl") {
									pCn = L"TrayButton";
									hTrayButton = NULL;
								}
							}
						}
						if (!bIsPrimaryTaskbar) InvalidateRect(hWndStart, NULL, TRUE);
					}
				}
			}
		}
		if (bIsPrimaryTaskbar && epw)
		{
			BOOL bWeatherAlignment = FALSE;
			if (bIsWeatherAvailable)
			{
				bWeatherAlignment = TRUE;
			}
			else
			{
				bWeatherAlignment = FALSE;
			}
			/*if (bIsWeatherAvailable && bWasLeftAlignedDueToSpaceConstraints && dwWeatherToLeft == 2)
			{
				bWeatherAlignment = FALSE;
			}*/
			if (!atomPeopleBand) atomPeopleBand = RegisterWindowMessageW(L"PeopleBand");
			REBARBANDINFOW rbi;
			ZeroMemory(&rbi, sizeof(REBARBANDINFOW));
			rbi.cbSize = sizeof(REBARBANDINFOW);
			rbi.fMask = RBBIM_CHILD;
			SendMessageW(hReBarWindow32, RB_GETBANDINFOW, 0, &rbi);
			BOOL bIsFirstBandPeopleBand = (GetClassWord(rbi.hwndChild, GCW_ATOM) == atomPeopleBand);
			if (bWeatherAlignment ? !bIsFirstBandPeopleBand : bIsFirstBandPeopleBand)
			{
				int s = 0;
				int k = SendMessageW(hReBarWindow32, RB_GETBANDCOUNT, 0, 0);
				if (bWeatherAlignment)
				{
					for (int i = k - 1; i >= 0; i--)
					{
						if (s == 0)
						{
							ZeroMemory(&rbi, sizeof(REBARBANDINFOW));
							rbi.cbSize = sizeof(REBARBANDINFOW);
							rbi.fMask = RBBIM_CHILD;
							SendMessageW(hReBarWindow32, RB_GETBANDINFOW, i, &rbi);
							if (rbi.hwndChild && (GetClassWord(rbi.hwndChild, GCW_ATOM) == atomPeopleBand))
							{
								s = 1;
							}
						}
						if (s == 1 && i >= 1)
						{
							SendMessageW(hReBarWindow32, RB_MOVEBAND, i, i - 1);
						}
					}
					SendNotifyMessageW(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)L"TraySettings");
				}
				else
				{
					for (int i = 0; i < k - 1; ++i)
					{
						SendMessageW(hReBarWindow32, RB_MOVEBAND, i, i + 1);
					}
					SendNotifyMessageW(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)L"TraySettings");
				}
			}
			int k = SendMessageW(hReBarWindow32, RB_GETBANDCOUNT, 0, 0);
			for (int i = 0; i < k - 1; ++i)
			{
				ZeroMemory(&rbi, sizeof(REBARBANDINFOW));
				rbi.cbSize = sizeof(REBARBANDINFOW);
				rbi.fMask = RBBIM_CHILD | RBBIM_CHILDSIZE;
				SendMessageW(hReBarWindow32, RB_GETBANDINFOW, i, &rbi);
				if (rbi.hwndChild && (GetClassWord(rbi.hwndChild, GCW_ATOM) == atomPeopleBand))
				{
					RECT rcpp = { 0, 0, 0, 0 };
					GetClientRect(rbi.hwndChild, &rcpp);
					if (rcpp.right && rcpp.bottom)
					{
						if (bIsTaskbarHorizontal)
						{
							if (rcpp.right - rcpp.left != rbi.cxMinChild) SendMessageW(hReBarWindow32, RB_MINIMIZEBAND, i, 0);
						}
						else
						{
							if (rcpp.bottom - rcpp.top != rbi.cxMinChild) SendMessageW(hReBarWindow32, RB_MINIMIZEBAND, i, 0);
						}
					}
					break;
				}
			}
		}
	}
	if (bWasCalled) return bWasCalled;
	return GetClientRect(hWnd, lpRect);
}

BOOL TaskbarCenter_SHWindowsPolicy(REFIID riid)
{
	if (IsEqualIID(riid, &POLID_TurnOffSPIAnimations) && (TaskbarCenter_ShouldCenter(dwOldTaskbarAl) || TaskbarCenter_ShouldCenter(dwMMOldTaskbarAl)))
	{
		DWORD flOldProtect = 0;
		if (!bTaskbarCenterHasPatchedSHWindowsPolicy && *((unsigned char*)_ReturnAddress() + 7) == 0x0F)
		{
			if (*((unsigned char*)_ReturnAddress() + 8) == 0x85 && VirtualProtect((unsigned char*)_ReturnAddress() + 9, 1, PAGE_EXECUTE_READWRITE, &flOldProtect))
			{
				*((unsigned char*)_ReturnAddress() + 9) += 2;
				VirtualProtect((unsigned char*)_ReturnAddress() + 9, 1, flOldProtect, &flOldProtect);
			}
			else if (*((unsigned char*)_ReturnAddress() + 8) == 0x84 && VirtualProtect((unsigned char*)_ReturnAddress() + 13, 2, PAGE_EXECUTE_READWRITE, &flOldProtect))
			{
				*((unsigned char*)_ReturnAddress() + 13) = 0x90;
				*((unsigned char*)_ReturnAddress() + 14) = 0x90;
				VirtualProtect((unsigned char*)_ReturnAddress() + 13, 2, flOldProtect, &flOldProtect);
			}
			bTaskbarCenterHasPatchedSHWindowsPolicy = TRUE;
		}
		return 1;
	}
	return SHWindowsPolicy(riid);
}

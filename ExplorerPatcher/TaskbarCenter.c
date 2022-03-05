#include "TaskbarCenter.h"

extern DWORD dwOldTaskbarAl;
extern DWORD dwMMOldTaskbarAl;
extern wchar_t* EP_TASKBAR_LENGTH_PROP_NAME;
#define EP_TASKBAR_LENGTH_TOO_SMALL 20

inline BOOL TaskbarCenter_IsTaskbarHorizontal(HWND hWnd)
{
	__int64 v1;
	__int64 result;
	v1 = *((__int64*)GetWindowLongPtrW(hWnd, 0) + 13);
	result = 1i64;
	if (v1)
		return (*(__int64(__fastcall**)(__int64))(*(__int64*)v1 + 96))(v1);
	return result;
}

inline BOOL TaskbarCenter_ShouldCenter(DWORD dwSetting)
{
	return (dwSetting & 0b001);
}

inline BOOL TaskbarCenter_ShouldStartBeCentered(DWORD dwSetting)
{
	return (dwSetting & 0b010);
}

inline BOOL TaskbarCenter_ShouldLeftAlignWhenSpaceConstrained(DWORD dwSetting)
{
	return (dwSetting & 0b100);
}

HRESULT TaskbarCenter_Center(HWND hWnd, RECT rc, BOOL bIsTaskbarHorizontal)
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
											SetPropW(hWnd, EP_TASKBAR_LENGTH_PROP_NAME, (bIsTaskbarHorizontal ? (d - (x - rc.left)) : (d - (y - rc.top))));
										}
									}
									else
									{
										SetPropW(hWnd, EP_TASKBAR_LENGTH_PROP_NAME, bIsTaskbarHorizontal ? w : h);
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
	HWND hWndStart = NULL;
	RECT rcStart;
	SetRect(&rcStart, 0, 0, 0, 0);
	if (GetClassWord(hWnd, GCW_ATOM) == RegisterWindowMessageW(L"MSTaskListWClass"))
	{
		BOOL bIsPrimaryTaskbar = GetClassWord(GetParent(hWnd), GCW_ATOM) == RegisterWindowMessageW(L"MSTaskSwWClass");
		DWORD dwSetting = (bIsPrimaryTaskbar ? dwOldTaskbarAl : dwMMOldTaskbarAl);
		HWND hWndTaskbar = NULL;
		if (bIsPrimaryTaskbar)
		{
			hWndTaskbar = GetParent(GetParent(GetParent(hWnd)));
		}
		else
		{
			hWndTaskbar = GetParent(GetParent(hWnd));
		}
		hWndStart = FindWindowExW(hWndTaskbar, NULL, L"Start", NULL);
		BOOL bIsTaskbarHorizontal = TaskbarCenter_IsTaskbarHorizontal(hWnd);
		if (TaskbarCenter_ShouldCenter(dwSetting))
		{
			if (TaskbarCenter_ShouldStartBeCentered(dwSetting) && hWndStart)
			{
				GetClientRect(hWndStart, &rcStart);
				HWND hTrayButton = NULL;
				while (hTrayButton = FindWindowExW(hWndTaskbar, hTrayButton, L"TrayButton", NULL))
				{
					if (!IsWindowVisible(hTrayButton)) continue;
					RECT rcTrayButton;
					GetClientRect(hTrayButton, &rcTrayButton);
					rcStart.right += (rcTrayButton.right - rcTrayButton.left);
				}
			}
			RECT rc;
			GetWindowRect(hWnd, &rc);
			MONITORINFO mi;
			ZeroMemory(&mi, sizeof(MONITORINFO));
			mi.cbSize = sizeof(MONITORINFO);
			GetMonitorInfoW(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);
			DWORD dwLength = 0;
			TaskbarCenter_Center(hWnd, mi.rcMonitor, bIsTaskbarHorizontal);
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
							while (hTrayButton = FindWindowExW(hWndTaskbar, hTrayButton, L"TrayButton", NULL))
							{
								if (!IsWindowVisible(hTrayButton)) continue;
								GetClientRect(hTrayButton, &rcTrayButton);
								MoveWindow(hTrayButton, ((mi.rcMonitor.right - mi.rcMonitor.left) - (rcStart.right - rcStart.left)) / 2 + dwDim, rcStart.top, rcTrayButton.right, rcTrayButton.bottom, TRUE);
								if (!bIsPrimaryTaskbar) InvalidateRect(hTrayButton, NULL, TRUE);
								dwDim += (rcTrayButton.right - rcTrayButton.left);
							}
						}
						else
						{
							SetWindowPos(hWndStart, NULL, rcStart.left, ((mi.rcMonitor.bottom - mi.rcMonitor.top) - (rcStart.bottom - rcStart.top)) / 2, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS);
							RECT rcTrayButton;
							GetClientRect(hWndStart, &rcTrayButton);
							DWORD dwDim = (rcTrayButton.right - rcTrayButton.left);
							HWND hTrayButton = NULL;
							while (hTrayButton = FindWindowExW(hWndTaskbar, hTrayButton, L"TrayButton", NULL))
							{
								if (!IsWindowVisible(hTrayButton)) continue;
								GetClientRect(hTrayButton, &rcTrayButton);
								MoveWindow(hTrayButton, rcStart.left, ((mi.rcMonitor.bottom - mi.rcMonitor.top) - (rcStart.bottom - rcStart.top)) / 2 + dwDim, rcTrayButton.right, rcTrayButton.bottom, TRUE);
								if (!bIsPrimaryTaskbar) InvalidateRect(hTrayButton, NULL, TRUE);
								dwDim += (rcTrayButton.right - rcTrayButton.left);
							}
						}
						if (!bIsPrimaryTaskbar) InvalidateRect(hWndStart, NULL, TRUE);
					}
				}
				else
				{
					if (TaskbarCenter_ShouldStartBeCentered(dwSetting) && hWndStart)
					{
						dwLength += (bIsTaskbarHorizontal ? (rcStart.right - rcStart.left) : (rcStart.bottom - rcStart.top));
					}
					bWasCalled = GetClientRect(hWnd, lpRect);
					long res = 0;
					if (bIsTaskbarHorizontal)
					{
						res = ((mi.rcMonitor.right - mi.rcMonitor.left) - dwLength) / 2 - (!TaskbarCenter_ShouldStartBeCentered(dwSetting) ? (rc.left - mi.rcMonitor.left) : 0);
					}
					else
					{
						res = ((mi.rcMonitor.bottom - mi.rcMonitor.top) - dwLength) / 2 - (!TaskbarCenter_ShouldStartBeCentered(dwSetting) ? (rc.top - mi.rcMonitor.top) : 0);
					}
					if (res + dwLength + 5 - (TaskbarCenter_ShouldStartBeCentered(dwSetting) ? (bIsTaskbarHorizontal ? (rc.left - mi.rcMonitor.left) : (rc.top - mi.rcMonitor.top)) : 0) < (bIsTaskbarHorizontal ? lpRect->right : lpRect->bottom))
					{
						if (bIsTaskbarHorizontal)
						{
							lpRect->left = res;
						}
						else
						{
							lpRect->top = res;
						}
						if (TaskbarCenter_ShouldLeftAlignWhenSpaceConstrained(dwSetting) || !bIsTaskbarHorizontal)
						{
							if (bIsTaskbarHorizontal)
							{
								lpRect->right = (TaskbarCenter_ShouldStartBeCentered(dwSetting) ? (rc.left - mi.rcMonitor.left) : 0) + 10 + lpRect->right;
							}
							else
							{
								lpRect->bottom = (TaskbarCenter_ShouldStartBeCentered(dwSetting) ? (rc.top - mi.rcMonitor.top) : 0) + 10 + lpRect->bottom;
							}
						}
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
							while (hTrayButton = FindWindowExW(hWndTaskbar, hTrayButton, L"TrayButton", NULL))
							{
								if (!IsWindowVisible(hTrayButton)) continue;
								GetClientRect(hTrayButton, &rcTrayButton);
								MoveWindow(hTrayButton, (rc.left - mi.rcMonitor.left) + lpRect->left - (rcStart.right - rcStart.left) + dwDim, rcStart.top, rcTrayButton.right, rcTrayButton.bottom, TRUE);
								if (!bIsPrimaryTaskbar) InvalidateRect(hTrayButton, NULL, TRUE);
								dwDim += (rcTrayButton.right - rcTrayButton.left);
							}
						}
						else
						{
							SetWindowPos(hWndStart, NULL, rcStart.left, (rc.top - mi.rcMonitor.top) + lpRect->top - (rcStart.bottom - rcStart.top), 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS);
							RECT rcTrayButton;
							GetClientRect(hWndStart, &rcTrayButton);
							DWORD dwDim = (rcTrayButton.right - rcTrayButton.left);
							HWND hTrayButton = NULL;
							while (hTrayButton = FindWindowExW(hWndTaskbar, hTrayButton, L"TrayButton", NULL))
							{
								if (!IsWindowVisible(hTrayButton)) continue;
								GetClientRect(hTrayButton, &rcTrayButton);
								MoveWindow(hTrayButton, rcStart.left, (rc.top - mi.rcMonitor.top) + lpRect->top - (rcStart.bottom - rcStart.top) + dwDim, rcTrayButton.right, rcTrayButton.bottom, TRUE);
								if (!bIsPrimaryTaskbar) InvalidateRect(hTrayButton, NULL, TRUE);
								dwDim += (rcTrayButton.right - rcTrayButton.left);
							}
						}
						if (!bIsPrimaryTaskbar) InvalidateRect(hWndStart, NULL, TRUE);
					}
				}
			}
		}
		else
		{
			if (GetPropW(hWnd, EP_TASKBAR_LENGTH_PROP_NAME))
			{
				RemovePropW(hWnd, EP_TASKBAR_LENGTH_PROP_NAME);
			}
		}
		if ((!TaskbarCenter_ShouldCenter(dwSetting) || !TaskbarCenter_ShouldStartBeCentered(dwSetting)) && hWndStart)
		{
			GetWindowRect(hWndStart, &rcStart);
			if (rcStart.left != 0 || rcStart.top != 0)
			{
				SetWindowPos(hWndStart, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS);
				if (!bIsPrimaryTaskbar) InvalidateRect(hWndStart, NULL, TRUE);
				RECT rcTrayButton;
				GetClientRect(hWndStart, &rcTrayButton);
				DWORD dwDim = (rcTrayButton.right - rcTrayButton.left);
				HWND hTrayButton = NULL;
				while (hTrayButton = FindWindowExW(hWndTaskbar, hTrayButton, L"TrayButton", NULL))
				{
					if (!IsWindowVisible(hTrayButton)) continue;
					GetClientRect(hTrayButton, &rcTrayButton);
					if (bIsTaskbarHorizontal)
					{
						MoveWindow(hTrayButton, dwDim, 0, rcTrayButton.right, rcTrayButton.bottom, TRUE);
					}
					else
					{
						MoveWindow(hTrayButton, 0, dwDim, rcTrayButton.right, rcTrayButton.bottom, TRUE);
					}
					if (!bIsPrimaryTaskbar) InvalidateRect(hTrayButton, NULL, TRUE);
					dwDim += (rcTrayButton.right - rcTrayButton.left);
				}
			}
		}
	}
	if (bWasCalled) return bWasCalled;
	return GetClientRect(hWnd, lpRect);
}
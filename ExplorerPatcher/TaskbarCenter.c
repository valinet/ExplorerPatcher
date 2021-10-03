#include "TaskbarCenter.h"

HANDLE hEvent;

BOOL TaskbarCenter_Notify()
{
	if (hEvent)
	{
		SetEvent(hEvent);
		return TRUE;
	}
	return FALSE;
}

BOOL GetClientRectHook(HWND hWnd, LPRECT lpRect)
{
	wchar_t wszClassName[100];
	ZeroMemory(wszClassName, 100);
	GetClassNameW(hWnd, wszClassName, 100);
	if (!wcscmp(wszClassName, L"MSTaskListWClass"))
	{
		TaskbarCenter_Notify();
	}
	return GetClientRect(hWnd, lpRect);
}

HRESULT TaskbarCenter_Initialize(HMODULE hExplorer)
{
	if (!(hEvent = CreateEventW(NULL, TRUE, FALSE, TASKBAR_CHANGED_NOTIFICATION)))
	{
		return E_NOTIMPL;
	}
	if (FindWindowExW(NULL, NULL, L"Shell_TrayWnd", NULL))
	{
		return E_NOTIMPL;
	}
	// This is one of the methods called by explorer!CTaskListWnd::_RecomputeLayout
	if (!VnPatchDelayIAT(hExplorer, "ext-ms-win-rtcore-ntuser-window-ext-l1-1-0.dll", "GetClientRect", GetClientRectHook))
	{
		return E_NOTIMPL;
	}
	return S_OK;
}
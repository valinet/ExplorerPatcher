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
	if (!wcscmp(wszClassName, L"Shell_TrayWnd") || !wcscmp(wszClassName, L"Shell_SecondaryTrayWnd"))
	{
		TaskbarCenter_Notify();
	}
	return GetClientRect(hWnd, lpRect);
}

HRESULT TaskbarCenter_Initialize(HMODULE hExplorer)
{
	// This is one of the methods called by explorer!CTaskListWnd::_RecomputeLayout
	if (!VnPatchDelayIAT(hExplorer, "ext-ms-win-rtcore-ntuser-window-ext-l1-1-0.dll", "GetClientRect", GetClientRectHook))
	{
		return E_NOTIMPL;
	}
	if (!(hEvent = CreateEventW(NULL, TRUE, FALSE, TASKBAR_CHANGED_NOTIFICATION)))
	{
		return E_NOTIMPL;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return E_NOTIMPL;
	}
	return S_OK;
}
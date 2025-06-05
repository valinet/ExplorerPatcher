#pragma once

#include <initguid.h>
#include <Windows.h>
#include <oleacc.h>
#include <tchar.h>
#pragma comment(lib, "Oleacc.lib")
#include <CommCtrl.h>
#include <uxtheme.h>
#include <valinet/hooking/iatpatch.h>
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MIN_DIM 600

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
interface ITaskGroup;
interface ITaskItem;
interface ITaskBtnGroup;

MIDL_INTERFACE("e587c396-8ac9-49b7-a16c-e2acfd140399")
ITaskListSite : IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE GetGroupLocation(ITaskGroup*, ITaskItem*, int, RECT*) = 0;
	virtual DWORD STDMETHODCALLTYPE GetStuckPlace() const = 0;
	virtual void STDMETHODCALLTYPE SwitchToItem(ITaskItem*) = 0;
	virtual void STDMETHODCALLTYPE CloseItem(ITaskItem*) = 0;
	virtual void STDMETHODCALLTYPE OnContextMenu(POINT, HWND, bool, ITaskGroup*, ITaskItem*) = 0;
	virtual void STDMETHODCALLTYPE SetHotItem(ITaskItem*) = 0;
	virtual void STDMETHODCALLTYPE HandleMouseEnter(int) = 0;
	virtual void STDMETHODCALLTYPE HandleMouseLeave(int) = 0;
	virtual void STDMETHODCALLTYPE NotifyExtendedUIDismissed(int, ITaskItem*) = 0;
	virtual void STDMETHODCALLTYPE DisableToolTip(int) = 0;
	virtual int STDMETHODCALLTYPE GetIconId(ITaskGroup*, ITaskItem*) = 0;
	virtual int STDMETHODCALLTYPE IsContextMenuActive() = 0;
	virtual HWND STDMETHODCALLTYPE GetWindow() = 0;
	virtual HRESULT STDMETHODCALLTYPE ShowLivePreview(ITaskItem*, DWORD) = 0;
	virtual int STDMETHODCALLTYPE IsLivePreviewActive() = 0;
	virtual int STDMETHODCALLTYPE IsTaskTopLevelUI(ITaskItem*) = 0;
	virtual int STDMETHODCALLTYPE IsTaskExtendedUI(ITaskBtnGroup*, ITaskItem*) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetHost(const GUID&, void**) = 0;
};

MIDL_INTERFACE("2be43f49-c23d-40d8-8092-2fb6577ee134")
ITaskListWndSite : IUnknown
{
	virtual void STDMETHODCALLTYPE CheckSize(int) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetStuckPlace(DWORD*) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetTaskListUITheme(const WCHAR**) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetUserPreferences(DWORD*) = 0;
	virtual int STDMETHODCALLTYPE HitTestForSizeableBorder(int, int) = 0;
	virtual HRESULT STDMETHODCALLTYPE UnhideTray() = 0;
	virtual HRESULT STDMETHODCALLTYPE SetScrollInfo(int, const SCROLLINFO&) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetScrollInfo(int, SCROLLINFO*) = 0;
	virtual HRESULT STDMETHODCALLTYPE HandleScroll(int, UINT, int) = 0;
	virtual int STDMETHODCALLTYPE IsHorizontal() = 0;
	/*virtual int STDMETHODCALLTYPE IsFullHeightOfTray() = 0;
	virtual void STDMETHODCALLTYPE UpdateTheme() = 0;
	virtual SyncDisplayChangeFlags STDMETHODCALLTYPE SyncDisplayChange(SyncDisplayChangeFlags, CCoSimpleArray<UINT>&) = 0;
	virtual void STDMETHODCALLTYPE ImmersiveShow() = 0;
	virtual void STDMETHODCALLTYPE HandleImmersiveLauncherVisibilityChange(HMONITOR, bool) = 0;
	virtual void STDMETHODCALLTYPE HandleSearchAppVisibilityChange(HMONITOR, bool) = 0;
	virtual void STDMETHODCALLTYPE HandleTaskViewVisibilityChange(bool) = 0;
	virtual bool STDMETHODCALLTYPE IsDesktopVisibleOnTrayMonitor() = 0;
	virtual void STDMETHODCALLTYPE HandleJumpViewVisibilityChange(bool) = 0;
	virtual void STDMETHODCALLTYPE HandleHoverUIVisibilityChange(bool) = 0;
	virtual void STDMETHODCALLTYPE NotifyFeedsAboutTaskListUpdated() = 0;*/
};

inline BOOL TaskbarCenter_IsTaskbarHorizontal(HWND hWnd)
{
	BOOL bRet = FALSE;

	void* pTaskListWnd = (void*)GetWindowLongPtrW(hWnd, 0);
	if (pTaskListWnd)
	{
		// Shift by sizeof(CImpWndProc)
		IUnknown* punkTaskListWnd = (IUnknown*)((PBYTE)pTaskListWnd + sizeof(void*) /*vtable*/ + sizeof(HWND) /*_hwnd*/);

		ITaskListSite* pTaskListSite = nullptr;
		if (SUCCEEDED(punkTaskListWnd->QueryInterface(IID_PPV_ARGS(&pTaskListSite))))
		{
			ITaskListWndSite* pHost = nullptr;
			if (SUCCEEDED(pTaskListSite->GetHost(IID_PPV_ARGS(&pHost))))
			{
				bRet = pHost->IsHorizontal() != 0;
				pHost->Release();
			}
			pTaskListSite->Release();
		}
	}

	return bRet;
}
#endif

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

BOOL TaskbarCenter_GetClientRectHook(HWND hWnd, LPRECT lpRect);

BOOL TaskbarCenter_SHWindowsPolicy(REFIID riid);

#ifdef __cplusplus
}
#endif

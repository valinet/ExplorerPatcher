#ifndef _H_TASKBARCENTER_H_
#define _H_TASKBARCENTER_H_
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

BOOL TaskbarCenter_GetClientRectHook(HWND hWnd, LPRECT lpRect);
#endif
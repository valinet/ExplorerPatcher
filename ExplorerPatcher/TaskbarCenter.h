#ifndef _H_TASKBARCENTER_H_
#define _H_TASKBARCENTER_H_
#include <initguid.h>
#include <Windows.h>
#include <oleacc.h>
#include <tchar.h>
#pragma comment(lib, "Oleacc.lib")
#include <valinet/hooking/iatpatch.h>

BOOL TaskbarCenter_GetClientRectHook(HWND hWnd, LPRECT lpRect);
#endif
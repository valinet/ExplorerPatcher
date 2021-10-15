#ifndef _H_TASKBARCENTER_H_
#define _H_TASKBARCENTER_H_
#include <initguid.h>
#include <Windows.h>
#include <oleacc.h>
#pragma comment(lib, "Oleacc.lib")
#include <valinet/hooking/iatpatch.h>

#define TASKBAR_CHANGED_NOTIFICATION L"Global\\ExplorerPatcher_TaskbarChangedNotification_{B37553B7-425C-44F6-A04A-126849EE59CB}"

HRESULT TaskbarCenter_Initialize(HMODULE);
#endif
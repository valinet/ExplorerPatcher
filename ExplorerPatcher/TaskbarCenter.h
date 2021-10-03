#ifndef _H_TASKBARCENTER_H_
#define _H_TASKBARCENTER_H_
#include <initguid.h>
#include <Windows.h>
#include <valinet/hooking/iatpatch.h>

#define TASKBAR_CHANGED_NOTIFICATION L"Global\\ExplorerPatcher_TaskbarChangedNotification"

HRESULT TaskbarCenter_Initialize(HMODULE);
#endif
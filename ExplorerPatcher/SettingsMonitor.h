#ifndef _H_SETTINGSMONITOR_H_
#define _H_SETTINGSMONITOR_H_
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <stdio.h>

typedef struct _Setting
{
    HKEY origin;
    wchar_t name[MAX_PATH];
    HKEY hKey;
    HANDLE hEvent;
    void(__stdcall *callback)(void*);
    void* data;
} Setting;
typedef struct _SettingsChangeParameters
{
    Setting* settings;
    DWORD size;
    HANDLE hThread;
} SettingsChangeParameters;
DWORD WINAPI MonitorSettings(SettingsChangeParameters*);
#endif

#ifndef _H_SETTINGSMONITOR_H_
#define _H_SETTINGSMONITOR_H_
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

typedef struct _Setting
{
    HKEY origin;
    wchar_t name[MAX_PATH];
    HKEY hKey;
    HANDLE hEvent;
    void(*callback)(void*);
    void* data;
} Setting;
typedef struct _SettingsChangeParameters
{
    Setting* settings;
    DWORD size;
} SettingsChangeParameters;
DWORD MonitorSettings(SettingsChangeParameters*);
#endif

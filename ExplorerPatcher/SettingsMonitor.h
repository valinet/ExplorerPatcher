#ifndef _H_SETTINGSMONITOR_H_
#define _H_SETTINGSMONITOR_H_
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

typedef struct _SettingsChangeParameters
{
    BOOL isStartMenuExperienceHost;
    void (*TaskbarAlChangedCallback)(INT64, DWORD);
    void* TaskbarAlChangedCallbackData;
    void (*Start_MaximumRecentAppsChangedCallback)(INT64, DWORD);
    void* Start_MaximumRecentAppsChangedCallbackData;
} SettingsChangeParameters;
DWORD MonitorSettingsChanges(SettingsChangeParameters* params);
#endif

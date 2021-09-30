#include "SettingsMonitor.h"

DWORD MonitorSettingsChanges(SettingsChangeParameters* params)
{
    HMODULE hModule = LoadLibraryW(L"Shlwapi.dll");
    if (hModule)
    {
        FARPROC SHRegGetValueFromHKCUHKLMFunc = GetProcAddress(hModule, "SHRegGetValueFromHKCUHKLM");
        DWORD dwSize = sizeof(DWORD);
        LONG lRes = ERROR_SUCCESS;
        HKEY hKeyCU, hKeyLM;

        dwSize = sizeof(DWORD);
        DWORD dwInitialTaskbarAl = 0, dwTaskbarAl = 0, dwInitialTaskbarAlWas = 0;
        if (!SHRegGetValueFromHKCUHKLMFunc || SHRegGetValueFromHKCUHKLMFunc(
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
            TEXT("TaskbarAl"),
            SRRF_RT_REG_DWORD,
            NULL,
            &dwInitialTaskbarAl,
            (LPDWORD)(&dwSize)
        ) != ERROR_SUCCESS)
        {
            dwInitialTaskbarAl = 0;
        }
        else
        {
            dwInitialTaskbarAlWas = 1;
        }

        dwSize = sizeof(DWORD);
        DWORD dwInitialMaximumFrequentApps = 6, dwMaximumFrequentApps = 6, dwInitialMaximumFrequentAppsWas = 0;
        if (!SHRegGetValueFromHKCUHKLMFunc || SHRegGetValueFromHKCUHKLMFunc(
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
            TEXT("Start_MaximumFrequentApps"),
            SRRF_RT_REG_DWORD,
            NULL,
            &dwInitialMaximumFrequentApps,
            (LPDWORD)(&dwSize)
        ) != ERROR_SUCCESS)
        {
            dwInitialMaximumFrequentApps = 6;
        }
        else
        {
            dwInitialMaximumFrequentAppsWas = 1;
        }

        while (TRUE)
        {
            lRes = RegOpenKeyExW(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
                0,
                KEY_READ,
                &hKeyCU
            );
            if (lRes != ERROR_SUCCESS)
            {
                return 0;
            }
            HANDLE hEvHKCU = CreateEvent(NULL, FALSE, FALSE, NULL);
            RegNotifyChangeKeyValue(
                hKeyCU,
                FALSE,
                REG_NOTIFY_CHANGE_LAST_SET,
                hEvHKCU,
                TRUE
            );

            lRes = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
                0,
                KEY_READ,
                &hKeyLM
            );
            if (lRes != ERROR_SUCCESS)
            {
                return 0;
            }
            HANDLE hEvHKLM = CreateEvent(NULL, FALSE, FALSE, NULL);
            RegNotifyChangeKeyValue(
                hKeyLM,
                FALSE,
                REG_NOTIFY_CHANGE_LAST_SET,
                hEvHKLM,
                TRUE
            );
            printf("!!! Setup monitor %d %d\n", hEvHKCU, hEvHKLM);

            HANDLE hEvents[2];
            hEvents[0] = hEvHKCU;
            hEvents[1] = hEvHKLM;
            DWORD rv = WaitForMultipleObjects(
                2,
                hEvents,
                FALSE,
                INFINITE
            );
            if (rv == WAIT_OBJECT_0)
            {
                dwSize = sizeof(DWORD);
                lRes = RegQueryValueExW(
                    hKeyCU,
                    TEXT("TaskbarAl"),
                    0,
                    NULL,
                    &dwTaskbarAl,
                    &dwSize
                );
                if (lRes != ERROR_SUCCESS && dwInitialTaskbarAlWas)
                {
                    if (params->TaskbarAlChangedCallback)
                    {
                        void* p = 0;
                        if (params->TaskbarAlChangedCallbackData)
                        {
                            p = params->TaskbarAlChangedCallbackData;
                        }
                        params->TaskbarAlChangedCallback(p, dwTaskbarAl);
                    }
                    else
                    {
                        exit(0);
                    }
                    dwInitialTaskbarAl = dwTaskbarAl;
                }
                if (lRes == ERROR_SUCCESS && dwTaskbarAl != dwInitialTaskbarAl)
                {
                    if (params->TaskbarAlChangedCallback)
                    {
                        void* p = 0;
                        if (params->TaskbarAlChangedCallbackData)
                        {
                            p = params->TaskbarAlChangedCallbackData;
                        }
                        params->TaskbarAlChangedCallback(p, dwTaskbarAl);
                    }
                    else
                    {
                        exit(0);
                    }
                    dwInitialTaskbarAl = dwTaskbarAl;
                }

                if (params->isStartMenuExperienceHost)
                {
                    dwSize = sizeof(DWORD);
                    lRes = RegQueryValueExW(
                        hKeyCU,
                        TEXT("Start_MaximumFrequentApps"),
                        0,
                        NULL,
                        &dwMaximumFrequentApps,
                        &dwSize
                    );
                    if (lRes != ERROR_SUCCESS && dwInitialMaximumFrequentAppsWas)
                    {
                        if (params->Start_MaximumRecentAppsChangedCallback)
                        {
                            void* p = 0;
                            if (params->Start_MaximumRecentAppsChangedCallbackData)
                            {
                                p = params->Start_MaximumRecentAppsChangedCallbackData;
                            }
                            params->Start_MaximumRecentAppsChangedCallback(p, dwMaximumFrequentApps);
                        }
                        else
                        {
                            exit(0);
                        }
                        dwInitialMaximumFrequentApps = dwMaximumFrequentApps;
                    }
                    if (lRes == ERROR_SUCCESS && dwMaximumFrequentApps != dwInitialMaximumFrequentApps)
                    {
                        if (params->Start_MaximumRecentAppsChangedCallback)
                        {
                            void* p = 0;
                            if (params->Start_MaximumRecentAppsChangedCallbackData)
                            {
                                p = params->Start_MaximumRecentAppsChangedCallbackData;
                            }
                            params->Start_MaximumRecentAppsChangedCallback(p, dwMaximumFrequentApps);
                        }
                        else
                        {
                            exit(0);
                        }
                        dwInitialMaximumFrequentApps = dwMaximumFrequentApps;
                    }
                }
            }
            else if (rv == WAIT_OBJECT_0 + 1)
            {
                dwSize = sizeof(DWORD);
                lRes = RegQueryValueExW(
                    hKeyCU,
                    TEXT("TaskbarAl"),
                    0,
                    NULL,
                    &dwTaskbarAl,
                    &dwSize
                );
                if (lRes != ERROR_SUCCESS)
                {
                    dwSize = sizeof(DWORD);
                    lRes = RegQueryValueExW(
                        hKeyLM,
                        TEXT("TaskbarAl"),
                        0,
                        NULL,
                        &dwTaskbarAl,
                        &dwSize
                    );
                    if (lRes != ERROR_SUCCESS && dwInitialTaskbarAlWas)
                    {
                        if (params->TaskbarAlChangedCallback)
                        {
                            void* p = 0;
                            if (params->TaskbarAlChangedCallbackData)
                            {
                                p = params->TaskbarAlChangedCallbackData;
                            }
                            params->TaskbarAlChangedCallback(p, dwTaskbarAl);
                        }
                        else
                        {
                            exit(0);
                        }
                        dwInitialTaskbarAl = dwTaskbarAl;
                    }
                    if (lRes == ERROR_SUCCESS && dwTaskbarAl != dwInitialTaskbarAl)
                    {
                        if (params->TaskbarAlChangedCallback)
                        {
                            void* p = 0;
                            if (params->TaskbarAlChangedCallbackData)
                            {
                                p = params->TaskbarAlChangedCallbackData;
                            }
                            params->TaskbarAlChangedCallback(p, dwTaskbarAl);
                        }
                        else
                        {
                            exit(0);
                        }
                        dwInitialTaskbarAl = dwTaskbarAl;
                    }
                }

                if (params->isStartMenuExperienceHost)
                {
                    dwSize = sizeof(DWORD);
                    lRes = RegQueryValueExW(
                        hKeyCU,
                        TEXT("Start_MaximumFrequentApps"),
                        0,
                        NULL,
                        &dwMaximumFrequentApps,
                        &dwSize
                    );
                    if (lRes != ERROR_SUCCESS)
                    {
                        dwSize = sizeof(DWORD);
                        lRes = RegQueryValueExW(
                            hKeyLM,
                            TEXT("Start_MaximumFrequentApps"),
                            0,
                            NULL,
                            &dwMaximumFrequentApps,
                            &dwSize
                        );
                        if (lRes != ERROR_SUCCESS && dwInitialMaximumFrequentAppsWas)
                        {
                            if (params->Start_MaximumRecentAppsChangedCallback)
                            {
                                void* p = 0;
                                if (params->Start_MaximumRecentAppsChangedCallbackData)
                                {
                                    p = params->Start_MaximumRecentAppsChangedCallbackData;
                                }
                                params->Start_MaximumRecentAppsChangedCallback(p, dwMaximumFrequentApps);
                            }
                            else
                            {
                                exit(0);
                            }
                            dwInitialMaximumFrequentApps = dwMaximumFrequentApps;
                        }
                        if (lRes == ERROR_SUCCESS && dwMaximumFrequentApps != dwInitialMaximumFrequentApps)
                        {
                            if (params->Start_MaximumRecentAppsChangedCallback)
                            {
                                void* p = 0;
                                if (params->Start_MaximumRecentAppsChangedCallbackData)
                                {
                                    p = params->Start_MaximumRecentAppsChangedCallbackData;
                                }
                                params->Start_MaximumRecentAppsChangedCallback(p, dwMaximumFrequentApps);
                            }
                            else
                            {
                                exit(0);
                            }
                            dwInitialMaximumFrequentApps = dwMaximumFrequentApps;
                        }
                    }
                }
            }

            CloseHandle(hEvHKCU);
            CloseHandle(hEvHKLM);
            RegCloseKey(hKeyCU);
            RegCloseKey(hKeyLM);
        }
    }
}
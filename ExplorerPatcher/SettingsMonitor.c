#include "SettingsMonitor.h"

DWORD WINAPI MonitorSettings(SettingsChangeParameters* params)
{
	BOOL bShouldExit = FALSE;

	while (TRUE)
	{
		HANDLE* handles = malloc(sizeof(HANDLE) * (params->size + 1));
		if (!handles)
		{
			return 0;
		}
		for (unsigned int i = 0; i < params->size; ++i)
		{
			params->settings[i].hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
			if (!params->settings[i].hEvent)
			{
				return 0;
			}
			handles[i] = params->settings[i].hEvent;
			if (RegCreateKeyExW(
				params->settings[i].origin,
				params->settings[i].name,
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,
				KEY_READ,
				NULL,
				&(params->settings[i].hKey),
				NULL
			) != ERROR_SUCCESS)
			{
				return 0;
			}
			if (RegNotifyChangeKeyValue(
				params->settings[i].hKey,
				FALSE,
				REG_NOTIFY_CHANGE_LAST_SET,
				params->settings[i].hEvent,
				TRUE
			) != ERROR_SUCCESS)
			{
				return 0;
			}
		}
		handles[params->size] = params->hExitEvent;
		DWORD dwRes = WaitForMultipleObjects(
			params->size + (params->hExitEvent ? 1 : 0),
			handles,
			FALSE,
			INFINITE
		);
		if (dwRes != WAIT_FAILED)
		{
			unsigned int i = dwRes - WAIT_OBJECT_0;
			if (i >= 0 && i < params->size)
			{
				params->settings[i].callback(params->settings[i].data);
			}
			else if (i == params->size && params->hExitEvent)
			{
				bShouldExit = TRUE;
			}
		}
		free(handles);
		for (unsigned int i = 0; i < params->size; ++i)
		{
			CloseHandle(params->settings[i].hEvent);
			RegCloseKey(params->settings[i].hKey);
		}
		if (bShouldExit)
		{
			break;
		}
	}
}


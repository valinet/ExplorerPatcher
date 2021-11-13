#include "SettingsMonitor.h"

DWORD WINAPI MonitorSettings(SettingsChangeParameters* params)
{
	BOOL bShouldExit = FALSE;
	HANDLE* handles = NULL;
	while (TRUE)
	{
		handles = calloc(sizeof(HANDLE), params->size);
		if (handles)
		{
			for (unsigned int i = 0; i < params->size; ++i)
			{
				if (i == 0)
				{
					if (params->settings[i].hEvent)
					{
						handles[i] = params->settings[i].hEvent;
						continue;
					}
					else
					{
						free(handles);
						free(params->settings);
						free(params);
						return 0;
					}
				}
				params->settings[i].hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
				if (!params->settings[i].hEvent)
				{
					free(handles);
					free(params->settings);
					free(params);
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
					free(handles);
					free(params->settings);
					free(params);
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
					free(handles);
					free(params->settings);
					free(params);
					return 0;
				}
			}
			DWORD dwRes = WaitForMultipleObjects(
				params->size,
				handles,
				FALSE,
				INFINITE
			);
			if (dwRes != WAIT_FAILED)
			{
				unsigned int i = dwRes - WAIT_OBJECT_0;
				if (i >= 1 && i < params->size)
				{
					params->settings[i].callback(params->settings[i].data);
				}
				else if (i == 0)
				{
					bShouldExit = TRUE;
				}
				for (unsigned int j = 1; j < params->size; ++j)
				{
					if (WaitForSingleObject(handles[j], 0) == WAIT_OBJECT_0)
					{
						params->settings[j].callback(params->settings[j].data);
					}
				}
			}
			free(handles);
			for (unsigned int i = 1; i < params->size; ++i)
			{
				if (params->settings[i].hEvent)
				{
					CloseHandle(params->settings[i].hEvent);
				}
				if (params->settings[i].hKey)
				{
					RegCloseKey(params->settings[i].hKey);
				}
			}
			if (bShouldExit)
			{
				break;
			}
		}
		else
		{
			free(params->settings);
			free(params);
			return 0;
		}
	}
	free(params->settings);
	free(params);
	return 0;
}


#ifndef EP_WEATHER_MICROSOFT_ICONS_H
#define EP_WEATHER_MICROSOFT_ICONS_H

#include <windows.h>

void epw_Weather_PrefetchMicrosoftIcons(void);
BOOL epw_Weather_GetMicrosoftIconScript(LPCWSTR* script);
void epw_Weather_FreeMicrosoftIconScript(void);

#endif

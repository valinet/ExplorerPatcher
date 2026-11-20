#ifndef EP_WEATHER_MICROSOFT_ICONS_H
#define EP_WEATHER_MICROSOFT_ICONS_H

#include <windows.h>

void epw_Weather_PrefetchMicrosoftIcons(void);
// On success the caller owns the snapshot, if one is available.
BOOL epw_Weather_GetMicrosoftIconScript(LPWSTR* script);
void epw_Weather_FreeMicrosoftIconScript(void);

#endif

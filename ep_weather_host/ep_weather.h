#ifndef _H_AS_H_
#define _H_AS_H_
#include <initguid.h>
#include <Windows.h>
#include <tchar.h>
#include <Shlwapi.h>

#pragma comment(lib, "Version.lib")
#pragma comment(lib, "Shlwapi.lib")

#ifndef NTDDI_WIN10_CO
#define DWMWA_USE_HOSTBACKDROPBRUSH 17            // [set] BOOL, Allows the use of host backdrop brushes for the window.
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20          // [set] BOOL, Allows a window to either use the accent color, or dark, according to the user Color Mode preferences.
#define DWMWA_WINDOW_CORNER_PREFERENCE 33         // [set] WINDOW_CORNER_PREFERENCE, Controls the policy that rounds top-level window corners
#define DWMWA_BORDER_COLOR 34                     // [set] COLORREF, The color of the thin border around a top-level window
#define DWMWA_CAPTION_COLOR 35                    // [set] COLORREF, The color of the caption
#define DWMWA_TEXT_COLOR 36                       // [set] COLORREF, The color of the caption text
#define DWMWA_VISIBLE_FRAME_BORDER_THICKNESS 37   // [get] UINT, width of the visible border around a thick frame window
#define DWMWCP_DEFAULT 0
#define DWMWCP_DONOTROUND 1
#define DWMWCP_ROUND 2
#define DWMWCP_ROUNDSMALL 3
#endif
#define DWMWA_MICA_EFFFECT 1029

#define ALLOC(x) calloc(1, x)
#define FREE(x) free(x)

extern HMODULE epw_hModule;
extern DWORD epw_OutstandingObjects;
extern DWORD epw_LockCount;

// {A6EA9C2D-4982-4827-9204-0AC532959F6D}
#define CLSID_EPWeather_Name "ExplorerPatcher Weather Host"
#define CLSID_EPWeather_TEXT "{A6EA9C2D-4982-4827-9204-0AC532959F6D}"
#define EP_Weather_Killswitch "Global\\EP_Weather_Killswitch_" CLSID_EPWeather_TEXT
DEFINE_GUID(CLSID_EPWeather,
    0xa6ea9c2d, 0x4982, 0x4827, 0x92, 0x4, 0xa, 0xc5, 0x32, 0x95, 0x9f, 0x6d);

#if defined(__cplusplus) && !defined(CINTERFACE)
#else
DEFINE_GUID(IID_IEPWeather,
    0xcdbf3734, 0xf847, 0x4f1b, 0xb9, 0x53, 0xa6, 0x5, 0x43, 0x4d, 0xc1, 0xe7);
#endif

#define EPW_WEATHER_CLASSNAME "ExplorerPatcher_Weather_" CLSID_EPWeather_TEXT

#define EP_WEATHER_KEEP_VALUE -1

#define EP_WEATHER_NUM_PROVIDERS 2
#define EP_WEATHER_PROVIDER_TEST 0
#define EP_WEATHER_PROVIDER_GOOGLE 1

#define EP_WEATHER_NUM_TUNITS 2
#define EP_WEATHER_TUNIT_CELSIUS 0
#define EP_WEATHER_TUNIT_FAHRENHEIT 1

#define EP_WEATHER_VIEW_ICONONLY 1
#define EP_WEATHER_VIEW_ICONTEMP 3
#define EP_WEATHER_VIEW_ICONTEXT 0
#define EP_WEATHER_VIEW_TEMPONLY 4
#define EP_WEATHER_VIEW_TEXTONLY 5

#define EP_WEATHER_UPDATE_NORMAL 1200
#define EP_WEATHER_UPDATE_REDUCED 3600

#define EP_WEATHER_WM_FETCH_DATA (WM_USER + 10)
#define EP_WEATHER_WM_SET_BROWSER_THEME (WM_USER + 11)
#define EP_WEATHER_WM_REBOUND_BROWSER (WM_USER + 12)
#define EP_WEATHER_WM_SETDEVMODE (WM_USER + 13)

#define EP_WEATHER_HEIGHT 391
#define EP_WEATHER_WIDTH 690
#endif

#ifndef _H_GUI_H_
#define _H_GUI_H_
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <Windows.h>
#pragma comment(lib, "Version.lib")
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <windowsx.h>
#include <tlhelp32.h>
#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")
#include <conio.h>
#include <stdio.h>
#include <Uxtheme.h>
#pragma comment(lib, "UxTheme.lib")
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#include "resources/resource.h"
#include "../ExplorerPatcher/getline.h"
#include "../ExplorerPatcher/fmemopen.h"
#include "../ExplorerPatcher/Localization.h"
#include "../ExplorerPatcher/utility.h"
#include "../ep_weather_host/ep_weather.h"
#include "../ep_weather_host/ep_weather_host_h.h"

#define MAX_LINE_LENGTH 2000
extern HMODULE hModule;

#define GUI_POSITION_X CW_USEDEFAULT
#define GUI_POSITION_Y CW_USEDEFAULT
#define GUI_POSITION_WIDTH 367
#define GUI_POSITION_HEIGHT 316
#define GUI_WINDOWSWITCHER_THEME_CLASS "ControlPanelStyle"
#define GUI_CAPTION_FONT_SIZE -12
#define GUI_SECTION_FONT_SIZE -12
#define GUI_SECTION_HEIGHT 32
#define GUI_TITLE_FONT_SIZE -12
#define GUI_LINE_HEIGHT 26
#define GUI_CAPTION_LINE_HEIGHT_DEFAULT 42
#define GUI_TEXTCOLOR RGB(0, 0, 0)
#define GUI_TEXTCOLOR_SELECTED RGB(255, 0, 0)
#define GUI_TEXTCOLOR_DARK RGB(240, 240, 240)
#define GUI_TEXTCOLOR_SELECTED_DARK RGB(255, 150, 150)
#define GUI_MAX_TABORDER 9999
#define GUI_PADDING 5
#define GUI_PADDING_LEFT GUI_PADDING * 3
#define GUI_SIDEBAR_WIDTH 150 // 110
#define GUI_PADDING_RIGHT GUI_PADDING * 3
#define GUI_PADDING_TOP GUI_PADDING
#define GUI_PADDING_BOTTOM GUI_PADDING
#define GUI_STATUS_PADDING 10

#define GUI_TIMER_READ_HELP 1
#define GUI_TIMER_READ_HELP_TIMEOUT 1000
#define GUI_TIMER_READ_REPEAT_SELECTION 2
#define GUI_TIMER_READ_REPEAT_SELECTION_TIMEOUT 1000
#define GUI_TIMER_REFRESH_FOR_PEOPLEBAND 2
#define GUI_TIMER_REFRESH_FOR_PEOPLEBAND_TIMEOUT 1000
typedef struct _GUI
{
	POINT location;
	SIZE size;
	RECT padding;
	UINT sidebarWidth;
	HBRUSH hBackgroundBrush;
	HTHEME hTheme;
	POINT dpi;
	MARGINS extent;
	UINT tabOrder;
	DWORD bCalcExtent;
	SIZE_T section;
	DWORD dwStatusbarY;
	HICON hIcon;
	RECT border_thickness;
	UINT GUI_CAPTION_LINE_HEIGHT;
	long long LeftClickTime;
	long long LastClickTime;
	HMODULE hExplorerFrame;
	void* pAccPropServices;
	HWND hAccLabel;
	BOOL bShouldAnnounceSelected;
	WCHAR sectionNames[20][64];
	BOOL bRebuildIfTabOrderIsEmpty;
	int dwPageLocation;
	DWORD last_section;
} GUI;

static BOOL GUI_Build(HDC hDC, HWND hWnd);

static LRESULT CALLBACK GUI_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

__declspec(dllexport) int ZZGUI(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);
#endif

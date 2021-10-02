#ifndef _H_GUI_H_
#define _H_GUI_H_
#include <Windows.h>
#pragma comment(lib, "Version.lib")
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
#include "resource.h"
#include "getline.h"
#include "fmemopen.h"
#include "utility.h"

#define MAX_LINE_LENGTH 2000
extern HMODULE hModule;

#define GUI_POSITION_X CW_USEDEFAULT
#define GUI_POSITION_Y CW_USEDEFAULT
#define GUI_POSITION_WIDTH 467
#define GUI_POSITION_HEIGHT 790
#define GUI_WINDOWSWITCHER_THEME_CLASS "ControlPanelStyle"
#define GUI_CAPTION_FONT_SIZE -22
#define GUI_TITLE_FONT_SIZE -12
#define GUI_LINE_HEIGHT 30
#define GUI_CAPTION_LINE_HEIGHT 42
#define GUI_TEXTCOLOR RGB(0, 0, 0)
#define GUI_PADDING 5
#define GUI_PADDING_LEFT GUI_PADDING * 3
#define GUI_PADDING_RIGHT GUI_PADDING * 3
#define GUI_PADDING_TOP GUI_PADDING
#define GUI_PADDING_BOTTOM GUI_PADDING
typedef struct _GUI
{
	POINT location;
	SIZE size;
	RECT padding;
	HBRUSH hBackgroundBrush;
	HTHEME hTheme;
	POINT dpi;
	MARGINS extent;
} GUI;

static HRESULT GUI_AboutProc(
	HWND hwnd,
	UINT uNotification,
	WPARAM wParam,
	LPARAM lParam,
	LONG_PTR lpRefData
);

static BOOL GUI_Build(HDC hDC, HWND hWnd);

static LRESULT CALLBACK GUI_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

__declspec(dllexport) int ZZGUI(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);
#endif

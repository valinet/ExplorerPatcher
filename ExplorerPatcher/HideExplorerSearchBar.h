#ifndef _H_HIDEEXPLORERSEARCHBAR_H_
#define _H_HIDEEXPLORERSEARCHBAR_H_
#include <Windows.h>
#include <commctrl.h>
#pragma comment(lib, "Comctl32.lib")

// https://stackoverflow.com/questions/30141592/how-do-i-find-a-handle-inside-a-control
HWND FindChildWindow(
    HWND hwndParent,
    wchar_t* lpszClass
);

// https://github.com/Open-Shell/Open-Shell-Menu/blob/master/Src/ClassicExplorer/ExplorerBHO.cpp
VOID HideExplorerSearchBar(HWND hWnd);

LRESULT HideExplorerSearchBarSubClass(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR uIdSubclass,
    DWORD_PTR dwRefData
);
#endif
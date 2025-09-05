#ifndef _H_OSUTILITY_H_
#define _H_OSUTILITY_H_
#include <Windows.h>
#include <dwmapi.h>
#include <valinet/utility/osversion.h>

// This allows compiling with older Windows SDKs as well
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

#ifdef __cplusplus
extern "C" {
#endif

extern RTL_OSVERSIONINFOW global_rovi;
extern DWORD32 global_ubr;

inline void InitializeGlobalVersionAndUBR()
{
    global_ubr = VnGetOSVersionAndUBR(&global_rovi);
}

inline BOOL IsWindows11()
{
    if (!global_rovi.dwMajorVersion) global_ubr = VnGetOSVersionAndUBR(&global_rovi);
    if (global_rovi.dwBuildNumber >= 21996) return TRUE;
    return FALSE;
}

inline BOOL IsDwmExtendFrameIntoClientAreaBrokenInThisBuild()
{
    if (!IsWindows11())
    {
        return FALSE;
    }
    if ((global_rovi.dwBuildNumber >= 21996 && global_rovi.dwBuildNumber < 22000) || (global_rovi.dwBuildNumber == 22000 && (global_ubr >= 1 && global_ubr <= 51)))
    {
        return TRUE;
    }
    return FALSE;
}

inline HRESULT SetMicaMaterialForThisWindow(HWND hWnd, BOOL bApply)
{
    if (!IsWindows11() || IsDwmExtendFrameIntoClientAreaBrokenInThisBuild()) return S_FALSE;
    DWORD dwAttribute = (global_rovi.dwBuildNumber >= 22523) ? 38 : DWMWA_MICA_EFFFECT;
    DWORD dwProp = (bApply ? ((global_rovi.dwBuildNumber >= 22523) ? 2 : 1) : 0);
    return DwmSetWindowAttribute(hWnd, dwAttribute, &dwProp, sizeof(DWORD));
}

inline BOOL IsWindows11Version22H2OrHigher()
{
    if (!global_rovi.dwMajorVersion) global_ubr = VnGetOSVersionAndUBR(&global_rovi);
    return global_rovi.dwBuildNumber >= 22621;
}

inline BOOL IsWindows11Version23H2OrHigher()
{
    if (!global_rovi.dwMajorVersion) global_ubr = VnGetOSVersionAndUBR(&global_rovi);
    return global_rovi.dwBuildNumber >= 22631;
}

inline BOOL IsWindows11BuildHigherThan25158()
{
    if (!global_rovi.dwMajorVersion) global_ubr = VnGetOSVersionAndUBR(&global_rovi);
    return global_rovi.dwBuildNumber > 25158;
}

inline BOOL IsWindows11Build25346OrHigher()
{
    if (!global_rovi.dwMajorVersion) global_ubr = VnGetOSVersionAndUBR(&global_rovi);
    return global_rovi.dwBuildNumber >= 25346;
}

inline BOOL IsWindows11Version22H2Build1413OrHigher()
{
    if (!global_rovi.dwMajorVersion) global_ubr = VnGetOSVersionAndUBR(&global_rovi);
    if (global_rovi.dwBuildNumber > 22621) return TRUE;
    return global_rovi.dwBuildNumber == 22621 && global_ubr >= 1413;
}

inline BOOL IsWindows11Version22H2Build2134OrHigher()
{
    if (!global_rovi.dwMajorVersion) global_ubr = VnGetOSVersionAndUBR(&global_rovi);
    if (global_rovi.dwBuildNumber > 22621) return TRUE;
    return global_rovi.dwBuildNumber == 22621 && global_ubr >= 2134;
}

#ifdef __cplusplus
}
#endif

#endif

#ifndef _H_UTILITY_H_
#define _H_UTILITY_H_
#include <Windows.h>
#include <tchar.h>
#include <windows.data.xml.dom.h>
#include <Shlobj_core.h>
#define _LIBVALINET_INCLUDE_UNIVERSAL
#include <valinet/universal/toast/toast.h>

#define APPID L"Microsoft.Windows.Explorer"

#pragma region "Weird stuff"
INT64 nimpl4_1(INT64 a1, DWORD* a2);
INT64 nimpl4_0(INT64 a1, DWORD* a2);
__int64 __fastcall nimpl2(__int64 a1, uintptr_t* a2);
ULONG nimpl3();
HRESULT nimpl();
HRESULT nimpl1(__int64 a1, uintptr_t* a2, uintptr_t* a3);
HRESULT nimpl1_2(__int64 a1, uintptr_t* a2, uintptr_t* a3);
HRESULT nimpl1_3(__int64 a1, uintptr_t* a2, uintptr_t* a3);
__int64 nimpl4(__int64 a1, __int64 a2, __int64 a3, BYTE* a4);
typedef struct _IActivationFactoryAA
{
    CONST_VTBL struct IActivationFactoryVtbl* lpVtbl;
    struct IActivationFactoryVtbl* lpVtbl2;
    struct IActivationFactoryVtbl* lpVtbl3;
} IActivationFactoryAA;
extern const IActivationFactoryAA XamlExtensionsFactory;
#pragma endregion

// https://stackoverflow.com/questions/1672677/print-a-guid-variable
void printf_guid(GUID guid);

LRESULT CALLBACK BalloonWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

__declspec(dllexport) CALLBACK ZZTestBalloon(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllexport) CALLBACK ZZTestToast(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllexport) CALLBACK ZZLaunchExplorer(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

__declspec(dllexport) CALLBACK ZZLaunchExplorerDelayed(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int nCmdShow);

POINT GetDefaultWinXPosition(BOOL bUseRcWork, BOOL* lpBottom, BOOL* lpRight);
#endif
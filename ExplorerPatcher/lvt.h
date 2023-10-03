#ifndef _H_LVT_H_
#define _H_LVT_H_
#include <initguid.h>
#include <Windows.h>
#include <inspectable.h>
#include <roapi.h>
#include <winstring.h>
#include <stdio.h>
#include <Shlwapi.h>
#include "osutility.h"

#define LVT_LOC_NONE 0
#define LVT_LOC_BOTTOMLEFT 1
#define LVT_LOC_TOPLEFT 2
#define LVT_LOC_TOPRIGHT 3

typedef struct _Windows_UI_Xaml_CornerRadius
{
    double TopLeft;
    double TopRight;
    double BottomRight;
    double BottomLeft;
} Windows_UI_Xaml_CornerRadius;

typedef struct _Windows_UI_Xaml_Thickness
{
    double Left;
    double Top;
    double Right;
    double Bottom;
} Windows_UI_Xaml_Thickness;

typedef enum _Windows_UI_Xaml_Visibility
{
    Windows_UI_Xaml_Visibility_Visible = 0,
    Windows_UI_Xaml_Visibility_Collapsed = 1
} Windows_UI_Xaml_Visibility;

#pragma region "Windows.UI.Xaml.IWindowStatics"

DEFINE_GUID(IID_Windows_UI_Xaml_IWindowStatics,
    0x93328409, 0x4ea1, 0x4afa, 0x83, 0xdc, 0x0c, 0x4e, 0x73, 0xe8, 0x8b, 0xb1);

typedef interface Windows_UI_Xaml_IWindowStatics Windows_UI_Xaml_IWindowStatics;

typedef struct Windows_UI_Xaml_IWindowStatics_Vtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in Windows_UI_Xaml_IWindowStatics* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in Windows_UI_Xaml_IWindowStatics* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in Windows_UI_Xaml_IWindowStatics* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in Windows_UI_Xaml_IWindowStatics* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in Windows_UI_Xaml_IWindowStatics* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in Windows_UI_Xaml_IWindowStatics* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* get_Current)(
        __RPC__in Windows_UI_Xaml_IWindowStatics* This,
        /* [out] */ __RPC__out void** value
        );

    END_INTERFACE
} Windows_UI_Xaml_IWindowStatics_Vtbl;

interface Windows_UI_Xaml_IWindowStatics // : IInspectable
{
    CONST_VTBL struct Windows_UI_Xaml_IWindowStatics_Vtbl* lpVtbl;
};
#pragma endregion

#pragma region "Windows.UI.Xaml.Core.CoreWindow"

typedef interface Windows_UI_Xaml_Core_ICoreWindow Windows_UI_Xaml_Core_ICoreWindow;

typedef struct Windows_UI_Xaml_Core_ICoreWindow_Vtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* get_AutomationHostProvider)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This);

    HRESULT(STDMETHODCALLTYPE* get_Bounds)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This,
        /* [out] */ __RPC__out RECT* value);

    HRESULT(STDMETHODCALLTYPE* get_CustomProperties)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This);

    HRESULT(STDMETHODCALLTYPE* get_Dispatcher)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This);

    HRESULT(STDMETHODCALLTYPE* get_FlowDirection)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This);

    HRESULT(STDMETHODCALLTYPE* put_FlowDirection)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This);

    HRESULT(STDMETHODCALLTYPE* get_IsInputEnabled)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This);

    HRESULT(STDMETHODCALLTYPE* put_IsInputEnabled)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This);

    HRESULT(STDMETHODCALLTYPE* get_PointerCursor)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This);

    HRESULT(STDMETHODCALLTYPE* put_PointerCursor)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This);

    HRESULT(STDMETHODCALLTYPE* get_PointerPosition)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This);

    HRESULT(STDMETHODCALLTYPE* get_Visible)(
        __RPC__in Windows_UI_Xaml_Core_ICoreWindow* This,
        /* [out] */ __RPC__out BOOL* value);

    // ...

    END_INTERFACE
} Windows_UI_Xaml_Core_ICoreWindow_Vtbl;

interface Windows_UI_Xaml_Core_ICoreWindow // : IInspectable
{
    CONST_VTBL struct Windows_UI_Xaml_Core_ICoreWindow_Vtbl* lpVtbl;
};
#pragma endregion

#pragma region "Windows.UI.Xaml.IWindow"

typedef interface Windows_UI_Xaml_IWindow Windows_UI_Xaml_IWindow;

typedef struct Windows_UI_Xaml_IWindow_Vtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in Windows_UI_Xaml_IWindow* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in Windows_UI_Xaml_IWindow* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in Windows_UI_Xaml_IWindow* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in Windows_UI_Xaml_IWindow* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in Windows_UI_Xaml_IWindow* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in Windows_UI_Xaml_IWindow* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* get_Bounds)(
        __RPC__in Windows_UI_Xaml_IWindow* This,
        /* [out] */ __RPC__out RECT* value);

    HRESULT(STDMETHODCALLTYPE* get_Visible)(
        __RPC__in Windows_UI_Xaml_IWindow* This,
        /* [out] */ __RPC__out BOOL* value);

    HRESULT(STDMETHODCALLTYPE* get_Content)(
        __RPC__in Windows_UI_Xaml_IWindow* This,
        /* [out] */ __RPC__out IInspectable** value);

    HRESULT(STDMETHODCALLTYPE* put_Content)(
        __RPC__in Windows_UI_Xaml_IWindow* This);

    HRESULT(STDMETHODCALLTYPE* get_CoreWindow)(
        __RPC__in Windows_UI_Xaml_IWindow* This,
        /* [out] */ __RPC__out Windows_UI_Xaml_Core_ICoreWindow** value);

    // ...

    END_INTERFACE
} Windows_UI_Xaml_IWindow_Vtbl;

interface Windows_UI_Xaml_IWindow // : IInspectable
{
    CONST_VTBL struct Windows_UI_Xaml_IWindow_Vtbl* lpVtbl;
};
#pragma endregion

#pragma region "Windows.UI.Xaml.IDependencyObject"

DEFINE_GUID(IID_Windows_UI_Xaml_IDependencyObject,
    0x5c526665, 0xf60e, 0x4912, 0xaf, 0x59, 0x5f, 0xe0, 0x68, 0x0f, 0x08, 0x9d);

typedef interface Windows_UI_Xaml_IDependencyObject Windows_UI_Xaml_IDependencyObject;

typedef struct Windows_UI_Xaml_IDependencyObject_Vtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in Windows_UI_Xaml_IDependencyObject* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in Windows_UI_Xaml_IDependencyObject* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in Windows_UI_Xaml_IDependencyObject* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in Windows_UI_Xaml_IDependencyObject* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in Windows_UI_Xaml_IDependencyObject* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in Windows_UI_Xaml_IDependencyObject* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* GetValue)(
        __RPC__in Windows_UI_Xaml_IDependencyObject* This,
        /* [in] */ __RPC__in IInspectable* dp,
        /* [out] */ __RPC__out IInspectable** result);

    HRESULT(STDMETHODCALLTYPE* SetValue)(
        __RPC__in Windows_UI_Xaml_IDependencyObject* This,
        /* [in] */ __RPC__in IInspectable* dp,
        /* [in] */ __RPC__in IInspectable* value);

    END_INTERFACE
} Windows_UI_Xaml_IDependencyObject_Vtbl;

interface Windows_UI_Xaml_IDependencyObject // : IInspectable
{
    CONST_VTBL struct Windows_UI_Xaml_IDependencyObject_Vtbl* lpVtbl;
};
#pragma endregion

#pragma region "Windows.UI.Xaml.IVisualTreeHelperStatics"

DEFINE_GUID(IID_Windows_UI_Xaml_IVisualTreeHelperStatics,
    0xe75758c4, 0xd25d, 0x4b1d, 0x97, 0x1f, 0x59, 0x6f, 0x17, 0xf1, 0x2b, 0xaa);

typedef interface Windows_UI_Xaml_IVisualTreeHelperStatics Windows_UI_Xaml_IVisualTreeHelperStatics;

typedef struct Windows_UI_Xaml_IVisualTreeHelperStatics_Vtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* FindElementsInHostCoordinatesPoint)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This);

    HRESULT(STDMETHODCALLTYPE* FindElementsInHostCoordinatesRect)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This);

    HRESULT(STDMETHODCALLTYPE* FindAllElementsInHostCoordinatesPoint)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This);

    HRESULT(STDMETHODCALLTYPE* FindAllElementsInHostCoordinatesRect)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This);

    HRESULT(STDMETHODCALLTYPE* GetChild)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_IDependencyObject* reference,
        /* [in] */ __RPC__in INT32 childIndex,
        /* [out] */ __RPC__out Windows_UI_Xaml_IDependencyObject* result);

    HRESULT(STDMETHODCALLTYPE* GetChildrenCount)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_IDependencyObject* reference,
        /* [out] */ __RPC__out INT32* result);

    HRESULT(STDMETHODCALLTYPE* GetParent)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_IDependencyObject* reference,
        /* [out] */ __RPC__out Windows_UI_Xaml_IDependencyObject** result);

    HRESULT(STDMETHODCALLTYPE* DisconnectChildrenRecursive)(
        __RPC__in Windows_UI_Xaml_IVisualTreeHelperStatics* This);

    END_INTERFACE
} Windows_UI_Xaml_IVisualTreeHelperStatics_Vtbl;

interface Windows_UI_Xaml_IVisualTreeHelperStatics // : IInspectable
{
    CONST_VTBL struct Windows_UI_Xaml_IVisualTreeHelperStatics_Vtbl* lpVtbl;
};
#pragma endregion

#pragma region "Windows.UI.Xaml.IFrameworkElement"

DEFINE_GUID(IID_Windows_UI_Xaml_IFrameworkElement,
    0xa391d09b, 0x4a99, 0x4b7c, 0x9d, 0x8d, 0x6f, 0xa5, 0xd0, 0x1f, 0x6f, 0xbf);

typedef interface Windows_UI_Xaml_IFrameworkElement Windows_UI_Xaml_IFrameworkElement;

typedef struct Windows_UI_Xaml_IFrameworkElement_Vtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* get_Triggers)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* get_Resources)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* put_Resources)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* get_Tag)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* put_Tag)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* get_Language)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* put_Language)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* get_ActualWidth)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [out] */ __RPC__out DOUBLE* value);

    HRESULT(STDMETHODCALLTYPE* get_ActualHeight)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [out] */ __RPC__out DOUBLE* value);

    HRESULT(STDMETHODCALLTYPE* get_Width)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [out] */ __RPC__out DOUBLE* value);

    HRESULT(STDMETHODCALLTYPE* put_Width)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [in] */ __RPC__in DOUBLE value);

    HRESULT(STDMETHODCALLTYPE* get_Height)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [out] */ __RPC__out DOUBLE* value);

    HRESULT(STDMETHODCALLTYPE* put_Height)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [in] */ __RPC__in DOUBLE value);

    HRESULT(STDMETHODCALLTYPE* get_MinWidth)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* put_MinWidth)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* get_MaxWidth)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* put_MaxWidth)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* get_MinHeight)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* put_MinHeight)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* get_MaxHeight)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [out] */ __RPC__out DOUBLE* value);

    HRESULT(STDMETHODCALLTYPE* put_MaxHeight)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [in] */ __RPC__in DOUBLE value);

    HRESULT(STDMETHODCALLTYPE* get_HorizontalAlignment)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* put_HorizontalAlignment)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* get_VerticalAlignment)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* put_VerticalAlignment)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* get_Margin)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* put_Margin)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    HRESULT(STDMETHODCALLTYPE* get_Name)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* value);

    HRESULT(STDMETHODCALLTYPE* put_Name)(
        __RPC__in Windows_UI_Xaml_IFrameworkElement* This);

    // ...

    END_INTERFACE
} Windows_UI_Xaml_IFrameworkElement_Vtbl;

interface Windows_UI_Xaml_IFrameworkElement // : IInspectable
{
    CONST_VTBL struct Windows_UI_Xaml_IFrameworkElement_Vtbl* lpVtbl;
};
#pragma endregion

#pragma region "Windows.UI.Xaml.Controls.IGrid2"

DEFINE_GUID(IID_Windows_UI_Xaml_Controls_IGrid2,
    0xf76efa41, 0x380e, 0x45db, 0xbe, 0x87, 0x9e, 0x13, 0x26, 0xba, 0x4b, 0x57);

typedef interface Windows_UI_Xaml_Controls_IGrid2 Windows_UI_Xaml_Controls_IGrid2;

typedef struct Windows_UI_Xaml_Controls_IGrid2_Vtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* get_BorderBrush)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This);

    HRESULT(STDMETHODCALLTYPE* put_BorderBrush)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This);

    HRESULT(STDMETHODCALLTYPE* get_BorderThickness)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This);

    HRESULT(STDMETHODCALLTYPE* put_BorderThickness)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This);

    HRESULT(STDMETHODCALLTYPE* get_CornerRadius)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This,
        /* [out] */ __RPC__deref_out_opt Windows_UI_Xaml_CornerRadius* value);

    HRESULT(STDMETHODCALLTYPE* put_CornerRadius)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_CornerRadius value);

    HRESULT(STDMETHODCALLTYPE* get_Padding)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This);

    HRESULT(STDMETHODCALLTYPE* put_Padding)(
        __RPC__in Windows_UI_Xaml_Controls_IGrid2* This);

    END_INTERFACE
} Windows_UI_Xaml_Controls_IGrid2_Vtbl;

interface Windows_UI_Xaml_Controls_IGrid2 // : IInspectable
{
    CONST_VTBL struct Windows_UI_Xaml_Controls_IGrid2_Vtbl* lpVtbl;
};
#pragma endregion

#pragma region "Windows.UI.Xaml.Controls.IBorder"

DEFINE_GUID(IID_Windows_UI_Xaml_Controls_IBorder,
    0x797c4539, 0x45bd, 0x4633, 0xa0, 0x44, 0xbf, 0xb0, 0x2e, 0xf5, 0x17, 0x0f);

typedef interface Windows_UI_Xaml_Controls_IBorder Windows_UI_Xaml_Controls_IBorder;

typedef struct Windows_UI_Xaml_Controls_IBorder_Vtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* get_BorderBrush)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    HRESULT(STDMETHODCALLTYPE* put_BorderBrush)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    HRESULT(STDMETHODCALLTYPE* get_BorderThickness)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    HRESULT(STDMETHODCALLTYPE* put_BorderThickness)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    HRESULT(STDMETHODCALLTYPE* get_Background)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    HRESULT(STDMETHODCALLTYPE* put_Background)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    HRESULT(STDMETHODCALLTYPE* get_CornerRadius)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This,
        /* [out] */ __RPC__deref_out_opt Windows_UI_Xaml_CornerRadius* value);

    HRESULT(STDMETHODCALLTYPE* put_CornerRadius)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_CornerRadius value);

    HRESULT(STDMETHODCALLTYPE* get_Padding)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    HRESULT(STDMETHODCALLTYPE* put_Padding)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    HRESULT(STDMETHODCALLTYPE* get_Child)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    HRESULT(STDMETHODCALLTYPE* put_Child)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    HRESULT(STDMETHODCALLTYPE* get_ChildTransitions)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    HRESULT(STDMETHODCALLTYPE* put_ChildTransitions)(
        __RPC__in Windows_UI_Xaml_Controls_IBorder* This);

    END_INTERFACE
} Windows_UI_Xaml_Controls_IBorder_Vtbl;

interface Windows_UI_Xaml_Controls_IBorder // : IInspectable
{
    CONST_VTBL struct Windows_UI_Xaml_Controls_IBorder_Vtbl* lpVtbl;
};
#pragma endregion

#pragma region "Windows.UI.Xaml.Controls.IControl"

DEFINE_GUID(IID_Windows_UI_Xaml_Controls_IControl,
    0xa8912263, 0x2951, 0x4f58, 0xa9, 0xc5, 0x5a, 0x13, 0x4e, 0xaa, 0x7f, 0x07);

typedef interface Windows_UI_Xaml_Controls_IControl Windows_UI_Xaml_Controls_IControl;

typedef struct Windows_UI_Xaml_Controls_IControl_Vtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* get_FontSize)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_FontSize)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_FontFamily)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_FontFamily)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_FontWeight)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_FontWeight)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_FontStyle)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_FontStyle)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_FontStretch)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_FontStretch)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_CharacterSpacing)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_CharacterSpacing)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_Foreground)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_Foreground)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_IsTabStop)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_IsTabStop)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_IsEnabled)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_IsEnabled)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_TabIndex)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_TabIndex)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_TabNavigation)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_TabNavigation)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_Template)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_Template)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_Padding)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This,
        /* [out] */ __RPC__out Windows_UI_Xaml_Thickness* value);

    HRESULT(STDMETHODCALLTYPE* put_Padding)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_Thickness value);

    HRESULT(STDMETHODCALLTYPE* get_HorizontalContentAlignment)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_HorizontalContentAlignment)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_VerticalContentAlignment)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_VerticalContentAlignment)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_Background)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_Background)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_BorderThickness)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_BorderThickness)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_BorderBrush)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* put_BorderBrush)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* get_FocusState)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* add_IsEnabledChanged)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* remove_IsEnabledChanged)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* ApplyTemplate)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    HRESULT(STDMETHODCALLTYPE* Focus)(
        __RPC__in Windows_UI_Xaml_Controls_IControl* This);

    END_INTERFACE
} Windows_UI_Xaml_Controls_IControl_Vtbl;

interface Windows_UI_Xaml_Controls_IControl // : IInspectable
{
    CONST_VTBL struct Windows_UI_Xaml_Controls_IControl_Vtbl* lpVtbl;
};
#pragma endregion

#pragma region "Windows.UI.Xaml.IUIElement"

DEFINE_GUID(IID_Windows_UI_Xaml_IUIElement,
    0x676d0be9, 0xb65c, 0x41c6, 0xba, 0x40, 0x58, 0xcf, 0x87, 0xf2, 0x01, 0xc1);

typedef interface Windows_UI_Xaml_IUIElement Windows_UI_Xaml_IUIElement;

typedef struct Windows_UI_Xaml_IUIElement_Vtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in Windows_UI_Xaml_IUIElement* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in Windows_UI_Xaml_IUIElement* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in Windows_UI_Xaml_IUIElement* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in Windows_UI_Xaml_IUIElement* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* get_DesiredSize)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* get_AllowDrop)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* put_AllowDrop)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* get_Opacity)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* put_Opacity)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* get_Clip)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* put_Clip)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* get_RenderTransform)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* put_RenderTransform)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* get_Projection)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* put_Projection)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* get_RenderTransformOrigin)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* put_RenderTransformOrigin)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* get_IsHitTestVisible)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* put_IsHitTestVisible)(
        __RPC__in Windows_UI_Xaml_IUIElement* This);

    HRESULT(STDMETHODCALLTYPE* get_Visibility)(
        __RPC__in Windows_UI_Xaml_IUIElement* This,
        /* [out] */ __RPC__out Windows_UI_Xaml_Visibility* value);

    HRESULT(STDMETHODCALLTYPE* put_Visibility)(
        __RPC__in Windows_UI_Xaml_IUIElement* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_Visibility value);

    // ...

    END_INTERFACE
} Windows_UI_Xaml_IUIElement_Vtbl;

interface Windows_UI_Xaml_IUIElement // : IInspectable
{
    CONST_VTBL struct Windows_UI_Xaml_IUIElement_Vtbl* lpVtbl;
};
#pragma endregion

#pragma region "Windows.UI.Xaml.Controls.ICanvasStatics"

DEFINE_GUID(IID_Windows_UI_Xaml_Controls_ICanvasStatics,
    0x40ce5c46, 0x2962, 0x446f, 0xaa, 0xfb, 0x4c, 0xdc, 0x48, 0x69, 0x39, 0xc9);

typedef interface Windows_UI_Xaml_Controls_ICanvasStatics Windows_UI_Xaml_Controls_ICanvasStatics;

typedef struct Windows_UI_Xaml_Controls_ICanvasStatics_Vtbl
{
    BEGIN_INTERFACE

    HRESULT(STDMETHODCALLTYPE* QueryInterface)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */
        _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This);

    HRESULT(STDMETHODCALLTYPE* GetIids)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [out] */ __RPC__out ULONG* iidCount,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*iidCount) IID** iids);

    HRESULT(STDMETHODCALLTYPE* GetRuntimeClassName)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [out] */ __RPC__deref_out_opt HSTRING* className);

    HRESULT(STDMETHODCALLTYPE* GetTrustLevel)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [out] */ __RPC__out TrustLevel* trustLevel);

    HRESULT(STDMETHODCALLTYPE* get_LeftProperty)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [out] */ __RPC__out IInspectable** value);

    HRESULT(STDMETHODCALLTYPE* GetLeft)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_IUIElement* element,
        /* [out] */ __RPC__out DOUBLE* result);

    HRESULT(STDMETHODCALLTYPE* SetLeft)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_IUIElement* element,
        /* [in] */ __RPC__in DOUBLE length);

    HRESULT(STDMETHODCALLTYPE* get_TopProperty)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [out] */ __RPC__out IInspectable** value);

    HRESULT(STDMETHODCALLTYPE* GetTop)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_IUIElement* element,
        /* [out] */ __RPC__out DOUBLE* result);

    HRESULT(STDMETHODCALLTYPE* SetTop)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_IUIElement* element,
        /* [in] */ __RPC__in DOUBLE length);

    HRESULT(STDMETHODCALLTYPE* get_ZIndexProperty)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [out] */ __RPC__out IInspectable** value);

    HRESULT(STDMETHODCALLTYPE* GetZIndex)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_IUIElement* element,
        /* [out] */ __RPC__out INT32* result);

    HRESULT(STDMETHODCALLTYPE* SetZIndex)(
        __RPC__in Windows_UI_Xaml_Controls_ICanvasStatics* This,
        /* [in] */ __RPC__in Windows_UI_Xaml_IUIElement* element,
        /* [in] */ __RPC__in INT32 value);

    END_INTERFACE
} Windows_UI_Xaml_Controls_ICanvasStatics_Vtbl;

interface Windows_UI_Xaml_Controls_ICanvasStatics // : IInspectable
{
    CONST_VTBL struct Windows_UI_Xaml_Controls_ICanvasStatics_Vtbl* lpVtbl;
};
#pragma endregion

Windows_UI_Xaml_IDependencyObject* LVT_FindChildByClassName(Windows_UI_Xaml_IDependencyObject* pRootDependencyObject, Windows_UI_Xaml_IVisualTreeHelperStatics* pVisualTreeHelperStatics, LPCWSTR pwszRefName, INT* prevIndex);

Windows_UI_Xaml_IDependencyObject* LVT_FindChildByName(Windows_UI_Xaml_IDependencyObject* pRootDependencyObject, Windows_UI_Xaml_IVisualTreeHelperStatics* pVisualTreeHelperStatics, LPCWSTR pwszRefName);

void LVT_StartUI_EnableRoundedCorners(HWND, DWORD, DWORD, HWND, RECT*);

void LVT_StartDocked_DisableRecommendedSection(HWND, BOOL, RECT*);

HRESULT IsThreadCoreWindowVisible(BOOL*);
#endif

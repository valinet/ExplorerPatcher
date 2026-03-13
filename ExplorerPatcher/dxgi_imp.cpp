#include "dxgi_imp.h"

#include <Windows.h>

EXTERN_C_START

void* SetupRealDXGIImportFunction(const char* pszName)
{
    static HMODULE hRealDXGI = []() -> HMODULE
    {
        TCHAR szRealDXGIPath[MAX_PATH];
        GetSystemDirectoryW(szRealDXGIPath, MAX_PATH);
        wcscat_s(szRealDXGIPath, MAX_PATH, L"\\dxgi.dll");
        return LoadLibraryW(szRealDXGIPath);
    }();
    return hRealDXGI ? GetProcAddress(hRealDXGI, pszName) : nullptr;
}

#define DXGI_SETUP_FUNC(func) static func##_t func##Func = (func##_t)SetupRealDXGIImportFunction(#func)

typedef HRESULT (WINAPI *ApplyCompatResolutionQuirking_t)(UINT*, UINT*);
HRESULT WINAPI ApplyCompatResolutionQuirking(UINT* p1, UINT* p2)
{
    DXGI_SETUP_FUNC(ApplyCompatResolutionQuirking);
    return ApplyCompatResolutionQuirkingFunc(p1, p2);
}
typedef HRESULT (WINAPI *CompatString_t)(const char*, DWORD*, char*, bool);
HRESULT WINAPI CompatString(const char* p1, DWORD* p2, char* p3, bool p4)
{
    DXGI_SETUP_FUNC(CompatString);
    return CompatStringFunc(p1, p2, p3, p4);
}
typedef HRESULT (WINAPI *CompatValue_t)(const char*, UINT64*);
HRESULT WINAPI CompatValue(const char* p1, UINT64* p2)
{
    DXGI_SETUP_FUNC(CompatValue);
    return CompatValueFunc(p1, p2);
}
typedef HRESULT (WINAPI *CreateDXGIFactory_t)(REFIID, void**);
HRESULT WINAPI CreateDXGIFactory(REFIID p1, void** p2)
{
    DXGI_SETUP_FUNC(CreateDXGIFactory);
    return CreateDXGIFactoryFunc(p1, p2);
}
typedef HRESULT (WINAPI *CreateDXGIFactory1_t)(REFIID, void**);
/*__declspec(dllexport)*/ HRESULT WINAPI CreateDXGIFactory1Original(REFIID p1, void** p2)
{
    DXGI_SETUP_FUNC(CreateDXGIFactory1);
    return CreateDXGIFactory1Func(p1, p2);
}
typedef HRESULT (WINAPI *CreateDXGIFactory2_t)(UINT, REFIID, void**);
HRESULT WINAPI CreateDXGIFactory2(UINT p1, REFIID p2, void** p3)
{
    DXGI_SETUP_FUNC(CreateDXGIFactory2);
    return CreateDXGIFactory2Func(p1, p2, p3);
}
typedef HRESULT (WINAPI *DXGID3D10CreateDevice_t)();
HRESULT WINAPI DXGID3D10CreateDevice()
{
    DXGI_SETUP_FUNC(DXGID3D10CreateDevice);
    return DXGID3D10CreateDeviceFunc();
}
typedef HRESULT (WINAPI *DXGID3D10CreateLayeredDevice_t)();
HRESULT WINAPI DXGID3D10CreateLayeredDevice()
{
    DXGI_SETUP_FUNC(DXGID3D10CreateLayeredDevice);
    return DXGID3D10CreateLayeredDeviceFunc();
}
typedef HRESULT (WINAPI *DXGID3D10GetLayeredDeviceSize_t)();
HRESULT WINAPI DXGID3D10GetLayeredDeviceSize()
{
    DXGI_SETUP_FUNC(DXGID3D10GetLayeredDeviceSize);
    return DXGID3D10GetLayeredDeviceSizeFunc();
}
typedef HRESULT (WINAPI *DXGID3D10RegisterLayers_t)();
HRESULT WINAPI DXGID3D10RegisterLayers()
{
    DXGI_SETUP_FUNC(DXGID3D10RegisterLayers);
    return DXGID3D10RegisterLayersFunc();
}
typedef HRESULT (WINAPI *DXGIDeclareAdapterRemovalSupport_t)();
/*__declspec(dllexport)*/ HRESULT WINAPI DXGIDeclareAdapterRemovalSupportOriginal()
{
    DXGI_SETUP_FUNC(DXGIDeclareAdapterRemovalSupport);
    return DXGIDeclareAdapterRemovalSupportFunc();
}
typedef HRESULT (WINAPI *DXGIDumpJournal_t)(void*);
HRESULT WINAPI DXGIDumpJournal(void* p1)
{
    DXGI_SETUP_FUNC(DXGIDumpJournal);
    return DXGIDumpJournalFunc(p1);
}
typedef HRESULT (WINAPI *DXGIGetDebugInterface1_t)(UINT, REFIID, void**);
HRESULT WINAPI DXGIGetDebugInterface1(UINT p1, REFIID p2, void** p3)
{
    DXGI_SETUP_FUNC(DXGIGetDebugInterface1);
    return DXGIGetDebugInterface1Func(p1, p2, p3);
}
typedef HRESULT (WINAPI *DXGIReportAdapterConfiguration_t)(void*);
HRESULT WINAPI DXGIReportAdapterConfiguration(void* p1)
{
    DXGI_SETUP_FUNC(DXGIReportAdapterConfiguration);
    return DXGIReportAdapterConfigurationFunc(p1);
}
typedef HRESULT (WINAPI *PIXBeginCapture_t)(UINT, void*);
HRESULT WINAPI PIXBeginCapture(UINT p1, void* p2)
{
    DXGI_SETUP_FUNC(PIXBeginCapture);
    return PIXBeginCaptureFunc(p1, p2);
}
typedef HRESULT (WINAPI *PIXEndCapture_t)();
HRESULT WINAPI PIXEndCapture()
{
    DXGI_SETUP_FUNC(PIXEndCapture);
    return PIXEndCaptureFunc();
}
typedef HRESULT (WINAPI *PIXGetCaptureState_t)();
HRESULT WINAPI PIXGetCaptureState()
{
    DXGI_SETUP_FUNC(PIXGetCaptureState);
    return PIXGetCaptureStateFunc();
}
typedef HRESULT (WINAPI *SetAppCompatStringPointer_t)(SIZE_T, const char*);
HRESULT WINAPI SetAppCompatStringPointer(SIZE_T p1, const char* p2)
{
    DXGI_SETUP_FUNC(SetAppCompatStringPointer);
    return SetAppCompatStringPointerFunc(p1, p2);
}
typedef HRESULT (WINAPI *UpdateHMDEmulationStatus_t)(bool);
HRESULT WINAPI UpdateHMDEmulationStatus(bool p1)
{
    DXGI_SETUP_FUNC(UpdateHMDEmulationStatus);
    return UpdateHMDEmulationStatusFunc(p1);
}

EXTERN_C_END

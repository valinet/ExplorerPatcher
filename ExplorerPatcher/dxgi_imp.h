#pragma once

#include <Windows.h>

EXTERN_C_START

__declspec(dllexport) HRESULT WINAPI ApplyCompatResolutionQuirking(UINT* p1, UINT* p2);
__declspec(dllexport) HRESULT WINAPI CompatString(const char* p1, DWORD* p2, char* p3, bool p4);
__declspec(dllexport) HRESULT WINAPI CompatValue(const char* p1, UINT64* p2);
__declspec(dllexport) HRESULT WINAPI CreateDXGIFactory(REFIID p1, void** p2);
__declspec(dllexport) HRESULT WINAPI CreateDXGIFactory1Original(REFIID p1, void** p2);
__declspec(dllexport) HRESULT WINAPI CreateDXGIFactory2(UINT p1, REFIID p2, void** p3);
__declspec(dllexport) HRESULT WINAPI DXGID3D10CreateDevice();
__declspec(dllexport) HRESULT WINAPI DXGID3D10CreateLayeredDevice();
__declspec(dllexport) HRESULT WINAPI DXGID3D10GetLayeredDeviceSize();
__declspec(dllexport) HRESULT WINAPI DXGID3D10RegisterLayers();
__declspec(dllexport) HRESULT WINAPI DXGIDeclareAdapterRemovalSupportOriginal();
__declspec(dllexport) HRESULT WINAPI DXGIDumpJournal(void* p1);
__declspec(dllexport) HRESULT WINAPI DXGIGetDebugInterface1(UINT p1, REFIID p2, void** p3);
__declspec(dllexport) HRESULT WINAPI DXGIReportAdapterConfiguration(void* p1);
__declspec(dllexport) HRESULT WINAPI PIXBeginCapture(UINT p1, void* p2);
__declspec(dllexport) HRESULT WINAPI PIXEndCapture();
__declspec(dllexport) HRESULT WINAPI PIXGetCaptureState();
__declspec(dllexport) HRESULT WINAPI SetAppCompatStringPointer(SIZE_T p1, const char* p2);
__declspec(dllexport) HRESULT WINAPI UpdateHMDEmulationStatus(bool p1);

EXTERN_C_END

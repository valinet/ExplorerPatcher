#ifndef _H_AS_FACTORY_H_
#define _H_AS_FACTORY_H_
#include "ep_weather.h"
ULONG STDMETHODCALLTYPE epw_factory_AddRef(IClassFactory* _this);
ULONG STDMETHODCALLTYPE epw_factory_Release(IClassFactory* _this);
HRESULT STDMETHODCALLTYPE epw_factory_QueryInterface(
    IClassFactory* _this,
    REFIID riid,
    void** ppv
);
HRESULT STDMETHODCALLTYPE epw_factory_LockServer(
    IClassFactory* _this,
    BOOL flock
);
HRESULT STDMETHODCALLTYPE epw_factory_CreateInstance(
    IClassFactory* _this,
    IUnknown* punkOuter,
    REFIID vTableGuid,
    void** ppv
);
typedef interface IEPWeatherFactory IEPWeatherFactory;
// {A25216A3-4223-4CB3-A572-11A7CC1AEE4E}
DEFINE_GUID(IID_IEPWeatherFactory,
    0xa25216a3, 0x4223, 0x4cb3, 0xa5, 0x72, 0x11, 0xa7, 0xcc, 0x1a, 0xee, 0x4e);
static const IClassFactoryVtbl IEPWeatherFactoryVtbl = {
    epw_factory_QueryInterface,
    epw_factory_AddRef,
    epw_factory_Release,
    epw_factory_CreateInstance,
    epw_factory_LockServer
};
static IClassFactory IClassFactoryInstance = { &IEPWeatherFactoryVtbl };
static IClassFactory* ClassFactory = &IClassFactoryInstance;
#endif

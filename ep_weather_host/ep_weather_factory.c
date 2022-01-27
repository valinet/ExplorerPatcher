#include "ep_weather_factory.h"
#include "ep_weather_host.h"

ULONG STDMETHODCALLTYPE epw_factory_AddRef(IClassFactory* _this)
{
    return(1);
}

ULONG STDMETHODCALLTYPE epw_factory_Release(IClassFactory* _this)
{
    return(1);
}

HRESULT STDMETHODCALLTYPE epw_factory_QueryInterface(
    IClassFactory* _this,
    REFIID riid,
    void** ppv
)
{
    if (!IsEqualIID(riid, &IID_IUnknown) &&
        !IsEqualIID(riid, &IID_IClassFactory))
    {
        *ppv = 0;
        return(E_NOINTERFACE);
    }
    *ppv = _this;
    _this->lpVtbl->AddRef(_this);
    return(NOERROR);
}

HRESULT STDMETHODCALLTYPE epw_factory_LockServer(
    IClassFactory* this,
    BOOL flock
)
{
    if (flock) InterlockedIncrement(&epw_LockCount);
    else
    {
        LONG dwOutstandingLocks = InterlockedDecrement(&epw_LockCount);
        LONG dwOutstandingObjects = InterlockedAdd(&epw_OutstandingObjects, 0);
        if (!dwOutstandingObjects && !dwOutstandingLocks)
        {
        }
    }
    return(NOERROR);
}

HRESULT STDMETHODCALLTYPE epw_factory_CreateInstance(
    IClassFactory* _this,
    IUnknown* punkOuter,
    REFIID vTableGuid,
    void** ppv
)
{
    HRESULT hr = E_NOINTERFACE;
    EPWeather* thisobj = NULL;

    *ppv = 0;

    if (punkOuter)
    {
        hr = CLASS_E_NOAGGREGATION;
    }
    else
    {
        BOOL bOk = FALSE;
        if (IsEqualIID(vTableGuid, &IID_IEPWeather))
        {
            if (!(thisobj = ALLOC(sizeof(EPWeather))))
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                thisobj->lpVtbl = &IEPWeather_Vtbl;
                bOk = TRUE;
            }
        }
        if (bOk)
        {
            thisobj->cbCount = 1;
            hr = thisobj->lpVtbl->QueryInterface(thisobj, vTableGuid, ppv);
            thisobj->lpVtbl->Release(thisobj);
            if (SUCCEEDED(hr)) InterlockedIncrement(&epw_OutstandingObjects);
        }
        else
        {
            return hr;
        }
    }

    return(hr);
}
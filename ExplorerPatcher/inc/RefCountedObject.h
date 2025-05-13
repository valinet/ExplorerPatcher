#pragma once

#include <wrl/client.h>

template <typename T>
class CRefCountedObject : public IUnknown, public T
{
public:
    template <typename ...TArgs>
    CRefCountedObject(TArgs&& ...args)
        : T(std::forward<TArgs>(args)...)
        , _cRef(0)
    {
    }

    virtual ~CRefCountedObject()
    {
    }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override
    {
        *ppvObject = nullptr;
        return E_NOTIMPL;
    }

    STDMETHODIMP_(ULONG) AddRef() override
    {
        return InterlockedIncrement(&_cRef);
    }

    STDMETHODIMP_(ULONG) Release() override
    {
        ULONG refCount = InterlockedDecrement(&_cRef);
        if (refCount == 0)
            delete this;
        return refCount;
    }

    ULONG _cRef;
};

template <typename T, typename ...TArgs>
Microsoft::WRL::ComPtr<CRefCountedObject<T>> CreateRefCountedObj(TArgs&& ...args)
{
    return new(std::nothrow) CRefCountedObject<T>(std::forward<TArgs>(args)...);
}

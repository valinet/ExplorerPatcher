#pragma once

#include <Windows.h>

#include <memsafe.h>

template<typename T>
class CTContainer_PolicyUnOwned
{
public:
    static void Destroy(T* p) {}
};

template<typename T>
class CTContainer_PolicyRelease
{
public:
    static void Destroy(T* p)
    {
        if (p)
            p->Release();
    }
};

class CTContainer_PolicyNewMem
{
public:
    template<typename T>
    static void Destroy(T* p)
    {
        delete p;
    }
};

class CTContainer_PolicyCoTaskMem
{
public:
    static void Destroy(void* p)
    {
        CoTaskMemFree(p);
    }
};

class CTContainer_PolicyLocalMem
{
public:
    static void Destroy(void* p)
    {
        DestroyMem(p);
    }

    static BOOL DestroyMem(void* p)
    {
        return !LocalFree(p);
    }
};

template <typename T>
class CTPolicyCoTaskMem : CTContainer_PolicyCoTaskMem
{
public:
    static void Destroy(void* p)
    {
        CTContainer_PolicyCoTaskMem::Destroy(p);
    }

    static HRESULT ReallocArray(T* pv, size_t cItems, T** ppv)
    {
        return CoReallocArray(pv, cItems, ppv);
    }
};

template <typename T>
class CTPolicyLocalMem : CTContainer_PolicyLocalMem
{
public:
    static void Destroy(void* p)
    {
        DestroyMem(p);
    }

    static HRESULT ReallocArray(T* pv, size_t cItems, T** ppv)
    {
        return LocalReallocArray(pv, cItems, ppv);
    }
};

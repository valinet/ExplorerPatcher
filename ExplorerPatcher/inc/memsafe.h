// Downloaded from:
// https://github.com/namealt/winsdk10/blob/d1acc505c51b11a6ceafb0f93c9dc584b8b4a9d3/Include/10.0.16299.0/um/memsafe.h
//
//    Copyright (C) Microsoft.  All rights reserved.
//
#if (_MSC_VER > 1000)
#pragma once
#endif

#ifndef __memsafe_h__
#define __memsafe_h__

#ifdef __cplusplus

//
//  Various heap allocation helpers, featuring
//    - Fully annotated
//    - HRESULT return values
//    - Integer overflow checks via intsafe.h
//    - Type safety via templates (no typecasting required)
//    - Zero initialization
//
//  CoAllocBytes
//  CoReallocBytes
//  CoAllocObject
//  CoAllocArray
//  CoReallocArray
//
//  CoAllocString
//  CoAllocStringLen
//  CoAllocStringDoubleNullTerminate
//  CoAllocStringOpt
//
//  LocalAllocBytes
//  LocalReallocBytes
//  LocalAllocObject
//  LocalAllocArray
//  LocalReallocArray
//
//  LocalAllocString
//  LocalAllocStringLen
//  LocalAllocStringDoubleNullTerminate
//  LocalAllocStringOpt
//
//  HeapAllocBytes
//  HeapReallocBytes
//  HeapAllocObject
//  HeapAllocArray
//  HeapReallocArray
//
//  HeapAllocString
//  HeapAllocStringLen
//  HeapAllocStringDoubleNullTerminate
//  HeapAllocStringOpt
//
//  GlobalAllocBytes
//  GlobalReallocBytes
//  GlobalAllocObject
//  GlobalAllocArray
//  GlobalReallocArray
//
//  GlobalAllocString
//  GlobalAllocStringLen
//  GlobalAllocStringDoubleNullTerminate
//  GlobalAllocStringOpt
//

#include <intsafe.h>
#include <strsafe.h>

// Flag for inhibiting zero-initialization
#define NO_ZERO_INIT        0x00000000

// Templates for isolating T* <--> void* conversions and integer arithmetic

template <class T, class TAllocPolicy>
 inline HRESULT _AllocBytes(_In_opt_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return TAllocPolicy::Alloc(hHeap, dwFlags, cb, (void**)ppv);
}

template <class T, class TAllocPolicy>
inline HRESULT _ReallocBytes(_In_opt_ HANDLE hHeap, _In_ DWORD dwFlags, _In_opt_ T *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return TAllocPolicy::Realloc(hHeap, dwFlags, pv, cb, (void**)ppv);
}

template <class T, class TAllocPolicy>
inline HRESULT _AllocArray(_In_opt_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    *ppv = NULL;
    size_t cb;
    HRESULT hr = SizeTMult(cItems, sizeof(T), &cb);
    if (SUCCEEDED(hr))
    {
        hr = TAllocPolicy::Alloc(hHeap, dwFlags, cb, (void**)ppv);
    }
    return hr;
}

template <class T, class TAllocPolicy>
inline HRESULT _ReallocArray(_In_opt_ HANDLE hHeap, _In_ DWORD dwFlags, _In_opt_ T *pv, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    *ppv = NULL;
    size_t cb;
    HRESULT hr = SizeTMult(cItems, sizeof(T), &cb);
    if (SUCCEEDED(hr))
    {
        hr = TAllocPolicy::Realloc(hHeap, dwFlags, pv, cb, (void**)ppv);
    }
    return hr;
}

// Templates for isolating string-specific functionality

template <class TAllocPolicy>
inline HRESULT _AllocStringWorker(_In_opt_ HANDLE hHeap, _In_ DWORD dwFlags, _In_reads_opt_(cch) PCNZWCH pszSource, _In_ size_t cch, _In_ size_t cchExtra, _Outptr_result_buffer_(cch+cchExtra) _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    *ppsz = NULL;
    size_t cchTotal;
    HRESULT hr = SizeTAdd(cch, cchExtra, &cchTotal);
    if (SUCCEEDED(hr))
    {
        // Note that we do not require dwFlags to include the allocator-specific
        // zero-initialization flag here.
        hr = _AllocArray<WCHAR,TAllocPolicy>(hHeap, dwFlags, cchTotal, ppsz);
        if (SUCCEEDED(hr))
        {
            // The source string may be shorter than cch, so zero-initialize
            // the entire buffer using STRSAFE_FILL_BEHIND_NULL.
            //
            // Note that _AllocStringDoubleNullTerminate relies on
            // zero-initialization to provide the 2nd NULL terminator.
            StringCchCopyNExW(*ppsz, cchTotal, pszSource, cch, NULL, NULL, STRSAFE_IGNORE_NULLS | STRSAFE_FILL_BEHIND_NULL);
        }
    }
    return hr;
}

template <class TAllocPolicy>
inline HRESULT _AllocString(_In_opt_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ PCWSTR pszSource, _Outptr_result_nullonfailure_ PWSTR *ppsz)
{
    // pszSource must be valid (non-NULL)
    return _AllocStringWorker<TAllocPolicy>(hHeap, dwFlags, pszSource, wcslen(pszSource), 1, ppsz);
}

template <class TAllocPolicy>
inline HRESULT _AllocStringLen(_In_opt_ HANDLE hHeap, _In_ DWORD dwFlags, _In_reads_or_z_opt_(cch) PCNZWCH pszSource, _In_ size_t cch, _Outptr_result_buffer_(cch+1) _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    // pszSource is optional (may be NULL)
    return _AllocStringWorker<TAllocPolicy>(hHeap, dwFlags, pszSource, cch, 1, ppsz);
}

//  Takes a single-null terminated string and allocates a double-null terminated string.
template <class TAllocPolicy>
inline HRESULT _AllocStringDoubleNullTerminate(_In_opt_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ PCWSTR pszSource, _Outptr_result_nullonfailure_ PZZWSTR *ppsz)
{
    // pszSource must be valid (non-NULL)
    return _AllocStringWorker<TAllocPolicy>(hHeap, dwFlags, pszSource, wcslen(pszSource), 2, ppsz);
}

template <class TAllocPolicy>
inline HRESULT _AllocStringOpt(_In_opt_ HANDLE hHeap, _In_ DWORD dwFlags, _In_opt_ PCWSTR pszSource, _Outptr_result_maybenull_ _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    // pszSource is optional (may be NULL)
    if (pszSource != NULL)
    {
        return _AllocString<TAllocPolicy>(hHeap, dwFlags, pszSource, ppsz);
    }
    *ppsz = NULL;
    return S_OK;
}

#ifndef NO_COALLOC_HELPERS

#include <objbase.h>

// CoTaskMemAlloc does not zero-initialize by default. Define a flag to enable
// zero-init behavior.
#define CO_MEM_ZERO_INIT    0x00000001

class CTCoAllocPolicy
{
private:
#if (NTDDI_VERSION < NTDDI_WIN10_RS1) || defined(COM_SUPPORT_MALLOC_SPIES)
    static size_t _CoTaskMemSize(_In_ _Post_writable_byte_size_(return) void *pv)
    {
        size_t cb = 0;
        IMalloc *pMalloc;
        if (SUCCEEDED(CoGetMalloc(1, &pMalloc))) // should never fail (static v-table)
        {
            // Returns (size_t)-1 if pv is NULL.
            // Result is indeterminate if pv does not belong to CoTaskMemAlloc.
            cb = pMalloc->GetSize(pv);
            pMalloc->Release();
        }
        return cb;
    }
#endif

public:
    static HRESULT Alloc(_In_opt_ HANDLE /*hHeap*/, _In_ DWORD dwFlags, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) void **ppv)
    {
        *ppv = CoTaskMemAlloc(cb);
        if (*ppv)
        {
            if (dwFlags & CO_MEM_ZERO_INIT)
            {
#ifdef COM_SUPPORT_MALLOC_SPIES
                // Zero-initialize the buffer
                // The actual size might be larger than cb due to spies present.
                // Initialize to the actual size in case of realloc later,
                // or there might be an uninitialized gap in between.
                size_t cbActual = _CoTaskMemSize(*ppv);
                ZeroMemory(*ppv, cbActual);
#else
                ZeroMemory(*ppv, cb);
#endif
            }
            return S_OK;
        }
        return E_OUTOFMEMORY;
    }

    static HRESULT Realloc(_In_opt_ HANDLE /*hHeap*/, _In_ DWORD dwFlags, _In_opt_ void *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) void **ppv)
    {
#if (NTDDI_VERSION < NTDDI_WIN10_RS1)
        size_t cbKeepIntact = 0;

        if (pv && (dwFlags & CO_MEM_ZERO_INIT))
        {
            // Get the current size, so we know how much to zero-initialize
            cbKeepIntact = _CoTaskMemSize(pv);
            if (cb < cbKeepIntact)
            {
                // Shrinking the buffer, only keep the new size
                cbKeepIntact = cb;
            }
        }
#else
        // As of Redstone CoTaskMemRealloc always zero-initializes 
        // the tail of the allocation.
        size_t cbKeepIntact = cb;
#endif

        // If pv is NULL, CoTaskMemRealloc allocates a new block
        *ppv = CoTaskMemRealloc(pv, cb);

        if (*ppv)
        {
            if (dwFlags & CO_MEM_ZERO_INIT)
            {
                // Zero-initialize the trailing part of the buffer
#ifdef COM_SUPPORT_MALLOC_SPIES
                // The actual size might be larger than cb due to due to spies present.
                size_t cbActual = _CoTaskMemSize(*ppv);
#else
                size_t cbActual = cb;
#endif
                if (cbActual > cbKeepIntact)
                {
                    ZeroMemory(((BYTE*)*ppv) + cbKeepIntact, cbActual - cbKeepIntact);
                }
            }
            return S_OK;
        }
        return E_OUTOFMEMORY;
    }
};

// CoTaskMemAlloc helpers

template <class T>
 inline HRESULT CoAllocBytes(_In_ DWORD dwFlags, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _AllocBytes<T, CTCoAllocPolicy>(NULL, dwFlags, cb, ppv);
}

template <class T>
inline HRESULT CoReallocBytes(_In_ DWORD dwFlags, _In_opt_ T *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _ReallocBytes<T, CTCoAllocPolicy>(NULL, dwFlags, pv, cb, ppv);
}

template <class T>
inline HRESULT CoAllocObject(_In_ DWORD dwFlags, _Outptr_result_buffer_(1) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _AllocBytes<T, CTCoAllocPolicy>(NULL, dwFlags, sizeof(T), ppv);
}

template <class T>
inline HRESULT CoAllocArray(_In_ DWORD dwFlags, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _AllocArray<T, CTCoAllocPolicy>(NULL, dwFlags, cItems, ppv);
}

template <class T>
inline HRESULT CoReallocArray(_In_ DWORD dwFlags, _In_opt_ T *pv, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _ReallocArray<T, CTCoAllocPolicy>(NULL, dwFlags, pv, cItems, ppv);
}

// Zero-initializing CoTaskMemAlloc helpers

template <class T>
 inline HRESULT CoAllocBytes(_In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return CoAllocBytes(CO_MEM_ZERO_INIT, cb, ppv);
}

template <class T>
inline HRESULT CoReallocBytes(_In_opt_ T *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return CoReallocBytes(CO_MEM_ZERO_INIT, pv, cb, ppv);
}

template <class T>
inline HRESULT CoAllocObject(_Outptr_result_buffer_(1) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return CoAllocObject(CO_MEM_ZERO_INIT, ppv);
}

template <class T>
inline HRESULT CoAllocArray(_In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return CoAllocArray(CO_MEM_ZERO_INIT, cItems, ppv);
}

template <class T>
inline HRESULT CoReallocArray(_In_opt_ T *pv, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return CoReallocArray(CO_MEM_ZERO_INIT, pv, cItems, ppv);
}

// CoTaskMemAlloc string helpers

inline HRESULT CoAllocString(_In_ PCWSTR pszSource, _Outptr_result_nullonfailure_ PWSTR *ppsz)
{
    return _AllocString<CTCoAllocPolicy>(NULL, NO_ZERO_INIT, pszSource, ppsz);
}

inline HRESULT CoAllocStringLen( _In_reads_or_z_opt_(cch) PCNZWCH pszSource, _In_ size_t cch, _Outptr_result_buffer_(cch+1) _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    return _AllocStringLen<CTCoAllocPolicy>(NULL, NO_ZERO_INIT, pszSource, cch, ppsz);
}

inline HRESULT CoAllocStringDoubleNullTerminate(_In_ PCWSTR pszSource, _Outptr_result_nullonfailure_ PZZWSTR *ppsz)
{
    return _AllocStringDoubleNullTerminate<CTCoAllocPolicy>(NULL, NO_ZERO_INIT, pszSource, ppsz);
}

inline HRESULT CoAllocStringOpt(_In_opt_ PCWSTR pszSource, _Outptr_result_maybenull_ _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    return _AllocStringOpt<CTCoAllocPolicy>(NULL, NO_ZERO_INIT, pszSource, ppsz);
}

#endif  // NO_COALLOC_HELPERS

#ifndef NO_LOCALALLOC_HELPERS

class CTLocalAllocPolicy
{
public:
    static HRESULT Alloc(_In_opt_ HANDLE /*hHeap*/, _In_ DWORD dwFlags, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) void **ppv)
    {
        // ignore flags other than zero-init, assume fixed
        *ppv = LocalAlloc(LMEM_FIXED | (dwFlags & LMEM_ZEROINIT), cb);
        return (*ppv) ? S_OK : E_OUTOFMEMORY;
    }

    static HRESULT Realloc(_In_opt_ HANDLE hHeap, _In_ DWORD dwFlags, _In_opt_ void *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) void **ppv)
    {
        if (pv == NULL)
        {
            return Alloc(hHeap, dwFlags, cb, ppv);
        }

        // LMEM_MOVEABLE is correct when reallocating LMEM_FIXED buffers
        *ppv = LocalReAlloc(pv, cb, LMEM_MOVEABLE | (dwFlags & LMEM_ZEROINIT));
        return (*ppv) ? S_OK : E_OUTOFMEMORY;
    }
};

// LocalAlloc helpers

template <class T>
 inline HRESULT LocalAllocBytes(_In_ DWORD dwFlags, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _AllocBytes<T, CTLocalAllocPolicy>(NULL, dwFlags, cb, ppv);
}

template <class T>
inline HRESULT LocalReallocBytes(_In_ DWORD dwFlags, _In_opt_ T *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _ReallocBytes<T, CTLocalAllocPolicy>(NULL, dwFlags, pv, cb, ppv);
}

template <class T>
inline HRESULT LocalAllocObject(_In_ DWORD dwFlags, _Outptr_result_buffer_(1) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _AllocBytes<T, CTLocalAllocPolicy>(NULL, dwFlags, sizeof(T), ppv);
}

template <class T>
inline HRESULT LocalAllocArray(_In_ DWORD dwFlags, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _AllocArray<T, CTLocalAllocPolicy>(NULL, dwFlags, cItems, ppv);
}

template <class T>
inline HRESULT LocalReallocArray(_In_ DWORD dwFlags, _In_opt_ T *pv, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _ReallocArray<T, CTLocalAllocPolicy>(NULL, dwFlags, pv, cItems, ppv);
}

// Zero-initializing LocalAlloc helpers

template <class T>
 inline HRESULT LocalAllocBytes(_In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return LocalAllocBytes(LMEM_ZEROINIT, cb, ppv);
}

template <class T>
inline HRESULT LocalReallocBytes(_In_opt_ T *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return LocalReallocBytes(LMEM_ZEROINIT, pv, cb, ppv);
}

template <class T>
inline HRESULT LocalAllocObject(_Outptr_result_buffer_(1) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return LocalAllocObject(LMEM_ZEROINIT, ppv);
}

template <class T>
inline HRESULT LocalAllocArray(_In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return LocalAllocArray(LMEM_ZEROINIT, cItems, ppv);
}

template <class T>
inline HRESULT LocalReallocArray(_In_opt_ T *pv, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return LocalReallocArray(LMEM_ZEROINIT, pv, cItems, ppv);
}

// LocalAlloc string helpers

inline HRESULT LocalAllocString(_In_ PCWSTR pszSource, _Outptr_result_nullonfailure_ PWSTR *ppsz)
{
    return _AllocString<CTLocalAllocPolicy>(NULL, NO_ZERO_INIT, pszSource, ppsz);
}

inline HRESULT LocalAllocStringLen( _In_reads_or_z_opt_(cch) PCNZWCH pszSource, _In_ size_t cch, _Outptr_result_buffer_(cch+1) _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    return _AllocStringLen<CTLocalAllocPolicy>(NULL, NO_ZERO_INIT, pszSource, cch, ppsz);
}

inline HRESULT LocalAllocStringDoubleNullTerminate(_In_ PCWSTR pszSource, _Outptr_result_nullonfailure_ PZZWSTR *ppsz) //todo sal 00?
{
    return _AllocStringDoubleNullTerminate<CTLocalAllocPolicy>(NULL, NO_ZERO_INIT, pszSource, ppsz);
}

inline HRESULT LocalAllocStringOpt(_In_opt_ PCWSTR pszSource, _Outptr_result_maybenull_ _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    return _AllocStringOpt<CTLocalAllocPolicy>(NULL, NO_ZERO_INIT, pszSource, ppsz);
}

#endif  // NO_LOCALALLOC_HELPERS

#ifndef NO_HEAPALLOC_HELPERS

class CTHeapAllocPolicy
{
public:
    static HRESULT Alloc(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) void **ppv)
    {
        *ppv = HeapAlloc(hHeap, dwFlags, cb);
        return (*ppv) ? S_OK : E_OUTOFMEMORY;
    }

    static HRESULT Realloc(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_opt_ void *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) void **ppv)
    {
        if (pv == NULL)
        {
            return Alloc(hHeap, dwFlags, cb, ppv);
        }
        *ppv = HeapReAlloc(hHeap, dwFlags, pv, cb);
        return (*ppv) ? S_OK : E_OUTOFMEMORY;
    }
};

// HeapAlloc helpers

template <class T>
 inline HRESULT HeapAllocBytes(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _AllocBytes<T, CTHeapAllocPolicy>(hHeap, dwFlags, cb, ppv);
}

template <class T>
inline HRESULT HeapReallocBytes(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_opt_ T *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _ReallocBytes<T, CTHeapAllocPolicy>(hHeap, dwFlags, pv, cb, ppv);
}

template <class T>
inline HRESULT HeapAllocObject(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _Outptr_result_buffer_(1) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _AllocBytes<T, CTHeapAllocPolicy>(hHeap, dwFlags, sizeof(T), ppv);
}

template <class T>
inline HRESULT HeapAllocArray(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _AllocArray<T, CTHeapAllocPolicy>(hHeap, dwFlags, cItems, ppv);
}

template <class T>
inline HRESULT HeapReallocArray(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_opt_ T *pv, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _ReallocArray<T, CTHeapAllocPolicy>(hHeap, dwFlags, pv, cItems, ppv);
}

// Zero-initializing HeapAlloc helpers (process heap)

template <class T>
 inline HRESULT HeapAllocBytes(_In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return HeapAllocBytes(GetProcessHeap(), HEAP_ZERO_MEMORY, cb, ppv);
}

template <class T>
inline HRESULT HeapReallocBytes(_In_opt_ T *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return HeapReallocBytes(GetProcessHeap(), HEAP_ZERO_MEMORY, pv, cb, ppv);
}

template <class T>
inline HRESULT HeapAllocObject(_Outptr_result_buffer_(1) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return HeapAllocObject(GetProcessHeap(), HEAP_ZERO_MEMORY, ppv);
}

template <class T>
inline HRESULT HeapAllocArray(_In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return HeapAllocArray(GetProcessHeap(), HEAP_ZERO_MEMORY, cItems, ppv);
}

template <class T>
inline HRESULT HeapReallocArray(_In_opt_ T *pv, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return HeapReallocArray(GetProcessHeap(), HEAP_ZERO_MEMORY, pv, cItems, ppv);
}

// HeapAlloc string helpers

inline HRESULT HeapAllocString(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ PCWSTR pszSource, _Outptr_result_nullonfailure_ PWSTR *ppsz)
{
    return _AllocString<CTHeapAllocPolicy>(hHeap, dwFlags, pszSource, ppsz);
}

inline HRESULT HeapAllocStringLen(_In_ HANDLE hHeap, _In_ DWORD dwFlags,  _In_reads_or_z_opt_(cch) PCNZWCH pszSource, _In_ size_t cch, _Outptr_result_buffer_(cch+1) _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    return _AllocStringLen<CTHeapAllocPolicy>(hHeap, dwFlags, pszSource, cch, ppsz);
}

inline HRESULT HeapAllocStringDoubleNullTerminate(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ PCWSTR pszSource, _Outptr_result_nullonfailure_ PZZWSTR *ppsz)
{
    return _AllocStringDoubleNullTerminate<CTHeapAllocPolicy>(hHeap, dwFlags, pszSource, ppsz);
}

inline HRESULT HeapAllocStringOpt(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_opt_ PCWSTR pszSource, _Outptr_result_maybenull_ _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    return _AllocStringOpt<CTHeapAllocPolicy>(hHeap, dwFlags, pszSource, ppsz);
}

// HeapAlloc string helpers (process heap)

inline HRESULT HeapAllocString(_In_ PCWSTR pszSource, _Outptr_result_nullonfailure_ PWSTR *ppsz)
{
    return HeapAllocString(GetProcessHeap(), NO_ZERO_INIT, pszSource, ppsz);
}

inline HRESULT HeapAllocStringLen(_In_reads_opt_(cch) PCNZWCH pszSource, _In_ size_t cch, _Outptr_result_buffer_(cch+1) _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    return HeapAllocStringLen(GetProcessHeap(), NO_ZERO_INIT, pszSource, cch, ppsz);
}

inline HRESULT HeapAllocStringDoubleNullTerminate(_In_ PCWSTR pszSource, _Outptr_result_nullonfailure_ PZZWSTR *ppsz)
{
    return HeapAllocStringDoubleNullTerminate(GetProcessHeap(), NO_ZERO_INIT, pszSource, ppsz);
}

inline HRESULT HeapAllocStringOpt(_In_opt_ PCWSTR pszSource, _Outptr_result_maybenull_ _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    return HeapAllocStringOpt(GetProcessHeap(), NO_ZERO_INIT, pszSource, ppsz);
}

#endif  // NO_HEAPALLOC_HELPERS

#ifndef NO_GLOBALALLOC_HELPERS

class CTGlobalAllocPolicy
{
public:
    static HRESULT Alloc(_In_opt_ HANDLE /*hHeap*/, _In_ DWORD dwFlags, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) void **ppv)
    {
        // ignore flags other than zero-init, assume fixed
        *ppv = GlobalAlloc(GMEM_FIXED | (dwFlags & GMEM_ZEROINIT), cb);
        return (*ppv) ? S_OK : E_OUTOFMEMORY;
    }

    static HRESULT Realloc(_In_opt_ HANDLE hHeap, _In_ DWORD dwFlags, _In_opt_ void *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) void **ppv)
    {
        if (pv == NULL)
        {
            return Alloc(hHeap, dwFlags, cb, ppv);
        }

        // GMEM_MOVEABLE is correct when reallocating GMEM_FIXED buffers
        *ppv = GlobalReAlloc(pv, cb, GMEM_MOVEABLE | (dwFlags & GMEM_ZEROINIT));
        return (*ppv) ? S_OK : E_OUTOFMEMORY;
    }
};

// GlobalAlloc helpers

template <class T>
 inline HRESULT GlobalAllocBytes(_In_ DWORD dwFlags, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _AllocBytes<T, CTGlobalAllocPolicy>(NULL, dwFlags, cb, ppv);
}

template <class T>
inline HRESULT GlobalReallocBytes(_In_ DWORD dwFlags, _In_opt_ T *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _ReallocBytes<T, CTGlobalAllocPolicy>(NULL, dwFlags, pv, cb, ppv);
}

template <class T>
inline HRESULT GlobalAllocObject(_In_ DWORD dwFlags, _Outptr_result_buffer_(1) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _AllocBytes<T, CTGlobalAllocPolicy>(NULL, dwFlags, sizeof(T), ppv);
}

template <class T>
inline HRESULT GlobalAllocArray(_In_ DWORD dwFlags, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _AllocArray<T, CTGlobalAllocPolicy>(NULL, dwFlags, cItems, ppv);
}

template <class T>
inline HRESULT GlobalReallocArray(_In_ DWORD dwFlags, _In_opt_ T *pv, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return _ReallocArray<T, CTGlobalAllocPolicy>(NULL, dwFlags, pv, cItems, ppv);
}

// Zero-initializing GlobalAlloc helpers

template <class T>
 inline HRESULT GlobalAllocBytes(_In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return GlobalAllocBytes<T>(GMEM_ZEROINIT, cb, ppv);
}

template <class T>
inline HRESULT GlobalReallocBytes(_In_opt_ T *pv, _In_ size_t cb, _Outptr_result_bytebuffer_(cb) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return GlobalReallocBytes<T>(GMEM_ZEROINIT, pv, cb, ppv);
}

template <class T>
inline HRESULT GlobalAllocObject(_Outptr_result_buffer_(1) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return GlobalAllocObject<T>(GMEM_ZEROINIT, ppv);
}

template <class T>
inline HRESULT GlobalAllocArray(_In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return GlobalAllocArray<T>(GMEM_ZEROINIT, cItems, ppv);
}

template <class T>
inline HRESULT GlobalReallocArray(_In_opt_ T *pv, _In_ size_t cItems, _Outptr_result_buffer_(cItems) _On_failure_(_Post_satisfies_(*ppv == 0)) T **ppv)
{
    return GlobalReallocArray<T>(GMEM_ZEROINIT, pv, cItems, ppv);
}

// GlobalAlloc string helpers

inline HRESULT GlobalAllocString(_In_ PCWSTR pszSource, _Outptr_result_nullonfailure_ PWSTR *ppsz)
{
    return _AllocString<CTGlobalAllocPolicy>(NULL, NO_ZERO_INIT, pszSource, ppsz);
}

inline HRESULT GlobalAllocStringLen( _In_reads_or_z_opt_(cch) PCNZWCH pszSource, _In_ size_t cch, _Outptr_result_buffer_(cch+1) _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    return _AllocStringLen<CTGlobalAllocPolicy>(NULL, NO_ZERO_INIT, pszSource, cch, ppsz);
}

inline HRESULT GlobalAllocStringDoubleNullTerminate(_In_ PCWSTR pszSource, _Outptr_result_nullonfailure_ PZZWSTR *ppsz)
{
    return _AllocStringDoubleNullTerminate<CTGlobalAllocPolicy>(NULL, NO_ZERO_INIT, pszSource, ppsz);
}

inline HRESULT GlobalAllocStringOpt(_In_opt_ PCWSTR pszSource, _Outptr_result_maybenull_ _On_failure_(_Post_satisfies_(*ppsz == 0)) PWSTR *ppsz)
{
    return _AllocStringOpt<CTGlobalAllocPolicy>(NULL, NO_ZERO_INIT, pszSource, ppsz);
}

#endif  // NO_GLOBALALLOC_HELPERS

#endif  // __cplusplus

#endif  // __memsafe_h__


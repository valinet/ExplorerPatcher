#pragma once

#include <intsafe.h>
#include <strsafe.h>

#include "ResultUtils.h"

namespace Windows::Internal
{
    class ResourceString
    {
    public:
        static bool FindAndSize(HINSTANCE hInstance, UINT uId, WORD wLanguage, const WCHAR** ppch, WORD* plen)
        {
            bool fRet = false;
            *ppch = nullptr;
            if (plen)
                *plen = 0;
            HRSRC hRes = FindResourceExW(hInstance, RT_STRING, MAKEINTRESOURCEW((uId >> 4) + 1), wLanguage);
            if (hRes)
            {
                HGLOBAL hStringSeg = LoadResource(hInstance, hRes);
                if (hStringSeg)
                {
                    WCHAR* pch = (WCHAR*)LockResource(hStringSeg);
                    if (pch)
                    {
                        for (uId = (char)uId & 0xF; uId; --uId)
                            pch += *pch + 1;
                        *ppch = *pch ? pch + 1 : L"";
                        if (plen)
                            *plen = *pch;
                        fRet = true;
                    }
                }
            }
            return fRet;
        }
    };

    template <typename ElementType>
    class CoTaskMemPolicy
    {
    public:
        static ElementType* Alloc(size_t bytes)
        {
            return (ElementType*)CoTaskMemAlloc(bytes);
        }

        static ElementType* Realloc(ElementType* p, size_t bytes)
        {
            return (ElementType*)CoTaskMemRealloc(p, bytes);
        }

        static void Free(ElementType* p)
        {
            CoTaskMemFree(p);
        }
    };

    template <typename ElementType>
    class LocalMemPolicy
    {
    public:
        static ElementType* Alloc(size_t bytes)
        {
            return (ElementType*)LocalAlloc(LMEM_FIXED, bytes);
        }

        static ElementType* Realloc(ElementType* p, size_t bytes)
        {
            return (ElementType*)LocalReAlloc(p, bytes, LMEM_MOVEABLE);
        }

        static void Free(ElementType* p)
        {
            LocalFree(p);
        }
    };

    template <typename Allocator>
    class NativeString
    {
    public:
        NativeString() : _pszStringData(nullptr), _cchStringData(0), _cchStringDataCapacity(0)
        {
        }

        NativeString(NativeString&& other) noexcept
            : _pszStringData(other._pszStringData)
            , _cchStringData(other._cchStringData)
            , _cchStringDataCapacity(other._cchStringDataCapacity)
        {
            other._pszStringData = nullptr;
            other._cchStringData = 0;
            other._cchStringDataCapacity = 0;
        }

    private:
        NativeString(const NativeString&) = delete;

    public:
        ~NativeString()
        {
            Free();
        }

        HRESULT Initialize(const WCHAR* psz, const size_t cch)
        {
            return _Initialize(psz, cch);
        }

        HRESULT Initialize(const WCHAR* psz)
        {
            return _Initialize(psz, s_cchUnknown);
        }

        HRESULT Initialize(const NativeString& other)
        {
            return _Initialize(other._pszStringData, other.GetCount());
        }

        HRESULT Initialize(HINSTANCE hInstance, UINT uId, WORD wLanguage)
        {
            HRESULT hr;
            const WCHAR* rgch;
            WORD cch;
            if (ResourceString::FindAndSize(hInstance, uId, wLanguage, &rgch, &cch))
            {
                hr = _Initialize(rgch, cch);
            }
            else
            {
                hr = E_FAIL;
            }
            return hr;
        }

        HRESULT Initialize(HINSTANCE hInstance, UINT uId)
        {
            return Initialize(hInstance, uId, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
        }

        HRESULT Initialize(HKEY hKey, const WCHAR* pszValueName)
        {
            return _InitializeFromRegistry(hKey, pszValueName, true);
        }

        HRESULT Initialize(HKEY hKey, const WCHAR* pszSubKey, const WCHAR* pszValueName)
        {
            HKEY hkeySub;
            HRESULT hr = HRESULT_FROM_WIN32(RegOpenKeyExW(hKey, pszSubKey, 0, KEY_READ, &hkeySub));
            if (SUCCEEDED(hr))
            {
                hr = Initialize(hkeySub, pszValueName);
                RegCloseKey(hkeySub);
            }
            return hr;
        }

        HRESULT InitializeNoExpand(HKEY hKey, const WCHAR* pszValueName)
        {
            return _InitializeFromRegistry(hKey, pszValueName, false);
        }

        HRESULT InitializeNoExpand(HKEY hKey, const WCHAR* pszSubKey, const WCHAR* pszValueName)
        {
            HKEY hkeySub;
            HRESULT hr = HRESULT_FROM_WIN32(RegOpenKeyExW(hKey, pszSubKey, 0, KEY_READ, &hkeySub));
            if (SUCCEEDED(hr))
            {
                hr = InitializeNoExpand(hkeySub, pszValueName);
                RegCloseKey(hkeySub);
            }
            return hr;
        }

        HRESULT InitializeFormat(const WCHAR* pszFormat, va_list argList)
        {
            return _InitializeHelper(pszFormat, argList, [](const WCHAR* pszFormat, va_list argList, WCHAR* pszStringData, size_t cchStringData) -> HRESULT
            {
                _set_errno(0);
                HRESULT hr = StringCchVPrintfW(pszStringData, cchStringData, pszFormat, argList);
                if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
                {
                    errno_t err;
                    _get_errno(&err);
                    if (err == EINVAL)
                    {
                        hr = E_INVALIDARG;
                    }
                }
                return hr;
            });
        }

        HRESULT InitializeFormat(const WCHAR* pszFormat, ...)
        {
            va_list args;
            va_start(args, pszFormat);
            return InitializeFormat(pszFormat, args);
        }

        HRESULT InitializeResFormat(HINSTANCE hInstance, UINT uId, ...)
        {
            va_list argList;
            va_start(argList, uId);
            NativeString spszFormat;
            HRESULT hr = spszFormat.Initialize(hInstance, uId);
            if (SUCCEEDED(hr))
            {
                hr = InitializeFormat(spszFormat._pszStringData, argList);
            }
            return hr;
        }

        HRESULT InitializeResMessage(HINSTANCE hInstance, UINT uId, ...)
        {
            va_list argList;
            va_start(argList, uId);
            NativeString spszFormat;
            HRESULT hr = spszFormat.Initialize(hInstance, uId);
            if (SUCCEEDED(hr))
            {
                hr = _InitializeHelper(spszFormat._pszStringData, argList, [](const WCHAR* pszFormat, va_list argList, WCHAR* pszStringData, size_t cchStringData) -> HRESULT
                {
                    va_list argListT = argList;
                    DWORD cchResult = FormatMessageW(FORMAT_MESSAGE_FROM_STRING, pszFormat, 0, 0, pszStringData, (DWORD)cchStringData, &argListT);
                    return ResultFromWin32Bool(cchResult);
                });
            }
            return hr;
        }

        void Free()
        {
            _Free();
        }

        void Attach(WCHAR* psz)
        {
            _Attach(psz);
        }

        void Attach(WCHAR* psz, const size_t cch)
        {
            _Attach(psz, cch);
        }

        WCHAR* Detach()
        {
            return _Detach();
        }

        HRESULT DetachInitializeIfEmpty(WCHAR** ppsz)
        {
            *ppsz = nullptr;
            HRESULT hr = S_OK;

            if (_pszStringData)
            {
                hr = Initialize(L"");
            }

            if (SUCCEEDED(hr))
            {
                *ppsz = Detach();
            }

            return hr;
        }

        WCHAR** FreeAndGetAddressOf()
        {
            return _FreeAndGetAddressOf();
        }

        HRESULT CopyTo(WCHAR** ppszDest) const
        {
            HRESULT hr;
            *ppszDest = nullptr;
            if (_pszStringData)
            {
                NativeString spszT;
                hr = spszT.Initialize(*this);
                if (SUCCEEDED(hr))
                {
                    *ppszDest = spszT.Detach();
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            }
            return hr;
        }

        HRESULT CopyTo(WCHAR* pszDest, size_t cchDest) const
        {
            if (!_pszStringData)
            {
                if (cchDest)
                    *pszDest = 0;
                return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            }
            return StringCchCopyW(pszDest, cchDest, _pszStringData);
        }

        const WCHAR* Get() const
        {
            return _Get();
        }

        const WCHAR* GetNonNull() const
        {
            return _pszStringData ? _pszStringData : L"";
        }

        size_t GetCount()
        {
            return _GetCount();
        }

        size_t GetCount() const
        {
            return _GetCount();
        }

        bool IsEmpty() const
        {
            return _IsEmpty();
        }

        bool HasLength() const
        {
            return !_IsEmpty();
        }

        int CompareOrdinal(const WCHAR* psz, const size_t cch) const
        {
            return CompareStringOrdinal(GetNonNull(), (int)GetCount(), psz ? psz : L"", psz ? (int)cch : 0, FALSE);
        }

        int CompareOrdinal(const WCHAR* psz) const
        {
            return CompareOrdinal(psz, s_cchUnknown);
        }

        int CompareOrdinal(const NativeString& other) const
        {
            return CompareOrdinal(other.GetNonNull(), other.GetCount());
        }

        int CompareOrdinalIgnoreCase(const WCHAR* psz, const size_t cch) const
        {
            return CompareStringOrdinal(GetNonNull(), (int)GetCount(), psz ? psz : L"", psz ? (int)cch : 0, TRUE);
        }

        int CompareOrdinalIgnoreCase(const WCHAR* psz) const
        {
            return CompareOrdinalIgnoreCase(psz, s_cchUnknown);
        }

        int CompareOrdinalIgnoreCase(const NativeString& other) const
        {
            return CompareOrdinalIgnoreCase(other.GetNonNull(), other.GetCount());
        }

        HRESULT Concat(const WCHAR* psz, const size_t cch)
        {
            return _Concat(psz, cch);
        }

        HRESULT Concat(WCHAR c)
        {
            return _Concat(c);
        }

        HRESULT Concat(const WCHAR* psz)
        {
            return _Concat(psz, psz ? wcslen(psz) : 0);
        }

        HRESULT Concat(const NativeString& other)
        {
            return _Concat(other.Get(), other.GetCount());
        }

        HRESULT Concat(HINSTANCE hInstance, UINT uId, WORD wLanguage)
        {
            HRESULT hr;
            const WCHAR* rgch;
            WORD cch;
            if (ResourceString::FindAndSize(hInstance, uId, wLanguage, &rgch, &cch))
            {
                hr = _Concat(rgch, cch);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            }
            return hr;
        }

        HRESULT Concat(HINSTANCE hInstance, UINT uId)
        {
            return Concat(hInstance, uId, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
        }

        HRESULT ConcatFormat(const WCHAR* pszFormat, va_list argList)
        {
            if (IsEmpty())
            {
                return InitializeFormat(pszFormat, argList);
            }
            NativeString strT;
            HRESULT hr = strT.InitializeFormat(pszFormat, argList);
            if (SUCCEEDED(hr))
            {
                hr = Concat(strT);
            }
            return hr;
        }

        HRESULT ConcatFormat(const WCHAR* pszFormat, ...)
        {
            va_list argList;
            va_start(argList, pszFormat);
            return ConcatFormat(pszFormat, argList);
        }

        bool RemoveAt(size_t iElem, size_t cchElem)
        {
            return _RemoveAt(iElem, cchElem);
        }

        bool TrimStart(const WCHAR* pszTrim)
        {
            return _TrimStart(pszTrim);
        }

        bool TrimEnd(const WCHAR* pszTrim)
        {
            return _TrimEnd(pszTrim);
        }

        inline static const WCHAR* const s_pszTrimWhitespaceCharacterSet =
            L"\u0020" // Space
            L"\u0009" // Tab
            L"\u3000" // Ideographic Space
            L"\u17D2" // Khmer Sign Coeng
            L"\u0F0B" // Tibetan Mark Intersyllabic Tsheg
            L"\u1680" // Ogham Space Mark
            L"\u180E" // Mongolian Vowel Separator
        ;

        bool TrimWhitespace()
        {
            bool fWasCharacterTrimmedEnd = _TrimEnd(s_pszTrimWhitespaceCharacterSet);
            bool fWasCharacterTrimmedStart = _TrimStart(s_pszTrimWhitespaceCharacterSet);
            return fWasCharacterTrimmedStart || fWasCharacterTrimmedEnd;
        }

        void ReplaceChars(const WCHAR wcFind, const WCHAR wcReplace)
        {
            _EnsureCount();
            for (size_t i = 0; i < _cchStringData; i++)
            {
                if (_pszStringData[i] == wcFind)
                    _pszStringData[i] = wcReplace;
            }
        }

        NativeString& operator=(NativeString&& other) noexcept
        {
            _Free();
            _pszStringData = other._pszStringData;
            _cchStringData = other._cchStringData;
            _cchStringDataCapacity = other._cchStringDataCapacity;
            other._pszStringData = nullptr;
            other._cchStringData = 0;
            other._cchStringDataCapacity = 0;
            return *this;
        }

    private:
        NativeString& operator=(const NativeString& other) = delete;

    public:
        WCHAR** operator&()
        {
            return FreeAndGetAddressOf();
        }

        /*WCHAR* operator*() const
        {
            return Get();
        }*/

        bool operator==(const WCHAR* pszOther) const
        {
            return pszOther ? CompareOrdinal(pszOther) == CSTR_EQUAL : !_pszStringData;
        }

        bool operator!=(const WCHAR* pszOther) const
        {
            return !operator==(pszOther);
        }

        HRESULT AppendMayTruncate(const WCHAR* psz, size_t cchMaxCapacity)
        {
            return _ConcatMayTruncate(psz, cchMaxCapacity);
        }

        HRESULT EnsureCapacity(size_t cchDesired)
        {
            return _EnsureCapacity(cchDesired);
        }

    private:
        void _EnsureCount()
        {
            if (_cchStringData == s_cchUnknown)
            {
                _cchStringData = _pszStringData ? wcslen(_pszStringData) : 0;
            }
        }

        HRESULT _EnsureCapacity(size_t cchDesired)
        {
            size_t cchCapacityCur;
            HRESULT hr = SizeTAdd(cchDesired, 1, &cchCapacityCur);
            if (SUCCEEDED(hr))
            {
                if (_cchStringDataCapacity == s_cchUnknown)
                {
                    _EnsureCount();
                    _cchStringDataCapacity = _pszStringData ? _cchStringData + 1 : 0;
                }
                if (_cchStringDataCapacity == 0) // First allocation
                {
                    size_t cbDesired;
                    hr = SizeTMult(cchCapacityCur, sizeof(WCHAR), &cbDesired);
                    if (SUCCEEDED(hr))
                    {
                        WCHAR* pvArrayT = Allocator::Alloc(cbDesired);
                        hr = pvArrayT ? S_OK : E_OUTOFMEMORY;
                        if (SUCCEEDED(hr))
                        {
                            _cchStringDataCapacity = cchCapacityCur;
                            _pszStringData = pvArrayT;
                            pvArrayT[0] = 0;
                        }
                    }
                }
                else if (cchCapacityCur > _cchStringDataCapacity) // Growing
                {
                    size_t celemNew;
                    hr = SizeTMult(_cchStringDataCapacity, 2, &celemNew); // Double the capacity
                    if (SUCCEEDED(hr))
                    {
                        if (celemNew - _cchStringDataCapacity > 2048)
                            celemNew = _cchStringDataCapacity + 2048; // Make sure it doesn't grow too much; TODO Check disassembly
                        if (cchCapacityCur <= celemNew)
                            cchCapacityCur = celemNew;
                        WCHAR* pvArrayT = Allocator::Realloc(_pszStringData, sizeof(WCHAR) * cchCapacityCur);
                        hr = pvArrayT ? S_OK : E_OUTOFMEMORY;
                        if (SUCCEEDED(hr))
                        {
                            _cchStringDataCapacity = cchCapacityCur;
                            _pszStringData = pvArrayT;
                        }
                    }
                }
            }
            return hr;
        }

        bool _IsEmpty() const
        {
            return !_pszStringData || !_pszStringData[0];
        }

        HRESULT _Initialize(const WCHAR* psz, size_t cch)
        {
            size_t cchDesired = cch;
            size_t cchStringData;
            HRESULT hr = S_OK;
            if (psz)
            {
                if (cchDesired == s_cchUnknown)
                {
                    cchDesired = wcslen(psz);
                    cchStringData = cchDesired;
                }
                else
                {
                    cchStringData = _NativeString_Min<size_t>(cchDesired, wcslen(psz)); // @MOD Prevent double evaluation
                }
                hr = _EnsureCapacity(cchDesired);
                if (SUCCEEDED(hr))
                {
                    StringCchCopyNW(_pszStringData, cchDesired + 1, psz, cchStringData);
                    _cchStringData = cchStringData;
                }
            }
            else
            {
                _Free();
            }
            return hr;
        }

        template <typename T>
        HRESULT _InitializeHelper(const WCHAR* pszFormat, va_list argList, const T& callback)
        {
            HRESULT hr;
            size_t cchCapacityGuess = 32;
            do
            {
                hr = _EnsureCapacity(cchCapacityGuess);
                if (SUCCEEDED(hr))
                {
                    hr = callback(pszFormat, argList, _pszStringData, _cchStringDataCapacity);
                    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
                    {
                        size_t cchCapacityT;
                        hr = SizeTAdd(_cchStringDataCapacity, 32, &cchCapacityT);
                        if (SUCCEEDED(hr))
                        {
                            cchCapacityGuess = cchCapacityT;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            while (SUCCEEDED(hr));
            if (SUCCEEDED(hr))
            {
                _cchStringData = s_cchUnknown;
            }
            else
            {
                _Free();
            }
            return hr;
        }

        HRESULT _InitializeFromRegistry(HKEY hKey, const WCHAR* pszValueName, bool fExpand)
        {
            DWORD dwType;
            DWORD cbT = 0;
            LSTATUS lRes = RegQueryValueExW(hKey, pszValueName, nullptr, &dwType, nullptr, &cbT);
            HRESULT hr = HRESULT_FROM_WIN32(lRes);
            if (SUCCEEDED(hr) && ((dwType != REG_SZ && dwType != REG_EXPAND_SZ) || cbT == 0 || (cbT & 1) != 0))
            {
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }

            WCHAR* pszT = nullptr;
            if (SUCCEEDED(hr))
            {
                pszT = Allocator::Alloc(cbT);
                hr = pszT ? S_OK : E_OUTOFMEMORY;
            }

            if (SUCCEEDED(hr))
            {
                lRes = RegQueryValueExW(hKey, pszValueName, nullptr, &dwType, (LPBYTE)pszT, &cbT);
                hr = HRESULT_FROM_WIN32(lRes);
            }

            DWORD cchT = 0;
            if (SUCCEEDED(hr))
            {
                cchT = (cbT / sizeof(WCHAR)) - 1;
                if (dwType == REG_EXPAND_SZ && fExpand)
                {
                    DWORD cchBuffer = ExpandEnvironmentStringsW(pszT, nullptr, 0);
                    if (cchBuffer != 0)
                    {
                        WCHAR* pszExpand = Allocator::Alloc(sizeof(WCHAR) * cchBuffer);
                        hr = pszExpand ? S_OK : E_OUTOFMEMORY;
                        if (SUCCEEDED(hr))
                        {
                            DWORD cchResult = ExpandEnvironmentStringsW(pszT, pszExpand, cchBuffer);
                            hr = ResultFromWin32Count(cchResult, cchBuffer);
                            if (SUCCEEDED(hr))
                            {
                                Allocator::Free(pszT);
                                pszT = pszExpand;
                                cchT = cchResult - 1;
                            }
                            else
                            {
                                Allocator::Free(pszExpand);
                            }
                        }
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                if (!pszT[cchT])
                {
                    _Attach(pszT, cchT + 1);
                    pszT = nullptr;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }
            }

            Allocator::Free(pszT);
            return hr;
        }

        size_t _GetCount()
        {
            _EnsureCount();
            return _cchStringData;
        }

        size_t _GetCount() const
        {
            if (_cchStringData != s_cchUnknown)
                return _cchStringData;
            return _pszStringData ? wcslen(_pszStringData) : 0;
        }

        const WCHAR* _Get() const
        {
            return _pszStringData;
        }

        HRESULT _Concat(const WCHAR c)
        {
            WCHAR sz[2] = { c, 0 };
            return _Concat(sz, 1);
        }

        HRESULT _Concat(const WCHAR* psz, const size_t cch)
        {
            HRESULT hr = S_OK;
            if (psz)
            {
                _EnsureCount();
                hr = _EnsureCapacity(cch + _cchStringData);
                if (SUCCEEDED(hr))
                {
                    StringCchCopyNW(&_pszStringData[_cchStringData], cch + 1, psz, cch);
                    _cchStringData += cch;
                }
            }
            return hr;
        }

        HRESULT _ConcatMayTruncate(const WCHAR* psz, size_t cchMaxCapacity)
        {
            _EnsureCount();
            HRESULT hr = S_OK;
            if (cchMaxCapacity > _cchStringData)
            {
                size_t cchDesired = _NativeString_Min<size_t>(cchMaxCapacity - _cchStringData, wcslen(psz)); // @MOD Prevent double evaluation
                hr = _Concat(psz, cchDesired);
            }
            else if (cchMaxCapacity < _cchStringData)
            {
                _cchStringData = cchMaxCapacity;
                _pszStringData[cchMaxCapacity] = 0;
            }
            return hr;
        }

        bool _RemoveAt(size_t iElem, size_t cchElem)
        {
            _EnsureCount();

            bool fRet = false;

            if (iElem < _cchStringData)
            {
                cchElem = _NativeString_Min<size_t>(cchElem, _cchStringData - iElem); // @MOD Prevent double evaluation
                if (cchElem)
                {
                    memmove(&_pszStringData[iElem], &_pszStringData[iElem + cchElem], sizeof(WCHAR) * (_cchStringData - iElem - cchElem));
                    _cchStringData -= cchElem;
                }
                _pszStringData[_cchStringData] = 0;
                fRet = true;
            }

            return fRet;
        }

        bool _TrimStart(const WCHAR* pszTrim)
        {
            _EnsureCount();

            bool fNeedsTrimming = false;

            size_t cch;
            for (cch = 0; cch < _cchStringData; ++cch)
            {
                if (!wcschr(pszTrim, _pszStringData[cch]))
                    break;
            }

            if (cch)
            {
                fNeedsTrimming = true;
                memmove(_pszStringData, &_pszStringData[cch], sizeof(WCHAR) * (_cchStringData - cch) + sizeof(WCHAR));
                _cchStringData -= cch;
            }

            return fNeedsTrimming;
        }

        bool _TrimEnd(const WCHAR* pszTrim)
        {
            _EnsureCount();

            size_t cch;
            for (cch = _cchStringData; cch; --cch)
            {
                if (!wcschr(pszTrim, _pszStringData[cch - 1]))
                    break;
            }

            bool fNeedsTrimming = false;

            if (cch != _cchStringData)
            {
                fNeedsTrimming = true;
                _pszStringData[cch] = 0;
                _cchStringData = cch;
            }

            return fNeedsTrimming;
        }

        void _Free()
        {
            if (_pszStringData)
            {
                Allocator::Free(_pszStringData);
                _pszStringData = nullptr;
            }
            _cchStringData = 0;
            _cchStringDataCapacity = 0;
        }

        void _Attach(WCHAR* psz)
        {
            return _Attach(psz, wcslen(psz) + 1);
        }

        void _Attach(WCHAR* psz, const size_t cch)
        {
            _Free();
            if (psz && cch)
            {
                _pszStringData = psz;
                _cchStringData = cch - 1;
                _cchStringDataCapacity = cch;
                psz[cch - 1] = 0;
            }
        }

        WCHAR* _Detach()
        {
            WCHAR* pszStringData = _pszStringData;
            _pszStringData = nullptr;
            _cchStringData = 0;
            _cchStringDataCapacity = 0;
            return pszStringData;
        }

        WCHAR** _FreeAndGetAddressOf()
        {
            _Free();
            _cchStringData = s_cchUnknown;
            _cchStringDataCapacity = s_cchUnknown;
            return &_pszStringData;
        }

        static const size_t s_cchUnknown = -1;

        WCHAR* _pszStringData;
        size_t _cchStringData;
        size_t _cchStringDataCapacity;

        template <typename T>
        static FORCEINLINE constexpr const T& (_NativeString_Min)(const T& a, const T& b)
        {
            return a < b ? a : b;
        }
    };
}

typedef Windows::Internal::NativeString<Windows::Internal::CoTaskMemPolicy<WCHAR>> CoTaskMemNativeString;

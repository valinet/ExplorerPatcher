#pragma once

#include <Windows.h>
#include <type_traits>

#include "ContainerPolicies.h"

template <typename T>
class CSimpleArrayStandardCompareHelper
{
public:
    int Compare(const T& t1, const T& t2) const
    {
        return t2 == t1 ? 0 : t2 < t1 ? 1 : -1;
    }
};

class CSimpleArrayCaseInsensitiveOrdinalStringCompareHelper
{
public:
    int Compare(const WCHAR* psz1, const WCHAR* psz2) const
    {
        return CompareStringOrdinal(psz1, -1, psz2, -1, TRUE) - CSTR_EQUAL;
    }
};

template <typename T>
class CSimpleArrayStandardMergeHelper
{
};

template <
    typename T,
    typename CompareHelper
>
class CTSimpleFixedArray
{
public:
    T* _parray;
    size_t _celem;

    CTSimpleFixedArray()
        : _parray(nullptr)
        , _celem(0)
    {
    }

    size_t GetSize() const { return _celem; }

    T& operator[](size_t iElem) { return _parray[iElem]; }
    const T& operator[](size_t iElem) const { return _parray[iElem]; }

    HRESULT GetAt(size_t iElem, T& tOut) const
    {
        HRESULT hr = TYPE_E_OUTOFBOUNDS;
        if (iElem < _celem)
        {
            tOut = _parray[iElem];
            hr = S_OK;
        }
        return hr;
    }

    T* GetData() const { return _parray; }
    T* begin() { return _parray; }
    T* begin() const { return _parray; }
    T* end() { return _parray + _celem; }
    T* end() const { return _parray + _celem; }

    HRESULT Find(const T& t, size_t* piElem, size_t iStartAt = 0) const
    {
        return FindEx(CompareHelper(), t, piElem, iStartAt);
    }

    template <typename Comparer>
    HRESULT FindEx(const Comparer& tcompare, const T& t, size_t* piElem, size_t iStartAt = 0) const
    {
        *piElem = 0;
        for (size_t i = iStartAt; i < _celem; ++i)
        {
            if (tcompare.Compare(_parray[i], t) == 0)
            {
                *piElem = i;
                return S_OK;
            }
        }
        return TYPE_E_ELEMENTNOTFOUND;
    }

    HRESULT BinarySearch(const T& t, size_t* piElem) const
    {
        return BinarySearchEx(CompareHelper(), t, piElem);
    }

    template <typename Comparer>
    HRESULT BinarySearchEx(const Comparer& tcompare, const T& t, size_t* piElem) const
    {
        *piElem = 0;

        HRESULT hr = TYPE_E_ELEMENTNOTFOUND;

        if (_celem != 0)
        {
            hr = S_OK;

            size_t iLow = 0;
            size_t iHigh = _celem - 1;
            while (true)
            {
                size_t iMid = (iLow + iHigh) / 2;

                int compare = tcompare.Compare(_parray[iMid], t);
                if (compare > 0)
                {
                    if (iMid != 0)
                    {
                        iHigh = iMid - 1;
                    }
                    else
                    {
                        hr = TYPE_E_ELEMENTNOTFOUND;
                    }
                }
                else if (compare < 0)
                {
                    iLow = iMid + 1;
                }
                else
                {
                    for (; iMid != 0; --iMid)
                    {
                        if (tcompare.Compare(_parray[iMid - 1], t) != 0)
                            break;
                    }
                    *piElem = iMid;
                    break;
                }

                if (iHigh < iLow)
                    hr = TYPE_E_ELEMENTNOTFOUND;

                if (FAILED(hr))
                {
                    *piElem = compare < 0 ? iLow : iMid;
                    break;
                }
            }
        }

        return hr;
    }

    template <typename TCallback>
    void ForEach(const TCallback& callback) const // @MOD Pass callback by reference
    {
        for (size_t iElement = 0; iElement < _celem; ++iElement)
        {
            callback(iElement, _parray[iElement]);
        }
    }
};

template <
    typename T,
    size_t MaxSize,
    typename Allocator,
    typename CompareHelper,
    typename MergeHelper = CSimpleArrayStandardMergeHelper<T>
>
class CTSimpleArray : public CTSimpleFixedArray<T, CompareHelper>
{
public:
    T* _parrayT;
    size_t _celemCapacity;

    CTSimpleArray()
        : CTSimpleFixedArray<T, CompareHelper>()
        , _parrayT(nullptr)
        , _celemCapacity(0)
    {
    }

    ~CTSimpleArray()
    {
        RemoveAll();
    }

    HRESULT Add(const T& t, size_t* piElemInsertedAt = nullptr)
    {
        return _Add(t, piElemInsertedAt);
    }

    HRESULT Add(T&& t, size_t* piElemInsertedAt = nullptr)
    {
        return _Add(std::move(t), piElemInsertedAt);
    }

    HRESULT InsertAt(const T& t, size_t iElem)
    {
        return _InsertAt(t, iElem);
    }

    HRESULT InsertAt(T&& t, size_t iElem)
    {
        return _InsertAt(std::move(t), iElem);
    }

    HRESULT SetAtIndex(size_t iElem, const T& t)
    {
        return _SetAtIndex(iElem, t);
    }

    HRESULT SetAtIndex(size_t iElem, T&& t)
    {
        return _SetAtIndex(iElem, std::move(t));
    }

    HRESULT Remove(const T& t, size_t* piElemRemovedAt = nullptr)
    {
        if (piElemRemovedAt)
            *piElemRemovedAt = 0;

        size_t iElem;
        HRESULT hr = this->Find(t, &iElem);
        if (SUCCEEDED(hr))
        {
            hr = RemoveAt(iElem);
            if (SUCCEEDED(hr) && piElemRemovedAt)
            {
                *piElemRemovedAt = iElem;
            }
        }

        return hr;
    }

    HRESULT RemoveAt(size_t iElem)
    {
        if (iElem >= this->_celem)
            return TYPE_E_OUTOFBOUNDS;
        if constexpr (!std::is_trivially_destructible_v<T>)
            this->_parray[iElem].~T();
        if (iElem != this->_celem - 1)
            memmove(std::addressof(this->_parray[iElem]), std::addressof(this->_parray[iElem + 1]), sizeof(T) * (this->_celem - iElem - 1));
        --this->_celem;
        return S_OK;
    }

    void RemoveAll()
    {
        if (this->_parray)
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                for (size_t i = 0; i < this->_celem; ++i)
                    this->_parray[i].~T();
            }
            Allocator::Destroy(this->_parray);
            this->_parray = nullptr;
        }
        this->_celem = 0;
        _celemCapacity = 0;
    }

    void TransferData(CTSimpleArray* other)
    {
        RemoveAll();
        this->_parray = other->_parray;
        this->_celem = other->_celem;
        this->_parrayT = other->_parrayT;
        this->_celemCapacity = other->_celemCapacity;
        other->_parray = nullptr;
        other->_celem = 0;
        other->_parrayT = nullptr;
        other->_celemCapacity = 0;
    }

    size_t GetCapacity() const
    {
        return _celemCapacity;
    }

    HRESULT Sort()
    {
        return SortEx(CompareHelper());
    }

    template <typename Comparer>
    HRESULT SortEx(const Comparer& tcompare)
    {
        HRESULT hr = S_OK;

        if (this->_celem > 1)
        {
            _parrayT = nullptr;
            hr = Allocator::ReallocArray(nullptr, this->_celem / 2, &_parrayT);
            if (SUCCEEDED(hr))
            {
                _MergeSort(tcompare, 0, this->_celem);
                Allocator::Destroy(_parrayT);
                _parrayT = nullptr;
            }
        }

        return hr;
    }

    HRESULT _EnsureCapacity(size_t celemCapacityDesired, size_t celemMaxCapacity = 4096)
    {
        HRESULT hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        if (celemCapacityDesired > MaxSize)
            return hr;

        // If we have enough capacity, we're done
        hr = S_OK;
        size_t celemCapacityCur = _celemCapacity;
        if (celemCapacityDesired <= celemCapacityCur)
            return hr;

        // Double the capacity
        size_t celemCapacityT;
        hr = SizeTMult(celemCapacityCur, 2, &celemCapacityT);
        if (FAILED(hr))
            return hr;

        // Make sure we don't grow too much
        celemCapacityT = celemCapacityT - celemCapacityCur > celemMaxCapacity ? celemCapacityCur + celemMaxCapacity : celemCapacityT;

        // Cap at desired capacity and max capacity
        celemCapacityT = celemCapacityDesired > celemCapacityT || celemCapacityT <= MaxSize ? max(celemCapacityDesired, celemCapacityT) : MaxSize;

        // Realloc
        T* pvArrayT;
        hr = Allocator::ReallocArray(this->_parray, celemCapacityT, &pvArrayT);
        if (FAILED(hr))
            return hr;

        _celemCapacity = celemCapacityT;
        this->_parray = pvArrayT;

        return hr;
    }

    HRESULT _MakeRoomAt(size_t iElem)
    {
        HRESULT hr = S_OK;

        size_t cElemGrowTo = max(this->_celem, iElem) + 1;
        if (cElemGrowTo > _celemCapacity)
        {
            hr = _EnsureCapacity(cElemGrowTo);
        }

        if (SUCCEEDED(hr))
        {
            if (iElem < this->_celem)
                memmove(std::addressof(this->_parray[iElem + 1]), std::addressof(this->_parray[iElem]), sizeof(T) * (this->_celem - iElem));
            this->_celem = cElemGrowTo;
        }

        return hr;
    }

    template <typename ArgType>
    void _InternalSetAtIndex(size_t iElem, ArgType&& t)
    {
        T* newPos = std::addressof(this->_parray[iElem]);
        if (newPos)
            new(newPos) T(std::forward<ArgType>(t));
    }

    template <typename ArgType>
    HRESULT _Add(ArgType&& t, size_t* piElemInsertedAt)
    {
        if (piElemInsertedAt)
            *piElemInsertedAt = 0;

        HRESULT hr = S_OK;

        if (this->_celem == _celemCapacity)
        {
            hr = _EnsureCapacity(_celemCapacity + 1);
        }

        if (SUCCEEDED(hr))
        {
            _InternalSetAtIndex(this->_celem++, std::forward<ArgType>(t));
            if (piElemInsertedAt)
                *piElemInsertedAt = this->_celem - 1;
        }

        return hr;
    }

    template <typename ArgType>
    HRESULT _InsertAt(ArgType&& t, size_t iElem)
    {
        HRESULT hr = _MakeRoomAt(iElem);

        if (SUCCEEDED(hr))
        {
            _InternalSetAtIndex(iElem, std::forward<ArgType>(t));
        }

        return hr;
    }

    template <typename ArgType>
    HRESULT _SetAtIndex(size_t iElem, ArgType&& t)
    {
        HRESULT hr = TYPE_E_OUTOFBOUNDS;

        if (iElem < this->_celem)
        {
            _InternalSetAtIndex(iElem, std::forward<ArgType>(t));
            hr = S_OK;
        }

        return hr;
    }

    template <typename Comparer>
    void _MergeThem(const Comparer& tcompare, size_t iFirst, size_t cElems)
    {
        size_t cHalf = cElems / 2;
        T* parraySrc = &this->_parray[iFirst];
        memcpy(_parrayT, parraySrc, sizeof(T) * cHalf);

        size_t iIn1 = 0;
        size_t iIn2 = cHalf;
        size_t iOut = 0;
        bool fDone = false;
        while (!fDone)
        {
            if (tcompare.Compare(_parrayT[iIn1], parraySrc[iIn2]) > 0)
            {
                memmove(&parraySrc[iOut], &parraySrc[iIn2], sizeof(T));
                ++iOut;
                if (++iIn2 == cElems)
                {
                    memcpy(&parraySrc[iOut], &_parrayT[iIn1], sizeof(T) * (cElems - iOut));
                    fDone = true;
                }
            }
            else
            {
                memmove(&parraySrc[iOut], &_parrayT[iIn1], sizeof(T));
                ++iOut;
                if (++iIn1 == cHalf)
                {
                    fDone = true;
                }
            }
        }
    }

    template <typename Comparer>
    void _MergeSort(const Comparer& tcompare, size_t iFirst, size_t cElems)
    {
        if (cElems == 1)
            return;

        if (cElems == 2)
        {
            if (tcompare.Compare(this->_parray[iFirst], this->_parray[iFirst + 1]) > 0)
            {
                memmove(_parrayT, &this->_parray[iFirst], sizeof(T));
                memmove(&this->_parray[iFirst], &this->_parray[iFirst + 1], sizeof(T));
                memmove(&this->_parray[iFirst + 1], _parrayT, sizeof(T));
            }
        }
        else
        {
            size_t cHalf = cElems >> 1;
            _MergeSort(tcompare, iFirst, cHalf);
            _MergeSort(tcompare, iFirst + cHalf, cElems - cHalf);
            _MergeThem(tcompare, iFirst, cElems);
        }
    }
};

template <
    typename T,
    size_t MaxSize = UINT_MAX - 1,
    typename CompareHelper = CSimpleArrayStandardCompareHelper<T>
>
class CCoSimpleArray : public CTSimpleArray<T, MaxSize, CTPolicyCoTaskMem<T>, CompareHelper>
{
public:
    CCoSimpleArray()
    {
    }

    CCoSimpleArray(CCoSimpleArray&& other) noexcept
    {
        this->TransferData(&other);
    }

    CCoSimpleArray& operator=(CCoSimpleArray&& other) noexcept
    {
        if (this != &other)
        {
            this->TransferData(&other);
        }
        return *this;
    }
};

template <
    typename T,
    size_t MaxSize = UINT_MAX - 1,
    typename CompareHelper = CSimpleArrayStandardCompareHelper<T>
>
class CLocalSimpleArray : public CTSimpleArray<T, MaxSize, CTPolicyLocalMem<T>, CompareHelper>
{
};

template <
    typename T,
    typename ElementAllocator,
    typename CompareHelper = CSimpleArrayStandardCompareHelper<T>
>
class CSimplePointerArray : public CCoSimpleArray<T*, UINT_MAX - 1, CompareHelper>
{
public:
    ~CSimplePointerArray()
    {
        RemoveAndReleaseAll();
    }

    HRESULT RemoveAndReleaseAt(size_t iElem)
    {
        T* pT;
        HRESULT hr = this->GetAt(iElem, pT);
        if (SUCCEEDED(hr))
        {
            hr = this->RemoveAt(iElem);
            if (SUCCEEDED(hr))
            {
                ElementAllocator::Destroy(pT);
            }
        }
        return hr;
    }

    void RemoveAndReleaseAll()
    {
        for (size_t i = 0; i < this->_celem; ++i)
        {
            ElementAllocator::Destroy(this->_parray[i]);
        }
        this->RemoveAll();
    }
};

template <
    typename T,
    typename CompareHelper = CSimpleArrayStandardCompareHelper<T*>
>
class CSimplePointerArrayNewMem : public CSimplePointerArray<T, CTContainer_PolicyNewMem, CompareHelper>
{
};

template <
    typename T,
    typename CompareHelper = CSimpleArrayStandardCompareHelper<T*>
>
class CSimplePointerArrayCoTaskMem : public CSimplePointerArray<T, CTPolicyCoTaskMem<T>, CompareHelper>
{
};

template <
    typename T,
    typename CompareHelper = CSimpleArrayStandardCompareHelper<T*>
>
class CSimplePointerArrayLocalMem : public CSimplePointerArray<T, CTPolicyLocalMem<T>, CompareHelper>
{
};

template <typename T>
class CSimplePointerArrayRelease : public CSimplePointerArray<T, CTContainer_PolicyRelease<T>>
{
};

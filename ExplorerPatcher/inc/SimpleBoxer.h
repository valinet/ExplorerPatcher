#pragma once

// Define Windows.Foundation.IReference`1<String>

#ifndef DEF___FIReference_1_HSTRING_USE
#define DEF___FIReference_1_HSTRING_USE
#if !defined(RO_NO_TEMPLATE_NAME)
namespace ABI { namespace Windows { namespace Foundation {
template <>
struct __declspec(uuid("fd416dfb-2a07-52eb-aae3-dfce14116c05"))
IReference<HSTRING> : IReference_impl<HSTRING>
{
    static const wchar_t* z_get_rc_name_impl()
    {
        return L"Windows.Foundation.IReference`1<String>";
    }
};
// Define a typedef for the parameterized interface specialization's mangled name.
// This allows code which uses the mangled name for the parameterized interface to access the
// correct parameterized interface specialization.
typedef IReference<HSTRING> __FIReference_1_HSTRING_t;
#define __FIReference_1_HSTRING ABI::Windows::Foundation::__FIReference_1_HSTRING_t
/* Foundation */ } /* Windows */ } /* ABI */ }

#endif // !defined(RO_NO_TEMPLATE_NAME)
#endif /* DEF___FIReference_1_HSTRING_USE */

namespace SimpleBoxer
{

template <typename T>
class Reference final
    : public Microsoft::WRL::RuntimeClass<
          ABI::Windows::Foundation::IReference<T>
        , ABI::Windows::Foundation::IPropertyValue
    >
{
public:
    Reference(const T& value)
        : _value(value)
    {
    }

    //~ Begin IInspectable Interface
    STDMETHODIMP GetRuntimeClassName(HSTRING* runtimeName) override
    {
        *runtimeName = nullptr;
        HRESULT hr = S_OK;
        auto name = ABI::Windows::Foundation::IReference<T>::z_get_rc_name_impl();
        if (name != nullptr)
        {
            hr = WindowsCreateString(name, static_cast<UINT32>(wcslen(name)), runtimeName);
        }
        return hr;
    }

    STDMETHODIMP GetTrustLevel(TrustLevel* trustLvl) override
    {
        *trustLvl = BaseTrust;
        return S_OK;
    }
    //~ End IInspectable Interface

    //~ Begin ABI::Windows::Foundation::IReference<T> Interface
    STDMETHODIMP get_Value(T* value) override
    {
        *value = _value;
        return S_OK;
    }
    //~ End ABI::Windows::Foundation::IReference<T> Interface

    //~ Begin ABI::Windows::Foundation::IPropertyValue Interface
    STDMETHODIMP get_Type(ABI::Windows::Foundation::PropertyType* value) override
    {
        *value = ABI::Windows::Foundation::PropertyType_OtherType; return S_OK;
    }

    STDMETHODIMP get_IsNumericScalar(boolean* value) override
    {
        *value = std::is_arithmetic_v<T> || std::is_enum_v<T>;
        return S_OK;
    }

    STDMETHODIMP GetUInt8(BYTE* value) override { return _ToScalar<BYTE>(value); }
    STDMETHODIMP GetInt16(INT16* value) override { return _ToScalar<INT16>(value); }
    STDMETHODIMP GetUInt16(UINT16* value) override { return _ToScalar<UINT16>(value); }
    STDMETHODIMP GetInt32(INT32* value) override { return _ToScalar<INT32>(value); }
    STDMETHODIMP GetUInt32(UINT32* value) override { return _ToScalar<UINT32>(value); }
    STDMETHODIMP GetInt64(INT64* value) override { return _ToScalar<INT64>(value); }
    STDMETHODIMP GetUInt64(UINT64* value) override { return _ToScalar<UINT64>(value); }

    STDMETHODIMP GetSingle(FLOAT* value) override { *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetDouble(DOUBLE* value) override { *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetChar16(WCHAR* value) override { *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetBoolean(boolean* value) override { *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetString(HSTRING* value) override { *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetGuid(GUID* value) override { *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetDateTime(ABI::Windows::Foundation::DateTime* value) override { *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetTimeSpan(ABI::Windows::Foundation::TimeSpan* value) override { *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetPoint(ABI::Windows::Foundation::Point* value) override { *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetSize(ABI::Windows::Foundation::Size* value) override { *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetRect(ABI::Windows::Foundation::Rect* value) override { *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetUInt8Array(UINT32* valueLength, BYTE** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetInt16Array(UINT32* valueLength, INT16** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetUInt16Array(UINT32* valueLength, UINT16** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetInt32Array(UINT32* valueLength, INT32** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetUInt32Array(UINT32* valueLength, UINT32** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetInt64Array(UINT32* valueLength, INT64** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetUInt64Array(UINT32* valueLength, UINT64** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetSingleArray(UINT32* valueLength, FLOAT** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetDoubleArray(UINT32* valueLength, DOUBLE** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetChar16Array(UINT32* valueLength, WCHAR** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetBooleanArray(UINT32* valueLength, boolean** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetStringArray(UINT32* valueLength, HSTRING** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetInspectableArray(UINT32* valueLength, IInspectable*** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetGuidArray(UINT32* valueLength, GUID** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetDateTimeArray(UINT32* valueLength, ABI::Windows::Foundation::DateTime** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetTimeSpanArray(UINT32* valueLength, ABI::Windows::Foundation::TimeSpan** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetPointArray(UINT32* valueLength, ABI::Windows::Foundation::Point** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetSizeArray(UINT32* valueLength, ABI::Windows::Foundation::Size** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    STDMETHODIMP GetRectArray(UINT32* valueLength, ABI::Windows::Foundation::Rect** value) override { *valueLength = {}; *value = {}; return E_NOTIMPL; }
    //~ End ABI::Windows::Foundation::IPropertyValue Interface

private:
    template <typename To>
    HRESULT _ToScalar(To* pTo) const
    {
        if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>)
        {
            *pTo = static_cast<To>(_value);
            return S_OK;
        }
        else
        {
            *pTo = {};
            return E_NOTIMPL;
        }
    }

    T _value;
};

template <typename T, typename = void>
struct reference_traits
{
    static HRESULT Create(const T& value, IInspectable** ppPropertyValue)
    {
        HRESULT hr = E_OUTOFMEMORY;
        Microsoft::WRL::ComPtr<Reference<T>> spReference = Microsoft::WRL::Make<Reference<T>>(value);
        if (spReference.Get())
        {
            *ppPropertyValue = static_cast<ABI::Windows::Foundation::IReference<T>*>(spReference.Detach());
            hr = S_OK;
        }
        return hr;
    }

    static HRESULT Create(
        ABI::Windows::Foundation::IPropertyValueStatics*, const T& value, IInspectable** ppPropertyValue)
    {
        return Create(value, ppPropertyValue);
    }

    using InterfaceType = ABI::Windows::Foundation::IReference<T>;
};

#define DEFINE_REFERENCE_TRAITS_2(typeCpp, typeWinRt, creatorFuncName) \
    template <> \
    struct reference_traits<typeCpp> \
    { \
        static HRESULT Create(typeWinRt const& value, IInspectable** ppPropertyValue) \
        { \
            *ppPropertyValue = nullptr; \
            HSTRING_HEADER hstrH; \
            HSTRING hstr; \
            HRESULT hr = WindowsCreateStringReference( \
                RuntimeClass_Windows_Foundation_PropertyValue, \
                ARRAYSIZE(RuntimeClass_Windows_Foundation_PropertyValue) - 1, &hstrH, &hstr); \
            if (SUCCEEDED(hr)) \
            { \
                ABI::Windows::Foundation::IPropertyValueStatics* pPropertyValueStatics = nullptr; \
                hr = GetActivationFactory(hstr, &pPropertyValueStatics); \
                if (SUCCEEDED(hr)) \
                { \
                    IInspectable* pPropertyValue = nullptr; \
                    hr = pPropertyValueStatics->creatorFuncName(value, &pPropertyValue); \
                    if (SUCCEEDED(hr)) \
                    { \
                        *ppPropertyValue = pPropertyValue; \
                    } \
                    pPropertyValueStatics->Release(); \
                } \
            } \
            return hr; \
        } \
        static HRESULT Create( \
            ABI::Windows::Foundation::IPropertyValueStatics* pPropertyValueStatics, typeWinRt const& value, \
            IInspectable** ppPropertyValue) \
        { \
            return pPropertyValueStatics->creatorFuncName(value, ppPropertyValue); \
        } \
        using InterfaceType = ABI::Windows::Foundation::IReference<typeWinRt>; \
    };

#define DEFINE_REFERENCE_TRAITS(typeCpp, creatorFuncName) \
    DEFINE_REFERENCE_TRAITS_2(typeCpp, typeCpp, creatorFuncName)

// DEFINE_REFERENCE_TRAITS(BYTE, CreateUInt8);
DEFINE_REFERENCE_TRAITS(UINT16, CreateUInt16);
DEFINE_REFERENCE_TRAITS(INT16, CreateInt16);
DEFINE_REFERENCE_TRAITS(UINT32, CreateUInt32);
DEFINE_REFERENCE_TRAITS(INT32, CreateInt32);
DEFINE_REFERENCE_TRAITS(UINT64, CreateUInt64);
DEFINE_REFERENCE_TRAITS(INT64, CreateInt64);
DEFINE_REFERENCE_TRAITS(float, CreateSingle);
DEFINE_REFERENCE_TRAITS(double, CreateDouble);
DEFINE_REFERENCE_TRAITS(WCHAR, CreateChar16);
DEFINE_REFERENCE_TRAITS(bool, CreateBoolean);
DEFINE_REFERENCE_TRAITS_2(boolean, bool, CreateBoolean);
DEFINE_REFERENCE_TRAITS(HSTRING, CreateString);
DEFINE_REFERENCE_TRAITS(IInspectable*, CreateInspectable);
DEFINE_REFERENCE_TRAITS(GUID, CreateGuid);
DEFINE_REFERENCE_TRAITS(ABI::Windows::Foundation::DateTime, CreateDateTime);
DEFINE_REFERENCE_TRAITS(ABI::Windows::Foundation::TimeSpan, CreateTimeSpan);
DEFINE_REFERENCE_TRAITS(ABI::Windows::Foundation::Point, CreatePoint);
DEFINE_REFERENCE_TRAITS(ABI::Windows::Foundation::Size, CreateSize);
DEFINE_REFERENCE_TRAITS(ABI::Windows::Foundation::Rect, CreateRect);

#undef DEFINE_REFERENCE_TRAITS_2
#undef DEFINE_REFERENCE_TRAITS

// Make it easy to convert from a C string to an HSTRING
// Note: we don't check for null terminators!

template <typename T>
struct reference_traits<T, std::enable_if_t<std::is_array_v<std::remove_reference_t<T>> && std::is_same_v<std::remove_extent_t<std::remove_reference_t<T>>, const wchar_t>>>
{
    template <size_t N>
    static HRESULT Create(const wchar_t (&value)[N], IInspectable** ppPropertyValue)
    {
        static_assert(static_cast<size_t>(static_cast<unsigned int>(N-1)) == N-1,
            "String length underflow or overflow");
        *ppPropertyValue = nullptr;
        HSTRING_HEADER hstrH;
        HSTRING hstr;
        HRESULT hr = WindowsCreateStringReference(value, N - 1, &hstrH, &hstr);
        if (SUCCEEDED(hr))
        {
            hr = reference_traits<HSTRING>::Create(hstr, ppPropertyValue);
        }
        return hr;
    }

    template <size_t N>
    static HRESULT Create(
        ABI::Windows::Foundation::IPropertyValueStatics* pPropertyValueStatics, const wchar_t (&value)[N],
        IInspectable** ppPropertyValue)
    {
        static_assert(static_cast<size_t>(static_cast<unsigned int>(N-1)) == N-1,
            "String length underflow or overflow");
        *ppPropertyValue = nullptr;
        HSTRING_HEADER hstrH;
        HSTRING hstr;
        HRESULT hr = WindowsCreateStringReference(value, N - 1, &hstrH, &hstr);
        if (SUCCEEDED(hr))
        {
            hr = reference_traits<HSTRING>::Create(pPropertyValueStatics, hstr, ppPropertyValue);
        }
        return hr;
    }

    using InterfaceType = ABI::Windows::Foundation::IReference<HSTRING>;
};

template <typename T>
struct reference_traits<T, std::enable_if_t<std::is_convertible_v<T, const wchar_t*>>>
{
    static HRESULT Create(const wchar_t* value, IInspectable** ppPropertyValue)
    {
        *ppPropertyValue = nullptr;
        HSTRING_HEADER hstrH;
        HSTRING hstr;
        HRESULT hr = WindowsCreateStringReference(value, static_cast<UINT32>(wcslen(value)), &hstrH, &hstr);
        if (SUCCEEDED(hr))
        {
            hr = reference_traits<HSTRING>::Create(hstr, ppPropertyValue);
        }
        return hr;
    }

    static HRESULT Create(
        ABI::Windows::Foundation::IPropertyValueStatics* pPropertyValueStatics, const wchar_t* value,
        IInspectable** ppPropertyValue)
    {
        *ppPropertyValue = nullptr;
        HSTRING_HEADER hstrH;
        HSTRING hstr;
        HRESULT hr = WindowsCreateStringReference(value, static_cast<UINT32>(wcslen(value)), &hstrH, &hstr);
        if (SUCCEEDED(hr))
        {
            hr = reference_traits<HSTRING>::Create(pPropertyValueStatics, hstr, ppPropertyValue);
        }
        return hr;
    }

    using InterfaceType = ABI::Windows::Foundation::IReference<HSTRING>;
};

template <typename T>
struct reference_traits<T, std::enable_if_t<std::is_enum_v<T>>>
{
    static HRESULT Create(const T& value, IInspectable** ppPropertyValue)
    {
        return reference_traits<std::underlying_type_t<T>>::Create(static_cast<std::underlying_type_t<T>>(value), ppPropertyValue);
    }

    using InterfaceType = ABI::Windows::Foundation::IReference<std::underlying_type_t<T>>;
};

template <typename T>
HRESULT BoxValue(const T& value, IInspectable** ppBoxed)
{
    return reference_traits<T>::Create(value, ppBoxed);
}

template <typename T>
Microsoft::WRL::ComPtr<IInspectable> InlineBoxValue(const T& value)
{
    Microsoft::WRL::ComPtr<IInspectable> spBoxed;
    reference_traits<T>::Create(value, &spBoxed);
    return spBoxed;
}

template <typename T, typename = std::enable_if_t<std::is_base_of_v<IInspectable, T>>>
HRESULT BoxValue(T* value, IInspectable** ppBoxed)
{
    *ppBoxed = value;
    if (value)
    {
        value->AddRef();
    }
    return S_OK;
}

template <typename T, typename = std::enable_if_t<std::is_base_of_v<IInspectable, T>>>
Microsoft::WRL::ComPtr<IInspectable> InlineBoxValue(T* value)
{
    return value;
}

template <typename T>
HRESULT BoxValue(
    ABI::Windows::Foundation::IPropertyValueStatics* pPropertyValueStatics, const T& value, IInspectable** ppBoxed)
{
    return reference_traits<T>::Create(pPropertyValueStatics, value, ppBoxed);
}

template <typename T>
Microsoft::WRL::ComPtr<IInspectable> InlineBoxValue(
    ABI::Windows::Foundation::IPropertyValueStatics* pPropertyValueStatics, const T& value)
{
    Microsoft::WRL::ComPtr<IInspectable> spBoxed;
    reference_traits<T>::Create(pPropertyValueStatics, value, &spBoxed);
    return spBoxed;
}

template <typename T, typename = std::enable_if_t<std::is_base_of_v<IInspectable, T>>>
HRESULT BoxValue(ABI::Windows::Foundation::IPropertyValueStatics*, T* value, IInspectable** ppBoxed)
{
    *ppBoxed = value;
    if (value)
    {
        value->AddRef();
    }
    return S_OK;
}

template <typename T, typename = std::enable_if_t<std::is_base_of_v<IInspectable, T>>>
Microsoft::WRL::ComPtr<IInspectable> InlineBoxValue(ABI::Windows::Foundation::IPropertyValueStatics*, T* value)
{
    return value;
}

// For object types deriving from IInspectable
template <typename T, typename = std::enable_if_t<std::is_base_of_v<IInspectable, T>>>
HRESULT UnboxValue(IInspectable* pBoxed, T** ppObject)
{
    return pBoxed->QueryInterface(IID_PPV_ARGS(ppObject));
}

// For HSTRING
inline HRESULT UnboxValue(IInspectable* pBoxed, HSTRING* pValue)
{
    *pValue = nullptr;

    if (!pBoxed)
    {
        return E_NOINTERFACE;
    }

    ABI::Windows::Foundation::IReference<HSTRING>* pTemp = nullptr;
    HRESULT hr = pBoxed->QueryInterface(IID_PPV_ARGS(&pTemp));
    if (SUCCEEDED(hr))
    {
        HSTRING value = nullptr;
        hr = pTemp->get_Value(&value);
        if (SUCCEEDED(hr))
        {
            *pValue = value;
        }

        pTemp->Release();
    }

    return hr;
}

// For value types
template <typename T>
HRESULT UnboxValue(IInspectable* pBoxed, T* pValue)
{
    *pValue = {};

    if (!pBoxed)
    {
        return E_NOINTERFACE;
    }

    HRESULT hr;

    if constexpr (std::is_enum_v<T>)
    {
        // @Note: MIDLRT does not generate IReference template specializations for enums.
        /*ABI::Windows::Foundation::IReference<T>* pTemp = nullptr;
        hr = pBoxed->QueryInterface(IID_PPV_ARGS(&pTemp));
        if (SUCCEEDED(hr))
        {
            T value = {};
            hr = pTemp->get_Value(&value);
            if (SUCCEEDED(hr))
            {
                *pValue = value;
            }

            pTemp->Release();
        }
        else*/
        {
            ABI::Windows::Foundation::IReference<std::underlying_type_t<T>>* pTemp2 = nullptr;
            hr = pBoxed->QueryInterface(IID_PPV_ARGS(&pTemp2));
            if (SUCCEEDED(hr))
            {
                std::underlying_type_t<T> value = {};
                hr = pTemp2->get_Value(&value);
                if (SUCCEEDED(hr))
                {
                    *pValue = static_cast<T>(value);
                }

                pTemp2->Release();
            }
        }
    }
    else
    {
        typename reference_traits<T>::InterfaceType* pTemp = nullptr;
        hr = pBoxed->QueryInterface(IID_PPV_ARGS(&pTemp));
        if (SUCCEEDED(hr))
        {
            T value = {};
            hr = pTemp->get_Value(&value);
            if (SUCCEEDED(hr))
            {
                *pValue = value;
            }

            pTemp->Release();
        }
    }

    return hr;
}

// Allow inference from Microsoft::WRL::ComPtrRef
template <typename T, std::enable_if_t<std::is_base_of_v<IInspectable, T>, int> = 0>
HRESULT UnboxValue(IInspectable* pBoxed, Microsoft::WRL::Details::ComPtrRef<Microsoft::WRL::ComPtr<T>> ppObject)
{
    return UnboxValue(pBoxed, ppObject.ReleaseAndGetAddressOf());
}

#define SimpleBoxer_HrInitVars(hrVarName) \
    Microsoft::WRL::ComPtr<ABI::Windows::Foundation::IPropertyValueStatics> _SimpleBoxer_spPropertyValueStatics; \
    if (SUCCEEDED(hrVarName)) \
    { \
        HSTRING_HEADER hstrH; \
        HSTRING hstr; \
        hrVarName = WindowsCreateStringReference( \
            RuntimeClass_Windows_Foundation_PropertyValue, \
            ARRAYSIZE(RuntimeClass_Windows_Foundation_PropertyValue) - 1, &hstrH, &hstr); \
        if (SUCCEEDED(hrVarName)) \
        { \
            hrVarName = GetActivationFactory(hstr, &_SimpleBoxer_spPropertyValueStatics); \
        } \
    }(void)0 \

#define SimpleBoxer_WilInitVars() \
    HRESULT _SimpleBoxer_hrInit = S_OK; \
    SimpleBoxer_HrInitVars(_SimpleBoxer_hrInit); \
    RETURN_IF_FAILED(_SimpleBoxer_hrInit)

#define SimpleBoxer_BoxValue(value, ppBoxed) \
    SimpleBoxer::BoxValue(_SimpleBoxer_spPropertyValueStatics.Get(), value, ppBoxed)

#define SimpleBoxer_InlineBoxValue(value) \
    SimpleBoxer::InlineBoxValue(_SimpleBoxer_spPropertyValueStatics.Get(), value)

}

#pragma comment(lib, "oleaut32.lib")

// Use SysFreeString to free the returned string!
namespace SimpleStringer
{
#define DEFINE_STRINGER(type, func) \
    inline HRESULT GenericToString(type value, WCHAR** ppszOut) \
    { \
        return ::func(value, LOCALE_USER_DEFAULT, 0, ppszOut); \
    }

    DEFINE_STRINGER(BYTE, VarBstrFromUI1);
    DEFINE_STRINGER(SHORT, VarBstrFromI2);
    DEFINE_STRINGER(INT, VarBstrFromI4);
    DEFINE_STRINGER(LONG, VarBstrFromI4);
    DEFINE_STRINGER(LONG64, VarBstrFromI8);
    DEFINE_STRINGER(FLOAT, VarBstrFromR4);
    DEFINE_STRINGER(DOUBLE, VarBstrFromR8);
    // DEFINE_STRINGER(VARIANT_BOOL, VarBstrFromBool);
    DEFINE_STRINGER(CHAR, VarBstrFromI1);
    DEFINE_STRINGER(USHORT, VarBstrFromUI2);
    DEFINE_STRINGER(UINT, VarBstrFromUI4);
    DEFINE_STRINGER(ULONG, VarBstrFromUI4);
    DEFINE_STRINGER(ULONG64, VarBstrFromUI8);

#undef DEFINE_STRINGER

    // Special case
    inline HRESULT BoolToString(bool value, WCHAR** ppszOut)
    {
        return VarBstrFromBool(value ? VARIANT_TRUE : VARIANT_FALSE, LOCALE_USER_DEFAULT, 0, ppszOut);
    }
}

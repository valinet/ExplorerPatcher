#include <windows.ui.xaml.h>

#include <wrl/client.h>
#include <wrl/implements.h>
#include <wrl/wrappers/corewrappers.h>
#include <wil/result_macros.h>

// ReSharper disable once CppUnusedIncludeDirective
#include "ClassicWinRtForwardDecl.h"
#include "def.h"
#include "hooking.h"
#include "osutility.h"
#include "SimpleBoxer.h"
#include "utility.h"
#include "valinet/hooking/iatpatch.h"

// We don't need logging here
#include "inc/PushNoWilResultMacrosLogging.h"

using namespace Microsoft::WRL;

namespace wf = ABI::Windows::Foundation;
namespace wfc = ABI::Windows::Foundation::Collections;
namespace wux = ABI::Windows::UI::Xaml;

EXTERN_C_START

void InjectShellExperienceHostFor22H2OrHigher();

void* memmem(void* haystack, size_t haystacklen, void* needle, size_t needlelen);

void InjectShellExperienceHost()
{
    if (!IsWindows11())
    {
        return;
    }
    if (IsWindows11Version22H2OrHigher())
    {
        InjectShellExperienceHostFor22H2OrHigher();
        return;
    }

    HKEY hKey;
    if (RegOpenKeyW(HKEY_CURRENT_USER, _T(SEH_REGPATH), &hKey) != ERROR_SUCCESS)
    {
        return;
    }
    RegCloseKey(hKey);
    HMODULE hQA = LoadLibraryW(L"Windows.UI.QuickActions.dll");
    if (hQA)
    {
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hQA;
        if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE)
        {
            PIMAGE_NT_HEADERS64 ntHeader = (PIMAGE_NT_HEADERS64)((u_char*)dosHeader + dosHeader->e_lfanew);
            if (ntHeader->Signature == IMAGE_NT_SIGNATURE)
            {
                PBYTE pSEHPatchArea = nullptr;
                BYTE seh_pattern1[14] =
                {
                    // mov al, 1
                    0xB0, 0x01,
                    // jmp + 2
                    0xEB, 0x02,
                    // xor al, al
                    0x32, 0xC0,
                    // add rsp, 0x20
                    0x48, 0x83, 0xC4, 0x20,
                    // pop rdi
                    0x5F,
                    // pop rsi
                    0x5E,
                    // pop rbx
                    0x5B,
                    // ret
                    0xC3
                };
                BYTE seh_off = 12;
                BYTE seh_pattern2[5] =
                {
                    // mov r8b, 3
                    0x41, 0xB0, 0x03,
                    // mov dl, 1
                    0xB2, 0x01
                };
                bool bTwice = false;
                PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeader);
                for (unsigned int i = 0; i < ntHeader->FileHeader.NumberOfSections; ++i)
                {
                    if (section->Characteristics & IMAGE_SCN_CNT_CODE)
                    {
                        if (section->SizeOfRawData && !bTwice)
                        {
                            PBYTE pSectionBegin = (PBYTE)hQA + section->VirtualAddress;
                            //DWORD dwOldProtect;
                            //VirtualProtect(pSectionBegin, section->SizeOfRawData, PAGE_EXECUTE_READWRITE, &dwOldProtect);
                            PBYTE pCandidate = nullptr;
                            while (true)
                            {
                                pCandidate = (PBYTE)memmem(
                                    !pCandidate ? pSectionBegin : pCandidate,
                                    !pCandidate ? section->SizeOfRawData : (uintptr_t)section->SizeOfRawData - (uintptr_t)(pCandidate - pSectionBegin),
                                    seh_pattern1,
                                    sizeof(seh_pattern1)
                                );
                                if (!pCandidate)
                                {
                                    break;
                                }
                                PBYTE pCandidate2 = pCandidate - seh_off - sizeof(seh_pattern2);
                                if (pCandidate2 > (PBYTE)(UINT_PTR)section->VirtualAddress)
                                {
                                    if (memmem(pCandidate2, sizeof(seh_pattern2), seh_pattern2, sizeof(seh_pattern2)))
                                    {
                                        if (!pSEHPatchArea)
                                        {
                                            pSEHPatchArea = pCandidate;
                                        }
                                        else
                                        {
                                            bTwice = true;
                                        }
                                    }
                                }
                                pCandidate += sizeof(seh_pattern1);
                            }
                            //VirtualProtect(pSectionBegin, section->SizeOfRawData, dwOldProtect, &dwOldProtect);
                        }
                    }
                    section++;
                }
                if (pSEHPatchArea && !bTwice)
                {
                    DWORD dwOldProtect;
                    VirtualProtect(pSEHPatchArea, sizeof(seh_pattern1), PAGE_EXECUTE_READWRITE, &dwOldProtect);
                    pSEHPatchArea[2] = 0x90;
                    pSEHPatchArea[3] = 0x90;
                    VirtualProtect(pSEHPatchArea, sizeof(seh_pattern1), dwOldProtect, &dwOldProtect);
                }
            }
        }
    }
}

// On 22H2 builds, the Windows 10 flyouts for network and battery can be enabled
// by patching either of the following functions in ShellExperienceHost. I didn't
// see any differences when patching with any of the 3 methods, although
// `SharedUtilities::IsWindowsLite` seems to be invoked in more places, whereas `GetProductInfo`
// and `RtlGetDeviceFamilyInfoEnum` are only called in `FlightHelper::CalculateRepaintEnabled`
// and either seems to get the job done. YMMV

/*LSTATUS WINAPI SEH_RegGetValueW(HKEY hkey, LPCWSTR lpSubKey, LPCWSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData)
{
    if (!lstrcmpW(lpValue, L"UseLiteLayout")) { *(DWORD*)pvData = 1; return ERROR_SUCCESS; }
    return RegGetValueW(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
}*/

/*BOOL WINAPI SEH_RtlGetDeviceFamilyInfoEnum(INT64 u0, PDWORD u1, INT64 u2)
{
    *u1 = 10;
    return TRUE;
}*/

BOOL WINAPI SEH_GetProductInfo(DWORD dwOSMajorVersion, DWORD dwOSMinorVersion, DWORD dwSpMajorVersion, DWORD dwSpMinorVersion, PDWORD pdwReturnedProductType)
{
    *pdwReturnedProductType = 119;
    return TRUE;
}

static bool g_bQuickActionControlTemplatesPatched;

// Fix battery flyout crashing on 25951+
void HandleLoadedQuickActions(HMODULE hModule)
{
    if (!hModule)
    {
        return;
    }

    static bool bPatched = false;
    if (bPatched)
    {
        return;
    }
    bPatched = true;

    PBYTE pSearch;
    DWORD cbSearch;
    if (TextSectionBeginAndSize(hModule, &pSearch, &cbSearch))
    {
#if defined(_M_X64)
        // Note: These patterns will break when the class sizes change!

        PBYTE pfn = nullptr;

        // Find source
        // 48 89 45 50 BA 90 00 00 00 8D 4A E8 FF 15 ?? ?? ?? ?? 48 89 45 58 48 8B C8 E8 ?? ?? ?? ?? 48 8B F0
        //             ^^^^^^^^^^^^^^^^^^^^^^^                                        .  ^^^^^^^^^^^
        // ref new QuickActions::QuickActionTemplates();
        // Ref: QuickActions::QuickActionTemplates::Instance::get()
        PBYTE matchSource = (PBYTE)FindPattern(
            pSearch, cbSearch,
            "\x48\x89\x45\x50\xBA\x90\x00\x00\x00\x8D\x4A\xE8\xFF\x15\x00\x00\x00\x00\x48\x89\x45\x58\x48\x8B\xC8\xE8\x00\x00\x00\x00\x48\x8B\xF0",
            "xxxxxxxxxxxxxx????xxxxxxxx????xxx"
        );
        if (matchSource)
        {
            pfn = matchSource + 25;
            pfn += 5 + *(int*)(pfn + 1);
        }

        // Find target
        // 48 8B D0 48 8B CB E8 ?? ?? ?? ?? 90 4D 85 ?? 74 10 49 8B ?? 49 8B ?? 48 8B 40 10 E8 ?? ?? ?? ?? 90 49 8B CC
        //                   xxxxxxxxxxxxxx nop
        // E8 ?? ?? ?? ?? 48 8B D3 48 8B CE E8 ?? ?? ?? ?? BA B8 00 00 00 8D 4A E8 FF 15 ?? ?? ?? ?? 48 89 45 ??
        //                                                 xxxxxxxxxxxxxxxxxxxxxxx mov edx & lea rcx
        // 48 8B C8 E8 ?? ?? ?? ?? 4C 8B
        //          .  xxxxxxxxxxx ctor call
        // ref new QuickActions::ControlCenter::ControlCenterTemplates();
        // Ref: QuickActions::QuickActionControl::QuickActionControl()
        PBYTE matchTarget = (PBYTE)FindPattern(
            pSearch, cbSearch,
            "\x48\x8B\xD0\x48\x8B\xCB\xE8\x00\x00\x00\x00\x90\x4D\x85\x00\x74\x10\x49\x8B\x00\x49\x8B\x00\x48\x8B\x40\x10\xE8\x00\x00\x00\x00\x90\x49\x8B\xCC"
            "\xE8\x00\x00\x00\x00\x48\x8B\xD3\x48\x8B\xCE\xE8\x00\x00\x00\x00\xBA\xB8\x00\x00\x00\x8D\x4A\xE8\xFF\x15\x00\x00\x00\x00\x48\x89\x45\x00"
            "\x48\x8B\xC8\xE8\x00\x00\x00\x00\x4C\x8B",
            "xxxxxxx????xxx?xxxx?xx?xxxxx????xxxx"
            "x????xxxxxxx????xxxxxxxxxx????xxx?"
            "xxxx????xx"
        );

        if (matchSource && matchTarget && pfn)
        {
            PBYTE pinsnMovEdxLeaRcxSrc = matchSource + 4;

            PBYTE pinsnCallLoadComponent = matchTarget + 6;
            PBYTE pinsnMovEdxLeaRcxDst = matchTarget + 52;
            PBYTE pinsnCallCtor = matchTarget + 73;

            DWORD dwOldProtect;
            if (VirtualProtect(matchTarget, 80, PAGE_EXECUTE_READWRITE, &dwOldProtect))
            {
                memset(pinsnCallLoadComponent, 0x90, 5); // nop
                memcpy(pinsnMovEdxLeaRcxDst, pinsnMovEdxLeaRcxSrc, 8); // change Platform::Details::Heap::Allocate(x, x) args
                *(int*)(pinsnCallCtor + 1) = (int)((INT_PTR)pfn - (INT_PTR)(pinsnCallCtor + 5)); // change ctor call target
                g_bQuickActionControlTemplatesPatched = true;
                VirtualProtect(matchTarget, 80, dwOldProtect, &dwOldProtect);
            }
        }
#elif defined(_M_ARM64)
        // Note: These patterns will break when the class sizes change!

        PBYTE pfn = nullptr;

        // Find source
        // B7 17 00 F9 ?? ?? ?? ?? ?? ?? ?? ?? 01 12 80 D2 00 0F 80 D2 00 01 3F D6 A0 13 00 F9 ?? ?? ?? ?? ?? 03 00 AA
        //                                     ^^^^^^^^^^^^^^^^^^^^^^^                         ^^^^^^^^^^^
        // ref new QuickActions::QuickActionTemplates();
        // Ref: QuickActions::QuickActionTemplates::Instance::get()
        PBYTE matchSource = (PBYTE)FindPattern_4_(
            pSearch, cbSearch,
            "\xB7\x17\x00\xF9\x00\x00\x00\x00\x00\x00\x00\x00\x01\x12\x80\xD2\x00\x0F\x80\xD2\x00\x01\x3F\xD6\xA0\x13\x00\xF9\x00\x00\x00\x00\x00\x03\x00\xAA",
            "xxxx????????xxxxxxxxxxxxxxxx?????xxx"
        );
        if (matchSource)
        {
            pfn = matchSource + 28;
            pfn = (PBYTE)ARM64_FollowBL((DWORD*)pfn);
        }

        // Find target
        // E1 03 ?? AA E0 03 ?? AA ?? ?? ?? ?? 1F 20 03 D5 E0 03 ?? AA ?? ?? ?? ?? 1F 20 03 D5 E0 03 ?? AA ?? ?? ?? ??
        //                         xxxxxxxxxxx nop
        // E1 03 ?? AA E0 03 ?? AA ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 01 17 80 D2 00 14 80 D2 00 01 3F D6 40 03 00 F9
        //                                                             xxxxxxxxxxxxxxxxxxxxxxx mov x1 & mov x0
        // ?? ?? ?? ?? ?? 03 00 AA
        // xxxxxxxxxxx ctor call
        // ref new QuickActions::ControlCenter::ControlCenterTemplates();
        // Ref: QuickActions::QuickActionControl::QuickActionControl()
        PBYTE matchTarget = (PBYTE)FindPattern_4_(
            pSearch, cbSearch,
            "\xE1\x03\x00\xAA\xE0\x03\x00\xAA\x00\x00\x00\x00\x1F\x20\x03\xD5\xE0\x03\x00\xAA\x00\x00\x00\x00\x1F\x20\x03\xD5\xE0\x03\x00\xAA\x00\x00\x00\x00"
            "\xE1\x03\x00\xAA\xE0\x03\x00\xAA\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x17\x80\xD2\x00\x14\x80\xD2\x00\x01\x3F\xD6\x40\x03\x00\xF9"
            "\x00\x00\x00\x00\x00\x03\x00\xAA",
            "xx?xxx?x????xxxxxx?x????xxxxxx?x????"
            "xx?xxx?x????????????xxxxxxxxxxxxxxxx"
            "?????xxx"
        );

        if (matchSource && matchTarget && pfn)
        {
            DWORD* pinsnMovX1MovX0Src = (DWORD*)(matchSource + 12);

            DWORD* pinsnBLLoadComponent = (DWORD*)(matchTarget + 8);
            DWORD* pinsnMovX1MovX0Dst = (DWORD*)(matchTarget + 56);
            DWORD* pinsnBLTemplatesCtor = (DWORD*)(matchTarget + 72);

            DWORD insnBLNew = ARM64_MakeBL((int)(((UINT_PTR)pfn - (UINT_PTR)pinsnBLTemplatesCtor) / 4));

            DWORD dwOldProtect;
            if (insnBLNew && VirtualProtect(matchTarget, 80, PAGE_EXECUTE_READWRITE, &dwOldProtect))
            {
                *pinsnBLLoadComponent = 0xD503201F; // nop
                memcpy(pinsnMovX1MovX0Dst, pinsnMovX1MovX0Src, 8); // change Platform::Details::Heap::Allocate(x, x) args
                *pinsnBLTemplatesCtor = insnBLNew; // change ctor call target
                g_bQuickActionControlTemplatesPatched = true;
                VirtualProtect(matchTarget, 80, dwOldProtect, &dwOldProtect);
            }
        }
#endif
    }
}

// Revert network flyout quick actions buttons to Cobalt-Nickel style
// If we're doing the quick actions patch but not this, they will only appear as non-interactive text blocks.
HRESULT WINAPI NetworkUX_WindowsCreateStringReference(
    PCWSTR sourceString, UINT32 length, HSTRING_HEADER* hstringHeader, HSTRING* string)
{
    static constexpr WCHAR c_szToggleButtonWinuiFluentTemplate[] = L"ToggleButtonWinuiFluentTemplate";
    static constexpr WCHAR c_szQuickToggleWinuiFluentTemplate[] = L"QuickToggleWinuiFluentTemplate";

    switch (length)
    {
        case ARRAYSIZE(c_szToggleButtonWinuiFluentTemplate) - 1:
            if (wcscmp(sourceString, c_szToggleButtonWinuiFluentTemplate) == 0 && g_bQuickActionControlTemplatesPatched)
            {
                sourceString = c_szQuickToggleWinuiFluentTemplate;
                length = ARRAYSIZE(c_szQuickToggleWinuiFluentTemplate) - 1;
            }
            break;
    }

    return WindowsCreateStringReference(sourceString, length, hstringHeader, string);
}

HRESULT NetworkUX_PatchResourceDictionary()
{
    SimpleBoxer_WilInitVars();
    ComPtr<IInspectable> spKey, spValue;

    ComPtr<wfc::IMap<IInspectable*, IInspectable*>> spResourceDictionary_Map; // = wux::Application::Current().Resources();
    {
        ComPtr<wux::IApplicationStatics> spApplicationStatics;
        RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
            Wrappers::HStringReference(RuntimeClass_Windows_UI_Xaml_Application).Get(), &spApplicationStatics));

        ComPtr<wux::IApplication> spApplication;
        RETURN_IF_FAILED(spApplicationStatics->get_Current(&spApplication));

        ComPtr<wux::IResourceDictionary> spResourceDictionary;
        RETURN_IF_FAILED(spApplication->get_Resources(&spResourceDictionary));
        RETURN_IF_FAILED(spResourceDictionary.As(&spResourceDictionary_Map));
    }

    ComPtr<IInspectable> spQuickActionPanelMargin; // = spResourceDictionary.Lookup(L"QuickActionPanelMargin");
    ComPtr<wux::IStyle> spQuickActionControlStyle; // = spResourceDictionary.Lookup(L"QuickActionControlStyle").try_as<wux::IStyle>();
    {
        {
            RETURN_IF_FAILED(SimpleBoxer_BoxValue(L"QuickActionPanelMargin", &spKey));
            spResourceDictionary_Map->Lookup(spKey.Get(), &spQuickActionPanelMargin);
        }
        {
            ComPtr<IInspectable> spQuickActionControlStyle_Object;
            RETURN_IF_FAILED(SimpleBoxer_BoxValue(L"QuickActionControlStyle", &spKey));
            if (SUCCEEDED(spResourceDictionary_Map->Lookup(spKey.Get(), &spQuickActionControlStyle_Object)))
            {
                spQuickActionControlStyle_Object.As(&spQuickActionControlStyle);
            }
        }
    }

    // Old: <Thickness x:Key="QuickActionPanelMargin" Value="12,0,0,12" /> <-- change back to this
    // New: <Thickness x:Key="QuickActionPanelMargin" Value="12,0,24,0" />
    if (spQuickActionPanelMargin.Get())
    {
        boolean bReplaced;
        RETURN_IF_FAILED(SimpleBoxer_BoxValue(L"QuickActionPanelMargin", &spKey));
        RETURN_IF_FAILED(SimpleBoxer_BoxValue(wux::Thickness(12.0, 0.0, 0.0, 12.0), &spValue));
        RETURN_IF_FAILED(spResourceDictionary_Map->Insert(spKey.Get(), spValue.Get(), &bReplaced));
    }

    // Old: Margin=(4,0,0,4) Width=90 Height=64 <-- change back to this
    // New: Margin=(12,0,0,0) Width=96 Height=90
    if (spQuickActionControlStyle.Get())
    {
        ComPtr<wfc::IVector<wux::SetterBase*>> spSetters_Vector; // = spQuickActionControlStyle.Setters();
        {
            ComPtr<wux::ISetterBaseCollection> spSetters;
            RETURN_IF_FAILED(spQuickActionControlStyle->get_Setters(&spSetters));
            RETURN_IF_FAILED(spSetters.As(&spSetters_Vector));
        }

        RETURN_IF_FAILED(spSetters_Vector->Clear());

        ComPtr<wux::ISetterFactory> spSetterFactory;
        RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
            Wrappers::HStringReference(RuntimeClass_Windows_UI_Xaml_Setter).Get(), &spSetterFactory));

        ComPtr<wux::IDependencyProperty> spMarginProperty, spWidthProperty, spHeightProperty;
        {
            ComPtr<wux::IFrameworkElementStatics> spFrameworkElementStatics;
            RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
                Wrappers::HStringReference(RuntimeClass_Windows_UI_Xaml_FrameworkElement).Get(), &spFrameworkElementStatics));

            RETURN_IF_FAILED(spFrameworkElementStatics->get_MarginProperty(&spMarginProperty));
            RETURN_IF_FAILED(spFrameworkElementStatics->get_WidthProperty(&spWidthProperty));
            RETURN_IF_FAILED(spFrameworkElementStatics->get_HeightProperty(&spHeightProperty));
        }

        struct
        {
            wux::IDependencyProperty* pProperty;
            ComPtr<IInspectable> spValue;
        } rgSettersToAdd[] =
        {
            { spMarginProperty.Get(), SimpleBoxer_InlineBoxValue(wux::Thickness(4.0, 0.0, 0.0, 4.0)) },
            { spWidthProperty.Get(), SimpleBoxer_InlineBoxValue(90.0) },
            { spHeightProperty.Get(), SimpleBoxer_InlineBoxValue(64.0) },
        };

        for (const auto& entry : rgSettersToAdd)
        {
            RETURN_IF_NULL_ALLOC(entry.spValue);

            ComPtr<wux::ISetter> spSetter;
            RETURN_IF_FAILED(spSetterFactory->CreateInstance(entry.pProperty, entry.spValue.Get(), &spSetter));

            ComPtr<wux::ISetterBase> spSetter_Base;
            RETURN_IF_FAILED(spSetter.As(&spSetter_Base));
            RETURN_IF_FAILED(spSetters_Vector->Append(spSetter_Base.Get()));
        }
    }

    return S_OK;
}

void (*NetworkUX_App_LoadResourceDictionariesFunc)();
void NetworkUX_App_LoadResourceDictionariesHook()
{
    NetworkUX_App_LoadResourceDictionariesFunc();

    if (g_bQuickActionControlTemplatesPatched)
    {
        NetworkUX_PatchResourceDictionary();
    }
}

void HandleLoadedNetworkUX(HMODULE hModule)
{
    if (!hModule)
    {
        return;
    }

    static bool bPatched = false;
    if (bPatched)
    {
        return;
    }
    bPatched = true;

    VnPatchIAT(hModule, (PSTR)"api-ms-win-core-winrt-string-l1-1-0.dll", (PSTR)"WindowsCreateStringReference", (uintptr_t)NetworkUX_WindowsCreateStringReference);

    PBYTE pSearch;
    DWORD cbSearch;
    if (TextSectionBeginAndSize(hModule, &pSearch, &cbSearch))
    {
#if defined(_M_X64)
        // NetworkUX::App::LoadResourceDictionaries()
        // 48 8B 40 10 E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 80 3D ?? ?? ?? ?? 00 75 05 E8
        //                            .  ^^^^^^^^^^^
        // Ref: NetworkUX::App::StaticOnLaunched()
        PBYTE match = (PBYTE)FindPattern(
            pSearch, cbSearch,
            "\x48\x8B\x40\x10\xE8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x80\x3D\x00\x00\x00\x00\x00\x75\x05\xE8",
            "xxxxx????x????xx????xxxx"
        );
        if (match)
        {
            match += 9;
            match += 5 + *(int*)(match + 1);
        }
#elif defined(_M_ARM64)
        // NetworkUX::App::LoadResourceDictionaries()
        // E0 03 ?? AA ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 08 ?? ?? ?? 48 00 00 35
        //                         ^^^^^^^^^^^
        // Ref: NetworkUX::App::StaticOnLaunched()
        PBYTE match = (PBYTE)FindPattern_4_(
            pSearch, cbSearch,
            "\xE0\x03\x00\xAA\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x48\x00\x00\x35",
            "xx?x????????????x???xxxx"
        );
        if (match)
        {
            match += 8;
            match = (PBYTE)ARM64_FollowBL((DWORD*)match);
        }
#endif
        if (match)
        {
            NetworkUX_App_LoadResourceDictionariesFunc = reinterpret_cast<decltype(NetworkUX_App_LoadResourceDictionariesFunc)>(match);
            funchook_prepare(funchook, (void**)&NetworkUX_App_LoadResourceDictionariesFunc, NetworkUX_App_LoadResourceDictionariesHook);
        }
    }
}

typedef HRESULT (WINAPI* GetActivationFactoryByPCWSTR_t)(PCWSTR activatableClassId, REFIID riid, void** ppv);
GetActivationFactoryByPCWSTR_t GetActivationFactoryByPCWSTRFunc;
HRESULT WINAPI SEH_GetActivationFactoryByPCWSTR(PCWSTR activatableClassId, REFIID riid, void** ppv)
{
    HRESULT hr = GetActivationFactoryByPCWSTRFunc(activatableClassId, riid, ppv);

    if (SUCCEEDED(hr))
    {
        if (wcscmp(activatableClassId, L"QuickActions.quickactions_XamlTypeInfo.XamlMetaDataProvider") == 0)
        {
            HandleLoadedQuickActions(GetModuleHandleW(L"Windows.UI.QuickActions.dll"));
        }
        else if (wcscmp(activatableClassId, L"NetworkUX.networkux_XamlTypeInfo.XamlMetaDataProvider") == 0)
        {
            HandleLoadedNetworkUX(GetModuleHandleW(L"NetworkUX.dll"));
        }
    }

    return hr;
}

void InjectShellExperienceHostFor22H2OrHigher()
{
    if (!IsWindows11Version22H2Build1413OrHigher())
    {
        HKEY hKey;
        if (RegOpenKeyW(HKEY_CURRENT_USER, _T(SEH_REGPATH), &hKey) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            HMODULE hQA = LoadLibraryW(L"Windows.UI.QuickActions.dll");
            if (hQA)
            {
                VnPatchIAT(hQA, (PSTR)"api-ms-win-core-sysinfo-l1-2-0.dll", (PSTR)"GetProductInfo", (uintptr_t)SEH_GetProductInfo);
                // VnPatchIAT(hQA, "ntdll.dll", "RtlGetDeviceFamilyInfoEnum", SEH_RtlGetDeviceFamilyInfoEnum);
                // VnPatchIAT(hQA, "api-ms-win-core-registry-l1-1-0.dll", "RegGetValueW", SEH_RegGetValueW);
            }
        }
    }

    if (global_rovi.dwBuildNumber >= 25951)
    {
        HMODULE hWincorlib = GetModuleHandleW(L"wincorlib.DLL");
        if (hWincorlib)
        {
            GetActivationFactoryByPCWSTRFunc = (GetActivationFactoryByPCWSTR_t)GetProcAddress(hWincorlib, "?GetActivationFactoryByPCWSTR@@YAJPEAXAEAVGuid@Platform@@PEAPEAX@Z");
        }

        HMODULE hSEH = GetModuleHandleW(nullptr);
        if (hSEH)
        {
            if (GetActivationFactoryByPCWSTRFunc)
            {
                VnPatchIAT(hSEH, (PSTR)"wincorlib.DLL", (PSTR)"?GetActivationFactoryByPCWSTR@@YAJPEAXAEAVGuid@Platform@@PEAPEAX@Z", (uintptr_t)SEH_GetActivationFactoryByPCWSTR);
            }
        }
    }
}

EXTERN_C_END

#include "inc/PopNoWilResultMacrosLogging.h"

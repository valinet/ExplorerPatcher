// SearchHost AppVerifier provider. Keep the provider independent of the CRT:
// Windows loads it very early during process initialization.
#include <windows.h>
#include <winternl.h>

__declspec(dllimport) NTSTATUS NTAPI RtlCreateUserThread(HANDLE, PSECURITY_DESCRIPTOR, BOOLEAN, ULONG, SIZE_T, SIZE_T, NTSTATUS (NTAPI*)(PVOID), PVOID, PHANDLE, PVOID);
__declspec(dllimport) NTSTATUS NTAPI LdrLoadDll(PWSTR, PULONG, PUNICODE_STRING, PVOID*);
__declspec(dllimport) NTSTATUS NTAPI LdrGetProcedureAddress(PVOID, PANSI_STRING, ULONG, PVOID*);

#define DLL_PROCESS_VERIFIER 4

typedef void (NTAPI *PROVIDER_DLL_CALLBACK)(PWSTR, PVOID, SIZE_T, PVOID);
typedef void (NTAPI *PROVIDER_HEAP_CALLBACK)(PVOID, SIZE_T);

typedef struct
{
    PWSTR name;
    DWORD flags;
    PVOID address;
    PVOID thunks;
} PROVIDER_DLL;

typedef struct
{
    DWORD size;
    PROVIDER_DLL* dlls;
    PROVIDER_DLL_CALLBACK dllLoad, dllUnload;
    PWSTR image;
    DWORD flags, debugFlags;
    PVOID stackTrace, heapCreate, heapDestroy;
    PROVIDER_HEAP_CALLBACK heapFree;
} PROVIDER_DESCRIPTOR;

static PROVIDER_DLL emptyDlls[1];

static void NTAPI OnLoad(PWSTR dllName, PVOID dllBase, SIZE_T dllSize, PVOID reserved)
{
}

static void NTAPI OnUnload(PWSTR dllName, PVOID dllBase, SIZE_T dllSize, PVOID reserved)
{
}

static void NTAPI OnHeapFree(PVOID allocation, SIZE_T size)
{
}

static PROVIDER_DESCRIPTOR descriptor = { sizeof(PROVIDER_DESCRIPTOR), emptyDlls, OnLoad, OnUnload, 0, 0, 0, 0, 0, 0, OnHeapFree };

static NTSTATUS NTAPI InitializeSearch(PVOID unused)
{
    static WCHAR kernelName[] = L"kernel32.dll";
    static CHAR environmentName[] = "GetEnvironmentVariableW";
#ifdef EP_PROVIDER_TEST
    static WCHAR programFiles[] = L"EP_AVRF_TEST_ROOT";
#else
    static WCHAR programFiles[] = L"ProgramFiles";
#endif
#if defined(_M_ARM64)
    static WCHAR dllSuffix[] = L"\\ExplorerPatcher\\ExplorerPatcher.arm64.dll";
#else
    static WCHAR dllSuffix[] = L"\\ExplorerPatcher\\ExplorerPatcher.amd64.dll";
#endif
    static CHAR entryName[] = "EPInitializeSearchHost";
    UNICODE_STRING kernel = { sizeof(kernelName) - sizeof(WCHAR), sizeof(kernelName), kernelName };
    ANSI_STRING environment = { sizeof(environmentName) - 1, sizeof(environmentName), environmentName };
    ANSI_STRING entry = { sizeof(entryName) - 1, sizeof(entryName), entryName };
    PVOID kernelModule = NULL, address = NULL, epModule = NULL;
    if (LdrLoadDll(NULL, NULL, &kernel, &kernelModule) < 0 ||
        LdrGetProcedureAddress(kernelModule, &environment, 0, &address) < 0)
    {
        return 0;
    }

    WCHAR path[MAX_PATH];
    DWORD length = ((DWORD (WINAPI*)(LPCWSTR, LPWSTR, DWORD))address)(programFiles, path, MAX_PATH);
    if (!length || length + ARRAYSIZE(dllSuffix) > MAX_PATH)
    {
        return 0;
    }
    for (unsigned i = 0; i < ARRAYSIZE(dllSuffix); ++i)
    {
        path[length + i] = dllSuffix[i];
    }
    UNICODE_STRING dll = { (USHORT)((length + ARRAYSIZE(dllSuffix) - 1) * sizeof(WCHAR)), (USHORT)((length + ARRAYSIZE(dllSuffix)) * sizeof(WCHAR)), path };
    if (LdrLoadDll(NULL, NULL, &dll, &epModule) >= 0 &&
        LdrGetProcedureAddress(epModule, &entry, 0, &address) >= 0)
    {
        ((HRESULT (WINAPI*)(void))address)();
    }
    return 0;
}

BOOL WINAPI VerifierEntry(HINSTANCE instance, DWORD reason, PVOID reserved)
{
    if (reason == DLL_PROCESS_VERIFIER)
    {
        *(PROVIDER_DESCRIPTOR**)reserved = &descriptor;
    }
    else if (reason == DLL_PROCESS_ATTACH)
    {
        HANDLE thread = NULL;
        if (RtlCreateUserThread((HANDLE)(LONG_PTR)-1, NULL, FALSE, 0, 0, 0, InitializeSearch, NULL, &thread, NULL) >= 0 && thread)
        {
            NtClose(thread);
        }
    }
    return TRUE;
}

#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <stdio.h>

HMODULE hModule = NULL;
HANDLE sigFinish = NULL;
void* pFinishProc = NULL;

void done() {
    WaitForSingleObject(sigFinish, INFINITE);
    FreeLibraryAndExitThread(hModule, 0);
}

void* worker() {
    wchar_t pattern[MAX_PATH];
    GetWindowsDirectoryW(pattern, MAX_PATH);
    wcscat_s(pattern, MAX_PATH, L"\\ep_extra_*.dll");

    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFileW(pattern, &data);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            wprintf(L">> Found ep_extra library: \"%s\"\n", data.cFileName);
            GetWindowsDirectoryW(pattern, MAX_PATH);
            wcscat_s(pattern, MAX_PATH, L"\\");
            wcscat_s(pattern, MAX_PATH, data.cFileName);
            HMODULE hLib = LoadLibraryW(pattern);
            if (hLib) {
                FARPROC proc = (FARPROC)(GetProcAddress(hLib, "setup"));
                if (proc) {
                    if (!proc()) FreeLibrary(hLib);
                }
                else FreeLibrary(hLib);
            }
        } while (FindNextFileW(hFind, &data));
        FindClose(hFind);
    }

    sigFinish = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (sigFinish) {
        BYTE payload[] = {
            0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rcx, sigFinish
            0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, SetEvent
            0xFF, 0xD0, // call SetEvent
            0xC9, // leave
            0xC3  // ret
        };
        *(INT64*)(payload + 2) = sigFinish;
        *(INT64*)(payload + 12) = SetEvent;

        pFinishProc = VirtualAlloc(NULL, sizeof(payload), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (pFinishProc) {
            memcpy(pFinishProc, payload, sizeof(payload));
            SHCreateThread(done, 0, CTF_NOADDREFLIB, NULL);
            return pFinishProc;
        }
    }
    return NULL;
}

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        hModule = hinstDLL;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

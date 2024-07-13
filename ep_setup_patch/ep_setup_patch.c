#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include "../ExplorerPatcher/utility.h"

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
) 
{
    if (__argc < 3)
    {
        return __LINE__;
    }

    WCHAR wszMainModulePath[MAX_PATH];
    WCHAR wszSetupPath[MAX_PATH];
    wcscpy_s(wszMainModulePath, MAX_PATH, __wargv[1]);
    wcscpy_s(wszSetupPath, MAX_PATH, __wargv[2]);

    HMODULE hModule = LoadLibraryExW(wszMainModulePath, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hModule == NULL)
    {
        return __LINE__;
    }

    CHAR hash[100];
    ZeroMemory(hash, 100);
    ComputeFileHash2(hModule, wszMainModulePath, hash, 100);

    FreeLibrary(hModule);

    HANDLE hFile = CreateFileW(wszSetupPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return __LINE__;
    }

    HANDLE hFileMapping = CreateFileMappingW(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (!hFileMapping)
    {
        CloseHandle(hFile);
        return __LINE__;
    }

    char* lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!lpFileBase)
    {
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return __LINE__;
    }

    memcpy(lpFileBase + DOSMODE_OFFSET, hash, strlen(hash));

    UnmapViewOfFile(lpFileBase);
    CloseHandle(hFileMapping);
    CloseHandle(hFile);

    return 0;
}
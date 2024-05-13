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
    WCHAR wszPath[MAX_PATH];
    GetModuleFileNameW(GetModuleHandle(NULL), wszPath, MAX_PATH);
    PathRemoveFileSpecW(wszPath);
    wcscat_s(wszPath, MAX_PATH, L"\\" _T(PRODUCT_NAME) L".amd64.dll");
    HMODULE hModule = LoadLibraryExW(wszPath, NULL, LOAD_LIBRARY_AS_DATAFILE);

    CHAR hash[100];
    ZeroMemory(hash, 100);
    if (__argc > 1) 
    {
        for (size_t i = 0; i < MIN(wcslen(__wargv[1]), 32); ++i) 
        {
            hash[i] = __wargv[1][i];
        }
    }
    else 
    {
        ComputeFileHash2(hModule, wszPath, hash, 100);
        FreeLibrary(hModule);
    }

    PathRemoveFileSpecW(wszPath);
    wcscat_s(wszPath, MAX_PATH, L"\\" _T(SETUP_UTILITY_NAME));

    HANDLE hFile = CreateFileW(wszPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    HANDLE hFileMapping = CreateFileMappingW(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (hFileMapping == 0)
    {
        CloseHandle(hFile);
        return 2;
    }

    char* lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (lpFileBase == 0)
    {
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return 3;
    }

    memcpy(lpFileBase + DOSMODE_OFFSET, hash, strlen(hash));

    UnmapViewOfFile(lpFileBase);
    CloseHandle(hFileMapping);
    CloseHandle(hFile);

    if (__argc > 1)
    {
        SHELLEXECUTEINFO ShExecInfo = { 0 };
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = L"runas";
        ShExecInfo.lpFile = wszPath;
        ShExecInfo.lpParameters = NULL;
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_SHOW;
        ShExecInfo.hInstApp = NULL;
        if (ShellExecuteExW(&ShExecInfo) && ShExecInfo.hProcess)
        {
            WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
            DWORD dwExitCode = 0;
            GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
            CloseHandle(ShExecInfo.hProcess);
            return dwExitCode;
        }
    }

    return 0;
}
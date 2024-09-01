#include <Windows.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <stdio.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include "resources/resource.h"
#include "../ExplorerPatcher/utility.h"
#include "../version.h"
#include <zlib.h>
#include <minizip/unzip.h>
#ifdef WITH_ENCRYPTION
#include "rijndael-alg-fst.c" // Include the C file for __forceinline to work
#endif
#pragma comment(lib, "zlibstatic.lib")

BOOL SetupShortcut(BOOL bInstall, WCHAR* wszPath, WCHAR* wszArguments)
{
    WCHAR wszTitle[MAX_PATH];
    ZeroMemory(wszTitle, MAX_PATH);
    WCHAR wszExplorerPath[MAX_PATH];
    ZeroMemory(wszExplorerPath, MAX_PATH);
    GetSystemDirectoryW(wszExplorerPath, MAX_PATH);
    wcscat_s(wszExplorerPath, MAX_PATH, L"\\ExplorerFrame.dll");
    if (bInstall)
    {
        HMODULE hExplorerFrame = LoadLibraryExW(wszExplorerPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (hExplorerFrame)
        {
            LoadStringW(hExplorerFrame, 50222, wszTitle, 260); // 726 = File Explorer
            wchar_t* p = wcschr(wszTitle, L'(');
            if (p)
            {
                p--;
                if (*p == L' ')
                {
                    *p = 0;
                }
                else
                {
                    p++;
                    *p = 0;
                }
            }
            if (wszTitle[0] == 0)
            {
                wcscat_s(wszTitle, MAX_PATH, _T(PRODUCT_NAME));
            }
        }
        else
        {
            wcscat_s(wszTitle, MAX_PATH, _T(PRODUCT_NAME));
        }
    }
    BOOL bOk = FALSE;
    WCHAR wszStartPrograms[MAX_PATH + 1];
    ZeroMemory(wszStartPrograms, MAX_PATH + 1);
    SHGetFolderPathW(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, wszStartPrograms);
    wcscat_s(wszStartPrograms, MAX_PATH + 1, L"\\" _T(PRODUCT_NAME));
    wszStartPrograms[wcslen(wszStartPrograms) + 1] = 0;
    SHFILEOPSTRUCTW op;
    ZeroMemory(&op, sizeof(SHFILEOPSTRUCTW));
    op.wFunc = FO_DELETE;
    op.pFrom = wszStartPrograms;
    op.fFlags = FOF_NO_UI;
    bOk = SHFileOperationW(&op);
    bOk = !bOk;
    if (bInstall)
    {
        if (!CreateDirectoryW(wszStartPrograms, NULL))
        {
            return FALSE;
        }
    }
    else
    {
        return bOk;
    }
    wcscat_s(wszStartPrograms, MAX_PATH, L"\\");
    wcscat_s(wszStartPrograms, MAX_PATH, wszTitle);
    wcscat_s(wszStartPrograms, MAX_PATH, L" (");
    wcscat_s(wszStartPrograms, MAX_PATH, _T(PRODUCT_NAME) L").lnk");
    ZeroMemory(wszExplorerPath, MAX_PATH);
    GetSystemDirectoryW(wszExplorerPath, MAX_PATH);
    wcscat_s(wszExplorerPath, MAX_PATH, L"\\shell32.dll");
    if (bInstall)
    {
        if (SUCCEEDED(CoInitialize(0)))
        {
            IShellLinkW* pShellLink = NULL;
            if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC, &IID_IShellLinkW, &pShellLink)))
            {
                pShellLink->lpVtbl->SetPath(pShellLink, wszPath);
                pShellLink->lpVtbl->SetArguments(pShellLink, wszArguments);
                pShellLink->lpVtbl->SetIconLocation(pShellLink, wszExplorerPath, 40 - 1);
                PathRemoveFileSpecW(wszExplorerPath);
                pShellLink->lpVtbl->SetWorkingDirectory(pShellLink, wszExplorerPath);
                pShellLink->lpVtbl->SetDescription(pShellLink, _T(PRODUCT_NAME));

                IPersistFile* pPersistFile = NULL;
                if (SUCCEEDED(pShellLink->lpVtbl->QueryInterface(pShellLink, &IID_IPersistFile, &pPersistFile)))
                {
                    if (SUCCEEDED(pPersistFile->lpVtbl->Save(pPersistFile, wszStartPrograms, TRUE)))
                    {
                        bOk = TRUE;
                    }
                    pPersistFile->lpVtbl->Release(pPersistFile);
                }
                pShellLink->lpVtbl->Release(pShellLink);
            }
            CoUninitialize();
        }
    }
    return bOk;
}

BOOL SetupUninstallEntry(BOOL bInstall, WCHAR* wszPath)
{
    DWORD dwLastError = ERROR_SUCCESS;
    HKEY hKey = NULL;

    if (bInstall)
    {

        if (!dwLastError)
        {
            dwLastError = RegCreateKeyExW(
                HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" _T(EP_CLSID) L"_" _T(PRODUCT_NAME),
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE | KEY_WOW64_64KEY,
                NULL,
                &hKey,
                NULL
            );
            if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
            {
                hKey = NULL;
            }
            if (hKey)
            {
                if (!dwLastError)
                {
                    dwLastError = RegSetValueExW(
                        hKey,
                        L"UninstallString",
                        0,
                        REG_SZ,
                        (const BYTE*)wszPath,
                        (wcslen(wszPath) + 1) * sizeof(wchar_t)
                    );
                }
                if (!dwLastError)
                {
                    dwLastError = RegSetValueExW(
                        hKey,
                        L"DisplayName",
                        0,
                        REG_SZ,
                        (const BYTE*)_T(PRODUCT_NAME),
                        (wcslen(_T(PRODUCT_NAME)) + 1) * sizeof(wchar_t)
                    );
                }
                if (!dwLastError)
                {
                    dwLastError = RegSetValueExW(
                        hKey,
                        L"Publisher",
                        0,
                        REG_SZ,
                        (const BYTE*)_T(PRODUCT_PUBLISHER),
                        (wcslen(_T(PRODUCT_PUBLISHER)) + 1) * sizeof(wchar_t)
                    );
                }
                if (!dwLastError)
                {
                    DWORD dw1 = TRUE;
                    dwLastError = RegSetValueExW(
                        hKey,
                        L"NoModify",
                        0,
                        REG_DWORD,
                        (const BYTE*)&dw1,
                        sizeof(DWORD)
                    );
                }
                if (!dwLastError)
                {
                    DWORD dw1 = TRUE;
                    dwLastError = RegSetValueExW(
                        hKey,
                        L"NoRepair",
                        0,
                        REG_DWORD,
                        (const BYTE*)&dw1,
                        sizeof(DWORD)
                    );
                }
                if (!dwLastError)
                {
                    PathRemoveFileSpecW(wszPath + 1);
#if defined(_M_X64)
                    wcscat_s(wszPath + 1, MAX_PATH - 2, L"\\" _T(PRODUCT_NAME) L".amd64.dll");
#elif defined(_M_ARM64)
                    wcscat_s(wszPath + 1, MAX_PATH - 2, L"\\" _T(PRODUCT_NAME) L".arm64.dll");
#endif
                    HMODULE hEP = LoadLibraryExW(wszPath + 1, NULL, LOAD_LIBRARY_AS_DATAFILE);
                    if (hEP)
                    {
                        DWORD dwLeftMost = 0;
                        DWORD dwSecondLeft = 0;
                        DWORD dwSecondRight = 0;
                        DWORD dwRightMost = 0;

                        QueryVersionInfo(hEP, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

                        WCHAR wszBuf[30];
                        swprintf_s(wszBuf, 30, L"%d.%d.%d.%d", dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);

                        if (!dwLastError)
                        {
                            dwLastError = RegSetValueExW(
                                hKey,
                                L"DisplayVersion",
                                0,
                                REG_SZ,
                                (const BYTE*)wszBuf,
                                (wcslen(wszBuf) + 1) * sizeof(wchar_t)
                            );
                            if (!dwLastError)
                            {
                                dwLastError = RegSetValueExW(
                                    hKey,
                                    L"VersionMajor",
                                    0,
                                    REG_DWORD,
                                    (const BYTE*)&dwSecondRight,
                                    sizeof(DWORD)
                                );
                                if (!dwLastError)
                                {
                                    dwLastError = RegSetValueExW(
                                        hKey,
                                        L"VersionMinor",
                                        0,
                                        REG_DWORD,
                                        (const BYTE*)&dwRightMost,
                                        sizeof(DWORD)
                                    );
                                }
                            }
                        }

                        FreeLibrary(hEP);
                    }
                }
                if (!dwLastError)
                {
                    GetWindowsDirectoryW(wszPath, MAX_PATH);
                    wcscat_s(wszPath, MAX_PATH, L"\\explorer.exe");
                    dwLastError = RegSetValueExW(
                        hKey,
                        L"DisplayIcon",
                        0,
                        REG_SZ,
                        (const BYTE*)wszPath,
                        (DWORD)((wcslen(wszPath) + 1) * sizeof(wchar_t))
                    );
                }
                RegCloseKey(hKey);
            }
        }
    }
    else
    {
        if (!dwLastError)
        {
            dwLastError = RegOpenKeyW(
                HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" _T(EP_CLSID) L"_" _T(PRODUCT_NAME),
                &hKey
            );
            if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
            {
                hKey = NULL;
            }
            if (hKey)
            {
                dwLastError = RegDeleteTreeW(hKey, NULL);
                RegCloseKey(hKey);
            }
        }
     }
     return !dwLastError;
}

typedef struct
{
    PBYTE base;
    ZPOS64_T size;
    ZPOS64_T curOffset;
} MemoryBuffer;

void MemoryBuffer_Destroy(MemoryBuffer** mem)
{
    if (*mem)
    {
        if ((*mem)->base)
            free((*mem)->base);
        free(*mem);
        *mem = NULL;
    }
}

voidpf ZCALLBACK MemOpenFile(voidpf opaque, const void* filename, int mode)
{
    MemoryBuffer* pMem = (MemoryBuffer*)opaque;
    return pMem;
}

uLong ZCALLBACK MemReadFile(voidpf opaque, voidpf stream, void* buf, uLong size)
{
    MemoryBuffer* pMem = (MemoryBuffer*)stream;
    uLong toRead = size;

    if (pMem->curOffset + toRead > pMem->size)
    {
        toRead = (uLong)(pMem->size - pMem->curOffset);
    }

    if (toRead > 0)
    {
        memcpy(buf, pMem->base + pMem->curOffset, toRead);
        pMem->curOffset += toRead;
    }

    return toRead;
}

uLong ZCALLBACK MemWriteFile(voidpf opaque, voidpf stream, const void* buf, uLong size)
{
    return 0;
}

ZPOS64_T ZCALLBACK MemTellFile(voidpf opaque, voidpf stream)
{
    MemoryBuffer* pMem = (MemoryBuffer*)stream;
    return pMem->curOffset;
}

long ZCALLBACK MemSeekFile(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
{
    MemoryBuffer* pMem = (MemoryBuffer*)stream;
    ZPOS64_T newOffset;

    switch (origin)
    {
        case ZLIB_FILEFUNC_SEEK_CUR:
            newOffset = pMem->curOffset + offset;
            break;
        case ZLIB_FILEFUNC_SEEK_END:
            newOffset = pMem->size + offset;
            break;
        case ZLIB_FILEFUNC_SEEK_SET:
            newOffset = offset;
            break;
        default:
            return -1;
    }

    if (newOffset > pMem->size)
    {
        return -1;
    }

    pMem->curOffset = newOffset;
    return 0;
}

int ZCALLBACK MemCloseFile(voidpf opaque, voidpf stream)
{
    return 0;
}

int ZCALLBACK MemErrorFile(voidpf opaque, voidpf stream)
{
    return 0;
}

void FillMemoryFileIOFunctions(zlib_filefunc64_def* pFileFunc, MemoryBuffer* pMem)
{
    pFileFunc->zopen64_file = MemOpenFile;
    pFileFunc->zread_file = MemReadFile;
    pFileFunc->zwrite_file = MemWriteFile;
    pFileFunc->ztell64_file = MemTellFile;
    pFileFunc->zseek64_file = MemSeekFile;
    pFileFunc->zclose_file = MemCloseFile;
    pFileFunc->zerror_file = MemErrorFile;
    pFileFunc->opaque = pMem;
}

#define AES_KEYBITS				256

#define KEYLENGTH( keybits )		( ( keybits ) / 8 )
#define RKLENGTH( keybits )		( ( keybits ) / 8 + 28 )
#define NROUNDS( keybits )			( ( keybits ) / 32 + 6 )

#if defined(WITH_ENCRYPTION) && !defined(ZIP_ENCRYPTION_KEY)
#define ZIP_ENCRYPTION_KEY 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
#endif

unzFile LoadZipFileFromResources(MemoryBuffer** outMem)
{
    *outMem = NULL;

    HRSRC hRsrc = FindResourceW(NULL, MAKEINTRESOURCE(IDR_EP_ZIP), RT_RCDATA);
    if (!hRsrc)
    {
        return NULL;
    }

    HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
    if (!hGlobal)
    {
        return NULL;
    }

    PBYTE pRsrc = (PBYTE)LockResource(hGlobal);
    DWORD cbRsrc = SizeofResource(NULL, hRsrc);
    if (!pRsrc || !cbRsrc)
    {
        return NULL;
    }

#ifdef WITH_ENCRYPTION
    if ((cbRsrc % 16) != 0)
    {
        return NULL;
    }
#endif

    MemoryBuffer* pMem = (MemoryBuffer*)malloc(sizeof(MemoryBuffer));
    if (!pMem)
    {
        return NULL;
    }

    pMem->base = (PBYTE)malloc(cbRsrc);
    pMem->size = cbRsrc;
    pMem->curOffset = 0;
    if (!pMem->base)
    {
        free(pMem);
        return NULL;
    }

    *outMem = pMem;

#ifdef WITH_ENCRYPTION
    BYTE keyBytes[32] = { ZIP_ENCRYPTION_KEY };

    UINT rk[RKLENGTH(AES_KEYBITS)] = { 0 };
    int nrounds = rijndaelKeySetupDec(rk, keyBytes, AES_KEYBITS);

    // Decrypt the data a block at a time
    for (UINT offset = 0; offset < cbRsrc; offset += 16)
    {
        rijndaelDecrypt(rk, nrounds, pRsrc + offset, pMem->base + offset);
    }
#else
    memcpy(pMem->base, pRsrc, cbRsrc);
#endif

    zlib_filefunc64_def fileFunc = { 0 };
    FillMemoryFileIOFunctions(&fileFunc, pMem);

    return unzOpen2_64(NULL, &fileFunc);
}

int g_cleanupFileCounter = 1;

// %APPDATA%\ExplorerPatcher\cleanup\<PID>_<counter>.tmp
BOOL StageFileForCleanup(const WCHAR* wszProblematicFilePath)
{
    WCHAR wszPath[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wszPath);
    wcscat_s(wszPath, MAX_PATH, L"\\ExplorerPatcher\\cleanup");
    CreateDirectoryW(wszPath, NULL);

    wcscat_s(wszPath, MAX_PATH, L"\\");
    WCHAR wszPID[10];
    _itow_s(GetCurrentProcessId(), wszPID, ARRAYSIZE(wszPID), 10);
    wcscat_s(wszPath, MAX_PATH, wszPID);
    wcscat_s(wszPath, MAX_PATH, L"_");
    WCHAR wszCounter[10];
    _itow_s(g_cleanupFileCounter++, wszCounter, ARRAYSIZE(wszCounter), 10);
    wcscat_s(wszPath, MAX_PATH, wszCounter);
    wcscat_s(wszPath, MAX_PATH, L".tmp");

    return MoveFileW(wszProblematicFilePath, wszPath);
}

__declspec(noinline) BOOL InstallResourceHelper(BOOL bInstall, HMODULE hModule, unzFile zipFile, const WCHAR* wszPath)
{
    WCHAR wszReplace[MAX_PATH];
    wcscpy_s(wszReplace, MAX_PATH, wszPath);
    PathRemoveExtensionW(wszReplace);
    wcscat_s(wszReplace, MAX_PATH, L".prev");
    BOOL bFileExists = PathFileExistsW(wszPath);
    BOOL bPrevExists = PathFileExistsW(wszReplace);
    if (bFileExists || bPrevExists)
    {
        BOOL bRet = !bPrevExists || DeleteFileW(wszReplace);
        if (bRet || (!bRet && GetLastError() == ERROR_FILE_NOT_FOUND))
        {
            if (bFileExists && !DeleteFileW(wszPath) && !StageFileForCleanup(wszPath))
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    if (!zipFile)
    {
        if (bInstall)
        {
            wchar_t path[MAX_PATH];
            GetModuleFileNameW(hModule, path, MAX_PATH);
            return CopyFileW(path, wszPath, FALSE);
        }
        return TRUE;
    }

    if (!bInstall)
    {
        return TRUE;
    }

    unz_file_info64 fileInfo = { 0 };
    // Caller (InstallResource) has already called unzOpenCurrentFile
    if (unzGetCurrentFileInfo64(zipFile, &fileInfo, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
    {
        return FALSE;
    }

    BOOL bRet = FALSE;
    void* pRscr = malloc(fileInfo.uncompressed_size);
    DWORD cbRscr = (DWORD)fileInfo.uncompressed_size;
    if (pRscr)
    {
        if (unzReadCurrentFile(zipFile, pRscr, cbRscr) == cbRscr)
        {
            HANDLE hFile = CreateFileW(wszPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile)
            {
                DWORD dwNumberOfBytesWritten = 0;
                int offset = 0;
                wchar_t wszDxgi[MAX_PATH];
                if (GetWindowsDirectoryW(wszDxgi, MAX_PATH))
                {
                    wcscat_s(wszDxgi, MAX_PATH, L"\\dxgi.dll");
                    if (!wcscmp(wszPath, wszDxgi))
                    {
                        WCHAR wszOwnPath[MAX_PATH];
                        GetModuleFileNameW(GetModuleHandle(NULL), wszOwnPath, MAX_PATH);
                        CHAR hash[100];
                        GetHardcodedHash(wszOwnPath, hash, 100);
                        WriteFile(hFile, pRscr, DOSMODE_OFFSET, &dwNumberOfBytesWritten, NULL);
                        offset += dwNumberOfBytesWritten;
                        WriteFile(hFile, hash, 32, &dwNumberOfBytesWritten, NULL);
                        offset += dwNumberOfBytesWritten;
                    }
                }
                bRet = WriteFile(hFile, (char*)pRscr + offset, cbRscr - offset, &dwNumberOfBytesWritten, NULL);
                CloseHandle(hFile);
            }
        }
        free(pRscr);
    }
    // Caller (InstallResource) will call unzCloseCurrentFile
    return bRet;
}

__declspec(noinline) BOOL InstallResource(BOOL bInstall, HMODULE hInstance, unzFile zipFile, const char* pszFileNameInZip, LPCWSTR pwszDirectory, LPCWSTR pwszFileName)
{
    if (bInstall && zipFile && pszFileNameInZip)
    {
        int resultLocateFile = unzLocateFile(zipFile, pszFileNameInZip, 0);
        if (resultLocateFile != UNZ_OK)
        {
            return resultLocateFile == UNZ_END_OF_LIST_OF_FILE; // Don't touch this file, we don't pack this file in the setup
        }

        if (unzOpenCurrentFile(zipFile) != UNZ_OK)
        {
            return FALSE;
        }
    }

    WCHAR wszPath[MAX_PATH];
    wcscpy_s(wszPath, MAX_PATH, pwszDirectory);
    wcscat_s(wszPath, MAX_PATH, L"\\");
    wcscat_s(wszPath, MAX_PATH, pwszFileName);
    BOOL bRet = InstallResourceHelper(bInstall, hInstance, zipFile, wszPath);
    if (bInstall && zipFile && pszFileNameInZip)
        unzCloseCurrentFile(zipFile);
    return bRet;
}

const WCHAR* GetSystemLanguages()
{
    wchar_t* wszLanguagesBuffer = NULL;
    ULONG ulNumLanguages = 0;
    ULONG cchLanguagesBuffer = 0;
    if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &ulNumLanguages, NULL, &cchLanguagesBuffer))
    {
        wszLanguagesBuffer = (wchar_t*)malloc(cchLanguagesBuffer * sizeof(wchar_t));
        if (wszLanguagesBuffer)
        {
            if (!GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &ulNumLanguages, wszLanguagesBuffer, &cchLanguagesBuffer))
            {
                free(wszLanguagesBuffer);
                wszLanguagesBuffer = NULL;
            }
        }
    }
    return wszLanguagesBuffer ? wszLanguagesBuffer : L"en-US";
}

BOOL SystemHasLanguageInstalled(const WCHAR* languages, const char* langCode, int cchLangCode)
{
    WCHAR szLangCode[100];
    MultiByteToWideChar(CP_UTF8, 0, langCode, cchLangCode, szLangCode, ARRAYSIZE(szLangCode));
    szLangCode[cchLangCode] = 0;
    for (const WCHAR* wszLang = languages; *wszLang; wszLang += wcslen(wszLang) + 1)
    {
        if (!_wcsicmp(wszLang, szLangCode))
        {
            return TRUE;
        }
    }
    return FALSE;
}

typedef enum LanguageCodeTreatment
{
    LCT_None,
    LCT_MUI, // module\en-US\module.dll.pri
    LCT_PRI, // resource\pris\resource.en-US.pri
} LanguageCodeTreatment;

__declspec(noinline) BOOL ExtractDirectory(unzFile zipFile, const char* dirNameInZip, LPCWSTR pwszDirectory, const WCHAR* languages, LanguageCodeTreatment langCodeTreatment)
{
    if (!zipFile)
    {
        return FALSE;
    }

    if (unzGoToFirstFile(zipFile) != UNZ_OK)
    {
        return FALSE;
    }

    BOOL bRet = TRUE;
    size_t dirNameLen = dirNameInZip ? strlen(dirNameInZip) : 0;

    do
    {
        char szFileNameInZip[260];
        unz_file_info64 fileInfo = { 0 };
        if (unzGetCurrentFileInfo64(zipFile, &fileInfo, szFileNameInZip, ARRAYSIZE(szFileNameInZip), NULL, 0, NULL, 0) != UNZ_OK)
        {
            return FALSE;
        }
        szFileNameInZip[fileInfo.size_filename] = 0;

        if (fileInfo.uncompressed_size == 0 || (fileInfo.external_fa & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            continue;
        }

        if (dirNameInZip && strncmp(szFileNameInZip, dirNameInZip, dirNameLen) != 0)
        {
            continue;
        }

        // Examples:
        // - "module/en-US/module.dll.mui" -> "en-US/module.dll.mui"
        // - "resource/pris/resource.en-US.pri" -> "pris/resource.en-US.pri"
        const char* filePathInDir = szFileNameInZip + dirNameLen;
        const char* lastSlash = strrchr(filePathInDir, '/');
        const char* fileName = lastSlash ? filePathInDir + (lastSlash - filePathInDir) + 1 : filePathInDir;
        const char* lastDot = strrchr(fileName, '.');
        const char* fileExt = lastDot ? fileName + (lastDot - fileName) + 1 : NULL;
        if (langCodeTreatment == LCT_MUI)
        {
            if (fileExt && !_stricmp(fileExt, "mui"))
            {
                if (!SystemHasLanguageInstalled(languages, filePathInDir, strchr(filePathInDir, '/') - filePathInDir))
                {
                    continue;
                }
            }
        }
        else if (langCodeTreatment == LCT_PRI)
        {
            if (fileExt && !_stricmp(fileExt, "pri") && strchr(fileName, '-') != NULL)
            {
                // Check if we're a language pri
                const char* secondLastDot = NULL;
                for (const char* p = lastDot - 1; p >= fileName; p--)
                {
                    if (*p == '.')
                    {
                        secondLastDot = p;
                        break;
                    }
                }
                if (secondLastDot != lastDot)
                {
                    const char* langCode = secondLastDot + 1;
                    if (!SystemHasLanguageInstalled(languages, langCode, lastDot - langCode))
                    {
                        continue;
                    }
                }
            }
        }

        if (unzOpenCurrentFile(zipFile) != UNZ_OK)
        {
            return FALSE;
        }

        WCHAR wszFileNameInZip[MAX_PATH];
        MultiByteToWideChar(CP_UTF8, 0, szFileNameInZip, -1, wszFileNameInZip, MAX_PATH);
        for (size_t i = 0; i < MAX_PATH && wszFileNameInZip[i] != 0; i++)
        {
            if (wszFileNameInZip[i] == '/')
            {
                wszFileNameInZip[i] = '\\';
            }
        }

        WCHAR wszPath[MAX_PATH];
        wcscpy_s(wszPath, MAX_PATH, pwszDirectory);
        wcscat_s(wszPath, MAX_PATH, L"\\");
        WCHAR* pwszPathInDir = wszPath + wcslen(wszPath);
        if (dirNameInZip)
        {
            wcscat_s(wszPath, MAX_PATH, wcschr(wszFileNameInZip, '\\') + 1); // Skip the directory name in the zip file
        }
        else
        {
            wcscat_s(wszPath, MAX_PATH, wszFileNameInZip);
        }

        for (WCHAR* p = pwszPathInDir; *p; p++)
        {
            if (*p == '\\')
            {
                *p = 0;
                CreateDirectoryW(wszPath, NULL);
                *p = '\\';
            }
        }

        bRet = InstallResourceHelper(TRUE, NULL, zipFile, wszPath);

        unzCloseCurrentFile(zipFile);
    } while (bRet && unzGoToNextFile(zipFile) == UNZ_OK);

    return bRet;
}

BOOL DeleteResource(LPCWSTR pwszDirectory, LPCWSTR pwszFileName)
{
    WCHAR wszPath[MAX_PATH];
    wcscpy_s(wszPath, MAX_PATH, pwszDirectory);
    wcscat_s(wszPath, MAX_PATH, L"\\");
    wcscat_s(wszPath, MAX_PATH, pwszFileName);
    return InstallResourceHelper(FALSE, NULL, NULL, wszPath);
}

/*BOOL ShouldDownloadOrDelete(BOOL bInstall, WCHAR* wszPath, LPCSTR chash)
{
    if (FileExistsW(wszPath))
    {
        if (bInstall)
        {
            char hash[100];
            ZeroMemory(hash, sizeof(char) * 100);
            ComputeFileHash(wszPath, hash, 100);
            bInstall = _stricmp(hash, chash) != 0;
        }
        else
        {
            InstallResourceHelper(FALSE, NULL, NULL, wszPath); // Delete
        }
    }

    return bInstall;
}

BOOL DownloadResource(BOOL bInstall, LPCWSTR pwszURL, DWORD dwSize, LPCSTR chash, LPCWSTR pwszDirectory, LPCWSTR pwszFileName)
{
    BOOL bOk = TRUE;
    WCHAR wszPath[MAX_PATH];
    wcscpy_s(wszPath, MAX_PATH, pwszDirectory);
    wcscat_s(wszPath, MAX_PATH, L"\\");
    wcscat_s(wszPath, MAX_PATH, pwszFileName);
    if (ShouldDownloadOrDelete(bInstall, wszPath, chash) && IsConnectedToInternet() == TRUE)
    {
        bOk = DownloadFile(pwszURL, dwSize, wszPath);
    }
    return bOk;
}*/

void ProcessTaskbarDlls(BOOL* bInOutOk, BOOL bInstall, BOOL bExtractMode, HINSTANCE hInstance, unzFile zipFile, WCHAR wszPath[260])
{
    LPCWSTR pwszTaskbarDllName = bExtractMode ? NULL : PickTaskbarDll();
    if (*bInOutOk) *bInOutOk = InstallResource(bInstall && (bExtractMode || pwszTaskbarDllName && !wcscmp(pwszTaskbarDllName, L"ep_taskbar.0.dll")), hInstance, zipFile, "ep_taskbar.0.dll", wszPath, L"ep_taskbar.0.dll");
    if (*bInOutOk) *bInOutOk = InstallResource(bInstall && (bExtractMode || pwszTaskbarDllName && !wcscmp(pwszTaskbarDllName, L"ep_taskbar.2.dll")), hInstance, zipFile, "ep_taskbar.2.dll", wszPath, L"ep_taskbar.2.dll");
    if (*bInOutOk) *bInOutOk = InstallResource(bInstall && (bExtractMode || pwszTaskbarDllName && !wcscmp(pwszTaskbarDllName, L"ep_taskbar.3.dll")), hInstance, zipFile, "ep_taskbar.3.dll", wszPath, L"ep_taskbar.3.dll");
    if (*bInOutOk) *bInOutOk = InstallResource(bInstall && (bExtractMode || pwszTaskbarDllName && !wcscmp(pwszTaskbarDllName, L"ep_taskbar.4.dll")), hInstance, zipFile, "ep_taskbar.4.dll", wszPath, L"ep_taskbar.4.dll");
    if (*bInOutOk) *bInOutOk = InstallResource(bInstall && (bExtractMode || pwszTaskbarDllName && !wcscmp(pwszTaskbarDllName, L"ep_taskbar.5.dll")), hInstance, zipFile, "ep_taskbar.5.dll", wszPath, L"ep_taskbar.5.dll");
}

BOOL RemoveDirectoryRecursive(const WCHAR* wszDirectoryPath)
{
    WCHAR szDir[MAX_PATH];
    wcscpy_s(szDir, MAX_PATH, wszDirectoryPath);
    wcscat_s(szDir, MAX_PATH, L"\\*");

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFileW(szDir, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return TRUE;
    }

    do
    {
        if (lstrcmpW(findFileData.cFileName, L".") != 0 && lstrcmpW(findFileData.cFileName, L"..") != 0)
        {
            WCHAR szFilePath[MAX_PATH];
            wcscpy_s(szFilePath, MAX_PATH, wszDirectoryPath);
            wcscat_s(szFilePath, MAX_PATH, L"\\");
            wcscat_s(szFilePath, MAX_PATH, findFileData.cFileName);

            if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                if (!RemoveDirectoryRecursive(szFilePath))
                {
                    FindClose(hFind);
                    return FALSE;
                }
            }
            else
            {
                if (!DeleteFileW(szFilePath) && !StageFileForCleanup(szFilePath))
                {
                    FindClose(hFind);
                    return FALSE;
                }
            }
        }
    }
    while (FindNextFileW(hFind, &findFileData));

    DWORD dwError = GetLastError();
    FindClose(hFind);

    if (dwError != ERROR_NO_MORE_FILES)
    {
        return FALSE;
    }

    return RemoveDirectoryW(wszDirectoryPath);
}


int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    global_ubr = VnGetOSVersionAndUBR(&global_rovi);

    BOOL bOk = TRUE, bInstall = TRUE, bWasShellExt = FALSE, bIsUpdate = FALSE, bForcePromptForUninstall = FALSE;

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    int argc = 0;
    LPWSTR* wargv = CommandLineToArgvW(
        lpCmdLine,
        &argc
    );

    WCHAR wszPath[MAX_PATH];
    ZeroMemory(wszPath, MAX_PATH * sizeof(WCHAR));

    if (argc >= 1 && !_wcsicmp(wargv[0], L"/extract"))
    {
        if (argc >= 2)
        {
            wcsncpy_s(wszPath, MAX_PATH, wargv[1], MAX_PATH);
            CreateDirectoryW(wargv[1], NULL);
        }
        else
        {
            GetCurrentDirectoryW(MAX_PATH, wszPath);
        }
        MemoryBuffer* pMem;
        unzFile zipFile = LoadZipFileFromResources(&pMem);
        bOk = zipFile != NULL;
        if (bOk)
        {
            bOk = ExtractDirectory(zipFile, NULL, wszPath, NULL, LCT_None);
        }
        if (zipFile)
            unzClose(zipFile);
        if (pMem)
            MemoryBuffer_Destroy(&pMem);
        return !bOk;
    }

#if defined(_M_X64)
    typedef BOOL (WINAPI *IsWow64Process2_t)(HANDLE hProcess, USHORT* pProcessMachine, USHORT* pNativeMachine);
    IsWow64Process2_t pfnIsWow64Process2 = (IsWow64Process2_t)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "IsWow64Process2");
    if (pfnIsWow64Process2)
    {
        USHORT processMachine, nativeMachine;
        if (pfnIsWow64Process2(GetCurrentProcess(), &processMachine, &nativeMachine))
        {
            if (nativeMachine == IMAGE_FILE_MACHINE_ARM64)
            {
                WCHAR szFormat[256];
                szFormat[0] = 0;
                int written = LoadStringW(hInstance, IDS_SETUP_UNSUPPORTED_ARCH, szFormat, ARRAYSIZE(szFormat));
                if (written > 0 && written < ARRAYSIZE(szFormat))
                {
                    WCHAR szMessage[256];
                    szMessage[0] = 0;
                    _swprintf_p(szMessage, ARRAYSIZE(szMessage), szFormat, L"ARM64", L"x64");
                    MessageBoxW(NULL, szMessage, _T(PRODUCT_NAME), MB_OK | MB_ICONERROR);
                }
                exit(0);
            }
        }
    }
#endif

    WCHAR wszOwnPath[MAX_PATH];
    ZeroMemory(wszOwnPath, ARRAYSIZE(wszOwnPath));
    if (!GetModuleFileNameW(NULL, wszOwnPath, ARRAYSIZE(wszOwnPath)))
    {
        exit(0);
    }

    bInstall = !(argc >= 1 && (!_wcsicmp(wargv[0], L"/uninstall") || !_wcsicmp(wargv[0], L"/uninstall_silent")));
    PathStripPathW(wszOwnPath);
    if (!_wcsicmp(wszOwnPath, L"ep_uninstall.exe"))
    {
        bInstall = FALSE;
        bForcePromptForUninstall = _wcsicmp(wargv[0], L"/uninstall_silent");
    }
    if (!GetModuleFileNameW(NULL, wszOwnPath, ARRAYSIZE(wszOwnPath)))
    {
        exit(0);
    }
    bIsUpdate = (argc >= 1 && !_wcsicmp(wargv[0], L"/update_silent"));
    if (!bInstall && (!_wcsicmp(wargv[0], L"/uninstall") || bForcePromptForUninstall))
    {
        wchar_t mbText[256];
        mbText[0] = 0;
        LoadStringW(hInstance, IDS_SETUP_UNINSTALL_PROMPT, mbText, ARRAYSIZE(mbText));
        if (MessageBoxW(NULL, mbText, _T(PRODUCT_NAME), MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDNO)
        {
            exit(0);
        }
    }

    if (!IsAppRunningAsAdminMode())
    {
        SHELLEXECUTEINFOW sei;
        ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
        sei.cbSize = sizeof(sei);
        sei.lpVerb = L"runas";
        sei.lpFile = wszOwnPath;
        sei.lpParameters = !bInstall ? L"/uninstall_silent" : lpCmdLine;
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;
        if (!ShellExecuteExW(&sei))
        {
            DWORD dwError = GetLastError();
            if (dwError == ERROR_CANCELLED)
            {
            }
        }
        exit(0);
    }

    MemoryBuffer* pMem = NULL;
    unzFile zipFile = NULL;
    if (bInstall)
    {
        zipFile = LoadZipFileFromResources(&pMem);
        if (!zipFile)
        {
            exit(0);
        }
    }

    DWORD bIsUndockingDisabled = FALSE, dwSize = sizeof(DWORD);
    RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell\\Update\\Packages", L"UndockingDisabled", RRF_RT_DWORD, NULL, &bIsUndockingDisabled, &dwSize);
    if (bIsUndockingDisabled)
    {
        wchar_t mbText[256];
        mbText[0] = 0;
        LoadStringW(hInstance, bInstall ? IDS_SETUP_INSTALL_LOGOFF : IDS_SETUP_UNINSTALL_LOGOFF, mbText, ARRAYSIZE(mbText));
        if (MessageBoxW(NULL, mbText, _T(PRODUCT_NAME), MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION) == IDYES)
        {
            RegDeleteKeyValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell\\Update\\Packages", L"UndockingDisabled");
        }
        else
        {
            exit(0);
        }
    }

    CreateEventW(NULL, FALSE, FALSE, _T(EP_SETUP_EVENTNAME));

    SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
    wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH));
    bOk = CreateDirectoryW(wszPath, NULL);
    if (bOk || (!bOk && GetLastError() == ERROR_ALREADY_EXISTS))
    {
        bOk = TRUE;
        HANDLE userToken = INVALID_HANDLE_VALUE;

        HWND hShellTrayWnd = FindWindowW(L"Shell_TrayWnd", NULL);
        if (hShellTrayWnd)
        {
            DWORD explorerProcessId = 0;
            GetWindowThreadProcessId(hShellTrayWnd, &explorerProcessId);
            if (explorerProcessId != 0)
            {
                HANDLE explorerProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, explorerProcessId);
                if (explorerProcess != NULL)
                {
                    OpenProcessToken(explorerProcess, TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, &userToken);
                    CloseHandle(explorerProcess);
                }
                if (userToken)
                {
                    HANDLE myToken = INVALID_HANDLE_VALUE;
                    OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, &myToken);
                    if (myToken != INVALID_HANDLE_VALUE)
                    {
                        DWORD cbSizeNeeded = 0;
                        SetLastError(0);
                        if (!GetTokenInformation(userToken, TokenUser, NULL, 0, &cbSizeNeeded) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                        {
                            TOKEN_USER* userTokenInfo = malloc(cbSizeNeeded);
                            if (userTokenInfo)
                            {
                                if (GetTokenInformation(userToken, TokenUser, userTokenInfo, cbSizeNeeded, &cbSizeNeeded))
                                {
                                    cbSizeNeeded = 0;
                                    SetLastError(0);
                                    if (!GetTokenInformation(myToken, TokenUser, NULL, 0, &cbSizeNeeded) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                                    {
                                        TOKEN_USER* myTokenInfo = malloc(cbSizeNeeded);
                                        if (myTokenInfo)
                                        {
                                            if (GetTokenInformation(myToken, TokenUser, myTokenInfo, cbSizeNeeded, &cbSizeNeeded))
                                            {
                                                if (EqualSid(userTokenInfo->User.Sid, myTokenInfo->User.Sid))
                                                {
                                                    CloseHandle(userToken);
                                                    userToken = INVALID_HANDLE_VALUE;
                                                }
                                            }
                                            free(myTokenInfo);
                                        }
                                    }
                                }
                                free(userTokenInfo);
                            }
                        }
                        CloseHandle(myToken);
                    }
                }
            }
            DWORD_PTR res = -1;
            if (!SendMessageTimeoutW(hShellTrayWnd, 1460, 0, 0, SMTO_ABORTIFHUNG, 2000, &res) && res)
            {
                HANDLE hExplorerRestartThread = CreateThread(NULL, 0, BeginExplorerRestart, NULL, 0, NULL);
                if (hExplorerRestartThread)
                {
                    WaitForSingleObject(hExplorerRestartThread, 2000);
                    CloseHandle(hExplorerRestartThread);
                    hExplorerRestartThread = NULL;
                }
                else
                {
                    BeginExplorerRestart(NULL);
                }
            }
        }
        Sleep(100);
        GetSystemDirectoryW(wszPath, MAX_PATH);
        wcscat_s(wszPath, MAX_PATH, L"\\taskkill.exe");
        SHELLEXECUTEINFOW sei;
        ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.hwnd = NULL;
        sei.hInstApp = NULL;
        sei.lpVerb = NULL;
        sei.lpFile = wszPath;
        sei.lpParameters = L"/f /im explorer.exe";
        sei.hwnd = NULL;
        sei.nShow = SW_SHOWMINIMIZED;
        if (ShellExecuteExW(&sei) && sei.hProcess)
        {
            WaitForSingleObject(sei.hProcess, INFINITE);
            CloseHandle(sei.hProcess);
        }

        Sleep(500);

        BOOL bAreRoundedCornersDisabled = FALSE;
        HANDLE h_exists = CreateEventW(NULL, FALSE, FALSE, L"Global\\ep_dwm_" _T(EP_CLSID));
        if (h_exists)
        {
            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                bAreRoundedCornersDisabled = TRUE;
            }
            else
            {
                bAreRoundedCornersDisabled = FALSE;
            }
            CloseHandle(h_exists);
        }
        else
        {
            if (GetLastError() == ERROR_ACCESS_DENIED)
            {
                bAreRoundedCornersDisabled = TRUE;
            }
            else
            {
                bAreRoundedCornersDisabled = FALSE;
            }
        }
        if (bAreRoundedCornersDisabled)
        {
            RegisterDWMService(0, 1);
            RegisterDWMService(0, 3);
        }

        WCHAR wszSCPath[MAX_PATH];
        GetSystemDirectoryW(wszSCPath, MAX_PATH);
        wcscat_s(wszSCPath, MAX_PATH, L"\\sc.exe");
        SHELLEXECUTEINFO ShExecInfo = { 0 };
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = L"runas";
        ShExecInfo.lpFile = wszSCPath;
        ShExecInfo.lpParameters = L"stop " _T(EP_DWM_SERVICENAME);
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_HIDE;
        ShExecInfo.hInstApp = NULL;
        if (ShellExecuteExW(&ShExecInfo) && ShExecInfo.hProcess)
        {
            WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
            DWORD dwExitCode = 0;
            GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
            CloseHandle(ShExecInfo.hProcess);
        }

        HWND hWnd = FindWindowW(L"ExplorerPatcher_GUI_" _T(EP_CLSID), NULL);
        if (hWnd)
        {
            DWORD dwGUIPid = 0;
            GetWindowThreadProcessId(hWnd, &dwGUIPid);
            if (dwGUIPid)
            {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwGUIPid);
                if (hProcess)
                {
                    DWORD dwSection = (DWORD)SendMessageW(hWnd, WM_MSG_GUI_SECTION, WM_MSG_GUI_SECTION_GET, 0);

                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);

                    HKEY hKey = NULL;

                    RegCreateKeyExW(
                        HKEY_CURRENT_USER,
                        TEXT(REGPATH),
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WOW64_64KEY | KEY_WRITE,
                        NULL,
                        &hKey,
                        NULL
                    );
                    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
                    {
                        hKey = NULL;
                    }
                    if (hKey)
                    {
                        RegSetValueExW(
                            hKey,
                            TEXT("OpenPropertiesAtNextStart"),
                            0,
                            REG_DWORD,
                            (const BYTE*)&dwSection,
                            sizeof(DWORD)
                        );
                        RegCloseKey(hKey);
                    }
                }
            }
        }

        Sleep(1000);

        // --------------------------------------------------------------------------------

        // C:\Program Files\ExplorerPatcher
        SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
        wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH));
        if (bOk && bInstall) bOk = InstallResource(bInstall, hInstance, NULL, NULL, wszPath, _T(SETUP_UTILITY_NAME));
        if (bOk)
        {
            if (!bInstall)
            {
                HKEY hKey;
                RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Classes\\CLSID\\" TEXT(EP_CLSID) L"\\InProcServer32",
                    REG_OPTION_NON_VOLATILE,
                    KEY_READ,
                    &hKey
                );
                if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
                {
                    hKey = NULL;
                }
                if (hKey)
                {
                    bWasShellExt = TRUE;
                    RegCloseKey(hKey);
                }
                if (bWasShellExt)
                {
                    WCHAR wszArgs[MAX_PATH];
                    wszArgs[0] = L'/';
                    wszArgs[1] = L'u';
                    wszArgs[2] = L' ';
                    wszArgs[3] = L'"';
                    SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszArgs + 4);
#if defined(_M_X64)
                    wcscat_s(wszArgs, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\" _T(PRODUCT_NAME) L".amd64.dll\"");
#elif defined(_M_ARM64)
                    wcscat_s(wszArgs, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\" _T(PRODUCT_NAME) L".arm64.dll\"");
#endif
                    wprintf(L"%s\n", wszArgs);
                    WCHAR wszApp[MAX_PATH * 2];
                    GetSystemDirectoryW(wszApp, MAX_PATH * 2);
                    wcscat_s(wszApp, MAX_PATH * 2, L"\\regsvr32.exe");
                    wprintf(L"%s\n", wszApp);
                    SHELLEXECUTEINFOW sei;
                    ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
                    sei.cbSize = sizeof(sei);
                    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
                    sei.hwnd = NULL;
                    sei.hInstApp = NULL;
                    sei.lpVerb = NULL;
                    sei.lpFile = wszApp;
                    sei.lpParameters = wszArgs;
                    sei.hwnd = NULL;
                    sei.nShow = SW_NORMAL;
                    if (ShellExecuteExW(&sei) && sei.hProcess)
                    {
                        WaitForSingleObject(sei.hProcess, INFINITE);
                        DWORD dwExitCode = 0;
                        GetExitCodeProcess(sei.hProcess, &dwExitCode);
                        SetLastError(dwExitCode);
                        CloseHandle(sei.hProcess);
                    }
                }
            }
        }
        if (bOk) bOk = InstallResource(bInstall, hInstance, zipFile, PRODUCT_NAME ".IA-32.dll", wszPath, _T(PRODUCT_NAME) L".IA-32.dll");
#if defined(_M_X64)
        if (bOk) bOk = InstallResource(bInstall, hInstance, zipFile, PRODUCT_NAME ".amd64.dll", wszPath, _T(PRODUCT_NAME) L".amd64.dll");
#elif defined(_M_ARM64)
        if (bOk) bOk = InstallResource(bInstall, hInstance, zipFile, PRODUCT_NAME ".arm64.dll", wszPath, _T(PRODUCT_NAME) L".arm64.dll");
#endif
        if (bOk) bOk = InstallResource(bInstall, hInstance, zipFile, "ep_gui.dll", wszPath, L"ep_gui.dll");
        if (bOk) bOk = InstallResource(bInstall, hInstance, zipFile, "ep_dwm.exe", wszPath, L"ep_dwm.exe");
        if (bInstall)
        {
            if (bOk) bOk = InstallResource(bInstall, hInstance, zipFile, "ep_weather_host.dll", wszPath, L"ep_weather_host.dll");
            if (bOk) bOk = InstallResource(bInstall, hInstance, zipFile, "ep_weather_host_stub.dll", wszPath, L"ep_weather_host_stub.dll");
            if (bOk) bOk = InstallResource(bInstall, hInstance, zipFile, "WebView2Loader.dll", wszPath, L"WebView2Loader.dll");
        }
        ProcessTaskbarDlls(&bOk, bInstall, FALSE, hInstance, zipFile, wszPath);
        const WCHAR* possibleDirs[] =
        {
            L"ar-SA", L"bg-BG", L"ca-ES", L"cs-CZ", L"da-DK", L"de-DE", L"el-GR", L"en-GB", L"en-US", L"es-ES",
            L"es-MX", L"et-EE", L"eu-ES", L"fi-FI", L"fr-CA", L"fr-FR", L"gl-ES", L"he-IL", L"hr-HR", L"hu-HU",
            L"id-ID", L"it-IT", L"ja-JP", L"ko-KR", L"lt-LT", L"lv-LV", L"nb-NO", L"nl-NL", L"pl-PL", L"pt-BR",
            L"pt-PT", L"ro-RO", L"ru-RU", L"sk-SK", L"sl-SI", L"sr-Latn-RS", L"sv-SE", L"th-TH", L"tr-TR", L"uk-UA",
            L"vi-VN", L"zh-CN", L"zh-TW", L"pris", L"StartUI",
        };
        for (size_t i = 0; bOk && i < ARRAYSIZE(possibleDirs); i++)
        {
            WCHAR wszDirectoryPath[MAX_PATH];
            wcscpy_s(wszDirectoryPath, MAX_PATH, wszPath);
            wcscat_s(wszDirectoryPath, MAX_PATH, L"\\");
            wcscat_s(wszDirectoryPath, MAX_PATH, possibleDirs[i]);
            if (FileExistsW(wszDirectoryPath))
            {
                bOk = RemoveDirectoryRecursive(wszDirectoryPath);
            }
        }
        DeleteResource(wszPath, L"Windows.UI.ShellCommon.pri");
        BOOL bUnpackCustomStartUI = (global_rovi.dwBuildNumber >= 22621 && global_rovi.dwBuildNumber <= 22635) || global_rovi.dwBuildNumber >= 25169;
        BOOL bNoPniduiInThisBuild = global_rovi.dwBuildNumber >= 25236;
        if (bInstall)
        {
            const WCHAR* languages = GetSystemLanguages();
            if (bNoPniduiInThisBuild)
            {
                if (bOk) bOk = ExtractDirectory(zipFile, "pnidui/", wszPath, languages, LCT_MUI);
            }
            if (bUnpackCustomStartUI)
            {
                if (bOk) bOk = ExtractDirectory(zipFile, "Windows.UI.ShellCommon/", wszPath, languages, LCT_PRI);
            }
        }

        if (bOk) bOk = InstallResource(bInstall && bNoPniduiInThisBuild, hInstance, zipFile, "pnidui/pnidui.dll", wszPath, L"pnidui.dll");

        if (bOk && bNoPniduiInThisBuild)
        {
            // Windows Registry Editor Version 5.00
            //
            // [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellServiceObjects\{C2796011-81BA-4148-8FCA-C6643245113F}]
            // "AutoStart"=""
            if (bInstall)
            {
                HKEY hKey;
                RegCreateKeyExW(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellServiceObjects\\{C2796011-81BA-4148-8FCA-C6643245113F}",
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_READ | KEY_WRITE,
                    NULL,
                    &hKey,
                    NULL
                );
                if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
                {
                    hKey = NULL;
                }
                if (hKey)
                {
                    RegSetValueExW(hKey, L"AutoStart", 0, REG_SZ, (const BYTE*)L"", 1 * sizeof(WCHAR));
                    RegCloseKey(hKey);
                }
            }
            else
            {
                RegDeleteKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellServiceObjects\\{C2796011-81BA-4148-8FCA-C6643245113F}");
            }
        }

        // --------------------------------------------------------------------------------

        // C:\Windows
        // + dxgi.dll
        if (bOk) GetWindowsDirectoryW(wszPath, MAX_PATH);
#if defined(_M_X64)
        if (bOk) bOk = InstallResource(bInstall, hInstance, zipFile, PRODUCT_NAME ".amd64.dll", wszPath, L"dxgi.dll");
#elif defined(_M_ARM64)
        if (bOk) bOk = InstallResource(bInstall, hInstance, zipFile, PRODUCT_NAME ".arm64.dll", wszPath, L"dxgi.dll");
#endif

        // --------------------------------------------------------------------------------

        // C:\Windows\SystemApps\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy
        // + dxgi.dll
        // + StartUI_.dll (download, optional)
        // + wincorlib.dll
        // + wincorlib_orig.dll (symlink)
        // - AppResolverLegacy.dll
        // - StartTileDataLegacy.dll
        // - Windows.UI.ShellCommon.pri
        // - en-US\StartTileDataLegacy.dll.mui
        // - pris2\Windows.UI.ShellCommon.en-US.pri
        if (bOk) GetWindowsDirectoryW(wszPath, MAX_PATH);
        if (bOk) wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy");
#if defined(_M_X64)
        if (bOk) bOk = InstallResource(bInstall, hInstance, zipFile, PRODUCT_NAME ".amd64.dll", wszPath, L"dxgi.dll");
#elif defined(_M_ARM64)
        if (bOk) bOk = InstallResource(bInstall, hInstance, zipFile, PRODUCT_NAME ".arm64.dll", wszPath, L"dxgi.dll");
#endif
        if (bOk) bOk = InstallResource(bInstall && IsWindows11(), hInstance, zipFile, "ep_startmenu.dll", wszPath, L"wincorlib.dll");
        if (bOk) bOk = DeleteResource(wszPath, L"wincorlib_orig.dll");
        if (bOk && IsWindows11() && bInstall)
        {
            // Symlink wincorlib_orig.dll to wincorlib.dll in System32
            WCHAR wszOrigPath[MAX_PATH];
            GetSystemDirectoryW(wszOrigPath, MAX_PATH);
            wcscat_s(wszOrigPath, MAX_PATH, L"\\wincorlib.dll");

            WCHAR wszSymLinkPath[MAX_PATH];
            wcscpy_s(wszSymLinkPath, MAX_PATH, wszPath);
            wcscat_s(wszSymLinkPath, MAX_PATH, L"\\wincorlib_orig.dll");
            bOk = CreateSymbolicLinkW(wszSymLinkPath, wszOrigPath, 0);
        }

        if (bOk) bOk = InstallResource(bInstall && bUnpackCustomStartUI, hInstance, zipFile, "StartUI/StartUI.dll", wszPath, L"StartUI_.dll");

        // Delete remnants from earlier versions
        if (bOk) bOk = DeleteResource(wszPath, L"AppResolverLegacy.dll");
        if (bOk) bOk = DeleteResource(wszPath, L"StartTileDataLegacy.dll");
        if (bOk && IsWindows11()) bOk = DeleteResource(wszPath, L"Windows.UI.ShellCommon.pri");

        // .\en-US
        if (bOk && IsWindows11())
        {
            WCHAR wszSubPath[MAX_PATH];
            wcscpy_s(wszSubPath, MAX_PATH, wszPath);
            wcscat_s(wszSubPath, MAX_PATH, L"\\en-US");
            if (FileExistsW(wszSubPath))
            {
                bOk = DeleteResource(wszSubPath, L"StartTileDataLegacy.dll.mui");
                if (bOk) bOk = RemoveDirectoryW(wszSubPath);
            }
        }

        // .\pris2
        if (bOk && IsWindows11())
        {
            WCHAR wszSubPath[MAX_PATH];
            wcscpy_s(wszSubPath, MAX_PATH, wszPath);
            wcscat_s(wszSubPath, MAX_PATH, L"\\pris2");
            if (FileExistsW(wszSubPath))
            {
                bOk = DeleteResource(wszSubPath, L"Windows.UI.ShellCommon.en-US.pri");
                if (bOk) bOk = RemoveDirectoryW(wszSubPath);
            }
        }

        // End remnant deletion

        // --------------------------------------------------------------------------------

        // C:\Windows\SystemApps\ShellExperienceHost_cw5n1h2txyewy
        // + dxgi.dll
        if (bOk) GetWindowsDirectoryW(wszPath, MAX_PATH);
        if (bOk) wcscat_s(wszPath, MAX_PATH, L"\\SystemApps\\ShellExperienceHost_cw5n1h2txyewy");
#if defined(_M_X64)
        if (bOk && IsWindows11()) bOk = InstallResource(bInstall, hInstance, zipFile, PRODUCT_NAME ".amd64.dll", wszPath, L"dxgi.dll");
#elif defined(_M_ARM64)
        if (bOk && IsWindows11()) bOk = InstallResource(bInstall, hInstance, zipFile, PRODUCT_NAME ".arm64.dll", wszPath, L"dxgi.dll");
#endif

        // --------------------------------------------------------------------------------

        if (bOk)
        {
            GetSystemDirectoryW(wszPath, MAX_PATH);
            WCHAR* pArgs = NULL;
            DWORD dwLen = (DWORD)wcslen(wszPath);
            wcscat_s(wszPath, MAX_PATH - dwLen, L"\\rundll32.exe \"");
            dwLen = (DWORD)wcslen(wszPath);
            pArgs = wszPath + dwLen - 2;
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath + dwLen);
            wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\ep_gui.dll\",ZZGUI");
            pArgs[0] = 0;
            bOk = SetupShortcut(bInstall, wszPath, pArgs + 1);
            ZeroMemory(wszPath, MAX_PATH);
        }
        if (bOk)
        {
            wszPath[0] = L'"';
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath + 1);
            wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\" _T(SETUP_UTILITY_NAME) L"\" /uninstall");
            bOk = SetupUninstallEntry(bInstall, wszPath);
        }
        ShExecInfo.lpParameters = bInstall ? L"start " _T(EP_DWM_SERVICENAME) : L"delete " _T(EP_DWM_SERVICENAME);
        if (ShellExecuteExW(&ShExecInfo) && ShExecInfo.hProcess)
        {
            WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
            DWORD dwExitCode = 0;
            GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
            CloseHandle(ShExecInfo.hProcess);
        }
        if (bOk)
        {
            WCHAR wszArgs[MAX_PATH];
            wszArgs[0] = L'/';
            wszArgs[1] = L's';
            wszArgs[2] = L' ';
            wszArgs[3] = L'"';
            if (!bInstall)
            {
                wszArgs[3] = L'/';
                wszArgs[4] = L'u';
                wszArgs[5] = L' ';
                wszArgs[6] = L'"';
            }
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszArgs + 4 + (bInstall ? 0 : 3));
            wcscat_s(wszArgs, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\ep_weather_host.dll\"");
            wprintf(L"%s\n", wszArgs);
            WCHAR wszApp[MAX_PATH * 2];
            GetSystemDirectoryW(wszApp, MAX_PATH * 2);
            wcscat_s(wszApp, MAX_PATH * 2, L"\\regsvr32.exe");
            wprintf(L"%s\n", wszApp);
            SHELLEXECUTEINFOW sei;
            ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_NOCLOSEPROCESS;
            sei.hwnd = NULL;
            sei.hInstApp = NULL;
            sei.lpVerb = NULL;
            sei.lpFile = wszApp;
            sei.lpParameters = wszArgs;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;
            if (ShellExecuteExW(&sei) && sei.hProcess)
            {
                WaitForSingleObject(sei.hProcess, INFINITE);
                DWORD dwExitCode = 0;
                GetExitCodeProcess(sei.hProcess, &dwExitCode);
                SetLastError(dwExitCode);
                CloseHandle(sei.hProcess);
            }
        }
        if (bOk)
        {
            WCHAR wszArgs[MAX_PATH];
            wszArgs[0] = L'/';
            wszArgs[1] = L's';
            wszArgs[2] = L' ';
            wszArgs[3] = L'"';
            if (!bInstall)
            {
                wszArgs[3] = L'/';
                wszArgs[4] = L'u';
                wszArgs[5] = L' ';
                wszArgs[6] = L'"';
            }
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszArgs + 4 + (bInstall ? 0 : 3));
            wcscat_s(wszArgs, MAX_PATH, _T(APP_RELATIVE_PATH) L"\\ep_weather_host_stub.dll\"");
            wprintf(L"%s\n", wszArgs);
            WCHAR wszApp[MAX_PATH * 2];
            GetSystemDirectoryW(wszApp, MAX_PATH * 2);
            wcscat_s(wszApp, MAX_PATH * 2, L"\\regsvr32.exe");
            wprintf(L"%s\n", wszApp);
            SHELLEXECUTEINFOW sei;
            ZeroMemory(&sei, sizeof(SHELLEXECUTEINFOW));
            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_NOCLOSEPROCESS;
            sei.hwnd = NULL;
            sei.hInstApp = NULL;
            sei.lpVerb = NULL;
            sei.lpFile = wszApp;
            sei.lpParameters = wszArgs;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;
            if (ShellExecuteExW(&sei) && sei.hProcess)
            {
                WaitForSingleObject(sei.hProcess, INFINITE);
                DWORD dwExitCode = 0;
                GetExitCodeProcess(sei.hProcess, &dwExitCode);
                SetLastError(dwExitCode);
                CloseHandle(sei.hProcess);
            }
        }
        if (bOk && bInstall)
        {
            HKEY hKey = NULL;
            RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Policies\\Microsoft\\Windows\\Explorer", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hKey, NULL);
            if (hKey && hKey != INVALID_HANDLE_VALUE)
            {
                RegCloseKey(hKey);
            }
            RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hKey, NULL);
            if (hKey && hKey != INVALID_HANDLE_VALUE)
            {
                RegCloseKey(hKey);
            }
        }
        if (!bInstall)
        {
            SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
            wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH));
            if (bOk) bOk = DeleteResource(wszPath, L"ep_weather_host.dll");
            if (bOk) bOk = DeleteResource(wszPath, L"ep_weather_host_stub.dll");
            if (bOk) bOk = DeleteResource(wszPath, L"WebView2Loader.dll");
        }

        if (bOk)
        {
            if (!bInstall)
            {
                SHGetFolderPathW(NULL, SPECIAL_FOLDER, NULL, SHGFP_TYPE_CURRENT, wszPath);
                wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH));
                bOk = RemoveDirectoryRecursive(wszPath);
            }
            if (bOk && (!bInstall || g_cleanupFileCounter > 1))
            {
                WCHAR wszDirToDelete[MAX_PATH];
                SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wszDirToDelete);
                wcscat_s(wszDirToDelete, MAX_PATH, _T(APP_RELATIVE_PATH));
                if (bInstall)
                {
                    wcscat_s(wszDirToDelete, MAX_PATH, L"\\cleanup");
                }

                HKEY hKey = NULL;
                RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL);
                if (hKey && hKey != INVALID_HANDLE_VALUE)
                {
                    WCHAR wszCommand[MAX_PATH];
                    wcscpy_s(wszCommand, MAX_PATH, L"cmd /c rmdir /s /q \"");
                    wcscat_s(wszCommand, MAX_PATH, wszDirToDelete);
                    wcscat_s(wszCommand, MAX_PATH, L"\"");
                    RegSetValueExW(hKey, L"ExplorerPatcherCleanup", 0, REG_SZ, (BYTE*)wszCommand, (DWORD)((wcslen(wszCommand) + 1) * sizeof(WCHAR)));
                    RegCloseKey(hKey);
                }
            }
            if (!bInstall)
            {
                wchar_t mbText[256];
                mbText[0] = 0;
                if (bWasShellExt)
                {
                    LoadStringW(hInstance, IDS_SETUP_UNINSTALL_RESTART, mbText, ARRAYSIZE(mbText));
                    if (MessageBoxW(NULL, mbText, _T(PRODUCT_NAME), MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION) == IDYES)
                    {
                        SystemShutdown(TRUE);
                    }
                }
                else
                {
                    LoadStringW(hInstance, IDS_SETUP_UNINSTALL_FINISH, mbText, ARRAYSIZE(mbText));
                    MessageBoxW(NULL, mbText, _T(PRODUCT_NAME), MB_ICONASTERISK | MB_OK | MB_DEFBUTTON1);
                }
            }
            else
            {
                if (bIsUpdate)
                {
                    HKEY hKey = NULL;
                    DWORD dwSize = 0;

                    RegCreateKeyExW(
                        HKEY_CURRENT_USER,
                        TEXT(REGPATH),
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WOW64_64KEY | KEY_WRITE,
                        NULL,
                        &hKey,
                        NULL
                    );
                    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE)
                    {
                        hKey = NULL;
                    }
                    if (hKey)
                    {
                        dwSize = TRUE;
                        RegSetValueExW(
                            hKey,
                            TEXT("IsUpdatePending"),
                            0,
                            REG_DWORD,
                            (const BYTE*)&dwSize,
                            sizeof(DWORD)
                        );
                        RegCloseKey(hKey);
                    }
                }
                //ZZRestartExplorer(0, 0, 0, 0);
            }
        }
        if (!bOk) //  && !(argc >= 1 && !_wcsicmp(wargv[0], L"/update_silent"))
        {
            wchar_t mbText[1024];
            mbText[0] = 0;
            LoadStringW(hInstance, IDS_SETUP_FAILED, mbText, ARRAYSIZE(mbText));
            MessageBoxW(NULL, mbText, _T(PRODUCT_NAME), MB_ICONERROR | MB_OK | MB_DEFBUTTON1);
        }
        if (bOk && bIsUndockingDisabled)
        {
            ExitWindowsEx(EWX_LOGOFF, SHTDN_REASON_FLAG_PLANNED);
            exit(0);
        }

        StartExplorerWithDelay(1000, userToken);
        if (userToken != INVALID_HANDLE_VALUE) CloseHandle(userToken);
    }

    if (zipFile)
        unzClose(zipFile);
    if (pMem)
        MemoryBuffer_Destroy(&pMem);

	return GetLastError();
}

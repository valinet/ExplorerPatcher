#include "updates.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <string>

#include <wil/resource.h>
#include <wil/result_macros.h>
#include <wrl/async.h>

static HRESULT String2IXMLDocument(const wchar_t* pwszData, ABI::Windows::Data::Xml::Dom::IXmlDocument** pOutXmlToastMessage)
{
    using namespace Microsoft::WRL;
    using namespace ABI::Windows::Data::Xml::Dom;

    *pOutXmlToastMessage = nullptr;

    CoInitialize(nullptr);
    RoInitialize(RO_INIT_MULTITHREADED);

    ComPtr<IInspectable> spInspectable;
    RETURN_IF_FAILED(RoActivateInstance(Wrappers::HStringReference(RuntimeClass_Windows_Data_Xml_Dom_XmlDocument).Get(), &spInspectable));

    ComPtr<IXmlDocument> spXmlDocument;
    RETURN_IF_FAILED(spInspectable.As(&spXmlDocument));
    spXmlDocument.CopyTo(pOutXmlToastMessage);

    ComPtr<IXmlDocumentIO> spXmlDocumentIO;
    RETURN_IF_FAILED(spXmlDocument.As(&spXmlDocumentIO));

    RETURN_IF_FAILED(spXmlDocumentIO->LoadXml(Wrappers::HStringReference(pwszData).Get()));

    return S_OK;
}

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Data::Xml::Dom;
using namespace ABI::Windows::UI::Notifications;

class CToastData : public RuntimeClass<RuntimeClassFlags<ClassicCom>, ITypedEventHandler<ToastNotification*, IInspectable*>>
{
public:
    CToastData(IToastNotifier* notifier, IToastNotificationFactory* notifFactory)
        : notifier(notifier), notifFactory(notifFactory), cookie{}, liLastUpdate{}
    {
    }

    void HideToast()
    {
        if (!this) return;
        IToastNotification* oldToast = toast.Get();
        if (oldToast)
        {
            if (notifier)
            {
                notifier->Hide(oldToast);
            }
            oldToast->remove_Activated(cookie);
            toast.Reset();
        }
    }

    void HideAndShowToast(IXmlDocument* inputXml)
    {
        if (!this) return;
        HideToast();
        if (notifFactory)
        {
            notifFactory->CreateToastNotification(inputXml, &toast);
        }
        IToastNotification* newToast = toast.Get();
        if (newToast && notifier)
        {
            ComPtr<IToastNotification2> toast2;
            if (SUCCEEDED(newToast->QueryInterface(IID_PPV_ARGS(&toast2))))
            {
                toast2->put_Tag(Wrappers::HStringReference(L"ep_updates").Get());
            }
            newToast->add_Activated(this, &cookie);
            notifier->Show(newToast);
        }
    }

    HRESULT UpdateDownloadProgress(SIZE_T downloaded, SIZE_T total)
    {
        if (!this) return E_FAIL;

        LARGE_INTEGER liNow;
        QueryPerformanceCounter(&liNow);
        /*if (downloaded != 0 && liNow.QuadPart - liLastUpdate.QuadPart < 1000000) // 100ms
        {
            return S_FALSE;
        }*/
        liLastUpdate = liNow;

        ComPtr<IInspectable> dataInspectable;
        RETURN_IF_FAILED(RoActivateInstance(Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_NotificationData).Get(), &dataInspectable));

        ComPtr<INotificationData> data;
        RETURN_IF_FAILED(dataInspectable.As(&data));

        ComPtr<IMap<HSTRING, HSTRING>> values;
        RETURN_IF_FAILED(data->get_Values(&values));

        WCHAR szProgressValue[64];
        if (total != 0)
        {
            swprintf_s(szProgressValue, L"%f", (double)downloaded / (double)total);
        }
        else
        {
            wcscpy_s(szProgressValue, ARRAYSIZE(szProgressValue), L"indeterminate");
        }

        WCHAR szProgressStatus[80];
        szProgressStatus[0] = 0;

        WCHAR szDownloaded[32];
        szDownloaded[0] = 0;
        StrFormatByteSizeW(downloaded, szDownloaded, ARRAYSIZE(szDownloaded));

        if (total != 0)
        {
            WCHAR szTotal[32];
            szTotal[0] = 0;
            StrFormatByteSizeW(total, szTotal, ARRAYSIZE(szTotal));
            swprintf_s(szProgressStatus, L"%s / %s", szDownloaded, szTotal);
        }
        else if (downloaded != 0)
        {
            wcscpy_s(szProgressStatus, ARRAYSIZE(szProgressStatus), szDownloaded);
        }
        else
        {
            wil::unique_hmodule hEPGui(LoadGuiModule());
            LoadStringW(hEPGui.get(), IDS_UPDATES_DOWNLOADING_0, szProgressStatus, ARRAYSIZE(szProgressStatus));
        }

        BOOLEAN bReplaced;
        RETURN_IF_FAILED(values->Insert(
            Wrappers::HStringReference(L"progressValue").Get(),
            Wrappers::HStringReference(szProgressValue, (UINT)wcslen(szProgressValue)).Get(),
            &bReplaced
        ));
        RETURN_IF_FAILED(values->Insert(
            Wrappers::HStringReference(L"progressStatus").Get(),
            Wrappers::HStringReference(szProgressStatus, (UINT)wcslen(szProgressStatus)).Get(),
            &bReplaced
        ));

        ComPtr<IToastNotifier2> notifier2;
        RETURN_IF_FAILED(notifier->QueryInterface(IID_PPV_ARGS(&notifier2)));

        NotificationUpdateResult result;
        RETURN_HR(notifier2->UpdateWithTag(data.Get(), Wrappers::HStringReference(L"ep_updates").Get(), &result));
    }

    STDMETHODIMP Invoke(IToastNotification* sender, IInspectable* argsInspectable) override
    {
        ComPtr<IToastActivatedEventArgs> args;
        RETURN_IF_FAILED(argsInspectable->QueryInterface(IID_PPV_ARGS(&args)));

        Wrappers::HString arguments;
        RETURN_IF_FAILED(args->get_Arguments(arguments.GetAddressOf()));

        const WCHAR* argumentsStr = arguments.GetRawBuffer(nullptr);
        if (!wcscmp(argumentsStr, L"action=update"))
        {
            HANDLE hEvent = CreateEventW(nullptr, FALSE, FALSE, L"EP_Ev_InstallUpdatesNoConfirm_" _T(EP_CLSID));
            if (hEvent)
            {
                if (GetLastError() == ERROR_ALREADY_EXISTS)
                {
                    SetEvent(hEvent);
                }
                CloseHandle(hEvent);
            }
        }

        return S_OK;
    }

private:
    IToastNotifier* notifier;
    IToastNotificationFactory* notifFactory;
    ComPtr<IToastNotification> toast;
    EventRegistrationToken cookie;
    LARGE_INTEGER liLastUpdate;
};

struct IsUpdateAvailableParameters
{
    HINTERNET hInternet;
    HANDLE hEvent;
};

BOOL IsUpdateAvailableHelper(
    WCHAR* url,
    char* szCheckAgainst,
    DWORD dwUpdateTimeout,
    BOOL* lpFail,
    CToastData* toastData,
    BOOL bUpdatePreferStaging,
    WCHAR* wszInfoURL,
    DWORD cchInfoURL,
    BOOL bNoConfirmation,
    HMODULE hModule,
    DWORD* pLeftMost, DWORD* pSecondLeft, DWORD* pSecondRight, DWORD* pRightMost)
{
    BOOL bIsUpdateAvailable = FALSE;

    IsUpdateAvailableParameters params;
    params.hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!params.hEvent)
    {
        return bIsUpdateAvailable;
    }

    Wrappers::HString hstrDownloadUrl;
    hstrDownloadUrl.Set(url);
    HINTERNET hInternet = nullptr;

    if (bUpdatePreferStaging)
    {
        BOOL bRet = FALSE;
        std::string jsonStr;

        hInternet = InternetOpenA(UPDATES_USER_AGENT, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
        if ((bRet = (hInternet != nullptr)))
        {
            HINTERNET hConnect = InternetOpenUrlW(
                hInternet,
                hstrDownloadUrl.GetRawBuffer(nullptr),
                nullptr,
                0,
                INTERNET_FLAG_RAW_DATA |
                INTERNET_FLAG_RELOAD |
                INTERNET_FLAG_RESYNCHRONIZE |
                INTERNET_FLAG_NO_COOKIES |
                INTERNET_FLAG_NO_UI |
                INTERNET_FLAG_NO_CACHE_WRITE |
                INTERNET_FLAG_DONT_CACHE,
                (DWORD_PTR)&params
            );
            if ((bRet = (hConnect != nullptr)))
            {
                char* buffer = (char*)malloc(UPDATES_BUFSIZ);
                if ((bRet = (buffer != nullptr)))
                {
                    DWORD dwRead = 0;
                    while (InternetReadFile(hConnect, buffer, UPDATES_BUFSIZ, &dwRead) && dwRead != 0)
                    {
                        jsonStr.append(buffer, dwRead);
                    }
                    bRet = !jsonStr.empty();
                    free(buffer);
                }
                InternetCloseHandle(hConnect);
            }
            InternetCloseHandle(hInternet);
        }

        WCHAR* pszJsonStr = nullptr;
        if (bRet)
        {
            pszJsonStr = (WCHAR*)malloc(sizeof(WCHAR) * (jsonStr.size() + 1));
            if ((bRet = (pszJsonStr != nullptr)))
            {
                MultiByteToWideChar(
                    CP_UTF8,
                    MB_PRECOMPOSED,
                    jsonStr.c_str(),
                    -1,
                    pszJsonStr,
                    jsonStr.size() + 1
                );
            }
        }

        if (bRet)
        {
            auto extract = [&](HSTRING* outHtmlUrl, HSTRING* outDownloadUrl) -> HRESULT
            {
                using namespace ABI::Windows::Data::Json;

                *outHtmlUrl = nullptr;
                *outDownloadUrl = nullptr;

                ComPtr<IJsonArrayStatics> jsonArrayStatics;
                RETURN_IF_FAILED(GetActivationFactory(Wrappers::HStringReference(RuntimeClass_Windows_Data_Json_JsonArray).Get(), &jsonArrayStatics));

                ComPtr<IJsonArray> releases;
                RETURN_IF_FAILED(jsonArrayStatics->Parse(Wrappers::HStringReference(pszJsonStr).Get(), &releases));

                ComPtr<IJsonObject> firstRelease;
                RETURN_IF_FAILED(releases->GetObjectAt(0, &firstRelease));

                RETURN_IF_FAILED(firstRelease->GetNamedString(Wrappers::HStringReference(L"html_url").Get(), outHtmlUrl));

                ComPtr<IJsonArray> assets;
                RETURN_IF_FAILED(firstRelease->GetNamedArray(Wrappers::HStringReference(L"assets").Get(), &assets));

                ComPtr<IIterable<IJsonValue*>> assetsIterable;
                RETURN_IF_FAILED(assets.As(&assetsIterable));

                ComPtr<IIterator<IJsonValue*>> assetsIterator;
                RETURN_IF_FAILED(assetsIterable->First(&assetsIterator));

                boolean bHasCurrent = false;
                RETURN_IF_FAILED(assetsIterator->get_HasCurrent(&bHasCurrent));

                while (bHasCurrent)
                {
                    ComPtr<IJsonValue> assetValue;
                    RETURN_IF_FAILED(assetsIterator->get_Current(&assetValue));

                    ComPtr<IJsonObject> asset;
                    RETURN_IF_FAILED(assetValue->GetObjectW(&asset)); // Note: W/A macros caused this to be renamed

                    Wrappers::HString name;
                    RETURN_IF_FAILED(asset->GetNamedString(Wrappers::HStringReference(L"name").Get(), name.ReleaseAndGetAddressOf()));

                    const WCHAR* pszName = name.GetRawBuffer(nullptr);
                    if (wcscmp(pszName, _T(SETUP_UTILITY_NAME)) == 0)
                    {
                        RETURN_IF_FAILED(asset->GetNamedString(Wrappers::HStringReference(L"browser_download_url").Get(), outDownloadUrl));
                        return S_OK;
                    }

                    RETURN_IF_FAILED(assetsIterator->MoveNext(&bHasCurrent));
                }

                RETURN_HR(HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
            };

            Wrappers::HString htmlUrl;
            if ((bRet = SUCCEEDED(extract(htmlUrl.ReleaseAndGetAddressOf(), hstrDownloadUrl.ReleaseAndGetAddressOf()))))
            {
                if (wszInfoURL)
                {
                    wcscpy_s(wszInfoURL, cchInfoURL, htmlUrl.GetRawBuffer(nullptr));
                    wprintf(L"[Updates] Release notes URL: \"%s\"\n", wszInfoURL);
                }
                if (hstrDownloadUrl.Get())
                {
                    wprintf(L"[Updates] Prerelease update URL: \"%s\"\n", hstrDownloadUrl.GetRawBuffer(nullptr));
                    bUpdatePreferStaging = FALSE; // Success
                }
            }
        }

        if (pszJsonStr)
        {
            free(pszJsonStr);
        }
    }

    if (!bUpdatePreferStaging && ((hInternet = InternetOpenA(
        UPDATES_USER_AGENT,
        INTERNET_OPEN_TYPE_PRECONFIG,
        nullptr,
        nullptr,
        0
    ))))
    {
        HINTERNET hConnect = InternetOpenUrlW(
            hInternet,
            hstrDownloadUrl.GetRawBuffer(nullptr),
            nullptr,
            0,
            INTERNET_FLAG_RAW_DATA |
            INTERNET_FLAG_RELOAD |
            INTERNET_FLAG_RESYNCHRONIZE |
            INTERNET_FLAG_NO_COOKIES |
            INTERNET_FLAG_NO_UI |
            INTERNET_FLAG_NO_CACHE_WRITE |
            INTERNET_FLAG_DONT_CACHE,
            (DWORD_PTR)&params
        );
        if (hConnect)
        {
            if (szCheckAgainst)
            {
                DWORD dwRead = 0;
                char hash[DOSMODE_OFFSET + UPDATES_HASH_SIZE + 1] = {};
                if (InternetReadFile(hConnect, hash, DOSMODE_OFFSET + UPDATES_HASH_SIZE, &dwRead)
                    && dwRead == DOSMODE_OFFSET + UPDATES_HASH_SIZE)
                {
#ifdef UPDATES_VERBOSE_OUTPUT
                    printf("[Updates] Hash of remote file is \"%s\" (%s).\n", DOSMODE_OFFSET + hash, (hash[0] == 0x4D && hash[1] == 0x5A) ? "valid" : "invalid");
#endif
                    BOOL bOldType = TRUE;
                    char *szLeftMost = nullptr, *szSecondLeft = nullptr, *szSecondRight = nullptr, *szRightMost = nullptr, *szRealHash = nullptr;
                    if (hModule)
                    {
                        if (hash[0] == 0x4D && hash[1] == 0x5A)
                        {
                            if (strchr(DOSMODE_OFFSET + hash, '.'))
                            {
                                szLeftMost = DOSMODE_OFFSET + hash;
                                if ((szSecondLeft = strchr(szLeftMost, '.')))
                                {
                                    *szSecondLeft = 0;
                                    szSecondLeft++;
                                    if ((szSecondRight = strchr(szSecondLeft, '.')))
                                    {
                                        *szSecondRight = 0;
                                        szSecondRight++;
                                        if ((szRightMost = strchr(szSecondRight, '.')))
                                        {
                                            *szRightMost = 0;
                                            szRightMost++;
                                            if ((szRealHash = strchr(szRightMost, '.')))
                                            {
                                                bOldType = FALSE;

                                                *szRealHash = 0;
                                                szRealHash++;
                                                DWORD dwRemoteLeftMost = atoi(szLeftMost);
                                                if (pLeftMost) *pLeftMost = dwRemoteLeftMost - ((dwRemoteLeftMost == 22622 && szRealHash[0] != '!') ? 1 : 0);
                                                DWORD dwRemoteSecondLeft = atoi(szSecondLeft);
                                                if (pSecondLeft) *pSecondLeft = dwRemoteSecondLeft;
                                                DWORD dwRemoteSecondRight = atoi(szSecondRight);
                                                if (pSecondRight) *pSecondRight = dwRemoteSecondRight;
                                                DWORD dwRemoteRightMost = atoi(szRightMost);
                                                if (pRightMost) *pRightMost = dwRemoteRightMost;
                                                DWORD dwLocalLeftMost = 0;
                                                DWORD dwLocalSecondLeft = 0;
                                                DWORD dwLocalSecondRight = 0;
                                                DWORD dwLocalRightMost = 0;
                                                BOOL bExtractedFromHash = FALSE;
                                                CHAR hashCopy[100];
                                                strcpy_s(hashCopy, 100, szCheckAgainst);
                                                char* szLocalLeftMost = nullptr, *szLocalSecondLeft = nullptr, *szLocalSecondRight = nullptr, *szLocalRightMost = nullptr, *szLocalRealHash = nullptr;
                                                if (strchr(hashCopy, '.'))
                                                {
                                                    szLocalLeftMost = hashCopy;
                                                    if ((szLocalSecondLeft = strchr(szLocalLeftMost, '.')))
                                                    {
                                                        *szLocalSecondLeft = 0;
                                                        szLocalSecondLeft++;
                                                        if ((szLocalSecondRight = strchr(szLocalSecondLeft, '.')))
                                                        {
                                                            *szLocalSecondRight = 0;
                                                            szLocalSecondRight++;
                                                            if ((szLocalRightMost = strchr(szLocalSecondRight, '.')))
                                                            {
                                                                *szLocalRightMost = 0;
                                                                szLocalRightMost++;
                                                                if ((szLocalRealHash = strchr(szLocalRightMost, '.')))
                                                                {
                                                                    *szLocalRealHash = 0;
                                                                    szLocalRealHash++;

                                                                    bExtractedFromHash = TRUE;
                                                                    dwLocalLeftMost = atoi(szLocalLeftMost);
                                                                    dwLocalSecondLeft = atoi(szLocalSecondLeft);
                                                                    dwLocalSecondRight = atoi(szLocalSecondRight);
                                                                    dwLocalRightMost = atoi(szLocalRightMost);
#ifdef UPDATES_VERBOSE_OUTPUT
                                                                    printf("[Updates] Local version obtained from hash is %d.%d.%d.%d.\n", dwLocalLeftMost, dwLocalSecondLeft, dwLocalSecondRight, dwLocalRightMost);
#endif
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                if (!bExtractedFromHash)
                                                {
                                                    QueryVersionInfo(hModule, VS_VERSION_INFO, &dwLocalLeftMost, &dwLocalSecondLeft, &dwLocalSecondRight, &dwLocalRightMost);
                                                }

                                                int res = 0;
                                                if (!res)
                                                {
                                                    if (dwLocalLeftMost < dwRemoteLeftMost)
                                                    {
                                                        res = -1;
                                                    }
                                                    if (dwLocalLeftMost > dwRemoteLeftMost)
                                                    {
                                                        res = 1;
                                                    }
                                                    if (!res)
                                                    {
                                                        if (dwLocalSecondLeft < dwRemoteSecondLeft)
                                                        {
                                                            res = -1;
                                                        }
                                                        if (dwLocalSecondLeft > dwRemoteSecondLeft)
                                                        {
                                                            res = 1;
                                                        }
                                                        if (!res)
                                                        {
                                                            if (dwLocalSecondRight < dwRemoteSecondRight)
                                                            {
                                                                res = -1;
                                                            }
                                                            if (dwLocalSecondRight > dwRemoteSecondRight)
                                                            {
                                                                res = 1;
                                                            }
                                                            if (!res)
                                                            {
                                                                if (dwLocalRightMost < dwRemoteRightMost)
                                                                {
                                                                    res = -1;
                                                                }
                                                                if (dwLocalRightMost > dwRemoteRightMost)
                                                                {
                                                                    res = 1;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                DWORD dwAllowDowngrades = FALSE, dwSize = sizeof(DWORD);
                                                RegGetValueW(HKEY_CURRENT_USER, _T(REGPATH), L"UpdateAllowDowngrades", RRF_RT_DWORD, nullptr, &dwAllowDowngrades, &dwSize);
                                                if ((res == 1 && dwAllowDowngrades) || res == -1)
                                                {
                                                    bIsUpdateAvailable = TRUE;
                                                }
                                                else if (res == 0)
                                                {
                                                    *(szSecondLeft - 1) = '.';
                                                    *(szSecondRight - 1) = '.';
                                                    *(szRightMost - 1) = '.';
                                                    *(szRealHash - 1) = '.';
                                                    if (_stricmp(DOSMODE_OFFSET + hash, szCheckAgainst))
                                                    {
                                                        bIsUpdateAvailable = TRUE;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    /*if (bOldType)
                    {
                        if (hash[0] == 0x4D && hash[1] == 0x5A && _stricmp(DOSMODE_OFFSET + hash, szCheckAgainst))
                        {
                            bIsUpdateAvailable = TRUE;
                        }
                    }*/
                }
                else
                {
#ifdef UPDATES_VERBOSE_OUTPUT
                    printf("[Updates] Failed. Read %d bytes.\n", dwRead);
#endif
                    if (lpFail) *lpFail = TRUE;
                }
            }
            else
            {
                WCHAR wszPath[MAX_PATH] = {};
                SHGetFolderPathW(nullptr, SPECIAL_FOLDER_LEGACY, nullptr, SHGFP_TYPE_CURRENT, wszPath);
                wcscat_s(wszPath, MAX_PATH, _T(APP_RELATIVE_PATH));
                BOOL bRet = CreateDirectoryW(wszPath, nullptr);
                if (bRet || (!bRet && GetLastError() == ERROR_ALREADY_EXISTS))
                {
                    wcscat_s(wszPath, MAX_PATH, L"\\Update for " _T(PRODUCT_NAME) L" from ");
                    WCHAR wszURL[MAX_PATH] = {};
                    wcscpy_s(wszURL, MAX_PATH, hstrDownloadUrl.GetRawBuffer(nullptr));
                    if (wszURL[97]) // Truncate the URL for display
                    {
                        wszURL[96] = L'.';
                        wszURL[97] = L'.';
                        wszURL[98] = L'.';
                        wszURL[99] = L'e';
                        wszURL[100] = L'x';
                        wszURL[101] = L'e';
                        wszURL[102] = 0;
                    }
                    for (unsigned int i = 0; true; ++i)
                    {
                        if (!wszURL[i])
                        {
                            break;
                        }
                        if (wszURL[i] == L'/')
                        {
                            wszURL[i] = L'\u2215';
                        }
                        else if (wszURL[i] == L':')
                        {
                            wszURL[i] = L'\ua789';
                        }
                    }
                    wcscat_s(wszPath, MAX_PATH, wszURL);
#ifdef UPDATES_VERBOSE_OUTPUT
                    wprintf(L"[Updates] Download path is \"%s\".\n", wszPath);
#endif

                    BOOL bRet = DeleteFileW(wszPath);
                    if (bRet || (!bRet && GetLastError() == ERROR_FILE_NOT_FOUND))
                    {
                        DWORD bIsUsingEpMake = 0, dwSize = sizeof(DWORD);
                        RegGetValueW(HKEY_CURRENT_USER, TEXT(REGPATH), L"UpdateUseLocal", RRF_RT_DWORD, nullptr, &bIsUsingEpMake, &dwSize);
                        if (bIsUsingEpMake) {
                            GetSystemDirectoryW(wszPath, MAX_PATH);
                            wcscat_s(wszPath, MAX_PATH, L"\\WindowsPowerShell\\v1.0\\powershell.exe");
                        }

                        FILE* f = nullptr;
                        if (!bIsUsingEpMake && !_wfopen_s(
                            &f,
                            wszPath,
                            L"wb"
                        ) && f)
                        {
                            BYTE* buffer = (BYTE*)malloc(UPDATES_BUFSIZ);
                            if (buffer != nullptr)
                            {
                                DWORD totalSize = 0;
                                CHAR szContentLength[24] = {};
                                DWORD dwSize = sizeof(szContentLength);
                                if (HttpQueryInfoA(hConnect, HTTP_QUERY_CONTENT_LENGTH, szContentLength, &dwSize, nullptr))
                                {
                                    totalSize = atoi(szContentLength);
                                }
                                toastData->UpdateDownloadProgress(0, totalSize);

                                DWORD dwTotalRead = 0;
                                DWORD dwRead = 0;
                                bRet = FALSE;
                                while ((bRet = InternetReadFile(
                                    hConnect,
                                    buffer,
                                    UPDATES_BUFSIZ,
                                    &dwRead
                                )))
                                {
                                    if (dwRead == 0)
                                    {
                                        bIsUpdateAvailable = TRUE;
#ifdef UPDATES_VERBOSE_OUTPUT
                                        printf("[Updates] Downloaded finished.\n");
#endif
                                        break;
                                    }
#ifdef UPDATES_VERBOSE_OUTPUT
                                    // printf("[Updates] Downloaded %d bytes.\n", dwRead);
#endif
                                    fwrite(
                                        buffer,
                                        sizeof(BYTE),
                                        dwRead,
                                        f
                                    );
                                    dwTotalRead += dwRead;
                                    toastData->UpdateDownloadProgress(dwTotalRead, totalSize);
                                    dwRead = 0;
                                }
                                free(buffer);
                            }
                            fclose(f);
                        }
                        if (bIsUsingEpMake || bIsUpdateAvailable)
                        {
                            bIsUpdateAvailable = FALSE;
#ifdef UPDATES_VERBOSE_OUTPUT
                            printf(
                                "[Updates] In order to install this update for the product \""
                                PRODUCT_NAME
                                "\", please allow the request.\n"
                            );
#endif

                            toastData->HideToast();

                            BOOL bHasErrored = FALSE;
                            BOOL bIsUACEnabled = FALSE;
                            DWORD(*CheckElevationEnabled)(BOOL*);
                            CheckElevationEnabled = (decltype(CheckElevationEnabled))GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CheckElevationEnabled");
                            if (CheckElevationEnabled) CheckElevationEnabled(&bIsUACEnabled);
                            DWORD dwData = FALSE, dwSize = sizeof(DWORD);
                            RegGetValueW(
                                HKEY_LOCAL_MACHINE,
                                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                                L"ConsentPromptBehaviorAdmin",
                                RRF_RT_DWORD,
                                nullptr,
                                &dwData,
                                &dwSize
                            );
                            LPWSTR wszSID = nullptr;
                            HANDLE hMyToken = nullptr;
                            if (OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hMyToken))
                            {
                                PTOKEN_USER ptu = nullptr;
                                DWORD dwSize = 0;
                                if (!GetTokenInformation(hMyToken, TokenUser, nullptr, 0, &dwSize)
                                    && ERROR_INSUFFICIENT_BUFFER == GetLastError())
                                {
                                    if (nullptr != (ptu = (PTOKEN_USER)LocalAlloc(LPTR, dwSize)))
                                    {
                                        if (!GetTokenInformation(hMyToken, TokenUser, ptu, dwSize, &dwSize))
                                        {
                                            LocalFree((HLOCAL)ptu);
                                            return FALSE;
                                        }
                                        ConvertSidToStringSidW(ptu->User.Sid, &wszSID);
                                        LocalFree((HLOCAL)ptu);
                                    }
                                }
                                CloseHandle(hMyToken);
                            }
                            size_t lnSID = (wszSID ? wcslen(wszSID) : 0);
                            DWORD bIsBuiltInAdministratorInApprovalMode = FALSE;
                            BOOL bIsBuiltInAdministratorAccount =
                                IsUserAnAdmin() &&
                                lnSID && !_wcsnicmp(wszSID, L"S-1-5-", 6) && wszSID[lnSID - 4] == L'-' && wszSID[lnSID - 3] == L'5' && wszSID[lnSID - 2] == L'0' && wszSID[lnSID - 1] == L'0';
                            if (bIsBuiltInAdministratorAccount)
                            {
                                DWORD dwSSize = sizeof(DWORD);
                                RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", L"FilterAdministratorToken", RRF_RT_DWORD, nullptr, &bIsBuiltInAdministratorInApprovalMode, &dwSSize);
                            }
                            LocalFree(wszSID);
                            if (!bNoConfirmation && (!bIsUACEnabled || !dwData || (bIsBuiltInAdministratorAccount && !bIsBuiltInAdministratorInApprovalMode)))
                            {
                                WCHAR wszMsg[500];
                                wszMsg[0] = 0;

                                WCHAR wszMsgFormat[500];
                                EP_L10N_ApplyPreferredLanguageForCurrentThread();
                                {
                                    wil::unique_hmodule hEPGui(LoadGuiModule());
                                    if (LoadStringW(hEPGui.get(), IDS_UPDATES_PROMPT, wszMsgFormat, ARRAYSIZE(wszMsgFormat)))
                                    {
                                        swprintf_s(wszMsg, ARRAYSIZE(wszMsg), wszMsgFormat, hstrDownloadUrl.GetRawBuffer(nullptr));
                                    }
                                }

                                if (MessageBoxW(
                                    FindWindowW(L"ExplorerPatcher_GUI_" _T(EP_CLSID), nullptr),
                                    wszMsg,
                                    _T(PRODUCT_NAME),
                                    MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION
                                ) == IDNO)
                                {
                                    bHasErrored = TRUE;
                                    SetLastError(ERROR_CANCELLED);
                                }
                            }

                            SHELLEXECUTEINFO ShExecInfo = { 0 };
                            ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
                            ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
                            ShExecInfo.hwnd = nullptr;
                            ShExecInfo.lpVerb = bIsUsingEpMake ? L"open" : L"runas";
                            ShExecInfo.lpFile = wszPath;
                            ShExecInfo.lpParameters = bIsUsingEpMake ? L"iex (irm 'https://raw.githubusercontent.com/valinet/ep_make/master/ep_make_safe.ps1')" : L"/update_silent";
                            ShExecInfo.lpDirectory = nullptr;
                            ShExecInfo.nShow = SW_SHOW;
                            ShExecInfo.hInstApp = nullptr;
                            if (!bHasErrored && ShellExecuteExW(&ShExecInfo) && ShExecInfo.hProcess)
                            {
                                WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
                                DWORD dwExitCode = 0;
                                if (GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode) && !dwExitCode)
                                {
                                    bIsUpdateAvailable = TRUE;
#ifdef UPDATES_VERBOSE_OUTPUT
                                    printf("[Updates] Update successful, File Explorer will probably restart momentarily.\n");
#endif
                                }
                                else
                                {
                                    SetLastError(dwExitCode);
#ifdef UPDATES_VERBOSE_OUTPUT
                                    printf("[Updates] Update failed because the following error has occured: %d.\n", dwExitCode);
#endif
                                }
                                CloseHandle(ShExecInfo.hProcess);
                            }
                            else
                            {
                                DWORD dwError = GetLastError();
                                if (dwError == ERROR_CANCELLED)
                                {
#ifdef UPDATES_VERBOSE_OUTPUT
                                    printf("[Updates] Update failed because the request was denied.\n");
#endif
                                }
                                else
                                {
#ifdef UPDATES_VERBOSE_OUTPUT
                                    printf("[Updates] Update failed because the following error has occured: %d.\n", GetLastError());
#endif
                                }
                            }
                        }
                    }
                }
            }
            InternetCloseHandle(hConnect);
        }
        else
        {
            if (lpFail) *lpFail = TRUE;
        }
        InternetCloseHandle(hInternet);
    }

    CloseHandle(params.hEvent);

    return bIsUpdateAvailable;
}

BOOL IsUpdateAvailable(LPCWSTR wszDataStore, char* szCheckAgainst, BOOL* lpFail, WCHAR* wszInfoURL, DWORD cchInfoURL, HMODULE hModule,
    DWORD* pLeftMost, DWORD* pSecondLeft, DWORD* pSecondRight, DWORD* pRightMost)
{
    HKEY hKey = nullptr;
    DWORD dwSize = 0;
    DWORD dwQueriedPolicy = 0;
    BOOL bIsPolicyMatch = FALSE;
    WCHAR szUpdateURL[MAX_PATH] = {};
    wcscat_s(szUpdateURL, MAX_PATH, _T(UPDATES_RELEASE_INFO_URL_STABLE));
#ifdef UPDATES_VERBOSE_OUTPUT
    printf("[Updates] Checking against hash \"%s\"\n", szCheckAgainst);
#endif
    DWORD dwUpdateTimeout = UPDATES_DEFAULT_TIMEOUT;
    DWORD bUpdatePreferStaging = FALSE;

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        wszDataStore,
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        nullptr,
        &hKey,
        nullptr
    );
    if (hKey == nullptr || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = nullptr;
    }
    if (hKey)
    {
        dwSize = sizeof(szUpdateURL);
        RegQueryValueExW(
            hKey,
            L"UpdateURL",
            nullptr,
            nullptr,
            (LPBYTE)szUpdateURL,
            &dwSize
        );
        if (dwSize == 1 && szUpdateURL[0] == 0)
        {
            wcscat_s(szUpdateURL, MAX_PATH, _T(UPDATES_RELEASE_INFO_URL_STABLE));
        }
        wcscat_s(szUpdateURL, MAX_PATH, _T("/download/"));
        wcscat_s(szUpdateURL, MAX_PATH, _T(SETUP_UTILITY_NAME));
        if (wszInfoURL)
        {
            dwSize = sizeof(WCHAR) * cchInfoURL;
            RegQueryValueExW(
                hKey,
                L"UpdateURL",
                nullptr,
                nullptr,
                (LPBYTE)wszInfoURL,
                &dwSize
            );
            if (dwSize == 1 && wszInfoURL[0] == 0)
            {
                wcscat_s(wszInfoURL, cchInfoURL, _T(UPDATES_RELEASE_INFO_URL_STABLE));
            }
        }
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            L"UpdateTimeout",
            nullptr,
            nullptr,
            (LPBYTE)&dwUpdateTimeout,
            &dwSize
        );
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            L"UpdatePreferStaging",
            nullptr,
            nullptr,
            (LPBYTE)&bUpdatePreferStaging,
            &dwSize
        );
        if (bUpdatePreferStaging)
        {
            szUpdateURL[0] = 0;
            wcscat_s(szUpdateURL, MAX_PATH, _T(UPDATES_RELEASE_INFO_URL_STAGING));
            dwSize = sizeof(szUpdateURL);
            RegQueryValueExW(
                hKey,
                L"UpdateURLStaging",
                nullptr,
                nullptr,
                (LPBYTE)szUpdateURL,
                &dwSize
            );
            if (dwSize == 1 && szUpdateURL[0] == 0)
            {
                wcscat_s(szUpdateURL, MAX_PATH, _T(UPDATES_RELEASE_INFO_URL_STAGING));
            }
        }
        RegCloseKey(hKey);
    }
#ifdef UPDATES_VERBOSE_OUTPUT
    wprintf(L"[Updates] Update URL: %s\n", szUpdateURL);
#endif
    return IsUpdateAvailableHelper(
        szUpdateURL,
        szCheckAgainst,
        dwUpdateTimeout,
        lpFail,
        nullptr,
        bUpdatePreferStaging,
        wszInfoURL,
        cchInfoURL,
        FALSE,
        hModule,
        pLeftMost, pSecondLeft, pSecondRight, pRightMost
    );
}

BOOL UpdateProduct(LPCWSTR wszDataStore, CToastData* toastData, BOOL bNoConfirmation, HMODULE hModule)
{
    HKEY hKey = nullptr;
    DWORD dwSize = 0;
    DWORD dwQueriedPolicy = 0;
    BOOL bIsPolicyMatch = FALSE;
    WCHAR szUpdateURL[MAX_PATH] = {};
    wcscat_s(szUpdateURL, MAX_PATH, _T(UPDATES_RELEASE_INFO_URL_STABLE));

    DWORD dwUpdateTimeout = UPDATES_DEFAULT_TIMEOUT;
    BOOL bUpdatePreferStaging = FALSE;

    RegCreateKeyExW(
        HKEY_CURRENT_USER,
        wszDataStore,
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WOW64_64KEY,
        nullptr,
        &hKey,
        nullptr
    );
    if (hKey == nullptr || hKey == INVALID_HANDLE_VALUE)
    {
        hKey = nullptr;
    }
    if (hKey)
    {
        dwSize = sizeof(szUpdateURL);
        RegQueryValueExW(
            hKey,
            L"UpdateURL",
            nullptr,
            nullptr,
            (LPBYTE)szUpdateURL,
            &dwSize
        );
        if (dwSize == 1 && szUpdateURL[0] == 0)
        {
            wcscat_s(szUpdateURL, MAX_PATH, _T(UPDATES_RELEASE_INFO_URL_STABLE));
        }
        wcscat_s(szUpdateURL, MAX_PATH, _T("/download/"));
        wcscat_s(szUpdateURL, MAX_PATH, _T(SETUP_UTILITY_NAME));
        dwSize = sizeof(DWORD);
        RegQueryValueExW(
            hKey,
            L"UpdateTimeout",
            nullptr,
            nullptr,
            (LPBYTE)&dwUpdateTimeout,
            &dwSize
        );
        dwSize = sizeof(BOOL);
        RegQueryValueExW(
            hKey,
            L"UpdatePreferStaging",
            nullptr,
            nullptr,
            (LPBYTE)&bUpdatePreferStaging,
            &dwSize
        );
        if (bUpdatePreferStaging)
        {
            szUpdateURL[0] = 0;
            wcscat_s(szUpdateURL, MAX_PATH, _T(UPDATES_RELEASE_INFO_URL_STAGING));
            dwSize = sizeof(szUpdateURL);
            RegQueryValueExW(
                hKey,
                L"UpdateURLStaging",
                nullptr,
                nullptr,
                (LPBYTE)szUpdateURL,
                &dwSize
            );
            if (dwSize == 1 && szUpdateURL[0] == 0)
            {
                wcscat_s(szUpdateURL, MAX_PATH, _T(UPDATES_RELEASE_INFO_URL_STAGING));
            }
        }
        RegCloseKey(hKey);
    }
#ifdef UPDATES_VERBOSE_OUTPUT
    wprintf(L"[Updates] Update URL: %s\n", szUpdateURL);
#endif
    return IsUpdateAvailableHelper(
        szUpdateURL,
        nullptr,
        dwUpdateTimeout,
        nullptr,
        toastData,
        bUpdatePreferStaging,
        nullptr, 0,
        bNoConfirmation,
        hModule,
        nullptr, nullptr, nullptr, nullptr
    );
}

BOOL ShowUpdateSuccessNotification(HMODULE hModule, CToastData* toastData)
{
    auto switchToThreadScopeExit = wil::scope_exit([](){ SwitchToThread(); });

    EP_L10N_ApplyPreferredLanguageForCurrentThread();
    wil::unique_hmodule hEPGui(LoadGuiModule());

    WCHAR buf[TOAST_BUFSIZ];
    DWORD dwLeftMost = 0;
    DWORD dwSecondLeft = 0;
    DWORD dwSecondRight = 0;
    DWORD dwRightMost = 0;
    QueryVersionInfo(hModule, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

    const WCHAR text[] =
        L"<toast scenario=\"reminder\" activationType=\"protocol\" launch=\"%s\" duration=\"%s\">\r\n"
        L"	<visual>\r\n"
        L"		<binding template=\"ToastGeneric\">\r\n"
        L"			<text><![CDATA[%s]]></text>\r\n"
        L"			<text><![CDATA[%s]]></text>\r\n"
        L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n%s" // <-- Progress bar goes here
        L"		</binding>\r\n"
        L"	</visual>\r\n"
        L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n%s" // <-- Buttons go here
        L"</toast>\r\n";
    WCHAR title[100];
    WCHAR body[200];
    title[0] = 0; body[0] = 0;

    LoadStringW(hEPGui.get(), IDS_UPDATES_SUCCESS_T, title, ARRAYSIZE(title));

    WCHAR bodyFormat[200] = {};
    if (LoadStringW(hEPGui.get(), IDS_UPDATES_INSTALLEDVER, bodyFormat, ARRAYSIZE(bodyFormat)))
    {
        swprintf_s(body, ARRAYSIZE(body), bodyFormat, dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
    }

    swprintf_s(buf, TOAST_BUFSIZ, text, L"", L"short", title, body, L"", L"");

    ComPtr<IXmlDocument> inputXml;
    String2IXMLDocument(buf, &inputXml);

    toastData->HideAndShowToast(inputXml.Get());

    return TRUE;
}

BOOL InstallUpdatesIfAvailable(
    HMODULE hModule,
    CToastData* toastData,
    DWORD dwOperation,
    DWORD bAllocConsole,
    DWORD dwUpdatePolicy)
{
    EP_L10N_ApplyPreferredLanguageForCurrentThread();
    wil::unique_hmodule hEPGui(LoadGuiModule());

    WCHAR wszInfoURL[MAX_PATH] = {};
    wcscat_s(wszInfoURL, MAX_PATH, _T(UPDATES_RELEASE_INFO_URL_STABLE));
    WCHAR buf[TOAST_BUFSIZ];
    DWORD dwLeftMost = 0;
    DWORD dwSecondLeft = 0;
    DWORD dwSecondRight = 0;
    DWORD dwRightMost = 0;
    QueryVersionInfo(hModule, VS_VERSION_INFO, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost);

    if (bAllocConsole)
    {
        switch (dwUpdatePolicy)
        {
        default:
        case UPDATE_POLICY_AUTO:
        {
            dwUpdatePolicy = UPDATE_POLICY_AUTO;
            printf("[Updates] Configured update policy on this system: \"Install updates automatically\".\n");
            break;
        }
        case UPDATE_POLICY_NOTIFY:
        {
            printf("[Updates] Configured update policy on this system: \"Check for updates but let me choose whether to download and install them\".\n");
            break;
        }
        case UPDATE_POLICY_MANUAL:
        {
            printf("[Updates] Configured update policy on this system: \"Manually check for updates\".\n");
            break;
        }
        }
    }

    const WCHAR text[] =
        L"<toast scenario=\"reminder\" activationType=\"protocol\" launch=\"%s\" duration=\"%s\">\r\n"
        L"	<visual>\r\n"
        L"		<binding template=\"ToastGeneric\">\r\n"
        L"			<text><![CDATA[%s]]></text>\r\n"
        L"			<text><![CDATA[%s]]></text>\r\n"
        L"			<text placement=\"attribution\"><![CDATA[ExplorerPatcher]]></text>\r\n%s" // <-- Progress bar goes here
        L"		</binding>\r\n"
        L"	</visual>\r\n"
        L"	<audio src=\"ms-winsoundevent:Notification.Default\" loop=\"false\" silent=\"false\"/>\r\n%s" // <-- Buttons go here
        L"</toast>\r\n";
    WCHAR title[100];
    WCHAR body[200];
    WCHAR actionsXml[200];
    WCHAR progressXml[200];
    title[0] = 0; body[0] = 0; actionsXml[0] = 0; progressXml[0] = 0;

    BOOL bIsInstall = dwOperation == UPDATES_OP_INSTALL || dwOperation == UPDATES_OP_INSTALL_NO_CONFIRM;
    if (dwOperation == UPDATES_OP_CHECK || bIsInstall)
    {
        // title[0] = 0; body[0] = 0; actionsXml[0] = 0; progressXml[0] = 0;

        if (bIsInstall)
        {
            LoadStringW(hEPGui.get(), IDS_UPDATES_DOWNLOADING_T, title, ARRAYSIZE(title));

            wcscpy_s(progressXml, ARRAYSIZE(progressXml),
                L"			<progress value=\"{progressValue}\" status=\"{progressStatus}\"/>\r\n"
            );
        }
        else if (dwOperation == UPDATES_OP_CHECK)
        {
            LoadStringW(hEPGui.get(), IDS_UPDATES_CHECKING_T, title, ARRAYSIZE(title));
        }

        WCHAR bodyFormat[200] = {};
        if (LoadStringW(hEPGui.get(), IDS_UPDATES_INSTALLEDVER, bodyFormat, ARRAYSIZE(bodyFormat)))
        {
            swprintf_s(body, ARRAYSIZE(body), bodyFormat, dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
        }

        swprintf_s(buf, TOAST_BUFSIZ, text, L"", L"long", title, body, progressXml, actionsXml);

        ComPtr<IXmlDocument> inputXml;
        String2IXMLDocument(buf, &inputXml);

        toastData->HideAndShowToast(inputXml.Get());

        if (bIsInstall)
        {
            toastData->UpdateDownloadProgress(0, 0);
        }
    }

    WCHAR dllName[MAX_PATH];
    GetModuleFileNameW(hModule, dllName, MAX_PATH);
    wprintf(L"[Updates] Path to module: %s\n", dllName);

    CHAR hash[100] = {};
    GetHardcodedHash(dllName, hash, 100);
    if (!strcmp(hash, "This"))
        ComputeFileHash2(hModule, dllName, hash, 100);
    else
        printf("[Updates] Using hardcoded hash.\n");

    BOOL bFail = FALSE, bReturnValue = FALSE;
    dwLeftMost = 0; dwSecondLeft = 0; dwSecondRight = 0; dwRightMost = 0;
    if (IsUpdateAvailable(_T(REGPATH), hash, &bFail, wszInfoURL, MAX_PATH, hModule, &dwLeftMost, &dwSecondLeft, &dwSecondRight, &dwRightMost))
    {
        printf("[Updates] An update is available.\n");
        if ((dwOperation == UPDATES_OP_DEFAULT && dwUpdatePolicy == UPDATE_POLICY_AUTO) || bIsInstall)
        {
            BOOL bOk = UpdateProduct(_T(REGPATH), toastData, dwOperation == UPDATES_OP_INSTALL_NO_CONFIRM, hModule);
            if (!bOk && bIsInstall)
            {
                title[0] = 0; body[0] = 0; actionsXml[0] = 0; progressXml[0] = 0;

                LoadStringW(hEPGui.get(), IDS_UPDATES_DLFAILED_T, title, ARRAYSIZE(title));
                LoadStringW(hEPGui.get(), IDS_UPDATES_DLFAILED_B, body, ARRAYSIZE(body));

                swprintf_s(buf, TOAST_BUFSIZ, text, L"", L"short", title, body, progressXml, actionsXml);

                ComPtr<IXmlDocument> inputXml;
                String2IXMLDocument(buf, &inputXml);

                toastData->HideAndShowToast(inputXml.Get());
            }
        }
        else if ((dwOperation == UPDATES_OP_DEFAULT && dwUpdatePolicy == UPDATE_POLICY_NOTIFY) || (dwOperation == UPDATES_OP_CHECK))
        {
            title[0] = 0; body[0] = 0; actionsXml[0] = 0; progressXml[0] = 0;

            if (!dwLeftMost)
            {
                LoadStringW(hEPGui.get(), IDS_UPDATES_AVAILABLE_T_U, title, ARRAYSIZE(title));
            }
            else
            {
                WCHAR titleFormat[100];
                if (LoadStringW(hEPGui.get(), IDS_UPDATES_AVAILABLE_T, titleFormat, ARRAYSIZE(titleFormat)))
                {
                    swprintf_s(title, ARRAYSIZE(title), titleFormat, dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
                }
            }

            LoadStringW(hEPGui.get(), IDS_UPDATES_AVAILABLE_B, body, ARRAYSIZE(body));

            WCHAR action[100];
            action[0] = 0;
            LoadStringW(hEPGui.get(), IDS_UPDATES_AVAILABLE_A, action, ARRAYSIZE(action));

            swprintf_s(actionsXml, ARRAYSIZE(actionsXml),
                L"	<actions>\r\n"
                L"		<action content=\"%s\" arguments=\"%s\"/>\r\n"
                L"	</actions>\r\n",
                action, L"action=update"
            );

            swprintf_s(buf, TOAST_BUFSIZ, text, wszInfoURL, L"long", title, body, progressXml, actionsXml);

            ComPtr<IXmlDocument> inputXml;
            String2IXMLDocument(buf, &inputXml);

            toastData->HideAndShowToast(inputXml.Get());
        }

        return TRUE;
    }
    else
    {
        if (bFail)
        {
            printf("[Updates] Unable to check for updates because the remote server is unavailable.\n");
        }
        else
        {
            printf("[Updates] No updates are available.\n");
        }
        if (dwOperation == UPDATES_OP_CHECK || bIsInstall)
        {
            title[0] = 0; body[0] = 0; actionsXml[0] = 0; progressXml[0] = 0;

            if (bFail)
            {
                LoadStringW(hEPGui.get(), IDS_UPDATES_CHECKFAILED_T, title, ARRAYSIZE(title));
                LoadStringW(hEPGui.get(), IDS_UPDATES_CHECKFAILED_B, body, ARRAYSIZE(body));
            }
            else
            {
                LoadStringW(hEPGui.get(), IDS_UPDATES_ISLATEST_T, title, ARRAYSIZE(title));
                LoadStringW(hEPGui.get(), IDS_UPDATES_ISLATEST_B, body, ARRAYSIZE(body));
            }

            swprintf_s(buf, TOAST_BUFSIZ, text, L"", L"short", title, body, progressXml, actionsXml);

            ComPtr<IXmlDocument> inputXml;
            String2IXMLDocument(buf, &inputXml);

            toastData->HideAndShowToast(inputXml.Get());
        }

        return FALSE;
    }
}

extern "C" DWORD bAllocConsole;
extern "C" DWORD bShowUpdateToast;
extern "C" DWORD dwUpdatePolicy;

DWORD CheckForUpdatesThread(LPVOID params)
{
    DWORD timeout = (DWORD)(UINT_PTR)params;
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = RoInitialize(RO_INIT_MULTITHREADED);
    }

    ComPtr<IToastNotificationManagerStatics> toastStatics;
    if (SUCCEEDED(hr))
    {
        hr = RoGetActivationFactory(
            Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(),
            IID_PPV_ARGS(&toastStatics)
        );
    }

    if (SUCCEEDED(hr))
    {
        ComPtr<IToastNotificationManagerStatics2> toastStatics2;
        if (SUCCEEDED(RoGetActivationFactory(
            Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(),
            IID_PPV_ARGS(&toastStatics2)
        )))
        {
            ComPtr<IToastNotificationHistory> history;
            if (SUCCEEDED(toastStatics2->get_History(&history)))
            {
                history->Remove(Wrappers::HStringReference(L"ep_updates").Get());
            }
        }
    }

    while (TRUE)
    {
        HWND hShell_TrayWnd = FindWindowExW(nullptr, nullptr, L"Shell_TrayWnd", nullptr);
        if (hShell_TrayWnd)
        {
            Sleep(timeout);
            break;
        }
        Sleep(100);
    }
    printf("[Updates] Starting daemon.\n");

    ComPtr<IToastNotifier> notifier;
    if (SUCCEEDED(hr))
    {
        hr = toastStatics->CreateToastNotifierWithId(Wrappers::HStringReference(APPID).Get(), &notifier);
    }

    ComPtr<IToastNotificationFactory> notifFactory;
    if (SUCCEEDED(hr))
    {
        hr = RoGetActivationFactory(
            Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotification).Get(),
            IID_PPV_ARGS(&notifFactory)
        );
    }

    HANDLE hEvents[3];
    hEvents[0] = CreateEventW(nullptr, FALSE, FALSE, L"EP_Ev_CheckForUpdates_" _T(EP_CLSID));
    hEvents[1] = CreateEventW(nullptr, FALSE, FALSE, L"EP_Ev_InstallUpdates_" _T(EP_CLSID));
    hEvents[2] = CreateEventW(nullptr, FALSE, FALSE, L"EP_Ev_InstallUpdatesNoConfirm_" _T(EP_CLSID));
    if (hEvents[0] && hEvents[1] && hEvents[2])
    {
        ComPtr<CToastData> toastData = Make<CToastData>(notifier.Get(), notifFactory.Get());
        if (bShowUpdateToast)
        {
            ShowUpdateSuccessNotification(hModule, toastData.Get());

            HKEY hKey = nullptr;

            RegCreateKeyExW(
                HKEY_CURRENT_USER,
                TEXT(REGPATH),
                0,
                nullptr,
                REG_OPTION_NON_VOLATILE,
                KEY_READ | KEY_WOW64_64KEY | KEY_WRITE,
                nullptr,
                &hKey,
                nullptr
            );
            if (hKey == nullptr || hKey == INVALID_HANDLE_VALUE)
            {
                hKey = nullptr;
            }
            if (hKey)
            {
                bShowUpdateToast = FALSE;
                RegSetValueExW(
                    hKey,
                    TEXT("IsUpdatePending"),
                    0,
                    REG_DWORD,
                    (const BYTE*)&bShowUpdateToast,
                    sizeof(DWORD)
                );
                RegCloseKey(hKey);
            }
        }
        if (dwUpdatePolicy != UPDATE_POLICY_MANUAL)
        {
            InstallUpdatesIfAvailable(hModule, toastData.Get(), UPDATES_OP_DEFAULT, bAllocConsole, dwUpdatePolicy);
        }
        while (TRUE)
        {
            switch (WaitForMultipleObjects(3, hEvents, FALSE, INFINITE))
            {
            case WAIT_OBJECT_0:
            {
                InstallUpdatesIfAvailable(hModule, toastData.Get(), UPDATES_OP_CHECK, bAllocConsole, dwUpdatePolicy);
                break;
            }
            case WAIT_OBJECT_0 + 1:
            {
                InstallUpdatesIfAvailable(hModule, toastData.Get(), UPDATES_OP_INSTALL, bAllocConsole, dwUpdatePolicy);
                break;
            }
            case WAIT_OBJECT_0 + 2:
            {
                InstallUpdatesIfAvailable(hModule, toastData.Get(), UPDATES_OP_INSTALL_NO_CONFIRM, bAllocConsole, dwUpdatePolicy);
                break;
            }
            default:
            {
                break;
            }
            }
        }

        // Unreachable, but just in case
        CloseHandle(hEvents[0]);
        CloseHandle(hEvents[1]);
        CloseHandle(hEvents[2]);
    }

    return 0;
}

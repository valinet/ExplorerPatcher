#include "MagicExplorerButton.h"
#include <shlobj.h>
#include <shobjidl.h>
#include <commctrl.h>
#include <shellapi.h>
#include <strsafe.h>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")

static BOOL        g_MEB_Initialized = FALSE;
static BOOL        g_MEB_ReadOnly = FALSE;
static HWND        g_MEB_ReBar = NULL;
static HWND        g_MEB_Toolbar = NULL;
static HIMAGELIST  g_MEB_ImageList = NULL;
static MEB_Config  g_MEB_Cfg;

static void MEB_GetModuleBasePath(wchar_t* buf, size_t cch) {
    DWORD len = GetModuleFileNameW(NULL, buf, (DWORD)cch);
    if (len && len < cch) {
        for (DWORD i = len; i > 0; --i) {
            if (buf[i-1] == L'\\' || buf[i-1] == L'/') { buf[i-1] = 0; break; }
        }
    } else if (cch) buf[0] = 0;
}

static void MEB_BuildRelativePath(const wchar_t* relative, wchar_t* out, size_t cchOut) {
    wchar_t base[MAX_PATH];
    MEB_GetModuleBasePath(base, MAX_PATH);
    StringCchPrintfW(out, cchOut, L"%s\\%s", base, relative);
}

static WORD MEB_GetPrimaryLang(void) {
    LANGID lid = GetUserDefaultUILanguage();
    return PRIMARYLANGID(lid);
}

static void MEB_GetDefaultConfig(MEB_Config* cfg) {
    ZeroMemory(cfg, sizeof(*cfg));
    StringCchCopyW(cfg->label, 64, L"* Configurar *");
    StringCchCopyW(cfg->command, MAX_PATH, L"notepad.exe");
    if (MEB_GetPrimaryLang() == LANG_SPANISH) {
        StringCchCopyW(cfg->args, 512, L"MagicExplorerButton\\Readme.es.md");
    } else {
        StringCchCopyW(cfg->args, 512, L"MagicExplorerButton\\Readme.en.md");
    }
    StringCchCopyW(cfg->iconPath, MAX_PATH,    L"MagicExplorerButton\\favicon.ico");
    StringCchCopyW(cfg->hotIconPath, MAX_PATH, L"MagicExplorerButton\\favicon2.ico");
}

static BOOL MEB_ReadRegistry(MEB_Config* cfg) {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, MEB_REG_ROOT, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return FALSE;

    DWORD type, cb;
    wchar_t tmp[1024];
    struct { const wchar_t* name; wchar_t* target; DWORD cch; } fields[] = {
        { L"Label",   cfg->label,    64 },
        { L"Command", cfg->command,  MAX_PATH },
        { L"Args",    cfg->args,     512 },
        { L"Icon",    cfg->iconPath, MAX_PATH },
        { L"HotIcon", cfg->hotIconPath, MAX_PATH },
    };
    for (int i=0;i<(int)(sizeof(fields)/sizeof(fields[0]));++i) {
        cb = sizeof(tmp);
        if (RegQueryValueExW(hKey, fields[i].name, NULL, &type, (LPBYTE)tmp, &cb) == ERROR_SUCCESS && type == REG_SZ) {
            tmp[(cb/sizeof(wchar_t))-1] = 0;
            StringCchCopyW(fields[i].target, fields[i].cch, tmp);
        }
    }
    RegCloseKey(hKey);
    return TRUE;
}

static BOOL MEB_WriteDefaultsIfMissing(const MEB_Config* cfg) {
    HKEY hKey;
    DWORD disp;
    LONG st = RegCreateKeyExW(HKEY_CURRENT_USER, MEB_REG_ROOT, 0, NULL, 0, KEY_WRITE|KEY_READ, NULL, &hKey, &disp);
    if (st != ERROR_SUCCESS) return FALSE;
    if (disp == REG_CREATED_NEW_KEY) {
        RegSetValueExW(hKey, L"Label",   0, REG_SZ, (const BYTE*)cfg->label,   (DWORD)((wcslen(cfg->label)+1)*sizeof(wchar_t)));
        RegSetValueExW(hKey, L"Command", 0, REG_SZ, (const BYTE*)cfg->command, (DWORD)((wcslen(cfg->command)+1)*sizeof(wchar_t)));
        RegSetValueExW(hKey, L"Args",    0, REG_SZ, (const BYTE*)cfg->args,    (DWORD)((wcslen(cfg->args)+1)*sizeof(wchar_t)));
        RegSetValueExW(hKey, L"Icon",    0, REG_SZ, (const BYTE*)cfg->iconPath,(DWORD)((wcslen(cfg->iconPath)+1)*sizeof(wchar_t)));
        RegSetValueExW(hKey, L"HotIcon", 0, REG_SZ, (const BYTE*)cfg->hotIconPath,(DWORD)((wcslen(cfg->hotIconPath)+1)*sizeof(wchar_t)));
    }
    RegCloseKey(hKey);
    return TRUE;
}

static BOOL MEB_EnsureRegistryAndLoadConfig(MEB_Config* cfg, BOOL* readOnly) {
    *readOnly = FALSE;
    MEB_GetDefaultConfig(cfg);
    if (!MEB_WriteDefaultsIfMissing(cfg)) {
        *readOnly = TRUE;
        return TRUE;
    }
    MEB_ReadRegistry(cfg);
    return TRUE;
}

static HICON MEB_LoadIconFromCfg(const wchar_t* relPath, int dpi) {
    if (!relPath || !relPath[0]) return NULL;
    wchar_t full[MAX_PATH*2];
    MEB_BuildRelativePath(relPath, full, _countof(full));
    int size = MulDiv(32, dpi, 96);
    return (HICON)LoadImageW(NULL, full, IMAGE_ICON, size, size, LR_LOADFROMFILE);
}

static void MEB_DestroyImages(void) {
    if (g_MEB_ImageList) {
        ImageList_Destroy(g_MEB_ImageList);
        g_MEB_ImageList = NULL;
    }
}

static void MEB_ApplyToolbarVisuals(void) {
    if (!g_MEB_Toolbar) return;
    int dpi = GetDpiForWindow(g_MEB_Toolbar);
    MEB_DestroyImages();
    g_MEB_ImageList = ImageList_Create(MulDiv(32,dpi,96), MulDiv(32,dpi,96), ILC_COLOR32|ILC_MASK, 2, 0);

    HICON hNorm = MEB_LoadIconFromCfg(g_MEB_Cfg.iconPath, dpi);
    HICON hHot  = MEB_LoadIconFromCfg(g_MEB_Cfg.hotIconPath, dpi);
    if (!hNorm && hHot) hNorm = hHot;
    if (!hNorm) hNorm = LoadIcon(NULL, IDI_APPLICATION);
    if (!hHot)  hHot  = hNorm;

    int normIndex = ImageList_AddIcon(g_MEB_ImageList, hNorm);
    int hotIndex  = ImageList_AddIcon(g_MEB_ImageList, hHot);
    if (hNorm) DestroyIcon(hNorm);
    if (hHot && hotIndex != normIndex) DestroyIcon(hHot);

    SendMessage(g_MEB_Toolbar, TB_SETIMAGELIST, 0, (LPARAM)g_MEB_ImageList);
    SendMessage(g_MEB_Toolbar, TB_SETHOTIMAGELIST, 0, (LPARAM)g_MEB_ImageList);

    TBBUTTON btn = {0};
    btn.iBitmap = 0;
    btn.idCommand = MEB_BUTTON_ID;
    btn.fsState = TBSTATE_ENABLED;
    btn.fsStyle = BTNS_BUTTON | BTNS_SHOWTEXT | BTNS_AUTOSIZE;
    btn.iString = (INT_PTR)g_MEB_Cfg.label;

    int count = (int)SendMessage(g_MEB_Toolbar, TB_BUTTONCOUNT, 0, 0);
    for (int i = count - 1; i >= 0; --i) {
        SendMessage(g_MEB_Toolbar, TB_DELETEBUTTON, i, 0);
    }
    SendMessage(g_MEB_Toolbar, TB_ADDSTRING, 0, (LPARAM)g_MEB_Cfg.label);
    SendMessage(g_MEB_Toolbar, TB_ADDBUTTONS, 1, (LPARAM)&btn);
    SendMessage(g_MEB_Toolbar, TB_AUTOSIZE, 0, 0);
}

static HWND MEB_FindReBarFromExplorer(HWND hExplorer) {
    HWND result = NULL;
    HWND child = NULL;
    while ((child = FindWindowExW(hExplorer, child, NULL, NULL)) != NULL) {
        wchar_t cls[64];
        if (GetClassNameW(child, cls, 64) && _wcsicmp(cls, L"ReBarWindow32") == 0) {
            result = child;
            break;
        }
        HWND sub = FindWindowExW(child, NULL, L"ReBarWindow32", NULL);
        if (sub) { result = sub; break; }
    }
    return result;
}

BOOL MEB_IsEnabled(void) {
    return TRUE;
}

static BOOL MEB_GetActiveFolderPath(HWND hwndExplorer, wchar_t* out, size_t cchOut) {
    if (!out || cchOut == 0) return FALSE;
    out[0] = 0;
    BOOL ok = FALSE;
    CoInitialize(NULL);
    IShellWindows* pSW = NULL;
    if (SUCCEEDED(CoCreateInstance(&CLSID_ShellWindows, NULL, CLSCTX_ALL, &IID_IShellWindows, (void**)&pSW)) && pSW) {
        long count=0;
        if (SUCCEEDED(pSW->lpVtbl->get_Count(pSW, &count))) {
            for (long i=0; i<count; ++i) {
                VARIANT idx; VariantInit(&idx); idx.vt = VT_I4; idx.lVal = i;
                IDispatch* disp = NULL;
                if (SUCCEEDED(pSW->lpVtbl->Item(pSW, idx, &disp)) && disp) {
                    IWebBrowserApp* wba = NULL;
                    if (SUCCEEDED(disp->lpVtbl->QueryInterface(disp, &IID_IWebBrowserApp, (void**)&wba)) && wba) {
                        SHANDLE_PTR hwndW = 0;
                        if (SUCCEEDED(wba->lpVtbl->get_HWND(wba, &hwndW)) && (HWND)hwndW == hwndExplorer) {
                            IServiceProvider* prov = NULL;
                            if (SUCCEEDED(disp->lpVtbl->QueryInterface(disp, &IID_IServiceProvider, (void**)&prov)) && prov) {
                                IShellBrowser* browser = NULL;
                                if (SUCCEEDED(prov->lpVtbl->QueryService(prov, &SID_STopLevelBrowser, &IID_IShellBrowser, (void**)&browser)) && browser) {
                                    IShellView* view = NULL;
                                    if (SUCCEEDED(browser->lpVtbl->QueryActiveShellView(browser, &view)) && view) {
                                        IPersistFolder2* pf2 = NULL;
                                        if (SUCCEEDED(view->lpVtbl->GetFolder(view, &IID_IPersistFolder2, (void**)&pf2)) && pf2) {
                                            PIDLIST_ABSOLUTE pidl = NULL;
                                            if (SUCCEEDED(pf2->lpVtbl->GetCurFolder(pf2, &pidl)) && pidl) {
                                                PWSTR pszPath = NULL;
                                                if (SUCCEEDED(SHGetNameFromIDList(pidl, SIGDN_FILESYSPATH, &pszPath)) && pszPath) {
                                                    StringCchCopyW(out, cchOut, pszPath);
                                                    CoTaskMemFree(pszPath);
                                                    ok = TRUE;
                                                }
                                                CoTaskMemFree(pidl);
                                            }
                                            pf2->lpVtbl->Release(pf2);
                                        }
                                        view->lpVtbl->Release(view);
                                    }
                                    browser->lpVtbl->Release(browser);
                                }
                                prov->lpVtbl->Release(prov);
                            }
                        }
                        wba->lpVtbl->Release(wba);
                    }
                    disp->lpVtbl->Release(disp);
                }
            }
        }
        pSW->lpVtbl->Release(pSW);
    }
    CoUninitialize();
    return ok;
}

static void MEB_LaunchCommand(HWND hExplorer) {
    BOOL shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    BOOL ctrl  = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

    if (shift) {
        MEB_EnsureRegistryAndLoadConfig(&g_MEB_Cfg, &g_MEB_ReadOnly);
        MEB_ApplyToolbarVisuals();
        return;
    }
    if (ctrl) {
        g_MEB_ReadOnly = TRUE;
    }

    if (g_MEB_ReadOnly) {
        wchar_t readmeRel[128];
        if (MEB_GetPrimaryLang() == LANG_SPANISH)
            StringCchCopyW(readmeRel, 128, L"MagicExplorerButton\\Readme.es.md");
        else
            StringCchCopyW(readmeRel, 128, L"MagicExplorerButton\\Readme.en.md");

        wchar_t fullPath[MAX_PATH*2];
        MEB_BuildRelativePath(readmeRel, fullPath, _countof(fullPath));
        ShellExecuteW(hExplorer, L"open", L"notepad.exe", fullPath, NULL, SW_SHOWNORMAL);
        return;
    }

    wchar_t argsExpanded[1024];
    StringCchCopyW(argsExpanded, 1024, g_MEB_Cfg.args);
    if (wcsstr(argsExpanded, L"%PATH%")) {
        wchar_t currentPath[MAX_PATH];
        if (MEB_GetActiveFolderPath(hExplorer, currentPath, MAX_PATH)) {
            wchar_t* pos = wcsstr(argsExpanded, L"%PATH%");
            if (pos) {
                wchar_t buffer[1024];
                size_t preLen = pos - argsExpanded;
                StringCchPrintfW(buffer, 1024, L"%.*s\"%s\"%s",
                    (int)preLen, argsExpanded, currentPath, pos + 6);
                StringCchCopyW(argsExpanded, 1024, buffer);
            }
        }
    }

    wchar_t commandFull[MAX_PATH*2];
    if (wcschr(g_MEB_Cfg.command, L'\\') || wcschr(g_MEB_Cfg.command, L'/')) {
        MEB_BuildRelativePath(g_MEB_Cfg.command, commandFull, _countof(commandFull));
    } else {
        StringCchCopyW(commandFull, _countof(commandFull), g_MEB_Cfg.command);
    }

    if (commandFull[0] == 0) {
        MessageBoxW(hExplorer, L"El valor 'Command' está vacío. Edita el registro para configurarlo.", L"MagicExplorerButton", MB_OK|MB_ICONWARNING);
        g_MEB_ReadOnly = TRUE;
        MEB_LaunchCommand(hExplorer);
        return;
    }

    ShellExecuteW(hExplorer, L"open", commandFull,
                  g_MEB_Cfg.args[0] ? argsExpanded : NULL,
                  NULL, SW_SHOWNORMAL);
}

BOOL MEB_HandleCommand(HWND hExplorer, WPARAM wParam, LPARAM lParam) {
    if (LOWORD(wParam) == MEB_BUTTON_ID) {
        MEB_LaunchCommand(hExplorer);
        return TRUE;
    }
    return FALSE;
}

void MEB_HandleNotify(HWND hExplorer, LPNMHDR hdr) {
    if (!hdr) return;
    if (hdr->hwndFrom == g_MEB_Toolbar) {
        if (hdr->code == NM_RCLICK) {
            MEB_EnsureRegistryAndLoadConfig(&g_MEB_Cfg, &g_MEB_ReadOnly);
            MEB_ApplyToolbarVisuals();
        }
    }
}

static void MEB_CreateToolbar(HWND rebar) {
    if (g_MEB_Toolbar) return;
    g_MEB_Toolbar = CreateWindowExW(
        0, TOOLBARCLASSNAMEW, NULL,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT |
        CCS_NOPARENTALIGN | CCS_NORESIZE,
        0, 0, 0, 0, rebar, NULL, GetModuleHandleW(NULL), NULL
    );
    if (!g_MEB_Toolbar) return;
    SendMessage(g_MEB_Toolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    MEB_ApplyToolbarVisuals();
}

static void MEB_InsertBand(HWND rebar) {
    if (!g_MEB_Toolbar) return;
    REBARBANDINFO rbbi = {0};
    rbbi.cbSize = sizeof(rbbi);
    rbbi.fMask  = RBBIM_CHILD | RBBIM_STYLE | RBBIM_SIZE;
    rbbi.fStyle = RBBS_NOGRIPPER | RBBS_VARIABLEHEIGHT;
    rbbi.hwndChild = g_MEB_Toolbar;
    rbbi.cx = 200;
    SendMessage(rebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbbi);
}

void MEB_InsertOrUpdate(HWND hExplorer) {
    if (!MEB_IsEnabled()) return;
    if (!g_MEB_Initialized) {
        MEB_EnsureRegistryAndLoadConfig(&g_MEB_Cfg, &g_MEB_ReadOnly);
        g_MEB_Initialized = TRUE;
    }
    if (!g_MEB_ReBar) {
        g_MEB_ReBar = MEB_FindReBarFromExplorer(hExplorer);
    }
    if (!g_MEB_ReBar) return;
    if (!g_MEB_Toolbar) {
        MEB_CreateToolbar(g_MEB_ReBar);
        MEB_InsertBand(g_MEB_ReBar);
    } else {
        MEB_ApplyToolbarVisuals();
    }
}
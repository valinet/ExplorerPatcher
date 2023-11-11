#ifndef _H_SYMBOLS_H_
#define _H_SYMBOLS_H_
#include <Windows.h>
#define _LIBVALINET_INCLUDE_UNIVERSAL
#include <valinet/universal/toast/toast.h>
#include <valinet/utility/osversion.h>
#include <roapi.h>
#include "utility.h"
#include "../ep_gui/resources/EPSharedResources.h"

#define EXIT_CODE_EXPLORER 1

#define TWINUI_PCSHELL_SB_NAME "twinui.pcshell"
#define TWINUI_PCSHELL_SB_0 "CImmersiveContextMenuOwnerDrawHelper::s_ContextMenuWndProc"
#define TWINUI_PCSHELL_SB_1 "CLauncherTipContextMenu::GetMenuItemsAsync"
#define TWINUI_PCSHELL_SB_2 "ImmersiveContextMenuHelper::ApplyOwnerDrawToMenu"
#define TWINUI_PCSHELL_SB_3 "ImmersiveContextMenuHelper::RemoveOwnerDrawFromMenu"
#define TWINUI_PCSHELL_SB_4 "CLauncherTipContextMenu::_ExecuteShutdownCommand"
#define TWINUI_PCSHELL_SB_5 "CLauncherTipContextMenu::_ExecuteCommand"
#define TWINUI_PCSHELL_SB_6 "CLauncherTipContextMenu::ShowLauncherTipContextMenu"
#define TWINUI_PCSHELL_SB_7 "IsUndockedAssetAvailable"
#define TWINUI_PCSHELL_SB_8 "winrt::Windows::Internal::Shell::implementation::MeetAndChatManager::OnMessage" // should be always last
#define TWINUI_PCSHELL_SB_LAST TWINUI_PCSHELL_SB_8
#define TWINUI_PCSHELL_SB_CNT 9
#define STARTDOCKED_SB_NAME "StartDocked"
#define STARTDOCKED_SB_0 "StartDocked::LauncherFrame::ShowAllApps" // UNUSED
#define STARTDOCKED_SB_1 "StartDocked::LauncherFrame::ShowAllApps"
#define STARTDOCKED_SB_2 "StartDocked::LauncherFrame::OnVisibilityChanged"
#define STARTDOCKED_SB_3 "StartDocked::SystemListPolicyProvider::GetMaximumFrequentApps"
#define STARTDOCKED_SB_4 "StartDocked::StartSizingFrame::StartSizingFrame"
#define STARTDOCKED_SB_CNT 5
#define STARTUI_SB_NAME "StartUI"
#define STARTUI_SB_0 "StartUI::SystemListPolicyProvider::GetMaximumFrequentApps"
#define STARTUI_SB_CNT 1
#pragma pack(push, 1)
typedef struct symbols_addr
{
    DWORD twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT];
    DWORD startdocked_PTRS[STARTDOCKED_SB_CNT];
    DWORD startui_PTRS[STARTUI_SB_CNT];
} symbols_addr;
#pragma pack(pop)

typedef struct _LoadSymbolsResult
{
    BOOL bSuccess : 1;
    BOOL bNeedToDownloadTwinuiPcshellSymbols : 1;
    BOOL bNeedToDownloadStartDockedSymbols : 1;
    BOOL bNeedToDownloadStartUISymbols : 1;
} LoadSymbolsResult;

inline BOOL NeedToDownloadSymbols(const LoadSymbolsResult* pLoadResult)
{
    return pLoadResult->bNeedToDownloadTwinuiPcshellSymbols || pLoadResult->bNeedToDownloadStartDockedSymbols || pLoadResult->bNeedToDownloadStartUISymbols;
}

typedef struct _DownloadSymbolsParams
{
    HMODULE hModule;
    BOOL bVerbose;
    LoadSymbolsResult loadResult;
} DownloadSymbolsParams;
DWORD DownloadSymbols(DownloadSymbolsParams* params);

LoadSymbolsResult LoadSymbols(symbols_addr* symbols_PTRS);

inline BOOL IsBuild(RTL_OSVERSIONINFOW rovi, DWORD32 ubr, DWORD BuildNumber, DWORD BuildMinor)
{
    return (rovi.dwMajorVersion == 10 &&
        rovi.dwMinorVersion == 0 &&
        rovi.dwBuildNumber == BuildNumber &&
        ubr == BuildMinor);
}
#endif
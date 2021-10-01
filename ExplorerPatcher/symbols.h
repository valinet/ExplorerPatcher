#ifndef _H_SYMBOLS_H_
#define _H_SYMBOLS_H_
#include <Windows.h>
#define _LIBVALINET_INCLUDE_UNIVERSAL
#include <valinet/universal/toast/toast.h>
#include <valinet/utility/osversion.h>
#include <roapi.h>
#include "utility.h"

#define EXIT_CODE_EXPLORER 1

#define SYMBOLS_RELATIVE_PATH "\\ExplorerPatcher"
#define TWINUI_PCSHELL_SB_NAME "twinui.pcshell"
#define TWINUI_PCSHELL_SB_0 "CImmersiveContextMenuOwnerDrawHelper::s_ContextMenuWndProc"
#define TWINUI_PCSHELL_SB_1 "CLauncherTipContextMenu::GetMenuItemsAsync"
#define TWINUI_PCSHELL_SB_2 "ImmersiveContextMenuHelper::ApplyOwnerDrawToMenu"
#define TWINUI_PCSHELL_SB_3 "ImmersiveContextMenuHelper::RemoveOwnerDrawFromMenu"
#define TWINUI_PCSHELL_SB_4 "CLauncherTipContextMenu::_ExecuteShutdownCommand"
#define TWINUI_PCSHELL_SB_5 "CLauncherTipContextMenu::_ExecuteCommand"
#define TWINUI_PCSHELL_SB_6 "CLauncherTipContextMenu::ShowLauncherTipContextMenu"
#define TWINUI_PCSHELL_SB_7 "winrt::Windows::Internal::Shell::implementation::MeetAndChatManager::OnMessage" // should be always last
#define TWINUI_PCSHELL_SB_CNT 8
#define STARTDOCKED_SB_NAME "StartDocked"
#define STARTDOCKED_SB_0 "StartDocked::LauncherFrame::ShowAllApps" // UNUSED
#define STARTDOCKED_SB_1 "StartDocked::LauncherFrame::ShowAllApps"
#define STARTDOCKED_SB_2 "StartDocked::LauncherFrame::OnVisibilityChanged"
#define STARTDOCKED_SB_3 "StartDocked::SystemListPolicyProvider::GetMaximumFrequentApps"
#define STARTDOCKED_SB_4 "StartDocked::StartSizingFrame::StartSizingFrame"
#define STARTDOCKED_SB_CNT 5
#pragma pack(push, 1)
typedef struct symbols_addr
{
    DWORD twinui_pcshell_PTRS[TWINUI_PCSHELL_SB_CNT];
    DWORD startdocked_PTRS[STARTDOCKED_SB_CNT];
} symbols_addr;
#pragma pack(pop)

typedef struct _DownloadSymbolsParams
{
    HMODULE hModule;
} DownloadSymbolsParams;
DWORD DownloadSymbols(DownloadSymbolsParams* params);

BOOL LoadSymbols(symbols_addr* symbols_PTRS);
#endif
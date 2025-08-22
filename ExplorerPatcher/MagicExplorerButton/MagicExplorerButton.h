#pragma once
#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEB_BUTTON_ID 19001
#define MEB_REG_ROOT L"Software\\ExplorerPatcher\\MagicExplorerButton"

typedef struct _MEB_Config {
    wchar_t label[64];
    wchar_t command[MAX_PATH];
    wchar_t args[512];
    wchar_t iconPath[MAX_PATH];
    wchar_t hotIconPath[MAX_PATH];
} MEB_Config;

BOOL MEB_IsEnabled(void);                 // Ahora siempre TRUE (placeholder)
void MEB_InsertOrUpdate(HWND hExplorer);  // Inserta o actualiza la banda
void MEB_HandleNotify(HWND hExplorer, LPNMHDR hdr);
BOOL MEB_HandleCommand(HWND hExplorer, WPARAM wParam, LPARAM lParam);

// Función de conveniencia para tu código existente
static inline void IfMagicExplorerButtonEnabled(HWND hExplorer) {
    if (MEB_IsEnabled()) {
        MEB_InsertOrUpdate(hExplorer);
    }
}

#ifdef __cplusplus
}
#endif

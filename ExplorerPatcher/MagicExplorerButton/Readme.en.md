# Magic Explorer Button

This button is added by an extended ExplorerPatcher build.

## Configuration

Registry path:
`HKCU\Software\ExplorerPatcher\MagicExplorerButton`

Values (REG_SZ):
- Label
- Command
- Args
- Icon
- HotIcon (optional)

Example defaults:
- Label = * Configurar *
- Command = notepad.exe
- Args = MagicExplorerButton\Readme.en.md
- Icon = MagicExplorerButton\favicon.ico
- HotIcon = MagicExplorerButton\favicon2.ico

If the registry key cannot be created or read, the button falls back to opening this README (read-only mode).

Interactions:
- Left click: run command (with %PATH% expansion if present).
- Right click OR Shift+Left click: reload configuration.
- Ctrl+Left click: force open this README (ignores command).
- In read-only mode any left click opens this README.

You can use %PATH% inside Args to inject the active folder path (only the current tab, no multi-tab enumeration yet).

No timers or background threads are used.

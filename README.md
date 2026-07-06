# ExplorerPatcher

This project aims to enhance the working environment on Windows.

## How to?

1. Download the latest setup program from the [Releases page](https://github.com/valinet/ExplorerPatcher/releases/latest).
   * Choose `ep_setup.exe` if your device uses an Intel or AMD processor, or `ep_setup_arm64.exe` if your device uses a Snapdragon processor.
2. Run the installer. It will automatically prompt for elevation, after which it will close `explorer.exe` and install the necessary files. When done, you will see the desktop again and the Windows 10 taskbar.
3. Right-click the taskbar and choose "Properties".
4. To change the taskbar style, go to the "Taskbar" section and look for "Taskbar style".
5. To use the Windows 10 Start menu, go to the "Start menu" section and change the Start menu style to Windows 10.
   * 如果切换后仍然显示 Windows 11 开始菜单，请确认 `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced\Start_ShowClassicMode` 的值为 `1`，然后重启 `StartMenuExperienceHost.exe` 和 `explorer.exe`。
   * 也可以使用 `tools/win10-style/Apply-Win10ExplorerStyle.ps1` 一键应用 Windows 10 风格任务栏、右下角托盘图标全部显示、经典右键菜单和 Windows 10 磁贴开始菜单；需要回退时运行 `tools/win10-style/Undo-Win10ExplorerStyle.ps1`。
6. To use the Windows 10 Alt+Tab, go to the "Window switcher" section and change the "Window switcher (Alt+Tab) style" to Windows 10.
7. Feel free to check other configuration options.

That's it!

**Note:** Some features may be unavailable on some Windows versions.

## Uninstalling

* Right-click the taskbar then click "Properties" or search for "ExplorerPatcher", and go to "Uninstall" section or
* Use "Programs and Features" in Control Panel, or "Apps and features" in the Settings app or
* Run `ep_setup.exe /uninstall` or
* Rename `ep_setup.exe` to `ep_uninstall.exe` and run that.

## Updating

* The program features built-in updates: go to "Properties" - "Updates" to configure, check for and install the latest updates. Learn more [here](https://github.com/valinet/ExplorerPatcher/wiki/Configure-updates).
* Download the latest version's [setup file for x64](https://github.com/valinet/ExplorerPatcher/releases/latest/download/ep_setup.exe) or [setup file for ARM64](https://github.com/valinet/ExplorerPatcher/releases/latest/download/ep_setup_arm64.exe) and simply run it.

## Donate

If you find this project essential to your daily life, please consider donating to support the development through the [Sponsor](https://github.com/valinet/ExplorerPatcher?sponsor) button at the top of this page, so that we can continue to keep supporting newer Windows builds.

## Discord Server

Join our Discord server if you need support, want to chat regarding this project, or just want to hang out with us!

[![Join on Discord](https://discordapp.com/api/guilds/1155912047897350204/widget.png?style=shield)](https://discord.gg/gsPcfqHTD2)

[Read more](https://github.com/valinet/ExplorerPatcher/wiki)

# Explorer Patcher Change log

This document includes the same release notes as in the [Releases](https://github.com/valinet/ExplorerPatcher/releases) section on GitHub.

## 22000.194.0.21

Tested on build: 22000.194.

* Implemented configuration GUI; to access it, right click the Start button (or press `Win`+`X`) and choose "Properties"

## 22000.194.0.20

Tested on build: 22000.194.

* Huge code refactoring, improved memory patching
* Updated README with better description of the software and how to use it
* Drastically reduced the number of symbols required (around 40MB to download, instead of over 400MB previously)
* Improved Start menu and search positioning, now it is not necessary to have the DLL in `C:\Windows\SystemApps\MicrosoftWindows.Client.CBS_cw5n1h2txyewy`, please remove it from there.
* Skin "Bluetooth" pop-up menu
* Option to hide the search bar in File Explorer completely
* Option to disable the control center button in the taskbar
* Removed the option to disable the modern search box in File Explorer. Instead, you now run a command which disables it globally on your user account (works in "Open" dialogs as well); read [here](https://github.com/valinet/ExplorerPatcher#disable-the-modern-search-box-in-File-Explorer)
* Removed the option to disable the immersive (new) context menu in File Explorer. Instead, you now run a command which disables it globally on your user account; read [here](https://github.com/valinet/ExplorerPatcher#disable-the-immersive-context-menu)
* Ability to disable command bar is described [here](https://github.com/valinet/ExplorerPatcher#disable-the-command-bar-in-File-Explorer)
* Option to apply Mica effect on File Explorer windows (requires `StartIsBack64.dll`), read [here](https://github.com/valinet/ExplorerPatcher#configuration)
* Option to skin system tray icons to match Windows 11 style (requires `StartisBack64.dll`), read [here](https://github.com/valinet/ExplorerPatcher#configuration)

## 22449.1000.0.18

Tested on the following builds: 22449.1000, 22000.176, 22000.1.

New in this version:

* Ability to disable the "modern search box" in File Explorer and uses the classic functional search from early Windows 10 versions or Windows 7/8. To disable this and still use the new search, set `General\AllowModernSearchBox = 1` in `settings.ini`

Fixes:

* Much improved algorithm for enabling the classic taskbar after symbols have downloaded on newer builds
* Restored compatibility with RTM build of Windows 11 (22000.1)
* Fixes the "modern search box" not working correctly when the DLL is used in `C:\Windows\SystemApps\MicrosoftWindows.Client.CBS_cw5n1h2txyewy`

## 22449.1000.0.16

New in this version:

- Compatibility with OS build 22449.1000.0.16.
- Fixed bug that prevented console from showing when the `AllocConsole` setting was specified in `settings.ini`

## 22000.168.0.14

* Start menu and search now respect the taskbar alignment setting
* Ability to customize the number of "Most used" apps in the Start menu apps list
* Symbols are automatically downloaded for Start menu and search; to have the application work with those two, place the DLL in the following additional 2 locations:
  * `C:\Windows\SystemApps\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy`
  * `C:\Windows\SystemApps\MicrosoftWindows.Client.CBS_cw5n1h2txyewy`

## 22000.168.0.12

* Support for showing the app list by default in the Windows 11 Start menu; to enable this feature, copy the DLL to `C:\Windows\SystemApps\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy` and restart Explorer (works only on 22000.168 for the moment, will be generally available after more testing is performed).
* `Win+X` is now shown correctly on multi monitor setups
* Other bug fixes

## 22000.168.0.11

Fixes [#3](https://github.com/valinet/ExplorerPatcher/issues/3) and [#10](https://github.com/valinet/ExplorerPatcher/issues/10).

## 22000.168.0.10

Improved Explorer hooking.

The application now comes in the form of a single DLL file (`dxgi.dll`) which you have to place in `%windir%` (usually `C:\Windows`). Restart Explorer and that's it.

Please make sure to uninstall the old version before using this new one.

## 22000.168.0.9

Implements [#6](https://github.com/valinet/ExplorerPatcher/issues/6) (option to revert to classic context menu). To disable this feature, add this to the settings.ini file:

```
[General]
AllowImmersiveContextMenus=1
```

## 22000.168.0.8

The popup menu for "Safe to Remove Hardware" is now skinned in the same  style as the Win+X menu and the taskbar context menus, in order to  improve UI consistency.

## 22000.168.0.7

Enables compatibility with [ArchiveMenu](https://github.com/valinet/archivemenu).

## 22000.168.0.6

Fixes [#5](https://github.com/valinet/ExplorerPatcher/issues/5) (removes the delay at logon on newer builds like 22000.168; the bug is similar to the effect introduced by `UndockingDisabled` on these newer builds).

## 22000.1.0.5

Offsets are now determined at runtime

The application was tested on builds 22000.1 and 22000.168.

- Library downloads and parses symbols in order to determine function hooking offsets at runtime and saves the data in a "settings.ini" file located in the application folder for future use; the file is invalidated when a new OS build is detected
- The main executable attempts to determine the location where a jump has to be patched out so that Explorer remains on the 'show old taskbar' code path; it will systematically patch each jz/jnz instruction and will check whether Explorer still runs fine, and, if it does so and does not crash, whether the old taskbar got actually shown; once the offset is determined, it is saved in the "settings.ini" file for future use
- Please have an unmetered active working Internet connection when running for the first time
- Messages from the patcher (i.e. install/uninstall successful message, symbol downloading message) will now display in a toast (Windows 10 notification) if possible; when Explorer is not running, it falls back to using standard MessageBox'es
- Disabled the pre/post build command that restarted sihost.exe in Debug builds

## 22000.1.0.4

New in this release

- Win+X combination is now handled and opens the power user menu

Of note from previous releases:

- Start menu is now displayed on monitor containing the cursor when  invoked with the Windows key if the appropriate configuration is made  into the registry (enables a functionality which was previously  available in Windows 8.1). To activate this, follow this tutorial: https://www.tenforums.com/tutorials/5943-start-menu-open-main-last-display-windows-10-a.html
- The power user menu (Win+X) is now skinned using the system theme, as it was in Windows 10

## 22000.1.0.3

Start menu is now displayed on monitor containing the cursor when  invoked with the Windows key if the appropriate configuration is made  into the registry (enables a functionality which was previously  available in Windows 8.1).

To activate this, follow this tutorial: https://www.tenforums.com/tutorials/5943-start-menu-open-main-last-display-windows-10-a.html

## 22000.1.0.2

Fixes [#1](https://github.com/valinet/ExplorerPatcher/issues/1) (the menu is now skinned using the system theme, as it was in Windows 10).

## 22000.1.0.1

Added functionality to bring back the power user menu (Win+X).

## 22000.1.0.0

Initial version of the application. Only x64 builds.

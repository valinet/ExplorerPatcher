# Explorer Patcher Changelog

This document includes the same release notes as in the [Releases](https://github.com/valinet/ExplorerPatcher/releases) section on GitHub.

## 22000.168.0.7

Enables compatibility with [ArchiveMenu](https://github.com/valinet/archivemenu).

## 22000.168.0.6

Fixes [#5](https://github.com/valinet/ExplorerPatcher/issues/5) (removes the delay at logon on newer builds like 22000.168; the bug is similar to the effect introduced by `UndockingDisabled` on these newer builds).

## 22000.1.0.5

Offsets are now determined at runtime

The application was tested on builds 22000.1 and 22000.168.

- Library downloads and parses symbols in order to determine
   function hooking offsets at runtime and saves the data in a
   "settings.ini" file located in the application folder for future
   use; the file is invalidated when a new OS build is detected
- The main executable attempts to determine the location where a
   jump has to be patched out so that Explorer remains on the 'show
   old taskbar' code path; it will systematically patch each jz/jnz
   instruction and will check whether Explorer still runs fine, and,
   if it does so and does not crash, whether the old taskbar got
   actually shown; once the offset is determined, it is saved in the
   "settings.ini" file for future use
- Please have an unmetered active working Internet connection when
   running for the first time
- Messages from the patcher (i.e. install/uninstall successful
   message, symbol downloading message) will now display in a toast
   (Windows 10 notification) if possible; when Explorer is not
   running, it falls back to using standard MessageBox'es
- Disabled the pre/post build command that restarted sihost.exe in
   Debug builds

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
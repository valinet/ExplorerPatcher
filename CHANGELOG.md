# Explorer Patcher Change log

This document includes the same release notes as in the [Releases](https://github.com/valinet/ExplorerPatcher/releases) section on GitHub.

## 22621.3880.66

Tested on OS builds 19045.4598, 22621.3296, 22621.3810, 26120.961, and 26244.5000. (Note: 22621 and 22631 share the same OS files)

##### 1

* Taskbar10: Introduced a new taskbar implementation: Windows 10 (ExplorerPatcher). (146070d, 0b86e55)
  * You can try this implementation out by changing the "Taskbar style" to "Windows 10 (ExplorerPatcher)".
  * For now, this is **only available for builds 22621, 22631, and 22635.** Other builds will not have the option.
  * Refer to [this wiki article](https://github.com/valinet/ExplorerPatcher/wiki/ExplorerPatcher's-taskbar-implementation) for more information including important ones.

##### 2

* Taskbar10: Due to false positive antivirus detections, the new taskbar implementation is no longer bundled in the setup program. (48c2a75)
  * If you want to use the new taskbar implementation, you can download the appropriate DLL for your system from the [Releases](https://github.com/ExplorerPatcher/ep_taskbar_releases/releases/latest) page of its releases repository, and then manually putting it in `C:\Program Files\ExplorerPatcher` without the architecture specifier.
  * For example, for 226xx builds on x64-based systems, download `ep_taskbar.2.amd64.dll`, rename to `ep_taskbar.2.dll`, and lastly put it in `C:\Program Files\ExplorerPatcher`.

##### 3

* Introduced support for ARM64 devices. (992b3a6, 2e4e4f5, b76c0e4, c9884b2, 57f63ad, 78788ec, 4799b4b, 5d0d218)
  * These builds are only tested on and made to work with 24H2 ARM64 builds. Older ARM64 Windows versions than 24H2 may not work as expected.
  * An ARM64 device to support the making and testing of ARM64 builds is not cheap, so please consider [donating @Amrsatrio via Ko-fi](https://ko-fi.com/amrsatrio) to recoup the costs.
* Added an "Update now" button into update notifications for easier updating. (2b9c747, 8c16a9a)
* Revised how files are packed in ep_setup for smaller size and easier maintenance. (30579b0, b253625, 04fd2b7, db54ce9, 126c024, c0201ff)
* ⚠️ **Note for users with the pre-release versions enabled:** Versions before this one will not be able to update to this version or later versions due to the updater code of previous versions not supporting multiple binaries in one GitHub release. Please manually update by downloading the appropriate binary from this page.

## 22621.3527.65

Tested on OS builds 22621.3296, 22621.3447, 22621.3527, 22635.3566, 26058.1000, 26120.461, and 26200.5001. (Note: 22621 and 22631 share the same OS files)

##### 1

* Taskbar10: The Windows 10 taskbar option is now no longer available on 26002+. (#3053, e57a6b0)
  * This is to comply with Microsoft's removal of the stock Windows 10 taskbar in `explorer.exe` of said builds.
* Start10: Fixed Pin to Start on 226xx.3420+ (22H2, 23H2) and 25169+ (24H2). (232fe6b)
* Start10: Reverted the menu closing delay fix when EP is injected only into `StartMenuExperienceHost.exe` for now. (e59c34c)

##### 2

* Start10: Fixed a bug where the recently introduced "account suggestions" prevents the user tile menu from opening on later 22H2/23H2 builds and 24H2. (d11445a)

##### 3

* All: Updated some patterns to work with 22635.3430+ (Beta) and recent 24H2 builds. (6d22947)
  * This should fix the Windows 10 start menu crashing and Win+X not working on both aforementioned builds when symbols are not yet downloaded.
  
##### 4

* Updates: Support for `ep_make`, a new script which builds ExplorerPatcher locally on your computer. Read more [here](https://github.com/valinet/ep_make). (80592f6)
* GUI: Reorganized "About" and "Uninstall" sections. (4794713)
* ep_weather: Fixed alignment.

##### 5

* Weather: Layout fixes. (57b44d2, 2112a18)

## 22621.3296.64

Tested on OS builds 22000.2538, 22621.1992, 22621.3155, 22621.3235, 22621.3296, 25951.1000, and 26058.1000.

##### 1

* Taskbar10: Fixed a bug where SCOOBE would repeatedly crash Explorer when Language Switcher is set to anything other than Windows 10 (the default). (fe7f800, 5c35f58)
* Taskbar10: Refined the method for aligning the Windows 11 Start menu and Search flyouts when using the Windows 10 taskbar on 22621.2792+. (8f84a96)
  * This should fix related crashes during logon and screen resolution change on 26063+.
* Taskbar10: Revised the method for restoring acrylic to the Windows 10 taskbar on 22621+. (5e7bad2)
  * This should fix the taskbar being fully transparent on recent builds such as 22635.3066 and 22621/22631.3296 despite not having any other customization software.
* ExplorerPatcher should now avoid further injection when the system is in safe mode. (95ea9e7)
* Setup: Moved uninstallation prompt dialog existence check to the GUI. (0589a25)
* Various changes to prepare for the alternate taskbar reimplementation that will be released in the future. (a0885c6, 0791bd7, fc61884, 623ecee)

##### 2

* Taskbar10: Revised the method for disabling DisableWin10Taskbar present on 26002+. (913b2d0)

##### 3

* Start10: Support for OS builds 226xx.3420+ and 24H2, including fixed animations (5e25663, c286ab5).
* Start10: Prevent menu closing delay when patching standalone (without ExplorerPatcher injecting `explorer.exe`) (45bd735).

## 22621.3007.63

Tested on OS builds 22000.2538, 22621.1992, 22621.3007, 22621.3085, and 22621.3155.

##### 1

* **Fixed a bug where `explorer.exe` would crash repeatedly when the system is in OOBE.** (36ebe5a)
  * ExplorerPatcher now no longer loads if it detects that the system is in OOBE or in credential reset.
* Taskbar10: The Network and Battery flyouts on later 22621 builds onwards and Windows 10 now open instantly without issues. (97fd483)
* Taskbar10: Allowed the use of search box (without highlights) on Windows 11. (0157ecc)
  * **The behavior when the Start or Search menu is open is currently not the same as Windows 10, and we have no plans to fix this yet. Please do not make new Issues regarding this.**
* Start10: Added proper handling when the Windows 10 start menu is not available (e.g. 24H2/Canary builds). (3c8809e)
* Start10: Removed the original method for fixing Jump List (right click) views. (79b0f68)
* File Explorer: The address bar shrinking is now more accurate with pixel-perfect height compared to Windows 7, 8.1, and 10 (without the modern search). (e0b97e2)
* GUI: Added "Uninstall" section containing a button to launch the uninstaller. (0c5021b)
* Setup: There should now be fewer .prev files, and uninstallation should be cleaner as well. (296c6a0)
* Symbols: Added `explorer.exe` symbols for 22621+ and unified the method for Windows 10 Alt+Tab on 22000. (1f2e2c4)
* Localization: Added translations for Lithuanian, Polish, Russian, and Turkish.

##### 2

* Taskbar10: Improved animation performance when centering and/or EP Weather is not enabled, also fixed search box positioning on small taskbar without centering. (22d9e3c)
* Setup: Fixed a bug that placed `wincorlib.dll` on Windows 10 when it is not supposed to, causing the start menu to crash. (610ba7f)

##### 3

* Taskbar10: Fixed flyout positioning on Windows 11 26058+. (dfe340d)
* Slightly improved performance when interacting with the taskbar, both new and old. (dfe340d)

##### 4

* Setup: Reverted the method for ending `explorer.exe` and its subprocesses. (fdc357b)

## 22621.2861.62

Tested on OS builds 22621.2715, 22621.2861, 22631.2787, 22631.2861, 22635.2915, and 23590.1000.

##### 1

* Taskbar10: Various *important* fixes: (ec68783)
  * Revised the method for enabling the old taskbar due to a very rare issue where the old taskbar couldn't be enabled with the previous method. (#2499)
  * Fixed crash on 25921+ due to the removal of pnidui.dll. (#2558)
  * Fixed potential stability issues when using the new taskbar on 22621.2787+.
* Taskbar10: Fixed white boxes on submenus when context menu skinning is disabled. (72f1458)
* File Explorer: Fixed crashes when using Windows 7/10 control interface on OS builds 22635.2915+. (3a1b8b8)
* Localization: Added translations for French, German, Hungarian, Korean, Romanian, and Ukrainian.
  * The properties window has been made slightly wider to accomodate the newly added languages. (#2574)
* Localization: Added a language switcher to the About section of Properties window. (7c3be29, a7a3d27)

##### 2

* Symbols: Fixed languages with longer strings such as French crashing Explorer when attempting to download symbols. (ce9f973)

**Note:** Due to the breakages as well as frequent changes happening in Canary builds, we strongly do not recommend using ExplorerPatcher on Canary builds for now.

## 22621.2506.60

Tested on OS builds 22000.2416, 22000.2538, 22621.2361, 22621.2506, 22621.2715, 22631.2787, 23585.1001, and 23590.1000.

#### Details

##### 1

* Taskbar10: Fixed Windows 11 Start menu and Search positioning on builds 22621.2787+ and 23545+ (Dev). (ac268b1, 7d0cdde)
* File Explorer: Added option to disable the modern navigation bar of Moment 4. (2dc1340)
* File Explorer: Restored "Apply Mica" functionality on OS builds 22621+. (f62c532)
* Localization: Officially added translations for the following languages: Chinese (Simplified), Chinese (Traditional), Dutch, Indonesian, Japanese
  * Thanks to [everyone involved](https://github.com/valinet/ExplorerPatcher-L10N#acknowledgements)!
* GUI: Decoupled the Properties window into `ep_gui.dll` from the main DLL in order to reduce the main DLL size and to allow scalable localization. (f6f6d89, 639d7aa)
  * `rundll32 C:\Windows\dxgi.dll,ZZGUI` will continue to work as before.

#### ⚠️ Important notice for translators ⚠️

In this update, most if not all user-facing parts of ExplorerPatcher have been made localizable.

* The English texts have been put together into [here](https://github.com/valinet/ExplorerPatcher/tree/master/ep_gui/resources/lang) and [here](https://github.com/valinet/ExplorerPatcher/tree/master/ep_setup/resources/lang).
* Non-English texts have been designed to be put into [this separate repository](https://github.com/valinet/ExplorerPatcher-L10N). Feel free to make a PR there if you want to contribute to translations.
* Some texts have been updated to be more concise and accurate, so for existing translation fork maintainers, please double check the translations before making a PR to the said repository.
* Also for translation fork maintainers, a large number of conflicts will happen if you decide to continue merging changes from the main repository.
* Please let us know through Issues if there are still user-facing parts of ExplorerPatcher that are not localizable.

We apologize for the additional work that this change might cause. We hope that this one-time change will make it easier for translators to localize ExplorerPatcher and also easier for both translators and users to keep ExplorerPatcher up to date.

## 22621.2428.59

Tested on OS builds 22000.2416, 22621.2428, 23555.1000, and 23560.1000.

#### Details

##### 1

Note: After updating to this version, the symbols will be re-downloaded even if they have been downloaded before.

* Taskbar10: Fixed Control Center and Toast Center positioning on build 25951 (Canary). (dca0b3a)
* Taskbar10: Fixed start menu position when the taskbar is at the left or right side on Moment 4 builds. (a57471f)
* Taskbar10: Fixed the Windows 10 taskbar background patch to not crash anymore on build 25951 (Canary). (b52bd79)
* Taskbar10: Made classic theme taskbar fonts more accurate. Thanks @aubymori! (8fc53a1)
* Start10: Fixed a bug where certain texts in the Windows 10 Start menu stayed in English. (655e62c, 5321766)
* Start10: Properly fixed start menu showing/hiding along with its original animations on builds 22000.65+. (7e2f768)
* GUI: Fixed a bug where "Remember last used section" doesn't remember the current page after being enabled. (11160c8)
* Symbols: Reworked how symbols are managed so that symbols don't need to be successfully downloaded in succession. (8412bd6)
* Setup: Fixed uninstallation of EP installations that have went through upgrades before the proper Pin to Start fix. (845d2b5, a7c87ce)

## 22621.2361.58

Tested on OS builds 22000.2416, 22621.1, 22621.2134, 22621.2361, 22631.2338, and 23545.1000.

#### Details

##### 1

* Taskbar10: Fixed Windows 10 taskbar not showing up on Windows 11 builds with "Never combine" on the new taskbar. (bc3bbc7)
* Taskbar10: Fixed pen menu crashing `explorer.exe` on 22621.2134+. (1977d78)
* Taskbar11: Fixed a bug that crashed `explorer.exe` when right clicking the new taskbar on Windows 11 builds with "Never combine" on the new taskbar. (6023718)
* File Explorer: EP now tries to avoid crashes related to the new Windows App SDK views. (b426d2c)
* On OS builds 22621+, fixed a bug that crashed `explorer.exe` when required functions in `twinui.pcshell.dll` (for Win+X and Windows 10 Alt+Tab) could not be found using the fallback method. (6023718)

##### 2

* Taskbar11: Fixed a bug that reset the "never combine" setting on OS builds 22621.2361+ (#2207) (085b3dd)
* Taskbar10: Fixed Wi-Fi flyout buttons on OS build 22621 (0706393)
* Start10: Fixed start menu folders, show recently added, and show frequently used apps settings not being applied on OS builds 22621.2134+ (e28940d)

##### 3

* Start10: Pin to Start/Unpin from Start has been properly fixed on Start Menu and Explorer (but not Search yet) of all Windows 11 builds. (15c07a0)
* Start10: Fixed non-UWP apps not appearing on Dev channel builds 23545+. (a4f5bd0)
* File Explorer: Fixed command bar settings not being applied on non-primary Explorer instances on Windows 11. (001e8d8)

##### 4

* Taskbar11: Restored the fix for the bug that reset the "never combine" setting on OS builds 22621.2361+, which was removed in 22621.2361.58.3 by accident. (9f04110)
* Start: "Start menu style" now requires restart so that Pin to Start/Unpin from Start on Explorer works properly. (bdd71ef)
* Taskbar10: Disabled the patch for proper acrylic background on Canary builds (25000+) for now. (4ee742f)

Many thanks to @Amrsatrio for sustained efforts in maintaining and improving ExplorerPatcher.

Thanks to @ARestrepo228 for hints on fixing Pin to Start/Unpin from Start.

## 22621.2283.57

Tested on OS build 22621.2283. Installer requires Internet connectivity.

#### Details

##### 1

* Taskbar10: Fixed Action Center, Control Center, and notification toasts placements on OS builds 22621.2134+ (thanks @Amrsatrio).
* Taskbar10: Fixed a bug that prevented Task View and/or the window switcher (`Alt`+`Tab`) from working on OS builds 22621.2134+ (thanks @Amrsatrio).
* Taskbar10: Fixed a bug that prevented the volume and brightness flyouts from displaying (thanks @Amrsatrio).
* Taskbar10: Fixed a bug that prevented the `Win`+`A` (Action Center), `Win`+`N` (Control Center), and `Win`+`B` (Focus on tray overflow button) shortcuts from working on OS builds 22621.2134+ (thanks @Amrsatrio).
* Taskbar10: Fixed the context menu of the new IME button OS builds 22621.2134+ (thanks @Amrsatrio).
* Taskbar11: Fixed a bug that crashed `explorer.exe` when right clicking the taskbar on OS builds 22621.2134+.
* Quality of life improvements regarding symbol data (thanks @Amrsatrio).

Learn about known issues and track the progress regarding this update [here](https://github.com/valinet/ExplorerPatcher/pull/2097). Special thanks to @Amrsatrio for providing support towards fixing ExplorerPatcher on newer OS builds.

##### 2

* Fixed a bug that crashed `explorer.exe` on OS builds lower than 22621 (Windows 11 22H2). (dfee1ae)

## 22621.1992.56

Tested on OS build 22621.1992. Installer requires Internet connectivity.

#### Details

##### 1

* Windows 10 Start menu: Fixed a bug that prevented the menu from working on OS builds 22621.1413 and newer (46c5041). Please read these important notes regarding the fix [here](https://github.com/valinet/ExplorerPatcher/discussions/1679).

##### 2

* Windows 10 Start menu: Fixed a bug that prevented centering on Windows 10 (275a91f).

##### 3

* Windows 10 taskbar: Correct centering of taskbar items when search box is enabled in Windows 10 (2e43c67).

## 22621.1555.55

Tested on OS build 22621.1555. Installer requires Internet connectivity.

#### Details

##### 1

* Weather: Fixed a bug that prevented the widget from loading when using the Microsoft icon pack. (968d969)

##### 2

* Simple Window Switcher
	* Support for individual list and grouping for UWP apps (implemented grouping and naming enhancements based on using information associated with `AppUserModelID`s)
	* Ability to switch between global and local window lists when the switcher is shown.
	* Maintain position in the list when certain events occur, like closing windows or switching between the global and local window lists.
	* `Del` key closes the currently selected window(s).
	* Fixed a bug that prevented newly spawned windows while the switcher is open from going to the back of the list.
	* Fixed a bug that prevented window lists from building properly when windows were slow to close.
	* Fixed a bug that prevented proper activation of pop-up windows under certain conditions. For example, the switcher is now able to correctly switch to the "Error Checking" window in This PC - right click C: - Properties - Tools - Error checking - Check.

## 22621.1413.54

Tested on OS build 22621.1413.

Please make sure you are connected to the Internet while installing, the application might need to perform one-time downloads for several resources in order to enable full functionality.

#### Details

##### 1

* Windows 10 taskbar: Fixed a bug that made the following functionalities have issues or stop working: Task View button, `Win-Tab`, `Alt-Tab` after pressing `Win-Tab`, flyouts alignment, notification center alignment, `Win` key shortcuts on OS build 22621.1413+ (thanks @CthRio for the heads up). (0ad140c)
* Setup: Fixed a bug that prevented File Explorer from starting automatically after servicing the application if the installer run using different credentials than the logged on user (thanks @Abestanis). (1738b45)
* Weather: Fixed widget icons when using Microsoft icon pack. (2a1aad2)
* Implemented a mechanism to stop repeated crashes. (d7e5b7d)

##### 2

* Weather: Fixed a bug that prevented the widget from displaying correctly. (a5e5287)

##### 3

* Windows 11 Start menu: Better enforcement for disabling the "Recommended" section. (27a8fd9)

##### 4

* Windows 11 Start menu: Fixed a bug that prevented the menu from working when the setting "Disable Recommended section" is used and the display scaling is 125%. (5649a83)

##### 5

* Fixed a bug that could crash File Explorer on older OS builds, like 17763 (LTSC 2019). (6bc2ea5)

## 22621.1344.53

Tested on OS builds 22621.1344, 22000.1574, and 19044.1466.

Please make sure you are connected to the Internet while installing, the application might need to perform one-time downloads for several resources in order to enable full functionality.

#### Details

##### 1

* Windows 10 taskbar: Fixed a bug that crashed `explorer` on OS build 22621.1344. (f9d702e)
* Weather: Fixed a bug that displayed the widget area using a different background color. (cc0af46)
* Weather: Fixed a bug that might throw a script error when certain elements are not ready. (c083327)
* Weather: Fixed a bug that could prevent the widget from properly loading. (a8c7fba)
* ep_extra: Implemented a loadable module for Windows 7's Alt-Tab. (ca8ce13)
* ep_extra: Implemented an `ep_extra`-based loader. (1f4b586)

## 22621.819.52

Tested on OS builds 22621.819 and 22000.1098.

Please make sure you are connected to the Internet while installing, the application might need to perform one-time downloads for several resources in order to enable full functionality.

#### Details

##### 1

* Windows 11 Start menu: Implemented centering on screen when taskbar is not at the bottom. (4212e35)
* Windows 11 taskbar: Option to use the stock taskbar context menu. (451db3c)
* Fixed a bug that could display the Start menu on a wrong monitor or outside the screen when the taskbar was moved to the top of the screen and the previous setting was at the right edge of the screen. (53fad19)

##### 2

* Windows 11 Start menu: Fixed a bug that prevented the disable "Recommended" section feature from working when the scaling level of the screen the Start menu is displayed on is set to 125% (120 DPI). (9f9d43e)

## 22621.608.51

Tested on OS builds 22621.608 and 22000.1042.

Please make sure you are connected to the Internet while installing, the application might need to perform one-time downloads for several resources in order to enable full functionality.

#### Details

##### 1

* Fixed a bug that could prevent the Windows 10 network or battery flyouts from showing on OS build 22000.
* Fixed the Windows 10 network and battery flyouts on OS build 22621.
* Weather: Fixed a bug that displayed the widget contents with incorrect left padding.

##### 2

* sws: Support for changing selection in window list using the mouse wheel (suggestion by andrewz).
* Fix broken "Cascade windows", "Show windows stacked", "Show windows side by side", and "Undo ..." options in taskbar context menu (reported by iamk9008).

##### 3

* sws: Option to have the scroll wheel change the selection when using the switcher:
  * "Never" (default), the same behavior as two versions ago, which means that, when the switcher is active, it does not react to the scroll wheel being used.
  * "When cursor is over the switcher" has the switcher react to the scroll whell and advance/reverse the selection only when the cursor is above the switcher
  * "Always" has the switcher react to the scroll whell and advance/reverse the selection regardless of where the cursor is placed. In this mode, background applications won't receive scroll wheel updates until the switcher is closed, regardless of the "Scroll inactive windows when hovering over them" setting from Windows.
* sws: Fixed a bug that had the scroll wheel move selections in the opposite direction compared to Windows 7 Alt-Tab's behavior. When enabled, the scrolling up selects the previous window in the list, while scrolling down selects the next window in the list. To obtain the previous behavior, which is to scroll up to select the next window, and to scroll down to select the previous window, set `ScrollWheelInvert` to `1` in `HKCU\Software\ExplorerPatcher\sws` (5cef3b1).
* sws: Fixed a bug that could unexpectedly move the switcher to another monitor when your cursor was placed on the other monitor, the option to have the switcher display on the monitor the cursor is placed on is enabled and the switcher finished refreshing its data in the background (https://github.com/valinet/sws/commit/8b68539201102801367ef8f3716b9f1260e2dbe5).
* sws: Fixed a bug that could prevent hotkey associations from being properly cleaned up when you disabled the setting to have a per-application window list (https://github.com/valinet/sws/commit/c5776e5a6a0c5495892a15e16a1def31b225fc51).
* sws: Fixed a bug that could prevent correct reload of settings when entries were directly deleted from the registry (cbc5f19).

##### 4

* Windows 11 taskbar: Fixed a bug that could crash `explorer.exe` when right clicking certain system tray icons on 22621-based builds. Thanks for the reports about this issue. (a6a88b1)

##### 5

* Windows 11 Start menu: Fixed a bug that prevented the menu from taking into account the "Layout" setting from Windows Settings - Personalization - Taskbar on 22621-based builds. (2572a80)

##### 6

* Fixed a bug that could cause the host process of ExplorerPatcher to crash under certain circumstances. (d7a0385)

## 22622.450.50

Tested on OS build 22622.450.

Please make sure you are connected to the Internet while installing, the application might need to perform one-time downloads for several resources in order to enable full functionality.

#### Details

##### 1

* Support for OS builds 22621+. Read more [here](https://github.com/valinet/ExplorerPatcher/issues/1082#issuecomment-1206690333).
* Added an option to shrink address bar height in File Explorer windows (thanks @krlvm).

##### 2

* Support for disabling the modern search bar in 32-bit applications as well (thanks @krlvm).
* Fixed a bug that could prevent deleting registry keys when the application was supposed to (for example, when uninstalling or toggling certain settings).

##### 3

* Fixed incorrect check for running dwm instances in `ep_dwm`
* Fixed a use-after-free bug in `ep_dwm` (thanks @ibhk)

## 22000.795.48

Tested on OS build 22000.795.

Please make sure you are connected to the Internet while installing, the application might need to perform one-time downloads for several resources in order to enable full functionality.

#### Details

##### 1

* Weather: Fixed a bug that could hang the widget and lead to an infinite loop with the program using an entire CPU core when the computer resumed from sleep or hibernation.
* Weather: Fixed a bug that had the widget display the error page when the computer lost network connectivity; instead, now the widget continues to display the cached data from the previous refresh (if any).

##### 2

* Weather: Fixed a bug that could hang explorer and the weather widget host process under certain circumstances, for example, when explorer restarted.

## 22000.778.47

Tested on OS build 22000.778.

Please make sure you are connected to the Internet while installing, the application might need to perform one-time downloads for several resources in order to enable full functionality.

#### Details

##### 1

* Built-in support for OS build 22000.778.
* Fixed a bug that had the Start button context menu / Win-X menu / power user menu fail to display and potentially lock the shell on OS builds 22000.778+ and 22621+

##### 2

* Fixed a system high DPI-related bug that caused wide Windows 10 taskbar buttons and incorrect (desktop) icon spacing

## 22000.708.46

Tested on OS build 22000.708.

Please make sure you are connected to the Internet while installing, the application might need to perform one-time downloads for several resources in order to enable full functionality.

#### Details

##### 1

* Built-in support for OS build 22000.708.
* Added configuration options for the new Windows Spotlight desktop background feature, including:
  * Hide the "Learn about this picture" icon
  * Choose which items from the Windows spotlight icon context menu to have replicated in the desktop context menu (legacy context menu only)
  * Set a schedule for "Switch to next picture"
  * Manipulate the feature from the Properties UI, bypassing the desktop icon
* The Properties UI hides sections that are not applicable to your current settings; for example, the "Weather" tab is not displayed if you've selected the Windows 11 taskbar, as none of the options in there apply when in this mode.

##### 2

* sws: Fixed a bug that created unnecessary paint events when a window was flashing and the switcher is not shown

##### 3

* Added option to hide the "Show desktop" button, but still retain its functionality, when using the Windows 10 taskbar
* Fixed a bug in Windows 10 where the Start menu was displayed centered by default

##### 4

* Weather: Show "Reload" link when data fails to load (thanks Varun A. for the suggestion)
* sws: Draw placeholder thumbnail when a proper thumbnail cannot be obtained (for example, due to a window having an invalid width or height)
* sws: Fixed a bug that could prevent the switcher from identifying when the desktop is in the foreground
* sws: Fixed a regression that could prevent the switcher from properly detecting foreground window changes
* sws: Fixed a bug that made very small windows have a rectangle area too small for properly working with in the switcher

##### 5

* Fixed a bug that could prevent Control Panel link redirection from working correctly
* Weather: Fixed a bug that prevented the widget from working when WebView2 Runtime >= 102.0.1245.33

##### 6

* Fix a bug in the Properties window that had the it fail to display some sections under default settings
  
## 22000.675.45

Tested on OS build 22000.675.

Please make sure you are connected to the Internet while installing, the application might need to perform one-time downloads for several resources in order to enable full functionality.

#### Details

##### 1

* sws: Fixed a bug that displayed a wrong window to switch to when a background application was denied the request to have the foreground window by the OS (#1084)

##### 2

* libvalinet: Fixed a memory leak in `toast.h`
* sws: Fixed a bug that caused the switcher to display non-responsive (hung) immersive (UWP) windows twice in the list

##### 3

* sws: Fixed a bug that created unnecessary paint events when a window was flashing and the switcher is not shown

## 22000.613.44

Tested on OS build 22000.613.

Please make sure you are connected to the Internet while installing, the application might need to perform one-time downloads for several resources in order to enable full functionality.

#### Details

##### 1

* Option to disable Win+F (Feedback Hub) hotkey
* Built-in support for OS build 22000.613

##### 2

* Weather: Fixed a bug that had "COM Surrogate" display as a running app in Task Manager after the widget flyout was opened the first time
* Weather: Fixed a bug that could hang or lock the shutdown/restart/sign out process when using the weather widget

## 22000.556.43

Tested on OS build 22000.556.

Please make sure you are connected to the Internet while installing, the application might need to perform one-time downloads for several resources in order to enable full functionality.

#### Details

##### 4

* Option to enable legacy file transfer dialog

##### 3

* Option to enable classic drive groupings in This PC (thanks @lordmilko)
* Choice of Windows 11 Command Bar, Windows 10 Ribbon or Windows 7 Command Bar for File Explorer windows

##### 2

* Fixed Windows 10 taskbar showing fully transparent instead of the acrylic effect on newer OS builds (22581+)

##### 1

* Option to disable window Snap quadrants in Windows 11 (thanks @lordmilko)

## 22000.556.42

Tested on OS build 22000.556.

Please make sure you are connected to the Internet while installing, the application might need to perform one-time downloads for several resources in order to enable full functionality.

#### Highlights

* Implemented Weather widget for the classic taskbar, similar to what is available in the more recent updates to Windows 10. Read more about it [here](https://github.com/valinet/ExplorerPatcher/wiki/Weather).
* Support for the Windows 10 Start menu in Windows 11. Read more about this [here](https://github.com/valinet/ExplorerPatcher/discussions/920).
* Option to center Windows 10 Start menu and Windows 10 taskbar.
* Support for Windows 10. Read more about this [here](https://github.com/valinet/ExplorerPatcher/discussions/898).

#### Details

##### 1

* The weather widget recomputes its area automatically, by default, in order to fit its contents, instead of remaining at a fixed size; there is also an option to choose between the two behavior
* Possibility to disable the icon in the weather widget
* Fixed a bug that prevented the weather widget flyout from displaying correctly when the taskbar was using small icons (#741)
* Fixed inconsistencies when displaying the weather widget and the system themes are disabled (aka the classic theme is used)
* Screen readers now read the weather data when requested for the weather widget
* Changing the Start button style or weather widget layout does not toggle taskbar auto-hide now; instead, the settings take effect immediately

##### 2

* The weather widget defaults to showing in the preferred language set in Windows, instead of English (#734)
* Fixed a bug that could corrupt registry entries of type REG_SZ set via the Properties UI (#734)
* Fixed a bug that reset the setting when pressing "Cancel" in an input box in the Properties UI (#734)

##### 3

* The weather widget shows an error screen when an error happens (like, using an incorrect location, or the network not working etc)
* The weather widget adjusts its size vertically to accommodate the entire contents (#734)

##### 4

* The weather widget supports dark mode (thanks @krlvm) (#755)

##### 5

* Fixed a bug that prevented correct registration of the weather flyout on certain systems (26b6646)
* Fixed a bug that made the weather flyout open with noticeable delay under certain circumstances
* Fixed a bug that prevented correct operation on builds without built-in symbols (#783)

##### 6

* Fixed several race conditions that could lead to incorrect operation of the weather widget (for example, `explorer.exe` crashing when disabling or enabling the widget)

##### 7

* Implemented 2 features that help in replacing the functionality of the quick launch toolbar with pinned taskbar items. Read more about it [here](https://github.com/valinet/ExplorerPatcher/discussions/819)

##### 8

* Implemented option to have the Start menu open on a specific monitor (#821)
* The weather widget supports setting window corner preference (rounded/not rounded)

##### 10

* Option to clear weather widget local data

##### 11

* Installer sets a Start menu shortcut for the "Properties" window

##### 14

* The weather widget positions and sizes itself with respect to the text size accessibility setting as well ([#734](https://github.com/valinet/ExplorerPatcher/discussions/734#discussioncomment-2190218))

##### 15

* Support for high contrast themes in the "Properties" window and in the weather widget (#885)
* Fixed a bug that could lead to a crash when `explorer.exe` starts (#879)
* Fixed a bug that could prevent the weather widget from launching under certain conditions

##### 16

* Initial support for Windows 10
* Enabling the weather widget will automatically download and install the Microsoft WebView2 Runtime on computers where it is not installed

##### 17

* Fixed a bug in the Weather widget that could display an erroneous Google search pop-up (thanks @plofhaan) ([#734](https://github.com/valinet/ExplorerPatcher/discussions/734#discussioncomment-2216475))

##### 18

* Fixed a bug that resulted in an access violation on log off when EP runs alongside 7+TT (#894)

##### 19

* Option to enable dev tools for weather widget debugging (#934)
* Fixed a bug that prevented the "Skin menus" setting from working in the `Win`+`X` menu on Windows 10

##### 20

* Fixed a bug that would display an information banner that obscured the weather widget in some occasions when displaying the widget in German (#934) (thanks @frederic2de)

##### 21

* Fixed program windows in older 22000-based OS builds

##### 22

* Added option to enable rounded corners on the Windows 10 Start menu (#937)
* Added option to disable the "Recommended" section on the Windows 11 Start menu
* Fixed a bug that prevented correct displaying of the weather widget contents when using a right-to-left language (#954)

##### 23

* Support for full screen Windows 10 Start menu

##### 24

* Support for "Show more tiles" option in the Windows 10 Start menu (#933)
* Fixed a bug that prevented extraction of all files when running `ep_setup.exe /extract`

##### 25

* Implemented floating/docked rounded corners in the Windows 10 Start menu

##### 26

* Implemented centered Windows 10 taskbar
* Option to hide the app list in the Windows 10 Start menu
* Fixed a bug that presented the weather widget with a wrong height when using the taskbar vertically

##### 27

* Implemented centered Windows 10 Start menu
* Fixed a bug that prevented the taskbar from displaying UWP icons when using the Windows 10 Start menu on newer Windows builds (#973)

##### 28

* Fixed a bug in the Windows 10 Start menu that prevented the menu from displaying when not using rounded corners
* Fixed a bug in the Windows 10 Start menu that prevented the menu from updating its position when the settings changed
* Fixed a bug in the Start menu that prevented the app from receiving settings change notifications when some registry keys were not available on the system
* Fixed a crash at startup in `explorer.exe` (in module `sndvolsso.dll`) on OS build 22567+

##### 29

* Fixed a bug that prevented the weather widget from working on OS server SKUs
* Enabled centered Start menu and taskbar in Windows 10
* Prompt before updating when running on the built-in Administrator account and the `FilterAdministratorToken` policy ("User Account Control: Use Admin Approval Mode for the built-in Administrator account") is disabled or not configured
* Reworked CHANGELOG format

##### .30

* Support for daytime/nighttime icons in the weather widget
* Improved contrast between the weather icons and the taskbar using light theme
* Implemented Mica effect for File Explorer windows (thanks @MishaTY) and option to hide the icon and title of File Explorer windows
* Fixed a bug that made the search, Cortana and task view buttons display on the left even though the taskbar was set to center with Start menu (#999)
* Disabling the "Recommended" section in the Windows 11 Start menu now works in newer OS builds as well (#995)

##### .31

* The Microsoft icon pack for the weather widget applies to the widget contents as well, in addition to the taskbar icon
* Fixed a bug that slightly moved the taskbar buttons to the right when dragging to rearrange one of them over the first or last item when using the centered Windows 10 taskbar (#1009)
* Fixed a bug that crashed the "Properties" window when accessing the "System tray" section in Windows 10 (#1013)
* Fixed a bug that prevented the Mica effect from working on File Explorer windows when the "Launch folder windows in a separate process" setting was used (#1021)
* Fixed a bug that made certain systems, under certain circumstances, to become stuck in "tablet mode"; symptoms included more spacing between taskbar icons, compact view permanently disabled in File Explorer, and/or item checkboxes permanently enabled in File Explorer (#1022). After installing this update, if you are still stuck with the tablet UI, open the Registry Editor, go to `HKEY_CURRENT_USER\SOFTWARE\Microsoft\TabletTip\ConvertibleSlateModeChanged` and delete the `ConvertibleSlateModeChanged` value which most likely is stuck to `1`.

##### .32

* Built-in support for 22000.556
* Weather widget can now show on the left/top side of the taskbar
* Weather widget can display condition on 2 lines
* Reliability improvements for the centered Windows 10 taskbar which now also works along with other taskbar toolbars
* Fixed a bug that had the Windows 10 Start menu default to being left-aligned on Windows 10
* Fixed a bug that prevented the "Centered, with Start button" modes of the Windows 10 taskbar from working correctly when the taskbar was vertically aligned

##### .33

* Weather: Implemented manual zoom levels (#1033)
* Weather: Fixed a bug that had the widget display, at startup, a day/night icon relating to time at the last hour change, instead of the actual current time
* Weather: Fixed a bug that made the widget display a line at the top for some places
* Fixed a bug that could make the centered taskbar not layout correctly when showing/hiding either of the search, Cortana or task view buttons the first time after the application started
* Fixed a bug that prevented the centered taskbar from working when animations are turned off system-wide (for example, in usual remote sessions)
* Fixed a bug that prevented the taskbar from displaying correctly when the weather widget is set to display at left/top (#1041)

##### .34

* Weather: Fixed a bug that prevented resizing other taskbar toolbars (#1043)
* Weather: Fixed bugs regarding left/top positioning option (#1041)

##### .35

* Option to allow version downgrades when updating the application after switching the servicing channels (#1051)
* ExplorerPatcher no longer sets the `MinWidth` registry entry automatically - this was used to mitigate an issue with `explorer.exe` where taskbar button labels were becoming too large. If you are affected by having this registry entry set, open the Registry Editor, go to `HKEY_CURRENT_USER\Control Panel\Desktop\WindowMetrics` and remove the `MinWidth` entry (which is probably set to `38`). The same procedure can be used in order to have this option set up in the registry. For more information, see #664.
* Setup will disable the `UndockingDisabled` registry entry at `HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Shell\Update\Packages` when servicing the application. Using `UndockingDisabled` with ExplorerPatcher is not necessary and can actually cause issues (for example, see #704).

##### .36

* Weather: Display time of last update in the Properties window

##### .37

* Fixed Windows 10 taskbar button thumbnails on newer OS builds (22572+)
* Fixed Windows 10 taskbar showing fully transparent instead of the acrylic effect on newer OS builds (22572+)

##### .38

* Fixed a bug that made the disable window rounded corners feature not work after signing out and then back into a user account

##### .39

* Weather: Fixed a bug that prevented the widget from starting up on new installations due to the folder `%APPDATA%\ExplorerPatcher` not existing

## 22000.469.41

Tested on OS build 22000.434.

#### New features

* Built-in support for OS builds 22000.434, 22000.438, 22000.466, and 22000.469
* Ability to choose a Windows 10 or Windows 11 Start button style for the Windows 10 taskbar (#436, thanks @krlvm)
* Support for screen readers in the Properties window (#627) (.1)
* Option to disable `Office` hotkeys (`Ctrl`+`Alt`+`Shift`+`Windows` key combinations) (#661) (.4)
* Simple Window Switcher can switch applications instead of windows (#665) (.5, .6)
* Option to disable rounded corners for windows (.7)
* Ability to hide the "Properties" item from the taskbar context menu (.9)
* Import/export functionality for program settings (.11)

#### Feature enhancements

* The option to toggle taskbar auto hide when double clicking the taskbar now works with the Windows 11 taskbar as well
* Performing a memory leak dump now displays GDI, peak GDI, USER and peak USER object counts as well
* The language switcher list displays only the options that work (previously, it showed some cryptic internal implementations, like "LOGONUI", "UAC", "SETTINGPAGE" and "OOBE"). Thus, the list of language switchers offers the following choices:
  * Windows 11 (default)
  * Windows 10 (with link to "Language Preferences")
  * Windows 10
* Simple Window Switcher now highlights windows that require user attention (windows that have their taskbar button flash and colored in orange) (.8)
* Reliability improvements for the option that maps the `Win`+`C` shortcut to open the clock flyout instead of Microsoft Teams (eliminated dependency on symbol data) (.10)
* When an update is available, the notification displays the version of the update (.12)
* The updater correctly detects when the current version is a pre-release but the user has switched the update channel to stable and does not suggest the older stable version as an update anymore (multiple reports, #540, #710) (.12)
* Import/export settings suggests a file name automatically (.15)


#### Fixes

* Fixed a bug that prevented the Properties UI's system menu from displaying and working correctly
* Fixed a bug that displayed a wrong timestamp (29/08/2021) instead of the current date and time on the notifications generated by ExplorerPatcher
* Fixed a wrong function prototype (5b4bd07#r62018175, thanks @Simplestas)
* Protected some state variables from changing internally if modified in the registry until `explorer` is restarted
* Fixed a bug that could unexpectedly prevent the [Win]+[Alt]+[D] shortcut from working properly
* Windows 10 language switcher displays correctly when the taskbar is placed in some location other than the bottom of the screen (#629) (.2)
* Available symbols download properly on Insider builds (tested on 22526.1000) (.3)
* Mitigated an `explorer.exe` bug where Windows 10 taskbar buttons were becoming too large under certain circumstances when the setting to show labels/never combine is used and the screen resolution/DPI changes (#664) (.6)
* Performance improvements and bug fixes for Simple Window Switcher (.8)

## 22000.376.40

Tested on OS build 22000.376.

#### Highlights

* Built-in support for OS build 22000.376 (.12)
* Hotfix: Windows 10 taskbar "always combine"/"show labels" setting is properly preserved when upgrading from an older release (multiple reports, #612, #614) (.21)
* Primary taskbar remembers position when moved to a secondary monitor (multiple issues, like #504)
* Ability to set Control Center as network icon action (merged #492)
* Added possibility to use the original Windows 10 (Alt-Tab) window switcher; thus, the available options are now:
  * Windows 11 switcher - full screeen, slow, tiny selection outline, slow opening times
  * Windows 10 switcher - pretty good but lacks customization options
  * Windows NT switcher - the classic simple icon-based interface hosted by `csrss`
  * Simple Window Switcher - my own take on implementing this kind of functionality
* Registry access in the "Properties" GUI is now virtualized; that means, the same lightweight infrastructure is maintained but more complex behaviors can be offered via the improved backend; as such, this version introduces the following new configuration options:
  * Primary and secondary taskbar placement
  * Automatically hide the taskbar
* Proper activation of the "Properties" window when another instance is running and minimized
* Symbols parsing success notification displays for longer
* Debug builds are clearly indicated in the "About" page of "Properties"
* Fixed solution to properly produce a debug setup program
* Possibility to uninstall by renaming `ep_setup.exe` to `ep_uninstall.exe` and running that (.4)
* Fixed a bug that crashed the "Properties" GUI when toggling certain settings (#527) (.6)
* The "Properties" window is restarted unelevated if it was open when application servicing was performed (#528) (.7, .13)
* Reliability improvements for File Explorer restarts (#529) (.7)
* When changing the main taskbar position and restarting File Explorer, the new position is now correctly saved and applied when File Explorer restarts (#523) (.7)
* Mitigation for the issue described in #416 (.7)
* Fixed a bug that prevented the Windows 10 window switcher from displaying when it was enabled, instead falling back to the Windows NT window switcher (#548) (.8)
* Fixed the "Show People in the taskbar" option and made it not require a restart to apply (#554) (.10)
* Ability to choose look of Snap Assist (window list when snapping a window): Windows 11 or Windows 10 style (.11)
* Fixed a bug that prevented the correct set up of "DisplayVersion" registry entry in the uninstall information registry key (.11)
* Secondary taskbars' context menu is displayed similarly to the primary taskbar's context menu for Windows 10 style (.12)
* Safeguards to prevent malicious executions on update mechanism hijacks for systems where User Account Control is disabled (#567) (.13)
* Option to prevent certain Control Panel links from being redirected to the Settings app (.14), including in build 22523 (.15)
* Settings are now stored in `HKEY_CURRENT_USER\Software\ExplorerPatcher` so that Windows does not reset them anymore across major OS updates (#576) (.16)
* Improved Properties UI layout by reducing wasted space and eliminating redundant elements (#590) (.17)
* Support for the `Win`+`Alt`+`D` shortcut to activate the clock flyout, as in Windows 10 (#591) (.17)
* Fixes for Windows 11 taskbar:
  * As shipped by Microsoft, a taskbar displayed on a secondary monitor does not react when the mouse is over it and auto hide is on; fixed this (#589) (.17)
  * As shipped by Microsoft, under certain circumstances, the main taskbar does not show its system tray when `explorer` starts up and auto hide is on; fixed this (.17)
  * As shipped by Microsoft, a taskbar displayed on a secondary monitor might display a wrong contextual menu when auto hide is on; fixed this (.17)
  * The clock flyouts now display correctly when using this taskbar
  * Fixed a bug that displayed wrong window previews when the combine taskbar buttons option was set to never combine (#564) (.17)
  * Possibility to set position on screen (top/bottom) from the Properties UI
* Restoring default settings only asks for elevation if required (for the moment, only if you have registered the application as shell extension) (.18)
* Fixed the context menu not working (and a potential associated crash) of the new Microsoft IME (#598, #588) (.19) (huge thanks to @Simplestas)
* GUI: Lock `ExplorerFrame` into memory (.20)


#### Simple Window Switcher

* Dramatically improved performance, refactored application; switched to building the window lists faster, on demand, so that the proper windows are always displayed (as far as I remember, the latest `IsAltTabWindow` is based on a function called `IsTaskedWindow` ripped straight from AltTab.dll from Windows 7 6.1.7600.16385)
* Proper history of window activations is maintained internally
* Implemented support for layered windows, thus making transparency possible when using the default theme (Acrylic and Mica brushes are still available, but those have the disadvantage that the system can disable them in certain scenarios, like saving energy when working on battery power)
* Improved reliability of startup delay and window dismiss when quickly Alt-Tabbing
* Window icons are retrieved async now
* Better icon drawing using GDI+ flat API
* Added some more debug messages
* Fixed some rendering problems when themes are disabled
* Fixed regression of [#161](https://github.com/valinet/ExplorerPatcher/issues/161#issuecomment-986234002) (.1)
* Possibility to disable per-application window lists (`Alt`+`) ([#283](https://github.com/valinet/ExplorerPatcher/issues/283#issuecomment-986261712)) (.2)
* Fixed bug that prevented proper loading of default settings (.3)
* Implemented a mitigation for #516: gestures for switching apps on Windows Precision Touchpad devices trigger the Windows 10 switcher instead of the Windows 11 switcher, which is much closer to how Simple Window Switcher looks and behaves; ideally, a full solution for this should be provided in the future, in the form of support for activation and navigation using Windows Precision Touchpad gestures in the Simple Window Switcher (.5)
* Fixed an issue that could hung the application and made window switchers unavailable (#525) (many thanks to @jdp1024) (.7)
* Possibility to configure window padding (.7)
* Support for closing window with middle button ([#110](https://github.com/valinet/ExplorerPatcher/discussions/110#discussioncomment-1793318)) (.9)
* Mitigated an issue that may have prevented Explorer from launching correctly when Simple Window Switcher is set as window switcher (.9)
* Fixed a crash that could make Explorer restart repeatedly at startup or even hang indefinitely (#525) (.15)

## 22000.348.39

Tested on build 22000.348.

#### New features

* Built-in support for build 22000.348.
* Implemented option to toggle taskbar auto-hide when double clicking the main taskbar (#389)
* Running `ep_setup.exe` again while EP is already installed will now update the program to the latest version. To uninstall, as the previous behavior did, run `ep_setup.exe /uninstall`
* Implemented absolute height and width parameters for the Windows 10 switcher. These are especially useful for ultra wide monitors, in a scenario similar to the one described in [this post](https://github.com/valinet/ExplorerPatcher/discussions/110#discussioncomment-1673007) - to configure, set `MaxWidthAbs` and/or `MaxHeightAbs` DWORD values in `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher\sws` (#110)
* Provides a simple mechanism for chainloading a custom library when the shell interface is created, from which you can execute your custom code (subject to change, see [this](https://github.com/valinet/ExplorerPatcher/discussions/408#discussioncomment-1674348) for more details) (#408)

#### Feature enhancements

* Option to receive pre-release versions, if available, when checking for updates
* Improved behavior regarding symbol data information; please refer to https://github.com/valinet/ExplorerPatcher/wiki/Symbols for more information (.1)

#### Fixes

* Fixed mismatches between defaults from EP and Windows' defaults
* Application starts with limited functionality on builds lacking hardcoded symbol information; symbol downloading is disabled for now, by default, but can be enabled in the "Advanced" settings section of "Properties"
* Improvements to how hung windows are treated by the Windows 10 window switcher; fixed an issue that severely delayed the time it took the window switcher to display when a window hung on the screen (#449)
* Clicking "Close" in the Windows 10 window switcher is now more tolerant to small mouse movements (#110) (.1)
* The existing "Properties" window is properly displayed if opening it when another instance is already running and is minimized (.2)

## 22000.318.38

Tested on build 22000.318.

#### New features

* Functional Windows 10 network flyout
* Functional Windows 10 battery flyout
* Implemented support for Windows 7 battery flyout (#274)
* Implemented `/extract` switch which unpacks the files from `ep_setup.exe` to disk (#396) (.1):
  * `ep_setup /extract` - extracts `ExplorerPatcher.IA-32.dll` and `ExplorerPatcher.amd64.dll` to current directory
  * `ep_setup /extract test` - extracts to `test` folder in current directory
  * `ep_setup /extract "C:\test with space"` - extracts to `C:\test with space` directory
* Taskbar toolbar layouts are preserved when switching between Windows 10 and Windows 11 taskbars and in general (these will be reset when installing this update but should be subsequently remembered) (#395) (.2)
* Implemented option to toggle taskbar auto-hide when double clicking the main taskbar (#389) (.3)
* Running `ep_setup.exe` again while EP is already installed will now update the program to the latest version. To uninstall, as the previous behavior did, run `ep_setup.exe /uninstall` (.4)
* Implemented absolute height and width parameters for the Windows 10 switcher. These are especially useful for ultra wide monitors, in a scenario similar to the one described in [this post](https://github.com/valinet/ExplorerPatcher/discussions/110#discussioncomment-1673007) - to configure, set `MaxWidthAbs` and/or `MaxHeightAbs` DWORD values in `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher\sws` (#110) (.5)
* Provides a simple mechanism for chainloading a custom library when the shell interface is created, from which you can execute your custom code (subject to change, see [this](https://github.com/valinet/ExplorerPatcher/discussions/408#discussioncomment-1674348) for more details) (#408) (.6)


#### Feature enhancements

* Improved reliability when invoking Control Center (`Win`+`A`) when the taskbar icon is disabled (the icon should now not reappear anymore sometimes) (#242)
* Small reorganization of some options in "Properties"
* Option to receive pre-release versions, if available, when checking for updates (.9)

#### Fixes

* Windows 10 network and battery flyout should now always launch when the tray icon is clicked (#410) (.1)
* Fixed mismatches between defaults from EP and Windows' defaults (.3)
* Application starts with limited functionality on builds lacking hardcoded symbol information; symbol downloading is disabled for now, by default, but can be enabled in the "Advanced" settings section of "Properties" (.7)
* Improvements to how hung windows are treated by the Windows 10 window switcher; fixed an issue that severely delayed the time it took the window switcher to display when a window hung on the screen (#449) (.8)

## 22000.318.37

Tested on build 22000.318 and 22000.346 (currently in Windows Insider beta and release preview channels).

#### New features

* The configuration interface is now accessed by right clicking on the taskbar and choosing "Properties" (previously, it was available in the `Win`+`X` menu). This behavior works when either the Windows 10 or Windows 11 taskbar is enabled. As well, you can launch the "Properties" window directly by pressing `Win`+`R` and typing `rundll32 C:\Windows\dxgi.dll,ZZGUI`, followed by `Enter`.
* Implemented a setup program:
  * To install, simply run `ep_setup.exe`. File Explorer will restart and the program will be enabled right away.
  * To uninstall, there are 2 options:
    * Run `ep_setup.exe` again.
    * Via "Programs and Features" in Control Panel, or "Apps and features" in the Settings app.
  * Learn more about the setup program [here](https://github.com/valinet/ExplorerPatcher/wiki/Installer-How-To)
* Implemented automatic updates; there are 3 settings to choose from when File Explorer starts:
  * Notify about available updates (default) - the program will check for updates and display a notification if a new build is available; you can go to "Properties" and install the update from there
  * Prompt to install available updates - the program will check for updates, download them if any, and prompt you to install them automatically
  * Do not check for updates - the program never checks for updates in the background
  * Of course, you can manually check for updates at any point using "Properties" - "Updates" - "Check for updates". To install the update you were notified about, go to "Properties" - "Updates" - "Install latest version".
  * When installing an update, you will be prompted using UAC to allow elevation - always check that the update originates from matches what you expect; official updates are currently served via https://github.com/valinet/ExplorerPatcher.
  * Learn more about how to configure updates on your system, including how to set a custom endpoint [here](https://github.com/valinet/ExplorerPatcher/wiki/Configure-updates)
* Implemented a proper right click menu for the Windows 11 taskbar - it displays the most common options, similar to previous Windows releases and the Windows 10 taskbar, including frequently accessed items like "Taskbar settings", "Task Manager" and "Show the desktop"
* System tray icons are now left intact when switching between the Windows 10 and Windows 11 taskbars, or when switching builds, reinstalling the application etc. Basically, now, once you set a certain layout for the system tray with the Windows 10 taskbar, it will always be remembered, instead of the annoying behavior where Windows was discarding your choices in order to accommodate the Windows 11 taskbar

#### Feature enhancements

* Hardcoded symbols are now based on file hashes, not build numbers
* Better organization for the settings in "Properties"
* Update toast notifications are displayed only as long as they are required. Subsequent notifications no longer have to wait for the previous ones to timeout, but rather they replace the previous ones (#346) (.2)

#### Fixes

* Mitigated an issue that prevented the Windows 11 taskbar from displaying properly under certain circumstances
* Fixed an issue that would crash the Windows 11 taskbar when it was enabled after positioning the Windows 10 taskbar on either side of the screen (left/right)
* Fixed a bug in "Windows 10 Window switcher" that may have lead to `explorer.exe` crashing when more than 20 windows are opened on the screen (probably the cause for a lot of crashes)
* Fixed numerous issues when injecting processes, including as shell extension; reliability improvements
* Adjusted the padding of the system tray hidden icons indicator so that it is now properly centered vertically when using the classic theme mitigations
* Fixed a memory leak in "Settings Manager"
* Removed verbose output from "Settings Manager"
* Corrected import from `dxgi.dll`
* Fixed typo in configuration UI (#346) (.1)
* Fixed typos and spelling in error message (#346) (.2)
* Fixed bug that prevented "Properties" from working when invoked from Quick Launch or other toolbars (#349) (.2)
* As you may have noticed, releases do not contain unpacked files anymore. Thus, for people looking for a quick way to get the unpacked files, the release notes now include a link to the artifacts generated during during the build process. The artifacts include the usual DLLs (including `dxgi.dll`), plus symbol files and all the helper executables generated during the build. (#351) (.2)
* Setup program version is synchronized with the version of the application (.2)
* Fixed a mismatch between the default value for the setting "Add shortcut to program settings in Win+X menu" displayed in the UI and actually used in the software (#352) (.2)
* Fixed an issue that prevented "Restore default settings" in the "Properties" UI from working (#374) (.3)
* Improved some wording in the Properties UI (#377) (.4)
* Added option to show separators between toolbars in the taskbar (#379) (.4)
* "Properties" is restarted when doing an install/update and closed when uninstalling the application (.5)
* "Properties" can open the last used section when starting (.5)

## 22000.318.36

Tested on build 22000.318.

#### Fixes

* Fixes an issue that prevented Explorer from starting up when Windows 10 taskbar was disabled and Windows 10 window switcher was enabled (#313) (.1)
* Lots of bug and issue fixes for shell extension failing to work under certain circumstances (#259)

## 22000.318.35

Tested on build 22000.318.

#### Feature enhancements

* Start menu position can now be changed without needing to restart File Explorer

#### Fixes

* Improved reliability of Start menu positioning when the monitor topology changes and at startup
* Start menu injection returns error codes from the remote process in the debug console
* Other Start menu quality of life improvements

## 22000.318.34

Tested on build 22000.318.

#### New features

* Added option to enable legacy list view ("SysListView32") in File Explorer (credit @toiletflusher, @anixx from WinClassic) (.1)

#### Feature enhancements

* Built-in support for build 22000.318

## 22000.282.33

Tested on build 22000.282.

#### New features

* Ability to choose language switcher flyout style (option available in the "Properties" - "System tray" section)

#### Fixes

* The key above the `Tab` key is now correctly identified for all keyboard layouts, so the per-application Windows 10 window switcher mode works correctly (#283)

## 22000.282.32

Tested on build 22000.282.

#### New features
* Windows 10 window switcher features new configuration options:
  * Always show switcher on primary monitor
  * Show windows only from current monitor
  * Theme selector: Mica, Acrylic and None
  * Corner preference: Rounded, Rounded small, Not rounded
  * More options for row height
  * [Alt]+[\`] shows the switcher only for windows of the foreground application
  * `[Enter]` switches to selected window (same as `[Space]`) (#240) (.1)
  * Navigation using arrow keys (#240) (.1)
* Ability to choose behavior for Cortana button: hidden, shown and opens Cortana, shown and opens Widgets (#255) (.3)
* Builds are now automatic, generated as soon as new content is pushed to the repository and compiled on GitHub's infrastructure (.6)

#### Feature enhancements
* Clock context menu options "Adjust date/time" and "Customize notification icons" open in Control Panel instead of Settings

#### Fixes
* Taskbar context menu not displaying properly when using classic theme mitigations should now be fixed
* Reliability improvements, correct injection, avoid double patching
* All windows should now be properly detected and included in the Windows 10 window switcher
* Performance enhancements for Windows 10 window switcher: layouts are now precomputed when a window change occurs, so it should be very fast to open and consistent, no matter the current load; also, fast switching does not trigger the window, as it should
* "Show Cortana button" taskbar menu entry now works again (#252) (.2)
* Fixes a bug that prevented correct opening of some applications (like `powershell`) when EP was registered as shell extension (#256) (.4) - PLEASE NOTE THAT RUNNING AS SHELL EXTENSION IS STILL EXPERIMENTAL, UNSUPPORTED, AND ONLY RECOMMENDED FOR SPECIFIC USE CASES THAT YOU SHOULD KNOW ABOUT ALREADY; otherwise, just dropping the DLL in `C:\Windows` is enough
* Windows 10 window switcher switcher now correctly displays UWP apps immediately after launch (#266) (.5)
* Selection and highlight rectangle are now correctly drawn when using light theme on Windows 10 window switcher (.5)
* Keyboard selection (left, right, up, down, space, Return) also works when window switcher is shown by holding down the `ALT` key (#263) (.7)

## 22000.282.31

Tested on build: 22000.282.

#### New features

* Cortana button now opens the Widgets panel
* Ability to choose what happens when clicking the Network icon in the system tray
* Possibility to use the legacy clock flyout
* Possibility to use the legacy volume flyout
* Fixes to fully support the classic theme, with a functional taskbar, system tray, Explorer windows, working context menus; read more about this feature [here](https://github.com/valinet/ExplorerPatcher/discussions/101)
* Choose type of flyout for clock (.1)
* Compatibility with build 22000.282 (.2)

#### Feature enhancements

* Reorganized settings in the GUI
* Added option not to have an accelerator for the `Properties` menu entry in `Win`+`X` (#162)
* The "Adjust date/time" and "Customize notification icons" links in the clock context menu now open the more versatile Control Panel applets (.4)
* The console can now be disabled and then dismissed without causing the Explorer process to restart (.4)
* Implemented a new exported function which restarts File Explorer cleanly, reloading your folder windows: `rundll32 C:\Windows\dxgi.dll,ZZRestartExplorer` (.4)

#### Fixes

* Fixed an issue where the Windows 10 window switcher failed to display some windows (#161)
* Fixed an issue that prevented Start from opening again until Explorer was restarted after opening File Explorer via the Start menu Explorer icon (#145)
* Fixed patching in libvalinet
* Fixed GUI launch path; GUI now launches in an external process, survives Explorer restarts
* Addresses an issue that prevented correct operation under certain circumstances and that could lead to a rare bug where Explorer would crash after a Control Panel window was opened (also related to Control Panel windows not always respecting preferences like "disable navigation bar") (.3)
* Fixed a bug that caused the cursor to move when invoking `Win`+`X` (.4)
* Fixed a bug that caused incorrect positioning of `Win`+`X` when the main taskbar was present on a monitor different than the primary one (.4)
* The Start menu now automatically reloads in the background only when its settings change, instead of when any settings change (.4)
* Improved the reliability of the "Restart File Explorer" link in the Properties GUI (.4)
* Improved the detection of scenarios when the patcher should inject and apply the full set of patches to File Explorer (.4)
* Other bug fixes (.4)

#### Experimental

**PLEASE NOTE THAT RUNNING AS SHELL EXTENSION IS STILL EXPERIMENTAL, UNSUPPORTED, AND ONLY RECOMMENDED FOR SPECIFIC USE CASES THAT YOU SHOULD KNOW ABOUT ALREADY. IT IS NOT REQUIRED FOR OBTAINING ANY OF THE CORE FUNCTIONALITY, IT IS AVAILABLE ONLY FOR SOME ADVANCED USE CASES AND IS STILL A BIGGER WORK-IN-PROGRESS THAN THE MAIN PROJECT.**

The application can now be registered as a shell extension. This will enable the Explorer related functionality to work in Open/Save file dialogs as well. This is especially useful for users wanting proper support of the classic theme in Windows 11.

Please note that this is experimental. For the moment, the preferred installation method remains dropping the DLL in `C:\Windows`. For interested users, I invite you to test this functionality and report your findings in the discussions board.

To enable this, put the 2 DLLs (`ExplorerPatcher.amd64.dll` and `ExplorerPatcher.IA-32.dll`) in a secure folder (for example, `C:\Program Files\ExplorerPatcher`). Then, in that folder, run this command: `regsvr32 ExplorerPatcher.amd64.dll`. After elevation, a message will display informing you of the operation outcome, and if it went well, Explorer will restart displaying the old taskbar.

To uninstall, run `regsvr32 /u ExplorerPatcher.amd64.dll` in the same folder and preferably reboot the computer to unload the DLLs from all applications. Then, the files can be deleted just fine.


## 22000.258.30.6

Tested on build: 22000.258.

* Reworked settings framework
  * More settings are available to customize
  * Most setting changes take effect immediatly
* Implemented Windows 10 window switcher (Alt+Tab)
* GUI
  * Revamped GUI, now the interface is split by categories and is displayed on two columns
  * Regular items do not display a "+" sign anymore at the beginning of their label
  * The current choice is ticked in the drop down menu
  * Regions are now calculated correctly
  * Solved memory leaks
* Option to disable immersive context menus (#96)
* General bug fixes
* Window switcher is now disabled by default (.1)
* Corrected typo in settings (.2)
* Added option to set tray clock to display seconds (.3)
* Added option to set the right click menu of the network system tray icon to launch either (.3):
  * Network settings in the Settings app
  * Network and Sharing Center in the Control Panel
  * Network Connections in the Control Panel
* Added preliminary support for advanced mitigations for correct rendering when using the classic theme (.3)
* GUI optionally loads UI file from DLL folder (helps for easy debugging) (.4)
* Small bug fix for symbols download (.5)
* Better method for closing windows in window switcher (.6)

## 22000.258.26.3

Tested on build: 22000.258.

* Compatibility with OS build 22000.258
* Option to open Network and Sharing Center instead if Network settings when right clicking the network icon in the system tray
* Centered network and sound right click menus and made them toggle on right click
* Reliability enhancements for Start menu positioning (#78) (.1)
* Fixes #85 (.2)
* Fixes #90 (added option to change taskbar icon size) (.3)

## 22000.194.0.25

Tested on build: 22000.194.

* Start menu is hooked from File Explorer; please remove the DLL from `C:\Windows\SystemApps\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy` when using this new version
* `Win`+`X` now opens even when the taskbar is set to autohide (fixes #63)
* `Win`+`C` now opens even when the taskbar is set to autohide (fixes #63)
* Bluetooth and Safe to Remove menus toggle their visibility when clicked
* Bluetooth and Safe to Remove menus are centered relative to the icon they are invoked from
* WiFi list now correctly toggles when clicking the Network icon in the taskbar
* The settings GUI now supports dark mode and switches correctly when the system theme changes
* The settings GUI draws correctly when themes are disabled (classic theme compatibility)
* Removed interoperability with StartAllBack or StartIsBack

## 22000.194.0.23

Tested on build: 22000.194.

* Fixed a bug that showed`Win`+`X` on the wrong monitor in certain scenarios
* `Win`+`X` shows in Windows 11 fashion (centered, above the Start button) if using a centered taskbar with centered Start button as well (using a program like [TaskbarX](https://github.com/valinet/TaskbarX))
* Fixed the bug that prevented the application from loading in`StartMenuExperienceHost.exe` (thanks to @BraINstinct0 for the report)
* Fixed padding and element sizes in GUI so it better fits on smaller screens (thanks to @Gaurav-Original-ClassicShellTester for the report)
* GUI shows application title when run outside of File Explorer
* GUI stays on screen and just reloads the settings when restoring defaults (instead of closing)
* Keyboard (tab) support for GUI: `Esc` to close the window, `Tab` to select the next option, `Shift`+`Tab` to select the previous option, `Space` to toggle (or activate) the option
* Possibility of running the GUI standalone; run this command: `rundll32.exe C:\Windows\dxgi.dll,ZZGUI`; this has the advantage that it stays on the screen after restarting File Explorer

## 22000.194.0.22

Tested on build: 22000.194.

* When the taskbar is located at the bottom of the screen, opening the power user menu (`Win`+`X`) now automatically highlights the "Desktop" entry in the list. Also, the menu items can be activated either with left click, either with right click. Thus, this enables a behavior where you can double click the Start button with the right mouse button in order to quickly show the desktop (thanks to @Gaurav-Original-ClassicShellTester for the suggestion)

## 22000.194.0.21

Tested on build: 22000.194.

* Implemented configuration GUI; to access it, right click the Start button (or press `Win`+`X`) and choose "Properties" (thanks to @BraINstinct0 for the suggestion)

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

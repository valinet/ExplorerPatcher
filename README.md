# Explorer Patcher for Windows 11
This project aims to bring back a productive working environment on Windows 11.

<details>
  <summary>About ExplorerPatcher and StartIsBack</summary>
  The well-known [StartIsBack](https://www.startisback.com/) application recently introduced support for Windows 11. Currently, regarding Windows 11 functionality, it offers the same features as this patcher, plus all the functionalities from the older releases of the software; thus the differences between the two at the moment, besides their internal implementation (probably) resort in the cost, licensing, support and development model: StartIsBack is a paid app, proprietary, closed source and comes with support, while ExplorerPatcher is free ([gratis and libre - as in free beer and free speech](https://en.wikipedia.org/wiki/Gratis_versus_libre)), open source and is provided as-is, with support offered in a best-effort attempt.
  ExplorerPatcher is offered more like a tool for people to study the source code and the techniques used, to learn and adapt it in order and to enable people to build each on top of the work of the others. The aim is to benefit both the community and its users. You are encouraged to take a look at the source and adapt it to your needs. While the source and the application in its current form will always be available here, I cannot make any guarantes over how long it will work in newer Windows 11 builds. Thus, these things being considered, if you would like, you can check out the beta test for StartIsBack [here](http://startisback.com/tbd/) and report any issues and make suggestions to its developer. It will probably mature in a release that will be better supported from a consumer point of view than ExplorerPatcher.
</details>

<img src="https://gist.githubusercontent.com/valinet/d0f72ff09773702584e77c46065b95e0/raw/94036ed3e38218b87744a29ae5c40b06be637daf/ep_img0.png" width=100% height=100%>

Screenshots: [<1>](https://gist.githubusercontent.com/valinet/d0f72ff09773702584e77c46065b95e0/raw/94036ed3e38218b87744a29ae5c40b06be637daf/ep_img0.png) [<2>](https://user-images.githubusercontent.com/6503598/131937638-d513ca72-ead7-459b-a8ce-619fb302b7da.png)

## dxgi.dll - the patcher
In the [Releases](https://github.com/valinet/ExplorerPatcher/releases) section, you will find a download for a DLL called `dxgi.dll`. It has been tested on the latest stable build of Windows 11 (22000.194), but it may work just fine on newer builds too. This is the runtime part of the application. You need to place this DLL in the following location: `C:\Windows`. This will enable the following functionality:

* use the classic taskbar from Windows 10 (without the nasty effects of `UndockingDisabled`)
* restores the classic power user menu (`Win+X`) when using the classic taskbar
* Start menu follows the taskbar alignment setting (`Left`/`Center`) specified in `Settings\Personalization\Taskbar\Taskbar behaviors\Taskbar alignment`
* ability to show the Start menu on the monitor containing the cursor when pressing the Windows key
* skin "Safe to Remove Hardware" and "Bluetooth" popup menus
* play log on sound, if enabled
* option to hide the search bar in File Explorer
* option to disable the control center button in the taskbar
* show the "All apps" list by default when opening the Start menu
* customize the maximum number of "Most used" apps displayed in the "All apps" list in Start
* disable the immersive contex menu system-wide
* disable the Windows 11 File Explorer command bar

After you have completed the above setup, make sure you have an active Internet connection and restart the Explorer process using Task Manager or by issuing the following command: `taskkill /f /im explorer.exe`. Once File Explorer restarts, some necessary files (symbol files) will be downloaded from Microsoft (around 50MB). This should be relatively quick, depending on your Internet connection speed. When this is done, File Explorer will restart again and will be ready for use. Notifications should show up informing you about the progress, and you can also use Task Manager to watch for network activity. This process only happens when a new Windows 11 build is installed on the machine.

Now, the classic taskbar should be enabled. Still, there is some more setup to do, depending on your preferences.

## Configuration interface

To configure the most common options, the application now comes with a configuration user interface. To open it, right click the Start button (or press `Win`+`X`) and choose "Properties". Alternatively, to open the GUI standalone, run the following command: `rundll32.exe C:\Windows\dxgi.dll,ZZGUI`.

<img src="https://user-images.githubusercontent.com/6503598/135729021-6befba47-84ae-4c65-8133-e380f1d36fe1.png"  width=60% height=60%>

The icon near an option signifies its current state:

* ✔️ enabled
* ❌ disabled
* ➕ performs an action that allows you to change that option (usually, the current value is located after the colon in its description)

The links at the bottom allow you to perform the most frequent actions.

## Recommended tools

Here is a list of things you may want to try to fix/enhance a few of the aspects which are not addressed by this patcher directly:

### Fix the battery applet

As you will notice, the battery flyout in the taskbar is broken in Windows 11. You can replace it with a much better alternative called [Battery Mode](https://en.bmode.tarcode.ru/) which has all the stock options and more.

### Disable blue highlight in menus

To disable the blue highlight in the context menu and return to the classic gray highlight from early Windows 11 builds, read [here](https://github.com/valinet/ExplorerPatcher/issues/18).

### Center taskbar icons

If you want the same behavior as the default one in Windows 11, which is to have the icons centered along with the Start button, but would like to use this proper classic taskbar which has features like button labels, toolbars and more, you can use my fork of the popular [TaskbarX](https://github.com/ChrisAnd1998/TaskbarX) program which fixes compatibility with Windows 11 and adds this behavior; a guide about how to set it up is available [here](https://github.com/valinet/ExplorerPatcher/issues/33).

### Disable window rounded corners

You can try one of my other utilities available [here](https://github.com/valinet/Win11DisableRoundedCorners).

## Manual configuration

To learn how to configure all the options manually, read on:

### Enable system tray icons
As you have noticed, some system tray icons are missing (for example, the clock, notification center button, network, battery, sound etc). To enable these icons, open the following using Run: 

```
%windir%\explorer.exe shell:::{05d7b0f4-2121-4eff-bf6b-ed3f69b894d9}\SystemIcons
```

In the window that appears, toggle to `On` whatever icons you would like to have enabled in the taskbar.

### Show labels for taskbar buttons
One of the great features the old taskbar had was the ability to ungroup the taskbar buttons, showing a button with a label for each window that the user currently has open. To enable this functionality, run either of the following commands, depending on your preference.

#### Ungroup icons on all taskbars

```
reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer" /f /v "NoTaskGrouping" /t REG_DWORD /d 1
```

#### Ungroup icons on main taskbar only

```
reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced" /f /v "TaskbarGlomLevel" /t REG_DWORD /d 2
```

#### Ungroup icons on secondary taskbars

```
reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced" /f /v "MMTaskbarGlomLevel" /t REG_DWORD /d 2
```

In the commands above, change `2` to `0` for "Always combine" and `2` to `1` for "Combine when taskbar is full`.

### Disable the immersive context menu

As you probably have noticed, Windows 11 introduced a simplified context menu in File Explorer. To get to the old menu which contains all entries from shell extensions, one has to click on "Show more options" or type Shift+F10. To disable this new menu, run the following command:

```
reg.exe add "HKCU\Software\Classes\CLSID\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}\InprocServer32" /f /ve
```

To restore back the immersive menu, run: 

```
reg.exe delete "HKCU\Software\Classes\CLSID\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}" /f
```

### Disable the command bar in File Explorer
In Windows 11, File Explorer comes with a command bar instead of the traditional ribbon. To disable this and get back the ribbon, run the following command:

```
reg.exe add "HKCU\Software\Classes\CLSID\{d93ed569-3b3e-4bff-8355-3c44f6a52bb5}\InprocServer32" /f /ve
```

To restore back the command bar, run: 

```
reg.exe delete "HKCU\Software\Classes\CLSID\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}" /f
```

If you want to get back the command bar from Windows 7, after enabling back the ribbon,  [OldNewExplorer](https://msfn.org/board/topic/170375-oldnewexplorer-119/) will allow you to do so, and many more options, like drive ungrouping in This PC, as it used to be prior to Windows 8.1.

### Disable the modern search box in File Explorer
In Windows 10 1903, Microsoft introduced a new search box in File Explorer. This new search control is clunky, does not search automatically and usually is kind of broken. The old search box remains available, but only in Control Panel windows. To enable it on all File Explorer windows, run the following command:

```
reg.exe add "HKCU\Software\Classes\CLSID\{1d64637d-31e9-4b06-9124-e83fb178ac6e}\TreatAs" /f /ve /t REG_SZ /d "{64bc32b5-4eec-4de7-972d-bd8bd0324537}"
```

To restore the modern search box, run: 

```
reg.exe delete "HKCU\Software\Classes\CLSID\{1d64637d-31e9-4b06-9124-e83fb178ac6e}" /f
```

Also, in the next section, which desribes the configuration options for the software, you will learn about how to disable the search box altogether, should you want to.

### Patcher settings
Now that you have set up the basic stuff, you can choose to enable additional settings to enhance the experience even more. For this, customize the following commands by changing the number acording to your needs:

* `HideExplorerSearchBar` completely removes the search box in File Explorer (default = 0)

  ```
  reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher" /f /v "HideExplorerSearchBar" /t REG_DWORD /d 1
  ```

* `HideControlCenterButton` disables the Control Center button and its associated shortcut key (`Win`+`A`) (default = 0)

  ```
  reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher" /f /v "HideControlCenterButton" /t REG_DWORD /d 1
  ```

* `MicaEffectOnTitlebar` enables Mica effect on File Explorer windows (requires `StartIsBack64.dll`) (default = 0)

  ```
  reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher" /f /v "MicaEffectOnTitlebar" /t REG_DWORD /d 1
  ```

* `SkinMenus` applies the immersive skin to "Safe to Remove Hardware" and "Bluetooth" pop-up menus (default = 1)

  ```
  reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher" /f /v "SkinMenus" /t REG_DWORD /d 1
  ```

* `SkinIcons` applies Windows 11 icon skins to taskbar buttons (requires `StartIsBack64.dll`) (default = 1)

  ```
  reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher" /f /v "SkinIcons" /t REG_DWORD /d 1
  ```

* `AllocConsole` will display a console window (for debugging purposes) (default = 0, for advanced users only)

  ```
  reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher" /f /v "AllocConsole" /t REG_DWORD /d 1
  ```

* `ExplorerReadyDelay` adds even more delay before the shell is announced that Explorer is ready loading (helps if you experience a delay at logon) - the unit is ms (milliseconds), 1000ms = 1 second (default = 0, for advanced users only)

  ```
  reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher" /f /v "ExplorerReadyDelay" /t REG_DWORD /d 1000
  ```

Also, if you chose to place the patcher in `C:\Windows\SystemApps\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy` as well, use the following commands to configure Start menu options:

* Open Start menu to "All apps" directly (replace with 0 to disable)

  ```
  reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\StartPage" /f /v "MakeAllAppsDefault" /t REG_DWORD /d 1
  ```

* Show only 4 most recent apps (change the number to customize)

  ```
  reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced" /f /v "Start_MaximumFrequentApps" /t REG_DWORD /d 4
  ```
## More configuration
Even more registry configuration settings are described in the following document, make sure to take a look on it [here](https://github.com/valinet/ExplorerPatcher/issues/9).

## Uninstallation
First, move the taskbar to the bottom, if you have moved it to the top or to the sides. This is so that the Windows 11 taskbar can render correctly.

Then, to uninstall, rename the DLL in the locations where you placed it from `dxgi.dll` to `dxgio.dll`. Then, reboot the computer and delete the `dxgio.dll` file from the locations where it is placed. You can also then safely delete the `%appdata%\ExplorerPatcher` directory as well. 

## More details
A changelog is available [here](https://github.com/valinet/ExplorerPatcher/blob/master/CHANGELOG.md).

<details>
  <summary>How does this work?</summary>
A detailed description of how this works is available on my web site [here](https://valinet.ro/2021/08/09/Restore-Windows-11-to-working-Windows-10-UI.html).

The way the application gets loaded is by exploiting the DLL search order in Windows. I take advantage of the fact that Explorer is one of the few system processes located in `%windir%` and not in `%windir%\System32`, so it does not affect most apps. Also, `%windir%` is not first in the search path. Read more about this technique [here](https://itm4n.github.io/windows-dll-hijacking-clarified/). The main advantage here is that you do not have to keep an extra process running in the memory; plus, due to the diverse nature of how Explorer is launched, hooking it can be difficult.

I picked `dxgi.dll` because it is not on the `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\KnownDLLs` list, because it has few exports and is loaded very early by Explorer, when calling the `DXGIDeclareAdapterRemovalSupport()` function.
</details>

<details>
  <summary>A note on antivirus false positives</summary>
The DLL you download may trigger a false positive alert in your antivirus program. This is fairly normal, a sign that the product you are using is decently capable, since it features detection methods a bit more advanced than what was state of the art in 1999. The program being flagged is usually done through heuristic analysis, not via a database of know viruses. That means that the antivirus program thinks that due to the nature of the code in this application, it likely may be a virus, a program that the user does not really mean to run. Injecting code into other executables is rarely done by legitimate programs. One such kind of legitimate programs is this patcher, that you deliberately choose to run and let it alter Explorer's code and memory in a controlled manner so that you achieve a certain effect. Thus, this patcher falls in the category of false positives: your antivirus thought such a program was not something you meant to run, but that's not the case this time.

If you still feel a bit iffy running this, then that's why the source code is available on the web site. You can take a look and even compile the DLL yourself and make an informed decision when running the software. Never run untrusted binaries from questionable sources, especially when you lack access to the source code and a way to reproduce that binary.
</details>

## License

Hooking is done using the excellent [funchook](https://github.com/kubo/funchook) library (GPLv2 with linking exception), which in turn is powered by the [diStorm3](https://github.com/gdabah/distorm/) (3-clause BSD) disassembler. Thus, I am offering this under GNU General Public License Version 2.0, which I believe is compatible.

## Compiling

The following prerequisites are necessary in order to compile this project:

* Microsoft C/C++ Optimizing Compiler - this can be obtained by installing either of these packages:

  * Visual Studio - this is a fully featured IDE; you'll need to check "C/C++ application development role" when installing. If you do not require the full suite, use the package bellow.
  * Build Tools for Visual Studio - this just installs the compiler, which you'll be able to use from the command line, or from other applications like CMake

  Download either of those [here](http://go.microsoft.com/fwlink/p/?LinkId=840931). The guide assumes you have installed either Visual Studio 2019, either Build Tools for Visual Studio 2019.

* A recent version of the [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk/) - for development, version 10.0.19041.0 was used, available [here](https://go.microsoft.com/fwlink/p/?linkid=2120843) (this may also be offered as an option when installing Visual Studio)

* [CMake](https://cmake.org/) - for easier usage, make sure to have it added to PATH during installation

* Git - you can use [Git for Windows](https://git-scm.com/download/win), or git command via the Windows Subsystem for Linux.

Steps:

1. Clone git repo along with all submodules

   ```
   git clone --recursive https://github.com/valinet/ExplorerPatcher
   ```

   If "git" is not found as a command, type its full path, or have its folder added to PATH, or open Git command window in the respective folder if using Git for Windows.

2. Compile funchook

   ```
   cd libs
   cd funchook
   md build
   cd build
   cmake -G "Visual Studio 16 2019" -A x64 ..
   ```

   If "cmake" is not found as a command, type its full path, or have its folder added to PATH.

   Type "Win32" instead of "x64" above, if compiling for x86. The command above works for x64.

   Now, in the `libs\funchook\build` folder, open the file `funchook-static.vcxproj` with any text editor, search and replace all occurences of `<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>` with `<RuntimeLibrary>MultiThreaded</RuntimeLibrary>`.

   Once done, you can now compile funchook:

   ```
   cmake --build . --config Release
   ```

3. Compile ExplorerPatcher

   * Double click the ExplorerPatcher.sln to open the solution in Visual Studio. Choose Release and your processor architecture in the toolbar. Press F6 to compile.

   * Open an "x86 Native Tools Command Prompt for VS 2019" (for x86), or "x64 Native Tools Command Prompt for VS 2019" (for x64) (search that in Start), go to folder containing solution file and type:

     * For x86:

       ```
       msbuild ExplorerPatcher.sln /property:Configuration=Release /property:Platform=x86
       ```

     * For x64:

       ```
       msbuild ExplorerPatcher.sln /property:Configuration=Release /property:Platform=x64
       ```

   The resulting exe and dll will be in "Release" folder (if you chose x86), or "x64\Release" (if you chose x64) in the folder containing the solution file.

That's it. later, if you want to recompile, make sure to update the repository and the submodules first:

```
git pull
git submodule update --init --recursive
```


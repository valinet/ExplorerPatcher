# Explorer Patcher for Windows 11
This application aims to bring back a productive working environment on Windows 11, by restoring dormant functionalities from Windows 10 in the newest OS builds.

The well-known [StartIsBack](https://www.startisback.com/) application recently introduced support for Windows 11. Currently, regarding Windows 11 functionality, it offers the same features as this patcher, plus all the functionalities from the older releases of the software; thus the differences between the two at the moment, besides their internal implementation (probably) resort in the cost, licensing, support and development model: StartIsBack is a paid app, proprietary, closed source and comes with support, while ExplorerPatcher is free ([gratis and libre - as in free beer and free speech](https://en.wikipedia.org/wiki/Gratis_versus_libre)), open source and is provided as-is, with support offered in a best-effort attempt.

ExplorerPatcher is offered more like a tool for people to study the source code and the techniques used, to learn and adapt it in order and to enable people to build each on top of the work of the others. The aim is to benefit both the community and its users. You are encouraged to take a look at the source and adapt it to your needs. While the source and the application in its current form will always be available here, I cannot make any guarantes over how long it will work in newer Windows 11 builds. Thus, these things being considered, if you would like, you can check out the beta test for StartIsBack [here](http://startisback.com/tbd/) and report any issues and make suggestions to its developer. It will probably mature in a release that will be better supported from a consumer point of view than ExplorerPatcher.

Current functionality enabled by this patcher includes:

* use the classic taskbar from Windows 10 (without the nasty effects of `UndockingDisabled`)
* restores the classic power user menu (`Win+X`) when using the classic taskbar
* ability for the Start menu and search to align to the left when using the classic taskbar (it follows the setting `Left`/`Center` in `Settings\Personalization\Taskbar\Taskbar behaviors\Taskbar alignment`)
* ability for the Start menu to show the app list by default when opening
* ability to customize the maximum number of "Most used" apps displayed in the app list in Start
* ability to show the Start menu on the monitor containing the cursor when you press the Windows key*
* disable the new context menus in File Explorer and restore the classic ones
* disable the "modern search box" in File Explorer and revert to the proper working one from initial Windows 10 builds (or from Windows 7 and 8)
* skin the "Safe to Remove Hardware" pop-up to match the context menus of the taskbar
* disables the logon delay which happened if you were to enable the classic taskbar using `UndockingDisabled`
* play log on sound, if enabled

It has been developed on and tested to work against the latest builds, 22000.176 and 22449.1000 (Insider Preview). It should work on newer builds as well as long as the internal structure does not change too much (hopefully Microsoft won't remove all the legacy code).

A changelog is available [here](https://github.com/valinet/ExplorerPatcher/blob/master/CHANGELOG.md).

A detailed description of how this works is available on my web site [here](https://valinet.ro/2021/08/09/Restore-Windows-11-to-working-Windows-10-UI.html).

The application comes in the form of a dynamic-link library (DLL). Precompiled binaries are available in [Releases](https://github.com/valinet/ExplorerPatcher/releases).

Screenshots: [<1>](https://gist.githubusercontent.com/valinet/d0f72ff09773702584e77c46065b95e0/raw/94036ed3e38218b87744a29ae5c40b06be637daf/ep_img0.png) [<2>](https://user-images.githubusercontent.com/6503598/131937638-d513ca72-ead7-459b-a8ce-619fb302b7da.png)

(*) There is a bug currently in Windows 11 22000.176 where search may not open properly on other monitors. This is not because of this patcher, and will probably (hopefully) be fixed by Microsoft in a future build.

## Installation

Simply copy the downloaded DLL named `dxgi.dll` to `%windir%` (usually `C:\Windows`) and restart Explorer.

At first launch, the application will notify you about missing symbols and will automatically download them from Microsoft. When this is done, a notification will show informing you that everything's done and Explorer will restart and display the old taskbar. Note that for those with relevant toast display turned off, this process may be done automatically without any notification. Normally giving it 5 mins will suffice.

After Explorer restarts, the classic taskbar will be available and fully functioning, but you will notice the system tray misses the status icons. Those can be easily enabled by opening `Run` and going to `%windir%\explorer.exe shell:::{05d7b0f4-2121-4eff-bf6b-ed3f69b894d9}\SystemIcons` and enabling each system icon you wish from there. For a list of other useful registry settings that can help you make the most out of this application, like disabling taskbar grouping, read [here](https://github.com/valinet/ExplorerPatcher/issues/9).

After you get the classic taskbar working, to make it work with the Start menu and search and enable related functionality, copy the DLL to the following 2 locations as well:

* `C:\Windows\SystemApps\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy`
* `C:\Windows\SystemApps\MicrosoftWindows.Client.CBS_cw5n1h2txyewy`

After that is done, kill both `StartMenuExperienceHost.exe` and `SearchHost.exe` from Task Manager, or simply log out and back in or restart the computer.

Two of the applets in the system tray do not work in this mode: battery and network. To replace the battery applet, I recommend the much more capable [Battery Mode](https://en.bmode.tarcode.ru/) application which has very good integration and allows showing both classic power modes and Windows 10 power schemes (better battery, better performance etc), changing the display brightness (for laptop screens AND monitors) etc. To replace the network icon, I recommend using the control center to switch networks and keep the legacy icon only as an indicator. You can also have it open the "Network" section in the Settings app as described [here](https://winaero.com/change-network-icon-click-action-in-windows-10).

If you want to disable the blue highlight in the context menu and return to the classic gray highlight from early Windows 11 builds, read [here](https://github.com/valinet/ExplorerPatcher/issues/18). Also, if you want to disable the rounded corners of the windows in Windows 11, I recommend you one of my other utilities available [here](https://github.com/valinet/Win11DisableRoundedCorners).

Downloaded symbols and application configuration is saved in the `%appdata%\ExplorerPatcher` folder.

To uninstall, simply delete `dxgi.dll` from all the directories above. If you get a "file in use" error when attempting to do so, simply rename it everywhere to `dxgia.dll`, reboot the computer and then delete the renamed DLL.

#### A note on antivirus false positives

The DLL you download may trigger a false positive alert in your antivirus program. This is fairly normal, a sign that the product you are using is decently capable, since it features detection methods a bit more advanced than what was state of the art in 1999. The program being flagged is usually done through heuristic analysis, not via a database of know viruses. That means that the antivirus program thinks that due to the nature of the code in this application, it likely may be a virus, a program that the user does not really mean to run. Injecting code into other executables is rarely done by legitimate programs. One such kind of legitimate programs is this patcher, that you deliberately choose to run and let it alter Explorer's code and memory in a controlled manner so that you achieve a certain effect. Thus, this patcher falls in the category of false positives: your antivirus thought such a program was not something you meant to run, but that's not the case this time.

If you still feel a bit iffy running this, then that's why the source code is available on the web site. You can take a look and even compile the DLL yourself and make an informed decision when running the software. Never run untrusted binaries from questionable sources, especially when you lack access to the source code and a way to reproduce that binary.

#### How does this work?

The way the application gets loaded is by exploiting the DLL search order in Windows. I take advantage of the fact that Explorer is one of the few system processes located in `%windir%` and not in `%windir%\System32`, so it does not affect most apps. Also, `%windir%` is not first in the search path. Read more about this technique [here](https://itm4n.github.io/windows-dll-hijacking-clarified/). The main advantage here is that you do not have to keep an extra process running in the memory; plus, due to the diverse nature of how Explorer is launched, hooking it can be difficult.

I picked `dxgi.dll` because it is not on the `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\KnownDLLs` list, because it has few exports and is loaded very early by Explorer, when calling the `DXGIDeclareAdapterRemovalSupport()` function.

## Configuration

The `settings.ini` file contains, among the offsets for the various hooked/exploited functions, a few parameters that you can tweak:

* `General\AllowImmersiveContextMenus = 1` will show the new context menus in Explorer instead of the legacy one
* `General\AllowModernSearchBox = 1` will leave the modern search box enabled in File Explorer
* `General\AllocConsole = 1` will show a console when the application runs (useful for diagnostics).

The rest of the parameters that you can tweak and have them work with the application are located in the registry and are described [here](https://github.com/valinet/ExplorerPatcher/issues/9).

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


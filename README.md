# Explorer Patcher for Windows 11
This project aims to bring back a productive working environment on Windows 11.

PayPal donations: [here](https://www.paypal.com/donate?business=valentingabrielradu%40gmail.com&no_recurring=0&item_name=ExplorerPatcher&currency_code=EUR) or using e-mail address valentingabrielradu@gmail.com

<details>
  <summary>About ExplorerPatcher and StartIsBack</summary>

  The well-known [StartIsBack](https://www.startisback.com/) application recently introduced support for Windows 11. Currently, regarding Windows 11 functionality, it offers the same features as this patcher, plus all the functionalities from the older releases of the software; thus the differences between the two at the moment, besides their internal implementation (probably) resort in the cost, licensing, support and development model: StartIsBack is a paid app, proprietary, closed source and comes with support, while ExplorerPatcher is free ([gratis and libre - as in free beer and free speech](https://en.wikipedia.org/wiki/Gratis_versus_libre)), open source and is provided as-is, with support offered in a best-effort attempt.
  ExplorerPatcher is offered more like a tool for people to study the source code and the techniques used, to learn and adapt it in order and to enable people to build each on top of the work of the others. The aim is to benefit both the community and its users. You are encouraged to take a look at the source and adapt it to your needs. While the source and the application in its current form will always be available here, I cannot make any guarantes over how long it will work in newer Windows 11 builds. Thus, these things being considered, if you would like, you can check out the beta test for StartIsBack [here](http://startisback.com/tbd/) and report any issues and make suggestions to its developer. It will probably mature in a release that will be better supported from a consumer point of view than ExplorerPatcher.
</details>

<img src="https://gist.githubusercontent.com/valinet/d0f72ff09773702584e77c46065b95e0/raw/94036ed3e38218b87744a29ae5c40b06be637daf/ep_img0.png" width=100% height=100%>

The functionality of this software includes:

* Taskbar from Windows 10:
  * "always combine" / "combine when full" / "never combine" options for main and secondary taskbars
  * Search button
  * Task View button
  * optional skinned, centered and toggled pop-up menus, or non-skinned (useful for classic theme users)
  * open "Network and Sharing Center" when clicking "Open Network & Internet settings" in the pop-ip menu of the system tray network icon
  * hide Control Center button
  * Show People on the taskbar
  * Show touch keyboard button

* File Explorer
  * disable the Windows 11 command bar
  * disable the Windows 11 context menu
  * disable even the Windows 10 (immersive) context menu (useful for classic theme users)
  * disable modern search bar (reverts to the search bar in early Windows 10 builds or Windows 7/8)
  * disable search bar completely
* Start menu from Windows 11
  * open Start on monitor containing the cursor
  * open Start at logon
  * open Start in "All apps" by default
  * positioning on screen
  * maximum number of frequent apps to show
* Window switcher (Alt-Tab) from Windows 10:
  * choose whether to include desktop in the window list
  * ability to set opacity of the window list
  * set row size, maximum extents etc
* Others
  * Open the time and date flyout when pressing `Win`+`C` (instead of Microsoft Teams)
  * Set default shutdown action for `Alt`+`F4` on the desktop
  * Show Command Prompt instead of PowerShell in the `Win`+`X` menu

Screenshots: [<1>](https://gist.githubusercontent.com/valinet/d0f72ff09773702584e77c46065b95e0/raw/94036ed3e38218b87744a29ae5c40b06be637daf/ep_img0.png) [<2>](https://user-images.githubusercontent.com/6503598/131937638-d513ca72-ead7-459b-a8ce-619fb302b7da.png)

## dxgi.dll - the patcher
In the [Releases](https://github.com/valinet/ExplorerPatcher/releases) section, you will find a download for a DLL called `dxgi.dll`. It has been tested on the latest stable build of Windows 11 (22000.258), but it may work just fine on newer builds too. This is the runtime part of the application. You need to place this DLL in the following location: `C:\Windows`. 

After you have completed the above setup, make sure you have an active Internet connection and restart the Explorer process using Task Manager or by issuing the following command: `taskkill /f /im explorer.exe`. Once File Explorer restarts, some necessary files (symbol files) will be downloaded from Microsoft (around 50MB). This should be relatively quick, depending on your Internet connection speed. When this is done, File Explorer will restart again and will be ready for use. Notifications should show up informing you about the progress, and you can also use Task Manager to watch for network activity. This process only happens when a new Windows 11 build is installed on the machine.

Now, the classic taskbar should be enabled. Still, there is some more setup to do, depending on your preferences.

Important! As you may notice, the usual system tray icons are not enabled by default. To enable them, open the GUI (`Win`+`X` and choose "Enable missing system tray icons").

## Configuration interface

To configure the most common options, the application now comes with a configuration user interface. To open it, right click the Start button (or press `Win`+`X`) and choose "Properties".

All of the options described above, and more, can be configured using the GUI. Below is a screenshot of the main page ("Taskbar") of the configuration interface.

<img src="https://user-images.githubusercontent.com/6503598/137494597-6ab591bc-3384-4b86-99be-80d2dedcd492.png"  width=70% height=70%>

The icon near an option signifies its current state:

* ✔️ enabled
* ❌ disabled
* ➕ link to another resource

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

You can manually configure the application by setting registry values. The registry entries supported by this application are described in [this file](https://github.com/valinet/ExplorerPatcher/blob/master/ExplorerPatcher/settings.reg).

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

If you encounter some issues when following the steps below, or should you want to find out more information and the latest tips from other users/me regarding how to pull this off, consult the relevant thread in Discussions located [here](https://github.com/valinet/ExplorerPatcher/discussions/190).

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

   Type "Win32" instead of "x64" above, if compiling for x86 (IA-32). The command above works for x64 (amd64).

   Now, in the `libs\funchook\build` folder, open the file `funchook-static.vcxproj` with any text editor, search and replace all occurences of `<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>` with `<RuntimeLibrary>MultiThreaded</RuntimeLibrary>`.

   Once done, you can now compile funchook:

   ```
   cmake --build . --config Release
   ```

3. Compile ExplorerPatcher

   * Double click the `ExplorerPatcher.sln` file to open the solution in Visual Studio. Choose Release and your processor architecture in the toolbar. Press `[Ctrl]`+`[Shift]`+`[B]` or choose "Build" - "Build solution" to compile.

   * Open an "x86 Native Tools Command Prompt for VS 2019" (for x86), or "x64 Native Tools Command Prompt for VS 2019" (for x64) (search that in Start), go to folder containing solution file and type:

     * For x86:

       ```
       msbuild ExplorerPatcher.sln /property:Configuration=Release /property:Platform=IA-32
       ```

     * For x64:

       ```
       msbuild ExplorerPatcher.sln /property:Configuration=Release /property:Platform=amd64
       ```

   Debug builds mostly work as well, although they may have some quirks or issues. I mostly test the software as release builds.

   The resulting libraries will be in the "build\Release" folder in the folder containing the solution file.

That's it. later, if you want to recompile, make sure to update the repository and the submodules first:

```
git pull
git submodule update --init --recursive
```


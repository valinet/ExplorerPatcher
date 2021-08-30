# Explorer Patcher
Explorer Patcher is a patcher that enables various stuff in Explorer. For the moment, it includes the following:

* allows using the old taskbar in Windows 11 without the side effects of UndockingDisabled and with fully working search, modern apps showing properly, screen snip still working etc
* enables the power user menu (Win+X) when using the classic taskbar in Windows 11
* shows the Start menu on the monitor containing the cursor when invoked with the Windows key

This has been tested on the following builds:

* 22000.1 - works as advertised, tested with `Start_ShowClassicMode` which shows the Windows 10 Start menu; taskbar works, Win+X works and is skinned, WiFi flyout works, battery flyout works, no delay at logon
* 22000.168 - works as advertised, Start button opens Windows 11 Start menu (a half broken Windows 10 menu can be restored by copying `StartMenuExperienceHost.exe` and its DLLs from 22000.1); taskbar works, Win+X works and is skinned, WiFi flyout DOES NOT work (use control center aka gear icon or [enable Windows 8 network flyout](https://winaero.com/change-network-icon-click-action-in-windows-10/?utm_source=software&utm_medium=in-app&utm_campaign=winaerotweaker&utm_content=networkflyout) instead, battery flyout DOES NOT work (use [Battery Mode](https://en.bmode.tarcode.ru/) utility), control center icon works, no delay at logon (!!!)

It should work on newer builds as well as long as the internal structure does not change too much (hopefully they don't remove all the legacy code), I am rolling with them at the moment and try to keep it working on the most recent CU (as of this time, 22000.168).

A changelog is available [here](https://github.com/valinet/ExplorerPatcher/blob/master/CHANGELOG.md).

A detailed description of how this works is available on my web site [here](https://valinet.ro/2021/08/09/Restore-Windows-11-to-working-Windows-10-UI.html).

Precompiled binaries are available in [Releases](https://github.com/valinet/ExplorerPatcher/releases).

## Installation

To install, save the executable in a safe directory, run it once as an administrator to have it register as [Taskman](https://www.geoffchappell.com/notes/windows/shell/explorer/taskman.htm) for Explorer and just restart Explorer or reboot.

The application does not currently offer a way to configure its behavior. In the mean time, I recommend commenting out whatever you do not like and compile your own executable, as described below (instructions are very simple).

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


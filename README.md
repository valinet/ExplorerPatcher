# Explorer Patcher
Explorer Patcher is a patcher that enables various stuff in Explorer. For the moment, it includes the following:

* allows using the old taskbar in Windows 11 without the side effects of UndockingDisabled and with fully working search, modern apps showing properly, screen snip still working etc
* enables the power user menu (Win+X) when using the classic taskbar in Windows 11

This has been tested only on Windows 11 build 22000.1. It probably does not work on other builds due to different offsets in explorer.exe and its libraries. Once this matures, a solution will be offered for dynamically determining the necessary offsets. As it stands, the application is more in a proof of concept phase.

A detailed description of how this works is available on my web site [here](https://valinet.ro/2021/08/09/Restore-Windows-11-to-working-Windows-10-UI.html).

Precompiled binaries are available in [Releases](https://github.com/valinet/ExplorerPatcher/releases).

Please help me fix the known issues described below in order to get feature parity with regular Windows 10 releases. As it stands, the application works but it is not perfect, yet.

## Installation

To install, save the executable in a safe directory, run it once as an administrator to have it register as [Taskman](https://www.geoffchappell.com/notes/windows/shell/explorer/taskman.htm) for Explorer and just restart Explorer or reboot.

The application does not currently offer a way to configure its behavior. In the mean time, I recommend commenting out whatever you do not like and compile your own executable, as described below (instructions are very simple).

## Known issues

* The power user menu (Win+X menu) is unskinned - it has the default menu appearance from Windows 11 instead of the look the clock, taskbar etc context menus get; at the moment, I think it boils down to correctly calling `ImmersiveContextMenuHelper::ApplyOwnerDrawToMenu`, but unfortunately I've yet to do it correctly
* The power user menu (Win+X menu) is mapped to Win+F for the moment, as I've yet to have the time to investigate where exactly Win+X is handled (i.e. it is not in `CImmersiveHotkeyNotification::OnMessage`)

## License

Hooking is done using the excellent [funchook](https://github.com/kubo/funchook) library (GPLv2 with linking exception), which in turn is powered by the [diStorm3](https://github.com/gdabah/distorm/) (3-clause BSD) disassembler. Thus, I am offering this under GNU General Public License Version 2.0, which I believe is compatible.

## Compiling

The following prerequisites are necessary in order to compile this project:

* Microsoft C/C++ Optimizing Compiler - this can be obtained by installing either of these packages:

  * Visual Studio - this is a fully featured IDE; you'll need to check "C/C++ application development role" when installing. If you do not require the full suite, use the package bellow.
  * Build Tools for Visual Studio - this just installs the compiler, which you'll be able to use from the command line, or from other applications like CMake

  Download either of those [here](http://go.microsoft.com/fwlink/p/?LinkId=840931). The guide assumes you have installed either Visual Studio 2019, either Build Tools for Visual Studio 2019.

* A recent version of the [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk/) - for development, version 10.19041.0 was used, available [here](https://go.microsoft.com/fwlink/p/?linkid=2120843) (this may also be offered as an option when installing Visual Studio)

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
   cmake --build . --config Release
   ```

   If "cmake" is not found as a command, type its full path, or have its folder added to PATH.

   Type "Win32" instead of "x64" above, if compiling for x86. The command above works for x64.

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


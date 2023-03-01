# ExplorerPatcher Windows 7 Alt-Tab Module

This module patches the Windows 7 genuine Alt-Tab implementation to work on newer Windows versions.

To install, make sure you have the following files in `C:\Windows`:

* `ep_extra_valinet.win7alttab.dll` - this DLL
* `ep_extra.dll` - a chainloader capable of being invoked by ExplorerPatcher (implements `ep_extra_EntryPoint` and which loads other `ep_extra_*.dll` modules
* `AltTab.dll` - a copy of the `AltTab.dll` in `C:\Windows\System32` from a Windows 7 installation

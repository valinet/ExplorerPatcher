# ExplorerPatcher Custom Libraries Chainloader

ExplorerPatcher has a simple, built-in mechanism that allows users to load their own DLL into `explorer.exe` right after ExplorerPatcher finishes initializing its hooks. Interested users should place a DLL called `ep_extra.dll` in `C:\Windows`. When ExplorerPatcher finishes its setup, it loads the `ep_extra.dll` library and calls the `ep_extra_EntryPoint` function. Although this is very useful so that users can load their custom code, it is quite limited at the moment, as it loads just one DLL.

This project is a solution to this issue. A chainloader is implemented here, that looks for other modules matching the `ep_extra_*.dll` pattern in `C:\Windows` as well, and loads them one after the other.

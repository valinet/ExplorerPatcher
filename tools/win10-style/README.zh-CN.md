# Windows 10 风格一键脚本

这个目录提供两个 PowerShell 脚本，用来一键应用或回退 Windows 10 风格体验。

## 一键应用

```powershell
Set-ExecutionPolicy -Scope Process Bypass -Force
.\tools\win10-style\Apply-Win10ExplorerStyle.ps1
```

脚本会尽量完成以下设置：

- 检查并安装 ExplorerPatcher
- 启用 Windows 10 风格任务栏
- 任务栏靠左
- 任务栏按钮从不合并
- 右下角托盘图标全部显示，不再收纳进隐藏区域
- 恢复经典右键菜单
- 启用 Windows 10 磁贴开始菜单
- 重启 `StartMenuExperienceHost.exe` 和 `explorer.exe`

## 一键回退

```powershell
Set-ExecutionPolicy -Scope Process Bypass -Force
.\tools\win10-style\Undo-Win10ExplorerStyle.ps1
```

如果想同时卸载 ExplorerPatcher：

```powershell
.\tools\win10-style\Undo-Win10ExplorerStyle.ps1 -UninstallExplorerPatcher
```

## 关键设置

Windows 10 磁贴开始菜单依赖这个注册表值：

```powershell
HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced
Start_ShowClassicMode = 1
```

如果已经在属性窗口里选择 Windows 10 开始菜单，但仍然显示 Windows 11 开始菜单，请确认上面的值为 `1`，然后重启 `StartMenuExperienceHost.exe` 和 `explorer.exe`。

## 注意

- 脚本会修改当前用户的注册表设置。
- 某些 Windows 11 26xxx / Insider 构建只部分支持 Windows 10 磁贴开始菜单。
- 建议运行前创建系统还原点。

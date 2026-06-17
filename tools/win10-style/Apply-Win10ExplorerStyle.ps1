#requires -version 5.1
[CmdletBinding()]
param(
    [switch]$SkipExplorerPatcherInstall,
    [switch]$NoRestart
)

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host "[Win10风格] $Message"
}

function Set-DwordValue {
    param(
        [Parameter(Mandatory)] [string]$Path,
        [Parameter(Mandatory)] [string]$Name,
        [Parameter(Mandatory)] [int]$Value,
        [switch]$Optional
    )

    if (-not (Test-Path $Path)) {
        New-Item -Path $Path -Force | Out-Null
    }

    try {
        New-ItemProperty -Path $Path -Name $Name -Value $Value -PropertyType DWord -Force | Out-Null
    }
    catch {
        if ($Optional) {
            Write-Warning "可选设置写入失败：$Path\$Name。$($_.Exception.Message)"
            return
        }
        throw
    }
}

function Set-ClassicContextMenu {
    $key = "HKCU:\Software\Classes\CLSID\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}\InprocServer32"
    if (-not (Test-Path $key)) {
        New-Item -Path $key -Force | Out-Null
    }
    Set-Item -Path $key -Value ""
}

function Set-AllTrayIconsVisible {
    Set-DwordValue -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer" -Name "EnableAutoTray" -Value 0 -Optional

    $notifyRoot = "HKCU:\Control Panel\NotifyIconSettings"
    if (Test-Path $notifyRoot) {
        Get-ChildItem -Path $notifyRoot -ErrorAction SilentlyContinue | ForEach-Object {
            New-ItemProperty -Path $_.PSPath -Name "IsPromoted" -Value 1 -PropertyType DWord -Force -ErrorAction SilentlyContinue | Out-Null
        }
    }
}

function Get-ExplorerPatcherInstall {
    $uninstallRoots = @(
        "HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall",
        "HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall",
        "HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall"
    )

    foreach ($root in $uninstallRoots) {
        if (-not (Test-Path $root)) {
            continue
        }

        $match = Get-ChildItem -Path $root -ErrorAction SilentlyContinue |
            ForEach-Object { Get-ItemProperty $_.PSPath -ErrorAction SilentlyContinue } |
            Where-Object { $_.DisplayName -eq "ExplorerPatcher" } |
            Select-Object -First 1

        if ($match) {
            return $match
        }
    }

    return $null
}

function Install-ExplorerPatcher {
    $existing = Get-ExplorerPatcherInstall
    if ($existing) {
        Write-Step "已检测到 ExplorerPatcher：$($existing.DisplayVersion)"
        return
    }

    if ($SkipExplorerPatcherInstall) {
        throw "未检测到 ExplorerPatcher。请去掉 -SkipExplorerPatcherInstall 后重新运行。"
    }

    Write-Step "未检测到 ExplorerPatcher，正在从官方 GitHub Release 下载最新版..."
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

    $release = Invoke-RestMethod -Uri "https://api.github.com/repos/valinet/ExplorerPatcher/releases/latest"
    $asset = $release.assets | Where-Object { $_.name -eq "ep_setup.exe" } | Select-Object -First 1
    if (-not $asset) {
        throw "最新版 Release 中没有找到 ep_setup.exe。"
    }

    $installer = Join-Path $env:TEMP "ep_setup.exe"
    Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $installer

    Write-Step "正在安装 ExplorerPatcher $($release.tag_name)..."
    Start-Process -FilePath $installer -Wait

    $installed = Get-ExplorerPatcherInstall
    if (-not $installed) {
        throw "安装程序已经结束，但没有检测到 ExplorerPatcher 安装项。"
    }
}

function Restart-WindowsShell {
    Write-Step "正在重启开始菜单和资源管理器..."
    Stop-Process -Name StartMenuExperienceHost -Force -ErrorAction SilentlyContinue
    Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 2
    Start-Process explorer.exe
    Start-Sleep -Seconds 5
}

function Test-Win10StyleState {
    $advanced = Get-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced" -ErrorAction SilentlyContinue
    $explorer = Get-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer" -ErrorAction SilentlyContinue
    $ep = Get-ItemProperty -Path "HKCU:\Software\ExplorerPatcher" -ErrorAction SilentlyContinue
    $epExplorer = Get-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher" -ErrorAction SilentlyContinue
    $contextMenuKey = Test-Path "HKCU:\Software\Classes\CLSID\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}\InprocServer32"

    [PSCustomObject]@{
        "已安装ExplorerPatcher" = [bool](Get-ExplorerPatcherInstall)
        "Win10风格任务栏"       = $epExplorer.OldTaskbar
        "任务栏靠左"            = $advanced.TaskbarAl
        "任务栏从不合并"        = $advanced.TaskbarGlomLevel
        "托盘图标全部显示"      = $explorer.EnableAutoTray -eq 0
        "隐藏小组件"            = $advanced.TaskbarDa
        "隐藏聊天按钮"          = $advanced.TaskbarMn
        "Win10磁贴开始菜单"     = $advanced.Start_ShowClassicMode
        "ExplorerPatcher开始菜单样式" = $ep.StartMenuStyle
        "经典右键菜单"          = $contextMenuKey
    }
}

Write-Step "正在应用 Windows 10 风格外壳设置..."
Install-ExplorerPatcher

$advancedPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced"
$epPath = "HKCU:\Software\ExplorerPatcher"
$epStartPath = "HKCU:\Software\ExplorerPatcher\StartMenu"
$epExplorerPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher"

Set-DwordValue -Path $epExplorerPath -Name "OldTaskbar" -Value 1
Set-DwordValue -Path $advancedPath -Name "TaskbarAl" -Value 0
Set-DwordValue -Path $advancedPath -Name "TaskbarGlomLevel" -Value 2
Set-DwordValue -Path $advancedPath -Name "TaskbarDa" -Value 0 -Optional
Set-DwordValue -Path $advancedPath -Name "TaskbarMn" -Value 0 -Optional

Set-DwordValue -Path $advancedPath -Name "Start_ShowClassicMode" -Value 1
Set-DwordValue -Path $epPath -Name "StartMenuStyle" -Value 1
Set-DwordValue -Path $epPath -Name "StartMenuPosition" -Value 1
Set-DwordValue -Path $epPath -Name "StartMenuOpenAtLogon" -Value 0
Set-DwordValue -Path $epPath -Name "StartMenuShowsAllApps" -Value 0
Set-DwordValue -Path $epStartPath -Name "StartMenuStyle" -Value 1
Set-DwordValue -Path $epExplorerPath -Name "StartMenuStyle" -Value 1

Set-ClassicContextMenu
Set-AllTrayIconsVisible

if (-not $NoRestart) {
    Restart-WindowsShell
}

Write-Step "完成。当前状态如下："
Test-Win10StyleState | Format-List

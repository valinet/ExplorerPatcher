#requires -version 5.1
[CmdletBinding()]
param(
    [switch]$NoRestart,
    [switch]$UninstallExplorerPatcher
)

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host "[Win10风格：回退] $Message"
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

function Restart-WindowsShell {
    Write-Step "正在重启开始菜单和资源管理器..."
    Stop-Process -Name StartMenuExperienceHost -Force -ErrorAction SilentlyContinue
    Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 2
    Start-Process explorer.exe
    Start-Sleep -Seconds 5
}

Write-Step "正在恢复本脚本改过的 Windows 11 默认设置..."

$advancedPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced"
$epPath = "HKCU:\Software\ExplorerPatcher"
$epExplorerPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher"
$contextMenuKey = "HKCU:\Software\Classes\CLSID\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}"

Set-DwordValue -Path $epExplorerPath -Name "OldTaskbar" -Value 0
Set-DwordValue -Path $advancedPath -Name "TaskbarAl" -Value 1
Set-DwordValue -Path $advancedPath -Name "TaskbarGlomLevel" -Value 0
Set-DwordValue -Path $advancedPath -Name "TaskbarDa" -Value 1 -Optional
Set-DwordValue -Path $advancedPath -Name "TaskbarMn" -Value 1 -Optional
Set-DwordValue -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer" -Name "EnableAutoTray" -Value 1 -Optional
Set-DwordValue -Path $advancedPath -Name "Start_ShowClassicMode" -Value 0
Set-DwordValue -Path $epPath -Name "StartMenuStyle" -Value 0

if (Test-Path $contextMenuKey) {
    Remove-Item -Path $contextMenuKey -Recurse -Force
}

if ($UninstallExplorerPatcher) {
    $setup = "C:\Program Files\ExplorerPatcher\ep_setup.exe"
    if (Test-Path $setup) {
        Write-Step "正在卸载 ExplorerPatcher..."
        Start-Process -FilePath $setup -ArgumentList "/uninstall" -Wait
    }
}

if (-not $NoRestart) {
    Restart-WindowsShell
}

Write-Step "完成。"

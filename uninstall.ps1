# レジストリを削除
$ErrorActionPreference = "Stop"

$RegPath = "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Netdog"
if (Test-Path $RegPath) {
    Remove-Item -Path $RegPath -Recurse -Force
}

# 環境変数を削除
$CurrentPath = [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::Machine)
$NewPath = $CurrentPath -replace "C:\\Program Files\\Netdog;", ""
$NewPath = $NewPath -replace ";C:\\Program Files\\Netdog", ""
[System.Environment]::SetEnvironmentVariable("Path", $NewPath, [System.EnvironmentVariableTarget]::Machine)

# アプリを削除
$InstallPath = "C:\Program Files\Netdog"
if (Test-Path $InstallPath) {
    Remove-Item -Path $InstallPath -Recurse -Force
}

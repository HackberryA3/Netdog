# レジストリを削除
$RegPath = "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Netdog"
if (Test-Path $RegPath) {
    Remove-Item -Path $RegPath -Recurse -Force
}

# アプリを削除
$InstallPath = "C:\Program Files\Netdog"
if (Test-Path $InstallPath) {
    Remove-Item -Path $InstallPath -Recurse -Force
}

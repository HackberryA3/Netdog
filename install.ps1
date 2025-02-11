# Githubから最新のexeをダウンロードして、Program Filesに配置し、環境変数を設定する

$ErrorActionPreference = "Stop"

if (-not ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
	$ScriptUrl = "https://github.com/HackberryA3/Netdog/releases/latest/download/install.ps1"
	Start-Process powershell -ArgumentList "Invoke-WebRequest -Uri $ScriptUrl | Invoke-Expression" -Verb RunAs -Wait -WindowStyle Hidden
	return 0
}

# ダウンロードするURL
$UrlNetdog = "https://github.com/HackberryA3/Netdog/releases/latest/download/Netdog.exe"
$UrlNc = "https://github.com/HackberryA3/Netdog/releases/latest/download/nc.exe"
$UrlUninstaller = "https://github.com/HackberryA3/Netdog/releases/latest/download/uninstall.ps1"

# ダウンロード先のファイルパス
$InstallPath = "C:\Program Files\Netdog"
$InstallPathNetdog = "$InstallPath\Netdog.exe"
$InstallPathNc = "$InstallPath\nc.exe"
$InstallPathUninstaller = "$InstallPath\uninstall.ps1"

# ディレクトリの作成
New-Item -ItemType Directory -Force -Path "C:\Program Files\Netdog"

# ダウンロード
Invoke-WebRequest -Uri $UrlNetdog -OutFile $InstallPathNetdog
Invoke-WebRequest -Uri $UrlNc -OutFile $InstallPathNc
Invoke-WebRequest -Uri $UrlUninstaller -OutFile $InstallPathUninstaller

# 環境変数の設定
# 既存のPathに追加する値が含まれていない場合のみ追加
$ExistingPath = [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::Machine)
if (-not ($ExistingPath -split ";" -contains $InstallPath)) {
    $UpdatedPath = (@($ExistingPath -split ";") + $InstallPath) -join ";"
    [System.Environment]::SetEnvironmentVariable("Path", $UpdatedPath, [System.EnvironmentVariableTarget]::Machine)
}

# 環墫変数の再読み込み
$env:Path = [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::Machine)

# レジストリ
$Repo = "HackberryA3/Netdog"

$ApiUrl = "https://api.github.com/repos/$Repo/releases/latest"
$Response = Invoke-RestMethod -Uri $ApiUrl
$LatestTag = $Response.tag_name

$ApiUrl = "https://api.github.com/repos/$Repo"
$Response = Invoke-RestMethod -Uri $ApiUrl
$Publisher = $Response.owner.login

$AppName = "Netdog"
$UninstallKey = "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$AppName"
$InstallPath = "$InstallPath"
$UninstallScript = "$InstallPath\uninstall.ps1"
$UninstallExe = "powershell.exe -ExecutionPolicy Bypass -File `"$UninstallScript`""
$IconPath = "$InstallPath\app.ico"

# レジストリキー作成
New-Item -Path $UninstallKey -Force

# 必要な情報を設定
Set-ItemProperty -Path $UninstallKey -Name "DisplayName" -Value $AppName
Set-ItemProperty -Path $UninstallKey -Name "UninstallString" -Value "$UninstallExe"
Set-ItemProperty -Path $UninstallKey -Name "QuietUninstallString" -Value "$UninstallExe -Quiet"
Set-ItemProperty -Path $UninstallKey -Name "DisplayVersion" -Value $LatestTag
Set-ItemProperty -Path $UninstallKey -Name "Publisher" -Value $Publisher
Set-ItemProperty -Path $UninstallKey -Name "InstallLocation" -Value $InstallPath
Set-ItemProperty -Path $UninstallKey -Name "DisplayIcon" -Value $IconPath
Set-ItemProperty -Path $UninstallKey -Name "NoModify" -Value 1
Set-ItemProperty -Path $UninstallKey -Name "NoRepair" -Value 1

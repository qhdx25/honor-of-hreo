@echo off
setlocal

set "APP_NAME=HonorOfHero"
set "APP_DIR=%LOCALAPPDATA%\%APP_NAME%"
set "ZIP_PATH=%~dp0HonorOfHero.zip"

powershell -NoProfile -ExecutionPolicy Bypass -Command "Expand-Archive -LiteralPath '%ZIP_PATH%' -DestinationPath $env:LOCALAPPDATA -Force"
if errorlevel 1 exit /b 1

powershell -NoProfile -ExecutionPolicy Bypass -Command "$desktop=[Environment]::GetFolderPath('Desktop'); $target=Join-Path $env:LOCALAPPDATA 'HonorOfHero\\honor-of-hero.exe'; $working=Join-Path $env:LOCALAPPDATA 'HonorOfHero'; $icon=Join-Path $working 'res\\icon.ico'; if (!(Test-Path $icon)) { $icon=$target }; $shortcut=Join-Path $desktop 'Honor of Hero.lnk'; $shell=New-Object -ComObject WScript.Shell; $link=$shell.CreateShortcut($shortcut); $link.TargetPath=$target; $link.WorkingDirectory=$working; $link.IconLocation=$icon; $link.Save()"
if errorlevel 1 exit /b 1

start "" "%APP_DIR%\honor-of-hero.exe"
exit /b 0

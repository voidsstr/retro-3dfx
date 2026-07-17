@echo off
rem ------------------------------------------------------------------
rem  3dfx Voodoo3 driver install (self-built H5/Napalm, Windows XP/2K)
rem  Safe to run twice: backup is only taken once, all copies use /Y.
rem ------------------------------------------------------------------
setlocal
set PKGDIR=%~dp0
set BACKUP=C:\RETRO_AGENT\3dfx-backup

echo [1/4] Backing up current 3dfx driver files to %BACKUP% ...
if not exist "%BACKUP%" mkdir "%BACKUP%"
if exist "%BACKUP%\backup-done.txt" goto skipbackup
if exist "%SystemRoot%\system32\drivers\3dfxvsm.sys" copy /Y "%SystemRoot%\system32\drivers\3dfxvsm.sys" "%BACKUP%\" >nul
if exist "%SystemRoot%\system32\3dfxvs*.*" copy /Y "%SystemRoot%\system32\3dfxvs*.*" "%BACKUP%\" >nul
if exist "%SystemRoot%\system32\glide3x.dll" copy /Y "%SystemRoot%\system32\glide3x.dll" "%BACKUP%\" >nul
echo backup taken by INSTALL.bat> "%BACKUP%\backup-done.txt"
goto backupdone
:skipbackup
echo        (backup already exists, keeping the original copies)
:backupdone

echo [2/4] Setting driver-signing policy to Ignore ...
reg add "HKLM\Software\Microsoft\Driver Signing" /v Policy /t REG_BINARY /d 00 /f >nul

echo [3/4] Copying glide3x.dll to system32 ...
copy /Y "%PKGDIR%glide3x.dll" "%SystemRoot%\system32\glide3x.dll"

echo [4/4] Installing driver (updrv + voodoo3.inf) ...
"%PKGDIR%updrv.exe" "%PKGDIR%voodoo3.inf" "PCI\VEN_121A&DEV_0005"
set RC=%ERRORLEVEL%
echo.
if "%RC%"=="0" goto ok
if "%RC%"=="2" goto okreboot
echo RESULT: FAILED (updrv exit code %RC%). Driver was NOT installed.
echo Backup of the previous driver is in %BACKUP%.
endlocal
exit /b 1
:ok
echo RESULT: OK - driver installed.
goto done
:okreboot
echo RESULT: OK - driver installed, Windows says a REBOOT IS REQUIRED.
:done
echo.
echo REMINDER: reboot the machine to activate the new display driver.
echo (Do NOT reboot a fleet box without explicit user approval.)
endlocal
exit /b 0

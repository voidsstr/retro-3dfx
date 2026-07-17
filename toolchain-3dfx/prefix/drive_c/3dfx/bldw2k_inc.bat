@echo off
set DEVTOOLS=c:\3dfxtools
call c:\3dfx\H5\W2K\Src\Video\SETENV.BAT %2
if "%BASEDIR%"=="" exit /b 1
set COFFBASE_TXT_FILE=%BASEDIR%\bin\coffbase.txt
set COD_FILES=0
cd /d %1
build -Z
echo BUILDEXIT=%ERRORLEVEL%

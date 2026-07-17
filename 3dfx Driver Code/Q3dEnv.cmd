@echo off
for /f %%d in ('cd') do set BUILD_ROOT=%%d
set BUILD_ROOT_SWLIBS=%BUILD_ROOT%\swlibs
set FX_GLIDE_HW=H5
set FX_HW_PROJECTS=glide3
set FX_TARGET=WIN32
set FX_COMPILER=MICROSOFT
set FX_DLL_BUILD=1

call tools
REM call h5\w2k\src\video\setenv.bat %1
call h5\winnt\src\video\setenv.bat %1
REM call h5\win9x\dx\setenv.bat %1
call setenv.bat %1
if defined NTDEBUG (
    set DEBUG=1 
) else (
    set DEBUG=
)
set DIRECTXSDK=%DEVTOOLS%\dx7asdk
set W2KDDK=%DEVTOOLS%\w2kddk
set W9XDDK=%DEVTOOLS%\Win98Ddk\1999-07
cd %BUILD_ROOT%\%FX_GLIDE_HW%
REM set SRC_ROOT=E:\Src\3dfx\Build
REM set SRCDRV=%SRC_ROOT%

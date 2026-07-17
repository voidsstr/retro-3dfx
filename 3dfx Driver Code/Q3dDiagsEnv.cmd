@echo off
for /f %%d in ('cd') do set BUILD_ROOT=%%d
set BUILD_ROOT_SWLIBS=%BUILD_ROOT%\swlibs
set FX_GLIDE_HW=H5
set FX_HW_PROJECTS=
set FX_TARGET=DOS
set FX_COMPILER=WATCOM
set FX_DLL_BUILD=

call tools
rem call h5\w2k\src\video\setenv %1
call setdosenv %1
if defined NTDEBUG (
    set DEBUG=1 
) else (
    set DEBUG=
)
set DIRECTXSDK=%DEVTOOLS%\dx7asdk
set W2KDDK=%DEVTOOLS%\w2kddk
set W9XDDK=%DEVTOOLS%\Win98Ddk\1998-10
REM cd %BUILD_ROOT%\%FX_GLIDE_HW%
REM set SRC_ROOT=E:\Src\3dfx\Build
REM set SRCDRV=%SRC_ROOT%
set WATCOM=%DEVTOOLS%\watcom

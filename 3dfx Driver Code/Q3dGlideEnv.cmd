@echo off
call h5\win9x\dx\tools.bat
call h5\win9x\dx\setenv.bat

for /f %%d in ('cd') do set BUILD_ROOT=%%d
set BUILD_ROOT_SWLIBS=%BUILD_ROOT%\swlibs
set FX_GLIDE_HW=H5
set FX_HW_PROJECTS=glide3
set FX_TARGET=WIN32
set FX_COMPILER=MICROSOFT
set FX_DLL_BUILD=1

call %FX_GLIDE_HW%\Win9x\dx\tools
call %FX_GLIDE_HW%\Win9x\dx\setenv %1
if defined NTDEBUG (
    set DEBUG=1 
) else (
    set DEBUG=
)
set DIRECTXSDK=%DEVTOOLS%\dx7asdk
set W9XDDK=%DEVTOOLS%\Win98Ddk\1998-10
set W2KDDK=%DEVTOOLS%\W2kDdk
set NTMAKEENV=%DEVTOOLS%\W2kDdk\bin



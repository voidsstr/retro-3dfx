@echo off
REM ------------------------------------------
REM SetEnv.BAT SET UP 3DFX BUILD ENVIRONMENT
REM for Voodoo4 Manufacturing Diags
REM ------------------------------------------

REM ------------------------------------------
REM STORE THE PATH
REM ------------------------------------------
IF NOT "%OLDPATH%"=="" GOTO DONTSTORE
SET OLDPATH=%PATH%
:DONTSTORE


REM ---------------------------------------------------------
REM SET THE FOLLOWING VARIABLES (before attempting to compile
REM ---------------------------------------------------------

REM You must set NMAKE to the directory that contains nmake.exe
REM from MSVC 4.2  (3Dfx always uses nmake for all builds.)
set NMAKE=c:\msvc\bin

REM You must set BUILD_DRIVE to the drive that you are building on ex:
set BUILD_DRIVE=c:

REM You must set BUILD_ROOT to the root of where you extracted your
REM source kit to ex:
set BUILD_ROOT=\3dfx\voodoo3\

REM You must set WATCOM to the location of your Watcom compiler files ex:
set WATCOM=c:\watcom


REM ------------------------------------------
REM CHECK CRITICAL VARS
REM ------------------------------------------
IF NOT "%BUILD_DRIVE%"=="" GOTO DRIVEDEFD
ECHO You must set the environment variable BUILD_DRIVE
ECHO   to the driver letter of the source tree.
ECHO eg. SET BUILD_DRIVE=d:
GOTO END
:DRIVEDEFD
IF NOT "%BUILD_ROOT%"=="" GOTO ROOTDEFD
ECHO You must set the environment variable BUILD_ROOT
ECHO    to the root directory of the source tree.
ECHO eg. SET BUILD_ROOT=\3dfx\devel 
ECHO NOTE: BUILD_ROOT doesn't need a drive letter.
GOTO END
:ROOTDEFD
IF NOT "%WATCOM%"=="" GOTO WATDEFD1
ECHO You must set the environment variable WATCOM
ECHO   to the install directory for Watcom
ECHO   C/C++ 10.6 or greater
ECHO eg. SET WATCOM=C:\devel\WATCOM
GOTO END
:WATDEFD1


REM ------------------------------------------
REM   SET COMMON VARS
REM ------------------------------------------
SET BUILD_ROOT_H5=%BUILD_ROOT%\H5
SET BUILD_ROOT_SWLIBS=%BUILD_ROOT%\swlibs

SET PATH=%WATCOM%\bin95;%WATCOM%\binw;%OLDPATH%
SET INCLUDE=%WATCOM%\H;%WATCOM%\H\NT;%INCLUDE%
REM SET INCLUDE=%INCLUDE%;%BUILD_ROOT_SWLIBS%\include\nmake;%MASM%\INCLUDE

SET LIB=%WATCOM%\LIB386\DOS;%MASM%\lib

SET FX_WATCOM_REG_CALL=
set FX_GLIDE_HW=H5
set FX_NO_GEN_BOF=1
set GLIDE_INIT_HWC=1
set FX_TARGET=DOS
set FX_GLIDE_PACKET_FIFO=1
set FX_INSTALL_DESTINATION=%STAGE_ROOT%\GLIDE\H5\DOS\DOSS\STATIC\ATRI\REL
set FX_COMPILER=WATCOM
set MKS_PATH=c:\DevTools\mks
REM set PATH=%NMAKE%;%WATCOM%\BIN95;%WATCOM%\BINNT;%WATCOM%\BINW;%MASM%\BIN;%PATH%
REM set INCLUDE=%WATCOM%\H;%WATCOM%\H\NT;%MASM%\INCLUDE;%INCLUDE%;%BUILD_ROOT%\3Dfx\include
REM set LIB=%WATCOM%\LIB386\DOS;%MASM%\LIB;%LIB%;%BUILD_ROOT%\3Dfx\lib

:END


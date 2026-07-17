@echo off
@echo.
@echo Setup for Win NT 4 build...
@echo.

REM
REM setup tool and src locations
REM

REM Define the following in Environment
set SRC_ROOT=E:\Src\3dfx\devel
set TOOLS_ROOT=E:\Q3dTools\swstore\netuse


set DDKDRV=%TOOLS_ROOT%
set SRCDRV=%SRC_ROOT%
set VCDRV=%TOOLS_ROOT%
set BUILD_ROOT=%SRC_ROOT%

REM BUILD_ROOT_SWLIBS is set in build.bat files
REM set BUILD_ROOT_SWLIBS=%BUILD_ROOT%\swlibs

REM if "%tools_set%"=="yes" goto end
set tools_set=yes

REM
REM clean up environment
REM

set path=C:\WINDOWS;C:\WINDOWS\COMMAND
if "%OS%" == "Windows_NT" set path=%SystemRoot%\system32;%SystemRoot%
set lib=
rem set include=
set include=%SRCDRV%\h5\incsrc

REM
REM set path to MKS tools
REM

set path=%TOOLS_ROOT%\mks32\6.2\mksnt;%path%

REM
REM set path to NMAKE
REM

set path=%TOOLS_ROOT%\NMAKE\6.00.8168.0\bin;%path%

REM
REM setup path for MASM
REM

set path=%TOOLS_ROOT%\MASM\6.13\BIN;%path%
set include=%TOOLS_ROOT%\MASM\6.13\inc;%include%
set lib=%TOOLS_ROOT%\MASM\6.13\lib;%lib%

REM
REM setup path for MSVC
REM


set MSDevDir=%VCDRV%\msvc32\6.0-sp3-cmdline\Common\MSDev98
set MSVCDir=%VCDRV%\msvc32\6.0-sp3-cmdline\vc98

set path=%MSDevDir%\bin;%MSVCDir%\bin;%path%
set include=%MSVCDir%\include;%MSVCDir%\mfc\include;%include%
set lib=%MSVCDir%\lib;%MSVCDir%\mfc\lib;%lib%

REM
REM setup DDK path
REM

set BASEDIR=%DDKDRV%\WinNtDdk\4.0-97-01
set PATH=%BASEDIR%\bin;%path%
set LIB=%BASEDIR%\lib;%lib%
set INCLUDE=%BASEDIR%\inc;%include%

REM
REM move include files
REM

REM copy %SRCDRV%\h5\incsrc\h3gdefs.h %SRCDRV%\h5\include
REM copy %SRCDRV%\h5\incsrc\h3defs.h  %SRCDRV%\h5\include
REM copy %SRCDRV%\h5\incsrc\h3regs.h  %SRCDRV%\h5\include

REM
REM miscellaneous stuff
REM 

set tmp=c:\temp
set dosx=-swapdir %tmp%
set DDK_VC5_COMPILE=0
set NTMAKEENV=%BASEDIR%\inc
set BUILD_MAKE_PROGRAM=nmake.exe
set BUILD_DEFAULT=-ei -nmake -i

REM
REM set processor type
REM

set PROCESSOR_ARCHITECTURE=x86
set BUILD_DEFAULT_TARGETS=-386
set Cpu=i386
set 386=1

if "%1%"=="" goto free
if "%1%"=="free" goto free
if "%1%"=="checked" goto checked
goto end

:free
set BUILD_ALT_DIR=fre
set DDKBUILDENV=free
set C_DEFINES=-D_IDWBUILD
set NTDBGFILES=1
set NTDEBUG=
set NTDEBUGTYPE=
set MSC_OPTIMIZATION=
goto done

:checked
set BUILD_ALT_DIR=chk
set DDKBUILDENV=checked
set C_DEFINES=-D_IDWBUILD -DRDRDBG -DSRVDBG
set NTDBGFILES=
set NTDEBUG=ntsd
set NTDEBUGTYPE=both
set MSC_OPTIMIZATION=/Od /Oi

:done
set NEW_CRTS=1
set SDK_INC_PATH=%BASEDIR%\inc
set DDK_INC_PATH=%BASEDIR%\inc\ddk
set WDM_INC_PATH=%BASEDIR%\inc\ddk\wdm
set CRT_INC_PATH=%BASEDIR%\inc
set OAK_INC_PATH=%BASEDIR%\inc
set SDK_LIB_DEST=%BASEDIR%\lib%BUILD_ALT_DIR%
set DDK_LIB_DEST=%BASEDIR%\lib%BUILD_ALT_DIR%
set SDK_LIB_PATH=%BASEDIR%\lib%BUILD_ALT_DIR%\*
set DDK_LIB_PATH=%BASEDIR%\lib%BUILD_ALT_DIR%\*
set CRT_LIB_PATH=%BASEDIR%\lib%BUILD_ALT_DIR%\*

:end

set BUILD_ROOT=%SRCDRV%
set COD_FILES=1
set LOCAL_TARGET=1


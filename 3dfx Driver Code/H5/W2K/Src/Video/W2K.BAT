@echo off
REM -----------------------------------------------------------------------------
REM Copyright (c) 1999, 3Dfx Interactive, Inc.
REM All Rights Reserved.
REM 
REM This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
REM the contents of this file may not be disclosed to third parties, copied or
REM duplicated in any form, in whole or in part, without the prior written
REM permission of 3Dfx Interactive, Inc.
REM 
REM RESTRICTED RIGHTS LEGEND:
REM Use, duplication or disclosure by the Government is subject to restrictions
REM as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
REM and Computer Software clause at DFARS 252.227-7013, and/or in similar or
REM successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
REM rights reserved under the Copyright Laws of the United States.
REM 
REM W2K.BAT --- Setup build environment for Napalm W2K display driver.
REM -----------------------------------------------------------------------------

REM
REM setup drive letters
REM

set DDKDRV=d:
set SRCDRV=d:
set VCDRV=d:\devtools

if "%tools_set%"=="yes" goto end
set tools_set=yes

REM
REM clean up environment
REM

set path=D:\NTTOOLS;C:\WINDOWS;C:\WINDOWS\COMMAND
set lib=
set include=

REM
REM setup path for MSVC
REM

set MSDevDir=%VCDRV%\msvc6_0\common\msdev98
set MSVCDir=%VCDRV%\msvc6_0\vc98

set path=%MSDevDir%\bin;%MSVCDir%\bin;%path%
set include=%MSVCDir%\include;%MSVCDir%\mfc\include;%include%
set lib=%MSVCDir%\lib;%MSVCDir%\mfc\lib;%lib%

REM
REM setup DDK path
REM

set BASEDIR=%DDKDRV%\w2kddk
set PATH=%BASEDIR%\bin;%path%
set LIB=%BASEDIR%\lib;%lib%
set INCLUDE=%BASEDIR%\inc;%include%

REM
REM setup path for MASM
REM

set path=%VCDRV%\masm614\bin;%path%
set include=%VCDRV%\masm614\inc;%include%
set lib=%VCDRV%\masm614\lib;%lib%

REM
REM move include files
REM

copy %SRCDRV%\3dfx\devel\h5\incsrc\h3gdefs.h %SRCDRV%\3dfx\devel\h5\include
copy %SRCDRV%\3dfx\devel\h5\incsrc\h3defs.h  %SRCDRV%\3dfx\devel\h5\include
copy %SRCDRV%\3dfx\devel\h5\incsrc\h3regs.h  %SRCDRV%\3dfx\devel\h5\include

REM
REM	miscellaneous stuff
REM 

set tmp=c:\tmp
set dosx=-swapdir %tmp%
set DDK_VC5_COMPILE=0
set NTMAKEENV=%BASEDIR%\bin
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

set BUILD_ROOT=%SRCDRV%\3dfx\devel
set COD_FILES=1
set LOCAL_TARGET=1


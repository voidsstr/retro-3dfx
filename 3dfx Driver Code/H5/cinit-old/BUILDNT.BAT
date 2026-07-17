@echo off

rem Get the files from VSS in
rem     $\devel\swlibs\fxagp
rem                    fxmemmap
rem                    fxmisc
rem                    fxntdrvr
rem                    include\nmake
rem                    newpci\pcilib
rem     $\devel\ape\incsrc
rem                 include
rem                 util
rem                 csim
rem                 hal
rem
rem then setup devel\swlibs\include by copying
rem     swlibs\fxagp\*.h                to swlibs\include
rem     swlibs\fxmemmap\*.h             to swlibs\include
rem     swlibs\fxmisc\3dfx.h            to swlibs\include
rem     swlibs\fxmisc\fx*.h             to swlibs\include
rem     swlibs\fxmisc\tsc.h             to swlibs\include
rem     swlibs\fxntdrvr\*.h             to swlibs\include
rem     swlibs\newpci\pcilib\fxpci.h    to swlibs\include
rem     swlibs\newpci\pcilib\sst1_pci.h to swlibs\include
rem
rem then set up your environment as if you were going to build an NT4
rem driver
rem
rem then change to devel\ape and run this batch file

rem * bat file to build from BUILD_ROOT\h3
rem * assumes that the ddk\dxsdk variables are set.

rem * save old paths
rem set _path=%path%
rem set _lib=%lib%
rem set _include=%include%

rem * set environment  for tools
rem set path=%c32bin%;%c16bin%;%masmbin%;%path%
rem set lib=%c32lib%
rem set include=%c32inc%;%ddkroot%\inc32

rem * set environment  for  csim
rem * BUILD_ROOT is predefined var.  in bat file to be sure
rem set BUILD_ROOT=d:\tdfx\devel
set BUILD_ROOT_SWLIBS=%BUILD_ROOT%\swlibs
set FX_COMPILER=MICROSOFT
set FX_HOST=WIN32
set FX_TARGET=WIN32
set FX_DLL_BUILD=1
set FX_PCI_TARGET=NT
rem set MSDEVDIR=%SDK32INC%\..
rem set DIAG_TOOLS=d:\mksnt
SET KERNEL=1
SET WINICE=1
SET FX_GLIDE_HW=ape

rem build the NT csim library files
@echo on
cd incsrc
nmake
cd ..\util
nmake
cd ..\csim
nmake debug=1 vcdefs="-DGDBG_INFO_ON -DKERNEL -DKERNEL_NT -DWINNT -DNO_FLOAT"   KERNEL=1  %1 %2 %3
cd ..\hal
nmake debug=1 vcdefs="-DGDBG_INFO_ON -DKERNEL -DKERNEL_NT -DWINNT -DNO_FLOAT"   KERNEL=1  %1 %2 %3

rem hal\makefile attempts to create a dll, so trash that and manually
rem create h4hal.lib

attrib -r -s -h h4hal.lib
del h4hal.lib

attrib -r -s -h ..\lib\h4hal.lib
del ..\lib\h4hal.lib
lib /nologo /out:h4hal.lib gdebug.obj fxhal.obj halio.obj info.obj guim.obj guiv.obj util.obj video.obj agpio.obj kernel.obj
copy h4hal.lib ..\lib
attrib +r ..\lib\h4hal.lib

@echo off
rem * gary's makefiles have hard coded h3 as the build directory in the
rem * makefiles.  You need to change it in the line below and in the
rem * makefiles in each directory if you are not building out of
rem * %BUILD_ROOT%\H3.

@echo on

cd ..

@echo off
rem set path=%_path%
rem set lib=%_lib%
rem set include=%_include%


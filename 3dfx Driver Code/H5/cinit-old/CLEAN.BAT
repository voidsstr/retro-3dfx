
@echo off

rem * bat file to build from BUILD_ROOT\h3
rem * assumes that the ddk\dxsdk variables are set.

rem * save old paths
set _path=%path%
set _lib=%lib%
set _include=%include%

rem * set environment  for tools
set path=%c32bin%;%c16bin%;%masmbin%;%path%
set lib=%c32lib%
set include=%c32inc%

rem * set environment  for  csim
rem * BUILD_ROOT is predefined var.  in bat file to be sure
set BUILD_ROOT=d:\tdfx\devel
set BUILD_ROOT_SWLIBS=%BUILD_ROOT%\swlibs
set FX_COMPILER=MICROSOFT
set FX_HOST=WIN32
set FX_TARGET=WIN32
set FX_DLL_BUILD=1
set FX_PCI_TARGET=NT
set MSDEVDIR=%SDK32INC%\..
SET KERNEL=1
SET WINICE=1

@echo on
cd incsrc
nmake clean > out
cd ..\util
nmake clean > out
cd ..\csim
nmake clean > out
cd ..\hal
nmake clean > out
del h3hal.lib


@echo on

cd ..

@echo off
set path=%_path%
set lib=%_lib%
set include=%_include%

@echo off

set _include_=%include%
set _lib_=%lib%
set _path_=%path%

set include=c:\dxddk50\inc;c:\msdev\include;%include%;c:\ddk95\inc32
rem set include=c:\msdev\include;%include%;c:\ddk95\inc32
set lib=c:\msdev\lib;%include%;c:\ddk95\lib;
set path=c:\msdev\bin;%path%;c:\masm611\bin

rem set include=c:\msdev\include;%include%;c:\ddk98\inc\memphis\inc
rem set lib=c:\msdev\lib;%include%;c:\ddk98\lib\i386\free\memphis
rem set path=c:\msdev\bin;%path%;c:\masm611\bin

@echo on

nmake %1 %2 %3 %4

@echo off

set include=%_include_%
set lib=%_lib_%
set path=%_path_%

@echo on

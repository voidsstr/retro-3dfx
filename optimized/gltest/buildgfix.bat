@echo off
set INCLUDE=C:\3dfxtools\msvc6_0\vc98\Include
set LIB=C:\3dfxtools\msvc6_0\vc98\Lib
set PATH=C:\3dfxtools\msvc6_0\vc98\Bin;C:\3dfxtools\msvc6_0\common\msdev98\Bin;%PATH%
cd /d C:\3dfx\gltest
cl /nologo /O2 gfix.c /link opengl32.lib gdi32.lib user32.lib
echo CL_EXIT=%ERRORLEVEL%

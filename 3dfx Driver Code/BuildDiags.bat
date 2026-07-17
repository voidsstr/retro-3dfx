@echo off
setlocal

call q3ddiagsenv.cmd

if {%1} == {} goto usage
if /i %1 == build goto build
if /i %1 == clean goto clean
goto usage

:build
mkdir %FX_GLIDE_HW%\diags\mftg\3dfx\include
mkdir %FX_GLIDE_HW%\diags\mftg\3dfx\lib

cd swlibs
nmake
cd ..

copy swlibs\include\*.h %FX_GLIDE_HW%\diags\mftg\3dfx\include
copy swlibs\lib\*.lib %FX_GLIDE_HW%\diags\mftg\3dfx\lib

cd %FX_GLIDE_HW%

cd incsrc
nmake
cd ..

cd cinit
nmake
cd ..

cd minihwc
nmake
cd ..

cd glide
nmake
cd ..

copy include\*.h diags\mftg\3dfx\include
copy lib\*.lib diags\mftg\3dfx\lib

cd diags\mftg
build
goto done

:clean
del /f /q %FX_GLIDE_HW%\diags\mftg\3dfx\include\*.*
del /f /q %FX_GLIDE_HW%\diags\mftg\3dfx\lib\*.*

cd swlibs
nmake clean
cd ..

cd %FX_GLIDE_HW%

cd incsrc
nmake clean
cd ..

cd cinit
nmake clean
cd ..

cd minihwc
nmake clean
cd ..

cd glide
nmake clean
cd ..

cd diags\mftg
clean
goto done

:usage
echo Usage: BuildDiags Build ^| Clean
goto done

:done
endlocal

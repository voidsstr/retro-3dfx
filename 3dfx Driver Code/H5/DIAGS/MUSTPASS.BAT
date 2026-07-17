@echo off
REM This test script runs all the tests in the subdirectories
@echo on
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
cd fbi
call mustpass.bat	%EXFLAGS%
cd ..
cd trex
call mustpass.bat	%EXFLAGS%
cd ..
cd gui
call mustpass.bat	%EXFLAGS%
cd ..
awk -f testsum.awk */*.log

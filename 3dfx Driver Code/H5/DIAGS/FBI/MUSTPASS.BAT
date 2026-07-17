@echo off
REM This test script runs all the tests in a reasonable order, starting with
REM simple tests and working up to more complex tests.  Some of the tests
REM are run multiple times, each time with different parameters.  For example,
REM some tests are run again with the "-a" option to turn on sub-pixel
REM parameter adjustments.  In these cases, a different random seed is used
REM to mix things up further.
REM 
REM
@echo on
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
call simple.bat		%EXFLAGS%
call misc.bat		%EXFLAGS%
call triangle.bat	%EXFLAGS%
call stipzw.bat		%EXFLAGS%
call fog.bat		%EXFLAGS%
call alpha.bat		%EXFLAGS%
call stress.bat		%EXFLAGS%
call lfb.bat		%EXFLAGS%
call tsetup.bat		%EXFLAGS%
call cmdfifo.bat	%EXFLAGS%
call agp.bat            %EXFLAGS%

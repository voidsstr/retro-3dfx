@echo off
REM This test script runs all the tests in a reasonable order, starting with
REM simple tests and working up to more complex tests.  Some of the tests
REM are run multiple times, each time with different parameters.  For example,
REM some tests are run again with the "-a" option to turn on sub-pixel
REM parameter adjustments, or the "-b" option to turn on bilinear filtering.
REM In these cases, a different random seed is used to mix things up further.
REM 
REM
@echo on
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
call easy.bat		%EXFLAGS%
call rectangl.bat	%EXFLAGS%
call ncc.bat		%EXFLAGS%
call persp.bat		%EXFLAGS%
call lod.bat		%EXFLAGS%
call misc.bat		%EXFLAGS%
call bigt.bat		%EXFLAGS%
call cmdfifo.bat	%EXFLAGS%
call agp.bat	%EXFLAGS%


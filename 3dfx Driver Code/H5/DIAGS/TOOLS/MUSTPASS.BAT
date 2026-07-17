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
set TRACE=\\samba\tarolli\trace\sst1
REM
REM
REM non-textured diags
REM 492 triangles, 44294 pixels
replay  -A              -I %TRACE%\alines.trace         %EXFLAGS%
REM 4134 triangles, 24131 pixels
replay  -A              -I %TRACE%\canstick.trace       %EXFLAGS%
REM 1538 triangles, 12241 pixels
replay  -A      -Z      -I %TRACE%\spheres1.trace       %EXFLAGS%
REM 598 triangles, 46944 pixels
replay  -A      -Z      -I %TRACE%\wasp_gs.trace        %EXFLAGS%
REM 671 triangles, 392138 pixels
replay  -A      -Z      -I %TRACE%\wasp_fog.trace       %EXFLAGS%
REM
REM ome LFB writes thru the pixel pipe (3Dfx shameless plug)
replay  -A -O 0x400000  -I %TRACE%\lfb.trace            %EXFLAGS%
REM
REM textured diags
REM 36 triangles, 203 490 pixels
replay  -A              -I %TRACE%\trip1.trace          %EXFLAGS%
REM 8 triangles, 22732 pixels
replay  -A -T           -I %TRACE%\movet6.trace         %EXFLAGS%
REM 5427 triangles, 528831 pixels
replay  -A -Z           -I %TRACE%\fight32.trace        %EXFLAGS%
REM 3682 triangles, 1691824 pixels
replay  -A -Z           -I %TRACE%\wiz2.trace           %EXFLAGS%
REM 1806 triangles, 635194 pixels
replay  -A -Z           -I %TRACE%\mango2.trace         %EXFLAGS%
REM 671 triangles, 392138 pixels
replay  -A -Z           -I %TRACE%\wasp.trace           %EXFLAGS%
REM 1000 triangles, 491781 pixels
replay  -A -Z -w 800    -I %TRACE%\wendel1.trace        %EXFLAGS%
REM 10320 triangles, 116189 pixels
replay  -A -Z -w 800    -I %TRACE%\min1.trace           %EXFLAGS%
REM 102 triangles, 780978 pixels
replay  -A              -I %TRACE%\fly40.trace          %EXFLAGS%
REM 162 triangles, 630016 pixels
replay  -A              -I %TRACE%\fly60.trace          %EXFLAGS%

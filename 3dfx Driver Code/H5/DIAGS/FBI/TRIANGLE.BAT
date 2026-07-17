REM simple to complex triangle tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
triangle			%EXFLAGS%
lines				%EXFLAGS%
lines	 -t-20	-l lines_1	%EXFLAGS%
lines1				%EXFLAGS%
lines1	 -t-20	-l lines1_1	%EXFLAGS%
rgbtri				%EXFLAGS%
rgbtri	 -t-3	-l rgbtri4	%EXFLAGS%
rgbtri	 -as99	-l rgbtri5	%EXFLAGS%
rgbtri1				%EXFLAGS%
rgbtri1	 -t-2	-l rgbtri2	%EXFLAGS%
rgbtri1	 -as98	-l rgbtri3	%EXFLAGS%
rgbatri				%EXFLAGS%
rgbatri	 -t-10	-l rgbatri1	%EXFLAGS%
rgbatri	 -as97	-l rgbatri2	%EXFLAGS%
ztri				%EXFLAGS%
ztri	 -as96	-l ztri1	%EXFLAGS%
rgbztri				%EXFLAGS%
rgbztri	 -as95	-l rgbztri1	%EXFLAGS%

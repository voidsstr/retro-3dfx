REM stipple and z and w tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
stipple	 %EXFLAGS%
zbias	 %EXFLAGS%
zclamp	 %EXFLAGS%
zfunc	 %EXFLAGS%
ziter	 %EXFLAGS%
zmask	 %EXFLAGS%
wbuffer	 %EXFLAGS%
wbias	 %EXFLAGS%
wclamp	 %EXFLAGS%
zfbuffer %EXFLAGS%
REM stencil tests
stmask		%EXFLAGS%
stfunc		%EXFLAGS%
stop		%EXFLAGS%

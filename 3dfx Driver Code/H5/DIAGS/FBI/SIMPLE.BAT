REM simple tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
register %EXFLAGS%
ioreg	 %EXFLAGS%
chip	 %EXFLAGS%
chip -Z  -l chip1 %EXFLAGS%
chip -T  -l chip2 %EXFLAGS%
nop	 %EXFLAGS%
nop1	 %EXFLAGS%
point	 %EXFLAGS%
dither2	 %EXFLAGS%
dither4	 %EXFLAGS%
chroma	 %EXFLAGS%
rgbclamp %EXFLAGS%
rgbiter	 %EXFLAGS%
rgbmask  %EXFLAGS%

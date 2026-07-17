REM ncc textures, many permutations
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
fmt           -B	   -n	-s 500	-l fmt_14 %EXFLAGS%
fmt	   -a -Bc	-r -n	-s 501	-l fmt_15 %EXFLAGS%
fmt	-8 -a -B	-r -n	-s 502	-l fmt_16 %EXFLAGS%
fmt	-8	   -Bc	-r -n	-s 503	-l fmt_17 %EXFLAGS%

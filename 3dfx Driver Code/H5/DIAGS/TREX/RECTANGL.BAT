REM rectangular textures, many permutations
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
fmt	      -B	-r	-s 400	-l fmt_20 %EXFLAGS%
fmt	   -a -Bc	-r	-s 401	-l fmt_21 %EXFLAGS%
fmt	-8 -a -B	-r	-s 402	-l fmt_22 %EXFLAGS%
fmt	-8    -Bc	-r	-s 403	-l fmt_23 %EXFLAGS%

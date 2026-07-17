REM BIG triangle tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
simple					-l simple_4b -t 40 %EXFLAGS%
fmt	-8 -a -B	-r	-s 402	-l fmt_12b -t  60 %EXFLAGS%
fmt	   -a -Bc	-r -n	-s 501	-l fmt_15b -t  40 %EXFLAGS%
lod	-8	-a	  -n -P -s 605	-l lod_3b  -t 120 %EXFLAGS%
trilin     -z-1 -a -BZ  -m  -P -s 704  -l tri_4b  -t 130 %EXFLAGS%
trilin  -8 -z-1 -a -BZ  -m  -P -s 705  -l tri_5b  -t  85 %EXFLAGS%
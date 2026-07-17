REM perspective tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
simple	    -a		     -P -s 99	-l simple_3 %EXFLAGS%
simple			     -P -s 98	-l simple_4 %EXFLAGS%
fmt	 -8 -a	-Bc	-r   -P	-s 600	-l fmt_100  %EXFLAGS%
fmt	    -a	-Bc	-r   -P	-s 601	-l fmt_101  %EXFLAGS%
fmt		-Bc	-r   -P	-s 602	-l fmt_102  %EXFLAGS%
floatstw 					    %EXFLAGS%
floatstw    -a	   	     -P -s 602	-l float_1  %EXFLAGS%
floatstw	-Bc	     -P -s 603	-l float_2  %EXFLAGS%
floatstw	-Bc		-s 604	-l float_3  %EXFLAGS%

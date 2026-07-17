REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
stress2d -xd0		-l strs_1			%EXFLAGS%
stress2d -xd1	-s2	-l strs_2			%EXFLAGS%
stress2d -xd2	-s3	-l strs_3			%EXFLAGS%
stress2d -xd3	-s4	-l strs_4			%EXFLAGS%
stress2d -p10	-s5	-l strs_5			%EXFLAGS%
stress2d -p100	-s6	-l strs_6			%EXFLAGS%
stress2d -p10   -s7     -l strs_7 -xtfw=1 	 	%EXFLAGS%
stress2d -p10   -s8     -l strs_8 -xtmw=1 	 	%EXFLAGS%
stress2d -p10   -s9     -l strs_9 -xonscreen		%EXFLAGS%

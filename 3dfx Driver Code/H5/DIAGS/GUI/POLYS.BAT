REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
poly						%EXFLAGS%
poly		-t 2	-s1	-l poly_1	%EXFLAGS%
poly		-t 7	-s2	-l poly_2	%EXFLAGS%
poly	-O 15	-t 10	-s3	-l poly_3	%EXFLAGS%
poly	-O 20	-t 15	-s4	-l poly_4	%EXFLAGS%
poly	-O 32	-t 20	-s5	-l poly_5	%EXFLAGS%

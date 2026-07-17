REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
linerev		-t 10				%EXFLAGS%
linerev		-t 100	-s 2	-l liner_1	%EXFLAGS%
gline						%EXFLAGS%
gline		-O 1	-s 3	-l line_1	%EXFLAGS%
gline		-O 2	-s 4	-l line_2	%EXFLAGS%
gline		-O 3	-s 5	-l line_3	%EXFLAGS%
gline		-O 4	-s 6	-l line_4	%EXFLAGS%
gline		-O 6	-s 8	-l line_6	%EXFLAGS%

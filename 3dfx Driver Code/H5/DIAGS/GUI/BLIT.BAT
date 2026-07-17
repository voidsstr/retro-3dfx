REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
blt				-t 10			%EXFLAGS%
blt	-P		-LBm	-t 4   	-s 2 -l blt_1	%EXFLAGS%
blt	-T		-LBmU	-t 5	-s 3 -l blt_2	%EXFLAGS%
blt	-PT		-LBm	-t 6	-s 4 -l blt_3	%EXFLAGS%
blt	-M		-LBmU	-t 7	-s 5 -l blt_4	%EXFLAGS%
blt	-MPTr		-LBm	-t 8	-s 6 -l blt_5	%EXFLAGS%
blt	-MPTkK		-LBmU	-t 9	-s 7 -l blt_6	%EXFLAGS%
blt	-MPTkKcr	-LBmU	-t 3	-s 8 -l blt_7	%EXFLAGS%
blt	-MPTkKcr	-LBmU	-t 32	-s 9 -l blt_8	%EXFLAGS%
blt	-MPTkKcr	-LBmU	-t 128	-s 2 -l blt_9	%EXFLAGS%
blt	-MPTkKcr	-LBmU	-t 600	-s 10 -l blt_10 -xh3	%EXFLAGS%


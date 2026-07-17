REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
hblt							%EXFLAGS%
hblt	-MPTkKcr	-LbBmU	-t 10	-s 2 -l hblt_1	%EXFLAGS%
hblt	-MPTkKcr	-LbBmU	-t 20	-s 3 -l hblt_2	%EXFLAGS%
hblt	-MPTkKcr	-LbBmU	-t 128	-s 4 -l hblt_3	%EXFLAGS%
hblt	-MPTkKcr	-LbBmU	-t 600	-s 5 -l hblt_4 -xh3	%EXFLAGS%
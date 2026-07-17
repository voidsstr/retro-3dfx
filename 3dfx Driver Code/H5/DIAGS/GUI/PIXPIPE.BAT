REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
rect			     -t 4			%EXFLAGS%
rect	-P		-LBm -t 2	-s 2 -l rop_1	%EXFLAGS%
rect			-LBm -t 3	-s 3 -l rop_2	%EXFLAGS%
rect	-T		-LBm -t 4	-s 4 -l rop_3	%EXFLAGS%
rect	-M		-LBm -t 5	-s 5 -l rop_4	%EXFLAGS%
rect	-MPTr		-LBm -t 13	-s 6 -l rop_5	%EXFLAGS%
rect	-MPTkK		-LBm -t 14	-s 7 -l rop_6	%EXFLAGS%
rect	-MPTkKcr	-LBm -t 15	-s 8 -l rop_7	%EXFLAGS%
rect	-MPTkKcr	-LBm -t 17	-s 9 -l rop_8	%EXFLAGS%

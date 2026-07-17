REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
lstip							%EXFLAGS%
lstip			-t 100	-s 2	-l lstip_1	%EXFLAGS%
lstip	-T			-s 3	-l lstip_2	%EXFLAGS%
lstip	-T		-t 200	-s 4	-l lstip_3	%EXFLAGS%
lstip	-PTkKcr  -LBm	-t 13	-s 8	-l lstip_4	%EXFLAGS%
lstip	-MPTkKcr -LBm	-t 17	-s 9	-l lstip_5	%EXFLAGS%

REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9

sblt	-MPTkKcr	-LBmU	-l sblt_10 -s1 -t10 %EXFLAGS%
sblt	-MPTkKcr	-LBmU	-l sblt_11 -s1 -t32 %EXFLAGS%
sblt	-MPTkKcr	-LBmU	-l sblt_12 -s1 -t128 %EXFLAGS%

sblt	-MPTkKcr	-LBmU	-l sblt_1 -s1 -u1	%EXFLAGS%
sblt	-MPTkKcr	-LBmU	-l sblt_2 -s2 -u3	%EXFLAGS%
sblt	-MPTkKcr	-LBmU	-l sblt_3 -s3 -u4	%EXFLAGS%
sblt	-MPTkKcr	-LBmU	-l sblt_4 -s4 -u5	%EXFLAGS%

sblt	-MPTkKcr	-LBmU	-l sblt_5 -s5 -u3 -y1	%EXFLAGS%
sblt	-MPTkKcr	-LBmU	-l sblt_6 -s6 -u4 -y2	%EXFLAGS%
sblt	-MPTkKcr	-LBmU	-l sblt_7 -s7 -u5 -y3	%EXFLAGS%

sblt	-MPTkKcr	-LBmU	-t 600	  -s8 -l sblt_8 -xh3	%EXFLAGS%

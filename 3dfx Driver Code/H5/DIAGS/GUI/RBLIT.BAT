REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9

REM these test random source format and strides
rblt				-t 10			%EXFLAGS%
rblt	-P		-LBm	-t 4   	-s 2 -l rblt_1	%EXFLAGS%
rblt	-T		-LBmU	-t 5	-s 3 -l rblt_2	%EXFLAGS%
rblt	-PT		-LBm	-t 6	-s 4 -l rblt_3	%EXFLAGS%
rblt	-M		-LBmU	-t 7	-s 5 -l rblt_4	%EXFLAGS%
rblt	-MPTr		-LBm	-t 8	-s 6 -l rblt_5	%EXFLAGS%
rblt	-MPTkK		-LBmU	-t 9	-s 7 -l rblt_6	%EXFLAGS%
rblt	-MPTkKcr	-LBmU	-t 3	-s 8 -l rblt_7	%EXFLAGS%
rblt	-MPTkKcr	-LBmU	-t 32	-s 9 -l rblt_8	%EXFLAGS%
rblt	-MPTkKcr	-LBmU	-t 128	-s 2 -l rblt_9	%EXFLAGS%
rblt	-MPTkKcr	-LBmU	-t 10	-s 10 -l rblt_10 -y1 %EXFLAGS%
rblt	-MPTkKcr	-LBmU	-t 10	-s 11 -l rblt_11 -y2 %EXFLAGS%
rblt	-MPTkKcr	-LBmU	-t 10	-s 12 -l rblt_12 -y3 %EXFLAGS%
rblt	-MPTkKcr	-LBmU	-t 600	-s 13 -l rblt_13 -xh3	%EXFLAGS%

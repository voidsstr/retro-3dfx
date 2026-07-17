@echo off
REM This test script runs all the tests in a reasonable order, starting with
REM simple tests and working up to more complex tests.  Some of the tests
REM are run multiple times, each time with different parameters.  For example,
REM some tests are run again with the "-a" option to turn on sub-pixel
REM parameter adjustments.  In these cases, a different random seed is used
REM to mix things up further.
REM 
REM
@echo on
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
guinop						%EXFLAGS%
greg						%EXFLAGS%
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
REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
lstip							%EXFLAGS%
lstip			-t 100	-s 2	-l lstip_1	%EXFLAGS%
lstip	-T			-s 3	-l lstip_2	%EXFLAGS%
lstip	-T		-t 200	-s 4	-l lstip_3	%EXFLAGS%
lstip	-PTkKcr  -LBm	-t 13	-s 8	-l lstip_4	%EXFLAGS%
lstip	-MPTkKcr -LBm	-t 17	-s 9	-l lstip_5	%EXFLAGS%
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
REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
hblt							%EXFLAGS%
hblt	-MPTkKcr	-LbBmU	-t 10	-s 2 -l hblt_1	%EXFLAGS%
hblt	-MPTkKcr	-LbBmU	-t 20	-s 3 -l hblt_2	%EXFLAGS%
hblt	-MPTkKcr	-LbBmU	-t 128	-s 4 -l hblt_3	%EXFLAGS%
hblt	-MPTkKcr	-LbBmU	-t 600	-s 5 -l hblt_4 -xh3 %EXFLAGS%
REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
poly						%EXFLAGS%
poly		-t 2	-s1	-l poly_1	%EXFLAGS%
poly		-t 7	-s2	-l poly_2	%EXFLAGS%
poly	-O 15	-t 10	-s3	-l poly_3	%EXFLAGS%
poly	-O 20	-t 15	-s4	-l poly_4	%EXFLAGS%
poly	-O 32	-t 20	-s5	-l poly_5	%EXFLAGS%
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
randpoly -MPTkKcrLBm -t 10	-l rpoly_1 -s1 -xd0	%EXFLAGS%
randpoly -MPTkKcrLBm -t 10	-l rpoly_2 -s2 -xd1	%EXFLAGS%
randpoly -MPTkKcrLBm -t 10	-l rpoly_3 -s3 -xd2	%EXFLAGS%
randpoly -MPTkKcrLBm -t 10	-l rpoly_4 -s4 -xd3	%EXFLAGS%
randpoly -MPTkKcrLBm -t 10	-l rpoly_5 -s5 -xd3	-y1 %EXFLAGS%
randpoly -MPTkKcrLBm -t 10	-l rpoly_6 -s6 -xd4	-y1 %EXFLAGS%
poly2	-p4			-l poly2_1 -s1 -xd0	%EXFLAGS%
poly2	-p4			-l poly2_2 -s2 -xd1	%EXFLAGS%
poly2	-p4			-l poly2_3 -s3 -xd2	%EXFLAGS%
poly2	-p4			-l poly2_4 -s4 -xd3	%EXFLAGS%
poly2	-p4	-t10		-l poly2_5 -s1 -xd0	%EXFLAGS%
poly2	-p4	-t10		-l poly2_6 -s2 -xd1	%EXFLAGS%
poly2	-p4	-t10		-l poly2_7 -s3 -xd2	%EXFLAGS%
poly2	-p4	-t10		-l poly2_8 -s4 -xd3	%EXFLAGS%
poly3   -p4			-l poly3_1 -s1 -xd0	%EXFLAGS%
poly3   -p4			-l poly3_2 -s2 -xd1	%EXFLAGS%
poly3   -p4			-l poly3_3 -s3 -xd2	%EXFLAGS%
poly3   -p4			-l poly3_4 -s4 -xd3	%EXFLAGS%
poly3		-t100		-l poly3_5 -s5 -xd3	%EXFLAGS%
REM kens directed tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
align							%EXFLAGS%
hbltalgn						%EXFLAGS%
hbltalgn -y1 -l hbltalgn_tiled				%EXFLAGS%
clip1							%EXFLAGS%
REM FAILS clip2							%EXFLAGS%
clip3 -p3						%EXFLAGS%
block	 						%EXFLAGS%
bgblock1 						%EXFLAGS%
bgblock2 -xcm 						%EXFLAGS%
destread -p50 						%EXFLAGS%
pattern	 						%EXFLAGS%
scr2scr	 						%EXFLAGS%
updatexy 						%EXFLAGS%
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
REM command fifo tests 
REM -h disableHoles, issues bump after writes and immediately executes them
REM -N-# out-of-order writes
REM
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
REM directed tests
cmd1	-W4 -h			-s 1		-l cmd1_h	%EXFLAGS%
cmd1	-W4 -h	-N 131		-s 2		-l cmd1_h1	%EXFLAGS%
cmd1	-W4 -h	-N 129	-g1	-s 3		-l cmd1_h2	%EXFLAGS%
cmd1	-W4 -h	-N 256		-s 4		-l cmd1_h3	%EXFLAGS%
cmd1	-W4 -h	-N 1200	-g1	-s 5		-l cmd1_h4	%EXFLAGS%
cmd1	-W4			-s 1				%EXFLAGS%
cmd1	-W4	-N 131		-s 2		-l cmd1_1	%EXFLAGS%
cmd1	-W4	-N 129		-s 3		-l cmd1_2	%EXFLAGS%
cmd1	-W4	-N 256	-g1	-s 4		-l cmd1_3	%EXFLAGS%
cmd1	-W4	-N 1200		-s 5		-l cmd1_5	%EXFLAGS%
cmd1	-W4 	-N 2199		-s 6		-l cmd1_6	%EXFLAGS%
REM jsr tests, note out-of-order writes are forced by the diag
jsr	-W4	-N175		-s1				%EXFLAGS%
jsr	-W4 -h	-N176	-g1	-s2		-l jsr_1	%EXFLAGS%
jsr	-W4	-N177	-g1	-s3		-l jsr_2	%EXFLAGS%
jsr	-W4 -h	-N152		-s4		-l jsr_3	%EXFLAGS%
jsr	-W4 	-N2152		-s5		-l jsr_4	%EXFLAGS%
REM rects
rect	-W4 -h		-g1 -MPTkKcr -LBm	-l rect_1_cmd	%EXFLAGS%
rect	-W4 -hN-133	    -MPTkKcr -LBm	-l rect_2_cmd	%EXFLAGS%
rect	-W4		-g1 -MPTkKcr -LBm	-l rect_3_cmd	%EXFLAGS%
rect	-W4 -N-134	    -MPTkKcr -LBm	-l rect_4_cmd	%EXFLAGS%
REM blits with random source base and strides
blt	-W4		    -MPTkKcr -LBmU -s 3	-l blt_1_cmd	%EXFLAGS%
blt	-W4 -N-129	-g1 -MPTkKcr -LBmU -s 6	-l blt_2_cmd	%EXFLAGS%
rblt	-W4 -hN-175	    -MPTkKcr -LBmU -s 4	-l rblt_1_cmd	%EXFLAGS%
rblt	-W4		-g1 -MPTkKcr -LBmU -s 5	-l rblt_2_cmd	%EXFLAGS%
sblt	-W4		-g1 -MPTkKcr -LBmU -s 5	-l sblt_2_cmd	%EXFLAGS%
REM host blits
hblt	-W4 -h -N129	    -MPTkKcr -LbBmU -s 5 -l hblt_1_cmd	%EXFLAGS%
hblt	-W4 -h -N130	-g1 -MPTkKcr -LbBmU -s 6 -l hblt_2_cmd	%EXFLAGS%
hblt	-W4 -h -N-132	-g1 -MPTkKcr -LbBmU -s 7 -l hblt_3_cmd	%EXFLAGS%
hblt	-W4    -N129	    -MPTkKcr -LbBmU -s 8 -l hblt_4_cmd	%EXFLAGS%
hblt	-W4    -N-137	-g1 -MPTkKcr -LbBmU -s 9 -l hblt_5_cmd	%EXFLAGS%
REM lines and polys
lstip	-W4 -N-140 	-T	-t 11	-s30	-l lstip_1_cmd	%EXFLAGS%
lstip	-W4 -N-141	-PTkKcr	-LBm	-s31	-l lstip_2_cmd	%EXFLAGS%
poly	-W4 -N-142			-s32	-l poly_1_cmd	%EXFLAGS%
poly	-W4 -N-2001	-O 32		-s33	-l poly_2_cmd	%EXFLAGS%
REM stress tests
gst1	-W4	-N 140	-D -O99	-t 14	-s 3	-l gst1_1_cmd	%EXFLAGS%
gst1	-W4 -h 	-N 141	-D -O99	-t 14	-s 4	-l gst1_2_cmd	%EXFLAGS%
gst1	-W4 -h	-N-129	-D -O99	-t 14	-s 5	-l gst1_3_cmd	%EXFLAGS%
gst1	-W4	-N-130	-D -O99	-t 14	-s 6	-l gst1_4_cmd	%EXFLAGS%

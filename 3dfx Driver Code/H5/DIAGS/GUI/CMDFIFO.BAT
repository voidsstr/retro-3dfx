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

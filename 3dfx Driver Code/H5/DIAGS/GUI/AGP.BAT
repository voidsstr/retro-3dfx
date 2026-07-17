REM command fifo tests 
REM -h disableHoles, issues bump after writes and immediately executes them
REM -N-# out-of-order writes
REM
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
REM rects
rect	-W14 -h		-g1 -MPTkKcr -LBm	-l rect_1_cmdagp	%EXFLAGS%
rect	-W14 -h -N-133	    -MPTkKcr -LBm	-l rect_2_cmdagp	%EXFLAGS%
rect	-W14	-h	-g1 -MPTkKcr -LBm	-l rect_3_cmdagp	%EXFLAGS%
rect	-W14 -h -N-134	    -MPTkKcr -LBm	-l rect_4_cmdagp	%EXFLAGS%
REM blits with random source base and strides
blt	-W14	-h 	    -MPTkKcr -LBmU -s 3	-l blt_1_cmdagp	%EXFLAGS%
blt	-W14 -h -N-129	-g1 -MPTkKcr -LBmU -s 6	-l blt_2_cmdagp	%EXFLAGS%
rblt	-W14 -h -N-175	    -MPTkKcr -LBmU -s 4	-l rblt_1_cmdagp	%EXFLAGS%
sblt	-W14 -h -N-175	    -MPTkKcr -LBmU -s 4	-l sblt_1_cmdagp	%EXFLAGS%
rblt	-W14 -h		-g1 -MPTkKcr -LBmU -s 5	-l rblt_2_cmdagp	%EXFLAGS%
REM host blits
hblt	-W14 -h -N129	    -MPTkKcr -LbBmU -s 5 -l hblt_1_cmdagp	%EXFLAGS%
hblt	-W14 -h -N130	-g1 -MPTkKcr -LbBmU -s 6 -l hblt_2_cmdagp	%EXFLAGS%
hblt	-W14 -h -N-132	-g1 -MPTkKcr -LbBmU -s 7 -l hblt_3_cmdagp	%EXFLAGS%
hblt	-W14 -h  -N129	    -MPTkKcr -LbBmU -s 8 -l hblt_4_cmdagp	%EXFLAGS%
hblt	-W14 -h  -N-137	-g1 -MPTkKcr -LbBmU -s 9 -l hblt_5_cmdagp	%EXFLAGS%
REM lines and polys
lstip	-W14 -h -N-140 	-T	-t 11	-s30	-l lstip_1_cmdagp	%EXFLAGS%
lstip	-W14 -h -N-141	-PTkKcr	-LBm	-s31	-l lstip_2_cmdagp	%EXFLAGS%
poly	-W14 -h -N-142			-s32	-l poly_1_cmdagp	%EXFLAGS%
poly	-W14 -h -N-2001	-O 32		-s33	-l poly_2_cmdagp	%EXFLAGS%
REM stress tests
gst1	-W14	-h -N 140	-D -O99	-t 14	-s 3	-l gst1_1_cmdagp	%EXFLAGS%
gst1	-W14 -h 	-N 141	-D -O99	-t 14	-s 4	-l gst1_2_cmdagp	%EXFLAGS%
gst1	-W14 -h	-N-129	-D -O99	-t 14	-s 5	-l gst1_3_cmdagp	%EXFLAGS%
gst1	-W14	-h -N-130	-D -O99	-t 14	-s 6	-l gst1_4_cmdagp	%EXFLAGS%

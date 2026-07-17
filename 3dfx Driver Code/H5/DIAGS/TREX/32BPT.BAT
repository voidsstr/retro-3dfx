REM This way stolen from easy.bat
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
texmem	-t2	--32bpt			-l texmem_32bpt	%EXFLAGS%
texmem	-t20	--32bpt			-l texmem_1_32bpt	%EXFLAGS%
texmem	-t200	--32bpt			-l texmem_2_32bpt	%EXFLAGS%
simple		--32bpt			-l simple_32bpt		%EXFLAGS%
simple		-a	--32bpt	-s   2	-l simple_1_32bpt	%EXFLAGS%
simple		-Z	--32bpt	-s   7	-l simple_2_32bpt	%EXFLAGS%
fmt	-8		--32bpt	-s   3	-l fmt_1_32bpt %EXFLAGS%
fmt			--32bpt	-s   4	-l fmt_2_32bpt %EXFLAGS%
fmt	-8 -a		--32bpt	-s 172	-l fmt_3_32bpt %EXFLAGS%
fmt	   -a	 -T	--32bpt	-s 141	-l fmt_4_32bpt %EXFLAGS%
fmt	-8 -a -b -T	--32bpt	-s 215	-l fmt_5_32bpt %EXFLAGS%
fmt	   -a -b --32bpt -A	-s 226	-l fmt_6_32bpt %EXFLAGS%
REM slightly complex tests, turn on clamping, random point/bilinear filtering
fmt	-8 -a -Bc	--32bpt	-s 331	-l fmt_7_32bpt %EXFLAGS%
fmt	   -a -Bc -T	-A	--32bpt	-s 342	-l fmt_8_32bpt %EXFLAGS%
REM and now turn on mirroring (flipping)
fmt	   -a -Bc    -f		--32bpt	-s 343  -l fmt_9_32bpt %EXFLAGS%
fmt	-8 -a -Bc -T -f	-A	--32bpt	-s 344  -l fmt_10_32bpt %EXFLAGS%
REM and now turn on SST_CC_MATMU or SST_CC_MRGBTMU
fmt	-v		--32bpt	-s 345	-l fmt_11_32bpt %EXFLAGS%
fmt	-v -a -Bc	--32bpt	-s 346	-l fmt_12_32bpt %EXFLAGS%
fmt	-8v -a -Bc -T -f -A	--32bpt	-s 345	-l fmt_13_32bpt %EXFLAGS%


REM Junk taken from misc.bat
pal		   -Bc		-t5	--32bpt	-l pal_32bpt		%EXFLAGS%
tchroma					--32bpt	-l tchroma_32bpt	%EXFLAGS%
tchroma		-a	-s2	-t1	--32bpt	-l tchroma1_32bpt	%EXFLAGS%
tchroma		-a	-s3	-t3	--32bpt	-l tchroma2_32bpt	%EXFLAGS%
tchroma			-s4	-t5	--32bpt	-l tchroma3_32bpt	%EXFLAGS%
tsustw -O9	-a	-s1	-t6	--32bpt	-l tmustw_32bpt		%EXFLAGS%
tsustw -O9 -z-1	-a	-s2	-t7	--32bpt	-l tmustw1_32bpt	%EXFLAGS%
tsustw -O9 -z-1	-a	-s3	-t8 -P	--32bpt	-l tmustw2_32bpt	%EXFLAGS%
tsustw -O9 -z-1	-a	-s5	-t5 -P	--32bpt	-l tmustw3_32bpt	%EXFLAGS%
tsustw -O9 -z-1	-a	-s6	-t6 -P	--32bpt	-l tmustw4_32bpt	%EXFLAGS%
tsustw -O9 -z-1	-a	-s7	-t7 -P	--32bpt	-l tmustw5_32bpt	%EXFLAGS%

REM download tests
download	-a -J	--32bpt	-l down_1_32bpt		%EXFLAGS%
download  -n	-a -J	--32bpt	-l down_2_32bpt		%EXFLAGS%


REM tstress tests
tstress1 -D			--32bpt	-l tstress1_32bpt	%EXFLAGS%
tstress2 -D			--32bpt	-l tstress2_32bpt	%EXFLAGS%
tstress2 -D	-FZ	-s 2	--32bpt	-l tstress2_2_32bpt	%EXFLAGS%
tstress2 -D	-FZ	-s 3	--32bpt	-l tstress2_3_32bpt	%EXFLAGS%
 

REM simple tests, use different seeds as tests are almost identical
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
REM reset						%EXFLAGS%
texmem	-t2						%EXFLAGS%
texmem	-t20				-l texmem_1	%EXFLAGS%
texmem	-t200				-l texmem_2	%EXFLAGS%
simple							%EXFLAGS%
simple		-a		-s   2	-l simple_1	%EXFLAGS%
simple		-Z		-s   7	-l simple_2	%EXFLAGS%
erase fmt.log
fmt	-8			-s   3	-l fmt_1 %EXFLAGS%
fmt				-s   4	-l fmt_2 %EXFLAGS%
fmt	-8 -a			-s 172	-l fmt_3 %EXFLAGS%
fmt	   -a	 -T		-s 141	-l fmt_4 %EXFLAGS%
fmt	-8 -a -b -T		-s 215	-l fmt_5 %EXFLAGS%
fmt	   -a -b	-A	-s 226	-l fmt_6 %EXFLAGS%
REM slightly complex tests, turn on clamping, random point/bilinear filtering
fmt	-8 -a -Bc		-s 331	-l fmt_7 %EXFLAGS%
fmt	   -a -Bc -T	-A	-s 342	-l fmt_8 %EXFLAGS%
REM and now turn on mirroring (flipping)
fmt	   -a -Bc    -f		-s 343  -l fmt_9 %EXFLAGS%
fmt	-8 -a -Bc -T -f	-A	-s 344  -l fmt_10 %EXFLAGS%
REM and now turn on SST_CC_MATMU or SST_CC_MRGBTMU
fmt	-v			-s 345	-l fmt_11 %EXFLAGS%
fmt	-v -a -Bc		-s 346	-l fmt_12 %EXFLAGS%
fmt	-8v -a -Bc -T -f -A	-s 345	-l fmt_13 %EXFLAGS%

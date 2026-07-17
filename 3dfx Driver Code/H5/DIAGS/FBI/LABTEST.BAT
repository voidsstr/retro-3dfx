REM simple tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
register %EXFLAGS%
chip	 %EXFLAGS%
chip -Z  -l chip1 %EXFLAGS%
nop	 %EXFLAGS%
REM FAILS nop1	 %EXFLAGS%
REM FAILS point	 %EXFLAGS%
REM FAILS dither2	 %EXFLAGS%
REM FAILS dither4	 %EXFLAGS%
REM FAILS chroma	 %EXFLAGS%
rgbclamp %EXFLAGS%
rgbiter	 %EXFLAGS%
REM FAILS rgbmask  %EXFLAGS%
REM misc tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
fastfill			%EXFLAGS%
REM FAILS rectclip			%EXFLAGS%
freg				%EXFLAGS%
freg	 -as94	-l freg1	%EXFLAGS%
altmap	 -as95			%EXFLAGS%
pixreg				%EXFLAGS%
REM FAILS yorigin				%EXFLAGS%
REM FAILS ccu				%EXFLAGS%
REM FAILS ccurand				%EXFLAGS%
yuv				%EXFLAGS%
rawlfb				%EXFLAGS%
REM simple to complex triangle tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
triangle			%EXFLAGS%
lines				%EXFLAGS%
lines	 -t-20	-l lines_1	%EXFLAGS%
lines1				%EXFLAGS%
lines1	 -t-20	-l lines1_1	%EXFLAGS%
rgbtri				%EXFLAGS%
rgbtri	 -t-3	-l rgbtri4	%EXFLAGS%
rgbtri	 -as99	-l rgbtri5	%EXFLAGS%
rgbtri1				%EXFLAGS%
rgbtri1	 -t-2	-l rgbtri2	%EXFLAGS%
rgbtri1	 -as98	-l rgbtri3	%EXFLAGS%
rgbatri				%EXFLAGS%
rgbatri	 -t-10	-l rgbatri1	%EXFLAGS%
rgbatri	 -as97	-l rgbatri2	%EXFLAGS%
ztri				%EXFLAGS%
ztri	 -as96	-l ztri1	%EXFLAGS%
rgbztri				%EXFLAGS%
rgbztri	 -as95	-l rgbztri1	%EXFLAGS%
REM stipple and z and w tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
stipple	 %EXFLAGS%
zbias	 %EXFLAGS%
zclamp	 %EXFLAGS%
REM FAILS zfunc	 %EXFLAGS%
ziter	 %EXFLAGS%
zmask	 %EXFLAGS%
wbuffer	 %EXFLAGS%
wbias	 %EXFLAGS%
wclamp	 %EXFLAGS%
zfbuffer %EXFLAGS%
REM fog tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
REM FAILS fogmodes -O 0			%EXFLAGS%
REM FAILS fogmodes -O 1	-l fog_1	%EXFLAGS%
REM FAILS fogmodes -O 2	-l fog_2	%EXFLAGS%
REM FAILS fogmodes -O 3	-l fog_3	%EXFLAGS%
REM FAILS fogmodes -O 4	-l fog_4	%EXFLAGS%
REM FAILS fogmodes -O 5	-l fog_5	%EXFLAGS%
REM FAILS fogmodes -O 6	-l fog_6	%EXFLAGS%
REM FAILS fogmodes -O 7	-l fog_7	%EXFLAGS%
REM FAILS fogmodes -O 8	-l fog_8	%EXFLAGS%
REM FAILS fogmodes -O 9	-l fog_9	%EXFLAGS%
REM FAILS fogmodes -O 9	-l fog_a -s 2	%EXFLAGS%
REM FAILS fogmodes -O 9	-l fog_b -s 3	%EXFLAGS%
REM FAILS fogmodes -O 9	-l fog_c -s 4	%EXFLAGS%
REM FAILS fogtable -O 0			%EXFLAGS%
REM FAILS fogtable -O 1	-l fogtab_1	%EXFLAGS%
REM alpha tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
aclamp	 %EXFLAGS%
afunc	 %EXFLAGS%
aiter	 %EXFLAGS%
amask	 %EXFLAGS%
awmask	 %EXFLAGS%
acu	 %EXFLAGS%
REM FAILS ablend	 %EXFLAGS%REM stipple and z and w tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
stress1	-D		-Z	-s1000			%EXFLAGS%
stress1	-D	-t 10   -Z	-s1001	-l stress1_1	%EXFLAGS%
stress1	-D	-t -9		-s1002	-l stress1_2	%EXFLAGS%
stress1	-D	-t  8   -Z	-s1003	-l stress1_3	%EXFLAGS%
stress2	-D		-Z	-s1004			%EXFLAGS%
stress2	-D	-t   9		-s1005	-l stress2_1	%EXFLAGS%
stress2	-D	-t -10  -Z	-s1006	-l stress2_2	%EXFLAGS%
stress2	-D	-t  10  -Z	-s1007	-l stress2_3	%EXFLAGS%
stress3	-D			-s1008			%EXFLAGS%
stress3	-D	-t 10   -Z	-s1009	-l stress3_1	%EXFLAGS%
REM FAILS stress3	-D	-t -9   -Z	-s1010	-l stress3_2	%EXFLAGS%
REM FAILS stress3	-D	-t -8   -Z	-s1011	-l stress3_3	%EXFLAGS%
stress4	-D		-Z	-s1012			%EXFLAGS%
REM FAILS stress4	-D	-t   9  -Z	-s1013	-l stress4_1	%EXFLAGS%
REM FAILS stress4	-D	-t -11  -Z	-s1014	-l stress4_2	%EXFLAGS%
stress4	-D	-t -10		-s1015	-l stress4_3	%EXFLAGS%
REM FAILS stress5	-D			-s1016			%EXFLAGS%
stress7	-D		 -Z	-s1017			%EXFLAGS%
REM FAILS stress7	-D	-t   8  -FZ	-s1018	-l stress7_1	%EXFLAGS%
REM FAILS stress7	-D	-t -10  -FZ	-s1019	-l stress7_2	%EXFLAGS%
REM FAILS stress7	-D	-t   9  -F	-s1020	-l stress7_3	%EXFLAGS%
stress8	-D			-s1021			%EXFLAGS%
REM FAILS stress8	-D	-t 10	-FZ	-s1022	-l stress8_1	%EXFLAGS%
REM FAILS stress8	-D	-t -9   -FZ	-s1023	-l stress8_2	%EXFLAGS%
REM FAILS stress8	-D	-t 10   -FZ	-s1024	-l stress8_3	%EXFLAGS%
stress9	-D		 -Z	-s1025			%EXFLAGS%
stress9	-D	-t 10		-s1026	-l stress9_1	%EXFLAGS%
stress9	-D	-t -9	 -Z	-s1027	-l stress9_2	%EXFLAGS%
stress9	-D	-t 11	 -Z	-s1028	-l stress9_3	%EXFLAGS%
REM lfb tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
REM FAILS lfbwrite -C -O 0	-s1			%EXFLAGS%
REM FAILS lfbwrite -C -O 1	-s2	-l lfbwrit1	%EXFLAGS%
REM FAILS lfbwrite -C -O 2	-s3	-l lfbwrit2	%EXFLAGS%
REM FAILS lfbwrite -C -O 3	-s4	-l lfbwrit3	%EXFLAGS%
REM FAILS lfbwrite -C -O 4	-s5	-l lfbwrit4	%EXFLAGS%
REM FAILS lfbwrite -C -O 5	-s6	-l lfbwrit5	%EXFLAGS%
REM FAILS lfbwrite -C -O 6	-s7	-l lfbwrit6	%EXFLAGS%
REM FAILS lfbwrite -C -O 7	-s8	-l lfbwrit7	%EXFLAGS%
REM FAILS lfbwrite -C -O 8	-s9	-l lfbwrit8	%EXFLAGS%  
REM FAILS lfbwrite -C -O 10	-s10	-l lfbwrit9	%EXFLAGS%
REM FAILS lfbread	 -C 	%EXFLAGS%
REM FAILS lfbread1 -C 	%EXFLAGS%
REM triangle setup tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
tsu	-O 9					%EXFLAGS%
tsu	-O 9	-t 4	-s 1	-l tsu_1	%EXFLAGS%
tsu	-O 9	-t 5	-s 2	-l tsu_2	%EXFLAGS%
tsu	-O 9	-t 6	-s 3	-l tsu_3	%EXFLAGS%
tsu	-O 9	-t 7	-s 4	-l tsu_4	%EXFLAGS%
tsu	-O 9	-t 8	-s 5	-l tsu_5	%EXFLAGS%
tsu	-O 9	-t 9	-s 7	-l tsu_6 %EXFLAGS%
tsu	-O 9	-t 11	-s 2	-l tsu_7 %EXFLAGS%

tsubench -O 0	-at5			%EXFLAGS%
tsubench -O 1	-at5	-l tsub_1	%EXFLAGS%
tsubench -O 2	-at5	-l tsub_2	%EXFLAGS%
tsubench -O 3	-at5	-l tsub_3	%EXFLAGS%
tsubench -O 4	-at5	-l tsub_4	%EXFLAGS%
tsubench -O 5	-at5	-l tsub_5	%EXFLAGS%
tsubench -O 6	-at5	-l tsub_6	%EXFLAGS%
REM command fifo tests 
REM -h disableHoles, issues bump after writes and immediately executes them
REM -N-# out-of-order writes
REM
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
REM some simple triangles, non tsu packets
rgbtri	-W4 					-l rgbtri_1_cmd	%EXFLAGS%
rgbtri	-W4 -h					-l rgbtri_2_cmd	%EXFLAGS%
rgbtri	-W4 -N-175				-l rgbtri_3_cmd	%EXFLAGS%
rgbtri	-W4 -h -N-175				-l rgbtri_4_cmd	%EXFLAGS%
REM random triangles
stress1	-W4		-DZ	-t  11		-l stress1_cmd	%EXFLAGS%
stress2	-W4 -h		-DZ	-t -11		-l stress2_cmd	%EXFLAGS%
REM FAILS stress3	-W4	-N-175	-DZ	-t  11		-l stress3_cmd	%EXFLAGS%
REM FAILS stress4	-W4 -h	-N-175	-DZ	-t -11		-l stress4_cmd	%EXFLAGS%
REM test out fogtable downloads
REM FAILS fogtable -W4 		-O1			-l fogtab_1_cmd	%EXFLAGS%
REM FAILS fogtable -W4 -h		-O1			-l fogtab_2_cmd	%EXFLAGS%
REM FAILS fogtable -W4    -N-175	-O1			-l fogtab_3_cmd	%EXFLAGS%
REM FAILS fogtable -W4 -h -N-175	-O1			-l fogtab_4_cmd	%EXFLAGS%
REM test chip fields
chip	-W4 -h					-l chip_1_cmd	%EXFLAGS%
chip	-W4 -h	-N-183				-l chip_2_cmd	%EXFLAGS%
chip	-W4					-l chip_3_cmd	%EXFLAGS%
chip	-W4	-N-191				-l chip_4_cmd	%EXFLAGS%
REM test packet type 3
tsu	-W4 -h		-O 9	-t 5 -s 2	-l tsu_1_cmd	%EXFLAGS%
tsu	-W4 -h	-N-177	-O 9	-t 5 -s 3	-l tsu_2_cmd	%EXFLAGS%
tsu	-W4		-O 9	-t 6 -s 4	-l tsu_3_cmd	%EXFLAGS%
tsu	-W4	-N-179	-O 9	-t 6 -s 5	-l tsu_4_cmd	%EXFLAGS%
REM test long triangle strips
tsubench -W4 -N-129	-O 0	-at5	-u20	-l tsub_1_cmd	%EXFLAGS%
tsubench -W4 -N-130	-O 1	-at5	-u20	-l tsub_2_cmd	%EXFLAGS%
tsubench -W4 -N-131	-O 2	-at5	-u20	-l tsub_3_cmd	%EXFLAGS%
tsubench -W4 -N-132	-O 3	-at5	-u20	-l tsub_4_cmd	%EXFLAGS%
tsubench -W4 -N-133	-O 4	-at5	-u20	-l tsub_5_cmd	%EXFLAGS%
tsubench -W4 -N-134	-O 5	-at5	-u20	-l tsub_6_cmd	%EXFLAGS%
tsubench -W4 -N-135	-O 6	-at5	-u20	-l tsub_7_cmd	%EXFLAGS%
REM test long independent strips (BDDBDD)
tsubench -W4 -N 129	-O 0 -bat5	-u20	-l tsub_10_cmd	%EXFLAGS%
tsubench -W4 -N-133	-O 0 -bat5	-u20	-l tsub_11_cmd	%EXFLAGS%
tsubench -W4 -N-132	-O 1 -bat5	-u20	-l tsub_12_cmd	%EXFLAGS%
tsubench -W4 -N 130	-O 2 -bat5	-u20	-l tsub_13_cmd	%EXFLAGS%
tsubench -W4 -N 131	-O 3 -bat5	-u20	-l tsub_14_cmd	%EXFLAGS%
tsubench -W4 -N-129	-O 3 -bat5	-u20	-l tsub_15_cmd	%EXFLAGS%
REM and finally some stress tests
stress5 -D -W4	  -N-129			-s9020	-l stress5_cmd	%EXFLAGS%
stress7 -D -W4 -h -N-199	-t-10		-s9021	-l stress7_1_cmd	%EXFLAGS%
stress7 -D -W4	  -N-131	-t9	-ZF	-s9022	-l stress7_2_cmd	%EXFLAGS%
stress7 -D -W4	  -N-197		-ZF	-s9023	-l stress7_3_cmd	%EXFLAGS%
stress8 -D -W4 -h -N-200	-t 8	-ZF	-s9024	-l stress8_1_cmd	%EXFLAGS%
stress8 -D -W4	  -N-135	-t-9	-ZF	-s9025	-l stress8_2_cmd	%EXFLAGS%
stress9 -D -W4 -h		-Z	-s9026	-l stress8_3_cmd	%EXFLAGS%
stress9 -D -W4 -h -N-132	-t 10		-s9027  -l stress9_1_cmd	%EXFLAGS%
stress9 -D -W4		-t -9	-Z	-s9028	-l stress9_2_cmd	%EXFLAGS%
stress9 -D -W4	  -N-188	-t 11	-Z	-s9029	-l stress9_3_cmd	%EXFLAGS%




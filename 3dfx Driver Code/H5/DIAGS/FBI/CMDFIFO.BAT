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
stress3	-W4	-N-175	-DZ	-t  11		-l stress3_cmd	%EXFLAGS%
stress4	-W4 -h	-N-175	-DZ	-t -11		-l stress4_cmd	%EXFLAGS%
REM test out fogtable downloads
fogtable -W4 		-O1			-l fogtab_1_cmd	%EXFLAGS%
fogtable -W4 -h		-O1			-l fogtab_2_cmd	%EXFLAGS%
fogtable -W4    -N-175	-O1			-l fogtab_3_cmd	%EXFLAGS%
fogtable -W4 -h -N-175	-O1			-l fogtab_4_cmd	%EXFLAGS%
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




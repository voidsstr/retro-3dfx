REM AGP command fifo tests 
REM -h disableHoles, issues bump after writes and immediately executes them
REM
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
REM some simple triangles, non tsu packets
rgbtri	-W14 -h					-l rgbtri_2_agp	%EXFLAGS%
rgbtri	-W14 -h -N175				-l rgbtri_4_agp	%EXFLAGS%
REM random triangles
stress1	-W14 -h		-DZ	-t  11		-l stress1_agp	%EXFLAGS%
stress2	-W14 -h		-DZ	-t -11		-l stress2_agp	%EXFLAGS%
stress3	-W14 -h	-N175	-DZ	-t  11		-l stress3_agp	%EXFLAGS%
stress4	-W14 -h	-N175	-DZ	-t -11		-l stress4_agp	%EXFLAGS%
REM test out fogtable downloads
fogtable -W14 -h 	-O1			-l fogtab_2_agp	%EXFLAGS%
fogtable -W14 -h -N175	-O1			-l fogtab_4_agp	%EXFLAGS%
REM test chip fields
chip	-W14 -h					-l chip_1_agp	%EXFLAGS%
chip	-W14 -h	-N183				-l chip_2_agp	%EXFLAGS%
REM test packet type 3
tsu	-W14 -h 	-O 9	-t 5 -s 2	-l tsu_1_agp	%EXFLAGS%
tsu	-W14 -h -N177	-O 9	-t 5 -s 3	-l tsu_2_agp	%EXFLAGS%
tsu	-W14 -h		-O 9	-t 6 -s 4	-l tsu_3_agp	%EXFLAGS%
tsu	-W14 -h	-N179	-O 9	-t 6 -s 5	-l tsu_4_agp	%EXFLAGS%
REM test long triangle strips
tsubench -W14 -h -N129	-O 0	-at5	-u20	-l tsub_1_agp	%EXFLAGS%
tsubench -W14 -h -N130	-O 1	-at5	-u20	-l tsub_2_agp	%EXFLAGS%
tsubench -W14 -h -N131	-O 2	-at5	-u20	-l tsub_3_agp	%EXFLAGS%
tsubench -W14 -h -N132	-O 3	-at5	-u20	-l tsub_4_agp	%EXFLAGS%
tsubench -W14 -h -N133	-O 4	-at5	-u20	-l tsub_5_agp	%EXFLAGS%
tsubench -W14 -h -N134	-O 5	-at5	-u20	-l tsub_6_agp	%EXFLAGS%
tsubench -W14 -h -N135	-O 6	-at5	-u20	-l tsub_7_agp	%EXFLAGS%
REM test long independent strips (BDDBDD)
tsubench -W14 -h -N129	-O 0 -bat5	-u20	-l tsub_10_agp	%EXFLAGS%
tsubench -W14 -h -N133	-O 0 -bat5	-u20	-l tsub_11_agp	%EXFLAGS%
tsubench -W14 -h -N132	-O 1 -bat5	-u20	-l tsub_12_agp	%EXFLAGS%
tsubench -W14 -h -N130	-O 2 -bat5	-u20	-l tsub_13_agp	%EXFLAGS%
tsubench -W14 -h -N131	-O 3 -bat5	-u20	-l tsub_14_agp	%EXFLAGS%
tsubench -W14 -h -N129	-O 3 -bat5	-u20	-l tsub_15_agp	%EXFLAGS%
REM and finally some stress tests
stress5 -D -W14 -h -N129			-s9020	-l stress5_agp	%EXFLAGS%
stress7 -D -W14 -h -N199	-t-10		-s9021	-l stress7_1_agp	%EXFLAGS%
stress7 -D -W14 -h -N131	-t9	-ZF	-s9022	-l stress7_2_agp	%EXFLAGS%
stress7 -D -W14 -h -N197		-ZF	-s9023	-l stress7_3_agp	%EXFLAGS%
stress8 -D -W14 -h -N196	-t 8	-ZF	-s9024	-l stress8_1_agp	%EXFLAGS%
stress8 -D -W14 -h -N195	-t-9	-ZF	-s9025	-l stress8_2_agp	%EXFLAGS%
stress9 -D -W14 -h		-Z	-s9026	-l stress8_3_agp	%EXFLAGS%
stress9 -D -W14 -h -N189	-t 10		-s9027  -l stress9_1_agp	%EXFLAGS%
stress9 -D -W14 -h	-t -9	-Z	-s9028	-l stress9_2_agp	%EXFLAGS%
stress9 -D -W14 -h -N188	-t 11	-Z	-s9029	-l stress9_3_agp	%EXFLAGS%





REM  MISC tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
limits		-a 	     -P				%EXFLAGS%
limits		-a -c	     -P		-l limits1	%EXFLAGS%
tcu	-z-1	-a -B	     -P				%EXFLAGS%
tcu	-z-1	-a -B 	     -P		-l tcu1		%EXFLAGS%
tcu		-a -Bc	     -P	-Tt6	-l tcu2		%EXFLAGS%
tcu	-z-1	-a -Br	     -P	-t7	-l tcu3		%EXFLAGS%
pal		   -Bc		-t5			%EXFLAGS%
tchroma							%EXFLAGS%
tchroma		-a	-s2	-t1	-l tchroma1	%EXFLAGS%
tchroma		-a	-s3	-t3	-l tchroma2	%EXFLAGS%
tchroma			-s4	-t5	-l tchroma3	%EXFLAGS%
tsustw -O9	-a	-s1	-t6			%EXFLAGS%
tsustw -O9 -z-1	-a	-s2	-t7	-l tmustw1	%EXFLAGS%
tsustw -O9 -z-1	-a	-s3	-t8 -P	-l tmustw2	%EXFLAGS%
tsustw -O9 -z-1	-a	-s5	-t5 -P	-l tmustw3	%EXFLAGS%
tsustw -O9 -z-1	-a	-s6	-t6 -P	-l tmustw4	%EXFLAGS%
tsustw -O9 -z-1	-a	-s7	-t7 -P	-l tmustw5	%EXFLAGS%
erase download.log
download -8	-a -J	-l down8		%EXFLAGS%
download	-a -J	-l down16		%EXFLAGS%
download -8n	-a -J	-l down8n		%EXFLAGS%
download  -n	-a -J	-l down16n		%EXFLAGS%
REM now some stress tests
tstress1 -D					%EXFLAGS%
tstress1 -D -z-1		-l tstress1_1	%EXFLAGS%
tstress2 -D					%EXFLAGS%
tstress2 -D	 -FZ	-s 2	-l tstress2_2	%EXFLAGS%
tstress2 -D -z-1 -FZ	-s 3	-l tstress2_3	%EXFLAGS%
REM and now some fault coverage test
fc_down1 -D	-y1				%EXFLAGS%
fc_it1	 -D					%EXFLAGS%
fc_lod	 -D					%EXFLAGS%
REM and now some performance tests
perf	-O 0	-at5	-u20			%EXFLAGS%
perf	-O 1	-at5	-u20	-l perf1	%EXFLAGS%
perf	-O 2	-at5	-u20	-l perf2	%EXFLAGS%
perf	-O 3	-at5	-u20	-l perf3	%EXFLAGS%
perf	-O 4	-at5	-u20	-l perf4	%EXFLAGS%

REM command fifo tests 
REM -h disableHoles, issues bump after writes and immediately executes them
REM -N-# out-of-order writes
REM
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
REM some simple triangles, non tsu packets
simple	-W14 -h					-l simple_2_cmdagp	%EXFLAGS%
simple	-W14 -h -N-175	-aZP			-l simple_4_cmdagp	%EXFLAGS%
REM random formats etc
fmt	-W14  -h		-8 -a -Bc	-s 331	-l fmt_1_cmdagp	%EXFLAGS%
fmt	-W14 -h -N-239	   -a -Bc -TP	-s 342	-l fmt_2_cmdagp 	%EXFLAGS%
fmt	-W14 -h		-8 -a -B  -rP	-s 402	-l fmt_3_cmdagp	%EXFLAGS%
fmt	-W14 -h -N-139	-8    -Bc -rP	-s 403	-l fmt_4_cmdagp	%EXFLAGS%
fmt	-W14 -h -N-139	-8 -a -B  -r -n	-s 502	-l fmt_5_cmdagp	%EXFLAGS%
REM some more...
floatstw -W14 -h					-l float_1_cmdagp	%EXFLAGS%
floatstw -W14 -h		-a	-P -s 602	-l float_2_cmdagp	%EXFLAGS%
floatstw -W14 -h -N-138	-Bc     -P -s 603	-l float_3_cmdagp	%EXFLAGS%
floatstw -W14 -h -N-137 -h	-Bc	   -s 604	-l float_4_cmdagp	%EXFLAGS%
lod	-W14 -h		-a -B  -M -P -s 607	-l float_5_cmdagp	%EXFLAGS%
lod	-W14 -h -N-136	-a -B -mM -P -s 608	-l float_6_cmdagp	%EXFLAGS%
pal	-W14 -h		-Bc	-t5		-l float_7_cmdagp	%EXFLAGS%
REM some texture memory writes
texmem	-W14 -h		-t2			-l texmem_1_cmdagp	%EXFLAGS%
texmem	-W14 -h -N150	-t20			-l texmem_2_cmdagp	%EXFLAGS%
texmem	-W14 -h -N-2201	-t200			-l texmem_3_cmdagp	%EXFLAGS%
REM test packet type 3
tsustw	-W14 -h -N-145	-O9	-a -s1	-t6	-l tsustw_1_cmdagp	%EXFLAGS%
tsustw	-W14 -h		-O9	-a -s2	-t7	-l tsustw_2_cmdagp	%EXFLAGS%
tsustw	-W14 -h -N-2299	-O9	-a -s3	-t8 -P	-l tsustw_3_cmdagp	%EXFLAGS%
REM and texture downloads!!!
download -W14 -h		-8	-a -J -s 3	-l down8_cmdagp	%EXFLAGS%
download -W14 -h			-a -J -s 4	-l down16_cmdagp	%EXFLAGS%
download -W14 -h -N-200	-8n	-a -J -s 5	-l down8n_cmdagp	%EXFLAGS%
download -W14 -h -N-2199	-n	-a -J -s 6	-l down16n_cmdagp	%EXFLAGS%
download -g 10 -W14 -h		-8	-a -J -s 3	-l down8_cmdagpp6	%EXFLAGS%
download -g 10 -W14 -h			-a -J -s 4	-l down16_cmdagpp6	%EXFLAGS%
download -g 10 -W14 -h -N-200	-8n	-a -J -s 5	-l down8n_cmdagpp6	%EXFLAGS%
download -g 10 -W14 -h -N-2199	-n	-a -J -s 6	-l down16n_cmdagpp6	%EXFLAGS%

REM and finally some stress tests
tstress1 -W14 -h -N-150	-D			-l tstress1_cmdagp -p 10	%EXFLAGS%
tstress1 -W14 -h -N-151	-D -z-1			-l tstress1_1_cmdagp -p 10	%EXFLAGS%
tstress2 -W14 -h -N-2301	-D			-l tstress2_1_cmdagp -p 10	%EXFLAGS%
tstress2 -W14 -h -N-3303	-D	-FZ	-s 2	-l tstress2_2_cmdagp -p 10	%EXFLAGS%
tstress2 -W14 -h -N-4404	-D -z-1	-FZ	-s 3	-l tstress2_3_cmdagp -p 10	%EXFLAGS%


REM LOD tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
lod		-a	     -P			 %EXFLAGS%
lod		-a -B	  -n -P -s 603	-l lod_1 %EXFLAGS%
lod	-8	-a -B	     -P -s 604	-l lod_2 %EXFLAGS%
lod	-8	-a	  -n -P -s 605	-l lod_3 %EXFLAGS%
lod	-8	-a -B	 -m  -P -s 606	-l lod_4 %EXFLAGS%
lod		-a -B	  -M -P -s 607	-l lod_5 %EXFLAGS%
lod		-a -B	 -mM -P -s 608	-l lod_6 %EXFLAGS%
trilin	   -z-1	-a 	     -P			 %EXFLAGS%
trilin	   -z-1	-a  -Z	     -P	-s 701	-l tri_1 %EXFLAGS%
trilin	   -z-1	-a -cr	 -m  -P -s 702	-l tri_2 %EXFLAGS%
trilin	-8 -z-1	-a	 -m  -P -s 703	-l tri_3 %EXFLAGS%
trilin	   -z-1	-a -BZ	 -m  -P	-s 704	-l tri_4 %EXFLAGS%
trilin	-8 -z-1	-a -BZ	 -m  -P	-s 705	-l tri_5 %EXFLAGS%
trilin	   -z-1	-a -BS	 -m  -P	-s 706	-l tri_6 %EXFLAGS%
trilin	-8 -z-1	-a -BScr -m  -P	-s 707	-l tri_7 %EXFLAGS%
trilin	   -z-1	-a -BSr  -m  -P	-s 708	-l tri_8 %EXFLAGS%
trilin	-8 -z-1	-a -BSr  -m  -P	-s 709	-l tri_9 %EXFLAGS%
trilin	-8 -z-1	-a -BScr -n -P	-s 710	-l tri_A %EXFLAGS%
detail	   -z-1	-a -B	 -m  -P	-s 800		 %EXFLAGS%
detail	-8 -z-1	-a -B	 -m  -P	-s 801	-l det_1 %EXFLAGS%
detail	   -z-1	-a -B	 -m  -P	-s 802	-l det_2 %EXFLAGS%
detail	   -z-1	-a -B	 -mM -P -s 803	-l det_3 %EXFLAGS%

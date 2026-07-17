REM stipple and z and w tests
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
stress3	-D	-t -9   -Z	-s1010	-l stress3_2	%EXFLAGS%
stress3	-D	-t -8   -Z	-s1011	-l stress3_3	%EXFLAGS%
stress4	-D		-Z	-s1012			%EXFLAGS%
stress4	-D	-t   9  -Z	-s1013	-l stress4_1	%EXFLAGS%
stress4	-D	-t -11  -Z	-s1014	-l stress4_2	%EXFLAGS%
stress4	-D	-t -10		-s1015	-l stress4_3	%EXFLAGS%
stress5	-D			-s1016			%EXFLAGS%
stress7	-D		 -Z	-s1017			%EXFLAGS%
stress7	-D	-t   8  -FZ	-s1018	-l stress7_1	%EXFLAGS%
stress7	-D	-t -10  -FZ	-s1019	-l stress7_2	%EXFLAGS%
stress7	-D	-t   9  -F	-s1020	-l stress7_3	%EXFLAGS%
stress8	-D			-s1021			%EXFLAGS%
stress8	-D	-t 10	-FZ	-s1022	-l stress8_1	%EXFLAGS%
stress8	-D	-t -9   -FZ	-s1023	-l stress8_2	%EXFLAGS%
stress8	-D	-t 10   -FZ	-s1024	-l stress8_3	%EXFLAGS%
stress9	-D		 -Z	-s1025			%EXFLAGS%
stress9	-D	-t 10		-s1026	-l stress9_1	%EXFLAGS%
stress9	-D	-t -9	 -Z	-s1027	-l stress9_2	%EXFLAGS%
stress9	-D	-t 11	 -Z	-s1028	-l stress9_3	%EXFLAGS%

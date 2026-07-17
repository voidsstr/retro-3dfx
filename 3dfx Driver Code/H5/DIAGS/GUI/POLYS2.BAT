setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
randpoly -MPTkKcrLBm -t 10	-l rpoly_1 -s1 -xd0	%EXFLAGS%
randpoly -MPTkKcrLBm -t 10	-l rpoly_2 -s2 -xd1	%EXFLAGS%
randpoly -MPTkKcrLBm -t 10	-l rpoly_3 -s3 -xd2	%EXFLAGS%
randpoly -MPTkKcrLBm -t 10	-l rpoly_4 -s4 -xd3	%EXFLAGS%
randpoly -MPTkKcrLBm -t 10	-l rpoly_5 -s5 -xd3	-y1 %EXFLAGS%
randpoly -MPTkKcrLBm -t 10	-l rpoly_6 -s6 -xd4	-y1 %EXFLAGS%
poly2	-p4			-l poly2_1 -s1 -xd0	%EXFLAGS%
poly2	-p4			-l poly2_2 -s2 -xd1	%EXFLAGS%
poly2	-p4			-l poly2_3 -s3 -xd2	%EXFLAGS%
poly2	-p4			-l poly2_4 -s4 -xd3	%EXFLAGS%
poly2	-p4	-t10		-l poly2_5 -s1 -xd0	%EXFLAGS%
poly2	-p4	-t10		-l poly2_6 -s2 -xd1	%EXFLAGS%
poly2	-p4	-t10		-l poly2_7 -s3 -xd2	%EXFLAGS%
poly2	-p4	-t10		-l poly2_8 -s4 -xd3	%EXFLAGS%
poly3   -p4			-l poly3_1 -s1 -xd0	%EXFLAGS%
poly3   -p4			-l poly3_2 -s2 -xd1	%EXFLAGS%
poly3   -p4			-l poly3_3 -s3 -xd2	%EXFLAGS%
poly3   -p4			-l poly3_4 -s4 -xd3	%EXFLAGS%
poly3		-t100		-l poly3_5 -s5 -xd3	%EXFLAGS%

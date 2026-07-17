REM lfb tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
lfbwrite -C -O 0	-s1			%EXFLAGS%
lfbwrite -C -O 1	-s2	-l lfbwrit1	%EXFLAGS%
lfbwrite -C -O 2	-s3	-l lfbwrit2	%EXFLAGS%
lfbwrite -C -O 3	-s4	-l lfbwrit3	%EXFLAGS%
lfbwrite -C -O 4	-s5	-l lfbwrit4	%EXFLAGS%
lfbwrite -C -O 5	-s6	-l lfbwrit5	%EXFLAGS%
lfbwrite -C -O 6	-s7	-l lfbwrit6	%EXFLAGS%
lfbwrite -C -O 7	-s8	-l lfbwrit7	%EXFLAGS%
lfbwrite -C -O 8	-s9	-l lfbwrit8	%EXFLAGS%  
lfbwrite -C -O 9	-s10	-l lfbwrit9	%EXFLAGS%  
lfbwrite -C -O 11	-s11	-l lfbwrit10	%EXFLAGS%
REM lfbread  -C 	%EXFLAGS%
REM lfbread1 -C 	%EXFLAGS%

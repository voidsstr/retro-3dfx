REM fog tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
fogmodes -O 0			%EXFLAGS%
fogmodes -O 1	-l fog_1	%EXFLAGS%
fogmodes -O 2	-l fog_2	%EXFLAGS%
fogmodes -O 3	-l fog_3	%EXFLAGS%
fogmodes -O 4	-l fog_4	%EXFLAGS%
fogmodes -O 5	-l fog_5	%EXFLAGS%
fogmodes -O 6	-l fog_6	%EXFLAGS%
fogmodes -O 7	-l fog_7	%EXFLAGS%
fogmodes -O 8	-l fog_8	%EXFLAGS%
fogmodes -O 9	-l fog_9	%EXFLAGS%
fogmodes -O 9	-l fog_a -s 2	%EXFLAGS%
fogmodes -O 9	-l fog_b -s 3	%EXFLAGS%
fogmodes -O 9	-l fog_c -s 4	%EXFLAGS%
fogtable -O 0			%EXFLAGS%
fogtable -O 1	-l fogtab_1	%EXFLAGS%

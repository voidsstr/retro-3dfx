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

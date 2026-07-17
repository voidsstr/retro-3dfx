REM kens directed tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
align							%EXFLAGS%
hbltalgn						%EXFLAGS%
hbltalgn -y1 -l hbltalgn_tiled				%EXFLAGS%
clip1							%EXFLAGS%
clip2							%EXFLAGS%
clip3 -p3						%EXFLAGS%
block	 						%EXFLAGS%
bgblock1 						%EXFLAGS%
bgblock2 -xcm 						%EXFLAGS%
destread -p50 						%EXFLAGS%
pattern	 						%EXFLAGS%
scr2scr	 						%EXFLAGS%
updatexy 						%EXFLAGS%

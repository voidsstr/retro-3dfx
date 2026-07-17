REM Avenger+ tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
yorigin +y	%EXFLAGS%
pixreg		%EXFLAGS%
fogmodes -O9	%EXFLAGS%
fogtable -O0	%EXFLAGS%
dither2		%EXFLAGS%
dither4		%EXFLAGS%
fastfill	%EXFLAGS%

rgbclamp	%EXFLAGS%
rgbiter		%EXFLAGS%
rgbmask		%EXFLAGS%
rgbtri		%EXFLAGS%
rgbtri1		%EXFLAGS%
rgbztri		%EXFLAGS%
rgbatri		%EXFLAGS%

aclamp		%EXFLAGS%
afunc		%EXFLAGS%
aiter		%EXFLAGS%
amask		%EXFLAGS%
awmask		%EXFLAGS%
acu		%EXFLAGS%
ablend		%EXFLAGS%

ccu		%EXFLAGS%
ccurand		%EXFLAGS%
chroma		%EXFLAGS%
lfbwrite -O11	%EXFLAGS%

zbias		%EXFLAGS%
zclamp		%EXFLAGS%
zfbuffer	%EXFLAGS%
zfunc		%EXFLAGS%
ziter		%EXFLAGS%
zmask		%EXFLAGS%
ztri		%EXFLAGS%

wbias		%EXFLAGS%
wbuffer		%EXFLAGS%
wclamp		%EXFLAGS%
freg		%EXFLAGS%
altmap		%EXFLAGS%

stmask		%EXFLAGS%
stfunc		%EXFLAGS%
stop		%EXFLAGS%
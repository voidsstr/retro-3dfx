REM misc tests
setlocal
set EXFLAGS=%1 %2 %3 %4 %5 %6 %7 %8 %9
fastfill			%EXFLAGS%
rectclip			%EXFLAGS%
freg				%EXFLAGS%
freg	 -as94	-l freg1	%EXFLAGS%
altmap	 -as95			%EXFLAGS%
pixreg				%EXFLAGS%
yorigin				%EXFLAGS%
yorigin  +y			%EXFLAGS%
ccu				%EXFLAGS%
ccurand				%EXFLAGS%
yuv				%EXFLAGS%
rawlfb				%EXFLAGS%

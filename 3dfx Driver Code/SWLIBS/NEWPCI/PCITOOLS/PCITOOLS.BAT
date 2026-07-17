rem
rem   This batch file compiles with watcom for DOS2 env.
rem
rem   This is an example of the commands that are needed to
rem   build this lib
rem
rem   You will need to modify this file to adopt to your
rem   build environment



SET WCC386=-otexan -I..\..\include  -D__DOS32__ -w4 -e25 -7 -fp5 -zq -5r -ei -mf -bt#dos

wcc386 detect.c

set WLINK=OP m d all OP Q OP SYMF SYS DOS4G
create tmp.lnk
echo name detect.exe >> tmp.lnk
echo file detect.obj >> tmp.lnk
echo libr ..\..\lib\fxdpmi.lib >> tmp.lnk
echo libr ..\..\lib\fxpci.lib  >> tmp.lnk
wlink @WLINK @tmp.lnk
del tmp.lnk

wcc386 pcirw.c
create tmp.lnk
echo name pcirw.exe >> tmp.lnk
echo file pcirw.obj >> tmp.lnk
echo libr ..\..\lib\fxdpmi.lib >> tmp.lnk
echo libr ..\..\lib\fxpci.lib >> tmp.lnk
wlink @WLINK @tmp.lnk
del tmp.lnk

copy *.exe ..\..\..\bin

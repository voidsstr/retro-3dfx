rem
rem   This batch file compiles with watcom for DOS2 env.
rem
rem   This is an example of the commands that are needed to
rem   build this lib
rem
rem   You will need to modify this file to adopt to your
rem   build environment


rem This sets up the watcom compiler vars
rem you probably need to change these for you development env

set WATCOM=c:\watcom
set PATH=%PATH%;c:watcom\binnt;c:\watcom\binw
set INCLUDE=c:\watcom\h;c:\watcom\h\nt;..\..\include

rem
rem if you want to build for Win32 instead of DOS
rem change bt=NT and copy the glide library from WIN32
rem Change NT to DOS4G on link line
rem

SET WCC386=-otexan -I..\include -D__DOS32__ -w4 -e25 -7 -fp5 -zq -5r -ei -mf -bt#dos      
wcc386 fxdpmi.c
create tmp.lnk
echo fxdpmi.obj >> tmp.lnk
wlib -b -c -n -q -p=512 fxdpmi @tmp.lnk
del tmp.lnk

copy fxdpmi.lib ..\lib

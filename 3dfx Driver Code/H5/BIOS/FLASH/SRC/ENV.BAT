@echo off
REM  The default Watcom build environment is needed 
REM  to build this utility with the exception that 
REM  Microsoft's NMAKE utility is also need.
PATH C:\WATCOM\BINNT;%path%;C:\WATCOM\BINW;C:\VC50\BIN
SET INCLUDE=C:\WATCOM\H;C:\WATCOM\H\NT
SET WATCOM=C:\WATCOM
SET EDPATH=C:\WATCOM\EDDAT

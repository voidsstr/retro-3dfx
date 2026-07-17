#
#	MAKEFILE - NMAKE script for EDIAG.EXE
#	Copyright (c) 1996-1998 Elpin Systems, Inc.
#	All rights reserved.
#
#	Written by:	Larry Coffey
#	Date:		3/9/98
#	Last modified:	5/7/98
#

##### Module Macros ####
TARGET		= ediag.lib
TSTOBJS		= main.obj cmdline.obj user.obj file.obj display.obj mem.obj tests.obj
LIBS		= elib.lib repdev.lib vgadata.lib ediag1.lib vgacore.lib 
INCDIR		= C:\WATCOM\h
MAKE		= wmake /f wmake.mak


all: lib subdirs $(TARGET) install

lib:
	mkdir lib

##### DEBUG Macro Defined #####
# To include debugging information for symbolic debugging set DEBUG=1
DEBUG		= 1

##### Build Option Macros #####
!ifdef DEBUG
DDEF		= -DDEBUG
COPT		= -e25 -zq -d2 -mf -5r -hw -od -wx
LOPT		= SYS dos4g op m debug all op st=8192 op maxe=25 op q
!else
DDEF		=
COPT		= -w4 -e25 -zq -otexan -d1 -mf -5r
LOPT		= SYS dos4g op m op st=8192 op maxe=25 op q op symf
!endif

##### General Macros #####
DEF		= 

##### Tool Macros #####
CC		= wcc386 
# -bt=dos -i$(INCDIR) $(COPT) $(DDEF) $(DEF)
LINK		= wlink $(LOPT)

##### Inference Rules #####
.c.obj:
	SET WCC386=-s -otexan -Id:\3dfx\devel\swlibs\include;  -D__DOS32__ -w4 -e25 -7 -fp5 -zq -5s -ei -mf -bt#dos -Id:\3dfx\devel\swlibs\include -Id:\3dfx\devel\h5\include  -I..\lua\include -DBUILD_DIAGS	
	$(CC) $*.c

##### Dependents For Goal and Command Line #####
$(TARGET):	$(TSTOBJS)
#	!wlib $(TARGET) -+$?
	!wlib $(TARGET) $?

subdirs: lib\vgadata.lib lib\repdev.lib lib\elib.lib lib\ediag1.lib lib\vgacore.lib .SYMBOLIC


lib\vgadata.lib: 
	cd vgadata
	$(MAKE)
	cd ..
	
lib\repdev.lib:	
	cd report
	$(MAKE)
	cd ..
	
lib\elib.lib:
	cd 3dfx
	$(MAKE)
	cd ..

lib\ediag1.lib:
	cd ediag1
	$(MAKE)
	cd ..

lib\vgacore.lib:
	cd vgacore
	$(MAKE)
	cd ..

install: .SYMBOLIC
	copy /y $(TARGET) lib

##### Clean Directory #####
clean: subdir-clean .SYMBOLIC
	-if exist *.obj del *.obj
	-if exist $(NAME).exe del $(NAME).exe
	-if exist *.map del *.map
	-if exist *.sym del *.sym
	-if exist *.lib del *.lib
	-if exist lib\*.lib del lib\*.lib
	
subdir-clean: .SYMBOLIC
	cd vgadata
	$(MAKE) clean
	cd ..
	cd report
	$(MAKE) clean
	cd ..
	cd 3dfx
	$(MAKE) clean
	cd ..	
	cd ediag1
	$(MAKE) clean
	cd ..
#
#	Copyright (c) 1996-1998 Elpin Systems, Inc.
#	All rights reserved.
#

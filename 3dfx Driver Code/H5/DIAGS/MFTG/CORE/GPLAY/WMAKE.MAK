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
TARGET		= gplay.lib
TSTOBJS		= gplay.obj hash.obj trap.obj
LIBS		= 
INCDIR		= -IC:\WATCOM\h -I..\..\3dfx\include
MAKE		= wmake /f wmake.mak

LCINCS = -I..\..\3dfx\include -I..\..\lua\include
LCOPTS = -w4 -e25 -7 -fp5 -zq -5s -ei -mf -bt#dos
LCDEFS = -DBUILD_DIAGS -D__DOS32__ -DDIAG_BUILD


all: $(TARGET)

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
LINK		= wlink $(LOPT)

# old compile stuff...
#	SET WCC386=-s -otexan -Id:\3dfx\devel\swlibs\include;  -D__DOS32__ -w4 -e25 -7 -fp5 -zq -5s -ei -mf -bt#dos -Id:\3dfx\devel\swlibs\include -Id:\3dfx\devel\h5\include  -I..\lua\include -DBUILD_DIAGS	

##### Inference Rules #####

.c.obj:
	set wcc386= -d2 $(LCINCS) $(LCDEFS) $(LCOPTS)
	$(CC) $*.c

##### Dependents For Goal and Command Line #####
$(TARGET):	$(TSTOBJS)
	!wlib $(TARGET) -+$?


##### Clean Directory #####
clean: .SYMBOLIC
	-if exist *.obj del *.obj
	-if exist $(NAME).exe del $(NAME).exe
	-if exist *.map del *.map
	-if exist *.sym del *.sym
	-if exist *.lib del *.lib
	

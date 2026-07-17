#
#	MAKEFILE - NMAKE script for FLASH.EXE
#	Copyright (c) 1998 Elpin Systems, Inc.
#	All rights reserved.
#
#	Written by:	Larry Coffey
#	Date:		3/4/98
#	Last modified:	3/9/98
#

##### Module Macros ####
NAME		= flash
TARGETNAME	= userflsh
TSTSRCS		= main.cpp user.cpp rom.cpp util.cpp data.cpp pciinfo.cpp
OBJS		=
LIBS		=
INCDIR		= C:\WATCOM\h

##### Include Macro #####
TSTINCLS	= $(NAME).h

##### DEBUG Macro Defined #####
# To include debugging information for symbolic debugging set DEBUG=1
DEBUG		= 0

##### Build Option Macros #####
!if $(DEBUG)
DDEF		= -DDEBUG -DUSERFLASH
COPT		= -e25 -zq -d2 -mf -5r -hw -od -wx
LOPT		= SYS dos4g op m debug all op st=8192 op maxe=25 op q
!else
DDEF		= -DUSERFLASH
COPT		= -w4 -e25 -zq -otexan -d1 -mf -5r
LOPT		= SYS dos4g op m op st=8192 op maxe=25 op q op symf
!endif

##### General Macros #####
DEF		=

##### Tool Macros #####
CC		= wpp386 -bt=dos -i$(INCDIR) $(COPT) $(DDEF) $(DEF)
LINK		= wlink $(LOPT)

##### Inference Rules #####
.cpp.obj:
	$(CC) $*.cpp

##### Main (default) Target #####
goal:	$(TARGETNAME).exe

##### Dependents For Goal and Command Line #####
$(TARGETNAME).exe:	$(TSTSRCS:.cpp=.obj) $(LIBS)
	$(LINK) @<<
	NAME $(TARGETNAME)
	FIL $(TSTSRCS:.cpp=.obj,)
<<

##### Dependents #####
$(TSTSRCS:.cpp=.obj):	$(TSTINCLS)
data.obj:		image.h

##### Clean Directory #####
clean:
	-if exist *.obj del *.obj
	-if exist $(TARGETNAME).exe del $(TARGETNAME).exe
	-if exist *.map del *.map
	-if exist *.sym del *.sym
#
#	Copyright (c) 1998 Elpin Systems, Inc.
#	All rights reserved.
#

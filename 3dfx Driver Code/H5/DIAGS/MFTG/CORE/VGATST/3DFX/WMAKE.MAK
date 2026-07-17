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
NAME		= elib.lib
TSTOBJS		= init.obj mode.obj ramdac.obj engine.obj report.obj data.obj
LIBS		= 
INCDIRS		= -I..\


all: $(NAME) install

#all: subdirs goal install

libs: all

# don't do anything for incs or bins

incs:
bins:

crdir:
	mkdir lib

##### Include Macro #####
TSTINCLS	= $(NAME).h

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
# -bt=dos $(INCDIRS) $(COPT) $(DDEF) $(DEF)
LINK		= wlink $(LOPT)

##### Inference Rules #####
.c.obj:
	SET WCC386=$(INCDIRS) -s -otexan -Id:\3dfx\devel\swlibs\include;  -D__DOS32__ -w4 -e25 -7 -fp5 -zq -5s -ei -mf -bt#dos
	$(CC) $*.c

##### Main (default) Target #####
#goal:	$(NAME).exe
goal: $(NAME).lib

##### Dependents For Goal and Command Line #####
$(NAME):	$(TSTOBJS)
	!wlib $(NAME) -+$?

##### Dependents For Goal and Command Line #####
#$(NAME).exe:	$(TSTSRCS:.cpp=.obj) $(LIBS)
#	$(LINK) @<<
#	NAME $(NAME)
#	FIL $(TSTSRCS:.cpp=.obj,)$(LIBS:.lib=.lib,)
#<<

##### Dependents #####
#$(TSTSRCS:.cpp=.obj):	$(TSTINCLS) makefile

subdirs: $(SUBDIRS)
	cd vgadata
	make
	cd ..
	cd report
	make
	cd ..
	cd 3dfx
	make
	cd ..
	cd ediag1
	make
	cd ..


install: .SYMBOLIC
	copy /y $(NAME) ..\lib

##### Clean Directory #####
clean: .SYMBOLIC
	-if exist *.obj del *.obj
	-if exist $(NAME).exe del $(NAME).exe
	-if exist *.map del *.map
	-if exist *.sym del *.sym
	-if exist *.lib del *.lib
#
#	Copyright (c) 1996-1998 Elpin Systems, Inc.
#	All rights reserved.
#

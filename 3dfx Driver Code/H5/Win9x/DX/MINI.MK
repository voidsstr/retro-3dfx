#############################################################################
#
#   Microsoft Confidential
#   Copyright (C) Microsoft Corporation 1991
#   All Rights Reserved.
#
#   MINI display driver makefile include
#
#   USAGE:
#
#   to use this makefile to build a Windows95 mini driver
#   define the needed macros and include this file.
#
#   NOTE your driver does not need a .RC file
#
#   REQUIRED MACROS
#
#       DRVNAME         the basename of your driver (ie vga)
#       SOURCES         the list of files needed to build your driver
#
#   OPTIONAL MACROS
#
#       DESCRIPTION     this becomes the descripton in the version resource.
#       DRVFILE         final name for the .DRV file, defaults to $(DRVNAME).drv
#       DRVDEF          the .def file to use, if not specifed a default will be used
#       DRVLIBS         any extra libs you need
#       DRVRESFILE      a .rc file
#
#############################################################################

!ifndef DRVNAME
!error DRVNAME not defined
!endif

!ifndef DESCRIPTION
DESCRIPTION=$(DRVNAME) Windows 95 Display Driver
!endif


!ifdef DDKDX
VERSIONHDR=$(DDK9X)\inc16\version.h
RCINCLUDE=$(DDKDX)\inc
!else
!error DDKDX not defined
!endif


#############################################################################
#
#   VERDIR is defined, build a 16 or 32 bit driver
#
#############################################################################

!ifdef VERDIR

!include ..\mini16.mk

#############################################################################
#
# build a res file with the correct version info
#
#############################################################################


$(DRVRES): $(VERSIONHDR)
        $(MYRC) -fo$*.res $(HWPRODUCT) -r -i..\.. $(BRDCFLAGS) -i$(RCINCLUDE) ..\$(DRVNAME).rc 

#############################################################################
#
#   VERDIR is not defined, set the goals and recurse
#
#############################################################################

!else   ## VERDIR

!if "$(BUILD)" == "ALL" || "$(BUILD)" == "all"
goal:   debug.a retail.a
!else
goal:   debug.a
!endif

retail debug: $@.a

retail.a debug.a: echo_target h2inc 
        @echo ***** Building $* version of $(DRVNAME)
        @if not exist $*\nul mkdir $*
        @$(MAKE) /nologo VERDIR=$*
	if not exist ..\bin\nul mkdir ..\bin
	if not exist ..\bin\$*\nul mkdir ..\bin\$*
	copy $*\*.drv ..\bin\$*
!if ("$(HP)" == "h5" || "$(HP)" == "H5")
	copy ..\bin\$*\$(DRVNAME).drv ..\bin\$*\3dfx16vs.drv
!elseif ("$(HP)" == "h4" || "$(HP)" == "H4")
	copy ..\bin\$*\$(DRVNAME).drv ..\bin\$*\3dfx16v3.drv
!else
	copy ..\bin\$*\$(DRVNAME).drv ..\bin\$*\3dfx16vb.drv
!endif

!ifdef	SIW95
	copy $*\*.nms ..\bin\$*
!endif

depend:
        @echo depend not supported yet

clean:
        -deltree /Y debug
        -deltree /Y retail
		-del ..\inc\shared.inc 
		-del ..\inc\h3g.inc
		-del ..\inc\h3gdefs.inc
		-del ..\inc\h3regs.inc

!endif #!ifdef VERDIR

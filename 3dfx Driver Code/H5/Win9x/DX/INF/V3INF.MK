#******************************************************************************
#
# Copyright:    Copyright STB Systems, Inc., 1999
#
# Description:  This is the Voodoo3.INF NMAKE file.
#
# History:      Date      Who  Description
#               --------  ---  -----------------------------------------------
#							   created
#				01/13/99  SR   copied and modified for Voodoo3
#******************************************************************************

# clear the suffixes and then add .INF
.SUFFIXES :
.SUFFIXES: .inf

#------------------------------------------------------------------------------
# Vendor Configuration Macros
#------------------------------------------------------------------------------

# The environement variable CUSTOMERNUMBER defines the 

VENDORS = /D_CUSTOMERNUMBER=$(CUSTOMERNUMBER)

#------------------------------------------------------------------------------
# Hardware Platform (HP) Macros
#------------------------------------------------------------------------------

# define NAPALM identifier if running on Napalm hardware

!if ("$(HP)" == "H5") || ("$(HP)" == "h5")
PLATFORMS = /DNAPALM
!endif

CL    = $(MSVC50)\BIN\CL

CLOPT = /EP /nologo $(VENDORS) $(PLATFORMS)

CLCMD = $(CL) $(CLOPT)


#------------------------------------------------------------------------------
#
# Inference Rules
#
#------------------------------------------------------------------------------
#        @if not exist release\NUL mkdir release >NUL


VOODOO3.INF:
        @$(CLCMD) V3.INF >TEMP.INF
        @$(STBTOOL)\MISC\remblank <TEMP.INF > Voodoo3.inf
        @if exist TEMP.INF del TEMP.INF


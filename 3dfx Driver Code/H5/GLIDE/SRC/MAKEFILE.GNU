#
# Copyright (c) 1995, 3Dfx Interactive, Inc.
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
# the contents of this file may not be disclosed to third parties, copied or
# duplicated in any form, in whole or in part, without the prior written
# permission of 3Dfx Interactive, Inc.
#
# RESTRICTED RIGHTS LEGEND:
# Use, duplication or disclosure by the Government is subject to restrictions
# as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
# and Computer Software clause at DFARS 252.227-7013, and/or in similar or
# successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
# rights reserved under the Copyright Laws of the United States.
#

# local defines, options, includes
# For some reason this version of gmake does not read in values
# from the DOS environment so set the ones we want now.
DEBUG		= 	1
FX_COMPILER	=	GCC
FX_HOST		= 	DOS
FX_TARGET	=	DURAL

FX_GLIDE_HW	=	CVG

BUILD_ROOT	=	d:/3dfx/devel
BUILD_ROOT_CVG	=	$(BUILD_ROOT)/cvg
BUILD_ROOT_SWLIBS=	$(BUILD_ROOT)/swlibs

# Turn on warnings, and make them errors to force us
# to check them out and deal.
ifeq ($(FX_COMPILER), MICROSOFT)
DBGOPTS         +=      /WX /Z7
endif # MICROSOFT
ifeq ($(FX_COMPILER), GCC)
DBGOPTS         +=      -Wall -W -Werror
endif # GCC

# Stuff for debugging
ifeq ($(DEBUG), 1)
DBGOPTS         =       -DGLIDE_DEBUG -DGDBG_INFO_ON
GLIDE_SANITY_ALL=       1

# Compiler debugging options here
ifeq ($(FX_COMPILER), MICROSOFT)
DBGOPTS         +=      /Z7
endif # MICROSOFT
ifeq ($(FX_COMPILER), GCC)
# Turn on stupid verbose mode so that we can see what the
# hell is going wrong w/ their stupid compiler.
DBGOPTS         +=      -g -v
endif # GCC
else
# Turn on optimization stuff
ifeq ($(FX_COMPILER), MICROSOFT)
DBGOPTS         +=      -O2
endif # MICROSOFT
ifeq ($(FX_COMPILER), GCC)
# GCC allows debugging info w/ optimization on. Yeah!
DBGOPTS         +=      -g -O2
endif # GCC

endif # DEBUG

# Display Options (Simulator, Hardware)
ifeq ($(FX_TARGET), DURAL)
# Currently we build for Dural for sh3 rather than sh3e
# because there is a bug in the floating point handling
# and the sh4 compiler is not ready.
#
# -ml is the little endian option.
DSPOPTS         =       -DGLIDE_HARDWARE -m3 -ml -DDURAL
else # !DURAL
# Default to hw case, and replace if simulator option is set
DSPOPTS         =       -DGLIDE_HARDWARE 

ifeq ($(FX_GLIDE_DSP_TARGET), CSIM)
DSPOPTS         =       -DGLIDE_SIMULATOR -DHAL_CSIM
LCINCS          =       -I$(BUILD_ROOT)/$(FX_GLIDE_HW)/csim
endif # CSIM
ifeq ($(FX_GLIDE_DSP_TARGET), SIM)
DSPOPTS         =       -DGLIDE_SIMULATOR -DGSIM
endif # SIM
endif # !DURAL

# Compile for specific hardware
# Default to SST1
LCDEFS          =       -DSST1
LADEFS          =       /DSST1

ifeq ($(FX_GLIDE_HW), CVG)
HWSPEC          =       cvg.c
LCDEFS          =       -DCVG -DGLIDE_INIT_HAL -DCVG_FIFO=1
FX_GLIDE_CTRISETUP=1
endif # CVG
ifeq ($(FX_GLIDE_HW), SST96)
HWSPEC          =       sst96.c
LCDEFS          =       -DSST96 -DGLIDE_USE_ALT_REGMAP
# Turn on group writes for SST96
FX_GLIDE_GROUP_WRITE=1
# Turn on FIFOed writes for SST96
FX_GLIDE_FIFO96=1
# Turn on C trisetup for SST96
FX_GLIDE_CTRISETUP=1
endif # SST96
ifeq ($(FX_GLIDE_HW), SST2)
LCDEFS          =       -DSST2
endif # SST2

# Optimization Options
# This is for turning on and off algorithmic optimizations,
# not flags to the C compiler.  Usually this involves
# enabling/disabling assembly language code, but it can also   
# change the way C code works, or how C code generates data to be 
# used by various pieces of code.
# Usually these are set with environment variables or arguments to 
# nmake.
#

# SST96 has no notion of alternate register mapping
ifneq ($(FX_GLIDE_HW), SST96)
# Turn on/off alt register set. (Glide Reg Map OPTS)
ifneq ($(FX_GLIDE_REG_MAP), ORIG)
GRMOPTS         =       -DGLIDE_USE_ALT_REGMAP
ASM_REGMAP_DEFS =       /DGLIDE_USE_ALT_REGMAP
endif
endif

# Turn on/off assembly language trisetup code.
# (C on is Assembly off) (A ssembly T ri S etup OPTS)
ifeq ($(FX_GLIDE_CTRISETUP), 1)
ATSOPTS         =       -DGLIDE_USE_C_TRISETUP
endif

ifndef FX_GLIDE_CTRISETUP
ASMTRISETUP =   xdraw.asm
endif

OPTOPTS         =       $(GRMOPTS) $(OTSOPTS) $(ATSOPTS)

# local defines, begin with basics and then add on
LCDEFS          +=      -DGLIDE_LIB

# Turn Off/On compilation of shameless plug
ifneq ($(FX_GLIDE_NO_PLUG), 1)
LCDEFS          +=      -DGLIDE_PLUG
endif

# Turn Off/On splash screen
ifneq ($(FX_GLIDE_NO_SPLASH), 1)
LCDEFS          +=      -DGLIDE_SPLASH
endif

# Turn On group writes
ifdef $(FX_GLIDE_GROUP_WRITE
LCDEFS          +=      -DGROUP_WRITE
endif

ifeq ($(FX_GLIDE_FIFO96), 1)
LCDEFS          +=      -DSST96_FIFO
endif

ifdef GLIDE_SANITY_ALL
LCDEFS          +=      -DGLIDE_SANITY_SIZE -DGLIDE_SANITY_ASSERT
else
ifdef GLIDE_SANITY_SIZE
LCDEFS          +=      -DGLIDE_SANITY_SIZE
endif
ifdef GLIDE_SANITY_ASSERT
LCDEFS          +=      -DGLIDE_SANITY_ASSERT
endif
endif

# Local Defs, Includes, and Options (C)
LCINCS          +=      -I$(CVGINC) -I$(BUILD_ROOT_SWLIBS)/include
LCOPTS          =       $(DBGOPTS) $(DSPOPTS) $(OPTOPTS)

# Local Defs, Includes, and Options (ASM)
LADEFS          +=      $(ASM_REGMAP_DEFS)
LAINCS          =       -I$(SST1INC)
LAOPTS          =       $(DBGOPTS) $(DSPOPTS) $(OPTOPTS) /Fl /Sa

AFILES          =       $(ASMTRISETUP) cpudtect.asm

# sources
HEADERS         =       glide.h gump.h glidesys.h glideutl.h
PRIVATE_HEADERS =       fxglide.h gsstdef.h 

INSTALL_DESTINATION =   $(BUILD_ROOT_CVG)

CFILES          =       gsplash.c g3df.c  gu.c guclip.c gpci.c gump.c \
                        diglide.c disst.c ditex.c gbanner.c gerror.c \
                        gmovie.c digutex.c ddgump.c gaa.c gdraw.c \
                        gglide.c glfb.c gsst.c gtex.c gtexdl.c \
                        gutex.c gxdraw.c ddgu.c $(HWSPEC)
OBJECTS         =       $(CFILES:.c=.o)
OTHER_CFILES    =       fxgasm.c 

CODFILES        =       $(CFILES:.c=.cod)

SST1INC         =       $(BUILD_ROOT_SST1)/include
CVGINC          =       $(BUILD_ROOT_CVG)/include
SWLIBSINC       =       $(BUILD_ROOT_SWLIBS)/include

# Different sublibs for simulator and hardware
# Default to hw and special case for simulators
ifeq ($(FX_GLIDE_HW), SST1)
SUBLIBRARIES    =       $(BUILD_ROOT_SST1)/lib/init.lib 
else
SUBLIBRARIES	=	$(BUILD_ROOT_CVG)/lib/cvghal.a
endif

ifeq ($(FX_GLIDE_DSP_TARGET), CSIM)
SUBLIBRARIES    =       $(BUILD_ROOT_CVG)/lib/cvgcsim.lib \
                        $(BUILD_ROOT_CVG)/lib/cvghal.lib
endif
ifeq ($(FX_GLIDE_DSP_TARGET), SIM)
SUBLIBRARIES    =       $(BUILD_ROOT_SWLIBS)/lib/fxmisc.lib \
                        $(BUILD_ROOT_SST1)/lib/util.lib \
                        $(BUILD_ROOT_SST1)/lib/gsim.lib \
                        $(BUILD_ROOT_SST1)/lib/sst.lib \
                        $(BUILD_ROOT_SST1)/lib/sst1init.lib \
                        $(BUILD_ROOT_SWLIBS)/lib/wing32.lib
endif

# targets
LDIRT           =       fxgasm.obj fxgasm.exe fxgasm.h
LIBRARIES       =       libglide2x.a
FX_DLL_LIBRARY  =       1
RCFILE          =       glide.rc

CFLAGS          =       $(LCINCS) $(LCDEFS) $(LCOPTS) $(OPTOPTS)
LIBPARTS 	= 	$(OBJECTS) $(SUBLIBRARIES)
.PHONY:	all clean

all: $(LIBRARIES) TAGS

clean:
	rm -rf $(OBJECTS) $(LIBRARIES)

$(LIBRARIES): $(OBJECTS) # $(SUBLIBRARIES)
	$(AR) -rcs $@ $(LIBPARTS)

$(OBJECTS): %.o : %.c $(HEADERS) $(PRIVATE_HEADERS) $(MAKEFILE)
	$(CC) -c $(CFLAGS) -o $@ $<

TAGS:  $(CFILES) $(HEADERS) $(PRIVATE_HEADERS) $(OTHER_FILES)
	etags $(CFILES) $(HEADERS) $(PRIVATE_HEADERS) $(OTHER_FILES)

gbanner.obj : banner.inc
gsplash.obj : splshdat.c

fxgasm.h: fxgasm.exe
	fxgasm -hex > fxgasm.h

glide.res: rcver.h
gglide.obj: rcver.h



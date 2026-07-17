#
# Copyright (c) 1997, 3Dfx Interactive, Inc.
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
# $Revision: 9 $
# $Date: 6/30/99 1:35p $
#

# define some commone stuff here
LCDEFS	= -DGDBG_INFO_ON -DBUILD_DIAGS -DHAL_CSIM -DH4
LCOPTS	=
LCINCS	= -I$(BUILD_ROOT)\h5\include -I..\lib
LLDOPTS =

DIAGS	= $(CFILES:.c=.exe)
LOGS	= $(CFILES:.c=.log)
BATS	= *.bat
LDIRT	= *.log $(LIBOBJS)

DATE	= $(DIAG_TOOLS)\date
ECHO	= $(DIAG_TOOLS)\echo

LLDLIBS	= $(LIBOBJS) ..\lib\diags.lib \
		$(BUILD_ROOT)\h5\lib\h4hal.lib \
		$(BUILD_ROOT_SWLIBS)\lib\fxmisc.lib
	
!include $(BUILD_ROOT_SWLIBS)/include/nmake/3dfx.mak

# udiag contains a structure that all the diags access
$(OBJECTS): ..\lib\udiag.h

$(DIAGS): $(LLDLIBS) $*.obj
	@$(ECHO) $@ making on \c > $*.log
	@$(DATE) >> $*.log
	$(LINK) -o $@ $*.obj $(LDLIBS) $(LDFLAGS)
	@$(ECHO) $@ made on \c > $*.log
	@$(DATE) >> $*.log

.SUFFIXES: .log

.FORCE:

testlist: .FORCE
	@$(ECHO) $(TARGETS)

testsum:
	@awk -f ../testsum.awk *.log

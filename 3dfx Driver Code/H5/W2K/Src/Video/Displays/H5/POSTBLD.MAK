# -----------------------------------------------------------------------------
# Copyright (c) 2000, 3Dfx Interactive, Inc.
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
# POSTBLD.MAK --- Final commands for Napalm W2K display driver builds.
# -----------------------------------------------------------------------------
!IFNDEF STRIP_SYMBOLS
STRIP_SYMBOLS=1
!ENDIF

all:
        @echo off
!IF $(STRIP_SYMBOLS) == 0
        if not exist obj$(BUILD_ALT_DIR)\I386\nms mkdir obj$(BUILD_ALT_DIR)\I386\nms
        copy obj$(BUILD_ALT_DIR)\I386\3dfxvs.dll obj$(BUILD_ALT_DIR)\I386\nms
        $(SIW2K)\nmsym.exe /translate:always,package /source:.;.\I386 obj$(BUILD_ALT_DIR)\I386\nms\3dfxvs.dll
!ENDIF
        if not exist obj$(BUILD_ALT_DIR)\I386\dll mkdir obj$(BUILD_ALT_DIR)\I386\dll
        rebase -B 0x10000 -X obj$(BUILD_ALT_DIR)\I386 obj$(BUILD_ALT_DIR)\I386\3dfxvs.dll
        copy obj$(BUILD_ALT_DIR)\I386\3dfxvs.dll obj$(BUILD_ALT_DIR)\I386\dll


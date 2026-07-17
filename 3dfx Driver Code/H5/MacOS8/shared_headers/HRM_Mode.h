/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Header: HRM_Mode.h, 4, 10/11/00 8:38:09 PM, Brent$
** $Log: 
**  4    3dfx      1.2.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  3    MacOS Dev Tree1.2         02/07/00 Kenneth Dyke    Added SLI/AA support.
**  2    MacOS Dev Tree1.1         01/31/00 Kenneth Dyke    Added prototypes for
**       PCI config register extensions.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 3     8/02/99 12:28p Kcd
** New SetPrefs/GetPrefs stuff.
** 
** 2     7/08/99 1:27p Kcd
** C++ support and graphics clock extension.
** 
** 1     7/02/99 3:29p Kcd
** Initial checkin.
**
*/

#ifndef __HRM_MODE_H__
#define __HRM_MODE_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

typedef FxBool (*hrmSetExclusiveModePtr)(hrmBoard_t *board);
typedef OSErr (*hrmSetVideoModePtr)(hrmBoard_t *board, int xRes, int yRes, int refresh);
typedef void (*hrmReleaseExclusiveModePtr)(hrmBoard_t *board);
typedef OSErr (*hrmSetPrefsPtr)(hrmBoard_t *board, FxU32 grxClock);
typedef FxU32 (*hrmGetPrefsPtr)(hrmBoard_t *board);
typedef OSErr (*hrmSetModeFlagsPtr)(hrmBoard_t *board, FxU32 displayModeID, FxU32 timingFlags);
typedef OSErr (*hrmWriteConfigRegisterPtr)(hrmBoard_t *board, unsigned long deviceID, unsigned long offset, unsigned long value);
typedef OSErr (*hrmReadConfigRegisterPtr)(hrmBoard_t *board, unsigned long deviceID, unsigned long offset, unsigned long *value);
typedef void (*hrmGetSlaveRegsPtr)(hrmBoard_t *board, FxU32 chipNumber, FxU32 *regs);
typedef void (*hrmSLIAAPtr)(hrmBoard_t *board, hrmSLIAAChipInfo_t *chipInfo, hrmSLIAAMemInfo_t *memInfo);

// Function Declarations
FxBool hrmSetExclusiveMode(hrmBoard_t *board);
OSErr hrmSetVideoMode(hrmBoard_t *board, int xRes, int yRes, int refresh);
void hrmReleaseExclusiveMode(hrmBoard_t *board);
OSErr hrmSetPrefs(hrmBoard_t *board, FxU32 prefs);
FxU32 hrmGetPrefs(hrmBoard_t *board);
OSErr hrmSetModeFlags(hrmBoard_t *board, unsigned long displayModeID, unsigned long timingFlags);
OSErr hrmWriteConfigRegister(hrmBoard_t *board, unsigned long deviceID, unsigned long offset, unsigned long value);
OSErr hrmReadConfigRegister(hrmBoard_t *board, unsigned long deviceID, unsigned long offset, unsigned long *value);
void hrmGetSlaveRegs(hrmBoard_t *board, FxU32 chipNumber, FxU32 *regs);
void hrmSLIAA(hrmBoard_t *board, hrmSLIAAChipInfo_t *chipInfo, hrmSLIAAMemInfo_t *memInfo);

#ifdef __cplusplus
}
#endif

#endif /* __HRM_MODE_H */

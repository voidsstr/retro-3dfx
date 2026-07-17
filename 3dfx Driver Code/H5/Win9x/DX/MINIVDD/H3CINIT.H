/* -*-c++-*- */
/* $Header: h3cinit.h, 3, 10/11/00 8:54:20 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   h3cinit.h
**
** Description: Macros & function definitions for Banshee/Avenger.
**
** $Revision: 3$
** $Date: 10/11/00 8:54:20 PM$
**
** $History: h3cinit.h $
** 
** *****************  Version 26  *****************
** User: Dalev        Date: 8/20/99    Time: 4:23p
** Updated in $/devel/h5/Win9x/dx/minivdd
** h3InitSetVideoMode() now uses mode values from dfptable.h if a
** non-scaling DFP is detected.
** 
** *****************  Version 25  *****************
** User: Andrew       Date: 8/05/99    Time: 1:02p
** Updated in $/devel/h5/Win9x/dx/minivdd
** added a parameter to h3InitSgram 
** 
** *****************  Version 24  *****************
** User: Edwin        Date: 6/29/99    Time: 3:52p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Remove obsolete Banshee ifdefs.
** 
** *****************  Version 22  *****************
** User: Kcd          Date: 5/26/99    Time: 5:49p
** Updated in $/devel/h3/cinit
** MacOS stuff.
** 
** *****************  Version 21  *****************
** User: Michael      Date: 1/05/99    Time: 8:45a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
**
*/


#ifndef __H3CINIT_H__
#define __H3CINIT_H__

#include <3dfx.h>

FxU32				// return # of MB of memory
h3InitGetMemSize(FxU32 regBase);// init register base

FxU32                           // return # of MB of memory
h3InitSgram(FxU32 regBase,      // init iegister base
            FxU32 sgramMode,
            FxU32 sgramMask,
            FxU32 sgramColor,
#ifdef H3VDD
            FxU32 dwVendorDeviceID,
#endif
	    char *vendorName);  // NULL or name of SGRAM vendor

void
h4InitPlls(FxU32 regBase,		// init register base
           FxU32 deviceID,		// H4 or H4_OEM
           FxU32 grxSpeedInMHz);	// desired clock frequency (MHz)

void 
h3InitVga(
    FxU32 regBase,              // memory base address
    FxU32 legacyDecode);        // 1=enable VGA decode, 0=disable

void
h3InitVideoProc(
    FxU32 regBase,              // memory base address
    FxU32 vidProcCfg);		// vidProcCfg register control bits

FxU16 *
h3InitFindVideoMode(FxU32 xRes,
                    FxU32 yRes,
                    FxU32 refresh);

FxU16 *
h3InitFindDFPMode(FxU32 xRes,
                    FxU32 yRes,
                    FxU32 refresh);

FxBool
h3InitSetVideoMode(
    PDEVTABLE pDev,
    FxU32 xRes,                 // x resolution
    FxU32 yRes,                 // y resolution
    FxU32 refresh,              // refresh freq
#if defined(H3VDD)
  FxU32 loadClut, // really a bool, should we load the lookup table
  FxU32 scanlinedouble); // set scanline double bit and double y?
#else
    FxU32 loadClut) ;		// initialize clut entries?
#endif

void
h3InitVideoDesktopSurface(
    FxU32 regBase,
    FxU32 enable,		// 1=enable desktop surface (DS), 1=disable
    FxU32 tiled,		// 0=DS linear, 1=tiled
    FxU32 pixFmt,		// pixel format of DS
    FxU32 clutBypass,		// bypass clut for DS?
    FxU32 clutSelect,		// 0=lower 256 CLUT entries, 1=upper 256
    FxU32 startAddress,		// board address of beginning of DS
    FxU32 stride);		// distance between scanlines of the DS, in
				// units of bytes for linear DS's and tiles for
				// tiled DS's

void
h3InitVideoOverlaySurface(
    FxU32 regBase,
    FxU32 enable,		// 1=enable Overlay surface (OS), 1=disable
    FxU32 stereo,		// 1=enable OS stereo, 0=disable
    FxU32 horizScaling,		// 1=enable horizontal scaling, 0=disable
    FxU32 dudx,			// horizontal scale factor (ignored if not
				// scaling)
    FxU32 verticalScaling,	// 1=enable vertical scaling, 0=disable
    FxU32 dvdy,			// vertical scale factor (ignored if not
				// scaling)
    FxU32 filterMode,		// duh
    FxU32 tiled,		// 0=OS linear, 1=tiled
    FxU32 pixFmt,		// pixel format of OS
    FxU32 clutBypass,		// bypass clut for OS?
    FxU32 clutSelect,		// 0=lower 256 CLUT entries, 1=upper 256
    FxU32 startAddress,		// board address of beginning of OS
    FxU32 stride);		// distance between scanlines of the OS, in
				// units of bytes for linear OS's and tiles for
				// tiled OS's

#ifndef H3VDD
void
h3InitMeasureSiProcess(
    FxU32 regBase);             // init register base
#endif // #ifndef H3VDD

void
h3InitBlockWrite(
    FxU32 regBase,
    FxU32 enable,		// 1=enable block writes, 0=disable
    FxU32 threshhold);		// block write threshhold

void
h3InitResetAll(
    FxU32 regBase);             // init register base

#endif /*  __H3CINIT_H__ */


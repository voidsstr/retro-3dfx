/* -*-c++-*- */
/* $Header: dibblt.c, 19, 10/11/00 8:54:11 PM, Brent$ */
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
** File name:   dibblt.c
**
** Description: GDI DibBlt functions on the 32-bit side.
**
** $Revision: 19$
** $Date: 10/11/00 8:54:11 PM$
**
** $History: dibblt.c $
** 
** *****************  Version 58  *****************
** User: Andrew       Date: 6/15/99    Time: 5:08p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Change _HWC_EXCLUSIVE to SKIP_FLAGS so that path can be reused with
** SLI_AA_SLAVE mode
** 
** *****************  Version 56  *****************
** User: Stb_shanna   Date: 3/19/99    Time: 5:08p
** Updated in $/devel/h3/win95/dx/minivdd
** Fixed PRS 5222 - The fix for PRS 5207 did not account for all of the
** places where Host2Scrn4to2432ConvBlt was used so an additional check
** was added where needed.
** 
** *****************  Version 55  *****************
** User: Stb_pzheng   Date: 3/18/99    Time: 5:00p
** Updated in $/devel/h3/win95/dx/minivdd
** Fixed PRS #5207. In 24bpp and 32bpp modes, punt 4bpp DIB to device
** convert blt if the bitmap size is bigger than 1024 pixels(4K bytes)
** 
** *****************  Version 54  *****************
** User: Stb_pzheng   Date: 3/12/99    Time: 10:37p
** Updated in $/devel/h3/win95/dx/minivdd
** Changed the way to reference bmiColors field to use bmiHeader.biSize in
** stead of relying on compiler. PRS # 4604
** 
** *****************  Version 53  *****************
** User: Edwin        Date: 2/16/99    Time: 12:46p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix 4283, we do not handle RLE compressed source in FxStretchDIBits's
** DIB_08TOxx path.
** 
** *****************  Version 52  *****************
** User: Stb_srogers  Date: 2/09/99    Time: 12:20p
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 51  *****************
** User: Cwilcox      Date: 2/03/99    Time: 5:16p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added missing 32bpp source cases to fxStretchDIBits.
** 
** *****************  Version 50  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 8:02a
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 49  *****************
** User: Andrew       Date: 1/20/99    Time: 4:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Code to handle 555 when in 565 mode to fix PRS 3748
** 
** *****************  Version 48  *****************
** User: Michael      Date: 1/05/99    Time: 7:50a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 47  *****************
** User: Michael      Date: 12/24/98   Time: 10:27a
** Updated in $/devel/h3/Win95/dx/minivdd
** ChrisE's (igx) optimization for FxStretchDIBits.  High-End Graphics
** WB9x tests have all 8bpp StretchDIB calls as one-to-one stretches.
** Test for this case and perform a screen blt.  For High-End Graphics,
** gains 26 Winmarks.  (About a 15.2% increase on my PII-300.)
** 
** *****************  Version 46  *****************
** User: Michael      Date: 12/19/98   Time: 7:55a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix problem in Host2Scrn8To16ConvBlt where last pixel in a
** scanline was getting set to the incorrect value.  Fixes PRS 3806.
** 
** *****************  Version 45  *****************
** User: Michael      Date: 11/24/98   Time: 12:47p
** Updated in $/devel/h3/Win95/dx/minivdd
** MarkL's (igx) fix for 3313.  Clipping was not getting set in
** LoadNativeDIBScan.  In Host2Scrn16to16StrBlt, moved where clipping was
** getting set due to modifications to LoadNativeDIBScan.
** 
** *****************  Version 44  *****************
** User: Michael      Date: 11/03/98   Time: 2:58p
** Updated in $/devel/h3/Win95/dx/minivdd
** bmiHeader.biHeight was incorrectly cast in all occurences.  Changed
** cast to long to be able to test for negative conditions.  Also handle
** problem when number of scans (cScan) doesn't match biHeight in
** FxDeviceBitmapBits.
** 
** *****************  Version 43  *****************
** User: Michael      Date: 11/02/98   Time: 2:03p
** Updated in $/devel/h3/Win95/dx/minivdd
** MarkL's (igx) fix for 2592.  SrcFormat incorrectly computed using
** PixelsInScanline.  Changed to use SrcDataPitch.
** 
** *****************  Version 42  *****************
** User: Michael      Date: 10/06/98   Time: 12:12p
** Updated in $/devel/h3/Win95/dx/minivdd
** Edwin found a bug in Andretti Racing caused by the previous fix to
** Host2Scrn16to16StrBlt.  This was fixed by calculating startOffset using
** srcH in Host2Scrn16to16StrBlt.  Fixes Andretti Racing hang, 2240, 2744,
** 2940 & 2982.
** 
** *****************  Version 41  *****************
** User: Michael      Date: 10/01/98   Time: 2:57p
** Updated in $/devel/h3/Win95/dx/minivdd
** Two fixes:  In FxStretchDIBits, punt all non-zero based dibs.  The
** routines were never designed to work for non-zero src offsets, as the
** code was taken from another original DIB routine where we checked for
** iScan==0, which is effectively the same thing.  Second, in
** Host2Scrn16to16StrBlt, modify startOffset to use DstH rather than
** height from lpSrc.  Since dibs are drawn bottom-up, using src was
** incorrect.  Fixes PRS 2240, 2744, and 2940.
** 
** *****************  Version 40  *****************
** User: Michael      Date: 8/21/98    Time: 12:55p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add MarkL's (igx) Host2Scrn8To16ConvBlt functionality.  Needed to
** fix #1209.
** 
** *****************  Version 39  *****************
** User: Andrew       Date: 7/17/98    Time: 3:19p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Quick Exit if HWC Exclusive is set
** 
** *****************  Version 38  *****************
** User: Ken          Date: 7/16/98    Time: 1:22p
** Updated in $/devel/h3/win95/dx/minivdd
** optimized thunk16.asm, and tweaked dibblt.c and strchblt.c for Mercury
** 
** *****************  Version 37  *****************
** User: Ken          Date: 7/09/98    Time: 11:55a
** Updated in $/devel/h3/win95/dx/minivdd
** added some NULLDRIVER ability as well as a performance measuring 
** technique PERFNOP (PERFNOP was here before, I'm just adding it
** officially to the makefile).
** 
** *****************  Version 36  *****************
** User: Michael      Date: 7/09/98    Time: 6:42a
** Updated in $/devel/h3/Win95/dx/minivdd
** ChrisEs (IGX) modification for original work he did on FxStretchDIBits.
** Fixes problems with spurious hangs, fails disney.com, hangs on mode
** change.
** 
** *****************  Version 35  *****************
** User: Andrew       Date: 6/25/98    Time: 4:31p
** Updated in $/devel/h3/Win95/dx/minivdd
** Bugs fixes for 1989.  May also fix instability.  With Host16to16
** commented in Start button text is upside down.
** 
** *****************  Version 34  *****************
** User: Michael      Date: 6/24/98    Time: 7:49p
** Updated in $/devel/h3/Win95/dx/minivdd
** Jump around ChirsE's (IGX) Host2Screen16to16StrBlt function in
** FxStretchDIBlt.  This is presently cause a great deal of driver
** instability.
** 
** *****************  Version 33  *****************
** User: Michael      Date: 6/23/98    Time: 11:12a
** Updated in $/devel/h3/Win95/dx/minivdd
** Minor fix for ChrisE's (IGX) previous fix.  Win 9x was crashing when
** selecting the 'Start' button at 8-, 24-, and 32-bpp.
** 
** *****************  Version 32  *****************
** User: Michael      Date: 6/22/98    Time: 7:15a
** Updated in $/devel/h3/Win95/dx/minivdd
** ChrisE's (IGX) addition of FxStretchDIBits and supporting
** functionality.  Fixes 1893.
** 
** *****************  Version 31  *****************
** User: Michael      Date: 6/04/98    Time: 5:47p
** Updated in $/devel/h3/Win95/dx/minivdd
** Chris E's (IGX) added code to each Host2Scrn routine to correctly
** detect when a DIB is completely clipped (ie: not visible).   Fixes
** 1631.
** 
** *****************  Version 30  *****************
** User: Andrew       Date: 6/04/98    Time: 8:27a
** Updated in $/devel/h3/Win95/dx/minivdd
** Wrong Defect # changed 1836 to 1703
** 
** *****************  Version 29  *****************
** User: Andrew       Date: 6/04/98    Time: 8:23a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed defeect 1703 by fixing a clipping problem
** 
** *****************  Version 28  *****************
** User: Andrew       Date: 5/20/98    Time: 12:31p
** Updated in $/devel/h3/Win95/dx/minivdd
** Change CLIP rect for FXENTER to be left, top, right, bottom.
** 
** *****************  Version 27  *****************
** User: Andrew       Date: 5/19/98    Time: 6:10p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed FXENTER rect to be left,top,right,bottom instead of left,top,
** width, height
** 
** *****************  Version 26  *****************
** User: Michael      Date: 5/15/98    Time: 4:31p
** Updated in $/devel/h3/Win95/dx/minivdd
** Backed out my previous changes.
** 
** *****************  Version 25  *****************
** User: Michael      Date: 5/15/98    Time: 9:27a
** Updated in $/devel/h3/Win95/dx/minivdd
** Add lpDst to FXPUNT and FXLEAVE_NOHWACCESS.
** 
** *****************  Version 24  *****************
** User: Ken          Date: 5/14/98    Time: 8:31a
** Updated in $/devel/h3/win95/dx/minivdd
** added a PERFNOP case to gdi32.   change the "undef" to a "define"
** in fifomgr.h, and enable in softice (doperfnop = 1).  Replaces every
** write
** to the command register with a "0"
** 
** *****************  Version 23  *****************
** User: Ken          Date: 4/27/98    Time: 5:35p
** Updated in $/devel/h3/win95/dx/minivdd
** complete (well, almost) enter/leave implementation for gdi32
** 
** *****************  Version 22  *****************
** User: Michael      Date: 4/22/98    Time: 2:34p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix bug of garbage showing up on right edge of device bitmaps by
** initializing local variables that get used.
** 
** *****************  Version 21  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
** 
** *****************  Version 20  *****************
** User: Ken          Date: 4/15/98    Time: 10:39a
** Updated in $/devel/h3/win95/dx/minivdd
** 24 and 32bpp 4-bit source dib conversion routine added
**
*/

// STB Begin Change
#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif
// STB End Change

#include "h3.h"
#include "thunk32.h"
#include "h3g.h"
 
#include "dibblt.h"
#include "strchblt.h"   // for stretching offscreen DIB buffer to screen

#include "entrleav.h"

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))


// variables to turn on and off DIB info messages.
int DBMBITSDbg  = 1;
int SETDIBTDDbg = 1;
int STRDIBITSbg = 0;

// The following tables were taken from COLINFO.ASM in DD16

// This table contains 5 bit values for use in returning the nearest physical
// match to the 24 bit logical value, we use this to quickly replicate what the
// DIB_ColorInfo function would have returned.

BYTE ByteTo5BitLookup[] = {    0x000, 0x000, 0x000, 0x000, 
   0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
   0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001,  // 0c thru 13
   0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002,  // 14 thru 1b
   0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003,  // 1c thru 23
   0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,  // 24 thru 2b
   0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005,  // 2c thru 33
   0x006, 0x006, 0x006, 0x006, 0x006, 0x006, 0x006, 0x006,  // 34 thru 3b
   0x007, 0x007, 0x007, 0x007, 0x007, 0x007, 0x007, 0x007,  // 3c thru 43
   0x008, 0x008, 0x008, 0x008, 0x008, 0x008, 0x008, 0x008,  // 44 thru 4b
   0x009, 0x009, 0x009, 0x009, 0x009, 0x009, 0x009, 0x009,  // 4c thru 53
   0x00a, 0x00a, 0x00a, 0x00a, 0x00a, 0x00a, 0x00a, 0x00a,  // 54 thru 5b
   0x00b, 0x00b, 0x00b, 0x00b, 0x00b, 0x00b, 0x00b, 0x00b,  // 5c thru 63
   0x00c, 0x00c, 0x00c, 0x00c, 0x00c, 0x00c, 0x00c, 0x00c,  // 64 thru 6b
   0x00d, 0x00d, 0x00d, 0x00d, 0x00d, 0x00d, 0x00d, 0x00d,  // 6c thru 73
   0x00e, 0x00e, 0x00e, 0x00e, 0x00e, 0x00e, 0x00e, 0x00e,  // 74 thru 7b
   0x00f, 0x00f, 0x00f, 0x00f, 0x00f, 0x00f, 0x00f, 0x00f,  // 7c thru 83
   0x010, 0x010, 0x010, 0x010, 0x010, 0x010, 0x010, 0x010,  // 84 thru 8b
   0x011, 0x011, 0x011, 0x011, 0x011, 0x011, 0x011, 0x011,  // 8c thru 93
   0x012, 0x012, 0x012, 0x012, 0x012, 0x012, 0x012, 0x012,  // 94 thru 9b
   0x013, 0x013, 0x013, 0x013, 0x013, 0x013, 0x013, 0x013,  // 9c thru a3
   0x014, 0x014, 0x014, 0x014, 0x014, 0x014, 0x014, 0x014,  // a4 thru ab
   0x015, 0x015, 0x015, 0x015, 0x015, 0x015, 0x015, 0x015,  // ac thru b3
   0x016, 0x016, 0x016, 0x016, 0x016, 0x016, 0x016, 0x016,  // b4 thru bb
   0x017, 0x017, 0x017, 0x017, 0x017, 0x017, 0x017, 0x017,  // bc thru c3
   0x018, 0x018, 0x018, 0x018, 0x018, 0x018, 0x018, 0x018,  // c4 thru cb
   0x019, 0x019, 0x019, 0x019, 0x019, 0x019, 0x019, 0x019,  // cc thru d3
   0x01a, 0x01a, 0x01a, 0x01a, 0x01a, 0x01a, 0x01a, 0x01a,  // d4 thru db
   0x01b, 0x01b, 0x01b, 0x01b, 0x01b, 0x01b, 0x01b, 0x01b,  // dc thru e3
   0x01c, 0x01c, 0x01c, 0x01c, 0x01c, 0x01c, 0x01c, 0x01c,  // e4 thru eb
   0x01d, 0x01d, 0x01d, 0x01d, 0x01d, 0x01d, 0x01d, 0x01d,  // ec thru f3
   0x01e, 0x01e, 0x01e, 0x01e, 0x01e, 0x01e, 0x01e, 0x01e,  // f4 thru fb
   0x01f, 0x01f, 0x01f, 0x01f                               // fc thru ff
};

// This table contains 6 bit values for use in returning the nearest physical
// match to the 24 bit logical value, we use this to quickly replicate what the
// DIB_ColorInfo function would have returned.

BYTE ByteTo6BitLookup[] = { 0x000, 0x000, 
    0x000, 0x000, 0x000, 0x000, 
    0x001, 0x001, 0x001, 0x001, // 06 thru 09
    0x002, 0x002, 0x002, 0x002, // 0a thru 0d
    0x003, 0x003, 0x003, 0x003, // 0e thru 11
    0x004, 0x004, 0x004, 0x004, // 12 thru 15
    0x005, 0x005, 0x005, 0x005, // 16 thru 19
    0x006, 0x006, 0x006, 0x006, // 1a thru 1d
    0x007, 0x007, 0x007, 0x007, // 1e thru 21
    0x008, 0x008, 0x008, 0x008, // 22 thru 25
    0x009, 0x009, 0x009, 0x009, // 26 thru 29
    0x00a, 0x00a, 0x00a, 0x00a, // 2a thru 2d
    0x00b, 0x00b, 0x00b, 0x00b, // 2e thru 31
    0x00c, 0x00c, 0x00c, 0x00c, // 32 thru 35
    0x00d, 0x00d, 0x00d, 0x00d, // 36 thru 39
    0x00e, 0x00e, 0x00e, 0x00e, // 3a thru 3d
    0x00f, 0x00f, 0x00f, 0x00f, // 3e thru 41
    0x010, 0x010, 0x010, 0x010, // 42 thru 45
    0x011, 0x011, 0x011, 0x011, // 46 thru 49
    0x012, 0x012, 0x012, 0x012, // 4a thru 4d
    0x013, 0x013, 0x013, 0x013, // 4e thru 51
    0x014, 0x014, 0x014, 0x014, // 52 thru 55
    0x015, 0x015, 0x015, 0x015, // 56 thru 59
    0x016, 0x016, 0x016, 0x016, // 5a thru 5d
    0x017, 0x017, 0x017, 0x017, // 5e thru 61
    0x018, 0x018, 0x018, 0x018, // 62 thru 65
    0x019, 0x019, 0x019, 0x019, // 66 thru 69
    0x01a, 0x01a, 0x01a, 0x01a, // 6a thru 6d
    0x01b, 0x01b, 0x01b, 0x01b, // 6e thru 71
    0x01c, 0x01c, 0x01c, 0x01c, // 72 thru 75
    0x01d, 0x01d, 0x01d, 0x01d, // 76 thru 79
    0x01e, 0x01e, 0x01e, 0x01e, // 7a thru 7d
    0x01f, 0x01f, 0x01f, 0x01f, // 7e thru 81
    0x020, 0x020, 0x020, 0x020, // 82 thru 85
    0x021, 0x021, 0x021, 0x021, // 86 thru 89
    0x022, 0x022, 0x022, 0x022, // 8a thru 8d
    0x023, 0x023, 0x023, 0x023, // 8e thru 91
    0x024, 0x024, 0x024, 0x024, // 92 thru 95
    0x025, 0x025, 0x025, 0x025, // 96 thru 99
    0x026, 0x026, 0x026, 0x026, // 9a thru 9d
    0x027, 0x027, 0x027, 0x027, // 9e thru a1
    0x028, 0x028, 0x028, 0x028, // a2 thru a5
    0x029, 0x029, 0x029, 0x029, // a6 thru a9
    0x02a, 0x02a, 0x02a, 0x02a, // aa thru ad
    0x02b, 0x02b, 0x02b, 0x02b, // ae thru b1
    0x02c, 0x02c, 0x02c, 0x02c, // b2 thru b5
    0x02d, 0x02d, 0x02d, 0x02d, // b6 thru b9
    0x02e, 0x02e, 0x02e, 0x02e, // ba thru bd
    0x02f, 0x02f, 0x02f, 0x02f, // be thru c1
    0x030, 0x030, 0x030, 0x030, // c2 thru c5
    0x031, 0x031, 0x031, 0x031, // c6 thru c9
    0x032, 0x032, 0x032, 0x032, // ca thru cd
    0x033, 0x033, 0x033, 0x033, // ce thru d1
    0x034, 0x034, 0x034, 0x034, // d2 thru d5
    0x035, 0x035, 0x035, 0x035, // d6 thru d9
    0x036, 0x036, 0x036, 0x036, // da thru dd
    0x037, 0x037, 0x037, 0x037, // de thru e1
    0x038, 0x038, 0x038, 0x038, // e2 thru e5
    0x039, 0x039, 0x039, 0x039, // e6 thru e9
    0x03a, 0x03a, 0x03a, 0x03a, // ea thru ed
    0x03b, 0x03b, 0x03b, 0x03b, // ee thru f1
    0x03c, 0x03c, 0x03c, 0x03c, // f2 thru f5
    0x03d, 0x03d, 0x03d, 0x03d, // f6 thru f9
    0x03e, 0x03e, 0x03e, 0x03e, // fa thru fd
    0x03f, 0x03f                // fe thru ff
};

#ifdef PERF_NEWDIB
// RGB565ColorTable is where the 16-bit palette will be stored when converted
// from 32bpp in Host2Scrn8To16ConvBlt
WORD RGB565ColorTable[256]; 
#endif

// this table is used to create a byte sized code for a switch table
// based on the pixel format of the source and the destination
BYTE LookupPixCode[ ] = 
{
    BPP00,BPP01,BPP00,BPP00,
    BPP04,BPP00,BPP00,BPP00,
    BPP08,BPP00,BPP00,BPP00,
    BPP00,BPP00,BPP00,BPP16,
    BPP16,BPP00,BPP00,BPP00,
    BPP00,BPP00,BPP00,BPP00,
    BPP24,BPP00,BPP00,BPP00,
    BPP00,BPP00,BPP00,BPP00,
    BPP32
};

// #pragma optimize("",on) 

// 3d nop command

#define SSTCP_PKT4_NOPCMD  ((0x48 << SSTCP_REGBASE_SHIFT))

#define CMDFIFO_BUILD_PK4( mask, regName, chip )\
    ( ((mask) << SSTCP_PKT4_MASK_SHIFT) | (chip << 11)\
    | (regName) | SSTCP_PKT4 )


// this macro is used to fix the lockup problems with web browsing,
// search for 3871 for description (see 4457 now)

#ifdef CMDFIFO
#define WAXBUG_3DNOPFIX \
  { \
    CMDFIFO_CHECKROOM(cf, 2); \
    SETPH( cf, CMDFIFO_BUILD_PK4(1, SSTCP_PKT4_NOPCMD, 0) ); \
    SET(cf, ghw3D->nopCMD, 0); \
    BUMP(2); \
    CMDFIFO_EPILOG(cf); \
  }
#else
#define WAXBUG_3DNOPFIX \
  { \
    SstRegs * ghw3D = (SstRegs*)(_FF(regBase[HWINFO_SST_3DREGS_INDEX]));\
    CMDFIFO_CHECKROOM(cf, 2); \
    SETPH( cf, CMDFIFO_BUILD_PK4(1, SSTCP_PKT4_NOPCMD, 0) ); \
    SET(cf, ghw3D->nopCMD, 0); \
    BUMP(2); \
    CMDFIFO_EPILOG(cf); \
  }
#endif

#ifdef NULLDRIVER
extern FxU32 nullAll, nullDevBit;
#endif // #ifdef NULLDRIVER

#ifdef PERF_NEWDIB
/*----------------------------------------------------------------------
Function name:  FxDeviceBitmapBits

Description:    Main 32-bit entry point for GDI DeviceBitmapBits
                function called from the 16-bit side.
Information:

Return:         BOOL     1 if success or,
                        -1 if failure.
----------------------------------------------------------------------*/
BOOL
FxDeviceBitmapBits(DeviceBitmapBitsParams *lp)

//  DIBENGINE FAR  *lpBitmap,   
//  WORD            fGet, 
//  WORD            iStart, 
//  WORD            cScans, 
//  LPVOID          lpDIBits,   
//  LPBITMAPINFO    lpBitmapInfo, 
//  LPDRAWMODE      lpDrawMode,
//  LPINT           lpTranslate)

{
    LPDIBENGINE     lpDst;
    LPBITMAPINFO    lpSrc;
    DWORD         * lpSrcData;
    LONG            retval;
    DWORD           lpTmp16;      // tmp 16:16 pointer
    DWORD         * ColorTable;   // Pointer to DIB color conversion table
    DWORD         * fillptr;
    int             i;
    int             DwordsToFill;      
    
    DEBUG_FIX;

    retval = lp->cScans;
    FarToFlat(lp->lpBitmap, lpDst);

#ifdef NULLDRIVER
    if (nullAll || nullDevBit)
    goto null_return;
#endif // #ifdef NULLDRIVER

    if (_FF(gdiFlags) & SKIP_FLAGS)
         return retval;

    // If this is a GET instead of a SET, PUNT
    if (lp->fGet == 1)
      FXPUNT(FXPUNT_NORMAL);
    
    // PUNT if 1, 4, or 8bpp destination
    if (lpDst->deBitsPixel < 15) 
      FXPUNT(FXPUNT_NORMAL);

    // PUNT if Starting scan is not first scan of DIB
    if (lp->iStart) 
      FXPUNT(FXPUNT_NORMAL);

    FarToFlat(lp->lpBitmapInfo, lpSrc);

	if ((long)(lpSrc->bmiHeader.biHeight == 0))
      FXPUNT(FXPUNT_NORMAL);

    // We don't handle when number of scans
    // doesn't match DIB's height
    if (lp->cScans != lpSrc->bmiHeader.biHeight)
        FXPUNT(FXPUNT_NORMAL);
        
    // If clipped, PUNT
    if ((lpDst->deWidth  != lpSrc->bmiHeader.biWidth ) ||
        (lpDst->deHeight != lpSrc->bmiHeader.biHeight)   )
    {
      FXPUNT(FXPUNT_NORMAL);
    }
        
    // BLACK OUTPUT DIBITMAPBITS OPTIMIZATION CHECK
    // Look for 16bpp dest special case for Winbench --
    // Color palette is 8 entries, all black.  This simplifies to a
    // black solidfill.

//PingZ 3/12/99 lpSrc->bmiColors may not be pointing to the actual color table!!!  The only
//to guarantee that is to add bmiHeader.biSize to lpSrc.  This is to fix PRS #4604
//    ColorTable = (DWORD *)lpSrc->bmiColors;
    ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;

    if (lpDst->deBitsPixel == 16) {
        if (lpSrc->bmiHeader.biClrUsed == 8) {
            if ((ColorTable[0]  &
                 ColorTable[1]  &
                 ColorTable[2]  &
                 ColorTable[3]  &
                 ColorTable[4]  &
                 ColorTable[5]  &
                 ColorTable[6]  &
                 ColorTable[7]) == 0)  {
                
                // This is an optimization candidate.  Fill with black.
                
                lpTmp16= ((DWORD)(lpDst->deBitsSelector)) << 16;
                FarToFlat(lpTmp16, fillptr)
                (DWORD)fillptr += lpDst->deBitsOffset;
                
                DwordsToFill = ((lpDst->deWidthBytes>>2)*lp->cScans);
                
                for (i=0; i < DwordsToFill; i++) {
                    *fillptr++ = 0;
                }
                
               FXPUNT(FXPUNT_NORMAL);
            }    
        }
    }
    
    // If we have to deal with wierd compression, PUNT
    if (lpSrc->bmiHeader.biCompression != BI_RGB) 
      FXPUNT(FXPUNT_NORMAL);
    
    switch(lpSrc->bmiHeader.biBitCount)
    {
      case 1:                             // 1  -> 8,16,24
      // Host2ScrnMonoBlt(...........);
      FXPUNT(FXPUNT_NORMAL);
      break;
              
      case 4:                             // 4  -> 16,24
      FarToFlat(lp->lpDIBits, lpSrcData);
      if (lpDst->deBitsPixel == 16)
          Host2Scrn4to16ConvBlt( lpDst,
                        (DWORD)NULL,
                        lp->cScans,
                        (int)0,
                        (int)0,
                        (int)0,
                        (int)0,
                        lpSrcData,
                        lpSrc,
                        (DWORD)NULL,
                        (DWORD)NULL,
                        SRCCOPY );
      else
          if(!Host2Scrn4to2432ConvBlt( lpDst,
                        (DWORD)NULL,
                        lp->cScans,
                        (int)0,
                        (int)0,
                        (int)0,
                        (int)0,
                        lpSrcData,
                        lpSrc,
                        (DWORD)NULL,
                        (DWORD)NULL,
                        SRCCOPY ))
          {
            FXPUNT(FXPUNT_NORMAL);
            break;
          }
                  
      break;
              
      case 8:                             // 8  -> 16,24
      FarToFlat(lp->lpDIBits, lpSrcData);
      if (lpDst->deBitsPixel == 16)
          Host2Scrn8To16ConvBlt( lpDst,
                        (DWORD)NULL,
                        lp->cScans,
                        (int)0,
                        (int)0,
                        lpSrcData,
                        lpSrc );                        
      else
          Host2Scrn8bppConvBlt( lpDst,
                        (DWORD)NULL,
                        lp->cScans,
                        (int)0,
                        (int)0,
                        lpSrcData,
                        lpSrc );  
      break;
                  
      case 16:                            // 16 -> 24
        FXPUNT(FXPUNT_NORMAL);
        break;
      
      case 24:                            // 24 -> 16
          // This routine will set up the
          // hardware with a dstFormat AND a srcFormat, and just ship the
          // data straight to the HW to let it do the conversion.
          // Host2ScrnColorConvBlt(......);
          FXPUNT(FXPUNT_NORMAL);
          break;
    
      default:
        return(-1);             // Fail if invalid bitcount
      break;
    }

#ifdef NULLDRIVER
null_return:
#endif // #ifdef NULLDRIVER

    FXLEAVE_NOHWACCESS("FxDeviceBitmapBits", retval);
}


/*----------------------------------------------------------------------
Function name:  FxSetDIBitsToDevice

Description:    Main 32-bit entry point for GDI SetDIBitsToDevice
                function called from the 16-bit side.
Information:
    In this routine, we know the destination is the device.
    This leaves us with the following conversions, if we blow
    off handling 1,4, and 8 bit destinations.
        1 -> 16/24
        4 -> 16/24
        8 -> 16/24
        24-> 16

    ### Heard a rumor that 16bpp is now a supported DIB format.
    ### Check on this.

Return:         BOOL     1 if success or,
                        -1 if failure.
----------------------------------------------------------------------*/
BOOL
FxSetDIBitsToDevice(SetDIBitsToDeviceParams *lpParams)

//  DIBENGINE FAR  *lpDestDev, 
//  WORD            X, 
//  WORD            Y, 
//  WORD            iScan, 
//  WORD            cScans, 
//  RECT FAR       *lpClipRect, 
//  DRAWMODE FAR   *lpDrawMode, 
//  LPVOID          lpDIBits, 
//  BITMAPINFO FAR *lpBitmapInfo, 
//  LPINT           lpTranslate) 

{
    LPDIBENGINE   lpDst;
    LPBITMAPINFO  lpSrc;
    DWORD      *  lpSrcData;
    LONG retval;

    DEBUG_FIX;

    retval = lpParams->cScans;

    FarToFlat(lpParams->lpDestDev, lpDst);

    // quick exit if in exclusive mode
    if (_FF(gdiFlags) & SKIP_FLAGS)
         return retval;

    // PUNT if 1, 4, or 8bpp destination
    if (lpDst->deBitsPixel < 15) 
      FXPUNT(FXPUNT_NORMAL);
    
    // PUNT if Starting scan is not first scan of DIB
    if (lpParams->iScan) 
      FXPUNT(FXPUNT_NORMAL);
    
    FarToFlat(lpParams->lpBitmapInfo, lpSrc);

	if ((long)(lpSrc->bmiHeader.biHeight == 0))
      FXPUNT(FXPUNT_NORMAL);
    

    // *********  REEXAMINE THIS PUNT FOR POSSIBLE SUPPORT ************
    // Punt negative destination coordinates
    //if (lpParams->X < (WORD)0 || lpParams->Y < (WORD)0) {
    //    bPunt = TRUE;
    //    goto SDIBTDUseDibEngine;
    //}
    
    // If we have to deal with wierd compression, PUNT
    if (lpSrc->bmiHeader.biCompression != BI_RGB) 
      FXPUNT(FXPUNT_NORMAL);

    switch (lpSrc->bmiHeader.biBitCount)
    {
      case 1:                             // 1  -> 8,16,24
      FXPUNT(FXPUNT_NORMAL);
      break;
              
      case 4:                             // 4  -> 16,24
      FarToFlat(lpParams->lpDIBits, lpSrcData);
      if (lpDst->deBitsPixel == 16)
          Host2Scrn4to16ConvBlt(lpDst,
                    (RECT FAR *)lpParams->lpClipRect,
                    lpParams->cScans,
                    (int)(short)lpParams->X,
                    (int)(short)lpParams->Y,
                    (int)0,
                    (int)0,                    
                    lpSrcData,
                    lpSrc,
                    (DWORD)NULL,
                    (DWORD)NULL,
                    SRCCOPY );
      else
          if(!Host2Scrn4to2432ConvBlt( lpDst,
                    (RECT FAR *)lpParams->lpClipRect,
                    lpParams->cScans,
                    (int)(short)lpParams->X,
                    (int)(short)lpParams->Y,
                    (int)0,
                    (int)0,
                    lpSrcData,
                    lpSrc,
                    (DWORD)NULL,
                    (DWORD)NULL,
                    SRCCOPY ))
          {
              FXPUNT(FXPUNT_NORMAL);
              break;
          }
      break;
         
      case 8:                             // 8  -> 16,24
      FarToFlat(lpParams->lpDIBits, lpSrcData);
      if (lpDst->deBitsPixel == 16)
          Host2Scrn8To16ConvBlt(lpDst,
                    (RECT FAR *)lpParams->lpClipRect,
                    lpParams->cScans,
                    (int)(short)lpParams->X,
                    (int)(short)lpParams->Y,
                    lpSrcData,
                    lpSrc );
      else
          Host2Scrn8bppConvBlt(lpDst,
                    (RECT FAR *)lpParams->lpClipRect,
                    lpParams->cScans,
                    (int)(short)lpParams->X,
                    (int)(short)lpParams->Y,
                    lpSrcData,
                    lpSrc ); 
      break;
            
      case 16:                            // 24 -> 16
      FarToFlat(lpParams->lpDIBits, lpSrcData);
      if (0x08 == lpDst->deBitsPixel)
          StretchHost2Scrn16To08ConvBlt(lpDst,
                    (RECT FAR *)lpParams->lpClipRect,
                    lpParams->cScans,
                    (int)(short)lpParams->X,
                    (int)(short)lpParams->Y,
                    0x0,
                    0x0,
                    lpSrcData,
                    lpSrc,
                    lpParams->lpDrawMode,
                    0x0,
                    lpParams->lpTranslate,
                    SRCCOPY);

      else
         FXPUNT(FXPUNT_NORMAL);
      break;
              
      case 24:                            // 24 -> 16
      FXPUNT(FXPUNT_NORMAL);
      break;
    
      default:
      retval = -1;
      FXPUNT(FXPUNT_NORMAL);
      break;
    }
    
    FXLEAVE_NOHWACCESS("FxSetDIBitsToDevice", retval);
}

/*----------------------------------------------------------------------
Function name:  FxStretchDIBits

Description:    Main 32-bit entry point for GDI StretchDIBits
                function called from the 16-bit side.
Information:

Return:         BOOL     1 if success or,
                        -1 if failure.
----------------------------------------------------------------------*/
BOOL
FxStretchDIBits(StretchDIBitsParams *lpParams)

//  DIBENGINE FAR  *lpDestDev, 
//  WORD            fGet,
//  WORD            dstX,
//  WORD            dstY,
//  WORD            dstWidth,
//  WORD            dstHeight,
//  WORD            srcX,
//  WORD            srcY,
//  WORD            srcWidth,
//  WORD            srcHeight,
//  LPVOID          lpBits,
//  BITMAPINFO FAR *lpBitmapInfo, 
//  LPINT           lpTranslate,
//  DWORD           Rop,
//  DIB_Brush8 FAR *lpPBrush,
//  DRAWMODE FAR   *lpDrawMode, 
//  RECT FAR       *lpClipRect

{

    LPDIBENGINE   lpDst;
    LPBITMAPINFO  lpSrc;
    DWORD      *  lpSrcData;
    BYTE          pixCode;
    LONG retval;
    
    DEBUG_FIX;
        
    retval = 1;
    FarToFlat(lpParams->lpDestDev, lpDst);
    
	if ( (short)lpParams->srcHeight < 0 )
	{
		FXPUNT(FXPUNT_NORMAL);
	}

    if (lpParams->fGet == 1)
        FXPUNT(FXPUNT_NORMAL);
        
    if (_FF(gdiFlags) & SKIP_FLAGS)
         return retval;

    FarToFlat(lpParams->lpBitmapInfo, lpSrc);
                    
    pixCode = MAKE_PIX_CODE(lpSrc->bmiHeader.biBitCount,lpDst->deBitsPixel);

    switch( pixCode )
    {
        // we don't handle 8bpp destinations in StretchDIB yet!
        case    DIB_01TO08:
        case    DIB_04TO08:
        case    DIB_08TO08:
        case    DIB_32TO08:
        case    DIB_01TO16:
        case    DIB_32TO16:
        case    DIB_01TO32: 
        case    DIB_16TO32:               
        case    DIB_32TO32:
            FXPUNT(FXPUNT_NORMAL); break;

        case    DIB_04TO16:
            if ( (BI_RGB == lpSrc->bmiHeader.biCompression) &&
                 (lpParams->srcHeight == lpParams->dstHeight) &&
                 (lpParams->srcWidth == lpParams->dstWidth) )
            {
                FarToFlat(lpParams->lpBits, lpSrcData);
                retval = Host2Scrn4to16ConvBlt(  lpDst,
                                (RECT FAR *) lpParams->lpClipRect,
                                (WORD) lpParams->srcHeight,
                                (int)(short) lpParams->dstX,
                                (int)(short) lpParams->dstY,
                                (int)(short) lpParams->srcX,
                                (int)(short) lpParams->srcY,
                                lpSrcData,
                                lpSrc,
                                (DIB_Brush8 FAR*) lpParams->lpPBrush,
                                (DRAWMODE FAR*) lpParams->lpDrawMode,
                                lpParams->Rop );                                        
            }
            else
            {
                FXPUNT(FXPUNT_NORMAL);
            }                                        
            break;
            
        case    DIB_16TO08:
            if ( (BI_RGB == lpSrc->bmiHeader.biCompression) &&
                 (lpParams->srcHeight == lpParams->dstHeight) &&
                 (lpParams->srcWidth == lpParams->dstWidth) )
            {
                FarToFlat(lpParams->lpBits, lpSrcData);
                retval = StretchHost2Scrn16To08ConvBlt(lpDst, 
                                    (RECT FAR *)lpParams->lpClipRect,
                                    lpParams->srcHeight,
                                    lpParams->dstX,
                                    lpParams->dstY,
                                    lpParams->srcX,
                                    lpParams->srcY,
                                    lpSrcData,
                                    lpSrc, 
                                    lpParams->lpDrawMode,
                                    lpParams->lpPBrush,
                                    lpParams->lpTranslate,
                                    lpParams->Rop);
            }        
            else
            {
                FXPUNT(FXPUNT_NORMAL);
            }
            break;

        case    DIB_08TO16:        
            if ( (BI_RGB == lpSrc->bmiHeader.biCompression) &&
                 (lpParams->srcHeight == lpParams->dstHeight) &&
                 (lpParams->srcWidth == lpParams->dstWidth) )
            {
                retval = StretchHost2Scrn8To16ConvBlt( lpDst, lpSrc, lpParams );
            }        
            else
            {
                FXPUNT(FXPUNT_NORMAL);
            }
            break;

        case    DIB_16TO16:
            // at 16bpp, we handle 565 format
            // Handle 555 cause the DIBENGINE sure does not!!!!!
            if (((BI_BITFIELDS == lpSrc->bmiHeader.biCompression) ||
                 (BI_RGB == lpSrc->bmiHeader.biCompression)) &&
                   (SRCCOPY == lpParams->Rop))
            {
                FarToFlat(lpParams->lpBits, lpSrcData);
                retval = Host2Scrn16to16StrBlt(lpDst,
                              (RECT FAR *)lpParams->lpClipRect,
                              (int)(short)lpParams->dstX,
                              (int)(short)lpParams->dstY,
                              (int)(short)lpParams->dstWidth,
                              (int)(short)lpParams->dstHeight,
                              (int)(short)lpParams->srcX,
                              (int)(short)lpParams->srcY,
                              (int)(short)lpParams->srcWidth,
                              (int)(short)lpParams->srcHeight,
                              lpSrcData,
                              lpSrc );
            }
            else
            {
                FXPUNT(FXPUNT_NORMAL);
            }
            break;
            
        case    DIB_04TO32:
            if ( (BI_RGB == lpSrc->bmiHeader.biCompression) &&
                 (lpParams->srcHeight == lpParams->dstHeight) &&
                 (lpParams->srcWidth == lpParams->dstWidth) )
            {
                FarToFlat(lpParams->lpBits, lpSrcData);
                retval = Host2Scrn4to2432ConvBlt( lpDst,
                                      (RECT FAR *)lpParams->lpClipRect,
                                      (WORD)lpParams->srcHeight,
                                      (int)(short)lpParams->dstX,
                                      (int)(short)lpParams->dstY,
                                      (int)(short)lpParams->srcX,
                                      (int)(short)lpParams->srcY,
                                      lpSrcData,
                                      lpSrc,
                                      (DIB_Brush8 FAR*) lpParams->lpPBrush,
                                      (DRAWMODE FAR*) lpParams->lpDrawMode,
                                      lpParams->Rop );
                if (!retval)
                    FXPUNT(FXPUNT_NORMAL);
            }
            else
            {
                FXPUNT(FXPUNT_NORMAL); 
            }
            break;
                       
        case    DIB_08TO32:               
            if ( (BI_RGB == lpSrc->bmiHeader.biCompression) &&
                 (lpParams->srcHeight == lpParams->dstHeight) &&
                 (lpParams->srcWidth == lpParams->dstWidth) ) 
            {                        
                retval = StretchHost2Scrn8To32ConvBlt( lpDst, lpSrc, lpParams );
            }
            else
            {
                FXPUNT(FXPUNT_NORMAL);
            }
            break;
    }                                                  
    
    FXLEAVE_NOHWACCESS("FxStretchDIBits", retval);
        
}


/*----------------------------------------------------------------------
Function name:  StretchHost2Scrn8To32ConvBlt

Description:    Performs colortable lookup on 256-entry palette and 
                writes data host-to-screen blt. 
                
Information:    

Return:         INT     1 if success or,
                        0 if rejected.
----------------------------------------------------------------------*/

int StretchHost2Scrn8To32ConvBlt(  LPDIBENGINE     lpDst,
                                   LPBITMAPINFO    lpSrc,
                                   StretchDIBitsParams *lpParams )

{
    RECT   * lpRect;            // Local pointer to Clipping Rectangle 
    BYTE   * SrcDataPtr;        // Pointer to actual start of clipped DIB data
    BYTE   * lpData;            // Working pointer to source data
    DRAWMODE   * pDrawMode;        
    DIB_Brush8 * pBrush;           
    LONG     CTop;              // Clipped extents
    LONG     CBottom;
    LONG     CLeft;
    LONG     CRight;
    DWORD    SrcDataPitch;      // Pitch of source DIB data, in bytes
    DWORD    TotalDwords;       // Total # of DWORDS to be xferred into FIFO 
    DWORD    DataDWord;         // Temp for RGB data xlated from lpColors 
    DWORD    TopClippedAmount;  // # of Scans clipped from 'top' of DIB 
    DWORD    LeftClippedAmount; // # of BYTES clipped from left of DIB
    DWORD    dwtemp;            // Temp data storage used for FIFO macros
    LONG     i;                 
    DWORD    j;
    DWORD  * ColorTable;        // Pointer to DIB color conversion table
    DWORD    PixelsInScanline;  // Since each DIB pixel will end up as a 
                                // DWORD in the FIFO, this is also a count 
                                // of dwords in a scanline
    DWORD    cmdData;           // value to be written into the command register
    DWORD    dwROP;
    int      DstX, DstY, SrcX, SrcY;
    WORD     cScans;
    DWORD*   lpSrcData;    
    
                             
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;
    // OK, Well, DIBs are usually stored backwards.  Scanline pointed to
    // by lpDIBits is LAST scanline (if biHeight positive).  We'll grab
    // scanlines out in reverse order to feed the engine in the top-down
    // order.  Assume iScan == 0 and cScans = biHeight.
    // 
    // So, addess of First default DIB scanline = 
    //          lpDIBits + biHeight (or cscans) * biWidth (in bytes)
    //
    // This makes our DEFAULT rectangle:
    //      start at lpDIBITS + (cScans - 1) * dwSrcWidthInBytes
    //      width = biWidth * BytesPerPixel rounded to next dword boundary
    //      height = cScans
    //      pitch = dwSrcWidthInBytes
    //
    // In which case the default destination rectangle would be:
    //      [X,Y]-[biWidth,cScans]
    //
    // Of course, this must be clipped against the actual clipping 
    // rectangle if it is non-null, which in winbench98 is almost always
    // true.
    
    //
    //  SET UP DESTINATION/TRANSFER VARIABLES
    //
    
    DstX = (int)(short)lpParams->dstX;
    DstY = (int)(short)lpParams->dstY;
    SrcX = (int)(short)lpParams->srcX;
    SrcY = (int)(short)lpParams->srcY;
    cScans = (WORD) lpParams->srcHeight; 
    
    FarToFlat(lpParams->lpBits, lpSrcData); 
    
    // DIBPitch for 8bpp dib is #pixels rounded to next dword boundary
    SrcDataPitch = (lpSrc->bmiHeader.biWidth + 3) & 0xFFFFFFFC;
    
    if (lpParams->lpClipRect) 
    {
        // If lpRect Non-Null, intersect with default dest rect
        // Get usable pointer to clip rectangle
        FarToFlat(lpParams->lpClipRect, lpRect);
    
        CTop    = max(lpRect->top,DstY);
        CLeft   = max(lpRect->left,DstX);
        CBottom = min(lpRect->bottom,DstY+(int)cScans);
        CRight  = min(lpRect->right,DstX+(int)lpSrc->bmiHeader.biWidth);
        
        // Added to fix Defect 1703
        // What happen is CRight is less then CLeft we get a negative PixelsInScanLine
        // There is never enough room and life gets worse until we come to a sw halt
        // Might as well check for a bogus y while we are at it
        if ((CRight <= CLeft) || (CBottom <= CTop))
        {
            return 0;
        }
                  
        TopClippedAmount  = lpRect->top  - (DWORD)DstY;
        LeftClippedAmount = lpRect->left - (DWORD)DstX;
        PixelsInScanline  = CRight - CLeft;
        
        if ((long)(lpSrc->bmiHeader.biHeight) < 0)  // top-down DIB
        {
            SrcDataPtr = (BYTE*)lpSrcData + SrcX + LeftClippedAmount +
                         (-((long)lpSrc->bmiHeader.biHeight)
                          - SrcY - cScans + TopClippedAmount ) * SrcDataPitch;
        }
        else
        {
            SrcDataPtr = (BYTE *)lpSrcData + SrcX + LeftClippedAmount +
                       + ((SrcY + cScans - TopClippedAmount - 1) * SrcDataPitch);
            SrcDataPitch = -(int)SrcDataPitch;      // walk the dib backwards 
        }
    }
    else
    {
    
        CTop    = DstY;
        CLeft   = DstX;
        CBottom = DstY+(int)cScans;
        CRight  = DstX+lpSrc->bmiHeader.biWidth;
        
        PixelsInScanline  = CRight - CLeft;

        if ((long)(lpSrc->bmiHeader.biHeight) < 0)  // top-down DIB
        {
            SrcDataPtr = (BYTE *)lpSrcData + SrcX +
                         (-((long)lpSrc->bmiHeader.biHeight)
                          - SrcY - cScans ) * SrcDataPitch;
        }
        else
        {
            SrcDataPtr = (BYTE *)lpSrcData + SrcX
                       + ((SrcY + cScans - 1) * SrcDataPitch);
            SrcDataPitch = -(int)SrcDataPitch;      // walk the dib backwards 
        }
    }

    // XXX FIXME
    // check that the lpsrc is/isn't always in host memory, or at least
    // isn't ever the screen, so we don't have to include it in the s/w
    // cursor exclusion test
    //        
    
    FXENTER("Host2Scrn8bppConvBlt", lpDst->deFlags, TRUE,
        cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
        FXENTER_RECT, CLeft, CTop, CRight, CBottom,
        FXENTER_NO_RECT, 0, 0, 0, 0,   );

    dwROP = lpParams->Rop;
            
    if (RopUsesSrc(dwROP >> 16))
    {
        // Use the Source
        if (RopUsesPat(dwROP >> 16) && (lpParams->lpDrawMode != 0) && (lpParams->lpPBrush != 0) )
        {
            CMDFIFO_SAVE(cf);
            FarToFlat(lpParams->lpDrawMode, pDrawMode);
            FarToFlat(lpParams->lpPBrush, pBrush);
            cmdData = HandleBrush(pBrush, dwROP, pDrawMode);
            CMDFIFO_RELOAD(cf);
        } 
        else
        {
        
            cmdData = (dwROP >> 16) << 24;   
        }
                    
        //  
        //  SET UP HARDWARE FOR COLOR HOST TO SCREEN BITBLT  
        //
        CMDFIFO_CHECKROOM(cf, 6);
        SETPH(cf,  (SSTCP_PKT2   |
                    srcFormatBit |
                    srcXYBit     |
                    dstSizeBit   |
                    dstXYBit     |
                    commandBit)); 
        // Source Format is 32bpp
        dwtemp = (PixelsInScanline << 2) | SSTG_PIXFMT_32BPP;
        SET(cf, _FF(lpGRegs)->srcFormat, dwtemp);
        SET(cf, _FF(lpGRegs)->srcXY, 0);
        SET(cf, _FF(lpGRegs)->dstSize, R32((CBottom - CTop), (CRight - CLeft)));
        SET(cf, _FF(lpGRegs)->dstXY, R32(CTop, CLeft));
        SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_HOST_BLT);
        BUMP(6);

        //
        //   TRANSLATE AND TRANSFER DATA TO FIFO
        //

        lpData = SrcDataPtr;
        TotalDwords = PixelsInScanline * (CBottom - CTop);
    
        //    ColorTable = (DWORD *)lpSrc->bmiColors;
        //PingZ 3/12/99 lpSrc->bmiColors may not be pointing to the actual color table!!!  The only
        //to guarantee that is to add bmiHeader.biSize to lpSrc.  This is to fix PRS #4604
        ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;
        
        if (TotalDwords < MAXCOLORDWORDS)
        {
        // Write whole transfer at once
            CMDFIFO_CHECKROOM(cf, TotalDwords+1);
            SETPH(cf, (SSTCP_PKT1 |
                   SSTCP_PKT1_2D |
                   (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT)  |
                   (TotalDwords  << SSTCP_PKT1_NWORDS_SHIFT)));

            for (i = 0; i < (CBottom - CTop); i++)
            {
                for (j = 0; j < PixelsInScanline; j++)
                {
                    DataDWord = ColorTable[lpData[j]];
                    SET(cf,_FF(lpGRegs)->launch[0], DataDWord);
                }
                (DWORD)lpData += SrcDataPitch;
            }

            BUMP(TotalDwords + 1);
        }
        else
        {
            // Write transfer one scanline at a time.
            //
            for (i = 0; i < (CBottom - CTop); i++)
            {
                CMDFIFO_CHECKROOM(cf, PixelsInScanline + 1);
                SETPH(cf, (SSTCP_PKT1 |
                       SSTCP_PKT1_2D |
                       (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
                       (PixelsInScanline << SSTCP_PKT1_NWORDS_SHIFT)));

                for (j = 0; j < PixelsInScanline; j++)
                {	    			
                    DataDWord = ColorTable[lpData[j]];
                    SET(cf,_FF(lpGRegs)->launch[0], DataDWord);                    
                }	

                (DWORD)lpData += SrcDataPitch;
                
                BUMP(PixelsInScanline + 1);
            }
        }                   
    }    
    else
    {
        // There is no source
        CMDFIFO_CHECKROOM(cf, 4);
        CMDFIFO_SAVE(cf);
        FarToFlat(lpParams->lpDrawMode, pDrawMode);
        FarToFlat(lpParams->lpPBrush, pBrush);
        cmdData = HandleBrush(pBrush, dwROP, pDrawMode);
        CMDFIFO_RELOAD(cf);       
        SETPH(cf, SSTCP_PKT2 | dstSizeBit | dstXYBit | commandBit);
        dwtemp = ((((DWORD)(CBottom - CTop )) << 16) |
                   ((DWORD)(PixelsInScanline)));
        SET(cf, _FF(lpGRegs)->dstSize,dwtemp);
        dwtemp  = (CTop << 16) | CLeft;                  
        SET(cf, _FF(lpGRegs)->dstXY , dwtemp);                
        SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_GO | SSTG_RECTFILL);
        BUMP(4);
    }    
    
    FXLEAVE2("StretchHost2Scrn8ToConvBlt", cf, lpDst, CBottom-CTop);

    return CBottom-CTop;
}

  
/*----------------------------------------------------------------------
Function name:  StretchHost2Scrn8To16ConvBlt

Description:    Performs colortable lookup on 256-entry palette and 
                writes 16bpp data to HW as a host-to-screen blit after
                converting the colors from 32-bit RGB values (from
                the 8bpp color lookup table) into 16-bit 565 colors.
                
Information:    

Return:         INT     1 if success or,
                        0 if rejected.
----------------------------------------------------------------------*/
int StretchHost2Scrn8To16ConvBlt(  LPDIBENGINE     lpDst,
                                   LPBITMAPINFO    lpSrc,
                                   StretchDIBitsParams *lpParams )   
{
    RECT   * lpRect;            // Local pointer to Clipping Rectangle 
    BYTE   * SrcDataPtr;        // Pointer to actual start of clipped DIB data
    BYTE   * lpData;            // Working pointer to source data
    DRAWMODE   * pDrawMode;        
    DIB_Brush8 * pBrush;       
    LONG     CTop;              // Clipped extents
    LONG     CBottom;
    LONG     CLeft;
    LONG     CRight;
    DWORD    SrcDataPitch;      // Pitch of source DIB data, in bytes
    DWORD    TotalDwords;       // Total # of DWORDS to be xferred into FIFO 
    DWORD    DataDword;         // Temp for RGB data xlated from lpColors 
    DWORD    TopClippedAmount;  // # of Scans clipped from 'top' of DIB 
    DWORD    LeftClippedAmount; // # of BYTES clipped from left of DIB
    DWORD    dwtemp;            // Temp data storage used for FIFO macros
    LONG     i;                 
    DWORD    j;
    DWORD  * ColorTable;        // Pointer to DIB color conversion table
    DWORD    PixelsInScanline;  // We'll be pushing two pixels per dword
    DWORD    DwordsInScanline;  // ACTUAL number of dwords of data in Scanline
    DWORD    RGBDword;          // Raw RGB data from DIB color table
    DWORD    LastRGB;           // Previously Looked up RGB value
    DWORD    Last565;           // Result of previous lookup
    DWORD    blubyte;           // Blue data from RGB
    DWORD    grnbyte;           // Green data from RGB
    DWORD    redbyte;           // Red data from RGB
    DWORD    clrUsed;           // Number of palette entries that are actually used
    DWORD    cmdData;           // value to be written into the command register
    BOOL     bUse565Table;      // This is set if the 32bpp palette is converted to 16bpp
    DWORD    dwROP;
    int      DstX, DstY, SrcX, SrcY;
    WORD     cScans;
    DWORD*   lpSrcData;    
    
        
    CMDFIFO_PROLOG(cf);
    DEBUG_FIX;
    
    bUse565Table = FALSE;
    
    DstX = (int)(short)lpParams->dstX;
    DstY = (int)(short)lpParams->dstY;
    SrcX = (int)(short)lpParams->srcX;
    SrcY = (int)(short)lpParams->srcY;
    cScans = (WORD) lpParams->srcHeight; 
    
    FarToFlat(lpParams->lpBits, lpSrcData); 
    
    // DIBPitch for 8bpp dib is #pixels rounded to next dword boundary
    SrcDataPitch = (lpSrc->bmiHeader.biWidth + 3) & 0xFFFFFFFC;
    
    if (lpParams->lpClipRect) 
    {
        // If lpRect Non-Null, intersect with default dest rect
        // Get usable pointer to clip rectangle
        FarToFlat(lpParams->lpClipRect, lpRect);
    
        CTop    = max(lpRect->top,DstY);
        CLeft   = max(lpRect->left,DstX);
        CBottom = min(lpRect->bottom,DstY+(int)cScans);
        CRight  = min(lpRect->right,DstX+(int)lpSrc->bmiHeader.biWidth);
        
        // Validate extents
        if ((CRight <= CLeft) || (CBottom <= CTop))
        {
            return 0;
        }
         
        TopClippedAmount  = lpRect->top  - (DWORD)DstY;
        LeftClippedAmount = lpRect->left - (DWORD)DstX;
        PixelsInScanline  = CRight - CLeft;

        SrcX += LeftClippedAmount;
        
        if ((long)(lpSrc->bmiHeader.biHeight) < 0) // top-down DIB
        {
            SrcDataPtr = (BYTE*)lpSrcData + SrcX + 
                         (-((long)lpSrc->bmiHeader.biHeight)
                          - SrcY - cScans + TopClippedAmount
                         ) * SrcDataPitch;
        }
        else
        {
            SrcDataPtr = (BYTE*)lpSrcData + SrcX
                       + ((SrcY + cScans - TopClippedAmount - 1) * SrcDataPitch);
            SrcDataPitch = -(int)SrcDataPitch;               
        }

    }
    else
    {
        CTop    = DstY;
        CLeft   = DstX;
        CBottom = DstY+(int)cScans;
        CRight  = DstX+lpSrc->bmiHeader.biWidth;

        PixelsInScanline  = CRight - CLeft;
        
        if ((long)(lpSrc->bmiHeader.biHeight) < 0) // top-down DIB
        {
            SrcDataPtr = (BYTE*)lpSrcData + SrcX + 
                         (-((long)lpSrc->bmiHeader.biHeight)
                          - SrcY - cScans
                         ) * SrcDataPitch;
        }
        else
        {
            SrcDataPtr = (BYTE *)lpSrcData + SrcX
                       + ((SrcY + cScans - 1) * SrcDataPitch);
            SrcDataPitch = -(int)SrcDataPitch;               
        }
    }
    
    // How many pixels are in the bitmap? If it's more than 256 we'll create a new palette 
    // containing RGB565 colours. That way we don't end up converting the same colour more
    // than once.
    clrUsed = (lpSrc->bmiHeader.biClrUsed == 0) ? 256 : lpSrc->bmiHeader.biClrUsed;
    if ( (PixelsInScanline * (CBottom - CTop)) > 256 )
    {
        ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;
        for ( j = 0; j < clrUsed; j++ )
        {
            RGBDword = ColorTable[j];
            blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
            grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
            redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
            RGB565ColorTable[j] = (WORD) (blubyte | (grnbyte << 5) | (redbyte << 11));
        }
        bUse565Table = TRUE;
    }
    
    // XXX FIXME
    // check that the lpsrc is/isn't always in host memory, or at least
    // isn't ever the screen, so we don't have to include it in the s/w
    // cursor exclusion test
    //
    FXENTER("Host2Scrn8To16ConvBlt", lpDst->deFlags, TRUE,
        cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
        FXENTER_RECT, CLeft, CTop, CRight, CBottom,
        FXENTER_NO_RECT, 0, 0, 0, 0,   );

    dwROP = lpParams->Rop;        
    
    if (RopUsesSrc(dwROP >> 16))
    {
        // Use the Source
        if (RopUsesPat(dwROP >> 16) && (lpParams->lpDrawMode != 0) && (lpParams->lpPBrush != 0) )
        {
            // If there is a pattern, load it into the HW            
            CMDFIFO_SAVE(cf);
            FarToFlat(lpParams->lpDrawMode, pDrawMode);
            FarToFlat(lpParams->lpPBrush, pBrush);
            cmdData = HandleBrush(pBrush, dwROP, pDrawMode);
            CMDFIFO_RELOAD(cf);
        } 
        else
        {
            cmdData = (dwROP >> 16) << 24;   
        }      
            
        //  
        //  SET UP HARDWARE FOR COLOR HOST TO SCREEN BITBLT  
        //
        CMDFIFO_CHECKROOM(cf, 6);
        SETPH(cf,  (SSTCP_PKT2   |
                    srcFormatBit |
                    srcXYBit     |
                    dstSizeBit   |
                    dstXYBit     |
                    commandBit)); 
        
        dwtemp = (PixelsInScanline + 1) & 0xFFFFFFFE;   // Round to even number of pixels
        dwtemp = (dwtemp << 2) | SSTG_PIXFMT_16BPP;        
        // Source Format is 32bpp
        SET(cf, _FF(lpGRegs)->srcFormat, dwtemp);
        SET(cf, _FF(lpGRegs)->srcXY, 0);
        SET(cf, _FF(lpGRegs)->dstSize, R32((CBottom - CTop), (CRight - CLeft)));
        SET(cf, _FF(lpGRegs)->dstXY, R32(CTop, CLeft));
        SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_HOST_BLT);
        BUMP(6);

        //
        //   TRANSLATE AND TRANSFER DATA TO FIFO
        //

        lpData = SrcDataPtr;
        DwordsInScanline = ((PixelsInScanline << 1) + 2) >> 2;
        TotalDwords      = DwordsInScanline * (CBottom - CTop);
    
        if (TotalDwords < MAXCOLORDWORDS)
        {
            // Write whole transfer at once
            CMDFIFO_CHECKROOM(cf, TotalDwords+1);
            SETPH(cf, (SSTCP_PKT1                                |
                       SSTCP_PKT1_2D                             |
                       (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT)     |
                       (TotalDwords  << SSTCP_PKT1_NWORDS_SHIFT)) );

            if (bUse565Table)
            {   
                // The 32-bit palette has been translated and copied into
                // the 16bpp table.                 
                for (i = 0; i < (CBottom - CTop); i++)
                {
                    for (j = 0; j < (PixelsInScanline & 0xFFFFFFFE); j+=2)
                    {                
                        DataDword = RGB565ColorTable[lpData[j]];                                               
                        DataDword |= (RGB565ColorTable[lpData[j+1]] << 16);                                
                        SET(cf,_FF(lpGRegs)->launch[0], DataDword);                      
                    }
            
                    if (PixelsInScanline & 1)
                    {                
                        DataDword = RGB565ColorTable[lpData[PixelsInScanline-1]];
                        SET(cf,_FF(lpGRegs)->launch[0], DataDword);
                    }
                    (DWORD)lpData += SrcDataPitch;
                }
            }
            else 
            {
                ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;
                LastRGB = 0xFFFFFFFF;
                // The DIB is less than 256 pixels, therefore there is no point
                // converting 256 colours to 16bpp. Do the conversion as we read
                // the 32bpp values from the palette
                for (i = 0; i < (CBottom - CTop); i++)
                {
                    for (j = 0; j < (PixelsInScanline & 0xFFFFFFFE); j+=2)
                    {
                        RGBDword = ColorTable[lpData[j]];
                        if (RGBDword == LastRGB) 
                        {
                            DataDword = Last565;
                        }    
                        else
                        {
                            LastRGB = RGBDword;
                            blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
                            grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
                            redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
                            DataDword= (blubyte | (grnbyte << 5) | (redbyte << 11));
                            Last565  = DataDword;
                        }
                                                    
                        RGBDword = ColorTable[lpData[j+1]];
                        if (RGBDword == LastRGB) 
                        {
                            DataDword |= (Last565 << 16);
                        }
                        else
                        {
                            LastRGB = RGBDword;
                            blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
                            grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
                            redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
                            Last565 = (blubyte | (grnbyte << 5) | (redbyte << 11));
                            DataDword |= (Last565 << 16);
                        }            
                            
                        SET(cf,_FF(lpGRegs)->launch[0], DataDword);
                          
                    }
        
                    if (PixelsInScanline & 1)
                    {
                        RGBDword = ColorTable[lpData[PixelsInScanline-1]];

                        if (RGBDword == LastRGB) 
                        {
                            DataDword = Last565;
                        }
                        else
                        {
                            blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
                            grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
                            redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
                            DataDword= (blubyte | (grnbyte << 5) | (redbyte << 11));
                        } 
            
                        SET(cf,_FF(lpGRegs)->launch[0], DataDword);

                    }

                    (DWORD)lpData += SrcDataPitch;
                }                
            }

            BUMP(TotalDwords + 1);
        }
        else
        {
            // Write transfer one scanline at a time.
            //
            for (i = 0; i < (CBottom - CTop); i++)
            {
                CMDFIFO_CHECKROOM(cf, DwordsInScanline + 1);
                SETPH(cf, (SSTCP_PKT1                            |
                           SSTCP_PKT1_2D                         |
                           (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
                           (DwordsInScanline << SSTCP_PKT1_NWORDS_SHIFT)));

                for (j = 0; j < (PixelsInScanline & 0xFFFFFFFE); j+=2)
                {
                
                    DataDword = RGB565ColorTable[lpData[j]];
                    DataDword |= (RGB565ColorTable[lpData[j+1]] << 16);                
                    SET(cf,_FF(lpGRegs)->launch[0], DataDword);                  
                }
            
                if (PixelsInScanline & 1)
                {
                    DataDword = RGB565ColorTable[lpData[PixelsInScanline-1]];                
                    SET(cf,_FF(lpGRegs)->launch[0], DataDword);
                }

                (DWORD)lpData += SrcDataPitch;
                   
                BUMP(PixelsInScanline + 1);
           }
       }   
    }
    
    else
    {
        // There is no source
        CMDFIFO_CHECKROOM(cf, 4);
        CMDFIFO_SAVE(cf);
        FarToFlat(lpParams->lpDrawMode, pDrawMode);
        FarToFlat(lpParams->lpPBrush, pBrush);
        cmdData = HandleBrush(pBrush, dwROP, pDrawMode);
        CMDFIFO_RELOAD(cf);       
        SETPH(cf, SSTCP_PKT2 | dstSizeBit | dstXYBit | commandBit);
        dwtemp = ((((DWORD)(CBottom - CTop )) << 16) |
                   ((DWORD)(PixelsInScanline)));
        SET(cf, _FF(lpGRegs)->dstSize,dwtemp);
        dwtemp  = (CTop << 16) | CLeft;                  
        SET(cf, _FF(lpGRegs)->dstXY , dwtemp);                
        SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_GO | SSTG_RECTFILL);
        BUMP(4);
    }    
       
    FXLEAVE2("StretchHost2Scrn8To16ConvBlt", cf, lpDst, CBottom-CTop);
    return CBottom-CTop;
}


/*----------------------------------------------------------------------
Function name:  StretchHost2Scrn16To08ConvBlt

Description:    Converts 16 bpp to 8 bpp
                
Information:    

Return:         INT     1 if success or,
                        0 if rejected.
----------------------------------------------------------------------*/
int StretchHost2Scrn16To08ConvBlt(  LPDIBENGINE     lpDst,
                           RECT FAR *      lpClipRect,
                           WORD            cScans, 
                           int             DstX,
                           int             DstY,
                           int             SrcX,
                           int             SrcY,
                           DWORD *         lpSrcData,   // buffer
                           LPBITMAPINFO    lpSrc,   
                           DWORD           lpDrawMode,
                           DWORD           lpPBrush,
                           DWORD           lpTranslate,
                           DWORD           dwROP)
{
    RECT   * lpRect;            // Local pointer to Clipping Rectangle 
    BYTE   * SrcDataPtr;        // Pointer to actual start of clipped DIB data
    WORD  * lpData;            // Working pointer to source data
    DRAWMODE   * pDrawMode;        
    DIB_Brush8 * pBrush;       
    LONG     CTop;              // Clipped extents
    LONG     CBottom;
    LONG     CLeft;
    LONG     CRight;
    DWORD    SrcDataPitch;      // Pitch of source DIB data, in bytes
    DWORD    TotalDwords;       // Total # of DWORDS to be xferred into FIFO 
    DWORD    DataDword;         // Temp for RGB data xlated from lpColors 
    DWORD    TopClippedAmount;  // # of Scans clipped from 'top' of DIB 
    DWORD    LeftClippedAmount; // # of BYTES clipped from left of DIB
    DWORD    dwtemp;            // Temp data storage used for FIFO macros
    LONG     i;                 
    DWORD    j;
    LONG     k;
    BYTE * ColorTable;        // Pointer to DIB color conversion table
    DWORD    PixelsInScanline;  // We'll be pushing two pixels per dword
    DWORD    DwordsInScanline;  // ACTUAL number of dwords of data in Scanline
    DWORD    cmdData;           // value to be written into the command register
    WORD     dataMask; 
       
    CMDFIFO_PROLOG(cf);
    DEBUG_FIX;
    
    // DIBPitch for 16bpp dib is #pixels rounded to next dword boundary
    if (lpSrc->bmiHeader.biWidth & 0x01)
       SrcDataPitch = (lpSrc->bmiHeader.biWidth + 1) << 1;
    else
       SrcDataPitch = lpSrc->bmiHeader.biWidth << 1;
    
    if (lpClipRect) 
    {
        // If lpRect Non-Null, intersect with default dest rect
        // Get usable pointer to clip rectangle
        FarToFlat(lpClipRect, lpRect);
    
        CTop    = max(lpRect->top,DstY);
        CLeft   = max(lpRect->left,DstX);
        CBottom = min(lpRect->bottom,DstY+(int)cScans);
        CRight  = min(lpRect->right,DstX+(int)lpSrc->bmiHeader.biWidth);
        
        // Validate extents
        if ((CRight <= CLeft) || (CBottom <= CTop))
        {
            return 0;
        }
         
        TopClippedAmount  = lpRect->top  - (DWORD)DstY;
        LeftClippedAmount = lpRect->left - (DWORD)DstX;
        PixelsInScanline  = CRight - CLeft;

        SrcX += LeftClippedAmount;
        
        if ((long)(lpSrc->bmiHeader.biHeight) < 0) // top-down DIB
        {
            SrcDataPtr = (BYTE*)lpSrcData + (SrcX<<1) + 
                         (-((long)lpSrc->bmiHeader.biHeight)
                          - SrcY - cScans + TopClippedAmount
                         ) * SrcDataPitch;
        }
        else
        {
            SrcDataPtr = (BYTE*)lpSrcData + (SrcX<<1)
                       + ((SrcY + cScans - TopClippedAmount - 1) * SrcDataPitch);
            SrcDataPitch = -(int)SrcDataPitch;               
        }

    }
    else
    {
        CTop    = DstY;
        CLeft   = DstX;
        CBottom = DstY+(int)cScans;
        CRight  = DstX+lpSrc->bmiHeader.biWidth;

        PixelsInScanline  = CRight - CLeft;
        
        if ((long)(lpSrc->bmiHeader.biHeight) < 0) // top-down DIB
        {
            SrcDataPtr = (BYTE*)lpSrcData + (SrcX<<1) + 
                         (-((long)lpSrc->bmiHeader.biHeight)
                          - SrcY - cScans
                         ) * SrcDataPitch;
        }
        else
        {
            SrcDataPtr = (BYTE *)lpSrcData + (SrcX<<1)
                       + ((SrcY + cScans - 1) * SrcDataPitch);
            SrcDataPitch = -(int)SrcDataPitch;               
        }
    }
    
    // XXX FIXME
    // check that the lpsrc is/isn't always in host memory, or at least
    // isn't ever the screen, so we don't have to include it in the s/w
    // cursor exclusion test
    //
    FXENTER("Host2Scrn8To16ConvBlt", lpDst->deFlags, TRUE,
        cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
        FXENTER_RECT, CLeft, CTop, CRight, CBottom,
        FXENTER_NO_RECT, 0, 0, 0, 0,   );

    // Is this 15 or 16 bit conversion?
    dataMask = (BI_BITFIELDS == lpSrc->bmiHeader.biCompression) ? 0xFFFF : 0x7FFF;

    if (RopUsesSrc(dwROP >> 16))
        {
        // Use the Source
        if (RopUsesPat(dwROP >> 16) && (lpDrawMode != 0) && (lpPBrush != 0) )
            {
            // If there is a pattern, load it into the HW            
            CMDFIFO_SAVE(cf);
            FarToFlat(lpDrawMode, pDrawMode);
            FarToFlat(lpPBrush, pBrush);
            cmdData = HandleBrush(pBrush, dwROP, pDrawMode);
            CMDFIFO_RELOAD(cf);
            } 
        else
            {
            cmdData = (dwROP >> 16) << 24;   
            }      
            
        //  
        //  SET UP HARDWARE FOR COLOR HOST TO SCREEN BITBLT  
        //
        CMDFIFO_CHECKROOM(cf, 6);
        SETPH(cf,  (SSTCP_PKT2   |
                    srcFormatBit |
                    srcXYBit     |
                    dstSizeBit   |
                    dstXYBit     |
                    commandBit)); 
        
        dwtemp = (PixelsInScanline + 3) & 0xFFFFFFFC;   // Round to even number of pixels
        dwtemp = dwtemp | SSTG_PIXFMT_8BPP;        
        // Source Format is 8 bpp
        SET(cf, _FF(lpGRegs)->srcFormat, dwtemp);
        SET(cf, _FF(lpGRegs)->srcXY, 0);
        SET(cf, _FF(lpGRegs)->dstSize, R32((CBottom - CTop), (CRight - CLeft)));
        SET(cf, _FF(lpGRegs)->dstXY, R32(CTop, CLeft));
        SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_HOST_BLT);
        BUMP(6);

        //
        //   TRANSLATE AND TRANSFER DATA TO FIFO
        //

        lpData = (WORD *)SrcDataPtr;
        DwordsInScanline = ((PixelsInScanline + 3) & ~(0x03))>>2;
        TotalDwords      = DwordsInScanline * (CBottom - CTop);
    
        // Write transfer one scanline at a time.
        //
        FarToFlat(lpTranslate, ColorTable);
        for (i = 0; i < (CBottom - CTop); i++)
            {
            CMDFIFO_CHECKROOM(cf, DwordsInScanline + 1);
            SETPH(cf, (SSTCP_PKT1                            |
                       SSTCP_PKT1_2D                         |
                       (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
                       (DwordsInScanline << SSTCP_PKT1_NWORDS_SHIFT)));

            for (j = 0; j+4 <= PixelsInScanline; j+=4)
               {
                
               DataDword = ColorTable[lpData[j] & dataMask];
               DataDword |= (ColorTable[lpData[j+1] & dataMask] << 8);                
               DataDword |= (ColorTable[lpData[j+2] & dataMask] << 16);                
               DataDword |= (ColorTable[lpData[j+3] & dataMask] << 24);                
               SET(cf,_FF(lpGRegs)->launch[0], DataDword);                  
                  }

            if (j < PixelsInScanline)
               {
               DataDword = 0;            
               k=0;
               for (; j < PixelsInScanline; j+=1)
                  {
                  DataDword |= (ColorTable[lpData[j] & dataMask] << k);
                  k+=8;
                  }

               SET(cf,_FF(lpGRegs)->launch[0], DataDword);                  
               }

           (DWORD)lpData += SrcDataPitch;
                   
           BUMP(PixelsInScanline + 1);
           }
       }   
    
    else
       {
        // There is no source
        CMDFIFO_CHECKROOM(cf, 4);
        CMDFIFO_SAVE(cf);
        FarToFlat(lpDrawMode, pDrawMode);
        FarToFlat(lpPBrush, pBrush);
        cmdData = HandleBrush(pBrush, dwROP, pDrawMode);
        CMDFIFO_RELOAD(cf);       
        SETPH(cf, SSTCP_PKT2 | dstSizeBit | dstXYBit | commandBit);
        dwtemp = ((((DWORD)(CBottom - CTop )) << 16) |
                   ((DWORD)(PixelsInScanline)));
        SET(cf, _FF(lpGRegs)->dstSize,dwtemp);
        dwtemp  = (CTop << 16) | CLeft;                  
        SET(cf, _FF(lpGRegs)->dstXY , dwtemp);                
        SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_GO | SSTG_RECTFILL);
        BUMP(4);
       }    
       
    FXLEAVE2("StretchHost2Scrn16To08ConvBlt", cf, lpDst, CBottom-CTop);
    return CBottom-CTop;
}

/*----------------------------------------------------------------------
Function name:  Host2Scrn8bppConvBlt

Description:    Performs colortable lookup on 256-entry palette and 
                writes data host-to-screen blt. The entries in the
                palette are 32-bit, the hardware takes care of
                converting this to the destination bit depth.
                
Information:    Can be called by FxSetDIBitsToDevice and FxDeviceBitmapBits.
                Calls from FxStretchDIBits have been seperated out to 
                StretchHost2Scrn8to32ConvBlt so as to reduce the number of params
                passed on the stack. This improves high end WB99 performance.

Return:         INT     1 if success or,
                        0 if rejected.
----------------------------------------------------------------------*/
int  Host2Scrn8bppConvBlt(LPDIBENGINE     lpDst,              // device dib
		                  RECT FAR *      lpClipRect,
		                  WORD            cScans, 
		                  int             DstX,
		                  int             DstY,   
		                  DWORD *         lpSrcData,          // buffer
		                  LPBITMAPINFO    lpSrc )
{
    RECT   * lpRect;            // Local pointer to Clipping Rectangle 
    BYTE   * SrcDataPtr;        // Pointer to actual start of clipped DIB data
    BYTE   * lpData;            // Working pointer to source data
    LONG     CTop;              // Clipped extents
    LONG     CBottom;
    LONG     CLeft;
    LONG     CRight;
    DWORD    SrcDataPitch;      // Pitch of source DIB data, in bytes
    DWORD    TotalDwords;       // Total # of DWORDS to be xferred into FIFO 
    DWORD    DataDWord;         // Temp for RGB data xlated from lpColors 
    DWORD    TopClippedAmount;  // # of Scans clipped from 'top' of DIB 
    DWORD    LeftClippedAmount; // # of BYTES clipped from left of DIB
    DWORD    dwtemp;            // Temp data storage used for FIFO macros
    LONG     i;                 
    DWORD    j;
    DWORD  * ColorTable;        // Pointer to DIB color conversion table
    DWORD    PixelsInScanline;  // Since each DIB pixel will end up as a 
                                // DWORD in the FIFO, this is also a count 
                                // of dwords in a scanline
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;
    // OK, Well, DIBs are usually stored backwards.  Scanline pointed to
    // by lpDIBits is LAST scanline (if biHeight positive).  We'll grab
    // scanlines out in reverse order to feed the engine in the top-down
    // order.  Assume iScan == 0 and cScans = biHeight.
    // 
    // So, addess of First default DIB scanline = 
    //          lpDIBits + biHeight (or cscans) * biWidth (in bytes)
    //
    // This makes our DEFAULT rectangle:
    //      start at lpDIBITS + (cScans - 1) * dwSrcWidthInBytes
    //      width = biWidth * BytesPerPixel rounded to next dword boundary
    //      height = cScans
    //      pitch = dwSrcWidthInBytes
    //
    // In which case the default destination rectangle would be:
    //      [X,Y]-[biWidth,cScans]
    //
    // Of course, this must be clipped against the actual clipping 
    // rectangle if it is non-null, which in winbench98 is almost always
    // true.
    
    //
    //  SET UP DESTINATION/TRANSFER VARIABLES
    //

    // DIBPitch for 8bpp dib is #pixels rounded to next dword boundary
    SrcDataPitch = (lpSrc->bmiHeader.biWidth + 3) & 0xFFFFFFFC;
    
    if (lpClipRect) 
    {
        // If lpRect Non-Null, intersect with default dest rect
        // Get usable pointer to clip rectangle
        FarToFlat(lpClipRect, lpRect);
    
        CTop    = max(lpRect->top,DstY);
        CLeft   = max(lpRect->left,DstX);
        CBottom = min(lpRect->bottom,DstY+(int)cScans);
        CRight  = min(lpRect->right,DstX+(int)lpSrc->bmiHeader.biWidth);
        
        // Added to fix Defect 1703
        // What happen is CRight is less then CLeft we get a negative PixelsInScanLine
        // There is never enough room and life gets worse until we come to a sw halt
        // Might as well check for a bogus y while we are at it
        if ((CRight <= CLeft) || (CBottom <= CTop))
        {
            return 0;
        }
                  
        TopClippedAmount  = lpRect->top  - (DWORD)DstY;
        LeftClippedAmount = lpRect->left - (DWORD)DstX;
        PixelsInScanline  = CRight - CLeft;
        
        if ((long)(lpSrc->bmiHeader.biHeight) < 0)  // top-down DIB
        {
			SrcDataPtr = (BYTE*)lpSrcData
					   + (TopClippedAmount * SrcDataPitch)
					   + LeftClippedAmount;                         
        }
        else
        {
			SrcDataPtr	= (BYTE *)lpSrcData
						+ ((cScans - TopClippedAmount - 1) * SrcDataPitch)
						+ LeftClippedAmount;
            SrcDataPitch = -(int)SrcDataPitch;      // walk the dib backwards 
        }
    }
    else
    {
    
        CTop    = DstY;
        CLeft   = DstX;
        CBottom = DstY+(int)cScans;
        CRight  = DstX+lpSrc->bmiHeader.biWidth;
        
        PixelsInScanline  = CRight - CLeft;

        if ((long)(lpSrc->bmiHeader.biHeight) < 0)  // top-down DIB
        {
			SrcDataPtr = (BYTE*)lpSrcData;                        
        }
        else
        {
			SrcDataPtr  = (BYTE *)lpSrcData 
						+ ((cScans - 1) * SrcDataPitch);                       
            SrcDataPitch = -(int)SrcDataPitch;      // walk the dib backwards 
        }
    }

    // XXX FIXME
    // check that the lpsrc is/isn't always in host memory, or at least
    // isn't ever the screen, so we don't have to include it in the s/w
    // cursor exclusion test
    //
    FXENTER("Host2Scrn8bppConvBlt", lpDst->deFlags, TRUE,
        cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
        FXENTER_RECT, CLeft, CTop, CRight, CBottom,
        FXENTER_NO_RECT, 0, 0, 0, 0,   );
        //  
        //  SET UP HARDWARE FOR COLOR HOST TO SCREEN BITBLT  
        //

        WAXBUG_3DNOPFIX;

        CMDFIFO_CHECKROOM(cf, 6);
        SETPH(cf,  (SSTCP_PKT2   |
                    srcFormatBit |
                    srcXYBit     |
                    dstSizeBit   |
                    dstXYBit     |
                    commandBit)); 
        // Source Format is 32bpp
        dwtemp = (PixelsInScanline << 2) | SSTG_PIXFMT_32BPP;
        SET(cf, _FF(lpGRegs)->srcFormat, dwtemp);
        SET(cf, _FF(lpGRegs)->srcXY, 0);
        SET(cf, _FF(lpGRegs)->dstSize, R32((CBottom - CTop), (CRight - CLeft)));
        SET(cf, _FF(lpGRegs)->dstXY, R32(CTop, CLeft));
        SETC(cf, _FF(lpGRegs)->command, SSTG_ROP_SRCCOPY | SSTG_HOST_BLT);
        BUMP(6);

        //
        //   TRANSLATE AND TRANSFER DATA TO FIFO
        //

        lpData = SrcDataPtr;
        TotalDwords = PixelsInScanline * (CBottom - CTop);
    
        //    ColorTable = (DWORD *)lpSrc->bmiColors;
        //PingZ 3/12/99 lpSrc->bmiColors may not be pointing to the actual color table!!!  The only
        //to guarantee that is to add bmiHeader.biSize to lpSrc.  This is to fix PRS #4604
        ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;
        
        if (TotalDwords < MAXCOLORDWORDS)
        {
        // Write whole transfer at once
            CMDFIFO_CHECKROOM(cf, TotalDwords+1);
            SETPH(cf, (SSTCP_PKT1 |
                   SSTCP_PKT1_2D |
                   (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT)  |
                   (TotalDwords  << SSTCP_PKT1_NWORDS_SHIFT)));

            for (i = 0; i < (CBottom - CTop); i++)
            {
                for (j = 0; j < PixelsInScanline; j++)
                {
                    DataDWord = ColorTable[lpData[j]];
                    SET(cf,_FF(lpGRegs)->launch[0], DataDWord);
                }
                (DWORD)lpData += SrcDataPitch;
            }

            BUMP(TotalDwords + 1);
        }
        else
        {
            // Write transfer one scanline at a time.
            //
            for (i = 0; i < (CBottom - CTop); i++)
            {
                CMDFIFO_CHECKROOM(cf, PixelsInScanline + 1);
                SETPH(cf, (SSTCP_PKT1 |
                       SSTCP_PKT1_2D |
                       (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
                       (PixelsInScanline << SSTCP_PKT1_NWORDS_SHIFT)));

                for (j = 0; j < PixelsInScanline; j++)
                {	    			
                    DataDWord = ColorTable[lpData[j]];
                    SET(cf,_FF(lpGRegs)->launch[0], DataDWord);                    
                }	

                (DWORD)lpData += SrcDataPitch;
                
                BUMP(PixelsInScanline + 1);
            }
        }                   
    
    WAXBUG_3DNOPFIX;
    FXLEAVE2("Host2Scrn8bppConvBlt", cf, lpDst, CBottom-CTop);

    return CBottom-CTop;
}


/*----------------------------------------------------------------------
Function name:  Host2Scrn8To16ConvBlt

Description:    Performs colortable lookup on 256-entry palette and 
                writes 16bpp data to HW as a host-to-screen blit after
                converting the colors from 32-bit RGB values (from
                the 8bpp color lookup table) into 16-bit 565 colors.
                
Information:    Can be called by FxSetDIBitsToDevice and FxDeviceBitmapBits.
                Calls from FxStretchDIBits have been seperated out to 
                StretchHost2Scrn8to16ConvBlt so as to reduce the number of params
                passed on the stack. This improves high end WB99 performance.

Return:         INT     1 if success or,
                        0 if rejected.
----------------------------------------------------------------------*/
int  Host2Scrn8To16ConvBlt(LPDIBENGINE     lpDst,           // device dib
                           RECT FAR *      lpClipRect,
                           WORD            cScans, 
                           int             DstX,
                           int             DstY,
                           DWORD *         lpSrcData,       // buffer
                           LPBITMAPINFO    lpSrc )   
{
    RECT   * lpRect;            // Local pointer to Clipping Rectangle 
    BYTE   * SrcDataPtr;        // Pointer to actual start of clipped DIB data
    BYTE   * lpData;            // Working pointer to source data
    LONG     CTop;              // Clipped extents
    LONG     CBottom;
    LONG     CLeft;
    LONG     CRight;
    DWORD    SrcDataPitch;      // Pitch of source DIB data, in bytes
    DWORD    TotalDwords;       // Total # of DWORDS to be xferred into FIFO 
    DWORD    DataDword;         // Temp for RGB data xlated from lpColors 
    DWORD    TopClippedAmount;  // # of Scans clipped from 'top' of DIB 
    DWORD    LeftClippedAmount; // # of BYTES clipped from left of DIB
    DWORD    dwtemp;            // Temp data storage used for FIFO macros
    LONG     i;                 
    DWORD    j;
    DWORD  * ColorTable;        // Pointer to DIB color conversion table
    DWORD    PixelsInScanline;  // We'll be pushing two pixels per dword
    DWORD    DwordsInScanline;  // ACTUAL number of dwords of data in Scanline
    DWORD    RGBDword;          // Raw RGB data from DIB color table
    DWORD    LastRGB;           // Previously Looked up RGB value
    DWORD    Last565;           // Result of previous lookup
    DWORD    blubyte;           // Blue data from RGB
    DWORD    grnbyte;           // Green data from RGB
    DWORD    redbyte;           // Red data from RGB
    DWORD    clrUsed;           // Number of palette entries that are actually used
    BOOL     bUse565Table;      // This is set if the 32bpp palette is converted to 16bpp
        
    CMDFIFO_PROLOG(cf);
    DEBUG_FIX;
    
    bUse565Table = FALSE;
    // DIBPitch for 8bpp dib is #pixels rounded to next dword boundary
    SrcDataPitch = (lpSrc->bmiHeader.biWidth + 3) & 0xFFFFFFFC;
    
    if (lpClipRect) 
    {
        // If lpRect Non-Null, intersect with default dest rect
        // Get usable pointer to clip rectangle
        FarToFlat(lpClipRect, lpRect);
    
        CTop    = max(lpRect->top,DstY);
        CLeft   = max(lpRect->left,DstX);
        CBottom = min(lpRect->bottom,DstY+(int)cScans);
        CRight  = min(lpRect->right,DstX+(int)lpSrc->bmiHeader.biWidth);
        
        // Validate extents
        if ((CRight <= CLeft) || (CBottom <= CTop))
        {
            return 0;
        }
         
        TopClippedAmount  = lpRect->top  - (DWORD)DstY;
        LeftClippedAmount = lpRect->left - (DWORD)DstX;
        PixelsInScanline  = CRight - CLeft;

        
        if ((long)(lpSrc->bmiHeader.biHeight) < 0) // top-down DIB
        {
			SrcDataPtr = (BYTE*)lpSrcData
					   + (TopClippedAmount * SrcDataPitch)
					   + LeftClippedAmount;
        }
        else
        {
			SrcDataPtr	= (BYTE *)lpSrcData
						+ ((cScans - TopClippedAmount - 1) * SrcDataPitch)
						+ LeftClippedAmount;   
            SrcDataPitch = -(int)SrcDataPitch;                                   
        }

    }
    else
    {
        CTop    = DstY;
        CLeft   = DstX;
        CBottom = DstY+(int)cScans;
        CRight  = DstX+lpSrc->bmiHeader.biWidth;

        PixelsInScanline  = CRight - CLeft;
        
        if ((long)(lpSrc->bmiHeader.biHeight) < 0) // top-down DIB
        {
			SrcDataPtr = (BYTE*)lpSrcData;
        }
        else
        {
			SrcDataPtr  = (BYTE *)lpSrcData 
						+ ((cScans - 1) * SrcDataPitch);
            SrcDataPitch = -(int)SrcDataPitch;               
        }
    }
    
    // How many pixels are in the bitmap? If it's more than 256 we'll create a new palette 
    // containing RGB565 colours. That way we don't end up converting the same colour more
    // than once.
    clrUsed = (lpSrc->bmiHeader.biClrUsed == 0) ? 256 : lpSrc->bmiHeader.biClrUsed;
    if ( (PixelsInScanline * (CBottom - CTop)) > 256 )
    {
        ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;
        for ( j = 0; j < clrUsed; j++ )
        {
            RGBDword = ColorTable[j];
            blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
            grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
            redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
            RGB565ColorTable[j] = (WORD) (blubyte | (grnbyte << 5) | (redbyte << 11));
        }
        bUse565Table = TRUE;
    }
    
    // XXX FIXME
    // check that the lpsrc is/isn't always in host memory, or at least
    // isn't ever the screen, so we don't have to include it in the s/w
    // cursor exclusion test
    //
    FXENTER("Host2Scrn8To16ConvBlt", lpDst->deFlags, TRUE,
        cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
        FXENTER_RECT, CLeft, CTop, CRight, CBottom,
        FXENTER_NO_RECT, 0, 0, 0, 0,   );

    //  
    //  SET UP HARDWARE FOR COLOR HOST TO SCREEN BITBLT  
    //
    CMDFIFO_CHECKROOM(cf, 6);
    SETPH(cf,  (SSTCP_PKT2   |
                srcFormatBit |
                srcXYBit     |
                dstSizeBit   |
                dstXYBit     |
                commandBit)); 
    
    dwtemp = (PixelsInScanline + 1) & 0xFFFFFFFE;   // Round to even number of pixels
    dwtemp = (dwtemp << 2) | SSTG_PIXFMT_16BPP;        
    // Source Format is 32bpp
    SET(cf, _FF(lpGRegs)->srcFormat, dwtemp);
    SET(cf, _FF(lpGRegs)->srcXY, 0);
    SET(cf, _FF(lpGRegs)->dstSize, R32((CBottom - CTop), (CRight - CLeft)));
    SET(cf, _FF(lpGRegs)->dstXY, R32(CTop, CLeft));
    SETC(cf, _FF(lpGRegs)->command, SSTG_ROP_SRCCOPY | SSTG_HOST_BLT);
    BUMP(6);

    //
    //   TRANSLATE AND TRANSFER DATA TO FIFO
    //

    lpData = SrcDataPtr;
    DwordsInScanline = ((PixelsInScanline << 1) + 2) >> 2;
    TotalDwords      = DwordsInScanline * (CBottom - CTop);

    if (TotalDwords < MAXCOLORDWORDS)
    {
        // Write whole transfer at once
        CMDFIFO_CHECKROOM(cf, TotalDwords+1);
        SETPH(cf, (SSTCP_PKT1                                |
                   SSTCP_PKT1_2D                             |
                   (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT)     |
                   (TotalDwords  << SSTCP_PKT1_NWORDS_SHIFT)) );

        if (bUse565Table)
        {   
            // The 32-bit palette has been translated and copied into
            // the 16bpp table.                 
            for (i = 0; i < (CBottom - CTop); i++)
            {
                for (j = 0; j < (PixelsInScanline & 0xFFFFFFFE); j+=2)
                {                
                    DataDword = RGB565ColorTable[lpData[j]];                                               
                    DataDword |= (RGB565ColorTable[lpData[j+1]] << 16);                                
                    SET(cf,_FF(lpGRegs)->launch[0], DataDword);                      
                }
        
                if (PixelsInScanline & 1)
                {                
                    DataDword = RGB565ColorTable[lpData[PixelsInScanline-1]];
                    SET(cf,_FF(lpGRegs)->launch[0], DataDword);
                }
                (DWORD)lpData += SrcDataPitch;
            }
        }
        else 
        {
            ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;
            LastRGB = 0xFFFFFFFF;
            // The DIB is less than 256 pixels, therefore there is no point
            // converting 256 colours to 16bpp. Do the conversion as we read
            // the 32bpp values from the palette
            for (i = 0; i < (CBottom - CTop); i++)
            {
                for (j = 0; j < (PixelsInScanline & 0xFFFFFFFE); j+=2)
                {
                    RGBDword = ColorTable[lpData[j]];
                    if (RGBDword == LastRGB) 
                    {
                        DataDword = Last565;
                    }    
                    else
                    {
                        LastRGB = RGBDword;
                        blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
                        grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
                        redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
                        DataDword= (blubyte | (grnbyte << 5) | (redbyte << 11));
                        Last565  = DataDword;
                    }
                                                
                    RGBDword = ColorTable[lpData[j+1]];
                    if (RGBDword == LastRGB) 
                    {
                        DataDword |= (Last565 << 16);
                    }
                    else
                    {
                        LastRGB = RGBDword;
                        blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
                        grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
                        redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
                        Last565 = (blubyte | (grnbyte << 5) | (redbyte << 11));
                        DataDword |= (Last565 << 16);
                    }            
                        
                    SET(cf,_FF(lpGRegs)->launch[0], DataDword);
                      
                }
    
                if (PixelsInScanline & 1)
                {
                    RGBDword = ColorTable[lpData[PixelsInScanline-1]];

                    if (RGBDword == LastRGB) 
                    {
                        DataDword = Last565;
                    }
                    else
                    {
                        blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
                        grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
                        redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
                        DataDword= (blubyte | (grnbyte << 5) | (redbyte << 11));
                    } 
        
                    SET(cf,_FF(lpGRegs)->launch[0], DataDword);

                }

                (DWORD)lpData += SrcDataPitch;
            }                
        }

        BUMP(TotalDwords + 1);
    }
    else
    {
        // Write transfer one scanline at a time.
        //
        for (i = 0; i < (CBottom - CTop); i++)
        {
            CMDFIFO_CHECKROOM(cf, DwordsInScanline + 1);
            SETPH(cf, (SSTCP_PKT1                            |
                       SSTCP_PKT1_2D                         |
                       (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
                       (DwordsInScanline << SSTCP_PKT1_NWORDS_SHIFT)));

            for (j = 0; j < (PixelsInScanline & 0xFFFFFFFE); j+=2)
            {
            
                DataDword = RGB565ColorTable[lpData[j]];
                DataDword |= (RGB565ColorTable[lpData[j+1]] << 16);                
                SET(cf,_FF(lpGRegs)->launch[0], DataDword);                  
            }
        
            if (PixelsInScanline & 1)
            {
                DataDword = RGB565ColorTable[lpData[PixelsInScanline-1]];                
                SET(cf,_FF(lpGRegs)->launch[0], DataDword);
            }

            (DWORD)lpData += SrcDataPitch;
               
            BUMP(PixelsInScanline + 1);
       }
   }   
       
    FXLEAVE2("Host2Scrn8To16ConvBlt", cf, lpDst, CBottom-CTop);
    return CBottom-CTop;
}


/*----------------------------------------------------------------------
Function name:  Host2Scrn4to16ConvBlt

Description:    This function operates in one of two ways depending
                on the size of the source DIB.
                If it is small 16-entry LUT is created containing 16-bit
                hicolor values.  It then reads in a byte of DIB
                data at a time and performs two lookups on the
                4-bit indices, producing 32-bits of data to go to
                the FIFO.  The 16bpp data is then bltted to the
                screen.
                If the DIB is large it creates a 256 entry LUT that
                contains 2 16-bit pixels per entry. This way 2 pixels
                from the source can be read, converted and written 
                at once.
                
Information:    Can be called by FxStretchDIBits, FxSetDIBitsToDevice
                and FxDeviceBitmapBits.

Return:         INT     1 if success or,
                        0 if rejected.
----------------------------------------------------------------------*/
int
Host2Scrn4to16ConvBlt(LPDIBENGINE     lpDst,              // device dib
                     RECT FAR *      lpClipRect,
                     WORD            cScans, 
                     int             DstX,
                     int             DstY,
                     int             SrcX,
                     int             SrcY,              
                     DWORD *         lpSrcData,          // buffer
                     LPBITMAPINFO    lpSrc,
                     DIB_Brush8 FAR* lpPBrush,
                     DRAWMODE FAR*   lpDrawMode,
                     DWORD           dwROP )   
{
    BYTE   * lpData;                  // Working pointer to source data
    BYTE   * SrcDataPtr;              // Ptr to start of clipped DIB data 
    RECT   * lpRect;                  // Local pointer for clip rect
    DRAWMODE   * pDrawMode;        
    DIB_Brush8 * pBrush;           
    LONG     CTop;                    // Clipped DIB extents
    LONG     CBottom;
    LONG     CLeft;
    LONG     CRight;
    DWORD    SrcDataPitch;            // Pitch of source DIB data, in BYTES
    DWORD    TotalDwords;             // Total # of DWORDS to be sent to FIFO
    DWORD    DataDWord;               // Temp Storage for destination datadword
    DWORD    TopClippedAmount;        // # of scanlines clipped from top of DIB
    DWORD    LeftClippedAmountBytes;  // # of BYTES clipped from left of DIB
    DWORD    LeftClippedAmountPixels; // # of BYTES clipped from left of DIB
    DWORD    PixelsInScanline;        // # if pixels in dest scanline
    DWORD    dwtemp;                  // Temp var for FIFO macros
    DWORD    BytesInScanline;         // # of BYTES with DIB data/scanline 
    LONG     i;                     
    DWORD    j;
    DWORD  * ColorTable;              // Local ptr to DIB color xlation tbl
    DWORD    cmdData;                 // value to be written into the command register    
    DWORD    tblindex;                // Index variable for building CLUT
    DWORD    blubyte,grnbyte,redbyte; // used for creating color mapping table
    BOOL     bUseDoubleLookups;
    TAKE_ADDRESS DWORD hclut[256];    // Color Lookup Table -- converts two
                                      // 4-bit indices into 2 16-bit hicolor
                                      // values 
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    // See Host2Scrn8bppConvBlt for background on clipping, data retrieval

    //
    //  SET UP DESTINATION/TRANSFER VARIABLES
    //
    
    // DIBPitch for 4bpp dib is #pixels rounded to next byte boundary 
    // divided by two to yield bytes, rounded to next dword boundary.
    SrcDataPitch = (((lpSrc->bmiHeader.biWidth + 1) >> 1) + 3) & 0xFFFFFFFC;
    
    BytesInScanline = 0;    
    bUseDoubleLookups = FALSE;

    if (lpClipRect) {
        // If lpRect Non-Null, intersect with default dest rect
    
        // Get usable pointer to clip rectangle
        FarToFlat(lpClipRect, lpRect);
    
        CTop    = max(lpRect->top,DstY);
        CLeft   = max(lpRect->left,DstX);
        CBottom = min(lpRect->bottom,DstY+(int)cScans);
        CRight  = min(lpRect->right,DstX+(int)lpSrc->bmiHeader.biWidth);
        
        // Added to fix Defect 1703
        // What happen is CRight is less then CLeft we get a negative PixelsInScanLine
        // There is never enough room and life gets worse until we come to a sw halt
        // Might as well check for a bogus y while we are at it
        if ((CRight <= CLeft) || (CBottom <= CTop))
        {
            return 0;
        }

        TopClippedAmount       = lpRect->top  - DstY;
        LeftClippedAmountPixels= lpRect->left - DstX;
        LeftClippedAmountBytes = (lpRect->left - DstX) >> 1;
        PixelsInScanline       = CRight - CLeft;

        SrcX += LeftClippedAmountPixels;

        if ((long)(lpSrc->bmiHeader.biHeight) < 0)  // top-down DIB
        {
            SrcDataPtr = (BYTE*)lpSrcData + (SrcX >> 1) +
                         (-((long)lpSrc->bmiHeader.biHeight)
                          - SrcY - cScans + TopClippedAmount
                         ) * SrcDataPitch;
        }
        else
        {
            SrcDataPtr	= (BYTE *)lpSrcData + (SrcX >> 1) 
                        + ((SrcY + cScans - TopClippedAmount - 1) * SrcDataPitch);
            SrcDataPitch = -(int)SrcDataPitch;               
        }
    }
    else
    {                          
        CTop    = DstY;
        CLeft   = DstX;
        CBottom = DstY+(int)cScans;
        CRight  = DstX+lpSrc->bmiHeader.biWidth;
        
        PixelsInScanline = CRight - CLeft;

        if ((long)(lpSrc->bmiHeader.biHeight) < 0)  // top-down DIB
        {
            SrcDataPtr = (BYTE*)lpSrcData + (SrcX >> 1) +
                         (-((long)lpSrc->bmiHeader.biHeight)
                          - SrcY - cScans
                         ) * SrcDataPitch;
        }
        else
        {
            SrcDataPtr = (BYTE *)lpSrcData + (SrcX >> 1)
                       + ((SrcY + cScans - 1) * SrcDataPitch);        
            SrcDataPitch = -(int)SrcDataPitch;               
        }
    }  

    // Each pixel is represented by 4 bits, therefore if the starting X position
    // in pixels or the width in pixels is not on a byte boundry, round up the number
    // of bytes in the scanline. We set up the hardware to ignore the extra nybble
    // sent.
   
    if ((SrcX & 0x01) || (PixelsInScanline & 0x01)) {
        BytesInScanline = (PixelsInScanline >> 1) + 1;
    } else {
        BytesInScanline = PixelsInScanline >> 1;
    } 

    //
    //  BUILD THE HICOLOR LOOKUP TABLE
    //
    //    ColorTable = (DWORD *)lpSrc->bmiColors;
    //PingZ 3/12/99 lpSrc->bmiColors may not be pointing to the actual color table!!!  The only
    //to guarantee that is to add bmiHeader.biSize to lpSrc.  This is to fix PRS #4604
    ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;
    
    // Build 16-entry 16bpp (hicolor) lookup table for 4bpp->16bpp conversions
    for (i = 0; i < 16; i++) 
    {
        blubyte = (DWORD)ByteTo5BitLookup[ColorTable[i] & 0x000000FF];
        grnbyte = (DWORD)ByteTo6BitLookup[(ColorTable[i] & 0xFF00) >> 8 ];
        redbyte = (DWORD)ByteTo5BitLookup[(ColorTable[i] & 0xFF0000) >> 16];
        hclut[i]= (blubyte | (grnbyte << 5) | (redbyte << 11));
    }                     
    
    if ( (PixelsInScanline * (CBottom - CTop) ) > 512)
    {
        // Build 256-entry byte-lookup table for handling two nybble 
        // (pixel) lookups at a time
        tblindex = 0;
    
        for (i=0; i<16; i++) {
            for (j=0; j<16; j++) {        
                hclut[tblindex++] = (DWORD)(hclut[j] & 0x0000FFFF) |
                                    ((hclut[i] & 0x0000FFFF) << 16);
            }                                           
        }
        bUseDoubleLookups = TRUE;
    }
    
    FXENTER("Host2Scrn4to16ConvBlt", lpDst->deFlags, TRUE,
        cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
        FXENTER_RECT, CLeft, CTop, CRight, CBottom,
        FXENTER_NO_RECT, 0, 0, 0, 0,   );
    
    if (RopUsesSrc(dwROP >> 16))
    {
        if (RopUsesPat(dwROP >> 16) && (lpDrawMode != 0) && (lpPBrush != 0) )
        {
            // If there is a pattern, load it into the HW            
            CMDFIFO_SAVE(cf);
            FarToFlat(lpDrawMode, pDrawMode);
            FarToFlat(lpPBrush, pBrush);
            cmdData = HandleBrush(pBrush, dwROP, pDrawMode);
            CMDFIFO_RELOAD(cf);
        } 
        else
        {
            cmdData = (dwROP >> 16) << 24;   
        }      
        
        //  
        //  SET UP HARDWARE FOR COLOR HOST TO SCREEN BITBLT  
        //
        CMDFIFO_CHECKROOM(cf, 6);
        SETPH(cf,  SSTCP_PKT2   |
          srcFormatBit |
          srcXYBit     |
          dstSizeBit   |
          dstXYBit     |
          commandBit); 

        // Source Format is 16bpp, set src pitch, swizzle words if necessary
        
        dwtemp = (BytesInScanline << 2) | SSTG_PIXFMT_16BPP;
        if (bUseDoubleLookups)
            dwtemp |= SSTG_HOST_WORD_SWIZZLE;
        SET(cf, _FF(lpGRegs)->srcFormat, dwtemp);
    
        // Set Start offset (0 or 1 pixel, 0 or 2 bytes)

        if (SrcX & 0x01) {
            dwtemp = 2;     // Pixel at start we must skip
        } else {
            dwtemp = 0;  
        }
        SET(cf, _FF(lpGRegs)->srcXY, dwtemp);
    
        // Each byte will translate into one dword of FIFO data
        SET(cf, _FF(lpGRegs)->dstSize, R32(CBottom - CTop, PixelsInScanline));
        
    
        // Destination X,Y blt start location
        SET(cf, _FF(lpGRegs)->dstXY, R32(CTop, CLeft));
                
        SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_HOST_BLT);
        BUMP(6);

        //
        //   TRANSLATE AND TRANSFER DATA TO FIFO
        //
    
        // POSSIBLE OPTIMIZATIONS:  Several areas can be improved by removing
        // the use of local temporary variables.  Also, dwtemp is reused many
        // times which may create unnecessary dependencies.  In addition, it
        // appears that 99% of all 4bpp deviceDIB calls are 16x16, and it may
        // be faster to do dual lookups on the nybble pairs than to build
        // the double-lookup table.
        // 12-9-99 - SteveH implemented the dual lookups. This helps WB99.

        lpData = SrcDataPtr;
        TotalDwords = BytesInScanline * (CBottom - CTop);
    
        // Grab byte of src DIB data (2 pixels), xlate into 16bpp FIFO data 
        // (still 2 pixels, 1 DWORD), and write to FIFO.
    
        if (TotalDwords < MAXCOLORDWORDS)	
        {   
            //
            // Write whole transfer at once
            //
            
            CMDFIFO_CHECKROOM(cf, TotalDwords + 1);
            SETPH(cf, SSTCP_PKT1                      |
                  SSTCP_PKT1_2D                       |
                  LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT |
                  (TotalDwords) << SSTCP_PKT1_NWORDS_SHIFT);

            if (bUseDoubleLookups)
            {
                // Lookup 2 pixels at a time from the 256 entry clut
                for (i = 0; i < (CBottom - CTop); i++)  
                {
                    for (j = 0; j < BytesInScanline; j++)  
            
                    {            
                        DataDWord = hclut[lpData[j]];
                    SET(cf,_FF(lpGRegs)->launch[0], DataDWord);
                    }
                    (DWORD)lpData += SrcDataPitch;
                }

            }
            else
            { 
                // The bitmap is small so it wasn't worthwhile building the 
                // 256 entry clut, instead read 1 pixel at a time from the
                // 16 entry clut.
                for (i = 0; i < (CBottom - CTop); i++)  
                {
                    for (j = 0; j < BytesInScanline; j++)  
                    {            
                        DataDWord = hclut[lpData[j] >> 4];
                        DataDWord |= hclut[lpData[j] & 0x0F] << 16;
                        SET(cf,_FF(lpGRegs)->launch[0], DataDWord);
                    }
        
                    (DWORD)lpData += SrcDataPitch;
                }                   
            }
            BUMP(TotalDwords + 1);        
        } 
        else 
        {
            //
            // Transfer one scanline at a time.
            //                                
            
            for (i = 0; i < (CBottom - CTop); i++)  
            {        
                CMDFIFO_CHECKROOM(cf, BytesInScanline + 1);
                SETPH(cf, SSTCP_PKT1                      |
                  SSTCP_PKT1_2D                           |
                  (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT)   |
                  (BytesInScanline << SSTCP_PKT1_NWORDS_SHIFT));

                for (j = 0; j < BytesInScanline; j++)  
                {	    		
                    DataDWord = hclut[lpData[j]];
                    SET(cf,_FF(lpGRegs)->launch[0], DataDWord);                
                }	

                (DWORD)lpData += SrcDataPitch;
                BUMP(BytesInScanline + 1);
            }
        }    
    }
    
    else
    {
        // There is no source
        CMDFIFO_CHECKROOM(cf, 4);
        CMDFIFO_SAVE(cf);
        FarToFlat(lpDrawMode, pDrawMode);
        FarToFlat(lpPBrush, pBrush);
        cmdData = HandleBrush(pBrush, dwROP, pDrawMode);
        CMDFIFO_RELOAD(cf);       
        SETPH(cf, SSTCP_PKT2 | dstSizeBit | dstXYBit | commandBit);
        dwtemp = ((((DWORD)(CBottom - CTop )) << 16) |
                   ((DWORD)(PixelsInScanline)));
        SET(cf, _FF(lpGRegs)->dstSize,dwtemp);
        dwtemp  = (CTop << 16) | CLeft;                  
        SET(cf, _FF(lpGRegs)->dstXY , dwtemp);                
        SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_GO | SSTG_RECTFILL);
        BUMP(4);
    }            

    FXLEAVE2("Host2Scrn4to16ConvBlt", cf, lpDst, CBottom-CTop);   
    return CBottom-CTop;
}


/*----------------------------------------------------------------------
Function name:  Host2Scrn4to2432ConvBlt

Description:    This functions reads in a byte of DIB
                data at a time and uses either the upper or lower nybble
                to index the 16 entry 32bpp colour palette. The 32bpp
                data is then written to the FIFO.
                
Information:    Can be called by FxStretchDIBits, FxSetDIBitsToDevice
                and FxDeviceBitmapBits.

Return:         INT     1 if success or,
                        0 if rejected.
----------------------------------------------------------------------*/
int
Host2Scrn4to2432ConvBlt(LPDIBENGINE  lpDst,              // device dib
                        RECT FAR *      lpClipRect,
                        WORD            cScans, 
                        int             DstX,
                        int             DstY,
                        int             SrcX,
                        int             SrcY,
                        DWORD *         lpSrcData,          // buffer
                        LPBITMAPINFO    lpSrc,
                        DIB_Brush8 FAR* lpPBrush,
                        DRAWMODE FAR*   lpDrawMode,
                        DWORD           dwROP )   
{
    BYTE   * lpData;                  // Working pointer to source data
    BYTE   * SrcDataPtr;              // Ptr to start of clipped DIB data 
    RECT   * lpRect;                  // Local pointer for clip rect
    DRAWMODE   * pDrawMode;        
    DIB_Brush8 * pBrush;               
    LONG     CTop;                    // Clipped DIB extents
    LONG     CBottom;
    LONG     CLeft;
    LONG     CRight;
    DWORD    SrcDataPitch;            // Pitch of source DIB data, in BYTES
    DWORD    TotalDwords;             // Total # of DWORDS to be sent to FIFO
    DWORD    DataDWord;               // Temp Storage for destination datadword
    DWORD    TopClippedAmount;        // # of scanlines clipped from top of DIB
    DWORD    LeftClippedAmountBytes;  // # of BYTES clipped from left of DIB
    DWORD    LeftClippedAmountPixels; // # of BYTES clipped from left of DIB
    DWORD    PixelsInScanline;        // # if pixels in dest scanline
    DWORD    dwtemp;                  // Temp var for FIFO macros
    DWORD    BytesInScanline;         // # of BYTES with DIB data/scanline 
    LONG     i;                     
    DWORD    j;
    DWORD  * ColorTable;              // Local ptr to DIB color xlation tbl
    DWORD    nibble;                  // = 0,1 for which nibble in byte to use
    DWORD    pixbyte;                 // real byte counter for pixel loop
    DWORD    cmdData;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    // See Host2Scrn8bppConvBlt for background on clipping, data retrieval

    //
    //  SET UP DESTINATION/TRANSFER VARIABLES
    //
    
    // DIBPitch for 4bpp dib is #pixels rounded to next byte boundary 
    // divided by two to yield bytes, rounded to next dword boundary.
    SrcDataPitch = (((lpSrc->bmiHeader.biWidth + 1) >> 1) + 3) & 0xFFFFFFFC;
    
    BytesInScanline=0;    

    if (lpClipRect) {
    // If lpRect Non-Null, intersect with default dest rect
    
        // Get usable pointer to clip rectangle
        FarToFlat(lpClipRect, lpRect);
    
        CTop    = max(lpRect->top,DstY);
        CLeft   = max(lpRect->left,DstX);
        CBottom = min(lpRect->bottom,DstY+(int)cScans);
        CRight  = min(lpRect->right,DstX+(int)lpSrc->bmiHeader.biWidth);
        
        // Added to fix Defect 1703
        // What happen is CRight is less then CLeft we get a negative PixelsInScanLine
        // There is never enough room and life gets worse until we come to a sw halt
        // Might as well check for a bogus y while we are at it
        if ((CRight <= CLeft) || (CBottom <= CTop))
        {
            return 0;
        }

        TopClippedAmount       = lpRect->top  - DstY;
        LeftClippedAmountPixels= lpRect->left - DstX;
        LeftClippedAmountBytes = (lpRect->left - DstX) >> 1;
        
        SrcX += LeftClippedAmountPixels;        

        if ((long)(lpSrc->bmiHeader.biHeight) < 0)  // top-down DIB
        {
            SrcDataPtr = (BYTE*)lpSrcData + (SrcX >> 1) +
                         (-((long)lpSrc->bmiHeader.biHeight)
                          - SrcY - cScans + TopClippedAmount
                         ) * SrcDataPitch;
        }
        else
        {
            SrcDataPtr	= (BYTE *)lpSrcData + (SrcX >> 1)
                        + ((SrcY + cScans - TopClippedAmount - 1) * SrcDataPitch);
            SrcDataPitch = -(int)SrcDataPitch;
        }        
    } 
    else 
    {                      
    
        CTop    = DstY;
        CLeft   = DstX;
        CBottom = DstY+(int)cScans;
        CRight  = DstX+lpSrc->bmiHeader.biWidth;
        
        if ((long)(lpSrc->bmiHeader.biHeight) < 0)  // top-down DIB
        {
        	SrcDataPtr =  (BYTE*)lpSrcData + (SrcX >> 1) +
                        (-((long)lpSrc->bmiHeader.biHeight)
                         - SrcY - cScans
                        ) * SrcDataPitch;
        }   
        else
        {
            SrcDataPtr = (BYTE *)lpSrcData + (SrcX >> 1)
                       + ((SrcY + cScans - 1) * SrcDataPitch);
            SrcDataPitch = -(int)SrcDataPitch;
        }
    }    

    //PingZ 3/18/99 For unknown reasons writing more than 1024 DWORDS to launch area in this case
    //creates bad corruption in 10x7x32 mode.  I looked at the "for loop" code, didn't see anything
    //wrong.  So, to wrok around, let's punt.  There is no WB99 performance hit for doing this. 
    //PRS #5207
    PixelsInScanline = CRight - CLeft;
    TotalDwords = PixelsInScanline * (CBottom - CTop);
    if(TotalDwords > MAXCOLORDWORDS)
      return 0;

    // Each pixel is represented by 4 bits, therefore if the starting X position
    // in pixels or the width in pixels is not on a byte boundry, round up the number
    // of bytes in the scanline. We set up the hardware to ignore the extra nybble
    // sent.
    
    if ((SrcX & 0x01) || (PixelsInScanline & 0x01)) {
        BytesInScanline = (PixelsInScanline >> 1) + 1;
    } else {
        BytesInScanline = PixelsInScanline >> 1;
    } 

    FXENTER("Host2Scrn4to2432ConvBlt", lpDst->deFlags, TRUE,
        cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
        FXENTER_RECT, CLeft, CTop, CRight, CBottom,
        FXENTER_NO_RECT, 0, 0, 0, 0,   );

    if (RopUsesSrc(dwROP >> 16))
    {
        // Use the Source
        if (RopUsesPat(dwROP >> 16) && (lpDrawMode != 0) && (lpPBrush != 0) )
        {
            // If there is a pattern, load it into the HW            
            CMDFIFO_SAVE(cf);
            FarToFlat(lpDrawMode, pDrawMode);
            FarToFlat(lpPBrush, pBrush);
            cmdData = HandleBrush(pBrush, dwROP, pDrawMode);
            CMDFIFO_RELOAD(cf);
        } 
        else
        {
            cmdData = (dwROP >> 16) << 24;   
        }      
                
        //  
        //  SET UP HARDWARE FOR COLOR HOST TO SCREEN BITBLT  
        //
        CMDFIFO_CHECKROOM(cf, 6);
        SETPH(cf,  (SSTCP_PKT2 | srcFormatBit | srcXYBit | dstSizeBit |
                 dstXYBit | commandBit)); 

        // Source Format is 32bpp, set src pitch
        dwtemp = (PixelsInScanline << 2) | SSTG_PIXFMT_32BPP;
        SET(cf, _FF(lpGRegs)->srcFormat, dwtemp);

        // No intra dword source clipping with 32bpp data    
        SET(cf, _FF(lpGRegs)->srcXY, 0);
    
        // Each byte will translate into one dword of FIFO data
        SET(cf, _FF(lpGRegs)->dstSize, R32(CBottom - CTop, PixelsInScanline));
    
        // Destination X,Y blt start location
        SET(cf, _FF(lpGRegs)->dstXY, R32(CTop, CLeft));
    
        SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_HOST_BLT);
        BUMP(6);

        //
        //   TRANSLATE AND TRANSFER DATA TO FIFO
        //
    
        lpData = SrcDataPtr;
        TotalDwords = PixelsInScanline * (CBottom - CTop);
    
        //PingZ 3/12/99 lpSrc->bmiColors may not be pointing to the actual color table!!!  The only
        //to guarantee that is to add bmiHeader.biSize to lpSrc.  This is to fix PRS #4604
        ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;
    
        // Set Start offset (0 or 1 pixel, 0 or 2 bytes)
        nibble = SrcX & 0x01;
        dwtemp = nibble;            // save for later reset
        if (TotalDwords < MAXCOLORDWORDS)
        {
            // Write whole transfer at once
            CMDFIFO_CHECKROOM(cf, TotalDwords + 1);
            SETPH(cf, SSTCP_PKT1                          |
                  SSTCP_PKT1_2D                       |
                  LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT |
                  (TotalDwords) << SSTCP_PKT1_NWORDS_SHIFT);

            for (i = 0; i < (CBottom - CTop); i++)
            {
                pixbyte = 0;
                nibble  = dwtemp;
                for (j = 0; j < PixelsInScanline; j++)
                {
                    if (nibble) {
                        DataDWord = ColorTable[(lpData[pixbyte++]&0x0F)];
                    }
                    else {
                        DataDWord = ColorTable[((lpData[pixbyte]&0xF0)>>4)];
                    }
                    SET(cf,_FF(lpGRegs)->launch[0], DataDWord);
                    nibble ^= 0x01;     // XOR with 1 to flip the nibble bit
                }
                (DWORD)lpData += SrcDataPitch;
            }
            BUMP(TotalDwords + 1);                
        }
        else
        {                                // Transfer one scanline at a time.

            for (i = 0; i < (CBottom - CTop); i++)
            {
            
                CMDFIFO_CHECKROOM(cf, PixelsInScanline + 1);
                SETPH(cf, SSTCP_PKT1                              |
                  SSTCP_PKT1_2D                           |
                  (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT)   |
                      (PixelsInScanline << SSTCP_PKT1_NWORDS_SHIFT));
                    
                pixbyte = 0;
                nibble  = dwtemp;
                for (j = 0; j < PixelsInScanline; j++)  {	    			
                    if (nibble) {
                        DataDWord = ColorTable[(lpData[pixbyte++]&0x0F)];
                    }
                    else {
                        DataDWord = ColorTable[((lpData[pixbyte]&0xF0)>>4)];
                    }
                    SET(cf,_FF(lpGRegs)->launch[0], DataDWord);
                    nibble ^= 0x01;     // XOR with 1 to flip the nibble bit
                }	
                
                (DWORD)lpData += SrcDataPitch;
                BUMP(PixelsInScanline + 1);
            }
        }
    }   
    
    else
    {
        // There is no source
        CMDFIFO_CHECKROOM(cf, 4);
        CMDFIFO_SAVE(cf);
        FarToFlat(lpDrawMode, pDrawMode);
        FarToFlat(lpPBrush, pBrush);
        cmdData = HandleBrush(pBrush, dwROP, pDrawMode);
        CMDFIFO_RELOAD(cf);       
        SETPH(cf, SSTCP_PKT2 | dstSizeBit | dstXYBit | commandBit);
        dwtemp = ((((DWORD)(CBottom - CTop )) << 16) |
                   ((DWORD)(PixelsInScanline)));
        SET(cf, _FF(lpGRegs)->dstSize,dwtemp);
        dwtemp  = (CTop << 16) | CLeft;                  
        SET(cf, _FF(lpGRegs)->dstXY , dwtemp);                
        SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_GO | SSTG_RECTFILL);
        BUMP(4);
    }            
    

    FXLEAVE2("Host2Scrn4to2432ConvBlt", cf, lpDst, CBottom-CTop);

    return CBottom-CTop;
}

#else       // PERF_NEWDIB

/*----------------------------------------------------------------------
Function name:  FxDeviceBitmapBits

Description:    Main 32-bit entry point for GDI DeviceBitmapBits
                function called from the 16-bit side.
Information:

Return:         BOOL     1 if success or,
                        -1 if failure.
----------------------------------------------------------------------*/
BOOL
FxDeviceBitmapBits(DeviceBitmapBitsParams *lp)

//  DIBENGINE FAR  *lpBitmap,   
//  WORD            fGet, 
//  WORD            iStart, 
//  WORD            cScans, 
//  LPVOID          lpDIBits,   
//  LPBITMAPINFO    lpBitmapInfo, 
//  LPDRAWMODE      lpDrawMode,
//  LPINT           lpTranslate)

{
    LPDIBENGINE     lpDst;
    LPBITMAPINFO    lpSrc;
    DWORD         * lpSrcData;
    LONG            retval;
    DWORD           lpTmp16;      // tmp 16:16 pointer
    DWORD         * ColorTable;   // Pointer to DIB color conversion table
    DWORD         * fillptr;
    int             i;
    int             DwordsToFill;      
    
    DEBUG_FIX;

    retval = lp->cScans;
    FarToFlat(lp->lpBitmap, lpDst);

#ifdef NULLDRIVER
    if (nullAll || nullDevBit)
	goto null_return;
#endif // #ifdef NULLDRIVER

    if (_FF(gdiFlags) & SKIP_FLAGS)
         return retval;

    // If this is a GET instead of a SET, PUNT
    if (lp->fGet == 1)
      FXPUNT(FXPUNT_NORMAL);
    
    // PUNT if 1, 4, or 8bpp destination
    if (lpDst->deBitsPixel < 15) 
      FXPUNT(FXPUNT_NORMAL);

    // PUNT if Starting scan is not first scan of DIB
    if (lp->iStart) 
      FXPUNT(FXPUNT_NORMAL);

    FarToFlat(lp->lpBitmapInfo, lpSrc);


// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB //rbissell@stb.com

	if ((long)(lpSrc->bmiHeader.biHeight == 0))
      FXPUNT(FXPUNT_NORMAL);

#else //STBPERF_TOPDOWNDIB

    // We don't handle top-down or zero-height DIBs
    if ((long)(lpSrc->bmiHeader.biHeight) <= 0) 
      FXPUNT(FXPUNT_NORMAL);

#endif //STBPERF_TOPDOWNDIB
// STB End Change


    // We don't handle when number of scans
    // doesn't match DIB's height
    if (lp->cScans != lpSrc->bmiHeader.biHeight)
        FXPUNT(FXPUNT_NORMAL);
        
    // If clipped, PUNT
    if ((lpDst->deWidth  != lpSrc->bmiHeader.biWidth ) ||
        (lpDst->deHeight != lpSrc->bmiHeader.biHeight)   )
	{
      FXPUNT(FXPUNT_NORMAL);
	}
        
    // BLACK OUTPUT DIBITMAPBITS OPTIMIZATION CHECK
    // Look for 16bpp dest special case for Winbench --
    // Color palette is 8 entries, all black.  This simplifies to a
    // black solidfill.

//PingZ 3/12/99 lpSrc->bmiColors may not be pointing to the actual color table!!!  The only
//to guarantee that is to add bmiHeader.biSize to lpSrc.  This is to fix PRS #4604
//    ColorTable = (DWORD *)lpSrc->bmiColors;
    ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;

    if (lpDst->deBitsPixel == 16) {
        if (lpSrc->bmiHeader.biClrUsed == 8) {
            if ((ColorTable[0]  &
                 ColorTable[1]  &
                 ColorTable[2]  &
                 ColorTable[3]  &
                 ColorTable[4]  &
                 ColorTable[5]  &
                 ColorTable[6]  &
                 ColorTable[7]) == 0)  {
                
                // This is an optimization candidate.  Fill with black.
                
                lpTmp16= ((DWORD)(lpDst->deBitsSelector)) << 16;
                FarToFlat(lpTmp16, fillptr)
		    (DWORD)fillptr += lpDst->deBitsOffset;
                
                DwordsToFill = ((lpDst->deWidthBytes>>2)*lp->cScans);
                
                for (i=0; i < DwordsToFill; i++) {
                    *fillptr++ = 0;
                }
                
               FXPUNT(FXPUNT_NORMAL);
            }    
        }
    }
    
    // If we have to deal with wierd compression, PUNT
    if (lpSrc->bmiHeader.biCompression != BI_RGB) 
      FXPUNT(FXPUNT_NORMAL);
    
    switch(lpSrc->bmiHeader.biBitCount)
    {
      case 1:                             // 1  -> 8,16,24
	  // Host2ScrnMonoBlt(...........);
      FXPUNT(FXPUNT_NORMAL);
	  break;
              
      case 4:                             // 4  -> 16,24
	  FarToFlat(lp->lpDIBits, lpSrcData);
	  if (lpDst->deBitsPixel == 16)
	      Host2Scrn4to16ConvBlt( lpDst,
				     (DWORD)NULL,
				     lp->cScans,
				     (int)0,
				     (int)0,
				     lpSrcData,
				     lpSrc );
	  else
	      if(!Host2Scrn4to2432ConvBlt( lpDst,
				       (DWORD)NULL,
				       lp->cScans,
				       (int)0,
				       (int)0,
				       lpSrcData,
				       lpSrc ))
		  {
              FXPUNT(FXPUNT_NORMAL);
	          break;
		  }
                  
	  break;
              
      case 8:                             // 8  -> 16,24
	  FarToFlat(lp->lpDIBits, lpSrcData);
	  if (lpDst->deBitsPixel == 16)
          Host2Scrn8bppTo16bppConvBlt( lpDst,
                                       (DWORD)NULL,
                                       lp->cScans,
                                       (int)0,
                                       (int)0,
                                       lpSrcData,
                                       lpSrc );
      else
	  Host2Scrn8bppConvBlt( lpDst,
				(DWORD)NULL,
				lp->cScans,
				(int)0,
				(int)0,
				lpSrcData,
				lpSrc );
	  break;
                  
      case 16:                            // 16 -> 24
      FXPUNT(FXPUNT_NORMAL);
	  break;
      
      case 24:                            // 24 -> 16
	  // This routine will set up the
	  // hardware with a dstFormat AND a srcFormat, and just ship the
	  // data straight to the HW to let it do the conversion.
	  // Host2ScrnColorConvBlt(......);
      FXPUNT(FXPUNT_NORMAL);
	  break;
    
      default:
	  return(-1);             // Fail if invalid bitcount
	  break;
    }

#ifdef NULLDRIVER
null_return:
#endif // #ifdef NULLDRIVER

    FXLEAVE_NOHWACCESS("FxDeviceBitmapBits", retval);
}


/*----------------------------------------------------------------------
Function name:  FxSetDIBitsToDevice

Description:    Main 32-bit entry point for GDI SetDIBitsToDevice
                function called from the 16-bit side.
Information:
    In this routine, we know the destination is the device.
    This leaves us with the following conversions, if we blow
    off handling 1,4, and 8 bit destinations.
        1 -> 16/24
        4 -> 16/24
        8 -> 16/24
        24-> 16

    ### Heard a rumor that 16bpp is now a supported DIB format.
    ### Check on this.

Return:         BOOL     1 if success or,
                        -1 if failure.
----------------------------------------------------------------------*/
BOOL
FxSetDIBitsToDevice(SetDIBitsToDeviceParams *lpParams)

//  DIBENGINE FAR  *lpDestDev, 
//  WORD            X, 
//  WORD            Y, 
//  WORD            iScan, 
//  WORD            cScans, 
//  RECT FAR       *lpClipRect, 
//  DRAWMODE FAR   *lpDrawMode, 
//  LPVOID          lpDIBits, 
//  BITMAPINFO FAR *lpBitmapInfo, 
//  LPINT           lpTranslate) 

{
    LPDIBENGINE   lpDst;
    LPBITMAPINFO  lpSrc;
    DWORD      *  lpSrcData;
    LONG retval;

    DEBUG_FIX;

    retval = lpParams->cScans;

    FarToFlat(lpParams->lpDestDev, lpDst);

    // quick exit if in exclusive mode
    if (_FF(gdiFlags) & SKIP_FLAGS)
         return retval;

    // PUNT if 1, 4, or 8bpp destination
    if (lpDst->deBitsPixel < 15) 
      FXPUNT(FXPUNT_NORMAL);
    
    // PUNT if Starting scan is not first scan of DIB
    if (lpParams->iScan) 
      FXPUNT(FXPUNT_NORMAL);
    
    FarToFlat(lpParams->lpBitmapInfo, lpSrc);

// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB //rbissell@stb.com

	if ((long)(lpSrc->bmiHeader.biHeight == 0))
      FXPUNT(FXPUNT_NORMAL);

#else //STBPERF_TOPDOWNDIB

	// We don't handle top-down or zero-height DIBs
    if ((long)(lpSrc->bmiHeader.biHeight) <= 0) 
      FXPUNT(FXPUNT_NORMAL);

#endif //STBPERF_TOPDOWNDIB
// STB End Change

    

    // *********  REEXAMINE THIS PUNT FOR POSSIBLE SUPPORT ************
    // Punt negative destination coordinates
    //if (lpParams->X < (WORD)0 || lpParams->Y < (WORD)0) {
    //    bPunt = TRUE;
    //    goto SDIBTDUseDibEngine;
    //}
    
    // If we have to deal with wierd compression, PUNT
    if (lpSrc->bmiHeader.biCompression != BI_RGB) 
      FXPUNT(FXPUNT_NORMAL);

    switch (lpSrc->bmiHeader.biBitCount)
    {
      case 1:                             // 1  -> 8,16,24
      FXPUNT(FXPUNT_NORMAL);
	  break;
              
      case 4:                             // 4  -> 16,24
	  FarToFlat(lpParams->lpDIBits, lpSrcData);
	  if (lpDst->deBitsPixel == 16)
	      Host2Scrn4to16ConvBlt(lpDst,
				    (RECT FAR *)lpParams->lpClipRect,
				    lpParams->cScans,
				    (int)(short)lpParams->X,
				    (int)(short)lpParams->Y,
				    lpSrcData,
				    lpSrc );
	  else
	      if(!Host2Scrn4to2432ConvBlt( lpDst,
				       (RECT FAR *)lpParams->lpClipRect,
				       lpParams->cScans,
				       (int)(short)lpParams->X,
				       (int)(short)lpParams->Y,
				       lpSrcData,
				       lpSrc ))
		  {
              FXPUNT(FXPUNT_NORMAL);
	          break;
		  }
	  break;
	     
      case 8:                             // 8  -> 16,24
	  FarToFlat(lpParams->lpDIBits, lpSrcData);
	  Host2Scrn8bppConvBlt(lpDst,
			       (RECT FAR *)lpParams->lpClipRect,
			       lpParams->cScans,
			       (int)(short)lpParams->X,
			       (int)(short)lpParams->Y,
			       lpSrcData,
			       lpSrc );
	  break;
            
      case 16:                            // 24 -> 16
      FXPUNT(FXPUNT_NORMAL);
	  break;
              
      case 24:                            // 24 -> 16
      FXPUNT(FXPUNT_NORMAL);
	  break;
    
      default:
	  retval = -1;
      FXPUNT(FXPUNT_NORMAL);
	  break;
    }
    
    FXLEAVE_NOHWACCESS("FxSetDIBitsToDevice", retval);
}


/*----------------------------------------------------------------------
Function name:  FxStretchDIBits

Description:    Main 32-bit entry point for GDI StretchDIBits
                function called from the 16-bit side.
Information:

Return:         BOOL     1 if success or,
                        -1 if failure.
----------------------------------------------------------------------*/
BOOL
FxStretchDIBits(StretchDIBitsParams *lpParams)

//  DIBENGINE FAR  *lpDestDev, 
//  WORD            fGet,
//  WORD            dstX,
//  WORD            dstY,
//  WORD            dstWidth,
//  WORD            dstHeight,
//  WORD            srcX,
//  WORD            srcY,
//  WORD            srcWidth,
//  WORD            srcHeight,
//  LPVOID          lpBits,
//  BITMAPINFO FAR *lpBitmapInfo, 
//  LPINT           lpTranslate,
//  DWORD           Rop,
//  DIB_Brush8 FAR *lpPBrush,
//  DRAWMODE FAR   *lpDrawMode, 
//  RECT FAR       *lpClipRect

{
    LPDIBENGINE   lpDst;
    LPBITMAPINFO  lpSrc;
    DWORD      *  lpSrcData;
    BYTE          pixCode;
    LONG retval;

    DEBUG_FIX;

    retval = 1;
    FarToFlat(lpParams->lpDestDev, lpDst);

    if (_FF(gdiFlags) & SKIP_FLAGS)
         return retval;

// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
// ROP3 useful information lies in the 3rd byte of the DWORD
// If the top four bits of this byte are equal to the bottom 4 bits
// then no patterns are used in the ROP.  We are trying to accelerate
// these.  Office uses SRCCOPY, SRCAND and SRCPAINT most often so 
// we should see some improvement with this.
    if ( ((lpParams->Rop&0xF0000)>> 16) != ((lpParams->Rop&0xF00000)>> 20)
    {
        FXPUNT(FXPUNT_NORMAL);
    }
#else
// STB End Changes
    // For now, only handle srccopy ROP
    if (lpParams->Rop != SRCCOPY)
    {
        FXPUNT(FXPUNT_NORMAL);
    }

// STB Begin Changes
#endif // #ifdef STBPERF_ALLOWDESTROPS
// STB End Changes								
    
    FarToFlat(lpParams->lpBitmapInfo, lpSrc);

	// We don't handle top-down or zero-height DIBs
    if ((long)lpSrc->bmiHeader.biHeight <= 0)
    {
        FXPUNT(FXPUNT_NORMAL);
    }

    // If DIB is not zero based, punt to DibEngine
    if ((lpParams->srcX != 0) ||
        (lpParams->srcY != 0))
    {
        FXPUNT(FXPUNT_NORMAL);
    }        

    // If this is a shrink instead of a stretch, PUNT
    if ( (lpParams->srcWidth >lpParams->dstWidth) || 
         (lpParams->srcHeight>lpParams->dstHeight) )
        FXPUNT(FXPUNT_NORMAL);


    pixCode = MAKE_PIX_CODE(lpSrc->bmiHeader.biBitCount,lpDst->deBitsPixel);

    // Note that you see no DIB_04TOzz codes because they have the same
    // value as the DIB_01TOzz codes since we don't handle either of them
    // I did this to keep the size of the pixCode to a byte

// STB Begin Changes
    // STB-SR 1/18/99 Changed meaning of DIB04TOzz. Now it is different than
	// DIB01TOzz, but DIB32TOzz and DIB24TOzz are the same.
// STB End Changes

    switch( pixCode )
    {
        // we don't handle 8bpp destinations in StretchDIB yet!
        case    DIB_01TO08:
        case    DIB_08TO08:
        case    DIB_16TO08:
        case    DIB_24TO08:

// STB Begin Changes
// STB-SR 1/18/99 Adding in the other possible combination
#ifdef STBPERF_4TO32BIT1TO1STR
        case    DIB_04TO08:
#endif
// STB End Changes

        // we don't handle 32bpp sources in StretchDIB yet!
// STB Begin Changes
// STB-SR 1/18/99 Removing redundant possible combination
#ifndef STBPERF_4TO32BIT1TO1STR
        case    DIB_32TO08:
#endif
// STB End Changes
        case    DIB_32TO16:
        case    DIB_32TO24:
// STB Begin Changes
// STB-SR 1/18/99 Removing redundant possible combination
#ifndef STBPERF_4TO32BIT1TO1STR
        case    DIB_32TO32:
#endif
// STB End Changes

            FXPUNT(FXPUNT_NORMAL); break;
        
        // we don't handle src DIBs of 1 or 4 bpp in StretchDIB
        case    DIB_01TO16:
        case    DIB_01TO24:
// STB Begin Changes
// STB-SR 1/18/99 Removing redundant possible combination
#ifndef STBPERF_4TO32BIT1TO1STR
        case    DIB_01TO32:
#endif
// STB End Changes
            FXPUNT(FXPUNT_NORMAL); break;

// STB Begin Changes
// STB-SR 1/18/99 Now we do handle 4 to 32 bpp if 1 to 1 Stretch
#ifdef STBPERF_4TO32BIT1TO1STR
        case    DIB_04TO16:
            if ( (lpParams->srcWidth ==lpParams->dstWidth) &&
                 (lpParams->srcHeight==lpParams->dstHeight) &&
                 (lpSrc->bmiHeader.biCompression == BI_RGB) &&
                 (lpParams->fGet != 1) )
            {
                FarToFlat(lpParams->lpBits, lpSrcData);
// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
                gdwRop3 = (lpRop-> & 0xFF0000)<<8;
#endif
                Host2Scrn4to16ConvBlt( lpDst,
                                      (RECT FAR *)lpParams->lpClipRect,
                                      (WORD)lpParams->srcHeight,
                                      (int)(short)lpParams->dstX,
                                      (int)(short)lpParams->dstY,
                                      lpSrcData,
                                      lpSrc );
// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
                gdwRop3 = SSTG_ROP_SRCCOPY;
#endif
			}
			else
			{
                FXPUNT(FXPUNT_NORMAL); 
		    }
            break;
        case    DIB_04TO32:
            if ( (lpParams->srcWidth ==lpParams->dstWidth) &&
                 (lpParams->srcHeight==lpParams->dstHeight) &&
                 (lpSrc->bmiHeader.biCompression == BI_RGB) &&
                 (lpParams->fGet != 1) )
            {
                FarToFlat(lpParams->lpBits, lpSrcData);
// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
                gdwRop3 = (lpRop-> & 0xFF0000)<<8;
#endif
                // PRS 5222 - PingZ's fix for 5207 didn't account for all of the
                // places where Host2Scrn4to2432ConvBlt was used, so I added
                // the checking of the return value here as well
                if (!Host2Scrn4to2432ConvBlt( lpDst,
                                      (RECT FAR *)lpParams->lpClipRect,
                                      (WORD)lpParams->srcHeight,
                                      (int)(short)lpParams->dstX,
                                      (int)(short)lpParams->dstY,
                                      lpSrcData,
                                      lpSrc ))
                    {
// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
                gdwRop3 = SSTG_ROP_SRCCOPY;
#endif
                        FXPUNT(FXPUNT_NORMAL);
                        break;
                    }

// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
                gdwRop3 = SSTG_ROP_SRCCOPY;
#endif
			}
			else
			{
                FXPUNT(FXPUNT_NORMAL); 
			}
            break;
#endif // #ifdef STBPERF_4TO32BIT1TO1STR
// STB End Changes

        case    DIB_08TO16:
        case    DIB_08TO24:
// STB Begin Changes
// STB-SR 1/18/99 Removing redundant possible combination
#ifndef STBPERF_4TO32BIT1TO1STR
        case    DIB_08TO32:
#endif
// STB End Changes
            // in researching the StretchDIB calls that affect
            // WB98 high-end tests, it turns out that the 8bpp StretchDIB
            // calls are one to one stretches! So, we can just blt
            // the data to the screen :)
            if ( (lpParams->srcWidth ==lpParams->dstWidth) &&
                 (lpParams->srcHeight==lpParams->dstHeight) &&
                 (BI_RGB == lpSrc->bmiHeader.biCompression) &&
                 (lpParams->fGet != 1)
               )
            {
                FarToFlat(lpParams->lpBits, lpSrcData);
// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
                gdwRop3 = (lpRop-> & 0xFF0000)<<8;
#endif
                Host2Scrn8bppConvBlt( lpDst,
                                      (RECT FAR *)lpParams->lpClipRect,
                                      (WORD)lpParams->srcHeight,
                                      (int)(short)lpParams->dstX,
                                      (int)(short)lpParams->dstY,
                                      lpSrcData,
                                      lpSrc );
// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
                gdwRop3 = SSTG_ROP_SRCCOPY;
#endif
            }
            else
                FXPUNT(FXPUNT_NORMAL); 
            break;

        case    DIB_16TO16:
// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
            if(lpParams->Rop != SRCCOPY)
		    {
		        FXPUNT(FXPUNT_NORMAL);
		    }
#endif
            // at 16bpp, we handle 565 format
            // Handle 555 cause the DIBENGINE sure does not!!!!!
            if ((BI_BITFIELDS == lpSrc->bmiHeader.biCompression) ||
                (BI_RGB == lpSrc->bmiHeader.biCompression))
            {
                FarToFlat(lpParams->lpBits, lpSrcData);
                Host2Scrn16to16StrBlt(lpDst,
                    (RECT FAR *)lpParams->lpClipRect,
                    (int)(short)lpParams->dstX,
                    (int)(short)lpParams->dstY,
                    (int)(short)lpParams->dstWidth,
                    (int)(short)lpParams->dstHeight,
                    (int)(short)lpParams->srcX,
                    (int)(short)lpParams->srcY,
                    (int)(short)lpParams->srcWidth,
                    (int)(short)lpParams->srcHeight,
                    lpSrcData,
                    lpSrc );
            }
            else
               {
                FXPUNT(FXPUNT_NORMAL);
               }
            break;
        
        // we don't currently handle any of these cases
        case    DIB_16TO24:
// STB Begin Changes
// STB-SR 1/18/99 Removing redundant possible combinations
#ifndef STBPERF_4TO32BIT1TO1STR
        case    DIB_16TO32:
        case    DIB_24TO32:
        case    DIB_24TO16:
        case    DIB_24TO24:
#endif
// STB End Changes
            FXPUNT(FXPUNT_NORMAL); break;
    }                                                  
    
    FXLEAVE_NOHWACCESS("FxStretchDIBits", retval);
}


/*----------------------------------------------------------------------
Function name:  Host2Scrn8bppConvBlt

Description:    Called only for DeviceBitmapBits functions.  Performs
                colortable lookup on 256-entry palette and writes
                data host-to-screen blt.
Information:

Return:         INT     1 if success or,
                        0 if rejected.
----------------------------------------------------------------------*/
 int  Host2Scrn8bppConvBlt(LPDIBENGINE     lpDst,              // device dib
		     RECT FAR *      lpClipRect,
		     WORD            cScans, 
		     int             X,
		     int             Y,
		     DWORD *         lpSrcData,          // buffer
		     LPBITMAPINFO    lpSrc )   
{
    RECT   * lpRect;            // Local pointer to Clipping Rectangle 
    BYTE   * SrcDataPtr;        // Pointer to actual start of clipped DIB data
    BYTE   * lpData;            // Working pointer to source data
    LONG     CTop;              // Clipped extents
    LONG     CBottom;
    LONG     CLeft;
    LONG     CRight;
    DWORD    SrcDataPitch;      // Pitch of source DIB data, in bytes
    DWORD    TotalDwords;       // Total # of DWORDS to be xferred into FIFO 
    DWORD    DataDWord;         // Temp for RGB data xlated from lpColors 
    DWORD    TopClippedAmount;  // # of Scans clipped from 'top' of DIB 
    DWORD    LeftClippedAmount; // # of BYTES clipped from left of DIB
    DWORD    dwtemp;            // Temp data storage used for FIFO macros
    LONG     i;                 
    DWORD    j;
    DWORD  * ColorTable;        // Pointer to DIB color conversion table
    DWORD    PixelsInScanline;  // Since each DIB pixel will end up as a 
		                // DWORD in the FIFO, this is also a count 
		                // of dwords in a scanline
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    // OK, Well, DIBs are usually stored backwards.  Scanline pointed to
    // by lpDIBits is LAST scanline (if biHeight positive).  We'll grab
    // scanlines out in reverse order to feed the engine in the top-down
    // order.  Assume iScan == 0 and cScans = biHeight.
    // 
    // So, addess of First default DIB scanline = 
    //          lpDIBits + biHeight (or cscans) * biWidth (in bytes)
    //
    // This makes our DEFAULT rectangle:
    //      start at lpDIBITS + (cScans - 1) * dwSrcWidthInBytes
    //      width = biWidth * BytesPerPixel rounded to next dword boundary
    //      height = cScans
    //      pitch = dwSrcWidthInBytes
    //
    // In which case the default destination rectangle would be:
    //      [X,Y]-[biWidth,cScans]
    //
    // Of course, this must be clipped against the actual clipping 
    // rectangle if it is non-null, which in winbench98 is almost always
    // true.
    
    //
    //  SET UP DESTINATION/TRANSFER VARIABLES
    //

    // DIBPitch for 8bpp dib is #pixels rounded to next dword boundary
    SrcDataPitch = (lpSrc->bmiHeader.biWidth + 3) & 0xFFFFFFFC;
    
    if (lpClipRect) 
    {
	// If lpRect Non-Null, intersect with default dest rect
        // Get usable pointer to clip rectangle
        FarToFlat(lpClipRect, lpRect);
    
        CTop    = max(lpRect->top,Y);
        CLeft   = max(lpRect->left,X);
        CBottom = min(lpRect->bottom,Y+(int)cScans);
        CRight  = min(lpRect->right,X+(int)lpSrc->bmiHeader.biWidth);
        
        // Added to fix Defect 1703
        // What happen is CRight is less then CLeft we get a negative PixelsInScanLine
        // There is never enough room and life gets worse until we come to a sw halt
        // Might as well check for a bogus y while we are at it
        if ((CRight <= CLeft) || (CBottom <= CTop))
            {
#if 0
            _asm int 03
#endif
            return 0;
            }
         

        TopClippedAmount  = lpRect->top  - (DWORD)Y;
        LeftClippedAmount = lpRect->left - (DWORD)X;
        PixelsInScanline  = CRight - CLeft;

// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB  //rbissell@stb

		if ((long)(lpSrc->bmiHeader.biHeight) <= 0)
		{
			SrcDataPtr = (BYTE*)lpSrcData
					   + (TopClippedAmount * SrcDataPitch)
					   + LeftClippedAmount;
		}
		else
		{
			SrcDataPtr	= (BYTE *)lpSrcData
						+ ((cScans - TopClippedAmount - 1) * SrcDataPitch)
						+ LeftClippedAmount;
		}

#else //STBPERF_TOPDOWNDIB

        SrcDataPtr   = (BYTE *)lpSrcData + 
	    ((cScans - TopClippedAmount - 1) * SrcDataPitch) + 
	    LeftClippedAmount;

#endif //STBPERF_TOPDOWNDIB
// STB End Change        

    }
    else
    {
    
        CTop    = Y;
        CLeft   = X;
        CBottom = Y+(int)cScans;
        CRight  = X+lpSrc->bmiHeader.biWidth;
        
        PixelsInScanline  = CRight - CLeft;

// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB //rbissell@stb

		if ((long)(lpSrc->bmiHeader.biHeight) <= 0)
			SrcDataPtr = (BYTE*)lpSrcData;
		else
			SrcDataPtr  = (BYTE *)lpSrcData 
						+ ((cScans - 1) * SrcDataPitch);

#else //STBPERF_TOPDOWNDIB

        SrcDataPtr   = (BYTE *)lpSrcData + 
	    (cScans - 1) * SrcDataPitch;

#endif //STBPERF_TOPDOWNDIB
// STB End Change

    }

// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB //rbissell@stb

	if ((long)(lpSrc->bmiHeader.biHeight) > 0)
		SrcDataPitch = -(int)SrcDataPitch;   // We _may_ need to walk the dib backwards

#else //STBPERF_TOPDOWNDIB

	SrcDataPitch = -(int)SrcDataPitch;   // We need to walk the dib backwards

#endif //STBPERF_TOPDOWNDIB
// STB End Change
 

    // XXX FIXME
    // check that the lpsrc is/isn't always in host memory, or at least
    // isn't ever the screen, so we don't have to include it in the s/w
    // cursor exclusion test
    //
    FXENTER("Host2Scrn8bppConvBlt", lpDst->deFlags, TRUE,
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
	    FXENTER_RECT, CLeft, CTop, CRight, CBottom,
	    FXENTER_NO_RECT, 0, 0, 0, 0,   );
    //  
    //  SET UP HARDWARE FOR COLOR HOST TO SCREEN BITBLT  
    //
    CMDFIFO_CHECKROOM(cf, 6);
    SETPH(cf,  (SSTCP_PKT2   |
		srcFormatBit |
		srcXYBit     |
		dstSizeBit   |
		dstXYBit     |
		commandBit)); 
    // Source Format is 32bpp
    dwtemp = (PixelsInScanline << 2) | SSTG_PIXFMT_32BPP;
    SET(cf, _FF(lpGRegs)->srcFormat, dwtemp);
    SET(cf, _FF(lpGRegs)->srcXY, 0);
    SET(cf, _FF(lpGRegs)->dstSize, R32((CBottom - CTop), (CRight - CLeft)));
    SET(cf, _FF(lpGRegs)->dstXY, R32(CTop, CLeft));

// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
    SETC(cf, _FF(lpGRegs)->command, gdwRop3 | SSTG_HOST_BLT);
#else
    SETC(cf, _FF(lpGRegs)->command, SSTG_ROP_SRCCOPY | SSTG_HOST_BLT);
#endif
// STB End Changes

    BUMP(6);

    //
    //   TRANSLATE AND TRANSFER DATA TO FIFO
    //

    lpData = SrcDataPtr;
    TotalDwords = PixelsInScanline * (CBottom - CTop);
    
//    ColorTable = (DWORD *)lpSrc->bmiColors;
//PingZ 3/12/99 lpSrc->bmiColors may not be pointing to the actual color table!!!  The only
//to guarantee that is to add bmiHeader.biSize to lpSrc.  This is to fix PRS #4604
    ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;
        
    if (TotalDwords < MAXCOLORDWORDS)
    {
	// Write whole transfer at once
	CMDFIFO_CHECKROOM(cf, TotalDwords+1);
	SETPH(cf, (SSTCP_PKT1 |
		   SSTCP_PKT1_2D |
		   (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT)  |
		   (TotalDwords  << SSTCP_PKT1_NWORDS_SHIFT)));

	for (i = 0; i < (CBottom - CTop); i++)
	{
	    for (j = 0; j < PixelsInScanline; j++)
	    {
                DataDWord = ColorTable[lpData[j]];
		SET(cf,_FF(lpGRegs)->launch[0], DataDWord);
	    }
	    (DWORD)lpData += SrcDataPitch;
	}

	BUMP(TotalDwords + 1);
    }
    else
    {
	// Write transfer one scanline at a time.
	//
	for (i = 0; i < (CBottom - CTop); i++)
	{
	    CMDFIFO_CHECKROOM(cf, PixelsInScanline + 1);
	    SETPH(cf, (SSTCP_PKT1 |
		       SSTCP_PKT1_2D |
		       (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
		       (PixelsInScanline << SSTCP_PKT1_NWORDS_SHIFT)));

	    for (j = 0; j < PixelsInScanline; j++)
	    {
	    			
		DataDWord = ColorTable[lpData[j]];
		SET(cf,_FF(lpGRegs)->launch[0], DataDWord);
                    
	    }	

	    (DWORD)lpData += SrcDataPitch;
                
	    BUMP(PixelsInScanline + 1);
        }
    }   

    FXLEAVE("Host2Scrn8bppConvBlt", cf, lpDst);

    return 1;
}


/*----------------------------------------------------------------------
Function name:  Host2Scrn8bppTo16bppConvBlt

Description:    Called only for DeviceBitmapBits functions.  Performs
                colortable lookup on 256-entry palette and writes
                16bpp data to BANSHEE as a host-to-screen blit after
                converting the colors from 32-bit RGB values (from
                the 8bpp color lookup table) into 16-bit 565 colors.
Information:

Return:         INT     1 if success or,
                        0 if rejected.
----------------------------------------------------------------------*/
int  Host2Scrn8bppTo16bppConvBlt(LPDIBENGINE     lpDst,           // device dib
                                 RECT FAR *      lpClipRect,
                                 WORD            cScans, 
                                 int             X,
                                 int             Y,
                                 DWORD *         lpSrcData,       // buffer
                                 LPBITMAPINFO    lpSrc )   
{
    RECT   * lpRect;            // Local pointer to Clipping Rectangle 
    BYTE   * SrcDataPtr;        // Pointer to actual start of clipped DIB data
    BYTE   * lpData;            // Working pointer to source data
    LONG     CTop;              // Clipped extents
    LONG     CBottom;
    LONG     CLeft;
    LONG     CRight;
    DWORD    SrcDataPitch;      // Pitch of source DIB data, in bytes
    DWORD    TotalDwords;       // Total # of DWORDS to be xferred into FIFO 
    DWORD    DataDword;         // Temp for RGB data xlated from lpColors 
    DWORD    TopClippedAmount;  // # of Scans clipped from 'top' of DIB 
    DWORD    LeftClippedAmount; // # of BYTES clipped from left of DIB
    DWORD    dwtemp;            // Temp data storage used for FIFO macros
    LONG     i;                 
    DWORD    j;
    DWORD  * ColorTable;        // Pointer to DIB color conversion table
    DWORD    PixelsInScanline;  // We'll be pushing two pixels per dword
    DWORD    DwordsInScanline;  // ACTUAL number of dwords of data in Scanline
    DWORD    RGBDword;          // Raw RGB data from DIB color table
    DWORD    LastRGB;           // Previously Looked up RGB value
    DWORD    Last565;           // Result of previous lookup
    DWORD    blubyte;           // Blue data from RGB
    DWORD    grnbyte;           // Green data from RGB
    DWORD    redbyte;           // Red data from RGB
    
    
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    
    //
    //  SET UP DESTINATION/TRANSFER VARIABLES
    //

    // DIBPitch for 8bpp dib is #pixels rounded to next dword boundary
    SrcDataPitch = (lpSrc->bmiHeader.biWidth + 3) & 0xFFFFFFFC;
    
    if (lpClipRect) 
    {
	// If lpRect Non-Null, intersect with default dest rect
        // Get usable pointer to clip rectangle
        FarToFlat(lpClipRect, lpRect);
    
        CTop    = max(lpRect->top,Y);
        CLeft   = max(lpRect->left,X);
        CBottom = min(lpRect->bottom,Y+(int)cScans);
        CRight  = min(lpRect->right,X+(int)lpSrc->bmiHeader.biWidth);
        
        // Validate extents
        if ((CRight <= CLeft) || (CBottom <= CTop))
        {
            return 0;
        }
         

        TopClippedAmount  = lpRect->top  - (DWORD)Y;
        LeftClippedAmount = lpRect->left - (DWORD)X;
        PixelsInScanline  = CRight - CLeft;

// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB  //rbissell@stb

		if ((long)(lpSrc->bmiHeader.biHeight) <= 0)
		{
			SrcDataPtr = (BYTE*)lpSrcData
					   + (TopClippedAmount * SrcDataPitch)
					   + LeftClippedAmount;
		}
		else
		{
			SrcDataPtr	= (BYTE *)lpSrcData
						+ ((cScans - TopClippedAmount - 1) * SrcDataPitch)
						+ LeftClippedAmount;
		}

#else //STBPERF_TOPDOWNDIB

        SrcDataPtr   = (BYTE *)lpSrcData + 
	    ((cScans - TopClippedAmount - 1) * SrcDataPitch) + 
	    LeftClippedAmount;

#endif //STBPERF_TOPDOWNDIB
// STB End Change

    }
    else
    {
    
        CTop    = Y;
        CLeft   = X;
        CBottom = Y+(int)cScans;
        CRight  = X+lpSrc->bmiHeader.biWidth;
        
        PixelsInScanline  = CRight - CLeft;

// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB //rbissell@stb

		if ((long)(lpSrc->bmiHeader.biHeight) <= 0)
			SrcDataPtr = (BYTE*)lpSrcData;
		else
			SrcDataPtr  = (BYTE *)lpSrcData 
						+ ((cScans - 1) * SrcDataPitch);

#else //STBPERF_TOPDOWNDIB

        SrcDataPtr   = (BYTE *)lpSrcData + 
	    (cScans - 1) * SrcDataPitch;

#endif //STBPERF_TOPDOWNDIB
// STB End Change
    }    

    dwtemp = (SrcDataPitch << 1) | SSTG_PIXFMT_16BPP;

// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB //rbissell@stb

	if ((long)(lpSrc->bmiHeader.biHeight) > 0)
		SrcDataPitch = -(int)SrcDataPitch;   // We _may_ need to walk the dib backwards

#else //STBPERF_TOPDOWNDIB

    SrcDataPitch = -(int)SrcDataPitch;   // We need to walk the dib backwards

#endif //STBPERF_TOPDOWNDIB
// STB End Change


    
    // XXX FIXME
    // check that the lpsrc is/isn't always in host memory, or at least
    // isn't ever the screen, so we don't have to include it in the s/w
    // cursor exclusion test
    //
    FXENTER("Host2Scrn8bppTo16bppConvBlt", lpDst->deFlags, TRUE,
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
	    FXENTER_RECT, CLeft, CTop, CRight, CBottom,
	    FXENTER_NO_RECT, 0, 0, 0, 0,   );
        
    //  
    //  SET UP HARDWARE FOR COLOR HOST TO SCREEN BITBLT  
    //
    CMDFIFO_CHECKROOM(cf, 6);
    SETPH(cf,  (SSTCP_PKT2   |
		srcFormatBit |
		srcXYBit     |
		dstSizeBit   |
		dstXYBit     |
		commandBit)); 
    // Source Format is 32bpp
    SET(cf, _FF(lpGRegs)->srcFormat, dwtemp);
    SET(cf, _FF(lpGRegs)->srcXY, 0);
    SET(cf, _FF(lpGRegs)->dstSize, R32((CBottom - CTop), (CRight - CLeft)));
    SET(cf, _FF(lpGRegs)->dstXY, R32(CTop, CLeft));

// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
    SETC(cf, _FF(lpGRegs)->command, gdwRop3 | SSTG_HOST_BLT);
#else
    SETC(cf, _FF(lpGRegs)->command, SSTG_ROP_SRCCOPY | SSTG_HOST_BLT);
#endif
// STB End Changes

    BUMP(6);

    //
    //   TRANSLATE AND TRANSFER DATA TO FIFO
    //


    // We'll save the last looked-up value to keep from looking up 10 jillion
    // of the same colors.  Most DIBs use fields of solid color, so this 
    // should save lots of time.
    LastRGB = 0xFFFFFFFF;

    lpData = SrcDataPtr;
    DwordsInScanline = ((PixelsInScanline << 1) + 2) >> 2;
    TotalDwords      = DwordsInScanline * (CBottom - CTop);
    
//    ColorTable = (DWORD *)lpSrc->bmiColors;
//PingZ 3/12/99 lpSrc->bmiColors may not be pointing to the actual color table!!!  The only
//to guarantee that is to add bmiHeader.biSize to lpSrc.  This is to fix PRS #4604
    ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;

    if (TotalDwords < MAXCOLORDWORDS)
    {
        // Write whole transfer at once
        CMDFIFO_CHECKROOM(cf, TotalDwords+1);
        SETPH(cf, (SSTCP_PKT1                                |
                   SSTCP_PKT1_2D                             |
                   (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT)     |
                   (TotalDwords  << SSTCP_PKT1_NWORDS_SHIFT)) );

        for (i = 0; i < (CBottom - CTop); i++)
        {
            for (j = 0; j < (PixelsInScanline & 0xFFFFFFFE); j+=2)
            {
                RGBDword = ColorTable[lpData[j]];
                if (RGBDword == LastRGB) 
                {
                    DataDword = Last565;
                }    
                else
                {
                    LastRGB = RGBDword;
	                blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
	                grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
	                redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
	                DataDword= (blubyte | (grnbyte << 5) | (redbyte << 11));
                    Last565  = DataDword;
                }
                
                
                RGBDword = ColorTable[lpData[j+1]];
                if (RGBDword == LastRGB) 
                {
	                DataDword |= (Last565 << 16);
	            }
	            else
                {
                    LastRGB = RGBDword;
	                blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
	                grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
	                redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
	                Last565 = (blubyte | (grnbyte << 5) | (redbyte << 11));
                    DataDword |= (Last565 << 16);
                }
                
                SET(cf,_FF(lpGRegs)->launch[0], DataDword);
                  
            }
            
            if (PixelsInScanline & 1)
            {
                RGBDword = ColorTable[lpData[PixelsInScanline-1]];

                if (RGBDword == LastRGB) 
                {
	                DataDword = Last565;
	            }
	            else
                {
	                blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
	                grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
	                redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
	                DataDword= (blubyte | (grnbyte << 5) | (redbyte << 11));
                }
                SET(cf,_FF(lpGRegs)->launch[0], DataDword);

            }

            (DWORD)lpData += SrcDataPitch;
        }

        BUMP(TotalDwords + 1);
    }
    else
    {
        // Write transfer one scanline at a time.
        //
        for (i = 0; i < (CBottom - CTop); i++)
        {
            CMDFIFO_CHECKROOM(cf, DwordsInScanline + 1);
            SETPH(cf, (SSTCP_PKT1                            |
                       SSTCP_PKT1_2D                         |
                       (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
                       (DwordsInScanline << SSTCP_PKT1_NWORDS_SHIFT)));

            for (j = 0; j < (PixelsInScanline & 0xFFFFFFFE); j+=2)
            {
                RGBDword = ColorTable[lpData[j]];
                if (RGBDword == LastRGB) 
                {
                    DataDword = Last565;
                }    
                else
                {
                    LastRGB = RGBDword;
	                blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
	                grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
	                redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
	                DataDword= (blubyte | (grnbyte << 5) | (redbyte << 11));
                    Last565  = DataDword;
                }
                
                
                RGBDword = ColorTable[lpData[j+1]];
                if (RGBDword == LastRGB) 
                {
	                DataDword |= (Last565 << 16);
	            }
	            else
                {
                    LastRGB = RGBDword;
	                blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
	                grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
	                redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
	                Last565 = (blubyte | (grnbyte << 5) | (redbyte << 11));
                    DataDword |= (Last565 << 16);
                }
                
                SET(cf,_FF(lpGRegs)->launch[0], DataDword);
                  
            }
            
            if (PixelsInScanline & 1)
            {
                RGBDword = ColorTable[lpData[PixelsInScanline-1]];

                if (RGBDword == LastRGB) 
                {
	                DataDword = Last565;
	            }
	            else
                {
	                blubyte = (DWORD)ByteTo5BitLookup[ RGBDword&0x0000FF      ];
	                grnbyte = (DWORD)ByteTo6BitLookup[(RGBDword&0x00FF00) >> 8];
	                redbyte = (DWORD)ByteTo5BitLookup[(RGBDword&0xFF0000) >>16];
	                DataDword= (blubyte | (grnbyte << 5) | (redbyte << 11));
                }
                
                SET(cf,_FF(lpGRegs)->launch[0], DataDword);

            }

            (DWORD)lpData += SrcDataPitch;
               
            BUMP(PixelsInScanline + 1);
       }
   }   

   FXLEAVE("Host2Scrn8bppTo16bppConvBlt", cf, lpDst);

   return 1;
}


/*----------------------------------------------------------------------
Function name:  Host2Scrn4to16ConvBlt

Description:    Called only for DeviceBitmapBits functions.  This
                function creates a 16-entry LUT containing 16-bit
                hicolor values.  It then reads in a byte of DIB
                data at a time and performs two lookups on the
                4-bit indices, producing 32-bits of data to go to
                the FIFO.  The 16bpp data is then bltted to the
                screen.
Information:

Return:         INT     1 if success or,
                        0 if rejected.
----------------------------------------------------------------------*/
int
Host2Scrn4to16ConvBlt(LPDIBENGINE     lpDst,              // device dib
		      RECT FAR *      lpClipRect,
		      WORD            cScans, 
		      int             X,
		      int             Y,
		      DWORD *         lpSrcData,          // buffer
		      LPBITMAPINFO    lpSrc )   
{
    BYTE   * lpData;                  // Working pointer to source data
    BYTE   * SrcDataPtr;              // Ptr to start of clipped DIB data 
    RECT   * lpRect;                  // Local pointer for clip rect
    LONG     CTop;                    // Clipped DIB extents
    LONG     CBottom;
    LONG     CLeft;
    LONG     CRight;
    DWORD    SrcDataPitch;            // Pitch of source DIB data, in BYTES
    DWORD    TotalDwords;             // Total # of DWORDS to be sent to FIFO
    DWORD    DataDWord;               // Temp Storage for destination datadword
    DWORD    TopClippedAmount;        // # of scanlines clipped from top of DIB
    DWORD    LeftClippedAmountBytes;  // # of BYTES clipped from left of DIB
    DWORD    LeftClippedAmountPixels; // # of BYTES clipped from left of DIB
    DWORD    PixelsInScanline;        // # if pixels in dest scanline
    DWORD    dwtemp;                  // Temp var for FIFO macros
    DWORD    BytesInScanline;         // # of BYTES with DIB data/scanline 
    LONG     i;                     
    DWORD    j;
    DWORD  * ColorTable;              // Local ptr to DIB color xlation tbl
    DWORD    tblindex;                // Index variable for building CLUT
    DWORD    blubyte,grnbyte,redbyte; // used for creating color mapping table
    TAKE_ADDRESS DWORD hclut[256];    // Color Lookup Table -- converts two
    // 4-bit indices into 2 16-bit hicolor
    // values 
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    // See Host2Scrn8bppConvBlt for background on clipping, data retrieval

    //
    //  SET UP DESTINATION/TRANSFER VARIABLES
    //
    
    // DIBPitch for 4bpp dib is #pixels rounded to next byte boundary 
    // divided by two to yield bytes, rounded to next dword boundary.
    SrcDataPitch = (((lpSrc->bmiHeader.biWidth + 1) >> 1) + 3) & 0xFFFFFFFC;
    
    BytesInScanline=0;    

    if (lpClipRect) {
	// If lpRect Non-Null, intersect with default dest rect
    
        // Get usable pointer to clip rectangle
        FarToFlat(lpClipRect, lpRect);
    
        CTop    = max(lpRect->top,Y);
        CLeft   = max(lpRect->left,X);
        CBottom = min(lpRect->bottom,Y+(int)cScans);
        CRight  = min(lpRect->right,X+(int)lpSrc->bmiHeader.biWidth);
        
        // Added to fix Defect 1703
        // What happen is CRight is less then CLeft we get a negative PixelsInScanLine
        // There is never enough room and life gets worse until we come to a sw halt
        // Might as well check for a bogus y while we are at it
        if ((CRight <= CLeft) || (CBottom <= CTop))
            {
#if 0
            _asm int 03
#endif
            return 0;
            }

        TopClippedAmount       = lpRect->top  - Y;
        LeftClippedAmountPixels= lpRect->left - X;
        LeftClippedAmountBytes = (lpRect->left - X) >> 1;
        PixelsInScanline       = CRight - CLeft;

// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB  //rbissell@stb

		if ((long)(lpSrc->bmiHeader.biHeight) <= 0)
		{
			SrcDataPtr = (BYTE*)lpSrcData
					   + (TopClippedAmount * SrcDataPitch)
					   + LeftClippedAmountBytes;
		}
		else
		{
			SrcDataPtr	= (BYTE *)lpSrcData
						+ (((DWORD)cScans - TopClippedAmount - 1) * SrcDataPitch)
						+ LeftClippedAmountBytes;
		}

#else //STBPERF_TOPDOWNDIB

        SrcDataPtr = (BYTE *)lpSrcData + 
	    (((DWORD)cScans - TopClippedAmount - 1) * SrcDataPitch) + 
	    LeftClippedAmountBytes;

#endif //STBPERF_TOPDOWNDIB
// STB End Change
        
    } else {                      
    
        CTop    = Y;
        CLeft   = X;
        CBottom = Y+(int)cScans;
        CRight  = X+lpSrc->bmiHeader.biWidth;
        
        TopClippedAmount       = 0;    //zero out locals that are used
        LeftClippedAmountPixels= 0;
        LeftClippedAmountBytes = 0;
        PixelsInScanline = CRight - CLeft;

// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB //rbissell@stb

		if ((long)(lpSrc->bmiHeader.biHeight) <= 0)
			SrcDataPtr = (BYTE*)lpSrcData;
		else
			SrcDataPtr = (BYTE *)lpSrcData + ((cScans - 1) * SrcDataPitch);

#else //STBPERF_TOPDOWNDIB

		SrcDataPtr   = (BYTE *)lpSrcData + (cScans - 1) * SrcDataPitch;

#endif //STBPERF_TOPDOWNDIB
// STB End Change
        
    }    

    // Moved by APS since BytesInScanline not set if lpClipRect
    // Bytes in scanline is the number of bytes that contain valid pixel
    //   data.  This could be #pixels or #pixels+1 depending on alignment.
    //
    //          WIDTH  OFST  bwidth
    //   IE:    even   even   pels/2
    //          even   odd    pels/2+1
    //          odd    even   pels/2+1
    //          odd    odd    pels/2+1
    //
    if ((LeftClippedAmountPixels & 0x01) || (PixelsInScanline & 0x01)) {
        BytesInScanline = (PixelsInScanline >> 1) + 1;
    } else {
        BytesInScanline = PixelsInScanline >> 1;
    } 


// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB //rbissell@stb

	if ((long)(lpSrc->bmiHeader.biHeight) > 0)
		SrcDataPitch = -(int)SrcDataPitch;   // We _may_ need to walk the DIB backwards

#else //STBPERF_TOPDOWNDIB

	// Source pitch in BYTES 
    SrcDataPitch = -(int)SrcDataPitch;  // We need to walk the DIB backwards

#endif //STBPERF_TOPDOWNDIB
// STB End Change

    
    //
    //  BUILD THE HICOLOR LOOKUP TABLE
    //
//    ColorTable = (DWORD *)lpSrc->bmiColors;
//PingZ 3/12/99 lpSrc->bmiColors may not be pointing to the actual color table!!!  The only
//to guarantee that is to add bmiHeader.biSize to lpSrc.  This is to fix PRS #4604
    ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;
    
    // Build 16-entry 16bpp (hicolor) lookup table for 4bpp->16bpp conversions
    for (i = 0; i < 16; i++) 
    {
	blubyte = (DWORD)ByteTo5BitLookup[ColorTable[i] & 0x000000FF];
	grnbyte = (DWORD)ByteTo6BitLookup[(ColorTable[i] & 0xFF00) >> 8 ];
	redbyte = (DWORD)ByteTo5BitLookup[(ColorTable[i] & 0xFF0000) >> 16];
	hclut[i]= (blubyte | (grnbyte << 5) | (redbyte << 11));
    }                     
    
    // Build 256-entry byte-lookup table for handling two nybble 
    //   (pixel) lookups at a time
    tblindex = 0;
    
    for (i=0; i<16; i++) {
        for (j=0; j<16; j++) {
        
            hclut[tblindex++] = (DWORD)(hclut[j] & 0x0000FFFF) |
		((hclut[i] & 0x0000FFFF) << 16);
        }                                           
    }

    FXENTER("Host2Scrn4to16ConvBlt", lpDst->deFlags, TRUE,
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
	    FXENTER_RECT, CLeft, CTop, CRight, CBottom,
	    FXENTER_NO_RECT, 0, 0, 0, 0,   );

    //  
    //  SET UP HARDWARE FOR COLOR HOST TO SCREEN BITBLT  
    //
    CMDFIFO_CHECKROOM(cf, 6);
    SETPH(cf,  SSTCP_PKT2   |
	  srcFormatBit |
	  srcXYBit     |
	  dstSizeBit   |
	  dstXYBit     |
	  commandBit); 

    // Source Format is 16bpp, set src pitch, swizzle words
    dwtemp = (BytesInScanline << 2) | 
	SSTG_HOST_WORD_SWIZZLE | 
	SSTG_PIXFMT_16BPP;
    SET(cf, _FF(lpGRegs)->srcFormat, dwtemp);
    
    // Set Start offset (0 or 1 pixel, 0 or 2 bytes)
    if (LeftClippedAmountPixels & 0x01) {
        dwtemp = 2;                     // Pixel at start we must skip
    } else {
        dwtemp = 0;                     //
    }
    SET(cf, _FF(lpGRegs)->srcXY, dwtemp);
    
    // Each byte will translate into one dword of FIFO data
    dwtemp = ((((DWORD)(CBottom - CTop )) << 16) |
               ((DWORD)(PixelsInScanline)));
    SET(cf, _FF(lpGRegs)->dstSize,dwtemp);
    
    // Destination X,Y blt start location
    dwtemp  = (CTop << 16) | CLeft;                  
    SET(cf, _FF(lpGRegs)->dstXY , dwtemp);
    
    // ROP here is always SRCCOPY 
// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
    SETC(cf, _FF(lpGRegs)->command, gdwRop3 | SSTG_HOST_BLT);
#else
    SETC(cf, _FF(lpGRegs)->command, SSTG_ROP_SRCCOPY | SSTG_HOST_BLT);
#endif
// STB End Changes

    BUMP(8);

    //
    //   TRANSLATE AND TRANSFER DATA TO FIFO
    //
    
    // POSSIBLE OPTIMIZATIONS:  Several areas can be improved by removing
    // the use of local temporary variables.  Also, dwtemp is reused many
    // times which may create unnecessary dependencies.  In addition, it
    // appears that 99% of all 4bpp deviceDIB calls are 16x16, and it may
    // be faster to do dual lookups on the nybble pairs than to build
    // the double-lookup table.

    lpData = SrcDataPtr;
    TotalDwords = BytesInScanline * (CBottom - CTop);
    
    // Grab byte of src DIB data (2 pixels), xlate into 16bpp FIFO data 
    // (still 2 pixels, 1 DWORD), and write to FIFO.
    
    if (TotalDwords < MAXCOLORDWORDS)	{   // Write whole transfer at once
    
	CMDFIFO_CHECKROOM(cf, TotalDwords + 1);
	SETPH(cf, SSTCP_PKT1                          |
	      SSTCP_PKT1_2D                       |
	      LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT |
	      (TotalDwords) << SSTCP_PKT1_NWORDS_SHIFT);

	for (i = 0; i < (CBottom - CTop); i++)  {
	    for (j = 0; j < BytesInScanline; j++)  {
            
                DataDWord = hclut[lpData[j]];
		SET(cf,_FF(lpGRegs)->launch[0], DataDWord);
	    }
	    (DWORD)lpData += SrcDataPitch;
	}

	BUMP(TotalDwords + 1);
        
    } else {                                // Transfer one scanline at a time.

	for (i = 0; i < (CBottom - CTop); i++)  {
            
	    CMDFIFO_CHECKROOM(cf, BytesInScanline + 1);
	    SETPH(cf, SSTCP_PKT1                              |
		  SSTCP_PKT1_2D                           |
		  (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT)   |
		  (BytesInScanline << SSTCP_PKT1_NWORDS_SHIFT));

	    for (j = 0; j < BytesInScanline; j++)  {
	    			
		DataDWord = hclut[lpData[j]];
		SET(cf,_FF(lpGRegs)->launch[0], DataDWord);
                    
	    }	

	    (DWORD)lpData += SrcDataPitch;
	    BUMP(BytesInScanline + 1);
	}

    }   

    FXLEAVE("Host2Scrn4to16ConvBlt", cf, lpDst);
   
    return 1;
}


/*----------------------------------------------------------------------
Function name:  Host2Scrn4to2432ConvBlt

Description:    Called only for DeviceBitmapBits functions.  This
                function creates a 16-entry LUT containing 16-bit
                hicolor values.  It then reads in a byte of DIB
                data at a time and  performs two lookups on the
                4-bit indices, producing 32-bits of data to go to
                the FIFO.  The 16bpp data is then bltted to the
                screen.
Information:

Return:         INT     1 if success or,
                        0 if rejected.
----------------------------------------------------------------------*/
int
Host2Scrn4to2432ConvBlt(LPDIBENGINE  lpDst,              // device dib
			RECT FAR *      lpClipRect,
			WORD            cScans, 
			int             X,
			int             Y,
			DWORD *         lpSrcData,          // buffer
			LPBITMAPINFO    lpSrc )   
{
    BYTE   * lpData;                  // Working pointer to source data
    BYTE   * SrcDataPtr;              // Ptr to start of clipped DIB data 
    RECT   * lpRect;                  // Local pointer for clip rect
    LONG     CTop;                    // Clipped DIB extents
    LONG     CBottom;
    LONG     CLeft;
    LONG     CRight;
    DWORD    SrcDataPitch;            // Pitch of source DIB data, in BYTES
    DWORD    TotalDwords;             // Total # of DWORDS to be sent to FIFO
    DWORD    DataDWord;               // Temp Storage for destination datadword
    DWORD    TopClippedAmount;        // # of scanlines clipped from top of DIB
    DWORD    LeftClippedAmountBytes;  // # of BYTES clipped from left of DIB
    DWORD    LeftClippedAmountPixels; // # of BYTES clipped from left of DIB
    DWORD    PixelsInScanline;        // # if pixels in dest scanline
    DWORD    dwtemp;                  // Temp var for FIFO macros
    DWORD    BytesInScanline;         // # of BYTES with DIB data/scanline 
    LONG     i;                     
    DWORD    j;
    DWORD  * ColorTable;              // Local ptr to DIB color xlation tbl
    DWORD    nibble;                  // = 0,1 for which nibble in byte to use
    DWORD    pixbyte;                 // real byte counter for pixel loop
    TAKE_ADDRESS DWORD hclut[256];    // Color Lookup Table -- converts two
				 // 4-bit indices into 2 16-bit hicolor values
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    // See Host2Scrn8bppConvBlt for background on clipping, data retrieval

    //
    //  SET UP DESTINATION/TRANSFER VARIABLES
    //
    
    // DIBPitch for 4bpp dib is #pixels rounded to next byte boundary 
    // divided by two to yield bytes, rounded to next dword boundary.
    SrcDataPitch = (((lpSrc->bmiHeader.biWidth + 1) >> 1) + 3) & 0xFFFFFFFC;
    
    BytesInScanline=0;    

    if (lpClipRect) {
	// If lpRect Non-Null, intersect with default dest rect
    
        // Get usable pointer to clip rectangle
        FarToFlat(lpClipRect, lpRect);
    
        CTop    = max(lpRect->top,Y);
        CLeft   = max(lpRect->left,X);
        CBottom = min(lpRect->bottom,Y+(int)cScans);
        CRight  = min(lpRect->right,X+(int)lpSrc->bmiHeader.biWidth);
        
        // Added to fix Defect 1703
        // What happen is CRight is less then CLeft we get a negative PixelsInScanLine
        // There is never enough room and life gets worse until we come to a sw halt
        // Might as well check for a bogus y while we are at it
        if ((CRight <= CLeft) || (CBottom <= CTop))
            {
#if 0
            _asm int 03
#endif
            return 0;
            }


        TopClippedAmount       = lpRect->top  - Y;
        LeftClippedAmountPixels= lpRect->left - X;
        LeftClippedAmountBytes = (lpRect->left - X) >> 1;

// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB  //rbissell@stb

		if ((long)(lpSrc->bmiHeader.biHeight) <= 0)
		{
			SrcDataPtr = (BYTE*)lpSrcData
					   + (TopClippedAmount * SrcDataPitch)
					   + LeftClippedAmountBytes;
		}
		else
		{
			SrcDataPtr	= (BYTE *)lpSrcData
						+ (((DWORD)cScans - TopClippedAmount - 1) * SrcDataPitch)
						+ LeftClippedAmountBytes;
		}

#else //STBPERF_TOPDOWNDIB

        SrcDataPtr = (BYTE *)lpSrcData + 
	    (((DWORD)cScans - TopClippedAmount - 1) * SrcDataPitch) + 
	    LeftClippedAmountBytes;

#endif //STBPERF_TOPDOWNDIB
// STB End Change
        
    } else {                      
    
        CTop    = Y;
        CLeft   = X;
        CBottom = Y+(int)cScans;
        CRight  = X+lpSrc->bmiHeader.biWidth;
        
        TopClippedAmount       = 0;    //zero out locals that are used
        LeftClippedAmountPixels= 0;
        LeftClippedAmountBytes = 0;

// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB //rbissell@stb

		if ((long)(lpSrc->bmiHeader.biHeight) <= 0)
			SrcDataPtr = (BYTE*)lpSrcData;
		else
			SrcDataPtr = (BYTE *)lpSrcData + ((cScans - 1) * SrcDataPitch);

#else //STBPERF_TOPDOWNDIB

		SrcDataPtr   = (BYTE *)lpSrcData + (cScans - 1) * SrcDataPitch;

#endif //STBPERF_TOPDOWNDIB
// STB End Change

    }    

//PingZ 3/18/99 For unknown reasons writing more than 1024 DWORDS to launch area in this case
//creates bad corruption in 10x7x32 mode.  I looked at the "for loop" code, didn't see anything
//wrong.  So, to wrok around, let's punt.  There is no WB99 performance hit for doing this. 
//PRS #5207
    PixelsInScanline = CRight - CLeft;
    TotalDwords = PixelsInScanline * (CBottom - CTop);
	if(TotalDwords > MAXCOLORDWORDS)
	  return 0;

    // Moved by APS since BytesInScanline not set if lpClipRect
    // Bytes in scanline is the number of bytes that contain valid pixel
    //   data.  This could be #pixels or #pixels+1 depending on alignment.
    //
    //          WIDTH  OFST  bwidth
    //   IE:    even   even   pels/2
    //          even   odd    pels/2+1
    //          odd    even   pels/2+1
    //          odd    odd    pels/2+1
    //
    if ((LeftClippedAmountPixels & 0x01) || (PixelsInScanline & 0x01)) {
        BytesInScanline = (PixelsInScanline >> 1) + 1;
    } else {
        BytesInScanline = PixelsInScanline >> 1;
    } 

// STB Begin Change
#ifdef STBPERF_TOPDOWNDIB //rbissell@stb

	if ((long)(lpSrc->bmiHeader.biHeight) > 0)
		SrcDataPitch = -(int)SrcDataPitch;   // We _may_ need to walk the DIB backwards

#else //STBPERF_TOPDOWNDIB

	// Source pitch in BYTES 
    SrcDataPitch = -(int)SrcDataPitch;  // We need to walk the DIB backwards

#endif //STBPERF_TOPDOWNDIB
// STB End Change

    
    
    FXENTER("Host2Scrn4to2432ConvBlt", lpDst->deFlags, TRUE,
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
	    FXENTER_RECT, CLeft, CTop, CRight, CBottom,
	    FXENTER_NO_RECT, 0, 0, 0, 0,   );

    //  
    //  SET UP HARDWARE FOR COLOR HOST TO SCREEN BITBLT  
    //
    CMDFIFO_CHECKROOM(cf, 6);
    SETPH(cf,  (SSTCP_PKT2 | srcFormatBit | srcXYBit | dstSizeBit |
		     dstXYBit | commandBit)); 

    // Source Format is 32bpp, set src pitch
    dwtemp = (PixelsInScanline << 2) | SSTG_PIXFMT_32BPP;
    SET(cf, _FF(lpGRegs)->srcFormat, dwtemp);

    // No intra dword source clipping with 32bpp data
    SET(cf, _FF(lpGRegs)->srcXY, 0);
    
    // Each byte will translate into one dword of FIFO data
    SET(cf, _FF(lpGRegs)->dstSize, R32(CBottom - CTop, PixelsInScanline));
    
    // Destination X,Y blt start location
    SET(cf, _FF(lpGRegs)->dstXY, R32(CTop, CLeft));
    
    // ROP here is always SRCCOPY 
// STB Begin Changes
// STB-SR 1/18/99 Allowing only SRC and DST Rops
#ifdef STBPERF_ALLOWDESTROPS
    SETC(cf, _FF(lpGRegs)->command, gdwRop3 | SSTG_HOST_BLT);
#else
    SETC(cf, _FF(lpGRegs)->command, SSTG_ROP_SRCCOPY | SSTG_HOST_BLT);
#endif
// STB End Changes

    BUMP(6);

    //
    //   TRANSLATE AND TRANSFER DATA TO FIFO
    //
    
    lpData = SrcDataPtr;
    TotalDwords = PixelsInScanline * (CBottom - CTop);
    
//    ColorTable = (DWORD *)lpSrc->bmiColors;

//PingZ 3/12/99 lpSrc->bmiColors may not be pointing to the actual color table!!!  The only
//to guarantee that is to add bmiHeader.biSize to lpSrc.  This is to fix PRS #4604
    ColorTable = (DWORD *)(((BYTE *)lpSrc) + lpSrc->bmiHeader.biSize) ;
    
    // Set Start offset (0 or 1 pixel, 0 or 2 bytes)
    nibble = LeftClippedAmountPixels & 0x01;
    dwtemp = nibble;    // save for later reset
    if (TotalDwords < MAXCOLORDWORDS)
    {
	// Write whole transfer at once
	CMDFIFO_CHECKROOM(cf, TotalDwords + 1);
	SETPH(cf, SSTCP_PKT1                          |
	      SSTCP_PKT1_2D                       |
	      LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT |
	      (TotalDwords) << SSTCP_PKT1_NWORDS_SHIFT);

	for (i = 0; i < (CBottom - CTop); i++)
	{
            pixbyte = 0;
            nibble  = dwtemp;
	    for (j = 0; j < PixelsInScanline; j++)
	    {
                if (nibble) {
                    DataDWord = ColorTable[(lpData[pixbyte++]&0x0F)];
                }
                else {
                    DataDWord = ColorTable[((lpData[pixbyte]&0xF0)>>4)];
                }
		SET(cf,_FF(lpGRegs)->launch[0], DataDWord);
                nibble ^= 0x01;     // XOR with 1 to flip the nibble bit
	    }
	    (DWORD)lpData += SrcDataPitch;
	}

	BUMP(TotalDwords + 1);
        
    }
    else
    {                                // Transfer one scanline at a time.

	for (i = 0; i < (CBottom - CTop); i++)
	{
            
	    CMDFIFO_CHECKROOM(cf, PixelsInScanline + 1);
	    SETPH(cf, SSTCP_PKT1                              |
		  SSTCP_PKT1_2D                           |
		  (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT)   |
		  (PixelsInScanline << SSTCP_PKT1_NWORDS_SHIFT));

            pixbyte = 0;
            nibble  = dwtemp;
	    for (j = 0; j < PixelsInScanline; j++)  {
	    			
                if (nibble) {
                    DataDWord = ColorTable[(lpData[pixbyte++]&0x0F)];
                }
                else {
                    DataDWord = ColorTable[((lpData[pixbyte]&0xF0)>>4)];
                }
		SET(cf,_FF(lpGRegs)->launch[0], DataDWord);
                nibble ^= 0x01;     // XOR with 1 to flip the nibble bit
	    }	

	    (DWORD)lpData += SrcDataPitch;
	    BUMP(PixelsInScanline + 1);
	}

    }   

    FXLEAVE("Host2Scrn4to2432ConvBlt", cf, lpDst);

    return 1;
}

#endif      // PERF_NEWDIB

/*----------------------------------------------------------------------
Function name:  Host2Scrn16to16StrBlt

Description:    Workhorse for 16bpp (src/dst) StretchDIBits

Information:
    Just like the StretchBlt code, we have to copy the data into
    an offscreen buffer then stretch it to the screen because the
    hardware host->screen stretch is broken :(

Return:         INT     1 if success or,
                        0 if rejected.
----------------------------------------------------------------------*/
int Host2Scrn16to16StrBlt(LPDIBENGINE     lpDst,              // device dib
                      RECT FAR *      lpClipRect,
                      int             dstX,
                      int             dstY,
                      int             dstW,
                      int             dstH,
                      int             srcX,
                      int             srcY,
                      int             srcW,
                      int             srcH,
                      DWORD *         lpSrcData,          // buffer
                      LPBITMAPINFO    lpSrc )   
{
    RECT *   lpRect;            // Local pointer to Clipping Rectangle 
    BYTE *   SrcDataPtr;        // Pointer to actual start of clipped DIB data
    BYTE *   inUseSrcDataPtr;   // Old Pointer
    RECT     ClipRect;
    DWORD    dwFP1616;
    DWORD    SrcDataPitch;      // Pitch of source DIB data, in bytes
    int      y;                 
    DWORD    srcDataFormat;
    DWORD    clipSelect;
                        // DWORD in the FIFO, this is also a count 
                        // of dwords in a scanline
    DWORD startOffset;
    DWORD dwordsInScanline;
    DWORD dwExtra;
    DWORD xShrink;
    int yWhole;
    int yFrac;
    int yff;
    int xWhole;
    int xFrac;
    int xff;
    int xInff;
    int savW;
    int savH;
    int srcAdjY;

    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    clipSelect = 0;

    SrcDataPitch = ((lpSrc->bmiHeader.biWidth << 1) + 3) & 0xFFFFFFFC;

    // if we've got a clip rect, prepare it for flat access
    if ( lpClipRect )
    {
        FarToFlat( lpClipRect, lpRect )
    }
    else
    {
        // setup a fake clip rect of the actual dest size for use in calculations below
        lpRect = &ClipRect;
        STACKFP(lpRect, dwFP1616); 
        FarToFlat(dwFP1616, lpRect);
        lpRect->left  = dstX;
        lpRect->top   = dstY;
        lpRect->right = dstX + dstW;
        lpRect->bottom= dstY + dstH;
    }

    if (srcW > dstW)
      xShrink = TRUE;
    else
      xShrink = FALSE;
      
    //Calculate X DDA parameters
    xWhole = srcW/dstW;
    xFrac = srcW%dstW;
    xff = 0;
   
    //Calculate Y DDA paramaters
    yWhole = srcH/dstH;
    yFrac = srcH%dstH;
    yff = 0;
    //Calculate X Adjustments
    xInff = 0;
   
    // Optimization -- if clip is smaller then make operation smaller
    savW = dstW;
    if (dstX < lpRect->left)
       {
       // Calculate new src start X
       srcX = srcX + (lpRect->left - dstX) * xWhole;

       // Calculate X new faction adjustment
       xff = xFrac * (lpRect->left - dstX);
       while (xff >= dstW)
         {
         xff -= dstW;
         srcX++;
         }

       // New Destination X
       dstX = lpRect->left;
       }


    if (dstX + dstW > lpRect->right)
       {
       // New destination size
       dstW = lpRect->right - lpRect->left;
   
       // Reduce source width
       // Whole Part
       srcW = srcW - xWhole * (savW - dstW);

       // Fractional Part
       xInff = xFrac * (savW - dstW);
       while (xInff >= savW)
         {
         xInff -= savW;
         srcW--;
         }
       }

    savH = dstH;
    srcAdjY = 0;
    if (dstY < lpRect->top)
       {
       srcAdjY = srcAdjY + (lpRect->top - dstY) * yWhole;

       // Calculate new fraction start point
       yff = yFrac * (lpRect->top - dstY);
       while (yff >= dstH)
         {
         yff -= dstH;
         srcAdjY++;
         }
       
       // New start dstY
       dstY = lpRect->top;
       }

    if (dstY + dstH > lpRect->bottom)
       {
       // New destination Height
       dstH = lpRect->bottom - lpRect->top;
       }

    if ((long)lpSrc->bmiHeader.biHeight > 0)      // Bottom-Up DIB
    {
        startOffset  =  (lpSrc->bmiHeader.biHeight - 1) * SrcDataPitch +
                        ((lpSrc->bmiHeader.biHeight - srcY - srcH + srcAdjY) * -(int)SrcDataPitch + (srcX << 1));
        SrcDataPitch = -(int)SrcDataPitch;
    }
    else                                    // Top-Down DIB
    {
        srcY = -((int)lpSrc->bmiHeader.biHeight) - srcY - srcH + srcAdjY;
        startOffset = srcY * SrcDataPitch + (srcX<<1);
    }


    SrcDataPtr       = (BYTE *)lpSrcData + startOffset;
    if (xShrink)
      {
      dwordsInScanline = dstW >> 1;
      // Any Extra's ?
      dwExtra = dstW & 0x01;
      }
    else
      {
      dwordsInScanline = srcW >> 1;
      // Any Extra's ?
      dwExtra = srcW & 0x01;
      }


    FXENTER("Host2Scrn16to16StrBlt", lpDst->deFlags, TRUE,
	    cf, lpDst, FXENTER_NO_SRC, 
	    (FXENTER_NO_SRCFORMAT | FXENTER_COMPUTE_DSTBA_FORMAT),
	    FXENTER_RECT, dstX, dstY, (dstX+dstW), (dstY+dstH),
	    FXENTER_NO_RECT, 0, 0, 0, 0,   );

    srcDataFormat = (srcW << 1) | SSTG_PIXFMT_16BPP;

    inUseSrcDataPtr = 0x0;

    for (y=0; y<dstH; y++)
      {
      if (SrcDataPtr != inUseSrcDataPtr)
         {
         inUseSrcDataPtr = SrcDataPtr;
         CMDFIFO_SAVE(cf);
         if (xShrink)
            {
            // Avoid Double decimation...
            // This routine will do a shrink so we need to do a 1-1 copy...
            srcW = dstW;
            LoadShrinkNativeDIBScan((DWORD *)SrcDataPtr, 0x0, dstW, xWhole, xFrac, xff, savW, lpSrc->bmiHeader.biCompression);
            }
         else
            {
            LoadNativeDIBScan((DWORD *)SrcDataPtr, 0x0, srcW, srcDataFormat, dwordsInScanline, dwExtra, lpSrc->bmiHeader.biCompression);
            }

         CMDFIFO_RELOAD(cf);
         CMDFIFO_CHECKROOM(cf, 3);
         SETPH(cf, SSTCP_PKT2 | clip1minBit | clip1maxBit);
         SET(cf, _FF(lpGRegs)->clip1min, RECT_TOP_LEFT(lpRect));
         SET(cf, _FF(lpGRegs)->clip1max, RECT_BOTTOM_RIGHT(lpRect));
         BUMP(3);
         clipSelect = SSTG_CLIPSELECT;
         }
      CMDFIFO_SAVE(cf);
      StretchBltBufferToScreen( lpDst, (WORD)dstX, (WORD)dstY, (WORD)dstW, 
                (WORD)srcW, srcDataFormat, clipSelect );
      CMDFIFO_RELOAD(cf);

      // Fraction Overflow
      yff += yFrac;
      if (yff >= savH)
         {
         SrcDataPtr += SrcDataPitch;
         yff -= savH;
         }
      dstY++;
      SrcDataPtr += (SrcDataPitch)*yWhole;
      }

    // Our src and dst formats might have changed, so lets set these
    // flags to force enterleave to redo the formats
    _FF(gdiFlags) |= (SDATA_GDIFLAGS_DST_WAS_DEVBIT |
		      SDATA_GDIFLAGS_SRC_WAS_DEVBIT);

    FXLEAVE("Host2Scrn16to16StrBlt", cf, lpDst);
}


/*----------------------------------------------------------------------
Function name:  LoadNativeDIBScan

Description:    Copies a single scanline of a DIB to the offscreen
                stretch buffer.  Assumes that the scanline data is
                in 15 or 16 bpp
Information:

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
int LoadNativeDIBScan(DWORD *lpData, int srcX, int srcW, DWORD srcDataFormat, DWORD dwordsInScanline, DWORD dwExtra, DWORD biCompression)
{
    DWORD j;
    DWORD dwData;
    WORD * lpWord;

    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    CMDFIFO_SETUP(cf); 

    CMDFIFO_CHECKROOM(cf, 10);
    SETPH(cf, ( SSTCP_PKT2 |
        srcFormatBit   |
        dstFormatBit   |
        dstBaseAddrBit |
        clip1minBit    |
        clip1maxBit    |
        srcXYBit       |
        dstSizeBit     |
        dstXYBit       |
        commandBit )); 

    SET(cf, _FF(lpGRegs)->dstBaseAddr, lpDriverData->stretchBltStart);
    SET(cf, _FF(lpGRegs)->dstFormat,   srcDataFormat);
    SET(cf, _FF(lpGRegs)->clip1min,    0);
    // Our Stretchbuffer is 8K bytes, or 4K 16bpp pixels.
    SET(cf, _FF(lpGRegs)->clip1max,    0x00010FF0);     
    SET(cf, _FF(lpGRegs)->srcFormat,   srcDataFormat);
    SET(cf, _FF(lpGRegs)->srcXY,       (srcX & 3));
    SET(cf, _FF(lpGRegs)->dstSize,     R32(1, srcW)); // Single scanline
    SET(cf, _FF(lpGRegs)->dstXY,       R32(0, 0));

    SETC(cf, _FF(lpGRegs)->command, SSTG_ROP_SRCCOPY | 
                                    SSTG_HOST_BLT    |
                                    SSTG_CLIPSELECT   );

    BUMP(10);

    CMDFIFO_CHECKROOM(cf, dwordsInScanline + dwExtra + 1);
    SETPH(cf, (SSTCP_PKT1 |
           SSTCP_PKT1_2D |
           (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
           ((dwordsInScanline + dwExtra) << SSTCP_PKT1_NWORDS_SHIFT)));
    
   if (BI_BITFIELDS == biCompression)
      {
      for (j = 0; j < dwordsInScanline; j++)
         {
         SET(cf,_FF(lpGRegs)->launch[0], lpData[j]);
         }
      }	
    else
      {
      for (j = 0; j < dwordsInScanline; j++)
         {
         dwData =  ((lpData[j] & 0x7FE07FE0) << 1) | (lpData[j] & 0x001F001F);
         SET(cf,_FF(lpGRegs)->launch[0], dwData);
         }	
      }

    // Don't step off the page
    if (dwExtra)
        {
        lpWord = (WORD *)&lpData[j];
        if (BI_BITFIELDS == biCompression)
            dwData = (DWORD)(*lpWord);
        else
            dwData =  (DWORD)((*lpWord & 0x7FE0) << 1) | (*lpWord & 0x001F);

        SET(cf,_FF(lpGRegs)->launch[0], dwData);
        }
    
    BUMP(PixelsInScanline + 1);

    CMDFIFO_EPILOG(cf);

    return 1;
}

/*----------------------------------------------------------------------
Function name:  LoadShrinkNativeDIBScan

Description:    Copies and Shrink single scanline of a DIB to the offscreen
                stretch buffer.  Assumes that the scanline data is 
                in 15 or 16 bpp.
Information:

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
int LoadShrinkNativeDIBScan(DWORD *lpData, int srcX, int dstW, int xWhole, int xFrac, int xStartff, int savW, DWORD biCompression)
{
    int j;
    DWORD dwordsInScanLine;
    DWORD dwExtra;
    int ff;
    DWORD inPixel;
    DWORD nBPP;
    DWORD xSize;
    DWORD pixels;
    DWORD dwFP1616;
    BYTE * pInBuffer;
    BYTE * pOutBuffer;
    WORD wholeInc;
    WORD wData;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    CMDFIFO_SETUP(cf); 

    CMDFIFO_CHECKROOM(cf, 10);
    SETPH(cf, ( SSTCP_PKT2 |
        srcFormatBit   |
        dstFormatBit   |
        dstBaseAddrBit |
        clip1minBit    |
        clip1maxBit    |
        srcXYBit       |
        dstSizeBit     |
        dstXYBit       |
        commandBit )); 

    dwordsInScanLine = dstW >> 1;
    dwExtra = dstW & 0x01;
    SET(cf, _FF(lpGRegs)->dstBaseAddr, lpDriverData->stretchBltStart);
    SET(cf, _FF(lpGRegs)->dstFormat,   (dstW << 1) | SSTG_PIXFMT_16BPP);
    SET(cf, _FF(lpGRegs)->clip1min,    0);
    // Our Stretchbuffer is 8K bytes, or 4K 16bpp pixels.
    SET(cf, _FF(lpGRegs)->clip1max,    0x00010FF0);     
    SET(cf, _FF(lpGRegs)->srcFormat,   (dstW << 1) | SSTG_PIXFMT_16BPP);
    SET(cf, _FF(lpGRegs)->srcXY, 0x0);
    SET(cf, _FF(lpGRegs)->dstSize, R32(1, dstW)); // Single scanline
    SET(cf, _FF(lpGRegs)->dstXY, R32(0, 0));

    SETC(cf, _FF(lpGRegs)->command, SSTG_ROP_SRCCOPY | SSTG_HOST_BLT | SSTG_CLIPSELECT);

    BUMP(10);
    CMDFIFO_CHECKROOM(cf, (dwordsInScanLine + dwExtra) + 1);
    SETPH(cf, (SSTCP_PKT1 | SSTCP_PKT1_2D | (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) | ((dwordsInScanLine + dwExtra) << SSTCP_PKT1_NWORDS_SHIFT)));
    
    ff = xStartff;
    inPixel = 0;
    xSize = 0;
    nBPP = 2;
    wholeInc = (WORD)(xWhole * nBPP);

    pixels = 0x0;
    inPixel = 0x0;
    // Simple Decimation
    pInBuffer = (BYTE *)lpData;
    pOutBuffer = (BYTE *)&pixels;
    STACKFP(pOutBuffer, dwFP1616)
    FarToFlat(dwFP1616, pOutBuffer);

    for (j = 0; j < dstW; j++)
      {
      if (BI_BITFIELDS == biCompression)
         {
         (*(WORD *)&pOutBuffer[xSize]) = (WORD)(*(WORD *)&pInBuffer[inPixel]);
         }
       else
         {
         wData = (WORD)(*(WORD *)&pInBuffer[inPixel]);
         wData = ((wData & 0x7FE0) << 1) | (wData & 0x001F);
         (*(WORD *)&pOutBuffer[xSize]) = wData;
         }

      xSize += 2;
      if (4 == xSize)
         {
         // do write
         SET(cf, _FF(lpGRegs)->launch[0], pixels);
         xSize = 0;
         }

      ff += xFrac;
      if (ff >= dstW)
         {
         ff -= dstW;
         inPixel += nBPP;
         }
      inPixel += wholeInc;     
      }
   
    // Leftovers???
    if (xSize)
      SET(cf, _FF(lpGRegs)->launch[0], pixels);

    BUMP(dwordsInScanLine + 1);

    CMDFIFO_EPILOG(cf);

    return 1;
}

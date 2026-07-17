/* -*-c++-*- */
/* $Header: output.h, 2, 10/11/00 8:54:50 PM, Brent$ */
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
** File name:   output.h
**
** Description: Macros, defines, extern, structures, etc. for 
**              GDI output functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:50 PM$
**
** $History: output.h $
** 
** *****************  Version 5  *****************
** User: Michael      Date: 1/11/99    Time: 3:55p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 4  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

//extern DWORD lastPtr;
//extern DWORD hwAll;	
//extern DWORD hwBlt; hwBitBltSS, hwBitBltPS, hwBitBltHS ;
//extern DWORD hwText, hwAltPolygon, hwWindPolygon, hwPolyline; 
//extern DWORD hwRect, hwOutput ;
//extern DWORD hwScanline,hwPolyScanline;

// DWORD_PER_SCAN is the number of dwords that are put into the command
// fifo to draw a sca for os_scanline or os_polyscanline.
#define DWORDS_PER_SCAN 4 

// on bit is foreground.
#define DOT  0x0f
#define DOT_STYLE_SIZE 7

// dash =24 bkground = 8
#define DASH  0x00ffffff
#define DASH_STYLE_SIZE  31

//  dash=12 background =8 dot=4
#define DOTDASH 0x00f00fff 
#define DOTDASH_STYLE_SIZE 31

// dash =12 dot1 =4 dot2=4  bkground =4
#define DASHDOTDOT  0x0f0f0fff
#define DASHDOTDOT_STYLE_SIZE  31

// cde 021198
//
// The ideal driver output would match the DIB engine's output
// exactly; however, the DIB engine's styled polyline routine
// appears to be starting in the middle of its own pattern; so,
// I'm going to set the following value so that all of our
// patterns start in the same amount, we'll see if that fixes it.

#define MATCH_DIB (8<<SSTG_LSPOS_INT_SHIFT)

// cde 021398

typedef struct
{
    DWORD   dwLineStipple;
    DWORD   dwLineStyle;
    DWORD   dwCmdData;
} PEN_STIPPLE_INFO;

#define       DASH_STYLE (      DASH_STYLE_SIZE<<SSTG_LSSIZE_SHIFT) | MATCH_DIB
#define        DOT_STYLE (       DOT_STYLE_SIZE<<SSTG_LSSIZE_SHIFT)
#define    DOTDASH_STYLE (   DOTDASH_STYLE_SIZE<<SSTG_LSSIZE_SHIFT) | MATCH_DIB
#define DASHDOTDOT_STYLE (DASHDOTDOT_STYLE_SIZE<<SSTG_LSSIZE_SHIFT) | MATCH_DIB

#define CMD_STIPPLE (SSTG_EN_LINESTIPPLE | SSTG_REVERSIBLE)

typedef DIB_Pen * LPpen;

extern DWORD HandleBrush(DIB_Brush8 * lpBrush, DWORD Rop, 
					DRAWMODE  * lpDrawMode);

extern DWORD HandleFormat(DIBENGINE FAR * Src);


LONG hwOsPolyLine
(
	OutputParams* pParams,
	LPDIBENGINE   lpDestDev,
	LPRECT        lpClipRect
);


LONG   hwOsRect 
(
	OutputParams* pParams,
	LPDIBENGINE   lpDestDev,
	LPRECT        lpClipRect
);

LONG   hwOsPolyScanline 
(
	OutputParams* pParams,
	LPDIBENGINE   lpDestDev,
	LPRECT        lpClipRect
);

LONG   hwOsScanline 
(
	OutputParams* pParams,
	LPDIBENGINE   lpDestDev,
	LPRECT        lpClipRect
);

extern LONG hwOsAltPolygon 
(
	OutputParams* pParams,
	LPDIBENGINE   lpDestDev,
	LPRECT        lpClipRect
);

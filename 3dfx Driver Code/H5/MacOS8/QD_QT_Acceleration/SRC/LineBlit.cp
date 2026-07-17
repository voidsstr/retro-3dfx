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
** $Header: AcceleratedLineBlit.c, 4, 10/27/99 12:49:26 PM, Kenneth Dyke$
** $Log: 
**  4    3dfx      1.3         10/27/99 Kenneth Dyke    Code cleanup, minor fixes.
**  3    3dfx      1.2         10/11/99 Kenneth Dyke    Some performance fixes for
**       a few apps (pattern blits).  Also fixed TypeStyler crash bug (region
**       buffer overflow).  Added more debugging code as well.
**  2    3dfx      1.1         09/13/99 Kenneth Dyke    Fix for Eudora Pro.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 6     8/23/99 2:34p Kcd
** Bug fixes related to FIFO overflows.
** Added magic 565->32-bit blit support.
** 
** 5     7/12/99 11:55a Kcd
** Turned off WAX bug hack.
** 
** 4     7/02/99 3:33p Kcd
** New headers & HRM integration.
**
*/

#define WANT_DCON	0
#include "DConLoader.h"

#include <Types.h>
#include <Quickdraw.h>
#include <NQDAcceleration.h>
#include <InstallAcceleration.h>
#include <Devices.h>
#include <Displays.h>

#include "h3defs.h"
#include "h3gdefs.h"
#include "minihwc.h"
#include "hwcio.h"
#include <GraphicsPrivHwc.h>
#include "Utilities.h"
#include "FIFOClasses.h"
#include "PatBlit.h"

#include "RegionParser.h"


/* internal procedure prototypes */
static FxBool clip_line(FxI32 x1, FxI32 y1, FxI32 x2, FxI32 y2,
                      FxI32 *x1out, FxI32 *y1out, FxI32 *x2out, FxI32 *y2out);
  

enum
{
	// Don't try to accelerate lines shorter than this.
	kMinLineSize = 4
};

#if 0                    
static long Intersect
(
    Point *p1,                 // Line Segment 1, point 0
    Point *p2,                 // Line Segment 1, point 1
    Point *p3,                 // Line Segment 2, point 0
    Point *p4,                 // Line Segment 2, point 1
    Point *intersection        // Point of intersection
);

#define DONT_INTERSECT 0
#define DO_INTERSECT 1
#define COLLINEAR 2

#endif

void  LineBlitDiagonal(NQDDrawVars  *drawVars, Rect &dstRect);
void  LineBlitHorizVert(NQDDrawVars  *drawVars, Rect &dstRect);

/********************************************************************************
	GetAcceleratedLineBlitProc
		NQDDrawVars *drawVars
		
	
*/
Int32  GetAcceleratedLineBlitProc(NQDDrawVars  *drawVars)
{
	h3Info *h3InfoDst;
	SInt32 hDiff, vDiff;

	if (!CommonBlitAccept(*drawVars, &h3InfoDst))
		return false;

	// if we're disabled, get out.
	if(gPreferences->m2D[h3InfoDst->prefsIdx]->disableFlags & kDisableLineBlit)
		return false;
	
	// We don't handle mask regions yet
	if(drawVars->combineMask)
		Punt(drawVars);

	if (drawVars->unmappedMode > notPatBic)
		Punt(drawVars);

	// Check for the horizontal or vertical case
	hDiff = drawVars->pt2.h - drawVars->pt1.h;
	vDiff = drawVars->pt2.v - drawVars->pt1.v;

	if(	(hDiff < kMinLineSize) && (hDiff >= -kMinLineSize) && 
		(vDiff < kMinLineSize) && (vDiff >= -kMinLineSize))
	{
		// These short lines take longer to set up than they would to draw with software.
		Punt(drawVars);
	}
	else if ((hDiff == 0) || (vDiff == 0))
	{
		if (drawVars->patSolid)
		{
		return AcceptThisBlit(*drawVars, PatBlitSolid);
	}
		
		UInt32 patType =  drawVars->patType;
		if (drawVars->patNewFlag && patType)
		{
			// New style pattern

			// The line drawer can't seem to handle RGB patterns. The expanded
			// data isn't being set up for us.
			if (patType == 2)
				Punt(drawVars);
			
			// no odd transfer modes
			if (drawVars->mode > notPatBic)
				Punt(drawVars);

			// Dereferencing these handles is only safe for new-style patterns
			Rect patRect = drawVars->patHdl[0]->patMap[0]->bounds;
			
			// See if this is actually a 8x8 1bit pat, in newPat form
				if (patRect.right == 8 && patRect.bottom == 8 &&
						drawVars->patHdl[0]->patMap[0]->pixelSize == 1)
					return AcceptThisBlit(*drawVars, PatBlit1BitPattern);
						
			// It appears that the NQD line code does not setup the expanded pattern
			// data for us. Therefore, we can't use the PatBlitSmallPattern case.		
			Punt(drawVars);
		}
		else
		{
			// Old-style pattern, always 1-bit 8x8.

			// no odd transfer modes except transparent+patCopy.
			if ((drawVars->mode > notPatBic) && (drawVars->mode != 44))
				Punt(drawVars);

			return AcceptThisBlit(*drawVars, PatBlit1BitPattern);
		}
	}
	else
	{
		// MBW -- The value of arbitrary lines is dubious, and they're giving
		// incorrect results.  Turning them off for now.
		Punt(drawVars);
		
		// Only do 1x1 lines right now
		if(!(drawVars->penSize.h == 1 && drawVars->penSize.v == 1))
			Punt(drawVars);
		
		return AcceptThisBlit(*drawVars, LineBlitDiagonal);
	}

	Punt(drawVars);
}

/********************************************************************************
	LineBlitDiagonal
		NQDDrawVars *drawVars
		Rect *dstRect
		
	
*/
void  LineBlitDiagonal(NQDDrawVars  *drawVars, Rect &dstRect)
{
	static FxU32 commandEx = 0;

	FxI32  height, width, clipMin, clipMax;
	FxU32  dstRowBytes, pixFmt;
	Rect   clipRect;
	FxI32  x1,y1,x2,y2;

	Fifo2DRegs		blitter((h3Info *)drawVars->refCon);
	pixFmt = blitter.setDepth(drawVars->dstPixMap.pixelSize);

	height = dstRect.bottom - dstRect.top;
	width = dstRect.right - dstRect.left;
	dstRowBytes = drawVars->dstPixMap.rowBytes;

	clipRect.left = dstRect.left - drawVars->dstPixMap.bounds.left;
	clipRect.top = dstRect.top - drawVars->dstPixMap.bounds.top;
	clipRect.right = clipRect.left + width;
	clipRect.bottom = clipRect.top + height;

	/* Set up clipping for line segment based on destination rect. */
	clipMin = clipRect.left | (clipRect.top << 16);
	clipMax = clipRect.right | (clipRect.bottom << 16);

	x1 = drawVars->pt1.h - drawVars->dstPixMap.bounds.left;
	y1 = drawVars->pt1.v - drawVars->dstPixMap.bounds.top;

	x2 = drawVars->pt2.h - drawVars->dstPixMap.bounds.left;
	y2 = drawVars->pt2.v - drawVars->dstPixMap.bounds.top;

	/* ARGHH.  Some apps (which shall remain nameless) give us coordinates way
		out of range of what we can deal with in hardware (13 bit signed).
		So, I have to clip to that range manually.  I then let WAX do the rest. */

/*
	dprintf("oldLine: %d,%d - %d,%d\n",x1,y1,x2,y2);
*/

	if (!clip_line(x1,y1,x2,y2, &x1, &y1, &x2, &y2))
		return;

/*
	dprintf("newLine: %d,%d - %d,%d\n",x1,y1,x2,y2);
*/  		  

	if(gNewNQDOperation) 
	{
		gNewNQDOperation = false;

		blitter.reg(reg_dstBaseAddr, (FxU32) drawVars->dstPixMap.baseAddr & kBaseAddrOffsetMask);
		blitter.reg(reg_dstFormat, dstRowBytes | pixFmt);
		blitter.reg(reg_colorFore, drawVars->foreColor);
	}

	// Note: This blit actually uses the clipping registers
	blitter.reg(reg_clip1min,clipMin);
	blitter.reg(reg_clip1max,clipMax);

	blitter.reg(reg_srcXY, (x1 & 0xffff) | (y1 << 16));
	blitter.reg(reg_dstXY, (x2 & 0xffff) | (y2 << 16));
	blitter.reg(reg_command, SSTG_LINE | SSTG_GO | SSTG_CLIPSELECT | 
			ModeToROP(drawVars->dstPixMap.pixelSize, drawVars->unmappedMode));

	blitter.go();
}

#define LEFT 0x01
#define RIGHT 0x02
#define BOTTOM 0x04
#define TOP 0x08

/********************************************************************************
	find_code
		FxI32 x
		FxI32 y
		
	
*/
static FxU32 find_code(FxI32 x, FxI32 y)
{
	FxU32 code;
	if (x < 0) 
		code = LEFT;
	else if (x >= 4096) 
		code = RIGHT;
	else code = 0;
	if (y >= 4096) 
		code |= BOTTOM;
	else if (y < 0) 
		code |= TOP;
	return code;
}

/********************************************************************************
	clip_line
		FxI32 x1
		FxI32 y1
		FxI32 x2
		FxI32 y2
		FxI32 *x1out
		FxI32 *y1out
		FxI32 *x2out
		FxI32 *y2out
		
	
*/
static FxBool clip_line(FxI32 x1, FxI32 y1, FxI32 x2, FxI32 y2,
                      FxI32 *x1out, FxI32 *y1out, FxI32 *x2out, FxI32 *y2out)
{
	FxU32 code1 = find_code(x1, y1);
	FxU32 code2 = find_code(x2, y2);
	FxU32 code;
	FxI32 x, y;

	while(code1 | code2)
	{
		if(code1 & code2)
			return FXFALSE;

		if(code1) code = code1;
			else code = code2;

	if(code & LEFT) 
	{
		x = 0;
		y = y1 + (FxI32)((float)(y2 - y1)/(x2 - x1)*(-x1));
	}
	else if(code & RIGHT) 
	{
		x = 4095;
		y = y1 + (FxI32)((float)(y2 - y1)/(x2 - x1)*(x-x1));
	}
	else if(code & BOTTOM) 
	{
		y = 4095;
		x = x1 + (FxI32)((float)(x2 - x1)/(y2 - y1)*(y - y1));
	}
	else if(code & TOP) 
	{
		y = 0;
		x = x1 + (FxI32)((float)(x2 - x1)/(y2 - y1)*(-y1));
	}

	if(code == code1) 
		code1 = find_code(x1 = x, y1 = y);
	else 
		code2 = find_code(x2 = x, y2 = y);

	}
	
	*x1out = x1;
	*y1out = y1;
	*x2out = x2;
	*y2out = y2;

	return FXTRUE;
}


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
** $Header: AcceleratedSlabBlit.c, 3, 10/27/99 12:49:30 PM, Kenneth Dyke$
** $Log: 
**  3    3dfx      1.2         10/27/99 Kenneth Dyke    Code cleanup, minor fixes.
**  2    3dfx      1.1         10/11/99 Kenneth Dyke    Some performance fixes for
**       a few apps (pattern blits).  Also fixed TypeStyler crash bug (region
**       buffer overflow).  Added more debugging code as well.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 3     7/02/99 3:33p Kcd
** New headers & HRM integration.
**
*/

// Includes
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

// Types

struct SlabTracker
{
	NQDDrawVars	*drawVars;

	Boolean		dirty;
	UInt32		nextDest;
	UInt32		width;
	UInt32		height;

	UInt32		*dstPtr;
	UInt32		*dstRow;
	UInt32		dstByte;

	Rect		minRect;
	Int32		alignLeft;
	UInt32		firstMask;
	UInt32		lastMask;
	Int32		patHPos;
	Int32		patVPos;
};

typedef void (*SlabFlushProc)(SlabTracker &slab);

/******************************************************************************/
// This structure is used to track the globbing of lines together into
// rectangles.
struct SlabTrackerCommon
{
	SlabBlitProc	proc;
	SlabFlushProc	flushProc;		// Only needed in H3SlabBlitFinish
	SInt32			dstRowBytes;
	Int8			*dstBaseAddr;

	Rect			minRect;
	Int32			alignLeft;
	UInt32			firstMask;
	UInt32			lastMask;
	Int32			patHPos;
	Int32			patVPos;
};



/*----------------------------------------------------------------------------*/
class GetSlabInfo32
{
public:
	enum {pixelSize = 32};

	static void
	get(NQDDrawVars *drawVars, UInt32 &pixWidth, UInt32 &destAddr)
	{
		destAddr = (UInt32)drawVars->dstPtr;
		pixWidth = drawVars->dstLngCnt;		// Don't we need to add one because
											// this is zero based?
	}
	
	static void
	PaintPixel(NQDDrawVars *drawVars, UInt32 addr)
	{
		*(UInt32 *) addr = drawVars->foreColor;
	}
};

/*----------------------------------------------------------------------------*/
class GetSlabInfo16
{
public:
	enum {pixelSize = 16};

	static void
	get(NQDDrawVars *drawVars, UInt32 &pixWidth, UInt32 &destAddr)
	{
		destAddr = (UInt32)drawVars->dstPtr + NumLeadingZeroBytes(drawVars->firstMask);
		if (drawVars->dstLngCnt == 0)
		{
			if (drawVars->firstMask)
			{
				pixWidth = 4;
				pixWidth -= NumLeadingZeroBytes(drawVars->firstMask);
				pixWidth -= NumTrailingZeroBytes(drawVars->firstMask);
			}
			else
			{
				pixWidth = 4;
				pixWidth -= NumLeadingZeroBytes(drawVars->lastMask);
				pixWidth -= NumTrailingZeroBytes(drawVars->lastMask);
			}
		}
		else
		{
			// Get a number of bytes for each long.  If there is stuff in the masks,
			// take the bytes out that arent used.
			pixWidth = (drawVars->dstLngCnt + 1) << 2;
			if (drawVars->firstMask)
				pixWidth -= NumLeadingZeroBytes(drawVars->firstMask);
			pixWidth -= NumTrailingZeroBytes(drawVars->lastMask);
		}
		pixWidth >>= 1;
	}

	static void
	PaintPixel(NQDDrawVars *drawVars, UInt32 addr)
	{
		*(UInt16 *) addr = drawVars->foreColor;
	}
};

/*----------------------------------------------------------------------------*/
class GetSlabInfo8
{
public:
	enum {pixelSize = 8};

	static void
	get(NQDDrawVars *drawVars, UInt32 &pixWidth, UInt32 &destAddr)
	{
		destAddr = (UInt32)drawVars->dstPtr + NumLeadingZeroBytes(drawVars->firstMask);
		if (drawVars->dstLngCnt == 0)
		{
			if (drawVars->firstMask)
			{
				pixWidth = 4;
				pixWidth -= NumLeadingZeroBytes(drawVars->firstMask);
				pixWidth -= NumTrailingZeroBytes(drawVars->firstMask);
			}
			else
			{
				pixWidth = 4;
				pixWidth -= NumLeadingZeroBytes(drawVars->lastMask);
				pixWidth -= NumTrailingZeroBytes(drawVars->lastMask);
			}
		}
		else
		{
			// Get a number of bytes for each long.  If there is stuff in the masks,
			// take the bytes out that arent used.
			pixWidth = (drawVars->dstLngCnt + 1) << 2;
			if (drawVars->firstMask)
				pixWidth -= NumLeadingZeroBytes(drawVars->firstMask);
			pixWidth -= NumTrailingZeroBytes(drawVars->lastMask);
		}
	}

	static void
	PaintPixel(NQDDrawVars *drawVars, UInt32 addr)
	{
		*(UInt8 *) addr = drawVars->foreColor;
	}
};

// Globals
SlabTrackerCommon			gSlabCommon;
SlabTracker					gSlab1;
SlabTracker					gSlab2;

// Function Prototypes
	// These Routine Are used to glom together slab blits.
void SlabInit(NQDDrawVars *drawVars, SlabBlitProc proc);
template <class info> void SlabFirstLine(NQDDrawVars  *drawVars);
template <class info> void SlabSecondLine(NQDDrawVars  *drawVars);
template <class info> inline void SlabFlush(SlabTracker &slab);


/********************************************************************************
	GetAcceleratedSlabBlitProc
		NQDDrawVars *drawVars

	Called by NQD; selects a routine to handle the drawing operation specified in 
	drawVars.
*/
Int32  GetAcceleratedSlabBlitProc(NQDDrawVars  *drawVars)
{
	h3Info			*h3InfoDst;

	if (!CommonBlitAccept(*drawVars, &h3InfoDst))
		return false;

	// if we're disabled, get out.
	if (gPreferences->m2D[h3InfoDst->prefsIdx]->disableFlags & kDisableSlabBlit)
		return false;
	
	if (drawVars->colorizeFlag)
		Punt(drawVars);

	if (drawVars->trimResult > 0)
		Punt(drawVars);

	if (drawVars->mode >= 16)
		Punt(drawVars);

	// See if PatBlit can handle this.
	SlabBlitProc proc = SelectSlabPatBlitProc(drawVars);
	if (proc != 0)
	{
		GetArbitration(h3InfoDst);

		// Fire up the accumulator
		SlabInit(drawVars, proc);
		drawVars->refCon = (long) h3InfoDst;
		return true;
	}

	Punt(drawVars);
}


#pragma mark -
/********************************************************************************
	SlabInit
		NQDDrawVars *drawVars
		SlabBlitProc proc

	This routine inits parameters from the current state for a blit. This only
	occurs once for many slab blits.  This is a result of the slab blit accept
	proc.
	
	The things that shouldn't change across multiple slab blit calls between
	accept calls are:
		color
		pixel depth

	dstLngCnt = this is how many longs we have.  We know that the pixel depth
	of the destination won't change unless we get here again, so we can do
	width based on dest long count.

*/
void SlabInit(NQDDrawVars *drawVars, SlabBlitProc proc)
{
	gSlab1.height			= 0;
	gSlab2.height			= 0;
	gSlab1.dirty			= false;
	gSlab2.dirty			= false;

	gSlabCommon.dstRowBytes	= drawVars->dstPixMap.rowBytes;
	gSlabCommon.dstBaseAddr	= drawVars->dstPixMap.baseAddr;

	gSlabCommon.proc = proc;

	gSlabCommon.minRect		= drawVars->minRect;
	gSlabCommon.alignLeft	= drawVars->alignLeft;
	gSlabCommon.firstMask	= drawVars->firstMask;
	gSlabCommon.lastMask	= drawVars->lastMask;
	gSlabCommon.patHPos		= drawVars->patHPos;
	gSlabCommon.patVPos		= drawVars->patVPos;

	if (drawVars->dstPixMap.pixelSize > 16)
	{
		drawVars->blitProc = SlabFirstLine<GetSlabInfo32>;
		gSlabCommon.flushProc = SlabFlush<GetSlabInfo32>;
	}
	else if (drawVars->dstPixMap.pixelSize == 16)
	{
		drawVars->blitProc = SlabFirstLine<GetSlabInfo16>;
		gSlabCommon.flushProc = SlabFlush<GetSlabInfo16>;
	}
	else
	{
		drawVars->blitProc = SlabFirstLine<GetSlabInfo8>;
		gSlabCommon.flushProc = SlabFlush<GetSlabInfo8>;
	}
}

/********************************************************************************
	SlabStart
		NQDDrawVars *drawVars
		SlabTracker &slab
		
	Initializes a new slab
*/
template <class info>
inline void SlabStart(NQDDrawVars  *drawVars, SlabTracker &slab)
{
	UInt32 	baseDest;

	info::get(drawVars, slab.width, baseDest);

	slab.dstPtr			= drawVars->dstPtr;
	slab.dstRow			= drawVars->dstRow;
	slab.dstByte		= baseDest;
	
	slab.minRect		= drawVars->minRect;
	slab.alignLeft		= drawVars->alignLeft;
	slab.firstMask		= drawVars->firstMask;
	slab.lastMask		= drawVars->lastMask;
	slab.patHPos		= drawVars->patHPos;
	slab.patVPos		= drawVars->patVPos;

	slab.nextDest		= baseDest + gSlabCommon.dstRowBytes;
	slab.height			= 1;

	slab.drawVars		= drawVars;

	slab.dirty		= true;
}

/********************************************************************************
	SlabFirstLine
		NQDDrawVars *drawVars
	
	This routine pushes the first line into the the first rectangle.
*/
template <class info>
void SlabFirstLine(NQDDrawVars  *drawVars)
{
	SlabStart<info>(drawVars, gSlab1);
	gSlab2.dirty		= false;

	drawVars->blitProc	= SlabSecondLine<info>;
}

/********************************************************************************
	SlabSecondLine
		NQDDrawVars *drawVars
	
	This looks at the second line.  If this is under the first line, then
	it adds the two and goes into a single rect state.  Otherwise, it goes
	into a 2 rect state.
*/
template <class info>
void SlabSecondLine(NQDDrawVars  *drawVars)
{
	UInt32 	destAddr;
	UInt32 	pixWidth;

	// Flush the slabs when they start getting big. These flushes will parellelize with
	// the blitter activity, and reduce the wait time at the end of the operation
	if (gSlab1.height > 20)
	{
		SlabFlush<info>(gSlab1);
		SlabFirstLine<info>(drawVars);
		return;
	}

	info::get(drawVars, pixWidth, destAddr);

	if (gSlab1.nextDest == destAddr && gSlab1.width == pixWidth)
	{
		// We are in a first rectangle blit.
		++gSlab1.height;
		gSlab1.nextDest += gSlabCommon.dstRowBytes;
	}
	else if (destAddr < (UInt32)gSlab1.dstRow + (UInt32)gSlabCommon.dstRowBytes)
	{
		// We are in the same 'row' so we must be a 2 rectangle case.
		SlabStart<info>(drawVars, gSlab2);
		
		drawVars->blitProc = SlabNextDualLine<info>;
	}
	else
	{
		SlabFlush<info>(gSlab1);
		SlabFirstLine<info>(drawVars);
	}
}

/********************************************************************************
	SlabNextDualLine
		NQDDrawVars *drawVars
		
	
*/
template <class info>
void SlabNextDualLine(NQDDrawVars  *drawVars)
{
	UInt32 	destAddr;
	UInt32	pixWidth;
	
	// Flush the slabs when they start getting big. These flushes will parellelize with
	// the blitter activity, and reduce the wait time at the end of the operation
	if (gSlab1.height > 40)
	{
		SlabFlush<info>(gSlab1);
		SlabFlush<info>(gSlab2);
		SlabFirstLine<info>(drawVars);
		return;	
	}

	info::get(drawVars, pixWidth, destAddr);

	if (gSlab1.nextDest == destAddr && gSlab1.width == pixWidth)
	{
		++gSlab1.height;
		gSlab1.nextDest += gSlabCommon.dstRowBytes;
	}
	else if(gSlab2.nextDest == destAddr && gSlab2.width == pixWidth)
	{
		++gSlab2.height;
		gSlab2.nextDest += gSlabCommon.dstRowBytes;
	}
	else
	{
		SlabFlush<info>(gSlab1);
		SlabFlush<info>(gSlab2);
		SlabFirstLine<info>(drawVars);
	}
}

/********************************************************************************
	SlabFlush
		SlabTracker &slab

	This routine is the one that actually goes ahead and draws out the line.
	This is called to flush the first slab (if there are two.)  It will set
	any patterns if need be.
*/
template <class info>
void SlabFlush(SlabTracker &slab)
{
	SInt32		dstX, dstY;
	UInt32		offset;
	Rect		dstRect;
	h3Info		*h3InfoDst = (h3Info *) slab.drawVars->refCon;
	Fifo2DRegs	blitter(h3InfoDst);
	extern UInt32	gPatternCmdValue;
	
	// We need to do several things with solid slabs
	if (gSlabCommon.proc == PatBlitSolid)
	{		
	// Special case handling for solid patCopy blits; just draw 1x1 slabs directly.
		if (slab.width == 1 && slab.height == 1 && slab.drawVars->mode == 8)
	{
		info::PaintPixel(slab.drawVars, slab.dstByte);
		return;
	}

		// If this is a 66MHz slot, always set gNewNQDOperation to true
		if (h3InfoDst->is66MHz)
			gNewNQDOperation = true;
		else 
		{			
			if (!gNewNQDOperation)
			{
				blitter.dstSize_dstAddr_cmd_go(slab.width | (slab.height << 16),
						(UInt32) slab.dstByte & 0x03FFFFFF, gPatternCmdValue, info::pixelSize);
				return;
			}
		}
	}

	// AOM.  We are going to use his logic to figure out what the x, and Y
	// coords are.   We just want to get the accumulator basically working,
	// and we'll get it cleaner later.

	// This gets us the offset of the upper-left corner of this collection of slabs in bytes
	offset = (UInt32) slab.dstByte - (UInt32) gSlabCommon.dstBaseAddr;

	dstX = (offset % gSlabCommon.dstRowBytes) >> (info::pixelSize >> 4);
	dstY = offset / gSlabCommon.dstRowBytes;

	dstRect.top = dstY + slab.drawVars->dstPixMap.bounds.top;
	dstRect.left = dstX + slab.drawVars->dstPixMap.bounds.left;
	dstRect.bottom = dstRect.top + slab.height;
	dstRect.right = dstRect.left + slab.width;
	
	if (!gNewNQDOperation)
	{
		// Continue running the blitter for this operation.
		/* MBW -- XXX -- NOTE:
			This assumes internal knowledge of PatBlit.cp, namely that the following will give 
			correct results for all of the blitProcs that may be chosen by SelectSlabPatBlitProc().
			If either the set of functions that may be chosen or the implementations of those
			functions change, this may need to be reevaluated.
		*/

		if(gPatternCmdValue != 0)
		{

			blitter.dstSize_dstXY_cmd_go(
					slab.width | (slab.height << 16),
					dstRect.left - slab.drawVars->dstPixMap.bounds.left | 
						(dstRect.top - slab.drawVars->dstPixMap.bounds.top << 16),
					gPatternCmdValue,
					info::pixelSize);
		}
	}
	else
	{
		// Call the patBlit blitter for the first part.
		// Restore the important parts of drawVars (for proper pattern alignment)
		Rect		minRect		= slab.drawVars->minRect;
		Int32		alignLeft	= slab.drawVars->alignLeft;
		UInt32		firstMask	= slab.drawVars->firstMask;
		UInt32		lastMask	= slab.drawVars->lastMask;
		Int32		patHPos		= slab.drawVars->patHPos;
		Int32		patVPos		= slab.drawVars->patVPos;
		
		// MBW -- XXX -- This code can take these values from either gSlabCommon or slab.
		slab.drawVars->minRect		= slab.minRect;
		slab.drawVars->alignLeft	= slab.alignLeft;
		slab.drawVars->firstMask	= slab.firstMask;
		slab.drawVars->lastMask		= slab.lastMask;
		slab.drawVars->patHPos		= slab.patHPos;
		slab.drawVars->patVPos		= slab.patVPos;
		
		gSlabCommon.proc(slab.drawVars, dstRect);

		slab.drawVars->minRect		= minRect;
		slab.drawVars->alignLeft	= alignLeft;
		slab.drawVars->firstMask	= firstMask;
		slab.drawVars->lastMask		= lastMask;
		slab.drawVars->patHPos		= patHPos;
		slab.drawVars->patVPos		= patVPos;

		blitter.reg(reg_dstXY, 0);
		blitter.go(info::pixelSize);
		blitter.alignFifo();
	}
	
	slab.dirty = false;
}

/********************************************************************************
	H3SlabBlitFinish
		long refCon
		
	Flush out any accumulated rectangles.
	
*/
Int32  H3SlabBlitFinish(long refCon)
{
	if (gSlab1.dirty)
	{
		gSlabCommon.flushProc(gSlab1);

		if (gSlab2.dirty)
		{
			gSlabCommon.flushProc(gSlab2);
		}
	}

	// NOTE!!  H3BlitFinish wants the h3DistInfo passed in.
	return H3BlitFinish(refCon);
}

#pragma mark -
#pragma mark ¥ Scratch
#if 0

#endif
/******************************************************************************/
/*----------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

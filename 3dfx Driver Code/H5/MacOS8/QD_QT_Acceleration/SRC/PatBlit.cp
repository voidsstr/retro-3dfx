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
** $Header: AcceleratedPatBlit.c, 3, 10/27/99 12:49:28 PM, Kenneth Dyke$
** $Log: 
**  3    3dfx      1.2         10/27/99 Kenneth Dyke    Code cleanup, minor fixes.
**  2    3dfx      1.1         10/11/99 Kenneth Dyke    Some performance fixes for
**       a few apps (pattern blits).  Also fixed TypeStyler crash bug (region
**       buffer overflow).  Added more debugging code as well.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 3     7/12/99 11:55a Kcd
** Turned off WAX bug hack.
** 
** 2     7/02/99 3:33p Kcd
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
#include "GraphicsPrivHwc.h"
#include "hdwr_res_mgr.h"
#include "hrm_fifo.h"

#include "Utilities.h"
#include "FIFOClasses.h"
#include "RegionParser.h"

#include "PatBlit.h"

#include "BitBlit.h"

// Global Variables
// MBW -- This now holds the full contents of the command register
//		for the blit, instead of just the pattern offset.
UInt32	gPatternCmdValue;

// Function Declarations
void PatBlitSolid(NQDDrawVars *drawVars, Rect &dstRect);
void PatBlitSolidHilite(NQDDrawVars *drawVars, Rect &dstRect);
void PatBlitSmallPattern(NQDDrawVars *drawVars, Rect &dstRect);
void PatBlit1BitPattern(NQDDrawVars *drawVars, Rect &dstRect);
void PushPatOffscreen(NQDDrawVars *drawVars);
void PatBlitLargePattern(NQDDrawVars *drawVars, Rect &dstRect);

/********************************************************************************
	GetAcceleratedPatBlitProc
		NQDDrawVars *drawVars
		
	
*/
Int32  GetAcceleratedPatBlitProc(NQDDrawVars  *drawVars)
{
	h3Info *h3InfoDst;
	UInt32 mode = drawVars->mode;
	UInt32 dstPixSize = drawVars->dstPixMap.pixelSize;

	if (!CommonBlitAccept(*drawVars, &h3InfoDst))
		return false;		
	
	// if we're disabled, get out.
	if(gPreferences->m2D[h3InfoDst->prefsIdx]->disableFlags & kDisablePatBlit)
		return false;
	
	// We actually can handle colorize, do this later
	if (drawVars->colorizeFlag)
		Punt(drawVars);
		
	// We could handle mask cases as well
	if (drawVars->hasMask)
		Punt(drawVars);
		
	// Is this a solid pattern, or a patterned pattern?
	if (drawVars->patSolid)
	{
		// MBW -- Some applications specify hilite mode as 58.
		//		Sick and wrong, but QuickDraw likes it, so...
		if(mode == 58)
			mode = drawVars->mode = 170;
			
		// Handle solid Hilite mode, and solid boolean ops
		if (mode == 170)
		{
			return AcceptThisBlit(*drawVars, PatBlitSolidHilite);
		} else if ((mode <= notPatBic) || (mode == 44))
		{			
			return AcceptThisBlit(*drawVars, PatBlitSolid);
		}
	} else
	{
		// Patterned pattern. Handle special cases first, then general cases
		
		// Is this a new style pattern?
		UInt32 patType =  drawVars->patType;

		if (drawVars->patNewFlag && patType)
		{
			// New style pattern
			
			// no odd transfer modes
			if (mode > notPatBic)
				Punt(drawVars);
			
			// Dereferencing these handles is only safe for new-style patterns
			Rect patRect = drawVars->patHdl[0]->patMap[0]->bounds;

			if((patRect.right - patRect.left <= 8) && (patRect.bottom - patRect.top <= 8))
			{
				// This pattern is actually a 8x8 1bit pat, in newPat form
				if (patRect.right == 8 && patRect.bottom == 8 &&
						drawVars->patHdl[0]->patMap[0]->pixelSize == 1)
					return AcceptThisBlit(*drawVars, PatBlit1BitPattern);
						
				// It's an 8x8 (or smaller) pattern.
				return AcceptThisBlit(*drawVars, PatBlitSmallPattern);
			} else
			{
#ifdef INSTALL_ADVANCED_2D_ACCEL
				// It's a large pattern
				if (CheckScratchSpace(h3InfoDst))
				{
					// We only move stuff that is larger than 1/4th of
					// the pattern area.
					if (((drawVars->minRect.right - drawVars->minRect.left) *
						(drawVars->minRect.bottom - drawVars->minRect.top) *
						drawVars->dstPixMap.pixelSize >> 3) >
						((drawVars->patVMask + 4) >> 2));
					{
						// Set up to call a blitter
						PushPatOffscreen(drawVars);
						return AcceptThisBlit(*drawVars, PatBlitLargePattern);
					}
				}
#endif
			}
		} else
		{
			// Old style 8x8 1bit pattern

			// no odd transfer modes except transparent+patCopy.
			if ((mode > notPatBic) && (mode != 44))
				Punt(drawVars);

			return AcceptThisBlit(*drawVars, PatBlit1BitPattern);
		}
	}
	
	Punt(drawVars);
}

/********************************************************************************
	SelectSlabPatBlitProc
		NQDDrawVars *drawVars
		
	
	Assumptions:  This routine is called AFTER the slab blit accept proc
	is called.   This returns the appropriate pat blit proc so slab blit
	doesn't need to do it's own pats.

	We don't do hilites, masks, or other crap.  Those were thrown out
	in SlabBlitAccept.
*/
SlabBlitProc  SelectSlabPatBlitProc(NQDDrawVars  *drawVars)
{
	UInt32 dstPixSize = drawVars->dstPixMap.pixelSize;
	UInt32 mode = drawVars->mode;

	// Is this a solid pattern, or a patterned pattern?
	if (drawVars->patSolid)
	{
		return PatBlitSolid;
	}
	else
	{
		// Patterned pattern. Handle special cases first, then general cases
		
		// Is this a new style pattern?
		UInt32 patType =  drawVars->patType;
		if (drawVars->patNewFlag && patType)
		{
			// New style pattern

			// no odd transfer modes
			if (mode > notPatBic)
				Punt(drawVars);

			Rect patRect = drawVars->patHdl[0]->patMap[0]->bounds;

			if((patRect.right - patRect.left <= 8) && (patRect.bottom - patRect.top <= 8))
			{
				// This pattern is actually a 8x8 1bit pat, in newPat form
				if (patRect.right == 8 && patRect.bottom == 8 &&
						drawVars->patHdl[0]->patMap[0]->pixelSize == 1)
					return PatBlit1BitPattern;
						
				// It's an 8x8 (or smaller) pattern.
				return PatBlitSmallPattern;
			}
		}
		else
		{
			// Old style 8x8 1bit pattern
			return PatBlit1BitPattern;
		}
	}
	
	return (SlabBlitProc)0;
}

/********************************************************************************
	PatBlitSolid
		NQDDrawVars *drawVars
		Rect dstRect
		
	NOTE:  This case uses the same ROP mode mapping as 1-bit source patterns and
		blits.  
*/
void PatBlitSolid(NQDDrawVars *drawVars, Rect &dstRect)
{
	Fifo2DRegs		blitter(FindH3Info(drawVars->dstPixMap.baseAddr));
	UInt32 pixFormat = blitter.setDepth(drawVars->dstPixMap.pixelSize);
	h3Info		*h3InfoDst = (h3Info *) drawVars->refCon;

	if (gNewNQDOperation)
	{
		UInt32 mode, color, rop;
		

		mode = drawVars->unmappedMode;
		
		// Transparent mode morphs into this:
		if(mode == 44) 
		{
			mode = patOr;
		}
		
		rop = ModeToROPSolid(mode & 7, drawVars, color);

		if(rop != 0)
		{	
			// Set up the operation. We are doing a rectangle fill, from source data.
			gPatternCmdValue = SSTG_RECTFILL | SSTG_GO;
	
			// We don't need the one-bit transparency flag that may have been put into
			// the rop value by ModeToROP1.  Extract just the rop value.
			gPatternCmdValue |= (rop & SSTG_ROP0);

			if (h3InfoDst->is66MHz)
			{
				gPatternCmdValue |= SSTG_MONO_PATTERN | SSTG_TRANSPARENT;
				blitter.reg(reg_pattern0alias, -1 );
				blitter.reg(reg_pattern1alias, -1 );
			}
			
			blitter.reg(reg_colorFore, color);
			blitter.reg(reg_dstBaseAddr, (FxU32) drawVars->dstPixMap.baseAddr & kBaseAddrOffsetMask);
			blitter.reg(reg_dstFormat, drawVars->dstPixMap.rowBytes | pixFormat);	

			blitter.reg(reg_dstSize, dstRect.right - dstRect.left | ((dstRect.bottom - dstRect.top) << 16)); 
			blitter.reg(reg_dstXY, dstRect.left - drawVars->dstPixMap.bounds.left | 
					(dstRect.top - drawVars->dstPixMap.bounds.top << 16));
			blitter.reg(reg_command, gPatternCmdValue);

			blitter.go();

			if (h3InfoDst->is66MHz)
			{
				blitter.queueNOP();
				blitter.queueNOP();
				blitter.queueNOP();
				blitter.queueNOP();
				blitter.queueNOP();
				blitter.queueNOP();
				gNewNQDOperation = true;
			}
			else
			{
				gNewNQDOperation = false;
			}

			blitter.alignFifo();

		}
		else
		{
			// This is a no-op blit.  Don't actually do anything.
			gPatternCmdValue = 0;
		}
	}
	else
	{
		if(gPatternCmdValue != 0)
		{
			blitter.dstSize_dstXY_cmd_go(
					dstRect.right - dstRect.left | ((dstRect.bottom - dstRect.top) << 16),
					dstRect.left - drawVars->dstPixMap.bounds.left | 
						(dstRect.top - drawVars->dstPixMap.bounds.top << 16),
					gPatternCmdValue);
			if (h3InfoDst->is66MHz)
			{
				blitter.queueNOP();
				blitter.queueNOP();
				blitter.queueNOP();
				blitter.queueNOP();
			}
		}
	}
}

/********************************************************************************
	PatBlitSolidHilite
		NQDDrawVars *drawVars
		Rect dstRect
		
	The first time through, the pattern registers are set to the color defined by
	backColor XOR hilitColor. Then, the dest is blitted onto itself, and pixels that
	match the backColor or the hilitColor are Xored with the pattern.
*/
void PatBlitSolidHilite(NQDDrawVars *drawVars, Rect &dstRect)
{
	h3Info *h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);
	Fifo2DRegs		blitter(h3InfoDst);
	UInt32 pixFormat = blitter.setDepth(drawVars->dstPixMap.pixelSize);

	UInt32 dstX = dstRect.left - drawVars->dstPixMap.bounds.left;
	UInt32 dstY = dstRect.top - drawVars->dstPixMap.bounds.top;

	if (gNewNQDOperation) 
	{
		UInt32 baseAddr = (UInt32) drawVars->dstPixMap.baseAddr & kBaseAddrOffsetMask;
		
		blitter.reg(reg_dstBaseAddr, baseAddr);
		blitter.reg(reg_srcBaseAddr, baseAddr);
		blitter.reg(reg_dstFormat, drawVars->dstPixMap.rowBytes | pixFormat);
		blitter.reg(reg_srcFormat, drawVars->dstPixMap.rowBytes | pixFormat);

		blitter.reg(reg_srcColorkeyMin, drawVars->backColor);
		blitter.reg(reg_srcColorkeyMax, drawVars->backColor);
		blitter.reg(reg_dstColorkeyMin, drawVars->hilitColor);
		blitter.reg(reg_dstColorkeyMax, drawVars->hilitColor);

		FifoSetPattern	setpat(h3InfoDst);
		UInt32 pixFormat = setpat.setDepth(drawVars->dstPixMap.pixelSize);
		setpat.fill(drawVars->dstPixMap.pixelSize >> 2, drawVars->backColor ^ drawVars->hilitColor);

		blitter.reg(reg_rop, SSTG_ROP_PATINVERT | (SSTG_ROP_PATINVERT << 8) | (SSTG_ROP_PATINVERT << 16));

		gNewNQDOperation = false;
	}
	
	blitter.reg(reg_commandEx, SSTG_PAT_FORCE_ROW0 | SSTG_EN_SRC_COLORKEY_EX | SSTG_EN_DST_COLORKEY_EX);

	blitter.reg(reg_dstSize, dstRect.right - dstRect.left | (dstRect.bottom - dstRect.top << 16));
	blitter.reg(reg_dstXY, dstX | (dstY << 16));
	blitter.reg(reg_srcXY, dstX | ((dstY) << 16));
	blitter.reg(reg_command, SSTG_BLT | SSTG_GO | SSTG_ROP_SRC << SSTG_ROP0_SHIFT);

	blitter.go();
}

/********************************************************************************
	PatBlitSmallPattern
		NQDDrawVars *drawVars
		Rect dstRect
		
	Uses the hardware 8x8 pattern registers to draw color patterns.
*/
void PatBlitSmallPattern(NQDDrawVars *drawVars, Rect &dstRect)
{
	h3Info			*h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);
	Fifo2DRegs		blitter(h3InfoDst);
	UInt32			dstPixelSize = drawVars->dstPixMap.pixelSize;
	UInt32 			pixFormat = blitter.setDepth(dstPixelSize);

	// Handle putting the pattern data into the registers
	if (gNewNQDOperation) 
	{
		FifoSetPattern	setpat(h3InfoDst);
		setpat.setDepth(dstPixelSize);

		// Did NQD munge this pattern into something other than 8x8?
		if ((drawVars->patHMask + 4) != dstPixelSize || (drawVars->patVMask + 4) !=
				drawVars->patRowBytes << 3)
		{
			// Stupid Quickdraw. Unmunge the pattern into our local buffer
			UInt32 patternBuffer[64];
			UInt32 *patBufPtr = patternBuffer;
			UInt32 hPos = 0;
			UInt32 vPos = 0;
			for (UInt32 vertIndex = 0; vertIndex < 8; ++vertIndex)
			{
				for (UInt32 horizIndex = 0; horizIndex < dstPixelSize >> 2; ++horizIndex)
				{
					*patBufPtr++ = *(UInt32 *) ((UInt32) drawVars->patExData + hPos + vPos);
					hPos = (hPos + 4) & drawVars->patHMask;
				}
				vPos = (vPos + drawVars->patRowBytes) & drawVars->patVMask;
			}
			setpat.load(dstPixelSize << 1, patternBuffer);
		} else
		{
			setpat.load(dstPixelSize << 1, drawVars->patExData);
		}

		blitter.reg(reg_dstBaseAddr, (FxU32) drawVars->dstPixMap.baseAddr & kBaseAddrOffsetMask);
		blitter.reg(reg_dstFormat, drawVars->dstPixMap.rowBytes | pixFormat);

		// Determine pattern offset
		// The pattern is set up so that the pattern source is long aligned with the dest.
		// However, since all the QD software transfers are long aligned, you have to take
		// firstMask into account.
		UInt32 patXOffset = (	((drawVars->patHPos) >> (dstPixelSize >> 4))
								 - drawVars->alignLeft 
								 + drawVars->dstPixMap.bounds.left) 
								& 7;
		UInt32 patYOffset = (	(drawVars->patVPos / drawVars->patRowBytes)
								- drawVars->minRect.top +
								drawVars->dstPixMap.bounds.top) 
								& 7;
		gPatternCmdValue = (patXOffset << 17) | (patYOffset << 20);
		
		gPatternCmdValue |= ModeToROP(dstPixelSize, drawVars->mode);
		gPatternCmdValue |= SSTG_RECTFILL | SSTG_GO;
	
		blitter.reg(reg_dstSize, dstRect.right - dstRect.left | (dstRect.bottom - dstRect.top << 16));
		blitter.reg(reg_dstXY, dstRect.left - drawVars->dstPixMap.bounds.left | 
				(dstRect.top - drawVars->dstPixMap.bounds.top << 16));

		blitter.reg(reg_command, gPatternCmdValue);

		blitter.go();
		blitter.alignFifo();

		gNewNQDOperation = false;
	}
	else
	{
		blitter.dstSize_dstXY_cmd_go(
				dstRect.right - dstRect.left | (dstRect.bottom - dstRect.top << 16),
				dstRect.left - drawVars->dstPixMap.bounds.left | 
					(dstRect.top - drawVars->dstPixMap.bounds.top << 16),
				gPatternCmdValue);
	}
}

/********************************************************************************
	PatBlit1BitPattern
		NQDDrawVars *drawVars
		Rect dstRect
		
	Uses the hardware 1bpp 8x8 pattern registers to draw black and white patterns.
*/
void PatBlit1BitPattern(NQDDrawVars *drawVars, Rect &dstRect)
{
	h3Info			*h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);
	Fifo2DRegs		blitter(h3InfoDst);
	UInt32			dstPixelSize = drawVars->dstPixMap.pixelSize;
	UInt32 			pixFormat = blitter.setDepth(dstPixelSize);

	// Handle putting the pattern data into the registers
	if (gNewNQDOperation) 
	{
		UInt32 fore, back;
		UInt32 mode = drawVars->unmappedMode;
		bool xferInvert = false;
		
		// Transparent mode morphs into this:
		if(mode == 44) 
		{
			mode = patOr;
		}

		if (drawVars->patType)
		{
			// Need to get the color info from the pattern; the foreground
			// and background colors may not be set up correctly
			ColorTable	*ct = *drawVars->patHdl[0][0].patMap[0][0].pmTable;
			RGBColor	rgbFore = ct->ctTable[1].rgb;
			RGBColor	rgbBack = ct->ctTable[0].rgb;
			
			// This gets the bit depth dependant color from the RGB colors
			if (dstPixelSize < 16)
			{
				fore = Color2Index(&rgbFore);
				back = Color2Index(&rgbBack);
				
				fore |= fore << 8;
				fore |= fore << 16;
				back |= back << 8;
				back |= back << 16;
			} else if (dstPixelSize == 16)
			{
				fore =  (((UInt32) rgbFore.red   >> 1) & 0x00007C00) |
						(((UInt32) rgbFore.green >> 6) & 0x000003E0) |
						(((UInt32) rgbFore.blue >> 11) & 0x0000001F);
				back =  (((UInt32) rgbBack.red   >> 1) & 0x00007C00) |
						(((UInt32) rgbBack.green >> 6) & 0x000003E0) |
						(((UInt32) rgbBack.blue >> 11) & 0x0000001F);
						
				fore |= fore << 16;
				back |= back << 16;
			} else
			{
				fore =  (((UInt32) rgbFore.red << 8)  & 0x00FF0000) |
						(((UInt32) rgbFore.green)     & 0x0000FF00) |
						(((UInt32) rgbFore.blue >> 8) & 0x000000FF);
				back =  (((UInt32) rgbBack.red << 8)  & 0x00FF0000) |
						(((UInt32) rgbBack.green)     & 0x0000FF00) |
						(((UInt32) rgbBack.blue >> 8) & 0x000000FF);
			}
		} else
		{
		fore = drawVars->foreColor;
		back = drawVars->backColor;
		}
		
		gPatternCmdValue = ModeToROP1(mode, fore, back, xferInvert);

		// 1-bit pattern data needs to go into the register byte-swapped.
		if(xferInvert)
		{
			blitter.reg(reg_pattern0alias, ~__lwbrx(drawVars->patData, 0));
			blitter.reg(reg_pattern1alias, ~__lwbrx(drawVars->patData, 4));
		}
		else
		{
			blitter.reg(reg_pattern0alias, __lwbrx(drawVars->patData, 0));
			blitter.reg(reg_pattern1alias, __lwbrx(drawVars->patData, 4));
		}
		
		blitter.reg(reg_colorFore, fore);
		blitter.reg(reg_colorBack, back);

		blitter.reg(reg_dstBaseAddr, (FxU32) drawVars->dstPixMap.baseAddr & kBaseAddrOffsetMask);
		blitter.reg(reg_dstFormat, drawVars->dstPixMap.rowBytes | pixFormat);
		
		// Determine pattern offset. Since we're grabbing the pattern from the unexpanded data,
		// we need to use pattern offset values that reference the unexpanded data, not the
		// expanded data. (QuickDraw can and will change the offsets, so that the topleft of the
		// expanded data is a different pixel than the topleft of the unexpanded data).
		Point offset = {0, 0};
		char **curA5Ptr = (char **) LMGetCurrentA5();
		if (curA5Ptr)
			offset = *(Point *) ((*curA5Ptr) - 168);
			
		gPatternCmdValue |= ((drawVars->dstPixMap.bounds.left - offset.h) & 7) << 17 | 
				((drawVars->dstPixMap.bounds.top - offset.v) & 7) << 20;
		gPatternCmdValue |= SSTG_RECTFILL | SSTG_GO | SSTG_MONO_PATTERN;

		blitter.reg(reg_dstSize, dstRect.right - dstRect.left | (dstRect.bottom - dstRect.top << 16));
		blitter.reg(reg_dstXY, dstRect.left - drawVars->dstPixMap.bounds.left | 
				(dstRect.top - drawVars->dstPixMap.bounds.top << 16));
	
		blitter.reg(reg_command, gPatternCmdValue);
	
		blitter.go();
		blitter.alignFifo();
		gNewNQDOperation = false;
	}
	else
	{
		blitter.dstSize_dstXY_cmd_go(
				dstRect.right - dstRect.left | (dstRect.bottom - dstRect.top << 16),
				dstRect.left - drawVars->dstPixMap.bounds.left | 
					(dstRect.top - drawVars->dstPixMap.bounds.top << 16),
				gPatternCmdValue);
	}
	
}

/********************************************************************************
	PushPatOffscreen
		NQDDrawVars *drawVars
		
	Copies a large pattern into the QuickDraw scratch space.
*/
void PushPatOffscreen(NQDDrawVars *drawVars)
{
	h3Info			*h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);

	// Since we're about to start using the blitter, we must arbitrate for the hardware here.
	GetArbitration(h3InfoDst);
	
	Rect r;
	
	r.top = r.left = 0;
	r.bottom = (drawVars->patVMask + 4) / drawVars->patRowBytes;

	switch(drawVars->dstPixMap.pixelSize)
	{
		case 8:
			r.right = drawVars->patRowBytes;
			doHostBlit<variantBitBlt8>(
				h3InfoDst,
				(UInt32)drawVars->patExData,
				drawVars->patRowBytes,
				r,
				h3InfoDst->scratchSpace->start & kBaseAddrOffsetMask,
				drawVars->patRowBytes,
				r,
				srcCopy,
				0,
				0xFFFFFFFF,
				0x00000000);
		break;
		
		case 16:
			r.right = drawVars->patRowBytes >> 1;
			doHostBlit<variantBitBlt16>(
				h3InfoDst,
				(UInt32)drawVars->patExData,
				drawVars->patRowBytes,
				r,
				h3InfoDst->scratchSpace->start & kBaseAddrOffsetMask,
				drawVars->patRowBytes,
				r,
				srcCopy,
				0,
				0x00000000,
				0xFFFFFFFF);
		break;
		
		case 32:
			r.right = drawVars->patRowBytes >> 2;
			doHostBlit<variantBitBlt32>(
				h3InfoDst,
				(UInt32)drawVars->patExData,
				drawVars->patRowBytes,
				r,
				h3InfoDst->scratchSpace->start & kBaseAddrOffsetMask,
				drawVars->patRowBytes,
				r,
				srcCopy,
				0,
				0x00000000,
				0xFFFFFFFF);
		break;
	}
}

/********************************************************************************
	PatBlitLargePattern
		NQDDrawVars *drawVars
		Rect &dstRect
		
	Draws large (larger than 8x8) color patterns, generally by bitblitting chunks
	of the pattern from the QD scratch space.
*/
void PatBlitLargePattern(NQDDrawVars *drawVars, Rect &dstRect)
{
	h3Info			*h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);
	Fifo2DRegs		blitter(h3InfoDst);
	UInt32			srcHt, srcWd;
	UInt32			pixFormat;
	UInt32			bytesToPixelShift;
	UInt32			srcX, srcY;
	UInt32			dstX, dstY;
	UInt32			width, height;
	UInt32			xRemaining, yRemaining;
	UInt32			cmd;
	bool			firstTime = true;

	if (drawVars->dstPixMap.pixelSize > 16)
		bytesToPixelShift = 2;
	else if (drawVars->dstPixMap.pixelSize == 16)
		bytesToPixelShift = 1;
	else
		bytesToPixelShift = 0;

	// Quickdraw rules for patterns state the these numbers need to be powers of 2.
	srcWd = drawVars->patRowBytes >> bytesToPixelShift;
	srcHt = (drawVars->patVMask + 4) / (drawVars->patRowBytes);

	pixFormat = blitter.setDepth(drawVars->dstPixMap.pixelSize);

	// +-------------------------------------------------------------
	// Spew onto the screen.
	cmd = SSTG_BLT | SSTG_GO | ModeToROP(drawVars->dstPixMap.pixelSize, drawVars->mode & 7);
	
	// We need to do a bunch of screen to screen blits.
	// Set up the basic stuff that won't change
	blitter.reg(reg_srcBaseAddr, h3InfoDst->scratchSpace->start & kBaseAddrOffsetMask);
	blitter.reg(reg_srcFormat, drawVars->patRowBytes | pixFormat);

	blitter.reg(reg_dstBaseAddr, (FxU32) drawVars->dstPixMap.baseAddr & kBaseAddrOffsetMask);
	blitter.reg(reg_dstFormat, drawVars->dstPixMap.rowBytes | pixFormat);

	// The keystone of this implementation is determining the X & Y
	// offsets into the destRect of the pattern.  We were given them
	// by the QD in reference to the minRect.  We need to change
	// those so that we can blit them relative to the region-parsed
	// dstRect.
	// NOTE:  These numbers are the work opposite of patHPos and patVPos.
	UInt32	leftEdge, topEdge;

	// Step 1: Get the x,y address of the pixel in the pattern that is 'pinned' to the 
	// topleft corner of the minRect.
	leftEdge = (drawVars->patHPos + NumLeadingZeroBytes(drawVars->firstMask)) >> bytesToPixelShift;
	topEdge = (drawVars->patVPos / drawVars->patRowBytes);

	// Step 2: Add the pixel offset from the topleft of minRect to the topLeft of dstRect.
	leftEdge += (dstRect.left - drawVars->minRect.left);
	topEdge += (dstRect.top - drawVars->minRect.top);

	// Step 3: Mod the offsets with the width and height of the pattern, and turn them into 
	// negative offsets from the bottomright of the pattern
	leftEdge = (srcWd - leftEdge) & (srcWd - 1);
	topEdge = (srcHt - topEdge) & (srcHt - 1);

	yRemaining = dstRect.bottom - dstRect.top;
	dstY = dstRect.top - drawVars->dstPixMap.bounds.top;

	while (yRemaining)
	{
		// We do the full width for every height
		xRemaining = dstRect.right - dstRect.left;

		// Determine height.  We can have three different conditions.
		// One full tile height for middle rows, and two
		// truncated heights for first and last row.
		if (firstTime && topEdge)
		{
			height = topEdge;
			srcY = srcHt - topEdge;
			firstTime = false;
		}
		else
		{
			height = srcHt;
			srcY = 0;
		}

		// Make sure that we limit outselves to the desired rect.
		if (height > yRemaining)
			height = yRemaining;

		// Set our X destination based off of region-parsed rect and dstPixMap.
		dstX = dstRect.left - drawVars->dstPixMap.bounds.left;

		// Do left edge block.
		if (leftEdge)
		{
			width = leftEdge;
			if (width > xRemaining)
				width = xRemaining;
			srcX = srcWd - leftEdge;
			blitter.reg(reg_srcXY, srcX | (srcY << 16));
			blitter.reg(reg_dstSize, width | (height << 16));
			blitter.reg(reg_dstXY, dstX | (dstY << 16));
			blitter.reg(reg_command, cmd);
			blitter.go();
			xRemaining -= width;
			dstX += width;
		}

		// Do as many full width blocks as we can.
		srcX = 0;
		blitter.reg(reg_srcXY, srcX | (srcY << 16));
		blitter.reg(reg_dstSize, srcWd | (height << 16));

		while (xRemaining >= srcWd)
		{
			blitter.reg(reg_dstXY, dstX | (dstY << 16));
			blitter.reg(reg_command, cmd);
			blitter.go();
			xRemaining -= srcWd;
			dstX += srcWd;
		}		

		// Move the right edge block.
		if (xRemaining)
		{
			width = xRemaining;
			blitter.reg(reg_dstSize, width | (height << 16));
			blitter.reg(reg_dstXY, dstX | (dstY << 16));
			blitter.reg(reg_command, cmd);
			blitter.go();
		}
		yRemaining -= height;
		dstY += height;

		firstTime = false;
	}
}

#pragma mark -
#pragma mark ¥ Scratch


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
** $Header: AcceleratedBitBlit.c, 3, 10/27/99 12:49:25 PM, Kenneth Dyke$
** $Log: 
**  3    3dfx      1.2         10/27/99 Kenneth Dyke    Code cleanup, minor fixes.
**  2    3dfx      1.1         10/11/99 Kenneth Dyke    Some performance fixes for
**       a few apps (pattern blits).  Also fixed TypeStyler crash bug (region
**       buffer overflow).  Added more debugging code as well.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 6     8/23/99 2:34p Kcd
** Bug fixes related to FIFO overflows.
** Added magic 565->32-bit blit support.
** 
** 5     8/03/99 2:20p Kcd
** Don't accelerate blits when source data is unaligned.
** 
** 4     7/26/99 1:55p Kcd
** Added support for 8->15 and 8->32 bit scaled blits.
** 
** 3     7/12/99 11:54a Kcd
** Turned off WAX hack for screen to screen blits.
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
#include <GraphicsPrivHwc.h>
#include "DynamicPatches.h"
#include "Utilities.h"
#include "FifoClasses.h"
#include "BitBlit.h"
#include "RegionParser.h"
#include "DrawPictPatch.h"
#include "ArithBlitVector.h"

// Defines
enum
{
	kMaxFifoBlock = (64*1024) - 64
};

// Since 15->32 seems to be broken, we use 16->32 for hardware depth conversion.
// This is technically not bit-accurate, but I'd be surprised if we got any
// complaints.
#define HARDWARE_16_TO_32 1


// Globals
bool	gDoDitherCopy = true;
UInt32	gBitBltCmdValue;
UInt8	*gPinTable = 0;

// Function Declarations

RegionBlitProc	PickStretchBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars *drawVars);
RegionBlitProc	PickShrinkBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars *drawVars);
RegionBlitProc	PickMaskBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars *drawVars);
RegionBlitProc	PickHiliteBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars *drawVars);
RegionBlitProc	PickReadBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars */*drawVars*/);

struct dither32to8
{
	typedef UInt32 srcPixel;
	typedef UInt8 dstPixel;
	enum
	{
		dstPixelsPerLong = 4,
		dstPixelSize = 8,
		srcPixFmt = SSTG_PIXFMT_8BPP
#ifdef VOODOO4
					| SSTG_HOST_BYTE_SWIZZLE
#endif
		,
		dstPixFmt = SSTG_PIXFMT_8BPP
	};

	static ColorSpec *getColorTable(NQDDrawVars *drawVars) { return(drawVars->colorTable); }
	static UInt8 * getITTable(NQDDrawVars *drawVars) { return((UInt8*)&(drawVars->invTable->iTTable)); }
};

struct dither32to16
{
	typedef UInt32 srcPixel;
	typedef UInt16 dstPixel;
	enum
	{
		dstPixelsPerLong = 2,
		dstPixelSize = 16,
		srcPixFmt = SSTG_PIXFMT_16BPP
#ifdef VOODOO4
					| SSTG_HOST_WORD_SWIZZLE
#endif
		,
		dstPixFmt = SSTG_PIXFMT_16BPP
	};

	static ColorSpec *getColorTable(NQDDrawVars */*drawVars*/) { return(nil); }
	static UInt8 * getITTable(NQDDrawVars */*drawVars*/) { return(nil); }
};

// These are the functions that do the meat of BitBlt, RgnBlt, and ScaleBlt
template <class variant> void  
AcceleratedScreenToScreenBlitLL (NQDDrawVars  *drawVars, Rect *dstRect);

template <class variant> void  
AcceleratedHostToScreenBlitLL (NQDDrawVars  *drawVars, Rect *dstRect);

template <class hostVariant, class screenVariant> void  
AcceleratedHostStretchBlitLL (NQDDrawVars  *drawVars, Rect *dstRect);

template <class screenVariant> void  
AcceleratedHostShrinkBlitLL(NQDDrawVars  *drawVars, Rect &dstRect);

template <class srcVariant, class mskVariant>
void MaskBlit(NQDDrawVars *drawVars, Rect &dstRect);

template <class variant>
void HiliteBlit(NQDDrawVars *drawVars, Rect &dstRect);

template <class variant>
void AcceleratedScreenToHostBlitLL(NQDDrawVars  *drawVars, Rect &dstRect);


/********************************************************************************
	GetAcceleratedBitBlitProc
		NQDDrawVars *drawVars
		
	This is the Accept proc for NQD bit blit operations. This proc handles both region
	and non-region versions of bit blit operations. It looks at the NQDDrawVars fields,
	and chooses the appropriate function to handle the operation.
	
	Generally, this function will determine the general type of blit to perform, and then
	call a pick function to choose a pointer. The various pick functions mostly deal with
	depth conversion issues for a particular type of blit operation.
*/
Int32  GetAcceleratedBitBlitProc(NQDDrawVars  *drawVars)
{
	h3Info *h3InfoDst, *h3InfoSrc;
	
	// This function can't use the common accept proc, make sure this code mirrors what happens there.
//	if (!CommonBlitAccept(*drawVars, &h3InfoDst))
//		return false;

	// For debugging, always bail if the caps lock keys is down
	if (CapsLockDown())
		return false;

	// Get the source and destination board for the blit
	h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);
	h3InfoSrc = FindH3Info(drawVars->srcPixMap.baseAddr);
	
	// Make sure we found a board we can accelerate
	if(!h3InfoDst || h3InfoDst->fifo->exclusiveMode)
	{
		// Handle blits from an accelerated source here
		
		// rcf Turning off reads for now
		return false;
		
		if (!h3InfoSrc || h3InfoSrc->fifo->exclusiveMode)
			return false;
			
		// if we're disabled, get out.
		if(gPreferences->m2D[h3InfoSrc->prefsIdx]->disableFlags & kDisableBitBlit)
			return false;
		
		if (drawVars->colorizeFlag || (drawVars->mode != srcCopy) || drawVars->hasMask)
			Punt(drawVars);
		
		return AcceptThisBlit(*drawVars, PickReadBlitVariantLL(drawVars->srcPixMap.pixelSize, 
				drawVars->dstPixMap.pixelSize, drawVars));
		
		Punt(drawVars);
	}
			
	// We only handle 8, 16, or 32bpp dests
	if (drawVars->dstPixMap.pixelSize < 8)
		return false;
	
	// We are starting a new operation
	gNewNQDOperation = true;
	
	// if we're disabled, get out.
	if(gPreferences->m2D[h3InfoDst->prefsIdx]->disableFlags & kDisableBitBlit)
		return false;
	
	// We can only colorize 1bpp sources
	if((drawVars->colorizeFlag) && (drawVars->srcPixMap.pixelSize != 1))
		Punt(drawVars);

	if((drawVars->mode > notSrcBic) && (drawVars->mode != transparent))
	{
#ifdef VOODOO4
		// Only do arithmetic blits if they're host blits.
		if(h3InfoDst != h3InfoSrc) 
		{
			if(drawVars->mode == hilite)
			{
				if (!CheckScratchSpace(h3InfoDst))
				{
					// Couldn't allocate scratch space.
					return false;
				}

				return AcceptThisBlit(*drawVars, 
					PickHiliteBlitVariantLL( 
						drawVars->srcPixMap.pixelSize, 
						drawVars->dstPixMap.pixelSize, 
						drawVars));

			}
			
			if (gAltivecAvailable)
			{
				// Try to use the Altivec-enabled arithmetic blit functions.
				RegionBlitProc proc = PickArithBlitVectorVariantLL(drawVars->srcPixMap.pixelSize, 
						drawVars->dstPixMap.pixelSize, drawVars);
				if (proc)
					return AcceptThisBlit(*drawVars, proc);
			}
			
			// If no Altivec, or can't use the Altivec procs, try the non-Altivec procs
			return AcceptThisBlit(*drawVars, PickArithBlitVariantLL(drawVars, 
					drawVars->srcPixMap.pixelSize, drawVars->dstPixMap.pixelSize));
		}
#endif

		Punt(drawVars);
	}
	
	// According to the docs, the chip doesn't do color keying or depth conversion on right-to-left blits.
	if((drawVars->hBump < 0) && (drawVars->mode == transparent))
		Punt(drawVars);

	if(h3InfoDst == h3InfoSrc) 
	{
		if (drawVars->hasMask)
			Punt(drawVars);

		return AcceptThisBlit(*drawVars, PickScreenBlitVariantLL(drawVars->srcPixMap.pixelSize, 
				drawVars->dstPixMap.pixelSize, drawVars));
	} 
	else 
	{	
		if (drawVars->hasMask)
		{
			if (!CheckScratchSpace(h3InfoDst))
			{
				// Couldn't allocate scratch space.
				return false;
			}
				
			return AcceptThisBlit(*drawVars, PickMaskBlitVariantLL(drawVars->srcPixMap.pixelSize, 
					drawVars->dstPixMap.pixelSize, drawVars));
		}
		else
		{
			return AcceptThisBlit(*drawVars, PickHostBlitVariantLL(drawVars->srcPixMap.pixelSize, 
					drawVars->dstPixMap.pixelSize, drawVars));
		}
	}

	Punt(drawVars);
}

/********************************************************************************
	GetAcceleratedStretchBlitProc
		NQDDrawVars *drawVars
		bool &isHostBlit
		
	This function is called by the Stretch code in the StdBits patch. It is designed to 
	act like a NQD accept proc.  On return, isHostBlit will be set to true iff the
	blit will be done with the host blitter.  This is done so that the caller knows
	whether or not to allocate gStretchBitsBlock.
*/
Int32  GetAcceleratedStretchBlitProc(NQDDrawVars  *drawVars, bool &isHostBlit, bool shrink)
{
	h3Info *h3InfoDst, *h3InfoSrc;

	isHostBlit = false;

	if (!CommonBlitAccept(*drawVars, &h3InfoDst))
		Punt(drawVars);

	h3InfoSrc = FindH3Info(drawVars->srcPixMap.baseAddr);

	// We can only colorize 1bpp sources
	if((drawVars->colorizeFlag) && !shrink && (drawVars->srcPixMap.pixelSize != 1))
		Punt(drawVars);

	if(drawVars->combineMask)
		Punt(drawVars);

	if((drawVars->mode > notSrcBic) && (drawVars->mode != transparent))
		Punt(drawVars);
	
	// According to the docs, the chip doesn't do color keying or depth conversion on right-to-left blits.
	if((drawVars->hBump < 0) && (drawVars->mode == transparent))
		Punt(drawVars);

	if(h3InfoDst == h3InfoSrc) 
	{
		if(shrink)
		{
			// Can't do it.
			Punt(drawVars);
		}
		else
		{
			return AcceptThisBlit(*drawVars, PickScreenBlitVariantLL(drawVars->srcPixMap.pixelSize, 
					drawVars->dstPixMap.pixelSize, drawVars));
		}
	} 
	else 
	{	
		isHostBlit = true;

		if(shrink)
		{
			return AcceptThisBlit(*drawVars, PickShrinkBlitVariantLL(drawVars->srcPixMap.pixelSize, 
					drawVars->dstPixMap.pixelSize, drawVars));
		}
		else
		{
		return AcceptThisBlit(*drawVars, PickStretchBlitVariantLL(drawVars->srcPixMap.pixelSize, 
				drawVars->dstPixMap.pixelSize, drawVars));
	}
	}

	Punt(drawVars);
}

#pragma mark -
#pragma mark ¥ Screen blit

/********************************************************************************
	doScreenBlit
		h3Info *board
		UInt32 srcBase
		SInt32 srcRowBytes
		Rect &srcRect
		UInt32 dstBase
		SInt32 dstRowBytes
		Rect &dstRect
		Rect &dstClipped
		bool backwardsX
		UInt32 mode
		UInt32 backColor
		bool alignFifo
		
	
*/
template <class variant> void 
doScreenBlit(
	h3Info *board,
	UInt32 srcBase,
	SInt32 srcRowBytes,
	Rect &srcRect,
	UInt32 dstBase,
	SInt32 dstRowBytes,
	Rect &dstRect,
	Rect &dstClipped,
	bool backwardsX,
	UInt32 mode,
	UInt32 *scaleTable,
	UInt32 foreColor,
	UInt32 backColor,
	bool alignFifo,
	Int32 colorizeFlag = 0)
{
	Fifo2DRegs		blitter(board);

	UInt32  height, width, srcHeight, srcWidth, srcX, srcY, dstX, dstY;
	UInt32	rop;
	bool isStretch = false;
	bool backwardsY = false;

	unsigned long cmdFlags = 0;
	
	if (srcRowBytes < 0) 
	{
		backwardsY = true;
		srcRowBytes = -srcRowBytes;
		dstRowBytes = -dstRowBytes;
	}
	
  	height = dstRect.bottom - dstRect.top;
  	width = dstRect.right - dstRect.left;
  	
  	srcHeight = srcRect.bottom - srcRect.top;
  	srcWidth = srcRect.right - srcRect.left;

	srcX = srcRect.left;
	srcY = srcRect.top;

	dstX = dstRect.left;
	dstY = dstRect.top;


	// See if we are doing a stretch blit
  	if ((height != srcHeight) || (width != srcWidth))
  		isStretch = true;
  		
	if (isStretch)
	{
		// Stretch blit
		/*	
			To avoid doing the messy Bresenham error calculations, stretch blits will always blit the
			full destination rect and use the clip registers to get the real dstRect.  It appears that
			the chip does the setup properly in this case, so it doesn't even slow us down appreciably.

			NOTE: Yellow Chicken will NEVER call us to do a stretch.  If we hit this case, we have
				been called directly from our patch on StdBits().
				
			NOTE: Stretch blits should ONLY be done top-down, left-to-right.  The chip ignores the direction
				bits when stretching. 
		*/
		
		// Stretch blit uses the source size as well.
		blitter.reg(reg_srcSize, srcWidth | (srcHeight << 16));

		cmdFlags |= SSTG_STRETCH_BLT | SSTG_CLIPSELECT;
	}
	else
	{
		// Non-stretch blit
		cmdFlags |= SSTG_BLT | SSTG_CLIPSELECT;
		
		// The direction bits in the control register can be used for these blits.
		
		if (backwardsY) 
		{
			/* Bottom to top */
			srcY += height - 1;
			dstY += height - 1;
			cmdFlags |= SSTG_YDIR;
		}
		
		if(backwardsX) 
		{
			/* Right to left */
			srcX += width - 1;
			dstX += width - 1;
			cmdFlags |= SSTG_XDIR;
		}
	}
	
	if(variant::srcPixelSize != 1)
	{
		if (mode == transparent) 
		{
			rop = (SSTG_ROP_SRC << SSTG_ROP0_SHIFT);
			blitter.reg(reg_srcColorkeyMin, backColor);
			blitter.reg(reg_srcColorkeyMax, backColor);
			blitter.reg(reg_rop, (SSTG_ROP_SRC << 0) | (SSTG_ROP_DST << 8) | (SSTG_ROP_DST << 16));
			blitter.reg(reg_commandEx, SSTG_EN_SRC_COLORKEY_EX);
		}
		else
		{
			rop = variant::modeToRop(mode);	
		}
	}
	else
	{
		UInt32 fore, back;
		if(colorizeFlag)
		{
			// We need to use foreColor and backColor
			fore = foreColor;
			back = backColor;
		}
		else
		{
			// ScaleTable is correct.
			fore = scaleTable[1];
			back = scaleTable[0];
		}
		
		// Transparent modes map as follows:
		if(mode == transparent) 
		{
			if(((back ^ backColor) & variant::dstPixelMask) == 0)
			{
				// transparent background
				mode = srcOr;
			}
			else if(((fore ^ backColor) & variant::dstPixelMask) == 0)
			{
				// transparent foreground
				mode = notSrcBic;	
			}
			else
			{
				// no transparency
				mode = srcCopy;	
			}
		}
		else
		{
			// Standard ROP modes
		}
		
		// Note:  In the 1-bit case, whomever put the data on the board did so
		//			with an inverting transfer.  We don't need to do anything about it.
		bool xferInvert;
		
		rop = ModeToROP1(	mode,
							fore,
							back,
							xferInvert);
									
		blitter.reg(reg_colorFore, fore);
		blitter.reg(reg_colorBack, back);
	}

	blitter.reg(reg_clip1min, (dstClipped.left & 0x0000FFFF) | ( ((dstClipped.top) << 16) & 0xFFFF0000) );
	blitter.reg(reg_clip1max, (dstClipped.right & 0x0000FFFF) |( ((dstClipped.bottom) << 16) & 0xFFFF0000) );
			
	blitter.reg(reg_dstBaseAddr, dstBase & kBaseAddrOffsetMask);
	blitter.reg(reg_dstFormat, dstRowBytes | variant::dstPixFmt);
	blitter.reg(reg_srcBaseAddr, srcBase & kBaseAddrOffsetMask);
	
	blitter.reg(reg_srcFormat, srcRowBytes | 
			(variant::srcPixFmt & ~(SSTG_HOST_BYTE_SWIZZLE | SSTG_HOST_WORD_SWIZZLE)));
	blitter.reg(reg_srcXY, (srcX & 0x0000FFFF) | ((srcY << 16) & 0xFFFF0000));
	blitter.reg(reg_dstSize, width | (height << 16));
	blitter.reg(reg_dstXY, (dstX & 0x0000FFFF) | ((dstY << 16) & 0xFFFF0000));

	/* Start Blit operation */
	gBitBltCmdValue = SSTG_GO | rop | cmdFlags;
	blitter.reg(reg_command, gBitBltCmdValue);
	blitter.go(variant::apertureDepth);
	if(alignFifo)
	{
		blitter.alignFifo();
	}
}

/********************************************************************************
	AcceleratedScreenToScreenBlitLL
		NQDDrawVars *drawVars
		Rect &dstRect
		
	
*/
template <class variant> void  
AcceleratedScreenToScreenBlitLL(NQDDrawVars  *drawVars, Rect &dstRect)
{
	if (gNewNQDOperation)
	{
		Rect adjSrcRect, adjDstRect, dstClipped;
		
		adjSrcRect = drawVars->origSrcRect;
		FastOffsetRect(adjSrcRect, -drawVars->srcPixMap.bounds.left, -drawVars->srcPixMap.bounds.top);

		adjDstRect = drawVars->origDstRect;
		FastOffsetRect(adjDstRect, -drawVars->dstPixMap.bounds.left, -drawVars->dstPixMap.bounds.top);

		dstClipped = dstRect;
		FastOffsetRect(dstClipped, -drawVars->dstPixMap.bounds.left, -drawVars->dstPixMap.bounds.top);
		
		doScreenBlit<variant>(
			(h3Info *)drawVars->refCon,
			(UInt32)drawVars->srcPixMap.baseAddr,
			drawVars->srcPixMap.rowBytes,
			adjSrcRect,
			(UInt32)drawVars->dstPixMap.baseAddr,
			drawVars->dstPixMap.rowBytes,
			adjDstRect,
			dstClipped,
			(drawVars->hBump < 0),
			drawVars->mode,
			drawVars->scaleTable,
			drawVars->foreColor,
			drawVars->backColor,
			true,
			drawVars->colorizeFlag);
			
		gNewNQDOperation = false;
	}
	else
	{
		Fifo2DRegs		blitter((h3Info *)drawVars->refCon);

		blitter.clip1_cmd_go(
			(dstRect.left - drawVars->dstPixMap.bounds.left) | 
				((dstRect.top - drawVars->dstPixMap.bounds.top) << 16),
			(dstRect.right - drawVars->dstPixMap.bounds.left) | 
				((dstRect.bottom - drawVars->dstPixMap.bounds.top) << 16),
			gBitBltCmdValue,
			variant::apertureDepth);
	}
}

/********************************************************************************
	PickScreenBlitVariantLL
		UInt32 srcDepth
		UInt32 dstDepth
		NQDDrawVars *drawVars
		
	
*/
RegionBlitProc PickScreenBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars */*drawVars*/)
{
	RegionBlitProc result = nil;
	
	switch(srcDepth)
	{
		case 1:
		switch(dstDepth)
		{
			case 8:		result = AcceleratedScreenToScreenBlitLL<variant1to8>;	break;
			case 16:	result = AcceleratedScreenToScreenBlitLL<variant1to16>;	break;
			case 32:	result = AcceleratedScreenToScreenBlitLL<variant1to32>;	break;
		}
		break;
		
		case 8:
		switch(dstDepth)
		{
			case 8:		result = AcceleratedScreenToScreenBlitLL<variantBitBlt8>;	break;
		}
		break;

		case 16:
		switch(dstDepth)
		{
			case 16:	result = AcceleratedScreenToScreenBlitLL<variantBitBlt16>;	break;
			case 32:	result = AcceleratedScreenToScreenBlitLL<variant16to32HW>;	break;
		}
		break;

		case 32:
		switch(dstDepth)
		{
			case 32:	result = AcceleratedScreenToScreenBlitLL<variantBitBlt32>;	break;
		}
		break;
	}
	
	return(result);
}


#pragma mark -
#pragma mark ¥ Host blit

/********************************************************************************
	doHostBlit
		h3Info *board
		UInt32 srcBase
		SInt32 srcRowBytes
		Rect &srcRect
		UInt32 dstBase
		SInt32 dstRowBytes
		Rect &dstRect
		UInt32 mode
		UInt32 *scaleTable
		UInt32 foreColor
		UInt32 backColor
		
	Performs a host blit; that is, it copies pixels from host memory, and puts them onscreen.
	
	The template parameter 'variant' is used to describe the attributes of the source and destination
	bit depths. See BitBlit.h.
*/
template <class variant>  
void doHostBlit(
	h3Info *board,
	UInt32 srcBase,
	SInt32 srcRowBytes,
	Rect &srcRect,
	UInt32 dstBase,
	SInt32 dstRowBytes,
	Rect &dstRect,
	UInt32 mode,
	UInt32 *scaleTable,
	UInt32 foreColor,
	UInt32 backColor,
	Int32 colorizeFlag = 0,
	bool customROP = false)
{
	UInt32  height, width, srcX, srcY, dstX, dstY;
	UInt32  srcDataStart, srcDataEnd, rop;
	bool xferInvert = false;
	bool needsClip = false;

	Fifo2DRegs		blitter(board);

  	height = dstRect.bottom - dstRect.top;
  	width = dstRect.right - dstRect.left;

	if (srcRowBytes < 0) 
	{
		srcRowBytes = -srcRowBytes;
		dstRowBytes = -dstRowBytes;
	}

	srcX = srcRect.left;
	srcY = srcRect.top;

	dstX = dstRect.left;
	dstY = dstRect.top;
		
	if(variant::dstPixelSize == 1)
	{
		// This is a transfer to put 1-bit data on the board.
		// The hardware needs to think it's dealing with 8-bit pixels.
		width = ((srcX & 7) + width + 7) >> 3;
		srcX >>= 3;
		dstX >>= 3;
	}

	if(variant::srcPixelSize == 4)
	{
		// To avoid dealing with sub-byte source addresses, we expand the blit
		// by up to 2 pixels to compensate for them and use the clip registers to restrict drawing
		// to the appropriate rect.
			
		if((srcX & 1) != 0)
		{
			// Left edge of the source would be a sub-byte address.  Expand the left side of the blit by 1.
			srcX--;
			dstX--;
			width++;
			needsClip = true;
		}

		if(((srcX + width) & 1) != 0)
		{
			// Right edge of the source would be a sub-byte address.  Expand the right side of the blit by 1.
			width++;
			needsClip = true;
		}
	}

	// start out with offsets in bits from the left edge of srcRect
	srcDataStart = srcX * variant::srcPixelSize;
	srcDataEnd = (srcX + width) * variant::srcPixelSize;	

	if(variant::dstPixelSize != 1)
	{
		// round "outwards" to the nearest byte boundary and change to byte offsets
		srcDataStart >>= 3;
		srcDataEnd += 7;
		srcDataEnd >>= 3;
	}
	
	// add in the address of the left edge of the first scanline of the source
	srcDataStart += srcBase + (srcY * srcRowBytes);
	srcDataEnd += srcBase + (srcY * srcRowBytes);
	
	// At this point, srcDataStart points to the first byte of (or byte containing) the leftmost pixel,
	// and srcDataEnd points to the first byte of (or byte containing) the rightmost pixel.
	
	/* Truncate to longword addresses & compute initial alignment. */
	UInt32 hwLeftCount = variant::hwLeftCount(srcX, srcDataStart);
	
	/* Calculate src row longs. */
	UInt32 hwLongCount = variant::hwLongCount(srcDataStart, srcDataEnd);

	// This allows blits that do non-standard things with the ROP and/or colorkey registers to use this
	// function for host blit transfers.
	if(customROP)
	{
		rop = mode;
		if((variant::srcPixelSize != 1) || (variant::dstPixelSize == 1))
		{
		}
		else
		{
			blitter.reg(reg_colorFore, foreColor);
			blitter.reg(reg_colorBack, backColor);
		}
	}
	else
	{
		if((variant::srcPixelSize != 1) || (variant::dstPixelSize == 1))
		{
			if(mode == transparent) 
			{
				rop = (SSTG_ROP_SRC << SSTG_ROP0_SHIFT);
				blitter.reg(reg_srcColorkeyMin, backColor);
				blitter.reg(reg_srcColorkeyMax, backColor);
				blitter.reg(reg_rop, (SSTG_ROP_SRC << 0) | (SSTG_ROP_DST << 8) | (SSTG_ROP_DST << 16));
				blitter.reg(reg_commandEx, SSTG_EN_SRC_COLORKEY_EX);
			}
			else
			{
				rop = variant::modeToRop(mode);	
			}
		}
		else
		{
			UInt32 fore, back;
			if(colorizeFlag)
			{
				// We need to use foreColor and backColor
				fore = foreColor;
				back = backColor;
			}
			else
			{
				// ScaleTable is correct.
				fore = scaleTable[1];
				back = scaleTable[0];
			}
			
			// Transparent modes map as follows:
			if(mode == transparent) 
			{
				if(((back ^ backColor) & variant::dstPixelMask) == 0)
				{
					// transparent background
					mode = srcOr;
				}
				else if(((fore ^ backColor) & variant::dstPixelMask) == 0)
				{
					// transparent foreground
					mode = notSrcBic;	
				}
				else
				{
					// no transparency
					mode = srcCopy;	
				}
			}
			else
			{
				// Standard ROP modes
			}
			
			rop = ModeToROP1(	mode,
								fore,
								back,
								xferInvert);
										
			blitter.reg(reg_colorFore, fore);
			blitter.reg(reg_colorBack, back);
		}
	}
	
	if(needsClip)
	{
		blitter.reg(reg_clip1min, (dstRect.left) | (dstRect.top << 16));
		blitter.reg(reg_clip1max, (dstRect.right) | (dstRect.bottom << 16));
	}
			
	blitter.reg(reg_dstBaseAddr, dstBase & kBaseAddrOffsetMask);
	blitter.reg(reg_dstFormat, dstRowBytes | variant::dstPixFmt);
	blitter.reg(reg_srcXY, hwLeftCount | (srcY << 16));
	blitter.reg(reg_dstSize, width | (height << 16));
	blitter.reg(reg_dstXY, (dstX & 0x0000FFFF) | ((dstY << 16) & 0xFFFF0000));
	blitter.reg(reg_srcFormat, (hwLongCount << 2) | variant::srcPixFmt);
	blitter.reg(reg_command, SSTG_HOST_BLT | rop | (needsClip?SSTG_CLIPSELECT:0));
	blitter.go(variant::apertureDepth);
	

#ifdef VOODOO4
	// If the source is 32 bpp, and it's on a Voodoo 4 board, set up the reads
	// to use the cacheable big-endian memory area. This will make all the reads
	// cacheline reads, improving speed.
	if (variant::srcPixelSize == 32 && FindH3Info((void *) srcDataStart))
	{
		srcDataStart += 0x02000000;
	}
#endif

	if(xferInvert)
	{
		pushDataToFifo<variant::xferInvert>(
			board, 
			(variant::xferInvert::srcPtr)srcDataStart, 
			srcRowBytes,
			hwLongCount,
			height, 
			scaleTable);
	}
	else
	{
		pushDataToFifo<variant::xfer>(
			board, 
			(variant::xfer::srcPtr)srcDataStart, 
			srcRowBytes,
			hwLongCount,
			height, 
			scaleTable);
	}
	
#ifdef VOODOO4
	// After doing cacheline reads from a source on a Voodoo4 card, need to flush the cache entries
	if (variant::srcPixelSize == 32 && FindH3Info((void *) srcDataStart))
	{		
		for (SInt32 srcDataOffset = srcRowBytes * height; srcDataOffset >= -32; srcDataOffset -= 32)
			__dcbf((void *) srcDataStart, srcDataOffset);
	}
#endif

	// MBW -- I'm not sure what this fixes.  kcd says it's not clear to him either.
	// rcf Apparently, the Windows group found a problem where small screen to screen
	// blits would cause a chip hang. This workaround prevents the hang from occurring. 
	if ((width <= 16) || (height <= 16))
	{
		blitter.queueNOP();
	}
}


/********************************************************************************
	AcceleratedHostToScreenBlitLL
		NQDDrawVars *drawVars
		Rect &dstRect
		
	This is a wrapper for doHostBlit.
*/
template <class variant> void  
AcceleratedHostToScreenBlitLL(NQDDrawVars  *drawVars, Rect &dstRect)
{
	Rect adjSrcRect, adjDstRect;
		
	adjSrcRect = dstRect;
	FastOffsetRect(adjSrcRect, 
			-drawVars->dstRect.left + drawVars->srcRect.left - drawVars->srcPixMap.bounds.left, 
			-drawVars->dstRect.top + drawVars->srcRect.top - drawVars->srcPixMap.bounds.top);
	
	adjDstRect = dstRect;
	FastOffsetRect(adjDstRect, -drawVars->dstPixMap.bounds.left, -drawVars->dstPixMap.bounds.top);

	doHostBlit<variant>(
		(h3Info *)drawVars->refCon,
		(UInt32)drawVars->srcPixMap.baseAddr,
		drawVars->srcPixMap.rowBytes,
		adjSrcRect,
		(UInt32)drawVars->dstPixMap.baseAddr,
		drawVars->dstPixMap.rowBytes,
		adjDstRect,
		drawVars->mode & 0x0000003f,
		drawVars->scaleTable,
		drawVars->foreColor,
		drawVars->backColor,
		drawVars->colorizeFlag);
		
}

/********************************************************************************
	PickHostBlitVariantLL
		UInt32 srcDepth
		UInt32 dstDepth
		NQDDrawVars *drawVars
		
	This function picks the appropriate template instantiation for the type of blit operation
	being performed.
*/
RegionBlitProc PickHostBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars *drawVars)
{
	RegionBlitProc result = nil;
	
	switch(dstDepth)
	{
		case 8:		
			switch(srcDepth)
			{
				case 1:		result = AcceleratedHostToScreenBlitLL<variant1to8>;		break;
#ifdef INSTALL_ADVANCED_2D_ACCEL
				case 4:		result = AcceleratedHostToScreenBlitLL<variant4to8>;		break;
#endif
				case 8:		
					if(drawVars->bMustScale)
					{
#ifdef INSTALL_ADVANCED_2D_ACCEL
						result = AcceleratedHostToScreenBlitLL<variantBitBlt8Scaled>;		
#endif
					}
					else
					{
						result = AcceleratedHostToScreenBlitLL<variantBitBlt8>;		
					}
				break;
			}
		break;

		case 16:		
			switch(srcDepth)
			{
				case 1:		result = AcceleratedHostToScreenBlitLL<variant1to16>;		break;
				case 16:	result = AcceleratedHostToScreenBlitLL<variantBitBlt16>;	break;

#ifdef INSTALL_ADVANCED_2D_ACCEL
				case 4:		result = AcceleratedHostToScreenBlitLL<variant4to16>;		break;
				case 8:		result = AcceleratedHostToScreenBlitLL<variant8to16>;		break;
				case 32:	result = AcceleratedHostToScreenBlitLL<variant32to16>;		break;
#endif
			}
		break;

		case 32:		
			switch(srcDepth)
			{
				case 1:		result = AcceleratedHostToScreenBlitLL<variant1to32>;		break;
				case 32:	result = AcceleratedHostToScreenBlitLL<variantBitBlt32>;	break;

#ifdef INSTALL_ADVANCED_2D_ACCEL
				case 4:		result = AcceleratedHostToScreenBlitLL<variant4to32>;		break;
				case 8:		result = AcceleratedHostToScreenBlitLL<variant8to32>;		break;

				case 16:
#if HARDWARE_16_TO_32
					h3Info *h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);

					if(!(gPreferences->m2D[h3InfoDst->prefsIdx]->disableFlags & kBitAccurateOnly))
						result = AcceleratedHostToScreenBlitLL<variant16to32HW>;
					else
#endif
						result = AcceleratedHostToScreenBlitLL<variant16to32>;	
				break;
#endif
			}
		break;
	}

	return(result);
}


#pragma mark -
#pragma mark ¥ Host stretch blit

/********************************************************************************
	AcceleratedHostStretchBlitLL
		NQDDrawVars *drawVars
		Rect &dstRect
		
	
*/
template <class hostVariant, class screenVariant> void  
AcceleratedHostStretchBlitLL(NQDDrawVars  *drawVars, Rect &dstRect)
{
	h3Info		*h3InfoDst = (h3Info *)drawVars->refCon;
	Rect		adjSrcRect, adjDstRect, scratchRect, adjDstClip;
		
	adjSrcRect = drawVars->origSrcRect;
	FastOffsetRect(adjSrcRect, -drawVars->srcPixMap.bounds.left, -drawVars->srcPixMap.bounds.top);

	adjDstRect = drawVars->origDstRect;
	FastOffsetRect(adjDstRect, -drawVars->dstPixMap.bounds.left, -drawVars->dstPixMap.bounds.top);
	
	adjDstClip = dstRect;
	FastOffsetRect(adjDstClip, -drawVars->dstPixMap.bounds.left, -drawVars->dstPixMap.bounds.top);
	
	if (gNewNQDOperation)
	{
		SInt32		scratchRowBytes;
		UInt32		leftPixels = 0;
		UInt32		scratchStart = gStretchBitsBlock->start;

		if(gPictCacheFillAttempt && gPictCache.current.banded)
			scratchStart += gPictCache.current.bandOffset;

		// Only do this part once per region-parsed blit.
		gNewNQDOperation = false;
		
		if(hostVariant::srcPixelSize < 8)
		{
			// this is the number of pixels of sub-byte slop on the left.
			leftPixels = ((adjSrcRect.left * hostVariant::srcPixelSize) & 7) / hostVariant::srcPixelSize;
		}

		scratchRect.left = leftPixels;
		scratchRect.top = 0;
		scratchRect.right = leftPixels + adjSrcRect.right - adjSrcRect.left;
		scratchRect.bottom = adjSrcRect.bottom - adjSrcRect.top;
		
		// Find out how many bytes we really need for this (4-byte aligned)
		scratchRowBytes = ((scratchRect.right * hostVariant::dstPixelSize) + 31);
		scratchRowBytes >>= 3;
		scratchRowBytes &= ~3;
		
		short xferMode = srcCopy;
		
		if(hostVariant::srcPixelSize == 1)
		{
			// We may want to use an inverting transfer mode to put this on the board.
			bool invertXfer;
			short mode = drawVars->mode;
			
			UInt32 fore, back;
			if(drawVars->colorizeFlag)
			{
				// We need to use foreColor and backColor
				fore = drawVars->foreColor;
				back = drawVars->backColor;
			}
			else
			{
				// ScaleTable is correct.
				fore = drawVars->scaleTable[1];
				back = drawVars->scaleTable[0];
			}
			
			// Transparent modes map as follows:
			if(mode == transparent) 
			{
				if(((back ^ drawVars->backColor) & screenVariant::dstPixelMask) == 0)
				{
					// transparent background
					mode = srcOr;
				}
				else if(((fore ^ drawVars->backColor) & screenVariant::dstPixelMask) == 0)
				{
					// transparent foreground
					mode = notSrcBic;	
				}
				else
				{
					// no transparency
					mode = srcCopy;	
				}
			}
			else
			{
				// Standard ROP modes
			}
			
			ModeToROP1(mode, fore, back, invertXfer);
			
			if(invertXfer)
				xferMode = notSrcCopy;
			
		}
		
		if(hostVariant::srcPixelSize < 8)
		{
			// The transfer doesn't try to left-align the pixels.
			scratchRect.left -= leftPixels;
			adjSrcRect.left -= leftPixels;
		}
		
		// Put the band into the scratch space unstretched, with srcCopy or notSrcCopy mode (based on invertXfer)
		doHostBlit<hostVariant>(
			h3InfoDst,
			(UInt32)drawVars->srcPixMap.baseAddr,
			drawVars->srcPixMap.rowBytes,
			adjSrcRect,
			(UInt32)scratchStart,
			scratchRowBytes,
			scratchRect,
			xferMode,
			drawVars->scaleTable,
			drawVars->foreColor,
			drawVars->backColor,
			drawVars->colorizeFlag);

		if(hostVariant::srcPixelSize < 8)
		{
			// Make sure the final stretch _does_ left-align the pixels.
			scratchRect.left += leftPixels;
			adjSrcRect.left += leftPixels;
		}
		
		// stretch to screen with the blitter using the real transfer mode
		doScreenBlit<screenVariant>(
			h3InfoDst,
			(UInt32)scratchStart,
			scratchRowBytes,
			scratchRect,
			(UInt32)drawVars->dstPixMap.baseAddr,
			drawVars->dstPixMap.rowBytes,
			adjDstRect,
			adjDstClip,
			false,
			drawVars->mode,
			drawVars->scaleTable,
			drawVars->foreColor,
			drawVars->backColor,
			true,
			drawVars->colorizeFlag);

		if(gPictCacheFillAttempt)
		{
			// This blit is a fill of the picture cache, and it just succeeded.
			gPictCache.current.inUse = true;
			
			// Save this hrm block and don't let the StdBitsPatch code deallocate it.
			gPictCache.current.board = h3InfoDst;
			gPictCache.current.cacheBlock = gStretchBitsBlock;
			gStretchBitsBlock = nil;
			
			if(!gPictCache.current.banded)
			{
				// Save enough information about the put-on-screen blit to be able to reproduce it later.
				gPictCache.current.cacheDepth = screenVariant::srcPixelSize;
				gPictCache.current.cacheRowBytes = scratchRowBytes;
				gPictCache.current.cacheRect = scratchRect;
				gPictCache.current.mode = drawVars->mode;
				gPictCache.current.colorizeFlag = drawVars->colorizeFlag;
				gPictCache.current.scale1bit[0] = drawVars->scaleTable[0];
				gPictCache.current.scale1bit[1] = drawVars->scaleTable[1];
				gPictCache.current.foreColor = drawVars->foreColor;
				gPictCache.current.backColor = drawVars->backColor;
				gPictCache.current.hShrink = false;
				gPictCache.current.vShrink = false;
				
				if(hostVariant::srcPixelSize == 1)
				{
					// Stash the real colors so we can reconstruct them even with a different depth or ctSeed.
					Index2Color(gPictCache.current.scale1bit[0], &gPictCache.current.rgbScale0);
					Index2Color(gPictCache.current.scale1bit[1], &gPictCache.current.rgbScale1);
					Index2Color(gPictCache.current.foreColor, &gPictCache.current.rgbFore);
					Index2Color(gPictCache.current.backColor, &gPictCache.current.rgbBack);
				}
			}
			else
			{
				// This is not the first band.
			}
		}
		
	}
	else
	{
		Fifo2DRegs		blitter((h3Info *)drawVars->refCon);

		blitter.clip1_cmd_go(
			(dstRect.left - drawVars->dstPixMap.bounds.left) | 
				((dstRect.top - drawVars->dstPixMap.bounds.top) << 16),
			(dstRect.right - drawVars->dstPixMap.bounds.left) | 
				((dstRect.bottom - drawVars->dstPixMap.bounds.top) << 16),
			gBitBltCmdValue,
			screenVariant::apertureDepth);
	}

}

/********************************************************************************
	PickStretchBlitVariantLL
		UInt32 srcDepth
		UInt32 dstDepth
		NQDDrawVars *drawVars
		
	This function picks the appropriate template instantiation for the type of blit operation
	being performed.
*/
RegionBlitProc PickStretchBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars *drawVars)
{
	RegionBlitProc result = nil;
		
	switch(dstDepth)
	{
		case 8:		
			switch(srcDepth)
			{
				case 1:		result = AcceleratedHostStretchBlitLL<variant1Xfer8, variant1to8>;		break;
				case 4:		result = AcceleratedHostStretchBlitLL<variant4to8, variantBitBlt8>;		break;
				case 8:		
					if(drawVars->bMustScale)
					{
						result = AcceleratedHostStretchBlitLL<variantBitBlt8Scaled, variantBitBlt8>;		
					}
					else
					{
						result = AcceleratedHostStretchBlitLL<variantBitBlt8, variantBitBlt8>;		
					}
				break;
				default:
					// If we're caching a picture, we want to deal with 2 or 4 bit sources by using QuickDraw.
					if(gPictCacheFillAttempt)
						result = AcceleratedHostShrinkBlitLL<variantBitBlt8>;	
				break;	

			}
		break;

		case 16:		
			switch(srcDepth)
			{
				case 1:		result = AcceleratedHostStretchBlitLL<variant1Xfer16, variant1to16>;	break;
				case 4:		result = AcceleratedHostStretchBlitLL<variant4to16, variantBitBlt16>;		break;
				case 8:		result = AcceleratedHostStretchBlitLL<variant8to16, variantBitBlt16>;		break;
				case 16:	result = AcceleratedHostStretchBlitLL<variantBitBlt16, variantBitBlt16>;	break;
				case 32:	result = AcceleratedHostStretchBlitLL<variant32to16, variantBitBlt16>;		break;
				default:
					// If we're caching a picture, we want to deal with 2 or 4 bit sources by using QuickDraw.
					if(gPictCacheFillAttempt)
						result = AcceleratedHostShrinkBlitLL<variantBitBlt16>;	
				break;	
			}
		break;

		case 32:		
			switch(srcDepth)
			{
				case 1:		result = AcceleratedHostStretchBlitLL<variant1Xfer32, variant1to32>;		break;
				case 4:		result = AcceleratedHostStretchBlitLL<variant4to32, variantBitBlt32>;		break;
				case 8:		result = AcceleratedHostStretchBlitLL<variant8to32, variantBitBlt32>;		break;
#if HARDWARE_16_TO_32
				case 16:	result = AcceleratedHostStretchBlitLL<variant16to32HW, variantBitBlt32>;	break;
#else
				case 16:	result = AcceleratedHostStretchBlitLL<variant16to32, variantBitBlt32>;	break;
#endif
				case 32:	result = AcceleratedHostStretchBlitLL<variantBitBlt32, variantBitBlt32>;	break;
				default:
					// If we're caching a picture, we want to deal with 2 or 4 bit sources by using QuickDraw.
					if(gPictCacheFillAttempt)
						result = AcceleratedHostShrinkBlitLL<variantBitBlt32>;	
				break;	
			}
		break;
	}

	return(result);
}

#pragma mark -
#pragma mark ¥ Host shrink blit

/********************************************************************************
	AcceleratedHostShrinkBlitLL
		NQDDrawVars *drawVars
		Rect &dstRect
	
	NOTE: This blitProc should ONLY EVER BE CALLED by our StdBitsPatch directly.
	It should NOT be used by any code that might actually be called by Yellow Chicken,
	because it will call the real StdBits to put things on the board.  This would mean
	a reentrant call to StdBits, which is a good way to make Really Bad Things happen.
	
*/
template <class screenVariant> void  
AcceleratedHostShrinkBlitLL(NQDDrawVars  *drawVars, Rect &dstRect)
{
	h3Info		*h3InfoDst = (h3Info *)drawVars->refCon;
	if (gNewNQDOperation)
	{
		SInt32		scratchRowBytes;
		UInt32		leftPixels = 0;
		UInt32		scratchStart = gStretchBitsBlock->start;
		Rect		adjSrcRect, adjDstRect, scratchRect, adjDstClip;

		if(gPictCacheFillAttempt && gPictCache.current.banded)
			scratchStart += gPictCache.current.bandOffset;

		// Only do this part once per region-parsed blit.
		gNewNQDOperation = false;
				
		adjSrcRect = drawVars->origSrcRect;
		FastOffsetRect(adjSrcRect, -drawVars->srcPixMap.bounds.left, -drawVars->srcPixMap.bounds.top);

		adjDstRect = drawVars->origDstRect;
		FastOffsetRect(adjDstRect, -drawVars->dstPixMap.bounds.left, -drawVars->dstPixMap.bounds.top);
		
		adjDstClip = dstRect;
		FastOffsetRect(adjDstClip, -drawVars->dstPixMap.bounds.left, -drawVars->dstPixMap.bounds.top);
		
		bool hShrink = (drawVars->origSrcRect.right - drawVars->origSrcRect.left) > 
						(drawVars->origDstRect.right - drawVars->origDstRect.left);

		bool vShrink = (drawVars->origSrcRect.bottom - drawVars->origSrcRect.top) > 
						(drawVars->origDstRect.bottom - drawVars->origDstRect.top);

		scratchRect.left = 0;
		scratchRect.top = 0;
		
		if(hShrink)
			scratchRect.right = adjDstRect.right - adjDstRect.left;
		else
			scratchRect.right = adjSrcRect.right - adjSrcRect.left;

		if(vShrink)
			scratchRect.bottom = adjDstRect.bottom - adjDstRect.top;
		else
			scratchRect.bottom = adjSrcRect.bottom - adjSrcRect.top;
		
		// Find out how many bytes we really need for this (4-byte aligned)
		scratchRowBytes = ((scratchRect.right * screenVariant::dstPixelSize) + 31);
		scratchRowBytes >>= 3;
		scratchRowBytes &= ~3;
		
		// Set up the destination device and port to point to the scratch block.
		{
			// Current GDevice is already set to that of our screen by our device loop
			GrafPtr savePort;
			GetPort(&savePort);
			
			RGBColor fore, back;			
			GetForeColor(&fore);
			GetBackColor(&back);

			// Set up a CGrafPort that we can use for the scratch block.
			CGrafPort trickPort;
			OpenCPort(&trickPort);
			trickPort.portPixMap[0]->baseAddr = (Ptr)((scratchStart
#ifdef VOODOO4
			// On the voodoo4/5, we need to offset the address that we give QuickDraw 
			// so that it's in the same aperture as the screen.
				& ~0x0C000000) | (0x0C000000 & (UInt32)drawVars->dstPixMap.baseAddr
#endif
			));
			trickPort.portPixMap[0]->rowBytes = scratchRowBytes | 0x8000;
			trickPort.portPixMap[0]->bounds = scratchRect;			
			
			SetPort((GrafPtr)&trickPort);
			RGBForeColor(&fore);
			RGBBackColor(&back);

			// Call QuickDraw to put the data on the board with the correct shrink algorithm.
			PatchStdBits::PatchFn((BitMap*)drawVars->reserved1, &drawVars->origSrcRect, &scratchRect, srcCopy, nil);
			
			// Clean up after.
			SetPort(savePort);
			CloseCPort(&trickPort);
		}				
		
		// stretch (or not) to screen with the blitter using the real transfer mode
		doScreenBlit<screenVariant>(
			h3InfoDst,
			(UInt32)scratchStart,
			scratchRowBytes,
			scratchRect,
			(UInt32)drawVars->dstPixMap.baseAddr,
			drawVars->dstPixMap.rowBytes,
			adjDstRect,
			adjDstClip,
			false,
			drawVars->mode,
			drawVars->scaleTable,
			drawVars->foreColor,
			drawVars->backColor,
			true,
			drawVars->colorizeFlag);

		if(gPictCacheFillAttempt)
		{
			// This blit is a fill of the picture cache, and it just succeeded.
			gPictCache.current.inUse = true;
			
			// Save this hrm block and don't let the StdBitsPatch code deallocate it.
			gPictCache.current.board = h3InfoDst;
			gPictCache.current.cacheBlock = gStretchBitsBlock;
			gStretchBitsBlock = nil;

			if(!gPictCache.current.banded)
			{
				// Save enough information about the put-on-screen blit to be able to reproduce it later.
				gPictCache.current.cacheDepth = screenVariant::dstPixelSize;
				gPictCache.current.cacheRowBytes = scratchRowBytes;
				gPictCache.current.cacheRect = scratchRect;
				gPictCache.current.mode = drawVars->mode;
				gPictCache.current.colorizeFlag = drawVars->colorizeFlag;
				gPictCache.current.scale1bit[0] = drawVars->scaleTable[0];
				gPictCache.current.scale1bit[1] = drawVars->scaleTable[1];
				gPictCache.current.foreColor = drawVars->foreColor;
				gPictCache.current.backColor = drawVars->backColor;
				
				// Save the shrink-matching information for this blit.
				gPictCache.current.hShrink = hShrink;
				gPictCache.current.vShrink = vShrink;
				gPictCache.current.hShrinkSize = gPictCache.dstRect.right - gPictCache.dstRect.left;
				gPictCache.current.vShrinkSize = gPictCache.dstRect.bottom - gPictCache.dstRect.top;
			}
			else
			{
				// This is not the first band.
			}
		}
	}
	else
	{
		Fifo2DRegs		blitter((h3Info *)drawVars->refCon);

		blitter.clip1_cmd_go(
			(dstRect.left - drawVars->dstPixMap.bounds.left) | 
				((dstRect.top - drawVars->dstPixMap.bounds.top) << 16),
			(dstRect.right - drawVars->dstPixMap.bounds.left) | 
				((dstRect.bottom - drawVars->dstPixMap.bounds.top) << 16),
			gBitBltCmdValue,
			screenVariant::apertureDepth);
	}

}

/********************************************************************************
	PickShrinkBlitVariantLL
		UInt32 srcDepth
		UInt32 dstDepth
		NQDDrawVars *drawVars
		
	This function picks the appropriate template instantiation for the type of blit operation
	being performed.
*/
RegionBlitProc PickShrinkBlitVariantLL(UInt32 /*srcDepth*/, UInt32 dstDepth, NQDDrawVars */*drawVars*/)
{
	RegionBlitProc result = nil;
		
	switch(dstDepth)
	{
		case 8:		
			result = AcceleratedHostShrinkBlitLL<variantBitBlt8>;		
		break;

		case 16:		
			result = AcceleratedHostShrinkBlitLL<variantBitBlt16>;
		break;

		case 32:		
			result = AcceleratedHostShrinkBlitLL<variantBitBlt32>;
		break;
	}

	return(result);
}

#pragma mark -
#pragma mark ¥ Mask blit

/********************************************************************************
	PickMaskBlitVariantLL
		UInt32 srcDepth
		UInt32 dstDepth
		NQDDrawVars *drawVars
		
	This function picks the appropriate template instantiation for the type of blit operation
	being performed.
*/
RegionBlitProc PickMaskBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars *drawVars)
{
	RegionBlitProc result = nil;
	
#ifndef INSTALL_ADVANCED_2D_ACCEL
	return nil;
#endif

	if(drawVars->mskPixMap.pixelSize != 1)
	{
		// The mask isn't 1 bit deep
		return(nil);
	}
	
 	if(	(drawVars->mskPixMap.pmTable != nil) &&
 		(drawVars->mskPixMap.pmTable[0] != nil) &&
 		(drawVars->mskPixMap.pmTable[0]->ctSeed != 1))
 	{
 		// The mask isn't using the standard 1-bit color table (0 = white, 1 = black)
		return(nil);
	}

	switch(dstDepth)
	{
		case 8:		
			switch(srcDepth)
			{
				case 1:		result = MaskBlit<variant1to8, variant1to8>;		break;
				case 8:		
					if(drawVars->bMustScale)
					{
						result = MaskBlit<variantBitBlt8Scaled, variant1to8>;		
					}
					else
					{
						result = MaskBlit<variantBitBlt8, variant1to8>;		
					}
				break;
			}
		break;

		case 16:		
			switch(srcDepth)
			{
				case 1:		result = MaskBlit<variant1to16, variant1to16>;			break;
				case 8:		result = MaskBlit<variant8to16, variant1to16>;			break;
				case 16:	result = MaskBlit<variantBitBlt16, variant1to16>;		break;
				case 32:	result = MaskBlit<variant32to16, variant1to16>;			break;
			}
		break;

		case 32:		
			switch(srcDepth)
			{
				case 1:		result = MaskBlit<variant1to32, variant1to32>;			break;
				case 8:		result = MaskBlit<variant8to32, variant1to32>;			break;
				case 16:	result = MaskBlit<variant16to32, variant1to32>;			break;
				case 32:	result = MaskBlit<variantBitBlt32, variant1to32>;		break;
			}
		break;
	}

	return(result);
}

/*----------------------------------------------------------------------------*\
	==> MaskBlit <==
	
	Okay, here's the plan.  We move the mask and source to offscreen memory,
	use the mask to clear out the masked area of the source and the non-masked
	are in the dest.  We then insert the remainder of the source area.
	
	Because we could do this for a large area we may have to do this in slabs.
	We will use the qd scratch space available. (256K)
\*----------------------------------------------------------------------------*/

template <class srcVariant, class mskVariant>
void MaskBlit(NQDDrawVars *drawVars, Rect &dstRect)
{
	h3Info		*h3InfoDst = (h3Info *)drawVars->refCon;
	Fifo2DRegs	blitter(h3InfoDst);

	UInt32	*srcLFB = (UInt32 *)h3InfoDst->scratchSpace->start;

	UInt32	rop;

	UInt32	slabWidth;					// Pixel width of slab
	UInt32	slabHeight;					// Pixel height of slab
	UInt32	hostSrcX, hostSrcY;
	UInt32	maskX, maskY;
	UInt32	screenDstX, screenDstY;

	UInt32	yRemaining;
	UInt32	offRectRowBytes;
	UInt32	linesPerSlab;
	UInt32	maskScaleTable[] = {0xFFFFFFFF, 0x00000000};

	Rect	fromRect, toRect;			// Temporary rects

	yRemaining		= dstRect.bottom - dstRect.top;
	slabWidth		= dstRect.right - dstRect.left;
	offRectRowBytes	= slabWidth * (srcVariant::dstPixelSize) >> 3;
	linesPerSlab	= kQDScratchSize / offRectRowBytes;

	// Set up all of our start locations.
	// How much has the region parser offset us by?
	UInt32 xOffset = dstRect.left - drawVars->minRect.left;
	UInt32 yOffset = dstRect.top - drawVars->minRect.top;

	hostSrcX = drawVars->srcRect.left + xOffset - drawVars->srcPixMap.bounds.left;
	hostSrcY = drawVars->srcRect.top + yOffset - drawVars->srcPixMap.bounds.top;

	maskX = drawVars->mskRect.left + xOffset - drawVars->mskPixMap.bounds.left;
	maskY = drawVars->mskRect.top + yOffset - drawVars->mskPixMap.bounds.top;

	screenDstX = dstRect.left - drawVars->dstPixMap.bounds.left;
	screenDstY = dstRect.top - drawVars->dstPixMap.bounds.top;

	// Do while we have source to move
	while (yRemaining)
	{
		// We do this in 128K source slabs at a time.  That way we don't overload
		// the fifo access, and we are sure that we don't max out the scratchSpace.
		if (yRemaining > linesPerSlab)
			slabHeight = linesPerSlab;
		else
			slabHeight = yRemaining;

		// Calculate the offscreen "to" rect.
		toRect.left		= 0;
		toRect.right	= slabWidth;
		toRect.top		= 0;
		toRect.bottom	= slabHeight;

		// scratch = source
		{
			// Calculate our rects
			fromRect.left	= hostSrcX;
			fromRect.right	= hostSrcX + slabWidth;
			fromRect.top	= hostSrcY;
			fromRect.bottom	= hostSrcY + slabHeight;

			doHostBlit<srcVariant>(
				h3InfoDst,
				(UInt32)drawVars->srcPixMap.baseAddr,
				drawVars->srcPixMap.rowBytes,
				fromRect,
				(UInt32)srcLFB,
				offRectRowBytes,
				toRect,
				srcCopy,
				drawVars->scaleTable,
				drawVars->foreColor,
				drawVars->backColor,
				drawVars->colorizeFlag);
		}

		// scratch ^= dest
		{
			blitter.reg(reg_srcBaseAddr, (FxU32)drawVars->dstPixMap.baseAddr & kBaseAddrOffsetMask);
			blitter.reg(reg_srcFormat, drawVars->dstPixMap.rowBytes | srcVariant::dstPixFmt);
			blitter.reg(reg_srcXY, screenDstX | (screenDstY << 16));

			blitter.reg(reg_dstBaseAddr, (FxU32) srcLFB & kBaseAddrOffsetMask);
			blitter.reg(reg_dstFormat, offRectRowBytes | srcVariant::dstPixFmt);
			blitter.reg(reg_dstXY, 0);

			blitter.reg(reg_dstSize, slabWidth | (slabHeight << 16));

			// Start Blit operation
			rop = kROPSource ^ kROPDest;
			blitter.reg(reg_command, SSTG_BLT | SSTG_GO | rop);
			blitter.go(srcVariant::apertureDepth);
		}

		// "Trim" the offscreen area
		{
			fromRect.left	= maskX;
			fromRect.right	= maskX + slabWidth;
			fromRect.top	= maskY;
			fromRect.bottom	= maskY + slabHeight;

			doHostBlit<mskVariant>(
				h3InfoDst,
				(UInt32)drawVars->mskPixMap.baseAddr,
				drawVars->mskPixMap.rowBytes,
				fromRect,
				(UInt32)srcLFB,
				offRectRowBytes,
				toRect,
				notSrcOr,
				maskScaleTable,
				0,
				0);
		}

		// dest ^= scratch
		{
			blitter.reg(reg_srcBaseAddr, (FxU32) srcLFB & kBaseAddrOffsetMask);
			blitter.reg(reg_srcFormat, offRectRowBytes | srcVariant::dstPixFmt);
			blitter.reg(reg_srcXY, 0);

			blitter.reg(reg_dstBaseAddr, (FxU32)drawVars->dstPixMap.baseAddr & kBaseAddrOffsetMask);
			blitter.reg(reg_dstFormat, drawVars->dstPixMap.rowBytes | srcVariant::dstPixFmt);
			blitter.reg(reg_dstSize, slabWidth | (slabHeight << 16));
			blitter.reg(reg_dstXY, (screenDstX & 0x0000FFFF) | ((screenDstY << 16) & 0xFFFF0000));

			// Start Blit operation
			rop = kROPSource ^ kROPDest;
			blitter.reg(reg_command, SSTG_BLT | SSTG_GO | rop);
			blitter.go(srcVariant::apertureDepth);
		}

		// Update variables and do again
		yRemaining	-= slabHeight;
		maskY		+= slabHeight;
		hostSrcY	+= slabHeight;
		screenDstY	+= slabHeight;
	}
}

#pragma mark -
#pragma mark ¥ hilite mode blit

/********************************************************************************
	PickHiliteBlitVariantLL
		UInt32 srcDepth
		UInt32 dstDepth
		NQDDrawVars *drawVars
		
	This function picks the appropriate template instantiation for the type of blit operation
	being performed.
*/
RegionBlitProc PickHiliteBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars *drawVars)
{
	RegionBlitProc result = nil;
	
#ifndef INSTALL_ADVANCED_2D_ACCEL
	return nil;
#endif

	switch(dstDepth)
	{
		case 8:		
			switch(srcDepth)
			{
				case 1:		result = HiliteBlit<variant1to8>;		break;
				case 8:		
					if(drawVars->bMustScale)
					{
						result = HiliteBlit<variantBitBlt8Scaled>;		
					}
					else
					{
						result = HiliteBlit<variantBitBlt8>;		
					}
				break;
			}
		break;

		case 16:		
			switch(srcDepth)
			{
				case 1:		result = HiliteBlit<variant1to16>;			break;
				case 8:		result = HiliteBlit<variant8to16>;			break;
				case 16:	result = HiliteBlit<variantBitBlt16>;		break;
				case 32:	result = HiliteBlit<variant32to16>;			break;
			}
		break;

		case 32:		
			switch(srcDepth)
			{
				case 1:		result = HiliteBlit<variant1to32>;			break;
				case 8:		result = HiliteBlit<variant8to32>;			break;
				case 16:	result = HiliteBlit<variant16to32>;			break;
				case 32:	result = HiliteBlit<variantBitBlt32>;		break;
			}
		break;
	}

	return(result);
}

template <class variant>
void HiliteBlit(NQDDrawVars *drawVars, Rect &dstRect)
{
	h3Info		*h3InfoDst = (h3Info *)drawVars->refCon;
	Fifo2DRegs	blitter(h3InfoDst);
	FifoSetPattern	setpat(h3InfoDst);

	UInt32	*srcLFB = (UInt32 *)h3InfoDst->scratchSpace->start;

	UInt32	rop;

	UInt32	slabWidth;					// Pixel width of slab
	UInt32	slabHeight;					// Pixel height of slab
	UInt32	hostSrcX, hostSrcY;
	UInt32	screenDstX, screenDstY;

	UInt32	yRemaining;
	UInt32	offRectRowBytes;
	UInt32	linesPerSlab;

	Rect	fromRect, toRect;			// Temporary rects

	yRemaining		= dstRect.bottom - dstRect.top;
	slabWidth		= dstRect.right - dstRect.left;
	offRectRowBytes	= slabWidth * (variant::dstPixelSize) >> 3;
	linesPerSlab	= kQDScratchSize / offRectRowBytes;

	// Set up all of our start locations.
	// How much has the region parser offset us by?
	UInt32 xOffset = dstRect.left - drawVars->minRect.left;
	UInt32 yOffset = dstRect.top - drawVars->minRect.top;

	hostSrcX = drawVars->srcRect.left + xOffset - drawVars->srcPixMap.bounds.left;
	hostSrcY = drawVars->srcRect.top + yOffset - drawVars->srcPixMap.bounds.top;

	screenDstX = dstRect.left - drawVars->dstPixMap.bounds.left;
	screenDstY = dstRect.top - drawVars->dstPixMap.bounds.top;

	// Do while we have source to move
	while (yRemaining)
	{
		// We do this in 128K source slabs at a time.  That way we don't overload
		// the fifo access, and we are sure that we don't max out the scratchSpace.
		if (yRemaining > linesPerSlab)
			slabHeight = linesPerSlab;
		else
			slabHeight = yRemaining;

		// Calculate the offscreen "to" rect.
		toRect.left		= 0;
		toRect.right	= slabWidth;
		toRect.top		= 0;
		toRect.bottom	= slabHeight;

		/* Blit from the screen to the scratch space as follows:
			if(src == hilite color)
				dst gets background color
			else
				dst gets src
		*/
		{
			setpat.fill(variant::dstPixelSize >> 2, drawVars->backColor);

			blitter.reg(reg_srcColorkeyMin, drawVars->hilitColor);
			blitter.reg(reg_srcColorkeyMax, drawVars->hilitColor);
			
			blitter.reg(reg_rop,  (SSTG_ROP_PATCOPY << 16) | (SSTG_ROP_PATCOPY << 8) | SSTG_ROP_SRC);

			blitter.reg(reg_commandEx, SSTG_PAT_FORCE_ROW0 | SSTG_EN_SRC_COLORKEY_EX);

			// set up source and dest
			blitter.reg(reg_srcBaseAddr, (FxU32)drawVars->dstPixMap.baseAddr & kBaseAddrOffsetMask);
			blitter.reg(reg_srcFormat, drawVars->dstPixMap.rowBytes | variant::dstPixFmt);
			blitter.reg(reg_srcXY, screenDstX | (screenDstY << 16));

			blitter.reg(reg_dstBaseAddr, (FxU32) srcLFB & kBaseAddrOffsetMask);
			blitter.reg(reg_dstFormat, offRectRowBytes | variant::dstPixFmt);
			blitter.reg(reg_dstXY, 0);

			blitter.reg(reg_dstSize, slabWidth | (slabHeight << 16));

			// Start Blit operation
			rop = kROPSource;
			blitter.reg(reg_command, SSTG_BLT | SSTG_GO | rop);
			blitter.go(variant::apertureDepth);
		}
		
		if(variant::srcPixelSize == 1)
		{
			// Unfortunately, source color keying doesn't work with 1-bit sources.  
			// Do things a little differently.
			UInt32 fore, back;
			
			fore = back = drawVars->backColor ^ drawVars->hilitColor;
			if(drawVars->backColor != drawVars->scaleTable[0])
			{
				// Don't hilight "0" bits 
				fore = 0;
			}

			if(drawVars->backColor != drawVars->scaleTable[1])
			{
				// Don't hilight "1" bits 
				back = 0;
			}
			
			blitter.reg(reg_dstColorkeyMin, drawVars->backColor);
			blitter.reg(reg_dstColorkeyMax, drawVars->backColor);
			blitter.reg(reg_commandEx, SSTG_EN_DST_COLORKEY_EX);

			blitter.reg(reg_rop,  (SSTG_ROP_SRC << 16) | (SSTG_ROP_ZERO << 8) | SSTG_ROP_SRC);

			blitter.go(variant::apertureDepth);

			// Calculate our rects
			fromRect.left	= hostSrcX;
			fromRect.right	= hostSrcX + slabWidth;
			fromRect.top	= hostSrcY;
			fromRect.bottom	= hostSrcY + slabHeight;

			doHostBlit<variant>(
				h3InfoDst,
				(UInt32)drawVars->srcPixMap.baseAddr,
				drawVars->srcPixMap.rowBytes,
				fromRect,
				(UInt32)srcLFB,
				offRectRowBytes,
				toRect,
				SSTG_ROP_ZERO << SSTG_ROP0_SHIFT,
				drawVars->scaleTable,
				fore,
				back,
				drawVars->colorizeFlag,
				true);
		}
		else
		{
			/* Host blit into the scratch space as follows:
				if((src != background color) && (dst == background color))
					dst gets (drawVars->hilitColor ^ drawVars->backColor)
				else
					dst gets 0
			*/
			blitter.reg(reg_srcColorkeyMin, drawVars->backColor);
			blitter.reg(reg_srcColorkeyMax, drawVars->backColor);
			blitter.reg(reg_dstColorkeyMin, drawVars->backColor);
			blitter.reg(reg_dstColorkeyMax, drawVars->backColor);

			setpat.fill(variant::dstPixelSize >> 2, drawVars->backColor ^ drawVars->hilitColor);

			blitter.reg(reg_rop,  (SSTG_ROP_ZERO << 16) | (SSTG_ROP_ZERO << 8) | SSTG_ROP_PATCOPY);
			blitter.reg(reg_commandEx, SSTG_PAT_FORCE_ROW0 | SSTG_EN_SRC_COLORKEY_EX | SSTG_EN_DST_COLORKEY_EX);
			blitter.go(variant::apertureDepth);

			// Calculate our rects
			fromRect.left	= hostSrcX;
			fromRect.right	= hostSrcX + slabWidth;
			fromRect.top	= hostSrcY;
			fromRect.bottom	= hostSrcY + slabHeight;

			doHostBlit<variant>(
				h3InfoDst,
				(UInt32)drawVars->srcPixMap.baseAddr,
				drawVars->srcPixMap.rowBytes,
				fromRect,
				(UInt32)srcLFB,
				offRectRowBytes,
				toRect,
				SSTG_ROP_ZERO << SSTG_ROP0_SHIFT,
				drawVars->scaleTable,
				drawVars->foreColor,
				drawVars->backColor,
				drawVars->colorizeFlag,
				true);
		}
		
		// XOR the contents of the scratch space back onto the screen.
		{
			blitter.reg(reg_commandEx, 0);

			blitter.reg(reg_srcBaseAddr, (FxU32) srcLFB & kBaseAddrOffsetMask);
			blitter.reg(reg_srcFormat, offRectRowBytes | variant::dstPixFmt);
			blitter.reg(reg_srcXY, 0);

			blitter.reg(reg_dstBaseAddr, (FxU32)drawVars->dstPixMap.baseAddr & kBaseAddrOffsetMask);
			blitter.reg(reg_dstFormat, drawVars->dstPixMap.rowBytes | variant::dstPixFmt);
			blitter.reg(reg_dstSize, slabWidth | (slabHeight << 16));
			blitter.reg(reg_dstXY, (screenDstX & 0x0000FFFF) | ((screenDstY << 16) & 0xFFFF0000));

			// Start Blit operation
			rop = kROPSource ^ kROPDest;
			blitter.reg(reg_command, SSTG_BLT | SSTG_GO | rop);
			blitter.go(variant::apertureDepth);
		}

		// Update variables and do again
		yRemaining	-= slabHeight;
		hostSrcY	+= slabHeight;
		screenDstY	+= slabHeight;
	}
}

#pragma mark -
#pragma mark ¥Dither Copy Support

template <class pixelType>
pixelType	DitherOnePixel(UInt32 src, SInt32 *rowErr, SInt32 *columnErr,
			ColorSpec *colorTable, UInt8 *iTTable);

inline UInt32 InverseTableLookup(UInt8 *iTTable, SInt32 r, SInt32 g, SInt32 b);

template <class variant>
void	DitherCopyBlit(NQDDrawVars *drawVars, Rect &dstRect);

void	BuildPinTable();

/********************************************************************************
	GetAcceleratedDitherBlitProc
		NQDDrawVars *drawVars
		
	This function is called by the Dither code in the StdBits patch. It is designed to 
	act like a NQD accept proc.
*/
Int32  GetAcceleratedDitherBlitProc(NQDDrawVars  *drawVars)
{
	h3Info *h3InfoDst, *h3InfoSrc;
	
	if (!CommonBlitAccept(*drawVars, &h3InfoDst))
		Punt(drawVars);

	h3InfoSrc = FindH3Info(drawVars->srcPixMap.baseAddr);

	// We only handle one depth case.
	if((drawVars->srcPixMap.pixelSize != 32) || ((drawVars->dstPixMap.pixelSize != 8) && (drawVars->dstPixMap.pixelSize != 16)))
		Punt(drawVars);

	// We can't colorize
	if(drawVars->colorizeFlag)
		Punt(drawVars);
	
	// We can't combine.
	if(drawVars->combineMask)
		Punt(drawVars);
	
	// What do you think this is, general case code?
	if(drawVars->mode != ditherCopy)
		Punt(drawVars);
	
	// The 32->16 dither case needs neither color table nor inverse table.
	if(drawVars->dstPixMap.pixelSize == 16)
	{
		if(h3InfoDst != h3InfoSrc) 
		{	
			// This is what we're here for.
			return AcceptThisBlit(*drawVars, DitherCopyBlit<dither32to16>);
		}
		else
			Punt(drawVars);
		
	}
	
	if((drawVars->invTable == NULL) || (drawVars->colorTable == NULL))
	{
		// We need both a color table and an inverse table for this.
		Punt(drawVars);
	}
	
	if(drawVars->invTable->iTabRes != 4)
	{
		// The inverse table has non-standard resolution.
		Punt(drawVars);
	}
	
	if(h3InfoDst != h3InfoSrc) 
	{	
		// This is what we're here for.
		return AcceptThisBlit(*drawVars, DitherCopyBlit<dither32to8>);
	}

	Punt(drawVars);
}

/*----------------------------------------------------------------------------*\
	==> BuildPinTable <==

	This routine builds the table need to pin numbers instead of having
	to do a comparison.  Using a table takes more memory but will significantly
	speed up the work because of no branching.
	
	The table is designed to handle values of generated bu the adding of
	one color entry (0-255) and two error values of (-128 to 128).
	
	So the range is -256 to 512
	
	Note that this code doesn't really need to be optimal, since it will
	only run once.
\*----------------------------------------------------------------------------*/
void BuildPinTable()
{
	SInt32	i;

	static UInt8 pinTableBuffer[768];
	
	gPinTable = &pinTableBuffer[256];

	for (i = -256; i < 0 ; i++)
	{
		gPinTable[i] = 0;
	}

	for (; i < 256; i++)
	{
		gPinTable[i] = i;
	}

	for (; i < 512; i++)
	{
		gPinTable[i] = 255;
	}
}

/*----------------------------------------------------------------------------*\
	==> Unpack32BitUnsigned <==
	This is a fast routine for breaking a four byte color into a bunch of
	signed longs so we can do other math on them.
\*----------------------------------------------------------------------------*/
inline void Unpack32BitUnsigned(UInt32 src, SInt32 &r, SInt32 &g, SInt32 &b)
{
	r = __rlwinm(src, 16, 24, 31);
	g = __rlwinm(src, 24, 24, 31);
	b = __rlwinm(src,  0, 24, 31);
}

/*----------------------------------------------------------------------------*\
	==> InverseTableLookup <==
	Even though we are getting a SInt32 we are expecting the data to all be
	pinned from 0 to 255.
\*----------------------------------------------------------------------------*/
inline UInt32 InverseTableLookup(UInt8 *iTTable, SInt32 r, SInt32 g, SInt32 b)
{
	UInt32 index = g;
	index = __rlwimi(index, b, 28, 28, 31);
	index = __rlwimi(index, r, 4, 20, 23);
	return iTTable[index];
}

inline UInt16 Build16BitPixel(SInt32 r, SInt32 g, SInt32 b)
{
	UInt32 result;
	
	result = __rlwinm(			b,  32-3,	27,	31);
	result = __rlwimi(result,	g,	   2,	22,	26);
	result = __rlwimi(result,	r,	   7,	17,	21);

	return(result);
}

/*----------------------------------------------------------------------------*\
	==> DitherOnePixel <==
	
	Here's how we are doing our dithering.  Roll together the accumulated
	error.  The accumulated error so far on this row, and also accumulated
	from the the pixel above.  Then, find the index in the inverse table using
	the high four bits from each component.  This index is what is returned.
	We then use this index to find the real RGB color in the color table,
	and we use the difference between the two to find an accumulated error.
\*----------------------------------------------------------------------------*/
template <>
inline UInt8 DitherOnePixel<UInt8>(
	UInt32		src,			// The source pixel
	SInt32		*rowErr,		// rowErr, put the new error here
	SInt32		*columnErr,		// columnErr, put the new error here
	ColorSpec	*colorTable,	// 
	UInt8		*iTTable)
{
	UInt32		result;
	SInt32		r, g, b;
	UInt32		hi, lo;
	ColorSpec	*entry;	
	SInt32		tmp;

	Unpack32BitUnsigned(src, r, g, b);

	// All the error components to our base pixel value, and pin to channel limits.
	r = gPinTable[r + rowErr[0] + columnErr[0]];
	g = gPinTable[g + rowErr[1] + columnErr[1]];
	b = gPinTable[b + rowErr[2] + columnErr[2]];

	// Look up the index of this color in the inverse table
	result = InverseTableLookup(iTTable, r, g, b);

	// The rest of this code calculates and saves the error on this pixel.
	
	// Look up the RGB color of the result index
	entry = colorTable + result;
	hi = ((UInt16*)&entry->rgb.red)[0];
	lo = ((UInt32*)&entry->rgb.green)[0];
	
	// The error for each channel is difference of the drawn value and
	// the intended value.  Divide by 2 because the error will propagate
	// in two directions (down and left/right on alternating lines).
	
	tmp = r - __rlwinm(hi, 24, 24, 31);
	tmp /= 2;
	rowErr[0] = tmp;
	columnErr[0] = tmp;

	tmp = g - __rlwinm(lo,  8, 24, 31);
	tmp /= 2;
	rowErr[1] = tmp;
	columnErr[1] = tmp;

	tmp = b - __rlwinm(lo, 24, 24, 31);
	tmp /= 2;
	rowErr[2] = tmp;
	columnErr[2] = tmp;

	return result;
}

template <>
inline UInt16 DitherOnePixel<UInt16>(
	UInt32		src,			// The source pixel
	SInt32		*rowErr,		// rowErr, put the new error here
	SInt32		*columnErr,		// columnErr, put the new error here
	ColorSpec	*/*colorTable*/,// 
	UInt8		*/*iTTable*/)
{
	UInt32		result;
	SInt32		r, g, b;
	SInt32		r1, g1, b1;
	SInt32		tmp;

	Unpack32BitUnsigned(src, r, g, b);

	// All the error components to our base pixel value, and pin to channel limits.
	r = gPinTable[r + rowErr[0] + columnErr[0]];
	g = gPinTable[g + rowErr[1] + columnErr[1]];
	b = gPinTable[b + rowErr[2] + columnErr[2]];

	// Build an actual 16-bit pixel from this color
	result = Build16BitPixel(r, g, b);

	// The rest of this code calculates and saves the error on this pixel.
	
	// Calculate the actual RGB color of drawn pixel (i.e. 32->16 conversion error)
	r1 = r;
	g1 = g;
	b1 = b;
	r1 = __rlwimi(r1, r1, 32-5, 29, 31);
	g1 = __rlwimi(g1, g1, 32-5, 29, 31);
	b1 = __rlwimi(b1, b1, 32-5, 29, 31);
	
	// The error for each channel is difference of the drawn value and
	// the intended value.  Divide by 2 because the error will propagate
	// in two directions (down and left/right on alternating lines).
	
	tmp = r - r1;
	tmp /= 2;
	rowErr[0] = tmp;
	columnErr[0] = tmp;

	tmp = g - g1;
	tmp /= 2;
	rowErr[1] = tmp;
	columnErr[1] = tmp;

	tmp = b - b1;
	tmp /= 2;
	rowErr[2] = tmp;
	columnErr[2] = tmp;

	return result;
}

static SInt32				columnErrCache[2048 * 3];

/*----------------------------------------------------------------------------*\
	==> DitherCopyBlit <==
	
	Walk back and forth row by row.  Convert the RGB to an indexed color using
	the InverseColorTable provided by QD.  We accumulate an error based on
	the last pixel in the row, and the pixel in the row immediately above us.
	We will accumulate these two errors when find the indexed color.  From
	the found color we will determine the error in regards to the original
	pixel and save it for the next pixel in the row, as well as stick it in
	the error accumulator for the next pixel in the column.
\*----------------------------------------------------------------------------*/
template <class variant>
void DitherCopyBlit(NQDDrawVars *drawVars, Rect &dstRect)
{
	// Assume everything has been set up.  Move the data
	h3Info						*h3InfoDst = (h3Info *)drawVars->refCon;
	
	variant::srcPixel			*srcPt;
	ColorSpec					*colorTable = variant::getColorTable(drawVars);
	UInt8						*iTTable = variant::getITTable(drawVars);
	
	// Set up the dimensions of the blit.
	UInt32						width		= dstRect.right - dstRect.left;
	UInt32						height		= dstRect.bottom - dstRect.top;
	UInt32						dstY		= dstRect.top - drawVars->dstPixMap.bounds.top;
	UInt32						dstX		= dstRect.left - drawVars->dstPixMap.bounds.left;
	UInt32						numLongs	= (width + (variant::dstPixelsPerLong - 1)) / variant::dstPixelsPerLong;
	UInt32						rowsLeft	= height;
	UInt32						pixelsLeft;

	// To control the board and move data
	Fifo2DRegs					blitter(h3InfoDst);
	FifoBlitData1				blitdata(h3InfoDst);
	UInt32						*fifoPtr;

	// For keeping track of error
	SInt32				rowAccum[3];
	
	// The widest possible screen we might be drawing on is 2048 pixels.
	
	// The column error cache holds the RGB error for each column.
	SInt32 						*columnErrCachePt	= columnErrCache;
	
	// The row cache holds the pixels generated for each line.
	static variant::dstPixel	rowCache[2048];
	variant::dstPixel 			*rowCachePt			= rowCache;
	
	bool				forwards = true;
	
	if (!gPinTable)
		BuildPinTable();

	// Find the source pointer for our move.
	UInt32	srcY		= dstRect.top - drawVars->dstRect.top + drawVars->srcRect.top - drawVars->srcPixMap.bounds.top;
	UInt32	srcX		= dstRect.left - drawVars->dstRect.left + drawVars->srcRect.left - drawVars->srcPixMap.bounds.left;
//	UInt32	srcX		= drawVars->srcRect.left = (dstRect.left - drawVars->minRect.left);
//	UInt32	srcY 		= drawVars->srcRect.top + (dstRect.top - drawVars->minRect.top);
	UInt32	srcRowBytes	= drawVars->srcPixMap.rowBytes;
	UInt32	*srcBase	= (UInt32*)(drawVars->srcPixMap.baseAddr + (srcY * drawVars->srcPixMap.rowBytes)) + srcX;

	// Set up all the source data pointers and whatnot.
	blitter.reg(reg_dstBaseAddr, (UInt32)drawVars->dstPixMap.baseAddr & kBaseAddrOffsetMask);
	blitter.reg(reg_dstFormat, drawVars->dstPixMap.rowBytes | variant::dstPixFmt);
	blitter.reg(reg_dstSize, width | (height << 16));
	blitter.reg(reg_dstXY, (dstX & 0x0000FFFF) | ((dstY << 16) & 0xFFFF0000));

	blitter.reg(reg_srcFormat, drawVars->dstPixMap.rowBytes | variant::srcPixFmt);
	blitter.reg(reg_srcXY, 0);

	// We only support source copy
	blitter.reg(reg_command, SSTG_HOST_BLT | ModeToROP(variant::dstPixelSize, srcCopy));

	blitter.go(variant::dstPixelSize);

	// Clear out the column cache
	columnErrCachePt = columnErrCache;
	for (UInt32 i = 0; i < (width * 3); ++i)
	{
		columnErrCachePt[i] = 0;
	}
	
	// Push the data into the buffer
	UInt32 maxLines = kMaxFifoBlock / (numLongs * sizeof(FxU32));
	while(rowsLeft)
	{
		UInt32 packetLines = MIN(rowsLeft ,maxLines);		
		fifoPtr = blitdata.start(numLongs * packetLines);		
		rowsLeft -= packetLines;
		
		for(;packetLines; packetLines--)
		{
			// Setup for each row.
			rowAccum[0] = 0;
			rowAccum[1] = 0;
			rowAccum[2] = 0;
			
			if(forwards)
			{
				// Do this row forwards
				srcPt				= srcBase;
				rowCachePt			= rowCache;
				columnErrCachePt	= columnErrCache;

				
				for(pixelsLeft = width; pixelsLeft; --pixelsLeft)
				{
					// Dither the next for pixels in the row
					*rowCachePt = DitherOnePixel<variant::dstPixel>(*srcPt, rowAccum, columnErrCachePt, colorTable, iTTable);

					// Update all the pointers
					++rowCachePt;
					columnErrCachePt += 3;
					++srcPt;
				}
			}
			else
			{
				// Do this row backwards
				srcPt				= srcBase + (width - 1);
				rowCachePt			= rowCache + (width - 1);
				columnErrCachePt	= columnErrCache + ((width - 1) * 3);

				for(pixelsLeft = width; pixelsLeft; --pixelsLeft)
				{
					// Dither the next for pixels in the row
					*rowCachePt = DitherOnePixel<variant::dstPixel>(*srcPt, rowAccum, columnErrCachePt, colorTable, iTTable);
					
					// Update all the pointers
					--rowCachePt;
					columnErrCachePt -= 3;
					--srcPt;
				}
			}
			
			// Do the next line the other way
			forwards = !forwards;

			// Spew the data offscreen
			fifoPtr = storeCacheLines<variantBitBlt::xfer>(
							fifoPtr,
							(UInt32*)rowCache,
							numLongs,
							0);
		
			// Adjust pointers for next row
			srcBase = (UInt32*)(srcRowBytes + (UInt32)srcBase);
			
		}
		blitdata.finish(fifoPtr);
	}
}

#pragma mark -
#pragma mark ¥ Read Blit

/********************************************************************************
	AcceleratedScreenToHostBlitLL
		NQDDrawVars *drawVars
		Rect &dstRect
		
	Accelerates reading data from the Voodoo card and writing the data to host memory
	(or another vendor's card). Note that since PCI reads take longer than writes, our
	acceleration of the read can take precedence over the other card's acceleration of
	the write operation.
*/
template <class variant>
void AcceleratedScreenToHostBlitLL(NQDDrawVars  *drawVars, Rect &dstRect)
{
	const UInt32 pixToBytesShift = variant::srcPixelSize >> 4;
	UInt32 destRowAddr = (UInt32) drawVars->dstPixMap.baseAddr +
			(dstRect.top - drawVars->dstPixMap.bounds.top) * drawVars->dstPixMap.rowBytes +
			((dstRect.left - drawVars->dstPixMap.bounds.left) << pixToBytesShift);
	UInt32 srcRowAddr = (UInt32) drawVars->srcPixMap.baseAddr + 0x02000000 + drawVars->srcPixMap.rowBytes *
			(dstRect.top - drawVars->dstRect.top + drawVars->srcRect.top - drawVars->srcPixMap.bounds.top) +
			((dstRect.left - drawVars->dstRect.left + drawVars->srcRect.left - 
			drawVars->srcPixMap.bounds.left) << pixToBytesShift);
			
	UInt32 destStartBytes = 4 - (destRowAddr & 3);
	
	// Caclulate masks for the left and right edges of the dest. These mask out the partial longs
	// on the left and right sides of the dest rect.
	UInt32 destStartMask = 0xFFFFFFFFU >> ((destRowAddr & 3) << 3);
	UInt32 destEndMask = ~(0xFFFFFFFFU >> ((destRowAddr + ((dstRect.right - dstRect.left) << 
			pixToBytesShift) & 3) << 3));

	// numLongs is the # of longs per line, minus the left and right masked longs.
	SInt32 numLongs = (dstRect.right - dstRect.left) >> (2 - (variant::srcPixelSize >> 4));
	if (!numLongs && (destStartMask & destEndMask))
	{
		destStartMask = destStartMask & destEndMask;
		destEndMask = 0;
	} else if ((destStartMask | destEndMask) == -1)
		--numLongs;
		
	destRowAddr += destStartBytes;
	srcRowAddr += destStartBytes;
	
	// For each line...
	for (SInt32 numLines = dstRect.bottom - dstRect.top; numLines > 0; --numLines)
	{
		UInt32 *destAddr = (UInt32 *) destRowAddr;
		UInt32 *srcAddr = (UInt32 *) srcRowAddr;
		
		destAddr[-1] = (destAddr[-1] & ~destStartMask) | (srcAddr[-1] & destStartMask);
		
		// Handle the bulk of the line a cache line at a time
		for (SInt32 numCacheLines = numLongs >> 3; numCacheLines > 0; --numCacheLines)
		{
			// This loop unrolls completely
			for (UInt32 index = 0; index < 8; ++index)
				*destAddr++ = *srcAddr++;
				
			// These flushes simply remove unmodified entries from the cache, to prevent them
			// from becoming stale data
			__dcbf(srcAddr, -36);
		}
		
		// Handle the rest of the line
		for (SInt32 numExtraLongs = numLongs & 7; numExtraLongs > 0; --numExtraLongs)
		{
			*destAddr++ = *srcAddr++;
		}
		
		// Handle the last partial long
		if (destEndMask)
		{
			*destAddr = (*destAddr & ~destEndMask) | (*srcAddr & destEndMask);
		}
		
		__dcbf(srcAddr, -64);
		__dcbf(srcAddr, -32);
		__dcbf(srcAddr, 0);
		destRowAddr += drawVars->dstPixMap.rowBytes;
		srcRowAddr += drawVars->srcPixMap.rowBytes;
	}
}

/********************************************************************************
	PickReadBlitVariantLL
		UInt32 srcDepth
		UInt32 dstDepth
		NQDDrawVars 
		
	
*/
RegionBlitProc PickReadBlitVariantLL(UInt32 srcDepth, UInt32 dstDepth, NQDDrawVars */*drawVars*/)
{
	RegionBlitProc result = nil;
	
#ifndef VOODOO4
	return nil;
#endif
	
	if (srcDepth != dstDepth)
		return nil;

	if (srcDepth > 16)
		result = AcceleratedScreenToHostBlitLL<variantSrc32>;
	else if (srcDepth == 16)
		result = AcceleratedScreenToHostBlitLL<variantSrc16>;
	else
		result = AcceleratedScreenToHostBlitLL<variantSrc8>;
		
	return result;
}

#pragma mark -
#pragma mark ¥ keep at end (#pragma tricks)

//#pragma always_inline on
//#pragma inline_depth(10)
//#pragma ppc_unroll_instructions_limit 1000
//#pragma ppc_unroll_factor_limit 8
//#pragma ppc_unroll_speculative on

// Defaults for these are:
//#pragma inline_max_size(256)
//#pragma inline_max_total_size(10000)

#pragma inline_max_size(1024)
#pragma inline_max_total_size(100000)


/********************************************************************************
	pushDataToFifo
		h3Info *board
		xfer::srcPtr src
		UInt32 srcRowBytes
		UInt32 hwLongCount
		UInt32 lineCount
		UInt32 *refcon
		
	
*/
template <class xfer>
void pushDataToFifo(
		h3Info *board, 
		xfer::srcPtr src, 
		UInt32 srcRowBytes,
		UInt32 hwLongCount,
		UInt32 lineCount,
		UInt32 *refcon)
{
	FifoBlitData1	blitdata(board);
	UInt32  maxLines, packetLines, transferLines;
	UInt32 newRowBytes = xfer::srcToFifoBytes(srcRowBytes);	
	
	// We can only transfer a packet at a time if the source rowbytes matches up.
	bool doChunks = ((hwLongCount << 2) == xfer::srcToFifoBytes(srcRowBytes));

    /* Calculate the max number of lines to do at once (only use half of FIFO so that
     * we can be writing one chunk while the other one is being read.  This is to deal
     * with the fact we will normally have hole counting off. 
     */
    // MBW -- since the fifo is about 256k in size, I have changed kMaxFifoBlock
    // to just under 64k (the maximum size of this packet type).
    maxLines = kMaxFifoBlock / (hwLongCount * sizeof(FxU32));
	
    // Outer loop sends packet headers.
	for(;lineCount > 0;)
	{
		packetLines = MIN(lineCount ,maxLines);		
		xfer::dstPtr fifoPtr = (xfer::dstPtr)blitdata.start( hwLongCount * packetLines );
		
		// We either transfer a line at a time or a packetful at a time, depending.
		transferLines = (doChunks)?(packetLines):(1);

		// Inner loop sends chunks (either one packet worth or one line worth).
		for(; packetLines > 0; packetLines -= transferLines)
		{
			fifoPtr = storeCacheLines<xfer>(
					fifoPtr, 
					src, 
					hwLongCount * transferLines, 
					refcon);

			src = (xfer::srcPtr)((FxU32)src + (srcRowBytes * transferLines));
			lineCount -= transferLines;
		}
		blitdata.finish((UInt32*)fifoPtr);
	}
}

// To specialize pushDataToFifo, do something like this:
/*
template <>
static void pushDataToFifo<variantBitBlt::xfer>(
		h3Info *board, 
		variantBitBlt::xfer::srcPtr src, 
		UInt32 srcRowBytes,
		UInt32 hwLongCount,
		UInt32 lineCount,
		UInt32 *refcon)
{
	// Optimized special case goes here
}
*/


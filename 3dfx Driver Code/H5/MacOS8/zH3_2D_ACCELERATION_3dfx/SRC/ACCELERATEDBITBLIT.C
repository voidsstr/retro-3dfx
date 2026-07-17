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
** $Header: ACCELERATEDBITBLIT.C, 6, 10/11/00 8:38:31 PM, Brent$
** $Log: 
**  6    3dfx      1.4.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  5    MacOS Dev Tree1.4         04/26/00 Stephane Huaulme resotered host to
**       screen bitblit
** 
**  4    MacOS Dev Tree1.3         04/02/00 Stephane Huaulme more region parsing
**       fixing...
**  3    MacOS Dev Tree1.2         03/25/00 Stephane Huaulme changed region parsing
**       (using same as CP)
**  2    MacOS Dev Tree1.1         02/01/00 Kenneth Dyke    Fix byte swizzling on
**       Voodoo3.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
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

#define FAST_DISPATCH_HACK 1
#define QD_CONFORMANCE_TESTING 0

/* The two most commonly defined macros in the known universe */
#define MIN(__x, __y) (((__x) < (__y)) ? (__x) : (__y))
#define MAX(__x, __y) (((__x) < (__y)) ? (__y) : (__x))

/*
 * NOTE: This define is to force the Alpha blits to be cleared.  This has absolutely no effect 
 *       whatsoever on the visual output, but it does allow us to pass the Quick Dick tests without
 *       failure due to alpha bits being munged.  IMHO, QuickDick should possibly be fixed to ignore
 *       alpha in tests, or at least provide the option of ignoring it.
 */
#if QD_CONFORMANCE_TESTING
#define ROP_MASK  0xF0
#else
#define ROP_MASK  0xFF
#endif

#include <Types.h>
#include <Quickdraw.h>
#include <NQDAcceleration.h>
#include <GraphicsAcceleration.h>
#include <Devices.h>
#include <Displays.h>

#include "h3defs.h"
#include "h3gdefs.h"
#include "minihwc.h"
#include "hwcio.h"
#include <GraphicsPrivHwc.h>
#include "RegionParser.h"
#include "H3Acceleration.h"

/* internal procedure prototypes */

void AcceleratedScreenToScreenBitBlit (NQDDrawVars  *drawVars);
void AcceleratedHostToScreenBitBlit (NQDDrawVars  *drawVars);


FxU32 dead = 0;

Int32  GetAcceleratedRgnBlitProc(NQDDrawVars  *drawVars)
{
	/* Let's just handle simple screen to screen copy operation for now */
	h3Info *h3InfoDst, *h3InfoSrc;
	h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);
	h3InfoSrc = FindH3Info(drawVars->srcPixMap.baseAddr);

    if(h3packetData != h3packetBuffer) {
		dprintf("UM, what the FUCK (RgnBlit)?: %d\n",h3packetData - h3packetBuffer);
	}

	if(dead)
		return false;

  if(!h3InfoDst || h3InfoDst->fifo->exclusiveMode)
	return false;

   switch(drawVars->dstPixMap.pixelSize) {
		case 8: {
		if((drawVars->mode != srcCopy) && (drawVars->mode != notSrcCopy) && 
           (drawVars->mode != transparent))
			return false; 
		}
		break;
		
		case 16: {
		if((drawVars->foreColor != 0x0000) && (drawVars->backColor != 0x7fff7fff))
			return false;
		}
        /* I currently can't deal with source bitmaps that have hosed alignment. */
        if((FxU32)drawVars->srcPixMap.baseAddr & 0x1)
          return false;
		break;
		case 32: {
		if((drawVars->foreColor != 0x0000) && (drawVars->backColor != 0x00ffffff))
			return false;
		}
        /* I currently can't deal with source bitmaps that have hosed alignment. */
        if((FxU32)drawVars->srcPixMap.baseAddr & 0x3) {
          dprintf("bailout due to bad alignment\n");
          return false;
        }
		break;
		default:
			return false;
  }

	/* Make sure we found a board we can accelerate, and that both src and dst
     * pointers are on that board. */
		if(h3InfoDst == h3InfoSrc) {		
			/* Make sure this isn't some whacked case where where we'd have to do */
            /* a screen-to-screen blit that has hosed source coordinates we can't handle. */
		  if(drawVars->colorizeFlag)
			return false;

		  if(drawVars->combineMask)
			return false;

			/* Handle simple cases now */
			if(drawVars->mode <= notSrcBic || drawVars->mode == transparent) {
				if(drawVars->srcPixMap.pixelSize >= 8) {
					drawVars->blitProc = RegionParser;
					drawVars->refCon = (long)h3InfoDst;
					BlitMoveRect = AcceleratedScreenToScreenBlitLL;
					return true;
				}
			}
		} else {

		  if(drawVars->colorizeFlag)
			return false;

		  if(drawVars->combineMask)
			return false;

			/* Handle simple cases now */
			if((drawVars->mode <= notSrcBic || drawVars->mode == transparent)) {
				if(drawVars->srcPixMap.pixelSize >= 8) {
					drawVars->blitProc = RegionParser;
					drawVars->refCon = (long)h3InfoDst;
					BlitMoveRect = AcceleratedHostToScreenBlitLL;
					return true;
				}
			}
		}
	return false;
}

/*

Pat    1 1 1 1 0 0 0 0 

Src    1 1 0 0 1 1 0 0 

Dst    1 0 1 0 1 0 1 0
----------------------
       1 1 0 0 1 1 0 0     srcCopy
       1 0 0 0 1 0 0 0     srcOr
       1 0 0 1 1 0 0 1     srcXor
       1 0 1 1 1 0 1 1     srcBic

       0 0 1 1 0 0 1 1     notSrcCopy
       0 0 1 0 0 0 1 0     notSrcOr
       0 1 1 0 0 1 1 0     notSrcXor
       1 1 1 0 1 1 1 0     notSrcBic

       1 1 1 1 0 0 0 0     patCopy
       1 0 1 0 0 0 0 0     patOr
       1 0 1 0 0 1 0 1     patXor
       1 0 1 0 1 1 1 1     patBic

       0 0 0 0 1 1 1 1     notPatCopy
       0 0 0 0 1 0 1 0     notPatOr
       0 1 0 1 1 0 1 0     notPatXor
       1 1 1 1 1 0 1 0     notPatPic

*/

unsigned long modeToRopBlit[16] = 
{
	((0xCC & ROP_MASK) << SSTG_ROP0_SHIFT),		// srcCopy
	((0x88 & ROP_MASK) << SSTG_ROP0_SHIFT),		// srcOr
	((0x99 & ROP_MASK) << SSTG_ROP0_SHIFT),		// srcXor
	((0xBB & ROP_MASK) << SSTG_ROP0_SHIFT),		// srcBic
	((0x33 & ROP_MASK) << SSTG_ROP0_SHIFT),		// notSrcCopy
	((0x22 & ROP_MASK) << SSTG_ROP0_SHIFT),		// notSrcOr
	((0x66 & ROP_MASK) << SSTG_ROP0_SHIFT),		// notSrcXor
	((0xEE & ROP_MASK) << SSTG_ROP0_SHIFT),		// notSrcBic

	(0xF0          << SSTG_ROP0_SHIFT),		// patCopy
	(0xA0          << SSTG_ROP0_SHIFT),		// patOr
	(0x5A          << SSTG_ROP0_SHIFT),		// patXor
	(0xAF          << SSTG_ROP0_SHIFT),		// patBic

	(0x0F          << SSTG_ROP0_SHIFT),		// notPatCopy
	(0x0A          << SSTG_ROP0_SHIFT),		// notPatOr
	(0xA5          << SSTG_ROP0_SHIFT),		// notPatXor
	(0xFA          << SSTG_ROP0_SHIFT),		// notPatBic
};

unsigned long modeToRopBlit8[16] = 
{
	((0xCC & ROP_MASK) << SSTG_ROP0_SHIFT),		// srcCopy
	((0xEE & ROP_MASK) << SSTG_ROP0_SHIFT),		// srcOr
	((0x66 & ROP_MASK) << SSTG_ROP0_SHIFT),		// srcXor
	((0x22 & ROP_MASK) << SSTG_ROP0_SHIFT),		// srcBic
	((0x33 & ROP_MASK) << SSTG_ROP0_SHIFT),		// notSrcCopy
	((0xBB & ROP_MASK) << SSTG_ROP0_SHIFT),		// notSrcOr
	((0x99 & ROP_MASK) << SSTG_ROP0_SHIFT),		// notSrcXor
	((0x88 & ROP_MASK) << SSTG_ROP0_SHIFT),		// notSrcBic

	(0xF0          << SSTG_ROP0_SHIFT),		// patCopy
	(0xA0          << SSTG_ROP0_SHIFT),		// patOr
	(0x5A          << SSTG_ROP0_SHIFT),		// patXor
	(0xAF          << SSTG_ROP0_SHIFT),		// patBic

	(0x0F          << SSTG_ROP0_SHIFT),		// notPatCopy
	(0x0A          << SSTG_ROP0_SHIFT),		// notPatOr
	(0xA5          << SSTG_ROP0_SHIFT),		// notPatXor
	(0xFA          << SSTG_ROP0_SHIFT),		// notPatBic
};

unsigned long modeToRop[16] = 
{
	(SSTG_ROP_SRC  << SSTG_ROP0_SHIFT),		// srcCopy
	(SSTG_ROP_AND  << SSTG_ROP0_SHIFT),		// srcOr
	(SSTG_ROP_XNOR << SSTG_ROP0_SHIFT),		// srcXor
	(SSTG_ROP_ORI  << SSTG_ROP0_SHIFT),		// srcBic
	(SSTG_ROP_NSRC << SSTG_ROP0_SHIFT),		// notSrcCopy
	(SSTG_ROP_ANDI << SSTG_ROP0_SHIFT),		// notSrcOr
	(SSTG_ROP_XOR  << SSTG_ROP0_SHIFT),		// notSrcXor
	(SSTG_ROP_OR   << SSTG_ROP0_SHIFT),		// notSrcBic

	(0xF0          << SSTG_ROP0_SHIFT),		// patCopy
	(SSTG_ROP_SRC  << SSTG_ROP0_SHIFT),		// patOr
	(0x5A          << SSTG_ROP0_SHIFT),		// patXor
	(SSTG_ROP_SRC  << SSTG_ROP0_SHIFT),		// patBic

	(0x0F          << SSTG_ROP0_SHIFT),		// notPatCopy
	(SSTG_ROP_SRC  << SSTG_ROP0_SHIFT),		// notPatOr
	(0xA5          << SSTG_ROP0_SHIFT),		// notPatXor
	(SSTG_ROP_SRC  << SSTG_ROP0_SHIFT),		// notPatBic
};

// Just use full rect 
void  AcceleratedScreenToScreenBitBlit(NQDDrawVars  *drawVars)
{
	AcceleratedScreenToScreenBlitLL(drawVars, &drawVars->dstRect);
}

void  AcceleratedHostToScreenBitBlit(NQDDrawVars  *drawVars)
{
	AcceleratedHostToScreenBlitLL(drawVars, &drawVars->dstRect);
}

void  AcceleratedScreenToScreenBlitLL(NQDDrawVars  *drawVars, Rect *clipRect)
{
  Rect * srcRect = &drawVars->srcRect;
  Rect * dstRect = &drawVars->dstRect;

  FxU32  height, width, srcX, srcY, dstX, dstY, clipMin, clipMax;
  FxU32  srcRowBytes, dstRowBytes, dstPixFmt, srcPixFmt, rop, commandEx = 0;

  hrmBoard_t *board = ((h3Info *)drawVars->refCon)->board;
  hwcBoardInfo *bInfo = ((h3Info *)drawVars->refCon)->bInfo;
  hrmFifoInfo *fifo = ((h3Info *)drawVars->refCon)->fifo;

  void (*setLfb)(volatile FxU32 *d, FxU32 s);
  unsigned long cmdFlags = 0;


  if(dead)
    return;


#if 0
	dprintf("            s2s src:  %d,%d,%d,%d  dst: %d,%d,%d,%d\n",
		srcRect->left,srcRect->top,srcRect->right,srcRect->bottom,
		drawVars->dstRect.left,drawVars->dstRect.top,drawVars->dstRect.right,drawVars->dstRect.bottom);
	dprintf("after cliping  clip:  %d,%d,%d,%d  dst: %d,%d,%d,%d\n",
		clipRect->left,clipRect->top,clipRect->right,clipRect->bottom,
		dstRect.left,dstRect.top,dstRect.right,dstRect.bottom);
#endif

  	height = dstRect->bottom - dstRect->top;
  	width = dstRect->right - dstRect->left;

	if (drawVars->srcPixMap.rowBytes > 0) {
		srcRowBytes = drawVars->srcPixMap.rowBytes;
		dstRowBytes = drawVars->dstPixMap.rowBytes;
	} else {
		srcRowBytes = -(drawVars->srcPixMap.rowBytes);
		dstRowBytes = -(drawVars->dstPixMap.rowBytes);
	}

    setLfb = fifo->setLfb;
    
	switch(drawVars->dstPixMap.pixelSize) {
		case 8:		dstPixFmt = SSTG_PIXFMT_8BPP;
					rop = modeToRopBlit8[drawVars->mode];
					//setLfb = __swizzleWrite32_8;
					
					#if QD_CONFORMANCE_TESTING
					commandEx = SSTG_PAT_FORCE_ROW0;
					BLITPAT_START(2);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0xffffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0xffffffff);
					#endif
					break;
		case 16:	dstPixFmt = SSTG_PIXFMT_16BPP;
					rop = modeToRopBlit[drawVars->mode];
					//setLfb = __swizzleWrite32_16;
					#if QD_CONFORMANCE_TESTING					
					commandEx = SSTG_PAT_FORCE_ROW0;
					BLITPAT_START(4);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x7fff7fff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x7fff7fff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x7fff7fff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x7fff7fff);
					#endif
					break;
		case 32:	dstPixFmt = SSTG_PIXFMT_32BPP;
					rop = modeToRopBlit[drawVars->mode];
					//setLfb = __swizzleWrite32_32;
					#if QD_CONFORMANCE_TESTING
					commandEx = SSTG_PAT_FORCE_ROW0;
					BLITPAT_START(8);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					#endif
					break;
		default:	dprintf("what the fuck?: %d\n",drawVars->srcPixMap.pixelSize);
					dstPixFmt = 0;
					break;
	}

	switch(drawVars->srcPixMap.pixelSize) {
		case 8:		srcPixFmt = SSTG_PIXFMT_8BPP;
					break;
		case 16:	srcPixFmt = SSTG_PIXFMT_16BPP;
					break;
		case 32:	srcPixFmt = SSTG_PIXFMT_32BPP;
					break;
		default:	dprintf("what the fuck?: %d\n",drawVars->srcPixMap.pixelSize);
					srcPixFmt = 0;
					break;
	}

	srcX = srcRect->left - drawVars->srcPixMap.bounds.left;
	srcY = srcRect->top - drawVars->srcPixMap.bounds.top;

	dstX = dstRect->left - drawVars->dstPixMap.bounds.left;
	dstY = dstRect->top - drawVars->dstPixMap.bounds.top;

	clipMin = ((clipRect->left - drawVars->dstPixMap.bounds.left)  & 0x0000FFFF)
	              | ( ((clipRect->top - drawVars->dstPixMap.bounds.top) << 16) & 0xFFFF0000);
	clipMax = ((clipRect->right - drawVars->dstPixMap.bounds.left) & 0x0000FFFF) 
	              |( ((clipRect->bottom - drawVars->dstPixMap.bounds.top) << 16) & 0xFFFF0000) ;

	if (drawVars->srcPixMap.rowBytes < 0) {
		/* Bottom to top */
		srcY += height - 1;
		dstY += height - 1;
		cmdFlags |= SSTG_YDIR;
	}
	
	if(drawVars->hBump < 0) {
		/* Right to left */
		srcX += width - 1;
		dstX += width - 1;
		cmdFlags |= SSTG_XDIR;
	}

	/* NB: The order of the data is *critical* and must match the hardware register ordering */
	if(drawVars->mode == transparent) {
		rop = (SSTG_ROP_SRC << SSTG_ROP0_SHIFT);
		REG2D_BEGIN(3, MASK_srcColorkeyMin | MASK_srcColorkeyMax | MASK_rop);
		REG2D_DATA(srcColorkeyMin, drawVars->backColor);
		REG2D_DATA(srcColorkeyMax, drawVars->backColor);
		REG2D_DATA(rop, (SSTG_ROP_SRC << 0) | (SSTG_ROP_DST << 8) | (SSTG_ROP_DST << 16));
		REG2D_END;
		commandEx |= SSTG_EN_SRC_COLORKEY_EX;
	}

	/* NB: The order of the data is *critical* and must match the hardware register ordering */
	REG2D_BEGIN(11, MASK_clip0min | MASK_clip0max | MASK_dstBaseAddr | MASK_dstFormat |
					MASK_srcBaseAddr | MASK_commandEx | MASK_srcFormat | 
                    MASK_srcXY | MASK_dstSize | MASK_dstXY | MASK_command);
	REG2D_DATA(clip0min, clipMin);
	REG2D_DATA(clip0max, clipMax);
	REG2D_DATA(dstBaseAddr, ((FxU32) drawVars->dstPixMap.baseAddr - bInfo->regInfo.rawLfbBase) & SSTG_BASEADDR);
	REG2D_DATA(dstFormat, dstRowBytes | dstPixFmt);
	REG2D_DATA(srcBaseAddr, ((FxU32) drawVars->srcPixMap.baseAddr - bInfo->regInfo.rawLfbBase) & SSTG_BASEADDR);
	REG2D_DATA(commandEx, commandEx);
	
	REG2D_DATA(srcFormat, srcRowBytes | srcPixFmt);
	REG2D_DATA(srcXY, srcX | (srcY << 16));
	REG2D_DATA(dstSize, width | (height << 16));
	REG2D_DATA(dstXY, dstX | (dstY << 16));

	/* Start Blit operation */
	REG2D_DATA(command, SSTG_BLT | SSTG_GO | rop | cmdFlags);
	REG2D_END;

    BUMP_N_GRIND;
}

void  AcceleratedHostToScreenBlitLL(NQDDrawVars  *drawVars, Rect *clipRect)
{
  Rect * srcRect = &drawVars->srcRect;
  Rect * dstRect = &drawVars->dstRect;

  FxU32  height, width, srcX, srcY, dstX, dstY, clipMin, clipMax, srcLongs, stride;
  FxU32  maxLines, hCount, vIndex, transferLines, hIndex, *src, *fifoPtr, *dataPtr;

  FxU32  srcRowBytes, dstRowBytes, srcFmt, dstFmt, srcDataStart, srcDataEnd, rop;
  FxU32  transferIndex, commandEx = 0, srcXmask, srcXhw;

  hrmBoard_t *board = ((h3Info *)drawVars->refCon)->board;
  hwcBoardInfo *bInfo = ((h3Info *)drawVars->refCon)->bInfo;
  hrmFifoInfo *fifo = ((h3Info *)drawVars->refCon)->fifo;

  void (*setLfb)(volatile FxU32 *d, FxU32 s);
  unsigned long cmdFlags = 0;

  	height = dstRect->bottom - dstRect->top;
  	width = dstRect->right - dstRect->left;

	//dprintf("dst: %d,%d,%d,%d\n",
	//	dstRect->left,dstRect->top,dstRect->right,dstRect->bottom);

	if(dead)
		return;

	if (drawVars->srcPixMap.rowBytes > 0) {
		srcRowBytes = drawVars->srcPixMap.rowBytes;
		dstRowBytes = drawVars->dstPixMap.rowBytes;
	} else {
		srcRowBytes = -(drawVars->srcPixMap.rowBytes);
		dstRowBytes = -(drawVars->dstPixMap.rowBytes);
	}

    setLfb = fifo->setLfb;

	switch(drawVars->dstPixMap.pixelSize) {
		case 8:		srcFmt = SSTG_PIXFMT_8BPP;
		            srcXmask = 0xffff;
					dstFmt = SSTG_PIXFMT_8BPP;
					rop = modeToRopBlit8[drawVars->mode];
//					setLfb = __swizzleWrite32_8;
					
					#if QD_CONFORMANCE_TESTING
					commandEx = SSTG_PAT_FORCE_ROW0;
					BLITPAT_START(2);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0xffffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0xffffffff);
					#endif
					break;
		case 16:	srcFmt = SSTG_PIXFMT_16BPP | SSTG_HOST_BYTE_SWIZZLE | SSTG_HOST_WORD_SWIZZLE;
					dstFmt = SSTG_PIXFMT_16BPP;
					rop = modeToRopBlit[drawVars->mode];
//					setLfb = __swizzleWrite32_16;
					srcXmask = 0xfffe;
					
					#if QD_CONFORMANCE_TESTING
					commandEx = SSTG_PAT_FORCE_ROW0;
					BLITPAT_START(4);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x7fff7fff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x7fff7fff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x7fff7fff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x7fff7fff);
					#endif
					break;
		case 32:	srcFmt = SSTG_PIXFMT_32BPP | SSTG_HOST_BYTE_SWIZZLE;
					dstFmt = SSTG_PIXFMT_32BPP;
					rop = modeToRopBlit[drawVars->mode];
//					setLfb = __swizzleWrite32_32;
					srcXmask = 0xfffc;
					
					#if QD_CONFORMANCE_TESTING
					commandEx = SSTG_PAT_FORCE_ROW0;
					BLITPAT_START(8);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					SET_FIFO_NOSWAP(*fifo->fifoPtr++,0x00ffffff);
					#endif
					break;
		default:	dprintf("what the fuck?: %d\n",drawVars->srcPixMap.pixelSize);
					srcFmt = 0;
					dstFmt = 0;
					break;
	}

	if(drawVars->dstPixMap.pixelSize == drawVars->srcPixMap.pixelSize) {
		transferIndex = HWCEXT_TRANSFER_NORMAL;
	} else if(drawVars->srcPixMap.pixelSize == 16 && drawVars->dstPixMap.pixelSize == 32) {
		dprintf("16TO32\n");
        transferIndex = HWCEXT_TRANSFER_16TO32;
	} else if(drawVars->srcPixMap.pixelSize == 32 && drawVars->dstPixMap.pixelSize == 16) {
        transferIndex = HWCEXT_TRANSFER_32TO16;
		dprintf("32TO16\n");
	} else if(drawVars->srcPixMap.pixelSize == 8  && drawVars->dstPixMap.pixelSize == 32) {
	      transferIndex = HWCEXT_TRANSFER_8TO32;
	  dprintf("8TO24\n");
	} else if(drawVars->srcPixMap.pixelSize == 8  && drawVars->dstPixMap.pixelSize == 16) {
	      transferIndex = HWCEXT_TRANSFER_8TO16;
	  dprintf("8TO16\n");
	} else {
		dprintf("OOPS! No supported transfer mode!\n");
		return;
	}

    /* On Banshee/Voodoo3, don't use host port byte swizzling */
    if(bInfo->pciInfo.deviceID <= 5) {
      srcFmt &= ~(SSTG_HOST_BYTE_SWIZZLE | SSTG_HOST_WORD_SWIZZLE);
    }
    
	srcX = srcRect->left - drawVars->srcPixMap.bounds.left;
	srcY = srcRect->top - drawVars->srcPixMap.bounds.top;
	dstX = dstRect->left - drawVars->dstPixMap.bounds.left;
	dstY = dstRect->top - drawVars->dstPixMap.bounds.top;

	clipMin = ((clipRect->left - drawVars->dstPixMap.bounds.left)  & 0x0000FFFF)
	              | ( ((clipRect->top - drawVars->dstPixMap.bounds.top) << 16) & 0xFFFF0000);
	clipMax = ((clipRect->right - drawVars->dstPixMap.bounds.left) & 0x0000FFFF) 
	              |( ((clipRect->bottom - drawVars->dstPixMap.bounds.top) << 16) & 0xFFFF0000) ;

	/* Find first & last longword addresses for first scan line. */
	srcDataStart = (FxU32) drawVars->srcPixMap.baseAddr + 
			  		(srcY * srcRowBytes) + srcX * (drawVars->srcPixMap.pixelSize >> 3);
	srcDataEnd = srcDataStart + ((width - 1) * (drawVars->srcPixMap.pixelSize >> 3));

	/* Truncate to longword addresses & compute initial alignment. */
	  srcX = srcDataStart & 0x3;
	
	/* Whoops.  The above was only correct until I started doing scaled blits too. */
	/* Now I need to know both.. the "original" srcX and the one I actually give to */
	/* the hardware. */
	srcXhw = srcX & srcXmask;
	
    /* Truncate addresses to longword alignment */ 
	srcDataStart &= ~3;
	srcDataEnd   &= ~3;

	/* Recalculate src row longs. */
	srcLongs = (srcDataEnd - srcDataStart + 1) >> 2;

	if(drawVars->mode == transparent) {
		rop = (SSTG_ROP_SRC << SSTG_ROP0_SHIFT);
		REG2D_BEGIN(3, MASK_srcColorkeyMin | MASK_srcColorkeyMax | MASK_rop);
		REG2D_DATA(srcColorkeyMin, drawVars->backColor);
		REG2D_DATA(srcColorkeyMax, drawVars->backColor);
		REG2D_DATA(rop, (SSTG_ROP_SRC << 0) | (SSTG_ROP_DST << 8) | (SSTG_ROP_DST << 16));
		REG2D_END;
		commandEx |= SSTG_EN_SRC_COLORKEY_EX;
	}

	REG2D_BEGIN(2, MASK_colorBack | MASK_colorFore);
	REG2D_DATA(colorBack, drawVars->backColor);
	REG2D_DATA(colorFore, drawVars->foreColor);
	REG2D_END;
	
	REG2D_BEGIN(10, MASK_clip0min | MASK_clip0max | MASK_dstBaseAddr | MASK_dstFormat |
					MASK_commandEx | MASK_srcFormat | 
                    MASK_srcXY | MASK_dstSize | MASK_dstXY | MASK_command);
	REG2D_DATA(clip0min,clipMin);
	REG2D_DATA(clip0max,clipMax);
	REG2D_DATA(dstBaseAddr, ((FxU32) drawVars->dstPixMap.baseAddr - bInfo->regInfo.rawLfbBase) & SSTG_BASEADDR);
	REG2D_DATA(dstFormat, dstRowBytes | dstFmt);
	REG2D_DATA(commandEx, commandEx);
	REG2D_DATA(srcFormat, (srcLongs << 2) | srcFmt);
	REG2D_DATA(srcXY, srcXhw | (srcY << 16));
	REG2D_DATA(dstSize, width | (height << 16));
	REG2D_DATA(dstXY, dstX | (dstY << 16));
	REG2D_DATA(command, SSTG_HOST_BLT | rop | cmdFlags);
	REG2D_END;
	BUMP_N_GRIND;

#if 0
	paramBlock.ioCRefNum = (short)bInfo->hdc;

	paramBlock.csCode = k3DfxExecuteBlitPacket; /* driver-specific status request */
	*(hwcControl_t **)(&paramBlock.csParam[0]) = &control;

	// Have 2D driver do the actual data transfer via the command fifo.
	control.req.optData.executeBlitReq.dataPtr = (FxU32)srcDataStart;
	control.req.optData.executeBlitReq.stride = srcRowBytes;
	control.req.optData.executeBlitReq.hCount = ((FxU32)srcDataEnd - (FxU32)srcDataStart + 4) >> 2;
	control.req.optData.executeBlitReq.vCount = height;
	control.req.optData.executeBlitReq.srcOffset = srcX;
	control.req.optData.executeBlitReq.width = width;
	control.req.optData.executeBlitReq.transferIndex = transferIndex;
    control.req.optData.executeBlitReq.cmdData = NULL; //h3packetBuffer;
    control.req.optData.executeBlitReq.cmdSize = 0; //h3packetData - h3packetBuffer

    h3packetData = h3packetBuffer;

  if ((width <= FIXW) FIXOP (height <= FIXH ))
  {
        WAXBUG_3DNOPFIX;
  }

	myErr = PBControl((ParmBlkPtr)&paramBlock, false);
	if(myErr != 0) {
		// Disable acceleration since hardware is probably wedged.
		//dead = 1;
		//UninstallAcceleration();
		//return;
	}
#else
	hCount = ((FxU32)srcDataEnd - (FxU32)srcDataStart + 4) >> 2;
	stride = srcRowBytes;
	dataPtr = (FxU32 *)srcDataStart;
	vIndex = 0;

  	switch(transferIndex) {
  		case HWCEXT_TRANSFER_NORMAL: 

        /* Calculate the max number of lines to do at once (only use half of FIFO so that
         * we can be writing one chunk while the other one is being read.  This is to deal
         * with the fact we will normally have hole counting off. */
	    maxLines = (16384 - 64) / (hCount * sizeof(FxU32));
	  
	    // Outer loop sends packet headers.
	    while(vIndex < height) {
	      transferLines = MIN((height - vIndex),maxLines);
	    
	      MAKE_ROOM( hCount * transferLines + 1 );

		  SET_FIFO_LFB(*fifo->fifoPtr++, (SSTCP_PKT1_2D | SSTCP_PKT1 | 
													(0x20UL << SSTCP_REGBASE_SHIFT) |
		 											((transferLines * hCount) << SSTCP_PKT1_NWORDS_SHIFT)));
		  fifoPtr = fifo->fifoPtr;
		  while(transferLines-- != 0) {
		    src = dataPtr;
		    hIndex = hCount;
		 	
		    while(hIndex-- != 0) {
#if PCI_COPYBACK	  
		  	  if(((FxU32)fifoPtr & 31) == 0) {
	            __dcbst(fifoPtr,-4);
		  		    __dcbz(fifoPtr,0);
		  	  }	  	
#endif	  	
		      *fifoPtr++ = *src++;
	        }
		    dataPtr = (FxU32 *)((FxU32)dataPtr + stride);
		    vIndex++;
		  }
		  fifo->fifoPtr = fifoPtr;
		  BUMP_N_GRIND;
	    }
	    break;
	    
	    case HWCEXT_TRANSFER_16TO32:

	    /* This case is easy since the hardware gets exactly one 32-bit word per pixel. */
	    maxLines = (16384 - 64) / (width * sizeof(FxU32));
	    
	    while(vIndex < height) {
	      transferLines = MIN((height - vIndex),maxLines);
	      
	      MAKE_ROOM(width * transferLines + 1);
	      
		  SET_FIFO_LFB(*fifo->fifoPtr++, (SSTCP_PKT1_2D | SSTCP_PKT1 | 
													(0x20UL << SSTCP_REGBASE_SHIFT) |
		 											((transferLines * width) << SSTCP_PKT1_NWORDS_SHIFT)));

		  fifoPtr = fifo->fifoPtr;
		  while(transferLines-- != 0) {
		    /* We have to manually skip to the right starting pixel */
		  	FxU16 *src16 = (FxU16 *)((FxU32)dataPtr + srcX);
		  	
		    hIndex = width;
		 	
		    while(hIndex-- != 0) {
#if PCI_COPYBACK	  
		  	  if(((FxU32)fifoPtr & 31) == 0) {
	            __dcbst(fifoPtr,-4);
		  		    __dcbz(fifoPtr,0);
		  	  }	  	
#endif	  	
			  {
			    FxU16 srcPixel = *src16++;
			    FxU32 temp5, temp3;
			  	/* Attempt to preload next cache line. */
				  __dcbt(src16,0);
				
			    /* Temp1 collects the 5 MSB's from each color, Temp2 collects the 3 MSB's */
			    temp5 = __rlwinm(srcPixel,17-8,8,12);
			    temp3 = __rlwinm(srcPixel,17-13,13,15);
			    __rlwimi(temp5,srcPixel,22-16,16,20);
			    __rlwimi(temp3,srcPixel,22-21,21,23);
			    __rlwimi(temp5,srcPixel,27-24,24,28);
			    __rlwimi(temp3,srcPixel,30,29,31);
			    temp5 |= temp3;
			    
		        *fifoPtr++ = temp5;
		      }
	        }
		    dataPtr = (FxU32 *)((FxU32)dataPtr + stride);
		    vIndex++;
		  }
		  fifo->fifoPtr = fifoPtr;
		  BUMP_N_GRIND;
	    }
	    break;
	      
	    case HWCEXT_TRANSFER_32TO16:
	    dprintf("TRANSFER_32TO16\n");
	    
	    /* In this case the hardware is expecting to get 32-bit words with two embedded 16-bit pixels */
	    hCount = (hCount +1) >> 1;
	    
	    maxLines = (16384 - 64) / (hCount * sizeof(FxU32));
	    
	    while(vIndex < height) {
	      transferLines = MIN((height - vIndex), maxLines);
	    	
	      MAKE_ROOM(hCount * transferLines + 1);
	    	
		  SET_FIFO_LFB(*fifo->fifoPtr++, (SSTCP_PKT1_2D | SSTCP_PKT1 | 
										(0x20UL << SSTCP_REGBASE_SHIFT) |
		 								((transferLines * hCount) << SSTCP_PKT1_NWORDS_SHIFT)));
	    	
	      fifoPtr = fifo->fifoPtr;
	      while(transferLines-- != 0) {
	      	FxU32 count;
	      	src = dataPtr;
	      		
	      	count = hCount;
	      	while(count-- != 0) {
#if PCI_COPYBACK	  
		  	  if(((FxU32)fifoPtr & 31) == 0) {
	            __dcbst(fifoPtr,-4);
		  		    __dcbz(fifoPtr,0);
		  	  }	  	
#endif	  	
			  {
			  	FxU32 temp32_1, temp32_2, temp16_1, temp16_2;
			  	temp32_1 = *src++;
			  	temp32_2 = *src++;
			  	
			  	/* Attempt to preload next cache line. */
			  	__dcbt(src,0);
			  		
			  	/* Extract RGB values from each pixel */
			    temp16_1 = __rlwinm(temp32_1,8-1,1,5);
			    temp16_2 = __rlwinm(temp32_2,32+8-17,17,21);
			    __rlwimi(temp16_1,temp32_1,16-6,6,10);
			    __rlwimi(temp16_2,temp32_2,32+16-22,22,26);
			    __rlwimi(temp16_1,temp32_1,24-11,11,15);
			    __rlwimi(temp16_2,temp32_2,32+24-27,27,31);
			    *fifoPtr++ = temp16_1 | temp16_2;
			  }
			}
		  
		    dataPtr = (FxU32 *)((FxU32)dataPtr + stride);
		    vIndex++;
		  }
		  fifo->fifoPtr = fifoPtr;
		  BUMP_N_GRIND;
		}
		break;

	    case HWCEXT_TRANSFER_8TO32:
	    //dprintf("TRANSFER_8TO32\n");
	    //dprintf("scaleTableIsRGB: %d\n",drawVars->scaleTableIsRGBTable);

	    /* This case is easy since the hardware gets exactly one 32-bit word per pixel. */
	    maxLines = (16384 - 64) / (width * sizeof(FxU32));
	    
	    //while(!Button());
	    
	    while(vIndex < height) {
	      transferLines = MIN((height - vIndex),maxLines);
	      
	      MAKE_ROOM(width * transferLines + 1);
	      
		  SET_FIFO_LFB(*fifo->fifoPtr++, (SSTCP_PKT1_2D | SSTCP_PKT1 | 
													(0x20UL << SSTCP_REGBASE_SHIFT) |
		 											((transferLines * width) << SSTCP_PKT1_NWORDS_SHIFT)));

		  fifoPtr = fifo->fifoPtr;
		  while(transferLines-- != 0) {
		    /* We have to manually skip to the right starting pixel */
		  	FxU8 *src8 = (FxU8 *)((FxU32)dataPtr + srcX);
		  	
		    hIndex = width;
		 	
		    while(hIndex-- != 0) {
#if PCI_COPYBACK	  
		  	  if(((FxU32)fifoPtr & 31) == 0) {
	            __dcbst(fifoPtr,-4);
		  		    __dcbz(fifoPtr,0);
		  	  }	  	
#endif	  	
			    {
			      FxU8 srcPixel = *src8++;
			      FxU32 temp;
			  	  /* Attempt to preload next cache line. */
				    __dcbt(src8,0);
			
				    temp = drawVars->scaleTable[srcPixel];
				    //temp = 0x00ffff00;
				  
		        *fifoPtr++ = temp;
		      }
	      }
		    dataPtr = (FxU32 *)((FxU32)dataPtr + stride);
		    vIndex++;
		  }
		  fifo->fifoPtr = fifoPtr;
		  BUMP_N_GRIND;
		  
		  //while(!Button());
	    }
	    break;

	    case HWCEXT_TRANSFER_8TO16:

	    /* This case is trickier to deal with than the others, so
	    /* I just recalculate things. */
	    
	    /* Regenerate original byte offsets */
		srcDataStart = ((FxU32)srcDataStart + srcX);
		srcDataEnd = srcDataStart + ((width - 1) * (drawVars->srcPixMap.pixelSize >> 3));
		
		/* FIXME -- Recalc initial alignment, since the calculation up above gets it wrong. */
		srcXhw = (srcDataStart & 1) * 2;
	    REG2D_BEGIN(10, MASK_srcXY);
	    REG2D_DATA(srcXY, srcXhw | (srcY << 16));
	    REG2D_END;
	    BUMP_N_GRIND;
		
		/* Truncate addresses to halfword alignment */		
		srcDataStart &= ~1;
		srcDataEnd   &= ~1;
	    
	    /* Count the number of aligned halfwords we will transfer (from the source). */
	    /* This is the same as the number of longwords we'll transfer to the HW. */
        hCount = ((FxU32)srcDataEnd - (FxU32)srcDataStart + 2) >> 1;
	    dataPtr = (FxU32 *)srcDataStart;
	    
	    maxLines = (16384 - 64) / (hCount * sizeof(FxU32));
	    
	    while(vIndex < height) {
	      transferLines = MIN((height - vIndex), maxLines);
	    	
	      MAKE_ROOM(hCount * transferLines + 1);
	    	
		  SET_FIFO_LFB(*fifo->fifoPtr++, (SSTCP_PKT1_2D | SSTCP_PKT1 | 
										(0x20UL << SSTCP_REGBASE_SHIFT) |
		 								((transferLines * hCount) << SSTCP_PKT1_NWORDS_SHIFT)));
	    	
	      fifoPtr = fifo->fifoPtr;
	      while(transferLines-- != 0) {
	      	FxU32 count;
			FxU8 *src8 = (FxU8 *)dataPtr;
	      		
	      	count = hCount;
	      	while(count-- != 0) {
#if PCI_COPYBACK	  
		  	  if(((FxU32)fifoPtr & 31) == 0) {
	            __dcbst(fifoPtr,-4);
		  		    __dcbz(fifoPtr,0);
		  	  }	  	
#endif	  	
          {
			  	  FxU32 temp16_1, temp16_2;
			  	
			  	  /* QuickDraw has a nice table built for us already... */
			  	  temp16_1 = drawVars->scaleTable[*src8++] << 16;// & 0xffff0000;
			  	  temp16_2 = drawVars->scaleTable[*src8++];// & 0x0000ffff;
			  	
			  	  /* Attempt to preload next cache line. */
			  	  __dcbt(src8,0);

			      *fifoPtr++ = temp16_1 | temp16_2;
			    }
			  }
		  
		    dataPtr = (FxU32 *)((FxU32)dataPtr + stride);
		    vIndex++;
		  }
		  fifo->fifoPtr = fifoPtr;
		  BUMP_N_GRIND;
		}
		break;

		default:
		dprintf("SHIT!!!!!\n");
		break;
    }
#endif
	//dprintf("res: %d\n",myErr);
	//dprintf(".");
	
#if 0
	preAlign = postAlign = 0;
	if((FxU32)srcDataStart & 0x4) preAlign = 1;
	if(!((FxU32)srcDataEnd & 0x4)) postAlign = 1;
	transfer = ((FxU32)srcDataEnd - (FxU32)srcDataStart + 4) >> 2;  // Total longwords to send
	transfer -= (preAlign + postAlign);
	transfer >>= 1; // convert to doublewords.

	for(y = 0; y < height; y++) {
		FxU32 *src4, dummy;
		SstGRegs *gregs = (SstGRegs *)bInfo->regInfo.waxBase;
		double *src8;
		src4 = (FxU32 *)srcDataStart;
		count = transfer;	
		if(preAlign) {
			gregs->launch[0] = *src4++;
		}
		src8 = (double *)src4;
		while(count > 0) {
			*(double *)&(gregs->launch[0]) = *src8++;
			count--;
		}
		if(postAlign) {
			src4 = (FxU32 *)src8;
			gregs->launch[0] = *src4++;
		}
		srcDataStart += srcRowBytes;
	}
#endif
  if ((width <= FIXW) FIXOP (height <= FIXH ))
  {
        WAXBUG_3DNOPFIX;
		BUMP_N_GRIND;
  }
#if 0
	// Begin copying data from source pixmap to launch, one longword at a time.
	for(y = 0; y < height; y++) {
		FxU32 *src = (FxU32 *)srcDataStart;
		while((FxU32)src <= srcDataEnd) {
			words++;
			src++;
			//HWC_WAX_STORE_NOSWAP(bInfo->regInfo, launch[0], *src++);
		}
		srcDataStart += srcRowBytes;
		srcDataEnd += srcRowBytes;
	}
	dprintf("desired words: %d  actual: %d  bump: %d\n",words,
		control.req.optData.executeBlitReq.hCount,
		control.req.optData.executeBlitReq.vCount);
#endif
#if 0
	/* wait for command to complete */
	{
		FxU32 status;
		count = 0;

		while(FXTRUE)
		{
			status = HWC_WAX_LOAD(bInfo->regInfo, status, status);
			__eieio();
			if(!(status & (SST_GUI_BUSY|SST_CMD0_BUSY)))
				break;
			count++;
			// Hung?
			if(count > 1000) {
				dprintf("SHIT!\n");
				break;
			}
		}
	}
#endif  	
}

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
** $Header: ACCELERATEDSLABBLIT.C, 3, 10/11/00 8:38:36 PM, Brent$
** $Log: 
**  3    3dfx      1.1.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    MacOS Dev Tree1.1         03/25/00 Stephane Huaulme changed region parsing
**       (using same as CP)
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 3     7/02/99 3:33p Kcd
** New headers & HRM integration.
**
*/

#define FAST_DISPATCH_HACK 1
#define DISPATCH_TAG SLABBLIT

#define DISPATCH_SLAB_BLIT 0

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
#include "H3Acceleration.h"

/* internal procedure prototypes */

#if DISPATCH_SLAB_BLIT
static void AcceleratedSlabBlit_8(NQDDrawVars  *drawVars);
static void AcceleratedSlabBlit_16(NQDDrawVars  *drawVars);
static void AcceleratedSlabBlit_32(NQDDrawVars  *drawVars);
#else
static void  AcceleratedSlabBlit (NQDDrawVars  *drawVars);
#endif

extern unsigned long modeToRop[16];
extern void RgnBlitMoveForward(NQDDrawVars	*inDrawVars);

extern FxU32 dead;
extern FxU32 currentPatBlitSerialNum;
extern FxU32 lastPatBlitSerialNum; 

Int32  GetAcceleratedSlabBlitProc (NQDDrawVars  *drawVars)
{
  PixMapPtr  pMap;
  unsigned short  patHeight, patWidth;
	h3Info *h3InfoDst;
	h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);

    if(h3packetData != h3packetBuffer) {
		dprintf("UM, what the FUCK (SlabBlit)?: %d\n",h3packetData - h3packetBuffer);
	}

	if(drawVars->trimResult > 0)
		return false;

  if(!h3InfoDst || h3InfoDst->fifo->exclusiveMode)
	return false;
 
  if(drawVars->mode >= patCopy && 
     drawVars->mode <= notPatBic &&
     (drawVars->mode & 1))
  	return false;

  if(drawVars->colorizeFlag)
	return false;

  if ((drawVars->dstPixMap.pixelSize < 8) || (drawVars->mode >= 16))
    return (false);

  if (drawVars->patSolid) {
#if DISPATCH_SLAB_BLIT
		switch(drawVars->dstPixMap.pixelSize) {
	    case 8:		drawVars->blitProc = AcceleratedSlabBlit_8;
	              break;
	    case 16:	drawVars->blitProc = AcceleratedSlabBlit_16;
	              break;
	    case 32:	drawVars->blitProc = AcceleratedSlabBlit_32;
	              break;
		  default:  return false;
						    break;
		}
#else		
		drawVars->blitProc = AcceleratedSlabBlit;
#endif		
		drawVars->refCon = (long)h3InfoDst;
    currentPatBlitSerialNum++;
    return (true);
  }
	
  if (drawVars->patHdl == nil) {
    return (false);
  }
  
  if (!drawVars->patSolid) {
    /* get pix map of pattern */
    pMap = *((*(drawVars->patHdl))->patMap);

    /* set pattern size */
    patHeight = pMap->bounds.bottom - pMap->bounds.top;
    patWidth = pMap->bounds.right - pMap->bounds.left;

	/* For now, let's assume quickdraw will only give us 8x8 patterns at the smallest */
	/* If this turns out not to be the case, then I'll write some code to expand the  */
	/* Data to 8x8 myself since that's the smallest pattern our hardware can deal with. */
	if(patHeight != 8 || patWidth != 8) {
		if(patHeight < 8 || patWidth < 8) {
			dprintf("Tiny pattern: %d x %d\n",patWidth,patHeight);
		}
		return false;
    }
  }

  pMap = *((*(drawVars->patHdl))->patMap);

  if (drawVars->patRowBytes ==
        ((pMap->bounds.right - pMap->bounds.left) * (drawVars->dstPixMap.pixelSize >> 3)))
  {
#if DISPATCH_SLAB_BLIT
		switch(drawVars->dstPixMap.pixelSize) {
	    case 8:		drawVars->blitProc = AcceleratedSlabBlit_8;
	              break;
	    case 16:	drawVars->blitProc = AcceleratedSlabBlit_16;
	              break;
	    case 32:	drawVars->blitProc = AcceleratedSlabBlit_32;
	              break;
		  default:  return false;
						    break;
		}
#else		
		drawVars->blitProc = AcceleratedSlabBlit;
#endif		
		drawVars->refCon = (long)h3InfoDst;
    currentPatBlitSerialNum++;
    return true;
  }
  return false;
}


#if DISPATCH_SLAB_BLIT

#undef SET_FIFO_LFB

/* 8-bit desktop */
#define SET_FIFO_LFB(d, s) __swizzleWrite32_8(&(d), s)
#define AcceleratedSlabBlit_TAG AcceleratedSlabBlit_8
#define PIXELFORMAT SSTG_PIXFMT_8BPP

#include "SlabBlitLL.h"

#undef SET_FIFO_LFB
#undef AcceleratedSlabBlit_TAG
#undef PIXELFORMAT

/* 16-bit desktop */
#define SET_FIFO_LFB(d, s) __swizzleWrite32_16(&(d), s)
#define AcceleratedSlabBlit_TAG AcceleratedSlabBlit_16
#define PIXELFORMAT SSTG_PIXFMT_16BPP

#include "SlabBlitLL.h"

#undef SET_FIFO_LFB
#undef AcceleratedSlabBlit_TAG
#undef PIXELFORMAT

/* 32-bit desktop */
#define SET_FIFO_LFB(d, s) __swizzleWrite32_32(&(d), s)
#define AcceleratedSlabBlit_TAG AcceleratedSlabBlit_32
#define PIXELFORMAT SSTG_PIXFMT_32BPP

#include "SlabBlitLL.h"

#undef SET_FIFO_LFB
#undef AcceleratedSlabBlit_TAG
#undef PIXELFORMAT

#else

#define AcceleratedSlabBlit_TAG AcceleratedSlabBlit

#include "SlabBlitLL.h"

#endif


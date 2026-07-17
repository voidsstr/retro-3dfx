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
** $Header: ACCELERATEDSCALEBLIT.C, 4, 10/11/00 8:38:35 PM, Brent$
** $Log: 
**  4    3dfx      1.2.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  3    MacOS Dev Tree1.2         04/02/00 Stephane Huaulme more region parsing
**       fixing...
**  2    MacOS Dev Tree1.1         03/25/00 Stephane Huaulme changed region parsing
**       (using same as CP)
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 5     8/23/99 2:34p Kcd
** Bug fixes related to FIFO overflows.
** Added magic 565->32-bit blit support.
** 
** 4     8/03/99 2:20p Kcd
** Don't accelerate blits when source data is unaligned.
** 
** 3     7/26/99 1:55p Kcd
** Added support for 8->15 and 8->32 bit scaled blits.
** 
** 2     7/02/99 3:33p Kcd
** New headers & HRM integration.
**
*/

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
#include "RegionParser.h"

/* internal procedure prototypes */

extern FxU32 dead;
extern void  AcceleratedScreenToScreenBlitLL (NQDDrawVars  *drawVars, Rect *clipRect);
extern void  AcceleratedHostToScreenBlitLL (NQDDrawVars  *drawVars, Rect *clipRect);
extern void  AcceleratedHostToScreenBitBlit(NQDDrawVars  *drawVars);
extern void  AcceleratedScreenToScreenBitBlit(NQDDrawVars  *drawVars);

Int32  GetAcceleratedScaleBlitProc(NQDDrawVars  *drawVars)
{
	/* Let's just handle simple screen to screen copy operation for now */
	h3Info *h3InfoDst, *h3InfoSrc;
	h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);
    h3InfoSrc = FindH3Info(drawVars->srcPixMap.baseAddr);

  if(!h3InfoDst) {
    return false;
  }
    
    
    //dprintf("scale src: %08lx dst: %08lx\n",h3InfoSrc,h3InfoDst);
    
    if(h3packetData != h3packetBuffer) {
		dprintf("UM, what the FUCK (ScaleBlit)?: %d\n",h3packetData - h3packetBuffer);
	}

	if(dead) {
	   dprintf("returning because we're dead\n");
		return false;
	}
	
  if(h3InfoDst->fifo->exclusiveMode) {
    dprintf("bailing out because of exclusive mode\n");
	return false;
   }
   
   dprintf("scaleBlit\n");
   
   switch(drawVars->dstPixMap.pixelSize) {
		case 8: {
		    dprintf("bad depth: 8\n");
			return false;
		}
		break;
		
		case 16:
		if((drawVars->foreColor != 0x0000) && (drawVars->backColor != 0x7fff7fff)) {
		    dprintf("bailing out due to bad colors16: %08lx %08lx\n",drawVars->foreColor,drawVars->backColor);
			return false;
		}
		break;
		
		case 32:
		if((drawVars->foreColor != 0x0000) && (drawVars->backColor != 0x00ffffff)) {
		    dprintf("bailing out due to bad colors32: %08lx %08lx\n",drawVars->foreColor,drawVars->backColor);
			return false;
		}
		break;
	
		default:
		    dprintf("unknown pixel size!: %d\n",drawVars->dstPixMap.pixelSize);
			return false;
  }

  //dprintf("foo\n");
  /* I currently can't deal with source bitmaps that have hosed alignment. */
  if(drawVars->srcPixMap.pixelSize == 32) {
    if((FxU32)drawVars->srcPixMap.baseAddr & 0x3) {
      dprintf("bailout due to alignment.\n");
      return false;
    }
  }  
  if(drawVars->srcPixMap.pixelSize == 16) {
    if((FxU32)drawVars->srcPixMap.baseAddr & 0x1) {
      dprintf("bailout due to alignment.\n");
      return false;
    }
  }
  //dprintf("bar\n");
  /* Make sure we found a board we can accelerate, and that both src and dst
   * pointers are on that board. */

  /* dprintf("scale colorize: %d  combine: %d mode: %d trim: %d  fg: %08lx  bg: %08lx\n",
	drawVars->colorizeFlag,drawVars->combineMask,drawVars->mode,drawVars->trimResult,
	drawVars->foreColor,drawVars->backColor); */

  dprintf("h3InfoSrc: %08lx  src: %d dst: %d, color: %d mode: %d c: %08lx %08lx\n",
    h3InfoSrc,drawVars->srcPixMap.pixelSize,drawVars->dstPixMap.pixelSize,drawVars->colorizeFlag,drawVars->mode,
    drawVars->foreColor, drawVars->backColor);
  
  /* Handle special case of offscreen 15->32 bit blit */
  if(h3InfoSrc && drawVars->srcPixMap.pixelSize == 16 && drawVars->dstPixMap.pixelSize == 32 &&
     drawVars->combineMask == 0 && drawVars->colorizeFlag == 0 && drawVars->mode == srcCopy) {

    dprintf("Magic 15->32 blit!\n");
    if(drawVars->trimResult <= 0) {
      dprintf("no clipping\n");
      drawVars->blitProc = AcceleratedScreenToScreenBitBlit;
      drawVars->refCon = (long)h3InfoDst;
      return true;				
    } else {
      dprintf("with clipping\n");
      drawVars->blitProc = RegionParser;
      drawVars->refCon = (long)h3InfoDst;
      BlitMoveRect = AcceleratedScreenToScreenBlitLL;
      return true;
	}     
  } 
  
  /* Handle simple cases now */
  if((drawVars->mode <= notSrcBic || drawVars->mode == transparent) &&
     drawVars->srcPixMap.pixelSize >= 8 && drawVars->dstPixMap.pixelSize >= 16 &&
     drawVars->combineMask == 0 && drawVars->colorizeFlag == 0) {
			
    if(drawVars->trimResult <= 0) {
      drawVars->blitProc = AcceleratedHostToScreenBitBlit;
      drawVars->refCon = (long)h3InfoDst;
      return true;				
    } else {
      drawVars->blitProc = RegionParser;
      drawVars->refCon = (long)h3InfoDst;
      BlitMoveRect = AcceleratedHostToScreenBlitLL;
      return true;
	}
  }
  return false;
}


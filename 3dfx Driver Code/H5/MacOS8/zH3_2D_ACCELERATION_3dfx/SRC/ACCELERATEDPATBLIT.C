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
** $Header: ACCELERATEDPATBLIT.C, 3, 10/11/00 8:38:34 PM, Brent$
** $Log: 
**  3    3dfx      1.1.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    MacOS Dev Tree1.1         03/25/00 Stephane Huaulme changed region parsing
**       (using same as CP)
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 3     7/12/99 11:55a Kcd
** Turned off WAX bug hack.
** 
** 2     7/02/99 3:33p Kcd
** New headers & HRM integration.
**
*/

#define FAST_DISPATCH_HACK 1

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
#include <hdwr_res_mgr.h>
#include <hrm_fifo.h>

#include "RegionParser.h"
#include "H3Acceleration.h"

/* internal procedure prototypes */
extern void RgnBlitMoveForward(NQDDrawVars	*inDrawVars);


static FxU32 doPause = 0;

static void  AcceleratedPatBlit (NQDDrawVars  *drawVars);
void  AcceleratedPatBlitLL(NQDDrawVars  *drawVars, Rect *clipRect);
extern unsigned long modeToRop[16];


//extern FxU32 dead;
FxU32 currentPatBlitSerialNum = 0;
FxU32 lastPatBlitSerialNum = 0; 

Int32  GetAcceleratedPatBlitProc(NQDDrawVars  *drawVars)
{
  PixMapPtr  pMap;
  unsigned short  patHeight, patWidth;
  /* Let's just handle simple screen to screen copy operation for now */
  h3Info *h3InfoDst;
  h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);

  currentPatBlitSerialNum++;

    if(h3packetData != h3packetBuffer) {
		dprintf("UM, what the FUCK (PatBlit)?: %d\n",h3packetData - h3packetBuffer);
	}

  if(dead)
    return false;

  /* Make sure we found a board we can accelerate, and that both src and dst
   * pointers are on that board. */
  if(!h3InfoDst || h3InfoDst->fifo->exclusiveMode)
    return false;
  dprintf("patBlit\n");
#if 1
	dprintf("patSolid: %d mode: %d type: %d data[0]: %08lx dataEx[0]: %08lx fg: %08lx bg: %08lx colorize: %d\n",
		drawVars->patSolid,drawVars->mode,drawVars->patType,drawVars->patData[0],drawVars->patExData[0], 
		drawVars->foreColor,drawVars->backColor,drawVars->colorizeFlag);
#endif
  // I can't quite figure out how to reliably deal with patOr,patBic or the inverted variants.  This seems
  // to be another case where the NQDrawVars struct is incomplete.
  if(drawVars->mode >= patCopy && 
     drawVars->mode <= notPatBic &&
     (drawVars->mode & 1) && !drawVars->patSolid)
  {
    //dprintf("bailing due to patCopy or notPatBic: %d\n",drawVars->mode);
  	return false;
  }
  
	if((drawVars->mode == 170) && drawVars->patSolid) {
		;
	} else if(drawVars->mode <= notPatBic) {
	  ;
	} else {
	  dprintf("bailing due to other bogus mode\n");
	  return false;
	}

  if(drawVars->colorizeFlag) {
      dprintf("bailing due to colorizeFlag\n");
	  return false;
  }
  
  /* Actually someday I may be able to implement this with magical raster ops. */
  if(drawVars->combineMask) {
      dprintf("bailing due to combineMask\n");
	  return false;
  }
  if (drawVars->dstPixMap.pixelSize < 8) // || (drawVars->mode >= 16)))
    return (false);
	
  if (drawVars->patSolid) {
    drawVars->blitProc = AcceleratedPatBlit;
	  drawVars->refCon = (long)h3InfoDst;
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
    drawVars->blitProc = AcceleratedPatBlit;
	drawVars->refCon = (long)h3InfoDst;
    return (true);
  }

  return (false);
}

Int32  GetAcceleratedPatRgnBlitProc(NQDDrawVars  *drawVars)
{
  PixMapPtr  pMap;
  unsigned short  patHeight, patWidth;
  /* Let's just handle simple screen to screen copy operation for now */
  static FxU32 currentXorSerialNum = 0;
  h3Info *h3InfoDst;
  h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);
  
  currentPatBlitSerialNum++;

    if(h3packetData != h3packetBuffer) {
		dprintf("UM, what the FUCK (PatRgnBlit)?: %d\n",h3packetData - h3packetBuffer);
	}

  if(dead)
    return false;

  /* Make sure we found a board we can accelerate, and that both src and dst
   * pointers are on that board. */
  if(!h3InfoDst || h3InfoDst->fifo->exclusiveMode)
    return false;
#if 1
	dprintf("patRgnSolid: %d mode: %d type: %d data[0]: %08lx dataEx[0]: %08lx fg: %08lx bg: %08lx colorize: %d\n",
		drawVars->patSolid,drawVars->unmappedMode,drawVars->patType,drawVars->patData[0],drawVars->patExData[0], 
		drawVars->foreColor,drawVars->backColor,drawVars->colorizeFlag);
#endif

	if((drawVars->mode == 170) && drawVars->patSolid) {
		;
	} else if(drawVars->mode <= notPatBic) {
	  ;
	} else {
	  return false;
	}

  if(drawVars->colorizeFlag) {
      dprintf("patRgn bailed due to colorizeFlag\n");
	  return false;
  }
  if(drawVars->combineMask) {
      dprintf("patRgn bailed due to combineMask\n");
	  return false;
  }

  if(drawVars->mode >= patCopy && 
     drawVars->mode <= notPatBic &&
     (drawVars->mode & 1) && !drawVars->patSolid)
  {
       //dprintf("patRgn bailed due to patCopy/notPatBic\n");
  	 return false;
  }

  if (drawVars->dstPixMap.pixelSize < 8) //|| (drawVars->mode >= 16))
	  return false;

  if (drawVars->patSolid) {
	  drawVars->blitProc = RegionParser;
	  drawVars->refCon = (long)h3InfoDst;
	  BlitMoveRect = AcceleratedPatBlitLL;
	  return true;
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
	drawVars->blitProc = RegionParser;
	drawVars->refCon = (long)h3InfoDst;
	BlitMoveRect = AcceleratedPatBlitLL;
	return true;
  }

  return (false);
}

static void  AcceleratedPatBlit(NQDDrawVars  *drawVars)
{
	AcceleratedPatBlitLL(drawVars, &drawVars->dstRect);
}

void  AcceleratedPatBlitLL(NQDDrawVars  *drawVars, Rect *clipRect)
{
  static FxU32 commandEx;
  
  Rect * dstRect = &drawVars->dstRect;

  PixMapPtr  pMap;
  unsigned short  patHeight, patWidth, patOffsetY;

  FxI32  height, width, dstX, dstY, clipMin, clipMax;
  FxU32  dstRowBytes, pixFmt, rop;

  hrmBoard_t *board = ((h3Info *)drawVars->refCon)->board;
  hwcBoardInfo *bInfo = ((h3Info *)drawVars->refCon)->bInfo;
  hrmFifoInfo *fifo = ((h3Info *)drawVars->refCon)->fifo;
  
  void (*setLfb)(volatile FxU32 *d, FxU32 s);
  void (*setPat)(volatile FxU32 *d, FxU32 s);

  unsigned long cmdFlags = 0;

  dprintf("patBlitLL: %d,%d,%d,%d\n",
    dstRect->left,dstRect->top,dstRect->right,dstRect->bottom);
  
  switch(drawVars->dstPixMap.pixelSize) {
    case 8:   pixFmt = SSTG_PIXFMT_8BPP;
              setPat = __swizzleWrite32_32;
              setLfb = __swizzleWrite32_8;
              break;
    case 16:  pixFmt = SSTG_PIXFMT_16BPP;
              setPat = __swizzleWrite16_32;
              setLfb = __swizzleWrite32_16;           
			  break;
    case 32:  pixFmt = SSTG_PIXFMT_32BPP;
              setPat = __swizzleWrite32_8;
              setLfb = __swizzleWrite32_32;
			  break;
    default:  dprintf("what the fuck?: %d\n",drawVars->srcPixMap.pixelSize);
			  pixFmt = 0;
			  break;
  }

  if(bInfo->pciInfo.deviceID <= 5) {
     /* On Banshee/Voodoo3, the global byte swizzling will do what we want. */
     setPat = __swizzleWrite32_32;
  } else {
     /* While on Napalm, the command FIFO is always little endian. */
     setLfb = __swizzleWrite32_8;
  }
     
  /* As it turns out, the X offset already seems to be compensated for,
   * presumably in the expanded pattern data or something. */
  patOffsetY = (drawVars->dstPixMap.bounds.top) & 7;
	
  height = dstRect->bottom - dstRect->top;
  width = dstRect->right - dstRect->left;
  dstRowBytes = drawVars->dstPixMap.rowBytes;

  dstX = dstRect->left - drawVars->dstPixMap.bounds.left;
  dstY = dstRect->top - drawVars->dstPixMap.bounds.top;

  dprintf("dst: %d,%d\n",dstX,dstY);
  
  clipMin = ((clipRect->left - drawVars->dstPixMap.bounds.left)  & 0x0000FFFF)
	              | ( ((clipRect->top - drawVars->dstPixMap.bounds.top) << 16) & 0xFFFF0000);
  clipMax = ((clipRect->right - drawVars->dstPixMap.bounds.left) & 0x0000FFFF) 
	              |( ((clipRect->bottom - drawVars->dstPixMap.bounds.top) << 16) & 0xFFFF0000) ;


  if(lastPatBlitSerialNum != currentPatBlitSerialNum) {
  	lastPatBlitSerialNum = currentPatBlitSerialNum;
    if (drawVars->patSolid) {
	  int patIndex, patCount;

      // Evil hack.
      if(drawVars->mode == 170) {
	    drawVars->patExData[0] = drawVars->backColor ^ drawVars->hilitColor;
      }
			
      // Only need to load one row's worth since it's a solid pattern.
      patCount = drawVars->dstPixMap.pixelSize >> 2;

      dprintf("patCount: %d\n",patCount);
      			
      BLITPAT_START(patCount);
      for(patIndex = 0; patIndex < patCount; patIndex++) {
        setPat((FxU32 *)&(*fifo->fifoPtr++),drawVars->patExData[0]);
      }
	   
      /* Set pattern register to row 0 only */
      commandEx = SSTG_PAT_FORCE_ROW0;
 
      /* These are the only two cases we should ever see... in the other
         cases QuickDraw does a mode conversion for us. */
      if(drawVars->mode == patOr) {
        REG2D_BEGIN(1, MASK_colorFore);  
        REG2D_DATA(colorFore, drawVars->foreColor);
        REG2D_END;    
      } else if(drawVars->mode == patBic) {
        REG2D_BEGIN(1, MASK_colorFore);
        REG2D_DATA(colorFore, drawVars->backColor);
        REG2D_END;
      }

    } else{
      unsigned long patIndex, patCount;

      /* get pix map of pattern */
      pMap = *((*(drawVars->patHdl))->patMap);
	
      /* set pattern size */
      patHeight = pMap->bounds.bottom - pMap->bounds.top;
      patWidth = pMap->bounds.right - pMap->bounds.left;

      patCount = drawVars->dstPixMap.pixelSize << 1;

      dprintf("patCount: %d\n",patCount);

      BLITPAT_START(patCount);
      for(patIndex = 0; patIndex < patCount; patIndex++) {
        setPat((FxU32 *)&(*fifo->fifoPtr++),drawVars->patExData[patIndex]);
      }
	  commandEx = 0;
    }
    BUMP_N_GRIND;
	    
    if(drawVars->mode == 170) {
      // Dude, this is pretty fucked up right here.
      REG2D_BEGIN(7, MASK_srcColorkeyMin | MASK_srcColorkeyMax | 
                     MASK_dstColorkeyMin | MASK_dstColorkeyMax |
                     MASK_rop | MASK_srcBaseAddr | MASK_srcFormat);
      REG2D_DATA(srcColorkeyMin, drawVars->backColor);
      REG2D_DATA(srcColorkeyMax, drawVars->backColor);
      REG2D_DATA(dstColorkeyMin, drawVars->hilitColor);
      REG2D_DATA(dstColorkeyMax, drawVars->hilitColor);
      REG2D_DATA(rop, SSTG_ROP_PATINVERT | (SSTG_ROP_PATINVERT << 8) | (SSTG_ROP_PATINVERT << 16));
      REG2D_DATA(srcBaseAddr, ((FxU32) drawVars->dstPixMap.baseAddr - bInfo->regInfo.rawLfbBase) & SSTG_BASEADDR);
      REG2D_DATA(srcFormat, dstRowBytes | pixFmt);
      REG2D_END;
      commandEx |= SSTG_EN_SRC_COLORKEY_EX | SSTG_EN_DST_COLORKEY_EX;
	} else {
      REG2D_BEGIN(1, MASK_srcFormat);
      REG2D_DATA(srcFormat, 0 | pixFmt);
      REG2D_END;
    }

    REG2D_BEGIN(3, MASK_dstBaseAddr | MASK_dstFormat | MASK_commandEx) 
    REG2D_DATA(dstBaseAddr, ((FxU32) drawVars->dstPixMap.baseAddr - bInfo->regInfo.rawLfbBase) & SSTG_BASEADDR);
    REG2D_DATA(dstFormat, dstRowBytes | pixFmt);
    REG2D_DATA(commandEx, commandEx);
    REG2D_END;

  }
	
  REG2D_BEGIN(4, MASK_clip0min | MASK_clip0max | MASK_dstSize | MASK_dstXY);
  REG2D_DATA(clip0min,clipMin);
  REG2D_DATA(clip0max,clipMax);
  REG2D_DATA(dstSize, width | (height << 16));
  REG2D_DATA(dstXY, dstX | (dstY << 16));
  REG2D_END;

  cmdFlags |= (patOffsetY << 20);

  if(drawVars->mode == 170) {
    rop = SSTG_ROP_SRC << SSTG_ROP0_SHIFT;
    REG2D_BEGIN(2, MASK_srcXY | MASK_command);
    REG2D_DATA(srcXY, dstX | ((dstY) << 16));
    REG2D_DATA(command, SSTG_BLT | SSTG_GO | rop | cmdFlags);
    REG2D_END;  
  } else {
    rop = modeToRop[drawVars->mode];
    REG2D_BEGIN(1, MASK_command);
    REG2D_DATA(command, SSTG_RECTFILL | SSTG_GO | rop | cmdFlags);
    REG2D_END;
  }

#if 0
  if ((width <= FIXW) FIXOP (height <= FIXH ))
  {
        WAXBUG_3DNOPFIX;
  }
#endif

  BUMP_N_GRIND;

  //dprintf("portRect: %d,%d\n",drawVars->port->portRect.left,drawVars->port->portRect.top);

#if 0 && PACKET_BLITS
  if(h3packetData - h3packetBuffer > 1024) {
    H3BlitFlush(bInfo);
  }
#endif

}

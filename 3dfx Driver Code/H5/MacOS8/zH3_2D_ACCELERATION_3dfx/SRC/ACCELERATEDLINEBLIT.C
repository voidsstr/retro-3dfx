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
** $Header: ACCELERATEDLINEBLIT.C, 4, 10/11/00 8:38:33 PM, Brent$
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
#include "H3Acceleration.h"
#include "RegionParser.h"

/* internal procedure prototypes */
static FxBool clip_line(FxI32 x1, FxI32 y1, FxI32 x2, FxI32 y2,
                      FxI32 *x1out, FxI32 *y1out, FxI32 *x2out, FxI32 *y2out);
                      
static FxU32 doPause = 0;
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

static void  AcceleratedLineBlit (NQDDrawVars  *drawVars);
void  AcceleratedLineBlitLL(NQDDrawVars  *drawVars, Rect *clipRect);
extern unsigned long modeToRopBlit8[16];
extern unsigned long modeToRopBlit[16];
extern void RgnBlitAcceleratedProc(NQDDrawVars *inDrawVars);
extern void RgnBlitMoveForward(NQDDrawVars	*inDrawVars);

extern FxU32 lastPatBlitSerialNum, currentPatBlitSerialNum;

Int32  GetAcceleratedLineBlitProc(NQDDrawVars  *drawVars)
{
  /* Let's just handle simple screen to screen copy operation for now */
  h3Info *h3InfoDst;
  h3InfoDst = FindH3Info(drawVars->dstPixMap.baseAddr);

  currentPatBlitSerialNum++;

  if(dead)
    return false;

  /* Make sure we found a board we can accelerate, and that both src and dst
   * pointers are on that board. */
  if(!h3InfoDst || h3InfoDst->fifo->exclusiveMode)
    return false;
  
  /* Must be 8 bits or greater. */
  if(drawVars->dstPixMap.pixelSize < 8)
  	return false;
  	
  /* Only do 1x1 lines right now */
	if(!(drawVars->penSize.h == 1 &&
	     drawVars->penSize.v == 1)) {
	  return false;
	}
	
	// Just accelerate non-patterned lines for now.
  if(drawVars->mode != srcCopy)
  	return false;
		
  /* Actually someday I may be able to implement this with magical raster ops. */
  if(drawVars->combineMask)
	  return false;

	//return false;
#if 1
	dprintf("line: %d mode: %d type: %d size: %d,%d fg: %08lx bg: %08lx pat: %08lx\n",
		drawVars->patSolid,drawVars->unmappedMode,drawVars->patType,drawVars->penSize.h,drawVars->penSize.v, 
		drawVars->foreColor,drawVars->backColor,drawVars->patExData);
	//dprintf("regions: %08lx %08lx %08lx\n",drawVars->rgnA,drawVars->rgnB,drawVars->rgnC);
#endif
	
	//dprintf("trim: %d\n",drawVars->trimResult);
	
	if(drawVars->trimResult > 0) {
	  dprintf("doing region shit\n");
		drawVars->blitProc = RegionParser;
		drawVars->refCon = (long)h3InfoDst;
		BlitMoveRect = AcceleratedLineBlitLL;
	}

	drawVars->blitProc = AcceleratedLineBlit;
	drawVars->refCon = (long)h3InfoDst;
	
  return (true);
}


static void  AcceleratedLineBlit(NQDDrawVars  *drawVars)
{
  //dprintf("using minRect\n");
	AcceleratedLineBlitLL(drawVars, &drawVars->minRect);
}

	
void  AcceleratedLineBlitLL(NQDDrawVars  *drawVars, Rect *dstRect)
{
  static FxU32 commandEx = 0;

  FxI32  height, width, clipMin, clipMax;
  FxU32  dstRowBytes, pixFmt, rop;
	Rect   clipRect;
	FxI32  x1,y1,x2,y2;
	
  hrmBoard_t *board = ((h3Info *)drawVars->refCon)->board;
  hwcBoardInfo *bInfo = ((h3Info *)drawVars->refCon)->bInfo;
  hrmFifoInfo *fifo = ((h3Info *)drawVars->refCon)->fifo;
  void (*setLfb)(volatile FxU32 *d, FxU32 s);

  unsigned long cmdFlags = 0;

//  FXUNUSED(srcRect);
	
	dprintf("minRect: %d,%d - %d,%d\n",drawVars->minRect.left,
	drawVars->minRect.top,
	drawVars->minRect.right,
	drawVars->minRect.bottom);
	
  dprintf("dstBnds: %d,%d - %d,%d\n",drawVars->dstPixMap.bounds.left,
  drawVars->dstPixMap.bounds.top,
  drawVars->dstPixMap.bounds.right,
  drawVars->dstPixMap.bounds.bottom);

  dprintf("line: %d,%d - %d,%d\n",drawVars->pt1.h,drawVars->pt1.v,
  drawVars->pt2.h,drawVars->pt2.v);
  
 	dprintf("dstRect: %d,%d - %d,%d\n",dstRect->left,dstRect->top,dstRect->right,dstRect->bottom);
	
  height = dstRect->bottom - dstRect->top;
  width = dstRect->right - dstRect->left;
  dstRowBytes = drawVars->dstPixMap.rowBytes;

  clipRect.left = dstRect->left - drawVars->dstPixMap.bounds.left;
  clipRect.top = dstRect->top - drawVars->dstPixMap.bounds.top;
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

  dprintf("oldLine: %d,%d - %d,%d\n",x1,y1,x2,y2);

  if(!clip_line(x1,y1,x2,y2, &x1, &y1, &x2, &y2))
    return;
    
  dprintf("newLine: %d,%d - %d,%d\n",x1,y1,x2,y2);
    
  setLfb = fifo->setLfb;
  
	switch(drawVars->dstPixMap.pixelSize) {
	    case 8:   pixFmt = SSTG_PIXFMT_8BPP;
								rop = modeToRopBlit8[drawVars->mode];
//								setLfb = __swizzleWrite32_8;
					      break;
	    case 16:  pixFmt = SSTG_PIXFMT_16BPP;
								rop = modeToRopBlit[drawVars->mode];
//								setLfb = __swizzleWrite32_16;
					      break;
	    case 32:  pixFmt = SSTG_PIXFMT_32BPP;
								rop = modeToRopBlit[drawVars->mode];
//								setLfb = __swizzleWrite32_32;
					      break;
	    default:  dprintf("what the fuck?: %d\n",drawVars->srcPixMap.pixelSize);
					      pixFmt = 0;
					      break;
	}
  
#define MAX(a,b) ((a) > (b) ? (a) : (b))
		  		  
  if(lastPatBlitSerialNum != currentPatBlitSerialNum) {
  	lastPatBlitSerialNum = currentPatBlitSerialNum;

	  REG2D_BEGIN(4, MASK_dstBaseAddr | MASK_dstFormat | MASK_commandEx | MASK_colorFore) 
    REG2D_DATA(dstBaseAddr, ((FxU32) drawVars->dstPixMap.baseAddr - bInfo->regInfo.rawLfbBase) & SSTG_BASEADDR);
    REG2D_DATA(dstFormat, dstRowBytes | pixFmt);
    REG2D_DATA(commandEx, 0);
	  REG2D_DATA(colorFore, drawVars->foreColor);
	  REG2D_END;
  }
	
  REG2D_BEGIN(2, MASK_clip0min | MASK_clip0max);  
  REG2D_DATA(clip0min,clipMin);
  REG2D_DATA(clip0max,clipMax);
  REG2D_END;
  
  BUMP_N_GRIND;

	REG2D_BEGIN(3, MASK_srcXY | MASK_dstXY | MASK_command);
	REG2D_DATA(srcXY, (x1 & 0xffff) | (y1 << 16));
	REG2D_DATA(dstXY, (x2 & 0xffff) | (y2 << 16));
	REG2D_DATA(command, SSTG_LINE | SSTG_GO | rop | cmdFlags);
	REG2D_END;  

  BUMP_N_GRIND;

}

#define LEFT 0x01
#define RIGHT 0x02
#define BOTTOM 0x04
#define TOP 0x08

static FxU32 find_code(FxI32 x, FxI32 y)
{
  FxU32 code;
  if(x < 0) code = LEFT;
  else if (x >= 4096) code = RIGHT;
  else code = 0;
  if(y >= 4096) code |= BOTTOM;
  else if(y < 0) code |= TOP;
  return code;
}

static FxBool clip_line(FxI32 x1, FxI32 y1, FxI32 x2, FxI32 y2,
                      FxI32 *x1out, FxI32 *y1out, FxI32 *x2out, FxI32 *y2out)
{
  FxU32 code1 = find_code(x1, y1);
  FxU32 code2 = find_code(x2, y2);
  FxU32 code;
  FxI32 x, y;
  
  while(code1 | code2) {
    if(code1 & code2) {
      return FXFALSE;
    }
    
    if(code1) code = code1;
    else code = code2;
    
    if(code & LEFT) {
      x = 0;
      y = y1 + (FxI32)((float)(y2 - y1)/(x2 - x1)*(-x1));
    }
    else if(code & RIGHT) {
      x = 4095;
      y = y1 + (FxI32)((float)(y2 - y1)/(x2 - x1)*(x-x1));
    }
    else if(code & BOTTOM) {
      y = 4095;
      x = x1 + (FxI32)((float)(x2 - x1)/(y2 - y1)*(y - y1));
    }
    else if(code & TOP) {
      y = 0;
      x = x1 + (FxI32)((float)(x2 - x1)/(y2 - y1)*(-y1));
    }
    
    if(code == code1) code1 = find_code(x1 = x, y1 = y);
    else code2 = find_code(x2 = x, y2 = y);
    
  }
  *x1out = x1;
  *y1out = y1;
  *x2out = x2;
  *y2out = y2;
  
  return FXTRUE;
}



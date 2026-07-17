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
** $Header: SLABBLITLL.H, 2, 10/11/00 8:38:43 PM, Brent$
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 3     7/12/99 11:55a Kcd
** Turned off WAX bug hack.
** 
** 2     7/09/99 7:18p Kcd
** Enabled WAX bug fix.
** 
** 1     7/02/99 3:31p Kcd
** Slab blit common code.
**
*/

static void  AcceleratedSlabBlit_TAG (NQDDrawVars  *drawVars)
{
  FxU32 commandEx;
  FxU32 patOffsetY;

  FxI32  height, width, dstX, dstY; //, clipMin, clipMax;
  FxU32  dstRowBytes, pixFmt, rop;
  hrmBoard_t *board = ((h3Info *)drawVars->refCon)->board;
  hwcBoardInfo *bInfo = ((h3Info *)drawVars->refCon)->bInfo;
  hrmFifoInfo *fifo = ((h3Info *)drawVars->refCon)->fifo;
  Rect slabRect;
  Rect *dstRect = &slabRect;

#if !DISPATCH_SLAB_BLIT
  void (*setLfb)(volatile FxU32 *d, FxU32 s);
  void (*setPat)(volatile FxU32 *d, FxU32 s);
#endif
  unsigned long cmdFlags = 0;
  
  /* Sigh.  I can't seem to figure out how to find the x,y origin
   * of the slab directly.  So, I'm going to take the dstPtr and figure it out
   * the hard way.  Ugh. */
  FxU32 offset, offsetX, offsetY, dstXright;
  //Rect slabRect;
	//Rect *dstRect = &slabRect;


  patOffsetY = drawVars->dstPixMap.bounds.top & 7;
  dstRowBytes = drawVars->dstPixMap.rowBytes;

  /* This gets us the offset in bytes */
  offset = (FxU32)drawVars->dstRow - (FxU32)drawVars->dstPixMap.baseAddr;
  offsetY = offset / dstRowBytes;

  /* Do nothing, just try to figure out where to look */
  dstX = (drawVars->leftEdge >> 16);
  dstXright = (drawVars->rightEdge >> 16);

  if(dstX < drawVars->minRect.left)
    dstX = drawVars->minRect.left;
  if(dstXright > drawVars->minRect.right)
    dstXright = drawVars->minRect.right;

  offsetX = (offset - (offsetY * dstRowBytes)) >> (drawVars->dstPixMap.pixelSize >> 4);

  height = 1;
  width = dstXright - dstX;
  dstX += offsetX;
  dstY = offsetY;

  rop = modeToRop[drawVars->mode];

#if DISPATCH_SLAB_BLIT
  pixFmt = PIXELFORMAT;
#else
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
#endif

  if(lastPatBlitSerialNum != currentPatBlitSerialNum) {
	lastPatBlitSerialNum = currentPatBlitSerialNum;

    if (drawVars->patSolid) {
	  int patIndex, patCount;
		patCount = drawVars->dstPixMap.pixelSize >> 2;
		BLITPAT_START(patCount);
		for(patIndex = 0; patIndex < patCount; patIndex++) {
			setPat((FxU32 *)&(*fifo->fifoPtr++),drawVars->patExData[0]);
        }
	 
	  /* Set pattern register to row 0 only */
      commandEx = SSTG_PAT_FORCE_ROW0;
  
    } else{
	  unsigned long patIndex, patCount;

  	  /* set pattern size */
      patCount = drawVars->dstPixMap.pixelSize << 1;

      BLITPAT_START(patCount);
      for(patIndex = 0; patIndex < patCount; patIndex++) {
        setPat((FxU32 *)&(*fifo->fifoPtr++),drawVars->patExData[patIndex]);
      }
	  commandEx = 0;
	}

	REG2D_BEGIN(6, MASK_clip0min | MASK_clip0max | MASK_dstBaseAddr |
                   MASK_dstFormat | MASK_commandEx | MASK_srcFormat );
    REG2D_DATA(clip0min, 0);
    REG2D_DATA(clip0max, 2048 | (2048 << 16));
    REG2D_DATA(dstBaseAddr, (FxU32) drawVars->dstPixMap.baseAddr - bInfo->regInfo.rawLfbBase);
    REG2D_DATA(dstFormat, dstRowBytes | pixFmt);
    REG2D_DATA(commandEx, commandEx);
    REG2D_DATA(srcFormat, 0 | pixFmt);
    REG2D_END;
  }

  cmdFlags |= (patOffsetY << 20);

  REG2D_BEGIN(3, MASK_dstSize | MASK_dstXY | MASK_command);
  REG2D_DATA(dstSize, width | (height << 16));
  REG2D_DATA(dstXY, dstX | (dstY << 16));
  REG2D_DATA(command, SSTG_RECTFILL | SSTG_GO | rop | cmdFlags);
  REG2D_END;
  
#if 0
  if ((width <= FIXW) FIXOP (height <= FIXH ))
  {
    WAXBUG_3DNOPFIX;
  }
#endif
  BUMP_N_GRIND;

	//dprintf("portRect: %d,%d\n",drawVars->port->portRect.left,drawVars->port->portRect.top);
#if 0
#if PACKET_BLITS
  if(h3packetData - h3packetBuffer > H3FIFO_SIZE) {
    H3BlitFlush(bInfo);
  }
#endif
#endif

}


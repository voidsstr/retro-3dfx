/******************************Module*Header*******************************\
* Module Name: cfifo.c
*
*   Command fifo routines.
*
* Copyright (c) 1997 3Dfx Interactive, Inc.
*
\**************************************************************************/

#include "precomp.h"


#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)

#define BUMPAGP(ppdev, nwords)  \
  { \
      CmdFifo   *fifo = (CmdFifo *) ppdev->pjFifoBase; \
                                    \
      P6_FENCE;                     \
      while (nwords > 0xffff)       \
      {                             \
          SET2(fifo->bump, 0xffff); \
          nwords -= 0xffff;         \
      }                             \
      if (nwords > 0)               \
          SET2(fifo->bump, nwords); \
                                    \
      CMDFIFOUNBUMPEDWORDS = 0;     \
  }

void FLUSHAGP(NT9XDEVICEDATA * ppdev)
{
  FxU32 nWords;

  nWords = CMDFIFOUNBUMPEDWORDS;

  if (nWords == 0)
    return;

  BUMPAGP(ppdev, nWords);
}

void NTAGPEPILOG(NT9XDEVICEDATA *ppdev, FxU32 hwPtr)
{
  FxU32 nWords;

  nWords = (hwPtr - CMDFIFOEPILOGPTR) / 4;
  CMDFIFOEPILOGPTR = hwPtr;

  CMDFIFOUNBUMPEDWORDS += nWords;

//  if (CMDFIFOUNBUMPEDWORDS < CMDFIFO_BUMPTHRESH)
//    return;

  nWords = CMDFIFOUNBUMPEDWORDS;

  BUMPAGP(ppdev, nWords);
}

void MYWRAPAGP(NT9XDEVICEDATA * ppdev, FxU32 hwPtr)
{
  FxU32 nWords;

  // calculate the # of words to bump
  // remember to add 2 words for the AGP jump!
  //
  nWords = (hwPtr + 8 - CMDFIFOEPILOGPTR) / 4;
  nWords += CMDFIFOUNBUMPEDWORDS;

  BUMPAGP(ppdev, nWords);

  CMDFIFOEPILOGPTR = (ULONG) CMDFIFOSTART;
}

#endif


FxBool CmdFifo0Init( PDEV*	ppdev,
                     ULONG fifoStart,
                     ULONG size,
                     FxBool disableHoles,
                     FxBool agpEnable)
{
    SstCRegs *sstc = (SstCRegs *) ppdev->pjCmdAgpBase;
    CmdFifo *fifo = (CmdFifo *) ppdev->pjFifoBase;

    H3PRINTF((ppdev, "  CmdFifo0Init\r\n"));
	
    DISPDBG( (1,"CmdFifo0Init(fifo=%d,start=0x%x,size=0x%x(%d),\n\t\tdisHoles=%d,agpEnable=%d)\n",
        0,fifoStart,size,size,disableHoles,agpEnable) );

// 	HalIdle();

    SET2(fifo->baseSize,0);			// disable the CMD fifo
    size = (size>>12) - 1;		// round up, convert to 4KB pages

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
	if (agpEnable)
    {
        FxU32 baseH = ppdev->fifoData.physBaseH;
        FxU32 baseL = ppdev->fifoData.physBaseL;

        SET2(sstc->agpReqSize, 0);
        SET2(sstc->hostAddrLow, 0);
        SET2(sstc->hostAddrHigh, 0);
        SET2(sstc->graphicsAddr, 0);
        SET2(sstc->graphicsStride, 0);
        SET2(fifo->bump, 0);

        SET2(fifo->baseAddrL,(baseH << (32-12)) | (baseL>>12));
        SET2(fifo->readPtrL,baseL);
        SET2(fifo->readPtrH,baseH);
        SET2(fifo->aMin,baseL-4);
        SET2(fifo->aMax,baseL-4);
    }
    else
#endif
    {
        SET2(fifo->baseAddrL,fifoStart>>12);
        H3PRINTF((ppdev, "    baseAddrL = %08lX\r\n", fifoStart>>12));
        SET2(fifo->readPtrL,fifoStart);
        H3PRINTF((ppdev, "    readPtrL = %08lX\r\n", fifoStart));
        SET2(fifo->readPtrH,0);
        H3PRINTF((ppdev, "    readPtrH = %08lX\r\n", 0));
        SET2(fifo->aMin,fifoStart-4);
        H3PRINTF((ppdev, "    aMin = %08lX\r\n", fifoStart-4));
        SET2(fifo->aMax,fifoStart-4);
        H3PRINTF((ppdev, "    aMax = %08lX\r\n", fifoStart-4));
    }

    SET2(fifo->depth,0);
    SET2(fifo->holeCount,0);
    SET2(sstc->cmdFifoThresh,(0x0f << 5) | 0x8); // Fifo LWM /HWM/ THRESHOLD

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
	if (agpEnable)
    {
        // Size field in baseSize register not important for AGP,
        // it must only be used for hole counting.

        SET2(fifo->baseSize, SST_EN_CMDFIFO | SST_CMDFIFO_DISABLE_HOLES | SST_CMDFIFO_AGP);
        CMDFIFOEPILOGPTR = (ULONG) CMDFIFOSTART;
        CMDFIFOUNBUMPEDWORDS = 0;
	}
	else
#endif
    {
        SET2(fifo->baseSize,size | SST_EN_CMDFIFO | (disableHoles ? SST_CMDFIFO_DISABLE_HOLES : 0));
	}
	
    DISPDBG( (2, "CMD FIFO placed at physical addr 0x%x\n",fifoStart) );
    return FXTRUE;
}


void CmdFifo0Disable( PDEV*	ppdev )
{
    CmdFifo *fifo = (CmdFifo *) ppdev->pjFifoBase;
    H3PRINTF((ppdev, "  CmdFifo0Disable\r\n"));
	
// 	HalIdle();

	SET2(fifo->baseSize,0);			// disable the CMD fifo
  H3PRINTF((ppdev, "    baseSize = %08lX\r\n", 0));
    DISPDBG( (2, "CMD FIFO 0 disabled") );
    return;
}


#ifdef USE_D3D_CODE
// This version of H3MakeRoom does absolute calculations for the
// roomToReadPtr.  The D3D code requests space for the max number of
// entries it might write to the cmdfifo, but it might not use all
// of the entries.  This confuses the relative calculations for
// roomToReadPtr in the other version of H3MakeRoom and eventually
// causes the sw to get into an infinite loop in the roomToReadPtr
// while loop, which makes the system appear to be hung
void
H3MakeRoom (PDEV* ppdev, ULONG **pfifoPtr, LONG n)
{
  FIFO_DATA *pfifoData = &ppdev->fifoData;
  CmdFifo   *fifo = (CmdFifo *)ppdev->pjFifoBase;
  LONG      N = n * 4;            // N = Size in bytes of n dwords.
  LONG      fullCnt = 0;

  ASSERTDD(N <= (LONG)pfifoData->fifoSize, "request too large");

  CMDFIFO_CHECKBUMP(n)

#if DBG
  if (STAT_FIFO_IN_USE & ppdev->flStatus)
  {
    _asm int 3;
  }
#endif
  // stall while glide dumps data into the fifo
  while (STAT_FIFO_IN_USE & ppdev->flStatus)
    ;

again:
  // do we need to stall?
  //
  // This loop tracks how much space is available between where we want
  // to write to in the cmdfifo and where the hw has last read from.
  // If the hw is so far behind that the cmdfifo is almost full, we
  // need to stall until the hw catchs up, at least we need to stall
  // to the point where there is enough space in the cmdfifo for the
  // current request
  while (pfifoData->roomToReadPtr <= N)
  {
    volatile ULONG curReadPtr;
#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
    // I was not sure if the fix below applied to the Win2K AGP_CMDFIFO case so I left this line as it was.  DanO 8/24/99
    ULONG curWritePtr = (ULONG)(*pfifoPtr) - (ULONG)pfifoData->fifoStart;
#else
	// add back in offset so that both curReadPtr and curWritePtr are the "the current location as a
	// number of bytes from the beginning of the Frame Buffer Memory".
    ULONG curWritePtr = (ULONG)(*pfifoPtr) - (ULONG)pfifoData->fifoStart + (ULONG)pfifoData->fifoOffset;
#endif

    curReadPtr = GET2(fifo->readPtrL);

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
    if( ppdev->doAGPFifo == TRUE )
        curReadPtr -= pfifoData->physBaseL;
#endif

    // the cmdfifo is a circular buffer, so we've got to account for
    // this when computing the actual space available in the fifo
    if (curReadPtr > curWritePtr)
    {
      // when the curReadPtr is > the curWritePtr, we only have the
      // room from curReadPtr minus curWritePtr available
      pfifoData->roomToReadPtr = curReadPtr - curWritePtr;
    }
    else if (curReadPtr < curWritePtr)
    {
      // when the curWritePtr is > the curReadPtr, we have from the curWritePtr
      // to the end of the cmdfifo plus from the start of the cmdfifo
      // to the curReadPtr available
      // or equivalently
      pfifoData->roomToReadPtr = pfifoData->fifoSize - curWritePtr + curReadPtr;
    }
    else // curReadPtr == curWritePtr
    {
      // either the fifo is completely full or completely empty
      if (GET2(fifo->depth))
      {
        // the fifo is completely full, we're probably hosed!
        pfifoData->roomToReadPtr = 0;
        fullCnt++;
        if (20 <= fullCnt)
        {
          ASSERTDD(0, "fifo full");
        }
        goto again;
      }
      else
      {
        // the fifo is completely empty
        pfifoData->roomToReadPtr = pfifoData->fifoSize;
      }
    }

    pfifoData->fifoLastRead = curReadPtr;

    DISPDBG((5,"  update: %d,%d left\n",pfifoData->roomToEnd,pfifoData->roomToReadPtr));
  }

  // do we need to wrap?
  //
  // this if tracks the space available to the physical end of the cmdfifo,
  // if there isn't enough room for the current request then we need to
  // wrap around to the start of the cmdfifo
  if (pfifoData->roomToEnd <= N)
  {
    // wrap to start of cmdfifo
    DISPDBG((5,"  wrapping with %d,%d left\n",pfifoData->roomToEnd,pfifoData->roomToReadPtr));
    {
#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
      if( ppdev->doAGPFifo == TRUE )
      {
          SET2( **pfifoPtr, pfifoData->fifoJmpHdr );
          H3PRINTF((ppdev, "    H3MakeRoom  %8lX = %08lX\r\n", *pfifoPtr, pfifoData->fifoJmpHdr));
          SET2( *(*pfifoPtr + 1), pfifoData->fifoJmpHdr2 );
          MYWRAPAGP(ppdev, (ULONG) *pfifoPtr);
      }
      else
#endif
      {
          SET2( **pfifoPtr, pfifoData->fifoJmpHdr );
          H3PRINTF((ppdev, "    H3MakeRoom  %8lX = %08lX\r\n", *pfifoPtr, pfifoData->fifoJmpHdr));
          P6_FENCE;
      }
    }
    *pfifoPtr = (ULONG *) pfifoData->fifoStart;
    pfifoData->roomToEnd = pfifoData->fifoSize;
    // force the roomToReadPtr to be updated
    pfifoData->roomToReadPtr = 0;
    goto again;
  }

  pfifoData->roomToReadPtr -= N;
  pfifoData->roomToEnd -= N;
}
#else
// make room for 'n' bytes
void H3MakeRoom(PDEV* ppdev, ULONG **pfifoPtr, LONG n)
{
    FIFO_DATA   *pfifoData = &ppdev->fifoData;
    CmdFifo *fifo = (CmdFifo *) ppdev->pjFifoBase;
    LONG        N = n * 4;      // N = Size in bytes of n dwords.

//#ifdef DBG
//	H3_GP_WAIT(ppdev,ppdev->pjBase);
//#endif

again:
    while (pfifoData->roomToReadPtr <= N) {		// do we need to stall?
        volatile ULONG curReadPtr = GET2(fifo->readPtrL);

        if (pfifoData->fifoLastRead <= curReadPtr)
           pfifoData->roomToReadPtr += curReadPtr - pfifoData->fifoLastRead;
        else
           pfifoData->roomToReadPtr += pfifoData->fifoSize - (pfifoData->fifoLastRead - curReadPtr);
        pfifoData->fifoLastRead = curReadPtr;
        DISPDBG((5,"  update: %d,%d left\n",pfifoData->roomToEnd,pfifoData->roomToReadPtr));
    }

    if (pfifoData->roomToEnd <= N) {		// wrap to front
        DISPDBG((5,"  wrapping with %d,%d left\n",pfifoData->roomToEnd,pfifoData->roomToReadPtr));
        {
#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
            if( ppdev->doAGPFifo == TRUE )
            {
                SET2( **pfifoPtr, pfifoData->fifoJmpHdr );
                SET2( *(*pfifoPtr + 1), pfifoData->fifoJmpHdr2 );
                MYWRAPAGP(ppdev, (ULONG) *pfifoPtr);
            }
            else
#endif
            {
                SET2( **pfifoPtr, pfifoData->fifoJmpHdr );
                H3PRINTF((ppdev, "    H3MakeRoom  %8lX = %08lX\r\n", *pfifoPtr, pfifoData->fifoJmpHdr));
                P6_FENCE;
            }
        }
        *pfifoPtr = (ULONG *) pfifoData->fifoStart;
        pfifoData->roomToReadPtr -= pfifoData->roomToEnd;
        pfifoData->roomToEnd = pfifoData->fifoSize;
        goto again;
    }

//#ifdef DBG
//	ASSERTDD( (pfifoData->roomToEnd == (LONG) (pfifoData->fifoSize - (((ULONG) *pfifoPtr) - (ULONG) pfifoData->fifoStart)) ), "roomToEnd size wrong in H3MakeRoom" );
//#endif

    pfifoData->roomToReadPtr -= N;
    pfifoData->roomToEnd -= N;
}
#endif

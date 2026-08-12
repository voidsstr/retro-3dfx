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

    SET2(fifo->baseSize,0);			        // disable the CMD fifo
    size = ((size + 4095) / 4096) - 1;  // round up, convert to 4KB pages

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
    if (IS_NAPALM)
    {
      SET2(sstc->cmdFifoThresh,(0x14 << 5) | 0x8); // Fifo LWM /HWM/ THRESHOLD
    }
    else
    {
      SET2(sstc->cmdFifoThresh,(0x0f << 5) | 0x8); // Fifo LWM /HWM/ THRESHOLD
    }

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

#if DBG
#define MIN_ROOM_TO_READ_PTR      8*1024
#else
#define MIN_ROOM_TO_READ_PTR      0
#endif

/* retro3dfx: CMD-FIFO flight-recorder instrumentation. Uses the driver's own
   ENABLE_LOG_FILE mechanism (h3printf/retroLogForce -> IOCTL -> miniport ->
   C:\3dfxvs.log): EngDebugPrint proved to be a no-op on free/retail XP.
   g_h3mrAnnounced latches the one-time positive-control announce;
   g_h3mrStallReported rate-limits stall reports so a flood can't fill the log. */
LONG g_h3mrAnnounced = 0;
LONG g_h3mrStallReported = 0;
LONG g_h3mrCallCount = 0;

void
H3MakeRoom (PDEV* ppdev, ULONG **pfifoPtr, LONG n)
{
  FIFO_DATA       *pfifoData = &ppdev->fifoData;
  CmdFifo         *fifo;
  LONG            N = n * 4;            // N = Size in bytes of n dwords.
  LONG            fullCnt = 0;
  ULONG           numChips;
  ULONG           i;
  LONG            roomToReadPtr, smallestRoomToReadPtr;
  volatile ULONG  curReadPtr;
  ULONG           curWritePtr;
  ULONG           spinCount = 0;        /* retro3dfx: CMD-FIFO wedge detector */
#if MIN_ROOM_TO_READ_PTR
  LONG            minRoomToReadPtr;
#endif


#if MIN_ROOM_TO_READ_PTR
  if (N + MIN_ROOM_TO_READ_PTR <= (LONG)pfifoData->fifoSize)
    minRoomToReadPtr = MIN_ROOM_TO_READ_PTR - 4;
  else
    minRoomToReadPtr = (LONG)pfifoData->fifoSize - N - 4;

  ASSERTDD((N + minRoomToReadPtr) <= (LONG)pfifoData->fifoSize, "request too large");
#else
  ASSERTDD(N <= (LONG)pfifoData->fifoSize, "request too large");
#endif

#if ENABLE_LOG_FILE
  /* retro3dfx positive control: one-time announce on the first H3MakeRoom call.
     Desktop 2D drawing hits H3MakeRoom shortly after boot, so a line in
     C:\3dfxvs.log proves the whole logging pipeline (display -> IOCTL ->
     miniport -> file) is live. Bypasses the runtime gate on purpose. */
  if (!g_h3mrAnnounced)
  {
    g_h3mrAnnounced = 1;
    retroLogForce(ppdev, "retro3dfx H3MakeRoom FIRST-CALL: fifoSize=%ld (positive control)\r\n",
                  (LONG)pfifoData->fifoSize);
  }

  /* retro3dfx heartbeat (verbose only, Retro3dfxLog >= 2): periodic liveness
     marker during activity. Gated so normal/benchmark runs pay nothing. */
  if (2 <= g_retroLogLevel && 0 == (g_h3mrCallCount % 4096L))
    h3printf(ppdev, "retro3dfx H3MakeRoom heartbeat #%ld\r\n", g_h3mrCallCount);
  g_h3mrCallCount++;
#endif

  CMDFIFO_CHECKBUMP(n)

  if (ppdev->bGDIMadeDirectAccess)
  {
    P6_FENCE;
    MODIFY_SLI_READ(ppdev, DISABLE_SLI_READ);
    ppdev->bGDIMadeDirectAccess = FALSE;
  }

  // determine number of active chips
#ifdef SLI_AA
  if (_FF(ddMultiChipConfig))
    numChips = _FF(dwNumUnits);
  else
#endif
    numChips = 1;

again:

  // add back in offset so that both curReadPtr and curWritePtr are the "the current location as a
  // number of bytes from the beginning of the Frame Buffer Memory".
  curWritePtr = (ULONG)(*pfifoPtr) - (ULONG)pfifoData->fifoStart + (ULONG)pfifoData->fifoOffset;

  smallestRoomToReadPtr = 0x7FFFFFFF;

  // loop over active chips to find one with smallest roomToReadPtr
  for (i = 0; i < numChips; i++)
  {
    fifo = (CmdFifo *)(_FF(regBase[i * HWINFO_SST_MAX_CHIP_INDEX + HWINFO_SST_CMDFIFOREGS_INDEX]) +
           FIELDOFFSET(SstCRegs, cmdFifo0));

    // grab last roomToReadPtr from pdev's FIFO_DATA struct
    roomToReadPtr = pfifoData->roomToReadPtr;

againThisChip:

    // do we need to stall?
    //
    // This loop tracks how much space is available between where we want
    // to write to in the cmdfifo and where the hw has last read from.
    // If the hw is so far behind that the cmdfifo is almost full, we
    // need to stall until the hw catchs up, at least we need to stall
    // to the point where there is enough space in the cmdfifo for the
    // current request
#if MIN_ROOM_TO_READ_PTR
    while (roomToReadPtr <= (N + minRoomToReadPtr))
#else
    while (roomToReadPtr <= N)
#endif
    {
      /* retro3dfx graduated stall flight-recorder. A healthy stall drains in well
         under 100K iterations. The first time any stall crosses 100K, force-log the
         FIFO state (rate-limited; flushed so it survives a subsequent hang). Keep a
         50M safety-break so a genuine infinite wedge recovers the CPU (soft frozen
         display) instead of pinning it into a TDR. Does NOT break early, so
         near-stock wedge behavior stays observable up to 50M. */
      ++spinCount;
#if ENABLE_LOG_FILE
      if (spinCount == 100000UL && g_h3mrStallReported < 24)
      {
        g_h3mrStallReported++;
        retroLogForce(ppdev, "retro3dfx H3MakeRoom STALL>=100K: N=%ld fifoSize=%ld curRead=%08lx curWrite=%08lx room=%ld fullCnt=%ld\r\n",
                      N, (LONG)pfifoData->fifoSize, (ULONG)curReadPtr, (ULONG)curWritePtr, roomToReadPtr, fullCnt);
      }
#endif
      if (spinCount > 50000000UL)
      {
#if ENABLE_LOG_FILE
        if (g_h3mrStallReported < 48)
        {
          g_h3mrStallReported++;
          retroLogForce(ppdev, "retro3dfx H3MakeRoom WEDGE-BREAK@50M: N=%ld fifoSize=%ld curRead=%08lx room=%ld\r\n",
                        N, (LONG)pfifoData->fifoSize, (ULONG)curReadPtr, roomToReadPtr);
        }
#endif
        roomToReadPtr = (LONG)pfifoData->fifoSize;  /* safety net: recover CPU */
        break;
      }
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
        roomToReadPtr = curReadPtr - curWritePtr;
      }
      else if (curReadPtr < curWritePtr)
      {
        // when the curWritePtr is > the curReadPtr, we have from the curWritePtr
        // to the end of the cmdfifo plus from the start of the cmdfifo
        // to the curReadPtr available
        // or equivalently
        roomToReadPtr = pfifoData->fifoSize - curWritePtr + curReadPtr;
      }
      else // curReadPtr == curWritePtr
      {
        // either the fifo is completely full or completely empty
        if (GET2(fifo->depth))
        {
          // the fifo is completely full, we're probably hosed!
          roomToReadPtr = 0;
          fullCnt++;
          if (20 <= fullCnt)
          {
            ASSERTDD(0, "fifo full");
          }
          goto againThisChip;
        }
        else
        {
          // the fifo is completely empty
          roomToReadPtr = pfifoData->fifoSize;
        }
      }
    } // while loop to stall for this chip

    if (roomToReadPtr < smallestRoomToReadPtr)
      smallestRoomToReadPtr = roomToReadPtr;
  } // for loop over active chips

  // at this point, all chips have stalled if necessary and there is room in the
  // fifo for this next set of commands

  // update the roomToReadPtr in the pdev's FIFO_DATA struct
  if (pfifoData->roomToReadPtr != smallestRoomToReadPtr)
  {
    pfifoData->roomToReadPtr = smallestRoomToReadPtr;
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
    *pfifoPtr = pfifoData->fifoPtr = (ULONG *) pfifoData->fifoStart;
    pfifoData->roomToEnd = pfifoData->fifoSize;
    // force the roomToReadPtr to be updated
    pfifoData->roomToReadPtr = 0;
    goto again;
  }

#if 0
  // move these to the CMDFIFO_CHECKROOM, CMDFIFO_EPILOG, CMDFIFO_SAVE,
  // GHW_FIFO_JMP_PLUS4 & CHECK_FIFO_ROOM macros
  pfifoData->roomToReadPtr -= N;
  pfifoData->roomToEnd -= N;
#endif
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

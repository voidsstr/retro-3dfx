/*
** Copyright (c) 1997-1998, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
*/
//#include "fxglobal.h"

#include "precomp.h"


#if defined(DEBUGFIFO) && defined(DBG) && defined(H3_FIFO)

//shadow registers for debugging
DWORD sLastHwPtr;
DWORD sInPacket;             // =0 expecting header, =1 not expecting hdr
DWORD sWordsLeftInPacket;    // # of words left in packet, counting header
DWORD sWrapping;
DWORD sLastPH;
DWORD sLastWR;
DWORD sCurrPH;
DWORD sCurrWR;


FxU32
countBits(FxU32 data)
{
    FxU32 nbits = 0;

    while (data != 0)
    {
      if ((data & 1) != 0)
         nbits += 1;

      data >>= 1;
    }
    return nbits;
}


void mySetCF(PDEV *ppdev, FxU32 *hwPtr, FxU32 hwIndex, FxU32 data);


void
mySetPH(PDEV *ppdev, FxU32 *hwPtr, FxU32 hwIndex, FxU32 data)
{
    FxU32 nWords;
    FxU32 jumpTo;

    //DISPDBG((1, "DF: PH%d - Addr=0x%08lx[%d] Val=0x%08lx \n", data & 0x7, hwPtr, hwIndex, data ));

    if (_FF(InPacket))
    {
      // Packet 5 has two packet headers (2 DWORD writes)
      if (_FF(CurrPH) != 5)
      {
        DISPDBG((0,"DF! Writing packet header while in packet \n"));
      }
      else
      {
        mySetCF(ppdev, hwPtr, hwIndex, data);
        _FF(CurrPH) = 0xfff;                // invalid packet header
        return;
      }
    }
    else
    {
      _FF(currPHHwPtr) = (FxU32)(hwPtr + hwIndex);
      _FF(currPHData) = data;
      _FF(CurrPH) = data & 0x07 ;
    }

    _FF(InPacket) = 1;

    switch (data & SSTCP_PKT)
    {
      case SSTCP_PKT0:
      switch (data & ~SSTCP_PKT0_ADDR)
      {
        case SSTCP_PKT0_JMP_LOCAL:
#if 0
        // make sure we're jumping back to top!
        //
        jumpTo = (data & SSTCP_PKT0_ADDR) >> SSTCP_PKT0_ADDR_SHIFT;
        jumpTo <<= 2;
        if (jumpTo != (_FF(cmdFifoStart) - _FF(lfbBase)))
            DISPDBG((0,"DF! Not jumping back to top \n"));
#else
        RIP( "DDraw used fifo jmp -- not expected\n" );
#endif

        break;

        default:
        {
            DISPDBG((0,"DF! Invalid packet 0 \n"));
        }
      }
      _FF(WordsLeftInPacket) = 1;
//      _FF(Wrapping) = 1;
      mySetCF(ppdev, hwPtr, hwIndex, data);
//      _FF(Wrapping) = 0;
      break;

      case SSTCP_PKT1:
      nWords = (data & SSTCP_PKT1_NWORDS) >> SSTCP_PKT1_NWORDS_SHIFT;
      if (nWords == 0)
//         DISPDBG((0,"DF! Packet 1 invalid number of words \n"));
         RIP("DF! Packet 1 invalid number of words \n");
      _FF(WordsLeftInPacket) = nWords + 1;
      mySetCF(ppdev, hwPtr, hwIndex, data);
      break;

      case SSTCP_PKT2:
      nWords = countBits(data & SSTCP_PKT2_MASK);
      if (nWords == 0)
//         DISPDBG((0,"DF! Packet 2 invalid number of words \n"));
         RIP("DF! Packet 2 invalid number of words \n");
      _FF(WordsLeftInPacket) = nWords + 1;
      mySetCF(ppdev, hwPtr, hwIndex, data);
      break;

      case SSTCP_PKT3:
      {
      FxU32 pmask = (data & SSTCP_PKT3_PMASK) >> SSTCP_PKT3_PMASK_SHIFT;
      nWords = 0;
      if (pmask & SST_SETUP_RGB)
      {
         if (data & SSTCP_PKT3_PACKEDCOLOR)
           ++nWords;
         else
         {
           if (data & SST_SETUP_A)
             nWords + 4;
           else nWords + 3;
         }
      }
      if (pmask & SST_SETUP_Z)
        ++ nWords;
      if (pmask & SST_SETUP_Wfbi)
        ++ nWords;
      if (pmask & SST_SETUP_W0)
        ++ nWords;
      if (pmask & SST_SETUP_ST0)
        nWords += 2;
      if (pmask & SST_SETUP_W1)
        ++ nWords;
      if (pmask & SST_SETUP_ST1)
        nWords += 2;
      if (nWords == 0)
//         DISPDBG((0,"DF! Packet 3 invalid number of parameters \n"));
         RIP("DF! Packet 3 invalid number of parameters \n");
      nWords += 2; // x & y for every vertex
      _FF(WordsLeftInPacket) = nWords * ((data & SSTCP_PKT3_NUMVERTEX) >> SSTCP_PKT3_NUMVERTEX_SHIFT) + 1;
      mySetCF(ppdev, hwPtr, hwIndex, data);
      }
      break;

      case SSTCP_PKT4:
      nWords = countBits(data & SSTCP_PKT4_MASK);
      if (nWords == 0)
//         DISPDBG((0,"DF! Packet 4 invalid number of words \n"));
         RIP("DF! Packet 4 invalid number of words \n");
      _FF(WordsLeftInPacket) = nWords + 1;
      mySetCF(ppdev, hwPtr, hwIndex, data);
      break;

       case SSTCP_PKT5:
      nWords = (data & SSTCP_PKT5_NWORDS) >> SSTCP_PKT5_NWORDS_SHIFT;
      if (nWords == 0)
//         DISPDBG((0,"DF! Packet5 invalid number of words \n"));
         RIP("DF! Packet5 invalid number of words \n");
      _FF(WordsLeftInPacket) = nWords + 2; // two packet headers
      mySetCF(ppdev, hwPtr, hwIndex, data);
      break;

      default:
         RIP("DF! Invalid packet \n");
    }

}


void
mySetCF(PDEV *ppdev, FxU32 *hwPtr, FxU32 hwIndex, FxU32 data)
{
    FxU32  rdPtr;

    //DISPDBG(( 1, "DF: PD   - Addr=0x%08lx[%d] Val=0x%08lx \n", hwPtr, hwIndex, data ));

    hwPtr += hwIndex;

//    rdPtr = LOAD_CMDFIFO_RDPTR() + _FF(cmdFifoOffset);
    rdPtr = LOAD_CMDFIFO_RDPTR() + (FxU32) ppdev->fifoData.fifoStart;
    if ((FxU32)hwPtr == rdPtr)
    {
    if (GET(ghwAC->cmdFifo0.depth) != 0)
      DISPDBG((0,"DF! Command fifo depth not zero \n"));
    }

//    if (_FF(cmdFifoSpace) & 0xFF000000)
//      DISPDBG((0,"DF! Command fifo space gone neg./too large \n"));

    // rdptr is incremented past the last word written before it is executed
    if (rdPtr > ((FxU32) ppdev->fifoData.fifoEnd)+4)
      DISPDBG((0,"DF! rdPtr past end of command fifo \n"));

    if (rdPtr < (FxU32) ppdev->fifoData.fifoStart)
      DISPDBG((0,"DF! rdPtr before start of command fifo \n"));

//    if (_FF(cmdFifoSpace) > (((_FF(cmdFifoEnd) - _FF(cmdFifoStart) + 8) / 4) - 2))
//      DISPDBG((0,"DF! cmdfifospace more free space than available \n"));

    if (GET(ghwAC->cmdFifo0.depth) > HW_CMDFIFO_TOTAL_SIZE )
      DISPDBG((0,"DF! command fifo depth larger than max depth \n"));

    if (!_FF(InPacket))
    {
      DISPDBG((0,"DF! writing command outside of packet \n"));
    }

    if (_FF(fifoDwordCount) == 0)
    {
//      DISPDBG((0,"DF! writing more words than expected \n"));
      RIP("DF! writing more words than expected \n");
    }

//    if ((_FF(cmdFifoSpace) <= 0) && !_FF(Wrapping))
//      DISPDBG((0,"DF! no command fifo space left & not wrapping \n"));

//    if ((FxU32)hwPtr != (_FF(LastHwPtr) + 4))
//        DISPDBG((0,"DF! hwptr farther ahead than last write \n"));

    if ((FxU32)hwPtr > (FxU32) ppdev->fifoData.fifoEnd)
      DISPDBG((0,"DF! hwptr past fifo end \n"));

    if ((FxU32)hwPtr < (FxU32) ppdev->fifoData.fifoStart)
      DISPDBG((0,"DF! hwptr less than the fifo start \n"));

    *hwPtr = data;
//    _FF(CurrWR) = 1;
//    _FF(LastHwPtr) += 4;

    _FF(WordsLeftInPacket) -= 1;
    if (_FF(WordsLeftInPacket) == 0)
    {
      _FF(InPacket) = 0;

      // last driver to finish a packet & it's packet
//      _FF(LastWR) = DX_SIGNATURE;
      _FF(LastPH) = _FF(CurrPH);
      _FF(lastPHHwPtr) = _FF(currPHHwPtr);
      _FF(lastPHData) = _FF(currPHData);
    }
    if (_FF(WordsLeftInPacket) < 0)
    {
      RIP( "Writing past end of packet -- U R Hung\n" );
    }

}

FxU32 trapOnDW = 0;

void
mySetDW(FxU32 hwPtr, FxU32 data)
{

    if (trapOnDW)
      DISPDBG((0,"DF! direct write when command fifo is on \n"));

    *(FxU32 *)hwPtr = data;
}


#ifdef CRASH
int hwPollStall ;
int poll        ;

void
PollStall()
{
   DWORD wentIdle;
   DWORD status;

   wentIdle=FALSE;

   if(hwPollStall)
   {
      while( (poll) && !(wentIdle) )
      {
         status= GET(ghw0->status);
         if( !(  status & SST_BUSY  ) )
         {
            wentIdle=TRUE;
         }
      }
   }
}
#endif

#endif // #if defined(DBG) && defined(H3_FIFO)

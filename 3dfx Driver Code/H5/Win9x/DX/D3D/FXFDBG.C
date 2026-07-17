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
** File name:   fxdbg.c
**
** Description: Miscellaneous routines
**
** $Revision: 2$
** $Date: 10/11/00 8:48:09 PM$
**
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 3     6/21/99 12:44p Andrew
** removed HAL_CSIM
** 
** 2     6/04/99 1:09p Cwilcox
** Removed #ifdef H3_AGP_WORKAROUND code.
** 
** 1     6/02/99 6:46a Michael
** Branch from H3
** 
** 11    1/26/99 5:30p Peterm
** Added unified header information
** 
** 10    12/18/98 9:15p Andrew
** Make code changes to allow to compile with DF=1
** 
** 9     11/22/98 9:14p Andrew
** Changes to support multi-monitor
** 
** 8     11/04/98 2:16a Artg
** mysetcf and my setdf now uses h3write32 for csim builds.
** Allows DF option to work
** 
** 7     10/29/98 2:12p Martin
** Modification of cmdFifo macros to support future changes for
** super-sampling AA.
** 
** 6     7/18/98 6:41p Ken
** added ability to use cmdfifo1 as the primary command fifo, #define
** PRIMARY_CMDFIFO at the top of inc\shared.h
** 
** 5     6/03/98 2:32p Miriam
** 3D performance enhancements (debugging also updated).
** 
** 4     5/07/98 11:11a Artg
** mysetph and mysetcf saves the data and hwptr from previous setph.
** 
** 3     4/21/98 12:01a Ken
** added agp workaround (set ar=1) for readback unreliability
** 
** 2     4/15/98 4:59p Suninn
** move dwordCount into shared.h
** 
** 1     3/31/98 11:35a Miriam
** command fifo debugging.
*/
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h" 
#include "fifomgr.h"

#ifdef DEBUGFIFO

//shadow registers for debugging
DWORD sLastHwPtr;
DWORD sInPacket;             // =0 expecting header, =1 not expecting hdr
DWORD sWordsLeftInPacket;    // # of words left in packet, counting header
DWORD sWrapping;
DWORD sLastPH;
DWORD sLastWR;
DWORD sCurrPH;
DWORD sCurrWR;

/*-------------------------------------------------------------------
Function Name:  dfgpf

Description:    printf-like OutputDebugString

Return:         void
-------------------------------------------------------------------*/
void __cdecl dfgpf(NT9XDEVICEDATA * ppdev, int debugPrintLevel, LPSTR szFormat, ...)
{
    char    str[256];
    #define START_STR       "3dfxD3D: "

    sLastHwPtr        = _FF(LastHwPtr) ;
    sInPacket         = _FF(InPacket) ;        
    sWordsLeftInPacket= _FF(WordsLeftInPacket) ;
    sWrapping         = _FF(Wrapping) ;
    sLastPH           = _FF(LastPH) ;
    sLastWR           = _FF(LastWR) ;
    sCurrPH           = _FF(CurrPH) ;
    sCurrWR           = _FF(CurrWR) ;

    lstrcpy(str, START_STR);
    wvsprintf(str+lstrlen(str), szFormat, (LPVOID)(&szFormat+1));
    if (debugPrintLevel < 1)
      OutputDebugString(str);

}



/*-------------------------------------------------------------------
Function Name:  countBits

Description:    returns the number of bits set in a passed in 32-bit
                word

Return:         FxU32 containing number of bits set
-------------------------------------------------------------------*/
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


/*-------------------------------------------------------------------
Function Name:  ResetHwPtr

Description:    Resets the hardware pointer

Return:         void
-------------------------------------------------------------------*/
void
ResetHwPtr(NT9XDEVICEDATA * ppdev, FxU32 *hwPtr)
{
    _FF(LastHwPtr) = (FxU32) (hwPtr - 1);
}


void mySetCF(NT9XDEVICEDATA * ppdev, FxU32 *hwPtr, FxU32 hwIndex, FxU32 data);


/*-------------------------------------------------------------------
Function Name:  mySetPH

Description:    Setup accelerator for a packet transfer

Return:         void
-------------------------------------------------------------------*/
void mySetPH(NT9XDEVICEDATA * ppdev, FxU32 *hwPtr, FxU32 hwIndex, FxU32 data)
{
    FxU32 nWords;
    FxU32 jumpTo;

    //dfgpf(ppdev,1, "DF: PH%d - Addr=0x%08lx[%d] Val=0x%08lx \n", data & 0x7, hwPtr, hwIndex, data );
    
    if (_FF(InPacket))
    {
      // Packet 5 has two packet headers (2 DWORD writes)  
      if (_FF(CurrPH) != 5)
      {
        dfgpf(ppdev,0,"DF! Writing packet header while in packet \n"); 
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
        // make sure we're jumping back to top!
        //
        jumpTo = (data & SSTCP_PKT0_ADDR) >> SSTCP_PKT0_ADDR_SHIFT;
        jumpTo <<= 2;
        if (jumpTo != (CMDFIFOSTART - _FF(lfbBase)))
            dfgpf(ppdev,0,"DF! Not jumping back to top \n");
        break;

        default:
        { 
            dfgpf(ppdev,0,"DF! Invalid packet 0 \n");
        }
      }
      _FF(WordsLeftInPacket) = 1;
      _FF(Wrapping) = 1;
      mySetCF(ppdev, hwPtr, hwIndex, data);
      _FF(Wrapping) = 0;
      break;

      case SSTCP_PKT1:
      nWords = (data & SSTCP_PKT1_NWORDS) >> SSTCP_PKT1_NWORDS_SHIFT;
      if (nWords == 0)
         dfgpf(ppdev,0,"DF! Packet 1 invalid number of words \n");
      _FF(WordsLeftInPacket) = nWords + 1;
      mySetCF(ppdev, hwPtr, hwIndex, data);
      break;

      case SSTCP_PKT2:
      nWords = countBits(data & SSTCP_PKT2_MASK);
      if (nWords == 0)
         dfgpf(ppdev,0,"DF! Packet 2 invalid number of words \n");
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
         dfgpf(ppdev,0,"DF! Packet 3 invalid number of parameters \n");
      nWords += 2; // x & y for every vertex
      _FF(WordsLeftInPacket) = nWords * ((data & SSTCP_PKT3_NUMVERTEX) >> SSTCP_PKT3_NUMVERTEX_SHIFT) + 1;
      mySetCF(ppdev, hwPtr, hwIndex, data);
      }
      break;

      case SSTCP_PKT4:
      nWords = countBits(data & SSTCP_PKT4_MASK);
      if (nWords == 0)
         dfgpf(ppdev,0,"DF! Packet 4 invalid number of words \n");
      _FF(WordsLeftInPacket) = nWords + 1;
      mySetCF(ppdev, hwPtr, hwIndex, data);
      break;
 
       case SSTCP_PKT5:
      nWords = (data & SSTCP_PKT5_NWORDS) >> SSTCP_PKT5_NWORDS_SHIFT;
      if (nWords == 0)
         dfgpf(ppdev,0,"DF! Packet5 invalid number of words \n");
      _FF(WordsLeftInPacket) = nWords + 2; // two packet headers
      mySetCF(ppdev, hwPtr, hwIndex, data);
      break;
      
      default:
         dfgpf(ppdev,0,"DF! Invalid packet \n");
    }
  
}


/*-------------------------------------------------------------------
Function Name:  mySetCF

Description:    Manage the command fifo depth

Return:         void
-------------------------------------------------------------------*/
void
mySetCF(NT9XDEVICEDATA * ppdev, FxU32 *hwPtr, FxU32 hwIndex, FxU32 data)
{
    FxU32  rdPtr;
    
    //dfgpf(ppdev, 1, "DF: PD   - Addr=0x%08lx[%d] Val=0x%08lx \n", hwPtr, hwIndex, data );
    
    hwPtr += hwIndex;

    rdPtr = LOAD_CMDFIFO_RDPTR(ghwAC) + CMDFIFOOFFSET;
    if ((FxU32)hwPtr == rdPtr)
    {
    if (GET(ghwAC->PRIMARY_CMDFIFO.depth) != 0)
      dfgpf(ppdev,0,"DF! Command fifo depth not zero \n");
    }

    if (CMDFIFOSPACE & 0xFF000000)
      dfgpf(ppdev,0,"DF! Command fifo space gone neg./too large \n");

    // rdptr is incremented past the last word written before it is executed
    if (rdPtr > (CMDFIFOEND+4))
      dfgpf(ppdev,0,"DF! rdPtr past end of command fifo \n");

    if (rdPtr < CMDFIFOSTART)
      dfgpf(ppdev,0,"DF! rdPtr before start of command fifo \n");

    if (CMDFIFOSPACE > (((CMDFIFOEND - CMDFIFOSTART + 8) / 4) - 2))
      dfgpf(ppdev,0,"DF! cmdfifospace more free space than available \n");

    if (GET(ghwAC->PRIMARY_CMDFIFO.depth) > (CMDFIFOEND - CMDFIFOSTART))
      dfgpf(ppdev,0,"DF! command fifo depth larger than max depth \n");
    
    if (!_FF(InPacket))
    {
      dfgpf(ppdev,0,"DF! writing command outside of packet \n");
    }

    if (_FF(fifoDwordCount) == 0)
    {
      dfgpf(ppdev,0,"DF! writing more words than expected \n");
    }
    
    if ((CMDFIFOSPACE <= 0) && !_FF(Wrapping))
      dfgpf(ppdev,0,"DF! no command fifo space left & not wrapping \n");

    if ((FxU32)hwPtr != (_FF(LastHwPtr) + 4))
        dfgpf(ppdev,0,"DF! hwptr farther ahead than last write \n");

    if ((FxU32)hwPtr > CMDFIFOEND)
      dfgpf(ppdev,0,"DF! hwptr past fifo end \n");

    if ((FxU32)hwPtr < CMDFIFOSTART)
      dfgpf(ppdev,0,"DF! hwptr less than the fifo start \n");
    
    *hwPtr = data;
    _FF(CurrWR) = 1;
    _FF(LastHwPtr) += 4;

    _FF(WordsLeftInPacket) -= 1;
    if (_FF(WordsLeftInPacket) == 0)
    {
      _FF(InPacket) = 0;

      // last driver to finish a packet & it's packet
      _FF(LastWR) = DX_SIGNATURE;
      _FF(LastPH) = _FF(CurrPH);
      _FF(lastPHHwPtr) = _FF(currPHHwPtr);
      _FF(lastPHData) = _FF(currPHData);
    }
}

FxU32 trapOnDW = 0;

/*-------------------------------------------------------------------
Function Name:  mySetDW

Description:    Write a FxU32 to the command fifo

Return:         void
-------------------------------------------------------------------*/
void
mySetDW(NT9XDEVICEDATA * ppdev, FxU32 hwPtr, FxU32 data)
{
    
    if (trapOnDW)
      dfgpf(ppdev,0,"DF! direct write when command fifo is on \n");

    *(FxU32 *)hwPtr = data;

}
#endif // #ifdef DEBUGFIFO

#ifdef CRASH
int hwPollStall ;
int poll        ;

/*-------------------------------------------------------------------
Function Name:  PollStall

Description:    Stall while status register indicates graphics busy

Return:         void
-------------------------------------------------------------------*/
void
PollStall(NT9XDEVICEDATA * ppdev)
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


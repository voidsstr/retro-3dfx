/* $Header: fifodbg.c, 3, 10/11/00 8:55:32 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File Name: 	fifomgr.c
**
** Description: Hardware access functions for Voodoo3/Napalm.
**
** $Revision: 3$
** $Date: 10/11/00 8:55:32 PM$
**
*/

#ifdef DEBUGFIFO

#ifdef IS_32
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h" 
#include "fifomgr.h"
#else
#ifdef IS_16
#include "h3.h"
#include "header.h"
#else
#include "thunk32.h"
#include "h3g.h"
#endif
#endif

//shadow registers for debugging

DWORD sInPacket;             // =0 expecting header, =1 not expecting header
DWORD sWordsLeftInPacket;    // # of words left in packet, counting header
DWORD sWrapping;
DWORD sLastPH;
DWORD sLastWR;
DWORD sCurrPH;
DWORD sCurrWR;

#ifndef IS_32

// Command FIFO debug functions for 16-bit display driver and 32-bit minivdd.

// Define arguments for DPF function

#ifdef IS_16
#define DBGFLAGS DBGLVL_NORMAL
#else
#define DBGFLAGS DBG_DEBUG, 128
#endif

void GPF()
{
    sInPacket         = _FF(InPacket) ;        
    sWordsLeftInPacket= _FF(WordsLeftInPacket) ;
    sWrapping         = _FF(Wrapping) ;
    sLastPH           = _FF(LastPH) ;
    sLastWR           = _FF(LastWR) ;
    sCurrPH           = _FF(CurrPH) ;
    sCurrWR           = _FF(CurrWR) ;
}

void mySetPH(FxU32 hwPtr, FxU32 hwIndex, FxU32 data)
{
  FxU32 temp;
  FxU32 nWords;
  FxU32 jumpTo;    

  DEBUG_FIX;

  if (_FF(InPacket))
  {
    DPF(DBGFLAGS, "DF! Writing packet header while in packet \n"); 
    GPF();
  }
    
  // Update shadow registers for debugging.

  _FF(InPacket) = 1;
  _FF(CurrPH) = data ;
  _FF(currPHHwPtr)=hwPtr+hwIndex;
  _FF(currPHData) = data;

  // Check for valid packet header.

  switch (data & SSTCP_PKT)
  {
    case SSTCP_PKT0:  if (SSTCP_PKT0_JMP_LOCAL != (data & SSTCP_PKT0_FUNC))
                      {
                        DPF(DBGFLAGS, "DF! Invalid packet 0 \n");
                        GPF();
                      }

                      // make sure we're jumping back to top!
     
                      jumpTo = ((data & SSTCP_PKT0_ADDR) >> SSTCP_PKT0_ADDR_SHIFT) << 2;
                      if (jumpTo != (CMDFIFOSTART - _FF(lfbBase)))
                      {
                        DPF(DBGFLAGS, "DF! Not jumping back to top \n");
                        GPF();
                      }

                      _FF(WordsLeftInPacket) = 1;
                      _FF(Wrapping) = 1;
                      mySetCF (hwPtr, hwIndex, data);
                       _FF(Wrapping) = 0;

                      break;

    case SSTCP_PKT1:  nWords = (data & SSTCP_PKT1_NWORDS) >> SSTCP_PKT1_NWORDS_SHIFT;
                      if (nWords == 0)
                      {
                        DPF(DBGFLAGS, "DF! Packet 1 invalid number of words \n");
                        GPF();
                      }

                      _FF(WordsLeftInPacket) = nWords + 1;
                      mySetCF (hwPtr, hwIndex, data);

                      break;

    case SSTCP_PKT2:  temp = data & SSTCP_PKT2_MASK;

                      // Count number of words in packet.
                      nWords = 0;
                      while (temp != 0)
                      {
                        if (temp & 1) nWords++;
                        temp >>= 1;
                      }

                      if (nWords == 0)
                      {
                        DPF(DBGFLAGS, "DF! Packet 2 invalid number of words \n");
                        GPF();
                      }

                      _FF(WordsLeftInPacket) = nWords + 1;
                      mySetCF (hwPtr, hwIndex, data);

                      break;
      
    default:          DPF(DBGFLAGS, "DF! Invalid packet \n");
                      GPF();
  }
}


void mySetCF(FxU32 hwPtr, FxU32 hwIndex, FxU32 data)
{
    FxU32 rdPtr;
    
    DEBUG_FIX;
    
    hwPtr += hwIndex * 4;

    rdPtr = LOAD_CMDFIFO_RDPTR(_FF(lpCRegs)) + CMDFIFOOFFSET;
    if (hwPtr == rdPtr)
    {
      if (GET(_FF(lpCRegs)->PRIMARY_CMDFIFO.depth) != 0)
      {
        DPF(DBGFLAGS, "DF! Command fifo depth not zero \n");
        GPF();
      }
    }

    if (CMDFIFOSPACE & 0xFF000000)
    {
      DPF(DBGFLAGS, "DF! Command fifo space gone neg./too large \n");
      GPF();
    }

    if (rdPtr > (CMDFIFOEND+4))
    {
      DPF(DBGFLAGS, "DF! rdPtr past end of command fifo \n");
      GPF();
    }

    if (rdPtr < CMDFIFOSTART)
    {
      DPF(DBGFLAGS, "DF! rdPtr before start of command fifo \n");
      GPF();
    }

    if (CMDFIFOSPACE > (((CMDFIFOEND - CMDFIFOSTART + 8) / 4) - 2))
    {
      DPF(DBGFLAGS, "DF! cmdfifospace more free space than available \n");
      GPF();
    }

    if (GET(_FF(lpCRegs)->PRIMARY_CMDFIFO.depth) > (CMDFIFOEND - CMDFIFOSTART))
    {
      DPF(DBGFLAGS, "DF! command fifo depth larger than max depth \n");
      GPF();
    }
    
    if (!_FF(InPacket))
    {
      DPF(DBGFLAGS, "DF! writing command outside of packet \n");
      GPF();
    }

    if (_FF(fifoDwordCount) == 0)
    {
      DPF(DBGFLAGS, "DF! writing more words than expected \n");
      GPF();
    }

    if ((CMDFIFOSPACE <= 0) && !_FF(Wrapping))
    {
      DPF(DBGFLAGS, "DF! no command fifo space left & not wrapping \n");
      GPF();
    }

    if (hwPtr > CMDFIFOEND)
    {
      DPF(DBGFLAGS, "DF! hwptr past fifo end \n");
      GPF();
    }

    if (hwPtr < CMDFIFOSTART)
    {
      DPF(DBGFLAGS, "DF! hwptr less than the fifo start \n");
      GPF();
    }

    #ifdef IS_16
    h3WRITE(NULL, (DWORD *)hwPtr, data);    
    #else
    *(FxU32 *)hwPtr = data;
    #endif

    _FF(CurrWR)    = 2;
    _FF(WordsLeftInPacket) -= 1;
    if (_FF(WordsLeftInPacket) == 0)
    {
      _FF(InPacket) = 0;

      // last driver to finish a packet & it's packet

      #ifdef IS_16
      _FF(LastWR) = DD16_SIGNATURE;
      #else
      _FF(LastWR) = TH32_SIGNATURE;
	  #endif

      _FF(LastPH) = _FF(CurrPH);
      _FF(lastPHHwPtr) = _FF(currPHHwPtr);
      _FF(lastPHData) = _FF(currPHData);

    }
}

#else // IS_32

// Command FIFO debug functions for 32-bit DirectDraw and Direct3D code.

void __cdecl GPF(NT9XDEVICEDATA * ppdev, int debugPrintLevel, LPSTR szFormat, ...)
{
    char    str[256];
    #define START_STR       "3dfxD3D: "

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
Function Name:  mySetPH

Description:    Setup accelerator for a packet transfer

Return:         void
-------------------------------------------------------------------*/

void mySetPH(NT9XDEVICEDATA * ppdev, FxU32 *hwPtr, FxU32 hwIndex, FxU32 data)
{
  FxU32 temp;
  FxU32 pmask;
  FxU32 nWords;
  FxU32 jumpTo;

  if (_FF(InPacket))
  {
    // Packet 5 has two packet headers (2 DWORD writes)  
    if (_FF(CurrPH) != 5)
    {
      GPF(ppdev,0,"DF! Writing packet header while in packet \n"); 
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
    case SSTCP_PKT0:  switch (data & ~SSTCP_PKT0_ADDR)
                      {
                        case SSTCP_PKT0_JMP_LOCAL:

                           // make sure we're jumping back to top!

                           jumpTo = ((data & SSTCP_PKT0_ADDR) >> SSTCP_PKT0_ADDR_SHIFT) << 2;
                           if (jumpTo != (CMDFIFOSTART - _FF(lfbBase)))
                           {
                             GPF(ppdev,0,"DF! Not jumping back to top \n");
                           }
                           break;

                        default:
                           {
                             GPF(ppdev,0,"DF! Invalid packet 0 \n");
                           }
                      }

                      _FF(WordsLeftInPacket) = 1;
                      _FF(Wrapping) = 1;
                      mySetCF(ppdev, hwPtr, hwIndex, data);
                      _FF(Wrapping) = 0;

                      break;

    case SSTCP_PKT1:  nWords = (data & SSTCP_PKT1_NWORDS) >> SSTCP_PKT1_NWORDS_SHIFT;

                      if (nWords == 0)
                        GPF(ppdev,0,"DF! Packet 1 invalid number of words \n");

                      _FF(WordsLeftInPacket) = nWords + 1;
                      mySetCF(ppdev, hwPtr, hwIndex, data);

                      break;

    case SSTCP_PKT2:  temp = data & SSTCP_PKT2_MASK;

                      // Count number of words in packet.
                      nWords = 0;
                      while (temp != 0)
                      {
                        if (temp & 1) nWords++;
                        temp >>= 1;
                      }

                      if (nWords == 0)
                        GPF(ppdev,0,"DF! Packet 2 invalid number of words \n");

                      _FF(WordsLeftInPacket) = nWords + 1;
                      mySetCF(ppdev, hwPtr, hwIndex, data);

                      break;

    case SSTCP_PKT3:  pmask = (data & SSTCP_PKT3_PMASK) >> SSTCP_PKT3_PMASK_SHIFT;
                      nWords = 0;
                      if (pmask & SST_SETUP_RGB)
                      {
                        if (data & SSTCP_PKT3_PACKEDCOLOR)
                          nWords++;
                        else if (data & SST_SETUP_A)
                          nWords += 4;
                        else nWords += 3;  
                      }
                      if (pmask & SST_SETUP_Z)
                        nWords++;
                      if (pmask & SST_SETUP_Wfbi)
                        nWords++;
                      if (pmask & SST_SETUP_W0)
                        nWords++;
                      if (pmask & SST_SETUP_W1)
                        nWords++;
                      if (pmask & SST_SETUP_ST0)
                        nWords += 2;
                      if (pmask & SST_SETUP_ST1)
                        nWords += 2;
                      if (nWords == 0)
                        GPF(ppdev,0,"DF! Packet 3 invalid number of parameters \n");
                      nWords += 2;  // X and Y per vertex

                      _FF(WordsLeftInPacket) = nWords * ((data & SSTCP_PKT3_NUMVERTEX) >> SSTCP_PKT3_NUMVERTEX_SHIFT) + 1;
                      mySetCF(ppdev, hwPtr, hwIndex, data);

                      break;

    case SSTCP_PKT4:  temp = data & SSTCP_PKT4_MASK;

                      // Count number of words in packet.
                      nWords = 0;
                      while (temp != 0)
                      {
                        if (temp & 1) nWords++;
                        temp >>= 1;
                      }

                      if (nWords == 0)
                        GPF(ppdev,0,"DF! Packet 4 invalid number of words \n");

                      _FF(WordsLeftInPacket) = nWords + 1;
                      mySetCF(ppdev, hwPtr, hwIndex, data);

                      break;
 
    case SSTCP_PKT5:  nWords = (data & SSTCP_PKT5_NWORDS) >> SSTCP_PKT5_NWORDS_SHIFT;

                      if (nWords == 0)
                        GPF(ppdev,0,"DF! Packet 5 invalid number of words \n");

                      _FF(WordsLeftInPacket) = nWords + 2; // 2 packet headers
                      mySetCF(ppdev, hwPtr, hwIndex, data);

                      break;
      
    default:          GPF(ppdev,0,"DF! Invalid packet type \n");
  }
}

/*-------------------------------------------------------------------
Function Name:  mySetCF

Description:    Manage the command fifo depth

Return:         void
-------------------------------------------------------------------*/

void mySetCF(NT9XDEVICEDATA * ppdev, FxU32 *hwPtr, FxU32 hwIndex, FxU32 data)
{
  FxU32  rdPtr;
    
  hwPtr += hwIndex;

  rdPtr = LOAD_CMDFIFO_RDPTR(ghwAC) + CMDFIFOOFFSET;
  if ((FxU32)hwPtr == rdPtr)
  {
    if (GET(ghwAC->PRIMARY_CMDFIFO.depth) != 0)
      GPF(ppdev,0,"DF! Command fifo depth not zero \n");
  }

  if (CMDFIFOSPACE & 0xFF000000)
    GPF(ppdev,0,"DF! Command fifo space gone neg./too large \n");

  if (rdPtr > (CMDFIFOEND+4))
    GPF(ppdev,0,"DF! rdPtr past end of command fifo \n");

  if (rdPtr < CMDFIFOSTART)
    GPF(ppdev,0,"DF! rdPtr before start of command fifo \n");

  if (CMDFIFOSPACE > (((CMDFIFOEND - CMDFIFOSTART + 8) / 4) - 2))
    GPF(ppdev,0,"DF! cmdfifospace more free space than available \n");

  if (GET(ghwAC->PRIMARY_CMDFIFO.depth) > (CMDFIFOEND - CMDFIFOSTART))
    GPF(ppdev,0,"DF! command fifo depth larger than max depth \n");
    
  if (!_FF(InPacket))
    GPF(ppdev,0,"DF! writing command outside of packet \n");

  if (_FF(fifoDwordCount) == 0)
    GPF(ppdev,0,"DF! writing more words than expected \n");
    
  if ((CMDFIFOSPACE <= 0) && !_FF(Wrapping))
    GPF(ppdev,0,"DF! no command fifo space left & not wrapping \n");

  if ((FxU32)hwPtr > CMDFIFOEND)
    GPF(ppdev,0,"DF! hwptr past fifo end \n");

  if ((FxU32)hwPtr < CMDFIFOSTART)
    GPF(ppdev,0,"DF! hwptr less than the fifo start \n");
    
  *hwPtr = data;
  _FF(CurrWR) = 1;

  _FF(WordsLeftInPacket) -= 1;
  if (_FF(WordsLeftInPacket) == 0)
  {
    _FF(InPacket) = 0;

    _FF(LastWR) = DX_SIGNATURE;
    _FF(LastPH) = _FF(CurrPH);
    _FF(lastPHHwPtr) = _FF(currPHHwPtr);
    _FF(lastPHData) = _FF(currPHData);
  }
}

#endif // IS_32

#endif // DEBUGFIFO

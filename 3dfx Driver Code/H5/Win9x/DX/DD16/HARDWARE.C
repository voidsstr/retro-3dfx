/* $Header: hardware.c, 2, 10/11/00 8:51:19 PM, Brent$ */
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
** File name:   hardware.c
**
** Description: This file is not used!
**
** $Revision: 2$ 
** $Date: 10/11/00 8:51:19 PM$
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 10    12/29/98 1:18p Michael
** Implement the 3Dfx/STB unified header.
** 
** 9     10/29/98 2:13p Martin
** Modification of cmdFifo macros to support future changes for
** super-sampling AA.
** 
** 8     7/18/98 6:41p Ken
** added ability to use cmdfifo1 as the primary command fifo, #define
** PRIMARY_CMDFIFO at the top of inc\shared.h
** 
** 7     4/28/98 6:15p Artg
** fifo debug saves previous packet header address and data
** 
** 6     4/28/98 3:57p Michael
** change Msg() to include a new parameter that will allow for selective
** debug message output.
** 
** 5     4/16/98 11:46p Andrew
** Another attempt to fix internal compiler error
** 
** 4     4/16/98 2:05p Suninn
** somehow the compiler like to have a space, check you the last version
** and you will see.  without a space, I get error inside a compiler
** file...#$#@!$@#
** 
** 3     4/16/98 10:28a Suninn
** move dwordCount into shared.h fifoDwordCount
** 
** 2     4/13/98 4:46p Andrew
** Rearranged the code so that the compiler would not get a internal error
** and choke.
**
*/

/********************************************************************************
*
* The file hardware.c implements low level DEBUGFIFO routines.
* This code was taken from minivdd\hardware.c.  A few minor modification
* were made to run in a 16-bit environment.
*
*
*********************************************************************************/

#ifdef DEBUGFIFO

#include "h3.h"
#include "header.h"


FxU32
countBits(FxU32 data)
{
    FxU32 nbits;

    DEBUG_FIX;

    nbits = 0;
    
    while (data != 0)
    {
    if ((data & 1) != 0)
        nbits += 1;

    data >>= 1;
    }

    return nbits;
}

FxU32 doStress = 0;

#ifdef STRESS0
void
stress()
{
    CMDFIFO_PROLOG(cmdFifo);

    DEBUG_FIX;

    CMDFIFO_SETUP(cmdFifo);
    
    while (doStress)
    {
    CMDFIFO_CHECKROOM(cmdFifo, 4);
    SETPH(cmdFifo,
          SSTCP_PKT2 |      
          dstSizeBit |
          dstXYBit |
          commandBit);
    SET(cmdFifo, _FF(lpGRegs)->dstSize, 0x01000100);
    SET(cmdFifo, _FF(lpGRegs)->dstXY, 0);
    SET(cmdFifo, _FF(lpGRegs)->command, 0x66000105);
    BUMP(4);
    }

    CMDFIFO_EPILOG(cmdFifo);
}
#else  // #ifdef STRESS0

FxU32 ssBC = 0;

void
stress()
{
    CMDFIFO_PROLOG(cmdFifo);

    DEBUG_FIX;

    CMDFIFO_SETUP(cmdFifo);
    
    while (doStress)
    {
    CMDFIFO_CHECKROOM(cmdFifo, 7);
    SETPH(cmdFifo, 0x39c00002);
    SET(cmdFifo, _FF(lpGRegs)->srcFormatBit, 0x00030500);
    SET(cmdFifo, _FF(lpGRegs)->srcSize, 0x008d00b2);
    SET(cmdFifo, _FF(lpGRegs)->srcXY, 0x00b20000);
    SET(cmdFifo, _FF(lpGRegs)->dstSize, 0x008d00b2);
    SET(cmdFifo, _FF(lpGRegs)->dstXY, 0x015c0001);
    SET(cmdFifo, _FF(lpGRegs)->command, 0xcc000101);
    BUMP(7);

    if (ssBC > 0)
    {
        ssBC -= 1;
        if (ssBC == 0)
        __asm int 3;
    }
    }

    CMDFIFO_EPILOG(cmdFifo);
}


#endif // #ifdef STRESS0

//shadow registers for debugging
DWORD sLastHwPtr;
DWORD sInPacket;             // =0 expecting header, =1 not expecting hdr
DWORD sWordsLeftInPacket;    // # of words left in packet, counting header
DWORD sWrapping;
DWORD sLastPH;
DWORD sLastWR;
DWORD sCurrPH;
DWORD sCurrWR;

STOP() {__asm int 3}

void
dfgpf()
{

    STOP();    
    sLastHwPtr        = _FF(LastHwPtr) ;
    sInPacket         = _FF(InPacket) ;        
    sWordsLeftInPacket= _FF(WordsLeftInPacket) ;
    sWrapping         = _FF(Wrapping) ;
    sLastPH           = _FF(LastPH) ;
    sLastWR           = _FF(LastWR) ;
    sCurrPH           = _FF(CurrPH) ;
    sCurrWR           = _FF(CurrWR) ;
}



void
ResetHwPtr(FxU32 hwPtr)
{
    DEBUG_FIX;
    
    _FF(LastHwPtr) = hwPtr - 4;
}


void mySetCF(FxU32 hwPtr, FxU32 hwIndex, FxU32 data);

void FixError(FxU32 hwPtr, FxU32 hwIndex, FxU32 data)
{
    FxU32 jumpTo;

    if (SSTCP_PKT0_JMP_LOCAL != (data & ~SSTCP_PKT0_ADDR))
      {
      DPF(DBGLVL_NORMAL, "DF! Invalid packet 0 \n");
      GPF();
      }

    // make sure we're jumping back to top!
    //

    jumpTo = (data & 0x1FFFFFC0) >> 4;
    if (jumpTo != (CMDFIFOSTART - _FF(lfbBase)))
       {
       DPF(DBGLVL_NORMAL, "DF! Not jumping back to top \n");
       GPF();
       }

   _FF(WordsLeftInPacket) = 1;
   _FF(Wrapping) = 1;
   mySetCF(hwPtr, hwIndex, data);
   _FF(Wrapping) = 0;
}

void mySetPH(FxU32 hwPtr, FxU32 hwIndex, FxU32 data)
{
    FxU32 nWords;
    
    DEBUG_FIX;

    // can print the packet header 
    // DPF(DBGLVL_NORMAL, DBG_DEBUG, 128, "DF: PH%d - Addr=0x%08lx[%d] Val=0x%08lx \n", data & 0x7, hwPtr, hwIndex, data );

    if (_FF(InPacket))
    {
      DPF(DBGLVL_NORMAL, "DF! Writing packet header while in packet \n"); 
      GPF();
    }
    
    _FF(InPacket) = 1;
    _FF(CurrPH) = data ;
    
    _FF(currPHHwPtr)=hwPtr+hwIndex;
    _FF(currPHData) = data;
    switch (data & SSTCP_PKT)
    {
      case SSTCP_PKT0:
      FixError(hwPtr, hwIndex, data);
      break;

      case SSTCP_PKT1:
      nWords = (data & SSTCP_PKT1_NWORDS) >> SSTCP_PKT1_NWORDS_SHIFT;
      if (nWords == 0)
      {
          DPF(DBGLVL_NORMAL, "DF! Packet 1 invalid number of words \n");
          GPF();
      }
      _FF(WordsLeftInPacket) = nWords + 1;
      mySetCF(hwPtr, hwIndex, data);
      break;

      case SSTCP_PKT2:
      nWords = countBits(data & SSTCP_PKT2_MASK);
      if (nWords == 0)
      {
          DPF(DBGLVL_NORMAL, "DF! Packet 2 invalid number of words \n");
          GPF();
      }
      _FF(WordsLeftInPacket) = nWords + 1;
      mySetCF(hwPtr, hwIndex, data);
      break;
      
      default:
      {
          DPF(DBGLVL_NORMAL, "DF! Invalid packet \n");
          GPF();
      }

    }
}


void mySetCF(FxU32 hwPtr, FxU32 hwIndex, FxU32 data)
{
    FxU32 rdPtr;
    
    DEBUG_FIX;
    
    // really dump data
    //DPF(DBGLVL_NORMAL, DBG_DEBUG, 128, "DF: PD   - Addr=0x%08lx[%d] Val=0x%08lx \n", hwPtr, hwIndex, data );
    hwPtr += hwIndex * 4;

    rdPtr = LOAD_CMDFIFO_RDPTR(_FF(lpCRegs)) + CMDFIFOOFFSET;
    if (hwPtr == rdPtr)
    {
    if (GET(_FF(lpCRegs)->PRIMARY_CMDFIFO.depth) != 0)
    {
        DPF(DBGLVL_NORMAL, "DF! Command fifo depth not zero \n");
        GPF();
    }
    }

    if (CMDFIFOSPACE & 0xFF000000)
    {
    DPF(DBGLVL_NORMAL, "DF! Command fifo space gone neg./too large \n");
    GPF();
    }
/*
    if (GET(_FF(lpIOregs)->status) & SST_CMD0_BUSY)
    GPF();
*/

    // rdptr is incremented past the last word written before it is executed
    if (rdPtr > (CMDFIFOEND+4))
    {
    DPF(DBGLVL_NORMAL, "DF! rdPtr past end of command fifo \n");
    GPF();
    }

    if (rdPtr < CMDFIFOSTART)
    {
    DPF(DBGLVL_NORMAL, "DF! rdPtr before start of command fifo \n");
    GPF();
    }

    if (CMDFIFOSPACE > (((CMDFIFOEND - CMDFIFOSTART + 8) / 4) - 2))
    {
    DPF(DBGLVL_NORMAL, "DF! cmdfifospace more free space than available \n");
    GPF();
    }

    if (GET(_FF(lpCRegs)->PRIMARY_CMDFIFO.depth) >
    (CMDFIFOEND - CMDFIFOSTART))
    {
    DPF(DBGLVL_NORMAL, "DF! command fifo depth larger than max depth \n");
    GPF();
    }
    
    if (!_FF(InPacket))
    {
    DPF(DBGLVL_NORMAL, "DF! writing command outside of packet \n");
    GPF();
    }

    if (_FF(fifoDwordCount) == 0)
    {
    DPF(DBGLVL_NORMAL, "DF! writing more words than expected \n");
    GPF();
    }

    if ((CMDFIFOSPACE <= 0) && !_FF(Wrapping))
    {
    DPF(DBGLVL_NORMAL, "DF! no command fifo space left & not wrapping \n");
    GPF();
    }

    if (hwPtr != (_FF(LastHwPtr) + 4))
    {
    DPF(DBGLVL_NORMAL, "DF! hwptr farther ahead than last write \n");
    GPF();
    }

    if (hwPtr > CMDFIFOEND)
    {
    DPF(DBGLVL_NORMAL, "DF! hwptr past fifo end \n");
    GPF();
    }

    if (hwPtr < CMDFIFOSTART)
    {
    DPF(DBGLVL_NORMAL, "DF! hwptr less than the fifo start \n");
    GPF();
    }
#ifdef IS_16
    h3WRITE(NULL, (DWORD *)hwPtr, data);    
#else
    *(FxU32 *)hwPtr = data;
#endif
    _FF(LastHwPtr) += 4;
    _FF(CurrWR) = 2;

    _FF(WordsLeftInPacket) -= 1;
    if (_FF(WordsLeftInPacket) == 0)
    {
      _FF(InPacket) = 0;

      // last driver to finish a packet & it's packet
      _FF(LastWR) = DD16_SIGNATURE;
      _FF(LastPH) = _FF(CurrPH);
      _FF(lastPHHwPtr) = _FF(currPHHwPtr);
      _FF(lastPHData) = _FF(currPHData);

    }
}

FxU32 trapOnDW = 0;

void mySetDW(DWORD * hwPtr, FxU32 data)
{
    DEBUG_FIX;
    
    if (trapOnDW)
    {
    GPF();
    }
#ifdef IS_16
    h3WRITE(NULL, hwPtr, data);    
#else
    *(FxU32 *)hwPtr = data;
#endif
}

#endif

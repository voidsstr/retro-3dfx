#include "vxd.h"
/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Revision: 2$
** $Date: 10/11/00 8:31:15 PM$
*/

#include <assert.h>
#include <stdlib.h>

#include <h3.h>
#include "../csim/csim.h"

// wait until we read not busy 3 times in a row
FxBool idleLoop(SstRegs *sst)
{
    FxU32 cntr;

    if(!sst)
        return(FXFALSE);

    if(halInfo.boardsFound == 1)
      {
	cntr = 0;
	while(cntr < 3)
	  {
	    if(!(IGET(sst->status) & SST_BUSY))
	      cntr++;
	    else
	      cntr = 0;
	  }

	// if the above loop waited for HSIM or HW to be idle
	// then we still need to wait for CSIM (in case cmd fifo is used)	
	if (halInfo.hsim || halInfo.hw) 
	  {
	    while (halInfo.csimLastRead & SST_BUSY)
	      IGET(sst->status);
	  }
      }
    else //multi-chip loop
      {
	SstRegs *multichipSST;
	FxU32 i;
	  
	for(i=0; i<halInfo.boardsFound; i++)
	  {
	    multichipSST = halInfo.boardInfo[i].virtAddr[0];
	    assert(multichipSST != NULL);

	    cntr = 0;
	    while(cntr < 3)
	      {
		if(!(IGET(multichipSST->status) & SST_BUSY))
		  cntr++;
		else
		  cntr = 0;
	      }	    

	    // if the above loop waited for HSIM or HW to be idle
	    // then we still need to wait for CSIM (in case cmd fifo is used)	
	    if (halInfo.hsim || halInfo.hw) 
	      {
		while (halInfo.csimLastRead & SST_BUSY)
		  IGET(multichipSST->status);
	      }
	  }	
      }
    return FXTRUE;
}

FX_EXPORT FxBool FX_CSTYLE
fxHalIdleNoNop( SstRegs *sst )
{
    GDBG_INFO(9,"fxHalIdleNoNop(0x%x)\n",sst);
    return idleLoop(sst);
}


FX_EXPORT FxBool FX_CSTYLE
fxHalIdle( SstRegs *sst )
{

    GDBG_INFO(9,"fxHalIdle(0x%x)\n",sst);
    if(!sst)
        return(FXFALSE);

    if(halInfo.boardsFound == 1)
      ISET(sst->nopCMD, 0x0);
    else
      {
	FxU32 i;
	SstRegs *multichipSST;

	for(i=0; i<halInfo.boardsFound; i++)
	  {	    
	    multichipSST = halInfo.boardInfo[i].virtAddr[0];
	    ISET(multichipSST->nopCMD, 0);
	  }
      }
    return idleLoop(sst);
}

//
// alternate idle routines which access the status
// register via the 2D register space
//

// wait until we read not busy 3 times in a row
FxBool idleLoop2(SstGRegs *sstg)
{
    FxU32 cntr;

    if(!sstg)
        return(FXFALSE);

    if(halInfo.boardsFound == 1)
      {
	cntr = 0;
	while(cntr < 3) 
	  {
	    if(!(IGET(sstg->status) & SST_BUSY)) 
	      cntr++;
	    else
	      cntr = 0;
	  }

	// if the above loop waited for HSIM or HW to be idle
	// then we still need to wait for CSIM (in case cmd fifo is used)	
	if (halInfo.hsim || halInfo.hw) 
	  {
	    while (halInfo.csimLastRead & SST_BUSY)
	      IGET(sstg->status);
	  }
      }
    else //multi-chip loop
      {
	SstRegs *multichipSST;
	SstGRegs *multichipSSTG;
	FxU32 i;
	  
	for(i=0; i<halInfo.boardsFound; i++)
	  {
	    multichipSST = halInfo.boardInfo[i].virtAddr[0];
	    assert(multichipSST != NULL);

	    multichipSSTG = (SstGRegs *)SST_GUI_ADDRESS(multichipSST);

	    cntr = 0;
	    while(cntr < 3)
	      {
		if(!(IGET(multichipSSTG->status) & SST_BUSY))
		  cntr++;
		else
		  cntr = 0;
	      }

	    if (halInfo.hsim || halInfo.hw) 
	      {
		while (halInfo.csimLastRead & SST_BUSY)
		  IGET(multichipSSTG->status);
	      }
	  }
      }

    return FXTRUE;
}

FX_EXPORT FxBool FX_CSTYLE
fxHalIdleNoNop2( SstRegs *sst )
{
    SstGRegs *sstg = (SstGRegs *) SST_GUI_ADDRESS(sst);

    GDBG_INFO(9,"fxHalIdleNoNop2(0x%x)\n",sstg);

    if (!sst || !sstg)
      return(FXFALSE);

    return idleLoop2(sstg);
}


FX_EXPORT FxBool FX_CSTYLE
fxHalIdle2( SstRegs *sst )
{
    SstGRegs *sstg = (SstGRegs *) SST_GUI_ADDRESS(sst);

    GDBG_INFO(9,"fxHalIdle2(0x%x)\n",sstg);
    if(!sst || !sstg)
        return(FXFALSE);

    
    if(halInfo.boardsFound == 1)
      ISET(sst->nopCMD, 0x0);
    else
      {
	FxU32 i;
	SstRegs *multichipSST;

	for(i=0; i<halInfo.boardsFound; i++)
	  {	    
	    multichipSST = halInfo.boardInfo[i].virtAddr[0];
	    assert(multichipSST != NULL);

	    ISET(multichipSST->nopCMD, 0);
	  }
      }

    return idleLoop2(sstg);
}

FX_EXPORT FxBool FX_CSTYLE
fxHalVsync( SstRegs *sst )
{
    GDBG_INFO(9,"fxHalVsync(0x%x)\n",sst);
    if(!sst)
        return(FXFALSE);

#if defined HAL_HSIM || defined HAL_HW
    if ( halInfo.hsim || halInfo.hw ) { 
      while(!(IGET(sst->status) & SST_VRETRACE));
    }
#endif
    return FXTRUE;
}

FX_EXPORT FxBool FX_CSTYLE
fxHalVsyncNot( SstRegs *sst )
{
    FxU32 cntr = 0;

    GDBG_INFO(9,"fxHalVsyncNot(0x%x)\n",sst);
    if(!sst)
        return(FXFALSE);

#if defined HAL_HSIM || defined HAL_HW
    if ( halInfo.hsim || halInfo.hw ) { 
      while(1) {
        if(!(IGET(sst->status) & SST_VRETRACE)) {
          if(++cntr >= 3)
            break;
        } else
          cntr = 0;
      }
    }
#endif
    return FXTRUE;
}

FX_EXPORT FxBool FX_CSTYLE
fxHalInitCmdFifo( SstRegs *sst, int which, FxU32 fifoStart,
                  FxU32 size, FxBool directExec, FxBool disableHoles, FxBool agpEnable)
{
    GDBG_INFO(1,"fxHalInitCmdFifo(0x%x,fifo=%d,start=0x%x,size=0x%x(%d),\n\t\t\tdirExec=%d,disHoles=%d,agpEnable=%d)\n",
                sst,which,fifoStart,size,size,directExec,disableHoles,agpEnable);
    fxHalIdleNoNop(sst);
#ifdef CVG
    {
        FxU32 init7;

        init7 = GET(sst->fbiInit7);             // init code might have set some bits
        init7 &= ~(SST_EN_CMDFIFO |
                  SST_EN_CMDFIFO_OFFSCREEN | SST_CMDFIFO_DISABLE_HOLES |
                  SST_CMDFIFO_REGS_SYNC_WRITES | SST_CMDFIFO_REGS_SYNC_READS |
                  SST_CMDFIFO_RDFETCH_THRESH | SST_CMDFIFO_PCI_TIMEOUT);
        init7 |= SST_CMDFIFO_REGS_SYNC_WRITES | SST_CMDFIFO_REGS_SYNC_READS |
                        (0x10<<SST_CMDFIFO_RDFETCH_THRESH_SHIFT) |
                        (0x1f<<SST_CMDFIFO_PCI_TIMEOUT_SHIFT);
        SET(sst->fbiInit7, init7);              // turn off the command fifo
        GET(sst->status);                       // prevent PCI bursts
#ifdef HAL_HSIM
// XXX we need to abstract
//      PCI config read/writes so that we can funnel them thru the CSIM
//      HSIM, and HW just like everything else
        // Disable all writes to the PCI fifo while we're setting up CMDFIFO
        if (halInfo.hsim)
        PCI_CFG_WR(SST_INIT_ENABLE,
                (PCI_CFG_RD(SST_INIT_ENABLE, 0x1) & ~(SST_INITWR_EN | SST_PCI_FIFOWR_EN)) |
                 SST_INITWR_EN, 0x1);
#endif

        SET(sst->cmdFifoBase,(((fifoStart+size-1)>>12)<<SST_CMDFIFO_END_SHIFT) | (fifoStart>>12));
        GET(sst->status);                       // prevent PCI bursts
        SET(sst->cmdFifoReadPtr,fifoStart);
        GET(sst->status);                       // prevent PCI bursts
        SET(sst->cmdFifoAmin,fifoStart-4);
        GET(sst->status);                       // prevent PCI bursts
        SET(sst->cmdFifoAmax,fifoStart-4);
        GET(sst->status);                       // prevent PCI bursts
        SET(sst->cmdFifoDepth,0);
        GET(sst->status);                       // prevent PCI bursts
        SET(sst->cmdFifoHoles,0);
        GET(sst->status);                       // prevent PCI bursts
        fxHalIdle(sst);

        SET(sst->fbiInit7, init7 | SST_EN_CMDFIFO |
                        (directExec ? 0 : SST_EN_CMDFIFO_OFFSCREEN) |
                        (disableHoles ? SST_CMDFIFO_DISABLE_HOLES : 0));
        // Can't perform STALL_ON_SST_IDLE() here because it will generate
        // writes to the CMDFIFO since the CMDFIFO is now enabled...
        GET(sst->status);
 
#ifdef HAL_HSIM
        // Enable writes to be pushed onto the CMDFIFO...
        if (halInfo.hsim)
        PCI_CFG_WR(SST_INIT_ENABLE,
                PCI_CFG_RD(SST_INIT_ENABLE, 0x1) | SST_INITWR_EN | SST_PCI_FIFOWR_EN, 0x1);
#endif
    }
#else
    {
        SstCRegs *sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);
        CmdFifo *fifo = which ? &sstc->cmdFifo1 : &sstc->cmdFifo0;
        FxU32 bn;
        FxDeviceInfo *info;

        if (fxHalVaddrToBoardNumber( sst, &bn ))        // find the board
          info = &halInfo.boardInfo[bn];
        else
          return(FXFALSE);

        SET(fifo->baseSize,0);                  // disable the CMD fifo
        if ( size != 0 ) {
          size = (size-1)>>12;            // number of 4KB pages minus 1
          if (agpEnable) {
            FxU32 *vAddr;
            FxU32 baseH,baseL;
            
            vAddr = agpMemAlloc(sst,(size+1) << 12);
            if (vAddr == NULL)
              return(FXFALSE);
            
            agpVirtToPhys(vAddr,&baseH,&baseL);
            SET(fifo->baseAddrL,baseH << (32-12) | baseL>>12);
            SET(fifo->readPtrL,baseL);
            SET(fifo->readPtrH,baseH);
            SET(fifo->aMin,baseL-4);
            SET(fifo->aMax,baseL-4);
          } else {
            SET(fifo->baseAddrL,fifoStart>>12);
            SET(fifo->readPtrL,fifoStart);
            SET(fifo->readPtrH,0);
            SET(fifo->aMin,fifoStart-4);
            SET(fifo->aMax,fifoStart-4);
          }
          SET(fifo->depth,0);
          SET(fifo->holeCount,0);
          SET(sstc->cmdFifoThresh,(0x09 << 5) | 0x2); // Fifo LWM /HWM/ THRESHOLD
          SET(fifo->baseSize,size | SST_EN_CMDFIFO 
              | (disableHoles ? SST_CMDFIFO_DISABLE_HOLES : 0)
              | (agpEnable ?  SST_CMDFIFO_AGP : 0));
        }
    }
#endif
    gdbg_info(2,"CMD FIFO placed at physical addr 0x%x\n",fifoStart);
    return FXTRUE;
}

/*
**  P6 Fence - all memory IO comes thru here, so this is the only place we need it
**
**  Here's the stuff to do P6 Fencing.  This is required for the
**  certain things on the P6
*/

#if defined(__WATCOMC__)
static FxU32 p6FenceVar;
  void p6Fence(void);
  #pragma aux p6Fence = \
    "xchg eax, p6FenceVar" \
    modify [eax];

  #define P6FENCE p6Fence()

#elif defined(__MSC__)
static FxU32 p6FenceVar;
  #define P6FENCE {_asm xchg eax, p6FenceVar}
#elif defined(__unix__)
  #define P6FENCE
#else
  #error "P6 Fencing in-line assembler code needs to be added for this compiler"
#endif

/*
** fxHalRead32():
**  Read 32-bit Word from specified address
**
*/
FxU32 fxHalRead32(FxU32 *addr)
{
    P6FENCE;
    return(GET(*addr));
}

/*
** fxHalWrite32():
**  Write 32-bit Word to specified address
**
*/
void fxHalWrite32(FxU32 *addr, FxU32 data)
{
    P6FENCE;
    SET(*addr,data);
    P6FENCE;
}


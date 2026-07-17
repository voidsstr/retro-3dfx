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
** NYI
**  auto wrap
** $Revision: 3$
** $Date: 10/11/00 8:11:48 PM$
*/

#include "udiag.h"
#include "sstdiag.h"


static void shadowStore32(volatile void *addr, FxU32 data);
SstRegs shadowRegisters3D[32][1 + MAX_NUM_TMUS];


#ifdef SST2
#include "../csim/sst2asm.h"
#else 
#include "../csim/h3asm.h"
#endif
//----------------------------------------------------------------------
// These routines keep a 32-entry history buffer around when diago.writeFifo
// is enabled.  This allows us to generate command packets to the CMD FIFO
// area without rewriting the diags.  Pretty tricky!
//----------------------------------------------------------------------

// register mask (ignore the wrap bits)
#define REG_MASK 0xC03FFF

// simple user-level structure for managing the command region
static struct {
    FxBool outOfOrder;
    FxI32 roomToEnd;		// how much room is left until the end
    FxI32 roomToReadPtr;	// how much room is left before we wait
    FxU32 lastReadPtrL;		// the last known position of the read ptr
    FxU32 lastReadPtrH;		// the last known position of the read ptr
    FxI32 size;			// ring buffer size within total area
    FxU32 totalSize;		// total size of CMD FIFO area
    FxU32 baseAddr;		// base address offset from chip base
    FxU32 baseAddrL;		// cmd fifo base address
    FxU32 *startAddr;		// virtual address of CMD FIFO aperture
    FxU32 *curP;
    CmdFifo *sstfifo;
    FxBool disableHoles;
  FxU32 aBump;
} cr[2];
static int crFirstTime = 1;
static int whichFifo = 0;

struct agpPkt6 {
  FxU32 sizeBytes;
  FxU32 baseLow;
  FxU32 hostAddrHigh;
  FxU32 graphicsAddr;
  FxU32 graphicsWidth;
  FxU32 moveCmd;
  int init;
} pkt6;

//-------------------------------------------
// Cmd fifo 0/1 select 
#define MAXSF 8
static int savedFifo[MAXSF];
static int savedFifoCur = 0;
void hb_selectFifo(int which)
{
  if (savedFifoCur == MAXSF) {
    GDBG_ERROR("selectFifo","overflow!");
    DIAG_FAIL();
  }
  savedFifo[savedFifoCur++] = whichFifo;
  whichFifo = which;
}
void hb_restoreFifo()
{
  if (savedFifoCur == 0) {
    GDBG_ERROR("restoreFifo","Underflow!");
    DIAG_FAIL();
  }
  whichFifo = savedFifo[--savedFifoCur];
}

void hb_resetAll()
{
  crFirstTime = 1;
}

static void flushBump(int wf)
{
  if (cr[wf].aBump) {
    P6FENCE;
    halStore32(&cr[whichFifo].sstfifo->bump,cr[wf].aBump);// manually bump the pointer
  }
  cr[wf].aBump = 0;
}

static void incBump(int wf,int n) 
{
  FxU32 saveSeed;
  if (cr[wf].aBump + n > SST_MASK(16)) 
    flushBump(wf);
  else {
    saveSeed = getSeed();	// save the random seed
#ifdef HAL_HSIM
    if (diago.halInfo->hsim && 
	( vconfigLookup("SST_FASTSIM",NULL))) {
      if (iRandom(1024) == 5)
	flushBump(wf);
    }
    else if ( vconfigLookup("H3_BUMPOFTEN",NULL)) {
	if (iRandom(4) == 1)
	  flushBump(wf);
      }
    else {
	if (iRandom(64) == 1)
	  flushBump(wf);
      }
#else
    if (iRandom(64) == 1)
      flushBump(wf);
#endif
    setSeed(saveSeed);		        // restore old seed
  }
  cr[wf].aBump += n;
}

static void crInit(int wf)
{
//    sst_idle_really(diago.sst);
  if (wf == 0)
    cr[wf].sstfifo = &((SstCRegs *)SST_CMDAGP_ADDRESS(diago.sst))->cmdFifo0;
 else
    cr[wf].sstfifo = &((SstCRegs *)SST_CMDAGP_ADDRESS(diago.sst))->cmdFifo1;
    
    cr[wf].baseAddr = GET(cr[wf].sstfifo->baseAddrL) << 12;
    cr[wf].baseAddrL = GET(cr[wf].sstfifo->baseAddrL);
    if ( diago.whichFifo >= 2 && (diago.agpEnable & (1 << wf))
	 || (diago.whichFifo < 2 && diago.agpEnable) ) 
      cr[wf].startAddr = agpPhysToVirt(cr[wf].baseAddrL >> 20,cr[wf].baseAddrL<<12);
    else 
      cr[wf].startAddr = (FxU32 *)(SST_BASE_ADDRESS(diago.sst) + SST_RAW_LFB_OFFSET + cr[wf].baseAddr);
    cr[wf].totalSize = ((GET(cr[wf].sstfifo->baseSize)&SST_CMDFIFO_SIZE)+1) << 12;
    cr[wf].lastReadPtrL = GET(cr[wf].sstfifo->readPtrL);
    cr[wf].lastReadPtrH = GET(cr[wf].sstfifo->readPtrH);
    cr[wf].size = diago.ringSize*4;
    if (cr[wf].size < 0) {
	cr[wf].size = -cr[wf].size;
	cr[wf].outOfOrder = 1;
	GDBG_INFO(5, "History Buffer %d will execute out-of-order\n", wf);
    }
    else
      GDBG_INFO(5, "History Buffer %d will execute in-order\n", wf);

    if (cr[wf].size < 256) {
	cr[wf].size = 256;
	gdbg_printf("WARNING: increasing CMD FIFO ring buffer size to %d words\n",
			cr[wf].size/4);
    }
    cr[wf].roomToEnd = cr[wf].size;
    if ( diago.whichFifo >= 2 && (diago.agpEnable & (1 << wf))
	 || (diago.whichFifo < 2 && diago.agpEnable) ) 
      cr[wf].roomToReadPtr = cr[wf].size - 8;		
    else
      cr[wf].roomToReadPtr = cr[wf].size - 4;		// very important!

    cr[wf].curP = cr[wf].startAddr;
    cr[wf].disableHoles = ((GET(cr[wf].sstfifo->baseSize) & SST_CMDFIFO_DISABLE_HOLES) != 0); 
    cr[wf].aBump = 0;

    // we cannot allow the entire FIFO to be written because then if the
    // read pointer hasn't moved we cannot tell if it really hasn't moved
    // or if it has looped entirely around and executed everything
    gdbg_info(1,"CMD FIFO %d enabled at virt addr 0x%x, %s, %s order\n",
		wf,cr[wf].startAddr,
		diago.directExec ? "internal" : "offscreen",
		cr[wf].outOfOrder ? "OUT of" : "IN");
    gdbg_info(2,"\t using a %d(0x%x) byte ring in a %d(0x%x) region with hole counting %s \n",
		cr[wf].size,cr[wf].size, cr[wf].totalSize, cr[wf].totalSize,
	      cr[wf].disableHoles ? "DISABLED" : "ENABLED"
	      );
    pkt6.init = 0;
}

//-------------------------------------------------
// stores packet data to AGP memory or framebuffer
static void cmdStore32(FxU32 *addr, FxU32 data)
{  
  if ( diago.whichFifo >= 2 && (diago.agpEnable & (1 << whichFifo))
       || (diago.whichFifo < 2 && diago.agpEnable) ) {
    AGPWRV(*(FxU32 *)addr, data);
  }
  else {
    halStore32(addr,data);
  }
}

//----------------------------------------------------------------------
// HBUF_SIZE must be a power of 2!!!!
#define HBUF_SIZE 64
#define SCRAM_SIZE (HBUF_SIZE*2)
#define WRAP(p) p &= HBUF_SIZE-1

static struct {
    FxU32 head, tail;		// ring pointers
    FxU32 addr[HBUF_SIZE];
    FxU32 data[HBUF_SIZE];
} histBuf[2];

static struct {
    FxU32 *addr[SCRAM_SIZE];
    FxU32  data[SCRAM_SIZE];
} scrambleBuf;

static FxI32 subLH(FxU32 L2,FxU32 H2,FxU32 L1,FxU32 H1)
{
  FxI64 n1,n2,diff;
  FX_SET64(n1,H1,L1);
  FX_SET64(n2,H2,L2);
  diff = FX_SUB64(n2,n1);
  if ( FX_HI64(diff) &&
       FX_HI64(diff) != (FxI32)-1) {   
    GDBG_ERROR("histbuf","internal error, unexpected difference  0x%08x:%08x\n",
	       FX_HI64(diff),FX_LO64(diff));
    return 0xdeadbeef;
  }
  return(FX_LO64(diff));
}

static FxBool gtLH(FxU32 L2,FxU32 H2,FxU32 L1,FxU32 H1)
{
  FxBool gt = 0;
  if (H2 > H1) gt = 1;
  else if (H2 < H1) gt = 0;
  else {
    return(L2 > L1);
  }
  return(gt);
}

//----------------------------------------------------------------------
// make room for 'n' bytes
static void makeRoom(SstRegs *sst, int n)
{
  FxU32 saveSeed = getSeed();
  FxU32 writeP;

  // sanity check #1: (write pointer) + (room to end) == size 
  writeP = SST_FAKE_ADDRESS_GET_BASE_OFFSET(cr[whichFifo].curP) -
    SST_FAKE_ADDRESS_GET_BASE_OFFSET(cr[whichFifo].startAddr);
				       
  if ( cr[whichFifo].roomToEnd + writeP != (FxU32)cr[whichFifo].size ) { 
    GDBG_ERROR("makeRoom","writeP(%d) + roomToEnd(%d) == %d (should be %d)\n",
	       writeP,cr[whichFifo].roomToEnd,
	       cr[whichFifo].roomToEnd+writeP,
	       cr[whichFifo].size);
    DIAG_INCERROR();
    DIAG_FAIL();
  }

  // sanity check #2: requested packet size will fit into the cmdfifo ring buffer
  // NOTE: packet must be smaller than the ring buffer size to allow room for the JMP packet 
  //       which wraps the cmdfifo
  if (n >= cr[whichFifo].size) {
    GDBG_ERROR("makeRoom","packet (%d bytes) is too large for the ring buffer (%d bytes)\n",
	       n,cr[whichFifo].size);
    DIAG_INCERROR();
    DIAG_FAIL();
  }

again:
    cr[whichFifo].roomToReadPtr -= n;
    cr[whichFifo].roomToEnd -= n;
    gdbg_info(5,"  makeroom[%d](%d): %d,%d left\n",
		whichFifo,n,cr[whichFifo].roomToEnd,cr[whichFifo].roomToReadPtr);
    while (cr[whichFifo].roomToReadPtr < 0) {		// do we need to stall?
	FxU32 curReadPtrL;
	FxU32 curReadPtrH;
#if defined H3_A0 || defined H3_A1 || defined H3_A2
	FxU32 curReadPtrL2;
	FxU32 curReadPtrH2;
	FxU32 status;
	static int first_time = 1;

	if ( first_time ) {
	  GDBG_INFO(0,"makeroom: HACK -- work-around CMDFIFO read ptr bug\n");
	  first_time = 0;
	}

	do {
	  curReadPtrL = GET(cr[whichFifo].sstfifo->readPtrL);
	  status = GET(sst->status);
	  curReadPtrL2 = GET(cr[whichFifo].sstfifo->readPtrL);
	  status = GET(sst->status);
	} while ( curReadPtrL != curReadPtrL2 );

	do {
	  curReadPtrH = GET(cr[whichFifo].sstfifo->readPtrH);
	  status = GET(sst->status);
	  curReadPtrH2 = GET(cr[whichFifo].sstfifo->readPtrH);
	  status = GET(sst->status);
	} while ( curReadPtrH != curReadPtrH2 );

#else
	//If running with multi-chips, take the most pessimistic value 
	//for the current read ptr
	if(diago.chipCount > 1)
	  {
	    FxU32 index;
	    FxU32 address;
	    FxU32 mask;
	    FxU32 readPtrL[32], readPtrH[32];
	    FxU32 roomToReadPtr[32];
	    FxU32 leastRoomIndex;

	    mask = SST_FAKE_ADDRESS_GET_BASE_OFFSET(~0);

	    //Go through and calculate how much space each chip has
	    //in its command fifo
	    for(index=0; index< diago.chipCount; index++)
	      {
		if(index > 0)
		  {
		    address = ((FxU32)&cr[whichFifo].sstfifo->readPtrL) & mask;
		    address |= (~mask) & (FxU32)diago.sstChildren[index-1];
		    readPtrL[index] = halLoad32(address);
		    
		    address = ((FxU32)&cr[whichFifo].sstfifo->readPtrH) & mask;
		    address |= (~mask) & (FxU32)diago.sstChildren[index-1];
		    readPtrH[index] = halLoad32(address);
		  }
		else
		  {
		    address = ((FxU32)&cr[whichFifo].sstfifo->readPtrL) & mask;
		    address |= (~mask) & (FxU32)diago.sst;
		    readPtrL[index] = halLoad32(address);
		    
		    address = ((FxU32)&cr[whichFifo].sstfifo->readPtrH) & mask;
		    address |= (~mask) & (FxU32)diago.sst;
		    readPtrH[index] = halLoad32(address);
		  }

		roomToReadPtr[index] = 	cr[whichFifo].roomToReadPtr;
		roomToReadPtr[index] += subLH(readPtrL[index], readPtrH[index],
					      cr[whichFifo].lastReadPtrL, cr[whichFifo].lastReadPtrH);		    
		if(gtLH(cr[whichFifo].lastReadPtrL,cr[whichFifo].lastReadPtrH, readPtrL[index], readPtrH[index]))
		  roomToReadPtr[index] += cr[whichFifo].size;
	      }
	    
	    //Figure out which chip has the least space in its command fifo
	    leastRoomIndex = 0;
	    for(index=1; index< diago.chipCount; index++)
	      {
		if(roomToReadPtr[index] < roomToReadPtr[leastRoomIndex])
		  leastRoomIndex = index;
	      }

	    //Record information about command fifo with the least room
	    curReadPtrL = readPtrL[leastRoomIndex];
	    curReadPtrH = readPtrH[leastRoomIndex];	    
	  }
	else
	  {
	    //Single chip case
	    curReadPtrL = GET(cr[whichFifo].sstfifo->readPtrL);
	    curReadPtrH = GET(cr[whichFifo].sstfifo->readPtrH);
	  }
#endif

#ifdef HAL_HSIM
	if (diago.halInfo->hsim && vconfigLookup("SST_FASTSIM",NULL)) {
	  flushBump(whichFifo);
	  PCI_STALL(1000);
	}
#endif
	cr[whichFifo].roomToReadPtr += subLH(curReadPtrL,curReadPtrH,
					     cr[whichFifo].lastReadPtrL,
					     cr[whichFifo].lastReadPtrH);
	if (gtLH(cr[whichFifo].lastReadPtrL,cr[whichFifo].lastReadPtrH,
		 curReadPtrL,curReadPtrH))
	    cr[whichFifo].roomToReadPtr += cr[whichFifo].size;
	cr[whichFifo].lastReadPtrL = curReadPtrL;
	cr[whichFifo].lastReadPtrH = curReadPtrH;
	gdbg_info(5,"  update: %d,%d left\n",cr[whichFifo].roomToEnd,cr[whichFifo].roomToReadPtr);
	if (iRandom(1)) flushBump(whichFifo);
    }
    if (cr[whichFifo].roomToEnd <= 0) {		// wrap to front
	gdbg_info(5,"  wrapping with %d,%d left\n",cr[whichFifo].roomToEnd,cr[whichFifo].roomToReadPtr);
	// if there's enuf room for a NOP packet, the once in a while wait
	// for idle before sending the jump packet
	if (cr[whichFifo].roomToEnd+n >= 12 && cr[whichFifo].roomToReadPtr+n >= 12) {
	    if (iRandom(64)==0) {
		gdbg_info(5,"  waiting for depth==0 XXX\n");
		while (GET(cr[whichFifo].sstfifo->depth)) {
#ifdef HAL_HSIM
		  if (diago.halInfo->hsim && vconfigLookup("SST_FASTSIM",NULL))
		    PCI_STALL(500);
#endif
		}
#if 0
		cmdStore32(cr[whichFifo].curP,(1<<SSTCP_PKT1_NWORDS_SHIFT) |
				SSTCP_REGBASE_FROM_ADDR(NOPCMD) | SSTCP_PKT1);
		cr[whichFifo].curP++;
		cmdStore32(cr[whichFifo].curP,0);
		cr[whichFifo].curP++;
		if (cr[whichFifo].disableHoles)		// if hole counting is disabled
		  incBump(whichFifo,2);
		fxHalIdle(sstg);
#endif
	    }
	}
	if ( diago.whichFifo >= 2 && (diago.agpEnable & (1 << whichFifo))
	     || (diago.whichFifo < 2 && diago.agpEnable) ) {
	  //gdbg_printf("sanity JMP AGP\n");
	  cmdStore32(cr[whichFifo].curP,
		     ((cr[whichFifo].baseAddr<<SSTCP_PKT0_ADDR_SHIFT)>>2)  & SSTCP_PKT0_ADDR
		     | SSTCP_PKT0_JMP_AGP);
	  cmdStore32(cr[whichFifo].curP+1,(cr[whichFifo].baseAddrL >> 13));
	}
	else
	  cmdStore32(cr[whichFifo].curP,((cr[whichFifo].baseAddr<<SSTCP_PKT0_ADDR_SHIFT)>>2) | SSTCP_PKT0_JMP_LOCAL);
	P6FENCE;
	if (cr[whichFifo].disableHoles)	 {		// if hole counting is disabled
	  if ( diago.whichFifo >= 2 && (diago.agpEnable & (1 << whichFifo))
	       || (diago.whichFifo < 2 && diago.agpEnable) ) 
	    incBump(whichFifo,2);
	  else
	    incBump(whichFifo,1);
	}
	cr[whichFifo].roomToReadPtr -= cr[whichFifo].roomToEnd;
	cr[whichFifo].roomToEnd = cr[whichFifo].size;
	cr[whichFifo].curP = cr[whichFifo].startAddr;
	goto again;
    }
    setSeed(saveSeed);
}

//----------------------------------------------------------------------
// output the scambleBuf either sequentially or in random order
static void flushScrambledEggs(int n)
{
    int i;
    FxU32 send_order[SCRAM_SIZE];

    if (cr[whichFifo].outOfOrder) {
	if (n*2*4 >= cr[whichFifo].size) {
	    GDBG_ERROR("flushScrambledEggs","packet is larger than half the ring buffer\n");
	    DIAG_INCERROR();
	}
	scrambleRandom(n,(int *)send_order);
	for (i=0; i<n; i++)
	    cmdStore32(scrambleBuf.addr[send_order[i]], scrambleBuf.data[send_order[i]]);
    }
    else {
	if (n*4 >= cr[whichFifo].size) {
	    GDBG_ERROR("flushScrambledEggs","packet is larger than the ring buffer\n");
	    DIAG_INCERROR();
	}
	for (i=0; i<n; i++)
	    cmdStore32(scrambleBuf.addr[i], scrambleBuf.data[i]);
    }
    if (cr[whichFifo].disableHoles)			// if hole counting is disabled
      incBump(whichFifo,n);

}

//----------------------------------------------------------------------
// copy n words from the history buffer to the scramble buffer
// NOTE: we must omit s*TriCMD writes from packet type #3
// NOTE: there is already a header word in the scramble buffer
static int copyHistToScramble(int n, int cur, FxU32 packetType)
{
    int i;

    for (i=0; i<n; i++) {			// output n regs
	if (packetType == SSTCP_PKT3) {
	    if (((histBuf[whichFifo].addr[histBuf[whichFifo].tail]-SST_3D_OFFSET) & REG_MASK) == SBEGINTRICMD)
		goto skip;
	    if (((histBuf[whichFifo].addr[histBuf[whichFifo].tail]-SST_3D_OFFSET) & REG_MASK) == SDRAWTRICMD)
		goto skip;
	}
	scrambleBuf.addr[cur] = cr[whichFifo].curP++;
	scrambleBuf.data[cur] = histBuf[whichFifo].data[histBuf[whichFifo].tail];
	gdbg_info(6,"    +scramble[%d] : 0x%x 0x%x\n",cur,scrambleBuf.addr[cur],scrambleBuf.data[cur]);
	cur++;
    skip:
	histBuf[whichFifo].tail++;
	WRAP(histBuf[whichFifo].tail);
    }
    return cur-1;
}

//----------------------------------------------------------------------
// flush some stuff out from the history buffer
// NOTE: we flush into the scrambleBuf from whence we output either
// sequentially or in scrambled order
static void histFlush(void)
{
    FxU32 n,p3continue=0;
    FxU32 iaddr, saveSeed, tail, mask, Dmask, nextAddr,lastAddr;
    FxU32 rAddr,rData;
    FxU32 p6cur,ii;
    static FxU32 cmd[2],ptype[2];	// hack

    if (!diago.writeFifo) return;		// if not enabled, then return
    if (histBuf[whichFifo].head == histBuf[whichFifo].tail) return;	// if empty then return
    saveSeed = getSeed();

    iaddr = (histBuf[whichFifo].addr[histBuf[whichFifo].tail]-SST_3D_OFFSET) & REG_MASK;
    if (ptype[whichFifo] == SSTCP_PKT3 && cmd[whichFifo]!=SSTCP_PKT3_BDDBDD) {// if last packet was TSU
	if (iaddr == SVX) {			// try continuing
	gdbg_info(7,"continuing packet type 3\n");
	   p3continue = 1;			// skips over expecting SSETUPMODE
	   goto chosen;
	}
    }
 again:
    iaddr = histBuf[whichFifo].addr[histBuf[whichFifo].tail];
    if ( ! SST_IS_REGISTER_ADDR(iaddr) ) {		// if it's not a register write
	if ( SST_IS_LFB_ADDR(iaddr) || SST_IS_YUV_ADDR(iaddr) || SST_IS_TEX_ADDR(iaddr) 
	     || SST_IS_TEX2_ADDR(iaddr)) { 
	    ptype[whichFifo] = SSTCP_PKT5;
	    goto chosen;
	}
	else {
	    GDBG_ERROR("histFlush","unrecognized write in writeFifo mode\n");
	    DIAG_INCERROR();
	}
    }

    iaddr = histBuf[whichFifo].addr[histBuf[whichFifo].tail];
    if (SST_IS_CMDAGP_ADDR(iaddr)) {
      if (SST_IS_MOVECMD(iaddr)) {
	    ptype[whichFifo] = SSTCP_PKT6;
	    goto chosen;
      }
    }
    iaddr = histBuf[whichFifo].addr[histBuf[whichFifo].tail];
    if ( SST_IS_3D_ADDR(iaddr) || SST_IS_3D_ALT_ADDR(iaddr) )
	Dmask = 0;
    else if ( SST_IS_2D_ADDR(iaddr) )
	Dmask = SSTCP_PKT1_2D;
    else
	GDBG_ERROR("histFlush","unrecognized write in writeFifo mode\n");

    ptype[whichFifo] = rRandom(1,diago.writeFifo);		// random packet type
    if (ptype[whichFifo] > SSTCP_PKT4) ptype[whichFifo] = SSTCP_PKT4;	// clamp to maximum
    if (ptype[whichFifo] == SSTCP_PKT2) {			// if a 2D packet, but not 2D reg
	if (Dmask == 0)				// if it's a 3D reg
	    goto again;
	if ((iaddr-SST_2D_OFFSET) < CLIP0MIN || (iaddr-SST_2D_OFFSET) > CLIP0MIN+28*4)
	    goto again;
    }
    if (ptype[whichFifo] == SSTCP_PKT3) {			// if a TSU packet, but not
	if (Dmask)				// if it's a 2D reg
	    goto again;
	if (((iaddr-SST_3D_OFFSET)&REG_MASK) != SSETUPMODE) // the sSetupMode register
	    goto again;
    }
    // if the sSetupMode register
    if (((iaddr-SST_3D_OFFSET)&REG_MASK) == SSETUPMODE && diago.writeFifo>=3) {
	if (iRandom(3))				// choose packet 3
	    ptype[whichFifo] = SSTCP_PKT3;			// most of the time
    }
chosen:
    gdbg_info(7,"selecting packet type %d\n",ptype[whichFifo]);
    tail = histBuf[whichFifo].tail;
    lastAddr = 0;				// make first test pass
    mask = 0;
    // now compose up a packet of type 'ptype[whichFifo]' and send it
    // in the switch statement we compute how many words and copy the header
    // to the scramble buffer
    switch(ptype[whichFifo]) {
	case SSTCP_PKT1:			// output 'n' sequential registers
	{
	    FxU32 inc = SSTCP_INC;

	    for (n=0; tail!=histBuf[whichFifo].head; n++) {	// first compute n
		nextAddr = histBuf[whichFifo].addr[tail];
		if (n==1 && nextAddr == iaddr)		// if 2nd addr==1st addr
		    inc = 0;				// switch to non-seq mode
		if (nextAddr != (iaddr + n*4*(inc?1:0)))
		    break;
		tail++;
		WRAP(tail);
	    }

	    makeRoom(diago.sst,(n+1)*4);	// make room, send header
	    scrambleBuf.addr[0] = cr[whichFifo].curP++;
	    scrambleBuf.data[0] = (n<<SSTCP_PKT1_NWORDS_SHIFT) | inc | Dmask |
				SSTCP_REGBASE_FROM_ADDR(iaddr) |
				SSTCP_PKT1;
            gdbg_info(6,"    ++scramble[%d] : 0x%x 0x%x\n",0,scrambleBuf.addr[0],scrambleBuf.data[0]);
	    break;
	}

         case SSTCP_PKT2:			// output 2D group
	    for (n=0; tail!=histBuf[whichFifo].head; n++) {	// first compute n
		nextAddr = histBuf[whichFifo].addr[tail];
		if (nextAddr <= lastAddr ||
			(nextAddr-SST_2D_OFFSET) < CLIP0MIN || 
			(nextAddr-SST_2D_OFFSET) > CLIP0MIN+28*4)
		    break;
		lastAddr = nextAddr;
		mask |= 1 << ((nextAddr - SST_2D_OFFSET - CLIP0MIN)>>2);
		tail++;
		WRAP(tail);
	    }

	    makeRoom(diago.sst,(n+1)*4);	// make room, send header
	    scrambleBuf.addr[0] = cr[whichFifo].curP++;
	    scrambleBuf.data[0] = (mask<<SSTCP_PKT2_MASK_SHIFT) | SSTCP_PKT2;
            gdbg_info(6,"    ++scramble[%d] : 0x%x 0x%x\n",0,scrambleBuf.addr[0],scrambleBuf.data[0]);
	    break;
	
	case SSTCP_PKT3:			// output TSU group
	{
	    FxU32 packed=0, nverts=0, i;
	    FxU32 expecting[32];
	    char *expecting_str[32];
	    static FxU32 smode;

	    if (!p3continue)
		smode = histBuf[whichFifo].data[tail];	// if not continuing
	    i = 0;
	    expecting[i] = SSETUPMODE;	// build up what we expect
	    expecting_str[i++] = "SSETUPMODE";
	    expecting[i] = SVX;
	    expecting_str[i++] = "SVX";
	    expecting[i] = SVY;
	    expecting_str[i++] = "SVY";
	    if (smode & SST_SETUP_RGB) {
		expecting[i] = SRED;
		expecting_str[i++] = "SRED";
		expecting[i] = SGREEN;
		expecting_str[i++] = "SGREEN";
		expecting[i] = SBLUE;
		expecting_str[i++] = "SBLUE";
	    }
	    if (smode & SST_SETUP_A) {
		expecting[i] = SALPHA;
		expecting_str[i++] = "SALPHA";
	    }
	    if (smode & SST_SETUP_Z) {
		expecting[i] = SVZ;
		expecting_str[i++] = "SVZ";
	    }
	    if (smode & SST_SETUP_Wfbi) {
		expecting[i] = SOOWFBI;
		expecting_str[i++] = "SOOWFBI";
	    }
	    if (smode & SST_SETUP_W0) {
		expecting[i] = SOOW0;
		expecting_str[i++] = "SOOW0";
	    }
	    if (smode & SST_SETUP_ST0) {
		expecting[i] = SSOW0;
		expecting_str[i++] = "SSOW0";
		expecting[i] = STOW0;
		expecting_str[i++] = "STOW0";
	    }
	    if (smode & SST_SETUP_W1) {
		expecting[i] = SOOW1;
		expecting_str[i++] = "SOOW1";
	    }
	    if (smode & SST_SETUP_ST1) {
		expecting[i] = SSOW1;
		expecting_str[i++] = "SSOW1";
		expecting[i] = STOW1;
		expecting_str[i++] = "STOW1";
	    }
	    expecting[i] = SDRAWTRICMD;
	    expecting_str[i] = "S*TRICMD";

	    for (n=p3continue; tail!=histBuf[whichFifo].head; n++) {	// first compute n
		nextAddr = histBuf[whichFifo].addr[tail] & REG_MASK;
		gdbg_info(7,"expecting %s, got 0x%x\n",expecting_str[n],nextAddr);
		if (expecting[n] == SDRAWTRICMD) {
		    if (nextAddr == SBEGINTRICMD) {
			if (nverts==0) {
			    cmd[whichFifo] = SSTCP_PKT3_BDDBDD;
			    goto OK;
			}
			if ((cmd[whichFifo] == SSTCP_PKT3_BDDBDD) && !(nverts %3))
			    goto OK;
		    }
		    if (nextAddr == SDRAWTRICMD) {
			if (nverts == 0)
			    cmd[whichFifo] = SSTCP_PKT3_DDDDDD;
			else if ((nverts == 3) && (cmd[whichFifo] == SSTCP_PKT3_BDDBDD))
			    cmd[whichFifo] = SSTCP_PKT3_BDDDDD;
		    }
		}

		if (nextAddr == SARGB) {
		    if ((expecting[n] == SRED) || (expecting[n] == SALPHA)) {
			packed = SSTCP_PKT3_PACKEDCOLOR;
			if (expecting[n] == SRED) {
			     n+=(smode & SST_SETUP_A)?3:2;
			     goto OK;
			}
			if (expecting[n] == SALPHA) goto OK;
		    }
		}
		if (packed && (nextAddr >= SRED && nextAddr <= SALPHA)) {
		    GDBG_ERROR("histFlush", "unexpected non-packed color in packet type 3 with PC=1\n");
		    DIAG_INCERROR();
		}
		if (nextAddr != expecting[n]) {
		    if (n==1)
			break;
		    GDBG_ERROR("histFlush", "expecting write to %s in packet type 3\n",
				expecting_str[n]);
		    DIAG_INCERROR();
		}
	    OK:
		if (expecting[n] == SDRAWTRICMD) {	// end of vertex?
		    nverts++;
		    n = 0;
		    if (nverts == 15)
			break;
		}
		tail++;
		WRAP(tail);
	    }
	    if (nverts==0) goto again;	// HACK!!!
	    if (nverts > 15) {
		GDBG_ERROR("histFlush", "nverts=%d in packet type 3\n",
				nverts);
		DIAG_INCERROR();
	    }

	    if (packed) {
		if (smode & SST_SETUP_RGB) i -= 3;
		if (smode & SST_SETUP_A) i -= 1;
		i++;
	    }
	    // i contains the number of words per vertex (including sBeginTriCMD or sDrawTriCMD)
	    // NOTE: the sBeginTriCMD or sDrawTriCMD doesn't get written into the scramble buffer
	    //       because it is skipped in copyHistToScramble() and n is decremented accordingly
	    n = nverts*i;
	    // don't include sBeginTriCMD/sDrawTriCMD in amount of data to be written to the cmdfifo,
	    // so multiply nverts by (i-1) instead of i, then add 1 for the header
	    makeRoom(diago.sst,(nverts*(i-1)+1)*4);		// make room, send header
	    scrambleBuf.addr[0] = cr[whichFifo].curP++;
	    scrambleBuf.data[0] = packed |	// formulate the header
			(((smode & 0xF0000) >> 16) << SSTCP_PKT3_SMODE_SHIFT) |
			((smode & 0xFF) << SSTCP_PKT3_PMASK_SHIFT) |
			(nverts<<SSTCP_PKT3_NUMVERTEX_SHIFT) | cmd[whichFifo] | SSTCP_PKT3;
            gdbg_info(6,"    ++scramble[%d] : 0x%x 0x%x\n",0,scrambleBuf.addr[0],scrambleBuf.data[0]);
	    if (!p3continue) {
		histBuf[whichFifo].tail++;			// delete the SSETUPMODE write
		WRAP(histBuf[whichFifo].tail);		// it's in the packet #3 header
	    }

	    break;
	}

	case SSTCP_PKT4:			// output group starting from next
	    for (n=0; tail!=histBuf[whichFifo].head; n++) {	// first compute n
		nextAddr = histBuf[whichFifo].addr[tail];
		if (nextAddr <= lastAddr || nextAddr >= iaddr + 14*4)
		    break;
		lastAddr = nextAddr;
		mask |= 1 << ((nextAddr - iaddr)>>2);
		tail++;
		WRAP(tail);
	    }

	    makeRoom(diago.sst,(n+1)*4);	// make room, send header
	    scrambleBuf.addr[0] = cr[whichFifo].curP++;
	    scrambleBuf.data[0] = (mask<<SSTCP_PKT4_MASK_SHIFT) | Dmask |
				SSTCP_REGBASE_FROM_ADDR(iaddr) |
				SSTCP_PKT4;
            gdbg_info(6,"    ++scramble[%d] : 0x%x 0x%x\n",0,scrambleBuf.addr[0],scrambleBuf.data[0]);
	    break;

	case SSTCP_PKT5:			// output group starting from next
	{
	    iaddr = histBuf[whichFifo].addr[histBuf[whichFifo].tail];
	    for (n=0; tail!=histBuf[whichFifo].head; n++) {	// first compute n
		nextAddr = histBuf[whichFifo].addr[tail];
		if (nextAddr != (iaddr + n*4))
		    break;
		tail++;
		WRAP(tail);
	    }

	    iaddr = SST_FAKE_ADDRESS_GET_OFFSET(iaddr);
	    if ( SST_IS_TEX_ADDR(iaddr) || SST_IS_TEX2_ADDR(iaddr)) {
		if (diago.halInfo->hsim & HSIM_TREX_BACKDOOR_TEXWRITES) {
		    CSIM_PRIVATE(diago.sstCSIM)->inCmdFifoExecMode = 1;
		    while (n-- > 0) {			// output n regs
			volatile void *addr;
			addr = (volatile void *)(histBuf[whichFifo].addr[histBuf[whichFifo].tail] +
				SST_BASE_ADDRESS(diago.sst));
			rData = histBuf[whichFifo].data[histBuf[whichFifo].tail];
			halStore32(addr,rData);
			histBuf[whichFifo].tail++;
			WRAP(histBuf[whichFifo].tail);
		    }
		    CSIM_PRIVATE(diago.sstCSIM)->inCmdFifoExecMode = 0;
		    goto end_of_loop;
		}
		
		iaddr -= SST_TEX_OFFSET;

		Dmask = SSTCP_PKT5_TEXPORT;
	    }
	    else if ( SST_IS_YUV_ADDR(iaddr) ) {
	        iaddr -= SST_YUV_OFFSET;
		Dmask = SSTCP_PKT5_YUV;
	    } else {
		iaddr -= SST_LFB_OFFSET;
		Dmask = SSTCP_PKT5_3DLFB;
	    }
	    makeRoom(diago.sst,(n+2)*4);	// make room, send header
	    scrambleBuf.addr[0] = cr[whichFifo].curP++;
	    scrambleBuf.data[0] = Dmask | (n<<SSTCP_PKT5_NWORDS_SHIFT) | SSTCP_PKT5;
	    scrambleBuf.addr[1] = cr[whichFifo].curP++;
	    scrambleBuf.data[1] = iaddr;
            gdbg_info(6,"    ++scramble[%d] : 0x%x 0x%x\n",0,scrambleBuf.addr[0],scrambleBuf.data[0]);
            gdbg_info(6,"    ++scramble[%d] : 0x%x 0x%x\n",1,scrambleBuf.addr[1],scrambleBuf.data[1]);
	    break;
	}

       case SSTCP_PKT6:
	 gdbg_info(6,"    pkt6 : 0x%x 0x%x\n",1,histBuf[whichFifo].addr[histBuf[whichFifo].tail],histBuf[whichFifo].data[histBuf[whichFifo].tail]);
	 rAddr = iaddr - SST_CMDAGP_OFFSET;
	 n = 0; 
	 p6cur = 0;
	 rData = histBuf[whichFifo].data[histBuf[whichFifo].tail];
	 histBuf[whichFifo].tail++; WRAP(histBuf[whichFifo].tail);
	 switch(rAddr) {
	 case AGPREQSIZE:	
	   pkt6.init |= 0x1;
	   pkt6.sizeBytes = rData;
	   break;
	 case HOSTADDRLOW:	
	   pkt6.init |= 0x2;
	   pkt6.baseLow = rData;
	   break;
	 case HOSTADDRHIGH:	
	   pkt6.init |= 0x4;
	   pkt6.hostAddrHigh = rData;
	   break;
	 case GRAPHICSADDR:	
	   pkt6.init |= 0x8;
	   pkt6.graphicsAddr = rData;
	   break;
	 case GRAPHICSSTRIDE:	
	   pkt6.init |= 0x16;
	   pkt6.graphicsWidth = rData;
	   break;
	 case MOVECMD:
	   if (pkt6.init != 31)
	     GDBG_ERROR("histFlush","packet 6 incompletely specified %x\n",pkt6.init);
	   pkt6.moveCmd = rData;
	   makeRoom(diago.sst,20);	// make room, send header
	   scrambleBuf.addr[0] = cr[whichFifo].curP++;
	   scrambleBuf.data[0] = 
	     (pkt6.sizeBytes << SSTCP_PKT6_NBYTES_SHIFT) & SSTCP_PKT6_NBYTES
	     | (pkt6.moveCmd & SST_AGPMOVE_SPACE) | SSTCP_PKT6;
	   scrambleBuf.addr[1] = cr[whichFifo].curP++;
	   scrambleBuf.data[1] = pkt6.baseLow;
	   scrambleBuf.addr[2] = cr[whichFifo].curP++;
	   scrambleBuf.data[2] = pkt6.hostAddrHigh;
	   scrambleBuf.addr[3] = cr[whichFifo].curP++;
	   scrambleBuf.data[3] = pkt6.graphicsAddr;
	   scrambleBuf.addr[4] = cr[whichFifo].curP++;
	   scrambleBuf.data[4] = pkt6.graphicsWidth;
	   p6cur = 5;
	   for (ii = 0;ii<5;ii++)
	     gdbg_info(6,"    +++scramble[%d] : 0x%x 0x%x\n",ii,scrambleBuf.addr[ii],scrambleBuf.data[ii]);
	   break;
	 }
	 break;
	default:
	    GDBG_ERROR("histFlush","packet type %d NYI\n",ptype[whichFifo]);
	    DIAG_INCERROR();
	    break;
    }
    // copy the rest of the data beyond the header word(s)
    n = copyHistToScramble(n,ptype[whichFifo] == SSTCP_PKT6 ? p6cur : ptype[whichFifo] == SSTCP_PKT5 ? 2 : 1,ptype[whichFifo]);
    flushScrambledEggs(n+1);			// scramble and send

end_of_loop:
    if (histBuf[whichFifo].tail != histBuf[whichFifo].head) goto again;
    setSeed(saveSeed);				// restore old seed
}

//----------------------------------------------------------------------
// store an entry into the history buffer
// flush it if full afterwards
static FxU32 altmap[256];

static void altMapInit(void)
{
    int i;
    for (i=0; i<256; i++)
	altmap[i] = i*4;
    for (i=0; i<8; i++) {
	altmap[i*3+8] = (8+i)*4;
	altmap[i*3+9] = (16+i)*4;
	altmap[i*3+10] = (24+i)*4;

	altmap[i*3+40] = (40+i)*4;
	altmap[i*3+41] = (48+i)*4;
	altmap[i*3+42] = (56+i)*4;
    }
}

static void crInitAll()
{
  if (crFirstTime) {
    histBuf[0].head = 0;
    histBuf[0].tail = 0;
    histBuf[1].head = 0;
    histBuf[1].tail = 0;
    if (diago.whichFifo == 2) {
      crInit(0);crInit(1);
    }
    else {
      whichFifo = diago.whichFifo;
      crInit(diago.whichFifo);
    }
    altMapInit();
    crFirstTime = 0;
  }
}

static void histStore32(volatile void *addr, FxU32 data)
{
    FxU32 iaddr, newhead;
    crInitAll();

    iaddr = SST_FAKE_ADDRESS_GET_OFFSET(addr);
    if ( SST_IS_3D_ALT_ADDR(iaddr) ) {		// if in ALTMAP space
	gdbg_info(9,"ALTMAPing 0x%x\n",iaddr); // remap it to actual register
	iaddr = SST_3D_OFFSET+altmap[(iaddr&0x3FFF)>>2];
    }

    gdbg_info(8,"+histBuf[%d]: a=0x%x d=0x%x\n",whichFifo,iaddr+SST_BASE_ADDRESS(diago.sst),data);
    histBuf[whichFifo].addr[histBuf[whichFifo].head] = iaddr;
    histBuf[whichFifo].data[histBuf[whichFifo].head] = data;
    newhead = histBuf[whichFifo].head + 1;
    WRAP(newhead);
    if (newhead == histBuf[whichFifo].tail) {		// if it's full
	histFlush();				// then flush some out
    }
    histBuf[whichFifo].head = newhead;
}

//----------------------------------------------------------------------
// layered store routines, allow us to keep a history buffer
// SET* macros come thru here for diags
void diagStore8 (volatile void *addr, FxU8 data)
{
    FxU32 Dmask;

    if (diago.writeFifo && diago.cmdFifoEnabled) {
	FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET(addr);
	crInitAll();
	if ( (SST_IS_TEX_ADDR(iaddr) || SST_IS_TEX2_ADDR(iaddr)) && 
	     (diago.halInfo->hsim & HSIM_TREX_BACKDOOR_TEXWRITES) ) {
	    CSIM_PRIVATE(diago.sstCSIM)->inCmdFifoExecMode = 1;
	    halStore8(addr,data);
	    CSIM_PRIVATE(diago.sstCSIM)->inCmdFifoExecMode = 0;
	} else if ( SST_IS_LFB_ADDR(iaddr) || SST_IS_YUV_ADDR(iaddr) || SST_IS_TEX_ADDR(iaddr) ||
		    SST_IS_TEX2_ADDR(iaddr)) {
	    FxU32 saveSeed;
	    FxU32 byte_en = (1<<(iaddr & 0x3)) ^ 0xF;
	    histFlush();
	    saveSeed = getSeed();	// save the random seed
	    makeRoom(diago.sst,3*4);	// make room, send header
	    setSeed(saveSeed);		// restore old seed
	    if ( SST_IS_TEX_ADDR(iaddr) ) {
	      iaddr -= SST_TEX_OFFSET;
	      Dmask = SSTCP_PKT5_TEXPORT;
	    } else if ( SST_IS_TEX2_ADDR(iaddr) ) {
	      iaddr -= SST_TEX_OFFSET;
	      Dmask = SSTCP_PKT5_TEXPORT;	      
	    } else if ( SST_IS_YUV_ADDR(iaddr) ) {
	      iaddr -= SST_YUV_OFFSET;
	      Dmask = SSTCP_PKT5_YUV;
	    } else {
	      iaddr -= SST_LFB_OFFSET;
	      Dmask = SSTCP_PKT5_3DLFB;
	    }
	    cmdStore32(cr[whichFifo].curP++, Dmask | 
	    		(byte_en<<SSTCP_PKT5_BYTEN_W2_SHIFT) |
			(1<<SSTCP_PKT5_NWORDS_SHIFT) | SSTCP_PKT5);
	    cmdStore32(cr[whichFifo].curP++,iaddr & ~0x3);
	    cmdStore32(cr[whichFifo].curP++,data << ((iaddr&0x3)*8) );
	    if (cr[whichFifo].disableHoles) 			// if hole counting is disabled
	        incBump(whichFifo,3);
	}
	else {
	    GDBG_ERROR("diagStore8","NYI for non-LFB/TEX writes in writeFifo mode\n");
	    DIAG_INCERROR();
	}
    }
    else {
	halStore8(addr,data);
    }
}

void diagStore16(volatile void *addr, FxU16 data)
{
    FxU32 Dmask;

    if (diago.writeFifo && diago.cmdFifoEnabled) {
	FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET(addr);
	crInitAll();
	if ( (SST_IS_TEX_ADDR(iaddr) || SST_IS_TEX2_ADDR(iaddr)) && 
	     (diago.halInfo->hsim & HSIM_TREX_BACKDOOR_TEXWRITES) ) {
	    CSIM_PRIVATE(diago.sstCSIM)->inCmdFifoExecMode = 1;
	    halStore16(addr,data);
	    CSIM_PRIVATE(diago.sstCSIM)->inCmdFifoExecMode = 0;
	} else if ( SST_IS_LFB_ADDR(iaddr) || SST_IS_YUV_ADDR(iaddr) || SST_IS_TEX_ADDR(iaddr) ||
		    SST_IS_TEX2_ADDR(iaddr) ) {
	    FxU32 saveSeed;
	    histFlush();
	    saveSeed = getSeed();	// save the random seed
	    makeRoom(diago.sst,3*4);	// make room, send header
	    setSeed(saveSeed);		// restore old seed
	    if ( SST_IS_TEX_ADDR(iaddr) ) {
	      iaddr -= SST_TEX_OFFSET;
	      Dmask = SSTCP_PKT5_TEXPORT;
	    } else if ( SST_IS_TEX2_ADDR(iaddr) ) {
	      iaddr -= SST_TEX_OFFSET;
	      Dmask = SSTCP_PKT5_TEXPORT;
	    } else if ( SST_IS_YUV_ADDR(iaddr) ) {
	      iaddr -= SST_YUV_OFFSET;
	      Dmask = SSTCP_PKT5_YUV;
	    } else {
	      iaddr -= SST_LFB_OFFSET;
	      Dmask = SSTCP_PKT5_3DLFB;
	    }
	    cmdStore32(cr[whichFifo].curP++, Dmask | 
	    		((iaddr&2?0x3:0xC)<<SSTCP_PKT5_BYTEN_W2_SHIFT) |
			(1<<SSTCP_PKT5_NWORDS_SHIFT) | SSTCP_PKT5);
	    cmdStore32(cr[whichFifo].curP++,iaddr & ~0x3);
	    cmdStore32(cr[whichFifo].curP++,data << (iaddr&2?16:0));
	    if (cr[whichFifo].disableHoles)			// if hole counting is disabled
	      incBump(whichFifo,3);
	}
	else {
	    GDBG_ERROR("diagStore16","NYI for non-LFB/TEX writes in writeFifo mode\n");
	    DIAG_INCERROR();
	}
    }
    else {
	halStore16(addr,data);
    }
}

void diagStore32(volatile void *addr, FxU32 data)
{
    FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET(addr);

    //Shadow the register writes
    shadowStore32(addr, data);
    
    if (diago.cmdFifoEnabled) {
	if ( diago.writeFifo && ! SST_IS_IO_ADDR(iaddr) && (! SST_IS_CMDAGP_ADDR(iaddr) 
	      || SST_IS_MOVECMD(iaddr))) {
	    histStore32(addr,data);
	    return;
	}
    }
    halStore32(addr,data);
}


void diagStore32f(volatile void *addr, FxFloat data)
{
    diagStore32(addr,*(FxU32 *)&data);
}

void shadowStore32(volatile void *addr, FxU32 data)
{
  FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET(addr);  

  //This only shadows 3d registers for now. And it doesn't
  //even do a good job at that; the evil decoding logic
  //is not included here. Consequently, some shadow 
  //registers may not be valid
  
  if(SST_IS_3D_ADDR(iaddr) || SST_IS_3D_ALT_ADDR(iaddr))
    {
      FxU32 chipMask;
      FxU32 registerIndex;
      FxU32 externalChipMask, externalChipIndex;
      FxU32 boardIndex;
      FxBool foundBoard;
      CsimPrivate *cp;
      SstRegs *sstCSIM;
      FxI32 n;
      
      //Figure out where write is going
      foundBoard=FXFALSE;
      for(boardIndex=0; boardIndex<4; boardIndex++)
	{
	  if((addr >= (void *)diago.halInfo->boardInfo[boardIndex].virtAddr[0]) &&
	     (addr < (void *)((FxU32)diago.halInfo->boardInfo[boardIndex].virtAddr[0] + SST_RAW_LFB_OFFSET)))
	    {
	      foundBoard=FXTRUE;
	      break;
	    }
	}
      if(!foundBoard)
	{
	  GDBG_ERROR("shadowStore32", "Write to address where no board is mapped!\n");
	  DIAG_FAIL();
	}

      //Figure out whether or not this write should be broadcast to
      //children devices
      if(boardIndex == 0)
	sstCSIM = diago.sstCSIM;
      else
	sstCSIM = diago.sstChildrenCSIM[boardIndex-1];
      cp = CSIM_PRIVATE(sstCSIM);
      
      if(cp->environment.parentDevice)
	externalChipMask = 0xF;
      else
	externalChipMask = (1<<boardIndex);
      
      //Need to check if we're writing to chipMask register
      registerIndex = (iaddr & 0x3FF) / 4;

      //For writes to registers other than chipMask, do chipMask masking
      if(registerIndex != 0x85)
	externalChipMask &= shadowRegisters3D[boardIndex][0].chipMask;
      assert(externalChipMask);
      
      chipMask = SST_CHIP_NUMBER(iaddr);

      //A chipmask of 0 is a broadcast
      if(chipMask == 0)
	chipMask = 0xF;

      for(externalChipIndex=0; externalChipIndex < (1<<4); externalChipIndex++)
	{
	  if(((1 << externalChipIndex) & externalChipMask) == 0)
	    continue;

	  //If in 2 pixels per clock mode, register writes to either TMU
	  //go to both TMU's
	  if((shadowRegisters3D[externalChipIndex][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) ||
	     (shadowRegisters3D[externalChipIndex][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) ||
	     (shadowRegisters3D[externalChipIndex][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK))
	    {
	      if(chipMask & 0x6)
		chipMask |= 0x6;
	    }
	  
	  //Go through and write data to registers
	  for(n=0; n<4; n++)
	    if(chipMask & (1<<n))
	      {	    
		((FxU32 *)&shadowRegisters3D[externalChipIndex][n])[registerIndex] = data;
	      }
	}
    }
}






// NYI P5 scrambling
static int p5Count[2];
static int p5Bump;
void cmdP5Start(FxU32 space,FxU32 maskW2,FxU32 maskWN,FxU32 nWords,FxU32 dstAddr,FxU32 data)
{
  FxU32 saveSeed;

  crInitAll();
  histFlush();
  saveSeed = getSeed();	// save the random seed
  GDBG_INFO(5,"P5 start: space %d maskW2 %x maskWn %x sizeWords %d dstAddr 0x%08x data 0x%08x\n",
	    space,maskW2,maskWN,nWords,dstAddr,data);
  if (nWords > (2<<19)) {
    GDBG_ERROR("cmdP5Start","Too many words in P5 %d\n",nWords);
    DIAG_INCERROR();
  }
  if ((signed)nWords*4 > cr[whichFifo].size - 2) {
    GDBG_ERROR("cmdP5Start","Pkt 5 is too big for fifo %d\n",nWords);
    DIAG_INCERROR();
  }
  makeRoom(diago.sst,(2+nWords)*4);	// make room, send header
  p5Count[whichFifo] = nWords;
  cmdStore32(cr[whichFifo].curP++,
	      (space << SSTCP_PKT5_SPACE_SHIFT)	& SSTCP_PKT5_SPACE |
	     (maskW2<<SSTCP_PKT5_BYTEN_W2_SHIFT) & SSTCP_PKT5_BYTEN_W2 | 
	     (maskWN<<SSTCP_PKT5_BYTEN_WN_SHIFT) & SSTCP_PKT5_BYTEN_WN |
	     (nWords << SSTCP_PKT5_NWORDS_SHIFT) & SSTCP_PKT5_NWORDS |
	     SSTCP_PKT5);

  cmdStore32(cr[whichFifo].curP++,dstAddr & ~0x3);
  cmdStore32(cr[whichFifo].curP++,data);
  p5Count[whichFifo]--;
  p5Bump = 0;
  if (cr[whichFifo].disableHoles)			// if hole counting is disabled
    if (p5Count[whichFifo] == 0) {
      incBump(whichFifo,3);

      p5Bump = 0;
    } 
    else if (iRandom(7)==1) {
      incBump(whichFifo,3);
      p5Bump = 0;
    }
    else
      p5Bump = 3;
  setSeed(saveSeed);		        // restore old seed
}

void cmdP5Data(FxU32 data)
{
  FxU32 saveSeed;
  if (p5Count[whichFifo] == 0) {
    GDBG_ERROR("cmdP5Data","Unexpected word in P5 %d\n");
    DIAG_INCERROR();
  }
  p5Count[whichFifo]--;
  //  GDBG_INFO(1,"P5 data 0x%08x (curP 0x%08x)\n",data,cr[whichFifo].curP);
  cmdStore32(cr[whichFifo].curP++,data);
  // todo: randomised bumps
  if (cr[whichFifo].disableHoles) {
    if (p5Count[whichFifo] == 0) {
      incBump(whichFifo,p5Bump+1);	// manually bump the pointer}
      p5Bump = 0;
    }
    else  {
      saveSeed = getSeed();	// save the random seed
      if (iRandom(7)==1) {
	incBump(whichFifo,p5Bump+1);	// manually bump the pointer}
	p5Bump = 0;
      }
      else
	p5Bump++;
      setSeed(saveSeed);		        // restore old seed
    }
  }
}


void hb_histFlushAll(void) {
  if (diago.whichFifo < 2) {
    hb_selectFifo(diago.whichFifo);
    histFlush();
    flushBump(diago.whichFifo);
    hb_restoreFifo();
  } else {
    hb_selectFifo(0);
    histFlush();
    flushBump(0);
    hb_restoreFifo();
    hb_selectFifo(1);
    histFlush();
    flushBump(1);
    hb_restoreFifo();
  }
}

typedef struct agpMoveCmd {
  FxU32 sizeBytes;
  FxU32 baseLow;
  FxU32 baseHigh;
  FxU32 srcWidth;
  FxU32 srcStride;
  FxU32 fbOffset;
  FxU32 dstStride;
  FxU32 space;
  FxU32 id;
} AGPMOVECMD;


void hb_moveCmd(  SstRegs *sst,
		 FxU32 sizeBytes,
		 FxU32 srcWidth,
		 FxU32 srcStride,
		 FxU32 *moveData,
		 FxU32 fbOffset,
		 FxU32 dstStride,
		 FxU32 space)
{
  static char *space_str[] = {"LFB","YUV","3DLFB","TEX"};
  AGPMOVECMD move;
  SstCRegs *sstc;
  AGPMOVECMD *cmdp = &move;

  sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);
  cmdp->sizeBytes = sizeBytes;
  cmdp->srcWidth = srcWidth;
  cmdp->srcStride = srcStride;
  cmdp->fbOffset = fbOffset;
  cmdp->dstStride = dstStride; 
  cmdp->space = space;
  cmdp->id = whichFifo;
    
  GDBG_INFO(1,"Agp virtual src addr 0x%x\n",moveData);	
  agpVirtToPhys((FxU32 *)moveData,&cmdp->baseHigh,&cmdp->baseLow);

  GDBG_INFO(1,"hb_moveCmd: MOVE cmd %s of size %d bytes from srcAddr {0x%04x,0x%08x} width %d stride %d\n",
  space_str[cmdp->space],cmdp->sizeBytes,cmdp->baseHigh,cmdp->baseLow,
  cmdp->srcWidth,cmdp->srcStride);
  GDBG_INFO(1,"\t\t\t\tto dstAddr 0x%08x stride %d\n",cmdp->fbOffset,cmdp->dstStride);

  SET(sstc->agpReqSize,cmdp->sizeBytes);
  SET(sstc->hostAddrLow,cmdp->baseLow & SST_AGP_MOVE_BASELOW );
  SET(sstc->hostAddrHigh,
      (cmdp->baseHigh << SST_AGP_SRC_BASEHIGH_SHIFT) & SST_AGP_SRC_BASEHIGH
      | (cmdp->srcWidth & SST_AGP_SRC_WIDTH)
      | (cmdp->srcStride <<SST_AGP_SRC_STRIDE_SHIFT) & SST_AGP_SRC_STRIDE);
  SET(sstc->graphicsAddr,cmdp->fbOffset & SST_AGP_FRAME_BUFFER_OFFSET);
  
  //      gdbg_info(1,"sanity %x %x %x\n",cmdp->dstStride, SST_AGP_DSTSTRIDE,cmdp->dstStride & SST_AGP_DSTSTRIDE);
  //      gdbg_info(1,"sanity %x\n",cmdp->srcStride);
  SET(sstc->graphicsStride,cmdp->dstStride & SST_AGP_DSTSTRIDE);
  SET(sstc->moveCMD,
      (cmdp->id << SST_AGPMOVE_CMDID_SHIFT) & SST_AGPMOVE_CMDID | 
      (cmdp->space << SST_AGPMOVE_SPACE_SHIFT) &  SST_AGPMOVE_SPACE);
}

void printHistoryBuffer(FxU32 bufferIndex)
{
  FxU32 j;
  char buffer[256], buffer2[256];
  char registerName[187];
  
  GDBG_INFO(0, "History Buffer %d\n", bufferIndex);
  GDBG_INFO(0, "Head: 0x%x    Tail: 0x%x    Size: 0x%x\n", histBuf[bufferIndex].head, histBuf[bufferIndex].tail, HBUF_SIZE);
  
  for(j=0; j<HBUF_SIZE; j++)
    {
      if((histBuf[bufferIndex].addr[j] & 0x3FF) == 0x2A0)
	sprintf(registerName, "sDrawTriCmd");
      else if((histBuf[bufferIndex].addr[j] & 0x3FF) == 0x2A4)
	sprintf(registerName, "sBeginTriCmd");
      else if((histBuf[bufferIndex].addr[j] & 0x3FF) == 0x80)
	sprintf(registerName, "triangleCMD");
      else
	sprintf(registerName, "Dunno");

      sprintf(buffer, "Entry[0x%03x]: Address=0x%08x(%16s)   Data=0x%08x",
	      j, histBuf[bufferIndex].addr[j], registerName, histBuf[bufferIndex].data[j]);
      
      if(histBuf[bufferIndex].head == j)
	{
	  sprintf(buffer2, " <--Head");
	  strcat(buffer, buffer2);
	}
      
      if(histBuf[bufferIndex].tail == j)
	{
	  sprintf(buffer2, " <--Tail");
	  strcat(buffer, buffer2);
	}
      
      sprintf(buffer2, "\n");
      strcat(buffer, buffer2);
      
      GDBG_INFO(0, "%s", buffer);
    }
}


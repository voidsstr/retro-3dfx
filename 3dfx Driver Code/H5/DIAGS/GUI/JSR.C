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
** $Date: 10/11/00 8:11:20 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#ifdef SST2
#include "../csim/sst2asm.h"
#else
#include "../csim/h3asm.h"
#endif

// maximum number of words to send in a row
#define MAX_SEND 32

#define FENCE

// simple user-level structure for managing the command region
static struct {
    FxBool outOfOrder;
    FxI32 roomToEnd;		// how much room is left until the end
    FxI32 roomToReadPtr;	// how much room is left before we wait
    FxU32 lastReadPtr;		// the last known position of the read ptr
    FxI32 size;			// ring buffer size within total area
    FxU32 totalSize;		// total size of CMD FIFO area
    FxU32 baseAddr;		// base address offset from chip base
    FxU32 *startAddr;		// virtual address of CMD FIFO aperture
    FxU32 *curP;
    CmdFifo *sstfifo;
} cr;

static void crInit(void)
{
    sst_idle_really(diago.sst);		// have to wait for idle before reading
    cr.sstfifo = diago.whichFifo ?
		&((SstCRegs *)SST_CMDAGP_ADDRESS(diago.sst))->cmdFifo1 :
		&((SstCRegs *)SST_CMDAGP_ADDRESS(diago.sst))->cmdFifo0;
    cr.baseAddr = GET(cr.sstfifo->baseAddrL) << 12;
    cr.startAddr = (FxU32 *)(cr.baseAddr + SST_RAW_LFB_OFFSET + SST_BASE_ADDRESS(diago.sst));
    cr.totalSize = ((GET(cr.sstfifo->baseSize)&SST_CMDFIFO_SIZE) + 1) << 12;
    cr.lastReadPtr = GET(cr.sstfifo->readPtrL);
    cr.size = diago.ringSize*4;
    if (cr.size < 0) {
	cr.size = -cr.size;
	cr.outOfOrder = 1;
    }
cr.outOfOrder = 1;		// force out of order regardless
    if (cr.size < 256) cr.size = 256;
    cr.curP = cr.startAddr + (cr.lastReadPtr-cr.baseAddr)/4;
    cr.roomToEnd = cr.size - (cr.curP - cr.startAddr)*4;
    cr.roomToReadPtr = cr.size - 4;		// very important!
    // we cannot allow the entire FIFO to be written because then if the
    // read pointer hasn't moved we cannot tell if it really hasn't moved
    // or if it has looped entirely around and executed everything
    gdbg_info(1,"CMD FIFO enabled at virt addr 0x%x, %s, %s order\n",
		cr.startAddr,
		diago.directExec ? "internal" : "offscreen",
		cr.outOfOrder ? "OUT of" : "IN");
    gdbg_info(2,"\t using a %d(0x%x) byte ring in a %d(0x%x) region\n",
		cr.size,cr.size, cr.totalSize, cr.totalSize);
}

static void makeRoom(SstRegs *sst, int n)
{
again:
    while (cr.roomToReadPtr < n) {		// do we need to stall?
	FxU32 curReadPtr = GET(cr.sstfifo->readPtrL);
	if (cr.lastReadPtr <= curReadPtr)
	    cr.roomToReadPtr += curReadPtr - cr.lastReadPtr;
	else
	    cr.roomToReadPtr += cr.size - (cr.lastReadPtr - curReadPtr);
	cr.lastReadPtr = curReadPtr;
	gdbg_info(5,"  update: %d,%d left\n",cr.roomToEnd,cr.roomToReadPtr);
    }
    if (cr.roomToEnd <= n) {		// wrap to front
	gdbg_info(5,"  wrapping with %d,%d left\n",cr.roomToEnd,cr.roomToReadPtr);
	SET(cr.curP[0],((cr.baseAddr<<SSTCP_PKT0_ADDR_SHIFT)>>2) | SSTCP_PKT0_JMP_LOCAL);
	FENCE;
	if (diago.disableHoles)	{	// if hole counting is disabled
	  P6FENCE;
	  SET(cr.sstfifo->bump,1);	// manually bump the pointer
	}
	cr.roomToReadPtr -= cr.roomToEnd;
	cr.roomToEnd = cr.size;
	cr.curP = cr.startAddr;
	goto again;
    }
    cr.roomToEnd -= n;			// compute room left
    cr.roomToReadPtr -= n;
}

#define MAX_JSR 10
#define MAX_REG 32
FxU32 regVals[MAX_REG];			// the last known value for the registers
FxU32 jsrRegs[MAX_JSR][MAX_REG];	// the last value written to a register by a jsr
FxU32 *jsrAddrs[MAX_JSR+1];		// address of each jsr in chip memory

// create a random packet and insert it into a JSR
FxU32 *createPacket(int i, FxU32 *addr)
{
    FxU32 iaddr, inc, n, mask, reg, regval;

    switch (iRandom(4)) {
	case 0:	// NOP packet
	case 3:	// NOP packet
		gdbg_info(3,"    pkt 0, nop\n");
		SET(addr[0], (i<<16) | SSTCP_PKT0_NOP);
		addr++;
		break;
	case 1:	// pkt #1, sequential regs
		n = 1+iRandom(24);
		reg = rRandom(0,26-n);
		inc = iRandom(1) ? SSTCP_INC : 0;
		iaddr = CLIP0MIN + reg*4;
		gdbg_info(3,"    pkt 1, %s, %d words @ 0x%x\n",
			inc ? "inc" : "rep", n,iaddr);
		SET(addr[0], (n<<SSTCP_PKT1_NWORDS_SHIFT) | SSTCP_PKT1_2D |
				(iaddr<<(SSTCP_REGBASE_SHIFT-2)) |
				inc | SSTCP_PKT1);
		addr++;
		while (n--) {
		    regval = rRandom(1,0xFFF);
		    SET(addr[0], regval);
		    jsrRegs[i][reg] = regval;
		    addr++;
		    if (inc) reg++;
		}
		break;
	case 2:	// 2D packet
		reg = 0;
		do {
		    mask = iRandom(0x1FFFFFF);	// can write 25 regs
		} while (mask==0);
		gdbg_info(3,"    pkt 2, mask=0x%x\n",mask);
		SET(addr[0], (mask<<SSTCP_PKT2_MASK_SHIFT) | SSTCP_PKT2);
		addr++;
		while (mask) {
		    if (mask & 1) {
			regval = rRandom(1,0xFFF);
			SET(addr[0], regval);
			jsrRegs[i][reg] = regval;
			addr++;
		    }
		    reg++;
		    mask >>= 1;
		}
		break;
	case 4: // pkt #4, bitmask
		reg = 0;
		iaddr = CLIP0MIN;
		do {
		    mask = iRandom(SSTCP_PKT4_MASK>>SSTCP_PKT4_MASK_SHIFT);
		} while (mask==0);
		gdbg_info(3,"    pkt 4, mask=0x%x @ 0x%x\n",mask,iaddr);
		SET(addr[0], (mask<<SSTCP_PKT4_MASK_SHIFT) | SSTCP_PKT4_2D |
				(iaddr<<(SSTCP_REGBASE_SHIFT-2)) | 
				SSTCP_PKT4);
		addr++;
		while (mask) {
		    if (mask & 1) {
			regval = rRandom(1,0xFFF);
			SET(addr[0], regval);
			jsrRegs[i][reg] = regval;
			addr++;
		    }
		    reg++;
		    mask >>= 1;
		}
		break;
    }
    return addr;
}

// create a JSR subroutine in command fifo memory with some random packets
static void createJsr(SstRegs *sst, int i)
{
    FxU32 *addr;
    int r,packets;

    for (r=0; r<MAX_REG; r++)	// clear out which regs I change
	jsrRegs[i][r] = 0;

    addr = jsrAddrs[i] = jsrAddrs[i] + iRandom(6);
    
    packets = iRandom(13);		// put up to 13 packets in this jsr
    gdbg_info(2,"jsr #%d defined at 0x%x, contains %d packets\n",i,addr,packets);
    addr += SST_RAW_LFB_OFFSET/4 + SST_BASE_ADDRESS(diago.sst)/4;
    while (packets--) {
	addr = createPacket(i,addr);
    }

    // finish it off with a return packet
    gdbg_info(3,"    pkt 0, ret\n");
    SET(addr[0], (i<<16)|SSTCP_PKT0_RET);
    jsrAddrs[i+1] = addr+1 - SST_RAW_LFB_OFFSET/4 - SST_BASE_ADDRESS(diago.sst)/4;
}

// we have to issue an NOP packet thru the cmd fifo
static void idle_cmd_fifo(SstRegs *sst)
{
    makeRoom(sst,2*4);			// make room for NOP packet
    SET(cr.curP[0],(1<<SSTCP_PKT1_NWORDS_SHIFT) |
			(NOPCMD<<(SSTCP_REGBASE_SHIFT-2)) | SSTCP_PKT1);
    SET(cr.curP[1],0);
    cr.curP += 2;
    if (diago.disableHoles) {		// if hole counting is disabled
      P6FENCE;
      SET(cr.sstfifo->bump,2);	// manually bump the pointer
    }
    fxHalIdleNoNop(sst);
}

FxU32 nopORjsr(FxU32 flag)
{
    if (iRandom(2)==0) {	// call a JSR
	int r;
	FxU32 jsr = iRandom(MAX_JSR-1);
	gdbg_info(3,"    jsr #%d @ 0x%x\n",jsr,jsrAddrs[jsr]);
	// keep track of register values by emulating the
	// execution of the JSR right now
	for (r=0; r<MAX_REG; r++) {
	    if (jsrRegs[jsr][r])
		regVals[r] = jsrRegs[jsr][r];
	}
	return (((FxU32)(jsrAddrs[jsr])<<(SSTCP_PKT0_ADDR_SHIFT-2))&SSTCP_PKT0_ADDR) |
			SSTCP_PKT0_JSR;
    }
    else {
	gdbg_info(3,"    nop\n");
	return (flag<<16) | SSTCP_PKT0_NOP ;
    }
}

void
main (int argc, char **argv)
{
    int j;
    SstRegs *sst;
    SstGRegs *sstg;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);
    if (!diago.writeFifo) {
	GDBG_ERROR("jsr", "must run with -W option to enable CMD FIFO\n");
        DIAG_FAIL();
    }

    // reserve 2 pages for jsrs
#ifdef CVG
    diago.maxTrashMem -= 0x2000;
#else
    diago.minTrashMem += 0x2000;
#endif

    // re-init the CMD FIFO, this is very bad and tricky, we init twice
    crInit();
    if (diago.writeFifo)	// HACK: don't funnel SETs thru diagStore*
	diago.writeFifo = 0;	// we write our own packets in this diag

    while (DIAG_STARTPASS())			// for each pass
    for (j=0; j<25; j++) {
	int limit,i,n;

#ifdef CVG
	jsrAddrs[0] = (FxU32 *)(cr.baseAddr - 0x2000);
#else
	jsrAddrs[0] = (FxU32 *)(diago.minTrashMem - 0x2000);
#endif
	
	for (i=0; i<MAX_JSR; i++)
	    createJsr(sst,i);
	gdbg_info(2,"jsr definitions complete @ 0x%x---------------------\n",
			jsrAddrs[MAX_JSR]);
#ifdef CVG
	if (jsrAddrs[MAX_JSR] > (FxU32 *)(cr.baseAddr))
	    GDBG_ERROR("main","jsr data overwrote CMD FIFO area\n");
#else
	if (jsrAddrs[MAX_JSR] < (FxU32 *)(diago.minTrashMem - 0x2000))
	    GDBG_ERROR("main","jsr data overwrote CMD FIFO area\n");
#endif
	limit = 256;				// do this many writes
	while (limit > 0) {
	    n = MAX_SEND;
	    if (n > (signed)cr.size/4/2)		// prevent overlap with read ptr
		n = cr.size/4/2;
	    n = 1+iRandom(iRandom(iRandom(n-1)));
	    gdbg_info(2,"writing %d dwords, room=%d,%d\n",n,cr.roomToEnd,cr.roomToReadPtr);
	    makeRoom(sst,n*4);

	    i = iRandom(3);			// some of the time send sequential
	    if (i == 0 || diago.directExec) {	// or if direct EXEC mode
		for (i=0; i<n; i++) {
		    SET(cr.curP[i], nopORjsr(i));
		}
	    }
	    else {	// send n words out of order to simulate a stupid P6
		FxU32 packetData[MAX_SEND];
		FxU32 send_order[MAX_SEND];

		for (i=0; i<n; i++) {
		    packetData[i] = nopORjsr(i);
		}
		scrambleRandom(n,send_order);
		for (i=0; i<n; i++) {
		    SET(cr.curP[send_order[i]],packetData[send_order[i]]);
		}
	    }
	    cr.curP += n;
	    limit -= n;
	    if (diago.disableHoles) {		// if hole counting is disabled
	      P6FENCE;
	      SET(cr.sstfifo->bump,n);	// manually bump the pointer
	    }
	}

	// wait for everything to quiet down
	idle_cmd_fifo(sst);

	// now test them, note we wrote 1 to the last register and 13 to the first
	DIAG_TESTREG32("clip0min",	regVals[0], GET(sstg->clip0min));
	DIAG_TESTREG32("clip0max",	regVals[1], GET(sstg->clip0max));
	DIAG_TESTREG32("dstBaseAddr",	regVals[2], GET(sstg->dstBaseAddr));
	DIAG_TESTREG32("dstFormat",	regVals[3], GET(sstg->dstFormat));
	DIAG_TESTREG32("srcColorkeyMin",regVals[4], GET(sstg->srcColorkeyMin));
	DIAG_TESTREG32("srcColorkeyMax",regVals[5], GET(sstg->srcColorkeyMax));
	DIAG_TESTREG32("dstColorkeyMin",regVals[6], GET(sstg->dstColorkeyMin));
	DIAG_TESTREG32("dstColorkeyMax",regVals[7], GET(sstg->dstColorkeyMax));
	DIAG_TESTREG32("bresError0",	regVals[8], GET(sstg->bresError0));
	DIAG_TESTREG32("bresError1",	regVals[9], GET(sstg->bresError1));
	DIAG_TESTREG32("rop",		regVals[10], GET(sstg->rop));
	DIAG_TESTREG32("srcBaseAddr",	regVals[11], GET(sstg->srcBaseAddr));
	DIAG_TESTREG32("commandEx",	regVals[12]&0xF, GET(sstg->commandEx));
	DIAG_TESTREG32("lineStipple",	regVals[13], GET(sstg->lineStipple));
	DIAG_TESTREG32("lineStyle",	regVals[14], GET(sstg->lineStyle));
	DIAG_TESTREG32("pattern0alias", regVals[15], GET(sstg->pattern0alias));
	DIAG_TESTREG32("pattern1alias", regVals[16], GET(sstg->pattern1alias));
	DIAG_TESTREG32("clip1min",	regVals[17], GET(sstg->clip1min));
	DIAG_TESTREG32("clip1max",	regVals[18], GET(sstg->clip1max));
	DIAG_TESTREG32("srcFormat",	regVals[19], GET(sstg->srcFormat));
	DIAG_TESTREG32("srcSize",	regVals[20], GET(sstg->srcSize));
	DIAG_TESTREG32("srcXY",		regVals[21], GET(sstg->srcXY));
	DIAG_TESTREG32("colorBack",	regVals[22], GET(sstg->colorBack));
	DIAG_TESTREG32("colorFore",	regVals[23], GET(sstg->colorFore));
	DIAG_TESTREG32("dstSize",	regVals[24], GET(sstg->dstSize));
	DIAG_TESTREG32("dstXY",		regVals[25], GET(sstg->dstXY));
    }
    SET(cr.sstfifo->baseSize,0);	// disable CMD fifo
    fxHalIdleNoNop(sst);
    DIAG_PASS(0);
}

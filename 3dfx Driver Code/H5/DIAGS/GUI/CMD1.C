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
** $Date: 10/11/00 8:11:08 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#ifdef SST2
#include "../csim/sst2asm.h"
#else
#include "../csim/h3asm.h"
#endif

// maximum number of words to send in a row
#define MAX_SEND 64

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
    sst_idle_really(diago.sst);
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
    cr.roomToEnd = cr.size;
    cr.roomToReadPtr = cr.size - 4;		// very important!
    cr.curP = (FxU32 *)(cr.lastReadPtr+ SST_RAW_LFB_OFFSET + SST_BASE_ADDRESS(diago.sst));
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
	if (diago.disableHoles) {		// if hole counting is disabled
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

void
main (int argc, char **argv)
{
    int j;
    SstRegs *sst;
    SstGRegs *sstg;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);
    if (!diago.writeFifo) {
	GDBG_ERROR("cmd1", "must run with -W option to enable CMD FIFO\n");
        DIAG_FAIL();
    }

    crInit();
    if (diago.writeFifo)	// HACK: don't funnel SETs thru diagStore*
	diago.writeFifo = 0;	// we write our own packets in this diag

    while (DIAG_STARTPASS())			// for each pass
    for (j=0; j<10; j++) {
	int limit,i,n;
	FxU32 send_order[MAX_SEND];

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
		    SET(cr.curP[i], (i<<16)|SSTCP_PKT0_NOP);
		}
	    }
	    else {	// send n words out of order to simulate a stupid P6
		scrambleRandom(n,send_order);
		for (i=0; i<n; i++) {
		    SET(cr.curP[send_order[i]], (send_order[i]<<16)|SSTCP_PKT0_NOP);
		}
	    }
	    cr.curP += n;
	    limit -= n;
	    if (diago.disableHoles) {		// if hole counting is disabled
	      P6FENCE;
	      SET(cr.sstfifo->bump,n);	// manually bump the pointer
	    }
	}

	// now write 13 2D registers to try to confuse things
	n = 14;			// 13 regs + 1 packet header
	makeRoom(sst,n*2*4);
	// write all the 2D regs in backwards order to the CMD fifo
	SET(cr.curP[n],(0x1FFF << SSTCP_PKT2_MASK_SHIFT) | SSTCP_PKT2);
	for (i=n-1; i>0; i--)
	    SET(cr.curP[n+i],n-i);

	// write all the 2D regs with 0xdead ahead of the good data
	SET(cr.curP[0],(0x1FFF << SSTCP_PKT2_MASK_SHIFT) | SSTCP_PKT2);
	for (i=1; i<n; i++)
	    SET(cr.curP[i],0xdead);
	cr.curP += n*2;
	if (diago.disableHoles)	{	// if hole counting is disabled
	  P6FENCE;
	  SET(cr.sstfifo->bump,n*2);	// manually bump the pointer
	}

	// wait for everything to quiet down
	idle_cmd_fifo(sst);

	// now test them, note we wrote 1 to the last register and 13 to the first
	DIAG_TESTREG32("commandEx",	1, GET(sstg->commandEx));
	DIAG_TESTREG32("srcBaseAddr",	2, GET(sstg->srcBaseAddr));
	DIAG_TESTREG32("rop",		3, GET(sstg->rop));
	DIAG_TESTREG32("bresError1",	4, GET(sstg->bresError1));
	DIAG_TESTREG32("bresError0",	5, GET(sstg->bresError0));
	DIAG_TESTREG32("dstColorkeyMax",6, GET(sstg->dstColorkeyMax));
	DIAG_TESTREG32("dstColorkeyMin",7, GET(sstg->dstColorkeyMin));
	DIAG_TESTREG32("srcColorkeyMax",8, GET(sstg->srcColorkeyMax));
	DIAG_TESTREG32("srcColorkeyMin",9, GET(sstg->srcColorkeyMin));
	DIAG_TESTREG32("dstFormat",	10, GET(sstg->dstFormat));
	DIAG_TESTREG32("dstBaseAddr",	11, GET(sstg->dstBaseAddr));
	DIAG_TESTREG32("clip0max",	12, GET(sstg->clip0max));
	DIAG_TESTREG32("clip0min",	13, GET(sstg->clip0min));
    }
    SET(cr.sstfifo->baseSize,0);	// disable CMD fifo
    fxHalIdleNoNop(sst);
    DIAG_PASS(0);
}

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
** $Revision: 4$
** $Date: 10/11/00 8:18:46 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

#define SSTCP_RESET {	\
	ph->addrMax = ph->addrSave = (FxU32) (cr->startAddr-1);	\
	ph->addrExec = ph->addrMax+4;	\
}

#define SETCP(d,s) _setcp(&(d),s)

// simple user-level structure for managing the command region
typedef struct {
    FxU32 size;
    FxI32 room;
    FxU32 *startAddr;
    FxU32 *curP;
} CommandRegion;

CommandRegion foobar;

typedef struct {
    FxU32 addrMax;
    FxU32 addrSave;
    FxU32 addrExec;
    FxU32 holeCount;
    FxU32 execCount;
    FxU32 startAddr;
    FxU32 total_execCount;
} Pothole;

Pothole foopot;

FxU32 gold_read, gold_write, gold_size;	// ring pointers
FxU32 *golden;				// golden expected data


void _setcp(void *addr, FxU32 data)
{
    FxU32 iaddr = (FxU32)addr;

    gdbg_info(10,"    %x",addr);
    gdbg_info_more(14,"(%d)",data);
    *(FxU32 *)addr = data;
restart:
    if (iaddr > foopot.addrMax) {
	int newWords = (iaddr - foopot.addrMax - 4)>>2;
	if (foopot.holeCount == 0 && newWords == 0) {
	    gdbg_info_more(12," == aMax+1");
	    foopot.addrMax = iaddr;
	    goto add_some;
	}

	foopot.holeCount += newWords;
	gdbg_info_more(12," > %x, holeCount = %d",foopot.addrMax,foopot.holeCount);
	foopot.addrMax = iaddr;
    }
    else if (iaddr < foopot.addrMax) {
	if (iaddr < foopot.addrSave && foopot.addrSave <= foopot.addrMax) {
	    gdbg_info_more(12, "auto detected wrap\n");
	    foopot.addrMax = foopot.addrSave = foopot.startAddr-4;
	    foopot.addrExec = foopot.addrMax+4;
	    // this would really be done by the ???
	    goto restart;
	}
	foopot.holeCount--;
	gdbg_info_more(12," < %x, holeCount = %d",foopot.addrMax,foopot.holeCount);
	if (foopot.holeCount == 0) {
add_some:
	    foopot.execCount += (foopot.addrMax - foopot.addrSave)>>2;
	    foopot.addrSave = foopot.addrMax;
	    gdbg_info_more(12," , execCount = %d",foopot.execCount);

// XXX try exec'ing random number!!!
	    // now execute everything
	    gdbg_info_more(16,"\n");
	    while (foopot.execCount > 0) {
		FxU32 data;

		data = *(FxU32 *)foopot.addrExec;	// get the next DWORD
		foopot.addrExec += 4;			// bump the pointer
		foopot.execCount--;			// dec. execCount
		foopot.total_execCount++;
		gdbg_info(16,"\texec %d\n",data);
		if (data != golden[gold_read]) {
		    GDBG_ERROR("_setcp", "data mismatch, expecting %d got %d\n",
				golden[gold_read],data);
		    DIAG_INCERROR();
		}
		gold_read++;
		if (gold_read >= gold_size) gold_read = 0;
	    }
	}
    }
    else if (iaddr == foopot.addrMax) {
	GDBG_ERROR("pothole","addr == addrMax\n");
	DIAG_INCERROR();
    }
    gdbg_info_more(12,"\n");
}

#define FENCE

// #define SSTCP_WRAP(sst) { FENCE; SSTCP_RESET; }
// theoretically, we can detect the wrap condition in the chip
#define SSTCP_WRAP(sst) { FENCE; }

// maximum number of words to send in a row
#define MAX_SEND 128

void
main (int argc, char **argv)
{
    SstRegs *sst;
    CommandRegion *cr = &foobar;
    Pothole *ph = &foopot;

    sst = SST_BEGIN(argc,argv);

    // Print Out Option Description
    if ( diago.printOpts ) {
        gdbg_printf( "replay option description:\n" );
        gdbg_printf( " -o # -> set ring buffer size (default = 1024)\n");
	DIAG_FAIL();
    }
    cr->size = 1024;
    if (diago.option)
	cr->size = diago.option;
    cr->startAddr = (FxU32 *)malloc(sizeof(FxU32) * cr->size);
    cr->room = cr->size - 1;
    cr->curP = cr->startAddr;
    ph->startAddr = (FxU32)cr->startAddr;
    gdbg_info(1,"ring buffer size = %d words\n",cr->size);

    // golden holds the expected data in the expected order
    gold_size = cr->size;
    gold_write = 0;
    gold_read = 0;
    golden = (FxU32 *)malloc(sizeof(FxU32)*gold_size);

    // XXX this code goes into CSIM
    ph->holeCount = 0;
    ph->execCount = 0;
    SSTCP_RESET;

    while (DIAG_STARTPASS()) {			// for each pass
	int limit,i,n;
	FxU32 send_order[MAX_SEND];

	limit = 1000000;			// do this many writes
	while (limit > 0) {
	    n = 1+iRandom(iRandom(iRandom(MAX_SEND-1)));
	    gdbg_info(2,"writing %d dwords, room=%d\n",n,cr->room);

	    if (cr->room < n) {			// wrap to front
		gdbg_info(5,"  wrapping with %d left\n",cr->room+n);
		cr->curP[0] = SSTCP_PKT0_JMP_LOCAL;	// issue a jump
		cr->room = cr->size - 1;
		cr->curP = cr->startAddr;
		SSTCP_WRAP(sst);
		if (ph->holeCount) {
		    GDBG_ERROR("main","holeCount is %d and should be 0\n",ph->holeCount);
		    DIAG_INCERROR();
		}
	    }					// and continue

	    if (iRandom(3) == 0) {		// some of the time send sequential
		for (i=0; i<n; i++) {
		    golden[gold_write++] = i;
		    if (gold_write >= gold_size) gold_write = 0;
		    SETCP(cr->curP[i], i);
		}
	    }
	    else {	// send n words out of order to simulate a stupid P6
		scrambleRandom(n,send_order);
		for (i=0; i<n; i++) {
		    golden[gold_write++] = i;
		    if (gold_write >= gold_size) gold_write = 0;
		    SETCP(cr->curP[send_order[i]], send_order[i]);
		}
	    }
	    cr->room -= n;			// compute room left
	    cr->curP += n;
	    limit -= n;
	}
	n = 1000000 - limit;
	if (ph->total_execCount != (unsigned)n) {
	     GDBG_ERROR("main", "execCount should be %d but is %d\n",
			n,ph->total_execCount);
	    DIAG_INCERROR();
	}
	ph->total_execCount = 0;
    }
    DIAG_PASS(0);
}

/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:10:41 PM$
*/

#include <fxos.h>

#include "udiag.h"
#include "sstdiag.h"

// NOTE: this is a special diagnostic in that it requires the simulator
//	 to track real time.  It should be run in one of the following modes:
//		1) without CSIM gfx output (set GSIM_WIN32=)
//		2) in a very small window (-w 20) so that Blits don't take long
//		3) on the real hardward without CSIM (set SST_REALHW=-1)

extern int diagSwaps;		// HACK: reach int sstdiag.c

void
main (int argc, char **argv)
{
    int i,j,k,n;
    float time, target;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    while (DIAG_STARTPASS()) {			// for each pass
	// test NOT waiting for VSYNC
	n = rRandom(100,120);			// chose between 100-120 swaps
	gdbg_info(2,"swap %d times NO vsync\n",n);
	timer(0);				// start clock timer

	for (i=0; i<n; i++)			// queue up swaps
	    SET(sst->swapbufferCMD,0);
	sst_idle_really(sst);			// wait for idle
	diagSwaps += n;

	time = timer(1);
	gdbg_info(3,"  time: %.2f secs\n",time);
	if (time > 1.0F) {
	    gdbg_printf("ERROR: time limit of 1 second exceeded\n");
	    DIAG_INCERROR();
	}

	// test waiting for VSYNC
	n = rRandom(100,120);			// chose between 100-120 swaps
	gdbg_info(2,"swap %d times WITH vsync\n",n);
	timer(0);				// start clock timer

	k = 1;

	for (i=0; i<n; i++)			// queue up swaps
	    SET(sst->swapbufferCMD,k);
	sst_idle_really(sst);			// wait for idle
	diagSwaps += n;

	time = timer(1);
	gdbg_info(3,"  time: %.2f secs\n",time);
	if (time < 1.0F) {
	    gdbg_printf("ERROR: time limit of 1 second not reached\n");
	    DIAG_INCERROR();
	}

	// test swap interval counter
	for (j=1; j<256; j++) {			// test every swapinterval
	    n = j > 40 ? 1 : 60/j;
	    gdbg_info(2,"swap %d times WITH interval=%d\n",n,j);
	    timer(0);				// start clock timer

	    for (i=0; i<n; i++)			// queue up swaps
		SET(sst->swapbufferCMD,((j-1)<<1) | 1);

	    sst_idle_really(sst);			// wait for idle
	    diagSwaps += n;

	    time = timer(1);
	    gdbg_info(3,"  time: %.2f secs\n",time);
	    // GMT: note we assume 60 Hz refresh here!!!
	    target = .9F * n*j/60.0F;
	    if (time < target) {
		gdbg_printf("ERROR: time limit of %.2f second not reached\n", target);
		DIAG_INCERROR();
	    }
	    target = 1.1F * n*j/60.0F;
	    if (time > target) {
		gdbg_printf("ERROR: time limit of %.2f second exceeded\n", target);
		DIAG_INCERROR();
	    }
	}
    }
    DIAG_PASS(0);
}
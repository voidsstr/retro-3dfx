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
** $Date: 10/11/00 8:10:20 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int n;
    long cx,cy,x,y, fbzMode,nop;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    SET(sst->r, 0xdeadb);		// set the color
    SET(sst->g, 0xbeefd);		// set the color
    SET(sst->b, 0xbadba);		// set the color

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<100; n++) {			// do 100 tests
	xyRandom(&x,&y);
	fbzMode = iRandom(0xFFFF);
	nop = iRandom(0xF);
	gdbg_info(2,"nop(%d) x,y = %d,%d  fbzMode = 0x%x\n",
			nop,x,y,fbzMode);

	SET(sst->vA.x,x<<SST_XY_FRACBITS);
	SET(sst->vA.y,y<<SST_XY_FRACBITS);
	SET(sst->vB.x,(x+4)<<SST_XY_FRACBITS);
	SET(sst->vB.y,y<<SST_XY_FRACBITS);
	SET(sst->vC.x,x<<SST_XY_FRACBITS);
	SET(sst->vC.y,(y+4)<<SST_XY_FRACBITS);
	SET(sst->fbzMode,fbzMode);
	SET(sst->nopCMD,nop);		// NOP comand

	sst_idle_really(sst);		// wait for the command to complete
	// read pixels around x,y, verify == 0
	for (cy = y-1; cy <= y+1; cy++)
	for (cx = x-1; cx <= x+1; cx++)
	{
	    DIAG_TEST_PIXEL(diago.curdrawbuffer,cx,cy,0);
	}
    }
    DIAG_PASS(1);
}

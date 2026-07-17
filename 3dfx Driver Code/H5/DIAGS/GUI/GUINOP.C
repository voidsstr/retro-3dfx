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
** $Date: 10/11/00 8:11:15 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int n;
    long cx,cy,x,y, cmdops;
    SstRegs *sst;
    SstGRegs *sstg;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);

    SET(sstg->colorBack, 0xdeadbeef);		// set the color
    SET(sstg->colorFore, 0xdeadbeef);		// set the color
    SET(sstg->clip0min,0x00000000);
    SET(sstg->clip0max,0xFFFFFFFF);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<10; n++) {			// do 10 tests
	xyRandom(&x,&y);
	cmdops = iRandom(0xFFFFFF) << 15;
	gdbg_info(2,"x,y = %d,%d  opts = 0x%x\n",x,y,cmdops);

	SET(sstg->dstXY,  (y<<16) | x);
	SET(sstg->dstSize, (y<<16) | x);
	SET(sstg->command, SSTG_NOP | SSTG_GO | cmdops | (SSTG_ROP_SRC<<SSTG_ROP0_SHIFT));

	sst_idle_really(sst);		// wait for the command to complete

	/* read pixels around x,y, verify == 0	*/
	for (cy = y-1; cy <= y+1; cy++)
	for (cx = x-1; cx <= x+1; cx++)
	{
	    DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,cx,cy,0);
	}
    }
    DIAG_PASS(1);
}

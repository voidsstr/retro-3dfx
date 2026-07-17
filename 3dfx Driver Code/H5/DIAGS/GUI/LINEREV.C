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
** $Date: 10/11/00 8:11:21 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

SstRegs *sst;
SstGRegs *sstg;

// test 1 random line with specified length
void test_1line_length(int len)
{
    long x1,y1, x2,y2;
    FxU32 csrc;
    FxU32 cmd = SSTG_LINE | SSTG_REVERSIBLE | (SSTG_ROP_XOR<<SSTG_ROP0_SHIFT);

    xyRandom(&x1,&y1);
    x2 = rRandom(x1-len,x1+len);
    y2 = rRandom(y1-len,y1+len);

    csrc = colRandom32();
    SET(sstg->colorFore, csrc);

    gdbg_info(2,"--line = %d,%d to %d,%d color=%x\n",
		    x1,y1,x2,y2,csrc);
    sstg_drawline(sstg, x1,y1,x2,y2, cmd);	// draw it forwards
    sstg_drawline(sstg, x2,y2,x1,y1, cmd);	// backwards with XOR
}

//----------------------------------------------------------------------
// This diag draws 100 random lines per pass and erases them by drawing
// the line again in the other direction with rop=XOR.  No checking is done
// until the diag is done, when the entire screen is checked for blackness.
//----------------------------------------------------------------------
void
main (int argc, char **argv)
{
    int n;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);

    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);
	       
    while (DIAG_STARTPASS()) {			// for each pass
	for (n=0; n<200; n++)				// do some lines
	    test_1line_length(iRandom(diago.tsize));	// of test size
    }
    DIAG_PASS(1);				// check for black screen
}

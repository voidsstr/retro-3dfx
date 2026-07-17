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
** $Date: 10/11/00 8:11:12 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

#define REVERSIBLE	0x1
#define POLYLINE	0x2
#define BRESERROR	0x4

int checkNeighbors;
SstRegs *sst;
SstGRegs *sstg;

// test a line by drawing it and checking every pixel
void test_line(long x1, long y1, long x2, long y2, FxU32 csrc)
{
    FxU32 cmd = (diago.option & POLYLINE ? SSTG_POLYLINE : SSTG_LINE) | 
		(diago.option & REVERSIBLE ? SSTG_REVERSIBLE : 0);
    static FxU32 berr;

    // NOTE: only generate new error term if color is not zero
    if ((diago.option & BRESERROR) && (csrc!=0)) {
	int adx, ady;
	adx = x1-x2;
	ady = y1-y2;
	if (adx < 0) adx = -adx;
	if (ady < 0) ady = -ady;
	if (adx >= ady)			// X major
	    berr = rRandom(ady-2*adx,3*ady)&0xFFFF | 0x80000000;
	else
	    berr = rRandom(adx-2*ady,3*adx)&0xFFFF | 0x80000000;
	gdbg_info(2,"--line = %d,%d to %d,%d color=0x%08x breserror = 0x%08x\n",
		    x1,y1,x2,y2,csrc, berr);
	SET(sstg->bresError0, berr);
    }
    else {
	gdbg_info(2,"--line = %d,%d to %d,%d color=0x%08x\n",
		    x1,y1,x2,y2,csrc);
    }
    cmd |= SSTG_ROP_SRC << SSTG_ROP0_SHIFT;
    SET(sstg->colorFore, csrc);
    sstg_drawline(sstg, x1,y1,x2,y2, cmd);
    sstg_idle(sst);			// wait for the command to complete
    sstg_checkline(sstg, x1,y1,x2,y2, cmd,0, csrc,0,0, checkNeighbors);
}

// test all possible lines with specified length
void test_all_lengths(int len)
{
    int i;
    long x1,y1, x2,y2;
    FxU32 csrc;

    xyRandom(&x1,&y1);				// pick random x,y
    csrc = colRandom32();
    gdbg_info(2,"test_all(%d)\n",len);

    for (i= -len; i<=len; i++) {
	x2 = x1 - len;
	y2 = y1 + i;
	test_line(x1,y1,x2,y2,csrc);		// draw it
	if ( diago.checkEveryTriangle ) {
	  if (diago.option & REVERSIBLE)		// and erase it
	    test_line(x2,y2,x1,y1,0);
	  else
	    test_line(x1,y1,x2,y2,0);
	}

	x2 = x1 + len;
	test_line(x1,y1,x2,y2,csrc);		// draw it
	if ( diago.checkEveryTriangle ) {
	  if (diago.option & REVERSIBLE)		// and erase it
	    test_line(x2,y2,x1,y1,0);
	  else
	    test_line(x1,y1,x2,y2,0);
	}
    }

    for (i= 1-len; i<len; i++) {
	x2 = x1 + i;
	y2 = y1 - len;
	test_line(x1,y1,x2,y2,csrc);		// draw it
	if ( diago.checkEveryTriangle ) {
	  if (diago.option & REVERSIBLE)		// and erase it
	    test_line(x2,y2,x1,y1,0);
	  else
	    test_line(x1,y1,x2,y2,0);
	}

	y2 = y1 + len;
	test_line(x1,y1,x2,y2,csrc);		// draw it
	if ( diago.checkEveryTriangle ) {
	  if (diago.option & REVERSIBLE)		// and erase it
	    test_line(x2,y2,x1,y1,0);
	  else
	    test_line(x1,y1,x2,y2,0);
	}
    }
}

// test 1 random line with specified length
void test_1line_length(int len)
{
    long x1,y1, x2,y2;
    long xdelta, ydelta;
    FxU32 csrc;

    // GREG - Let's not always have the src onscreen, shall we?!
    len = (len>>1);				// We are going to randomize on either side of the on-screen point
    xyRandom(&x1,&y1);				// Get a point on the screen
    xdelta = rRandom(-len, len);
    ydelta = rRandom(-len, len);
    x2 = x1 + xdelta + iRandom(1);		// Don't always generate even-length lines
    y2 = y1 + ydelta + iRandom(1);
    x1 = x1 - xdelta;
    y1 = y1 - ydelta;

    csrc = colRandom32();
    test_line(x1,y1,x2,y2,csrc);		// draw it
    if ( diago.checkEveryTriangle ) {
      if (diago.option & REVERSIBLE)		// and erase it
	test_line(x2,y2,x1,y1,0);
      else
	test_line(x1,y1,x2,y2,0);
    }
}

void
main (int argc, char **argv)
{
    int n;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);

    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);
    SET(sstg->colorBack, 0);
	       
    // Print Out Option Description
    if ( diago.printOpts ) {
	gdbg_printf( "line option description (bit fields):\n" );
	gdbg_printf( " %d -> REVERSIBLE lines\n", REVERSIBLE );
	gdbg_printf( " %d -> POLYLINE   lines\n", POLYLINE );
	gdbg_printf( " %d -> BRESERROR  lines\n", BRESERROR );

        DIAG_FAIL();
    }
    checkNeighbors = (diago.option & (REVERSIBLE|POLYLINE)) != (REVERSIBLE|POLYLINE);
    if ( ! diago.checkEveryTriangle && checkNeighbors ) {
      GDBG_INFO(0,"Forcing checkNeighbors OFF\n");
      checkNeighbors = 0;
    }
    if ((diago.option & (REVERSIBLE|BRESERROR)) == (REVERSIBLE|BRESERROR)) {
	GDBG_ERROR("line", "cannot use REVERSIBLE and BRESERROR options at the same time\n" );
        DIAG_FAIL();
    }

    while (DIAG_STARTPASS()) {			// for each pass
	test_all_lengths(0);			// test all zero length lines
	test_all_lengths(1);			// etc.
	test_all_lengths(2);
	test_all_lengths(3);
	for (n=0; n<100; n++)				// do 100 tests
	    test_1line_length(diago.tsize);		// of test size
    }
    DIAG_PASS(checkNeighbors);
}

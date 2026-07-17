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
** $Date: 10/11/00 8:11:23 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

#define REVERSIBLE	0x1
#define POLYLINE	0x2
#define BRESERROR	0x4

void
main (int argc, char **argv)
{
    int n,loadStyle;
    FxU32 stip, style, cfore, cback;
    FxU32 rop, srcFormat, cmdops,cmdXops;
    long x1,y1, x2,y2, temp;
    SstRegs *sst;
    SstGRegs *sstg;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);

    // HACK: fetch the srcFormat from the actual CSIM
    srcFormat = CSIM_PRIVATE(diago.sstCSIM)->gui.srcFormat;

    SET(sstg->clip0min,0x00000000);
    SET(sstg->clip0max,0xFFFFFFFF);
    SET(sstg->lineStyle, style=0xFF);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<100; n++) {			// do 100 tests
	if ((n % 19) == 0) {			// init pattern once in a while
	    sstg_setpattern_random(sstg);
	}
	if (diago.clamp) {			// if random clipping
	    SET(sstg->clip0min, (iRandom(87)<<16) | iRandom(99));
	    SET(sstg->clip0max, ((diago.ymaxscreen-iRandom(25)-1)<<16) | (diago.xmaxscreen-iRandom(50)-1));
	    SET(sstg->clip1min, (iRandom(37)<<16) | iRandom(49));
	    SET(sstg->clip1max, ((diago.ymaxscreen-iRandom(75)-1)<<16) | (diago.xmaxscreen-iRandom(90)-1));
	}

	{ 
	  FxI32 xdelta, ydelta;
	  FxI32 len = diago.tsize >> 1;
	  
	  xyRandom(&x1,&y1);				// Get a point on the screen
	  xdelta = rRandom(-len, len);
	  ydelta = rRandom(-len, len);
	  x2 = x1 + xdelta + iRandom(1);		// Don't always generate even-length lines
	  y2 = y1 + ydelta + iRandom(1);
	  x1 = x1 - xdelta;
	  y1 = y1 - ydelta;
	}

	cfore = colRandom32();
	SET(sstg->colorFore,cfore);

	sstg_random_command_bits(&cmdops,&cmdXops);
	sstg_random_colors(sstg, cfore,&cback,0);
	cmdops |= iRandom(1) ? SSTG_LINE : SSTG_POLYLINE;
	cmdops |= SSTG_EN_LINESTIPPLE;

	stip = iRandom(0xFFFFFFFF);
	if (loadStyle = iRandom(1) ) {
	    style = iRandom(iRandom(iRandom(0xFF)));	// repeat count
	    style = iRandom(style);	// skew it towards smaller ones
	    style |= (temp=iRandom(0x1F)) << SSTG_LSSIZE_SHIFT;	// size
	    style |= iRandom(temp) << SSTG_LSPOS_INT_SHIFT;	// int pos
	    style |= iRandom((style&0xFF)) << SSTG_LSPOS_FRAC_SHIFT;	// frac pos
	}
	if (diago.rectangular) {		// if random rops
	    cmdops |= iRandom(0xFF) << SSTG_ROP0_SHIFT;
	    rop = iRandom(0xFFFFFF);		// then pick totally random
	}
	else {					// else use SRC for all rops
	    cmdops |= SSTG_ROP_SRC << SSTG_ROP0_SHIFT;
	    rop = SSTG_ROP_SRC | (SSTG_ROP_SRC<<8) | (SSTG_ROP_SRC<<16);
	}
	SET(sstg->rop, rop);			// set the rop and colors
	SET(sstg->commandEx, cmdXops);
	sstgCheckForIdle(sst, cmdXops);
	SET(sstg->lineStipple, stip);
	if (loadStyle) SET(sstg->lineStyle, style);

	gdbg_info(2,"\n");
	gdbg_info(2,"%sline = %d,%d to %d,%d  stip=0x%08x ld=%d style=%d %d %d.%d\n",
			(cmdops & SSTG_COMMAND)==SSTG_LINE ? "" : "poly",
			x1,y1,x2,y2,stip,loadStyle,
			(style & SSTG_LSREPEAT) >> SSTG_LSREPEAT_SHIFT,
			(style & SSTG_LSSIZE) >> SSTG_LSSIZE_SHIFT,
			(style & SSTG_LSPOS_INT) >> SSTG_LSPOS_INT_SHIFT,
			(style & SSTG_LSPOS_FRAC) >> SSTG_LSPOS_FRAC_SHIFT);
	sstg_print_stuff(cmdops, cmdXops, rop, srcFormat, cfore, cback);

	sstg_drawline(sstg, x1,y1,x2,y2, cmdops);
	sstg_idle(sst);			// wait for the command to complete
	style = sstg_checkline(sstg, x1,y1,x2,y2, cmdops,cmdXops, cfore,cback,style, 0);

	if ( diago.checkEveryTriangle || !diago.writeFifo ) {
	  if ( iRandom(3) == 1 ) {        // randomly read back & compare style register
	    FxU32 s;
	    sst_idle_really(sst);
	    s = GET(sstg->lineStyle);
	    if ( s != style )
	      GDBG_ERROR("lstip","error reading lineStyle, expected 0x%x, got 0x%x\n",
			 style,s);
	  }
	}

	// clear out the entire area
	if (y2 < y1) {
	    temp = y1; y1 = y2; y2 = temp;
	}
	if (x2 < x1) {
	    temp = x1; x1 = x2; x2 = temp;
	}
	DIAG_FORCE_RECT(CSIM_BUF_2D_DST,
			x1,y1, x2-x1+1, y2-y1+1, 0);
    }
    DIAG_PASS(0);
}

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
** $Date: 10/11/00 8:11:33 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

static FxU32 screen[MAXSCREEN][MAXSCREEN];

void
main (int argc, char **argv)
{
    int j,n;
    long x,y, w,h, xc,yc;
    FxU32 rop, srcFormat, cmdops,cmdXops;
    FxU32 cfore,cback,cdest;
    SstRegs *sst;
    SstGRegs *sstg;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);
    sstg_init_random_screen(sstg,screen);

    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);

    // HACK: fetch the srcFormat from the actual CSIM
    srcFormat = CSIM_PRIVATE(diago.sstCSIM)->gui.srcFormat;

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<256; n++) {			// do all 256 rop combos
	if ((n % 7) == 0) {			// init pattern every 7 rops
	    sstg_setpattern_random(sstg);
	}
	xyRandom(&x,&y);
	if (iRandom(7)==0) {			// generate some negative coords
	    x -= 4*diago.tsize;
	    y -= 3*diago.tsize;
	}
	if (diago.clamp) {			// if random clipping
	    SET(sstg->clip0min, (iRandom(87)<<16) | iRandom(99));
	    SET(sstg->clip0max, ((diago.ymaxscreen-iRandom(25)-1)<<16) | (diago.xmaxscreen-iRandom(50)-1));
	    SET(sstg->clip1min, (iRandom(37)<<16) | iRandom(49));
	    SET(sstg->clip1max, ((diago.ymaxscreen-iRandom(75)-1)<<16) | (diago.xmaxscreen-iRandom(90)-1));
	}

	for (j=0; j<5; j++) {			// and do a number of colors
	    w = rRandom(1,diago.tsize);		// generate random width height
	    h = rRandom(1,diago.tsize);

	    sstg_random_command_bits(&cmdops,&cmdXops);
	    cfore = colRandom32();
	    cdest = ONSCREEN(x,y) ? screen[y][x] : 0;
	    rop = sstg_random_colors(sstg, cfore,&cback,cdest);
	    SET(sstg->colorFore,cfore);

	    if (diago.rectangular) {		// if random rops
		rop = iRandom(0xFFFFFF);	// then pick totally random
		cmdops |= iRandom(0xFF) << SSTG_ROP0_SHIFT;
	    }
	    else {				// else use n as the actual rop
		if (rop == 0) {			// if using Rop[0]
		    cmdops |= n << SSTG_ROP0_SHIFT;
		    if (n == 0) rop = 0xFFFFFF;	// if rop is ZERO set other rops to ONE
		}
		else {
		    rop = (rop-1) * 8;		// compute shift amount
		    if (n == 0)			// if rop is ZERO set other rops to ONE
			rop = ~(0xFF << rop);
		    else			// else use ZERO as other rops
			rop = n << rop;
		}
	    }
	    SET(sstg->rop, rop);		// set the rop
	    SET(sstg->commandEx, cmdXops);
	    sstgCheckForIdle(sst, cmdXops);

	    gdbg_info(2,"\n");
	    gdbg_info(2,"x,y = %d,%d    w,h = %d,%d    pox,y = %d,%d\n",
			x,y,w,h,
			((cmdops & SSTG_X_PATOFFSET)>>SSTG_X_PATOFFSET_SHIFT) & 7,
			((cmdops & SSTG_Y_PATOFFSET)>>SSTG_Y_PATOFFSET_SHIFT) & 7);
	    sstg_print_stuff(cmdops, cmdXops, rop, srcFormat, cfore, cback);

	    sstg_drawrect(sstg,x,y,w,h, cmdops);
	    sstg_idle(sst);			// wait for the command to complete

	    for (yc = y-1; yc <= y+h; yc++)	// check the entire rectangle
	    for (xc = x-1; xc <= x+w; xc++)	// with a 1 pixel border
	    if (ONSCREEN(xc,yc)) {
		cdest = screen[yc][xc];		// compute predicted result
		if (xc < x || xc >= x+w || yc < y || yc >= y+h) {
		    // if outside rect, then unchanged
		    cdest = sstg_destination_mask(cdest);
		    gdbg_info(8,"checking %d,%d 0x%x (outside)\n",xc,yc,cdest);
		    DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,xc,yc,cdest);// test the pixel
		}
		else {
		    cdest = sstg_check_pixel(xc,yc,cmdops,cmdXops, rop,cfore,cdest,0);
		    screen[yc][xc] = cdest;	// update our shadow screen
		}
	    }
	}
	if (DIAG_EXCEEDED_PIXEL_LIMIT())
	    break;
    }
    DIAG_PASS(0);
}

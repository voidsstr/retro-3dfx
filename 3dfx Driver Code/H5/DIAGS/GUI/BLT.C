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
** $Date: 10/11/00 8:11:03 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

static FxU32 screen[MAXSCREEN][MAXSCREEN];
static FxU32 next_screen[MAXSCREEN][MAXSCREEN];

static int maxHeight = -1;

char *
Xusage()
{
    gdbg_printf("\n");
    gdbg_printf("\"-xh #\"\toverrides -t with random height 0 <= h <= # (default use -t)\n");
    gdbg_printf("\n");
    exit(1);
    return(0);
}

/* myParseOpts
 *
 * look for "-x" options and interpret them for this test
 *
 */

#define XGETARG() opts[1] ? done = 1, ++opts : \
			  (--argc > 0) ? done = 1, *++argv : \
					 (char *)Xusage()

void
XParseOpts(int argc, char **argv)
{
    char *opts = 0;
    FxBool aopt = 0;
    FxU32 bopt = 0;
    FxBool done;
    
    while ((--argc > 0) && (**++argv))
    {
	if (argv[0][0] != '-')
	    continue;
	if (argv[0][1] != 'x')
	    continue;

	/* now parse all extended parameters */
	done = 0;
	opts = &argv[0][2];
	if (*opts == '\0')
	    Xusage();
	
	while (!done && *opts)
	{
	    switch (*opts)
	    {
	      case 'h':
		  sscanf(XGETARG(), "%i", &maxHeight);
		  break;

	      default:
		  Xusage();
	    }
	    
	    opts += 1;
	}
    }
}



// NOTE: source and destination are always the same format
// NOTE: strides are constant and are the width of the screen
void
main (int argc, char **argv)
{
    int j,n;
    long xs,ys, xd,yd, w,h, xc,yc, xdelta, ydelta;
    FxU32 rop, srcFormat, cmdops,cmdXops;
    FxU32 cfore,cback,cdest;
    SstRegs *sst;
    SstGRegs *sstg;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);

    XParseOpts(argc, argv);

    // HACK: fetch the srcFormat from the actual CSIM
    srcFormat = CSIM_PRIVATE(diago.sstCSIM)->gui.srcFormat;

    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);

    sstg_init_random_screen(sstg,screen);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<256; n++) {
	if ((n % 13) == 0) {			// init pattern once in a while
	    sstg_setpattern_random(sstg);
	}
	if (diago.clamp) {			// if random clipping
	    SET(sstg->clip0min, (iRandom(87)<<16) | iRandom(99));
	    SET(sstg->clip0max, ((diago.ymaxscreen-iRandom(25)-1)<<16) | (diago.xmaxscreen-iRandom(50)-1));
	    SET(sstg->clip1min, (iRandom(37)<<16) | iRandom(49));
	    SET(sstg->clip1max, ((diago.ymaxscreen-iRandom(75)-1)<<16) | (diago.xmaxscreen-iRandom(90)-1));
	}

	for (j=0; j<2; j++) {			// and do a number of tests
	    xyRandom(&xs,&ys);			// NOTE: source is on screen
	    do {				// generate random width height
		w = rRandom(1,diago.tsize);
		if (maxHeight < 0)
		    h = rRandom(1,diago.tsize);
		else
		    h = iRandom(maxHeight);
	    } while (!ONSCREEN(xs+w-1, ys+h-1));// until it's entirely onscreen

	    if (iRandom(1))			// generate random destination
		xyRandom(&xd,&yd);		// anywhere
	    else {				// overlapping source sometimes
		xd = xs + rRandom(-w,w);
		yd = ys + rRandom(-h,h);
	    }
	    xdelta = xs - xd;			// difference between corners
	    ydelta = ys - yd;

	    sstg_random_command_bits(&cmdops,&cmdXops);

	    // don't allow right to left blits with
	    // color keying.  since we may need the direction flag for
	    // overlapping blits, just turn off colorkeying
	    if (cmdops & SSTG_XDIR)
	    {
		cmdXops &= ~(SSTG_EN_SRC_COLORKEY_EX |
			     SSTG_EN_DST_COLORKEY_EX);
	    }

	    cfore = screen[ys][xs];
	    cdest = ONSCREEN(xd,yd) ? screen[yd][xd] : 0;
	    rop = sstg_random_colors(sstg, cfore,&cback,cdest);
	    if (diago.rectangular) {		// if random rops
		cmdops |= iRandom(0xFF) << SSTG_ROP0_SHIFT;
		rop = iRandom(0xFFFFFF);	// then pick totally random
	    }
	    else if (rop == 0) 			// else use SRC
		cmdops |= SSTG_ROP_SRC << SSTG_ROP0_SHIFT;
	    else
		rop = SSTG_ROP_SRC << (rop-1)*8;
	    SET(sstg->rop, rop);		// set the rop	    
	    SET(sstg->commandEx, cmdXops);
	    sstgCheckForIdle(sst, cmdXops);

	    gdbg_info(2,"\n");
	    gdbg_info(2,"xs,ys = %d,%d    xd,yd = %d,%d    w,h = %d,%d    pox,y = %d,%d\n",
			xs,ys, xd,yd, w,h,
			((cmdops & SSTG_X_PATOFFSET)>>SSTG_X_PATOFFSET_SHIFT) & 7,
			((cmdops & SSTG_Y_PATOFFSET)>>SSTG_Y_PATOFFSET_SHIFT) & 7);
	    sstg_print_stuff(cmdops, cmdXops, rop, srcFormat, cfore, cback);

	    sstg_drawblt(sstg,xs,ys, xd,yd,w,h, cmdops, &cmdXops);
	    sstg_idle(sst);			// wait for the command to complete

	    for (yc = yd-1; yc <= yd+h; yc++)	// check the entire rectangle
	    for (xc = xd-1; xc <= xd+w; xc++)	// with a 1 pixel border
	    if (ONSCREEN(xc,yc)) {
		cdest = screen[yc][xc];		// compute predicted result
		if (xc < xd || xc >= xd+w || yc < yd || yc >= yd+h) {
		    // if outside rect, then unchanged
		    cdest = sstg_destination_mask(cdest);
		    gdbg_info(8,"checking %d,%d 0x%x (outside)\n",xc,yc,cdest);
		    DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,xc,yc,cdest);// test the pixel
		}
		else {
		    FxU32 csrc = screen[yc+ydelta][xc+xdelta];
		    int srcKey = sstg_src_colorkey(csrc);
		    GDBG_INFO(8,"converting color 0x%x\n",csrc);
		    // NOTE: call into csim to convert colors
		    csrc = csimColorConvert(&CSIM_PRIVATE(diago.sstCSIM)->gui, csrc);

		    cdest = sstg_check_pixel(xc,yc,cmdops,cmdXops, rop,csrc,cdest,srcKey);
		    next_screen[yc][xc] = cdest;	// update our shadow screen
		}
	    }

	    for (yc = yd; yc < yd+h; yc++)	// now copy the results
	    for (xc = xd; xc < xd+w; xc++)	// to the shadow screen
	    if (ONSCREEN(xc,yc)) {
		screen[yc][xc] = next_screen[yc][xc];
	    }

	    // now verify that dstXY got updated properly
	    if (diago.updatexy && diago.checkEveryTriangle) {
		FxU32 new, expected;
		if (xs < xd) xd += w-1;	// compute actual dstXY register
		if (ys < yd) yd += h-1;	// that we sent to the chip

		expected = (cmdops & SSTG_UPDATE_DSTX ? xd+w : xd) & 0x1FFF;
		expected += ((cmdops & SSTG_UPDATE_DSTY ? yd+h : yd)& 0x1FFF)<<16;
		new = GET(sstg->dstXY);
		DIAG_TESTREG32("dstXY",expected,new);
	    }
	}
	if (DIAG_EXCEEDED_PIXEL_LIMIT())
	    break;
    }
    DIAG_PASS(0);
}

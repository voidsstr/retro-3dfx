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
** $Date: 10/11/00 8:11:34 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "h3gdefs.h"

#define CEIL(x,y)      ( ((x)+(y)-1) / (y) )  // divide x/y, rounding up
#define MAX_RAND_TRIES 1000   // Max times we'll try to generate a random param

static FxU8 *screen;
static FxU8 *next_screen;

static FxU32 src_pixels[MAXSCREEN*MAXSCREEN];


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



//
// given a destination pixel (0 is the leftmost/topmost) and a source and
// destination size, calculate and return the corresponding source coord
//
FxI32 sstg_compute_stretch_src(FxI32 dstCoord, FxU32 srcSize, FxU32 destSize)
{
    double srcCoord;

    srcCoord = (double)srcSize * (2.0 * dstCoord + 1.0) / (2.0 * destSize);
    return (FxI32)(ceil(srcCoord +
			((destSize > srcSize) ? 0.000001 : 0.0))) - 1;
}
		   

// NOTE: source is a random format and stride is random
void
main (int argc, char **argv)
{
    int j,n,p, monotrans;
    long xs,ys, sw,sh, xd,yd, dw,dh, xc,yc, w, h;
    FxU32 rop, srcBase, srcFormat, dstFormat, destBA, cmdops,cmdXops;
    FxU32 cfore,cback,cdest;
    int dst_is_tiled = 0;
    int src_is_tiled = 0;
    SstRegs *sst;
    SstGRegs *sstg;
    int pack;
    int end_x, end_y;
    int rand_tries;	// infinite loop sanity checker

//#define STRETCHTEST
#ifdef STRETCHTEST
    int i;
    FxU32 dstWidth = 5, srcWidth = 3;
    FxI32 bresError = 0;
    int doError = 0;

    if (argc > 1)
    {
	srcWidth = atoi(argv[1]);
	--argc, ++argv;
    }
	
    if (argc > 1)
    {
	dstWidth = atoi(argv[1]);
	--argc, ++argv;
    }

    if (argc > 1)
    {
	bresError = atoi(argv[1]);
	doError = 1;
	--argc, ++argv;
	printf("bresError is %d\n", bresError);
    }

    for (i = 0; i < dstWidth; i++)
	printf("dstX %3i gets srcX %3i\n", i,
	       sstg_compute_stretch_src(i, srcWidth, dstWidth));
#endif

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);
    screen = sstg_alloc_shadow_memory();
    next_screen = sstg_alloc_shadow_memory();

    XParseOpts(argc, argv);

    if ( diago.ytiled == 1 || diago.ytiled == 2 ) dst_is_tiled = 1;
    if ( diago.ytiled == 1 || diago.ytiled == 3 ) src_is_tiled = 1;

    sst_idle_really(sst);
    dstFormat = GET(sstg->dstFormat);

    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);
#ifndef STRETCHTEST
    sstg_init_random_memory(screen);
#endif
    
    destBA = GET(sstg->dstBaseAddr);

#ifdef STRETCHTEST
    SET(sstg->srcXY, (10 << 16) | 10);
    SET(sstg->dstXY, (0  << 16) |  0);
    SET(sstg->dstSize, (1 << 16) | dstWidth);
    SET(sstg->srcSize, (1 << 16) | srcWidth);
    SET(sstg->bresError0, ((doError ? 1 : 0) << 31) | bresError);
    SET(sstg->command, SSTG_GO | (SSTG_ROP_SRC << SSTG_ROP0_SHIFT) |
	SSTG_STRETCH_BLT);

    exit(0);
#endif 

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<256; n++) {
	if ((n % 15) == 0) {			// init pattern once in a while
	    sstg_setpattern_random(sstg);
	}
	if (diago.clamp) {			// if random clipping
	    SET(sstg->clip0min, (iRandom(87)<<16) | iRandom(99));
	    SET(sstg->clip0max, ((diago.ymaxscreen-iRandom(25)-1)<<16) | (diago.xmaxscreen-iRandom(50)-1));
	    SET(sstg->clip1min, (iRandom(37)<<16) | iRandom(49));
	    SET(sstg->clip1max, ((diago.ymaxscreen-iRandom(75)-1)<<16) |
		(diago.xmaxscreen-iRandom(90)-1));
	}

	for (j=0; j<3; j++) {			// and do a number of tests
	    // now pick a random source format (valid with destination)
	    // 8bpp dest requires 1 or 8 bpp source
	    // 16,24,32 bpp dest require anything but 8bpp
#define SRCFORMAT (srcFormat & SSTG_SRC_FORMAT)
	    diago.dstFormat <<= SSTG_DST_FORMAT_SHIFT;
	    srcFormat = sstg_random_srcFormat(diago.dstFormat, src_is_tiled);
	    diago.dstFormat >>= SSTG_DST_FORMAT_SHIFT;	    

	    // we don't want the source region to overlap the dest region
	    // at all, because the source region will have random-ish
	    // strides, making the overlap test very, very gross (and in
	    // some cases, it's impossible to choose a blit direction that
	    // avoids bad overlap).
	    //
	    srcBase = sstg_random_srcBaseAddr(sstg, srcFormat,
				    	dstFormat, destBA,
				    	0 /* no overlap with dest */);
	    
	    SET(sstg->srcFormat, srcFormat);

	    // choose a source x, y, and a width and height
	    //
	    sstg_random_src_rect(CSIM_BUF_2D_STRETCH_SRC, 
				 srcFormat, srcBase, &xs, &ys, &sw, &sh);
	    
	    // This error should never happen ...
	    if ((xs > (SST_MASK(SSTG_XY_SIZE - 1))) ||
		(ys > (SST_MASK(SSTG_XY_SIZE - 1))) ||
		(sw > (SST_MASK(SSTG_XY_SIZE - 1))) ||
		(sh > (SST_MASK(SSTG_XY_SIZE - 1)))) {
	      gdbg_info(2,"Illegal src x,y,w, or h = %d,%d,%d,%d\n",xs,ys,sw,sh);
	      gdbg_error("sblt","Diag generated illegal src rectangle\n");
	      DIAG_FAIL();
	    }

	    pack = (srcFormat & SSTG_SRC_PACK) >> SSTG_SRC_PACK_SHIFT;
	    if (((xs != 0) || (ys != 0)) && (pack > 0))
	    {
		xs = 0;
		ys = 0;
	    }

	    xyRandom(&xd,&yd);			// generate random destination

	    sstg_random_command_bits(&cmdops,&cmdXops);

	    if (SRCFORMAT == SSTG_PIXFMT_1BPP)
		cmdXops &= ~SSTG_EN_SRC_COLORKEY_EX;	// disable src colorkey
	    {
		// WARNING: this depends on csim regs which might still be in CMD FIFO
		// so we save/backdoor/restore csim regs around csimReadPixel2d
		SstGRegs *hack_sstg = &CSIM_PRIVATE(diago.sstCSIM)->gui;
		FxU32 hack1 = hack_sstg->srcFormat;
		FxU32 hack2 = hack_sstg->srcBaseAddr;
		hack1 = hack_sstg->srcFormat;		// save old regs
		hack2 = hack_sstg->srcBaseAddr;
		hack_sstg->srcFormat = srcFormat;	// backdoor it
		hack_sstg->srcBaseAddr = srcBase;
		cfore = csimReadPixel(diago.sstCSIM, CSIM_BUF_2D_STRETCH_SRC, xs,ys);
		hack_sstg->srcFormat = hack1;		// restore
		hack_sstg->srcBaseAddr = hack2;
	    }

	    SET(sstg->colorFore, cfore);

	    rop = sstg_random_colors(sstg, cfore,&cback, sstg_destination_mask(iRandom(~0)));

	    if (diago.rectangular) {		// if random rops
		cmdops |= iRandom(0xFF) << SSTG_ROP0_SHIFT;
		rop = iRandom(0xFFFFFF);	// then pick totally random
	    }
	    else if (rop == 0) 			// else use SRC
		cmdops |= SSTG_ROP_SRC << SSTG_ROP0_SHIFT;
	    else
		rop = SSTG_ROP_SRC << (rop-1)*8;
	    
	    rand_tries = 0;
	    do {
	      if (++rand_tries > MAX_RAND_TRIES) {
		gdbg_error("sblt","too many tries for rand dst blt width\n");
		DIAG_FAIL();
	      }
	      if (iRandom(1)) {
		dw = rRandom(sw, diago.tsize);	// stretch X
	      }
	      else {
		dw = iRandom(sw);		// decimate X
	      }
	      end_x = xd + dw;
	    } while ((end_x > 4095) || (end_x < -4096));

	    rand_tries = 0;
	    do {
	      if (++rand_tries > MAX_RAND_TRIES) {
		gdbg_error("sblt","too many tries for rand dst blt height\n");
		DIAG_FAIL();
	      }
	      if (maxHeight >= 0) {
		dh = iRandom(maxHeight);
	      }
	      else {
		if (iRandom(1)) {	    
		  dh = rRandom(sh, diago.tsize);// stretch Y
		}
		else {
		  dh = iRandom(sh);		// decimate Y
		}
	      }
	      end_y = yd + dh;
	    } while ((end_y > 4095) || (end_y < -4096));
	    
	    // This error should never happen ...
	    if ((end_x > 4095) || (end_x < -4096) ||
		(end_y > 4095) || (end_y < -4096)) {
	      gdbg_info(2,"Illegal dest endpoint = %d,%d\n",end_x,end_y);
	      gdbg_error("sblt","Diag generated illegal dest endpt\n");
	      DIAG_FAIL();
	    }

	    gdbg_info(2,"\n");
	    gdbg_info(2,"xs,ys = %d,%d  sw,sh = %d,%d  xd,yd = %d,%d  dw,dh = %d,%d",
		      xs,ys, sw,sh, xd,yd, dw,dh);
	    gdbg_info_more(2, "   pox,y = %d,%d\n",
			((cmdops & SSTG_X_PATOFFSET)
			  >> SSTG_X_PATOFFSET_SHIFT) & 7,
			((cmdops & SSTG_Y_PATOFFSET)
			  >>SSTG_Y_PATOFFSET_SHIFT) & 7);
	    sstg_print_stuff(cmdops, cmdXops, rop, srcFormat, cfore, cback);

	    //--------------------------------------------
	    SET(sstg->rop, rop);		// set the rop
	    SET(sstg->commandEx, cmdXops);
	    sstgCheckForIdle(sst, cmdXops);


	    cmdops &= ~(SSTG_XDIR | SSTG_YDIR);	// clear out the direction bits

	    sstg_draw_S_blt(sstg,xs,ys,sw,sh,  xd,yd,dw,dh, cmdops);
	    sstg_idle(sst);		// wait for the command to complete
	    monotrans = (cmdops & SSTG_TRANSPARENT) &&
			((srcFormat & SSTG_SRC_FORMAT) == SSTG_PIXFMT_1BPP);

	    w = dw;
	    h = dh;

#if 0
	    // now read the pixels from the source, we have to wait until
	    // after idle (for command fifo to flush out) and set registers
	    // that csimReadPixel2d uses
	    p = 0;
	    for (yc = yd; yc < yd+h; yc++)	
	    for (xc = xd; xc < xd+w; xc++)	// and store in an array
	    if (ONSCREEN(xc,yc)) {
		src_pixels[p++] =
		    csimReadPixel2d(&CSIM_PRIVATE(diago.sstCSIM)->gui,
				     CSIM_BUF_2D_SRC,
				     (xc - xd) + xs,
				     (yc - yd) + ys);

		gdbg_info(7,"src_pixels[%d] = 0x%x\n",p-1,src_pixels[p-1]);
	    }
#endif
	    //--------------------------------------------
	    if ( diago.checkEveryTriangle ) {
            p = 0;
	    for (yc = yd-1; yc <= yd+h; yc++)	// check the entire rectangle
	    for (xc = xd-1; xc <= xd+w; xc++)	// with a 1 pixel border
	    if (ONSCREEN(xc,yc))
	    {
		// compute predicted result
		cdest = sstg_get_screen_color(xc, yc, screen);

		if (xc < xd || xc >= xd+w || yc < yd || yc >= yd+h)
		{
		    // if outside rect, then unchanged
		    cdest = sstg_destination_mask(cdest);
		    gdbg_info(8,"checking %d,%d 0x%x (outside)\n",xc,yc,cdest);
		    sstg_test_screen_pixel(xc, yc, cdest);
		}
		else
		{
		    FxI32 srcx = sstg_compute_stretch_src(xc - xd, sw, dw);
		    FxI32 srcy = sstg_compute_stretch_src(yc - yd, sh, dh);
		    FxU32 csrc = 
			csimReadPixel(diago.sstCSIM,
				      CSIM_BUF_2D_STRETCH_SRC,
				      srcx + xs , srcy + ys);
		    
		    int srcKey = sstg_src_colorkey(csrc);

		    if (monotrans && !(csrc&1))
		    {
			gdbg_info(8,"checking %d,%d 0x%x (monotrans)\n",
				  xc,yc,cdest);
			sstg_test_screen_pixel(xc, yc, cdest);
		    }
		    else
		    {
			GDBG_INFO(8,"converting color 0x%x\n",csrc);
			// NOTE: call into csim to convert colors
			csrc = csimColorConvert(
			    &CSIM_PRIVATE(diago.sstCSIM)->gui, csrc);
			cdest = sstg_check_pixel(xc,yc,cmdops,cmdXops, rop,
						 csrc,cdest,srcKey);
		    }
		    sstg_set_screen_color(xc, yc, next_screen, cdest);
		}
	    }

	    //--------------------------------------------
	    for (yc = yd; yc < yd+h; yc++)	// now copy the results
	    for (xc = xd; xc < xd+w; xc++)	// to the shadow screen
	    if (ONSCREEN(xc,yc))
	    {
		sstg_set_screen_color(xc, yc, screen,
				      sstg_get_screen_color(xc, yc,
						    next_screen));
	    }
	    }

	    // now verify that dstXY got updated properly
	    if (diago.updatexy && diago.checkEveryTriangle)
	    {
		FxU32 new, expected;

		if ((dw * dh) != 0)
		{
		    if (cmdops & SSTG_XDIR)
			xd += dw-1;	// compute actual dstXY register
		    if (cmdops & SSTG_YDIR)
			yd += dh-1;	// that we sent to the chip
		}
		

		expected = (cmdops & SSTG_UPDATE_DSTX ? xd+dw : xd) & 0x1FFF;
		expected += ((cmdops & SSTG_UPDATE_DSTY ? yd+dh : yd)& 0x1FFF)<<16;
		new = GET(sstg->dstXY);
		DIAG_TESTREG32("dstXY",expected,new);
	    }
	}
	if (DIAG_EXCEEDED_PIXEL_LIMIT())
	    break;
    }

    if (getenv("DIAG_WAIT"))
	getchar();

    DIAG_PASS(0);
}



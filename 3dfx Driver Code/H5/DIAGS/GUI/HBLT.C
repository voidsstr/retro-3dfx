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
** $Date: 10/11/00 8:11:18 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

FxU32 srcFormat;

static FxU32 screen[MAXSCREEN][MAXSCREEN];
static FxU32 host_pixels[MAXSCREEN*MAXSCREEN];


//----------------------------------------------------------------------
// draw a host-to-scr blt, 
//----------------------------------------------------------------------
void sstg_drawHblt(SstGRegs *sstg, int xs, int ys, int xd, int yd, int w, int h, FxU32 cmd)
{
    FxU32 addr, bit, byte;
    FxU32 ydir;
    
    SET(sstg->command, cmd | SSTG_HOST_BLT);

    // old: SET(sstg->srcXY,(ys<<16) | (xs & 0xFFFF));
    // new: convert xs, ys to byte/bit position
    //
    addr = sstg_compute_blit_address(srcFormat, 0, xs, ys, w);
    if ((srcFormat & SSTG_SRC_FORMAT) == SSTG_PIXFMT_1BPP)
    {
	byte = addr % 4;
	bit = (byte * 8) + (xs % 8);

	gdbg_info(7, "hblt.exe: starting bit position: %d\n", bit);
	// should ignore everything above bit 31
	SET(sstg->srcXY, (iRandom(0xFFFFFFFF) & ~31) | bit);
    }
    else
    {
	// color format -- just send the byte address
	byte = addr & 3;
	gdbg_info(7, "hblt.exe: starting byte position: %d\n", byte);
	// should ignore everything above bit 2
	SET(sstg->srcXY, (iRandom(0xFFFFFFFF) & ~3) | byte);
    }
    

    SET(sstg->dstXY,(yd<<16) | (xd & 0xFFFF));
    SET(sstg->dstSize,(h<<16) | w);

    ydir = cmd & SSTG_YDIR;

    switch(srcFormat & SSTG_SRC_PACK) {
	case SSTG_SRC_PACK_SRC:
		sendBltData0(sstg, byte, bit, xd, yd, w,h, ydir, srcFormat,
			     host_pixels);
		break;
	case SSTG_SRC_PACK_8:
		sendBltDataPacked(sstg, 8, byte, xd,yd, w,h, ydir, srcFormat,
				  host_pixels);
		break;
	case SSTG_SRC_PACK_16:
		sendBltDataPacked(sstg, 16, byte, xd,yd, w,h, ydir, srcFormat,
				  host_pixels);
		break;
	case SSTG_SRC_PACK_32:
		sendBltDataPacked(sstg, 32, byte, xd,yd, w,h, ydir, srcFormat,
				  host_pixels);
		break;
    }
}



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


void
main (int argc, char **argv)
{
    int j,n, monotrans;
    long xs,ys, xd,yd, w,h, xc,yc, xdelta, ydelta;
    FxU32 rop, cmdops,cmdXops;
    FxU32 cfore,cback,cdest;
    SstRegs *sst;
    SstGRegs *sstg;
    FxI32 ycStart, ycEnd, ycInc;
#if defined H3_A0 || defined H3_A1 || defined H3_A2
    FxU32 clip0min, clip0max, clip1min, clip1max;
#endif


    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);

    XParseOpts(argc, argv);

    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);
#if defined H3_A0 || defined H3_A1 || defined H3_A2
    clip0min = 0x00000000;
    clip0max = (diago.ymaxscreen<<16) | diago.xmaxscreen;
#endif

    sstg_init_random_screen(sstg,screen);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<256; n++) {
	if ((n % 7) == 0) {			// init pattern every 7 times
	    sstg_setpattern_random(sstg);
	}
	if (diago.clamp) {			// if random clipping
#if defined H3_A0 || defined H3_A1 || defined H3_A2
	    clip0min = (iRandom(87)<<16) | iRandom(99);
	    clip0max = ((diago.ymaxscreen-iRandom(25)-1)<<16) | (diago.xmaxscreen-iRandom(50)-1);
	    clip1min = (iRandom(37)<<16) | iRandom(49);
	    clip1max = ((diago.ymaxscreen-iRandom(75)-1)<<16) | (diago.xmaxscreen-iRandom(90)-1);

	    SET(sstg->clip0min, clip0min);
	    SET(sstg->clip0max, clip0max);
	    SET(sstg->clip1min, clip1min);
	    SET(sstg->clip1max, clip1max);
#else
	    SET(sstg->clip0min, (iRandom(87)<<16) | iRandom(99));
	    SET(sstg->clip0max, ((diago.ymaxscreen-iRandom(25)-1)<<16) | (diago.xmaxscreen-iRandom(50)-1));
	    SET(sstg->clip1min, (iRandom(37)<<16) | iRandom(49));
	    SET(sstg->clip1max, ((diago.ymaxscreen-iRandom(75)-1)<<16) | (diago.xmaxscreen-iRandom(90)-1));
#endif
	}

	for (j=0; j<2; j++) {			// and do a number of tests
	    int pix = 0;

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

	    // now pick a random source format (valid with destination)
	    // 8bpp dest requires 1 or 8 bpp source
	    // 16,24,32 bpp dest require anything but 8bpp
	    // host blit never has tiled source, so pass in 0
	    diago.dstFormat <<= SSTG_DST_FORMAT_SHIFT;
	    srcFormat = sstg_random_srcFormat(diago.dstFormat, 0);
	    diago.dstFormat >>= SSTG_DST_FORMAT_SHIFT;
	    
	    // also pick random byte/word swizzle
	    if (diago.bilinear) {		// -b (byte/word swapping)
		if (iRandom(1)) srcFormat |= SSTG_HOST_BYTE_SWIZZLE;
		if (iRandom(1)) srcFormat |= SSTG_HOST_WORD_SWIZZLE;
	    }
	    SET(sstg->srcFormat, srcFormat);

#define SRCFORMAT (srcFormat & SSTG_SRC_FORMAT)
	    sstg_random_command_bits(&cmdops,&cmdXops);
	    if (SRCFORMAT == SSTG_PIXFMT_1BPP)
		cmdXops &= ~SSTG_EN_SRC_COLORKEY_EX;	// disable source colorkey
	    cfore = iRandom(0xFFFFFFFF);
	    SET(sstg->colorFore, cfore);
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

#if defined H3_A0 || defined H3_A1 || defined H3_A2
#define ROP_USES_DST(x)     (((x)^((x)<<1)) & 0xFE)     /* test whether rop uses the dst data */
	    // force 2d pipe to idle with a 3D NOP if --
	    // 1. rop uses DST (either dst color keying or rop accesses dst surface)
	    // 2. hblt starting dst posn is outside y clip region
	    {
	      static int firstTime = 1;
	      FxU32 dst_colorkey = cmdXops & SSTG_EN_DST_COLORKEY_EX;
	      FxU32 src_colorkey = cmdXops & SSTG_EN_SRC_COLORKEY_EX;
	      FxU8 rop0 = (FxU8) ((cmdops&SSTG_ROP0)>>SSTG_ROP0_SHIFT);
	      FxU8 rop2 = (FxU8) ((rop>>8)&0xFF);
	      
	      if ( dst_colorkey || ROP_USES_DST(rop0) || (src_colorkey && ROP_USES_DST(rop2)) ) {

		FxU32 clipselect = cmdops & SSTG_CLIPSELECT;
		if ( ( clipselect==0 && (yd<(signed)HIWORD(clip0min) || yd>(signed)HIWORD(clip0max)) ) || 
		     ( clipselect!=0 && (yd<(signed)HIWORD(clip1min) || yd>(signed)HIWORD(clip1max)) ) ) {
		
		  if ( firstTime ) {
		    GDBG_INFO(0,"HACK -- sending 3D NOP before Host Blt\n");
		    firstTime = 0;
		  }
		  SET(sst->nopCMD,0);

		}
	      }
	    }
#endif
	    sstg_drawHblt(sstg,xs,ys, xd,yd,w,h, cmdops);
	    sstg_idle(sst);			// wait for the command to complete
	    monotrans = (cmdops & SSTG_TRANSPARENT) &&
			((srcFormat & SSTG_SRC_FORMAT) == SSTG_PIXFMT_1BPP);

	    if (cmdops & SSTG_YDIR)
	    {
		// negative y direction
		ycStart = yd + 1;
		ycEnd = yd - h;
		ycInc = -1;		
	    }
	    else
	    {
		// positive y direction
		ycStart = yd - 1;
		ycEnd = yd + h;
		ycInc = 1;
	    }
	    
	    // check the entire rectangle
	    for (yc = ycStart;
		 ycInc > 0 ? yc <= ycEnd : yc >= ycEnd;
		 yc += ycInc)
	    for (xc = xd-1; xc <= xd+w; xc++)	// with a 1 pixel border
	    if (ONSCREEN(xc,yc)) {
		cdest = screen[yc][xc];		// compute predicted result
		if (xc < xd || xc >= xd+w ||
		    ((ycInc == 1) && ((yc < yd) || (yc >= yd+h))) ||
		    ((ycInc == -1) && ((yc > yd) || (yc <= yd-h))))
		{
		    // if outside rect, then unchanged
		    cdest = sstg_destination_mask(cdest);
		    gdbg_info(8,"checking %d,%d 0x%x (outside)\n",xc,yc,cdest);
		    DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,xc,yc,cdest); // test the pixel
		}
		else {
		    FxU32 csrc = host_pixels[pix++];
		    int srcKey = sstg_src_colorkey(csrc);
		    
		    if (monotrans && !(csrc&1)) {
			gdbg_info(8,"checking %d,%d 0x%x (monotrans)\n",xc,yc,cdest);
			DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,xc,yc,cdest); // test the pixel
			continue;
		    }
		    GDBG_INFO(8,"converting color 0x%x\n",csrc);
		    // NOTE: call into csim to convert colors
		    csrc = csimColorConvert(&CSIM_PRIVATE(diago.sstCSIM)->gui, csrc);

		    cdest = sstg_check_pixel(xc,yc,cmdops,cmdXops, rop,csrc,cdest,srcKey);
		    screen[yc][xc] = cdest;	// update our shadow screen
		}
	    }

	    // now verify that dstXY got updated properly
	    if (diago.updatexy && diago.checkEveryTriangle) {
		FxU32 new, expected;

		if ( (cmdops & SSTG_GO) == 0  &&  (w*h) == 0 ) {
		  GDBG_INFO(0,"skipping update\n");
		  expected = (xd & 0x1FFF) + ((yd & 0x1FFF)<<16);
		} else {
		  expected = (cmdops & SSTG_UPDATE_DSTX ? xd+w : xd) & 0x1FFF;
		  expected += ((cmdops & SSTG_UPDATE_DSTY ? yd+h : yd)& 0x1FFF)<<16;
		}
		new = GET(sstg->dstXY);
		DIAG_TESTREG32("dstXY",expected,new);
	    }
	}
	if (DIAG_EXCEEDED_PIXEL_LIMIT())
	    break;
    }
    DIAG_PASS(0);
}

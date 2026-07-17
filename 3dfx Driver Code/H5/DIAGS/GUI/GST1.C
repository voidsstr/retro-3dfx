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
** $Date: 10/11/00 8:11:14 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#ifdef SST2
#include "sst2asm.h"
#else
#include "h3asm.h"
#endif

enum GSTMODES { TEST_RECTS, TEST_LINES, TEST_BLITS, TEST_HBLITS,
		TEST_BASE, TEST_STRIDES, TEST_3D };

void doTriangle(SstRegs *sst, Triangle *t, int onscreen)
{
    static int once=1;
    randomStressTriangle(t, diago.tsize, onscreen, once);
    once = -1;
    randomRgbaTriangle(t);		// with random colors
    setupTriangle(t,1,0,0);		// setup RGB slopes
    sortTriangle(t);			// sort it
    printTriangle(2,t,1,0,0);
    printTriangleSlopes(3,t,1,0,0);
    drawTriangle(sst,t,1,0,0);
}

void sendBlit(SstGRegs *sstg, FxU32 cmd, FxU32 srcFormat, int w, int h)
{
    int dwords;

    if ((cmd & SSTG_COMMAND) != SSTG_HOST_BLT) return;

    if (w < 0) w = -w;
    if (h < 0) h = -h;
    w += 1;	// adjust these to real values
    h += 1;

    switch(srcFormat & SSTG_SRC_FORMAT) {
	case SSTG_PIXFMT_1BPP:
		dwords = (w+31)/32 * h;
		break;
	case SSTG_PIXFMT_8BPP:
		dwords = (h+3)/4;
		break;
	case SSTG_PIXFMT_16BPP:
		dwords = (w+1)/2 * h;
		break;
	case SSTG_PIXFMT_24BPP:
	case SSTG_PIXFMT_32BPP:
		dwords = w * h;
		break;
	case SSTG_PIXFMT_422YUV:
	case SSTG_PIXFMT_422UYV:
	default: GDBG_ERROR("sendBlit", "invalid source pixel format\n");
    }
#if XXX
    gdbg_info(7,"sending %d dwords for host blt\n", dwords);
    while (dwords-- > 0) {
	SET(sstg->bltData, iRandom(0xFFFFFFFF));
    }
#endif
}

FxU32 cmd;				// current bltCommand
FxU32 srcFormat;
FxI32 srcBase, dstBase, bytesPerScreen;
SstRegs *sstg;

#if XXX
// returns 0 if OK, 1 if not
int checkSrcBaseAddr(void)
{
    int k = (cmd & SSTG_DST_IS_TILED) ? 4096 : 1;

    return srcBase <= dstBase*k + bytesPerScreen+4096*3 + (diago.tsize+4)*diago.xmaxscreen*2;
}

// returns 0 if OK, 1 if not
int checkDstBaseAddr(void)
{
    if (cmd & SSTG_DST_IS_TILED) {
	if (dstBase == 0) return 1;
	if (dstBase > bytesPerScreen/4096) return 1;
	return srcBase <= dstBase*4096 + bytesPerScreen + (diago.tsize+4)*diago.xmaxscreen*2;
    }
    else {
	if (dstBase & 0x7) return 1;
	return srcBase <= dstBase + bytesPerScreen + (diago.tsize+4)*diago.xmaxscreen*2;
    }
}

void doSrcBaseAddr(void)
{
again1:
    do {
	srcBase = iRandom(bytesPerScreen*3) & ~7;
    } while (checkSrcBaseAddr());
    if (sstTrashMem((FxU32)srcBase,(FxU32)bytesPerScreen))
	goto again1;
    SET(sstg->bltSrcBaseAddr,srcBase);
}

void doDstBaseAddr(void)
{
    if (cmd & SSTG_DST_IS_TILED) {
	do {
	    dstBase = 1+iRandom(bytesPerScreen/4096 - 1);
	} while (checkDstBaseAddr());
    }
    else {
	do {
	    dstBase = iRandom(bytesPerScreen) & ~7;
	} while (checkDstBaseAddr());
    }
    SET(sstg->bltDstBaseAddr,dstBase);
}
#endif

void
main (int argc, char **argv)
{
    int cx,cy;				// center of screen
    int l,r,b,t;			// bounds of drawing region
    int w,h;				// current w,h in bltSize
    int xs,ys;
    int xd,yd;
    static Triangle tri;

    sstg = SST_BEGIN2d(argc,argv);
    if (!diago.diff) {
	gdbg_error("gst1","must run with -D option, forcing -D\n");
	diago.diff = 1;
    }

    // Handle Options
    if ( diago.printOpts )
    {
	gdbg_printf( "gst1 option description (they are cumulative):\n"  );
	gdbg_printf( " %d -> Test rects ( default )\n", TEST_RECTS );
	gdbg_printf( " %d -> Test lines\n", TEST_LINES );
	gdbg_printf( " %d -> Test blits\n", TEST_BLITS );
	gdbg_printf( " %d -> Test host blits\n", TEST_HBLITS );
	gdbg_printf( " %d -> Test base addr\n", TEST_BASE );
	gdbg_printf( " %d -> Test strides\n", TEST_STRIDES );
	gdbg_printf( " %d -> Test 3D commands\n", TEST_3D );
	exit( 0 );
    }

    bytesPerScreen = diagfb.colBufferAddr[1];
    sstg_init_random_memory(NULL);

    cx = diago.xmaxscreen/2;
    cy = diago.ymaxscreen/2;
    t = 5 * diago.tsize;
    if (t > diago.ymaxscreen/8) t = diago.ymaxscreen/8;
    l = cx - t;
    r = cx + t;
    b = cy - t;
    t = cy + t;
    w = h = 0;

    SET(sstg->fbzColorPath, SST_RGBSEL_RGBA | (diago.adjust?SST_PARMADJUST:0));
    SET(sstg->fbzMode, SST_RGBWRMASK | drawbufferRandom());

#if XXX
    // setup some reasonable starting modes
    SET(sstg->bltSrcXY,(cy<<16) | cx);
    SET(sstg->bltDstXY,(cy<<16) | cx);
    SET(sstg->bltSize, 0);
    SET(sstg->bltColor,0xdeadbeef);
    SET(sstg->bltRop, 0);
    SET(sstg->bltCommand, SSTG_RECTFILL);
    SET(sstg->bltClipX, diago.xmaxscreen);
    SET(sstg->bltClipY, diago.ymaxscreen);
    sst_idle_really(sstg);			// idle before read
    dstBase = GET(sstg->bltDstBaseAddr);
    if (diago.option >= TEST_BASE) {
	srcBase = bytesPerScreen*3/2;
	srcBase &= ~7;
	SET(sstg->bltSrcBaseAddr,srcBase);
    }
#endif
    while (DIAG_STARTPASS()) {			// for each pass
	int j,go;

	// first check the screens every pass to make sure we are OK
	DIAG_DIFFMEMORY();

	// write a lot of registers
	for (j=0; j<1345; j++) {
	    int k = rRandom(CLIP0MIN/4,COMMAND/4);
	    if (diago.option >= TEST_3D) {
		if (iRandom(50)==0)
		    SET(sstg->fbzMode, SST_RGBWRMASK | drawbufferRandom());
		if (iRandom(50)==0)
		    doTriangle(sstg,&tri, 1);
	    }
#if XXX
	    switch(k*4) {
		case BLTSRCBASEADDR:
			if (diago.option >= TEST_BASE) {
			    doSrcBaseAddr();
			}
			break;

		case BLTDSTBASEADDR:
			if (diago.option >= TEST_BASE) {
			    doDstBaseAddr();
			}
			break;

		case BLTXYSTRIDES:
			if (diago.option >= TEST_STRIDES) {
			    int ds = iRandom(diago.xmaxscreen)*2;
			    int ss = iRandom(diago.xmaxscreen)*2;
			    ds &= ~7;
			    ss &= ~7;
			    SET(sstg->bltXYstrides,(ds<<16) | ss);
			}
			break;

		case BLTSRCCHROMARANGE:
			SET(sstg->bltSrcChromaRange, iRandom(0xFFFFFFFF));
			break;

		case BLTDSTCHROMARANGE:
			SET(sstg->bltDstChromaRange, iRandom(0xFFFFFFFF));
			break;

		case BLTCLIPX:
		{
			int left, right;
			left = rRandom(l-diago.tsize,l+diago.tsize);
			right = rRandom(r-diago.tsize,r+diago.tsize);
			SET(sstg->bltClipX, (left<<16) | right);
			break;
		}

		case BLTCLIPY:
		{
			int bot,top;
			bot = rRandom(b-diago.tsize,b+diago.tsize);
			top = rRandom(t-diago.tsize,t+diago.tsize);
			SET(sstg->bltClipY, (bot<<16) | top);
			break;
		}

		case BLTSRCXY:
			xs = rRandom(l,r);
			ys = rRandom(b,t);
			SET(sstg->bltSrcXY, (ys<<16) | xs);
			break;

		case BLTDSTXY:
			xd = rRandom(l,r);
			yd = rRandom(b,t);
			go = iRandom(1) ? SSTG_GO : 0;
			SET(sstg->bltDstXY, (yd<<16) | xd | go);
			if (go) sendBlit(sstg,cmd,srcFormat,w,h);
			break;

		case BLTSIZE:
		{
			int bsize;

			if ((cmd & SSTG_COMMAND) == SSTG_FRECTFILL) {
			    w = rRandom(0,diago.tsize/2);
			    h = rRandom(0,diago.tsize/2);
			}
			else if ((cmd & SSTG_COMMAND) == SSTG_HOST_BLT) {
			    if ((cmd & SSTG_SRC_FORMAT)==SSTG_PIXFMT_1BPP) {
				w = rRandom(0,7);
				h = rRandom(0,diago.tsize);
			    }
			    else {
				w = rRandom(0,diago.tsize);
				h = rRandom(0,diago.tsize);
			    }
			}
			else {
			    w = rRandom(-diago.tsize,diago.tsize);
			    h = rRandom(-diago.tsize,diago.tsize);
			}
			if ((cmd & SSTG_COMMAND) == SSTG_BLT) {
			    // random bases guarantee no overlap
			    if (diago.option < TEST_BASE) {
				if (xs < xd) {		// must go right to left
				    if (w > 0) w = -w;
				}
				else if (w < 0) w = -w;
				if (ys < yd) {		// must go top to bottom
				    if (h > 0) h = -h;
				}
				else if (h < 0) h = -h;
			    }
			}
			bsize = ((h&0xFFF)<<16) | (w&0xFFF);
			go = iRandom(1) ? SSTG_GO : 0;
			SET(sstg->bltSize, bsize | go);
			if (go) sendBlit(sstg,cmd,srcFormat,w,h);
			
			break;
		}

		case BLTROP:
			SET(sstg->bltRop, iRandom(0xFFFF));
			break;

		case BLTCOLOR:
			SET(sstg->bltColor, iRandom(0xFFFFFFFF));
			break;

		case RESERVEDF:
		case RESERVEDG:
		case BLTDATA:
		case BLTCOMMAND:
		{
		try_again:
			cmd = iRandom(0xFFFFFF);
			if (!diago.ytiled)
			    cmd &= ~(SSTG_SRC_IS_TILED | SSTG_DST_IS_TILED);
			if ((cmd & SSTG_SRC_FORMAT) > SSTG_PIXFMT_24BPPdit4)
				goto try_again;
			if (cmd & SSTG_SRC_IS_TILED) {
cmd &= ~SSTG_SRC_IS_TILED;
			}
			if (checkDstBaseAddr())
			    doDstBaseAddr();

			switch (cmd & SSTG_COMMAND) {
			    case SSTG_BLT:		// make sure direction is OK
			    // random bases guarantee no overlap
			    if (diago.option < TEST_BASE) {
				if (xs < xd) {		// must go right to left
				    if (w > 0) w = -w;
				}
				else if (w < 0) w = -w;
				if (ys < yd) {		// must go top to bottom
				    if (h > 0) h = -h;
				}
				else if (h < 0) h = -h;
			    }
				SET(sstg->bltSize, ((h&0xFFF)<<16) | (w&0xFFF));
				break;

			    case SSTG_HOST_BLT:
				if (diago.option >= TEST_HBLIT) {
				    if (w < 0) {
					w = rRandom(0,diago.tsize);
					SET(sstg->bltSize, ((h&0xFFF)<<16) | (w&0xFFF));
				    }
				    if (h < 0) {
					h = rRandom(0,diago.tsize);
					SET(sstg->bltSize, ((h&0xFFF)<<16) | (w&0xFFF));
				    }
				    if ((w > 7) && ((cmd & SSTG_SRC_FORMAT)==SSTG_PIXFMT_1BPP)) {
					w = rRandom(0,7);
					SET(sstg->bltSize, ((h&0xFFF)<<16) | (w&0xFFF));
				    }
				    go = cmd & SSTG_GO;
				}
				else goto try_again;
				break;
			    case SSTG_RECTFILL:		// no problemo
				break;
			    case SSTG_FRECTFILL:
				if (w < 0 || h < 0) {	// force positive sizes
				    w = rRandom(0,diago.tsize/3);
				    h = rRandom(0,diago.tsize/3);
				    SET(sstg->bltSize, (h<<16) | w);
				};
				break;
			    default:
				goto try_again;
			}

			SET(sstg->bltCommand, cmd);
			if (go) sendBlit(sstg,cmd,srcFormat,w,h);
			break;
		}

		default:
			GDBG_ERROR("main", "default case reached\n");
	    }
#endif
	    if (DIAG_EXCEEDED_PIXEL_LIMIT())
		break;
	}
    }
    DIAG_PASS(0);
}

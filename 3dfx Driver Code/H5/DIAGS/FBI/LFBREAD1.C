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
** $Date: 10/11/00 8:10:15 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

SstRegs *sst;

// return an LFB read of 1 pixel in 24-bit RGB format (truncated)
unsigned long lfbRead(int x, int y)
{
    unsigned short *lfbAddr = (unsigned short *)SST_LFB_ADDRESS(sst);
    unsigned long c;

    // Always sync after lfbmode change
    sst_idle_really(diago.sst);
    gdbg_info(5,"checking %d,%d\n",x,y);
    lfbAddr += lfbOffset(x&~1,y);
    c = GET(((unsigned long *)lfbAddr)[0]);
    if (x & 1) c >>= 16;		// get high word (odd x pixel)
    return ((c & 0xF800)<<8) | ((c & 0x07E0)<<5) | ((c & 0x001F)<<3);
}

void
main (int argc, char **argv)
{
    int i,n;
    long x,y,w,h;
    unsigned long cFastFill, cTriangle, cLfbwrite, cLast;
    unsigned long fbzMode, *lfbAddr;

    sst = SST_BEGIN(argc,argv);

    // select CO as local color for triangles
    SET(sst->fbzColorPath, SST_LOCALSELECT | SST_CC_REPLACE);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<33; n++) {			// do 33 tests
    again:
	xyRandom(&x,&y);			// pick random x,y onscreen
	do {					// and random width and height
	    w = 4+iRandom(10);
	    h = 3+iRandom(10);
	} while (w * h > 87);
	if (!ONSCREEN(x+w-1,y+h-1))
	    goto again;

	cFastFill = colRandom24();		// and random colors
	cTriangle = colRandom24();
	cLfbwrite = colRandom24();
	gdbg_info(3,"cFastFill = 0x%06x\n", cFastFill);
	gdbg_info(3,"cTriangle = 0x%06x\n", cTriangle);
	gdbg_info(3,"cLfbwrite = 0x%06x\n", cLfbwrite);

	// set up all the state for random triangles, fastfills, and lfb writes
	fbzMode = SST_RGBWRMASK | drawbufferRandom();
	SET(sst->fbzMode, fbzMode);
#ifdef CVG
	SET(sst->lfbMode, SST_LFB_888 |
			(diago.curdrawbuffer ?
				(SST_LFB_WRITEBACKBUFFER|SST_LFB_READBACKBUFFER) :
				(SST_LFB_WRITEFRONTBUFFER | SST_LFB_READFRONTBUFFER)));
#else // H3
	SET(sst->lfbMode, SST_LFB_888 | SST_LFB_READCOLORBUFFER);
#endif

	SET(sst->c0,cTriangle);			// set the color for triangles
	SET(sst->c1,cFastFill);			// set the color for fastfill

	// now issue some random drawing commands in random order
	for (i=rRandom(1,5); i>0; i--) {
	    switch(iRandom(2)) {
		case 0:	// fastfill rectangle
			gdbg_info(2,"fastfill %d,%d to %d,%d\n",x,y,x+w,y+h);
			SET(sst->clipLeftRight, (x<<16) | (x+w));
			SET(sst->clipBottomTop, (y<<16) | (y+h));
			SET(sst->fastfillCMD, 1);
			cLast = cFastFill;
			break;

		case 1: // right angle triangle
			gdbg_info(2,"triangle %d,%d to %d,%d\n",x,y,x+w,y+h);
			SET(sst->vA.x, x<<SST_XY_FRACBITS);
			SET(sst->vA.y, y<<SST_XY_FRACBITS);
			SET(sst->vB.x, (x+w)<<SST_XY_FRACBITS);
			SET(sst->vB.y, y<<SST_XY_FRACBITS);
			SET(sst->vC.x, x<<SST_XY_FRACBITS);
			SET(sst->vC.y, (y+h)<<SST_XY_FRACBITS);
			SET(sst->triangleCMD, 0);
			cLast = cTriangle;
			break;

		case 2: // lfb write 3x3 rectangle
			gdbg_info(2,"lfb writes %d,%d to %d,%d\n",x,y,x+3,y+3);
			lfbAddr = (unsigned long *)SST_LFB_ADDRESS(sst);
			lfbAddr += lfbOffset(x,y);
			SET(lfbAddr[0],cLfbwrite);
			SET(lfbAddr[1],cLfbwrite);
			SET(lfbAddr[2],cLfbwrite);

			lfbAddr = (unsigned long *)SST_LFB_ADDRESS(sst);
			lfbAddr += lfbOffset(x,y+1);
			SET(lfbAddr[0],cLfbwrite);
			SET(lfbAddr[1],cLfbwrite);
			SET(lfbAddr[2],cLfbwrite);

			lfbAddr = (unsigned long *)SST_LFB_ADDRESS(sst);
			lfbAddr += lfbOffset(x,y+2);
			SET(lfbAddr[0],cLfbwrite);
			SET(lfbAddr[1],cLfbwrite);
			SET(lfbAddr[2],cLfbwrite);
			cLast = cLfbwrite;
			break;
	    }
	    // and DO NOT wait until they are done
	    // issue some LFB reads right away
	    cLast &= 0xF8FCF8;		// truncate to 565
	    for (i=rRandom(1,5); i>0; i--) {
		switch(iRandom(3)) {
		    case 0:	DIAG_TESTLFBREAD16(x,y,lfbRead(x,y),cLast);
				break;
		    case 1:	DIAG_TESTLFBREAD16(x+1,y,lfbRead(x+1,y),cLast);
				break;
		    case 2:	DIAG_TESTLFBREAD16(x,y+1,lfbRead(x,y+1),cLast);
				break;
		    case 3:	DIAG_TESTLFBREAD16(x+1,y+1,lfbRead(x+1,y+1),cLast);
				break;
		}
	    }
	}
    }
    DIAG_PASS(0);
}



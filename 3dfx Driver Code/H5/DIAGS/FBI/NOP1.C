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
** $Date: 10/11/00 8:10:20 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int n;
    long x,y, nop;
    FxU32 fbiPix, fbiTri, pixelCount,triangleCount;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    pixelCount = 0;
    triangleCount = 0;

    SET(sst->r, 0xdeadb);		// set the color
    SET(sst->g, 0xbeefd);		// set the color
    SET(sst->b, 0xbadba);		// set the color

    while (DIAG_STARTPASS())		// for each pass
    for (n=0; n<100; n++) {		// do 100 tests
	xyRandom(&x,&y);
	nop = iRandom(0xF);
	gdbg_info(2,"nop(%d) x,y = %d,%d\n", nop,x,y);

	pixelCount++;
	// draw a pixel using any command
	if (sst_drawpixel(sst,x,y,0xFFFF,1) == 0)
	    triangleCount++;
	SET(sst->nopCMD,nop);		// NOP comand
	if (nop & SST_NOP_RESET_PIXEL_STATS) {
	    pixelCount=0;
	}
	if (nop & SST_NOP_RESET_TRIANGLE_STATS) {
	    triangleCount=0;
	}

	sst_idle_really(sst);		// wait for the command to complete
	if (!diago.sliEnabled && !diago.aaEnabled)
	{
	    fbiPix = GET(sst->stats.fbiPixelsIn);
	    DIAG_TESTREG32("fbiPixelsIn",pixelCount,fbiPix);
	    fbiPix = GET(sst->stats.fbiPixelsOut);
	    DIAG_TESTREG32("fbiPixelsOut",pixelCount,fbiPix);
	    fbiTri = GET(sst->fbiTrianglesOut);
	    DIAG_TESTREG32("fbiTrianglesOut",triangleCount,fbiTri);
	}
    }
    DIAG_PASS(0);
}

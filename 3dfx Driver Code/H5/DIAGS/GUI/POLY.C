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
** $Date: 10/11/00 8:11:28 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

// This diagnostic tests only polygon rasterization, ie the edge-filling
// rules and command protocol for edges

void
main (int argc, char **argv)
{
    int j,maxVerts, xc,yc;
    FxU32 cfore, srcFormat;
    Polygon poly;
    SstRegs *sst;
    SstGRegs *sstg;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);

    // Print Out Option Description
    if ( diago.printOpts ) {
	gdbg_printf( "poly option description:\n" );
	gdbg_printf( " -O # -> max vertices per polygon, default is 10\n");
	DIAG_FAIL();
    }
    maxVerts = 10;
    if (diago.option) {
	maxVerts = diago.option;
	if (maxVerts < 3) {
	    GDBG_ERROR("main", "maxVerts is too small (%d is <3)\n",maxVerts);
	    DIAG_FAIL();
	}
    }

    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);

    // HACK: fetch the srcFormat from the actual CSIM
    srcFormat = CSIM_PRIVATE(diago.sstCSIM)->gui.srcFormat;

    while (DIAG_STARTPASS()) {			// for each pass
	for (j=0; j<100; j++) {			// and do a number of polygons
	    cfore = iRandom(0xFFFFFF);
	    SET(sstg->colorFore, cfore);

	    poly.numVerts = rRandom(3,maxVerts);
	    sstg_random_poly(&poly, 0, 0, 0);
	    sstg_print_poly(&poly);
	    sstg_draw_poly(sstg,&poly, SSTG_ROP_SRC << SSTG_ROP0_SHIFT, 1);
	    sstg_idle(sst);			// wait for the command to complete
	    cfore = sstg_destination_mask(cfore);

	    // check the entire rectangle, with a 1 pixel border
	    for (yc = poly.yb-1; yc <= poly.yt+1; yc++)
	    for (xc = poly.xl-1; xc <= poly.xr+1; xc++)
	    if (ONSCREEN(xc,yc)) {
		DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,
				xc,yc,
				sstg_inside_poly(&poly,xc,yc) ? cfore : 0);
	    }

	    // now cleanup, erase this polygon
	    DIAG_FORCE_RECT(CSIM_BUF_2D_DST,
				poly.xl,
				poly.yb,
				poly.xr - poly.xl + 1,
				poly.yt - poly.yb + 1,
				0);
	    if (DIAG_EXCEEDED_PIXEL_LIMIT())
		break;
	}
    }
    DIAG_PASS(0);
}

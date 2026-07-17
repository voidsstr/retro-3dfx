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
** $Date: 10/11/00 8:10:35 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int j,n, fbz,fbzCP;
    static Triangle t,t1;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    if (!diago.diff) {
	gdbg_error("stress1","must run with -D option, forcing -D\n");
	diago.diff = 1;
    }
    if (diago.zeroLodFrac && !diago.hasAuxBuffer) {
	gdbg_error("stress1","must run without -Z option when no zbuffer present, turning off -Z\n");
	diago.zeroLodFrac = 0;
    }

    // setup some reasonable starting modes
    fbz = SST_RGBWRMASK;
    fbzCP = SST_RGBSEL_RGBA;
    if (diago.zeroLodFrac)			// enable Zbuffer
	fbz |= SST_ENDEPTHBUFFER | SST_ZAWRMASK | SST_ZFUNC_GT;
    SET(sst->fbzColorPath, fbzCP);

    // draw random triangles in a small area in order to stress the
    // read-ahead hardware, we use 2* triangle size as the area

    while (DIAG_STARTPASS()) {			// for each pass
	if (diago.checkEveryTriangle || !diago.diff)
	    DIAG_DIFFSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1);
	for (j=0; j<20; j++) {			// do 20 areas
	    int cx,cy;
	    cx = iRandom(diago.xmaxscreen*XY_ONE-1);
	    cy = iRandom(diago.ymaxscreen*XY_ONE-1);
	    gdbg_info(2,"area center = %d.%x,%d.%x\n",cx>>4,cx&0xF,cy>>4,cy&0xFF);
	    SET(sst->fbzMode, fbz | drawbufferRandom());
	    if (iRandom(1)) {
		fbzCP ^= SST_PARMADJUST;
		SET(sst->fbzColorPath, fbzCP);
	    }

	    for (n=0; n<10; n++) {		// 10 triangles per area
		int s = diago.tsize * XY_ONE/4;
		if (s < 0) s = -s;
		do {
		    t.vA.x = cx + rRandom(-s,s);
		    t.vA.y = cy + rRandom(-s,s);
		} while (!ONSCREEN_FRAC(t.vA.x,t.vA.y));
		// GMT: every once in a while generate a small triangle off 
		// somewhere else on the screen. This is a regression test for
		// an SLI bug
		if (iRandom(3)==0) {
		    gdbg_info(2,"drawing small triangle somewhere else\n");
		    randomTriangle(&t1,-1,1);		// pick random triangle
		    if (iRandom(1)) {			// make most of them very flat
			t1.vA.y = t1.vB.y+1;
			t1.vC.y = t1.vB.y+2;
		    }
		    randomRgbaTriangle(&t1);		// with random colors
		    randomZTriangle(&t1);		// with random Z
		    areaTriangle(&t1);			// compute the area (before setup)
		    setupTriangle(&t1,1,0,1);		// setup Z slopes
		    sortTriangle(&t1);			// sort it
		    drawTriangle(sst,&t1,1,0,1);
		}

		randomTriangle1(&t,diago.tsize,1);	// pick random triangle
		randomRgbaTriangle(&t);			// with random colors
		randomZTriangle(&t);			// with random Z

		areaTriangle(&t);			// compute the area (before setup)
		setupTriangle(&t,1,0,1);		// setup Z slopes
		sortTriangle(&t);			// sort it
		printTriangle(2,&t,1,0,1);
		printTriangleSlopes(3,&t,1,0,1);

		// NOTE: if sub-pixel parameter adjustment is OFF we may get
		//	 color overflows/underflows etc
		drawTriangle(sst,&t,1,0,1);
	    }
	}
    }
    DIAG_PASS(0);
}
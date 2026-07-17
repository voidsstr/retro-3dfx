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
    static Triangle t;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    if (!diago.diff) {
	gdbg_error("stress2","must run with -D option, forcing -D\n");
	diago.diff = 1;
    }
    if (diago.zeroLodFrac && !diago.hasAuxBuffer) {
	gdbg_error("stress2","must run without -Z option when no zbuffer present, turning off -Z\n");
	diago.zeroLodFrac = 0;
    }

    // setup some reasonable starting modes
    fbz = SST_RGBWRMASK;
    fbzCP = SST_RGBSEL_RGBA;
    if (diago.zeroLodFrac)			// enable Zbuffer
	fbz |= SST_ENDEPTHBUFFER | SST_ZAWRMASK | SST_ZFUNC_GT;
    SET(sst->fbzColorPath, fbzCP);

    // draw random triangles, each one's A vertex is just aa little lower in Y
    // than the previous one's C vertex (stresses read-ahead logic)

    while (DIAG_STARTPASS()) {			// for each pass
	if (diago.checkEveryTriangle || !diago.diff)
	    DIAG_DIFFSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1);
	for (j=0; j<5; j++) {			// do 5 areas
	    SET(sst->fbzMode, fbz | drawbufferRandom());
	    if (iRandom(1)) {
		fbzCP ^= SST_PARMADJUST;
		SET(sst->fbzColorPath, fbzCP);
	    }

	    n = diago.tsize;			// get triangle size
	    if (n < 0) n = -n;			// absolute value
	    n = (diago.ymaxscreen-12*n);	// make room for drawing
	    if (n < XY_ONE) {			// sanity check
		gdbg_error("main","triangle size is too large\n");
		DIAG_FAIL();
	    }
	    t.vC.x = iRandom(diago.xmaxscreen*XY_ONE-1);
	    t.vC.y = rRandom(XY_ONE,n*XY_ONE);

	    for (n=0; n<10; n++) {		// 10 triangles per area
		do {
		    t.vA.x = t.vC.x + rRandom(-1*XY_ONE,1*XY_ONE);
		    t.vA.y = t.vC.y - rRandom(XY_ONE/2,4*XY_ONE);
		} while (!ONSCREEN_FRAC(t.vA.x,t.vA.y));
		do {
		    randomTriangle1(&t,diago.tsize,1);	// pick random triangle
		} while (t.vA.y > t.vB.y || t.vA.y > t.vC.y);
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

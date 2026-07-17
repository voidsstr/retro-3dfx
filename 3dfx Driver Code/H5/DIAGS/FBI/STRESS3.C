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
** $Date: 10/11/00 8:10:36 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int amode,j,n,fbz,fbzCP;
    static Triangle t;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    if (!diago.diff) {
	gdbg_error("stress3","must run with -D option, forcing -D\n");
	diago.diff = 1;
    }
    if (diago.zeroLodFrac && !diago.hasAuxBuffer) {
	gdbg_error("stress3","must run without -Z option when no alpha present, turning off -Z\n");
	diago.zeroLodFrac = 0;
    }

    // setup some reasonable starting modes
    fbz = SST_RGBWRMASK;
    fbzCP = SST_RGBSEL_RGBA;
    if (diago.zeroLodFrac && diago.rgb == 16)
	fbz |= SST_ZAWRMASK | SST_ENALPHABUFFER;
    SET(sst->fbzColorPath, fbzCP);

    // draw random triangles in a small area in order to stress the
    // read-ahead hardware, we use 2* triangle size as the area

    while (DIAG_STARTPASS()) {			// for each pass
	if (diago.checkEveryTriangle || !diago.diff)
	    DIAG_DIFFSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1);
	for (j=0; j<5; j++) {			// do 5 areas
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

		randomTriangle1(&t,diago.tsize,1);	// pick random triangle
		randomRgbaTriangle(&t);			// with random colors
		randomZTriangle(&t);			// with random Z

		areaTriangle(&t);			// compute the area (before setup)
		setupTriangle(&t,1,1,0);		// setup RGBA slopes
		sortTriangle(&t);			// sort it
		printTriangle(2,&t,1,1,0);
		printTriangleSlopes(3,&t,1,1,0);

		// NOTE: if sub-pixel parameter adjustment is OFF we may get
		//	 color overflows/underflows etc
	    again:
		do {
		    amode = iRandom(0xFFFFFFFF) & (SST_ENALPHABLEND |
			SST_RGBSRCFACT|SST_RGBDSTFACT|SST_ASRCFACT|SST_ADSTFACT);
		} while (!goodAlphaMode(amode,fbz));
		if (!diago.zeroLodFrac) {	// if no alpha buffer
		    int fact;			// disallow any dest. alpha factors
		    fact = (amode & SST_RGBSRCFACT) >> SST_RGBSRCFACT_SHIFT;
		    if (fact == SST_A_DSTALPHA) goto again;
		    if (fact == SST_AOM_DSTALPHA) goto again;
		    if (fact == SST_A_SATURATE) goto again;
		    fact = (amode & SST_ASRCFACT) >> SST_ASRCFACT_SHIFT;
		    if (fact == SST_A_DSTALPHA) goto again;
		    if (fact == SST_AOM_DSTALPHA) goto again;
		    if (fact == SST_A_SATURATE) goto again;
		    fact = (amode & SST_RGBDSTFACT) >> SST_RGBDSTFACT_SHIFT;
		    if (fact == SST_A_DSTALPHA) goto again;
		    if (fact == SST_AOM_DSTALPHA) goto again;
		    fact = (amode & SST_ADSTFACT) >> SST_ADSTFACT_SHIFT;
		    if (fact == SST_A_DSTALPHA) goto again;
		    if (fact == SST_AOM_DSTALPHA) goto again;
		}
		SET(sst->alphaMode,amode);
		gdbg_info(2,"alphaMode = 0x%08x\n",amode);
		drawTriangle(sst,&t,1,1,0);
	    }
	}
    }
    DIAG_PASS(0);
}

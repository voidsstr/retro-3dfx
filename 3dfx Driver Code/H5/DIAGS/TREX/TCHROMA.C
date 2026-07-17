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
** $Date: 10/11/00 8:19:21 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

void
main (int argc, char **argv)
{
    int n, format;
    Triangle *t;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    if(diago.bigAssTextures)
      t = buildTriangle(2048, 2048);
    else
      t = buildTriangle(256, 256);

    t->next = NULL;
    SET(sst->c1, 0xFECDBA);			// hack for -v option
    SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT |
		SST_ENTEXTUREMAP |
		(diago.adjust?SST_PARMADJUST:0));

    // NOTE: always runs in non-perspective, bilinear mode

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<100; n++) {			// do 100 tests
	if (diago.rgb == 16)
	    SET(sst->fbzMode, SST_RGBWRMASK | SST_ZAWRMASK | SST_ENALPHABUFFER |
		drawbufferRandom());
	else
	    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());

	format = SST_P8;			// only check paletted
	format |= SST_TMINFILTER | SST_TMAGFILTER;	// force bilinear
	if (diago.clamp) {
	    if (iRandom(1)) format |= SST_TCLAMPS;
	    if (iRandom(1)) format |= SST_TCLAMPT;
	    if (iRandom(1)) format |= SST_TCLAMPW;
	}
	gdbg_info(3,"bilin=%d  clamp s,t,w=%d,%d,%d\n",
			(format & SST_TMINFILTER) != 0,
			(format & SST_TCLAMPS) != 0,
			(format & SST_TCLAMPT) != 0,
			(format & SST_TCLAMPW) != 0);
	if (diago.flip) {
	    if (iRandom(1)) t->tex->tLOD ^= SST_TMIRRORS;
	    if (iRandom(1)) t->tex->tLOD ^= SST_TMIRRORT;
	    gdbg_info(3,"mirror=%s %s \n",
			t->tex->tLOD & SST_TMIRRORS ? "S" : " ",
			t->tex->tLOD & SST_TMIRRORT ? "T" : " ");
	    SET(SST_TREX(sst,t->tex->trex)->tLOD,t->tex->tLOD);
	}
	t->tex->tMode = format | SST_TC_REPLACE | SST_TCA_REPLACE;
	// init random chroma mode, but not colors (texRandomTextureMap does this)
	t->tex->cmask = n & 0xF;				// which case to test
	t->tex->tChromarange = SST_ENCHROMAKEY_TMU;	// always enable chroma

	if (iRandom(1))					// random stuff
	    t->tex->tChromarange |= SST_ENCHROMARANGE;
	/*
	if (iRandom(1))
	    t->tex->tChromarange |= SST_ENCOLORSUBSTITUTION;
	if (n < 32)					// make sure we stress this
	    t->tex->tChromarange |= SST_ENCOLORSUBSTITUTION;
	*/

	if (iRandom(1))
	    t->tex->tChromarange |= SST_CHROMARANGE_BLOCK_OR;
	if (iRandom(1))
	    t->tex->tChromarange |= SST_CHROMARANGE_BLUE_EX  |
					SST_CHROMARANGE_GREEN_EX |
					SST_CHROMARANGE_RED_EX;
	if (t->tex->tChromarange & SST_ENCHROMARANGE)
	    gdbg_info(2,"mask:%x  CRange enabled: %s %sclusive\n",
		t->tex->cmask,
		(t->tex->tChromarange & SST_CHROMARANGE_BLOCK_OR) ? "Union" : "Intersection",
		(t->tex->tChromarange & SST_CHROMARANGE_BLUE_EX) ? "Ex" : "In");
	else
	    gdbg_info(2,"mask:%x  CRange disabled\n",t->tex->cmask);

	// this routine does special stuff when chroma is enabled
	texRandomTextureMap(sst,diago.trex, 0,1,1,t->tex);	// 2x2

	// generate a simple right triangle, vertex A is on a pixel center
	t->vA.x = iRandom(diago.xmaxscreen-diago.tsize-1)*XY_ONE+XY_ONE/2;
	t->vA.y = iRandom(diago.ymaxscreen-diago.tsize-1)*XY_ONE+XY_ONE/2;
	t->vB.x = t->vA.x + diago.tsize*XY_ONE;
	t->vB.y = t->vA.y;
	t->vC.x = t->vA.x;
	t->vC.y = t->vA.y + diago.tsize*XY_ONE;
	t->vA.s = FX_BIT64(6+SST_ST64_FRACBITS);	// 64 or center of (0,0)
	t->vA.t = FX_BIT64(6+SST_ST64_FRACBITS);
	t->vA.w = FX_BIT64(32);
	t->vB.w = FX_BIT64(32);
	t->vC.w = FX_BIT64(32);
	if (n < 32) {				// keep it simple
	    t->vB.s = t->vA.s + FX_BIT64(8+SST_ST64_FRACBITS);
	    t->vB.t = t->vA.t;
	    t->vC.s = t->vA.s;
	    t->vC.t = t->vA.t + FX_BIT64(8+SST_ST64_FRACBITS);
	}
	else {					// mix it up a bit
	    // make sure we check positive clamp and flip
	    t->vB.s = t->vA.s + iRandom64(FX_BIT64(9+SST_ST64_FRACBITS));
	    t->vB.t = iRandom64(t->vA.t);
	    t->vC.s = iRandom64(t->vA.s);
	    t->vC.t = t->vA.t + iRandom64(FX_BIT64(9+SST_ST64_FRACBITS));
	    // make sure we test negative clamp and flip
	    t->vA.s -= iRandom64(3*t->vA.s);
	    t->vA.t -= iRandom64(3*t->vA.t);
	}

	areaTriangle(t);			// compute the area (before setup)
	if (!setupStwTriangle(t))		// setup STW slopes
	    GDBG_ERROR("main","oops\n");
	printStwTriangle(4,t);
	printStwTriangleSlopes(5,t);

	// NOTE: sub-pixel parameter adjustment is OFF so we may end up with
	//	 color overflows/underflows etc
	drawStwTriangle(sst,t);

	sst_idle(sst);				// wait for the command to complete
	// NOTE: insideStTriangle checks alpha so we pass checkA=0 to checkTriangle
	checkTriangle(t,1,1,diago.adjust, insideStTriangle,0,0,0);
	eraseTriangle(sst,t,1,1,0);		// erase the triangle
    }
    DIAG_PASS(0);
}

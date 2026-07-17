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
** $Date: 10/11/00 8:19:00 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

void
main (int argc, char **argv)
{
    int n,j,k, format,slog,tlog, array[7];
    Vertex a,b,c;
    Triangle *t;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    if(diago.bigAssTextures)
      t = buildTriangle(2048,2048);
    else
      t = buildTriangle(256,256);

    t->next = NULL;
    diago.adjust = 1;			// need this on
     
    while (DIAG_STARTPASS()) {			// for each pass
	scrambleRandom(7,array);
	for (n=0; n<7; n++) {
	    // random 8-bit/16-bit uncompressed, NCC option forces 8-bit
	    format = texRandomFormat(diago.tex8==0,diago.tex32,diago.ncc);
	    t->tex->tMode = format | SST_TC_REPLACE | SST_TCA_REPLACE;

	    slog = tlog = 8;			// now pick random texture size
	    j = array[n];			// begin with 256x256
	    if (j > 3)
		slog -= j-3;
	    else
		tlog -= j;

	    texRandomTextureMap(sst,diago.trex, 1,slog,tlog,t->tex);

	    SET(sst->fbzColorPath, SST_ENTEXTUREMAP | SST_RGBSEL_TREXOUT |
			SST_ASEL_TREXOUT | (diago.adjust?SST_PARMADJUST:0));

	    if (diago.rgb == 16)
		SET(sst->fbzMode, SST_RGBWRMASK | SST_ZAWRMASK | SST_ENALPHABUFFER |
			drawbufferRandom());
	    else
		SET(sst->fbzMode, SST_RGBWRMASK | SST_ZAWRMASK | drawbufferRandom());

	    // now go thru each mipmap level and draw all the texels using
	    // a rectangle and then check it
	    for (k=0; k<9; k++) {
	      if(diago.bigAssTextures)
		{  //For big textures, we're testing LOD 3-11, not 0-8
		  t->tex->lodmin = (k+3)<<SST_LOD_FRACBITS;
		  t->tex->lodmax = (k+3)<<SST_LOD_FRACBITS;
		}
	      else  //256x256 textures
		{
		  t->tex->lodmin = k<<SST_LOD_FRACBITS;
		  t->tex->lodmax = k<<SST_LOD_FRACBITS;
		}

		t->tex->tLOD &= ~(SST_LODMIN | SST_LODMAX);	// clear out bits
		t->tex->tLOD |= (t->tex->lodmin<<SST_LODMIN_SHIFT) |
			(t->tex->lodmax<<SST_LODMAX_SHIFT);
		SET(SST_TREX(sst,t->tex->trex)->tLOD,t->tex->tLOD);	

		t->vA.x = iRandom(diago.xmaxscreen-(1<<slog)-1)*XY_ONE;
		t->vA.y = iRandom(diago.ymaxscreen-(1<<tlog)-1)*XY_ONE;
		t->vB.x = t->vA.x + ((1<<slog)<<SST_XY_FRACBITS);
		t->vB.y = t->vA.y;
		t->vC.x = t->vA.x;
		t->vC.y = t->vA.y + ((1<<tlog)<<SST_XY_FRACBITS);
		t->vA.s = 0;
		t->vA.t = 0;
		t->vA.w = 0;
		t->vB.s = ((FxI64)256)<<SST_ST64_FRACBITS;
		t->vB.t = 0;
		t->vB.w = 0;
		t->vC.s = 0;
		t->vC.t = ((FxI64)256)<<SST_ST64_FRACBITS;
		t->vC.w = 0;
		if (slog > tlog)
		    t->vC.t >>= slog-tlog;
		else
		    t->vB.s >>= tlog-slog;
		a = t->vA;			// save these
		b = t->vB;
		c = t->vC;
		areaTriangle(t);		// compute the area (before setup)
		setupStwTriangle(t);		// setup STW slopes
		sortTriangle(t);		// sort it
		printStwTriangle(4,t);
		printStwTriangleSlopes(5,t);
		drawStwTriangle(sst,t);

		sst_idle(sst);			// wait for the command to complete
		checkTriangle(t,1,1,diago.adjust, insideStTriangle,0,0,0);
		eraseTriangle(sst,t,1,texFormatHasAlpha(format),0);	// erase the triangle

		t->vA = a;
		t->vB = b;
		t->vC = c;
		t->vA.x = t->vB.x;
		t->vA.y = t->vC.y;
		t->vA.s = t->vB.s;
		t->vA.t = t->vC.t;
		areaTriangle(t);		// compute the area (before setup)
		setupStwTriangle(t);		// setup STW slopes
		sortTriangle(t);		// sort it
		printStwTriangle(4,t);
		printStwTriangleSlopes(5,t);
		drawStwTriangle(sst,t);

		sst_idle(sst);			// wait for the command to complete
		checkTriangle(t,1,1,diago.adjust, insideStTriangle,0,0,0);
		eraseTriangle(sst,t,1,texFormatHasAlpha(format),0);	// erase the triangle

		if (slog > 0) slog--;
		if (tlog > 0) tlog--;
	    }
	}
    }
    DIAG_PASS(1);				// check for black screen
}

/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:19:12 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

void
main (int argc, char **argv)
{
    int i,j,k,n, format;
    Triangle *t;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    if(diago.bigAssTextures)
      t = buildTriangle(2048, 2048);
    else
      t = buildTriangle(256, 256);

    t->next = NULL;
    diago.adjust = 0;				// force this off

  while (DIAG_STARTPASS()) {			// for each pass
    SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT | SST_ENTEXTUREMAP |
		(diago.adjust?SST_PARMADJUST:0));

    for (n=0; n<50; n++) {			// do 50 tests
	static FxU32 _fmt[] = {SST_P8, SST_AP88, SST_P8_ARGB6666};

	if (diago.rgb == 16)
	    SET(sst->fbzMode, SST_RGBWRMASK | SST_ZAWRMASK | SST_ENALPHABUFFER |
		drawbufferRandom());
	else
	    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());
	// use random 8 or 16 bit texture format, size = 1x1, in replace mode
	format = _fmt[iRandom(2)];
	if (diago.bilinear) {			// if bilinear enabled
	    if (diago.bilinear > 0 || iRandom(1))
		format |= SST_TMINFILTER | SST_TMAGFILTER;
	}
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
	t->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) | 
			format | SST_TC_REPLACE | SST_TCA_REPLACE;
	texRandomTextureMap(sst,diago.trex,0, 0,0,t->tex);

    again:
	randomTriangle(t,diago.tsize,1);	// pick random triangle
	randomSt1418Triangle(t);		// with random texcoords
//	printStwTriangle(4,t);
	if (diago.perspective)			// if testing perspective
	    randomW230Triangle(t);		// generate random W

	areaTriangle(t);			// compute the area (before setup)
	if (!setupStwTriangle(t))		// setup STW slopes
	    goto again;				// reject bad slopes
	if (t->area == 0)			// reject zero area triangles
	    goto again;
	sortTriangle(t);			// sort it
	printStwTriangle(4,t);
	printStwTriangleSlopes(5,t);

	// NOTE: sub-pixel parameter adjustment is OFF so we may end up with
	//	 color overflows/underflows etc
	drawStwTriangle(sst,t);

	// now do a random number of random things: either triangles or palettes
	i = 1+iRandom(17);
	gdbg_info(2,"%d random loops\n",i);
	while (i-- > 0) {
	    for (k = 1+iRandom(iRandom(20)); k>0; k--) {
		volatile unsigned long *nTab;
		if (iRandom(3))
		    j = t->tex->mip[8]->data[0] & 0xFF;// use same value
		else
		    j = iRandom(255);		// pick a random index

		t->tex->palette[j] = 0x80000000 | iRandom(0xFFFFFF);
		gdbg_info(5,"palette[%d] = 0x%x\n",j,t->tex->palette[j]);
		nTab = sst->nccTable0;		// point to NCC table
		nTab += rRandom(4,11)&~1;	// pick random EVEN I or Q slot
		SET(nTab[j&1], ((j>>1)<<24) | t->tex->palette[j]);
		
	    }
	    gdbg_info(5,"triangle\n");
	    SET(sst->triangleCMD,t->area);
	}

	sst_idle(sst);				// wait for the command to complete
	checkTriangle(t,1,1,diago.adjust, insideStTriangle,0,0,0);
	eraseTriangle(sst,t,1,0,0);		// erase the triangle
    }
  }
  DIAG_PASS(0);
}

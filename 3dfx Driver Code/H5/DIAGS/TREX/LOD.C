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
** $Date: 10/11/00 8:19:08 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

void
main (int argc, char **argv)
{
    int n, format, minLod;
    Triangle *t;
    SstRegs *sst;
    FxU32 logLargestPossibleLOD;

    sst = SST_BEGIN(argc,argv);
    diago.perspective = 1;			// need to run in perspective
    diago.adjust = 1;				// and with adjust on
 
    if(diago.bigAssTextures)
      t = buildTriangle(2048, 2048);
    else
      t = buildTriangle(256, 256);

    SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT |
		SST_ENTEXTUREMAP |
		(diago.adjust?SST_PARMADJUST:0));
    t->next = NULL;

    if(diago.bigAssTextures)
      logLargestPossibleLOD = 11;
    else
      logLargestPossibleLOD = 8;


  while (DIAG_STARTPASS()) {			// for each pass
    // now create and download mipmaps and change tLOD register
    format = texRandomFormat(diago.tex8==0,diago.tex32,diago.ncc);
    minLod = iRandom(2);
    
    //Tiled big textures can only be up to 1024x1024
    if(diago.bigAssTextures && diago.ytiled)
      minLod += 1;
    
    t->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) |
			format | SST_TC_REPLACE | SST_TCA_REPLACE;
    texRandomTextureMap(sst,diago.trex,1,logLargestPossibleLOD-minLod,
    			logLargestPossibleLOD-minLod,t->tex);

    for (n=0; n<50; n++) {			// do 50 tests
	// set random fbzMode bits
	if ((diago.rgb == 16 || texFormatHasAlpha(t->tex->tMode)) && (diago.rgb != 32))
	    SET(sst->fbzMode, SST_RGBWRMASK | SST_ZAWRMASK | SST_ENALPHABUFFER |
		drawbufferRandom());
	else
	    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());
	texRandomZAbuffer(sst,format);

	// set random tLOD bits
	if (diago.lodbias) {			// random bias (signed)
	    t->tex->tLOD &= ~SST_LODBIAS;
	    t->tex->tLOD |= iRandom(SST_LODBIAS) & SST_LODBIAS;
	}
	t->tex->lodmin = iRandom(iRandom(logLargestPossibleLOD<<SST_LOD_FRACBITS));
	if (t->tex->lodmin < (minLod<<SST_LOD_FRACBITS))
	    t->tex->lodmin= minLod<<SST_LOD_FRACBITS;
	t->tex->lodmax = rRandom(t->tex->lodmin,logLargestPossibleLOD<<SST_LOD_FRACBITS);
	gdbg_info(2,"LOD bias=%x.%x  min,max = %x.%x to %x.%x\n",
(t->tex->tLOD & SST_LODBIAS)>>(SST_LODBIAS_SHIFT+SST_LOD_FRACBITS),
((t->tex->tLOD & SST_LODBIAS)>>(SST_LODBIAS_SHIFT+SST_LOD_FRACBITS-4))&0xC,
			t->tex->lodmin>>SST_LOD_FRACBITS,
			(t->tex->lodmin<<(4-SST_LOD_FRACBITS)) & 0xF,
			t->tex->lodmax>>SST_LOD_FRACBITS,
			(t->tex->lodmax<<(4-SST_LOD_FRACBITS)) & 0xF);
	t->tex->tLOD &= ~(SST_LODMIN | SST_LODMAX);	// clear out bits
	t->tex->tLOD |= (t->tex->lodmin<<SST_LODMIN_SHIFT) |
			(t->tex->lodmax<<SST_LODMAX_SHIFT);
	if(diago.bigAssTextures)
	    t->tex->tLOD |= SST_TBIG;

	SET(sst->tLOD,t->tex->tLOD);		// set all TREX chips

	// set random textureMode bits
	if (diago.loddither) {
	    if (iRandom(1))			// randomly toggle the bit
		t->tex->tMode ^= SST_TLODDITHER;
	}
	if (diago.bilinear) {			// if bilinear enabled
	    t->tex->tMode &= ~(SST_TMINFILTER | SST_TMAGFILTER);
	    if (iRandom(1)) t->tex->tMode |= SST_TMINFILTER;
	    if (iRandom(1)) t->tex->tMode |= SST_TMAGFILTER;
	}
	gdbg_info(2,"min,mag filters = %d,%d  LODdither=%d\n",
			(t->tex->tMode & SST_TMINFILTER)!=0,
			(t->tex->tMode & SST_TMAGFILTER)!=0,
			(t->tex->tMode & SST_TLODDITHER)!=0);
	SET(SST_TREX(sst,t->tex->trex)->textureMode,t->tex->tMode);

    again:
	randomTriangle(t,diago.tsize,1);	// pick random triangle
	randomSt1418Triangle(t);		// with random texcoords
	if (diago.perspective)			// if testing perspective
	    randomW230Triangle(t);		// generate random W

	areaTriangle(t);			// compute the area (before setup)
	if (!setupStwTriangle(t))		// setup STW slopes
	    goto again;				// reject bad slopes
	sortTriangle(t);			// sort it
	printStwTriangle(4,t);
	printStwTriangleSlopes(5,t);

	// NOTE: sub-pixel parameter adjustment is OFF so we may end up with
	//	 color overflows/underflows etc
	drawStwTriangle(sst,t);

	sst_idle(sst);				// wait for the command to complete
	checkTriangle(t,1,1,diago.adjust, insideStTriangle,0,0,0);
	eraseTriangle(sst,t,1,			// erase the triangle
			texFormatHasAlpha(format),0);
    }
  }
  DIAG_PASS(0);
}

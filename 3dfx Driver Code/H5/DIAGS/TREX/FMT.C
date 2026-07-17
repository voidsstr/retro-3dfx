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
** $Date: 10/11/00 8:19:06 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

void
main (int argc, char **argv)
{
    int n, format,slog,tlog;
    Triangle *t;
    SstRegs *sst;
    FxI32 largestPossibleLOD;

    sst = SST_BEGIN(argc,argv);

    if(diago.bigAssTextures)
      {
	t = buildTriangle(2048, 2048);
      }
    else
      {
	t = buildTriangle(256, 256);
      }

    t->next = NULL;
    SET(sst->c1, 0x23FECDBA);			// hack for -v option
    SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT |
		SST_ENTEXTUREMAP |
		(diago.adjust?SST_PARMADJUST:0));
  
    if(diago.bigAssTextures)
      largestPossibleLOD=11;
    else
      largestPossibleLOD=8;

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<50; n++) {			// do 50 tests
	// GMT: big ugly hack, use -v option to test these
	if (diago.vsync) {	// test input from textures
	    SET(sst->combineMode,0);
	    switch (iRandom(5)) {
		case 0:		// mselect=4
		  diago.vsync = SST_CC_MATMU | SST_CCA_MATMU;
		  SET(sst->fbzColorPath, SST_RGBSEL_C1 | SST_ASEL_C1 |
			SST_ENTEXTUREMAP | diago.vsync | 
			SST_CC_REVERSE_BLEND | SST_CCA_REVERSE_BLEND |
			(diago.adjust?SST_PARMADJUST:0));
		  break;
		case 1:		// mselect=5
		  diago.vsync = SST_CC_MRGBTMU;
		  SET(sst->fbzColorPath, SST_RGBSEL_C1 | SST_ASEL_TREXOUT |
			SST_ENTEXTUREMAP | diago.vsync | SST_CC_REVERSE_BLEND |
			(diago.adjust?SST_PARMADJUST:0));
		  break;
		case 2:		// add local RGB
		  diago.vsync = SST_CC_ADD_CLOCAL | SST_CC_ADD_ALOCAL;
		  SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT |
			SST_ENTEXTUREMAP | diago.vsync | 
			(diago.adjust?SST_PARMADJUST:0));
		  break;
		case 3:		// add local A
		  diago.vsync = SST_CCA_ADD_CLOCAL | SST_CCA_ADD_ALOCAL;
		  SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT |
			SST_ENTEXTUREMAP | diago.vsync | 
			(diago.adjust?SST_PARMADJUST:0));
		  break;
		case 4:		// local_sel=2
		  diago.vsync = -1;
		  SET(sst->fbzColorPath, SST_CC_ADD_CLOCAL | SST_CC_MZERO |
		  	SST_ENTEXTUREMAP | (diago.adjust?SST_PARMADJUST:0));
		  SET_FBI(sst->combineMode, SST_CM_USE_COMBINE_MODE | 
		  		SST_CM_CCA_OTHERSELECT_TA |
		  		SST_CM_CC_LOCALSELECT_TRGB);
		  break;
		case 5:		// local_sel=5
		  diago.vsync = -2;
		  SET(sst->fbzColorPath, SST_CC_ADD_CLOCAL | SST_CC_MZERO |
		  	SST_ENTEXTUREMAP | (diago.adjust?SST_PARMADJUST:0));
		  SET_FBI(sst->combineMode, SST_CM_USE_COMBINE_MODE | 
		  		SST_CM_CCA_OTHERSELECT_TA |
		  		SST_CM_CC_LOCALSELECT_TA);
		  break;
	    }
	}
    
	if (diago.rgb == 16)
	    SET(sst->fbzMode, SST_RGBWRMASK | SST_ZAWRMASK | SST_ENALPHABUFFER |
		drawbufferRandom());
	else
	    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());

	// random 8-bit/16-bit uncompressed, NCC option forces 8-bit
	format = texRandomFormat(diago.tex8==0,diago.tex32,diago.ncc);
	slog = tlog = iRandom(largestPossibleLOD);		// random LOD [0,largestPossibleLOD]

	//Make sure we don't try to download a tiled texture bigger than 1024x1024
	if(diago.ytiled)
	  {
	    if(slog > 10)
	      slog = 10;
	    
	    if(tlog > 10)
	      tlog = 10;
	  }

	if (diago.rectangular) {		// random rectangular
	    if (iRandom(1))			// within 8:1 aspect ratio
		tlog = slog - iRandom(3);
	    else
		slog = tlog - iRandom(3);

	    if(tlog < 0)
	      tlog = 0;
	    if(slog < 0)
	      slog = 0;

	    GDBG_INFO(0, "slog = %d   tlog = %d\n", slog, tlog);
	}	

	if (diago.bilinear) {			// if bilinear enabled
	    if (diago.bilinear > 0 || iRandom(1))
		format |= SST_TMINFILTER | SST_TMAGFILTER;
	    if (iRandom(1))
		t->tex->tDetail ^= SST_TFILTER_SEPARATE;
	    t->tex->tDetail ^= iRandom(SST_TMINFILTER_RGB | SST_TMINFILTER_A |
				SST_TMAGFILTER_RGB | SST_TMAGFILTER_A);
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
	// TREX0 gets random nxn texture with random 8-bit format
	if (diago.perspective)
	    format |= SST_TPERSP_ST;
	t->tex->tMode = format | SST_TC_REPLACE | SST_TCA_REPLACE;
	texRandomTextureMap(sst,diago.trex, 0,slog,tlog,t->tex);
	if (diago.bilinear) {			// if bilinear enabled
	    SET(SST_TREX(sst,t->tex->trex)->tDetail,t->tex->tDetail);
	}
	if (diago.flip) {
	    if (iRandom(1)) t->tex->tLOD ^= SST_TMIRRORS;
	    if (iRandom(1)) t->tex->tLOD ^= SST_TMIRRORT;
	    gdbg_info(3,"mirror=%s %s \n",
			t->tex->tLOD & SST_TMIRRORS ? "S" : " ",
			t->tex->tLOD & SST_TMIRRORT ? "T" : " ");
	    SET(SST_TREX(sst,t->tex->trex)->tLOD,t->tex->tLOD);
	}
	texRandomZAbuffer(sst,format);
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
    DIAG_PASS(0);
}

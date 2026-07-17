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
** $Date: 10/11/00 8:18:59 PM$
*/

#include "allocate.h"
#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

void
main (int argc, char **argv)
{
    int m,n, format,minLod,minLod2;
    int detailMax,detailBias,detailScale;
    FxU32 largestPossibleLOD;
    int ipass=0;
    Triangle *t, *t2;
    SstRegs *sst;

    // Don't let this diag run triple buffered or it will run out of memory 
    // when run with bigAssTextures and 32bpp rendering at 2048x1536
    for(m=0; m<argc; m++)
      {
	if(!strncmp(argv[m], "-3", 2))
	  {
	    GDBG_INFO(0, "\n");
	    GDBG_INFO(0, "\n");
	    GDBG_INFO(0, "\n");
	    GDBG_INFO(0, "Warning: Stripping this arg \"%s\"!\n", argv[m]);
	    GDBG_INFO(0, "\n");
	    GDBG_INFO(0, "\n");
	    GDBG_INFO(0, "\n");
	    for(n=m; n<argc-1; n++)
	      argv[n] = argv[n+1];
	    
	    argc--;
	    m--;
	  }	
      }

    inhibitTwoPixelsPerClock();   //This diag multitextures and can't run in 2ppc

    sst = SST_BEGIN(argc,argv);

    if(diago.bigAssTextures)
      {
	t = buildTriangle(2048, 2048);
	t2 = buildTriangle(2048, 2048);
      }
    else
      {
	t = buildTriangle(256, 256);
	t2 = buildTriangle(256, 256);
      }


    if (diago.trex == 0) {
	gdbg_error("detail","must run with at least 2 TREX chips, use -z-1\n");
	DIAG_FAIL();
    }
    diago.perspective = 1;			// need to run in perspective
    diago.adjust = 1;				// and with adjust on
 
    if(diago.bigAssTextures)
      largestPossibleLOD = 11;
    else 
      largestPossibleLOD = 8;


    SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT |
		SST_ENTEXTUREMAP |
		(diago.adjust?SST_PARMADJUST:0));
    t->next = t2;				// link the 2 triangles

  while (DIAG_STARTPASS()) {			// for each pass

    // backdoor texture downloads may overwrite a memory location still 
    // being used for rendering, so we have to idle the chip between passes
    ipass++;
    if ( ipass > 1 && (diago.halInfo->hsim & HSIM_TREX_BACKDOOR_TEXWRITES) )
      sst_idle_really(sst);

  for (m=0; m<5; m++) {				// 5 different textures
    // now create and download mipmaps and change tLOD register
    format = texRandomFormat(diago.tex8==0,diago.tex32,diago.ncc);

    //Make sure that we don't try to download a tiled texture
    //bigger than 1024x1024
    if(diago.ytiled != 0 && diago.bigAssTextures)
      {
	minLod = 1 + iRandom(1);
	minLod2 = 1 + iRandom(1);
      }
    else
      {
	minLod = iRandom(2);
	minLod2 = iRandom(2);
      }



    t2->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) |
			format | SST_TC_REPLACE | SST_TCA_REPLACE;
    t->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) |
			format | SST_TC_BLEND_LOD | SST_TCA_BLEND_LOD;

    //Make sure that both textures will be in texture memory
    unallocateAll();  //This will only unallocate the textures

    texRandomTextureMapEx(sst,1,1,largestPossibleLOD-minLod2,largestPossibleLOD-minLod2,t2->tex,0);
    texRandomTextureMapEx(sst,0,1,largestPossibleLOD-minLod,largestPossibleLOD-minLod,t->tex,0);

    for (n=0; n<10; n++) {			// do 10 tests
	// set random fbzMode bits
	if (diago.rgb == 16)
	    SET(sst->fbzMode, SST_RGBWRMASK | SST_ZAWRMASK | SST_ENALPHABUFFER |
		drawbufferRandom());
	else
	    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());

	// set random tLOD bits
	if (diago.lodbias) {			// random bias (signed)
	    t->tex->tLOD &= ~SST_LODBIAS;
	    t->tex->tLOD |= iRandom(SST_LODBIAS) & SST_LODBIAS;
	    t2->tex->tLOD &= ~SST_LODBIAS;
	    t2->tex->tLOD |= iRandom(SST_LODBIAS) & SST_LODBIAS;
	}
	t->tex->lodmin = iRandom(iRandom(8<<SST_LOD_FRACBITS));
	t2->tex->lodmin = iRandom(iRandom(8<<SST_LOD_FRACBITS));
	if (t->tex->lodmin < (minLod<<SST_LOD_FRACBITS))
	  t->tex->lodmin= minLod<<SST_LOD_FRACBITS;
	if (t2->tex->lodmin < (minLod2<<SST_LOD_FRACBITS))
	  t2->tex->lodmin= minLod2<<SST_LOD_FRACBITS;
	t->tex->lodmax = rRandom(t->tex->lodmin,8<<SST_LOD_FRACBITS);
	t2->tex->lodmax = rRandom(t2->tex->lodmin,8<<SST_LOD_FRACBITS);
	gdbg_info(2,"LOD.1 bias=%x.%x  min,max = %x.%x to %x.%x\n",
		  (t2->tex->tLOD & SST_LODBIAS)>>(SST_LODBIAS_SHIFT+SST_LOD_FRACBITS),
		  ((t2->tex->tLOD & SST_LODBIAS)>>(SST_LODBIAS_SHIFT+SST_LOD_FRACBITS-4))&0xC,
		  t2->tex->lodmin>>SST_LOD_FRACBITS,
		  (t2->tex->lodmin<<(4-SST_LOD_FRACBITS)) & 0xF,
		  t2->tex->lodmax>>SST_LOD_FRACBITS,
		  (t2->tex->lodmax<<(4-SST_LOD_FRACBITS)) & 0xF);
	gdbg_info(2,"LOD.0 bias=%x.%x  min,max = %x.%x to %x.%x\n",
		  (t->tex->tLOD & SST_LODBIAS)>>(SST_LODBIAS_SHIFT+SST_LOD_FRACBITS),
		  ((t->tex->tLOD & SST_LODBIAS)>>(SST_LODBIAS_SHIFT+SST_LOD_FRACBITS-4))&0xC,
		  t->tex->lodmin>>SST_LOD_FRACBITS,
		  (t->tex->lodmin<<(4-SST_LOD_FRACBITS)) & 0xF,
		  t->tex->lodmax>>SST_LOD_FRACBITS,
		  (t->tex->lodmax<<(4-SST_LOD_FRACBITS)) & 0xF);
	t->tex->tLOD &= ~(SST_LODMIN | SST_LODMAX);	// clear out bits
	t->tex->tLOD |= (t->tex->lodmin<<SST_LODMIN_SHIFT) |
	  (t->tex->lodmax<<SST_LODMAX_SHIFT);
	t2->tex->tLOD &= ~(SST_LODMIN | SST_LODMAX);	// clear out bits
	t2->tex->tLOD |= (t2->tex->lodmin<<SST_LODMIN_SHIFT) |
	  (t2->tex->lodmax<<SST_LODMAX_SHIFT);

	if(diago.bigAssTextures)
	  {
	    t->tex->tLOD |= SST_TBIG;
	    t2->tex->tLOD |= SST_TBIG;
	  }

	SET_0(sst->tLOD,t->tex->tLOD);
	SET_1(sst->tLOD,t2->tex->tLOD);

	// set random textureMode bits
	if (diago.loddither) {
	    if (iRandom(1))			// randomly toggle the bit
		t->tex->tMode ^= SST_TLODDITHER;
	}
	// if bilinear enabled, random filters for each texture
	if (diago.bilinear) {
	    t->tex->tMode &= ~(SST_TMINFILTER | SST_TMAGFILTER);
	    t2->tex->tMode &= ~(SST_TMINFILTER | SST_TMAGFILTER);
	    if (diago.bilinear>0 || iRandom(1))
		t->tex->tMode |= SST_TMINFILTER;
	    if (diago.bilinear>0 || iRandom(1))
		t->tex->tMode |= SST_TMAGFILTER;
	    if (diago.bilinear>0 || iRandom(1))
		t2->tex->tMode |= SST_TMINFILTER;
	    if (diago.bilinear>0 || iRandom(1))
		t2->tex->tMode |= SST_TMAGFILTER;
	}
	gdbg_info(3,"min,mag filters = t0:%d,%d    t1:%d,%d  LODdither=%d\n",
			(t->tex->tMode & SST_TMINFILTER)!=0,
			(t->tex->tMode & SST_TMAGFILTER)!=0,
			(t2->tex->tMode & SST_TMINFILTER)!=0,
			(t2->tex->tMode & SST_TMAGFILTER)!=0,
			(t->tex->tMode & SST_TLODDITHER)!=0);
	if (iRandom(1))
	    t->tex->tMode |= SST_TC_REVERSE_BLEND | SST_TCA_REVERSE_BLEND;
	else
	    t->tex->tMode &= ~(SST_TC_REVERSE_BLEND | SST_TCA_REVERSE_BLEND);
	SET_0(sst->textureMode,t->tex->tMode);
	SET_1(sst->textureMode,t2->tex->tMode);


	// set detail blending controls in Trex0
	detailMax = iRandom(0xFF);
	detailBias = iRandom(iRandom(iRandom(0x1F)));
	if (iRandom(1)) detailBias = -detailBias;
	detailBias &= 0x3F;
	detailScale = iRandom(iRandom(0x7));
	t->tex->tDetail = (detailScale<<SST_DETAIL_SCALE_SHIFT) |
			  (detailBias<<SST_DETAIL_BIAS_SHIFT) |
			  (detailMax<<SST_DETAIL_MAX_SHIFT);
	SET(SST_TREX(sst,0)->tDetail,t->tex->tDetail);
	detailBias = (detailBias<<(32-6))>>(32-6);
	gdbg_info(2,"detail Max=%d  Bias=%d  Scale=%d (%d)\n",
			detailMax,detailBias,detailScale,1<<detailScale);

    again:
	randomTriangle(t,diago.tsize,1);	// pick random triangle
	randomSt1418Triangle(t);		// with random texcoords
	randomW230Triangle(t);			// generate random W
	t2->vA = t->vA;				// copy triangle vertex data
	t2->vB = t->vB;
	t2->vC = t->vC;
	randomSt1418Triangle(t2);		// generate unique s,t,w
	randomW230Triangle(t2);		// generate random W

	areaTriangle(t);			// compute the area (before setup)
	areaTriangle(t2);
	if (!setupStwTriangle(t))		// setup STW slopes
	    goto again;				// reject bad slopes
	if (!setupStwTriangle(t2))
	    goto again;
	sortTriangle(t);			// sort it
	sortTriangle(t2);
	printStwTriangle(4,t);
	printStwTriangleSlopes(5,t);
	drawStwTriangle2(sst,t,t2);		// draw triangle with 2 textures

	sst_idle(sst);				// wait for the command to complete
	checkTriangle(t,1,1,diago.adjust, insideStTriangle,0,0,0);
	eraseTriangle(sst,t,1,			// erase the triangle
			texFormatHasAlpha(format),0);
    }
  }

  }
  DIAG_PASS(0);
}

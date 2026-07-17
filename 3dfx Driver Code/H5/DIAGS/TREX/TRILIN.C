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
** $Date: 10/11/00 8:19:25 PM$
*/

#include "allocate.h"

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

static int 
randomOddEven(Triangle *t, Triangle *t2)
{
    int oddTrex = iRandom(1);
    if (oddTrex == 0) {				// set the ODD bit
	t->tex->tLOD |= SST_LOD_ODD;
	t2->tex->tLOD &= ~SST_LOD_ODD;
    }
    else {
	t->tex->tLOD &= ~SST_LOD_ODD;
	t2->tex->tLOD |= SST_LOD_ODD;
    }
    gdbg_info(2,"Odd trex = %d , even=%d\n",oddTrex,1-oddTrex);
    return oddTrex;
}

void
main (int argc, char **argv)
{
    int m,n, format,slog,tlog, oddTrex, minLod;
    int ipass=0;
    FxU32 largestPossibleLOD;
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
    if (diago.trex == 0) {
	gdbg_error("trilin","must run with at least 2 TREX chips, use -z-1\n");
	DIAG_FAIL();
    }
    diago.perspective = 1;			// need to run in perspective
    diago.adjust = 1;				// and with adjust on

    if(diago.bigAssTextures)
      largestPossibleLOD = 11;
    else
      largestPossibleLOD = 8;

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

    t->next = t2;

    //Force 1 pixel per clock
    SET(sst->combineMode, 0);
    SET_0(sst->combineMode, 0);
    SET_1(sst->combineMode, 0);

 
    SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT |
		SST_ENTEXTUREMAP |
		(diago.adjust?SST_PARMADJUST:0));
    if (diago.tsplit) {
	t2->tex->tLOD |= SST_LOD_TSPLIT;
	t->tex->tLOD |= SST_LOD_TSPLIT;
    }

  while (DIAG_STARTPASS()) {			// for each pass

    // backdoor texture downloads may overwrite a memory location still 
    // being used for rendering, so we have to idle the chip between passes
    ipass++;
    if ( ipass > 1 && (diago.halInfo->hsim & HSIM_TREX_BACKDOOR_TEXWRITES) )
      sst_idle_really(sst);
    
  for (m=0; m<7; m++) {				// 7 different textures
    if (t->tex->tLOD & SST_LOD_TSPLIT)		// each random Odd/Even
	oddTrex = randomOddEven(t,t2);

    // now create and download mipmaps and change tLOD register
    // NOTE: we load entire mipmaps (both even and odd) into each TREX chip
    // unless if TSPLIT is set in tLOD
    format = texRandomFormat(diago.tex8==0,diago.tex32,diago.ncc);


    //Make sure that we don't try to download a tiled texture
    //bigger than 1024x1024
    if(diago.ytiled != 0 && diago.bigAssTextures)
      minLod = iRandom(1) + 1;
    else
      minLod = iRandom(2);
      

    slog = largestPossibleLOD - minLod;
    tlog = largestPossibleLOD - minLod;
	if (diago.rectangular) {		// random rectangular
	    if (iRandom(1))			// within 8:1 aspect ratio
		tlog = slog - iRandom(3);
	    else
		slog = tlog - iRandom(3);
	    
	    if(slog < 0)
	      slog = 0;
	    if(tlog < 0)
	      tlog = 0;
	}

    t2->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) | SST_TRILINEAR |
			format | SST_TC_REPLACE | SST_TCA_REPLACE;
    t->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) | SST_TRILINEAR |
			format | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC;


    //Make sure that both textures will be in texture memory
    unallocateAll();  //This will only unallocate the textures

    texRandomTextureMap(sst,1,1,slog,tlog,t2->tex);
    texRandomTextureMap(sst,0,1,slog,tlog,t->tex);


    for (n=0; n<5; n++) {			// 5 tests each
	// set random fbzMode bits
	if (diago.rgb == 16)
	    SET(sst->fbzMode, SST_RGBWRMASK | SST_ZAWRMASK | SST_ENALPHABUFFER |
		drawbufferRandom());
	else
	    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());

	// set random tLOD bits
	if (!(t->tex->tLOD & SST_LOD_TSPLIT))	// if not TSPLIT change odd/even
	    oddTrex = randomOddEven(t,t2);	// every triangle
	if (diago.zeroLodFrac) {
	    t->tex->tLOD ^= SST_LOD_ZEROFRAC;
	    t2->tex->tLOD ^= SST_LOD_ZEROFRAC;
	}
	if (diago.lodbias) {			// random bias (signed)
	    int temp;
	    t->tex->tLOD &= ~SST_LODBIAS;
	    t2->tex->tLOD &= ~SST_LODBIAS;
	    temp = iRandom(SST_LODBIAS) & SST_LODBIAS;
	    t->tex->tLOD |= temp;			// use same bias for both chips
	    t2->tex->tLOD |= temp;
	}
	t->tex->lodmin = iRandom(iRandom(8<<SST_LOD_FRACBITS));
	if (t->tex->lodmin < (minLod<<SST_LOD_FRACBITS))
	    t->tex->lodmin= minLod<<SST_LOD_FRACBITS;
	t2->tex->lodmin = t->tex->lodmin;
	t->tex->lodmax = t2->tex->lodmax = rRandom(t->tex->lodmin,8<<SST_LOD_FRACBITS);
	gdbg_info(2,"LOD bias=%x.%x  min,max = %x.%x to %x.%x\n",
			(t->tex->tLOD & SST_LODBIAS)>>(SST_LODBIAS_SHIFT+SST_LOD_FRACBITS),
			((t->tex->tLOD & SST_LODBIAS)>>(SST_LODBIAS_SHIFT+SST_LOD_FRACBITS-4))&0xC,
			t->tex->lodmin>>SST_LOD_FRACBITS,
			t->tex->lodmin & SST_MASK(SST_LOD_FRACBITS),
			t->tex->lodmax>>SST_LOD_FRACBITS,
			t->tex->lodmax & SST_MASK(SST_LOD_FRACBITS));
	t->tex->tLOD &= ~(SST_LODMIN | SST_LODMAX);	// clear out bits
	t2->tex->tLOD &= ~(SST_LODMIN | SST_LODMAX);	// clear out bits
	t->tex->tLOD |= (t->tex->lodmin<<SST_LODMIN_SHIFT) | (t->tex->lodmax<<SST_LODMAX_SHIFT);
	t2->tex->tLOD |= (t->tex->lodmin<<SST_LODMIN_SHIFT) | (t->tex->lodmax<<SST_LODMAX_SHIFT);
	SET_0(sst->tLOD,t->tex->tLOD);
	SET_1(sst->tLOD,t2->tex->tLOD);

	// set random textureMode bits
	if (diago.loddither) {
	    if (iRandom(1)) {			// randomly toggle the bit
		t->tex->tMode ^= SST_TLODDITHER;
		t2->tex->tMode ^= SST_TLODDITHER;
	    }
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
	gdbg_info(3,"min,mag filters = t0:%d,%d    t1:%d,%d    LODdither=%d ZeroFrac=%d\n",
			(t->tex->tMode & SST_TMINFILTER)!=0,
			(t->tex->tMode & SST_TMAGFILTER)!=0,
			(t2->tex->tMode & SST_TMINFILTER)!=0,
			(t2->tex->tMode & SST_TMAGFILTER)!=0,
			(t->tex->tMode & SST_TLODDITHER)!=0,
			(t->tex->tLOD & SST_LOD_ZEROFRAC)!=0);
	if (diago.clamp) {
	    if (iRandom(1)) t->tex->tMode ^= SST_TCLAMPS;
	    if (iRandom(1)) t->tex->tMode ^= SST_TCLAMPT;
	    if (iRandom(1)) t->tex->tMode ^= SST_TCLAMPW;
	    if (iRandom(1)) t2->tex->tMode ^= SST_TCLAMPS;
	    if (iRandom(1)) t2->tex->tMode ^= SST_TCLAMPT;
	    if (iRandom(1)) t2->tex->tMode ^= SST_TCLAMPW;
	    gdbg_info(3,"clamping = t0:%c,%c,%c    t1:%c,%c,%c\n",
			t->tex->tMode & SST_TCLAMPS ? 'S':'-',
			t->tex->tMode & SST_TCLAMPT ? 'T':'-',
			t->tex->tMode & SST_TCLAMPW ? 'W':'-',
			t2->tex->tMode & SST_TCLAMPS ? 'S':'-',
			t2->tex->tMode & SST_TCLAMPT ? 'T':'-',
			t2->tex->tMode & SST_TCLAMPW ? 'W':'-');
	}
	if (oddTrex == 0)
	    t->tex->tMode &= ~(SST_TC_REVERSE_BLEND | SST_TCA_REVERSE_BLEND);
	else
	    t->tex->tMode |= SST_TC_REVERSE_BLEND | SST_TCA_REVERSE_BLEND;
	SET_0(sst->textureMode,t->tex->tMode);
	SET_1(sst->textureMode,t2->tex->tMode);

    again:
	randomTriangle(t,diago.tsize,1);	// pick random triangle
	randomSt1418Triangle(t);		// with random texcoords
	randomW230Triangle(t);			// generate random W
	t2->vA = t->vA;				// copy triangle vertex data
	t2->vB = t->vB;
	t2->vC = t->vC;

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

	drawStwTriangle(sst,t);

	sst_idle(sst);				// wait for the command to complete
	checkTriangle(t,1,1,diago.adjust, insideStTriangle,0,0,0);
	eraseTriangle(sst,t,1,			// erase the triangle
			texFormatHasAlpha(format),0);
    }
  }

  }
  DIAG_PASS(0);
}





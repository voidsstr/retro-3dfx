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
** $Date: 10/11/00 8:19:22 PM$
*/

#include "allocate.h"
#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"


int combTest;	// test combineMode
static char blanks[] = "";
static char buf1[32],buf2[32];
static char *mSelectTrex_str[] = {  "0","Loc","AOth","ALoc",
				"LOD","LodF","0","Msel_7"};
static char *tComposite_str(char *buf, char *names_mSelect[], unsigned long tMode)
{
    tMode <<= SST_TCOMBINE_SHIFT;
    sprintf(buf,"%s(%s%s)*%s%s%s",
		tMode & SST_TC_INVERT_OUTPUT ? "!" : blanks,
		tMode & SST_TC_ZERO_OTHER ? "0" : "Other",
		tMode & SST_TC_SUB_CLOCAL ? "-Local" : blanks,
		tMode & SST_TC_REVERSE_BLEND ? blanks : "~",
		names_mSelect[(tMode & SST_TC_MSELECT)>>SST_TC_MSELECT_SHIFT],
		tMode & SST_TC_ADD_CLOCAL ? "+Loc" :
			tMode & SST_TC_ADD_ALOCAL ? "+ALoc" : blanks);
    return buf;
}

int tcuRandom(void)
{
    int mode;
again:
    // check for illegal combos
    mode = iRandom(SST_TCOMBINE) & SST_TCOMBINE;
    if (diago.trex == 0) {
	if ((mode & SST_TC_MSELECT) == SST_TC_MAOTHER) goto again;
	if (!combTest) mode |= SST_TC_ZERO_OTHER;
    }
    return mode;
}

int detailRandom(void)
{
    int detailMax,detailBias,detailScale;

    detailMax = iRandom(0xFF);
    detailBias = iRandom(iRandom(iRandom(0x1F)));
    if (iRandom(1)) detailBias = -detailBias;
    detailBias &= 0x3F;
    detailScale = iRandom(iRandom(0x7));
    gdbg_info(3,"detail Max=%d  Bias=%d  Scale=%d (%d)\n",
		detailMax,
		(detailBias<<(32-6))>>(32-6),
		detailScale,1<<detailScale);
    return (detailScale<<SST_DETAIL_SCALE_SHIFT) |
		(detailBias<<SST_DETAIL_BIAS_SHIFT) |
		(detailMax<<SST_DETAIL_MAX_SHIFT);
}

void
main (int argc, char **argv)
{
    int trexA, trexB, overlap_flag;
    int m,n,c0,c1,c0s,c0t,c1s,c1t;
    Triangle *t,*t2;
    SstRegs *sst;

    inhibitTwoPixelsPerClock();   //This diag multitextures and can't run in 2ppc

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

    sst = SST_BEGIN(argc,argv);
    diago.perspective = 1;			// need to run in perspective
    diago.adjust = 1;				// and with adjust on
    combTest = 1;  //diago.vsync;		// copy vsync flag and turn off
    diago.vsync = 0;				// so as to not confuse hacks in stwtri.c


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

    if (diago.trex != 0)
	t->next = t2;
    overlap_flag = diago.trex != 0;
    if (overlap_flag) diago.multiTexBaseAddr = 0;

    /*
    if(diago.tex32)
      {
	GDBG_ERROR("main", "Can't run tcu with 32 bit per texel textures!\n");
	DIAG_FAIL();
      }
    */
 
    SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT |
		SST_ENTEXTUREMAP |
		(diago.adjust?SST_PARMADJUST:0));

  while (DIAG_STARTPASS())			// for each pass
  for (m=0; m<5; m++) {				// test 5 textures
    // now create and download texture maps
    // regardless of parameters we randomly select 8/16 bit or NCC
    t->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) |
                        texRandomFormat(iRandom(1),diago.tex32,iRandom(1));  
    t2->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) |
			texRandomFormat(iRandom(1),diago.tex32,iRandom(1));
    c0s = c0t = rRandom(2,5);		// random size texture maps
    c1s = c1t = rRandom(2,5);
    if (diago.rectangular) {		// random rectangular
      if (iRandom(1))			// within 8:1 aspect ratio
	c0t = c0s - iRandom(3);
      else
	c0s = c0t - iRandom(3);
      if (iRandom(1))			// within 8:1 aspect ratio
	c1t = c1s - iRandom(3);
      else
	c1s = c1t - iRandom(3);
    }	// NOTE: slog or tlog might actually be negative!
    
    trexA = 0;
    trexB = 1;
    if (diago.trex < -1)	{	// optionally choose random trex
      trexA = iRandom(-1-diago.trex);
      trexB = rRandom(trexA+1,-diago.trex);
    }

    //Make sure that both textures will be in texture memory
    unallocateAll();  //This will only unallocate the textures

    texRandomTextureMap(sst,trexA,0,c0s,c0t,t->tex);
    if (diago.trex != 0)
      texRandomTextureMap(sst,trexB,0,c1s,c1t,t2->tex);

    // if using combine mode, set random constant colors
    if (combTest) {
	SET(SST_TREX(sst,trexA)->chromaKey,t->tex->tChromakey=colRandom32());
	SET(SST_TREX(sst,trexA)->chromaRange,t->tex->tChromarange=colRandom32());
	if (diago.trex != 0) {
	    SET(SST_TREX(sst,trexB)->chromaKey,t2->tex->tChromakey=colRandom32());
	    SET(SST_TREX(sst,trexB)->chromaRange,t2->tex->tChromarange=colRandom32());
	}
    }

    for (n=0; n<6; n++) {			// 6 triangles each
	t->tex->tMode &= ~(SST_TCOMBINE | SST_TACOMBINE);
	t2->tex->tMode &= ~(SST_TCOMBINE | SST_TACOMBINE);

	// set random fbzMode bits
	if (diago.rgb == 16)
	    SET(sst->fbzMode, SST_RGBWRMASK | SST_ZAWRMASK | SST_ENALPHABUFFER |
		drawbufferRandom());
	else
	    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());

	// set random textureMode bits
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
	gdbg_info(3,"min,mag filters = t0:%d,%d    t1:%d,%d\n",
			(t->tex->tMode & SST_TMINFILTER)!=0,
			(t->tex->tMode & SST_TMAGFILTER)!=0,
			(t2->tex->tMode & SST_TMINFILTER)!=0,
			(t2->tex->tMode & SST_TMAGFILTER)!=0);
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

	// select random (but valid) RGB and Alpha combines, but not both PASS
    again_rgb:
	do {
	    c0 = tcuRandom();
	    c1 = tcuRandom();
	} while (c0==SST_TC_PASS && c1==SST_TC_PASS);
	if (!combTest) c1 |= SST_TC_ZERO_OTHER;		// avoid undefined inputs
	if ((c1 & SST_TC_MSELECT)==SST_TC_MAOTHER)
	    goto again_rgb;
	t->tex->tMode |= c0;
	t2->tex->tMode |= c1;
    again_alpha:
	do {
	    c0 = tcuRandom();
	    c1 = tcuRandom();
	} while (c0==SST_TC_PASS && c1==SST_TC_PASS);
	if (!combTest) c1 |= SST_TC_ZERO_OTHER;		// avoid undefined inputs
	if ((c1 & SST_TC_MSELECT)==SST_TC_MAOTHER)
	    goto again_alpha;
	c0 = (c0>>SST_TCOMBINE_SHIFT)<<SST_TACOMBINE_SHIFT;
	c1 = (c1>>SST_TCOMBINE_SHIFT)<<SST_TACOMBINE_SHIFT;
	t->tex->tMode |= c0;
	t2->tex->tMode |= c1;
	gdbg_info(2,"tcu%d: %x RGB:%s  A:%s\n",trexB,c1,
			tComposite_str(buf1,mSelectTrex_str,
				(t2->tex->tMode & SST_TCOMBINE)>>SST_TCOMBINE_SHIFT),
		tComposite_str(buf2,mSelectTrex_str,
				(t2->tex->tMode & SST_TACOMBINE)>>SST_TACOMBINE_SHIFT));
	gdbg_info(2,"tcu%d: %x RGB:%s  A:%s\n",trexA,c0,
			tComposite_str(buf1,mSelectTrex_str,
				(t->tex->tMode & SST_TCOMBINE)>>SST_TCOMBINE_SHIFT),
		tComposite_str(buf2,mSelectTrex_str,
				(t->tex->tMode & SST_TACOMBINE)>>SST_TACOMBINE_SHIFT));

	// all other TREX chips are defaulted to pass-thru mode
	SET(SST_TREX(sst,trexA)->textureMode,t->tex->tMode);
	if (diago.trex != 0)
	    SET(SST_TREX(sst,trexB)->textureMode,t2->tex->tMode);

	// set detail blending controls in TrexA,B
	t->tex->tDetail = detailRandom();
	t2->tex->tDetail = detailRandom();
	SET(SST_TREX(sst,trexA)->tDetail,t->tex->tDetail);
	if (diago.trex != 0)
	    SET(SST_TREX(sst,trexB)->tDetail,t2->tex->tDetail);

	if (combTest) {
	    FxU32 temp;

	    t->tex->combMode = 
	    t2->tex->combMode = SST_CM_DISABLE_CHROMA_SUBSTITUTION | SST_CM_USE_COMBINE_MODE;

	    // be careful not to use other texture if only 1 TMU
	    do temp = iRandom(7) << SST_CM_TC_OTHERSELECT_SHIFT;
	    while (diago.trex==0 && (
		(temp==SST_CM_TC_OTHERSELECT_OTHER_TRGB) ||
		(temp==SST_CM_TC_OTHERSELECT_OTHER_TA)));
	    t->tex->combMode |= temp;
	    do temp = iRandom(7) << SST_CM_TC_LOCALSELECT_SHIFT;
	    while (diago.trex==0 && (
		(temp==SST_CM_TC_LOCALSELECT_OTHER_TRGB) ||
		(temp==SST_CM_TC_LOCALSELECT_OTHER_TA)));
	    t->tex->combMode |= temp;
	    do temp = iRandom(3) << SST_CM_TCA_LOCALSELECT_SHIFT;
	    while (diago.trex==0 && temp==SST_CM_TCA_LOCALSELECT_OTHER_TA);
	    t->tex->combMode |= temp;
	    do temp = iRandom(3) << SST_CM_TCA_OTHERSELECT_SHIFT;
	    while (diago.trex==0 && temp==SST_CM_TCA_OTHERSELECT_OTHER_TA);
	    t->tex->combMode |= temp;

	    do temp = iRandom(7) << SST_CM_TC_MSELECT_7_SHIFT;
	    while (diago.trex==0 && temp==SST_CM_TC_MSELECT_7_OTHER_TRGB);
	    t->tex->combMode |= temp;
	    t->tex->combMode |= iRandom(3) << SST_CM_TC_INVERT_OTHER_SHIFT;
	    t->tex->combMode |= iRandom(3) << SST_CM_TC_INVERT_LOCAL_SHIFT;
	    if (iRandom(1)) t->tex->combMode |= SST_CM_TC_INVERT_ADD_LOCAL;
	    t->tex->combMode |= iRandom(2) << SST_CM_TC_OUTSHIFT_SHIFT;
	    t->tex->combMode |= iRandom(3) << SST_CM_TCA_INVERT_OTHER_SHIFT;
	    t->tex->combMode |= iRandom(3) << SST_CM_TCA_INVERT_LOCAL_SHIFT;
	    if (iRandom(1)) t->tex->combMode |= SST_CM_TCA_INVERT_ADD_LOCAL;
	    t->tex->combMode |= iRandom(2) << SST_CM_TCA_OUTSHIFT_SHIFT;

	    t2->tex->combMode |= iRandom(7) << SST_CM_TC_OTHERSELECT_SHIFT;
	    t2->tex->combMode |= iRandom(3) << SST_CM_TCA_OTHERSELECT_SHIFT;
	    // be careful not to use other texture in upstream TMU
	    do temp = iRandom(7) << SST_CM_TC_OTHERSELECT_SHIFT;
	    while ((temp==SST_CM_TC_OTHERSELECT_OTHER_TRGB) ||
		   (temp==SST_CM_TC_OTHERSELECT_OTHER_TA));
	    t2->tex->combMode |= temp;
	    do temp = iRandom(5) << SST_CM_TC_LOCALSELECT_SHIFT;
	    while ((temp==SST_CM_TC_LOCALSELECT_OTHER_TRGB) ||
		   (temp==SST_CM_TC_LOCALSELECT_OTHER_TA));
	    t2->tex->combMode |= temp;
	    do temp = iRandom(3) << SST_CM_TCA_LOCALSELECT_SHIFT;
	    while (temp==SST_CM_TCA_LOCALSELECT_OTHER_TA);
	    t2->tex->combMode |= temp;
	    do temp = iRandom(3) << SST_CM_TCA_OTHERSELECT_SHIFT;
	    while (temp==SST_CM_TCA_OTHERSELECT_OTHER_TA);
	    t2->tex->combMode |= temp;

	    do temp = iRandom(7) << SST_CM_TC_MSELECT_7_SHIFT;
	    while (temp==SST_CM_TC_MSELECT_7_OTHER_TRGB);
	    t2->tex->combMode |= temp;
	    t2->tex->combMode |= iRandom(7) << SST_CM_TC_MSELECT_7_SHIFT;
	    t2->tex->combMode |= iRandom(3) << SST_CM_TC_INVERT_OTHER_SHIFT;
	    t2->tex->combMode |= iRandom(3) << SST_CM_TC_INVERT_LOCAL_SHIFT;
	    if (iRandom(1)) t2->tex->combMode |= SST_CM_TC_INVERT_ADD_LOCAL;
	    t2->tex->combMode |= iRandom(2) << SST_CM_TC_OUTSHIFT_SHIFT;
	    t2->tex->combMode |= iRandom(3) << SST_CM_TCA_INVERT_OTHER_SHIFT;
	    t2->tex->combMode |= iRandom(3) << SST_CM_TCA_INVERT_LOCAL_SHIFT;
	    if (iRandom(1)) t2->tex->combMode |= SST_CM_TCA_INVERT_ADD_LOCAL;
	    t2->tex->combMode |= iRandom(2) << SST_CM_TCA_OUTSHIFT_SHIFT;

	    gdbg_info(2,"combineMode%d: %x\n",trexB,t2->tex->combMode);
	    gdbg_info(2,"combineMode%d: %x\n",trexA,t->tex->combMode);
	    SET(SST_TREX(sst,trexA)->combineMode, t->tex->combMode);
	    if (diago.trex != 0)
	      SET(SST_TREX(sst,trexB)->combineMode, t2->tex->combMode);
	}
	else {
	    t->tex->combMode = t2->tex->combMode = 0;
	}

    again:
	randomTriangle(t,diago.tsize,1);	// pick random triangle
	randomSt1418Triangle(t);		// with random texcoords
	randomW230Triangle(t);			// generate random W
	randomRgbaTriangle(t);			// random colors too
	t2->vA = t->vA;				// copy triangle vertex data
	t2->vB = t->vB;
	t2->vC = t->vC;
	randomSt1418Triangle(t2);		// generate unique s,t,w
	randomW230Triangle(t2);		// generate random W

	areaTriangle(t);			// compute the area (before setup)
	areaTriangle(t2);
	setupTriangle(t,1,1,0);		// setup RGBA slopes
	if (!setupStwTriangle(t))		// setup STW slopes
	    goto again;				// reject bad slopes
	if (!setupStwTriangle(t2))
	    goto again;
	sortTriangle(t);			// sort it
	sortTriangle(t2);
	printTriangle(2,t,1,1,0);
	printStwTriangle(4,t);
	printStwTriangleSlopes(5,t);

        SET(sst->r,t->vA.r);			// send RGBA also
        SET(sst->g,t->vA.g);
        SET(sst->b,t->vA.b);
        SET(sst->a,t->vA.a);
        SET(sst->drdx,t->drdx);
        SET(sst->dgdx,t->dgdx);
        SET(sst->dbdx,t->dbdx);
        SET(sst->dadx,t->dadx);
        SET(sst->drdy,t->drdy);
        SET(sst->dgdy,t->dgdy);
        SET(sst->dbdy,t->dbdy);
        SET(sst->dady,t->dady);

	if (diago.trex != 0)
	    drawStwTriangle2(sst,t,t2);
	else
	    drawStwTriangle(sst,t);

	sst_idle(sst);				// wait for the command to complete
	checkTriangle(t,1,1,diago.adjust, insideStTriangle,0,0,0);
	eraseTriangle(sst,t,1,1,0);		// erase the triangle
    }
  }
  DIAG_PASS(0);
}

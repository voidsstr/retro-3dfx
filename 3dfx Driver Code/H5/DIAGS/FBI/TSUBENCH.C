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
** $Date: 10/11/00 8:10:46 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

enum TSUMODES { TEST_XY, TEST_XY_P_RGB, TEST_XY_P_RGBA,
	TEST_XY_RGB, TEST_XY_RGBA, TEST_XY_RGBZ, TEST_XY_RGBAZ };

int packedARGB;
FxU32 smode;

void sendV(SstRegs *sst, Vertex *v)
{
    float fx = v->fx, fy = v->fy;

    SETF(sst->sVx,fx);
    SETF(sst->sVy,fy);
    gdbg_info(3,"Vertex: %g %g  (unsnapped) \n",fx,fy);
    if ((smode & (SST_SETUP_RGB|SST_SETUP_A)) && packedARGB) {
	FxU32 argb;
	argb  = ((int)v->fa)<<24;
	argb |= ((int)v->fr)<<16;
	argb |= ((int)v->fg)<<8;
	argb |= ((int)v->fb)<<0;
	SET(sst->sARGB,argb);
    }
    else {
	if (smode & SST_SETUP_RGB) {
	    SETF(sst->sRed,v->fr);
	    SETF(sst->sGreen,v->fg);
	    SETF(sst->sBlue,v->fb);
	}
	if (smode & SST_SETUP_A) {
	    SETF(sst->sAlpha,v->fa);
	}
    }

    if (smode & SST_SETUP_Z) {
	SETF(sst->sVz,v->fz);
    }
    if (smode & SST_SETUP_Wfbi) {
	SETF(sst->sOowfbi,v->fw);
    }
}

void main (int argc, char **argv)
{
    int n,startTime,endTime, aflag,zflag;
    FxU32 fbzCP, csrc, auxbits;
    static Triangle t;
    SstRegs *sst;
    
    // Parse the Command Line and Initialize the Simulator
    sst = SST_BEGIN( argc, argv );
    FXUNUSED(startTime);
    FXUNUSED(endTime);

    // Handle Options
    if ( diago.printOpts )
    {
	gdbg_printf( "tsubench option description:\n"  );
	gdbg_printf( "-b -> draw independent tris\n" );
	gdbg_printf( "-u -> number of triangles in the strip\n" );
	gdbg_printf( "-O -> setup parameters listed below\n" );
	gdbg_printf( " %d -> XY triangles\n", TEST_XY );
	gdbg_printf( " %d -> packed RGB\n", TEST_XY_P_RGB );
	gdbg_printf( " %d -> packed RGBA\n", TEST_XY_P_RGBA );
	gdbg_printf( " %d -> float RGB\n", TEST_XY_RGB );
	gdbg_printf( " %d -> float RGBA\n", TEST_XY_RGBA );
	gdbg_printf( " %d -> float RGB+Z\n", TEST_XY_RGBZ );
	gdbg_printf( " %d -> float RGBA+Z\n", TEST_XY_RGBAZ );
	exit( 0 );
    }

    while (DIAG_STARTPASS()) {			// for each pass
	int i,istop,itris;
	int size = diago.tsize;

	if (size < 0) size = -size;
	istop = diago.dstFormat;
	if (istop <=0) istop = 10;

	packedARGB = 0;
	switch (diago.option) {
	    case TEST_XY:
		smode = 0;
		break;
	    case TEST_XY_P_RGB:
		smode = SST_SETUP_RGB;
		packedARGB = 1;
		break;
	    case TEST_XY_P_RGBA:
		smode = SST_SETUP_RGB | SST_SETUP_A;
		packedARGB = 1;
		break;
	    case TEST_XY_RGB:
		smode = SST_SETUP_RGB;
		break;
	    case TEST_XY_RGBA:
		smode = SST_SETUP_RGB | SST_SETUP_A;
		break;
	    case TEST_XY_RGBZ:
		smode = SST_SETUP_RGB | SST_SETUP_Z;
		break;
	    case TEST_XY_RGBAZ:
	    default:
		smode = SST_SETUP_RGB | SST_SETUP_A | SST_SETUP_Z;
		break;
	}

	randomFloatRgbaTriangle(&t);
	t.vA.fz = fexpRandom(8,SST_Z_INTBITS);
	t.vB.fz = fexpRandom(8,SST_Z_INTBITS);
	t.vC.fz = fexpRandom(8,SST_Z_INTBITS);
	t.vA.fw = fexpRandom(-1,8);
	t.vB.fw = fexpRandom(-1,8);
	t.vC.fw = fexpRandom(-1,8);

	// Initialize for Simple Drawing Based on C1
	fbzCP = diago.adjust ? SST_PARMADJUST : 0;
	if (smode & SST_SETUP_RGB)
	    fbzCP |= SST_RGBSEL_RGBA | SST_CC_REPLACE;
	else 
	    fbzCP |= SST_RGBSEL_C1 | SST_CC_PASS;

	// need to decide which to check A,Z,W
	// and build up fbzColorPath flags
	zflag = aflag = auxbits = 0;
	switch (smode & (SST_SETUP_A|SST_SETUP_Z|SST_SETUP_Wfbi)) {
	    case SST_SETUP_A:
	    do_a:
		gdbg_info(2, "smode = 0x%x, testing Alpha iterator\n",smode);
	    do_a1:
		fbzCP |= SST_ASEL_RGBA | SST_ALOCAL_ITERATOR;
		aflag = 1;
		break;
	    case SST_SETUP_Z:
	    do_z:
		if (diago.rgb != 16) {
		    fbzCP |= SST_ASEL_C1 | SST_CCA_PASS;
		    aflag = 999;
		}
		gdbg_info(2, "smode = 0x%x, testing Z iterator\n",smode);
		zflag = 1;
		break;
	    case SST_SETUP_Wfbi:
	    do_w:
		if (diago.rgb != 16) {
		    fbzCP |= SST_ASEL_C1 | SST_CCA_PASS;
		    aflag = 999;
		}
		gdbg_info(2, "smode = 0x%x, testing W iterator\n",smode);
		zflag = -1;
		break;
	    case SST_SETUP_A | SST_SETUP_Z:
		if (diago.rgb==16) {	// do one or the other
		    if (iRandom(1)) goto do_a; else goto do_z;
		}
	    do_az:
		// we can do both
		gdbg_info(2, "smode = 0x%x, testing Alpha+Z iterators\n",smode);
		zflag = 1;
		goto do_a1;
	    case SST_SETUP_A | SST_SETUP_Wfbi:
		if (diago.rgb==16) {	// do one or the other
		    if (iRandom(1)) goto do_a; else goto do_w;
		}
	    do_aw:
		// we can do both
		gdbg_info(2, "smode = 0x%x, testing Alpha+W iterators\n",smode);
		zflag = -1;
		goto do_a1;
	    case SST_SETUP_Z | SST_SETUP_Wfbi:
		if (iRandom(1)) goto do_z; else goto do_w;
	    case SST_SETUP_A | SST_SETUP_Z | SST_SETUP_Wfbi:
		if (diago.rgb==16) {	// do one or the other
		  switch(iRandom(2)) {
		    case 0: goto do_a;
		    case 1: goto do_z;
		    case 2: goto do_w;
		  }
		}
		if (iRandom(1)) goto do_az; else goto do_aw;
	    default:		// none setup so don't write the alpha or aux planes
		gdbg_info(2, "smode = 0x%x, no auxbuffer\n",smode);
		if (diago.rgb != 16) {
		    fbzCP |= SST_ASEL_C1 | SST_CCA_PASS;
		    aflag = 999;
		}
		break;
	}
	// now build up fbzMode bits for the AUX planes
	if (aflag && (diago.rgb==16)) auxbits |=  SST_ZAWRMASK | SST_ENALPHABUFFER;
	if (zflag) auxbits |= SST_ZAWRMASK;
	if (zflag<0) auxbits |= SST_WBUFFER | SST_ENDEPTHBUFFER | SST_ZFUNC;

	if (diago.xmaxscreen - size*istop/2 < 10) {
	    GDBG_ERROR("main","triangle strip is too large, extends off screen\n");
	    DIAG_INCERROR();
	}
	SET(sst->fbzColorPath, fbzCP);
	SET(sst->fbzMode, SST_RGBWRMASK | auxbits | drawbufferRandom());
	csrc = colRandom16();			// and random colors
	SET(sst->c1,csrc);

	sst_idle_really(sst);
	startTime = DIAG_TIME();
	gdbg_info(1,"start of 10 strips at time = %d ns\n",startTime);

      for (n=0; n<10; n++) {			// do # strips
	t.vA.x = iRandom((diago.xmaxscreen-size*istop/2)*XY_ONE);
	t.vA.y = iRandom((diago.ymaxscreen-size-2)*XY_ONE);
	t.vA.fx = t.vA.x/(float)XY_ONE;
	t.vA.fy = t.vA.y/(float)XY_ONE;

	t.vB.fx = t.vA.fx;
	t.vB.fy = t.vA.fy + diago.tsize;

	t.vC.fx = t.vB.fx;
	t.vC.fy = t.vB.fy;

	SET(sst->sSetupMode, smode);
	sendV(sst,&t.vA);
	SET(sst->sBeginTriCMD,0);

	sendV(sst,&t.vB);
	SET(sst->sDrawTriCMD,0);

	itris = diago.bilinear ? (istop+2)/3 : istop;
	gdbg_info(2,"generating %d random tris in a %s\n",
			itris, smode & SST_SETUP_FAN ? "fan" : "strip");
	// NOTE: i==0 for the first triangle since we already output vA and vB
	for (i=0; i<istop; i++) {
	    if (i & 1) {
		t.vC.fy = t.vC.fy + diago.tsize;
	    }
	    else {
		t.vC.fx = t.vC.fx+diago.tsize;
		t.vC.fy = t.vC.fy-diago.tsize;
	    }
	    t.vC.fr = fexpRandom(-1,SST_RGBA_INTBITS);
	    t.vC.fg = fexpRandom(-3,SST_RGBA_INTBITS);
	    t.vC.fb = fexpRandom(-2,SST_RGBA_INTBITS);
	    t.vC.fa = fexpRandom(-4,SST_RGBA_INTBITS);
	    t.vC.fz = fexpRandom(8,SST_Z_INTBITS);
	    t.vC.fw = fexpRandom(-1,8);

	    sendV(sst,&t.vC);
	    if (diago.bilinear && !((i+2)%3)) {
		SET(sst->sBeginTriCMD,0);
	    }
	    else
		SET(sst->sDrawTriCMD,0);
	}
      }
	sst_idle_really(sst);
	endTime = DIAG_TIME();
	gdbg_info(1,"end of 10 strips at time = %d ns\n",endTime);
	gdbg_printf("10 strips of %d triangles in %d ns, %.2fM triangles/sec\n",
		itris,endTime-startTime,itris*10*1e3/(endTime-startTime));
    }
    DIAG_PASS(0);
}

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
** $Date: 10/11/00 8:19:29 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

enum TSUMODES { TEST_XY, TEST_XY_BAD, TEST_RND_3D, TEST_CULL, TEST_FLIPS,
		TEST_RGBAZW_FLAT, TEST_RGBAZW_SHADED, TEST_STW_SHADED };

int packedARGB;				// flags packed ARGB format
int trashW0, trashW1, trashST1;		// set these to trash and setup
FxU32 smode;

void sendV(SstRegs *sst, Vertex *v)
{
    float fx = v->fx, fy = v->fy;

    // add in random little bits here.....up to 1/16 but no more!
    if (diago.option >= TEST_XY_BAD) {
	fx += fRandom(0)/16.1F;
	fy += fRandom(0)/16.1F;
    }
    SETF(sst->sVx,fx);
    SETF(sst->sVy,fy);
    gdbg_info(3,"Vertex: %g %g  (unsnapped) \n",fx,fy);
    if ((smode & (SST_SETUP_RGB|SST_SETUP_A)) && packedARGB) {
	FxU32 argb;
	argb  = (v->a >> SST_RGBA_FRACBITS)<<24;
	argb |= (v->r >> SST_RGBA_FRACBITS)<<16;
	argb |= (v->g >> SST_RGBA_FRACBITS)<<8;
	argb |= (v->b >> SST_RGBA_FRACBITS)<<0;
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
    if (smode & SST_SETUP_W0) {
	SETF(sst->sOow0,v->fw);
    }
    if (smode & SST_SETUP_ST0) {
	SETF(sst->sSow0,v->fs);
	SETF(sst->sTow0,v->ft);
    }
    if (smode & SST_SETUP_W1) {
	SETF(sst->sOow1,v->fw);
    }
    if (smode & SST_SETUP_ST1) {
	SETF(sst->sSow1,v->fs);
	SETF(sst->sTow1,v->ft);
    }
    // attempt to trash values in lower chips
    if (trashW0) {
	SETF(sst->sOow0,-1.2345F);
    }
    if (trashW1) {
	SETF(sst->sOow1,1.2345F);
    }
    if (trashST1) {
	SETF(sst->sSow1,-2.9876F);
	SETF(sst->sTow1,2.9876F);
    }
}

void main (int argc, char **argv)
{
    int area,n, culled,pingpong, aflag,zflag;
    FxU32 fbzCP, csrc, auxbits;
    Triangle *t, *tcheck;
    SstRegs *sst,*sstF;


    inhibitTwoPixelsPerClock();

    // Parse the Command Line and Initialize the Simulator
    sst = SST_BEGIN( argc, argv );
    sstF = SST_CHIP(sst,0xF);		// chip field F

    if(diago.bigAssTextures)
      {
	t = buildTriangle(2048, 2048);
	tcheck = buildTriangle(2048, 2048);
      }
    else
      {
	t = buildTriangle(256, 256);
	tcheck = buildTriangle(256, 256);
      }


    t->next = NULL;
    // have to prevent negative W because we don't clamp correctly in checkTriangle
    diago.adjust = 1;

    // Handle Options
    if ( diago.printOpts )
    {
	gdbg_printf( "tsu option description (cumulative):\n"  );
	gdbg_printf( " %d -> Test XY snapped( default )\n", TEST_XY );
	gdbg_printf( " %d -> Test XY unsnapped\n", TEST_XY_BAD );
	gdbg_printf( " %d -> Test random regs\n", TEST_RND_3D );
	gdbg_printf( " %d -> Test CULL\n", TEST_CULL );
	gdbg_printf( " %d -> Test fan/strip flips\n", TEST_FLIPS );
	gdbg_printf( " %d -> Test RGBAZW flat\n", TEST_RGBAZW_FLAT );
	gdbg_printf( " %d -> Test RGBAZW shaded\n", TEST_RGBAZW_SHADED );
	gdbg_printf( " %d -> Test STW shaded\n", TEST_STW_SHADED );
	exit( 0 );
    }

    while (DIAG_STARTPASS()) {			// for each pass
      for (n=0; n<20; n++) {			// do # strips
	int i,istop;
	int size = diago.tsize;

	t->tex->trex = diago.trex;
	if (t->tex->trex < 0)			// optionally choose random trex
	    t->tex->trex = iRandom(-t->tex->trex);

	smode = iRandom(1) ? SST_SETUP_FAN : 0;
	if (diago.option >= TEST_RGBAZW_FLAT) {	// add in random flat params
	    if (iRandom(1)) smode |= SST_SETUP_RGB;
	    if (iRandom(1)) smode |= SST_SETUP_A;
	    if (iRandom(1)) smode |= SST_SETUP_Z;
	    if (iRandom(1)) smode |= SST_SETUP_Wfbi;
	    if (diago.option >= TEST_STW_SHADED) {

	      if (t->tex->trex == 0) {
		if (iRandom(1)) smode |= SST_SETUP_W0 | SST_SETUP_ST0;
		if (iRandom(1)) smode |= SST_SETUP_ST0 | (diago.perspective?SST_SETUP_W0:0);
	      }
	      else if (t->tex->trex >= 1) {
		if (iRandom(1)) smode |= SST_SETUP_W1 | SST_SETUP_ST1;
		if (iRandom(1)) smode |= SST_SETUP_ST1 | (diago.perspective?SST_SETUP_W1:0);
	      }
	    }
	    packedARGB = iRandom(1);
	    if (packedARGB) {
		randomRgbaTriangle(t);
		t->vA.r &= ~SST_MASK(SST_RGBA_FRACBITS);	// truncate the frac bits
		t->vA.g &= ~SST_MASK(SST_RGBA_FRACBITS);
		t->vA.b &= ~SST_MASK(SST_RGBA_FRACBITS);
		t->vA.a &= ~SST_MASK(SST_RGBA_FRACBITS);
		t->vB.r &= ~SST_MASK(SST_RGBA_FRACBITS);	// truncate the frac bits
		t->vB.g &= ~SST_MASK(SST_RGBA_FRACBITS);
		t->vB.b &= ~SST_MASK(SST_RGBA_FRACBITS);
		t->vB.a &= ~SST_MASK(SST_RGBA_FRACBITS);
		t->vC.r &= ~SST_MASK(SST_RGBA_FRACBITS);	// truncate the frac bits
		t->vC.g &= ~SST_MASK(SST_RGBA_FRACBITS);
		t->vC.b &= ~SST_MASK(SST_RGBA_FRACBITS);
		t->vC.a &= ~SST_MASK(SST_RGBA_FRACBITS);
		t->vA.fr = (float)(t->vA.r>>SST_RGBA_FRACBITS);
		t->vA.fg = (float)(t->vA.g>>SST_RGBA_FRACBITS);
		t->vA.fb = (float)(t->vA.b>>SST_RGBA_FRACBITS);
		t->vA.fa = (float)(t->vA.a>>SST_RGBA_FRACBITS);
		t->vB.fr = (float)(t->vB.r>>SST_RGBA_FRACBITS);
		t->vB.fg = (float)(t->vB.g>>SST_RGBA_FRACBITS);
		t->vB.fb = (float)(t->vB.b>>SST_RGBA_FRACBITS);
		t->vB.fa = (float)(t->vB.a>>SST_RGBA_FRACBITS);
		t->vC.fr = (float)(t->vC.r>>SST_RGBA_FRACBITS);
		t->vC.fg = (float)(t->vC.g>>SST_RGBA_FRACBITS);
		t->vC.fb = (float)(t->vC.b>>SST_RGBA_FRACBITS);
		t->vC.fa = (float)(t->vC.a>>SST_RGBA_FRACBITS);
	    }
	    else randomFloatRgbaTriangle(t);

	    t->vA.fz = fexpRandom(8,SST_Z_INTBITS);
	    t->vA.z64 = float2fix64(t->vA.fz,SST_Z64_FRACBITS);
	    t->vB.fz = fexpRandom(8,SST_Z_INTBITS);
	    t->vB.z64 = float2fix64(t->vB.fz,SST_Z64_FRACBITS);
	    t->vC.fz = fexpRandom(8,SST_Z_INTBITS);
	    t->vC.z64 = float2fix64(t->vC.fz,SST_Z64_FRACBITS);
	    t->vA.fs = fexpRandom(-1,8);
	    t->vA.s = float2fix64(t->vA.fs,SST_ST64_FRACBITS);
	    t->vA.ft = fexpRandom(-1,8);
	    t->vA.t = float2fix64(t->vA.ft,SST_ST64_FRACBITS);
	    t->vA.fw = fexpRandom(-4,2);
	    t->vA.w = float2fix64(t->vA.fw,SST_W64_FRACBITS);
	    t->vB.fs = fexpRandom(-1,8);
	    t->vB.s = float2fix64(t->vB.fs,SST_ST64_FRACBITS);
	    t->vB.ft = fexpRandom(-1,8);
	    t->vB.t = float2fix64(t->vB.ft,SST_ST64_FRACBITS);
	    t->vB.fw = fexpRandom(-4,2);
	    t->vB.w = float2fix64(t->vB.fw,SST_W64_FRACBITS);
	    t->vC.fs = fexpRandom(-1,8);
	    t->vC.s = float2fix64(t->vC.fs,SST_ST64_FRACBITS);
	    t->vC.ft = fexpRandom(-1,8);
	    t->vC.t = float2fix64(t->vC.ft,SST_ST64_FRACBITS);
	    t->vC.fw = fexpRandom(-4,2);
	    t->vC.w = float2fix64(t->vC.fw,SST_W64_FRACBITS);
	    // every once in a while test degenerate S,T values
	    if (iRandom(2)==0) {
		gdbg_info(2,"testing very small numbers\n");
		t->vB.fs = (float)(t->vB.fs / pow(2,129));
		t->vB.s = float2fix64(t->vB.fs,SST_ST64_FRACBITS);
		t->vB.ft = (float)(t->vB.ft / pow(2,129));
		t->vB.t = float2fix64(t->vB.ft,SST_ST64_FRACBITS);
	    }
	    if (diago.option < TEST_RGBAZW_SHADED) {
		t->vB.r = t->vA.r;		// copy vA to vB and vC
		t->vB.g = t->vA.g;
		t->vB.b = t->vA.b;
		t->vB.a = t->vA.a;
		t->vB.z64 = t->vA.z64;
		t->vB.w = t->vA.w;
		t->vB.fr = t->vA.fr;
		t->vB.fg = t->vA.fg;
		t->vB.fb = t->vA.fb;
		t->vB.fa = t->vA.fa;
		t->vB.fz = t->vA.fz;
		t->vB.fw = t->vA.fw;
		t->vC.r = t->vA.r;
		t->vC.g = t->vA.g;
		t->vC.b = t->vA.b;
		t->vC.a = t->vA.a;
		t->vC.z64 = t->vA.z64;
		t->vC.w = t->vA.w;
		t->vC.fr = t->vA.fr;
		t->vC.fg = t->vA.fg;
		t->vC.fb = t->vA.fb;
		t->vC.fa = t->vA.fa;
		t->vC.fz = t->vA.fz;
		t->vC.fw = t->vA.fw;
	    }
	}
	if (diago.option >= TEST_CULL) {	// add in random culling
	    if (iRandom(1)) smode |= SST_SETUP_EN_CULLING;
	    if (iRandom(1)) smode |= SST_SETUP_CULL_NEGATIVE;
	    if (iRandom(1)) smode |= SST_SETUP_DIS_PINGPONG;
	}

	// Initialize for Simple Drawing Based on C1
	fbzCP = diago.adjust ? SST_PARMADJUST : 0;
	zflag = aflag = auxbits = 0;
	if (smode & (SST_SETUP_ST0|SST_SETUP_ST1)) {
	    fbzCP |= SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT | SST_ENTEXTUREMAP;
	    aflag = 1;
	    if (diago.rgb != 16) {
		fbzCP |= SST_CC_PASS | SST_CCA_PASS;
	    }
	    if (smode & SST_SETUP_Wfbi) {
		zflag = -1;
		aflag = 0;
	    }
	}
	else {
	  if (smode & SST_SETUP_RGB)
		fbzCP |= SST_RGBSEL_RGBA | SST_CC_REPLACE;
	  else 
		fbzCP |= SST_RGBSEL_C1 | SST_CC_PASS;

	  // need to decide which to check A,Z,W
	  // and build up fbzColorPath flags
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
	}
	// now build up fbzMode bits for the AUX planes
	if (aflag && (diago.rgb==16)) auxbits |=  SST_ZAWRMASK | SST_ENALPHABUFFER;
	if (zflag) auxbits |= SST_ZAWRMASK;
	if (zflag<0) auxbits |= SST_WBUFFER | SST_ENDEPTHBUFFER | SST_ZFUNC;

        // use the 16-bit 4444 RGBA texture format if alpha is being tested,
	// else use RGB 565, in both cases, the size = 8x8, in replace mode
	// this gets loaded into one TMU chip, the other is in pass-thru mode
        t->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) |  SST_TC_REPLACE | SST_TCA_REPLACE;
	t->tex->tMode |= aflag ? SST_ARGB4444 : SST_RGB565;
        texRandomTextureMap(sst,t->tex->trex,0, 6,6,t->tex);

	randomTriangle(t, size, 1);
	trashW0 = trashW1 = trashST1 = 0;
	if (!diago.writeFifo) {	// cmd fifo sniffer expects writes in a special order
	    if ((smode & SST_SETUP_Wfbi) && !(smode & (SST_SETUP_W0|SST_SETUP_W1)))
		trashW0 = iRandom(1);
	    if ((smode & (SST_SETUP_Wfbi|SST_SETUP_W0)) && !(smode & SST_SETUP_W1))
		trashW1 = iRandom(1);
	    if ((smode & (SST_SETUP_ST0)) && !(smode & SST_SETUP_ST1))
		trashST1 = iRandom(1);
	}
	gdbg_info(2,"trash: %s %s %s\n",
			trashW0 ? "W0" : "",
			trashW1 ? "W1" : "",
			trashST1 ? "ST1" : "");
	if (trashW0) smode |= SST_SETUP_W0;
	if (trashW1) smode |= SST_SETUP_W1;
	if (trashST1) smode |= SST_SETUP_ST1;

	// if we are setting up TMU1, test out upstream broadcast mode
	if ((smode & SST_SETUP_W1) && !trashW1) {
	    if (iRandom(1)) {
		smode &= ~SST_SETUP_W1;		// send to W0
		smode |= SST_SETUP_W0;		// and reply on broadcast
		trashW0 = 0;
		gdbg_info(2,"testing W0 broadcast\n");
	    }
	}
	if ((smode & SST_SETUP_ST1) && !trashST1) {
	    if (iRandom(1)) {
		smode &= ~SST_SETUP_ST1;	// send to ST0
		smode |= SST_SETUP_ST0;		// and rely on broadcast
		gdbg_info(2,"testing ST0 broadcast\n");
	    }
	}
	if (iRandom(1)) {		// send fbz* first, then TSU regs
	    SET(sstF->fbzColorPath, fbzCP);
	    SET(sstF->fbzMode, SST_RGBWRMASK | auxbits | drawbufferRandom());
	
	    SET(sst->sSetupMode, smode);
	    sendV(sst,&t->vA);
	    SET(sst->sBeginTriCMD,0);
	}
	else {				// intermix fbz* with TSU regs
	    SET(sst->sSetupMode, smode);
	    SET(sstF->fbzColorPath, fbzCP);
    
	    sendV(sst,&t->vA);
	    SET(sst->sBeginTriCMD,0);

	    SET(sstF->fbzMode, SST_RGBWRMASK | auxbits | drawbufferRandom());
	}
	pingpong = 0;

	sendV(sst,&t->vB);
	SET(sst->sDrawTriCMD,0);

	do {
	    csrc = colRandom32();		// and random colors
	} while (csrc == 0);
	SET(sstF->c1,csrc);
	if (size < 0) size = -size;


	// copy t into tcheck (including textures)
	copyTriangle(tcheck, t);

	istop = iRandom(20);
	gdbg_info(2,"generating %d random tris in a %s\n",
			istop, smode & SST_SETUP_FAN ? "fan" : "strip");
	// NOTE: i==0 for the first triangle since we already output vA and vB
	for (i=0; i<istop; i++) {
	    if (i > 0) {
		int tries = 0;

		// emulate the TSU vertex stack
		if (smode & SST_SETUP_FAN) {
		    t->vB = t->vC;
		}
		else {
		    t->vA = t->vB;
		    t->vB = t->vC;
		    pingpong ^= 1;
		}

		if (iRandom(10)==0) {		// once in a while gen a 0 area tri
		    t->vC = iRandom(1) ? t->vA : t->vB;
		}
		else
		do {
		    if (++tries > 1000) {
			GDBG_ERROR("main", "too many retries generating vertex C\n");
			DIAG_INCERROR();
			break;
		    }
		    if (smode & SST_SETUP_FAN) {
			t->vC.x = t->vA.x + rRandom(-size*XY_ONE,size*XY_ONE);
			t->vC.y = t->vA.y + rRandom(-size*XY_ONE,size*XY_ONE);
		    }
		    else {
			t->vC.x = t->vB.x + rRandom(-size*XY_ONE,size*XY_ONE);
			t->vC.y = t->vB.y + rRandom(-size*XY_ONE,size*XY_ONE);
		    }
		    t->vC.fx = fix2float(&t->vC.x,SST_XY_FRACBITS);
		    t->vC.fy = fix2float(&t->vC.y,SST_XY_FRACBITS);
		 } while (!ONSCREEN_FRAC(t->vC.x,t->vC.y));

		if (diago.option >= TEST_RGBAZW_SHADED) {
		    if (packedARGB) {
			t->vC.r = iRandom(255)<<SST_RGBA_FRACBITS;
			t->vC.g = iRandom(255)<<SST_RGBA_FRACBITS;
			t->vC.b = iRandom(255)<<SST_RGBA_FRACBITS;
			t->vC.a = iRandom(255)<<SST_RGBA_FRACBITS;
			t->vC.fr = (float)(t->vC.r>>SST_RGBA_FRACBITS);
			t->vC.fg = (float)(t->vC.g>>SST_RGBA_FRACBITS);
			t->vC.fb = (float)(t->vC.b>>SST_RGBA_FRACBITS);
			t->vC.fa = (float)(t->vC.a>>SST_RGBA_FRACBITS);
		    }
		    else {
			t->vC.fr = fexpRandom(-1,SST_RGBA_INTBITS);
			t->vC.fg = fexpRandom(-3,SST_RGBA_INTBITS);
			t->vC.fb = fexpRandom(-2,SST_RGBA_INTBITS);
			t->vC.fa = fexpRandom(-4,SST_RGBA_INTBITS);
			t->vC.r = float2fix(t->vC.fr,SST_RGBA_FRACBITS);
			t->vC.g = float2fix(t->vC.fg,SST_RGBA_FRACBITS);
			t->vC.b = float2fix(t->vC.fb,SST_RGBA_FRACBITS);
			t->vC.a = float2fix(t->vC.fa,SST_RGBA_FRACBITS);
		    }
		    t->vC.fz = fexpRandom(8,SST_Z_INTBITS);
		    t->vC.z64 = float2fix64(t->vC.fz,SST_Z64_FRACBITS);
		    t->vC.fs = fexpRandom(-1,8);
		    t->vC.s = float2fix64(t->vC.fs,SST_ST64_FRACBITS);
		    t->vC.ft = fexpRandom(-1,8);
		    t->vC.t = float2fix64(t->vC.ft,SST_ST64_FRACBITS);
		    t->vC.fw = fexpRandom(-4,2);
		    t->vC.w = float2fix64(t->vC.fw,SST_W64_FRACBITS);
		}
	    }

	    // we need to copy the vertices to tcheck because we need to
	    // sort them for checkTriangle, yet we need the original unsorted
	    // vertices for generating new vertices
	    tcheck->vA = t->vA;
	    tcheck->vB = t->vB;
	    tcheck->vC = t->vC;
	    areaTriangle(tcheck);		// compute the area (before setup)
	    area = tcheck->area;			// pre-sorted area
	    setupFloatTriangle(tcheck,smode & SST_SETUP_RGB,aflag,zflag>0,
		zflag<0 || (smode & (SST_SETUP_ST0|SST_SETUP_ST1)));
	    // check for out-of-range slopes from sliver triangles
	    if (fabs(tcheck->fdsdx) > 2000.0F || fabs(tcheck->fdsdy) > 2000.0F ||
		fabs(tcheck->fdtdx) > 2000.0F || fabs(tcheck->fdtdy) > 2000.0F ||
		fabs(tcheck->fdwdx) > 2000.0F || fabs(tcheck->fdwdy) > 2000.0F)
	    {
		gdbg_info(2,"out-of-range slopes detected\n");
		tcheck->vC = tcheck->vB;		// make it degenerate
		t->vC = t->vB;
	    }

	    sortTriangle(tcheck);			// sort it
	    printTriangle(2,tcheck,smode & SST_SETUP_RGB,aflag,zflag);
	    printTriangleSlopes(3,tcheck,smode & SST_SETUP_RGB,aflag,zflag);
	    if (smode & (SST_SETUP_ST0|SST_SETUP_ST1)) {
		printStwTriangle(4,tcheck);
		printStwTriangleSlopes(5,tcheck);
	    }

	    areaTriangle(t);				// original triangle area
	    gdbg_info(3,"    area = %d/256 or %g\n", area,area/256.0F);
	    if (smode & SST_SETUP_EN_CULLING) {
		int neg_area = area < 0;
		int enpp = (smode & SST_SETUP_DIS_PINGPONG)==0;
		int cull_neg = (smode & SST_SETUP_CULL_NEGATIVE) != 0;
		culled = !(neg_area ^ cull_neg ^ (pingpong&enpp));
		gdbg_info(3,"    cull enabled: cull=%d : area.sign=%d ^ cull_bit=%d ^ pingpong=%d\n",
				culled, neg_area,cull_neg,pingpong&enpp);
	    }
	    else culled = 0;

	    sendV(sst,&t->vC);
	    SET(sst->sDrawTriCMD,0);

	    sst_idle(sst);				// wait for the command to complete
	    if ((smode & SST_SETUP_ST0) || ((smode&SST_SETUP_ST1)&&!trashST1)) {
		if (zflag)
		    checkTriangle(tcheck,
				culled?0:sst_argb_form_result(csrc,0,0,0),
				0,
				diago.adjust,
				insideTriangle,
				0,0,zflag);		// check just Z/W
		checkTriangle(tcheck,culled?0:1,1,diago.adjust, insideStTriangle,0,0,0);
	    }
	    else					// non-textured
		checkTriangle(tcheck,
				culled?0:sst_argb_form_result(csrc,0,0,0),
				1,			// bloat the check
				diago.adjust,
				insideTriangle,
				smode & SST_SETUP_RGB,
				aflag,zflag);
	    eraseTriangle(sst,tcheck,1,1,1);		// erase the triangle

	    if (diago.option >= TEST_RND_3D) 
	    if (iRandom(3)==0)
	    {		// test some random reg writes
		if (iRandom(1)) SET(sstF->c0,csrc+0xabcd);
		if (iRandom(1)) SET(sstF->alphaMode,0);
		if (iRandom(1)) SET(sstF->fogMode,0);
		if (iRandom(1)) SET(sstF->lfbMode,0);
		if (iRandom(1)) SET(sstF->chromaKey,0);
		if (iRandom(1)) SET(sstF->chromaRange,0);
	    }

	    if (diago.option >= TEST_FLIPS) {		// change sSetupMode
		if (iRandom(2)==0) {
		    smode ^= iRandom(0xFFFF)<<16;
		    SET(sst->sSetupMode, smode);
		}
	    }
	}
      }
    }
    DIAG_PASS(0);
}

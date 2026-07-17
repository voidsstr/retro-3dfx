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
** $Date: 10/11/00 8:19:07 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

//---------------------------------------------------------------------
// NOTE: same as within floatstw.c
void drawFloatStwTriangle(SstRegs *sst, Triangle *t)
{
    SET(sst->vA.x,t->vA.x);
    SET(sst->vA.y,t->vA.y);
    SET(sst->vB.x,t->vB.x);
    SET(sst->vB.y,t->vB.y);
    SET(sst->vC.x,t->vC.x);
    SET(sst->vC.y,t->vC.y);
    SETF(sst->Fs,t->vA.fs);
    SETF(sst->Ft,t->vA.ft);
    SETF(sst->Fw,t->vA.fw);
    SETF(sst->Fdsdx,t->fdsdx);
    SETF(sst->Fdsdy,t->fdsdy);
    SETF(sst->Fdtdx,t->fdtdx);
    SETF(sst->Fdtdy,t->fdtdy);
    SETF(sst->Fdwdx,t->fdwdx);
    SETF(sst->Fdwdy,t->fdwdy);
    if (t->area != 0)
	SET(sst->triangleCMD,t->area);
}

static float plusMinus(void)
{
    return iRandom(1) ? 1.0F : -1.0F;
}

// construct up a floating point value from bits
// the range of numbers is plus/minus 2**15 so we want the max exponent=14
// since the mantissa is between 1.0 and 2.0
float totallyRandomFloat()
{
    int x,exp;
    x = iRandom(0x7FFFFF);		// 23-bit random mantissa
    exp = rRandom(-32,14) +127;		// IEEE is 127 biased
    x |= exp << 23;
    if (iRandom(1)) x |= 0x80000000;    // randomly turn on sign bit
    return *(float*)&x;
}

void TOTALLYrandomFloatStwTriangle_Setup(Triangle *t)
{
    double dx,dy;

    t->vA.fw = totallyRandomFloat();
    t->vA.fs = totallyRandomFloat();
    t->vA.ft = totallyRandomFloat();
    t->vA.fw = totallyRandomFloat();
    t->fdsdx = totallyRandomFloat();
    t->fdsdy = totallyRandomFloat();
    t->fdtdx = totallyRandomFloat();
    t->fdtdy = totallyRandomFloat();
    t->fdwdx = totallyRandomFloat();
    t->fdwdy = totallyRandomFloat();

    // compute these solely for printing out the approx triangle
    dx = (double)(t->vB.x - t->vA.x);		// calculate vB,vC
    dy = (double)(t->vB.y - t->vA.y);		// from vA and slopes
    t->vB.fs = (float)(t->vA.fs + (dx * t->fdsdx)/XY_ONE + (dy * t->fdsdy)/XY_ONE);
    t->vB.ft = (float)(t->vA.ft + (dx * t->fdtdx)/XY_ONE + (dy * t->fdtdy)/XY_ONE);
    t->vB.fw = (float)(t->vA.fw + (dx * t->fdwdx)/XY_ONE + (dy * t->fdwdy)/XY_ONE);
    dx = (double)(t->vC.x - t->vA.x);
    dy = (double)(t->vC.y - t->vA.y);
    t->vC.fs = (float)(t->vA.fs + (dx * t->fdsdx)/XY_ONE + (dy * t->fdsdy)/XY_ONE);
    t->vC.ft = (float)(t->vA.ft + (dx * t->fdtdx)/XY_ONE + (dy * t->fdtdy)/XY_ONE);
    t->vC.fw = (float)(t->vA.fw + (dx * t->fdwdx)/XY_ONE + (dy * t->fdwdy)/XY_ONE);
#if 0
    // old safe way for computing random floats, just too stuffy for us
    again:	// need: s = plusMinus();
	t->vA.fw = fexpRandom(-32,16);
	t->vA.fs = s * fexpRandom(-31,15);
	t->vA.ft = s * fexpRandom(-31,15);
	t->vA.fw = s * fexpRandom(-31,15);
	t->fdsdx = s * fexpRandom(-31,15);
	t->fdsdy = s * fexpRandom(-31,15);
	t->fdtdx = s * fexpRandom(-31,15);
	t->fdtdy = s * fexpRandom(-31,15);
	t->fdwdx = s * fexpRandom(-31,15);
	t->fdwdy = s * fexpRandom(-31,15);
    // don't let 1/w go negative, be a little conservative
    if ((t->vB.fw < 0.00000001F) || (t->vC.fw <= 0.00000001F))
	goto again;
#endif

    // we need to do this so that the self-checking code which works off the
    // 64-bit fixed point regs, ends up doing the same calcs as the hardware
    t->vA.s = float2fix64(t->vA.fs,SST_ST64_FRACBITS);
    t->vA.t = float2fix64(t->vA.ft,SST_ST64_FRACBITS);
    t->vA.w = float2fix64(t->vA.fw,SST_W64_FRACBITS);
    t->vB.s = float2fix64(t->vB.fs,SST_ST64_FRACBITS);
    t->vB.t = float2fix64(t->vB.ft,SST_ST64_FRACBITS);
    t->vB.w = float2fix64(t->vB.fw,SST_W64_FRACBITS);
    t->vC.s = float2fix64(t->vC.fs,SST_ST64_FRACBITS);
    t->vC.t = float2fix64(t->vC.ft,SST_ST64_FRACBITS);
    t->vC.w = float2fix64(t->vC.fw,SST_W64_FRACBITS);
    t->dsdx = float2fix64(t->fdsdx,SST_ST64_FRACBITS);
    t->dsdy = float2fix64(t->fdsdy,SST_ST64_FRACBITS);
    t->dtdx = float2fix64(t->fdtdx,SST_ST64_FRACBITS);
    t->dtdy = float2fix64(t->fdtdy,SST_ST64_FRACBITS);
    t->dwdx = float2fix64(t->fdwdx,SST_W64_FRACBITS);
    t->dwdy = float2fix64(t->fdwdy,SST_W64_FRACBITS);
}

void
main (int argc, char **argv)
{
    int n;
    Triangle *t;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    diago.perspective = 1;		// force perspective
    diago.bilinear = 1;			// force bilinear, makes s,t fractions relevant

    if(diago.bigAssTextures)
      t = buildTriangle(2048, 2048);
    else
      t = buildTriangle(256, 256);

    t->next = NULL;
 
  while (DIAG_STARTPASS()) {			// for each pass
    // use the 8-bit 332 RGB texture format, size = 16x16, in replace mode
    t->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) |
			SST_TMINFILTER | SST_TMAGFILTER |
			SST_RGB332 | SST_TC_REPLACE | SST_TCA_REPLACE;
    texRandomTextureMap(sst,diago.trex,0, 4,4,t->tex);
    SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT |
    		SST_ENTEXTUREMAP |
		(diago.adjust?SST_PARMADJUST:0));

    for (n=0; n<50; n++) {			// do 50 tests
	SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());
	if (diago.clamp) {
	    if (iRandom(1)) t->tex->tMode ^= SST_TCLAMPS;
	    if (iRandom(1)) t->tex->tMode ^= SST_TCLAMPT;
	    if (iRandom(1)) t->tex->tMode ^= SST_TCLAMPW;
	}
	gdbg_info(3,"clamp s,t,w=%d,%d,%d\n",
			(t->tex->tMode & SST_TCLAMPS) != 0,
			(t->tex->tMode & SST_TCLAMPT) != 0,
			(t->tex->tMode & SST_TCLAMPW) != 0);
	// set just the active TREX chip
	SET(SST_TREX(sst,t->tex->trex)->textureMode,t->tex->tMode);

	randomTriangle(t,diago.tsize,1);	// pick random triangle
	areaTriangle(t);			// compute the area (before setup)
	sortTriangle(t);			// sort it
	TOTALLYrandomFloatStwTriangle_Setup(t);// random STW floating point

	printStwTriangle(4,t);
	printStwTriangleSlopes(5,t);

	drawFloatStwTriangle(sst,t);

	sst_idle(sst);				// wait for the command to complete
	checkTriangle(t,1,1,diago.adjust, insideStTriangle,0,0,0);
	eraseTriangle(sst,t,1,0,0);		// erase the triangle
    }
  }
  DIAG_PASS(1);					// check for black screen
}

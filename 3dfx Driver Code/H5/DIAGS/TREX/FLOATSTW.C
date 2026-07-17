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


//---------------------------------------------------------------------
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

void drawAltFloatStwTriangle(SstRegs *sst, Triangle *t)
{
    sst = SST_WRAP(sst,0x80);		// set high bit in WRAP field

    SET(sst->vA.x,t->vA.x);
    SET(sst->vA.y,t->vA.y);
    SET(sst->vB.x,t->vB.x);
    SET(sst->vB.y,t->vB.y);
    SET(sst->vC.x,t->vC.x);
    SET(sst->vC.y,t->vC.y);
    SETF(sst->Fs_ALT,t->vA.fs);
    SETF(sst->Fdsdx_ALT,t->fdsdx);
    SETF(sst->Fdsdy_ALT,t->fdsdy);
    SETF(sst->Ft_ALT,t->vA.ft);
    SETF(sst->Fdtdx_ALT,t->fdtdx);
    SETF(sst->Fdtdy_ALT,t->fdtdy);
    SETF(sst->Fw_ALT,t->vA.fw);
    SETF(sst->Fdwdx_ALT,t->fdwdx);
    SETF(sst->Fdwdy_ALT,t->fdwdy);
    if (t->area != 0)
	SET(sst->triangleCMD,t->area);
}

//---------------------------------------------------------------------
// this tests the floating point STW registers and their slopes
void floatSTW(SstRegs *sst, Triangle *t)
{
    randomTriangle(t,diago.tsize,1);		// pick random triangle
    areaTriangle(t);				// compute the area (before setup)
    sortTriangle(t);				// sort it
    randomFloatStwTriangle_Setup(t);		// random STW after setup
    printStwTriangle(4,t);
    printStwTriangleSlopes(5,t);

    // NOTE: if sub-pixel parameter adjustment is OFF we may get
    //	 color overflows/underflows etc
    if (iRandom(1) || diago.trexStandAlone || diago.writeFifo)
	drawFloatStwTriangle(sst,t);
    else
	drawAltFloatStwTriangle(sst,t);

    sst_idle(sst);				// wait for the command to complete
    checkTriangle(t,1,1,diago.adjust, insideStTriangle,0,0,0);
    eraseTriangle(sst,t,1,0,0);		// erase the triangle
}

void
main (int argc, char **argv)
{
    int n, format;
    SstRegs *sst;
    Triangle *t;

    sst = SST_BEGIN(argc,argv);
    diago.tsize /= 2;				// reduce the size by 2
    SET(sst->fbzMode, SST_RGBWRMASK);

    if(diago.bigAssTextures)
      t = buildTriangle(2048, 2048);
    else
      t = buildTriangle(256, 256);

    t->next = NULL;

    // NOTE: alternate regmapping should already be enabled via init code

    while (DIAG_STARTPASS()) {			// for each pass
	SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT |
    		SST_ENTEXTUREMAP |
		(diago.adjust?SST_PARMADJUST:0));
	// use the 8-bit 332 RGB texture format, size = 16x16, in replace mode
	format = SST_RGB332;
	t->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) | 
			format | SST_TC_REPLACE | SST_TCA_REPLACE;
	texRandomTextureMap(sst,diago.trex, 0,4,4, t->tex);
	for (n=0; n<30; n++) {			// do 30 tests
	    if (diago.bilinear) {			// if bilinear enabled
		if (diago.bilinear > 0)
		    t->tex->tMode |= SST_TMINFILTER | SST_TMAGFILTER;
		else if (iRandom(1))
		    t->tex->tMode ^= SST_TMINFILTER | SST_TMAGFILTER;
	    }
	    if (diago.clamp) {
		if (iRandom(1)) t->tex->tMode ^= SST_TCLAMPS;
		if (iRandom(1)) t->tex->tMode ^= SST_TCLAMPT;
		if (iRandom(1)) t->tex->tMode ^= SST_TCLAMPW;
	    }
	    gdbg_info(3,"bilin=%d  clamp s,t,w=%d,%d,%d\n",
			(t->tex->tMode & SST_TMINFILTER) != 0,
			(t->tex->tMode & SST_TCLAMPS) != 0,
			(t->tex->tMode & SST_TCLAMPT) != 0,
			(t->tex->tMode & SST_TCLAMPW) != 0);
	    SET(SST_TREX(sst,t->tex->trex)->textureMode,t->tex->tMode);
	    floatSTW(sst,t);			// float STW
	}
    }
    DIAG_PASS(0);
}

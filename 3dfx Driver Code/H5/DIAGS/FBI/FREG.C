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
** $Date: 10/11/00 8:10:11 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

unsigned long csrc=0xF0F0F0;

//---------------------------------------------------------------------
// here are the actual tests, each one is responsible for setting up
// the proper settings for fbzColorPath and fbzMode before it runs
// the state of these registers is trashed after the test is run
//---------------------------------------------------------------------
// this tests the floating point vertex regs, vA, vB, vC
void floatVertex(SstRegs *sst)
{
    static Triangle t;

    gdbg_info(2,"float vertex test\n");
    randomFloatTriangle(&t,diago.tsize,0);
    areaTriangle(&t);
    sortTriangle(&t);				// sort it
    printTriangle(3,&t,0,0,0);

    SET(sst->c1, csrc);
    SET(sst->fbzColorPath, SST_RGBSEL_C1 | SST_ASEL_C1);
    SET(sst->fbzMode, SST_ENRECTCLIP | SST_RGBWRMASK);
    drawFloatTriangle(sst,&t,0,0,0,0);

    sst_idle(sst);				// wait for the command to complete
    checkTriangle(&t,csrc,1,0, insideTriangle,0,0,0);
    eraseTriangle(sst,&t,1,0,0);		// erase the triangle
}

// this tests the floating point RGBA registers and their slopes
void floatRgba(SstRegs *sst)
{
    static Triangle t;
    int checkA = (diago.rgb==15) || diago.hasAuxBuffer;

    gdbg_info(2,"float rgba test\n");
    if (!checkA) {
	gdbg_printf("WARNING: skipping alpha test because there is no abuffer\n");
	SET(sst->fbzMode, SST_ENRECTCLIP | SST_RGBWRMASK);
    }
    else if (diago.rgb == 16)
	SET(sst->fbzMode, SST_ENRECTCLIP | SST_RGBWRMASK | SST_ZAWRMASK | SST_ENALPHABUFFER);
    else
	SET(sst->fbzMode, SST_ENRECTCLIP | SST_RGBWRMASK );
    SET(sst->fbzColorPath, SST_RGBSEL_RGBA | (diago.adjust?SST_PARMADJUST:0));
    randomTriangle(&t,diago.tsize,0);		// pick random triangle
    randomFloatRgbaTriangle(&t);		// with random float colors
    areaTriangle(&t);				// compute the area (before setup)
    setupFloatTriangle(&t,1,1,0,0);		// setup float RGBA slopes
    sortTriangle(&t);				// sort it
    printTriangle(2,&t,1,1,0);
    printTriangleSlopes(3,&t,1,1,0);
    drawFloatTriangle(sst,&t,1,1,0,0);

    sst_idle(sst);				// wait for the command to complete
    checkTriangle(&t,1,1,diago.adjust, insideTriangle,1,checkA,0);

    SET(sst->fbzColorPath, SST_RGBSEL_C1 | SST_ASEL_C1);	// use C1
    eraseTriangle(sst,&t,1,checkA,0);		// erase the triangle
    SET(sst->alphaMode, 0);
    SET(sst->drdx,0);				// prevent needless overflows
    SET(sst->dgdx,0);
    SET(sst->dbdx,0);
    SET(sst->dadx,0);
    SET(sst->drdy,0);
    SET(sst->dgdy,0);
    SET(sst->dbdy,0);
    SET(sst->dady,0);
}

// this tests the floating point Z register and its slopes
void floatZ(SstRegs *sst)
{
    static Triangle t;

    gdbg_info(2,"float z test\n");
    if (!diago.hasAuxBuffer) {
	gdbg_printf("WARNING: skipping float z test because there is no zbuffer\n");
	return;
    }
    randomTriangle(&t,diago.tsize,1);		// pick random triangle
    randomFloatZTriangle(&t);			// with random Z

    areaTriangle(&t);				// compute the area (before setup)
    setupFloatTriangle(&t,0,0,1,0);		// setup Z slopes
    sortTriangle(&t);				// sort it
    printTriangle(2,&t,0,0,1);
    printTriangleSlopes(3,&t,0,0,1);

    // NOTE: if sub-pixel parameter adjustment is OFF we may get
    //	 color overflows/underflows etc
    SET(sst->fbzMode, SST_ENRECTCLIP | SST_ENDEPTHBUFFER | SST_ZAWRMASK |
			SST_ZFUNC_LT | SST_ZFUNC_EQ | SST_ZFUNC_GT);
    SET(sst->fbzColorPath, diago.adjust?SST_PARMADJUST:0);
    drawFloatTriangle(sst,&t,0,0,1,0);

    sst_idle(sst);				// wait for the command to complete
    checkTriangle(&t,1,1,diago.adjust, insideTriangle,0,0,1);
    eraseTriangle(sst,&t,0,0,1);		// erase the triangle
}

// this tests the floating point W register and its slopes
void floatW(SstRegs *sst)
{
    static Triangle t;

    gdbg_info(2,"float w test\n");
    if (!diago.hasAuxBuffer) {
	gdbg_printf("WARNING: skipping float w test because there is no zbuffer\n");
	return;
    }
    SET(sst->fbzMode, SST_ENRECTCLIP | SST_ENDEPTHBUFFER | SST_ZAWRMASK |
			SST_WBUFFER |
			SST_ZFUNC_LT | SST_ZFUNC_EQ | SST_ZFUNC_GT);
    SET(sst->fbzColorPath, diago.adjust?SST_PARMADJUST:0);
    randomTriangle(&t,diago.tsize,1);		// pick random triangle
    randomFloatWTriangle(&t);			// with random W

    areaTriangle(&t);				// compute the area (before setup)
    setupFloatTriangle(&t,0,0,0,1);		// setup W slopes
    sortTriangle(&t);				// sort it
    printTriangle(2,&t,0,0,-1);
    printTriangleSlopes(3,&t,0,0,-1);

    // NOTE: if sub-pixel parameter adjustment is OFF we may get
    //	 color overflows/underflows etc
    drawFloatTriangle(sst,&t,0,0,0,1);

    sst_idle(sst);				// wait for the command to complete
    checkTriangle(&t,1,1,diago.adjust, insideTriangle,0,0,-1);
    eraseTriangle(sst,&t,0,0,-1);		// erase the triangle
}

void
main (int argc, char **argv)
{
    int n;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    diago.tsize /= 2;				// reduce the size by 2
    SET(sst->fbzMode, SST_ENRECTCLIP | SST_RGBWRMASK | SST_ZAWRMASK);
    // NOTE: we assume SST_BEGIN (or child) sets up rectclipper to full screen

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<20; n++) {			// do 20 tests
	floatVertex(sst);			// float vertex coords
	floatRgba(sst);				// float RGBA
	floatZ(sst);				// float Z
	floatW(sst);				// float W
    }
    DIAG_PASS(1);	// verify that offscreen pixels did not draw
}

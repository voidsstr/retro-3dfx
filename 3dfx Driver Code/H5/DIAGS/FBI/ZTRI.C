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
** $Date: 10/11/00 8:10:56 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int n;
    static Triangle t;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    SET(sst->c1, 0);				// used for erasing triangles
    SET(sst->fbzMode, SST_ENDEPTHBUFFER | SST_ZAWRMASK |
			SST_ZFUNC_LT | SST_ZFUNC_EQ | SST_ZFUNC_GT);
    SET(sst->fbzColorPath, SST_RGBSEL_RGBA | (diago.adjust?SST_PARMADJUST:0));

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<20; n++) {			// do 20 tests
	randomTriangle(&t,diago.tsize,1);	// pick random triangle
	randomZTriangle(&t);			// with random Z

	areaTriangle(&t);			// compute the area (before setup)
	setupTriangle(&t,0,0,1);		// setup Z slopes
	sortTriangle(&t);			// sort it
	printTriangle(2,&t,0,0,1);
	printTriangleSlopes(3,&t,0,0,1);

	// NOTE: if sub-pixel parameter adjustment is OFF we may get
	//	 color overflows/underflows etc
	drawTriangle(sst,&t,0,0,1);

	sst_idle(sst);				// wait for the command to complete
	checkTriangle(&t,1,1,diago.adjust, insideTriangle,0,0,1);

	eraseTriangle(sst,&t,0,0,1);		// erase the triangle
    }
    DIAG_PASS(1);				// check for black screen
}
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
    long x,y;
    FxU32 zsrc, zdst, wmask, stenMode;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    SET(sst->dzdx,0);
    SET(sst->dzdy,0);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<200; n++) {			// do 200 tests
	int zmax = (diago.rgb==32)? 0xFFFFFF : 0xFFFF;
	xyRandom(&x,&y);			// pick random x,y
	zsrc = iRandom(zmax);			// and random z
	zdst = iRandom(zmax);
	wmask = iRandom(1);			// and random writemask
	gdbg_info(2,"zsrc,zdst = 0x%06x, 0x%06x wmask = %d\n",
			zsrc,zdst, wmask);
	DIAG_FORCE_PIXEL(CSIM_BUF_3D_AUX1,x,y,zdst); // set (x,y) = zdst

	if (diago.rgb == 32) {
	    if (n%50 == 0)
		stenMode = 0;
	    else if (iRandom(10)==0)
		stenMode = iRandom(0xFFFFFFF) & ~SST_STENCIL_ENABLE;
	    SET(sst->stencilMode, stenMode);
	}
	// set the writemask bit and draw a pixel using any command
	SET(sst->fbzMode, SST_ENDEPTHBUFFER | (wmask ? SST_ZAWRMASK : 0) |
			SST_ZFUNC_LT | SST_ZFUNC_EQ | SST_ZFUNC_GT);
	SET(sst->z,zsrc<<SST_Z_FRACBITS);
	sst_drawpixel(sst,x,y,0,0);

	// now merge in stencil planes if writemask is set
	if (stenMode & SST_STENCIL_WMASK) {
	    zsrc |= ((stenMode & SST_STENCIL_REF)<<(24-SST_STENCIL_REF_SHIFT)) &
		    ((stenMode & SST_STENCIL_WMASK)<<(24-SST_STENCIL_WMASK_SHIFT));
	    zdst |= ((stenMode & SST_STENCIL_REF)<<(24-SST_STENCIL_REF_SHIFT)) &
		    ((stenMode & SST_STENCIL_WMASK)<<(24-SST_STENCIL_WMASK_SHIFT));
	}
	sst_idle(sst);				// wait for the command to complete
	DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1, x,y, wmask ? zsrc : zdst);
    }
    DIAG_PASS(0);
}

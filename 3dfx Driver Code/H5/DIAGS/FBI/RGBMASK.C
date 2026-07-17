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
** $Date: 10/11/00 8:10:27 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int n;
    long x,y;
    FxU32 csrc, cdst, wmask, rgba32mask, rmode;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    rmode = GET(sst->renderMode);
    rmode &= ~(SST_RM_RED_WMASK | SST_RM_GREEN_WMASK | 
		SST_RM_BLUE_WMASK | SST_RM_ALPHA_WMASK);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<200; n++) {			// do 200 tests
	xyRandom(&x,&y);			// pick random x,y
	csrc = colRandom32();			// and random colors
	cdst = colRandom32();
	if (n%50 == 0) SET(sst->stencilMode, 0);
	wmask = iRandom(1);			// and random writemask
	gdbg_info(2,"csrc,cdst = 0x%x, 0x%x wmask = %d\n", csrc,cdst,wmask);

	// set the writemask bit and draw a pixel using any command
	SET(sst->fbzMode,(wmask ? SST_RGBWRMASK : 0)  | drawbufferRandom());
	DIAG_FORCE_PIXEL(diago.curdrawbuffer,x,y,cdst);// set (x,y) = cdst
	if (diago.rgb == 32) {
	    if (iRandom(10)==0)
		SET(sst->stencilMode, iRandom(0xFFFFFFF) & ~SST_STENCIL_ENABLE);
	    rgba32mask = 0;
	    if (iRandom(1)) rgba32mask |= SST_RM_RED_WMASK;
	    if (iRandom(1)) rgba32mask |= SST_RM_GREEN_WMASK;
	    if (iRandom(1)) rgba32mask |= SST_RM_BLUE_WMASK;
	    if (iRandom(1)) rgba32mask |= SST_RM_ALPHA_WMASK;
	    SET(sst->renderMode,rmode | rgba32mask);
	    gdbg_info(2,"rgba masks = %d%d%d%d\n", 
			(rgba32mask & SST_RM_RED_WMASK) != 0,
			(rgba32mask & SST_RM_GREEN_WMASK) != 0,
			(rgba32mask & SST_RM_BLUE_WMASK) != 0,
			(rgba32mask & SST_RM_ALPHA_WMASK) != 0 );

	    // now modify prediction which is in csrc
	    if (!(rgba32mask & SST_RM_RED_WMASK))
		csrc = (csrc & ~0xFF0000) | (cdst & 0xFF0000);
	    if (!(rgba32mask & SST_RM_GREEN_WMASK))
		csrc = (csrc & ~0xFF00) | (cdst & 0xFF00);
	    if (!(rgba32mask & SST_RM_BLUE_WMASK))
		csrc = (csrc & ~0xFF) | (cdst & 0xFF);
	    if (!(rgba32mask & SST_RM_ALPHA_WMASK))
		csrc = (csrc & ~0xFF000000) | (cdst & 0xFF000000);
	}
	sst_drawpixel(sst,x,y,csrc,1);

	sst_idle(sst);				// wait for the command to complete
	csrc = sst_argb_form_result(csrc,0,0,0);
	cdst = sst_argb_form_result(cdst,0,0,0);
	DIAG_TEST_PIXEL(diago.curdrawbuffer, x,y, wmask ? csrc: cdst);
    }
    DIAG_PASS(0);
}
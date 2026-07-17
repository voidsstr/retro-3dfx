/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:10:33 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int n;
    long x,y;
    FxU32 stref, stdst, stmode, stmask;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    if (diago.rgb < 32) {
	gdbg_printf("INFO: stmask should run with -7 option (32bpp)\n");
    }
    
    SET(sst->stencilOp, (SST_SOP_REPLACE<<SST_STENCIL_SFAIL_OP_SHIFT) |
			(SST_SOP_REPLACE<<SST_STENCIL_ZFAIL_OP_SHIFT) |
			(SST_SOP_REPLACE<<SST_STENCIL_ZPASS_OP_SHIFT));
    // start off with both enabled
    SET(sst->fbzMode, SST_RGBWRMASK|SST_ZAWRMASK);

    while (DIAG_STARTPASS())			// for each pass
    if (diago.rgb == 32)			// only do something in 32bpp mode
    for (n=0; n<800; n++) {			// do a lot of tests!
	xyRandom(&x,&y);			// pick random x,y

	if (n < 256) {			// make sure each bit can be set
	    stdst = 0;
	    stmode = SST_STENCIL_ENABLE;	// always fail
	    stmask = n;
	    stref = 0xFF;
	    stmode |= stmask<<SST_STENCIL_WMASK_SHIFT;
	    gdbg_info(2,"stencilMode = 0x%x\n", stmode);
	    SET(sst->stencilMode, stmode | (stref<<SST_STENCIL_REF_SHIFT));
	}
	else if (n < 512) {		// make sure each bit can be cleared
	    stdst = 0xFF;
	    stmode = SST_STENCIL_ENABLE;	// always fail
	    stmask = n&0xFF;
	    stref = 0x00;
	    stmode |= stmask<<SST_STENCIL_WMASK_SHIFT;
	    gdbg_info(2,"stencilMode = 0x%x\n", stmode);
	    SET(sst->stencilMode, stmode | (stref<<SST_STENCIL_REF_SHIFT));
	}
	else if (iRandom(5)==0) {		// every once in a while
	    stdst = iRandom(0xFF);
	    // totally random stencilMode, set enable, clear writemask
	    stmode = iRandom(0xFFFFFFFF) | SST_STENCIL_ENABLE;
	    stmode &= ~SST_STENCIL_WMASK;
	    // fetch ref from random stmode and create random writemask
	    stref = (stmode & SST_STENCIL_REF)>>SST_STENCIL_REF_SHIFT;
	    stmask = iRandom(0xFF);
	    stmode |= stmask<<SST_STENCIL_WMASK_SHIFT;
	    SET(sst->stencilMode, stmode);
	    gdbg_info(2,"stencilMode = 0x%x\n", stmode);
	}
	gdbg_info(2,"stref,stdst = 0x%02x, 0x%02x wmask = %02x\n", stref,stdst, stmask);
	DIAG_FORCE_PIXEL(CSIM_BUF_3D_AUX1,x,y,stdst<<24);
	if (iRandom(7)==0) {
	    // also set random ZAWRMASK (shouldn't effect anything)
	    SET(sst->fbzMode, iRandom(0xFFF) & (SST_RGBWRMASK|SST_ZAWRMASK));
	}

	sst_drawpixel(sst,x,y,0,0);

	sst_idle(sst);				// wait for the command to complete
	DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1, x,y, 
		((stdst & ~stmask) | (stref & stmask))<<24);
    }
    DIAG_PASS(0);
}
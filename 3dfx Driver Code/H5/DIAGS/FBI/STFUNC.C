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
** $Date: 10/11/00 8:10:31 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int n,j;
    long x,y;
    FxU32 stref, stmask, stfunc;	// random settings
    FxU32 stdst, stmode, stresult;
    FxU32 fbz,c1;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    if (diago.rgb < 32) {
	gdbg_printf("INFO: stfunc should run with -7 option (32bpp)\n");
    }
    // set things up to increment and wrap on stencil fail
    // and negate on stencil pass (and z pass, since it's disabled)
    SET(sst->stencilOp, (SST_SOP_INC<<SST_STENCIL_SFAIL_OP_SHIFT) |
			(SST_SOP_ZERO<<SST_STENCIL_ZFAIL_OP_SHIFT) |
			(SST_SOP_NEG<<SST_STENCIL_ZPASS_OP_SHIFT));

    while (DIAG_STARTPASS())			// for each pass
    if (diago.rgb == 32)			// only do something in 32bpp mode
    for (n=0; n<50; n++) {			// do a 50 of tests!
	xyRandom(&x,&y);			// pick random x,y
	stref = rRandom(100,200);		// random stencil reference value
	stdst = 0;
	DIAG_FORCE_PIXEL(CSIM_BUF_3D_AUX1,x,y,stdst<<24);
	c1 = colRandom32();

	if (iRandom(7)==0 || n==0) {
	    // also set random ZAWRMASK (shouldn't effect anything)
	    fbz  =  drawbufferRandom();
	    if (iRandom(1)) fbz |= SST_RGBWRMASK;
	    if (iRandom(1)) fbz |= SST_ZAWRMASK;
	    SET(sst->fbzMode, fbz);
	}

	stmode = SST_STENCIL_ENABLE;
	if (n <= 8) {				// test each possible 1-bit mask
	    stmask = (1<<n) & 0xFF;
	}
	else {
	    stmask = iRandom(0xFF);		// totally random mask
	}
	stmode |= 0xFF<<SST_STENCIL_WMASK_SHIFT;	// solid writemask
	stmode |= stref<<SST_STENCIL_REF_SHIFT;		// or in ref
	stmode |= stmask<<SST_STENCIL_MASK_SHIFT;	// or in mask
	for (j=0;j<8;j++) {			// for each possible func
	    stfunc = j<<SST_STENCIL_FUNC_SHIFT;
	    gdbg_info(2,"---new test---\n");
	    gdbg_info(2,"stencilMode = 0x%x , stfunc,ref,mask=%02x %02x %02x\n",
	    	stmode|stfunc,stfunc>>SST_STENCIL_FUNC_SHIFT,stref,stmask);
	    SET(sst->stencilMode, stmode|stfunc);
	    sst_drawpixel(sst,x,y,c1,0);

	    sst_idle(sst);				// wait for the command to complete
	    if (stfunc & SST_SFUNC_LT)
		if ((stref & stmask) < (stdst & stmask)) goto stpass;
	    if (stfunc & SST_SFUNC_EQ)
		if ((stref & stmask) == (stdst & stmask)) goto stpass;
	    if (stfunc & SST_SFUNC_GT)
		if ((stref & stmask) > (stdst & stmask)) goto stpass;
	    // Oops, stencil failed
	    stresult = stdst+1;
	    stresult &= 0xFF;
	    gdbg_info(3,"stencil FAIL, stresult=%02x\n",stresult);
	    DIAG_TEST_PIXEL(diago.curdrawbuffer, x,y, 0);
	    goto merge;
	stpass:
	    stresult = ~stdst;
	    stresult &= 0xFF;
	    gdbg_info(3,"stencil PASS, stresult=%02x\n",stresult);
	    DIAG_TEST_PIXEL(diago.curdrawbuffer, x,y, fbz&SST_RGBWRMASK? c1:0);
	    DIAG_FORCE_PIXEL(diago.curdrawbuffer,x,y,0);
	merge:
	    DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1, x,y, stresult<<24);
	    stdst = stresult;
	}
    }
    DIAG_PASS(0);
}
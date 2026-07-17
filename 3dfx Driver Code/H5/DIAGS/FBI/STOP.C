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

FxU32 stOp(FxU32 stdst, FxU32 stop, FxU32 stref)
{
    switch (stop) {
	case SST_SOP_KEEP:	return stdst;
	case SST_SOP_ZERO:	return 0;
	case SST_SOP_REPLACE:	return stref;
	case SST_SOP_INCSAT:	return stdst==0xFF ? stdst : stdst+1;
	case SST_SOP_DECSAT:	return stdst==0x00 ? stdst : stdst-1;
	case SST_SOP_NEG:	return ~stdst;
	case SST_SOP_INC:	return stdst+1;
	case SST_SOP_DEC:	return stdst-1;
    }
}

void
main (int argc, char **argv)
{
    int j,n;
    long x,y,z;
    FxU32 stref,stdst, stop, stz, fbz;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    if (diago.rgb < 32) {
	gdbg_printf("INFO: stop should run with -7 option (32bpp)\n");
    }
    
    while (DIAG_STARTPASS())			// for each pass
    if (diago.rgb == 32)			// only do something in 32bpp mode
    for (n=0; n<50; n++) {			// do 50 iterations
	xyRandom(&x,&y);			// pick random x,y
	stref = iRandom(0xFF);
	z = iRandom(0xFFFFFF);
	SET(sst->z,z);

	// test stencil fail operation
	SET(sst->stencilMode, SST_STENCIL_ENABLE |
			(0xFF<<SST_STENCIL_WMASK_SHIFT)|// solid writemask
			(0xFF<<SST_STENCIL_MASK_SHIFT)|	// solid mask
			(stref<<SST_STENCIL_REF_SHIFT)|	// or in ref
			SST_SFUNC_EQ);
	
	do stdst = iRandom(0xFF); while (stdst == stref);
	DIAG_FORCE_PIXEL(CSIM_BUF_3D_AUX1,x,y,stdst<<24);
	gdbg_info(2,"stref,stdst = 02x, %02x\n", stref,stdst);

	for (j=0;j<8;j++) {			// for each possible operation
	    stop = j;
	    gdbg_info(2,"---new SFAIL test---\n");

	    if (stdst == stref) {
		// switch to stfunc=never
		SET(sst->stencilMode, SST_STENCIL_ENABLE |
			(0xFF<<SST_STENCIL_WMASK_SHIFT)|
			(0xFF<<SST_STENCIL_MASK_SHIFT)|
			(stref<<SST_STENCIL_REF_SHIFT));
	    }
	    SET(sst->stencilOp, (stop<<SST_STENCIL_SFAIL_OP_SHIFT) |
				(((stop+1)&7)<<SST_STENCIL_ZFAIL_OP_SHIFT) |
				(((stop-1)&7)<<SST_STENCIL_ZPASS_OP_SHIFT));
	    sst_drawpixel(sst,x,y,0,0);

	    sst_idle(sst);			// wait for the command to complete
	    gdbg_info(3,"stencil FAIL, op=%02x\n",stop);
	    
	    stdst = 0xFF & stOp(stdst,stop,stref);
	    DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1, x,y, stdst<<24);
	}
	// switch to stfunc=always for last two tests
	SET(sst->stencilMode, SST_STENCIL_ENABLE |
			(0xFF<<SST_STENCIL_WMASK_SHIFT)|
			(0xFF<<SST_STENCIL_MASK_SHIFT)|
			(stref<<SST_STENCIL_REF_SHIFT) |
			(SST_SFUNC_LT|SST_SFUNC_EQ|SST_SFUNC_GT));


	// test zfail operation, enable depthbuffer with func=never
	SET(sst->fbzMode, (iRandom(0xFFF) & (SST_RGBWRMASK|SST_ZAWRMASK)) |
		SST_ENDEPTHBUFFER);
	for (j=0;j<8;j++) {			// for each possible operation
	    stop = j;
	    gdbg_info(2,"---new ZFAIL test---\n");

	    SET(sst->stencilOp, (((stop-1)&7)<<SST_STENCIL_SFAIL_OP_SHIFT) |
				(stop<<SST_STENCIL_ZFAIL_OP_SHIFT) |
				(((stop+1)&7)<<SST_STENCIL_ZPASS_OP_SHIFT));
	    sst_drawpixel(sst,x,y,0,0);

	    sst_idle(sst);			// wait for the command to complete
	    gdbg_info(3,"z FAIL, op=%02x\n",stop);
	    
	    stdst = 0xFF & stOp(stdst,stop,stref);
	    DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1, x,y, stdst<<24);
	}


	// test zpass operation, disable depthbuffer
	fbz = iRandom(0xFFF) & (SST_RGBWRMASK|SST_ZAWRMASK);
	SET(sst->fbzMode, fbz | SST_ENDEPTHBUFFER | SST_ZFUNC);
	for (j=0;j<8;j++) {			// for each possible operation
	    stop = j;
	    gdbg_info(2,"---new ZPASS test---\n");

	    SET(sst->stencilOp, (((stop-1)&7)<<SST_STENCIL_SFAIL_OP_SHIFT) |
				(((stop+1)&7)<<SST_STENCIL_ZFAIL_OP_SHIFT) |
				(stop<<SST_STENCIL_ZPASS_OP_SHIFT));
	    sst_drawpixel(sst,x,y,0,0);

	    sst_idle(sst);			// wait for the command to complete
	    gdbg_info(3,"z PASS, op=%02x\n",stop);
	    
	    stdst = 0xFF & stOp(stdst,stop,stref);
	    stz = stdst << 24;
	    if (fbz & SST_ZAWRMASK)		// if Z writemask set
		stz |= z>>SST_Z_32BPP_FRACBITS;	// merge in z bits
	    DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1, x,y, stz);
	}

	if (iRandom(7)==0) {
	    // also set random ZAWRMASK
	    fbz = iRandom(0xFFF) & (SST_RGBWRMASK|SST_ZAWRMASK);
	    SET(sst->fbzMode, fbz | SST_ENDEPTHBUFFER | SST_ZFUNC);
	}
    }
    DIAG_PASS(0);
}
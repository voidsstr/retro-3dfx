/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:10:50 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

// simplifying print macro for printing colors in .12 format
#define PW(c)	printFix("%4d.%03x",c,SST_RGBA_FRACBITS)
#define S	(1<<SST_RGBA_FRACBITS)

void
main (int argc, char **argv)
{
    int i,n, spanlen, clamp;
    long x,y;
    long w, dw;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<100; n++) {			// do 100 tests
	clamp = iRandom(1);			// random clamp or modulo
	do {
	    xyRandom(&x,&y);			// pick random x,y
	    spanlen = iRandom(42) + 1;		// and random span length
	} while (x + spanlen > diago.xmaxscreen);
	gdbg_info(2,"span %d,%d len=%d %s\n",x,y,spanlen, clamp?"clamp":"mod");

	w = rRandom(-10*S,270*S);		// random W in [-10,270] range
	dw = rRandom(-10*S,10*S);		// +-10
	if (diago.rgb == 16)
	    SET(sst->fbzMode, SST_ENALPHABUFFER | SST_ZAWRMASK);
	else
	    SET(sst->fbzMode, SST_RGBWRMASK);
	SET(sst->fbzColorPath, SST_ALOCAL_W | SST_CCA_REPLACE |
				(clamp ? SST_RGBAZ_CLAMP : 0));
	SETF(sst->Fw, w/(float)S);
	SETF(sst->Fdwdx,dw/(float)S);
	w = float2fix(w/(float)S,SST_RGBA_FRACBITS);
	dw = float2fix(dw/(float)S,SST_RGBA_FRACBITS);
	gdbg_info(3,"w = %s dwdx = %s\n",PW(w),PW(dw));

	sst_drawspan(sst,x,y,spanlen);		// now draw the span
	sst_idle(sst);				// wait for the command to complete
	for (i=0; i<spanlen; i++) {
	    long cw;

	    gdbg_info(4,"\tw = %s\n",PW(w));
	    cw = w >> SST_RGBA_FRACBITS;
	    if (clamp) {
		if (cw > 0xFF) cw = 0xFF;
		if (cw < 0) cw = 0;
	    }
	    else {				// else modulo to 256
	        if (cw == 0x100) cw = 0xFF;	// check for 1 unit overflow
	        if (cw == 0xFFFFFFFF) cw= 0;	// check for 1 unit underflow
	    }
	    cw &= 0xFF;

	    if (diago.rgb == 16) {
		DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1, x+i,y, cw);
		DIAG_FORCE_PIXEL(CSIM_BUF_3D_AUX1,x+i,y,0); // reset the pixel
	    }
	    else {
		cw &= diago.rgb==15 ? 0x80 : 0xFF;
		DIAG_TEST_PIXEL(diago.curdrawbuffer,x+i,y,cw<<24);
	    }
	    w += dw;				// increment w
	}
    }
    DIAG_PASS(0);
}

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
** $Date: 10/11/00 8:09:56 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

// simplifying print macro for printing colors in .12 format
#define PC(c)	printFix("%4d.%03x",c,SST_RGBA_FRACBITS)
#define S	(1<<SST_RGBA_FRACBITS)

void
main (int argc, char **argv)
{
    int i,n, spanlen, clamp;
    long x,y;
    long a, da;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    if(diago.aaEnabled)
      {
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "Warning! Can't run aclamp with AA. Just going to pass the f'er\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	DIAG_PASS(0);
      }


    SET(sst->r, 0);
    SET(sst->g, 0);
    SET(sst->b, 0);
    SET(sst->drdx, 0);
    SET(sst->dgdx, 0);
    SET(sst->dbdx, 0);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<100; n++) {			// do 100 tests
	clamp = iRandom(1);			// random clamp or modulo
	do {
	    xyRandom(&x,&y);			// pick random x,y
	    spanlen = iRandom(42) + 1;		// and random span length
	} while (x + spanlen > diago.xmaxscreen);
	gdbg_info(2,"span %d,%d len=%d %s\n",x,y,spanlen, clamp?"clamp":"mod");

	a = rRandom(-5*S,260*S);		// random Alpha in [-5,260] range
	da = rRandom(-5*S,5*S);			// +-5
	if (diago.rgb==16)
	    SET(sst->fbzMode, SST_ENALPHABUFFER | SST_ZAWRMASK);
	else
	    SET(sst->fbzMode, SST_RGBWRMASK);
	SET(sst->fbzColorPath, (clamp ? SST_RGBAZ_CLAMP : 0));
	SET(sst->a, a);
	SET(sst->dadx,da);
	gdbg_info(3,"a = %s dadx = %s\n",PC(a),PC(da));

	sst_drawspan(sst,x,y,spanlen);		// now draw the span
	sst_idle(sst);				// wait for the command to complete
	for (i=0; i<spanlen; i++) {
	    long ca;

	    ca = a >> SST_RGBA_FRACBITS;
	    if (clamp) {
		if (ca > 0xFF) ca = 0xFF;
		if (ca < 0) ca = 0;
	    }
	    else {				// else modulo to 256
	        if (ca == 0x100) ca = 0xFF;	// check for 1 unit overflow
	        if (ca == 0xFFFFFFFF) ca = 0;	// check for 1 unit underflow
	    }
	    ca &= 0xFF;

	    if (diago.rgb==16) {
		DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1,x+i,y, (unsigned short)ca);
		DIAG_FORCE_PIXEL(CSIM_BUF_3D_AUX1,x+i,y,0); // reset the pixel
	    }
	    else {
		ca &= diago.rgb==15 ? 0x80 : 0xFF;
		DIAG_TEST_PIXEL(diago.curdrawbuffer,x+i,y,ca<<24);
	    }
	    a += da;				// increment alpha
	}
    }
    DIAG_PASS(0);
}

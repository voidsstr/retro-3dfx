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
** $Date: 10/11/00 8:10:53 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

// simplifying print macro for printing z values in .16 format
#define PZ(z)	printFix(diago.rgb==32?"%7x.%01x":"%5x.%03x",z,SST_Z_FRACBITS)
#define S	(1<<SST_Z_FRACBITS)

void
main (int argc, char **argv)
{
    int i,n, spanlen, clamp;
    long x,y;
    long z,dz;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<50; n++) {			// do 50 tests
	int zmax = (diago.rgb==32)? 0xFFFFFF : 0xFFFF;
	clamp = iRandom(1);			// random clamp or modulo
	do {
	    xyRandom(&x,&y);			// pick random x,y
	    spanlen = iRandom(45) + 1;		// and random span length
	} while (x + spanlen > diago.xmaxscreen);
	gdbg_info(2,"span %d,%d len=%d %s\n",x,y,spanlen, clamp?"clamp":"mod");

	z = rRandom(-10*S,10*S);	// random z in [-10,+10] range
	if (n&1) z+= zmax*S;		// optionally add 16K
	dz = rRandom(-5*S,5*S);		// +-5
	SET(sst->fbzMode, SST_ENDEPTHBUFFER | SST_ZAWRMASK |
			SST_ZFUNC_LT | SST_ZFUNC_EQ | SST_ZFUNC_GT );
	SET(sst->fbzColorPath, (clamp ? SST_RGBAZ_CLAMP : 0));
	SET(sst->z,z);
	SET(sst->dzdx,dz);
	gdbg_info(3,"   z = %s\n", PZ(z));
	gdbg_info(3,"dzdx = %s\n", PZ(dz));

	sst_drawspan(sst,x,y,spanlen);		// now draw the span
	sst_idle(sst);				// wait for the command to complete
	for (i=0; i<spanlen; i++) {
	    long cz;

	    cz = z >> SST_Z_FRACBITS;
	    if (clamp) {
		if (cz > zmax) cz = zmax;
		if (cz < 0) cz = 0;
	    }
	    else {				// else modulo to 256
	        if (cz == zmax+1) cz = zmax;	// check for 1 unit overflow
	        if (cz == 0xFFFFFFFF) cz = 0;	// check for 1 unit underflow
		cz &= zmax;
	    }
	    DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1, x+i,y, cz);
	    z += dz;
	}
    }
    DIAG_PASS(0);
}

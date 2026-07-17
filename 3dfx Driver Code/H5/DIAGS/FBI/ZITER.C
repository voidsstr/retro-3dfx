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
** $Date: 10/11/00 8:10:55 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

// simplifying print macro for printing z values in .16 format
#define PZ(z)	printFix(diago.rgb==32?"%7x.%01x":"%5x.%03x",z,SST_Z_FRACBITS)
#define S	(1<<SST_Z_FRACBITS)

void
main (int argc, char **argv)
{
    int i,n, spanlen;
    long x,y;
    long z,dz;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    SET(sst->fbzMode, SST_ENDEPTHBUFFER | SST_ZAWRMASK |
			SST_ZFUNC_LT | SST_ZFUNC_EQ | SST_ZFUNC_GT);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<50; n++) {			// do 50 tests
	do {
	    xyRandom(&x,&y);			// pick random x,y
	    spanlen = iRandom(39) + 1;		// and random span length
	} while (x + spanlen > diago.xmaxscreen);
	gdbg_info(2,"span %d,%d len=%d\n",x,y,spanlen);

	// NOTE: we do not allow any overflows or underflows on the colors
	if (diago.rgb==32)
	    z = rRandom(7000*S,0x7FFFFF*S);		// random z in [7000,24-bit] range
	else
	    z = rRandom(7000*S,9000*S);		// random z in [7000,9000] range
	dz = rRandom(-140*S,140*S);		// +-140
	SET(sst->z,z);
	SET(sst->dzdx,dz);

	gdbg_info(3,"   z = %s\n", PZ(z));
	gdbg_info(3,"dzdx = %s\n", PZ(dz));

	sst_drawspan(sst,x,y,spanlen);		// now draw the span
	sst_idle(sst);				// wait for the command to complete
	for (i=0; i<spanlen; i++) {
	    DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1,x+i,y, z>>SST_Z_FRACBITS);
	    z += dz;
	}
    }
    DIAG_PASS(0);
}

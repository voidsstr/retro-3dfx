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
** $Date: 10/11/00 8:10:52 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

// NOTE: for this diagnostic we assume that applying zbias does
//	 not have any side effect on zbuffering, ie. if zbuffering
//	 works without zbias, then it works with zbias; zbias only
//	 effects the z value going into the zbuffer compare logic

// simplifying print macro for printing z values in .16 format
#define PZ(z)	printFix(diago.rgb==32?"%7x.%01x":"%5x.%03x",z,SST_Z_FRACBITS)
#define S	(1<<SST_Z_FRACBITS)

void
main (int argc, char **argv)
{
    int i,n, spanlen;
    long x,y;
    long z,dz, zbias;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    SET(sst->fbzMode, SST_ENDEPTHBUFFER | SST_ZAWRMASK | SST_ENZBIAS |
			SST_ZFUNC_LT | SST_ZFUNC_EQ | SST_ZFUNC_GT);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<50; n++) {			// do 50 tests
	int zmax = (diago.rgb==32)? 0xFFFFFF : 0xFFFF;
	do {
	    xyRandom(&x,&y);			// pick random x,y
	    spanlen = iRandom(29) + 1;		// and random span length
	} while (x + spanlen > diago.xmaxscreen);
	zbias = iRandom(32)-16;			// +- 16
	gdbg_info(2,"span %d,%d len=%d  zbias = %d(0x%x)\n",
			x,y,spanlen, zbias,zbias&zmax);

	z = rRandom(-10*S,10*S);	// random z in [-10,+10] range
	if (n&1) z+= zmax*S;
	dz = rRandom(-5*S,5*S);		// +-5
	SET(sst->z,z);
	SET(sst->dzdx,dz);
	SET(sst->zaColor,zbias);		// zbias goes here
	gdbg_info(3,"   z = %s\n", PZ(z));
	gdbg_info(3,"dzdx = %s\n", PZ(dz));

	sst_drawspan(sst,x,y,spanlen);		// now draw the span
	sst_idle(sst);				// wait for the command to complete
	for (i=0; i<spanlen; i++) {
	    long cz;

	    cz = z >> SST_Z_FRACBITS;
	    if (cz == zmax+1) cz = zmax;	// check for 1 unit overflow
	    if (cz == 0xFFFFFFFF) cz = 0;	// check for 1 unit underflow
	    cz &= zmax;				// take modulo

	    cz += zbias;			// now add in the bias
	    if (cz > zmax) cz = zmax;		// and clamp again
	    if (cz < 0) cz = 0;

	    DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1, x+i,y, cz);
	    z += dz;
	}
    }
    DIAG_PASS(0);
}

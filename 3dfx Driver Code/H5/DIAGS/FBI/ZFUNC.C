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
** $Date: 10/11/00 8:10:54 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

// simplifying print macro for printing z values in .16 format
#define PZ(z)	printFix(diago.rgb==32?"%7x.%01x":"%5x.%03x",z,SST_Z_FRACBITS)
#define S	(1<<SST_Z_FRACBITS)

void
main (int argc, char **argv)
{
    int i,n, spanlen, zfunc;
    long x,y;
    long z,dz, oldz, doldz, zac;
    FxU32 c = 0x808080, oldc = 0x404040;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    SET(sst->drdx,0);
    SET(sst->dgdx,0);
    SET(sst->dbdx,0);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<50; n++) {			// do 50 tests
	int zmax = (diago.rgb==32)? 0xFFFFFF : 0xFFFF;
	do {
	    xyRandom(&x,&y);			// pick random x,y
	    spanlen = iRandom(18) + 1;		// and random span length
	} while (x + spanlen > diago.xmaxscreen);
	gdbg_info(2,"span %d,%d len=%d\n",x,y,spanlen);

	// NOTE: we do not allow any overflows or underflows on the Zs
	z = rRandom(2000*S,(zmax-2000)*S);	// random z in [2000,max-2k] range
	dz = rRandom(-2*S,2*S);			// +-2
	oldz = z + rRandom(-3*S,3*S);		// keep OLDZ within 3 of Z
	doldz = rRandom(-1*S,1*S);		// +=1

	// now draw the old span first, in color=oldc and z=oldz
	SET(sst->r, oldc << SST_RGBA_FRACBITS);
	SET(sst->g, oldc << SST_RGBA_FRACBITS);
	SET(sst->b, oldc << SST_RGBA_FRACBITS);
	SET(sst->fbzMode, SST_ENDEPTHBUFFER | SST_RGBWRMASK | SST_ZAWRMASK |
			SST_ZFUNC_LT | SST_ZFUNC_EQ | SST_ZFUNC_GT);
	SET(sst->z,oldz);
	SET(sst->dzdx,doldz);
	sst_drawspan(sst,x,y,spanlen);		// now draw the span

	zfunc  = iRandom(1) ? SST_ZFUNC_LT : 0;
	zfunc |= iRandom(1) ? SST_ZFUNC_EQ : 0;
	zfunc |= iRandom(1) ? SST_ZFUNC_GT : 0;
	if (iRandom(2) == 0) {			// special "cockpit" bit
	    zfunc |= SST_ZCOMPARE_TO_ZACOLOR;
	    zac = (oldz >> SST_Z_FRACBITS) + rRandom(-1,1);
	    SET(sst->zaColor, zac);
	}
	SET(sst->fbzMode, SST_ENDEPTHBUFFER | SST_RGBWRMASK | SST_ZAWRMASK | zfunc);
	SET(sst->z,z);
	SET(sst->dzdx,dz);
	SET(sst->r, c << SST_RGBA_FRACBITS);
	SET(sst->g, c << SST_RGBA_FRACBITS);
	SET(sst->b, c << SST_RGBA_FRACBITS);
	gdbg_info(2,"zfunc = %d %s%s%s %s\n",(zfunc&SST_ZFUNC)>>SST_ZFUNC_SHIFT, 
			zfunc & SST_ZFUNC_LT ? "<" : " ",
			zfunc & SST_ZFUNC_GT ? ">" : " ",
			zfunc & SST_ZFUNC_EQ ? "=" : " ",
			zfunc & SST_ZCOMPARE_TO_ZACOLOR ? "->zaColor" : " ");
	gdbg_info(3,"   z = %s   oldz = %s  zac = %04x\n", PZ(z),PZ(oldz),zac);
	gdbg_info(3,"dzdx = %s  dozdx = %s\n", PZ(dz),PZ(doldz));

	sst_drawspan(sst,x,y,spanlen);		// now draw the span
	sst_idle(sst);				// wait for the command to complete
	for (i=0; i<spanlen; i++) {
	    FxU32 cz = (FxU32)(z >> SST_Z_FRACBITS);
	    FxU32 coldz = (FxU32)(oldz >> SST_Z_FRACBITS);
	    FxU32 testz;		// z to compare with

	    testz = zfunc & SST_ZCOMPARE_TO_ZACOLOR ? zac : cz;
	    if (zfunc & SST_ZFUNC_LT	)	// perform Z Function
		if (testz < coldz) goto ok;
	    if (zfunc & SST_ZFUNC_EQ)
		if (testz == coldz) goto ok;
	    if (zfunc & SST_ZFUNC_GT)
		if (testz > coldz) goto ok;
	    // zfunc failed, test both color and z for old values
	    DIAG_TEST_PIXEL(diago.curdrawbuffer,x+i,y, oldc);
	    DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1,x+i,y, coldz);
	    goto merge;
	ok:
	    // zfunc passed, test both color and z for new values
	    DIAG_TEST_PIXEL(diago.curdrawbuffer,x+i,y, c);
	    DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1,x+i,y, cz);
	merge:
	    z += dz;
	    oldz += doldz;
	}
    }
    DIAG_PASS(0);
}

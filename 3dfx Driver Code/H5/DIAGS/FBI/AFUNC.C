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
** $Date: 10/11/00 8:09:58 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

// simplifying print macro for printing colors in .12 format
#define PC(c)	printFix("%4d.%03x",c,SST_RGBA_FRACBITS)
#define S	(1<<SST_RGBA_FRACBITS)

void
main (int argc, char **argv)
{
    int i,n, spanlen, afunc,aref;
    long x,y;
    long r,g,b,a, dr,dg,db,da;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    if(diago.aaEnabled)
      {
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "Warning! Can't run afunc with AA. Just going to pass the f'er\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	DIAG_PASS(0);
      }


    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<50; n++) {			// do 50 tests
	// NOTE: we do not allow any overflows or underflows on the colors
	r = rRandom(100*S,156*S);		// random rgba in [100,156] range
	g = rRandom(100*S,156*S);		// in .12 format
	b = rRandom(100*S,156*S);
	a = rRandom(100*S,156*S);
	dr = rRandom(-2*S,2*S);			// +-2
	dg = rRandom(-1*S,1*S);			// +-1
	db = rRandom(-S/2,S/2);			// +-.5
	da = rRandom(-2*S,2*S);			// +-2
	
	afunc  = iRandom(1) ? SST_ALPHAFUNC_LT : 0;
	afunc |= iRandom(1) ? SST_ALPHAFUNC_EQ : 0;
	afunc |= iRandom(1) ? SST_ALPHAFUNC_GT : 0;
	aref = a/S + rRandom(-3,3);		// keep AREF within 3 of alpha

	SET(sst->alphaMode, SST_ENALPHAFUNC | afunc | (aref<<SST_ALPHAREF_SHIFT));
	gdbg_info(2,"afunc = %2d %s%s%s %d(0x%x)\n",afunc, 
			afunc&SST_ALPHAFUNC_LT ? "<" : " ",
			afunc&SST_ALPHAFUNC_GT ? ">" : " ",
			afunc&SST_ALPHAFUNC_EQ ? "=" : " ",
			aref,aref);

	do {
	    xyRandom(&x,&y);			// pick random x,y
	    spanlen = iRandom(13) + 1;		// and random span length
	} while (x + spanlen > diago.xmaxscreen);
	gdbg_info(3,"span %d,%d len=%d\n",x,y,spanlen);

	SET(sst->r, r);
	SET(sst->g, g);
	SET(sst->b, b);
	SET(sst->a, a);
	SET(sst->drdx,dr);
	SET(sst->dgdx,dg);
	SET(sst->dbdx,db);
	SET(sst->dadx,da);
	gdbg_info(3,"   rgba = %s %s %s %s\n",PC(r),PC(g),PC(b),PC(a));
	gdbg_info(3,"dx:rgba = %s %s %s %s\n",PC(dr),PC(dg),PC(db),PC(da));

	sst_drawspan(sst,x,y,spanlen);		// now draw the span
	sst_idle(sst);				// wait for the command to complete
	for (i=0; i<spanlen; i++) {
	    long cr,cg,cb,ca;
	    unsigned long c;

	    cr = r >> SST_RGBA_FRACBITS;
	    cg = g >> SST_RGBA_FRACBITS;
	    cb = b >> SST_RGBA_FRACBITS;
	    ca = a >> SST_RGBA_FRACBITS;
	    c  = ca<<24;
	    c |= cr<<16;			// compute RGB color
	    c |= cg<<8;
	    c |= cb<<0;

	    if (afunc & SST_ALPHAFUNC_LT)	// perform Alpha Function
		if (ca < aref) goto ok;
	    if (afunc & SST_ALPHAFUNC_EQ)
		if (ca == aref) goto ok;
	    if (afunc & SST_ALPHAFUNC_GT)
		if (ca > aref) goto ok;
	    c = 0;				// test failed, set color = 0

	ok:
	    // truncate to 565 or 555 RGB
	    c = sst_argb_form_result(c,0,0,0);
	    DIAG_TEST_PIXEL(diago.curdrawbuffer, x+i,y, c);
	    if (c)				// reset the pixel
		DIAG_FORCE_PIXEL(diago.curdrawbuffer, x+i,y,0);
	    r += dr;				// increment the color
	    g += dg;
	    b += db;
	    a += da;
	}
    }
    DIAG_PASS(0);
}

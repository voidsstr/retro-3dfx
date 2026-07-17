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
** $Date: 10/11/00 8:09:59 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

// simplifying print macro for printing colors in .12 format
#define PC(c)	printFix("%4d.%03x",c,SST_RGBA_FRACBITS)
#define S	(1<<SST_RGBA_FRACBITS)

void
main (int argc, char **argv)
{
    int i,n, spanlen;
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
	GDBG_INFO(0, "Warning! Can't run aiter with AA. Just going to pass the f'er\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	GDBG_INFO(0, "****************************************************************\n");
	DIAG_PASS(0);
      }


    SET(sst->alphaMode,SST_ENALPHABLEND |
		(SST_A_SRCALPHA<<SST_RGBSRCFACT_SHIFT) |
		(SST_A_ZERO<<SST_RGBDSTFACT_SHIFT) |
		(SST_A_ONE<<SST_ASRCFACT_SHIFT) |
		(SST_A_ZERO<<SST_ADSTFACT_SHIFT));

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<50; n++) {			// do 50 tests
	do {
	    xyRandom(&x,&y);			// pick random x,y
	    spanlen = iRandom(41) + 1;		// and random span length
	} while (x + spanlen > diago.xmaxscreen);
	gdbg_info(2,"span %d,%d len=%d\n",x,y,spanlen);

	// NOTE: we do not allow any overflows or underflows on the colors
	r = rRandom(100*S,156*S);		// random rgba in [100,156] range
	g = rRandom(100*S,156*S);		// in .12 format
	b = rRandom(100*S,156*S);
	a = rRandom(100*S,156*S);
	dr = rRandom(-S/2,S/2);			// +-.5
	dg = rRandom(-1*S,1*S);			// +-1
	db = rRandom(-2*S,2*S);			// +-2
	da = rRandom(-2*S,2*S);			// +-2
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
	    cr = cr * (ca + 1);
	    cg = cg * (ca + 1);
	    cb = cb * (ca + 1);

	    c  = ca << 24;
	    c |= (cr>>SST_RGBA_INTBITS)<<16;
	    c |= (cg>>SST_RGBA_INTBITS)<<8;
	    c |= (cb>>SST_RGBA_INTBITS)<<0;
	    // truncate to 565 or 555 RGB
	    c = sst_argb_form_result(c,0,0,0);
	    DIAG_TEST_PIXEL(diago.curdrawbuffer,x+i,y, c);
	    DIAG_FORCE_PIXEL(diago.curdrawbuffer,x+i,y,0);
	    r += dr;				// increment the color
	    g += dg;
	    b += db;
	    a += da;
	}
    }
    DIAG_PASS(1);
}

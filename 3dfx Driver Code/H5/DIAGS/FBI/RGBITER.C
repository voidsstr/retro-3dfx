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
** $Date: 10/11/00 8:10:27 PM$
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
    long r,g,b, dr,dg,db;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    if(diago.aaEnabled)
      {
	GDBG_INFO(0, "***************************************************************\n");
	GDBG_INFO(0, "***************************************************************\n");
	GDBG_INFO(0, "***************************************************************\n");
	GDBG_INFO(0, "***************************************************************\n");
	GDBG_INFO(0, "Warning! Can't run rgbiter with AA. Just going to pass the f'er\n");
	GDBG_INFO(0, "***************************************************************\n");
	GDBG_INFO(0, "***************************************************************\n");
	GDBG_INFO(0, "***************************************************************\n");
	GDBG_INFO(0, "***************************************************************\n");
	DIAG_PASS(0);
      }

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<50; n++) {			// do 50 tests
	do {
	    xyRandom(&x,&y);			// pick random x,y
	    spanlen = iRandom(45) + 1;		// and random span length
	} while (x + spanlen > diago.xmaxscreen);
	gdbg_info(2,"span %d,%d len=%d\n",x,y,spanlen);

	// NOTE: we do not allow any overflows or underflows on the colors
	r = rRandom(100*S,156*S);		// random rgba in [100,156] range
	g = rRandom(100*S,156*S);		// in .12 format
	b = rRandom(100*S,156*S);
	dr = rRandom(-1*S,1*S);			// +-1
	dg = rRandom(-2*S,2*S);			// +-2
	db = rRandom(-S/2,S/2);			// +-.5
	SET(sst->r, r);
	SET(sst->g, g);
	SET(sst->b, b);
	SET(sst->drdx,dr);
	SET(sst->dgdx,dg);
	SET(sst->dbdx,db);
	gdbg_info(3,"   rgb = %s %s %s\n",PC(r),PC(g),PC(b));
	gdbg_info(3,"dx:rgb = %s %s %s\n",PC(dr),PC(dg),PC(db));

	sst_drawspan(sst,x,y,spanlen);		// now draw the span
	sst_idle(sst);				// wait for the command to complete
	for (i=0; i<spanlen; i++) {
	    FxU32 good;
	    good = ((r>>SST_RGBA_FRACBITS)<<16) |
		   ((g>>SST_RGBA_FRACBITS)<<8)  |
		   b>>SST_RGBA_FRACBITS;
	    good = sst_argb_form_result(good,0,0,0);
	    DIAG_TEST_PIXEL(diago.curdrawbuffer,x+i,y,good);
	    r += dr;				// increment the color
	    g += dg;
	    b += db;
	}
    }
    DIAG_PASS(0);
}

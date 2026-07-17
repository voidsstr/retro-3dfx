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
** $Date: 10/11/00 8:10:32 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int i,n, spanlen;
    long x,y;
    unsigned long csrc, stip, enstip, enpat;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<50; n++) {			// do 50 tests
	do {
	    xyRandom(&x,&y);			// pick random x,y
	    spanlen = iRandom(32) + 9;		// and random span length
	} while (x + spanlen > diago.xmaxscreen);
	csrc = colRandom32();				// random color
	stip = colRandom32();				// and random stipple
	enstip = iRandom(1) ? SST_ENSTIPPLE : 0;
	enpat  = iRandom(1) ? SST_ENSTIPPLEPATTERN : 0;

	gdbg_info(2,"csrc = 0x%x, stipple = 0x%x : %s %s\n",
			csrc,stip,
			enstip ? "ENabled":"DISabled",
			enpat ? "patterned":"");
	SET(sst->stipple,stip);			// set stipple register
	SET(sst->fbzMode,enstip | enpat | SST_RGBWRMASK | drawbufferRandom());
	SET(sst->fbzColorPath, SST_RGBSEL_C1 | SST_ASEL_C1);
	SET(sst->c1, csrc);			// set color
	sst_drawspan(sst,x,y,spanlen);

	sst_idle(sst);				// wait for the command to complete
	csrc = sst_argb_form_result(csrc,0,0,0);
	for (i=0; i<spanlen; i++) {
	    int stipOK;
	    if (enpat) {
		stipOK = stip>>((y&3)*8);	// isolate byte
		stipOK >>= 7-((x+i)&7);		// isolate bit
		stipOK &= 0x1;
	    }
	    else stipOK = stip & 0x80000000;

	    if (stipOK || !enstip) {		// if stipple passed or not enabled
		DIAG_TEST_PIXEL(diago.curdrawbuffer, x+i,y, csrc);
		DIAG_FORCE_PIXEL(diago.curdrawbuffer,x+i,y,0);// reset the pixel
	    }
	    else
		DIAG_TEST_PIXEL(diago.curdrawbuffer, x+i,y, 0);
	    if (!enpat) stip = (stip<<1) | (stip>>31);
	}
    }
    DIAG_PASS(0);
}
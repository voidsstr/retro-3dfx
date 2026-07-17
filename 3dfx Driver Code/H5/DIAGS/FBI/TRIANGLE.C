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
** $Date: 10/11/00 8:10:44 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int n;
    unsigned long csrc;
    static Triangle t;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    SET(sst->fbzColorPath, SST_RGBSEL_C1 | SST_ASEL_C1);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<20; n++) {			// do 20 tests
	randomTriangle(&t,diago.tsize,1);	// pick random triangle
	areaTriangle(&t);
	sortTriangle(&t);			// sort it
	printTriangle(2,&t,0,0,0);
	gdbg_info(3,"    area = %d\n", t.area);

	csrc = colRandom32();			// and random colors
	SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());
	SET(sst->c1, csrc);
	gdbg_info(4,"    csrc = 0x%x\n", csrc);
	drawTriangle(sst,&t,0,0,0);

	sst_idle(sst);				// wait for the command to complete
	csrc = sst_argb_form_result(csrc,0,0,0);
	checkTriangle(&t,csrc,1,0, insideTriangle,0,0,0);
	eraseTriangle(sst,&t,1,0,0); 		// erase the triangle
    }
    DIAG_PASS(0);
}
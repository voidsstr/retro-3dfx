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
** $Date: 10/11/00 8:10:22 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "hsimio.h"

void
main (int argc, char **argv)
{
    int n,aw,enpipe;
    long x,y;
    unsigned long csrc, cdst=0;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<50; n++) {			// do 50 tests
	xyRandom(&x,&y);			// pick random x,y
	csrc = colRandom32();			// and random colors
	gdbg_info(2,"csrc = 0x%x\n", csrc);
	enpipe = rRandom(-1,1);

	SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());
	if (diago.rgb==32 && enpipe<0)		// get previous color just in case
	    cdst = CSIM_PIXEL_RD(diago.curdrawbuffer,x,y);
	aw=sst_drawpixel(sst,x,y,csrc,enpipe);	// draw a pixel using any command

	sst_idle(sst);				// wait for the command to complete
	csrc = sst_argb_form_result(csrc,0,0,0);
	if (aw == 1) {				// if alpha was not written...
	    csrc &= 0x00FFFFFF;			// merge in previous alpha
	    csrc |= cdst & 0xFF000000;		// or zero in the case of 16bpp
	}
	DIAG_TEST_PIXEL(diago.curdrawbuffer,x,y,csrc);
    }
    DIAG_PASS(0);
}

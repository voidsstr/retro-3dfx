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
** $Date: 10/11/00 8:10:02 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int n;
    long x,y;
    unsigned short asrc, adst, wmask;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<50; n++) {			// do 50 tests
	xyRandom(&x,&y);			// pick random x,y
	asrc = iRandom(0xFF);			// and random a
	adst = iRandom(0xFF);
	wmask = iRandom(1);			// and random writemask
	gdbg_info(2,"asrc,adst = 0x%04x, 0x%04x wmask = %d\n", asrc,adst,wmask);

	if (diago.rgb==16) {
	  // set the writemask bit and draw a pixel using a triangle
	  SET(sst->fbzMode, SST_ENALPHABUFFER | (wmask ? SST_ZAWRMASK : 0));

	  DIAG_FORCE_PIXEL(CSIM_BUF_3D_AUX1,x,y,adst); // set (x,y) = adst
	  sst_drawpixel(sst,x,y,asrc<<24,0);

	  sst_idle(sst);			// wait for the command to complete
	  DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1,x,y, wmask ? asrc : adst);
	}
	else {
	  FxU32 amask = diago.rgb==15 ? 0x80 : 0xFF;
	  // set the writemask bit and draw a pixel using a triangle
	  SET(sst->fbzMode, (wmask ? SST_RGBWRMASK : 0));

	  DIAG_FORCE_PIXEL(diago.curdrawbuffer,x,y,adst<<24); // set (x,y) = adst
	  sst_drawpixel(sst,x,y,asrc<<24,0);

	  sst_idle(sst);			// wait for the command to complete
	  DIAG_TEST_PIXEL(diago.curdrawbuffer,x,y, ((wmask ? asrc : adst)&amask)<<24);
	}
    }
    DIAG_PASS(0);
}

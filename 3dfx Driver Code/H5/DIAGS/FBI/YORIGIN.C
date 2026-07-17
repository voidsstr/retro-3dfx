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
** $Date: 10/11/00 8:10:51 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int n,yorigin,lfb;
    long x,y;
    unsigned long csrc;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    if(diago.sliEnabled)
      {
	GDBG_INFO(0, "SLI detected, test is going to be skipped\n");
	DIAG_PASS(-1);
      }

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<50; n++) {			// do 50 tests
	xyRandom(&x,&y);			// pick random x,y
	csrc = colRandom32();			// and random colors
	yorigin = iRandom(1);			// and random YORIGIN bit

	if(diago.sliEnabled)
	  {
	    GDBG_INFO(2, "SLI enabled => Can't use yorigin swapping\n");
	    yorigin = 0;
	  }

	gdbg_info(2,"csrc = 0x%x  YORIGIN=%d\n", csrc,yorigin);

	SET(sst->fbzMode, SST_RGBWRMASK |
			  (yorigin ? SST_YORIGIN : 0) |
			  drawbufferRandom());

	lfb = sst_drawpixel(sst,x,y,csrc,1);	// draw a pixel using any command
	sst_idle(sst);				// wait for the command to complete
	csrc = sst_argb_form_result(csrc,0,0,0);
	if (yorigin)				// flip y over
	    y = diago.ymaxscreen-1-y;
	DIAG_TEST_PIXEL(diago.curdrawbuffer,x,y,csrc);
    }
    DIAG_PASS(0);
}

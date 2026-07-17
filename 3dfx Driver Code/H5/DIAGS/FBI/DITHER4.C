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
** $Date: 10/11/00 8:10:07 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    static char xyUsed[MAXSCREEN][MAXSCREEN];
    static char xyEndit[MAXSCREEN][MAXSCREEN];
    static unsigned long xyCsrc[MAXSCREEN][MAXSCREEN];
    int n;
    long x,y;
    unsigned long csrc,cdst,endit,dit4;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    dit4 = 1;

    // NOTE: we render 500 points to different pixels on the screen and then
    // check each one afterwards
    while (DIAG_STARTPASS()) {			// for each pass
      memset(xyUsed,0,sizeof(xyUsed));		// clear screen array
      for (n=0; n<500; n++) {			// do 500 random tests
	// first check all 256 values for each of R,G,B
	if (n < 256) {
	    csrc = n | (n<<8) | (n<<16);
	    endit = 1;
	}
	else {
	    csrc = colRandom32();		// use random color
	    endit = iRandom(1);
	}
	do {
	    xyRandom(&x,&y);			// pick random x,y
	} while (xyUsed[x][y]);

	gdbg_info(2,"csrc = 0x%x ENDITHER = %d %s\n",
		csrc,endit, dit4 ? "4x4" : "2x2");

	// set the writemask bit and draw a pixel using any command
	SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom() |
			(endit ? SST_ENDITHER : 0) |
			(dit4 ? 0: SST_DITHER2x2));
	sst_drawpixel(sst,x,y,csrc,1);		// draw a pixel using any command
	xyUsed[x][y] = (char)diago.curdrawbuffer+10;
	xyCsrc[x][y] = csrc;
	xyEndit[x][y] = (char)endit;
      }

      sst_idle(sst);				// wait for the command to complete
      for (y=0; y<diago.ymaxscreen; y++)
      for (x=0; x<diago.xmaxscreen; x++)
      if (xyUsed[x][y]) {
	diago.curdrawbuffer = xyUsed[x][y]-10;
	csrc = xyCsrc[x][y];
	endit = xyEndit[x][y];
	cdst = sst_argb_form_result(csrc,4*endit,x,y);
	DIAG_TEST_PIXEL(diago.curdrawbuffer,x,y,cdst);
      }
    }
    DIAG_PASS(0);
}
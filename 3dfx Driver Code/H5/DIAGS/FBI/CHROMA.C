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
** $Date: 10/11/00 8:10:05 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int n, fail,entest, enpipe,lfb, enrange, enunion;
    long x,y;
    unsigned long csrc, cdst, ckey, crng;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<100; n++) {			// do 100 tests
	xyRandom(&x,&y);			// pick random x,y
	csrc = colRandom32();			// and random colors
	cdst = colRandom32();
	entest = iRandom(3);			// enable it 7/8 times
	fail = iRandom(1);
	enpipe = rRandom(-1,1);
	enunion = iRandom(1);
	enrange = iRandom(2);

	// in the code below we pick a value for ckey (and range)
	// that satisfies the above requests

	{   // SST-96 style chromarange
	    // 3 ways to fail (and hence 3 ways to pass):
	    //	0. csrc == ckey, crng is disabled;
	    //  1. csrc is within  (ckey, crng) and inclusive testing.
	    //  2. csrc is without (ckey, crng) and exclusive testing.

	    if (enrange == 0) {
		// range is disabled, pass/fail if csrc !=/== ckey.
		if (fail)
		    ckey = csrc;		// force failure
		else				// force success
		    while ((ckey = colRandom24()) == csrc);
		crng = 0;
		gdbg_info(2,"CRange disabled\n");
	    }
	    else {
		int a,b,c;	// should r,g,b be INSIDE the range

		// if we want failure and we are in INCLUSIVE mode
		// or we want passing and we are in EXCLUSIVE mode
		if ((fail && (enrange == 1)) || (!fail && (enrange == 2))) {
		    if (enunion == fail) {
			// if UNION mode and FAIL, or INTERSECTION and PASS
			// then generate at least one of the colors within the range
			do {
				a = iRandom(1);
				b = iRandom(1);
				c = iRandom(1);
			} while (a==0 && b==0 && c==0);
		    }
		    else	// INTERSECTION mode - all colors inside
			a=b=c=1;
		} 
		// if we want failure and we are in EXCLUSIVE mode
		// or we want passing and we are in INCLUSIVE mode
		else {
		    if (enunion == fail) {
			// if UNION mode and FAIL, or INTERSECTION and PASS
			// then generate at least one of the colors outside the range
			do {
				a = iRandom(1);
				b = iRandom(1);
				c = iRandom(1);
			} while (a==1 && b==1 && c==1);
		    }
		    else	// all colors outside
			a=b=c=0;
		}
		ckeyRandom888(a,b,c, csrc, &ckey,&crng);

		crng |= SST_ENCHROMARANGE;
		if (enunion) crng |= SST_CHROMARANGE_BLOCK_OR;
		if (enrange == 2)  { 	// do exclusive testing.
		    crng |= SST_CHROMARANGE_BLUE_EX  |
			    SST_CHROMARANGE_GREEN_EX |
			    SST_CHROMARANGE_RED_EX   ;
		}
		gdbg_info(2,"CRange enabled: %s %sclusive\n",
				enunion ? "Union" : "Intersection",
				enrange==2 ? "Ex" : "In");
	    }
	    gdbg_info(3,"EN:%d  fail:%d csrc = 0x%08x, ckey = 0x%08x, cdst = 0x%08x\n",
			entest,fail, csrc, ckey, cdst);
	    SET(sst->chromaRange, crng);
	}
	if (entest) entest = SST_ENCHROMAKEY;	// turn entest into enable bit
	SET(sst->chromaKey,ckey);
	SET(sst->fbzMode, entest | SST_RGBWRMASK | drawbufferRandom());
	DIAG_FORCE_PIXEL(diago.curdrawbuffer,x,y,cdst);// set (x,y) = cdst
	lfb=sst_drawpixel(sst,x,y,csrc,enpipe);	// draw a pixel, any command
	sst_idle(sst);				// wait to complete
	csrc = sst_argb_form_result(csrc,0,0,0);
	cdst = sst_argb_form_result(cdst,0,0,0);
	if (lfb==1) {				// if alpha not written
	    csrc &= 0x00FFFFFF;			// merge in previous alpha
	    csrc |= cdst & 0xFF000000;
	}

	DIAG_TEST_PIXEL(diago.curdrawbuffer,x,y,
			(fail && entest && (lfb==0 || enpipe>=0)) ? cdst : csrc);
    }
    DIAG_PASS(0);
}

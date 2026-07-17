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
** $Date: 10/11/00 8:10:16 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

#define SCREEN_SIZE 1024
int rowHits[SCREEN_SIZE];
int colHits[SCREEN_SIZE];
unsigned long c1 = 0xFF0000;		// red
unsigned long c2 = 0x00FF00;		// green
Triangle t1,t2;

// return c1 if inside triangle 1, c2 if inside triangle 2
int insideLine(Triangle *t, int x, int y, unsigned long col)
{
    if (insideTriangle(&t1,x,y,c1)) {
	colHits[x]++;
	rowHits[y]++;
	return sst_argb_form_result(c1,0,0,0);
    }
    if (insideTriangle(&t2,x,y,c2)) {
	colHits[x]++;
	rowHits[y]++;
	return sst_argb_form_result(c2,0,0,0);
    }
    return 0;
}

void
main (int argc, char **argv)
{
    int n, dx,dy,adx,ady,tris=0;
    int xmin,xmax;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    SET(sst->fbzColorPath, SST_RGBSEL_C1);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<20; n++) {			// do 20 tests
	double a = aRandom();
	double f;

	if (diago.tsize >= 0)
	    f = diago.tsize * XY_ONE;
	else
	    f = iRandom(-diago.tsize * XY_ONE);
    try_again:
	t1.vA.x = iRandom(diago.xmaxscreen*XY_ONE-1);
	t1.vA.y = iRandom(diago.ymaxscreen*XY_ONE-1);
	t1.vB.x = (long)(0.5 + t1.vA.x + f * cos(a));
	t1.vB.y = (long)(0.5 + t1.vA.y + f * sin(a));
	if (!ONSCREEN_FRAC(t1.vB.x,t1.vB.y)) goto try_again;

	// NOTE: we keep line end points on screen, but not entirety of triangles
#define PV(v)	PXY(v.x),PXY(v.y)
#define PXY(x)	printFix("%4d.%x",x,SST_XY_FRACBITS)
	gdbg_info(2,"line = %s,%s to %s,%s\n",PV(t1.vA),PV(t1.vB));
	SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());

	adx = dx = t1.vB.x - t1.vA.x;
	ady = dy = t1.vB.y - t1.vA.y;
	if (adx < 0) adx = -adx;
	if (ady < 0) ady = -ady;
	if (adx >= ady) {			// XMAJOR
	    t1.vA.y -= XY_HALF;
	    t1.vB.y -= XY_HALF;
	    t1.vC.y = t1.vB.y + XY_ONE;
	    t1.vC.x = t1.vB.x;
	    t2 = t1;
	    t2.vB.y = t1.vA.y + XY_ONE;
	    t2.vB.x = t1.vA.x;
	}
	else {					// YMAJOR
	    t1.vA.x -= XY_HALF;
	    t1.vB.x -= XY_HALF;
	    t1.vC.x = t1.vB.x + XY_ONE;
	    t1.vC.y = t1.vB.y;
	    t2 = t1;
	    t2.vB.x = t1.vA.x + XY_ONE;
	    t2.vB.y = t1.vA.y;
	}

	areaTriangle(&t1);
	areaTriangle(&t2);
	sortTriangle(&t1);			// sort it
	sortTriangle(&t2);
	printTriangle(3,&t1,0,0,0);
	printTriangle(3,&t2,0,0,0);
	tris+=2;
	SET(sst->c1, c1);			// red
	drawTriangle(sst,&t1,0,0,0);
	SET(sst->c1, c2);			// green
	drawTriangle(sst,&t2,0,0,0);

	sst_idle(sst);				// wait for the command to complete 
	checkTriangle(&t1,c1,2,0, insideLine,0,0,0);

	xmin = t1.vA.x;
	xmax = t1.vB.x;
	if (xmin > xmax) {xmin = xmax; xmax = t1.vA.x;}
	if (xmin > t1.vC.x) xmin = t1.vC.x;
	if (xmax < t1.vC.x) xmax = t1.vC.x;
	if (!diago.checkEveryTriangle) continue;

	if (adx >= ady) {			// XMAJOR, check each column
	    for (dx=0; dx<SCREEN_SIZE; dx++) {
		if (dx >= ((xmin+XY_HALF-1)>>SST_XY_FRACBITS) &&
		    dx < ((xmax+XY_HALF-1)>>SST_XY_FRACBITS))
		{
		    if (colHits[dx] != 1) {	// inside line
			gdbg_printf(
			  "ERROR: expecting 1 hit in column %d but got %d hits\n",
			  dx,colHits[dx]);
			DIAG_INCERROR();
		    }
		}
		else {
		    if (colHits[dx] != 0) {	// outside line
			gdbg_printf(
			  "ERROR: expecting 0 hits in column %d but got %d hits\n",
			  dx,colHits[dx]);
			DIAG_INCERROR();
		    }
		}
	    }
	}
	else {					// YMAJOR, check each row
	    for (dx=0; dx<SCREEN_SIZE; dx++) {
		if (dx >= ((t1.vA.y+XY_HALF-1)>>SST_XY_FRACBITS) &&
		    dx < ((t1.vC.y+XY_HALF-1)>>SST_XY_FRACBITS))
		{
		    if (rowHits[dx] != 1) {	// within line
			gdbg_printf(
			  "ERROR: expecting 1 hit in row %d but got %d hits\n",
			  dx,rowHits[dx]);
			DIAG_INCERROR();
		    }
		}
		else {
		    if (rowHits[dx] != 0) {	// outside line
			gdbg_printf(
			  "ERROR: expecting 0 hits in row %d but got %d hits\n",
			  dx,rowHits[dx]);
			DIAG_INCERROR();
		    }
		}
	    }
	}
	xmin >>= SST_XY_FRACBITS;		// erase the 2 triangles
	xmax >>= SST_XY_FRACBITS;
	DIAG_FORCE_RECT(diago.curdrawbuffer,
			xmin,t1.vA.y>>SST_XY_FRACBITS,
			xmax-xmin+2,(t1.vC.y>>SST_XY_FRACBITS)-(t1.vA.y>>SST_XY_FRACBITS)+2,
			0);
	for (dx=0; dx<SCREEN_SIZE; dx++) {	// reset counters
	    rowHits[dx] = colHits[dx] = 0;
	}
    }
    DIAG_PASS(1);				// check for black screen
}

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
** $Date: 10/11/00 8:10:17 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

unsigned long c1 = 0xFF0000;		// red
unsigned long c2 = 0x00FF00;		// green
Triangle t1,t2;

void drawTSUtriangle(SstRegs *sst, Triangle *t)
{
    unsigned long saveseed = getSeed();	// don't disturb things
    t->vA.fx = ((float)t->vA.x)*1.0F/16.0F;
    t->vA.fy = ((float)t->vA.y)*1.0F/16.0F;
    t->vB.fx = ((float)t->vB.x)*1.0F/16.0F;
    t->vB.fy = ((float)t->vB.y)*1.0F/16.0F;
    t->vC.fx = ((float)t->vC.x)*1.0F/16.0F;
    t->vC.fy = ((float)t->vC.y)*1.0F/16.0F;
    switch(iRandom(2)) {
    case 0:	// A
	    SETF(sst->sVx,t->vA.fx);
	    SETF(sst->sVy,t->vA.fy);
	    SET(sst->sBeginTriCMD,0);
	    if (iRandom(1)) {
		SETF(sst->sVx,t->vB.fx);
		SETF(sst->sVy,t->vB.fy);
		SET(sst->sDrawTriCMD,0);
		SETF(sst->sVx,t->vC.fx);
		SETF(sst->sVy,t->vC.fy);
	    }
	    else {
		SETF(sst->sVx,t->vC.fx);
		SETF(sst->sVy,t->vC.fy);
		SET(sst->sDrawTriCMD,0);
		SETF(sst->sVx,t->vB.fx);
		SETF(sst->sVy,t->vB.fy);
	    }
	    break;
    case 1:	// B
	    SETF(sst->sVx,t->vB.fx);
	    SETF(sst->sVy,t->vB.fy);
	    SET(sst->sBeginTriCMD,0);
	    if (iRandom(1)) {
		SETF(sst->sVx,t->vA.fx);
		SETF(sst->sVy,t->vA.fy);
		SET(sst->sDrawTriCMD,0);
		SETF(sst->sVx,t->vC.fx);
		SETF(sst->sVy,t->vC.fy);
	    }
	    else {
		SETF(sst->sVx,t->vC.fx);
		SETF(sst->sVy,t->vC.fy);
		SET(sst->sDrawTriCMD,0);
		SETF(sst->sVx,t->vA.fx);
		SETF(sst->sVy,t->vA.fy);
	    }
	    break;
    case 2:	// C
	    SETF(sst->sVx,t->vC.fx);
	    SETF(sst->sVy,t->vC.fy);
	    SET(sst->sBeginTriCMD,0);
	    if (iRandom(1)) {
		SETF(sst->sVx,t->vB.fx);
		SETF(sst->sVy,t->vB.fy);
		SET(sst->sDrawTriCMD,0);
		SETF(sst->sVx,t->vA.fx);
		SETF(sst->sVy,t->vA.fy);
	    }
	    else {
		SETF(sst->sVx,t->vA.fx);
		SETF(sst->sVy,t->vA.fy);
		SET(sst->sDrawTriCMD,0);
		SETF(sst->sVx,t->vB.fx);
		SETF(sst->sVy,t->vB.fy);
	    }
	    break;
    }
    SET(sst->sDrawTriCMD,0);
    setSeed(saveseed);
}

// return c1 if inside triangle 1, c2 if inside triangle 2
int insideLine(Triangle *t, int x, int y, unsigned long col)
{
    if (insideTriangle(&t1,x,y,c1)) {
	return sst_argb_form_result(c1,0,0,0);
    }
    if (insideTriangle(&t2,x,y,c2)) {
	return sst_argb_form_result(c2,0,0,0);
    }
    return 0;
}

void
main (int argc, char **argv)
{
    int j,n, dx,dy,adx,ady,tris=0;
    int xmin,xmax;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    SET(sst->fbzColorPath, SST_RGBSEL_C1);
    SET(sst->sSetupMode, 0);
    // NOTE: we assume SST_BEGIN (or child) sets up rectclipper to full screen

    while (DIAG_STARTPASS())			// for each pass
    for (j=0; j<3; j++) {			// do 3 loops
      SET(sst->fbzMode, SST_ENRECTCLIP | SST_RGBWRMASK | drawbufferRandom());
      for (n=0; n<4; n++) {			// of 4 (or 8) lines each
	double a = aRandom();
	double f;

	if (diago.tsize >= 0)
	    f = diago.tsize * XY_ONE;
	else
	    f = iRandom(-diago.tsize * XY_ONE);
try_again:
	t1.vA.x = rRandom(-10*XY_ONE,10*XY_ONE);	// +- 10
	t1.vA.y = rRandom(-10*XY_ONE,10*XY_ONE);
	if (n < 5) {		// do 5 long lines, in [-4k,+4k] range
// NOTE: we back off 1 on MAXCOORD so if a triangle starts on the limit
// e.g. -4096 or 4095 and then we push that location and then pop it and
// move over a pixel, we are still within the limit
#define MAXCOORD 4095
	  // randomly choose left,right or bottom,top edge
	  if (iRandom(1)) {
	    t1.vA.x -= (MAXCOORD-10)*XY_ONE;
	    if (iRandom(1)) t1.vA.y = rRandom(-MAXCOORD*XY_ONE, (MAXCOORD-1)*XY_ONE);
	  }
	  else {
	    if (iRandom(1)) t1.vA.y -= (MAXCOORD-10)*XY_ONE;
	  }
	  t1.vB.x = (long)(0.5 + t1.vA.x + f * cos(a));
	  t1.vB.y = (long)(0.5 + t1.vA.y + f * sin(a));
	  if (iRandom(1)) {
	    t1.vB.x = (MAXCOORD-1-iRandom(10))*XY_ONE;
	    if (iRandom(1)) t1.vB.y = rRandom(-MAXCOORD*XY_ONE, (MAXCOORD-1)*XY_ONE);
	  }
	  else {
	    if (iRandom(1)) t1.vB.y = (MAXCOORD-1-iRandom(10))*XY_ONE;
	  }
	  if (t1.vB.x < -MAXCOORD*XY_ONE || t1.vB.x > MAXCOORD*XY_ONE-1 ||
		t1.vB.y < -MAXCOORD*XY_ONE || t1.vB.y > MAXCOORD*XY_ONE-1)
	  {
		a = aRandom();
		goto try_again;
	  }
	}
	else {
	  // randomly choose left,right or bottom,top edge
	  if (iRandom(1)) t1.vA.x += diago.xmaxscreen*XY_ONE;
	  if (iRandom(1)) t1.vA.y += diago.ymaxscreen*XY_ONE;
	  t1.vB.x = (long)(0.5 + t1.vA.x + f * cos(a));
	  t1.vB.y = (long)(0.5 + t1.vA.y + f * sin(a));
	}
#define PV(v)	PXY(v.x),PXY(v.y)
#define PXY(x)	printFix("%4d.%x",x,SST_XY_FRACBITS)
	gdbg_info(2,"line = %s,%s to %s,%s\n",PV(t1.vA),PV(t1.vB));

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
	if (n < 5) {
	    if (iRandom(1)) 
		drawTSUtriangle(sst,&t1);
	    else
		drawTriangle(sst,&t1,0,0,0);
	    SET(sst->c1, c2);			// green
	    if (iRandom(1)) 
		drawTSUtriangle(sst,&t2);
	    else
		drawTriangle(sst,&t2,0,0,0);
	}
	else {
	    drawTriangle(sst,&t1,0,0,0);
	    SET(sst->c1, c2);			// green
	    drawTriangle(sst,&t2,0,0,0);
	}

	sst_idle(sst);				// wait for the command to complete
	checkTriangle(&t1,c1,2,0, insideLine,0,0,0);

	xmin = t1.vA.x;
	xmax = t1.vB.x;
	if (xmin > xmax) {xmin = xmax; xmax = t1.vA.x;}
	if (xmin > t1.vC.x) xmin = t1.vC.x;
	if (xmax < t1.vC.x) xmax = t1.vC.x;
	xmin >>= SST_XY_FRACBITS;		// erase the 2 triangles
	xmax >>= SST_XY_FRACBITS;
	// some lines may be 8000 pixels long and diagonal, 
	// so this may take a while!
	DIAG_FORCE_RECT(diago.curdrawbuffer,
			xmin,t1.vA.y>>SST_XY_FRACBITS,
			xmax-xmin+2,(t1.vC.y>>SST_XY_FRACBITS)-(t1.vA.y>>SST_XY_FRACBITS)+2,
			0);
      }
    }
    DIAG_PASS(1);				// check for black screen
}

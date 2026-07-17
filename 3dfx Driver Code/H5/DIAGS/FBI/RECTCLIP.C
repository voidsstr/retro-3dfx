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
** $Revision: 3$
** $Date: 10/11/00 8:10:24 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

// NOTE: this could be optimized to first draw all the pixels and
//	then wait for the chip to be idle and then check all the pixels
//	but hey, that's harder!  So if it doesn't take too long we'll
//	leave things this way

int clipper, clipType;

// draw a pixel that is outside the clip rect and check for 0
void outside(SstRegs *sst, long x, long y)
{
    int lfb, enpipe;	// selects: LFB(-1), triangle(0), LFB+ENPIXPIPE(+1)
    FxU32 csrc = 0xFFFFFFFF;

    if (!ONSCREEN(x,y)) return;
    enpipe = rRandom(-1,1);
    SET(sst->fbzMode, SST_ENRECTCLIP | SST_RGBWRMASK | drawbufferRandom());
    lfb = sst_drawpixel(sst,x,y,csrc,enpipe);
    sst_idle(sst);				// wait for idle
    csrc = sst_argb_form_result(csrc,0,0,0);
    if (lfb==1) csrc &= 0x00FFFFFF;		// if alpha not written
    if (lfb && (enpipe<0)) {			// if LFB and pixpipe is disabled
	DIAG_TEST_PIXEL(diago.curdrawbuffer,x,y,csrc);	// then pixel is drawn
	DIAG_FORCE_PIXEL(diago.curdrawbuffer,x,y,0);	// reset to 0
    }
    else
	DIAG_TEST_PIXEL(diago.curdrawbuffer,x,y,0);	// test the pixel
}

// draw a pixel that is inside the clip rect and check for it
void inside(SstRegs *sst, long x, long y)
{
    int lfb, enpipe;	// selects: LFB(-1), triangle(0), LFB+ENPIXPIPE(+1)
    FxU32 csrc = 0xFFFFFFFF;

    enpipe = rRandom(-1,1);
    SET(sst->fbzMode, SST_ENRECTCLIP | SST_RGBWRMASK | drawbufferRandom());
    lfb = sst_drawpixel(sst,x,y,csrc,enpipe);
    sst_idle(sst);				// wait for idle
    csrc = sst_argb_form_result(csrc,0,0,0);
    if (lfb==1) csrc &= 0x00FFFFFF;		// if alpha not written
    DIAG_TEST_PIXEL(diago.curdrawbuffer,x,y,csrc);// test the pixel
    sst_drawpixel(sst,x,y,0x0,0);		// erase the pixel
}


long xl,yb,xr,yt;		// GLOBAL rectangle clipping coords

//---------------------------------------------------------------------
// return whether or not x,y is inside the triangle
int insideTriangleRectClip(Triangle *t, int x, int y, unsigned long col)
{
    int exclusive = (clipper==1) && (clipType != 0);
    if ((x < xl || x >= xr || y < yb || y >= yt) ^ exclusive) return 0;
    col = sst_argb_form_result(col,0,0,0);
    return insideTriangle(t,x,y,col);
}

void
main (int argc, char **argv)
{
    int n,temp1,temp2;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    //If AA is enabled, make sure that the jitter is 0 for all samples.
    //Otherwise, some of the triangles can step outside of the valid
    //range. This causes the diag to hang.
    if(diago.aaEnabled)
      SET(sst->aaCtrl, SST_AA_CONTROL_AA_ENABLE);

    while (DIAG_STARTPASS())			// for each pass
    for (n=0; n<10; n++) {			// do 10 tests
	if (n == 0) {
	    xl = 0; xr = diago.xmaxscreen;
	    yb = 0; yt = diago.ymaxscreen;
	}
	else {
	    xyRandom(&xl,&yb);			// pick random x,y
	    xyRandom(&xr,&yt);			// pick random x,y
	}
	if (xl > xr) {long t=xl; xl=xr; xr=t; } 
	if (yb > yt) {long t=yb; yb=yt; yt=t; } 
	clipper = iRandom(1);		// pick a clipper
#ifdef CVG
	clipper = 0;
#endif
	gdbg_info(2,"cliprect%d = %d,%d to %d,%d\n",clipper,xl,yb,xr,yt);

	if (clipper == 0) {
	    SET(sst->clipLeftRight,(xl<<SST_CLIPLEFT_SHIFT) | xr);
	    SET(sst->clipBottomTop,(yb<<SST_CLIPBOTTOM_SHIFT) | yt);
	}
#ifndef CVG
	else {	// test only inclusive at this point
	    SET(sst->clipLeftRight,(0<<SST_CLIPLEFT_SHIFT) | diago.xmaxscreen);
	    SET(sst->clipBottomTop,(0<<SST_CLIPBOTTOM_SHIFT) | diago.ymaxscreen);
	    SET(sst->clipLeftRight1, SST_ENRECTCLIP1 | (xl<<SST_CLIPLEFT_SHIFT) | xr);
	    SET(sst->clipBottomTop1,(yb<<SST_CLIPBOTTOM_SHIFT) | yt);
	}
	if (clipper==0) SET(sst->clipLeftRight1, 0);	// disable 2nd clipper
#endif

	// first test all around the corners
	outside(sst,xl+1,yb-1);		// around lower left corner
	outside(sst,xl  ,yb-1);
	outside(sst,xl-1,yb+1);

	outside(sst,xl-1,yt-1);		// around upper left corner
	outside(sst,xl  ,yt);
	outside(sst,xl+1,yt);

	outside(sst,xr-1,yb-1);		// around lower right corner
	outside(sst,xr  ,yb);
	outside(sst,xr  ,yb+1);

	outside(sst,xr  ,yt-1);		// around upper right corner
	outside(sst,xr  ,yt);
	outside(sst,xr-1,yt);

	// now in the middle of each edge
	outside(sst,xl-1,(yb+yt)/2);
	outside(sst,xr,(yb+yt)/2);
	outside(sst,(xl+xr)/2,yb-1);
	outside(sst,(xl+xr)/2,yt);

	// check for null rectangle
	if (xr <= xl || yt <= yb) continue;

	// one on each edge and corner but inside
	inside(sst,xl  ,yb);
	if (yb+1 != yt) {
	    inside(sst,xl  ,yb+1);
	    inside(sst,xl  ,yt-1);
	    if (xl+1 != xr) inside(sst,xl+1,yt-1);
	}
	if (xl+1 != xr) {
	    inside(sst,xl+1,yb);
	    inside(sst,xr-1,yb);
	    if (yb+1 != yt) {
		inside(sst,xr-1,yb+1);
		inside(sst,xr-1,yt-1);
	    }
	}

	// now some random ones inside
	{
	    int j, w,h;
	    w = xr-xl-1;
	    h = yt-yb-1;
	    for (j=0; j<10; j++) {
		inside(sst,xl+iRandom(w),yb+iRandom(h));
	    }
	}

	// now test some random triangles to make sure everything works
	// on triangles that render more than one pixel
	SET(sst->fbzColorPath, SST_RGBSEL_C1 | SST_ASEL_C1);
	SET(sst->fbzMode, SST_ENRECTCLIP | SST_RGBWRMASK | drawbufferRandom());
	clipType = 0;
#ifndef CVG
	if (clipper == 1) {
	    clipType = iRandom(1) ? 0 : SST_RECTCLIP1_EX;
	    SET(sst->clipBottomTop1,clipType | (yb<<SST_CLIPBOTTOM_SHIFT) | yt);
	    gdbg_info(3,"    %s\n",clipType ? "exclusive" : "inclusive");
	}
#endif
	{
	    int j;
	    static Triangle t;

	    for (j=0; j<10; j++) {
		SET(sst->c1, 0xFFFFFF);
	     again:
		switch(iRandom(2)) {
		    case 0:
			t.vA.x = xl+rRandom(-3,3);
			break;
		    case 1:
			t.vA.x = (xl+xr)/2+rRandom(-3,3);
			break;
		    case 2:
			t.vA.x = xr+rRandom(-3,3);
			break;
		}
		switch(iRandom(2)) {
		    case 0:
			t.vA.y = yb+rRandom(-3,3);
			break;
		    case 1:
			t.vA.y = (yb+yt)/2+rRandom(-3,3);
			break;
		    case 2:
			t.vA.y = yt+rRandom(-3,3);
			break;
		}
		t.vA.x = (t.vA.x<<SST_XY_FRACBITS) + iRandom(XY_ONE-1);
		t.vA.y = (t.vA.y<<SST_XY_FRACBITS) + iRandom(XY_ONE-1);
		if (!ONSCREEN_FRAC(t.vA.x,t.vA.y)) goto again;

		randomTriangle1(&t,diago.tsize,1);
		areaTriangle(&t);
		sortTriangle(&t);		// sort it
		printTriangle(2,&t,0,0,0);
		drawTriangle(sst,&t,0,0,0);

		sst_idle(sst);			// wait for the command to complete
		checkTriangle(&t,0xFFFFFF,1,0, insideTriangleRectClip,0,0,0);

		SET(sst->c1, 0);		// erase the triangle
		drawTriangle(sst,&t,0,0,0);
	    }

	    // now test the [-4k,+4k] drawing surface
	    gdbg_info(2,"stressing limits\n");
#define MAXCOORD 4096
	    // left
	    temp1 = rRandom(-MAXCOORD,-MAXCOORD+4);
	    temp2 = rRandom(-MAXCOORD,MAXCOORD-1);
	    sst_drawpixel(sst,temp1,temp2,0xFFFFFF,0);
	    sst_drawpixel(sst,rRandom(-MAXCOORD,-MAXCOORD+4),1,0xFFFFFF,0);
	    // right
	    temp1 = rRandom(MAXCOORD-5,MAXCOORD-1);
	    temp2 = rRandom(-MAXCOORD,MAXCOORD-1);
	    sst_drawpixel(sst,temp1,temp2,0xFFFFFF,0);
	    sst_drawpixel(sst,rRandom(MAXCOORD-5,MAXCOORD-1),2,0xFFFFFF,0);
	    // bottom
	    temp1 = rRandom(-MAXCOORD,MAXCOORD-1);
	    temp2 = rRandom(-MAXCOORD,-MAXCOORD+4);
	    sst_drawpixel(sst,temp1,temp2,0xFFFFFF,0);
	    sst_drawpixel(sst,3,rRandom(-MAXCOORD,-MAXCOORD+4),0xFFFFFF,0);
	    // top
	    temp1 = rRandom(-MAXCOORD,MAXCOORD-1);
	    temp2 = rRandom(MAXCOORD-5,MAXCOORD-1);
	    sst_drawpixel(sst,temp1,temp2,0xFFFFFF,0);
	    sst_drawpixel(sst,4,rRandom(MAXCOORD-5,MAXCOORD-1),0xFFFFFF,0);
	}
    }
    DIAG_PASS(1);
}

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
** $Date: 10/11/00 8:10:08 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

SstRegs *sst;

static void
fastfill(int x, int y, int w, int h, int ffdither)
{
    SET(sst->clipLeftRight, (x<<16) | (x+w));
    SET(sst->clipBottomTop, (y<<16) | (y+h));
    SET(sst->fastfillCMD, 
	ffdither ? 0 : SST_FASTFILL_DISABLE_DITHER);
}

static void
checkRow0(int buf, int x, int y, int w)
{
    while (w-- > 0) {
	DIAG_TEST_PIXEL(buf,x++,y,0);
    }
}

static void
checkRectangle(int buf, int x, int y, int w, int h, FxU32 csrc, int dit2)
{
    int i;

    checkRow0(buf,x-1,y-1,w+2);		// check row below for 0
    checkRow0(buf,x-1,y+h,w+2);		// check row above for 0
    while (h-- > 0) {
	DIAG_TEST_PIXEL(buf,x-1,y,0);	// check pixel before for 0
	for (i=x; i<x+w; i++) {				// check each pixel in the row
	    FxU32 good = buf==CSIM_BUF_3D_AUX1 ? 
			csrc : sst_argb_form_result(csrc,dit2*2,i,y);
	    DIAG_TEST_PIXEL(buf,i,y,good);
	}
	DIAG_TEST_PIXEL(buf,x+w,y,0);	// check pixel after for 0
    }
}

// test the rendering accuracy of the fastfill command
void test_render(SstRegs *sst, int sdram)
{
    int n;
    long x,y,w,h;
    unsigned long csrc,dit2, fbzMode, renMode, combineMode;
    unsigned long dither, ffdither=1,auxplanes,zsrc;

    gdbg_info(2,"fastfill render test-----------\n");
    renMode = GET(sst->renderMode);
    for (n=0; n<20; n++) {			// do 20 tests
    again:
	xyRandom(&x,&y);			// pick random x,y onscreen
	do {					// and random width and height
	    w = iRandom(50);
	    h = iRandom(50);
	} while (w * h > 500);
	if (!ONSCREEN(x+w-1,y+h-1))
	    goto again;

	csrc = colRandom24();			// and random colors
	dit2 = iRandom(1);
	auxplanes = iRandom(1);
	zsrc = colRandom32();
	gdbg_info(3,"fastfill %d,%d to %d,%d\n",x,y,x+w,y+h);
	gdbg_info(4,"csrc = 0x%08x  dit2=%d  auxplanes=%d\n", csrc,dit2,auxplanes);

	// turn on all sorts of things that are ignored
	fbzMode =	SST_ENCHROMAKEY |	// ignored by FASTFILL
			SST_ENSTIPPLE |		// ignored by FASTFILL
			SST_ENDEPTHBUFFER |	// ignored by FASTFILL
			SST_ENALPHAMASK |	// ignored by FASTFILL
			SST_RGBWRMASK |
			SST_DITHER2x2 |
			(auxplanes ? SST_ZAWRMASK : 0) |	
			(dit2 ? SST_ENDITHER : 0) |
			drawbufferRandom();

	//Pick a random value for combineMode
	//This insures that the csim is functioning properly

	if(combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK)
	  {
	    combineMode = iRandom(0xFFFFFFFF);

	    if(!(combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK))
	      {
		int i;
		
		for(i=0; i<12; i++)
		  SET_0_1(sst->nopCMD, 0);
	      }
	  }
	else
	  combineMode = iRandom(0xFFFFFFFF);

	SET(sst->combineMode, combineMode);
	SET(sst->fbzMode, fbzMode);
	SET(sst->c1,csrc);			// set the color
	SET(sst->zaColor,zsrc);
	// if 15bpp mode, alpha comes from MSB of zsrc unless renMode overrides it
	if (diago.rgb == 15) {
	    renMode &= ~SST_RM_ALPHAMODE;
	    switch (iRandom(3)) {
		case 0:
		    renMode |= SST_RM_ALPHA_ZERO;
		    csrc &= 0x00FFFFFF;
		    break;
		case 1:
		    renMode |= SST_RM_ALPHA_ONE;
		    csrc |= 0x80000000;
		    break;
		case 2:
		    renMode |= SST_RM_ALPHA_MSB;
		    if (zsrc&0x80000000)
			csrc |= 0x80000000;
		    break;
	    }
	    SET(sst->renderMode, renMode);
	}
	ffdither = iRandom(1);                  // randomly disable fastfill dithering
	fastfill(x,y,w,h,ffdither);
	sst_idle(sst);				// wait for the command to complete

	if ( sdram ) 
	  dither = dit2;                        // dithering depends only on fbzMode
	else
	  dither = dit2 && ffdither;            // dithering depends on fbzMode and fastfillCMD

	// check the entire rectangle
	checkRectangle(diago.curdrawbuffer,x,y,w,h,csrc,dither);
	if (auxplanes) {
	    zsrc &= (diago.rgb==32) ? 0xFFFFFF : 0xFFFF;
	    checkRectangle(CSIM_BUF_3D_AUX1,x,y,w,h,zsrc,0);
	}
	DIAG_FORCE_RECT(diago.curdrawbuffer,x,y,w,h,0);// reset the rectangle to 0
	DIAG_FORCE_RECT(CSIM_BUF_3D_AUX1,x,y,w,h,0);
    }
}

// test the independence of the writemasks (only in 32bpp mode)
void test_wmasks(SstRegs *sst, int sdram)
{
    static int once = 1;
    int n;
    long x,y,w,h;
    unsigned long csrc,zsrc,ssrc, fbzMode,renMode,stenMode;

    gdbg_info(2,"fastfill writemask test-----------\n");
    renMode = GET(sst->renderMode);

    for (n=0; n<64; n++) {			// do 64 tests
    again:
	xyRandom(&x,&y);			// pick random x,y onscreen
	do {					// and random width and height
	    w = iRandom(50);
	    h = iRandom(50);
	} while (w * h > 100);
	if (!ONSCREEN(x+w-1,y+h-1))
	    goto again;

	csrc = colRandom32();			// and random colors
	zsrc = colRandom24();			// and random depth info
	ssrc = iRandom(0xFF);			// and random stencil info

	// turn on all sorts of things that are ignored
	fbzMode =	SST_ENCHROMAKEY |	// ignored by FASTFILL
			SST_ENSTIPPLE |		// ignored by FASTFILL
			SST_ENDEPTHBUFFER |	// ignored by FASTFILL
			SST_ENALPHAMASK |	// ignored by FASTFILL
			SST_RGBWRMASK |
			drawbufferRandom();
	// toggle the writemask bits
	if (once) {	// the first pass, do explicit setting
	    // first clear all the writemasks
	    fbzMode &= ~SST_ZAWRMASK;
	    renMode &= ~SST_RM_RGB_WMASK;
	    renMode &= ~SST_RM_ALPHA_WMASK;
	    stenMode = 0;
	    if (n & 1) fbzMode |= SST_ZAWRMASK;
	    if (n & 2) renMode |= SST_RM_RED_WMASK;
	    if (n & 4) renMode |= SST_RM_GREEN_WMASK;
	    if (n & 8) renMode |= SST_RM_BLUE_WMASK;
	    if (n & 16) renMode |= SST_RM_ALPHA_WMASK;
	    if (n & 32) stenMode |= SST_STENCIL_WMASK;
	}
	else {	// randomly after the first pass
	    if (iRandom(1)) fbzMode ^= SST_ZAWRMASK;
	    if (iRandom(1)) renMode ^= SST_RM_ALPHA_WMASK;
	    if (iRandom(1)) renMode ^= SST_RM_RED_WMASK;
	    if (iRandom(1)) renMode ^= SST_RM_GREEN_WMASK;
	    if (iRandom(1)) renMode ^= SST_RM_BLUE_WMASK;
	    stenMode = iRandom(0xFF) << SST_STENCIL_WMASK_SHIFT;
	}
	stenMode |= ssrc << SST_STENCIL_REF_SHIFT;

	gdbg_info(3,"fastfill %d,%d to %d,%d\n",x,y,x+w,y+h);
	gdbg_info(4,"csrc = 0x%08x    zsrc = 0x%06x    ssrc = 0x%02x\n",
			csrc,zsrc,ssrc);
	gdbg_info(4,"masks: %s%s%s%s %s %s\n",
			renMode & SST_RM_ALPHA_WMASK ? "A" : " ",
			renMode & SST_RM_RED_WMASK ? "R" : " ",
			renMode & SST_RM_GREEN_WMASK ? "G" : " ",
			renMode & SST_RM_BLUE_WMASK ? "B" : " ",
			fbzMode & SST_ZAWRMASK ? "Z" : " ",
			stenMode & SST_STENCIL_WMASK ? "S" : " ");

	// now draw it
	SET(sst->c1,csrc);			// set the color
	SET(sst->zaColor,zsrc);			// and stencil+depth
	SET(sst->fbzMode, fbzMode);
	SET(sst->renderMode, renMode);
	SET(sst->stencilMode, stenMode);
	fastfill(x,y,w,h,0);
	sst_idle(sst);				// wait for the command to complete

	// check color buffer (RGB + A)
	if ((renMode & SST_RM_ALPHA_WMASK)==0)
	    csrc &= ~0xFF000000;
	if ((renMode & SST_RM_RED_WMASK)==0)
	    csrc &= ~0x00FF0000;
	if ((renMode & SST_RM_GREEN_WMASK)==0)
	    csrc &= ~0x0000FF00;
	if ((renMode & SST_RM_BLUE_WMASK)==0)
	    csrc &= ~0x000000FF;
	// check the entire rectangle in the color buffer
	checkRectangle(diago.curdrawbuffer,x,y,w,h,csrc,0);

	// check aux buffer (depth+stencil)
	zsrc |= ssrc << 24;
	if ((fbzMode & SST_ZAWRMASK)==0)
	    zsrc &= ~0x00FFFFFF;
	if ((stenMode & SST_STENCIL_WMASK)==0)
	    zsrc &= ~0xFF000000;
	// check the entire rectangle in the aux buffer
	checkRectangle(CSIM_BUF_3D_AUX1,x,y,w,h,zsrc,0);

	// reset the rectangle to 0
	DIAG_FORCE_RECT(diago.curdrawbuffer,x,y,w,h,0);
	DIAG_FORCE_RECT(CSIM_BUF_3D_AUX1,x,y,w,h,0);
    }
    once = 0;
    SET(sst->renderMode, renMode | SST_RM_RGB_WMASK | SST_RM_ALPHA_WMASK);
    SET(sst->stencilMode,0);
}

void
main (int argc, char **argv)
{
    SstIORegs *sstio;
    FxU32 sdram;

    sst = SST_BEGIN(argc,argv);

    sstio = (SstIORegs *) SST_IO_ADDRESS(sst);
    sdram = GET(sstio->dramInit1) & SST_MCTL_TYPE_SDRAM;
    GDBG_INFO(1,"fastfill: memory type is %s\n",sdram?"SDRAM":"SGRAM");

    // select RGBA as default color, to make sure that FASTFILL overrides it
    SET(sst->fbzColorPath, SST_RGBSEL_RGBA | SST_ASEL_RGBA);
    // turn on all sorts of pixel modes to try to prevent drawing
    // FASTFILL is supposed to ignore all these
    SET(sst->fogMode,SST_ENFOGGING);
    SET(sst->alphaMode,SST_ENALPHAFUNC |
			SST_ENALPHABLEND |
			(SST_A_ZERO<<SST_RGBSRCFACT_SHIFT) |
			(SST_A_ZERO<<SST_RGBDSTFACT_SHIFT) |
			(SST_A_ZERO<<SST_ASRCFACT_SHIFT) |
			(SST_A_ZERO<<SST_ADSTFACT_SHIFT));

    while (DIAG_STARTPASS()) {			// for each pass
	SET(sst->c0, colRandom32());
	test_render(sst, sdram);
	if (diago.rgb == 32)
	    test_wmasks(sst, sdram);
    }
    DIAG_PASS(0);
}

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
** $Date: 10/11/00 8:09:56 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

/* These are here to make Scott's verification code work. */
#include "fbi.h"

/* This is for generating RGB colors. */
#define S	(1<<SST_RGBA_FRACBITS)
#define FBI_SET_COLOR(x,c) x.alpha=(short)((c>>24)&0xff); \
                           x.red=(short)((c>>16)&0xff); \
                           x.green=(short)((c>>8)&0xff); \
                           x.blue=(short)((c)&0xff)
#define FBI_GET_COLOR(x) (((x.alpha&0xff)<<24)|((x.red&0xff)<<16)|\
				((x.green&0xff)<<8)|(x.blue&0xff))

/* My own draw pixel which sets alpha. (Gary's doesn't always.) */
int ablend_drawpixel(SstRegs *sst,int x, int y, FxU32 col, FxU32 renderMode)
{
  gdbg_info(10,"  drawpixel(%d,%d)\n",x,y);

  if (iRandom(1) && ((renderMode & SST_RM_DITHER_ROTATION) == 0)) {
    FxU32 *lfb =  (FxU32 *)SST_LFB_ADDRESS(sst);
    gdbg_info(15, "using LFB access\n");
    SET(sst->lfbMode,(SST_LFB_8888 | SST_LFB_ENPIXPIPE));
    SET(lfb[lfbOffset(x,y)],col);
  }
  else {
    gdbg_info(15, "using TRIANGLE command\n");
    SET(sst->fbzColorPath, (SST_RGBSEL_C1 | SST_ASEL_C1));
    SET(sst->c1, col);

    //setPixelsPerClock toggles between 1 and 2 pixels per clock rendering
    //if appropriate (i.e. --pixelsPerClock <= 0)
    setPixelsPerClock(sst);

    x <<= SST_XY_FRACBITS;
    y <<= SST_XY_FRACBITS;    
    SET(sst->vA.x,x);
    SET(sst->vA.y,y);
    SET(sst->vB.x,x+XY_ONE);
    SET(sst->vB.y,y);
    SET(sst->vC.x,x+XY_ONE);
    SET(sst->vC.y,y+XY_ONE);
    SET(sst->triangleCMD,0);
  }
  return 0;
}

static int _afactor(FxU32 fact, FxU32 csrc, FxU32 cdst)
{
    switch(fact) {
	case SST_A_ZERO:	return 0;
	case SST_A_SRCALPHA:	return (csrc>>24);
	case SST_A_DSTALPHA:	return (cdst>>24);
	case SST_A_ONE:		return 0xff;
	case SST_AOM_SRCALPHA:	return 0xff-(csrc>>24);
	case SST_AOM_DSTALPHA:	return 0xff-(cdst>>24);
	default:GDBG_ERROR("_afactor","invalid value %d\n",fact);
		return 0;
    }
}

/* This deals with conforming to Scott's fbi.c conventions and calls ablend. */
FxU32 ablend_verify(FxU32 alphaMode, FxU32 fogMode,
			FxU32 fbzMode, FxU32 renderMode,
			FxU32 cDst,
			FxU32 cSrc,
			long x, 
			long y)
{
  int asrcf, adstf, dm;
  static int dithmat[4][4][4] = {
		{{0,8,2,10}, {12,4,14,6}, {3,11,1,9}, {15,7,13,5}},
		{{12,0,14,2}, {4,8,6,10}, {15,3,13,1}, {7,11,5,9}},
		{{4,12,6,14}, {8,0,10,2}, {7,15,5,13}, {11,3,9,1}},
		{{8,4,10,6}, {0,12,2,14}, {11,7,9,5}, {3,15,1,13}}
		};
  int (*pdithmat)[4];
  
  /* These are the structures which are passed to Scott's verification code. */
  ablendMode amode;
  fbiColors srcColor, dstColor, prefog;
  fbiColors expect;

  FBI_SET_COLOR(srcColor,cSrc);
  FBI_SET_COLOR(dstColor,cDst);
  FBI_SET_COLOR(prefog,0x00000000);
  amode.ablend_en = 1;
  amode.alpha_fact_src = (unsigned char)((alphaMode & SST_RGBSRCFACT) >> SST_RGBSRCFACT_SHIFT);
  amode.alpha_fact_dst = (unsigned char)((alphaMode & SST_RGBDSTFACT) >> SST_RGBDSTFACT_SHIFT);
  amode.subtract = 0;
  amode.reverse = 0;
  amode.subtract = (fogMode & SST_RGB_BLEND_SUB) != 0;
  amode.reverse = (fogMode & SST_RGB_BLEND_REVERSE) != 0;
  if (renderMode & SST_RM_DITHER_ROTATION)
    {
      if(diago.aaEnabled)
	pdithmat = dithmat[(fogMode & SST_DITHER_ROTATE_BLEND_AA)>>SST_DITHER_ROTATE_BLEND_AA_SHIFT];
      else
	pdithmat = dithmat[(fogMode & SST_DITHER_ROTATE_BLEND)>>SST_DITHER_ROTATE_BLEND_SHIFT];
    }
  else
	pdithmat = dithmat[0];
  if ((fbzMode & SST_DITHER2x2) && (diago.deviceID >= SST_DEVICE_ID_H3))
    dm = pdithmat[y & 1][x & 1];
  else
    dm = pdithmat[y & 3][x & 3];

  /* Truncate it. */
  if (diago.rgb < 32) {
    dstColor.red &= 0xf8;
    dstColor.green &= diago.rgb == 16 ? 0xfc : 0xf8;
    dstColor.blue &= 0xf8;
  }
  // now blend the RGB
  ablend(&amode,fbzMode & SST_ENDITHERSUBTRACT, dm, &srcColor,&prefog,&dstColor,&expect);

  /* Truncate it. */
  if (diago.rgb < 32) {
    expect.red &= 0xf8;
    expect.green &= diago.rgb == 16 ? 0xfc : 0xf8;
    expect.blue &= 0xf8;
  }

  // now handle the alpha channel
  if (diago.rgb == 16) {
    // Note: only 2 factors are allowed for alpha, zero and one
    asrcf = (((alphaMode & SST_ASRCFACT) >> SST_ASRCFACT_SHIFT)==SST_A_ONE) ? 255:0;
    adstf = (((alphaMode & SST_ADSTFACT) >> SST_ADSTFACT_SHIFT)==SST_A_ONE) ? 255:0;
  }
  else {
    // Note: in 1555 and 8888 ARGB mode, more factors allowed
    asrcf = _afactor((alphaMode & SST_ASRCFACT) >> SST_ASRCFACT_SHIFT,cSrc,cDst);
    adstf = _afactor((alphaMode & SST_ADSTFACT) >> SST_ADSTFACT_SHIFT,cSrc,cDst);
  }
  if (fogMode & SST_A_BLEND_SUB) {
    expect.alpha = ((srcColor.alpha * (asrcf+1))>>8) - ((dstColor.alpha * (adstf+1))>>8);
    if (fogMode & SST_A_BLEND_REVERSE)
	expect.alpha = -expect.alpha;
  }
  else
    expect.alpha = ((srcColor.alpha * (asrcf+1))>>8) + ((dstColor.alpha * (adstf+1))>>8);
  if (expect.alpha < 0) expect.alpha = 0;
  if (expect.alpha > 0xFF) expect.alpha = 0xFF;
  if (diago.rgb == 15) {
    if ((renderMode & SST_RM_ALPHAMODE) == SST_RM_ALPHA_ZERO) expect.alpha = 0;
    if ((renderMode & SST_RM_ALPHAMODE) == SST_RM_ALPHA_ONE) expect.alpha = 0xff;
    expect.alpha &= 0x80;
  }

  gdbg_info(10,"predicted color: %08x\n",FBI_GET_COLOR(expect));
  
  return FBI_GET_COLOR(expect); 
}

/* This initiates the tests. */
void
main (int argc, char **argv)
{
  long n,x,y;
  FxU32 dstColor,srcColor,good;
  FxU32 alphaMode, fogMode, fbzMode;
  FxU32 rmode, rot;
  SstRegs *sst;
  
  sst = SST_BEGIN(argc,argv);

  /* Make it not dither. */
  fbzMode = SST_RGBWRMASK;
  if (diago.rgb == 16)
    fbzMode |= SST_ENALPHABUFFER | SST_ZAWRMASK;
  SET(sst->fbzMode, fbzMode);
  rmode = GET(sst->renderMode);
  rmode |= SST_RM_DITHER_ROTATION;
  SET(sst->renderMode, rmode);
  rot = 0;
  
  /* Test all possible methods for one pass. */
  while (DIAG_STARTPASS())			
    for (n=0; n<400; n++)
    {
	if (iRandom(7) == 0)
	if (diago.rgb < 32) {
	    fbzMode ^= SST_ENDITHERSUBTRACT;
	    if (iRandom(1)) fbzMode ^= SST_DITHER2x2;
	    SET(sst->fbzMode, fbzMode);
	    if (diago.rgb == 15) {
		// set random alpha replacement mode
		rmode &= ~SST_RM_ALPHAMODE;
		rmode |= iRandom(2)<<SST_RM_ALPHAMODE_SHIFT;
		SET(sst->renderMode, rmode);
	    }
	}

      /* Make sure that the first pixel is not affected by the previous alpha mode. */
      SET(sst->alphaMode, 0x00000000);
      
      /* Pick a random position. */
      xyRandom(&x,&y);
      rot = iRandom(3);

      /* Choose two random colors. */
      dstColor = iRandom(0xffffffff); /* Alpha gets lost from the first pixel. */
      srcColor = iRandom(0xffffffff);
      GDBG_INFO(3,"src=0x%08x dst=0x%08x ditsub=%d  dit_rot=%d\n",
		srcColor,dstColor, (fbzMode & SST_ENDITHERSUBTRACT) != 0,rot);

      /* Draw one pixel to the frame buffer. */
      ablend_drawpixel(sst, x, y, dstColor, rmode);

      /* Generate a random alpha mode which blends but does nothing else. */
      do
	{
	  alphaMode = SST_ENALPHABLEND;
	  alphaMode |= (iRandom(0xFFFFFF) & (SST_RGBSRCFACT |
					 SST_RGBDSTFACT |
					 SST_ASRCFACT |
					 SST_ADSTFACT));
	} while (!goodAlphaMode(alphaMode,fbzMode));

      if(diago.aaEnabled)
	fogMode = rot<<SST_DITHER_ROTATE_BLEND_AA_SHIFT;
      else
	fogMode = rot<<SST_DITHER_ROTATE_BLEND_SHIFT;

      if (iRandom(1)) fogMode |= SST_RGB_BLEND_REVERSE;
      if (iRandom(1)) fogMode |= SST_RGB_BLEND_SUB;
      if (iRandom(1)) fogMode |= SST_A_BLEND_REVERSE;
      if (iRandom(1)) fogMode |= SST_A_BLEND_SUB;

      /* Set the alpha mode register. */
      SET(sst->alphaMode, alphaMode);
      SET(sst->fogMode, fogMode);
      
      /* Draw the other pixel with this alpha mode. */
      ablend_drawpixel(sst, x, y, srcColor, rmode);
      if (diago.rgb == 15) {	// smear the alpha MSB
	if (dstColor & 0x80000000) 
	    dstColor |= 0xFF000000;
	else dstColor &= 0x00FFFFFF;
	if ((rmode & SST_RM_ALPHAMODE) == SST_RM_ALPHA_ZERO) dstColor &= 0x00FFFFFF;
	if ((rmode & SST_RM_ALPHAMODE) == SST_RM_ALPHA_ONE) dstColor |= 0xFF000000;
      }

      sst_idle(sst);			// wait for the command to complete
      good = ablend_verify(alphaMode, fogMode, fbzMode, rmode, dstColor,srcColor,x,y);
      if (diago.rgb == 16) {
	DIAG_TEST_PIXEL(diago.curdrawbuffer,x,y,good & 0x00FFFFFF);
	DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1,x,y,(unsigned short)(good>>24));
      }
      else {
	DIAG_TEST_PIXEL(diago.curdrawbuffer,x,y,good);
      }
    }
  
  DIAG_PASS(0);
}

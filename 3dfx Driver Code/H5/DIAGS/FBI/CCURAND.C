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
** $Date: 10/11/00 8:10:04 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

/* These are here to make Scott's verification code work. */
#include "fbi.h"

/* This is for generating RGB colors. */
#define S	(1<<SST_RGBA_FRACBITS)

/* These are macros for dealing with Scott's code. */
/* These produce an unsigned long, but without alpha. */
#define FBI_SET_COLOR(x,c) x.alpha=(short)((c>>24)&0xff); \
                           x.red=(short)((c>>16)&0xff); \
                           x.green=(short)((c>>8)&0xff); \
                           x.blue=(short)((c)&0xff)
#define FBI_GET_COLOR(x) (((x.red&0xff)<<16)|((x.green&0xff)<<8)|(x.blue&0xff))
#define FBI_PRINT_COLOR(c) printf("r: %x  g: %x  b: %x\n",c.red,c.green,c.blue)

int usingLFB;	// GMT: global hack

unsigned long ccu_verify(unsigned long fbzColorPath,
			 unsigned long combineModereg,
			 unsigned long c0,
			 unsigned long c1,
			 unsigned long iter,
			 unsigned long tex,
			 unsigned long lfb,
			 unsigned long z_iter,
			 long x, 
			 long y)
{
  /* These are the structures which are passed to Scott's verification code. */
  fbiColors rgba_iter, rgba_tex, rgba_lfb, color1, color0;
  fbiColors expect;

  FBI_SET_COLOR(rgba_iter,iter);
  FBI_SET_COLOR(rgba_tex,tex);
  FBI_SET_COLOR(rgba_lfb,lfb);
  FBI_SET_COLOR(color0,c0);
  FBI_SET_COLOR(color1,c1);

  if (usingLFB) {
    fbzColorPath &= ~(SST_RGBSELECT | SST_ASELECT);
    fbzColorPath |= SST_RGBSEL_LFB | SST_ASEL_LFB;
    combineModereg &= ~(SST_CM_CC_OTHERSELECT | SST_CM_CCA_OTHERSELECT);
    combineModereg |= SST_CM_CC_OTHERSELECT_LFB_RGB | SST_CM_CCA_OTHERSELECT_LFB_A;
  }
    
  colorcombine(fbzColorPath,combineModereg,
		usingLFB ? &rgba_lfb: &rgba_iter,&rgba_tex,&rgba_lfb,
		&color1,&color0,
		(z_iter>>(SST_Z_INTBITS-8)) & 0xFF,
		0,
		&expect);

  gdbg_info(10,"predicted color: %08x\n",FBI_GET_COLOR(expect));

  return FBI_GET_COLOR(expect); 
}


/* This is the drawpixel function used to test the CCU. */
int ccu_drawpixel(SstRegs *sst,
		  unsigned long fbzCP, 
		  unsigned long combineMode,
		  int x, int y,
		  unsigned long col0, 
		  unsigned long col1,
		  unsigned long collfb,
		  unsigned long zacol,
		  unsigned long riter, 
		  unsigned long giter, 
		  unsigned long biter, 
		  unsigned long aiter,
		  unsigned long ziter
		  )
{
  gdbg_info(10,"  drawpixel(%d,%d)\n",x,y);

  /* Set the registers with simple values. */
  SET(sst->zaColor, zacol);
  SET(sst->c0, col0);
  SET(sst->c1, col1);
  SET(sst->r, riter);
  SET(sst->g, giter);
  SET(sst->b, biter);
  SET(sst->a, aiter);
  SET(sst->z, ziter);

  /* Set the color path. */
  SET(sst->fbzColorPath,fbzCP);
  SET_FBI(sst->combineMode,combineMode);

  // randomly choose between LFB and triangle
  if (usingLFB) {
    unsigned long *lfb =  (unsigned long *)SST_LFB_ADDRESS(sst);
    gdbg_info(5, "using LFB access\n");
    SET(sst->lfbMode,(SST_LFB_8888 | SST_LFB_ENPIXPIPE));
    SET(lfb[lfbOffset(x,y)],collfb);
  }
  else {
    //setPixelsPerClock toggles between 1 and 2 pixels per clock rendering
    //if appropriate (i.e. --pixelsPerClock <= 0)
    setPixelsPerClock(sst);

    gdbg_info(5, "using TRIANGLE command\n");
    // set x,y coords
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
  return usingLFB;
}

/* This initiates the tests. */
void
main (int argc, char **argv)
{
  long n,x,y;
  unsigned long csrc0,csrc1,lfb,zaColor,good;
  unsigned long riter,giter,biter,aiter,ziter;
  unsigned char tex_a,tex_c;
  unsigned long fbzCP,combineMode;
  SstRegs *sst;

  sst = SST_BEGIN(argc,argv);

  /* Make it not dither. */
  SET(sst->fbzMode, SST_RGBWRMASK );

  // some misc. init stuff
  SET(sst->drdx,0);
  SET(sst->dgdx,0);
  SET(sst->dbdx,0);
  SET(sst->dadx,0);
  SET(sst->dzdx,0);

  /* Test all possible methods for one pass. */
  while (DIAG_STARTPASS())
  for (n=0; n<100; n++)
    {
      /* Pick a random position. */
      xyRandom(&x,&y);

      /* Choose two random colors... */
      csrc0 = iRandom(0xFFFFFFFF);
      csrc1 = iRandom(0xFFFFFFFF);
      zaColor = iRandom(0xFFFF);
      lfb = colRandom24();
      
      /* Choose random RGBA iterated colors. */
      riter = iRandom(SST_MASK(SST_RGBA_INTBITS));
      giter = iRandom(SST_MASK(SST_RGBA_INTBITS));
      biter = iRandom(SST_MASK(SST_RGBA_INTBITS));
      aiter = iRandom(SST_MASK(SST_RGBA_INTBITS));
      ziter = iRandom(SST_MASK(SST_Z_INTBITS));
      
      /* Randomly decide if the texture color and texture 
	 alpha will be one or zero. */
      tex_a = iRandom(1);
      tex_c = iRandom(1);

      /* Choose a random color path, throw it out and try 
	 again if it is not a valid color path. */
      while( !goodCcuPath(fbzCP = iRandom(SST_MASK(25)),FXTRUE));
      // force zero alpha
      fbzCP &= ~(SST_CCA_MSELECT|SST_CCA_ADD_CLOCAL|SST_CCA_ADD_ALOCAL|SST_CCA_INVERT_OUTPUT);
      fbzCP |= SST_CCA_MONE | SST_CCA_REVERSE_BLEND;

      combineMode = 0;
      // randomly switch over to combineMode settings
      if (iRandom(1)) {
	FxU32 temp;
	combineMode |= SST_CM_USE_COMBINE_MODE;
	// set new random modes
	do {
	    temp = iRandom(7) << SST_CM_CC_OTHERSELECT_SHIFT;
	} while ((temp == SST_CM_CC_OTHERSELECT_TRGB) ||
		 (temp == SST_CM_CC_OTHERSELECT_TA) ||
		 (temp == SST_CM_CC_OTHERSELECT_LFB_RGB));
	combineMode |= temp;
	do {
	    temp = iRandom(7) << SST_CM_CC_LOCALSELECT_SHIFT;
	} while ((temp == SST_CM_CC_LOCALSELECT_TRGB) ||
		 (temp == SST_CM_CC_LOCALSELECT_TA));
	combineMode |= temp;
	combineMode |= iRandom(3) << SST_CM_CC_MSELECT_7_SHIFT;
	combineMode |= iRandom(3) << SST_CM_CC_INVERT_OTHER_SHIFT;
	combineMode |= iRandom(3) << SST_CM_CC_INVERT_LOCAL_SHIFT;
	if (iRandom(1)) combineMode |= SST_CM_CC_INVERT_ADD_LOCAL;
	combineMode |= iRandom(2) << SST_CM_CC_OUTSHIFT_SHIFT;
      }
      gdbg_info(2,"Color Path = 0x%x\n", fbzCP );
      gdbg_info(2,"Combi Mode = 0x%x\n", combineMode );
      
      gdbg_info(2,"csrc0 = 0x%x\n", csrc0 );
      gdbg_info(2,"csrc1 = 0x%x\n", csrc1 );
      gdbg_info(2,"lfb   = 0x%x\n", lfb );

      gdbg_info(2,"riter = 0x%x\n", riter );
      gdbg_info(2,"giter = 0x%x\n", giter );
      gdbg_info(2,"biter = 0x%x\n", biter );
      gdbg_info(2,"aiter = 0x%x\n", aiter );
      gdbg_info(2,"ziter = 0x%x\n", ziter );
      gdbg_info(2,"zaCol = 0x%x\n", zaColor );

      usingLFB = iRandom(1);
      ccu_drawpixel(sst,
		    fbzCP,combineMode,
		    x,y,
		    csrc0,csrc1,lfb,zaColor,
		    riter*S,
		    giter*S,
		    biter*S,
		    aiter*S,
		    usingLFB ? zaColor : ziter);
      sst_idle(sst);				// wait for the command to complete

      good = ccu_verify(fbzCP,combineMode,
			csrc0,csrc1,
			(((aiter&0xff)<<24)|
			 ((riter&0xff)<<16)|
			 ((giter&0xff)<<8)|
			 (biter&0xff)),
			0x00000000,
			lfb,
			usingLFB ? zaColor : (ziter>>SST_Z_FRACBITS),
			x,y);
      DIAG_TEST_PIXEL(diago.curdrawbuffer,
		     x,y,
		     sst_argb_form_result(good,0,0,0));
    }
  
  DIAG_PASS(0);
}

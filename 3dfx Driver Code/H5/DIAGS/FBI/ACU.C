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
** $Date: 10/11/00 8:09:57 PM$
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
#define FBI_GET_COLOR(x) (((x.alpha&0xff)<<24)|((x.red&0xff)<<16)|((x.green&0xff)<<8)|(x.blue&0xff))
#define FBI_PRINT_COLOR(c) printf("r: %x  g: %x  b: %x\n",c.red,c.green,c.blue)


unsigned long acu_verify(unsigned long fbzColorPath,
			 unsigned long combineModereg,
			 unsigned long c0,
			 unsigned long c1,
			 unsigned long iter,
			 unsigned long tex,
			 unsigned long lfb,
			 unsigned long z_iter,
			 unsigned long w_iter,
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

  colorcombine(fbzColorPath,combineModereg,
		&rgba_iter,&rgba_tex,&rgba_lfb,
		&color1,&color0,z_iter,w_iter,&expect);

  // Route alpha into the other colors, since other colors are FF and get
  // multiplied by alpha in the alphablending stage
  expect.red = expect.alpha;
  expect.green = expect.alpha;
  expect.blue = expect.alpha;

  if (diago.rgb < 32) {
    expect.red &= 0xf8;
    expect.green &= diago.rgb==16 ? 0xfc : 0xf8;
    expect.blue &= 0xf8;
  }

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
		  unsigned long riter, 
		  unsigned long giter, 
		  unsigned long biter, 
		  unsigned long aiter
		  )
{
  gdbg_info(10,"  drawpixel(%d,%d)\n",x,y);

  /* Set the registers with simple values. */
  SET(sst->c0, col0);
  SET(sst->c1, col1);
  SET(sst->r, riter);
  SET(sst->g, giter);
  SET(sst->b, biter);
  SET(sst->a, aiter);

  /* Set the color path. */
  SET(sst->fbzColorPath,fbzCP);
  SET_FBI(sst->combineMode,combineMode);

  //setPixelsPerClock toggles between 1 and 2 pixels per clock rendering
  //if appropriate (i.e. --pixelsPerClock <= 0)
  setPixelsPerClock(sst);

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
  return 0;
}


/* This determines if the randomly-generated color path is a 
   valid one. */
int goodPath(unsigned long fbzColorPath)
{
  /* Alpha RGB select. */
  if(( (fbzColorPath & SST_ASELECT) == SST_ASEL_TREXOUT) ||
     ( (fbzColorPath & SST_ASELECT) == SST_ASEL_LFB))
    {
      gdbg_info(100,"  Color path invalidated by: Alpha RGB select    %x\n",fbzColorPath);
      return 0;
    }
  /* Alpha mselect. */
  if(( ((fbzColorPath & SST_CCA_MSELECT) >> SST_CCA_MSELECT_SHIFT) == 0x04) ||
     ( ((fbzColorPath & SST_CCA_MSELECT) >> SST_CCA_MSELECT_SHIFT) == 0x07))
    {
      gdbg_info(100,"  Color path invalidated by: Alpha mselect       %x\n",fbzColorPath);
      return 0;
    }

  /* Enabled TREX input. */
  if( fbzColorPath & SST_ENTEXTUREMAP )
    {
      gdbg_info(100,"  Color path invalidated by: Enabled TREX        %x\n",fbzColorPath);
      return 0;
    }

  if((fbzColorPath & (SST_CCA_ADD_CLOCAL|SST_CCA_ADD_ALOCAL)) == (SST_CCA_ADD_CLOCAL|SST_CCA_ADD_ALOCAL))
    {
      gdbg_info(100,"  Color path invalidated by: Invalid add mode    %x\n",fbzColorPath);
      return 0;
    }
  
  /* If it makes it through all of that... */
  return 1;
}


/* This initiates the tests. */
void
main (int argc, char **argv)
{
  long n,x,y;
  unsigned long csrc0,csrc1;
  unsigned long riter,giter,biter,aiter,ziter,witer;
  unsigned long fbzCP,combineMode;
  SstRegs *sst;

  sst = SST_BEGIN(argc,argv);

  /* Make it not dither. */
  SET(sst->fbzMode, SST_RGBWRMASK );

  // some misc. init stuff
  ziter = witer = 0;
  SET(sst->z,0);
  SET(sst->w,0);
  SET(sst->drdx,0);
  SET(sst->dgdx,0);
  SET(sst->dbdx,0);
  SET(sst->dadx,0);

  // set up alphablending to multiply source alpha (that comes out of the ACU)
  // by the source color, which is FF
  SET(sst->alphaMode, (SST_ENALPHABLEND|
		       ((SST_A_SRCALPHA<<SST_RGBSRCFACT_SHIFT)&SST_RGBSRCFACT)|
		       ((SST_A_ZERO<<SST_RGBDSTFACT_SHIFT)&SST_RGBDSTFACT)|
		       ((SST_A_ONE<<SST_ASRCFACT_SHIFT)&SST_ASRCFACT)|
		       ((SST_A_ZERO<<SST_ADSTFACT_SHIFT)&SST_ADSTFACT)));
  
  /* Test all possible methods for one pass. */
  while (DIAG_STARTPASS())			
  for (n=0; n<100; n++)
    {
      FxU32 mask = diago.rgb==16?0x00FFFFFF:(diago.rgb==15?0x80FFFFFF:0xFFFFFFFF);
      /* Pick a random position. */
      xyRandom(&x,&y);
      if (iRandom(10)==0) SET(sst->z,ziter=iRandom(0x0FFFFFFF));
      if (iRandom(10)==0) SETF(sst->Fw,(float)(witer=iRandom(0xFF)));

      /* Peg the the colors both to 1 so that alpha can be routed into the frame buffer. */
      csrc0 = 0x00ffffff & ((iRandom(0xff)&0xff)<<24);
      csrc1 = 0x00ffffff & ((iRandom(0xff)&0xff)<<24);
      
      /* Choose random RGBA iterated colors. */
      riter = 0x000000ff;
      giter = 0x000000ff;
      biter = 0x000000ff;
      aiter = iRandom(0xff)&0x000000ff;
      
      // Choose a random color path, throw it out and try 
      // again if it is not a valid color path
      do
	{
	  fbzCP = SST_CC_MONE;
	  /* This is a mask which represents the bits that affect only alpha. */
	  fbzCP |= (iRandom(SST_MASK(25)) & 0x03fe006c);
	} while( !goodPath(fbzCP) );
      combineMode = 0;

      // randomly switch over to combineMode settings
      if (iRandom(1)) {
	combineMode |= SST_CM_USE_COMBINE_MODE;
	// copy fbzCP other select mode and then zap fbzCP
	combineMode |= ((fbzCP & SST_ASELECT)>>SST_ASELECT_SHIFT) << SST_CM_CCA_OTHERSELECT_SHIFT;
	fbzCP &= ~SST_ASELECT;
	// set new random combineMode fields
	combineMode |= iRandom(3) << SST_CM_CCA_LOCALSELECT_SHIFT;
	combineMode |= iRandom(3) << SST_CM_CCA_INVERT_OTHER_SHIFT;
	combineMode |= iRandom(3) << SST_CM_CCA_INVERT_LOCAL_SHIFT;
	if (iRandom(1)) combineMode |= SST_CM_CCA_INVERT_ADD_LOCAL;
	combineMode |= iRandom(2) << SST_CM_CCA_OUTSHIFT_SHIFT;
      }

      gdbg_info(2,"Color Path = 0x%x\n", fbzCP );
      gdbg_info(2,"Combi Mode = 0x%x\n", combineMode );
      
      gdbg_info(2,"csrc0 = 0x%x\n", csrc0 );
      gdbg_info(2,"csrc1 = 0x%x\n", csrc1 );

      gdbg_info(2,"riter = 0x%x\n", riter );
      gdbg_info(2,"giter = 0x%x\n", giter );
      gdbg_info(2,"biter = 0x%x\n", biter );
      gdbg_info(2,"aiter = 0x%x\n", aiter );
      gdbg_info(2,"ziter = 0x%x\n", ziter );
      gdbg_info(2,"witer = 0x%x\n", witer );

      /* Clear the pixel first to make sure that it doesn't interfere with the alpha blend. */
      SET(sst->combineMode,0);
      sst_drawpixel(sst,x,y,0x00000000,0x00);

      ccu_drawpixel(sst,
		    fbzCP,combineMode,
		    x,y,
		    csrc0,csrc1,
		    riter*S,
		    giter*S,
		    biter*S,
		    aiter*S
		    );
      
      sst_idle(sst);			// wait for the command to complete
      DIAG_TEST_PIXEL(diago.curdrawbuffer,
		     x,y,
		     acu_verify(fbzCP,combineMode,
				csrc0,csrc1,
				(((aiter&0xff)<<24)|
				 ((riter&0xff)<<16)|
				 ((giter&0xff)<<8)|
				 (biter&0xff)),
				0,0,
				(ziter>>20) & 0xFF,
				witer,
				x,y) & mask
		     );
    }
  
  DIAG_PASS(0);
}

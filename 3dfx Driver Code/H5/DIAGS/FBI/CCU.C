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
** $Date: 10/11/00 8:10:03 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

/* These are here to make Scott's verification code work. */
#include "fbi.h"

/* These are macros for dealing with Scott's code. */
/* These produce an unsigned long, but without alpha. */
#define FBI_SET_COLOR(x,c) x.alpha=(short)((c>>24)&0xff); \
                           x.red=(short)((c>>16)&0xff); \
                           x.green=(short)((c>>8)&0xff); \
                           x.blue=(short)((c)&0xff)
#define FBI_GET_COLOR(x) (((x.red&0xff)<<16)|((x.green&0xff)<<8)|(x.blue&0xff))
#define FBI_PRINT_COLOR(c) printf("r: %x  g: %x  b: %x\n",c.red,c.green,c.blue)

int usingLFB;

/* A copy is kept of the current color path, to be passed into the
   verification function. */
unsigned long fbzCPcpy;


unsigned long ccu_verify(unsigned long fbzColorPath,
			 unsigned long c0,
			 unsigned long c1,
			 long x,
			 long y)
{
  /* These are the structures which are passed to Scott's verification code. */
  fbiColors rgba_iter, rgba_tex, rgba_lfb, color1, color0;
  short z_iter;
  fbiColors expect;

  FBI_SET_COLOR(rgba_iter,0x00000000);
  FBI_SET_COLOR(rgba_tex,0x00000000);
  FBI_SET_COLOR(rgba_lfb,0x00000000);
  FBI_SET_COLOR(color0,c0);
  FBI_SET_COLOR(color1,c1);
  z_iter = 0;

  colorcombine(fbzColorPath,0,
		usingLFB ? &rgba_lfb : &rgba_iter,&rgba_tex,&rgba_lfb,
		&color1,&color0,z_iter,0,&expect);

  return FBI_GET_COLOR(expect); 
}


/* This is the drawpixel function used to test the CCU. */
int ccu_drawpixel(SstRegs *sst,int x, int y,
		  unsigned long col0, unsigned long col1,
		  int method)
{
  gdbg_info(10,"  drawpixel(%d,%d)\n",x,y);
  
  switch(method)
    {
    case 0:
      /* C0 passthrough */
      fbzCPcpy = (SST_RGBSEL_C1 | SST_LOCALSELECT |
		  SST_ASEL_C1 | SST_ALOCAL_C0 |
		  SST_CC_MONE | SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL
		  );
      break;
      
    case 1:
      /* Pass C1 right through. */
      fbzCPcpy = (SST_RGBSEL_C1 | SST_CC_MONE );
      break;

    case 2:
      /* Blend C0 and C1 using the alpha from C0 */
      /* (C1-C0)alpha+C0 */
      fbzCPcpy = (SST_RGBSEL_C1 | SST_LOCALSELECT |
	   SST_ASEL_C1 | SST_ALOCAL_C0 | 
	   SST_CC_MAOTHER | SST_CC_SUB_CLOCAL | SST_CC_ADD_CLOCAL );
      if (iRandom(1))
	fbzCPcpy |= SST_CC_REVERSE_BLEND;

    case 3:
      /* Add C0 and C1 */
      fbzCPcpy = (SST_RGBSEL_C1 | SST_LOCALSELECT |
		  SST_CC_MONE | SST_CC_ADD_CLOCAL );
      break;

    case 4:
      /* Sub C0 from C1 */
      fbzCPcpy = (SST_RGBSEL_C1 | SST_LOCALSELECT |
		  SST_CC_MONE | SST_CC_SUB_CLOCAL );
      break;

    case 5:
      /* Mult C0 and C1 */
      fbzCPcpy = (SST_RGBSEL_C1 | SST_LOCALSELECT |
		  SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND );
      break;

    case 6:
      /* Zero */
      fbzCPcpy = (SST_RGBSEL_C1 | SST_LOCALSELECT |
		  SST_CC_MZERO );
      break;

    case 7:
      /* One */
      fbzCPcpy = (SST_RGBSEL_C1 | SST_LOCALSELECT |
		  SST_CC_MZERO | SST_CC_INVERT_OUTPUT); 
      break;

    case 8:
      /* Blend C0 and C1 using the alpha from C0 */
      /* (C1-C0)alpha+C0 */
      fbzCPcpy = (SST_RGBSEL_C1 | SST_LOCALSELECT |
		  SST_ASEL_C1 | SST_ALOCAL_C0 | 
		  SST_CC_MALOCAL | SST_CC_SUB_CLOCAL | SST_CC_ADD_CLOCAL );      
      break;

    case 9:
      /* Blend C0 and C1 using the alpha from C0 */
      /* (C1-C0)!alpha+C0 */
      /* This is exactly the same as the previous method, but the 
	 alpha reverser is on, so the line should be exactly the
	 same as the line drawn by the above method, but backwards. */
      fbzCPcpy = (SST_RGBSEL_C1 | SST_LOCALSELECT |
	   SST_ASEL_C1 | SST_ALOCAL_C0 | SST_CC_REVERSE_BLEND |
	   SST_CC_MALOCAL | SST_CC_SUB_CLOCAL | SST_CC_ADD_CLOCAL );     
      if (iRandom(1))
	fbzCPcpy |= SST_CC_REVERSE_BLEND;
      break;
    }
  
      // force zero alpha
      fbzCPcpy &= ~(SST_CCA_MSELECT|SST_CCA_ADD_CLOCAL|SST_CCA_ADD_ALOCAL|SST_CCA_INVERT_OUTPUT);
      fbzCPcpy |= SST_CCA_MONE | SST_CCA_REVERSE_BLEND;

      SET(sst->fbzColorPath,fbzCPcpy);
      SET(sst->c0, col0);
      SET(sst->c1, col1);

  // set x,y coords
  // randomly choose between LFB and triangle
  if (usingLFB=iRandom(1)) {
    unsigned long *lfb =  (unsigned long *)SST_LFB_ADDRESS(sst);
    gdbg_info(5, "using LFB access\n");
    SET(sst->lfbMode,(SST_LFB_8888 | SST_LFB_ENPIXPIPE));
    SET(lfb[lfbOffset(x,y)],col1);
  }
  else {
    //setPixelsPerClock toggles between 1 and 2 pixels per clock rendering
    //if appropriate (i.e. --pixelsPerClock <= 0)
    setPixelsPerClock(sst);

    gdbg_info(5, "using TRIANGLE command\n");
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


/* This initiates the tests. */
void
main (int argc, char **argv)
{
  int n,j;
  long xinitial,x,y;
  unsigned long a;
  unsigned long csrc0,csrc1, good;
  SstRegs *sst;
  int pass_elevation = 0;

  sst = SST_BEGIN(argc,argv);

  /* Make it not dither. */
  SET(sst->fbzMode, SST_RGBWRMASK );

  /* Test all possible methods for one pass. */
  while (DIAG_STARTPASS())
  for (j=0; j<5; j++)
    {
      /* Choose two random 24-bit colors... */
      csrc0 = colRandom24();
      csrc1 = colRandom24(); 
      
      gdbg_info(2,"csrc0 = 0x%x\n", csrc0 );
      gdbg_info(2,"csrc1 = 0x%x\n", csrc1 );

      xinitial = 30;
      x = xinitial;
      
#define NUM_PER_PASS 9
      /* ...and test all known combine modes with those two colors. */
      for (n=0; n<NUM_PER_PASS; n++) 
	{		      
	  y = pass_elevation + 5; /* Draw lines where we can see them. */

	  gdbg_info(4,"method number = %d\n", n );

	  /* Unless the alpha matters (in the blend tests) set a random 
	     alpha to make sure that it does not get used in any way. */
	  switch(n)
	    {
	    case 0:
	    case 1:
	    case 3:
	    case 4:
	    case 5:
	    case 6:
	    case 7:
	      /* Blend in the random alpha. */
	      csrc0 |= (iRandom(0xff) << 24);
	      csrc1 |= (iRandom(0xff) << 24);
	      break;
	    case 2:
	    case 8:
	      /* Clear the alpha byte so that it can be set later. */
	      csrc0 &= (0xffffff);
	      csrc1 &= (0xffffff);
	      break;
	    default:
	      break;
	    }
	  
	  /* This switch is here so that each individual drawing method can 
	     be commented and placed in (x,y) individually. */
	  switch(n)
	    {
	    case 0:
	      /* Draw using C0 */
	      ccu_drawpixel(sst,x,y,
			    csrc0,csrc1,n);
	      break;
	      
	    case 1:
	      /* Draw using C1 */
	      x+=5;
	      ccu_drawpixel(sst,x,y,
			    csrc0,csrc1,n);
	      break;

	    case 2:
	      /* These two test blending using the other alpha instead
		 of the local alpha.  Both ways are tested, using the
		 reverse blend bit and not. */
	      /* Test all possible alphas for these chosen colors. */
	      for(a=0;a<256;a+=iRandom(32))
		{
		  /* Blend the current alpha into both colors. */
		  /* draw a pixel using the current method. */
		  ccu_drawpixel(sst,(xinitial+a),y+n+1,
				(csrc0|(a<<24)),(csrc1|(a<<24)),n);
		  
		  /* wait for the command to complete */
		  sst_idle(sst);			  

		  good = ccu_verify(fbzCPcpy,(a<<24)|csrc0,(a<<24)|csrc1,x,y); 
		  DIAG_TEST_PIXEL(diago.curdrawbuffer,
				(xinitial+a),y+n+1,
				sst_argb_form_result(good,0,0,0));
		}
	      break;

	    case 3:
	      /* Draw using C0 + C1 */
	      x+=5;
	      ccu_drawpixel(sst,x,y,
			    csrc0,csrc1,n);
	      break;

	    case 4:
	      /* Draw using C1 - C0 */
	      x+=5;
	      ccu_drawpixel(sst,x,y,
			    csrc0,csrc1,n);
	      break;

	    case 5:
	      /* Draw using C1 * C0 */
	      x+=5;
	      ccu_drawpixel(sst,x,y,
			    csrc0,csrc1,n);
	      break;

	    case 6:
	      /* Draw using SST_CC_ZERO */
	      x+=5;
	      ccu_drawpixel(sst,x,y,
			    csrc0,csrc1,n);
	      break;

	    case 7:
	      /* Draw using SST_CC_ONE */
	      x+=5;
	      ccu_drawpixel(sst,x,y,
			    csrc0,csrc1,n);
	      break;

	    case 8:
	      /* These two test blending using the local alpha instead
		 of the other alpha.  Both ways are tested, using the
		 reverse blend bit and not. */
	      /* Test all possible alphas for these chosen colors. */
	      for(a=0;a<256;a+=iRandom(32))
		{
		  /* Blend the current alpha into both colors. */
		  /* draw a pixel using the current method. */
		  ccu_drawpixel(sst,(xinitial+a),y+n-1,
				(csrc0|(a<<24)),(csrc1|(a<<24)),n);
		  
		  /* wait for the command to complete */
		  sst_idle(sst);			  

		  good = ccu_verify(fbzCPcpy,(a<<24)|csrc0,(a<<24)|csrc1,x,y); 
		  DIAG_TEST_PIXEL(diago.curdrawbuffer,
				(xinitial+a),y+n-1,
				sst_argb_form_result(good,0,0,0));
		}
	      break;
	    }

	  /* This will evaluate the pixel to make sure it now has the 
	     appropriate value. */
	  if( (n!=2) && (n!=8) ) {
		/* wait for the command to complete */
		sst_idle(sst);			  
		good = ccu_verify(fbzCPcpy,csrc0,csrc1,x,y); 
		DIAG_TEST_PIXEL(diago.curdrawbuffer,
				x,y,
				sst_argb_form_result(good,0,0,0));
	  }
	}
      
      if( pass_elevation > 200 )
	pass_elevation = 0;
      else
	pass_elevation += NUM_PER_PASS + 4;
	 
    }
  
//  getchar();  // Stay onscreen for a second...
  
  DIAG_PASS(0);
}

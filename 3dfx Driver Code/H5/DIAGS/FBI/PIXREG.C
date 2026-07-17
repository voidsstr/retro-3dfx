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
** $Date: 10/11/00 8:10:22 PM$
*/

#include "udiag.h"
#include "sstdiag.h"


void main (int argc, char **argv)
{
    enum { CHROMA, AFUNC, ZFUNC, STENCIL } pixRegister;
    int n;
    long fbzMode;
    long colorPath;
    long alphaMode;
    long stencilMode;
    long x;
    long y;
    long chromaColor;
    long pixelColor;
    long failPixels;
    long totalPixels;
    
    SstRegs *sst;
    
    sst = SST_BEGIN(argc,argv);
    
    chromaColor = pixelColor = 0x004F4F4F;

    // Passthrough Alpha
    // SST_ASEL_C1 | SST_RGBSEL_C1
    
    while (DIAG_STARTPASS())
      for ( pixRegister = CHROMA;
	    pixRegister <= (diago.rgb<32?ZFUNC:STENCIL);
//	    pixRegister <= (diago.rgb<32?ZFUNC:ZFUNC);
	    pixRegister++ )
      {
	  totalPixels = failPixels = 0;
	  SET( sst->nopCMD, 0x01 );		// reset counters
	  for (n=0; n<50; n++)			// 50 pixels per pixRegister per pass
	  {
	      if ( iRandom( 0x01 ) )
	      {
		  switch ( pixRegister )
		  {
		    case CHROMA:
		      gdbg_info( 2, "Chroma, FAIL\n" );
		      fbzMode = SST_RGBWRMASK | SST_ENCHROMAKEY;
		      colorPath = SST_RGBSEL_C1;
		      alphaMode = 0x0;
		      stencilMode = 0x0;
		      break;
		    case AFUNC:
		      gdbg_info( 2, "Alpha, FAIL\n" );
		      fbzMode = SST_RGBWRMASK;
		      colorPath = SST_RGBSEL_C1;
		      alphaMode = SST_ENALPHAFUNC;   // Alpha Never
		      stencilMode = 0x0;
		      break;
		    case ZFUNC:
		      gdbg_info( 2, "Z, FAIL\n" );
		      fbzMode = SST_RGBWRMASK | SST_ZAWRMASK | SST_ENDEPTHBUFFER;
		      colorPath = SST_RGBSEL_C1;
		      alphaMode = 0x0;
		      stencilMode = 0x0;
		      break;
		    case STENCIL:
		      gdbg_info( 2, "STENCIL, FAIL\n" );
		      fbzMode = SST_RGBWRMASK;
		      colorPath = SST_RGBSEL_C1;
		      alphaMode = 0x0;
		      // stencilFunction is never!
		      stencilMode = SST_STENCIL_ENABLE |(3<<SST_STENCIL_REF_SHIFT);
		      break;
		  }
		  failPixels++;
	      }
	      else
	      {
		  switch ( pixRegister )
		  {
		    case CHROMA:
		      gdbg_info( 2, "Chroma, PASS\n" );
		      fbzMode = SST_RGBWRMASK;
		      colorPath = SST_RGBSEL_C1;
		      alphaMode = 0x0;
		      stencilMode = 0x0;
		      break;
		    case AFUNC:
		      gdbg_info( 2, "Alpha, PASS\n" );
		      fbzMode = SST_RGBWRMASK;
		      colorPath = SST_RGBSEL_C1;
		      alphaMode = 0x0;
		      stencilMode = 0x0;
		      break;
		    case ZFUNC:
		      gdbg_info( 2, "Z, PASS\n" );
		      fbzMode = SST_RGBWRMASK | SST_ZAWRMASK | SST_ENDEPTHBUFFER |
				SST_ZFUNC_LT | SST_ZFUNC_EQ | SST_ZFUNC_GT;
		      colorPath = SST_RGBSEL_C1;
		      alphaMode = 0x0;
		      stencilMode = 0x0;
		      break;
		    case STENCIL:
		      gdbg_info( 2, "STENCIL, PASS\n" );
		      fbzMode = SST_RGBWRMASK;
		      colorPath = SST_RGBSEL_C1;
		      alphaMode = 0x0;
		      stencilMode = SST_SFUNC_LT | SST_SFUNC_EQ | SST_SFUNC_GT;
		      break;
		    default:
		      GDBG_ERROR("pixreg.c::main()", "Internal error! Look at source");
		  }
	      }
	      totalPixels++;

	      xyRandom(&x,&y);
	      
	      SET(sst->fbzMode,fbzMode);
	      SET( sst->fbzColorPath, colorPath );
	      SET( sst->alphaMode, alphaMode );
	      SET( sst->stencilMode, stencilMode );
    	      SET(sst->chromaKey, chromaColor );
	      
	      sst_drawpixel( sst, x, y, pixelColor, 0 );
	      
	      sst_idle_really(sst);		// wait for the command to complete
	  }
	  

	  if(!diago.sliEnabled && !diago.aaEnabled)
	    {
	      switch( pixRegister )
		{
		case CHROMA:
		  gdbg_info( 2, "Chromakey Test total = %d, failed = %d\n", totalPixels, failPixels );
		  DIAG_TESTREG32( "fbiChromaFail", failPixels, GET( sst->stats.fbiChromaFail ) );
		  DIAG_TESTREG32( "fbiPixelsIn", totalPixels, GET( sst->stats.fbiPixelsIn ) );
		  DIAG_TESTREG32( "fbiPixelsOut", totalPixels - failPixels, GET( sst->stats.fbiPixelsOut ) );
		  break;
		case AFUNC:
		  gdbg_info( 2, "Alpha Test total = %d, failed = %d\n", totalPixels, failPixels );
		  DIAG_TESTREG32( "fbiAFuncFail", failPixels, GET( sst->stats.fbiAfuncFail ) );
		  DIAG_TESTREG32( "fbiPixelsIn", totalPixels, GET( sst->stats.fbiPixelsIn ) );
		  DIAG_TESTREG32( "fbiPixelsOut", totalPixels - failPixels, GET( sst->stats.fbiPixelsOut ) );
		  break;
		case ZFUNC:
		  gdbg_info( 2, "Z Test total = %d, failed = %d\n", totalPixels, failPixels );
		  DIAG_TESTREG32( "fbiZFuncFail", failPixels, GET( sst->stats.fbiZfuncFail ) );
		  DIAG_TESTREG32( "fbiPixelsIn", totalPixels, GET( sst->stats.fbiPixelsIn ) );
		  DIAG_TESTREG32( "fbiPixelsOut", totalPixels - failPixels, GET( sst->stats.fbiPixelsOut ) );
		  break;
		case STENCIL:
		  gdbg_info( 2, "Stencil Test total = %d, failed = %d\n", totalPixels, failPixels );
		  DIAG_TESTREG32( "fbiStencilFail", failPixels, GET( sst->fbiStencilFail ) );
		  DIAG_TESTREG32( "fbiPixelsIn", totalPixels, GET( sst->stats.fbiPixelsIn ) );
		  DIAG_TESTREG32( "fbiPixelsOut", totalPixels - failPixels, GET( sst->stats.fbiPixelsOut ) );
		  break;
		}
	    }
	  
      }
    DIAG_PASS(0);
}

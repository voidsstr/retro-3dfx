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
** $Date: 10/11/00 8:10:09 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "fbi.h"

#define _PACKCOLOR( s ) ( (((long) s.alpha) << 24) | \
			  (((long) s.red) << 16) | \
			  (((long) s.green) << 8 ) | \
			  (((long) s.blue)) )

#define S	(1<<SST_Z_FRACBITS)

enum FOGMODES { TESTFOGADD, TESTFOGALPHA, TESTFOGCONST, 
		TESTFOGENABLE, TESTFOGMULT, TESTFOGZ,
		TESTFOGW, TESTFOGDITHER, TESTFOGZONES,
		TESTFOG_ANY };

void main (int argc, char **argv)
{
    SstRegs *sst;
    
    // Counters
    FxU32 n,fogIndex;

    // Per Test Variables
    FxU32 x,y;
    
    FxU32 fMode;
    FxU32 color;
    FxU32 fogColor;
    FxU32 fogTable[64];
    FxU32 oow;

    enum FOGMODES diagFogMode;
    
    // Setup For Scott's fbi emulation
    fogMode fbiFogMode;
    fbiColors fbiSrcColor;
    fbiColors fbiFogColor;
    fbiColors diagColor;
    FxU16 iteratedAlpha;
    FxU16 iteratedZ;
    FxI16 wExponent;
    FxI16 wMantissa;

    // Parse the Command Line and Initialize the Simulator
    sst = SST_BEGIN( argc, argv );

    // Handle Options

    if ( diago.printOpts )
    {
	gdbg_printf( "fogtest option description:\n"  );
	gdbg_printf( " %d -> Test fogAdd    Bit( default )\n", TESTFOGADD );
	gdbg_printf( " %d -> Test fogAlpha  Bit\n", TESTFOGALPHA );
	gdbg_printf( " %d -> Test fogConst  Bit\n", TESTFOGCONST );
	gdbg_printf( " %d -> Test fogEnable Bit\n", TESTFOGENABLE );
	gdbg_printf( " %d -> Test fogMult   Bit\n", TESTFOGMULT );
	gdbg_printf( " %d -> Test fogZ      Bit\n", TESTFOGZ );
	gdbg_printf( " %d -> Test fogW      Bit\n", TESTFOGW );
	gdbg_printf( " %d -> Test fogDither Bit\n", TESTFOGDITHER );
	gdbg_printf( " %d -> Test fogZones  Bit\n", TESTFOGZONES );
	gdbg_printf( " %d -> Test random    Bits\n", TESTFOG_ANY );
	exit( 0 );
    }

    diagFogMode = diago.option;
    
    // Initialize for Simple Drawing
    SET(sst->fbzMode, SST_RGBWRMASK );
    SET(sst->dadx,0);
    SET(sst->dzdx,0);
    SET(sst->dwdx,0);
    
    // for each pass test 100 pixels
    while (DIAG_STARTPASS()) {		
      // Initialize the Fog Table To be a linear ramp of 3,6,9...
      // Store table unpacked since Scott's fog function doesn't utilized a
      //   packed table
      for ( fogIndex = 0; fogIndex < 64; fogIndex++ )
      {	// 0x0C is a slope of 3, and we randomly set the 2 LSBs
      	// which are ignored except for in fog_zones mode
    	fogTable[fogIndex] = ((fogIndex+1)*3*256) | 0x0C | iRandom(3);
      }

      // Download Fog Table and Pack on the Fly
      for ( fogIndex = 0; fogIndex < 64; fogIndex+=2 )
      {
      	SET( sst->fogTable[fogIndex/2], (fogTable[fogIndex + 1] << 16) | fogTable[fogIndex] );
      }

      for (n=0; n<150; n++)
      {
	  // Generate a Random Point
	  xyRandom(&x,&y);  
	  
	  // Test Selected Fog Function
	  switch( diagFogMode )
	  {
	    case TESTFOGADD:
	      fMode = SST_ENFOGGING | ( iRandom(1) ? SST_FOGADD : 0x0 ); 
	      break;
	    case TESTFOGALPHA:
	      fMode = SST_ENFOGGING | ( iRandom(1) ? SST_FOG_ALPHA : 0x0 ); 
	      break;
	    case TESTFOGCONST:
	      fMode = SST_ENFOGGING | (iRandom( 0x01 ) ? SST_FOG_CONSTANT : 0x0); 
	      break;
	    case TESTFOGENABLE:
	      fMode = ( iRandom( 0x01 ) ? SST_ENFOGGING : 0 ) | SST_FOG_CONSTANT;
	      break;
	    case TESTFOGMULT:
	      fMode = SST_ENFOGGING | ( iRandom(1) ? SST_FOGMULT : 0x0 );
	      break;
	    case TESTFOGZ:
	      fMode = SST_ENFOGGING | ( iRandom(1) ? SST_FOG_Z : 0x0 );
	      break;
	    case TESTFOGW:
	      fMode = SST_ENFOGGING | ( iRandom(1) ? SST_FOG_Z|SST_FOG_ALPHA : 0x0 );
	      break;
	    case TESTFOGDITHER:
	      fMode = SST_ENFOGGING | ( iRandom(1) ? SST_FOG_DITHER : 0x0 );
	      break;
	    case TESTFOGZONES:
	      fMode = SST_ENFOGGING | ( iRandom(1) ? SST_FOG_ZONES : 0x0 );
	      break;
	    case TESTFOG_ANY:
	      fMode = iRandom(0xFFFF);	// any old random bits
	      break;
	    default:
	      GDBG_ERROR("fogmodes", "Unimplemented Fogmode Test\n" );
	      DIAG_FAIL();
	      
	      break;
	  }
	  gdbg_info(2, "point (%.3d,%.3d) fog mode 0x%.8x\n", x, y, fMode );
	  
	  // Choose a random point and fog color;
	  color = colRandom32();
	  fogColor = colRandom24();
	  iteratedAlpha = (FxU16)(color>>24) & 0xFF;
	  iteratedZ = iRandom(0xFFFF);
          // Generate a random 1/w ,
	  if ((fMode&(SST_FOG_Z|SST_FOG_ALPHA))==(SST_FOG_Z|SST_FOG_ALPHA)) {
	    oow = iRandom(255);
	    SETF( sst->Fw, (float)oow);
	    wExponent = (FxI16)oow;
	  }
	  else {
            // we randomly generate a 1.30 fixed point number 
            // and then shift it right (which generates the random exponent value)
            oow = iRandom(1 << SST_W_FRACBITS) >> iRandom(15);
	    if (diago.rgb < 32) {
		wExponent = (FxI16)wBufferValue( oow ) >> 12;
		wMantissa = (FxI16)wBufferValue( oow ) & 0x0FFF;
	    }
	    else {
		wExponent = (FxI16)(wBufferValue( oow ) >> 19);
		wMantissa = (FxI16)((wBufferValue( oow ) >> 7)&0x0FFF);
		if (wExponent & 0x10) {
		    wExponent = 0xF;
		    wMantissa = (FxI16)0x0FFF;
		}
	    }
            SET( sst->w, oow );
	  }

	  // Configure Hardware for *this* pass
	  SET( sst->fogMode, fMode );
	  SET( sst->fogColor, fogColor);     
	  SET( sst->c1, color );
	  SET( sst->a, iteratedAlpha<<SST_RGBA_FRACBITS);

	  if(diago.rgb == 32)
	    SET( sst->z, iteratedZ<<(SST_Z_32BPP_INTBITS+SST_Z_32BPP_FRACBITS-16));
	  else
	    SET( sst->z, iteratedZ<<(SST_Z_16BPP_INTBITS+SST_Z_16BPP_FRACBITS-16));

	  // Draw Pixel and Wait for Command to Complete
	  sst_drawpixel( sst, x, y, color, 0 );
	  sst_idle(sst);			
	  
	  // Setup arguments to fog()
	  fbiFogMode.fog_enable = (fMode & SST_ENFOGGING)!=0;
	  fbiFogMode.fog_mult = (fMode & SST_FOGMULT)!=0;
	  fbiFogMode.fog_add = (fMode & SST_FOGADD)!=0;
	  fbiFogMode.fog_constant = (fMode & SST_FOG_CONSTANT)!=0;
	  fbiFogMode.fog_alpha = (fMode & SST_FOG_ALPHA)!=0;
	  fbiFogMode.fog_z = (fMode & SST_FOG_Z)!=0;
          fbiFogMode.fog_dither = (fMode & SST_FOG_DITHER)!=0;
          fbiFogMode.fog_zones = (fMode & SST_FOG_ZONES)!=0;
	  
          fbiSrcColor.alpha = (short)((color & 0xFF000000) >> 24);
	  fbiSrcColor.red = (short)((color & 0x00FF0000) >> 16);
	  fbiSrcColor.green = (short)((color & 0x0000FF00) >> 8);
	  fbiSrcColor.blue = (short)(color & 0x000000FF);
	  
	  fbiFogColor.red = (short)((fogColor & 0x00FF0000) >> 16);
	  fbiFogColor.green = (short)((fogColor & 0x0000FF00) >> 8);
	  fbiFogColor.blue = (short)(fogColor & 0x000000FF);
	  fbiFogColor.alpha = 0;

	  fog( &fbiFogMode, x,y,
	       &fbiSrcColor,
	       &fbiFogColor,
	       fogTable,
	       iteratedAlpha,
	       (short)(iteratedZ>>8),
	       wExponent,
	       wMantissa,
	       &diagColor );    

	  gdbg_info(10,"   src color %.8x\n", color );
	  color = _PACKCOLOR( diagColor );
	  gdbg_info(10,"   fog color %.8x\n", fogColor );
	  gdbg_info(10,"   diag color %.8x\n", color );

	  color = sst_argb_form_result(color,0,0,0);	  
          DIAG_TEST_PIXEL( diago.curdrawbuffer, x, y, color );
      }
    }

    DIAG_PASS(0);

}

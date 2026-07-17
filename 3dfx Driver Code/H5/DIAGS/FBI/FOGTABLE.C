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
** $Date: 10/11/00 8:10:10 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "fbi.h"

#define _PACKCOLOR( s ) ( (((long) s.alpha) << 24) | \
                          (((long) s.red) << 16) | \
                          (((long) s.green) << 8 ) | \
                          (((long) s.blue)) )

#define S       (1<<SST_Z_FRACBITS)


void main (int argc, char **argv)
{
    SstRegs *sst;
    
    // Counters
    FxU32 n;
    
    // Per Test Variables
    FxU32 x,y;
    FxU32 oow;
    FxU32 fMode;
    FxU32 color;
    FxU32 fogColor;
    FxU32 fogTable[FOG_TABLE_SIZE];
    
    // Setup For Scott's fbi emulation
    fogMode fbiFogMode;
    fbiColors fbiSrcColor;
    fbiColors fbiFogColor;
    fbiColors diagColor;
    FxI16 iteratedAlpha;
    FxI16 iteratedZ;
    FxI16 wExponent;
    FxI16 wMantissa;
    
    // Parse the Command Line and Initialize the Simulator
    sst = SST_BEGIN( argc, argv );
    
    if ( diago.printOpts ) {
	gdbg_printf( "fogtable option description:\n"  );
	gdbg_printf( " 0 -> Test basic for table\n");
	gdbg_printf( " 1 -> Test FOG_ZONES  Bit\n");
	exit( 0 );
    }

    // Initialize for Simple Drawing Based on C1
    SET(sst->fbzMode, SST_RGBWRMASK);
    SET( sst->fbzColorPath, SST_RGBSEL_C1 );
    
    // Set Fog Mode
    fMode = SST_ENFOGGING;
    if (diago.option) fMode |= SST_FOG_ZONES;
    
    // for each pass test 100 pixels
    while (DIAG_STARTPASS()) {
        // GMT: test out some very small values here
        // GMT: test out some very large values here
        for (n=0; n<200; n++)           
        {
            // Generate a Random Point
            xyRandom(&x,&y);  
            
            // Generate a random 1/w ,
            // we randomly generate a 1.30 fixed point number 
            // and then shift it right (which generates the random exponent value)
            oow = iRandom(1 << SST_W_FRACBITS) >> iRandom(15);
            gdbg_info(2,"point (%.3d,%.3d) foow=%.4x\n",x,y,wBufferValue( oow ) );
            
            // Choose a random point color and fog color;
	    color = colRandom32();
	    fogColor = colRandom24();
            
            // Configure Hardware for *this* pass
            SET( sst->fogMode, fMode );
            SET( sst->fogColor, fogColor);     
            SET( sst->c1, color );
            SET( sst->w, oow );
            SET( sst->zaColor, oow>>(SST_W_FRACBITS-16));
            sst_random_fog_table(sst,fogTable);
	    
            // Draw Pixel and Wait for Command to Complete
            if (sst_drawpixel( sst, x, y, color, 0 )) {
                // if an LFB access then reduce oow to the resolution of zaColor
                oow &= 0xFFFF<<(SST_W_FRACBITS-16);
            }
            sst_idle(sst);
            
            // Setup arguments to fog
            fbiFogMode.fog_enable = (fMode & SST_ENFOGGING)!=0;
            fbiFogMode.fog_mult = (fMode & SST_FOGMULT)!=0;
            fbiFogMode.fog_add = (fMode & SST_FOGADD)!=0;
            fbiFogMode.fog_constant = (fMode & SST_FOG_CONSTANT)!=0;
            fbiFogMode.fog_alpha = (fMode & SST_FOG_ALPHA)!=0;
            fbiFogMode.fog_z = (fMode & SST_FOG_Z)!=0;
            fbiFogMode.fog_dither = 0;
            fbiFogMode.fog_zones = diago.option != 0;
            
            fbiSrcColor.alpha = (short)((color & 0xFF000000) >> 24);
            fbiSrcColor.red =  (short)((color & 0x00FF0000) >> 16);
            fbiSrcColor.green = (short)((color & 0x0000FF00) >> 8);
            fbiSrcColor.blue = (short)(color & 0x000000FF);
            
            fbiFogColor.red = (short)((fogColor & 0x00FF0000) >> 16);
            fbiFogColor.green = (short)((fogColor & 0x0000FF00) >> 8);
            fbiFogColor.blue = (short)(fogColor & 0x000000FF);
            fbiFogColor.alpha = 0;
            
            iteratedAlpha = 0;
            
            iteratedZ = 0;
            
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
            
            fog( &fbiFogMode, x,y,
                 &fbiSrcColor,
                 &fbiFogColor, 
                 fogTable,     
                 iteratedAlpha,   
                 iteratedZ,
                 wExponent,
                 wMantissa,
                 &diagColor ); 
            
            gdbg_info(10,"   src color %.8x\n", color );
            color = _PACKCOLOR( diagColor );
            gdbg_info(10,"   fog color %.8x\n", fogColor );
            gdbg_info(10,"   diag color %.8x\n", color );
	    color = sst_argb_form_result(color,0,0,0);	  
            DIAG_TEST_PIXEL( diago.curdrawbuffer, x, y,	color );
        }
    }    
    DIAG_PASS(0);
}

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
** $Date: 10/11/00 8:10:49 PM$
*/

#define INRANGE( subject, low, high ) ( ((subject) >= (low)) && ((subject) <= (high)) )

#include "udiag.h"
#include "sstdiag.h"


void main (int argc, char **argv)
{
    int spanlen;
    long i, j;		// Counters
    long exp;
    long x;
    long y;
    long oow1;		// 1 / w <2.30>
    long dwdx;		// doow/dx <2.30>
    int bias;
    int bufferVal;
    
    
    double fw1;               // floating point...
    double fw2;
    double foow1;
    double foow2;
    double fdwdx;
    
    SstRegs *sst;
    
    sst = SST_BEGIN(argc,argv);

    SET(sst->fbzMode, SST_ENDEPTHBUFFER | SST_WBUFFER | SST_ENZBIAS |
	SST_ZAWRMASK | SST_RGBWRMASK | SST_ZFUNC_GT | SST_ZFUNC_EQ | SST_ZFUNC_LT);
    
    SET( sst->fbzColorPath, SST_RGBSEL_C1 );
    
    while( DIAG_STARTPASS() )
    {
	// Test Exponents in the range of [2**17,2**-1) for 16bpp. up to 2**30 for 32bpp
	for ( exp=0; exp<(diago.rgb<32?18:30); exp++ )
	{
	    gdbg_info( 2, "exp %d\n", exp );
	    for ( i = 0; i < 20; i++ )   // Do 20 per exponent
	    {
		int zmax = diago.rgb == 32 ? 0xFFFFFF : 0xFFFF;
		do
		{
		    xyRandom(&x,&y);			// pick random x,y
		    spanlen = iRandom(19) + 1;		// and random span length
		} while (x + spanlen > diago.xmaxscreen);// Clip
		
		gdbg_info( 2, "  span %d, %d len=%d\n", x, y, spanlen );

		fw1 = fexpRandom(exp-1,exp);
		foow1 = 1.0 / fw1;
		oow1 = (unsigned long)(foow1 * (double)(0x01 << SST_W_FRACBITS));

		// Make sure that span crosses at least 1 floating point w exponent
		do {
		    fw2 = fw1;
		    if (iRandom(1))
			fw2 *= rfRandom(1,5);
		    else
			fw2 /= rfRandom(1,5);
		}
		while (fw2 < .60);
		foow2 = 1.0 / fw2;
		gdbg_info(4,"  w1  = %12.12f\n", fw1 );
		gdbg_info(4,"  w2  = %12.12f\n", fw2 );
				
		fdwdx = (foow2 - foow1) / (double) spanlen;
		
		dwdx = (long) (fdwdx * (double) ( 0x01 << SST_W_FRACBITS ));
		
		bias = iRandom( zmax );		// Random 16 bit bias
		if (bias & ((zmax+1)>>1))	// if MSB is set
		    bias |= ~zmax;		// sign extend it
		
		SET( sst->c1, 0x000000FF ); 
		SET( sst->w, oow1 );
		SET( sst->dwdx, dwdx );
		SET( sst->dwdy, 0 );
		SET( sst->zaColor, bias );
		
		gdbg_info(3,"  oow  = %.8x(%12.12f)\n", oow1, foow1 );
		gdbg_info(3,"  dwdx = %.8x(%12.12f)\n", dwdx, fdwdx );
		gdbg_info(3,"  bias = %.4x\n", bias );
		
		sst_drawspan( sst, x, y, spanlen );
		sst_idle( sst );
		
		for ( j = 0; j < spanlen; j++ )
		{
		    int wfl = wBufferValue( oow1 );
		    gdbg_info(20, "  Pixel %d of %d\n", j, spanlen );
	 	    gdbg_info(20,  "   dwdx = %08x\n", dwdx );
		    gdbg_info(20,  "   oow  = %08x\n", oow1 );
		    if (diago.rgb < 32)
			gdbg_info(20,  "   wfl  = %04x , exp=%d\n", wfl, wfl>>12 );
		    else
			gdbg_info(20,  "   wfl  = %06x , exp=%d\n", wfl, wfl>>19 );
		    gdbg_info(20,  "   bias = %06x\n", bias&zmax );
		    bufferVal = wfl + bias;
		    // emulate hardware exactly, exp=f, mant=000 produces
		    // a wBufferValue of 0x10000 then adds zbias then clamps
		    // whereas the CSIM produces 0xFFFF then adds zbias and clamps
		    if (0x4000==(oow1 & 0x3FFFFFFC)) {
			bufferVal++;
			GDBG_INFO(0,"****WARNING: special case hit, see Gary T\n");
		    }
		    if ( bufferVal > zmax ) bufferVal = zmax;
		    if ( bufferVal < 0 ) bufferVal = 0;
		    DIAG_TEST_PIXEL(CSIM_BUF_3D_AUX1, x+j, y, bufferVal);
		    oow1 += dwdx;
		}
	    }
	}   
    }
    DIAG_PASS( 0 );
}

/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:10:54 PM$
*/

#define INRANGE( subject, low, high ) ( ((subject) >= (low)) && ((subject) <= (high)) )

#include "udiag.h"
#include "sstdiag.h"

void main (int argc, char **argv)
{
    int spanlen;
    long i, j;		// Counters
    long exp;
    long x, y;
    long oow1;		// 1 / w <2.30>
    long dwdx;		// doow/dx <2.30>

    double fw1;		// floating point...
    double fw2;
    double foow1;
    double foow2;
    double fdwdx;
    
    SstRegs *sst;
    
    sst = SST_BEGIN(argc,argv);

    SET(sst->fbzMode, SST_ENDEPTHBUFFER | SST_WBUFFER | SST_DEPTH_FLOAT_SEL |
	SST_ZAWRMASK | SST_RGBWRMASK | SST_ZFUNC_GT | SST_ZFUNC_EQ | SST_ZFUNC_LT);
    
    SET( sst->fbzColorPath, SST_RGBSEL_C1 );
    
    while( DIAG_STARTPASS() )
    {
	// Test Exponents in the range of [2**17,2**-1) for 16bpp. up to 2**28 for 32bpp
	for ( exp=0; exp<(diago.rgb<32?18:28); exp++ )
	{
	    gdbg_info( 2, "exp %d\n", exp );
	    for ( i = 0; i < 10; i++ )   // Do 10 per exponent
	    {
		do {
		    xyRandom(&x,&y);			// pick random x,y
		    spanlen = iRandom(19) + 1;		// and random span length
		} while (x + spanlen > diago.xmaxscreen);// Clip
		
		gdbg_info( 2, "  span %d, %d len=%d\n", x, y, spanlen );
		fw1 = fexpRandom(exp-1,exp);
		foow1 = 1.0 / fw1;
		oow1 = (unsigned long)(foow1 * (double)(0x01 << 28));

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
		gdbg_info(4,"  z1  = %12.12f\n", fw1 );
		gdbg_info(4,"  z2  = %12.12f\n", fw2 );
		
		fdwdx = (foow2 - foow1) / (double) spanlen;
		dwdx = (long) (fdwdx * (double) ( 0x01 << 28 ));
		
		SET( sst->c1, 0x000000FF ); 
		SET( sst->z, oow1 );
		SET( sst->dzdx, dwdx );
		SET( sst->dzdy, 0 );

		gdbg_info(3,"  ooz  = %08x(%12.12f)\n", oow1, foow1 );
		gdbg_info(3,"  dzdx = %08x(%12.12f)\n", dwdx, fdwdx );
		
		sst_drawspan( sst, x, y, spanlen );
		sst_idle( sst );
		
		for ( j = 0; j < spanlen; j++ )
		{
		    int wfl = wBufferValue( oow1<<2 );
		    gdbg_info(20, "  Pixel %d of %d\n", j, spanlen );
		    gdbg_info(20,  "   dzdx = %08x\n", dwdx );
		    gdbg_info(20,  "   ooz  = %08x , %g\n", 
				oow1, oow1/(double)(0x01 << 28) );
		    if (diago.rgb < 32)
			gdbg_info(20,  "   zfl  = %04x , exp=%d\n", wfl, wfl>>12 );
		    else
			gdbg_info(20,  "   zfl  = %06x , exp=%d\n", wfl, wfl>>19 );
		    DIAG_TEST_PIXEL( CSIM_BUF_3D_AUX1, x+j, y, wfl);
		    oow1 += dwdx;
		}
	    }
	}   
    }
    DIAG_PASS( 0 );
}

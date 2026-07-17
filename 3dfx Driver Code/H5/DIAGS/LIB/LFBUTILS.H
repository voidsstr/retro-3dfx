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
** Last Edited: Thu Aug 24 01:20:09 1995 by jdt (John D. Tynefield) on vlsi3
**
** $Revision: 2$
** $Date: 10/11/00 8:11:56 PM$
**
*/

#ifndef _LFBUTILS_H_
#define _LFBUTILS_H_

enum { LFB_FRONT_BUFFER = 0, LFB_BACK_BUFFER = 1 };

enum TestOpt { TEST565, TESTX555, TEST1555, TESTX888, TEST8888,  
	       TEST16N565, TEST16NX555, TEST16N1555, TEST16N16,
	       TESTZ32, NUM_TEST_OPTS, TESTRANDOM  };

FxU32 packBits( FxU32 n, ... /*val1, width1, ..., val<n>, width<n>*/ );
FxU32 rndfogMode( void );
FxU32 rndalphaMode( void );
FxU32 rndlfbMode( enum TestOpt mode, FxU32 *pixelsToWrite );
FxU32 rndfbzMode( void );
FxU32 rndfbzColorPath( void );

FxBool flipYOrigin( SstRegs *shadowRegs );

int testPixelInFB( int x, int y, SstRegs *shadowRegs,  fbiColors *pixel );

void lfbWrite( FxU32 x, FxU32 y, FxU32 value,
	       FxU32 pixelsToWrite, SstRegs *shadowRegs, SstRegs *sst );

void mangleColor( FxU32 *writeValue, SstRegs *shadowRegs, FxU32 pixelsToWrite,
		  FxU32 a1, FxU32 r1, FxU32 g1, FxU32 b1, FxU32 depth1,
		  FxU32 a2, FxU32 r2, FxU32 g2, FxU32 b2, FxU32 depth2 );


FxBool EmulateChromaKey( fbiColors *pixel,
			 FxU32 x, FxU32 y, FxU32 depth,
			 SstRegs *shadowRegs,
			 SstRegs *realRegs );
FxBool EmulateCCU( fbiColors *pixel,
		   FxU32 x, FxU32 y, FxU32 depth,
		   SstRegs *shadowRegs,
		   SstRegs *realRegs );
FxBool EmulateZCompare( fbiColors *pixel,
			 FxU32 x, FxU32 y, FxU32 depth,
			 SstRegs *shadowRegs,
			 SstRegs *realRegs );
FxBool EmulateFog( fbiColors *pixel,
		   FxU32 x, FxU32 y, FxU32 depth,
		   SstRegs *shadowRegs,
		   SstRegs *realRegs );
FxBool EmulateAlphaBlend( fbiColors *pixel,
			  FxU32 x, FxU32 y, FxU32 depth,
			  SstRegs *shadowRegs,
			  SstRegs *realRegs );
FxBool EmulatePixPipe( fbiColors *pixel,
		     FxU32 x, FxU32 y, FxU32 *depth,
		     SstRegs *shadowRegs,
		     SstRegs *realRegs );

#endif

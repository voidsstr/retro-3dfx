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
** $Date: 10/11/00 8:19:24 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

void
main (int argc, char **argv)
{
    int n;
    int bpt;  //bytes per texel
    Triangle *t;
    FxU32 srcColor = 0x5555;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    if(diago.bigAssTextures)
      t = buildTriangle(2048, 2048);
    else
      t = buildTriangle(256, 256);

    t->next = NULL;

    if(diago.bigAssTextures)
      {
	GDBG_ERROR("tfstress::main", "Can't use --bigAssTextures\n");
	DIAG_FAIL();
      }

    // Print Out Option Description
    if ( diago.printOpts )
    {
	gdbg_printf( "replay option description:\n" );
	gdbg_printf( " -O -> initialize color buffer to specified color\n");

        DIAG_FAIL();
    }
    if (diago.option) {
	srcColor = diago.option;
    }
    gdbg_info(2,"src color=%x\n", srcColor);
    SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT |
		SST_ENTEXTUREMAP | SST_PARMADJUST);

    while (DIAG_STARTPASS()) {			// for each pass
	// use the 16-bit SST_AI88 RGB texture format, size = 1x1, in replace mode
	t->tex->tMode = SST_AI88 | SST_TC_REPLACE | SST_TCA_REPLACE;
	texRandomTextureMap(sst,diago.trex,0, 0,0,t->tex);
	// now re-download it

	if(SST_T8BIT(t->tex->tMode))
	  bpt=1;
	else if(SST_T16BIT(t->tex->tMode))
	  bpt=2;
	else if(SST_T32BIT(t->tex->tMode))
	  bpt=4;
	else 
	  GDBG_ERROR("main", "gdamn, something is f'ed  %s(%d)\n", __FILE__, __LINE__);

	sstDownLoadTexture(sst,t->tex->trex,   		// NOTE: this sets texBaseAddr
			 t->tex->mip[8]->mipmapBaseAddress,
			 t->tex->tiled,t->tex->tStride,
			 t->tex->tMode,0,0,0,
			 bpt, t->tex->tLOD,  
			 (unsigned long *)&srcColor);


	// now render full screen triangles (2) with the texture
	for (n=0; n<1000; n++) {
	    // generate a simple right triangle, vertex A is on a pixel center
	    t->vA.x = 0;
	    t->vA.y = 0;
	    t->vB.x = diago.xmaxscreen*XY_ONE;
	    t->vB.y = 0;
	    t->vC.x = t->vB.x;
	    t->vC.y = diago.ymaxscreen*XY_ONE;
	    t->vA.s = t->vB.s = t->vC.s = 0;
	    t->vA.t = t->vB.s = t->vC.s = 0;
	    t->vA.w = t->vB.w = t->vC.w = FX_BIT64(32);
	    areaTriangle(t);			// compute the area (before setup)
	    if (!setupStwTriangle(t))		// setup STW slopes
		GDBG_ERROR("main","oops\n");
	    sortTriangle(t);			// sort it
	    printStwTriangle(4,t);
	    printStwTriangleSlopes(5,t);
	    drawStwTriangle(sst,t);
	}
    }

    DIAG_PASS(0);
}

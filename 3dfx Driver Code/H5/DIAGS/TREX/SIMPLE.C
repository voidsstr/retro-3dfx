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
** $Date: 10/11/00 8:19:16 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

void
main (int argc, char **argv)
{
    int n, fbzCP, randomTexturing;
    unsigned int fbzMode;
    static Triangle *t;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);
    
    if(diago.bigAssTextures)
      t = buildTriangle(2048, 2048);
    else
      t = buildTriangle(256, 256);

    t->next = NULL;
    randomTexturing = diago.zeroLodFrac;	// -Z option
    SET(sst->c1, 0x12345678);
 
  while (DIAG_STARTPASS()) {			// for each pass
    if(diago.compressedTextures)
      {
	t->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) | 
	  SST_TC_REPLACE | SST_TCA_REPLACE;
	
	switch(iRandom(3))
	  {
	  case 0:
	    t->tex->tMode |= SST_COMPRESSED_TEXTURES | SST_3DFX_COMPRESSED;
	    break;
	  case 1:
	    t->tex->tMode |= SST_COMPRESSED_TEXTURES | SST_DXT1;
	    break;
	  case 2:
	    t->tex->tMode |= SST_COMPRESSED_TEXTURES | SST_DXT2;
	    break;
	  case 3:
	    t->tex->tMode |= SST_COMPRESSED_TEXTURES | SST_DXT4;
	    break;
	  }	  
      }
    else if(diago.tex32)
      { // use the 16-bit 565 RGB texture format, size = 8x8, in replace mode
	t->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) | 
	  SST_ARGB8888 | SST_TC_REPLACE | SST_TCA_REPLACE;
      }
    else
      {
	// use the 16-bit 565 RGB texture format, size = 8x8, in replace mode
	t->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) | 
	  SST_RGB565 | SST_TC_REPLACE | SST_TCA_REPLACE;
      }

    if(diago.bigAssTextures)
      { //For big textures, allow for random texture map sizes
	FxI32 logWidth, logHeight;
	FxI32 maxLog;

	if(diago.ytiled)
	  maxLog = 10;  //Only 1024x1024 and smaller tiled textures are downloadable 
	else
	  maxLog = 11;

	logWidth = iRandom(maxLog);
	logHeight = logWidth + rRandom(-3, 3);
	
	if(logHeight < 0)
	  logHeight = 0;
	else if(logHeight > maxLog)
	  logHeight = maxLog;

	texRandomTextureMap(sst,diago.trex,0, logWidth,logHeight,t->tex);
      }
    else
      texRandomTextureMap(sst,diago.trex,0, 3,3,t->tex);
    fbzCP = SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT | 
		SST_ENTEXTUREMAP | (diago.adjust?SST_PARMADJUST:0);
    SET(sst->fbzColorPath, fbzCP);

    for (n=0; n<50; n++) {			// do 50 tests
	if (diago.rgb == 16)
	  SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom() |
	      ((texRandomZAbuffer(sst,t->tex->tMode) || diago.tex32 || diago.compressedTextures) ? 
	       SST_ENALPHABUFFER | SST_ZAWRMASK : 0));
	else
	    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());
    again:
	randomTriangle(t,diago.tsize,1);	// pick random triangle
	randomSt1418Triangle(t);		// with random texcoords
//	printStwTriangle(4,t);
	if (diago.perspective)			// if testing perspective
	    randomW230Triangle(t);		// generate random W

	areaTriangle(t);			// compute the area (before setup)
	if (!setupStwTriangle(t))		// setup STW slopes
	    goto again;				// reject bad slopes
	sortTriangle(t);			// sort it
	printStwTriangle(4,t);
	printStwTriangleSlopes(5,t);

	if (randomTexturing)
	if (iRandom(1)) {
	    fbzCP ^= SST_ENTEXTUREMAP;		// toggle texturing
	    if ((fbzCP & SST_ENTEXTUREMAP)==0) {
		fbzCP &= ~SST_RGBSELECT;
		fbzCP |= SST_RGBSEL_C1;

		fbzCP &= ~SST_ASELECT;
		fbzCP |= SST_ASEL_C1;
	    }
	    else {
		fbzCP &= ~SST_RGBSELECT;
		fbzCP |= SST_RGBSEL_TREXOUT;

		fbzCP &= ~SST_ASELECT;
		fbzCP |= SST_ASEL_TREXOUT;
	    }
	    SET(sst->fbzColorPath, fbzCP);
	}
	// NOTE: sub-pixel parameter adjustment is OFF so we may end up with
	//	 color overflows/underflows etc
	drawStwTriangle(sst,t);
	
	sst_idle(sst);				// wait for the command to complete
	fbzMode = GET(sst->fbzMode);
	if (fbzCP & SST_ENTEXTUREMAP)
	    checkTriangle(t,1,1,diago.adjust, insideStTriangle,0,0,0);
	eraseTriangle(sst,t,1,0,0);		// erase the triangle
    }
  }
  DIAG_PASS(1);					// check for black screen
}

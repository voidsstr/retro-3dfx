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
** $Revision: 7$
** $Date: 10/11/00 8:19:28 PM$
*/

#include <assert.h>

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

#ifdef CVG
#include "cvgasm.h"
#define TEXBASE_SHIFT 3
#endif
#ifdef SST2
#include "sst2asm.h"
#define TEXBASE_SHIFT 0
#endif
#if !defined(CVG) && !defined(SST2)
#include "h3asm.h"
#define TEXBASE_SHIFT 0
#endif

void randomTmuCombineMode(SstRegs *sst,int upstream_tmu_exists)
{
    FxU32 combineMode = 0;
    FxU32 temp;

    // use -S option to disable random combinemode
    if (diago.tsplit == 1) return;

	if (iRandom(1)) combineMode |= SST_CM_DISABLE_CHROMA_SUBSTITUTION;

	combineMode &= ~SST_CM_USE_COMBINE_MODE;
	combineMode |= shadowRegisters3D[0][0].combineMode & SST_CM_USE_COMBINE_MODE;

	// be careful not to use other texture if only 1 TMU
	do temp = iRandom(7) << SST_CM_TC_OTHERSELECT_SHIFT;
	while (upstream_tmu_exists==0 && (
		(temp==SST_CM_TC_OTHERSELECT_OTHER_TRGB) ||
		(temp==SST_CM_TC_OTHERSELECT_OTHER_TA)));
	combineMode |= temp;
	do temp = iRandom(7) << SST_CM_TC_LOCALSELECT_SHIFT;
	while (upstream_tmu_exists==0 && (
		(temp==SST_CM_TC_LOCALSELECT_OTHER_TRGB) ||
		(temp==SST_CM_TC_LOCALSELECT_OTHER_TA)));
	combineMode |= temp;
	do temp = iRandom(3) << SST_CM_TCA_LOCALSELECT_SHIFT;
	while (upstream_tmu_exists==0 && temp==SST_CM_TCA_LOCALSELECT_OTHER_TA);
	combineMode |= temp;
	do temp = iRandom(3) << SST_CM_TCA_OTHERSELECT_SHIFT;
	while (upstream_tmu_exists==0 && temp==SST_CM_TCA_OTHERSELECT_OTHER_TA);
	combineMode |= temp;

	do temp = iRandom(7) << SST_CM_TC_MSELECT_7_SHIFT;
	while (upstream_tmu_exists==0 && temp==SST_CM_TC_MSELECT_7_OTHER_TRGB);
	combineMode |= temp;
	combineMode |= iRandom(3) << SST_CM_TC_INVERT_OTHER_SHIFT;
	combineMode |= iRandom(3) << SST_CM_TC_INVERT_LOCAL_SHIFT;
	if (iRandom(1)) combineMode |= SST_CM_TC_INVERT_ADD_LOCAL;
	combineMode |= iRandom(2) << SST_CM_TC_OUTSHIFT_SHIFT;
	combineMode |= iRandom(3) << SST_CM_TCA_INVERT_OTHER_SHIFT;
	combineMode |= iRandom(3) << SST_CM_TCA_INVERT_LOCAL_SHIFT;
	if (iRandom(1)) combineMode |= SST_CM_TCA_INVERT_ADD_LOCAL;
	combineMode |= iRandom(2) << SST_CM_TCA_OUTSHIFT_SHIFT;

	// turn off bits that CSIM complains about
	if (!(combineMode & SST_CM_USE_COMBINE_MODE))
	    combineMode &= ~(SST_CM_TC_INVERT_ADD_LOCAL | SST_CM_TCA_INVERT_ADD_LOCAL);

	//Don't change the number of pixelsPerClock
	combineMode &= ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK;
	combineMode |= shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK;
	assert((shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) ==
	       (shadowRegisters3D[0][1].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK));
	assert((shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) ==
	       (shadowRegisters3D[0][2].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK));

	//If is 2ppc, don't set combine mode so that we're chaining the TMUs
	if((combineMode & SST_CM_USE_COMBINE_MODE) &&
	   (shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK))
	  {
	    combineMode &= ~(SST_CM_TC_OTHERSELECT  | SST_CM_TC_LOCALSELECT | SST_CM_TC_MSELECT_7 |
			     SST_CM_TCA_OTHERSELECT | SST_CM_TCA_LOCALSELECT);
	    
	    combineMode |= (iRandom(5) + 2) << SST_CM_TC_OTHERSELECT_SHIFT;
	    combineMode |= ((iRandom(1) ? iRandom(1) : (iRandom(3) + 4))) << SST_CM_TC_LOCALSELECT_SHIFT;
	    combineMode |= ((iRandom(1) ? 0 : (iRandom(3) + 4))) << SST_CM_TC_MSELECT_7_SHIFT;
	    combineMode |= (iRandom(2) + 1) << SST_CM_TCA_OTHERSELECT_SHIFT;
	    combineMode |= ((iRandom(1) ? 0 : (iRandom(1) + 2))) << SST_CM_TCA_LOCALSELECT_SHIFT;

	    /*
	    combineMode |= SST_CM_TC_OTHERSELECT_LOCAL_TRGB | SST_CM_TC_LOCALSELECT_SHIFT |
	      SST_CM_TC_MSELECT_7_LOCAL_TRGB | SST_CM_TCA_OTHERSELECT_LOCAL_TA |
	      SST_CM_TCA_LOCALSELECT_LOCAL_TA;
	    */
	      
	  }

	SET(sst->combineMode, combineMode);
}

// broadcasting combineMode to multiple chips really doesn't make
// much sense, so here we generate a different value for each chip
// we do broadcast it upstream when sending to FBI
void randomCombineMode(SstRegs *base_sst, int chip)
{
    FxU32 combineMode = 0;
    FxU32 temp;
    static FxU32 callCounter=0;

    //It's too damn hard to keep track of this when running with a command fifo
    if(diago.writeFifo > 0)
      return;

    if(shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK && diago.writeFifo == 0)       
      assert(csimActiveTMUs(diago.sstCSIM) < 2);

    // use -S option to disable random combinemode
    if (diago.tsplit == 1) return;

    if ((chip == 0) || (chip & 1)) {	// if written to FBI
      if (iRandom(1)) combineMode |= SST_CM_USE_COMBINE_MODE;

	// set new random RGB modes
	do {
	    temp = iRandom(7) << SST_CM_CC_OTHERSELECT_SHIFT;
	} while (temp == SST_CM_CC_OTHERSELECT_LFB_RGB);
	combineMode |= temp;

	temp = iRandom(7) << SST_CM_CC_LOCALSELECT_SHIFT;
	combineMode |= temp;
	combineMode |= iRandom(3) << SST_CM_CC_MSELECT_7_SHIFT;
	combineMode |= iRandom(3) << SST_CM_CC_INVERT_OTHER_SHIFT;
	combineMode |= iRandom(3) << SST_CM_CC_INVERT_LOCAL_SHIFT;
	if (iRandom(1)) combineMode |= SST_CM_CC_INVERT_ADD_LOCAL;
	combineMode |= iRandom(2) << SST_CM_CC_OUTSHIFT_SHIFT;

	do {
	    temp = iRandom(3) << SST_CM_CCA_OTHERSELECT_SHIFT;
	} while (temp == SST_CM_CCA_OTHERSELECT_LFB_A);
	combineMode |= temp;
	// set new random combineMode fields
	combineMode |= iRandom(3) << SST_CM_CCA_LOCALSELECT_SHIFT;
	combineMode |= iRandom(3) << SST_CM_CCA_INVERT_OTHER_SHIFT;
	combineMode |= iRandom(3) << SST_CM_CCA_INVERT_LOCAL_SHIFT;
	if (iRandom(1)) combineMode |= SST_CM_CCA_INVERT_ADD_LOCAL;
	combineMode |= iRandom(2) << SST_CM_CCA_OUTSHIFT_SHIFT;

	// turn off bits that CSIM complains about
	if (!(combineMode & SST_CM_USE_COMBINE_MODE))
	    combineMode &= ~(SST_CM_CC_INVERT_ADD_LOCAL | SST_CM_CCA_INVERT_ADD_LOCAL);

	//Don't change the number of pixelsPerClock
	combineMode &= ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK;
	combineMode |= shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK;
	assert((shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) ==
	       (shadowRegisters3D[0][1].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK));
	assert((shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) ==
	       (shadowRegisters3D[0][2].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK));

	//If is 2ppc, don't set combine mode so that we're chaining the TMUs
	if((combineMode & SST_CM_USE_COMBINE_MODE) &&
	   (shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK))
	  {
	    combineMode &= ~(SST_CM_TC_OTHERSELECT  | SST_CM_TC_LOCALSELECT | SST_CM_TC_MSELECT_7 |
			     SST_CM_TCA_OTHERSELECT | SST_CM_TCA_LOCALSELECT);

	    combineMode |= (iRandom(5) + 2) << SST_CM_TC_OTHERSELECT_SHIFT;
	    combineMode |= ((iRandom(1) ? iRandom(1) : (iRandom(3) + 4))) << SST_CM_TC_LOCALSELECT_SHIFT;
	    combineMode |= ((iRandom(1) ? 0 : (iRandom(3) + 4))) << SST_CM_TC_MSELECT_7_SHIFT;
	    combineMode |= (iRandom(2) + 1) << SST_CM_TCA_OTHERSELECT_SHIFT;
	    combineMode |= ((iRandom(1) ? 0 : (iRandom(1) + 2))) << SST_CM_TCA_LOCALSELECT_SHIFT;	      

	    if(((combineMode & SST_CM_CCA_OTHERSELECT) == SST_CM_CCA_OTHERSELECT_LFB_A) ^
	       ((combineMode & SST_CM_CC_OTHERSELECT) == SST_CM_CC_OTHERSELECT_LFB_RGB))
	      {
		combineMode &= ~(SST_CM_TC_OTHERSELECT  | SST_CM_TC_LOCALSELECT | SST_CM_TC_MSELECT_7 |
				 SST_CM_TCA_OTHERSELECT | SST_CM_TCA_LOCALSELECT);
		combineMode |= SST_CM_CC_OTHERSELECT_IRGB | SST_CM_CCA_OTHERSELECT_IA;
	      }
	  }

	//Make sure the FBI isn't using TMU data when texturing is disabled
	if(!(shadowRegisters3D[0][0].fbzColorPath & SST_ENTEXTUREMAP))
	  {
	    if(((combineMode & SST_CM_CC_OTHERSELECT) == SST_CM_CC_OTHERSELECT_TRGB) ||
	       ((combineMode & SST_CM_CC_OTHERSELECT) == SST_CM_CC_OTHERSELECT_TA))
	      {
		combineMode &= ~SST_CM_CC_OTHERSELECT;
		combineMode |= SST_CM_CC_OTHERSELECT_IRGB;
	      }	      
	    
	    if(((combineMode & SST_CM_CC_LOCALSELECT) == SST_CM_CC_LOCALSELECT_TRGB) ||
	       ((combineMode & SST_CM_CC_LOCALSELECT) == SST_CM_CC_LOCALSELECT_TA))
	      {
		combineMode &= ~SST_CM_CC_LOCALSELECT;
		combineMode |= SST_CM_CC_LOCALSELECT_IRGB;
	      }	      
	      
	    if((combineMode & SST_CM_CCA_OTHERSELECT) == SST_CM_CCA_OTHERSELECT_TA)
	      {
		combineMode &= ~SST_CM_CCA_OTHERSELECT;
		combineMode |= SST_CM_CCA_OTHERSELECT_IA;
	      }

	    if((combineMode & SST_CM_CCA_LOCALSELECT) == SST_CM_CCA_LOCALSELECT_TA)
	      {
		combineMode &= ~SST_CM_CCA_LOCALSELECT;
		combineMode |= SST_CM_CCA_LOCALSELECT_IA;
	      }
	  }

	SET(SST_CHIP(base_sst,chip)->combineMode, combineMode);	
    }

    // convert chip number into a mask
    if (chip == 0) chip = 0x7;		// 2 TMUs and FBI
    chip &= 0x6;			// mask off FBI
    if (diago.trex == 0) chip &= 0x2;	// optionally mask off 2nd TMU
    if (chip & 2) randomTmuCombineMode(SST_TMU(base_sst,0),diago.trex);
    if (chip & 4) randomTmuCombineMode(SST_TMU(base_sst,1),0);

    //Make sure we're consistent on whether or not combineMode is being used
    if((shadowRegisters3D[0][0].combineMode & SST_CM_USE_COMBINE_MODE) != 
       (shadowRegisters3D[0][1].combineMode & SST_CM_USE_COMBINE_MODE))
      SET_0(base_sst->combineMode, (shadowRegisters3D[0][1].combineMode & ~SST_CM_USE_COMBINE_MODE) |
	    (shadowRegisters3D[0][0].combineMode & SST_CM_USE_COMBINE_MODE));
    if((shadowRegisters3D[0][0].combineMode & SST_CM_USE_COMBINE_MODE) != 
       (shadowRegisters3D[0][2].combineMode & SST_CM_USE_COMBINE_MODE))
      SET_1(base_sst->combineMode, (shadowRegisters3D[0][2].combineMode & ~SST_CM_USE_COMBINE_MODE) |
	    (shadowRegisters3D[0][0].combineMode & SST_CM_USE_COMBINE_MODE));


    //If we're in an illegal state, use some safe register values
    if(shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK)
      {
	if(csimActiveTMUs(diago.sstCSIM) > 1)
	{
	  SET_FBI(base_sst->combineMode, shadowRegisters3D[0][0].combineMode |
		  SST_CM_USE_COMBINE_MODE);		  
	  SET_0(base_sst->combineMode, SST_CM_TC_OTHERSELECT_LOCAL_TRGB |
		SST_CM_TC_LOCALSELECT_LOCAL_TRGB | SST_CM_TC_MSELECT_7_LOCAL_TRGB |
		SST_CM_TCA_OTHERSELECT_LOCAL_TA | SST_CM_TCA_LOCALSELECT_LOCAL_TA |
		SST_CM_USE_COMBINE_MODE | SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
	  SET_1(base_sst->combineMode, SST_CM_TC_OTHERSELECT_LOCAL_TRGB |
		SST_CM_TC_LOCALSELECT_LOCAL_TRGB | SST_CM_TC_MSELECT_7_LOCAL_TRGB |
		SST_CM_TCA_OTHERSELECT_LOCAL_TA | SST_CM_TCA_LOCALSELECT_LOCAL_TA |
		SST_CM_USE_COMBINE_MODE | SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
	}
      }
    
    if(shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK &&
       diago.writeFifo == 0)
      assert(csimActiveTMUs(diago.sstCSIM) < 2);

    if(((combineMode & SST_CM_CCA_OTHERSELECT) == SST_CM_CCA_OTHERSELECT_LFB_A) ^
       ((combineMode & SST_CM_CC_OTHERSELECT) == SST_CM_CC_OTHERSELECT_LFB_RGB))
      {
	GDBG_INFO(0, "callCounter = %d\n", callCounter);
	assert(0);
      }

    callCounter++;
}

void randomStencils(SstRegs *sst)
{
    int s = iRandom(3);

    // use -S option to disable random stenciling
    if (diago.tsplit == 1) return;

    gdbg_info(4,"switching to random stencil case %d\n",s);
    switch(s) {
	case 0:		// increment all pixels to count DC
	    SET(sst->stencilMode, SST_STENCIL_ENABLE | SST_STENCIL_FUNC | SST_STENCIL_WMASK);
	    SET(sst->stencilOp, (SST_SOP_KEEP<<SST_STENCIL_SFAIL_OP_SHIFT) |
			(SST_SOP_INC<<SST_STENCIL_ZFAIL_OP_SHIFT) |
			(SST_SOP_INC<<SST_STENCIL_ZPASS_OP_SHIFT));
	    break;
	case 1:		// disable totally
	    SET(sst->stencilMode, 0);
	    break;
	case 2:		// increment all pixels that pass the zbuffer
	    SET(sst->stencilMode, SST_STENCIL_ENABLE | SST_STENCIL_FUNC | SST_STENCIL_WMASK);
	    SET(sst->stencilOp, (SST_SOP_KEEP<<SST_STENCIL_SFAIL_OP_SHIFT) |
			(SST_SOP_KEEP<<SST_STENCIL_ZFAIL_OP_SHIFT) |
			(SST_SOP_INC<<SST_STENCIL_ZPASS_OP_SHIFT));
	    break;
	case 3:		// totally random
	    SET(sst->stencilMode, iRandom(0xFFFFFFFF));
	    SET(sst->stencilOp, iRandom(0xFFF) & 0x777);
    }
}

// these keep track of whether or not we have sent these values to the chip
// for the current triangle
int sentVAx, sentVAy;
int sentVBx, sentVBy;
int sentVCx, sentVCy;
int toBeSent;

// array of texture info for each TREX chip, the big problem with this diag
// is that some of the bits of tMode and tLOD have to be in sync with the 
// textureBaseAddr.
struct {
    int tMode;
    int tLOD;
    int addr[9];	// physical base addresses of each LOD
#ifndef CVG
    int tiled;
    int tStride;
#endif
} textures[4][16];

// array of current textureMode and tLOD registers for each TREX chip
int texs[4];
unsigned long tModes[4];
unsigned long tLODs[4];

#define FIX2FLOAT(x) (.0625F * (float)(x))

// given a texture size and a trex chip,
// return a random index into the textures[] array
int randomTex(int trex, int desired_bpt)
{
    int n;
    int bpt;
    int counter=0;

    //If using 4 bytes per texel textures, just return the only 32bpt texture format
    if(desired_bpt == 4)
      return((SST_ARGB8888)>>SST_TFORMAT);

    
    do {
	n = iRandom(13);

	if(SST_T4BIT_COMPRESSED(textures[trex][n].tMode))
	  bpt=0;
	else if(SST_T8BIT_COMPRESSED(textures[trex][n].tMode))
	  bpt=1;
	else if(SST_T8BIT(textures[trex][n].tMode))
	  bpt=1;
	else if(SST_T16BIT(textures[trex][n].tMode))
	  bpt=2;

	counter++;
    } while (bpt != desired_bpt && counter<100);
    return n;
}

#define LMASK (SST_TC_ADD_CLOCAL | SST_TC_ADD_ALOCAL)

int tcuRandom(void)
{
    int mode;
    do {
	mode = iRandom(SST_TCOMBINE) & SST_TCOMBINE;
    } while (((mode & LMASK)==LMASK) || ((mode&SST_TC_MSELECT)>SST_TC_MLODFRAC));
    return mode;
}

// download a new set of textures, 14 in all, 7 each 8-bit and 16-bit
// each texture is a different aspect ratio, none are TSPLIT
// the first 7 are loaded from low mem to high mem, last 7 from high mem to low
// TODO: add in TSPLIT textures, perhaps randomly? 
void initTextures(SstRegs *sst, int trex)
{
    int j,n,ar,slog,tlog,array[14], offset;
    FxU32 hsim_save = diago.halInfo->hsim;	// save state of backdoor texture writes
    static Texture *tex=NULL;

    if(tex == NULL)
      {
	if(diago.bigAssTextures)
	  tex = buildTexture(2048, 2048);
	else
	  tex = buildTexture(256, 256);	
      }

    gdbg_info(2, "initTextures(trex=%d)\n",trex);
    // enable backdoor texture writes, it just takes too long otherwise
    sst_idle_really(sst);
    if ( diago.halInfo->hsim )
      diago.halInfo->hsim |= HSIM_TREX_BACKDOOR_TEXWRITES;
    scrambleRandom(14,array);

    if(diago.bigAssTextures)
      { //for 2048x2048 textures, we test LOD's 3-11, not 0-8
	tex->lodmin = 3;			// diag will change this randomly
	tex->lodmax = 11;
	offset = 3;
      }
    else
      { //256x256 case
	tex->lodmin = 0;			// diag will change this randomly
	tex->lodmax = 8;
	offset = 0;
      }

    tex->tLOD = 0;

    for (n=0; n<14; n++) {
	slog = tlog = 8;		// now pick random texture size
	j = array[n];			// begin with 256x256

	if (j >= 7) {			// pick between 8, 16 bit
	  if(diago.compressedTextures)
	    tex->tMode = SST_3DFX_COMPRESSED | SST_COMPRESSED_TEXTURES;
	  else
	    tex->tMode = SST_RGB565;
	    j -= 7;
	}
	else 
	  {
	    if(diago.compressedTextures)
	      tex->tMode = SST_DXT2 | SST_COMPRESSED_TEXTURES;
	    else 
	      tex->tMode = SST_RGB332;
	  }
	
	if (j > 3) {			// pick an aspect ratio
	    ar = j-3;
	    slog -= ar;
	}
	else {
	    ar = j;
	    tlog -= ar;
	}

	texRandomTextureMapEx(sst,trex, 1,slog,tlog,tex,0);

	tModes[trex] = tex->tMode;		// shadow regs
	tLODs[trex] = tex->tLOD;			// shadow regs
	textures[trex][n].tMode = tex->tMode;
	textures[trex][n].tLOD = tex->tLOD;
	texs[trex] = n;

	//For linear, put the physical address of each mipmap into .addr
	//For tiled, put texture base address in each of the mipmamps
	for (j=offset; j< (FxI32)tex->nMipmaps; j++)
	  {
	    if(tex->tiled)
	      textures[trex][n].addr[j-offset] = tex->mip[j]->textureBaseAddress;
	    else
	      textures[trex][n].addr[j-offset] = tex->mip[j]->mipmapBaseAddress;
	  }

	textures[trex][n].tStride = tex->tStride;
	textures[trex][n].tiled = tex->tiled;
    }

    for (n=0; n<14; n++) {
#ifdef CVG
        gdbg_info(3,"texture %d on TREX %d\n",n,trex);
#else
	gdbg_info(3,"texture %d (%s) on TREX %d\n",n,
		  textures[trex][n].tiled?"Tiled":"Linear",trex);
#endif
	for (j=0; j<=8; j++)			// copy all the base addresses
	    gdbg_info(3,"  lod %d at 0x%x\n",j,textures[trex][n].addr[j]);
    }
    // restore state of backdoor texture writes
    if ( diago.halInfo->hsim )
      diago.halInfo->hsim = hsim_save;
}

//----------------------------------------------------------------------
// initialize a new random triangle with non-zero area, can go offscreen
// sometimes we generate the same triangle, 
// sometimes a totally random one, sometimes one close by
//----------------------------------------------------------------------
void newTriangle(Triangle *t, int onscreen, int i)
{
    randomStressTriangle(t, diago.tsize, onscreen, i);
    randomFloatStwTriangle_Setup(t);		// random STW floating point
    printStwTriangle(3,t);
    printStwTriangleSlopes(4,t);

    sentVAx = 0;
    sentVAy = 0;
    sentVBx = 0;
    sentVBy = 0;
    sentVCx = 0;
    sentVCy = 0;
    toBeSent = 6;
}

void flushTriangle(SstRegs *sst, Triangle *t)
{
    if (!sentVAx) if (iRandom(1)) SET(sst->vA.x,t->vA.x); else SETF(sst->FvA.x,t->vA.fx);
    if (!sentVAy) if (iRandom(1)) SET(sst->vA.y,t->vA.y); else SETF(sst->FvA.y,t->vA.fy);
    if (!sentVBx) if (iRandom(1)) SET(sst->vB.x,t->vB.x); else SETF(sst->FvB.x,t->vB.fx);
    if (!sentVBy) if (iRandom(1)) SET(sst->vB.y,t->vB.y); else SETF(sst->FvB.y,t->vB.fy);
    if (!sentVCx) if (iRandom(1)) SET(sst->vC.x,t->vC.x); else SETF(sst->FvC.x,t->vC.fx);
    if (!sentVCy) if (iRandom(1)) SET(sst->vC.y,t->vC.y); else SETF(sst->FvC.y,t->vC.fy);
    gdbg_info(2,"flushed triangle\n");
    sentVAx = sentVAy = sentVBx = sentVBy = sentVCx = sentVCy = 1;
    toBeSent = 0;
}

//----------------------------------------------------------------------
#define SET1418(a,b) SET(a,(unsigned long)(b>>(SST_ST64_FRACBITS-SST_ST_FRACBITS)))
#define SETW(a,w) SET(a,(unsigned long)(w>>(SST_W64_FRACBITS-SST_W_FRACBITS)))

#define CCU_USES_TEXMAP(ccu) (((ccu & SST_RGBSELECT) == SST_RGBSEL_TREXOUT) || 	\
			      ((ccu & SST_ASELECT) == SST_ASEL_TREXOUT) || 	\
			      ((ccu & SST_CC_MSELECT) == SST_CC_MATREX) || 	\
			      ((ccu & SST_CC_MSELECT) == SST_CC_MRGBTMU) || 	\
			      ((ccu & SST_CCA_MSELECT) == SST_CCA_MATREX) || 	\
			      ( ccu & SST_LOCALSELECT_OVERRIDE_WITH_ATEX) )
void
main (int argc, char **argv)
{
    int k,n, chip;
    int lfbBytesPerPixel=2;
    FxU32 selection;
    FxU32 amode, fbzMode, fbzColorPath, renMode, fogTable[FOG_TABLE_SIZE];
    Triangle *t;
    SstRegs *sst, *base_sst;
    FxU32 zaColor, c1;

    base_sst = sst = SST_BEGIN(argc,argv);
    if (!diago.diff) {
	gdbg_error("tstress2","must run with -D option, forcing -D\n");
	diago.diff = 1;
    }
    if (diago.trex < 0)				// ignore random TREX hint
	diago.trex = -diago.trex;
    diago.multiTexBaseAddr = 0;			// and multiTexBaseAddr
    diago.perspective = 1;			// need to run in perspective
    diago.adjust = 1;				// and with adjust on

    //Don't allow sub-pixel correction with AA
    if(diago.aaEnabled)
      diago.adjust = 0;

    //Make sure that the subsample jitter is 0,0. Otherwise, there can be goofy
    //mismatches between the csim and rtl/hw. This occurs because this diag only
    //sends down partial triangles
    if(diago.aaEnabled)
      SET(base_sst->aaCtrl, SST_AA_CONTROL_AA_ENABLE);


    if(diago.bigAssTextures)
      t = buildTriangle(2048, 2048);
    else
      t = buildTriangle(256, 256);

    t->tex->tMode = 0;

    // force backdoor texture writes during initTextures and
    // frontdoor texture writes elsewhere
    if ( diago.halInfo->hsim ) {
      if ( diago.halInfo->hsim & HSIM_TREX_BACKDOOR_TEXWRITES ) 
	gdbg_printf("WARNING: forcing frontdoor texture writes after initTextures\n");
      else
	gdbg_printf("WARNING: forcing backdoor texture writes during initTextures\n");
      diago.halInfo->hsim &= ~HSIM_TREX_BACKDOOR_TEXWRITES;
    }

    // setup some reasonable starting modes
    fbzMode = SST_ENRECTCLIP | SST_RGBWRMASK;
    if (diago.zeroLodFrac)			// enable Zbuffer
	fbzMode |= SST_ENDEPTHBUFFER | SST_ZAWRMASK | SST_ZFUNC_GT;
    SET(sst->fbzMode, fbzMode);
    fbzColorPath = SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT | SST_ENTEXTUREMAP |
		(diago.adjust?SST_PARMADJUST:0);
    SET(sst->fbzColorPath, fbzColorPath);
    renMode = GET(sst->renderMode);

    // send random (but valid) register writes to the chip
    while (DIAG_STARTPASS())  {			// for each pass
      newTriangle(t,1,1);
      sst_random_fog_table(sst,fogTable);

      t->tex->tMode &= ~SST_TNCCSELECT;		// NCC table 0
	t->tex->tMode &= ~SST_TFORMAT;		// palette texture (init the table)
	t->tex->tMode |= SST_P8;
	texRandomNccTable(sst, t->tex);
	t->tex->tMode &= ~SST_TFORMAT;
      texRandomNccTable(sst, t->tex);		// NCC table 0, NCC texture
      t->tex->tMode |= SST_TNCCSELECT;		// NCC table 1
      texRandomNccTable(sst, t->tex);
      for (k=0; k <= diago.trex; k++)		// init textures in each TREX
	initTextures(base_sst, k);
      for (k=0; k <= diago.trex; k++) {		// reset TCU in each TREX
	tModes[k] &= ~(SST_TCOMBINE | SST_TACOMBINE);
	tModes[k] |= SST_TC_REPLACE | SST_TCA_REPLACE;
	sst = SST_TREX(base_sst,k);
	SET(sst->textureMode, tModes[k]);

	if(shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK &&
	   diago.writeFifo == 0)
	  assert(csimActiveTMUs(diago.sstCSIM) < 2);
      }

      if (diago.checkEveryTriangle || !diago.diff)
	DIAG_DIFFSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1);

      for (n=0; n<100; ) {		// at least 100 triangles/commands
	do {				// first pick a random chip
	    chip = rRandom(0,(4<<diago.trex)-1);
	}				// that includes FBI most of the time
	while (chip!=0 && (chip&1!=0) && iRandom(2));
	sst = SST_CHIP(base_sst,chip);
	k = iRandom(C1+40) & ~3;
	gdbg_info(5,"switch(%d)  chip #%d\n",k,chip);

	switch(k) {			// random register address
	  case VA_X:
		if (!sentVAx) {
		    sentVAx = 1;
		    toBeSent--;
		}
		SET(sst->vA.x,t->vA.x);
	  	break;
	  case VA_Y:
		if (!sentVAy) {
		    sentVAy = 1;
		    toBeSent--;
		}
		SET(sst->vA.y,t->vA.y);
	  	break;
	  case VB_X:
		if (!sentVAy) {
		    sentVBx = 1;
		    toBeSent--;
		}
		SET(sst->vB.x,t->vB.x);
	  	break;
	  case VB_Y:
		if (!sentVAy) {
		    sentVBy = 1;
		    toBeSent--;
		}
		SET(sst->vB.y,t->vB.y);
	  	break;
	  case VC_X:
		if (!sentVAy) {
		    sentVCx = 1;
		    toBeSent--;
		}
		SET(sst->vC.x,t->vC.x);
	  	break;
	  case VC_Y:
		if (!sentVAy) {
		    sentVCy = 1;
		    toBeSent--;
		}
		SET(sst->vC.y,t->vC.y);
	  	break;
	  case R:
		SET(sst->r,t->vA.r);
	  	break;
	  case G:
		SET(sst->g,t->vA.g);
	  	break;
	  case B:
		SET(sst->b,t->vA.b);
	  	break;
	  case A:
		SET(sst->a,t->vA.a);
	  	break;
	  case Z:
		SET(sst->z,(FxU32)(t->vA.z64>>(SST_Z64_SIZE-SST_Z_16BPP_SIZE)));
	  	break;
	  case W:
		SETW(sst->w,t->vA.w);
	  	break;
	  case DRDX:
		SET(sst->drdx,t->drdx);
	  	break;
	  case DGDX:
		SET(sst->dgdx,t->dgdx);
	  	break;
	  case DBDX:
		SET(sst->dbdx,t->dbdx);
	  	break;
	  case DADX:
		SET(sst->dadx,t->dadx);
	  	break;
	  case DZDX:
		SET(sst->dzdx,(FxU32)(t->dzdx64>>(SST_Z64_SIZE-SST_Z_16BPP_SIZE)));
	  	break;
	  case DWDX:
		SETW(sst->dwdx,t->dwdx);
	  	break;
	  case DRDY:
		SET(sst->drdy,t->drdy);
	  	break;
	  case DGDY:
		SET(sst->dgdy,t->dgdy);
	  	break;
	  case DBDY:
		SET(sst->dbdy,t->dbdy);
	  	break;
	  case DADY:
		SET(sst->dady,t->dady);
	  	break;
	  case DZDY:
		SET(sst->dzdy,(FxU32)(t->dzdy64>>(SST_Z64_SIZE-SST_Z_16BPP_SIZE)));
	  	break;
	  case DWDY:
		SETW(sst->dwdy,t->dwdy);
	  	break;

	  case TRIANGLECMD:
		if (toBeSent > 1) {	// keep going
		    break;
		}
		flushTriangle(sst,t);

		SET(sst->triangleCMD,t->area);
		newTriangle(t,iRandom(3),-1);
		n++;
		break;
	  case FVA_X:
		if (!sentVAx) {
		    sentVAx = 1;
		    toBeSent--;
		}
		SETF(sst->FvA.x,t->vA.fx);
		break;
	  case FVA_Y:
		if (!sentVAy) {
		    sentVAy = 1;
		    toBeSent--;
		}
		SETF(sst->FvA.y,t->vA.fy);
	  	break;
	  case FVB_X:
		if (!sentVAy) {
		    sentVBx = 1;
		    toBeSent--;
		}
		SETF(sst->FvB.x,t->vB.fx);
	  	break;
	  case FVB_Y:
		if (!sentVAy) {
		    sentVBy = 1;
		    toBeSent--;
		}
		SETF(sst->FvB.y,t->vB.fy);
	  	break;
	  case FVC_X:
		if (!sentVAy) {
		    sentVCx = 1;
		    toBeSent--;
		}
		SETF(sst->FvC.x,t->vC.fx);
	  	break;
	  case FVC_Y:
		if (!sentVAy) {
		    sentVCy = 1;
		    toBeSent--;
		}
		SETF(sst->FvC.y,t->vC.fy);
	  	break;
	  case FR:
		SETF(sst->Fr,t->vA.fr);
	  	break;
	  case FG:
		SETF(sst->Fg,t->vA.fg);
	  	break;
	  case FB:
		SETF(sst->Fb,t->vA.fb);
	  	break;
	  case FA:
		SETF(sst->Fa,t->vA.fa);
	  	break;
	  case FZ:
		SETF(sst->Fz,t->vA.fz);
	  	break;
	  case FW:
		SETF(sst->Fw,t->vA.fw);
	  	break;
	  case FDRDX:
		SETF(sst->Fdrdx,t->fdrdx);
	  	break;
	  case FDGDX:
		SETF(sst->Fdgdx,t->fdgdx);
	  	break;
	  case FDBDX:
		SETF(sst->Fdbdx,t->fdbdx);
	  	break;
	  case FDADX:
		SETF(sst->Fdadx,t->fdadx);
	  	break;
	  case FDZDX:
		SETF(sst->Fdzdx,t->fdzdx);
	  	break;
	  case FDWDX:
		SETF(sst->Fdwdx,t->fdwdx);
	  	break;
	  case FDRDY:
		SETF(sst->Fdrdy,t->fdrdy);
	  	break;
	  case FDGDY:
		SETF(sst->Fdgdy,t->fdgdy);
	  	break;
	  case FDBDY:
		SETF(sst->Fdbdy,t->fdbdy);
	  	break;
	  case FDADY:
		SETF(sst->Fdady,t->fdady);
	  	break;
	  case FDZDY:
		SETF(sst->Fdzdy,t->fdzdy);
	  	break;
	  case FDWDY:
		SETF(sst->Fdwdy,t->fdwdy);
	  	break;

	  case S:
		SET1418(sst->s,t->vA.s);
	  	break;
	  case T:
		SET1418(sst->t,t->vA.t);
	  	break;
	  case FS:
		SET1418(sst->dsdx,t->dsdx);
	  	break;
	  case FT:
		SETF(sst->Ft,t->vA.ft);
	  	break;
	  case DSDX:
		SETF(sst->Fdsdx,t->fdsdx);
	  	break;
	  case DSDY:
		SET1418(sst->dsdy,t->dsdy);
	  	break;
	  case DTDX:
		SET1418(sst->dtdx,t->dtdx);
	  	break;
	  case DTDY:
		SET1418(sst->dtdy,t->dtdy);
	  	break;
	  case FDSDX:
		SETF(sst->Fs,t->vA.fs);
	  	break;
	  case FDSDY:
		SETF(sst->Fdsdy,t->fdsdy);
	  	break;
	  case FDTDX:
		SETF(sst->Fdtdx,t->fdtdx);
	  	break;
	  case FDTDY:
		SETF(sst->Fdtdy,t->fdtdy);
	  	break;

	  case FTRIANGLECMD:
		flushTriangle(sst,t);
		
		SETF(sst->FtriangleCMD,(float)t->area);
		newTriangle(t,iRandom(6),-1);
		n++;
		break;

	  // use this group of registers to generate random trex regs
	  case FBZCOLORPATH:
	        while (!goodCcuPath(fbzColorPath=iRandom(0xFFFFFFFF), FXFALSE));
		if( CCU_USES_TEXMAP(fbzColorPath) )
		  fbzColorPath |= SST_ENTEXTUREMAP;
		
		//Make sure we don't enter an illegal state
		if(shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK)
		  if(csimActiveTMUs2(fbzColorPath, shadowRegisters3D[0][0].combineMode,
				     shadowRegisters3D[0][1].textureMode,
				     shadowRegisters3D[0][1].combineMode) > 1)
		    fbzColorPath &= ~SST_ENTEXTUREMAP;			


		//Make sure the FBI isn't using TMU data when texturing is disabled
		if(!(fbzColorPath & SST_ENTEXTUREMAP))
		  {
		    FxU32 combineMode=shadowRegisters3D[0][0].combineMode;


		    if(CCU_USES_TEXMAP(fbzColorPath))
		      {
			fbzColorPath &= ~(SST_RGBSELECT | SST_ASELECT | 
					  SST_LOCALSELECT_OVERRIDE_WITH_ATEX |
					  SST_CC_MSELECT | SST_CCA_MSELECT);
			fbzColorPath |= (SST_RGBSEL_RGBA | SST_ASEL_RGBA |
					 SST_CC_MONE | SST_CCA_MONE);
		      }

		    if(((combineMode & SST_CM_CC_OTHERSELECT) == SST_CM_CC_OTHERSELECT_TRGB) ||
		       ((combineMode & SST_CM_CC_OTHERSELECT) == SST_CM_CC_OTHERSELECT_TA))
		      {
			combineMode &= ~SST_CM_CC_OTHERSELECT;
			combineMode |= SST_CM_CC_OTHERSELECT_IRGB;
		      }	      
	    
		    if(((combineMode & SST_CM_CC_LOCALSELECT) == SST_CM_CC_LOCALSELECT_TRGB) ||
		       ((combineMode & SST_CM_CC_LOCALSELECT) == SST_CM_CC_LOCALSELECT_TA))
		      {
			combineMode &= ~SST_CM_CC_LOCALSELECT;
			combineMode |= SST_CM_CC_LOCALSELECT_IRGB;
		      }	      
		    
		    if((combineMode & SST_CM_CCA_OTHERSELECT) == SST_CM_CCA_OTHERSELECT_TA)
		      {
			combineMode &= ~SST_CM_CCA_OTHERSELECT;
			combineMode |= SST_CM_CCA_OTHERSELECT_IA;
		      }

		    if((combineMode & SST_CM_CCA_LOCALSELECT) == SST_CM_CCA_LOCALSELECT_TA)
		      {
			combineMode &= ~SST_CM_CCA_LOCALSELECT;
			combineMode |= SST_CM_CCA_LOCALSELECT_IA;
		      }
		    
		    if((fbzColorPath & (SST_CC_ADD_CLOCAL | SST_CC_ADD_ALOCAL)) == (SST_CC_ADD_CLOCAL | SST_CC_ADD_ALOCAL))
		      fbzColorPath = fbzColorPath & (~(SST_CC_ADD_ALOCAL | SST_CC_ADD_ALOCAL));

		    if((fbzColorPath & (SST_CCA_ADD_CLOCAL | SST_CCA_ADD_ALOCAL)) == (SST_CCA_ADD_CLOCAL | SST_CCA_ADD_ALOCAL))
		      fbzColorPath = fbzColorPath & (~(SST_CCA_ADD_ALOCAL | SST_CCA_ADD_ALOCAL));

		    fbzColorPath &= ~SST_LOCALSELECT_OVERRIDE_WITH_ATEX;
		   
		    SET(base_sst->combineMode, combineMode);
		  }

		//Don't allow sub-pixel correction with AA
		if(diago.aaEnabled)
		  fbzColorPath &= ~(SST_PARMADJUST);

		SET(base_sst->fbzColorPath,fbzColorPath);		
		break;
	  case FOGMODE:
		SET(sst->fogMode,iRandom(0xFFFFFFFF));
		break;
	  case STATUS:			// change renderMode
		renMode ^= iRandom(0xFFFFFFFF) & (SST_RM_ALPHAMODE |
				SST_RM_RED_WMASK | SST_RM_GREEN_WMASK | 
				SST_RM_BLUE_WMASK | SST_RM_ALPHA_WMASK);
		if ((renMode & SST_RM_ALPHAMODE) == SST_RM_ALPHAMODE)
		    renMode ^= SST_RM_ALPHAMODE;

		//Make sure fbi and tmu's are using the same band height
		renMode &= ~SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION;
		renMode |= shadowRegisters3D[0][0].renderMode & SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION;
		
		SET(sst->renderMode,renMode);
		break;
	  case INTRCTRL:		// change stencilMode
		if (diago.rgb == 32)
		    randomStencils(sst);
		break;
	  case ALPHAMODE:
		// if not going to FBI then use any old random alphaMode
		if ((chip != 0) && !(chip & 1)) {
		    SET(sst->alphaMode, iRandom(0xFFFFFFFF));
		    break;
		}
		while (!goodAlphaMode(amode=iRandom(0xFFFFFFFF),fbzMode));
		// don't allow pixels to fail too often
		if (amode & SST_ENALPHAFUNC) {
		    if (amode & SST_ALPHAFUNC_EQ) {
			if (iRandom(4))		// most of time turn on GT
			    amode |= SST_ALPHAFUNC_GT;
		    }
		}
		SET(sst->alphaMode,amode);
		break;
	  case FBZMODE:
		// if not going to FBI then use any old random fbzMode
		if ((chip != 0) && !(chip & 1)) {
		    SET(sst->fbzMode, iRandom(0xFFFFFFFF));
		    break;
		}
		// generate a random, but valid fbz mode
		fbzMode = iRandom(0xFFFFFFFF);
#ifdef CVG
		// turn off high drawbuffer bit (its unused)
		fbzMode &= ~ (1<<(SST_DRAWBUFFER_SHIFT+1));
#endif
		// keep RGBmask mostly on, cliprect always on
		if (iRandom(5)) fbzMode |= SST_RGBWRMASK;
		fbzMode |= SST_ENRECTCLIP;

		if(diago.chipCount > 1)
		  fbzMode &= ~SST_YORIGIN;
		if (!diago.zeroLodFrac)		// turn off zbuffer
		    fbzMode &= ~(SST_ENDEPTHBUFFER | SST_ZAWRMASK);
		// if no aux buffer present then turn off alpha buffer
		// or if in 15 or 32 bpp mode
		if (diago.rgb!=16 || !diago.hasAuxBuffer)
		   fbzMode &= ~SST_ENALPHABUFFER;
		// do some alphablending sanity checks for 16bpp modes
		if (diago.rgb == 16) {
		  if (amode & SST_ENALPHABLEND) {
		    // if alphamode uses destination alpha, but alphabuffer not enabled
		    // then turn on alphabuffer
		    if (!(fbzMode & SST_ENALPHABUFFER) &&
			!goodAlphaMode(amode,fbzMode) && diago.hasAuxBuffer)
			fbzMode |= SST_ENALPHABUFFER;
		    // if alphamode uses destination alpha, turn off depthbuffer
		    if (!goodAlphaMode(amode,fbzMode))
			fbzMode &= ~SST_ENDEPTHBUFFER;
		  }
		}
		else {
		    // turn off dithering in 32bpp mode
		    if (diago.rgb == 32)
			fbzMode &= ~(SST_ENDITHER |SST_ENDITHERSUBTRACT);
		}		// if both bits are on, randomly turn one off
		if ((fbzMode & (SST_ENALPHABUFFER|SST_ENDEPTHBUFFER)) ==
			(SST_ENALPHABUFFER|SST_ENDEPTHBUFFER))
		   fbzMode ^=  iRandom(1) ? SST_ENALPHABUFFER : SST_ENDEPTHBUFFER;

		fbzMode &= ~ SST_ENSTIPPLE;	// col8 rendering doesn't match CSIM
		SET(sst->fbzMode, fbzMode);
		break;
	  case LFBMODE:
		{
		    unsigned long lmode;
		    do {
			lmode = iRandom(0xFFFFFFFF);
		    } while (!goodLfbMode(lmode));
		    SET(sst->lfbMode,lmode);
		    if ((chip == 0) || (chip & 1)) {	// if written to FBI
			lmode &= SST_LFB_FORMAT;
			if (lmode >= SST_LFB_888 && lmode <= SST_LFB_Z1555)
			    lfbBytesPerPixel = 4;
			else
			    lfbBytesPerPixel = 2;
		    }
		}
		break;
	  case CLIPLEFTRIGHT:
		{
		    int t1,t2;
		    t1 = iRandom(33);	t2 = iRandom(33);
		    t1 = (t1<<16) | (diago.xmaxscreen-t2);
		    SET(sst->clipLeftRight,t1);
		}
		break;
	  case CLIPBOTTOMTOP:
		{
		    int t1,t2;
		    t1 = iRandom(29);	t2 = iRandom(29);
		    t1 = (t1<<16) | (diago.ymaxscreen-t2);
		    SET(sst->clipBottomTop,t1);
		}
	  case NOPCMD:
		if (iRandom(3)==0)
		    SET(base_sst->nopCMD,iRandom(0xFFFFFFFF));
		break;
	  case FASTFILLCMD:
		if (iRandom(1)) {
		    int x,y,w,h;
#ifdef H4
		    SstIORegs *sstio = (SstIORegs *) SST_IO_ADDRESS(base_sst);
		    int sdram = GET(sstio->dramInit1) & SST_MCTL_TYPE_SDRAM;
#endif
		again:
		    xyRandom(&x,&y);		// pick random x,y onscreen
		    do {			// and random width and height
			w = iRandom(15);
			h = iRandom(13);
		    } while (w * h > 200);
		    if (!ONSCREEN(x+w-1,y+h-1))
			goto again;
		    SET(base_sst->clipLeftRight, (x<<16) | (x+w));
		    SET(base_sst->clipBottomTop, (y<<16) | (y+h));
		    SET(base_sst->fastfillCMD,iRandom(0xFFFFFFFD));
		    x = iRandom(79);	w = iRandom(32);
		    y = iRandom(67);	h = iRandom(32);
		    SET(base_sst->clipLeftRight, (x<<16) | (diago.xmaxscreen-w));
		    SET(base_sst->clipBottomTop, (y<<16) | (diago.ymaxscreen-h));
		}
		break;
	  case SWAPBUFFERCMD:
		if (iRandom(4)==0)
		    DIAG_SWAPBUFFER();
		break;

	  case FOGCOLOR:
		SET(sst->fogColor,iRandom(0xFFFFFFFF));
		break;
	  case ZACOLOR:
	        zaColor = iRandom(0xFFFFFFFF);
	        SET(sst->zaColor,zaColor);
		break;
	  case CHROMAKEY:
		SET(sst->chromaKey,iRandom(0x00FFFFFF));
		break;

	  case USERINTRCMD:
		randomCombineMode(base_sst,chip);
		break;

	  case STIPPLE:
		SET(sst->stipple,iRandom(0xFFFFFFFF));
		break;
	  case C0:
		SET(sst->c0,iRandom(0xFFFFFFFF));
		break;
	  case C1:
	        c1 = iRandom(0xFFFFFFFF);
		SET(sst->c1,c1);
		break;

	  case C1+4:
	  case C1+8:
		if (toBeSent > 2) {	// keep going
		    break;
		}
		gdbg_info(5,"special case\n");
		switch (iRandom(3)) {
		    case 0:		// NCC table (random 0,1)
			k = chip & 0xE;
			if (k == 2) k = 0;
			else if (k == 4) k = 1;
			else if (k == 8) k = 2;
			else {
			    k = iRandom(diago.trex);	// pick a new TREX chip (just one)
			    chip &= 0x1;		// keep FBI if selected
			    chip |= 1 << (k+1);
			    sst = SST_CHIP(base_sst,chip);
			}
			// k = TREX # (0,1,2) that we are dealing with
			gdbg_info(5,"new NCC/Pal table, cur tMode = 0x%x\n",tModes[k]);
			t->tex->tMode = tModes[k];
			if (texFormatHasPalette(tModes[k])) {
			    // write a few random entries
			    for (k=rRandom(1,5); k>0; k--) {
				volatile unsigned long *nTab;
				nTab = sst->nccTable0;	// point to NCC table
				nTab += rRandom(4,11);	// pick random I or Q slot
				SET(*nTab, 0x80000000 | iRandom(0x7FFFFFFF));
			    }
			}
			else {
			    if (iRandom(1))
				t->tex->tMode ^= SST_TNCCSELECT;
			    texRandomNccTable(sst, t->tex);
			}
			break;
		    case 1:	// FOG TABLE write
			for (k=rRandom(1,3); k>0; k--) {
			    FxU32 foge = sst_random_fog_table_entry();
			    foge |= sst_random_fog_table_entry()<<16;
			    SET(sst->fogTable[iRandom(31)],foge);
			}
			break;
		    case 2:	// LFB accesses, random 16 or 32 bit writes/reads
			if (!diago.floatSTW) {		// if disabled, skip it
			    break;
			}
			// disallow RGBA from texture map when writing LFB
			if( CCU_USES_TEXMAP(fbzColorPath) ) {
			  FxU32 t, saveSeed;
			  saveSeed = getSeed();
			  while (!goodCcuPath(t=iRandom(0xFFFFFFFF), FXTRUE)); // FBI-only Ccu setting
			  setSeed(saveSeed);
			  SET(base_sst->fbzColorPath,t);
			}
			
			//Make sure that the FBI's combine mode isn't using the texture data
			//for the local or other RGBA
			if(shadowRegisters3D[0][0].combineMode & SST_CM_USE_COMBINE_MODE)
			  {
			    FxU32 combineMode;

			    combineMode = shadowRegisters3D[0][0].combineMode;
			    
			    while(((combineMode & SST_CM_CC_OTHERSELECT) ==
				   SST_CM_CC_OTHERSELECT_TRGB) ||
				  ((combineMode & SST_CM_CC_OTHERSELECT) ==
				   SST_CM_CC_OTHERSELECT_TA))
			      {
				combineMode &= ~SST_CM_CC_OTHERSELECT;
				combineMode |= iRandom(SST_CM_CC_OTHERSELECT) & SST_CM_CC_OTHERSELECT;
			      }

			    while(((combineMode & SST_CM_CC_LOCALSELECT) ==
				   SST_CM_CC_LOCALSELECT_TRGB) ||
				  ((combineMode & SST_CM_CC_LOCALSELECT) ==
				   SST_CM_CC_LOCALSELECT_TA))
			      {
				combineMode &= ~SST_CM_CC_LOCALSELECT;
				combineMode |= iRandom(SST_CM_CC_LOCALSELECT) & SST_CM_CC_LOCALSELECT;
			      }

			    if((combineMode & SST_CM_CCA_OTHERSELECT) == SST_CM_CCA_OTHERSELECT_TA)
			      {
				combineMode &= ~SST_CM_CCA_OTHERSELECT;
				combineMode |= SST_CM_CCA_OTHERSELECT_IA;
			      }
				
			    if((combineMode & SST_CM_CCA_LOCALSELECT) == SST_CM_CCA_LOCALSELECT_TA)
			      {
				combineMode &= ~SST_CM_CCA_LOCALSELECT;
				combineMode |= SST_CM_CCA_LOCALSELECT_IA;
			      }

			    if(((combineMode & SST_CM_CCA_OTHERSELECT) == SST_CM_CCA_OTHERSELECT_LFB_A) ^
			       ((combineMode & SST_CM_CC_OTHERSELECT) == SST_CM_CC_OTHERSELECT_LFB_RGB))
			      {
				combineMode &= ~(SST_CM_TC_OTHERSELECT  | SST_CM_TC_LOCALSELECT | SST_CM_TC_MSELECT_7 |
						 SST_CM_TCA_OTHERSELECT | SST_CM_TCA_LOCALSELECT);
				combineMode |= SST_CM_CC_OTHERSELECT_IRGB | SST_CM_CCA_OTHERSELECT_IA;
			      }

			    SET_FBI(base_sst->combineMode, combineMode);				  
			    SET_0(base_sst->combineMode, SST_CM_TC_OTHERSELECT_LOCAL_TRGB | 
				  SST_CM_TCA_OTHERSELECT_LOCAL_TA | 
				  (shadowRegisters3D[0][1].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK));
			    SET_1(base_sst->combineMode, SST_CM_TC_OTHERSELECT_LOCAL_TRGB | 
				  SST_CM_TCA_OTHERSELECT_LOCAL_TA |
				  (shadowRegisters3D[0][2].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK));

			    if(shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK &&
			       diago.writeFifo == 0)
			      assert(csimActiveTMUs(diago.sstCSIM) < 2);
			  }

			for (k=rRandom(1,10); k>0; k--) {
			    int x = iRandom(diago.xmaxscreen-1);
			    int y = iRandom(diago.ymaxscreen-1);
			    long addr = SST_LFB_ADDRESS(base_sst);
			    int bpp = lfbBytesPerPixel;
#ifdef CVG
			    int lfbWrites = iRandom(3);		// do mostly writes
#else
			    int lfbWrites = 1;                  // lfb reads not supported
#endif

			    if (!lfbWrites) bpp = 2;
			    gdbg_info(6,"LFB access at %d,%d bpp=%d\n",x,y,bpp);
			    addr += y * bpp * 1024;
			    if (lfbWrites && bpp == 2) {	// 16-bit per pixel
				if (iRandom(1)) {		// write a short
				    addr += x * bpp;
				    SET16(*(unsigned short *)addr, (short)iRandom(0xFFFF));
				    break;
				}
			    }
			    // reading/writing 32-bits or 2 pixels so force even x
			    if (bpp == 2)
				x &= ~1;
			    addr += x * bpp;
			    if (lfbWrites)		// do mostly writes
				SET(*(unsigned long *)addr, iRandom(0xFFFFFFFF));
			    else	// GMT: no way of verifying data read!!!
				GET(*(unsigned long *)addr);
			    
			}
			if( CCU_USES_TEXMAP(fbzColorPath) )
			  SET(base_sst->fbzColorPath,fbzColorPath);
			n++;
			break;
		    default:
			k = rRandom(2,5);
			gdbg_info(5,"batch of %d triangles\n",k);
			for (; k>0; k--) {
			    flushTriangle(sst,t);

			    SET(base_sst->triangleCMD,t->area);
			    // sometimes generate a new triangle, sometimes not
			    if (iRandom(1))
				newTriangle(t,iRandom(5),-1);
			}
			n += k;
			break;
		}
	  	break;

	  default:
		// make sure chip is only referencing one TREX chip
		k = chip & 0xE;
		if (k == 2) k = 0;
		else if (k == 4) k = 1;
		else if (k == 8) k = 2;
		else {
		    k = iRandom(diago.trex);	// pick a new TREX chip (just one)
		    chip &= 0x1;		// keep FBI if selected
		    chip |= 1 << (k+1);
		    sst = SST_CHIP(base_sst,chip);
		}
		// k = TREX # (0,1,2) that we are dealing with
		selection = iRandom(8);
		gdbg_info(5,"change texture case(%d)\n", selection);

		switch(selection) {	  
		    case 0:
		    {
			int tmode,tcu;
			// pick random tmode, same tex size as last
			tmode = texRandomFormat(SST_T16BIT(tModes[k]),SST_T32BIT(tModes[k]),
						iRandom(1));
			gdbg_info(6,"\ttextureMode: %x\n",tmode);
			tmode |= iRandom(0xFF);
		    again_rgb:
			tcu = tcuRandom();
			if (k == diago.trex ||
			    (shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK))
			  { // if furthest upstream or in 2ppc
			    tcu |= SST_TC_ZERO_OTHER;	// avoid undefined inputs
			    if ((tcu & SST_TC_MSELECT)==SST_TC_MAOTHER)
			      goto again_rgb;
			  }
			tmode |= tcu;
		    again_alpha:
			tcu = tcuRandom();
			if (k == diago.trex || 
			    (shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK))
			  { // if furthest upstream or in 2ppc
			    tcu |= SST_TCA_ZERO_OTHER;	// avoid undefined inputs
			    if ((tcu & SST_TCA_MSELECT)==SST_TCA_MAOTHER)
			      goto again_alpha;
			  }
			tmode |= (tcu>>SST_TCOMBINE_SHIFT)<<SST_TACOMBINE_SHIFT;
			tmode = SST_TC_ZERO_OTHER | SST_TCA_ZERO_OTHER | SST_TC_MALOCAL | SST_TCA_MALOCAL;
			tModes[k] = tmode;
			SET(sst->textureMode,tmode);
			if(shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK && 
			   diago.writeFifo == 0)			   
			  {
			    FxU32 nActiveTMUs;

			    nActiveTMUs = csimActiveTMUs(diago.sstCSIM);
			    
			    assert(nActiveTMUs < 2);
			  }
			break;
		    }
		    case 1:		// tLOD, change all fields but the shape
		    {
			int tmin,tmax,tLOD = tLODs[k];
			gdbg_info(6,"\ttLOD\n");
			tLOD &= ~(SST_LODMIN | SST_LODMAX);

			tmin = iRandom(iRandom(8<<SST_LOD_FRACBITS));
			tmax = rRandom(tmin,8<<SST_LOD_FRACBITS);

			if(diago.bigAssTextures)
			  { //for 2048x2048 textures, we're only testing lod's 3-11
			    tmin += 3<<SST_LOD_FRACBITS;
			    tmax += 3<<SST_LOD_FRACBITS;
			  }

			tLOD |= tmin<<SST_LODMIN_SHIFT;
			tLOD |= tmax<<SST_LODMAX_SHIFT;
			tLOD ^= iRandom(SST_LODBIAS) & SST_LODBIAS;
			if (iRandom(1)) tLOD ^= SST_LOD_ODD;			
			if (iRandom(1)) tLOD ^= SST_LOD_ZEROFRAC;
			// texture mirroring not supported until rev 6
			if (CSIM_PRIVATE(diago.sstCSIM)->info->tmuRevision >= 6) {
			    if (iRandom(1)) tLOD ^= SST_TMIRRORS;			
			    if (iRandom(1)) tLOD ^= SST_TMIRRORT;
			}
// TODO: implement TSPLIT
			tLODs[k] = tLOD;
			SET(sst->tLOD,tLOD);
			if (tLODs[k] & SST_TMULTIBASEADDR) {
			    int t = texs[k];
			    SET(sst->texBaseAddr1,textures[k][t].addr[1]>>TEXBASE_SHIFT);
			    SET(sst->texBaseAddr2,textures[k][t].addr[2]>>TEXBASE_SHIFT);
			    SET(sst->texBaseAddr38,textures[k][t].addr[3]>>TEXBASE_SHIFT);
			}
			break;
		    }
		    case 2:		// tDetail
		    {	// set random detail blending settings
			int detailMax,detailBias,detailScale;
			gdbg_info(6,"\ttDetail\n");
			detailMax = iRandom(0xFF);
			detailBias = iRandom(iRandom(iRandom(0x1F)));
			if (iRandom(1)) detailBias = -detailBias;
			detailBias &= 0x3F;
			detailScale = iRandom(iRandom(0x7));
			detailMax = (detailScale<<SST_DETAIL_SCALE_SHIFT) |
					(detailBias<<SST_DETAIL_BIAS_SHIFT) |
					(detailMax<<SST_DETAIL_MAX_SHIFT);
#define SEP_FILTER (SST_TMINFILTER_RGB|SST_TMAGFILTER_RGB|\
		SST_TMINFILTER_A|SST_TMAGFILTER_A|SST_TFILTER_SEPARATE)

			// separate RGB,A filter control bits
			detailMax |= iRandom(SEP_FILTER) & SEP_FILTER;
			SET(sst->tDetail,detailMax);
			break;
		    }
		    case 3:		// texBaseAddr
		    {
		      int bpt;
		      int t;
		      int tLOD = tLODs[k];

		      if(SST_T4BIT_COMPRESSED(tModes[k]))
			bpt=0;
		      else if(SST_T8BIT_COMPRESSED(tModes[k]))
			bpt=1;		      
		      else if(SST_T8BIT(tModes[k]))
			bpt=1;
		      else if(SST_T16BIT(tModes[k]))
			bpt=2;
		      else if(SST_T32BIT(tModes[k]))
			bpt=4;
		      
		      // select random texture of the same bit depth as the current one
		      t = randomTex(k, bpt);

			gdbg_info(6,"\ttexBaseAddr\n");
			if (textures[k][t].tiled)
			  SET(sst->texBaseAddr, textures[k][t].addr[0] >> TEXBASE_SHIFT | SST_TEXTURE_IS_TILED |
			      (textures[k][t].tStride<<SST_TEXTURE_TILESTRIDE_SHIFT));
			else
			  SET(sst->texBaseAddr,textures[k][t].addr[0]>>TEXBASE_SHIFT);

			texs[k] = t;
			// however, the aspect ratio data may have changed
			// so copy the new textures aspect ratio data
			tLOD &= ~(SST_LOD_S_IS_WIDER | SST_LOD_ASPECT);
			tLOD |= textures[k][t].tLOD & (SST_LOD_S_IS_WIDER | SST_LOD_ASPECT);
			tLODs[k] = tLOD;
			SET(sst->tLOD,tLOD);
			if (tLODs[k] & SST_TMULTIBASEADDR) {
			    SET(sst->texBaseAddr1,textures[k][t].addr[1]>>TEXBASE_SHIFT);
			    SET(sst->texBaseAddr2,textures[k][t].addr[2]>>TEXBASE_SHIFT);
			    SET(sst->texBaseAddr38,textures[k][t].addr[3]>>TEXBASE_SHIFT);
			}
			break;
		    }
		    // NOTE: we can't change the multi-base addresses to a different
		    // aspect texture, so we hack it, picking mipmap <= 'n'
		    case 4:		// texBaseAddr1
		    {
			int t = texs[k];
			gdbg_info(6,"\ttexBaseAddr1\n");
			SET(sst->texBaseAddr1,textures[k][t].addr[iRandom(1)]>>TEXBASE_SHIFT);
			break;
		    
		    }
		    case 5:		// texBaseAddr2
		    {
			int t = texs[k];
			gdbg_info(6,"\ttexBaseAddr2\n");
			SET(sst->texBaseAddr2,textures[k][t].addr[iRandom(2)]>>TEXBASE_SHIFT);
			break;
		    }
		    case 6:		// texBaseAddr38
		    {			// cannot change this one
			int t = texs[k];
			gdbg_info(6,"\ttexBaseAddr38\n");
			SET(sst->texBaseAddr38,textures[k][t].addr[3]>>TEXBASE_SHIFT);
			break;
		    }
		    case 7:		// download (again) a small LOD
		    if (iRandom(4)==0) {	// only 1/16 of the time
		      int bpt;
		      int t;
		      int ar,lod=5,slog=3,tlog=3;	// biggest is 8x8

		      static Texture *tex=NULL;

		      if(tex == NULL)
			{
			  if(diago.bigAssTextures)
			    tex = buildTexture(2048, 2048);
			  else
			    tex = buildTexture(256, 256);	
			}

		      if(SST_T4BIT_COMPRESSED(tModes[k]))
			bpt=0;
		      else if(SST_T8BIT_COMPRESSED(tModes[k]))
			bpt=1;		      
		      else if(SST_T8BIT(tModes[k]))
			bpt=1;
		      else if(SST_T16BIT(tModes[k]))
			bpt=2;
		      else if(SST_T32BIT(tModes[k]))
			bpt=4;

		      t = randomTex(k, bpt);


			// fetch its texture Mode and recompute its slog,tlog
			tex->tMode = textures[k][t].tMode;
			ar = (textures[k][t].tLOD & SST_LOD_ASPECT) >> SST_LOD_ASPECT_SHIFT;
			if (textures[k][t].tLOD & SST_LOD_S_IS_WIDER)
			    tlog -= ar;
			else
			    slog -= ar;
			ar = iRandom(1);	// randomly download mipmaps
			if (!ar) {
			    ar = iRandom(3);	// pick just one level
			    lod += ar;
			    slog -= ar;	//if (slog < 0) slog = 0;
			    tlog -= ar; //if (tlog < 0) tlog = 0;
			    ar = 0;
			}
			gdbg_info(2,"texture download, mips=%d slog=%d tlog=%d\n",
					ar,slog,tlog);

			// download a small texture map, and randomly its mipmaps
#ifndef CVG
			tex->tiled = textures[k][t].tiled;
			tex->tStride = textures[k][t].tStride;
#endif
			texRandomTextureMapEx(base_sst,k,
				ar,slog,tlog,tex,textures[k][t].addr[lod]);
			// reset the texture base address and tLOD registers in this chip
#ifndef CVG
			if (textures[k][texs[k]].tiled)
			  SET(sst->texBaseAddr, textures[k][texs[k]].addr[0] | SST_TEXTURE_IS_TILED |
			      (textures[k][texs[k]].tStride<<SST_TEXTURE_TILESTRIDE_SHIFT));
			else
			  SET(sst->texBaseAddr,textures[k][texs[k]].addr[0]>>TEXBASE_SHIFT);
#else
			SET(sst->texBaseAddr,textures[k][texs[k]].addr[0]>>TEXBASE_SHIFT);
#endif
			SET(sst->tLOD,tLODs[k]);
			// reset all the tmodes in this chip and downstream
			// as they are trashed by texRandomTextureMapEx()
			while (k >= 0) {
			    SET(SST_TREX(base_sst,k)->textureMode,tModes[k]);
			    k--;
			}
			if(shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK &&
			   diago.writeFifo == 0)
			  assert(csimActiveTMUs(diago.sstCSIM) < 2);
			break;
		    }
		    break;

		    /**********************************************************************/
		    /****      Randomly toggle between 1 and 2 pixels per clock       *****/
		    /**********************************************************************/		    
                    case 8:
		      {
			FxU32 selection;
			FxU32 oldPixelsPerClock;
			
			selection=diago.pixelsPerClock;
			if(selection != 1 && selection != 2)
			  selection=iRandom(1) + 1;

			if(csimActiveTMUs(diago.sstCSIM) > 1)
			  selection=1;

			if(shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK)
			  oldPixelsPerClock = 2;
			else
			  oldPixelsPerClock = 1;

			switch(selection)
			  {
			  case 1:  //1 pixel per clock

			    //Add Scott's fix for hardware bug
			    if(oldPixelsPerClock == 2)
			      {
				FxU32 counter;

				GDBG_INFO(0, "Changing from 2 pixels per clock to 1 pixel per clock\n");
				GDBG_INFO(0, "  Inserting 12 TMU NOPs to flush TMUs\n");

				for(counter=0; counter<12; counter++)
				  SET_0_1(base_sst->nopCMD, 0);
			      }

			    SET_FBI(base_sst->combineMode, shadowRegisters3D[0][0].combineMode &
				    ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
			    SET_0(base_sst->combineMode, shadowRegisters3D[0][1].combineMode &
				  ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
			    SET_1(base_sst->combineMode, shadowRegisters3D[0][2].combineMode &
				  ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
			      break;

			  case 2:  //2 pixels per clock
			    SET_FBI(base_sst->combineMode, shadowRegisters3D[0][0].combineMode |
				SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
			    SET_0(base_sst->combineMode, shadowRegisters3D[0][1].combineMode |
				SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
			    SET_1(base_sst->combineMode, shadowRegisters3D[0][2].combineMode |
				SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
			    
			    //Make sure both TMUs have the same state
			    SET_0_1(base_sst->textureMode, shadowRegisters3D[0][1].textureMode);
			    SET_0_1(base_sst->tDetail, shadowRegisters3D[0][1].tDetail);
			    SET_0_1(base_sst->tLOD, shadowRegisters3D[0][1].tLOD);
			    SET_0_1(base_sst->texBaseAddr, shadowRegisters3D[0][1].texBaseAddr);
			    SET_0_1(base_sst->texBaseAddr1, shadowRegisters3D[0][1].texBaseAddr1);
			    SET_0_1(base_sst->texBaseAddr2, shadowRegisters3D[0][1].texBaseAddr2);
			    SET_0_1(base_sst->texBaseAddr38, shadowRegisters3D[0][1].texBaseAddr38);
			    SET_0_1(base_sst->combineMode, shadowRegisters3D[0][1].combineMode);
			    SET_0_1(base_sst->chromaKey, shadowRegisters3D[0][1].chromaKey);
			    SET_0_1(base_sst->chromaRange, shadowRegisters3D[0][1].chromaRange);

			    SET_0_1(base_sst->r, shadowRegisters3D[0][1].r);
			    SET_0_1(base_sst->g, shadowRegisters3D[0][1].g);
			    SET_0_1(base_sst->b, shadowRegisters3D[0][1].b);
			    SET_0_1(base_sst->a, shadowRegisters3D[0][1].a);
			    SET_0_1(base_sst->s, shadowRegisters3D[0][1].s);
			    SET_0_1(base_sst->t, shadowRegisters3D[0][1].t);
			    SET_0_1(base_sst->w, shadowRegisters3D[0][1].w);

			    SET_0_1(base_sst->drdx, shadowRegisters3D[0][1].drdx);
			    SET_0_1(base_sst->dgdx, shadowRegisters3D[0][1].dgdx);
			    SET_0_1(base_sst->dbdx, shadowRegisters3D[0][1].dbdx);
			    SET_0_1(base_sst->dadx, shadowRegisters3D[0][1].dadx);
			    SET_0_1(base_sst->dsdx, shadowRegisters3D[0][1].dsdx);
			    SET_0_1(base_sst->dtdx, shadowRegisters3D[0][1].dtdx);
			    SET_0_1(base_sst->dwdx, shadowRegisters3D[0][1].dwdx);

			    SET_0_1(base_sst->drdy, shadowRegisters3D[0][1].drdy);
			    SET_0_1(base_sst->dgdy, shadowRegisters3D[0][1].dgdy);
			    SET_0_1(base_sst->dbdy, shadowRegisters3D[0][1].dbdy);
			    SET_0_1(base_sst->dady, shadowRegisters3D[0][1].dady);
			    SET_0_1(base_sst->dsdy, shadowRegisters3D[0][1].dsdy);
			    SET_0_1(base_sst->dtdy, shadowRegisters3D[0][1].dtdy);
			    SET_0_1(base_sst->dwdy, shadowRegisters3D[0][1].dwdy);			   
			    
			    if(diago.writeFifo == 0)
			      assert(csimActiveTMUs(diago.sstCSIM) < 2);

			    break;			    

			  default:
			    assert(0);
			  }
		      }
		      break;
		}
		break;
		
	}
      }
    }
    DIAG_PASS(0);
}

// TODO: make sure this works with 2 - 3 trex chips

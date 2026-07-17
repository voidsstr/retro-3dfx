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
** $Revision: 6$
** $Date: 10/11/00 8:19:26 PM$
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
    int addr[12];	// physical base addresses of each LOD
    int tiled;
    int tStride;
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
    int j,n,ar,slog,tlog,array[14];
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
      { //for 2048x2048 textures, we test LOD's 0-11
	tex->lodmin = 0;			// diag will change this randomly
	tex->lodmax = 11;
      }
    else
      { //256x256 case
	tex->lodmin = 0;			// diag will change this randomly
	tex->lodmax = 8;
      }

    tex->tLOD = 0;

    for (n=0; n<14; n++) {
      
      if(diago.bigAssTextures) //2048x2048 case
	slog = tlog = 11;
      else	
	slog = tlog = 8;

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
	    assert(ar <= 3);
	    slog -= ar;
	}
	else {
	    ar = j;
	    assert(ar <= 3);
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
	for (j=0; j< (FxI32)tex->nMipmaps; j++)
	  {
	    if(tex->tiled)
	      textures[trex][n].addr[j] = tex->mip[j]->textureBaseAddress;
	    else
	      textures[trex][n].addr[j] = tex->mip[j]->mipmapBaseAddress;
	  }

	textures[trex][n].tStride = tex->tStride;
	textures[trex][n].tiled = tex->tiled;
    }

    for (n=0; n<14; n++) {
	gdbg_info(3,"texture %d (%s) on TREX %d\n",n,
		  textures[trex][n].tiled?"Tiled":"Linear",trex);

	if(diago.bigAssTextures)
	  {
	    for (j=0; j<=11; j++)
	      gdbg_info(3,"  lod %d at 0x%x\n",j,textures[trex][n].addr[j]);
	  }
	else
	  {
	    for (j=0; j<=8; j++)	
	      gdbg_info(3,"  lod %d at 0x%x\n",j,textures[trex][n].addr[j]);
	  }
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

void
main (int argc, char **argv)
{
    int k,n, chip;
    int lfbBytesPerPixel=2;
    FxU32 selection;
    Triangle *t;
    SstRegs *sst, *base_sst;

    base_sst = sst = SST_BEGIN(argc,argv);
    if (!diago.diff) {
	gdbg_error("tstress1","must run with -D option, forcing -D\n");
	diago.diff = 1;
    }
    if (diago.trex < 0)				// ignore random TREX hint
	diago.trex = -diago.trex;
    diago.multiTexBaseAddr = 0;			// and multiTexBaseAddr
    diago.perspective = 1;			// need to run in perspective
    diago.adjust = 1;				// and with adjust on

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

    //Don't allow sub-pixel correction with AA
    if(diago.aaEnabled)
      diago.adjust = 0;

    //Make sure that the subsample jitter is 0,0. Otherwise, there can be goofy
    //mismatches between the csim and rtl/hw. This occurs because this diag only
    //sends down partial triangles
    if(diago.aaEnabled)
      SET(base_sst->aaCtrl, SST_AA_CONTROL_AA_ENABLE);


    // setup some reasonable starting modes
    SET(sst->fbzMode, SST_ENRECTCLIP | SST_RGBWRMASK);
    SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT | SST_ENTEXTUREMAP |
		(diago.adjust?SST_PARMADJUST:0));

    // send random (but valid) register writes to the chip
    while (DIAG_STARTPASS())  {			// for each pass
      newTriangle(t,1,1);

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
      }

      if (diago.checkEveryTriangle || !diago.diff)
	DIAG_DIFFSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1);

      for (n=0; n<100; ) {		// at least 100 triangles/commands
	do {				// first pick a random chip
	    chip = rRandom(0,(4<<diago.trex)-1);
	}				// that includes FBI most of the time
	while (chip!=0 && (chip&1!=0) && iRandom(2));
	sst = SST_CHIP(base_sst,chip);

	k = iRandom(C1) & ~3;
	gdbg_info(5,"switch(%d)  chip #%d\n",k,chip);

	switch(k) {			// random register address
	  case STATUS:			// increase probability
	  case VA_X:
		if (!sentVAx) {
		    sentVAx = 1;
		    toBeSent--;
		}
		SET(sst->vA.x,t->vA.x);
	  	break;
	  case INTRCTRL:		// increase probability
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
	  case FOGMODE:
	  case ALPHAMODE:
	  case FBZMODE:
	  case LFBMODE:
		break;
	  case CLIPLEFTRIGHT:
	  case CLIPBOTTOMTOP:
	  case NOPCMD:
	  case FASTFILLCMD:
	  case SWAPBUFFERCMD:
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
			if (k == diago.trex) {		// if furthest upstream
			    tcu |= SST_TC_ZERO_OTHER;	// avoid undefined inputs
			    if ((tcu & SST_TC_MSELECT)==SST_TC_MAOTHER)
				goto again_rgb;
			}
			tmode |= tcu;
		    again_alpha:
			tcu = tcuRandom();
			if (k == diago.trex) {		// if furthest upstream
			    tcu |= SST_TC_ZERO_OTHER;	// avoid undefined inputs
			    if ((tcu & SST_TC_MSELECT)==SST_TC_MAOTHER)
				goto again_alpha;
			}
			tmode |= (tcu>>SST_TCOMBINE_SHIFT)<<SST_TACOMBINE_SHIFT;
			tModes[k] = tmode;
			SET(sst->textureMode,tmode);
			break;
		    }
		    case 1:		// tLOD, change all fields but the shape
		    {
			int tmin,tmax,tLOD = tLODs[k];
			gdbg_info(6,"\ttLOD\n");
			tLOD &= ~(SST_LODMIN | SST_LODMAX);

			if(diago.bigAssTextures)			
			  { //for 2048x2048 textures, we're testing LODs 0-11			    
			    tmin = iRandom(iRandom(11<<SST_LOD_FRACBITS));
			    tmax = rRandom(tmin,11<<SST_LOD_FRACBITS);
			  }
			else
			  {  //256x256 textures, we're testing LODs 0-8
			    tmin = iRandom(iRandom(8<<SST_LOD_FRACBITS));
			    tmax = rRandom(tmin,8<<SST_LOD_FRACBITS);
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
			    
			    if(diago.bigAssTextures)
			      {
				SET(sst->texBaseAddr1,textures[k][t].addr[4]>>TEXBASE_SHIFT);
				SET(sst->texBaseAddr2,textures[k][t].addr[5]>>TEXBASE_SHIFT);
				SET(sst->texBaseAddr38,textures[k][t].addr[6]>>TEXBASE_SHIFT);
			      }
			    else
			      {
				SET(sst->texBaseAddr1,textures[k][t].addr[1]>>TEXBASE_SHIFT);
				SET(sst->texBaseAddr2,textures[k][t].addr[2]>>TEXBASE_SHIFT);
				SET(sst->texBaseAddr38,textures[k][t].addr[3]>>TEXBASE_SHIFT);
			      }
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

			if(diago.bigAssTextures)
			  {
			    if (textures[k][t].tiled)
			      SET(sst->texBaseAddr, textures[k][t].addr[3] | SST_TEXTURE_IS_TILED |
				  (textures[k][t].tStride<<SST_TEXTURE_TILESTRIDE_SHIFT));
			    else
			      SET(sst->texBaseAddr,textures[k][t].addr[3]>>TEXBASE_SHIFT);
			  }
			else
			  {
			    if (textures[k][t].tiled)
			      SET(sst->texBaseAddr, textures[k][t].addr[0] | SST_TEXTURE_IS_TILED |
				  (textures[k][t].tStride<<SST_TEXTURE_TILESTRIDE_SHIFT));
			    else
			      SET(sst->texBaseAddr,textures[k][t].addr[0]>>TEXBASE_SHIFT);
			  }
			  
			texs[k] = t;
			// however, the aspect ratio data may have changed
			// so copy the new textures aspect ratio data
			tLOD &= ~(SST_LOD_S_IS_WIDER | SST_LOD_ASPECT);
			tLOD |= textures[k][t].tLOD & (SST_LOD_S_IS_WIDER | SST_LOD_ASPECT);
			tLODs[k] = tLOD;
			SET(sst->tLOD,tLOD);

			if(diago.bigAssTextures)
			  {
			    if (tLODs[k] & SST_TMULTIBASEADDR) {
			      SET(sst->texBaseAddr1,textures[k][t].addr[4]>>TEXBASE_SHIFT);
			      SET(sst->texBaseAddr2,textures[k][t].addr[5]>>TEXBASE_SHIFT);
			      SET(sst->texBaseAddr38,textures[k][t].addr[6]>>TEXBASE_SHIFT);
			    }
			  }
			else
			  {
			    if (tLODs[k] & SST_TMULTIBASEADDR) {
			      SET(sst->texBaseAddr1,textures[k][t].addr[1]>>TEXBASE_SHIFT);
			      SET(sst->texBaseAddr2,textures[k][t].addr[2]>>TEXBASE_SHIFT);
			      SET(sst->texBaseAddr38,textures[k][t].addr[3]>>TEXBASE_SHIFT);
			    }
			  }

			break;
		    }
		    // NOTE: we can't change the multi-base addresses to a different
		    // aspect texture, so we hack it, picking mipmap <= 'n'
		    case 4:		// texBaseAddr1
		    {
			int t = texs[k];
			gdbg_info(6,"\ttexBaseAddr1\n");

			if(diago.bigAssTextures)
			  SET(sst->texBaseAddr1,textures[k][t].addr[iRandom(1)+3]>>TEXBASE_SHIFT);
			else
			  SET(sst->texBaseAddr1,textures[k][t].addr[iRandom(1)]>>TEXBASE_SHIFT);
			break;
		    
		    }
		    case 5:		// texBaseAddr2
		    {
			int t = texs[k];
			gdbg_info(6,"\ttexBaseAddr2\n");
			if(diago.bigAssTextures)
			  SET(sst->texBaseAddr2,textures[k][t].addr[iRandom(2)+3]>>TEXBASE_SHIFT);
			else
			  SET(sst->texBaseAddr2,textures[k][t].addr[iRandom(2)]>>TEXBASE_SHIFT);
			break;
		    }
		    case 6:		// texBaseAddr38
		    {			// cannot change this one
			int t = texs[k];
			gdbg_info(6,"\ttexBaseAddr38\n");
			if(diago.bigAssTextures)
			  SET(sst->texBaseAddr38,textures[k][t].addr[6]>>TEXBASE_SHIFT);
			else
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
			tex->tiled = textures[k][t].tiled;
			tex->tStride = textures[k][t].tStride;
			texRandomTextureMapEx(base_sst,k,
					      ar,slog,tlog,tex,textures[k][t].addr[lod]);
			// reset the texture base address and tLOD registers in this chip

			if(diago.bigAssTextures)
			  { //2048x2048 textures
			    if (textures[k][texs[k]].tiled)
			      SET(sst->texBaseAddr, textures[k][texs[k]].addr[3] | SST_TEXTURE_IS_TILED |
				  (textures[k][texs[k]].tStride<<SST_TEXTURE_TILESTRIDE_SHIFT));
			    else
			      SET(sst->texBaseAddr,textures[k][texs[k]].addr[3]>>TEXBASE_SHIFT);
			  }
			else
			  { //256x256 textures
			    if (textures[k][texs[k]].tiled)
			      SET(sst->texBaseAddr, textures[k][texs[k]].addr[0] | SST_TEXTURE_IS_TILED |
				  (textures[k][texs[k]].tStride<<SST_TEXTURE_TILESTRIDE_SHIFT));
			    else
			      SET(sst->texBaseAddr,textures[k][texs[k]].addr[0]>>TEXBASE_SHIFT);
			  }


			SET(sst->tLOD,tLODs[k]);
			// reset all the tmodes in this chip and downstream
			// as they are trashed by texRandomTextureMapEx()
			while (k >= 0) {
			    SET(SST_TREX(base_sst,k)->textureMode,tModes[k]);
			    k--;
			}
			break;
		    }
		    
		    /**********************************************************************/
		    /****      Randomly toggle between 1 and 2 pixels per clock       *****/
		    /**********************************************************************/		    
                    case 8:
		      {
			FxU32 selection;
			
			selection=diago.pixelsPerClock;
			if(selection != 1 && selection != 2)
			  selection=iRandom(1) + 1;

			if(csimActiveTMUs(diago.sstCSIM) > 1)
			  selection=1;

			break;

			switch(selection)
			  {
			  case 1:  //1 pixel per clock
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
			    if(iRandom(1))
			      {
				SET_1(base_sst->textureMode, shadowRegisters3D[0][1].textureMode);
				SET_1(base_sst->tDetail, shadowRegisters3D[0][1].tDetail);
				SET_1(base_sst->tLOD, shadowRegisters3D[0][1].tLOD);
				SET_1(base_sst->texBaseAddr, shadowRegisters3D[0][1].texBaseAddr);
				SET_1(base_sst->texBaseAddr1, shadowRegisters3D[0][1].texBaseAddr1);
				SET_1(base_sst->texBaseAddr2, shadowRegisters3D[0][1].texBaseAddr2);
				SET_1(base_sst->texBaseAddr38, shadowRegisters3D[0][1].texBaseAddr38);
				SET_1(base_sst->combineMode, shadowRegisters3D[0][1].combineMode);
				SET_1(base_sst->chromaKey, shadowRegisters3D[0][1].chromaKey);
				SET_1(base_sst->chromaRange, shadowRegisters3D[0][1].chromaRange);
			      }
			    else
			      {
				SET_0(base_sst->textureMode, shadowRegisters3D[0][2].textureMode);
				SET_0(base_sst->tDetail, shadowRegisters3D[0][2].tDetail);
				SET_0(base_sst->tLOD, shadowRegisters3D[0][2].tLOD);
				SET_0(base_sst->texBaseAddr, shadowRegisters3D[0][2].texBaseAddr);
				SET_0(base_sst->texBaseAddr1, shadowRegisters3D[0][2].texBaseAddr1);
				SET_0(base_sst->texBaseAddr2, shadowRegisters3D[0][2].texBaseAddr2);
				SET_0(base_sst->texBaseAddr38, shadowRegisters3D[0][2].texBaseAddr38);
				SET_0(base_sst->combineMode, shadowRegisters3D[0][2].combineMode);
				SET_0(base_sst->chromaKey, shadowRegisters3D[0][2].chromaKey);
				SET_0(base_sst->chromaRange, shadowRegisters3D[0][2].chromaRange);
			      }

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

			    break;			    

			  default:
			    assert(0);
			  }
		      }
		      break;
		}
		break;
	  case FOGCOLOR:

		SET(sst->fogColor,iRandom(0xFFFFFFFF));
		break;
	  case ZACOLOR:
		SET(sst->zaColor,iRandom(0xFFFFFFFF));
		break;
	  case CHROMAKEY:
		SET(sst->chromaKey,iRandom(0x00FFFFFF));
		break;

	  case STIPPLE:
		SET(sst->stipple,iRandom(0xFFFFFFFF));
		break;
	  case C0:
		SET(sst->c0,iRandom(0xFFFFFFFF));
		break;
	  case C1:
		SET(sst->c1,iRandom(0xFFFFFFFF));
		break;

	  default:
		if (toBeSent > 2) {	// keep going
		    break;
		}
		gdbg_info(5,"default case\n");
		switch (iRandom(1)) {
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
			if (!texFormatHasPalette(tModes[k]))
			    if (iRandom(1))
				t->tex->tMode ^= SST_TNCCSELECT;
			texRandomNccTable(sst, t->tex);
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
	}
      }
    }
    DIAG_PASS(0);
}

// TODO: make sure this works with 2 - 3 trex chips

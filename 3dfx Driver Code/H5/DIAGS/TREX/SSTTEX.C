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
** $Date: 10/11/00 8:19:17 PM$
*/

#include <assert.h>

#include "allocate.h"
#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

#define H3  (!defined CVG && !defined SHARK && !defined SST2 && !defined H4)

//---------------------------------------------------------------------------
// random texture utilities and data
//---------------------------------------------------------------------------
static int hw_w_is_neg;			// HACK
static int hw_clampst_on_neg_w;		// HACK

static char *tfmt_str[] = {"RGB332","Y-422","A8","I8",
			   "AI44","P8","P6666","*****",
			   "ARGB8332","AY-8422","RGB565","ARGB1555",
			   "ARGB4444","AI88","AP88","ARGB8888",};

static char *tfmt_str_compressed[] = {"3Dfx-comp", "DXT1", "DXT2/DXT3", "DXT4/DXT5",
				      "INVALID!", "INVALID!", "INVALID!", "INVALID!",
				      "INVALID!", "INVALID!", "INVALID!", "INVALID!",
				      "INVALID!", "INVALID!", "INVALID!", "INVALID!"};

//----------------------------------------------------------------------
// process complete chroma test, key or range with all options
// return 1 if chroma test passes (pixel should be discarded)
//----------------------------------------------------------------------
static int chromaTest(FxU32 r, FxU32 g, FxU32 b, FxU32 key, FxU32 range)
{
    if (range & SST_ENCHROMARANGE) {
	FxU32 rlo, rhi, glo, ghi, blo, bhi;	// chromalimits
	FxU32 rpass, gpass, bpass;
            
	blo = (key        ) & 0xFF;
	glo = (key >>    8) & 0xFF;
	rlo = (key >>   16) & 0xFF;
    
	bhi = (range      ) & 0xFF;
	ghi = (range >>  8) & 0xFF;
	rhi = (range >> 16) & 0xFF;

	// Assume inclusive mode.
	rpass = (r >= rlo) && (r <= rhi);
	gpass = (g >= glo) && (g <= ghi);
	bpass = (b >= blo) && (b <= bhi);

	// Invert if exclusive mode.
	if (range & SST_CHROMARANGE_RED_EX  ) rpass = !rpass;
	if (range & SST_CHROMARANGE_GREEN_EX) gpass = !gpass;
	if (range & SST_CHROMARANGE_BLUE_EX ) bpass = !bpass;

	// Check for union or intersection modes.
	if (range & SST_CHROMARANGE_BLOCK_OR)
	    return rpass || gpass || bpass;	// UNION mode.
    	else
	    return rpass && gpass && bpass;	// INTERSECTION mode.
     }
     else {	// simple chromaKey test!!!
	return ((r<<16) | (g<<8) | (b)) == key;
    } 
}

//----------------------------------------------------------------------
// randomly enables alphablending to stress flow control
//----------------------------------------------------------------------
int texRandomZAbuffer(SstRegs *sst, unsigned long texMode)
{
    if (!diago.randomZA) return 0;
    if (!diago.hasAuxBuffer) return 0;
#if 0
	// GMT: for now always enable it to really stress things
    if (iRandom(1)) {
	SET(sst->alphaMode, 0);
	gdbg_info(3,"alphablending = 0\n");
	return 0;
    }
#endif
    SET(sst->alphaMode, SST_ENALPHABLEND |
	(SST_A_ONE<<SST_RGBSRCFACT_SHIFT) | (SST_A_ZERO<<SST_RGBDSTFACT_SHIFT) |
	(SST_A_ONE<<SST_ASRCFACT_SHIFT) | (SST_A_ZERO<<SST_ADSTFACT_SHIFT));
    gdbg_info(3,"alphablending = 1\n");
    return 1;
}

//----------------------------------------------------------------------
// return whether a texture format has an alpha in it or not
//----------------------------------------------------------------------
int texFormatHasAlpha(unsigned long texMode)
{
  if(texMode & SST_COMPRESSED_TEXTURES)
    {
      switch(texMode & SST_TFORMAT)
	{
	case SST_3DFX_COMPRESSED:
	case SST_DXT1:
	  return(0);

	case SST_DXT2:
	case SST_DXT4:
	  return(1);

	default:
	  assert(0);
	}
    }
  else
    {
      switch(texMode & SST_TFORMAT) 
	{
	case SST_RGB332:
	case SST_YIQ422:
	case SST_I8:
	case SST_RGB565:
	case SST_P8:
	  return 0;
	case SST_A8:
	case SST_AI44:
	case SST_ARGB8332:
	case SST_AYIQ8422:
	case SST_ARGB1555:
	case SST_ARGB4444:
	case SST_AI88:
	case SST_AP88:
	case SST_P8_ARGB6666:
        case SST_ARGB8888:
	  return 1;
	default:
	  assert(0);
	}
    }

    return(0);
}

// return whether a texture format is palettized or not
int texFormatHasPalette(unsigned long texMode)
{
    texMode &= SST_TFORMAT;
    return (texMode == SST_P8 || texMode == SST_AP88 || texMode == SST_P8_ARGB6666);
}

//----------------------------------------------------------------------
// return a random texture format
//----------------------------------------------------------------------
int texRandomFormat(int t16, int t32, int ncc)
{
    int format;

    if(diago.compressedTextures)
      {
	switch(iRandom(3))
	  {
	  case 0:
	    format = SST_3DFX_COMPRESSED | SST_COMPRESSED_TEXTURES;
	    break;
	  case 1:
	    format = SST_DXT1 | SST_COMPRESSED_TEXTURES;
	    break;
	  case 2:
	    format = SST_DXT2 | SST_COMPRESSED_TEXTURES;
	    break;
	  case 3:
	    format = SST_DXT4 | SST_COMPRESSED_TEXTURES;
	    break;
	  }
      }
    else if (t32)
      {
	format = SST_ARGB8888;
      }
    else if (t16) {				// 16 bit
	if (ncc) {			//	NCC or palette
	    format = iRandom(1) ? SST_AYIQ8422 : SST_AP88;
	}
	else
	do {
	    format = SST_ARGB8332 + (iRandom(5)<<SST_TFORMAT_SHIFT);
	} while (format==SST_AYIQ8422);
    }
    else {				// 8 bit
	if (ncc) {			//	NCC or palette
	    static FxU32 _fmt8[] = {SST_YIQ422, SST_P8, SST_P8_ARGB6666};
	    format = _fmt8[iRandom(2)];
	}
	else
	do {
	    format = iRandom(4)<<SST_TFORMAT_SHIFT;
	} while (format==SST_YIQ422);
    }

    if (ncc && !texFormatHasPalette(format))
	if (iRandom(1)) format |= SST_TNCCSELECT;

    return format;
}

//----------------------------------------------------------------------
// create and download a random NCC table or 8-bit LUT palette, 
// sst points to TREX chip
//----------------------------------------------------------------------
void texRandomNccTable(SstRegs *sst, Texture *tx)
{
    int i;
    volatile unsigned long *nTab;

    // if PALETTE texture
    if (texFormatHasPalette(tx->tMode)) {
	for (i=0; i<256; i++)			// create random palette
	    tx->palette[i] = 0x80000000 | iRandom(0xFFFFFF);

	nTab = sst->nccTable0;			// point to first I[] slot
	nTab += 4;
	for (i=0; i<256; i+=2)	{		// download palette
	    SET(nTab[0+(i&7)], ((i>>1)<<24) | tx->palette[i]);
	    SET(nTab[1+(i&7)], ((i>>1)<<24) | tx->palette[i+1]);
	}
	return;
    }

    // else NCC table
    for (i=0; i<16; i++) {			// random intensity table
	tx->ncc.yRGB[i] = iRandom(255);
    }
    for (i=0; i<4; i++) {
	tx->ncc.iRGB[i][0] = rRandom(-255,255);	// random iRGB table
	tx->ncc.iRGB[i][1] = rRandom(-255,255);
	tx->ncc.iRGB[i][2] = rRandom(-255,255);
	tx->ncc.qRGB[i][0] = rRandom(-255,255);	// random qRGB table
	tx->ncc.qRGB[i][1] = rRandom(-255,255);
	tx->ncc.qRGB[i][2] = rRandom(-255,255);
    }

    nTab = (tx->tMode & SST_TNCCSELECT) ? sst->nccTable1 : sst->nccTable0;

    for (i=0; i<4; i++) {	// first the Y table
	SET(nTab[i],	(tx->ncc.yRGB[i*4+3]<<24) |
			(tx->ncc.yRGB[i*4+2]<<16) |
			(tx->ncc.yRGB[i*4+1]<<8) |
			 tx->ncc.yRGB[i*4+0] );
    }
    for (i=0; i<4; i++) {	// then pack I,Q tables: R|G|B
	SET(nTab[i+4],	((tx->ncc.iRGB[i][0]&0x1FF)<<18) |
			((tx->ncc.iRGB[i][1]&0x1FF)<<9) |
			(tx->ncc.iRGB[i][2]&0x1FF));
    }
    for (i=0; i<4; i++) {	// then pack I,Q tables: R|G|B
	SET(nTab[i+8],	((tx->ncc.qRGB[i][0]&0x1FF)<<18) |
			((tx->ncc.qRGB[i][1]&0x1FF)<<9) |
			(tx->ncc.qRGB[i][2]&0x1FF));
    }
}

//----------------------------------------------------------------------
// create and download one random mipmap level at a specified address
//----------------------------------------------------------------------
static void
_texRandomMipmap(SstRegs *sst, Texture *tx,
		int ar, int lod, int slog, int tlog)
{
    FxU32 s,t,tformat;
    unsigned int *temp;
    unsigned int   *data32;
    unsigned short *data16;
    unsigned char  *data8; 
    char *formatString;
    FxU32 mipmapWidth, mipmapHeight, mipmapAddress;
    SstRegs *trex;
    int bpt;            // bytes per texel

    mipmapWidth   = 1<<slog;
    mipmapHeight  = 1<<tlog;

    //Make sure that compressed textures are at least their minimum size
    if(SST_T4BIT_COMPRESSED(tx->tMode))
      {
	if(mipmapWidth < 8)
	  mipmapWidth = 8;
	if(mipmapHeight < 4)
	  mipmapHeight = 4;
      }
    else if(SST_T8BIT_COMPRESSED(tx->tMode))
      {
	if(mipmapWidth < 4)
	  mipmapWidth = 4;
	if(mipmapHeight < 4)
	  mipmapHeight = 4;
      }

    mipmapAddress = tx->mip[lod]->mipmapBaseAddress;

    //Allocate temporary space
    temp = malloc(sizeof(int) * mipmapWidth * mipmapHeight);    
    assert(temp != NULL);
    data32 = temp;
    data16 = (FxU16 *)temp;
    data8 = (FxU8 *)temp;
    
    tformat = tx->tMode & SST_TFORMAT;
    if(tx->tMode & SST_COMPRESSED_TEXTURES)
      formatString = tfmt_str_compressed[tformat>>SST_TFORMAT_SHIFT];
    else
      formatString = tfmt_str[tformat>>SST_TFORMAT_SHIFT];

    gdbg_info(2, "random %3dx%-3d ar=%d %s texture loaded at 0x%x into TREX %d\n",
		mipmapWidth, mipmapHeight, ar,
	        formatString, 
		mipmapAddress,tx->trex);
    trex = SST_TREX(sst,tx->trex);		// get pointer to TREX
    SET(trex->textureMode,tx->tMode);		// set textureMode before download
    SET(SST_TREX(sst,tx->trex)->combineMode,tx->combMode);

    //Calculate the number of bytes per texel
    if(SST_T4BIT_COMPRESSED(tx->tMode))
      bpt=0;
    if(SST_T8BIT_COMPRESSED(tx->tMode))
      bpt=1;
    if(SST_T8BIT(tx->tMode))
      bpt=1;
    else if(SST_T16BIT(tx->tMode))
      bpt=2;
    else if(SST_T32BIT(tx->tMode))
      bpt=4;

    // if chrom-testing enabled, then do very special stuff
    if ((tx->tChromarange & SST_ENCHROMAKEY_TMU) &&
	!(tx->combMode & SST_CM_DISABLE_CHROMA_SUBSTITUTION))
    {
	FxU16 pass[2],fail[2],p,f;
	FxU32 csrc,crng;

	if (slog != 1) GDBG_ERROR("_texRandomMipmap","slog != 1\n");
	if (tlog != 1) GDBG_ERROR("_texRandomMipmap","tlog != 1\n");
	// take the 1st entry in the palette
	pass[0] = 1;
again:
	csrc = tx->palette[pass[0]] & 0xFFFFFF;
	// and generate a key/range such that csrc passes
	if (tx->tChromarange & SST_ENCHROMARANGE) {
	    int a,b,c, incl;

	    incl = (tx->tChromarange & SST_CHROMARANGE_BLUE_EX) == 0;
	    if (tx->tChromarange & SST_CHROMARANGE_BLOCK_OR) {
		do {
			a = iRandom(1);
			b = iRandom(1);
			c = iRandom(1);
		} while (a==!incl && b==!incl && c==!incl);
	    }
	    else		// all in or out of range
		a=b=c = incl;
	    ckeyRandom888(a,b,c, csrc, &tx->tChromakey,&crng);
	    crng = tx->tChromarange | (crng & 0xFFFFFF);
	}
	else {
	    tx->tChromakey = csrc;
	    crng = tx->tChromarange;
	}
	// now find a palette index that fails
	for (fail[0]=0; fail[0]<256; fail[0]++) {
	    if (!chromaTest((tx->palette[fail[0]]>>16)&0xFF,
			(tx->palette[fail[0]]>>8)&0xFF,
			(tx->palette[fail[0]]>>0)&0xFF,
			tx->tChromakey,crng)) break;
	}
	if (fail[0] > 255) {
	    pass[0]++;
	    goto again;
	}
	// now find another palette index that fails
	for (fail[1]=fail[0]+1; fail[1]<256; fail[1]++) {
	    if (!chromaTest((tx->palette[fail[1]]>>16)&0xFF,
			(tx->palette[fail[1]]>>8)&0xFF,
			(tx->palette[fail[1]]>>0)&0xFF,
			tx->tChromakey,crng)) break;
	}
	if (fail[1] > 255)		// if couldn't find one
	    fail[1] = fail[0];		// just use the one first one we found
	// now find another palette index that passes
	for (pass[1]=0; pass[1]<256; pass[1]++) {
	    if (pass[1] != pass[0])
	    if (chromaTest((tx->palette[pass[1]]>>16)&0xFF,
			(tx->palette[pass[1]]>>8)&0xFF,
			(tx->palette[pass[1]]>>0)&0xFF,
			tx->tChromakey,crng)) break;
	}
	if (pass[1] > 255)		// if couldn't find one
	    pass[1] = pass[0];		// just use the one first one we found

	tx->tChromarange |= crng & 0xFFFFFF;
	gdbg_info(3," chroma fail=%d,%d pass=%d,%d  mask=%x\n",
			fail[0],fail[1],pass[0],pass[1],tx->cmask);

	// now init the texture map to pass/fail pattern to match the mask
	// where the mask matches the one in CSIM for my sanity
	for (t=0; t<mipmapHeight; t++) {
	    for (s=0; s<mipmapWidth; s++) {
	      setMipmapData(tx, lod, s, t, tx->cmask&(8>>(s+t*2)) ? pass[(p++)&1] : fail[(f++)&1]);

	      *data8++ = (FxU8)getMipmapData(tx, lod, s, t);
	    }
	}
    }
    else
      for (t=0; t<mipmapHeight; t++) {	// init to any old random bits
	for (s=0; s<mipmapWidth; s++) {
	  if(SST_T4BIT_COMPRESSED(tx->tMode))
	    {
	      static FxU8 value=0;
	      FxU32 index;
		
	      value = iRandom(0xFF);
		
	      setMipmapData(tx, lod, s, t, value);
	      
	      //Find the index. These textures are stored in 8x4 chunks
	      index = ((s/8) + (t/4)*(mipmapWidth/8))*32;
	      index += (s%8) + ((t%4) * 8);
	      
	      //Put things together little endian style
	      if(index & 1)
		data8[index/2] = (data8[index/2] & 0xF) | ((value & 0xF) << 4);  //Store the ms-nibble
	      else  //store the first nibble
		data8[index/2] = (data8[index/2] & 0xF0) | (value & 0xF);  //Store the ls-nibble
	    }	  	  
	  else if(SST_T8BIT_COMPRESSED(tx->tMode))
	    {
	      static FxU8 value=0;
	      FxU32 index;

	      value = iRandom(0xFF);
	      
	      setMipmapData(tx, lod, s, t, value);
	      
	      //Find the index. These textures are stored in 4x4 chunks
	      index = ((s/4) + (t/4)*(mipmapWidth/4))*16;	      
	      index += (s%4) + ((t%4) * 4);

	      data8[index] = value;
	    }
	  else if (SST_T8BIT(tx->tMode))
	    {
	      FxU8 value;	     
	      value = iRandom(0xFF);

	      setMipmapData(tx, lod, s, t, value);
	      *data8++ = value;
	    }
	  else if(SST_T16BIT(tx->tMode)) {
	    FxU16 value;	     
	    value = iRandom(0xFFFF);

	    setMipmapData(tx, lod, s, t, value);	      
	    *data16++ = value;
	  }
	  else if(SST_T32BIT(tx->tMode))
	    {
	      FxU32 value;	     
	      value = iRandom(0xFFFFFFFF);
		
	      setMipmapData(tx, lod, s, t, value);	      
	      *data32++ = value;
	    }
	  else
	    assert(0);
	}
      }
     
#ifdef ENDB
      {	
	int i, j, n, psize;

	if(tx->tMode & SST_COMPRESSED_TEXTURES)
	  {
	    if(SST_T4BIT_COMPRESSED(tx->tMode))
	      {
		n = mipmapWidth * mipmapHeight / 2;
		if(n < 16)
		  n = 16;

		psize = 4;
	      }
	    else if(SST_T8BIT_COMPRESSED(tx->tMode))
	      {
		n = mipmapWidth * mipmapHeight;
		if(n < 16)
		  n = 16;

		psize = 4;
	      }
	    else
	      assert(0);
	  }
	else
	  {
	    n = mipmapWidth *mipmapHeight * bpt;
	    
	    // determine the packing size for most efficient downloading
	    // NOTE: texture size in bytes is either 1, 2, or a multiple of 4
	    if ( tx->tiled ) 
	      psize = (mipmapWidth * bpt - 1) % 4 + 1;
	    else
	      psize = (n-1) % 4 + 1;
	  }
	
	GDBG_INFO(10,"packing size = %d\n",psize);

	// pack texels  (i.e. swap to little endian)
	data8 = data16 = data32 = temp;
	if ( bpt == 1 && psize == 2 ) {		      	// pack bytes into words
	  for (i=0, j=0; i<n; i+=2)
	    data16[j++] = data8[i] | (data8[i+1]<<8);
	} else if ( bpt == 1 && psize == 4 ) {		// pack bytes into dwords
	  for (i=0, j=0; i<n; i+=4)
	    data32[j++] = data8[i] | (data8[i+1]<<8) | (data8[i+2]<<16) | (data8[i+3]<<24);
	} else if ( bpt == 0 && psize == 4 ) {
	  for (i=0, j=0; i<n; i+=4)
	    data32[j++] = data8[i] | (data8[i+1]<<8) | (data8[i+2]<<16) | (data8[i+3]<<24);	  
	} else if ( bpt == 2 && psize == 4 ) {		// pack words into dwords
	  for (i=0, j=0; i<n/2; i+=2) 
	    data32[j++] = data16[i] | (data16[i+1]<<16);	  
	} 
      }
#endif

    // textureMode and tLOD must be set before download
    if ( !tx->tiled && (tx->tLOD & SST_LOD_TSPLIT)) t = ~t;	// HACK: complement addr for TSPLIT case

    sstDownLoadTexture(sst,tx->trex,   		// NOTE: this sets texBaseAddr
		       mipmapAddress,
		       tx->tiled,tx->tStride,
		       tx->tMode,ar,slog,tlog,
		       bpt, tx->tLOD,
		       (unsigned long *)temp);

    //Remember to free up temp
    free(temp);
}

//----------------------------------------------------------------------
// create and download a random texture (including all its mipmaps)
// into a TREX chip, setting textureMode and tLOD in the process
// note that we don't really care what format it is,
// except if its compressed in which case we download a random table
//----------------------------------------------------------------------
void texRandomTextureMap(SstRegs *sst, int trex, int mipmaps,
			 int slog, int tlog, Texture *tx)
{
  texRandomTextureMapEx(sst, trex, mipmaps, slog, tlog, tx, -1);
}

#ifdef CVG
# define TEXBASE_SHIFT	3
# define TEXBASE_ALIGN	7
#else
# define TEXBASE_SHIFT	0
# define TEXBASE_ALIGN	15
#endif

// Same as above, but if addrHint >=0 then it uses that as the base address
// Note that for linear textures, addr point to the beginning of the current lod.
// For tiled textures, addr always points to the beginning of lod 0, i.e., 
// the base address that gets written to the base address register.
//
// if addrHint < 0, 
//    then linear/tiled is selected based on diago.ytiled
//    addr is chosen randomly
//    if tiled, tile stride (tx->tStride) is chosen randomly
//
// if addrHint >= 0
//    then tx->tile (set by the caller) determines linear/tiled
//    addr = addrHint
//    if tiled, tile stride (tx->tStride) must be set by the caller
void texRandomTextureMapEx(SstRegs *sst, int trex, int mipmaps,
			 int slog, int tlog, Texture *tx, int addrHint)
{
    int ar,lod,t,addr,s_is_wider;
    int minPaddr=0x7fffffff;
    int uSlop=0, vSlop=0;
    int bitsPerTexel;
    int odd = tx->tLOD & SST_LOD_ODD ? 1 : 0;
    int tsplit = (tx->tLOD & SST_LOD_TSPLIT) ? 1 : 0;
    tiledStruct mipmap;
    int umin, vmin, umax, vmax, amin, amax, lodmin, lodmax;
    FxI32 base;
    FxU32 largestLOD;
    FxBool compressedTexture;
    
    //Always run in 1 pixel per clock when generating textures
    SET_FBI(diago.sst->combineMode, shadowRegisters3D[0][0].combineMode & ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
    SET_0(diago.sst->combineMode, shadowRegisters3D[0][1].combineMode & ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
    SET_1(diago.sst->combineMode, shadowRegisters3D[0][2].combineMode & ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);

    if(SST_T8BIT(tx->tMode))
      {
	compressedTexture = FXFALSE;
	bitsPerTexel = 8;
      }
    else if(SST_T16BIT(tx->tMode))
      {
	compressedTexture = FXFALSE;
	bitsPerTexel = 16;
      }
    else if(SST_T32BIT(tx->tMode))
      {
	compressedTexture = FXFALSE;
	bitsPerTexel = 32;
      }
    else if(SST_T4BIT_COMPRESSED(tx->tMode))
      {
	compressedTexture = FXTRUE;
	bitsPerTexel = 4;
      }
    else if(SST_T8BIT_COMPRESSED(tx->tMode))
      {
	compressedTexture = FXTRUE;
	bitsPerTexel = 8;
      }

    //Figure out what the largest possible LOD is
    if(diago.bigAssTextures)
      {
	largestLOD = 11;
      }
    else
      largestLOD = 8;

    if (trex < 0)
      trex = iRandom(-trex);
    tx->trex = trex;			// save it away
    
    gdbg_info(2, "texRandomTextureMap: s,tlog = %d %d\n",slog,tlog);
    ar = slog - tlog;			// compute aspect ratio before clamp
    if (slog < 0) slog = 0;
    if (tlog < 0) tlog = 0;

    if (ar > 0) {			// compute minimum LOD number
	lod = largestLOD-slog;				//s is wider (or square)
	s_is_wider = 1;
    } else {					// t is wider
	lod = largestLOD-tlog;
	s_is_wider = 0;
	ar = -ar;
    }

    if ( addrHint < 0 ) {
      tx->tiled = diago.ytiled;
      if ( tx->tiled < 0 )
	tx->tiled = iRandom(1);
    }

    tsplit = (tx->tLOD & SST_LOD_TSPLIT) ? 1 : 0;
    if ( tx->tiled )
      tsplit = 0;   // tsplit is ignored for tiled textures
	
    // have to set all the downstream TREX chips to pass-thru mode
    for (t=0; t<trex; t++)
      SET(SST_TREX(sst,t)->textureMode,SST_TC_PASS | SST_TCA_PASS);
    
    // compute MAX lod and then set tLOD register
    tx->lodmin = tx->lodmax = lod<<SST_LOD_FRACBITS;
    if (mipmaps) tx->lodmax = largestLOD<<SST_LOD_FRACBITS;
    tx->tLOD &= SST_LOD_TSPLIT | SST_LOD_ODD | SST_TBIG;	// clear all bits but TSPLIT, ODD and TBIG

    tx->tLOD |= (tx->lodmin<<SST_LODMIN_SHIFT) |
			(tx->lodmax<<SST_LODMAX_SHIFT) |
			((s_is_wider) ? SST_LOD_S_IS_WIDER : 0) |
			(ar << SST_LOD_ASPECT_SHIFT);

    //Set the SST_TBIG bit if --bigAssTextures
    if(diago.bigAssTextures)
      tx->tLOD |= SST_TBIG;
    else
      assert((tx->tLOD & SST_TBIG) == 0);

    SET(SST_TREX(sst,trex)->tLOD,tx->tLOD);
    if ((tx->tMode & SST_TFORMAT) == SST_YIQ422 ||
	(tx->tMode & SST_TFORMAT) == SST_AYIQ8422 ||
	texFormatHasPalette(tx->tMode))
      {	
	if(isTwoPixelsPerClockInhibited())
	  texRandomNccTable(SST_TREX(sst,trex),tx);
	else //If not multitexturing, copy palette/NCC table to both TMUs	  
	  texRandomNccTable(sst,tx);
      }

    // allocate space for the texture
    if ( tx->tiled ) {                          
      ////////////////////////////////////////////////////////////////////////
      //
      //                           tiled texture      
      //
      ////////////////////////////////////////////////////////////////////////
      FxU32 textureSize;
      static FxU32 tiledTextureCount=0;
      FxI32 firstTileU, firstTileV, firstTileIndex;
      FxI32 lastTileU,  lastTileV, lastTileIndex;
      char textureName[32];
      PlacementStrategy placement;
      
      //Pick a tile stride for the texture
      if(tx->tMode & SST_COMPRESSED_TEXTURES)
	{
	  if(SST_T4BIT_COMPRESSED(tx->tMode))
	    {
	      //4bpt compressed textures in 8x4 blocks
	      
	      if(tx->tLOD & SST_TBIG)
		tx->tStride = (3072 / (SST_TILE_WIDTH / 2)) + 1 + iRandom(1);
	      else
		tx->tStride = (384 / (SST_TILE_WIDTH / 2)) + 1 + iRandom(1);
	    }
	  else if(SST_T8BIT_COMPRESSED(tx->tMode))
	    {
	      //8bpt compressed textures in 4x4 blocks

	      if(tx->tLOD & SST_TBIG)
		tx->tStride = (3072 / (SST_TILE_WIDTH / 4)) + 1 + iRandom(1);
	      else
		tx->tStride = (384 / (SST_TILE_WIDTH / 4)) + 1 + iRandom(1);
	    }
	  else
	    assert(0);
	}
      else
	{
	  if(tx->tLOD & SST_TBIG)	
	    tx->tStride = (3072 / (SST_TILE_WIDTH * 8 / bitsPerTexel)) + 1 + iRandom(1);
	  else
	    tx->tStride = (384 / (SST_TILE_WIDTH * 8 / bitsPerTexel)) + 1 + iRandom(1);
	}

      // get bounding box of texture/mipmap
      mipmap = sstTiledMipMapOffset2(lod, tx->tLOD, tx->tMode);
      umin = mipmap.uoff;
      vmin = mipmap.voff;

      if ( mipmaps ) {
	umax = umin + mipmap.utot - 1;
	vmax = vmin + mipmap.vtot - 1;
      } else {
	umax = umin + (1<<slog) - 1;
	vmax = vmin + (1<<tlog) - 1;
      }

      //Calculate which tiles need to be allocated for texture
      if(tx->tMode & SST_COMPRESSED_TEXTURES)
	{
	  if(SST_T4BIT_COMPRESSED(tx->tMode))
	    { //4bpt compressed textures in 8x4 blocks
	      firstTileU = roundDownDivide(umin, SST_TILE_WIDTH / 2);
	      firstTileV = roundDownDivide(vmin, SST_TILE_HEIGHT * 4);
	      firstTileIndex = firstTileV * tx->tStride + firstTileU;
	      lastTileU  = roundDownDivide(umax, SST_TILE_WIDTH / 2) + 1;
	      lastTileV = roundDownDivide(vmax, SST_TILE_HEIGHT * 4) + 1;
	      lastTileIndex = lastTileV * tx->tStride + lastTileU;	      
	    }
	  else if(SST_T8BIT_COMPRESSED(tx->tMode))
	    { //8bpt compressed textures in 4x4 blocks
	      firstTileU = roundDownDivide(umin, SST_TILE_WIDTH / 4);
	      firstTileV = roundDownDivide(vmin, SST_TILE_HEIGHT * 4);
	      firstTileIndex = firstTileV * tx->tStride + firstTileU;
	      lastTileU  = roundDownDivide(umax, SST_TILE_WIDTH / 4) + 1;
	      lastTileV = roundDownDivide(vmax, SST_TILE_HEIGHT * 4) + 1;
	      lastTileIndex = lastTileV * tx->tStride + lastTileU;      
	    }
	  else
	    assert(0);	  
	}
      else
	{
	  firstTileU = roundDownDivide(umin, SST_TILE_WIDTH * 8 / bitsPerTexel);
	  firstTileV = roundDownDivide(vmin, SST_TILE_HEIGHT);
	  firstTileIndex = firstTileV * tx->tStride + firstTileU;
	  lastTileU  = roundDownDivide(umax, (SST_TILE_WIDTH * 8 / bitsPerTexel)) + 1;
	  lastTileV = roundDownDivide(vmax, SST_TILE_HEIGHT) + 1;
	  lastTileIndex = lastTileV * tx->tStride + lastTileU;
	}
      assert(lastTileIndex >= firstTileIndex);
            
      //Calculate the size of the texture
      textureSize = SST_TILE_SIZE * (lastTileIndex - firstTileIndex + 1);

      //Set up for texture allocation
      sprintf(textureName, "tTexRandom %d", tiledTextureCount++);

      if(diago.randomPlacement || addrHint < 0)
	placement = randomPlacement;
      else
	placement = normalPlacement;

      //Allocate a chunk of frame buffer memory for the texture with slop
      if(!allocateWorker(&addr, textureSize, textureName, placement, FXFALSE))
	{ //Ran out of texture memory	  
	  GDBG_INFO(1, "Warning! Ran out of texture memory! Unallocating textures %s(%d)\n",
		    __FILE__, __LINE__);
	  
	  //Unallocate everything that isn't locked
	  unallocateAll();
	  
	  //Try to reallocate; this time die on failure.
	  addr=allocate(textureSize, textureName, placement);
	}
      memoryMap();
      
      //Write the base address down
      if(SST_T4BIT_COMPRESSED(tx->tMode))
	{ //4bpt compressed textures in 8x4 blocks
	  base = tiledAddress(addr, tx->tStride, 16, roundDownDivide(-umin, 8), roundDownDivide(-vmin, 4));
	}
      else if(SST_T8BIT_COMPRESSED(tx->tMode))
	{ //8bpt compressed textures in 4x4 blocks
	  base = tiledAddress(addr, tx->tStride, 16, roundDownDivide(-umin, 4), roundDownDivide(-vmin, 4));
	}
      else
	{ //non-compressed texture
	  base = tiledAddress(addr, tx->tStride, bitsPerTexel/8, -umin, -vmin);
	}

      //Make sure that base if aligned by 16
      base = (base + 15) & (~15);
      base = base & SST_TEXTURE_FULL_ADDRESS;

      GDBG_INFO(170,"texRandomTextureMap(tmu #%d): addr=0x%x, stride=%d (Tiled)\n",trex,addr,tx->tStride);
    } else {                          
      ////////////////////////////////////////////////////////////////////////
      //
      //                           linear texture      
      //
      ////////////////////////////////////////////////////////////////////////
      static FxU32 linearTextureCount=0;
      FxU32 textureSize;
      char textureName[32];
      PlacementStrategy placement;

      // find lowest/highest lod's to be downloaded
      if ( tsplit ) 
	{
	  if(tx->tLOD & SST_TBIG) //2048x2048 case	    
	    lodmax = lodmin = odd ? CEIL(lod,2)*2 : FLOOR(lod,2)*2+1;
	  else //256x256 case
	    lodmax = lodmin = odd ? FLOOR(lod,2)*2+1 : CEIL(lod,2)*2;
	}
      else
	lodmax = lodmin = lod;

      if ( mipmaps )
	{
	  if(tx->tLOD & SST_TBIG) //2048x2048 case	    
	    lodmax = ((!tsplit) || (tsplit && !odd)) ? largestLOD : (largestLOD-1);
	  else  //256x256 case
	    lodmax = (tsplit && odd) ? (largestLOD - 1) : largestLOD;
	}
      
      // find min/max memory offsets from base
      amin = sstLinearMipMapOffset2(lodmin, tx->tLOD, tx->tMode);
      amax = sstLinearMipMapOffset2(lodmax, tx->tLOD, tx->tMode) +
	sstLinearMipMapSize2(lodmax, tx->tLOD, tx->tMode);

      sprintf(textureName, "lTexRandom %d", linearTextureCount++);

      if(diago.randomPlacement || addrHint < 0)
	placement = randomPlacement;
      else
	placement = normalPlacement;

      //Allocate a chunk of frame buffer memory for the texture with slop
      textureSize = amax-amin + 64;
      if(!allocateWorker(&addr, textureSize, textureName, placement, FXFALSE))
	{ //Ran out of texture memory	  
	  GDBG_INFO(1, "Warning! Ran out of texture memory! Unallocating textures %s(%d)\n",
		    __FILE__, __LINE__);
	  
	  //Unallocate everything that isn't locked
	  unallocateAll();
	  
	  //Try to reallocate; this time die on failure.
	  addr=allocate(textureSize, textureName, placement);
	}
      memoryMap();

      //Calculate the base and 16 byte align it
      base = addr - amin;      
      base = (base + 15) & 0xFFFFFFF0;
      base = base & SST_TEXTURE_FULL_ADDRESS;

      //Recalculate the texture address based on the base
      addr = base + amin;
      addr = addr & SST_TEXTURE_FULL_ADDRESS;

      GDBG_INFO(170,"texRandomTextureMap(tmu #%d): addr=0x%x (Linear)\n",trex,addr);
      GDBG_INFO(170,"  base=0x%x  amin=0x%x  amax=0x%x\n",base,amin,amax);
    }

    // now create and download each mipmap level until done
    slog++; tlog++; lod--;			// prime the loop
    do {
	lod++;
	if (slog > 0) slog--;
	if (tlog > 0) tlog--;

	tx->mip[lod]->textureBaseAddress = base;
	
	//Calculate the base of the mipmap
	if(tx->tiled)
	  {
	    mipmap = sstTiledMipMapOffset2(lod, tx->tLOD, tx->tMode);
	    umin = mipmap.uoff;
	    vmin = mipmap.voff;
	    
	    if(tx->tMode & SST_COMPRESSED_TEXTURES)
	      { 
		convertToMicroTile(tx->tMode, &umin, &vmin);

		tx->mip[lod]->mipmapBaseAddress = tiledAddress(base, tx->tStride, 16, umin, vmin);
	      }
	    else
	      { //non-compressed texture
		tx->mip[lod]->mipmapBaseAddress = tiledAddress(base, tx->tStride, bitsPerTexel/8, umin, vmin);
	      }	    	    
	  }
	else	  
	  tx->mip[lod]->mipmapBaseAddress = base + sstLinearMipMapOffset2(lod, tx->tLOD, tx->tMode);

	tx->mip[lod]->mipmapBaseAddress &= SST_TEXTURE_FULL_ADDRESS;

	tx->mip[lod]->lod = lod;
	tx->mip[lod]->width = 1<<slog;		// store data in texture
	tx->mip[lod]->height = 1<<tlog;		
	assert(tx->mip[lod]->width * tx->mip[lod]->height <= tx->mip[lod]->nData);

	GDBG_INFO(170,"texRandomTextureMap: lod=%d width=%d height=%d \n\t\ttextureBaseAddress = 0x%x  mipBaseAddress = 0x%x\n",
		  lod, tx->mip[lod]->width, tx->mip[lod]->height, tx->mip[lod]->textureBaseAddress,
		  tx->mip[lod]->mipmapBaseAddress);
	GDBG_INFO(170,"tx->tStride = %d\n", tx->tStride);

	// if TSPLIT and odd/even mismatch, then skip over texture downlaod
	if(tx->tLOD & SST_TBIG) //2048x2048 case	    
	  {
	    if ( tsplit && (odd == (lod&1)) ) {
	      GDBG_INFO(170, "This split texture lod is going to be skipped\n");
	      continue;
	    }
	  }
	else //256x256 case
	  {
	    if ( tsplit && (odd != (lod&1)) ) {
	      GDBG_INFO(170, "This split texture lod is going to be skipped\n");
	      continue;
	    }
	  }

	_texRandomMipmap(sst,tx,ar,lod,slog,tlog);
	  
    } while (mipmaps && (slog || tlog));

    // if multiple base addresses are enabled, then reset things in TREX
    // since texBaseAddr is used for all downloads
    if(diago.multiTexBaseAddr)
      {
	FxU32 texBaseAddrTemp;
		
	tx->tLOD |= SST_TMULTIBASEADDR;
	SET(SST_TREX(sst,trex)->tLOD,tx->tLOD);

	if(tx->tiled)
	  texBaseAddrTemp = SST_TEXTURE_IS_TILED | (tx->tStride<<SST_TEXTURE_TILESTRIDE_SHIFT);
	else
	  texBaseAddrTemp = 0;
	
	if(lod >= 0)
	  SET(SST_TREX(sst,trex)->texBaseAddr, 
	      SST_TEXTURE_MUNGE_ADDRESS(tx->mip[0]->textureBaseAddress) | texBaseAddrTemp);
	else
	  SET(SST_TREX(sst,trex)->texBaseAddr, 
	      SST_TEXTURE_MUNGE_ADDRESS(tx->mip[lod]->textureBaseAddress) | texBaseAddrTemp);	      
	
	if(lod >= 1)
	  SET(SST_TREX(sst,trex)->texBaseAddr1,tx->mip[1]->textureBaseAddress);
	else
	  SET(SST_TREX(sst,trex)->texBaseAddr1,tx->mip[lod]->textureBaseAddress);
	
	if(lod >= 2)
	  SET(SST_TREX(sst,trex)->texBaseAddr2,tx->mip[2]->textureBaseAddress);
	else
	  SET(SST_TREX(sst,trex)->texBaseAddr2,tx->mip[lod]->textureBaseAddress);
	
	if(lod > 3)
	  SET(SST_TREX(sst,trex)->texBaseAddr38,tx->mip[lod]->textureBaseAddress);
	else
	  SET(SST_TREX(sst,trex)->texBaseAddr38,tx->mip[3]->textureBaseAddress);
      }
    
    // if chrom-testing enabled, then send chromakey data to specific TREX
    if ((tx->tChromarange & SST_ENCHROMAKEY_TMU) &&
	!(tx->combMode & SST_CM_DISABLE_CHROMA_SUBSTITUTION))
    {
	SET(SST_TREX(sst,trex)->chromaKey,tx->tChromakey);
	SET(SST_TREX(sst,trex)->chromaRange,tx->tChromarange);
    }

    //Flush the texture cache
    GDBG_INFO(186, "Flushing texture cache\n");
    SET_0(sst->texBaseAddr, ~shadowRegisters3D[0][1].texBaseAddr);
    SET_0(sst->texBaseAddr, ~shadowRegisters3D[0][1].texBaseAddr);
    SET_1(sst->texBaseAddr, ~shadowRegisters3D[0][2].texBaseAddr);
    SET_1(sst->texBaseAddr, ~shadowRegisters3D[0][2].texBaseAddr);
    
    texturingSetPixelsPerClock(sst, trex);
}

//This is here for backwards compatibility
void texRandomTextureMapNoOverlap(SstRegs *sst, int trex, int mipmaps,
				  int slog, int tlog, Texture *tex,
				  FxU32 *start, FxU32 *end)
{
  assert(start != NULL);
  assert(end != NULL);

  texRandomTextureMapEx(sst, trex, mipmaps, slog, tlog, tex, 0);
}

//----------------------------------------------------------------------
// bilinearly blend four 8-bit colors into one color
//----------------------------------------------------------------------
static int
bilinearFilter(unsigned int ufrac, unsigned int vfrac,
	    unsigned char A, unsigned char B,
	    unsigned char C, unsigned char D)
{
    int e,f,res;

    e = (vfrac * (C - A)) + (A << 8);
    f = (vfrac * (D - B)) + (B << 8);	// e,f are 8.8
    // if tmu rev > 3, use old full precision code
    if (CSIM_PRIVATE(diago.sstCSIM)->info->tmuRevision > 3) 
      res = ((((ufrac * ((f - e)>>0)) + (e << 8)) + 0x8000) >> 16) & 0xff;
    else
      res = ((((ufrac * ((f - e + 0x40)>>7)) + (e << 1)) + 0x100) >> 9) & 0xff;
    gdbg_info(8,"\tdiag biblend: .%x,.%x %02x %02x %02x %02x = %04x %04x = %-2x\n",
		ufrac,vfrac, A,B,C,D, e,f,res);
    return res;
}

//----------------------------------------------------------------------
// perform a texture combine on one channel
//----------------------------------------------------------------------
static FxU8 tcu(int tMode, int tDetail,
		FxU8 other, FxU8 local,
		FxU8 aother, FxU8 otherTextureAlpha, FxU8 alocal, FxU8 localTextureAlpha,
		FxU8 mselect_6, FxU8 mselect_7, FxU8 addlocal_3,
		int invert_other_local,int outshift,
		int raw_lod, int lod_frac, int reversed)
{
    int res,rlocal,dres, beta,inc;

    res = tMode & SST_TC_ZERO_OTHER ? 0 : other;
    switch (invert_other_local & SST_CM_TC_INVERT_OTHER) {
	case SST_CM_TC_INVERT_OTHER_X:
		res = res; break;
	case SST_CM_TC_INVERT_OTHER_ZERO_MINUS_X:
		res = 0x00 - res; break;
	case SST_CM_TC_INVERT_OTHER_ONE_MINUS_X:
		res = 0xFF - res; break;
	case SST_CM_TC_INVERT_OTHER_X_MINUS_HALF:
		res = res - 0x80; break;
    }
    gdbg_info(8,"other=%02x  %02x\n",other,res);
    rlocal = local;
    if (!(tMode & SST_TC_SUB_CLOCAL))
	rlocal = 0;
    switch (invert_other_local & SST_CM_TC_INVERT_LOCAL) {
	case SST_CM_TC_INVERT_LOCAL_X:
		rlocal = rlocal; break;
	case SST_CM_TC_INVERT_LOCAL_ZERO_MINUS_X:
		rlocal = 0x00 - rlocal; break;
	case SST_CM_TC_INVERT_LOCAL_ONE_MINUS_X:
		rlocal = 0xFF - rlocal; break;
	case SST_CM_TC_INVERT_LOCAL_X_MINUS_HALF:
		rlocal = rlocal - 0x80; break;
    }
    gdbg_info(8,"local=%02x %02x\n",local,rlocal);
    res += rlocal;
    inc = 1;
    if (reversed) reversed = 0xFF;
    switch (tMode & SST_TC_MSELECT) {
	case SST_TC_MONE:
	    beta = 0;
	    break;
	case SST_TC_MCLOCAL:
	    beta = local;
	    break;
	case SST_TC_MAOTHER:
	  beta = otherTextureAlpha;
	  break;
	case SST_TC_MALOCAL:
	  beta = localTextureAlpha;
	  break;
	case SST_TC_MLOD:
	    {
		int dMax, dScale, dBias;

		// fetch hardware settings from shadow register
		dMax = (tDetail & SST_DETAIL_MAX)>>SST_DETAIL_MAX_SHIFT;
		dScale = (tDetail & SST_DETAIL_SCALE)>>SST_DETAIL_SCALE_SHIFT;
		dBias = (tDetail & SST_DETAIL_BIAS)>>SST_DETAIL_BIAS_SHIFT;
		dBias = SIGN_EXTEND(dBias,6);

		beta = (dBias<<8) + ~raw_lod;		// add in bias
		if (beta < 0) beta = 0;			// clamp low to 0
		else {
		    beta <<= dScale;			// shift by dScale
		    beta >>= 8;				// truncate fraction
		    if (beta > dMax) beta = dMax;	// clamp high
		}
		if (hw_w_is_neg) beta = dMax;
		//GDBG_INFO(0, "diag detail factor = %d\n", beta);
	    }
	    break;
	case SST_TC_MLODFRAC:
	    beta = lod_frac & 0xFF;
	    if (!reversed) inc = 0;		// DON'T INCREMENT		
	    break;
	case SST_TC_MONE6:
	    beta = mselect_6;
	    break;
	case SST_TC_MCMSELECT7:
	    beta = mselect_7;
	    break;
	default:
	   GDBG_ERROR("tcu","invalid TC combine mode 0x%x\n",
		(tMode & SST_TCOMBINE)>>SST_TCOMBINE_SHIFT);
    }
    gdbg_info(8,"beta = %d 0x%x + %d\n",beta,beta,inc);
    beta ^= reversed;				// reverse first
    beta += inc;				// then increment (or not)
    res *= beta;				// multiply
    res >>= 8;					// truncate
    switch (tMode & (SST_TC_ADD_CLOCAL|SST_TC_ADD_ALOCAL)) {
      case 0:
	dres = 0;
	break;
      case SST_TC_ADD_CLOCAL:
	dres = local;
	break;
      case SST_TC_ADD_ALOCAL:
	dres = localTextureAlpha;
	break;
      case SST_TC_ADD_CLOCAL|SST_TC_ADD_ALOCAL:
	dres = addlocal_3;	// mux_select_3
	break;
    }
    if (invert_other_local & SST_CM_TC_INVERT_ADD_LOCAL)
	dres = 255-dres;
    res += dres;
    res <<= outshift;
    if (res < 0) res = 0;
    if (res > 255) res = 255;
    if (tMode & SST_TC_INVERT_OUTPUT) {
	res ^= 0xFF;
    }
    return res;
}

//----------------------------------------------------------------------
// perform one texture access and expand to 32-bit 8888 RGBA format
// HACK: uses sst*to* routines from within the simulator
//----------------------------------------------------------------------
static void _texTo8888(unsigned char *out, Texture *tx, int lod, int u, int v)
{  

  //Convert u and v from .8 to .0
  u >>= 8;
  v >>= 8;      

  if(tx->tMode & SST_COMPRESSED_TEXTURES)
    {
      /////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////
      //           Compressed textures
      /////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////
      FxU32 texelBlock[4]; 
      FxU32 uShiftRight, vShiftRight, bitsPerTexel, mask;
      FxI32 x, y;
      FxU8 nibble;

      //This is the bounding box for the compressed texel block.
      FxI32 minU, maxU, minV, maxV;   
      
      switch(tx->tMode & SST_TFORMAT)
	{
	case SST_3DFX_COMPRESSED:  //These are arranged in 8x4 blocks
	case SST_DXT1:
	  uShiftRight = 3;
	  vShiftRight = 2;
	  bitsPerTexel = 4;
	  mask = 0xf;
	  break;

	case SST_DXT2:             //These are arranged in 4x4 blocks
	case SST_DXT4:
	  uShiftRight = 2;
	  vShiftRight = 2;
	  bitsPerTexel = 8;
	  mask = 0xff;
	  break;

	default:
	  assert(0);
	}
      
      //Calculate the bounding box for the compressed texel block
      minU = (u >> uShiftRight) << uShiftRight;
      minV = (v >> vShiftRight) << vShiftRight;
      maxU = minU + ((1 << uShiftRight) - 1);
      maxV = minV + ((1 << vShiftRight) - 1);
                  
      //Build up the 128 bits for the texture in little endian format
      for(y=maxV; y>= minV; y--)
	for(x=maxU; x>=minU; x--)
	  {
	    nibble = (FxU8)getMipmapData(tx, lod, x, y);

	    //This is inefficient, but f it (basically a 128 bit shift register)
	    texelBlock[3] = (texelBlock[3]<<bitsPerTexel) | ((texelBlock[2]>>(32-bitsPerTexel)) & mask);
	    texelBlock[2] = (texelBlock[2]<<bitsPerTexel) | ((texelBlock[1]>>(32-bitsPerTexel)) & mask);
	    texelBlock[1] = (texelBlock[1]<<bitsPerTexel) | ((texelBlock[0]>>(32-bitsPerTexel)) & mask);
	    texelBlock[0] = (texelBlock[0]<<bitsPerTexel) | (nibble & mask);
	  }

      GDBG_INFO(201, "texelBlock: 0x%08x_%08x_%08x_%08x\n", texelBlock[3],
		texelBlock[2], texelBlock[1], texelBlock[0]);      
      
      csimDecompressTexel(texelBlock, tx->tMode, u, v, out);
      GDBG_INFO(201, "csimDecompressTexel (%d, %d) = 0x%02x %02x %02x %02x\n",
		u, v, out[0], out[1], out[2], out[3]);

    }
  else
    {
      /////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////
      //         Non-Compressed textures
      /////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////
      unsigned int   texel32;
      unsigned short texel16;
      unsigned char  texel8;


      texel32 = getMipmapData(tx, lod, u, v);
      texel16 = (unsigned short)texel32;
      texel8  = (unsigned char)texel16;
      
      GDBG_INFO(187, "        _texTo8888: lod=0x%x  u=0x%x  v=0x%x  texel32=0x%x\n", 
		lod, u, v, texel32);

      switch(tx->tMode & SST_TFORMAT) {
      case SST_RGB332:
	sstRgba332to8888(out,texel8);
	break;
      case SST_YIQ422:
	sstYab422to8888(&tx->ncc,out,texel8);
	break;
      case SST_A8:
	out[0] = out[1] = out[2] = out[3] = texel8;
	break;
      case SST_I8:
	out[0] = out[1] = out[2] = texel8;
	out[3] = 0xFF;
	break;
      case SST_AI44:
	sstAi44to8888(out,texel8);
	break;
      case SST_P8:
	texel16 = 0xFF00;		// make it look like AP88
      case SST_AP88:
	u = tx->palette[texel8];
	out[0] = (unsigned char)(u>>16);
	out[1] = (unsigned char)(u>>8);
	out[2] = (unsigned char)(u>>0);
	out[3] = texel16>>8;
	break;
      case SST_P8_ARGB6666:
	GDBG_INFO(10,"pal[%d] = 0x%06x\n",texel8, tx->palette[texel8]);
	sstRgba6666to8888(out,tx->palette[texel8]);
	break;

      case SST_RGB565:
	sstRgba565to8888(out,texel16);
	break;
      case SST_ARGB1555:
	sstRgba1555to8888(out,texel16);
	break;
      case SST_ARGB4444:
	sstRgba4444to8888(out,texel16);
	break;
      case SST_ARGB8332:
	sstRgba332to8888(out,texel8);
	out[3] = texel16>>8;
	break;
      case SST_AYIQ8422:
	sstYab422to8888(&tx->ncc,out,texel8);
	out[3] = texel16>>8;
	break;
      case SST_AI88:
	out[0] = out[1] = out[2] = texel8 & 0xFF;
	out[3] = texel16>>8;
	break;
      case SST_ARGB8888:
	out[0] = (texel32>>16) & 0xFF; //Red
	out[1] = (texel32>>8)  & 0xFF; //Green
	out[2] = (texel32>>0)  & 0xFF; //Blue
	out[3] = (texel32>>24) & 0xFF; //Alpha	      
	break;
      default:
	GDBG_ERROR("_texTo8888", "unknown texture format\n");
      }
    }
}

//----------------------------------------------------------------------
// clamp or wrap a texture coordinate pair
//----------------------------------------------------------------------

static void _clamp(Texture *tx, int lod, int *u,int *v,
			unsigned int *uf,unsigned int *vf)
{
    int maxU;
    int maxV;
    
    maxU = (tx->mip[lod]->width<<8) - 1;
    maxV = (tx->mip[lod]->height<<8) - 1;

    *uf = *u & 0xFF;				// assign initial fractions
    *vf = *v & 0xFF;
    gdbg_info(9,"\tclamp top: uf,vf = %x,%x  maxU,V=%x,%x\n",*uf,*vf,maxU,maxV);
    if (hw_clampst_on_neg_w) {
	*u = *uf = 0;
	*v = *vf = 0;
	gdbg_info(9,"\tclamp on neg w, u=v=0\n");
    }
    if (tx->tLOD & SST_TMIRRORS) {		// mirror S
	if (tx->tMode & SST_TCLAMPS) {		// clamp S
	    if (*u<0 || *u > maxU+maxU+1) *u = 0;
	}
	if (*u & (maxU+1)) *u = maxU - *u;
    }
    else if (tx->tMode & SST_TCLAMPS) {		// clamp S
	if (*u < 0) {
	    *u = *uf = 0;
	    gdbg_info(9,"\tclamp on neg s, u=0\n");
	}
	if (*u > maxU) {
	    *u = maxU;
	    *uf = 0x100;
	    gdbg_info(9,"\tclamp on pos s, u=%x\n",*u);
	}
    }
    *u &= maxU;					// wrap S

    if (tx->tLOD & SST_TMIRRORT) {		// mirror T
	if (tx->tMode & SST_TCLAMPT) {		// clamp T
	    if (*v<0 || *v > maxV+maxV+1) *v = 0;
	}
	if (*v & (maxV+1)) *v = maxV - *v;
    }
    else if (tx->tMode & SST_TCLAMPT) {		// clamp T
	if (*v < 0) {
	    *v = *vf = 0;
	    gdbg_info(9,"\tclamp on neg t, u=0\n");
	}
	if (*v > maxV) {
	    *v = maxV;
	    *vf = 0x100;
	    gdbg_info(9,"\tclamp on pos t, v=%x\n",*v);
	}
    }
    *v &= maxV;				// wrap T
    gdbg_info(9,"\tclamp bot: u,v = %x,%x uf,vf = %x,%x\n",(*u)>>8,(*v)>>8,*uf,*vf);
}

// access a texture map (1 or 4 times), point-sample or bilinear filter it
static void 
_access4(unsigned char *out, Texture *tx, int lod, int u0, int v0, int bilinRGB, int bilinA)
{
    unsigned int ufrac,vfrac;		// fractional texture coordinates
    FxU32 mask;

    lod >>= 8;				// comes in .8 format, we need .0
    
    //At this point, in 256x256 we're using .8 format in [0,256)
    //For 2048x2048 we're using .11 format in [0,256)
    //If we just pretend that we're using .11 instead of .8
    //it's just like multiplying by 8. Consequently, for 2048x2048
    //were on [0,2048)
    //For big textures, scale so that u,v are in [0,2047]x[0,2047]

    u0 >>= lod;
    v0 >>= lod;
    gdbg_info(7,"      _access4  in: u,v = %x.%02x %x.%02x lod=%d\n",
		u0>>8,u0&0xFF,v0>>8,v0&0xFF, lod);

    if (bilinRGB || bilinA) {
    	int u1,v1;
	unsigned int ufrac1,vfrac1;		// fractional texture coordinates
	unsigned char A[4],B[4],C[4],D[4];	// 4 texels, RGBA components

	u1 = u0 + 0x80;			// + 1/2
	v1 = v0 + 0x80;
	u0 -= 0x80;			// - 1/2
	v0 -= 0x80;
	_clamp(tx,lod,&u0,&v0,&ufrac,&vfrac);
	_clamp(tx,lod,&u1,&v1,&ufrac1,&vfrac1);
	if (ufrac1 == 0x100) ufrac = 0x100;
	if (vfrac1 == 0x100) vfrac = 0x100;
	// access texture 4 times, convert all formats to 8888
	gdbg_info(7,"      _access4 bilin: u,v = %x.%02x,%x.%02x  %x.%02x,%x.%02x\n",
			u0>>8,u0&0xFF,v0>>8,v0&0xFF,u1>>8,u1&0xFF,v1>>8,v1&0xFF);

	_texTo8888(A,tx,lod,u0,v0);
	_texTo8888(B,tx,lod,u1,v0);
	_texTo8888(C,tx,lod,u0,v1);
	_texTo8888(D,tx,lod,u1,v1);
	// if special chroma/alpha mode
	if ((tx->tChromarange & SST_ENCHROMAKEY_TMU) &&
	    !(tx->combMode & SST_CM_DISABLE_CHROMA_SUBSTITUTION))
	{
	    mask = 0;
	    if (chromaTest(A[0],A[1],A[2],tx->tChromakey,tx->tChromarange)) mask |= 8;
	    if (chromaTest(B[0],B[1],B[2],tx->tChromakey,tx->tChromarange)) mask |= 4;
	    if (chromaTest(C[0],C[1],C[2],tx->tChromakey,tx->tChromarange)) mask |= 2;
	    if (chromaTest(D[0],D[1],D[2],tx->tChromakey,tx->tChromarange)) mask |= 1;

	    // force alpha to 0xFF (solid) by default
	    A[3] = B[3] = C[3] = D[3] = 0xFF;

	    // set alpha=0 if chroma tests passes
	    if (mask & 8) A[3] = 0;
	    if (mask & 4) B[3] = 0;
	    if (mask & 2) C[3] = 0;
	    if (mask & 1) D[3] = 0;

	    // if not color subsituting then blacken out color too
	    if (!(tx->tChromarange & SST_ENCOLORSUBSTITUTION)) {
		if (mask & 8) A[0] = A[1] = A[2] = 0;
		if (mask & 4) B[0] = B[1] = B[2] = 0;
		if (mask & 2) C[0] = C[1] = C[2] = 0;
		if (mask & 1) D[0] = D[1] = D[2] = 0;
	    }
	}
	if (bilinA) {
	    out[3] = bilinearFilter(ufrac,vfrac,A[3],B[3],C[3],D[3]);
	}
	else {
	    if (ufrac >= 0x80)
		if (vfrac >= 0x80) out[3] = D[3];
		else out[3] = B[3];
	    else if (vfrac >= 0x80) out[3] = C[3];
		else out[3] = A[3];
	} 
	// bilinear filter the 4 texels
	if (bilinRGB) {
	    if ((tx->tChromarange & SST_ENCHROMAKEY_TMU) &&	// if special chroma/alpha mode
		!(tx->combMode & SST_CM_DISABLE_CHROMA_SUBSTITUTION))
	    {
		GDBG_INFO(12,"case %d, alpha=0x%x\n",mask,out[3]);
		if (tx->tChromarange & SST_ENCOLORSUBSTITUTION) {
		    switch (mask) {
			// |AB|	the texels are arranged like this
			// |CD|	with (0,0) in the upper left
			case 0:			// JIM:7 all failed
			        break;
			case 0xF:		// JIM:6 all passed
			        if ( CSIM_PRIVATE(diago.sstCSIM)->info->tmuRevision > 4 ) {
				  if (ufrac < 0x80)
				    ufrac = 0;       // snap S to 0
				  else
				    ufrac = 0x100;   // snap S to 1
				  if (vfrac < 0x80)
				    vfrac = 0;       // snap T to 0
				  else 
				    vfrac = 0x100;   // snap T to 1
				}
				break;
			case 3:	vfrac = 0;		// JIM:2/11 CD passed
				break;
			case 5:	ufrac = 0;		// BD passed
				break;
			case 0xA: ufrac = 0x100;	// AC passed
				break;
			case 0xC: vfrac = 0x100;	// AB passed
				break;
			case 1:			// JIM:4/5/12 D passed
			    if (ufrac < 0x80) {
				if (vfrac < 0x80) {	// in A
				    D[0]=(A[0]*2+C[0]+B[0])>>2;
				    D[1]=(A[1]*2+C[1]+B[1])>>2;
				    D[2]=(A[2]*2+C[2]+B[2])>>2;
				}
				else {			// in C
				    D[0]=(A[0]+C[0]*2+B[0])>>2;
				    D[1]=(A[1]+C[1]*2+B[1])>>2;
				    D[2]=(A[2]+C[2]*2+B[2])>>2;
				}
			    }
			    else {
				if (vfrac < 0x80) {	// in B
				    D[0]=(A[0]+C[0]+B[0]*2)>>2;
				    D[1]=(A[1]+C[1]+B[1]*2)>>2;
				    D[2]=(A[2]+C[2]+B[2]*2)>>2;
				}
				else {			// JIM:12 in D
				    D[0]=(C[0]+B[0])>>1;	// drop A
				    D[1]=(C[1]+B[1])>>1;
				    D[2]=(C[2]+B[2])>>1;
				}
			    }
				break;

			case 2:			// JIM:4/5 C passed
			    if (ufrac < 0x80) {
				if (vfrac < 0x80) {	// in A
				    C[0]=(A[0]*2+B[0]+D[0])>>2;
				    C[1]=(A[1]*2+B[1]+D[1])>>2;
				    C[2]=(A[2]*2+B[2]+D[2])>>2;
				}
				else {			// in C
				    C[0]=(A[0]+D[0])>>1;
				    C[1]=(A[1]+D[1])>>1;
				    C[2]=(A[2]+D[2])>>1;
				}
			    }
			    else {
				if (vfrac < 0x80) {	// in B
				    C[0]=(A[0]+B[0]*2+D[0])>>2;
				    C[1]=(A[1]+B[1]*2+D[1])>>2;
				    C[2]=(A[2]+B[2]*2+D[2])>>2;
				}
				else {			// in D
				    C[0]=(A[0]+B[0]+D[0]*2)>>2;
				    C[1]=(A[1]+B[1]+D[1]*2)>>2;
				    C[2]=(A[2]+B[2]+D[2]*2)>>2;
				}
			    }
				break;
			case 4:				// B passed
			    if (ufrac < 0x80) {
				if (vfrac < 0x80) {	// in A
				    B[0]=(A[0]*2+C[0]+D[0])>>2;
				    B[1]=(A[1]*2+C[1]+D[1])>>2;
				    B[2]=(A[2]*2+C[2]+D[2])>>2;
				}
				else {			// in C
				    B[0]=(A[0]+C[0]*2+D[0])>>2;
				    B[1]=(A[1]+C[1]*2+D[1])>>2;
				    B[2]=(A[2]+C[2]*2+D[2])>>2;
				}
			    }
			    else {
				if (vfrac < 0x80) {	// in B
				    B[0]=(A[0]+D[0])>>1;
				    B[1]=(A[1]+D[1])>>1;
				    B[2]=(A[2]+D[2])>>1;
				}
				else {			// in D
				    B[0]=(A[0]+C[0]+D[0]*2)>>2;
				    B[1]=(A[1]+C[1]+D[1]*2)>>2;
				    B[2]=(A[2]+C[2]+D[2]*2)>>2;
				}
			    }
				break;
			case 8:				// A passed
			    if (ufrac < 0x80) {
				if (vfrac < 0x80) {	// in A
				    A[0]=(B[0]+C[0])>>1;
				    A[1]=(B[1]+C[1])>>1;
				    A[2]=(B[2]+C[2])>>1;
				}
				else {			// in C
				    A[0]=(B[0]+C[0]*2+D[0])>>2;
				    A[1]=(B[1]+C[1]*2+D[1])>>2;
				    A[2]=(B[2]+C[2]*2+D[2])>>2;
				}
			    }
			    else {
				if (vfrac < 0x80) {	// in B
				    A[0]=(B[0]*2+C[0]+D[0])>>2;
				    A[1]=(B[1]*2+C[1]+D[1])>>2;
				    A[2]=(B[2]*2+C[2]+D[2])>>2;
				}
				else {			// in D
				    A[0]=(B[0]+C[0]+D[0]*2)>>2;
				    A[1]=(B[1]+C[1]+D[1]*2)>>2;
				    A[2]=(B[2]+C[2]+D[2]*2)>>2;
				}
			    }
				break;

			case 6:			// JIM:3/10 BC passed
			    if (ufrac < 0x80) {
				if (vfrac < 0x80) {	// in A
				    B[0]=C[0]=(A[0]*3+D[0])>>2;
				    B[1]=C[1]=(A[1]*3+D[1])>>2;
				    B[2]=C[2]=(A[2]*3+D[2])>>2;
				}
				else {			// in C
				    B[0]=C[0]=(A[0]+D[0])>>1;
				    B[1]=C[1]=(A[1]+D[1])>>1;
				    B[2]=C[2]=(A[2]+D[2])>>1;
				}
			    }
			    else {
				if (vfrac < 0x80) {	// in B
				    B[0]=C[0]=(A[0]+D[0])>>1;
				    B[1]=C[1]=(A[1]+D[1])>>1;
				    B[2]=C[2]=(A[2]+D[2])>>1;
				}
				else {			// in D
				    B[0]=C[0]=(A[0]+D[0]*3)>>2;
				    B[1]=C[1]=(A[1]+D[1]*3)>>2;
				    B[2]=C[2]=(A[2]+D[2]*3)>>2;
				}
			    }
			    break;

			case 9:			// JIM:3/10 AD passed
			    if (ufrac < 0x80) {
				if (vfrac < 0x80) {	// in A
				    A[0]=D[0]=(C[0]+B[0])>>1;
				    A[1]=D[1]=(C[1]+B[1])>>1;
				    A[2]=D[2]=(C[2]+B[2])>>1;
				}
				else {			// in C
				    A[0]=D[0]=(C[0]*3+B[0])>>2;
				    A[1]=D[1]=(C[1]*3+B[1])>>2;
				    A[2]=D[2]=(C[2]*3+B[2])>>2;
				}
			    }
			    else {
				if (vfrac < 0x80) {	// in B
				    A[0]=D[0]=(C[0]+B[0]*3)>>2;
				    A[1]=D[1]=(C[1]+B[1]*3)>>2;
				    A[2]=D[2]=(C[2]+B[2]*3)>>2;
				}
				else {			// in D
				    A[0]=D[0]=(C[0]+B[0])>>1;
				    A[1]=D[1]=(C[1]+B[1])>>1;
				    A[2]=D[2]=(C[2]+B[2])>>1;
				}
			    }
			    break;

			case 7:			// JIM:1/8/9 BCD passed
				ufrac = vfrac = 0;	// point sample A
				break;
			case 0xB:		// JIM:1/8/9 ACD passed
				ufrac = 0x100;	// point sample B
				vfrac = 0;
				break;
			case 0xD:		// JIM:1/8/9 ABD passed
				ufrac = 0;		// point sample C
				vfrac = 0x100;
				break;
			case 0xE:		// JIM:1/8/9 ABC passed
				ufrac = vfrac = 0x100;// point sample D
				break;
		    }
		}
	    }
	    out[0] = bilinearFilter(ufrac,vfrac,A[0],B[0],C[0],D[0]);
	    out[1] = bilinearFilter(ufrac,vfrac,A[1],B[1],C[1],D[1]);
	    out[2] = bilinearFilter(ufrac,vfrac,A[2],B[2],C[2],D[2]);
	}
	else {
	    if (ufrac >= 0x80)
		if (vfrac >= 0x80) {out[0]=D[0]; out[1]=D[1]; out[2]=D[2];}
		else {out[0]=B[0]; out[1]=B[1]; out[2]=B[2];}
	    else if (vfrac >= 0x80) {out[0]=C[0]; out[1]=C[1]; out[2]=C[2];}
		else {out[0]=A[0]; out[1]=A[1]; out[2]=A[2];}
	}
    }
    else {
	_clamp(tx,lod,&u0,&v0,&ufrac,&vfrac);
	_texTo8888(out,tx,lod,u0,v0);		// simple point-sample
    }
    gdbg_info(7,"      _access4 out: u,v = %x.%02x %x.%02x\trgba: %02x %02x %02x %02x\n",
			u0>>8,u0&0xFF,v0>>8,v0&0xFF,out[0],out[1],out[2],out[3]);
}

//----------------------------------------------------------------------
// return biased, clamped LOD in .8 format, also sets *bilinear
//----------------------------------------------------------------------
static int lodBiasClamp(Texture *tx, int lod, int x, int y, int *bilinearRGB, int *bilinearA)
{
    int bias, retval;
    FxI32 lodmin, lodmax;
    
    if (tx->tMode & SST_TLODDITHER) {	// add in dither fraction
	int d =  (((x^y)&1)<<1) | (y&1);
	lod += d << (8-SST_LOD_FRACBITS);
	gdbg_info(26,"    dit = .%x  new lod = %x.%02x\n",
			d << (8-SST_LOD_FRACBITS),lod>>8,lod&0xFF);
    }
    bias = (tx->tLOD & SST_LODBIAS)>>SST_LODBIAS_SHIFT;
    bias = SIGN_EXTEND(bias,SST_LOD_SIZE);
    bias <<= 8-SST_LOD_FRACBITS;
    lod += bias;

    //GDBG_INFO(0, "lodBiasClamp incoming LOD = %x\n", lod);
    //For big ass textures, shift the lodmin and lodmax by -3
    if(tx->tLOD & SST_TBIG)
      {
	GDBG_INFO(187,"    lodBiasClamp: lodmin and lodmax shifted down by -3\n");
	lodmin = tx->lodmin - (3<<SST_LOD_FRACBITS);
	lodmax = tx->lodmax - (3<<SST_LOD_FRACBITS);
      }
    else
      {
	lodmin = tx->lodmin;
	lodmax = tx->lodmax;
      }

    GDBG_INFO(187, "    lodBiasClamp: bias = 0x%x   lodmin=%x.%x  lodmax=%x.%x\n", bias,
	      (lodmin >> 2), ((lodmin << 2)&0xF),
	      (lodmax >> 2), ((lodmax << 2)&0xF));

    
    
    if (lod > 0x800) lod = 0x800;	// hard clamp to 8

    retval = lod;			// return .8 format

    // perform min/mag filter tests on original LOD
    lod >>= 8-SST_LOD_FRACBITS;		// lod is in 8.2 format
    if (lod < lodmin || hw_w_is_neg) {// compute bilinear option
	lod = lodmin;
	retval = lod<<(8-SST_LOD_FRACBITS);
	if (tx->tDetail & SST_TFILTER_SEPARATE) {
	    *bilinearRGB = tx->tDetail & SST_TMAGFILTER_RGB;
	    *bilinearA = tx->tDetail & SST_TMAGFILTER_A;
	}
	else *bilinearRGB = *bilinearA = tx->tMode & SST_TMAGFILTER;
    }
    else {
	if (tx->tDetail & SST_TFILTER_SEPARATE) {
	    *bilinearRGB = tx->tDetail & SST_TMINFILTER_RGB;
	    *bilinearA = tx->tDetail & SST_TMINFILTER_A;
	}
	else *bilinearRGB = *bilinearA = tx->tMode & SST_TMINFILTER;
    }
    if (lod >= lodmax) {		// clamp LOD
	lod = lodmax;
	retval = lod<<(8-SST_LOD_FRACBITS);
    }

    //For 2048x2048 textures, move the lod range from [-3,8] to [0,11]
    if(tx->tLOD & SST_TBIG)
      {
	retval += (3<<8);
      }

    GDBG_INFO(187, "    lodBiasClamp: clamped LOD = %x\n", retval);	      

    return retval;
}

// optionally increment LOD for trilinear case
static int lodIncrement(Texture *tx, int lod_8)
{
  assert(lod_8 >= 0);
  if(tx->tLOD & SST_TBIG)
    assert(lod_8 <= 0xBFF);
  else
    assert(lod_8 <= 0x8FF);

    if (tx->tMode & SST_TRILINEAR) {	// special trilinear adjust
	int trexODD, lodODD;
	// if TREX is odd and LOD is even, or vica versa then increment
	trexODD = (tx->tLOD & SST_LOD_ODD) != 0;

	if(tx->tLOD & SST_TBIG) //2048x2048 case	    
	  lodODD = (lod_8 & 0x100) == 0;
	else  //256x256 casae
	  lodODD = (lod_8 & 0x100) != 0;
	
	GDBG_INFO(187, "lodIncrement: trexODD=%d  lodODD=%d\n", trexODD, lodODD);
	GDBG_INFO(187, "lodIncrement: tx->tLOD=0x%x  lod_8=0x%x\n", tx->tLOD,
		  lod_8);
	if (trexODD != lodODD) {
	    lod_8 += 0x100;			// then increment it
	    
	    if(tx->tLOD & SST_TBIG)
	      {  //2048x2048 case
		if (trexODD && (lod_8>=0xC00))	// special check for 11+1 >= 12
		  lod_8 = 0xA00;
	      }
	    else
	      {  //256x256 case
		if (trexODD && (lod_8>=0x900))	// special check for 8+1 >= 9
		  lod_8 = 0x700;
	      }
	}
    }
    if (tx->tLOD & SST_LOD_ZEROFRAC)
	lod_8 &= ~0xFF;			// then zero the LOD fraction
    return lod_8;
}

#include <../csim/trexfunc.h>

#define PST(w)	printFix64("%4d.%08x",w,SST_ST64_FRACBITS)
#define PW(w)	printFix64("%3d.%08x",w,SST_W64_FRACBITS)

//----------------------------------------------------------------------
// given (x,y) compute u,v,lod texel coordinates and return
// u,v texture coords are in .8 format, already shifted right by LOD
// returns
//----------------------------------------------------------------------
static int
uvLodTriangle(Triangle *t, int x,int y, int *pu,int*pv)
{
	int dx,dy, u,v;
	FxI64 s64,t64,w64;
	TRX_STLOD_STRUCT st;		// jimm's structure
	TRX_STLOD_STRUCT *pst = &st;

	// WARNING: using diago.adjust is a bit of a hack
	if (diago.adjust) {		// .4 distance to pixel center
	    dx = (x<<SST_XY_FRACBITS) + XY_HALF - t->vA.x;
	    dy = (y<<SST_XY_FRACBITS) + XY_HALF - t->vA.y;
	}
	else {				// integer distance in .4 format
	    dx = (x - (t->vA.x>>SST_XY_FRACBITS))<<SST_XY_FRACBITS;
	    dy = (y - (t->vA.y>>SST_XY_FRACBITS))<<SST_XY_FRACBITS;
	}
//gdbg_info(11,"dx= %d(0x%x)  dy= %d(0x%x)\n",dx,dx,dy,dy);
//gdbg_info(12,"dwdx= %s  dwdy= %s\n",PW(t->dwdx),PW(t->dwdy));
	s64 = t->vA.s + ((dx*t->dsdx)>>SST_XY_FRACBITS) +
			((dy*t->dsdy)>>SST_XY_FRACBITS);
	t64 = t->vA.t + ((dx*t->dtdx)>>SST_XY_FRACBITS) +
			((dy*t->dtdy)>>SST_XY_FRACBITS);
	w64 = t->vA.w + ((dx*t->dwdx)>>SST_XY_FRACBITS) +
			((dy*t->dwdy)>>SST_XY_FRACBITS);
//gdbg_info(12,"  dwdx = %08x_%08x\n",FX_HI64(t->dwdx),FX_LO64(t->dwdx));
//gdbg_info(12,"  dwdy = %08x_%08x\n",FX_HI64(t->dwdy),FX_LO64(t->dwdy));
//gdbg_info(12,"  vA.w = %08x_%08x\n",FX_HI64(t->vA.w),FX_LO64(t->vA.w));
//gdbg_info(12,"  w64  = %08x_%08x\n",FX_HI64(w64),FX_LO64(w64));
	s64 &= SST_MASK64(SST_ST64_SIZE);
	t64 &= SST_MASK64(SST_ST64_SIZE);
	w64 &= SST_MASK64(SST_W64_SIZE);

	if (FX_EQ064(w64))			// if w==0 
	    w64 = FX_BIT64(0);			// set to minimum value
	hw_clampst_on_neg_w = 0;
	gdbg_info(10,"   stw= %s %s %s\n",PST(s64),PST(t64),PW(w64));

	{				// bit-accurate hardware emulation
		pst->it_s_i64 = s64;			// load up input
		pst->it_t_i64 = t64;
		pst->it_w_inv_i64 = w64;
		pst->tpersp_st = t->tex->tMode & SST_TPERSP_ST ? 1 : 0;
		pst->tmirrors = t->tex->tLOD & SST_TMIRRORS;
		pst->tmirrort = t->tex->tLOD & SST_TMIRRORT;
		trx_st(pst);			// calculate results
		u = (int)pst->hw_s_fxd_12_i64;	// retrieve results
		v = (int)pst->hw_t_fxd_12_i64;

		//Decide how much precision to use
		if(t->tex->tLOD & SST_TBIG)
		  {
		    //for 2048x2048 textures, use 11 bits of fraction
		    u = ((u>>4) << (SST_UV_FRACBITS + 3)) | pst->hw_s_frac_11;
		    v = ((v>>4) << (SST_UV_FRACBITS + 3)) | pst->hw_t_frac_11;
		  }
		else
		  {
		    //for 256x256 textures, use 8 bits of fraction
		    u = ((u>>4) << SST_UV_FRACBITS) | pst->hw_s_frac_8;
		    v = ((v>>4) << SST_UV_FRACBITS) | pst->hw_t_frac_8;
		  }		

		gdbg_info(11,"   raw u,v= %x %x    clamp flags: u:%c%c v:%c%c w:%c\n",
				u,v,
				pst->hw_s_fxd_is_neg ? '-':' ',
				pst->hw_s_fxd_clmp_pos ? '+':' ',
				pst->hw_t_fxd_is_neg ? '-':' ',
				pst->hw_t_fxd_clmp_pos ? '+':' ',
				pst->hw_w_is_neg ? '-':' ');
		// HACK: trx_st mask s,t to 8.4, here we take into account the clamp
		//	bits and either make the number very negative or positive
		//	while keeping the modulo the same
		u &= ~0x20000000;
		if (pst->hw_s_fxd_is_neg)
		    u |= 0xC0000000;
		else if (pst->hw_s_fxd_clmp_pos)
		    u |= 0x40000000;
		v &= ~0x20000000;
		if (pst->hw_t_fxd_is_neg)
		    v |= 0xC0000000;
		else if (pst->hw_t_fxd_clmp_pos)
		    v |= 0x40000000;
		if (pst->hw_w_is_neg && (t->tex->tMode & SST_TCLAMPW)) {
		    u = v = 0;
		    hw_clampst_on_neg_w = 1;
		    gdbg_info(11,"  w is neg, clamping u,v = 0\n");
		}
		hw_w_is_neg = pst->hw_w_is_neg;
	}
	// always compute LOD, slow but sure footed...
	pst->reg_dsdx_i64 = t->dsdx;		// load up input
	pst->reg_dsdy_i64 = t->dsdy;
	pst->reg_dtdx_i64 = t->dtdx;
	pst->reg_dtdy_i64 = t->dtdy;
	pst->reg_dwdx_i64 = t->dwdx;
	pst->reg_dwdy_i64 = t->dwdy;
	trx_lod(pst);				// calculate results
	*pu = u;
	*pv = v;
	return pst->hw_lod_7_8s;
}

//----------------------------------------------------------------------
// return the iterators (clamped) for this triangle at x,y
//----------------------------------------------------------------------
void get_iterators(unsigned char it[], int x, int y, Triangle *t)
{
    int c,dx,dy;

    if (diago.adjust) {		// .4 distance to pixel center
	dx = (x<<SST_XY_FRACBITS) + XY_HALF - t->vA.x;
	dy = (y<<SST_XY_FRACBITS) + XY_HALF - t->vA.y;
    }
    else {			// integer distance in .4 format
	dx = (x - (t->vA.x>>SST_XY_FRACBITS))<<SST_XY_FRACBITS;
	dy = (y - (t->vA.y>>SST_XY_FRACBITS))<<SST_XY_FRACBITS;
    }
    c = t->vA.r + ((dx*t->drdx)>>SST_XY_FRACBITS) +
		  ((dy*t->drdy)>>SST_XY_FRACBITS);
    c >>= SST_RGBA_FRACBITS;
    if (c == 0x100) c = 0xFF;	// check for 1 unit overflow
    if (c == 0xFFFFFFFF) c = 0;	// check for 1 unit underflow
    it[0] = c;

    c = t->vA.g + ((dx*t->dgdx)>>SST_XY_FRACBITS) +
		  ((dy*t->dgdy)>>SST_XY_FRACBITS);
    c >>= SST_RGBA_FRACBITS;
    if (c == 0x100) c = 0xFF;	// check for 1 unit overflow
    if (c == 0xFFFFFFFF) c = 0;	// check for 1 unit underflow
    it[1] = c;

    c = t->vA.b + ((dx*t->dbdx)>>SST_XY_FRACBITS) +
		  ((dy*t->dbdy)>>SST_XY_FRACBITS);
    c >>= SST_RGBA_FRACBITS;
    if (c == 0x100) c = 0xFF;	// check for 1 unit overflow
    if (c == 0xFFFFFFFF) c = 0;	// check for 1 unit underflow
    it[2] = c;

    c = t->vA.a + ((dx*t->dadx)>>SST_XY_FRACBITS) +
		  ((dy*t->dady)>>SST_XY_FRACBITS);
    c >>= SST_RGBA_FRACBITS;
    if (c == 0x100) c = 0xFF;	// check for 1 unit overflow
    if (c == 0xFFFFFFFF) c = 0;	// check for 1 unit underflow
    it[3] = c;    
}

void other_select(FxU8 *other, FxU8 *otherTex, FxU8 *localTex, FxU8 *it, Triangle *t)
{
    switch(t->tex->combMode & SST_CM_TC_OTHERSELECT) {
	case SST_CM_TC_OTHERSELECT_OTHER_TRGB:
	    other[0] = otherTex[0];
	    other[1] = otherTex[1];
	    other[2] = otherTex[2];
	    break;
	case SST_CM_TC_OTHERSELECT_OTHER_TA:
	    other[0] = otherTex[3];
	    other[1] = otherTex[3];
	    other[2] = otherTex[3];
	    break;
	case SST_CM_TC_OTHERSELECT_LOCAL_TRGB:
	    other[0] = localTex[0];
	    other[1] = localTex[1];
	    other[2] = localTex[2];
	    break;
	case SST_CM_TC_OTHERSELECT_LOCAL_TA:
	    other[0] = localTex[3];
	    other[1] = localTex[3];
	    other[2] = localTex[3];
	    break;
	case SST_CM_TC_OTHERSELECT_IRGB:
	    other[0] = it[0];
	    other[1] = it[1];
	    other[2] = it[2];
	    break;
	case SST_CM_TC_OTHERSELECT_IA:
	    other[0] = it[3];
	    other[1] = it[3];
	    other[2] = it[3];
	    break;
	case SST_CM_TC_OTHERSELECT_CR_RGB:
	    other[0] = (FxU8)(t->tex->tChromarange >> 16);
	    other[1] = (FxU8)(t->tex->tChromarange >>  8);
	    other[2] = (FxU8)(t->tex->tChromarange >>  0);
	    break;
	case SST_CM_TC_OTHERSELECT_CR_A:
	    other[0] = (FxU8)(t->tex->tChromarange >> 24);
	    other[1] = (FxU8)(t->tex->tChromarange >> 24);
	    other[2] = (FxU8)(t->tex->tChromarange >> 24);
	    break;
    }
    switch(t->tex->combMode & SST_CM_TCA_OTHERSELECT) {
	case SST_CM_TCA_OTHERSELECT_OTHER_TA:
	    other[3] = otherTex[3];
	    break;
	case SST_CM_TCA_OTHERSELECT_LOCAL_TA:
	    other[3] = localTex[3];
	    break;
	case SST_CM_TCA_OTHERSELECT_IA:
	    other[3] = it[3];
	    break;
	case SST_CM_TCA_OTHERSELECT_CR_A:
	    other[3] = (FxU8)(t->tex->tChromarange >> 24);
	    break;
    }
}

void local_select(FxU8 *local, FxU8 *otherTex, FxU8 *localTex, FxU8 *it, Triangle *t)
{
    switch(t->tex->combMode & SST_CM_TC_LOCALSELECT) {
	case SST_CM_TC_LOCALSELECT_LOCAL_TRGB:
	    local[0] = localTex[0];
	    local[1] = localTex[1];
	    local[2] = localTex[2];
	    break;
	case SST_CM_TC_LOCALSELECT_LOCAL_TA:
	    local[0] = localTex[3];
	    local[1] = localTex[3];
	    local[2] = localTex[3];
	    break;
	case SST_CM_TC_LOCALSELECT_OTHER_TRGB:
	    local[0] = otherTex[0];
	    local[1] = otherTex[1];
	    local[2] = otherTex[2];
	    break;
	case SST_CM_TC_LOCALSELECT_OTHER_TA:
	    local[0] = otherTex[3];
	    local[1] = otherTex[3];
	    local[2] = otherTex[3];
	    break;
	case SST_CM_TC_LOCALSELECT_IRGB:
	    local[0] = it[0];
	    local[1] = it[1];
	    local[2] = it[2];
	    break;
	case SST_CM_TC_LOCALSELECT_IA:
	    local[0] = it[3];
	    local[1] = it[3];
	    local[2] = it[3];
	    break;
	case SST_CM_TC_LOCALSELECT_CK_RGB:
	    local[0] = (FxU8)(t->tex->tChromakey >> 16);
	    local[1] = (FxU8)(t->tex->tChromakey >>  8);
	    local[2] = (FxU8)(t->tex->tChromakey >>  0);
	    break;
	case SST_CM_TC_LOCALSELECT_CK_A:
	    local[0] = (FxU8)(t->tex->tChromakey >> 24);
	    local[1] = (FxU8)(t->tex->tChromakey >> 24);
	    local[2] = (FxU8)(t->tex->tChromakey >> 24);
	    break;
    }
    switch(t->tex->combMode & SST_CM_TCA_LOCALSELECT) {
	case SST_CM_TCA_LOCALSELECT_LOCAL_TA:
	    local[3] = localTex[3];
	    break;
	case SST_CM_TCA_LOCALSELECT_OTHER_TA:
	    local[3] = otherTex[3];
	    break;
	case SST_CM_TCA_LOCALSELECT_IA:
	    local[3] = it[3];
	    break;
	case SST_CM_TCA_LOCALSELECT_CK_A:
	    local[3] = (FxU8)(t->tex->tChromakey >> 24);
	    break;
    }
}

void msel7_select(FxU8 *msel7, FxU8 *otherTex, FxU8 *localTex, FxU8 *it, Triangle *t)
{
  switch(t->tex->combMode & SST_CM_TC_MSELECT_7) {
  case SST_CM_TC_MSELECT_7_LOCAL_TRGB:
    msel7[0] = localTex[0];
    msel7[1] = localTex[1];
    msel7[2] = localTex[2];
    gdbg_info(7,"msel7[0] = localTex[0] = %02x\n",localTex[0]);
    break;
  case SST_CM_TC_MSELECT_7_ZERO:
  case SST_CM_TC_MSELECT_7_ZERO3:
    msel7[0] = 0;
    msel7[1] = 0;
    msel7[2] = 0;
    break;
  case SST_CM_TC_MSELECT_7_OTHER_TRGB:
    msel7[0] = otherTex[0];
    msel7[1] = otherTex[1];
    msel7[2] = otherTex[2];
    break;
  case SST_CM_TC_MSELECT_7_IRGB:
    msel7[0] = it[0];
    msel7[1] = it[1];
    msel7[2] = it[2];
    break;
  case SST_CM_TC_MSELECT_7_IA:
    msel7[0] = it[3];
    msel7[1] = it[3];
    msel7[2] = it[3];
    break;
  case SST_CM_TC_MSELECT_7_CR_RGB:
    msel7[0] = (FxU8)(t->tex->tChromarange >> 16);
    msel7[1] = (FxU8)(t->tex->tChromarange >>  8);
    msel7[2] = (FxU8)(t->tex->tChromarange >>  0);
    break;
  case SST_CM_TC_MSELECT_7_CR_A:
    msel7[0] = (FxU8)(t->tex->tChromarange >> 24);
    msel7[1] = (FxU8)(t->tex->tChromarange >> 24);
    msel7[2] = (FxU8)(t->tex->tChromarange >> 24);
    break;
  }
}

//----------------------------------------------------------------------
// read a texture map given pixel (x,y), filter it, return data in *out
//----------------------------------------------------------------------
void texTo8888(unsigned char *out, Triangle *t, int x, int y)
{
    int raw_lod0, raw_lod1, lodB0_8, lodB1_8, lod0_8, lod1_8;
    int u0,v0, u1,v1;
    int bilinear0RGB, bilinear0A, bilinear1RGB, bilinear1A;
    int reverse, outshift, lodODD;
    unsigned char it[4],other[4],local[4],otherTex[4],localTex[4],msel7[4];

    // get and clamp the iterators here
    get_iterators(it,x,y,t);
    
    if(t->next)
      {
	otherTex[0] = 0;
	otherTex[1] = 0;
	otherTex[2] = 0;
	otherTex[3] = 0;
      }
    else
      {
	otherTex[0] = 23;		// init to random trash
	otherTex[1] = 99;
	otherTex[2] =  7;
	otherTex[3] = 13;
      }

    if (t->next) {			// if 2nd texture active, access it
	raw_lod1 = uvLodTriangle(t->next, x,y, &u1, &v1);
	lodB1_8 = lodBiasClamp(t->next->tex,raw_lod1,x,y, &bilinear1RGB,&bilinear1A);
	lod1_8 = lodIncrement(t->next->tex,lodB1_8);
	gdbg_info(25,"    raw lod.1 = %x.%02x\n",raw_lod1>>8,raw_lod1&0xFF);
	GDBG_INFO(25,"    clamped lod.1 = %x.%02x\n",lod1_8>>8,lod1_8&0xFF);
	_access4(localTex,t->next->tex,lod1_8,u1,v1,bilinear1RGB,bilinear1A);

	if(t->next->tex->tLOD & SST_TBIG) //2048x2048 case	    
	  lodODD = (lodB1_8&0x100)==0;
	else
	  lodODD = (lodB1_8&0x100)!=0;

	reverse = ((t->next->tex->tMode & SST_TRILINEAR) && lodODD) ^
			!(t->next->tex->tMode & SST_TC_REVERSE_BLEND);
	gdbg_info(6,"    tcu1 det,lod: %02x %02x\n",
			raw_lod1,lod1_8&0xFF);
	gdbg_info(6,"    tcu1 local: %02x %02x %02x %02x\n",
			localTex[0],localTex[1],localTex[2],localTex[3]);

	other_select(other, otherTex,localTex,it,t->next);
	local_select(local, otherTex,localTex,it,t->next);
	msel7_select(msel7, otherTex,localTex,it,t->next);
	outshift = (t->next->tex->combMode & SST_CM_TC_OUTSHIFT)>>SST_CM_TC_OUTSHIFT_SHIFT;
	// put trash into other inputs
	otherTex[0]=tcu(t->next->tex->tMode,t->next->tex->tDetail,
		   other[0],local[0],other[3],otherTex[3],local[3],localTex[3],0,msel7[0],it[0],
		   t->next->tex->combMode,
		   outshift,
		   raw_lod1,lod1_8&0xFF,reverse);
	otherTex[1]=tcu(t->next->tex->tMode,t->next->tex->tDetail,
		   other[1],local[1],other[3],otherTex[3],local[3],localTex[3],0,msel7[1],it[1],
		   t->next->tex->combMode,
		   outshift,
		   raw_lod1,lod1_8&0xFF,reverse);
	otherTex[2]=tcu(t->next->tex->tMode,t->next->tex->tDetail,
		   other[2],local[2],other[3],otherTex[3],local[3],localTex[3],0,msel7[2],it[2],
		   t->next->tex->combMode,
		   outshift,
		   raw_lod1,lod1_8&0xFF,reverse);
	reverse = ((t->next->tex->tMode & SST_TRILINEAR) && lodODD) ^
			!(t->next->tex->tMode & SST_TCA_REVERSE_BLEND);
	outshift = (t->next->tex->combMode & SST_CM_TCA_OUTSHIFT)>>SST_CM_TCA_OUTSHIFT_SHIFT;
	// copy the alpha combine bits to the color combine bits
	lodODD = t->next->tex->tMode & ~SST_TCOMBINE;
	lodODD |= ((t->next->tex->tMode & SST_TACOMBINE)>>SST_TACOMBINE_SHIFT)<<SST_TCOMBINE_SHIFT;
	otherTex[3]=tcu(lodODD,t->next->tex->tDetail,
		   other[3],local[3],other[3],otherTex[3],local[3],localTex[3],
		   it[3],(FxU8)(t->next->tex->tChromarange>>24),it[3],
		   t->next->tex->combMode>>(SST_CM_TCA_INVERT_OTHER_SHIFT-SST_CM_TC_INVERT_OTHER_SHIFT),
		   outshift,
		   raw_lod1,lod1_8&0xFF,reverse);
	gdbg_info(6,"    tcu1 out:   %02x %02x %02x %02x\n",
			otherTex[0],otherTex[1],otherTex[2],otherTex[3]);
    }
    // always access 1st texture
    raw_lod0 = uvLodTriangle(t, x,y, &u0, &v0);
    lodB0_8 = lodBiasClamp(t->tex,raw_lod0,x,y, &bilinear0RGB,&bilinear0A);
    lod0_8 = lodIncrement(t->tex,lodB0_8);
    gdbg_info(25,"    raw lod.0 = %x.%02x\n",raw_lod0>>8,raw_lod0&0xFF);
    _access4(localTex,t->tex,lod0_8,u0,v0,bilinear0RGB,bilinear0A);

    {			// now apply TCU operation
	if(t->tex->tLOD & SST_TBIG) //2048x2048 case	    
	  lodODD = (lodB0_8&0x100)==0;
	else
	  lodODD = (lodB0_8&0x100)!=0;
	reverse = ((t->tex->tMode & SST_TRILINEAR) && lodODD) ^
			!(t->tex->tMode & SST_TC_REVERSE_BLEND);
	gdbg_info(6,"    tcu0 det,lod: %02x %02x\n",
			raw_lod0,lod0_8&0xFF);
	gdbg_info(6,"    tcu0 local: %02x %02x %02x %02x\n",
			localTex[0],localTex[1],localTex[2],localTex[3]);

	other_select(other, otherTex,localTex,it,t);
	local_select(local, otherTex,localTex,it,t);
	msel7_select(msel7, otherTex,localTex,it,t);
	outshift = (t->tex->combMode & SST_CM_TC_OUTSHIFT)>>SST_CM_TC_OUTSHIFT_SHIFT;
	out[0]=tcu(t->tex->tMode,t->tex->tDetail,
		   other[0],local[0],other[3],otherTex[3],local[3],localTex[3],0,msel7[0],it[0],
		   t->tex->combMode,
		   outshift,
		   raw_lod0,lod0_8&0xFF,reverse);
	out[1]=tcu(t->tex->tMode,t->tex->tDetail,
		   other[1],local[1],other[3],otherTex[3],local[3],localTex[3],0,msel7[1],it[1],
		   t->tex->combMode,
		   outshift,
		   raw_lod0,lod0_8&0xFF,reverse);
	out[2]=tcu(t->tex->tMode,t->tex->tDetail,
		   other[2],local[2],other[3],otherTex[3],local[3],localTex[3],0,msel7[2],it[2],
		   t->tex->combMode,
		   outshift,
		   raw_lod0,lod0_8&0xFF,reverse);
	reverse = ((t->tex->tMode & SST_TRILINEAR) && lodODD) ^
			!(t->tex->tMode & SST_TCA_REVERSE_BLEND);
	outshift = (t->tex->combMode & SST_CM_TCA_OUTSHIFT)>>SST_CM_TCA_OUTSHIFT_SHIFT;
	// copy the alpha combine bits to the color combine bits
	lodODD = t->tex->tMode & ~SST_TCOMBINE;
	lodODD |= ((t->tex->tMode & SST_TACOMBINE)>>SST_TACOMBINE_SHIFT)<<SST_TCOMBINE_SHIFT;
	out[3]=tcu(lodODD,t->tex->tDetail,
		   other[3],local[3],other[3],otherTex[3],local[3],localTex[3],
		   it[3],(FxU8)(t->tex->tChromarange>>24),it[3],
		   t->tex->combMode>>(SST_CM_TCA_INVERT_OTHER_SHIFT-SST_CM_TC_INVERT_OTHER_SHIFT),
		   outshift,
		   raw_lod0,lod0_8&0xFF,reverse);
	gdbg_info(6,"    tcu0 out:   %02x %02x %02x %02x\n",
			out[0],out[1],out[2],out[3]);
    }
}



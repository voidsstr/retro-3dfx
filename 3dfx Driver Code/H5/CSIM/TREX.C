#include "vxd.h"
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
** $Date: 10/11/00 8:09:21 PM$
*/

//#include <math.h>

#include <assert.h>
#include <h3.h>
#include "h3sim.h"
#include "miptable.h"
#include "rgbfmt.h"
#include "trexfunc.h"

static int lodODD;


// copy data from the chip's registers to a decoded NCC table
void
_validateNcc(volatile NccTable *nccTab, volatile unsigned long *sstNC)
{
    int i,t1;
    volatile int *pni;

    pni = (long *)nccTab->yRGB;
    for (i=0; i<4; i++)	{	// copy the Y table, 4 ints, 16 bytes
    	t1 = sstNC[i];
#ifdef ENDB
	t1 = (t1<<24) | ((t1<<8)&0xFF0000) | ((t1>>8)&0xFF00) | ((t1>>24)&0xFF);
#endif
    	pni[i] = t1;
    }

    pni = nccTab->iRGB[0];
    for (i=4; i<12; i++) {	// decode I,Q tables into 32-bit integers
	t1 = sstNC[i];		// 1 word holds 3 values, R|G|B, 9-bits each
	pni[0] = ((t1 & 0x07FC0000)<< 5)>>23;	// convert each to a 32-bit int
	pni[1] = ((t1 & 0x0003FE00)<<14)>>23;	// that is sign-extended
	pni[2] = ((t1 & 0x000001FF)<<23)>>23;
	pni += 3;
    }
}

// If an NCC table is invalid, decode the packed NCC table
NccTable *
sstValidateNcc(SstRegs *sst)
{
    TmuData *td = TMU_PRIVATE(sst);

    if (sst->textureMode & SST_TNCCSELECT) {
	if (!td->nccValid1) {
	    GDBG_INFO(50,"sstValidateNcc #1\n");
	    _validateNcc(&td->ncc1,sst->nccTable1);
	    td->nccValid1 = 1;
	}
	return &td->ncc1;
    }
    else {
	if (!td->nccValid0) {
	    GDBG_INFO(50,"sstValidateNcc #0\n");
	    _validateNcc(&td->ncc0,sst->nccTable0);
	    td->nccValid0 = 1;
	}
	return &td->ncc0;
    }
}


//----------------------------------------------------------------------
// bilinearly blend four 8-bit colors into one color
// the fractions for U,V coords come from SST
//----------------------------------------------------------------------
static int
bilinearFilter(SstRegs *sst, 
	    int ufrac, int vfrac,
	    unsigned char A, unsigned char B,
	    unsigned char C, unsigned char D)
{
    int e,f,res;
    CsimPrivate *cp = CSIM_PRIVATE(sst);

    e = (vfrac * (C - A)) + (A << 8);
    f = (vfrac * (D - B)) + (B << 8);	// e,f are 8.8
    // for revisions after #3, use full precision code
    if (cp->info->tmuRevision > 3)
      res = ((((ufrac * ((f - e)>>0)) + (e << 8)) + 0x8000) >> 16) & 0xff;
    else 
      res = ((((ufrac * ((f - e + 0x40)>>7)) + (e << 1)) + 0x100) >> 9) & 0xff;
    GDBG_INFO(177,"\t\tbiblend: .%x,.%x  %02x %02x %02x %02x = %04x %04x = %-2x\n",
		ufrac,vfrac, A,B,C,D,e,f,res);
    return res;
}



//----------------------------------------------------------------------
// compute address of a texel given LOD, U, and V
//----------------------------------------------------------------------
static FxU32 sstTexelAddr( SstRegs *sst, int lod, int u, int v, int read )
{
  FxU32 texAddr;
  FxU32 stride, depth;
  FxI32 offset, addr;

  //Make sure we don't get any negative LODs
  if(lod < 0)
    {
      GDBG_ERROR("sstTexelAddr", "Negative LOD! %s(%d)\n",
		 __FILE__, __LINE__);
      return(0);
    }

  // get base address
  if ( read && (sst->tLOD & SST_TMULTIBASEADDR) ) {
    if (lod == 0)
      {
	texAddr = SST_TEXTURE_UNMUNGE_ADDRESS(sst->texBaseAddr);
	GDBG_INFO(187, "Multibase: using texBaseAddr\n");
      }
    else if (lod == 1)
      {
	texAddr = sst->texBaseAddr1;
	GDBG_INFO(187, "Multibase: using texBaseAddr1\n");
      }
    else if (lod == 2)
      {
	texAddr = sst->texBaseAddr2;
	GDBG_INFO(187, "Multibase: using texBaseAddr2\n");
      }
    else 
      {
	texAddr = sst->texBaseAddr38;
	GDBG_INFO(187, "Multibase: using texBaseAddr38\n");
      }
  } else {
    texAddr = SST_TEXTURE_UNMUNGE_ADDRESS(sst->texBaseAddr);
  }

  if (texAddr & 0xF)		// must be multiple of 16 bytes
    GDBG_ERROR("sstTexelAddr","invalid (unaligned-16) texture base address=0x%x\n",texAddr);

  if ( sst->texBaseAddr & SST_TEXTURE_IS_TILED ) {  
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    //                   Tiled texel address
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    tiledStruct mipmap;
    FxI32 uTotal, vTotal;

    //Locate the offset of the mipmap
    mipmap = sstTiledMipMapOffset2(lod, sst->tLOD, sst->textureMode);
    uTotal = mipmap.uoff + u;
    vTotal = mipmap.voff + v;

    GDBG_INFO(187, "tiled lod %d located at %d,%d relative to texBaseAddr\n",
	      lod, mipmap.uoff, mipmap.voff);
    GDBG_INFO(187, "Texel(%d,%d) at %d,%d relative to texBaseAddr\n",
	      u, v, uTotal, vTotal);

    if(sst->textureMode & SST_COMPRESSED_TEXTURES)
      {
	//Convert to microtiles
	if(SST_T4BIT_COMPRESSED(sst->textureMode))
	  {
	    if(uTotal > 0)
	      uTotal /= 8;
	    else
	      uTotal = (uTotal - 7)/8;
	  }
	else if(SST_T8BIT_COMPRESSED(sst->textureMode))
	  {
	    if(uTotal > 0)
	      uTotal /= 4;
	    else
	      uTotal = (uTotal - 3)/4;
	  }
	else 
	  assert(0);

	if(vTotal > 0)
	  vTotal /= 4;
	else
	  vTotal = (vTotal - 3)/4;

	GDBG_INFO(187, "Compressed texel(%d,%d) in microtile at %d,%d relative to texBaseAddr\n",
		  u, v, uTotal, vTotal);
	
	depth = 16;   //All compressed texture block are 128 bits
      }
    else
      {
	// Non-Compressed textures
	if(SST_T8BIT(sst->textureMode))
	  depth=1;
	else if(SST_T16BIT(sst->textureMode))
	  depth=2;
	else if(SST_T32BIT(sst->textureMode))
	  depth=4;
	else
	  GDBG_ERROR("sstTexelAddr", "Illegal TFormat in textureMode!");
      }

    stride = (sst->texBaseAddr & SST_TEXTURE_TILESTRIDE) >> SST_TEXTURE_TILESTRIDE_SHIFT;
    
    addr = tiledAddress(texAddr, stride, depth, uTotal, vTotal);
    GDBG_INFO(198,"sstTexelAddr Tiled: addr=0x%x, base=0x%x, stride=0x%x, depth=%d\n",
	      addr,texAddr,stride,depth);
  } else { 
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    //                   Linear texel address
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    FxI32 logTextureWidth;
    
    // get mipmap level base offset
    offset = sstLinearMipMapOffset2(lod, sst->tLOD, sst->textureMode);

    GDBG_INFO(187, "linear lod %d located at offset %d(0x%x) relative to texBaseAddr\n",
	      lod, offset, offset);

    if(sst->tLOD & SST_TBIG)
      logTextureWidth = 11-lod;
    else
      logTextureWidth = 8-lod;
    
    if(!(sst->tLOD & SST_LOD_S_IS_WIDER))
      logTextureWidth -= ((sst->tLOD & SST_LOD_ASPECT) >> SST_LOD_ASPECT_SHIFT);
    
    if(logTextureWidth < 0)
      logTextureWidth = 0;

    // get texel offset within mipmap
    if(!(sst->textureMode & SST_COMPRESSED_TEXTURES))
      { //Non-compressed textures
	FxI32 depth;

	if(SST_T8BIT(sst->textureMode))
	  depth = 1;
	else if(SST_T16BIT(sst->textureMode))
	  depth = 2;
	else if(SST_T32BIT(sst->textureMode))
	  depth = 4;
	else 
	  assert(0);
	
	offset += ((v<<logTextureWidth) + u) * depth;
      }
    else
      { //Compressed textures
	//Things get a little more wacky for compressed textures
	//We return the address of the 128-bit line	
	switch(sst->textureMode & SST_TFORMAT)
	  {
	  case SST_3DFX_COMPRESSED:
	  case SST_DXT1:
	    //Need to remember that 4bpt compressed are at least 8 wide
	    if(logTextureWidth < 3)
	      logTextureWidth = 3;
	    
	    offset += (((v>>2) << (logTextureWidth-3)) + (u>>3))<<4;
	    break;
	  case SST_DXT2:
	  case SST_DXT4:
	    //Need to remember that 8bpt compressed are at least 4 wide
	    if(logTextureWidth < 2)
	      logTextureWidth = 2;

	    offset += (((v>>2) << (logTextureWidth-2)) + (u>>2))<<4;
	    break;
	  default:
	    assert(0);
	  }
      }

    // compute final address
    addr = texAddr + offset;
    
    addr &= SST_TEXTURE_FULL_ADDRESS;

    GDBG_INFO(187, "linear address = 0x%x baseAddress = 0x%x  u,v=%d,%d \n",
	      addr, texAddr, u, v);

    if(addr < 0)
      {
	GDBG_ERROR("sstTexelAddr", "Linear address calculation is negative! %s(%d)\n", 
		   __FILE__, __LINE__);
	GDBG_ERROR("sstTexelAddr", "    lod=%d u=%d v=%d texAddr=%d\n", lod, u, v, texAddr);

	return(0);	
      }
  }  //End of linear texel address computation

  addr &= SST_TEXTURE_FULL_ADDRESS;

  return addr;
}


//----------------------------------------------------------------------
// write a 32-bit word into texture memory, the caller has already decoded
// the chip field of the address and passed us a pointer to the trex chip
//----------------------------------------------------------------------
void sstTrexWriteMemWorker(SstRegs *sst, long addr, FxU32 data, int nbytes, TexturePort texturePort,
			   volatile FxU8 *memory, char *functionName, char *writerName)
			   
{
    int lod, u, v, base;
    FxU32 offset;
    CsimPrivate *cp = CSIM_PRIVATE(sst);
    TmuData *td = TMU_PRIVATE(sst);

    if (addr < 0)
      {
	GDBG_ERROR(functionName,"invalid address=0x%x data=%d(0x%08x)\n",
		   addr,data,data);
	return;
      }

    switch(texturePort)
      {
      case tmu0TexturePort:
	if(addr > SST_TEX0_SIZE)
	  {
	    GDBG_ERROR(functionName, "invalid tmu0TexturePort address=0x%x data=%d(0x%08x)\n",
		       addr,data,data);
	    return;
	  }
	break;
      case tmu1TexturePort:
	if(addr > SST_TEX1_SIZE)
	  {
	    GDBG_ERROR(functionName, "invalid tmu1TexturePort address=0x%x data=%d(0x%08x)\n",
		       addr,data,data);
	    return;
	  }
	break;
      case largeTexturePort:
	if(addr > SST_TEX2_SIZE)
	  {
	    GDBG_ERROR(functionName, "invalid largeTexturePort address=0x%x data=%d(0x%08x)\n",
		       addr,data,data);
	    return;
	  }
	break;

      default:
	assert(0);
      }

    if (addr & (nbytes-1)) {
	GDBG_ERROR(functionName, "unaligned address=0x%x  data=%d(0x%08x)\n",
			addr,data,data);
	return;
    }

    if ( sst->texBaseAddr & SST_TEXTURE_IS_TILED ) { // tiled memory
      
      if(sst->textureMode & SST_COMPRESSED_TEXTURES)
	{	
	  FxU32 wordSelect;   //Which of 4 words of the 128bit block to write
	  FxU32 byteSelect; 

	  if(SST_T4BIT_COMPRESSED(sst->textureMode))
	    {
	      if(!(sst->tLOD & SST_TBIG))
		{  //256x256 textures
		  lod = (addr & SST_TEXTURE_LOD4_COMPRESSED) >> SST_TEXTURE_LOD4_COMPRESSED_SHIFT;
		  u   = (addr & SST_TEXTURE_S4_COMPRESSED) >> SST_TEXTURE_S4_COMPRESSED_SHIFT;
		  v   = (addr & SST_TEXTURE_T4_COMPRESSED) >> SST_TEXTURE_T4_COMPRESSED_SHIFT;
		  wordSelect = (addr & SST_TEXTURE_WORDSELECT4_COMPRESSED) 
		    >> SST_TEXTURE_WORDSELECT4_COMPRESSED_SHIFT;
		}
	      else
		{ //2048x2048 textures
		  lod = (addr & SST_TEXTURE_BIG_LOD4_COMPRESSED) >> SST_TEXTURE_BIG_LOD4_COMPRESSED_SHIFT;
		  u   = (addr & SST_TEXTURE_BIG_S4_COMPRESSED) >> SST_TEXTURE_BIG_S4_COMPRESSED_SHIFT;
		  v   = (addr & SST_TEXTURE_BIG_T4_COMPRESSED) >> SST_TEXTURE_BIG_T4_COMPRESSED_SHIFT;
		  wordSelect = (addr & SST_TEXTURE_BIG_WORDSELECT4_COMPRESSED) 
		    >> SST_TEXTURE_BIG_WORDSELECT4_COMPRESSED_SHIFT;
		}
	      byteSelect = addr & 0x3;
	      	      
	      //Find the microtile offset
	      offset = sstTexelAddr(sst,
				    lod,
				    u * 8,
				    v * 4,
				    0);
	      
	      GDBG_INFO(195, "4-bit compressed: u=%d   v=%d  offset=%d  addr=0x%x\n", u, v, offset, addr);
	      //Then add in the wordSelect
	      offset += wordSelect*4 + byteSelect;
	    }
	  else if(SST_T8BIT_COMPRESSED(sst->textureMode))
	    {
	      if(!(sst->tLOD & SST_TBIG))
		{  //256x256 textures
		  lod = (addr & SST_TEXTURE_LOD8_COMPRESSED) >> SST_TEXTURE_LOD8_COMPRESSED_SHIFT;
		  u   = (addr & SST_TEXTURE_S8_COMPRESSED) >> SST_TEXTURE_S8_COMPRESSED_SHIFT;
		  v   = (addr & SST_TEXTURE_T8_COMPRESSED) >> SST_TEXTURE_T8_COMPRESSED_SHIFT;
		  wordSelect = (addr & SST_TEXTURE_WORDSELECT8_COMPRESSED) 
		    >> SST_TEXTURE_WORDSELECT8_COMPRESSED_SHIFT;		  
		}
	      else
		{ //2048x2048 textures
		  lod = (addr & SST_TEXTURE_BIG_LOD8_COMPRESSED) >> SST_TEXTURE_BIG_LOD8_COMPRESSED_SHIFT;
		  u   = (addr & SST_TEXTURE_BIG_S8_COMPRESSED) >> SST_TEXTURE_BIG_S8_COMPRESSED_SHIFT;
		  v   = (addr & SST_TEXTURE_BIG_T8_COMPRESSED) >> SST_TEXTURE_BIG_T8_COMPRESSED_SHIFT;
		  wordSelect = (addr & SST_TEXTURE_BIG_WORDSELECT8_COMPRESSED) 
		    >> SST_TEXTURE_BIG_WORDSELECT8_COMPRESSED_SHIFT;
		}
	      byteSelect = addr & 0x3;

	      //Find the micro tileoffset
	      offset = sstTexelAddr(sst,
				    lod,
				    u * 4,
				    v * 4,
				    0);

	      GDBG_INFO(195, "8-bit compressed: u=%d   v=%d  offset=%d  addr=0x%x\n", u, v, offset, addr);
	      //Then add in the wordSelect
	      offset += wordSelect*4 + byteSelect;
	    }	      
	}
      else
	{
	  //**********************************************************************
	  //                    Non-compressed Textures
	  //**********************************************************************
	  if ( SST_T8BIT(sst->textureMode) ) {
	    if(! (sst->tLOD & SST_TBIG))
	      {  //256x256 textures
		lod = (addr&SST_TEXTURE_LOD8) >> SST_TEXTURE_LOD8_SHIFT;
		v = (addr&SST_TEXTURE_T8) >> SST_TEXTURE_T8_SHIFT;
		u = (addr&SST_TEXTURE_S8) >> SST_TEXTURE_S8_SHIFT;
	      }
	    else 
	      {  //2048x2048 textures  (lod 0: 2048x2048, lod 3: 256x256)
		lod = ((addr&SST_TEXTURE_BIG_LOD8) >> SST_TEXTURE_BIG_LOD8_SHIFT);
		v = (addr&SST_TEXTURE_BIG_T8) >> SST_TEXTURE_BIG_T8_SHIFT;
		u = (addr&SST_TEXTURE_BIG_S8) >> SST_TEXTURE_BIG_S8_SHIFT;
	      }
	
	    offset = sstTexelAddr(sst,lod,u,v,0);         
	  }
	  else if(SST_T16BIT(sst->textureMode))
	    {
	      if(! (sst->tLOD & SST_TBIG))
		{  //256x256 textures  	    
		  lod = (addr&SST_TEXTURE_LOD16) >> SST_TEXTURE_LOD16_SHIFT;
		  v = (addr&SST_TEXTURE_T16) >> SST_TEXTURE_T16_SHIFT;
		  u = (addr&SST_TEXTURE_S16) >> SST_TEXTURE_S16_SHIFT;
		}
	      else
		{  //2048x2048 textures  (lod 0: 2048x2048, lod 3: 256x256)
		  lod = ((addr&SST_TEXTURE_BIG_LOD16) >> SST_TEXTURE_BIG_LOD16_SHIFT);
		  v = (addr&SST_TEXTURE_BIG_T16) >> SST_TEXTURE_BIG_T16_SHIFT;
		  u = (addr&SST_TEXTURE_BIG_S16) >> SST_TEXTURE_BIG_S16_SHIFT;
		}
	  
	      offset = sstTexelAddr(sst,lod,u,v,0) + (addr&0x1);
	    }
	  else if(SST_T32BIT(sst->textureMode))
	    {
	      if(! (sst->tLOD & SST_TBIG))
		{  //256x256 textures	    
		  lod = (addr&SST_TEXTURE_LOD32) >> SST_TEXTURE_LOD32_SHIFT;
		  v = (addr&SST_TEXTURE_T32) >> SST_TEXTURE_T32_SHIFT;
		  u = (addr&SST_TEXTURE_S32) >> SST_TEXTURE_S32_SHIFT;
		}
	      else
		{  //2048x2048 textures  (lod 0: 2048x2048, lod 3: 256x256)
		  lod = ((addr&SST_TEXTURE_BIG_LOD32) >> SST_TEXTURE_BIG_LOD32_SHIFT);
		  v = (addr&SST_TEXTURE_BIG_T32) >> SST_TEXTURE_BIG_T32_SHIFT;
		  u = (addr&SST_TEXTURE_BIG_S32) >> SST_TEXTURE_BIG_S32_SHIFT;
		}
	  
	      offset = sstTexelAddr(sst,lod,u,v,0) + (addr&0x3);	
	    }
	  else
	    {
	      GDBG_ERROR("sstTrexWriteMem", "Diggity damn! Illegal TFORMAT in textureMode");
	    }
	}
    } else {                                         // linear memory
      FxI32 startLOD;
      base = SST_TEXTURE_UNMUNGE_ADDRESS(sst->texBaseAddr);

      //Add in offset to starting LOD
      if(sst->tLOD & SST_LOD_TSPLIT)
	{
	  if(sst->tLOD & SST_LOD_ODD)
	    startLOD = sstLinearMipMapOffset2(0, sst->tLOD, sst->textureMode);
	  else
	    startLOD = sstLinearMipMapOffset2(1, sst->tLOD, sst->textureMode);
	}
      else
	startLOD = sstLinearMipMapOffset2(0, sst->tLOD, sst->textureMode);

      base += startLOD;

      if (base & 0xF)	// must be multiple of 16 bytes
	GDBG_ERROR("sstTrexWriteMem","invalid (unaligned-16) texture base address=0x%x\n",base);

      offset = base + addr;

      GDBG_INFO(207, "Linear write textureport: baseAddress=0x%x, startLOD=0x%x, final address=0x%x\n",
		SST_TEXTURE_UNMUNGE_ADDRESS(sst->texBaseAddr), startLOD, offset);
    }

    offset &= SST_TEXTURE_FULL_ADDRESS;
    
    if (nbytes == 4)
      writeMem32(memory,offset,data, cp->memorySizeInBytes,  writerName);
    else if (nbytes == 2)
      writeMem16(memory,offset,data, cp->memorySizeInBytes, writerName);
    else if (nbytes == 1)
      writeMem8(memory,offset,data, cp->memorySizeInBytes, writerName);
    else 
      GDBG_ERROR(functionName, "invalid nbytes of %d\n",nbytes);
}

void sstTrexBackdoor(SstRegs *sst, long addr, FxU32 data, int nbytes, TexturePort texturePort)
{
  CsimPrivate *cpriv;
  volatile FxU8 *memory;
  
  cpriv = CSIM_PRIVATE(sst);
  
  if(halInfo.hsim & HSIM_HW_SIMULATION)
    memory = cpriv->hsim_memory;
  else  
    //If not using the hardware, just write into csim's framebuffer
    memory = cpriv->memory;

  sstTrexWriteMemWorker(sst, addr, data, nbytes, texturePort, memory,
			"sstTrexBackdoor", "hsim");
  
}

void sstTrexWriteMem(SstRegs *sst, long addr, FxU32 data, int nbytes, TexturePort texturePort)
{
  CsimPrivate *cpriv;

  cpriv = CSIM_PRIVATE(sst);
  sstTrexWriteMemWorker(sst, addr, data, nbytes, texturePort, cpriv->memory,
			"sstTrexWriteMem", "csim");
}



//----------------------------------------------------------------------
// computes LOD, adjusts u,v and sets ufrac,vfrac; no clamping/wrapping
// returns bilinear flags
// NOTE: no rounding is done, LOD_BIAS should be used for rounding
//----------------------------------------------------------------------
void
sstTextureLOD(SstRegs *sst, int *biRGB, int *biA)
{
    int tMode = sst->textureMode;
    int lod,lodmin,lodmax,bias, u,v;
    TmuData *td = TMU_PRIVATE(sst);

    lodmin = (sst->tLOD & SST_LODMIN) >> SST_LODMIN_SHIFT;
    lodmax = (sst->tLOD & SST_LODMAX) >> SST_LODMAX_SHIFT;
    lodmin <<= (8-SST_LOD_FRACBITS);	// get into .8 format
    lodmax <<= (8-SST_LOD_FRACBITS);	// get into .8 format

    //If using 2048x2048, move lodmin and lodmax down -3
    GDBG_INFO(175,"\tlodmin = %x.%02x   lodmax = %x.%02x\n",lodmin>>8,lodmin&0xFF, 
	      lodmax>>8,lodmax&0xFF);
    if(sst->tLOD & SST_TBIG)
      {
	lodmin -= 3<<8;
	lodmax -= 3<<8;
	
	GDBG_INFO(175,"\tbigAssTextures temporarily shifted lodmin = %x.%02x   lodmax = %x.%02x\n",lodmin>>8,
		  lodmin&0xFF, lodmax>>8,lodmax&0xFF);	
      }


    // compute Level Of Detail as max(LODMIN,interpolated LOD)
    if ((lodmin != lodmax) ||
	((tMode & SST_TMINFILTER) ^ ((tMode & SST_TMAGFILTER)>>1)) ||
	(sst->tDetail & SST_TFILTER_SEPARATE) ||
	((sst->textureMode&SST_TC_MSELECT)==SST_TC_MLOD) ||	// hack for detail
	((sst->textureMode&SST_TCA_MSELECT)==SST_TCA_MLOD))	// hack for detail
    {
	lod = sstLod(sst);		// .8 format
	GDBG_INFO(175,"\ttrue lod = %x.%02x\n",lod>>8,lod&0xFF);	
	td->detail_lod = lod;	// save LOD before dither/bias
	if (tMode & SST_TLODDITHER) {	// add in dither fraction
#if 1
	    // 2x2 array lookup
	    // 3 1  lsb = y
	    // 0 2  msb = x^y
	    static int LodDithmat[2][2] = {0,2,3,1};  // 2 by 2
	    int d = LodDithmat[CSIM_PRIVATE(sst)->fbiData.spanFbi.y&1]
				[CSIM_PRIVATE(sst)->fbiData.spanFbi.x&1];
#else	    // dynamic computation of 2x2 array
	    int d = ((sst->_spanFbi.y)&1) | 
			(((sst->_spanFbi.x^sst->_spanFbi.y)&1)<<1);
#endif
	    GDBG_INFO(175,"\tlod,dither = %x.%02x %d\n",lod>>8,lod&0xFF,d);
	    lod += d <<(8-SST_LOD_FRACBITS);
	}
    }
    else {
	GDBG_INFO(175,"\tlod = 0, computation skipped\n");
	lod = 0;
	td->detail_lod = lod;		// save LOD before bias
    }
    // add in LOD bias
    bias = (sst->tLOD & SST_LODBIAS)>>SST_LODBIAS_SHIFT;
    bias = bias<<(32-SST_LOD_SIZE);	// cause sign extension
    lod += bias>>(32-8-SST_LOD_SIZE+SST_LOD_FRACBITS);
      
    if (lod < lodmin || td->st.hw_w_is_neg) {// take maximum (clamps low)
	lod = lodmin;			// and select min/mag filter
	if (sst->tDetail & SST_TFILTER_SEPARATE) {
	    *biRGB = sst->tDetail & SST_TMAGFILTER_RGB;
	    *biA = sst->tDetail & SST_TMAGFILTER_A;
	}
	else *biRGB = *biA = tMode & SST_TMAGFILTER;
    }
    else {
	if (sst->tDetail & SST_TFILTER_SEPARATE) {
	    *biRGB = sst->tDetail & SST_TMINFILTER_RGB;
	    *biA = sst->tDetail & SST_TMINFILTER_A;
	}
	else *biRGB = *biA = tMode & SST_TMINFILTER;
    }

    if (lod > lodmax) lod = lodmax;	// clamp high
    if (lod > 0x800) lod = 0x800;	// hard clamp to 8

    lodODD = lod & 0x100;		// snapshot odd/even status now
    //For 2048x2048 textures, LOD 0 is odd and LOD 1 is even. This is goofy,
    //but it is correct.

    //GDBG_INFO(187, "sstTextureLOD:  sst->tLOD = 0x%x   lod=0x%x\n", sst->tLOD, lod);
    //GDBG_INFO(187, "lodODD = %d\n", lodODD);
    //GDBG_INFO(187, "sst->tLOD & SST_LOD_ODD = 0x%x\n", sst->tLOD & SST_LOD_ODD);
    
    if (tMode & SST_TRILINEAR) {	// special trilinear adjust
	// if TREX is odd and LOD is even, then increment
	if (sst->tLOD & SST_LOD_ODD) {		// if TREX has odd mipmaps
	    if (!lodODD) {			// and lod is even
	      if (lod == 0x800)		// if already at maximum
		lod = 0x700;		// then set it to maximum odd
	      else
		lod += 0x100;		// then increment it
	    }
	}
	else {					// if TREX has even mipmaps
	  if (lodODD)				// and lod is odd
	    lod += 0x100;			// then increment it
	}
    }
    if (sst->tLOD & SST_LOD_ZEROFRAC)	// if ZEROFRAC bit is set
	lod &= ~0xFF;			// then zero the LOD fraction
    
    //For 2048x2048 textures, add the 3 that was subtracted out back in
    if(sst->tLOD & SST_TBIG)
	lod += 3<<8;

    td->lod = lod;
    lod >>= 8;				// get into .0 format

    // s,t are always scaled to [0,255] regardless of
    // actual texture width, lodmin compensates for this!
    // .8 format for 256x256 and .11 format for 2048x2048
    u = td->u;				
    v = td->v;                          
    
    //for 2048x2048 textures, just pretend we're in .8 format
    //This effectively multiplies [0,256) to [0,2048)

    u >>= lod+SST_UV_FRACBITS-8;	// .8 format
    v >>= lod+SST_UV_FRACBITS-8;

    td->u = u;				// .8 format
    td->v = v;
    GDBG_INFO(170,"\tu=%x.%02x v=%x.%02x  lod=%d\n", u>>8,u&0xFF,v>>8,v&0xFF,lod);
}




//----------------------------------------------------------------------
// compute the texture color, do the following steps
//	1) compute u,v texture indicies (s/w, t/w)
//	1) mipmap LOD calculations
//	2) rgb or texture lookup (1-4 pixels)
//	3) NCC decompress 8-bit textures
//	4) apply Min or Mag filter (only options are nearest or bilinear)
//	5) write the results into the OUT array
//----------------------------------------------------------------------
void
sstTextureColor(SstRegs *sst, unsigned char out[])
{
    int u0,v0,u1,v1;			// texture indicies
    int ar, biRGB, biA, lod, maxU, maxV, tMode;
    int bpt;  //bits per texel
    FxI32 slog, tlog;

    NccTable *nccTab;
    Pal256   *p256;
    CsimPrivate *cp = CSIM_PRIVATE(sst);
    TmuData *td = TMU_PRIVATE(sst);

    sstTextureUV(sst);			// compute U,V
    sstTextureLOD(sst,&biRGB,&biA);	// compute LOD, adjust U,V
    tMode = sst->textureMode;
    lod = td->lod >> 8;			// get integer portion of LOD (.0 format)
    ar = (sst->tLOD & SST_LOD_ASPECT) >> SST_LOD_ASPECT_SHIFT;

    if (sst->tLOD & SST_LOD_S_IS_WIDER) 
      {
	if(sst->tLOD & SST_TBIG)  //2048x2048 textures
	  {
	    maxU = 0x7FFFF>>lod;
	    maxV = 0x7FFFF>>(lod+ar);

	    slog = 11-lod;
	    assert(slog >= 0);

	    tlog = slog-ar;
	    if(tlog < 0)
	      tlog = 0;
	  }
	else  //256x256 textures
	  {
	    maxU = 0xFFFF>>lod;
	    maxV = 0xFFFF>>(lod+ar);

	    slog = 8-lod;
	    assert(slog >= 0);

	    tlog = slog-ar;
	    if(tlog < 0)
	      tlog = 0;
	  }
      }
    else 
      {
	if(sst->tLOD & SST_TBIG)  //2048x2048 textures
	  {
	    maxU = 0x7FFFF>>(lod+ar);
	    maxV = 0x7FFFF>>lod;

	    tlog = 11-lod;
	    assert(tlog >= 0);

	    slog = tlog-ar;
	    if(slog < 0)
	      slog = 0;
	  }
	else  //256x256 textures
	  {
	    maxU = 0xFFFF>>(lod+ar);
	    maxV = 0xFFFF>>lod;

	    tlog = 8-lod;
	    assert(tlog >= 0);

	    slog = tlog-ar;
	    if(slog < 0)
	      slog = 0;
	  }
      }
    GDBG_INFO(170,"\tmaxU=%x maxV=%x\n", maxU, maxV);


    if(sst->textureMode & SST_COMPRESSED_TEXTURES)
      {
	if(SST_T4BIT_COMPRESSED(tMode))
	  bpt = 4;	
	else if(SST_T8BIT_COMPRESSED(tMode))
	  bpt = 8;
	else
	  GDBG_ERROR("sstTextureColor", "Opps-a-daisy! Illegal f'n compressed tformat in texturemode\n");
      }
    else
      {
	if(SST_T8BIT(tMode))
	  bpt = 8;
	else if(SST_T16BIT(tMode))
	  bpt = 16;
	else if(SST_T32BIT(tMode))
	  bpt = 32;
	else
	  GDBG_ERROR("sstTextureColor", "Opps-a-daisy! Illegal f'n tformat in texturemode\n");
      }

    u0 = td->u;
    v0 = td->v;

    // now access the texture, if bilinear then 4 times, else point sample once
    if (biRGB || biA) {
	unsigned char A[4],B[4],C[4],D[4];	// 4 texels, RGBA components
	unsigned int uf,vf;
	FxU8 mask;
	FxU8 t8[4];	 // texture access results  8,16,32 bits			
	FxU16 t16[4];
	FxU32 t32[4];

	u0 -= 0x80;				// - 1/2
	v0 -= 0x80;
	u1 = u0 + 0x100;			// + 1/2
	v1 = v0 + 0x100;
	uf = u0 & 0xff;				// get fractions
	vf = v0 & 0xff;

	if (td->st.hw_w_is_neg && (tMode & SST_TCLAMPW)) { // clamp on -W
	    u0 = u1 = uf = 0;
	    v0 = v1 = vf = 0;
	}

	if (sst->tLOD & SST_TMIRRORS) {		// mirror S
	    if (tMode & SST_TCLAMPS) {		// clamp S
		if (u0<0 || u1 > maxU+maxU+1) u0 = u1 = uf = 0;
	    }
	    if (u0 & (maxU+1)) u0 = maxU-u0;
	    if (u1 & (maxU+1)) u1 = maxU-u1;
	}
	else if (tMode & SST_TCLAMPS) {		// clamp S
	    if (u0 < 0) u0 = u1 = uf = 0;
	    if (u1 > maxU) u1 = u0 = maxU, uf = 0x100;
	}
	u0 &= maxU;				// wrap S
	u1 &= maxU;

	if (sst->tLOD & SST_TMIRRORT) {		// mirror T
	    if (tMode & SST_TCLAMPT) {		// clamp T
		if (v0<0 || v1 > maxV+maxV+1) v0 = v1 = vf = 0;
	    }
	    if (v0 & (maxV+1)) v0 = maxV-v0;
	    if (v1 & (maxV+1)) v1 = maxV-v1;
	}
	else if (tMode & SST_TCLAMPT) {		// clamp T
	    if (v0 < 0) v0 = v1 = vf = 0;
	    if (v1 > maxV) v1 = v0 = maxV, vf = 0x100;
	}
	v0 &= maxV;				// wrap T
	v1 &= maxV;

	GDBG_INFO(170,"\tclamped u,v = %x.%02x %x.%02x  %x.%02x %x.%02x  lod=%x.%02x\n",
		u0>>8,u0&0xFF,v0>>8,v0&0xFF,
		u1>>8,u1&0xFF,v1>>8,v1&0xFF,
		td->lod>>8, td->lod&0xFF);
	u0 >>= 8;				// convert to .0 format
	v0 >>= 8;
	u1 >>= 8;				// convert to .0 format
	v1 >>= 8;

	if (bpt == 4 || SST_T8BIT_COMPRESSED(sst->textureMode))
	  {  //All 4bpt textures are compressed
	    //Need to read all 128 bits to decompress the texel
	    FxU32 texelBlock[4];
	    
	    csimReadMem128(cp, sstTexelAddr(sst,lod,u0,v0,1), texelBlock);
	    csimDecompressTexel(texelBlock, tMode, u0, v0, A);

	    csimReadMem128(cp, sstTexelAddr(sst,lod,u1,v0,1), texelBlock);
	    csimDecompressTexel(texelBlock, tMode, u1, v0, B);	    	    

	    csimReadMem128(cp, sstTexelAddr(sst,lod,u0,v1,1), texelBlock);
	    csimDecompressTexel(texelBlock, tMode, u0, v1, C);	    	    

	    csimReadMem128(cp, sstTexelAddr(sst,lod,u1,v1,1), texelBlock);
	    csimDecompressTexel(texelBlock, tMode, u1, v1, D);	    	    
	  }
	else if (bpt == 8) {		// read 1st byte of each texel into t8[]
	    t8[0] = (FxU8)csimReadMem8(cp,sstTexelAddr(sst,lod,u0,v0,1));
	    t8[1] = (FxU8)csimReadMem8(cp,sstTexelAddr(sst,lod,u1,v0,1));
	    t8[2] = (FxU8)csimReadMem8(cp,sstTexelAddr(sst,lod,u0,v1,1));
	    t8[3] = (FxU8)csimReadMem8(cp,sstTexelAddr(sst,lod,u1,v1,1));
	}
	else if (bpt == 16) {
	    t16[0] = (FxU16)csimReadMem16(cp,sstTexelAddr(sst,lod,u0,v0,1));
	    t16[1] = (FxU16)csimReadMem16(cp,sstTexelAddr(sst,lod,u1,v0,1));
	    t16[2] = (FxU16)csimReadMem16(cp,sstTexelAddr(sst,lod,u0,v1,1));
	    t16[3] = (FxU16)csimReadMem16(cp,sstTexelAddr(sst,lod,u1,v1,1));
	    t8[0] = (FxU8)t16[0];
	    t8[1] = (FxU8)t16[1];
	    t8[2] = (FxU8)t16[2];
	    t8[3] = (FxU8)t16[3];
	}
	else  //bpt == 32
	  {
	    t32[0] = (FxU32)csimReadMem32(cp,sstTexelAddr(sst,lod,u0,v0,1));
	    t32[1] = (FxU32)csimReadMem32(cp,sstTexelAddr(sst,lod,u1,v0,1));
	    t32[2] = (FxU32)csimReadMem32(cp,sstTexelAddr(sst,lod,u0,v1,1));
	    t32[3] = (FxU32)csimReadMem32(cp,sstTexelAddr(sst,lod,u1,v1,1));	    
	  }

	// access texture 4 times, convert all formats to 8888
	if(!(tMode & SST_COMPRESSED_TEXTURES))
	  {  //Non-compressed texture formats (i.e. original Napalm formats)
	    //Compressed formats have already been converted to 32bpt format above
	    switch (tMode & SST_TFORMAT) {
	    case SST_RGB332:
	      _sstRgba332to8888(A,t8[0]);
	      _sstRgba332to8888(B,t8[1]);
	      _sstRgba332to8888(C,t8[2]);
	      _sstRgba332to8888(D,t8[3]);
	      break;
	    case SST_YIQ422:
	      nccTab = sstValidateNcc(sst);	// validate and return NCC
	      GDBG_INFO(176,"\t\tYIQ422 texel = %02x\n",t8[0]);
	      GDBG_INFO(176,"\t\tYIQ422 texel = %02x\n",t8[1]);
	      GDBG_INFO(176,"\t\tYIQ422 texel = %02x\n",t8[2]);
	      GDBG_INFO(176,"\t\tYIQ422 texel = %02x\n",t8[3]);
	      _sstYab422to8888(nccTab,A,t8[0]);
	      _sstYab422to8888(nccTab,B,t8[1]);
	      _sstYab422to8888(nccTab,C,t8[2]);
	      _sstYab422to8888(nccTab,D,t8[3]);
	      break;
	    case SST_A8:
	      A[0] = A[1] = A[2] = A[3] = t8[0];
	      B[0] = B[1] = B[2] = B[3] = t8[1];
	      C[0] = C[1] = C[2] = C[3] = t8[2];
	      D[0] = D[1] = D[2] = D[3] = t8[3];
	      break;
	    case SST_I8:
	      A[0] = A[1] = A[2] = t8[0];
	      B[0] = B[1] = B[2] = t8[1];
	      C[0] = C[1] = C[2] = t8[2];
	      D[0] = D[1] = D[2] = t8[3];
	      A[3] = B[3] = C[3] = D[3] = 0xFF;
	      break;
	    case SST_AI44:
	      _sstAi44to8888(A,t8[0]);
	      _sstAi44to8888(B,t8[1]);
	      _sstAi44to8888(C,t8[2]);
	      _sstAi44to8888(D,t8[3]);
	      break;

	    case SST_P8:
	      p256 = &td->pal256;
	      _sstPal256to8888(p256, A,t8[0]);
	      _sstPal256to8888(p256, B,t8[1]);
	      _sstPal256to8888(p256, C,t8[2]);
	      _sstPal256to8888(p256, D,t8[3]);
	      break;

	    case SST_P8_ARGB6666:
	      p256 = &td->pal256;
	      _sstPal6666to8888(p256, A,t8[0]);
	      _sstPal6666to8888(p256, B,t8[1]);
	      _sstPal6666to8888(p256, C,t8[2]);
	      _sstPal6666to8888(p256, D,t8[3]);
	      break;

	    case SST_RGB565:
	      _sstRgba565to8888(A,t16[0]);
	      _sstRgba565to8888(B,t16[1]);
	      _sstRgba565to8888(C,t16[2]);
	      _sstRgba565to8888(D,t16[3]);
	      break;
	    case SST_ARGB1555:
	      _sstRgba1555to8888(A,t16[0]);
	      _sstRgba1555to8888(B,t16[1]);
	      _sstRgba1555to8888(C,t16[2]);
	      _sstRgba1555to8888(D,t16[3]);
	      break;
	    case SST_ARGB4444:
	      _sstRgba4444to8888(A,t16[0]);
	      _sstRgba4444to8888(B,t16[1]);
	      _sstRgba4444to8888(C,t16[2]);
	      _sstRgba4444to8888(D,t16[3]);
	      break;
	    case SST_ARGB8332:
	      _sstRgba332to8888(A,t8[0]);
	      _sstRgba332to8888(B,t8[1]);
	      _sstRgba332to8888(C,t8[2]);
	      _sstRgba332to8888(D,t8[3]);
	      goto getAlpha;

	    case SST_AYIQ8422:
	      nccTab = sstValidateNcc(sst);	// validate and return NCC
	      GDBG_INFO(176,"\t\tAYIQ422 texel = %04x\n",t16[0]);
	      GDBG_INFO(176,"\t\tAYIQ422 texel = %04x\n",t16[1]);
	      GDBG_INFO(176,"\t\tAYIQ422 texel = %04x\n",t16[2]);
	      GDBG_INFO(176,"\t\tAYIQ422 texel = %04x\n",t16[3]);
	      _sstYab422to8888(nccTab,A,t8[0]);
	      _sstYab422to8888(nccTab,B,t8[1]);
	      _sstYab422to8888(nccTab,C,t8[2]);
	      _sstYab422to8888(nccTab,D,t8[3]);
	      goto getAlpha;
	    case SST_AI88:
	      A[0] = A[1] = A[2] = t8[0];
	      B[0] = B[1] = B[2] = t8[1];
	      C[0] = C[1] = C[2] = t8[2];
	      D[0] = D[1] = D[2] = t8[3];
	      goto getAlpha;

	    case SST_AP88:
	      p256 = &td->pal256;
	      _sstPal256to8888(p256, A, t8[0]);
	      _sstPal256to8888(p256, B, t8[1]);
	      _sstPal256to8888(p256, C, t8[2]);
	      _sstPal256to8888(p256, D, t8[3]);
	      goto getAlpha;
	      break;
				
	    getAlpha:
	      A[3] = t16[0]>>8;
	      B[3] = t16[1]>>8;
	      C[3] = t16[2]>>8;
	      D[3] = t16[3]>>8;
	      break;
		
	    case SST_ARGB8888:
	      A[0] = (unsigned char)(t32[0] >> 16) & 0xFF;  //Red
	      A[1] = (unsigned char)(t32[0] >> 8)  & 0xFF;  //Green
	      A[2] = (unsigned char)(t32[0] >> 0)  & 0xFF;  //Blue
	      A[3] = (unsigned char)(t32[0] >> 24) & 0xFF;  //Alpha
	      B[0] = (unsigned char)(t32[1] >> 16) & 0xFF;  //Red
	      B[1] = (unsigned char)(t32[1] >> 8)  & 0xFF;  //Green
	      B[2] = (unsigned char)(t32[1] >> 0)  & 0xFF;  //Blue
	      B[3] = (unsigned char)(t32[1] >> 24) & 0xFF;  //Alpha
	      C[0] = (unsigned char)(t32[2] >> 16) & 0xFF;  //Red
	      C[1] = (unsigned char)(t32[2] >> 8)  & 0xFF;  //Green
	      C[2] = (unsigned char)(t32[2] >> 0)  & 0xFF;  //Blue
	      C[3] = (unsigned char)(t32[2] >> 24) & 0xFF;  //Alpha
	      D[0] = (unsigned char)(t32[3] >> 16) & 0xFF;  //Red
	      D[1] = (unsigned char)(t32[3] >> 8)  & 0xFF;  //Green
	      D[2] = (unsigned char)(t32[3] >> 0)  & 0xFF;  //Blue
	      D[3] = (unsigned char)(t32[3] >> 24) & 0xFF;  //Alpha
	      break;

	    default:
	      GDBG_ERROR("sstTextureColor", "Oh poop, not supposed to be here!");
	      break;
	    }
	  }

	if (sst->chromaRange & SST_ENCHROMAKEY_TMU &&
	    !(sst->combineMode & SST_CM_DISABLE_CHROMA_SUBSTITUTION)) {	// if special chroma/alpha mode

	    GDBG_INFO(178,"    texelA pre-chroma RGBA =%3x%3x%3x%3x\n",A[0],A[1],A[2],A[3]);
	    GDBG_INFO(178,"    texelB pre-chroma RGBA =%3x%3x%3x%3x\n",B[0],B[1],B[2],B[3]);
	    GDBG_INFO(178,"    texelC pre-chroma RGBA =%3x%3x%3x%3x\n",C[0],C[1],C[2],C[3]);
	    GDBG_INFO(178,"    texelD pre-chroma RGBA =%3x%3x%3x%3x\n",D[0],D[1],D[2],D[3]);

	    mask = 0;
	    if (sstChromaTest(A[0],A[1],A[2],sst->chromaKey,sst->chromaRange)) mask |= 8;
	    if (sstChromaTest(B[0],B[1],B[2],sst->chromaKey,sst->chromaRange)) mask |= 4;
	    if (sstChromaTest(C[0],C[1],C[2],sst->chromaKey,sst->chromaRange)) mask |= 2;
	    if (sstChromaTest(D[0],D[1],D[2],sst->chromaKey,sst->chromaRange)) mask |= 1;

	    // force alpha to 0xFF (solid) by default
	    A[3] = B[3] = C[3] = D[3] = 0xFF;

	    // set alpha=0 if chroma tests passes
	    if (mask & 8) A[3] = 0;
	    if (mask & 4) B[3] = 0;
	    if (mask & 2) C[3] = 0;
	    if (mask & 1) D[3] = 0;

	    // if not color subsituting then blacken out color too
	    if (!(sst->chromaRange & SST_ENCOLORSUBSTITUTION)) {
		if (mask & 8) A[0] = A[1] = A[2] = 0;
		if (mask & 4) B[0] = B[1] = B[2] = 0;
		if (mask & 2) C[0] = C[1] = C[2] = 0;
		if (mask & 1) D[0] = D[1] = D[2] = 0;
	    }
	}
	u0 = uf;		// use u0,v0 as fracs for Alpha
	v0 = vf;
	if (biA == 0) {		// adjust weights for point sampling
	    if (u0 >= 0x80) u0 = 0x100; else u0 = 0;
	    if (v0 >= 0x80) v0 = 0x100; else v0 = 0;
	}
	out[3] = bilinearFilter(sst,u0,v0,A[3],B[3],C[3],D[3]);

	if (biRGB == 0) {	// adjust weights for point sampling
	    if (uf >= 0x80) uf = 0x100; else uf = 0;
	    if (vf >= 0x80) vf = 0x100; else vf = 0;
	}
	else			// color substitution according to JimM
	if (sst->chromaRange & SST_ENCHROMAKEY_TMU &&
	    !(sst->combineMode & SST_CM_DISABLE_CHROMA_SUBSTITUTION)) {	// if special chroma/alpha mode
	    GDBG_INFO(178,"case %d, alpha=0x%x, uf=0x%x, vf=0x%x\n",mask,out[3],uf,vf);
	    if (sst->chromaRange & SST_ENCOLORSUBSTITUTION)
	    switch (mask) {
		// |AB|	the texels are arranged like this
		// |CD|	with (0,0) in the upper left
		case 0:			// JIM:7 all failed
		        break;
		case 0xF:		// JIM:6 all passed
		        if (cp->info->tmuRevision > 4) {
			  if (uf < 0x80)
			    uf = 0;       // snap S to 0
			  else
			    uf = 0x100;   // snap S to 1
			  if (vf < 0x80)
			    vf = 0;       // snap T to 0
			  else 
			    vf = 0x100;   // snap T to 1
		        }
			break;
		case 3:	vf = 0;		// JIM:2/11 CD passed
			break;
		case 5:	uf = 0;			// BD passed
			break;
		case 0xA: uf = 0x100;		// AC passed
			break;
		case 0xC: vf = 0x100;		// AB passed
			break;
		case 1:			// JIM:4/5/12 D passed
		    if (uf < 0x80) {
			if (vf < 0x80) {	// in A
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
			if (vf < 0x80) {	// in B
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
		    if (uf < 0x80) {
			if (vf < 0x80) {	// in A
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
			if (vf < 0x80) {	// in B
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
		    if (uf < 0x80) {
			if (vf < 0x80) {	// in A
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
			if (vf < 0x80) {	// in B
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
		    if (uf < 0x80) {
			if (vf < 0x80) {	// in A
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
			if (vf < 0x80) {	// in B
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
		    if (uf < 0x80) {
			if (vf < 0x80) {	// in A
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
			if (vf < 0x80) {	// in B
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
		    if (uf < 0x80) {
			if (vf < 0x80) {	// in A
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
			if (vf < 0x80) {	// in B
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
			uf = vf = 0;	// point sample A
			break;
		case 0xB:		// JIM:1/8/9 ACD passed
			uf = 0x100;	// point sample B
			vf = 0;
			break;
		case 0xD:		// JIM:1/8/9 ABD passed
			uf = 0;		// point sample C
			vf = 0x100;
			break;
		case 0xE:		// JIM:1/8/9 ABC passed
			uf = vf = 0x100;// point sample D
			break;
	    }
	}

	out[0] = bilinearFilter(sst,uf,vf,A[0],B[0],C[0],D[0]);
	out[1] = bilinearFilter(sst,uf,vf,A[1],B[1],C[1],D[1]);
	out[2] = bilinearFilter(sst,uf,vf,A[2],B[2],C[2],D[2]);
    }
    else {	// POINT SAMPLED case
	FxU8 t8;				// texture access results, 8,16 bits
	FxU16 t16; 
	FxU32 t32;

	if (td->st.hw_w_is_neg && ((tMode & SST_TCLAMPW))) {// clamp on -W
	    u0 = v0 = 0;
	}
	if (sst->tLOD & SST_TMIRRORS) {		// mirror S
	    if (tMode & SST_TCLAMPS) {		// clamp S
		if (u0<0 || u0 > maxU+maxU+1) u0 = 0;
	    }
	    if (u0 & (maxU+1)) u0 = maxU-u0;
	}
	else if (tMode & SST_TCLAMPS) {		// clamp S
	    if (u0 < 0) u0 = 0;
	    if (u0 > maxU) u0 = maxU;
	}
	u0 &= maxU;				// wrap S

	if (sst->tLOD & SST_TMIRRORT) {		// mirror T
	    if (tMode & SST_TCLAMPT) {		// clamp T
		if (v0<0 || v0 > maxV+maxV+1) v0 = 0;
	    }
	    if (v0 & (maxV+1)) v0 = maxV-v0;
	}
	else if (tMode & SST_TCLAMPT) {		// clamp T
	    if (v0 < 0) v0 = 0;
	    if (v0 > maxV) v0 = maxV;
	}
	v0 &= maxV;				// wrap T

	GDBG_INFO(170,"\tclamped u,v = %x.%02x %x.%02x  lod=%x.%02x\n",
		u0>>8,u0&0xFF,v0>>8,v0&0xFF, td->lod>>8, td->lod&0xFF);

	if (bpt == 4 || SST_T8BIT_COMPRESSED(sst->textureMode))
	  {
	    FxU32 texelBlock[4];
	    FxU32 texelBlockAddress;
	    
	    texelBlockAddress = sstTexelAddr(sst, lod, u0>>8, v0>>8, 1);
	    GDBG_INFO(187, "texelBlockAddress=0x%x  u=%d  v=%d\n",
		      texelBlockAddress, u0>>8, v0>>8);

	    csimReadMem128(cp, texelBlockAddress, texelBlock);
	    csimDecompressTexel(texelBlock, tMode, u0>>8, v0>>8, out);
	  }
	else if (bpt == 8) {	
	    t8 = (FxU8)csimReadMem8(cp,sstTexelAddr(sst,lod,u0>>8,v0>>8,1));
	}
	else if (bpt == 16) {
	    t16 = (FxU16)csimReadMem16(cp,sstTexelAddr(sst,lod,u0>>8,v0>>8,1));
	    t8 = (FxU8)t16;
	}
	else //bpt==32
	  {
	    t32 = (FxU32)csimReadMem32(cp,sstTexelAddr(sst,lod,u0>>8,v0>>8,1));
	  }

	if(!(tMode & SST_COMPRESSED_TEXTURES))
	  {  //Non-compressed texture formats (i.e. original Napalm formats)	    
	    //Compressed formats have already been converted to 32bpt format above
	    switch (tMode & SST_TFORMAT) {
	    case SST_RGB332:
	      _sstRgba332to8888(out,t8);
	      break;
	    case SST_YIQ422:
	      nccTab = sstValidateNcc(sst);	// validate and return NCC
	      GDBG_INFO(176,"\t\tYIQ422 texel = %02x\n",t8);
	      _sstYab422to8888(nccTab,out,t8);
	      break;
	    case SST_A8:
	      out[0] = out[1] = out[2] = out[3] = t8;
	      break;
	    case SST_I8:
	      out[0] = out[1] = out[2] = t8;
	      out[3] = 0xFF;
	      break;
	    case SST_AI44:
	      _sstAi44to8888(out,t8);
	      break;
	    case SST_P8:
	      _sstPal256to8888(&td->pal256, out,t8);
	      break;
	    case SST_P8_ARGB6666:
	      _sstPal6666to8888(&td->pal256, out,t8);
	      break;

	    case SST_RGB565:
	      _sstRgba565to8888(out,t16);
	      break;
	    case SST_ARGB1555:
	      _sstRgba1555to8888(out,t16);
	      break;
	    case SST_ARGB4444:
	      _sstRgba4444to8888(out,t16);
	      break;
	    case SST_ARGB8332:
	      _sstRgba332to8888(out,t8);
	      out[3] = t16>>8;
	      break;
	    case SST_AYIQ8422:
	      nccTab = sstValidateNcc(sst);	// validate and return NCC
	      GDBG_INFO(176,"\t\tAYIQ422 texel = %04x\n",t16);
	      _sstYab422to8888(nccTab,out,t8);
	      out[3] = t16>>8;
	      break;
	    case SST_AI88:
	      out[0] = out[1] = out[2] = t8;
	      out[3] = t16>>8;
	      break;
	    case SST_AP88:
	      _sstPal256to8888(&td->pal256, out, t8);
	      out[3] = t16>>8;
	      break;
	    case SST_ARGB8888:
	      out[0] = (unsigned char)(t32 >> 16) & 0xFF;  //Red
	      out[1] = (unsigned char)(t32 >> 8)  & 0xFF;  //Green
	      out[2] = (unsigned char)(t32 >> 0)  & 0xFF;  //Blue
	      out[3] = (unsigned char)(t32 >> 24) & 0xFF;  //Alpha
	      break;

	    default:
	      GDBG_ERROR("sstTextureColor", "Oh poop, not supposed to be here either!");
	      break;
	    }
	  }
    }
}

//----------------------------------------------------------------------
// compute the texture color, do the following steps
//	1) compute reciprocal of 1/w iterator
//	2) if texturing is enabled, compute new texture color
//	3) combine texture color with incoming RGBA (texture compositing)
// INPUT: the incoming color is other
// OUTPUT: leave resulting color in out
//----------------------------------------------------------------------
void _getAndClampIterators(SstRegs *sst, unsigned char iterators[]);

void
sstTrexColor(SstRegs *sst, unsigned char out[4], unsigned char other[4])
{
    unsigned long tMode = sst->textureMode;
    unsigned long tril;
    TmuData *td = TMU_PRIVATE(sst);
    unsigned char iterators[4];
    unsigned char mselect_7[4];
    unsigned char textureColor[4];
    unsigned char c_other[3], a_other;
    unsigned char c_local[3], a_local;
    unsigned char otherTextureColor[4];

    //Copy down the value of other before it is modified
    otherTextureColor[0]=other[0];
    otherTextureColor[1]=other[1];
    otherTextureColor[2]=other[2];
    otherTextureColor[3]=other[3];

    // do this if not PASSing or if using new combineMode register
    if (SST_TREX_ACTIVE(sst->textureMode) || (sst->combineMode & SST_CM_USE_COMBINE_MODE))
      {
	sstTextureColor(sst,textureColor);
	GDBG_INFO(171,"\t-RGBA into TREX.%d =%3x%3x%3x%3x\n",
		  td->myNumber,
		  other[0], other[1], other[2], other[3]);
	GDBG_INFO(172,"\t-RGBA texture map =%3x%3x%3x%3x\n",
		  textureColor[0],textureColor[1],textureColor[2],textureColor[3]);
	_getAndClampIterators(sst,iterators);

    //------------------------------------------------------------------
    // perform RGB selection for the "mselect_7" color
    //------------------------------------------------------------------    
	switch(sst->combineMode & SST_CM_TC_MSELECT_7)
	  {
	  case SST_CM_TC_MSELECT_7_LOCAL_TRGB:
	    mselect_7[0] = textureColor[0];
	    mselect_7[1] = textureColor[1];
	    mselect_7[2] = textureColor[2];
	    break;
	  case SST_CM_TC_MSELECT_7_ZERO:
	  case SST_CM_TC_MSELECT_7_ZERO3:
	    mselect_7[0] = 0;
	    mselect_7[1] = 0;
	    mselect_7[2] = 0;
	    break;
	  case SST_CM_TC_MSELECT_7_OTHER_TRGB:
	    mselect_7[0] = other[0];
	    mselect_7[1] = other[1];
	    mselect_7[2] = other[2];
	    break;
	  case SST_CM_TC_MSELECT_7_IRGB:
	    mselect_7[0] = iterators[0];
	    mselect_7[1] = iterators[1];
	    mselect_7[2] = iterators[2];
	    break;
	  case SST_CM_TC_MSELECT_7_IA:
	    mselect_7[0] = iterators[3];
	    mselect_7[1] = iterators[3];
	    mselect_7[2] = iterators[3];
	    break;
	  case SST_CM_TC_MSELECT_7_CR_RGB:
	    mselect_7[0] = (unsigned char)((sst->chromaRange >> 16) & 0xFF);
	    mselect_7[1] = (unsigned char)((sst->chromaRange >> 8)  & 0xFF);
	    mselect_7[2] = (unsigned char)((sst->chromaRange >> 0)  & 0xFF);
	    break;
	  case SST_CM_TC_MSELECT_7_CR_A:
	    mselect_7[0] = (unsigned char)((sst->chromaRange >> 24) & 0xFF);
	    mselect_7[1] = (unsigned char)((sst->chromaRange >> 24) & 0xFF);
	    mselect_7[2] = (unsigned char)((sst->chromaRange >> 24) & 0xFF);
	    break;
	  default:
	    GDBG_ERROR("sstTrexColor", "Invalid SST_CM_TC_MSELECT_7 value");
	    exit(-1);
	    break;
	  }
    
	//------------------------------------------------------------------
	// perform RGBA selection for the "(c|a)_other" color
	//------------------------------------------------------------------    
    
    //Do RGB
	switch(sst->combineMode & SST_CM_TC_OTHERSELECT)
	  {
	  case SST_CM_TC_OTHERSELECT_OTHER_TRGB:
	    c_other[0] = other[0];
	    c_other[1] = other[1];
	    c_other[2] = other[2];
	    break;
	  case SST_CM_TC_OTHERSELECT_OTHER_TA:
	    c_other[0] = other[3];
	    c_other[1] = other[3];
	    c_other[2] = other[3];
	    break;
	  case SST_CM_TC_OTHERSELECT_LOCAL_TRGB:
	    c_other[0] = textureColor[0];
	    c_other[1] = textureColor[1];
	    c_other[2] = textureColor[2];
	    break;
	  case SST_CM_TC_OTHERSELECT_LOCAL_TA:
	    c_other[0] = textureColor[3];
	    c_other[1] = textureColor[3];
	    c_other[2] = textureColor[3];
	    break;
	  case SST_CM_TC_OTHERSELECT_IRGB:
	    c_other[0] = iterators[0];
	    c_other[1] = iterators[1];
	    c_other[2] = iterators[2];
	    break;
	  case SST_CM_TC_OTHERSELECT_IA:
	    c_other[0] = iterators[3];
	    c_other[1] = iterators[3];
	    c_other[2] = iterators[3];
	    break;
	  case SST_CM_TC_OTHERSELECT_CR_RGB:
	    c_other[0] = (unsigned char)((sst->chromaRange >> 16) & 0xFF);
	    c_other[1] = (unsigned char)((sst->chromaRange >> 8)  & 0xFF);
	    c_other[2] = (unsigned char)((sst->chromaRange >> 0)  & 0xFF);
	    break;
	  case SST_CM_TC_OTHERSELECT_CR_A:
	    c_other[0] = (unsigned char)((sst->chromaRange >> 24) & 0xFF);
	    c_other[1] = (unsigned char)((sst->chromaRange >> 24) & 0xFF);
	    c_other[2] = (unsigned char)((sst->chromaRange >> 24) & 0xFF);
	    break;
	  default:
	    GDBG_ERROR("sstTrexColor", "Invalid SST_CM_TC_OTHERSELECT value");
	    exit(-1);
	    break;
	  }
    
	//Do Alpha
	switch(sst->combineMode & SST_CM_TCA_OTHERSELECT)
	  {
	  case SST_CM_TCA_OTHERSELECT_OTHER_TA:
	    a_other = other[3];
	    break;
	  case SST_CM_TCA_OTHERSELECT_LOCAL_TA:
	    a_other = textureColor[3];
	    break;
	  case SST_CM_TCA_OTHERSELECT_IA:
	    a_other = iterators[3];
	    break;
	  case SST_CM_TCA_OTHERSELECT_CR_A:
	    a_other = (unsigned char)((sst->chromaRange >> 24) & 0xFF);
	    break;
	  default:
	    GDBG_ERROR("sstTrexColor", "Invalid SST_CM_TCA_OTHERSELECT value");
	    exit(-1);
	    break;
	  }
    
    
	//------------------------------------------------------------------
	// perform RGBA selection for the "(c|a)_local" color
	//------------------------------------------------------------------    
	//Do RGB
	switch(sst->combineMode & SST_CM_TC_LOCALSELECT)
	  {
	  case SST_CM_TC_LOCALSELECT_LOCAL_TRGB:
	    c_local[0] = textureColor[0];
	    c_local[1] = textureColor[1];
	    c_local[2] = textureColor[2];
	    break;
	  case SST_CM_TC_LOCALSELECT_LOCAL_TA:
	    c_local[0] = textureColor[3];
	    c_local[1] = textureColor[3];
	    c_local[2] = textureColor[3];
	    break;
	  case SST_CM_TC_LOCALSELECT_OTHER_TRGB:
	    c_local[0] = other[0];
	    c_local[1] = other[1];
	    c_local[2] = other[2];
	    break;
	  case SST_CM_TC_LOCALSELECT_OTHER_TA:
	    c_local[0] = other[3];
	    c_local[1] = other[3];
	    c_local[2] = other[3];
	    break;
	  case SST_CM_TC_LOCALSELECT_IRGB:
	    c_local[0] = iterators[0];
	    c_local[1] = iterators[1];
	    c_local[2] = iterators[2];
	    break;
	  case SST_CM_TC_LOCALSELECT_IA:
	    c_local[0] = iterators[3];
	    c_local[1] = iterators[3];
	    c_local[2] = iterators[3];
	    break;
	  case SST_CM_TC_LOCALSELECT_CK_RGB:
	    c_local[0] = (unsigned char)((sst->chromaKey >> 16) & 0xFF);
	    c_local[1] = (unsigned char)((sst->chromaKey >> 8)  & 0xFF);
	    c_local[2] = (unsigned char)((sst->chromaKey >> 0)  & 0xFF);
	    break;
	  case SST_CM_TC_LOCALSELECT_CK_A:
	    c_local[0] = (unsigned char)((sst->chromaKey >> 24) & 0xFF);
	    c_local[1] = (unsigned char)((sst->chromaKey >> 24) & 0xFF);
	    c_local[2] = (unsigned char)((sst->chromaKey >> 24) & 0xFF);
	    break;	    
	  default:
	    GDBG_ERROR("sstTrexColor", "Invalid SST_CM_TC_LOCALSELECT value");
	    exit(-1);
	    break;
	  }
    
	//Do Alpha
	switch(sst->combineMode & SST_CM_TCA_LOCALSELECT)
	  {
	  case SST_CM_TCA_LOCALSELECT_LOCAL_TA:
	    a_local = textureColor[3];
	    break;
	  case SST_CM_TCA_LOCALSELECT_OTHER_TA:
	    a_local = other[3];
	    break;
	  case SST_CM_TCA_LOCALSELECT_IA:
	    a_local = iterators[3];
	    break;
	  case SST_CM_TCA_LOCALSELECT_CK_A:
	    a_local = (unsigned char)((sst->chromaKey >> 24) & 0xFF);
	    break;
	  default:
	    GDBG_ERROR("sstTrexColor", "Invalid SST_CM_TCA_LOCALSELECT value");
	    exit(-1);
	    break;
	  }

    
	//Reconstruct original function input format
	other[0] = c_other[0];
	other[1] = c_other[1];
	other[2] = c_other[2];
	other[3] = a_other;
    
	out[0] = c_local[0];
	out[1] = c_local[1];
	out[2] = c_local[2];
	out[3] = a_local;
    
	tril = (tMode & SST_TRILINEAR) && lodODD;
	// GDBG_INFO(173,"til=%d lodOdd=%d\n",tril,lodODD);
	//    if (((tMode & SST_TCOMBINE) != SST_TC_REPLACE) ||
	//(sst->combineMode & SST_CM_USE_COMBINE_MODE))
	//{
	sstCompositeRGB(sst, FXFALSE, out, other, iterators, mselect_7, 
			textureColor, otherTextureColor,
			tMode, tril ^ !(tMode & SST_TC_REVERSE_BLEND) ? 0xFF : 0);
	
	//      }
	//    if (((tMode & SST_TACOMBINE) != SST_TCA_REPLACE) ||
	//	(sst->combineMode & SST_CM_USE_COMBINE_MODE))
	//      {
	sstCompositeA(sst, FXFALSE, out, other, iterators, 
		      textureColor[3], otherTextureColor[3], tMode, 
		      tril ^ !(tMode & SST_TCA_REVERSE_BLEND) ? 0xFF : 0);
	//      }
	GDBG_INFO(173,"\t-RGBA combine out =%3x%3x%3x%3x\n",
		  out[0],out[1],out[2],out[3]);
      }
    else {					// if PASSing colors thru
      out[0] = other[0];			// then simply copy them to out
      out[1] = other[1];
      out[2] = other[2];
      out[3] = other[3];
    }
}

#ifdef HAL_HSIM

#include "tstbench.h"

static char *errorMsg = "called, but not running on a PC\n";

//----------------------------------------------------------------------
// All the code below is to implement a CSIM trex texel output checking fifo

#define DISABLE_TF_FIFO_CHECKING                // disable tf fifo checking for H3

#define TREX_FIFO_SIZE (256*1024)

static int trexFifoHead, trexFifoTail;		// FIFO head and tail pointers

typedef struct {				// a FIFO entry
	unsigned long color;
	short x;
	short y;
	unsigned short triangle;
	unsigned short pixel;
} Tfifo;

static Tfifo *trexFifo;				// the actual FIFO

//----------------------------------------------------------------------
// pop one entry out of the CSIM trex fifo and return it
static
Tfifo *trexFifoPop(void)
{
    Tfifo *tf = trexFifo+trexFifoHead;
#ifdef DISABLE_TF_FIFO_CHECKING
  return NULL;
#endif
    if (trexFifoTail == trexFifoHead) {		// if the queue is empty then
	GDBG_ERROR("trexFifoPop",		// its an error
			"CSIM's trex fifo is empty, HSIM has %d entries\n",
			TREX0_TF_STATUS());
    }
    trexFifoHead = (trexFifoHead+1) % TREX_FIFO_SIZE;
    return tf;
}

//----------------------------------------------------------------------
// add an entry onto the CSIM trex fifo, tail points to empty entry
static
void trexFifoPush(unsigned long rgba, int x, int y, int tcount, int pcount)
{
#ifdef DISABLE_TF_FIFO_CHECKING
  return;
#endif
    trexFifo[trexFifoTail].color = rgba;	// create an entry
    trexFifo[trexFifoTail].x = x;
    trexFifo[trexFifoTail].y = y;
    trexFifo[trexFifoTail].triangle = tcount;
    trexFifo[trexFifoTail].pixel = pcount;

    trexFifoTail = (trexFifoTail + 1) % TREX_FIFO_SIZE;
}

// compare an entry
void trexFifoCompare(Tfifo *tf, unsigned long hwCol)
{
#ifdef DISABLE_TF_FIFO_CHECKING
  return;
#endif
    GDBG_INFO(134,"trexFifoCompare: triangle #%d, pixel #%d  xy[%d,%d] argb=0x%08x\n",
			tf->triangle,tf->pixel,tf->x,tf->y,hwCol);
    // NOTE: GMT we only compare these in standalone mode
    // CSIM does not do col8 rendering, we basically assume that standalone
    // mode has an old triangle engine in the TMU chip
    if (halInfo.hsim & HSIM_TREX_STANDALONE)
    if (tf->color != hwCol) {
	gdbg_printf("ERROR(hsim): expecting TREX0_TF_RD to be %d(0x%08x)"
			" but read %d(0x%08x)\n",
			tf->color,tf->color,hwCol,hwCol);
	gdbg_printf("\t\ttriangle #%d, pixel #%d  xy[%d,%d]\n",
			tf->triangle,tf->pixel,tf->x,tf->y);
#ifdef __unix__
	DIAG_INCERROR();	// diag hack, we can call this on non-DLL platforms
#endif
    }
}

// if there's any texels in the HSIM trex output fifo then check them first
void trexFifoCheckAllHsim(void)
{
#ifdef DISABLE_TF_FIFO_CHECKING
  return;
#endif
    GDBG_INFO(134,"trexFifoCheckAllHsim\n");
    while (TREX0_TF_STATUS()) {
	trexFifoCompare(trexFifoPop(),TREX0_TF_RD());
    }
}


void trexFifoCheckAllCsim(void)
{
#ifdef DISABLE_TF_FIFO_CHECKING
  return;
#endif
    GDBG_INFO(134,"trexFifoCheckAllCsim\n");
    while (trexFifoTail != trexFifoHead) {
	trexFifoCompare(trexFifoPop(),TREX0_TF_RD());
    }
}

//----------------------------------------------------------------------
// add an entry to the FIFO of texels that we expect to come out of TREX
// if this FIFO gets full then issue a warning, and advance simulation
// until it has some room in it
//----------------------------------------------------------------------
void trexAddToCheckFifo(unsigned long rgba, int x, int y, int tcount, int pcount)
{
    int timeOut=0;

#ifdef DISABLE_TF_FIFO_CHECKING
  static int firstTime = 1;
  if ( firstTime ) {
    GDBG_INFO(1,"trexAddToCheckFifo: tf fifo checking is DISABLED\n");
    firstTime = 0;
  }
  return;
#endif

    if (!trexFifo) {			// first time init code
	trexFifo = (Tfifo *)malloc(TREX_FIFO_SIZE * sizeof(*trexFifo));
	trexFifoHead = trexFifoTail = 0;
    }
    GDBG_INFO(134,"trexAddToCheckFifo: triangle #%d, pixel #%d  xy[%d,%d] argb=0x%08x\n",
			tcount,pcount,x,y,rgba);
    trexFifoCheckAllHsim();

    // if my fifo is full, then advance simulation time until there's an
    // entry in the HSIM fifo that I can read
    while (((trexFifoTail + 1) % TREX_FIFO_SIZE) == trexFifoHead) {
	gdbg_printf("WARNING: trex check fifo is full, advancing simulation\n");
	GDBG_INFO(134,"\tcalling SIM_ADVANCE()\n");
	SIM_ADVANCE();
	if (++timeOut > 1000) {
	    GDBG_ERROR("trexAddToCheckFifo", "timed out waiting for HSIM\n");
	    DIAG_FAIL();
	}
	trexFifoCheckAllHsim();
    }
    trexFifoPush(rgba, x, y, tcount, pcount);
}

#endif

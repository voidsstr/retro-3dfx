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
** $Date: 10/11/00 8:19:23 PM$
*/
#include <assert.h>

#include "allocate.h"
#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

#define MAX_WORDS 1024
void
main (int argc, char **argv)
{
    int n,i,j,k,format,trex;
    Triangle *t;
    FxU32 scramble[MAX_WORDS], tarray[MAX_WORDS], aarray[MAX_WORDS];
    FxU32 wordSelectArray[MAX_WORDS];  //Used for compressed textures (only in tiled mode)
    FxU32 got;
    SstRegs *sst;
    CsimPrivate *cpriv;

    sst = SST_BEGIN(argc,argv);
    cpriv = CSIM_PRIVATE(diago.sstCSIM);

    if(diago.bigAssTextures)
      t = buildTriangle(2048, 2048);
    else
      t = buildTriangle(256, 256);

    t->next = NULL;

    //We don't need to color buffers, so unallocate them
    unlockByName("Color Buffer #0");
    unlockByName("Color Buffer #1");
    unlockByName("Color Buffer #2");
    unlockByName("Aux Buffer");
    unallocateAll();
    memoryMap();

    // make rawlfb all linear
    {
      SstIORegs *sstio;
      int tStride = 1;
      int aStride = 0;
      sstio = (SstIORegs *)(SST_IO_ADDRESS(sst));
      SET(sstio->lfbMemoryConfig,
	  ((0x1FFF)<<SST_RAW_LFB_TILE_BEGIN_PAGE_SHIFT) |
	  (tStride<<SST_RAW_LFB_TILE_STRIDE_SHIFT) |
	  (aStride<<SST_RAW_LFB_ADDR_STRIDE_SHIFT) |
	  (3<<23));
    }

    // sanity check some arguments
    if (diago.tsize < 0)
	diago.tsize = -diago.tsize;

    if ((diago.tsize + 1)*(diago.tsize + 1) > MAX_WORDS)
      diago.tsize = (int)sqrt(MAX_WORDS-1);
    
    GDBG_INFO(0, "diago.tsize = %d\n", diago.tsize);

    while (DIAG_STARTPASS()) {			// for each pass
	for (n=0; n<50; n++) {
	    FxI32 baseAddr;
	    FxI32 mipmapAddress;
	    FxU32 lod, logWidth, logHeight, largestPossibleLod, logAspectRatio, bitsPerTexel;
	    FxU32 *startAddr;
	    FxI32 texturePort;
	    FxU32 mungedBase;
	    FxI32 tiled = diago.ytiled;
	    FxBool bigAssTexture, wideTexture, compressedTexture;
	    if ( tiled < 0 ) 
	      tiled = iRandom(1);
	    

	    // random 8-bit/16-bit uncompressed, NCC or not
	    i = iRandom(1);
	    if(diago.tex32)
	      j=iRandom(1);
	    else
	      j=0;
	    k = iRandom(1);
	    format = texRandomFormat(i,j,k);
	    t->tex->tMode = format | SST_TC_REPLACE | SST_TCA_REPLACE;

	    if(t->tex->tMode & SST_COMPRESSED_TEXTURES)
	      compressedTexture = FXTRUE;
	    else
	      compressedTexture = FXFALSE;

	    bitsPerTexel = sstBitsPerTexel(t->tex->tMode);

	    //Select if the texture is big or not
	    if(diago.bigAssTextures)
	      {
		if(iRandom(1))
		  bigAssTexture = FXTRUE;
		else
		  bigAssTexture = FXFALSE;
	      }
	    else
	      bigAssTexture = FXFALSE;

	    //Pick an lod level
	    if(bigAssTexture)
	      largestPossibleLod = 11;
	    else
	      largestPossibleLod = 8;
	    
	    logWidth = iRandom(largestPossibleLod);
	    logHeight = iRandom(largestPossibleLod);

	    //We can only download tiled textures up to 1024x1024
	    if(tiled)
	      {
		if(logWidth > 10)
		  logWidth = 10;
		if(logHeight > 10)
		  logHeight = 10;
	      }

	    if(logWidth > logHeight)
	      {
		wideTexture = FXTRUE;
		logAspectRatio = logWidth - logHeight;

		if(logAspectRatio > 3)
		  {
		    logHeight = logWidth - 3;
		    logAspectRatio = 3;
		  }
		lod = largestPossibleLod - logWidth;
	      }
	    else
	      {
		wideTexture = FXFALSE;
		logAspectRatio = logHeight - logWidth;
		
		if(logAspectRatio > 3)
		  {
		    logWidth = logHeight - 3;
		    logAspectRatio = 3;
		  }

		lod = largestPossibleLod - logHeight;
	      }

	    GDBG_INFO(2, "%dx%d %s texture: lod=%d\n",
		      (1<<logWidth), (1<<logHeight), (tiled ? "tiled" : "linear"), lod);
	    GDBG_INFO(2, "wideTexture=%d bigAssTexture=%d compressedTexture=%d\n",
		      wideTexture, bigAssTexture, compressedTexture);

	    //Select a texture port to use.
	    if(bigAssTexture || SST_T32BIT(t->tex->tMode))
	      {
		trex = 0;
		gdbg_info(1,"TMU #0 because bigAssTexture or 32bpt\n");
		texturePort = SST_TEX2_ADDRESS(sst);
	      }
	    else
	      {
		trex = diago.trex;
		if ( trex < 0 )
		  trex = iRandom(-trex);
		gdbg_info(1,"TMU #%d\n",trex);
		texturePort = trex == 0 ? SST_TEX0_ADDRESS(sst) : SST_TEX1_ADDRESS(sst);
	      }
	    
	    //Set up tLOD
	    SET(SST_TREX(sst,trex)->tLOD, (wideTexture ? SST_LOD_S_IS_WIDER : 0x0) | 
		(logAspectRatio<<SST_LOD_ASPECT_SHIFT) | (bigAssTexture ? SST_TBIG : 0));


	    //----------------------------------------------------------
	    // TILED MODE
	    if ( tiled ) {
	      int u, v, du;
	      int a, tStride;
	      int depth;
	      int sz = 1<<iRandom(2);     // size of download in bytes: 1, 2, or 4
	      FxI32 umin, umax, vmin, vmax;   //Region of lod to download
	      FxU8 *startAddr8;
	      FxU16 *startAddr16;
	      FxU32 *startAddr32;
	      FxU32 allocatedSpace;
	      FxI32 textureSize;
	      tiledStruct mipmap;
	      
	      // Figure out how big texture is
	      mipmap = sstTiledMipMapOffset(lod, logAspectRatio, sstBitsPerTexel(t->tex->tMode), bigAssTexture, wideTexture, compressedTexture);
	      
	      if(t->tex->tMode & SST_COMPRESSED_TEXTURES)
		{
		  depth = 16;
		  sz = 16;
		  convertToMicroTile(t->tex->tMode, &mipmap.uoff, &mipmap.voff);
		  convertToMicroTile(t->tex->tMode, &mipmap.utot, &mipmap.vtot);
		}
	      else
		depth = sstBitsPerTexel(t->tex->tMode) / 8;

	      // Pick a tile stride
	      tStride = (mipmap.utot * depth) / SST_TILE_WIDTH + iRandom(8);
	      textureSize = tStride * SST_TILE_SIZE * (mipmap.vtot / SST_TILE_HEIGHT + 2);
	      
	      //Alocate space for the texture
	      allocatedSpace = allocate(textureSize + 2*SST_TILE_SIZE, "tiled", randomPlacement);
	      unallocateByName("tiled");
	      //Page align the f'er
	      allocatedSpace = (allocatedSpace + (SST_TILE_SIZE - 1)) & (~(SST_TILE_SIZE - 1));
	      allocatedSpace += SST_TILE_SIZE;
	      
	      //Find the base address
	      baseAddr = tiledAddress(allocatedSpace, tStride, depth, -mipmap.uoff, -mipmap.voff);
	      mungedBase = SST_TEXTURE_MUNGE_ADDRESS(baseAddr) | 
		((tStride << SST_TEXTURE_TILESTRIDE_SHIFT) & SST_TEXTURE_TILESTRIDE) | SST_TEXTURE_IS_TILED;		
	      
	      GDBG_INFO(2, "tiled texture base address = 0x%x\n", baseAddr);
	      GDBG_INFO(2, "Allocated @ 0x%x, tStride = %d  textureSize = %d  depth = %d\n", allocatedSpace,
			tStride, textureSize, depth);
	      
	      // select a rectangular region to download
	      umin = iRandom((1<<logWidth) - 1);
	      umax = rRandom(umin, (1<<logWidth) - 1);
	      vmin = iRandom((1<<logHeight) - 1);
	      vmax = rRandom(vmin, (1<<logHeight) - 1);
	      
	      if(umax - umin > diago.tsize)
		umax = umin + diago.tsize;
	      if(vmax - vmin > diago.tsize)
		vmax = vmin + diago.tsize;	     

	      //Convert to microtiles if compressed
	      if(t->tex->tMode & SST_COMPRESSED_TEXTURES)
		{
		  convertToMicroTile(t->tex->tMode, &umin, &vmin);
		  convertToMicroTile(t->tex->tMode, &umax, &vmax);
		}

	      GDBG_INFO(2, "Mipmap download region = [%d,%d]x[%d,%d]\n",
			umin, vmin, umax, vmax);

	      du = sz<depth ? 1 : sz/depth;
	      	      
	      // generate texel data
	      for (j=0, v=0; v <= vmax-vmin; v++) {
		for (u=0; u <= umax-umin; u+=du, j++) {
		  tarray[j] = iRandom(SST_MASK(sz*8));
		  scramble[j] = j;

		  if(bigAssTexture)
		    {
		      if (depth == 1) 
			aarray[j] = (lod<<SST_TEXTURE_BIG_LOD8_SHIFT) | 
			  ((umin+u)<<SST_TEXTURE_BIG_S8_SHIFT) | ((vmin+v)<<SST_TEXTURE_BIG_T8_SHIFT);
		      else if (depth == 2)
			aarray[j] = (lod<<SST_TEXTURE_BIG_LOD16_SHIFT) | 
			  ((umin+u)<<SST_TEXTURE_BIG_S16_SHIFT) | ((vmin+v)<<SST_TEXTURE_BIG_T16_SHIFT);
		      else if (depth == 4)
			aarray[j] = (lod<<SST_TEXTURE_BIG_LOD32_SHIFT) | 
			  ((umin+u)<<SST_TEXTURE_BIG_S32_SHIFT) | ((vmin+v)<<SST_TEXTURE_BIG_T32_SHIFT);
		      else if(SST_T4BIT_COMPRESSED(t->tex->tMode))
			{
			  wordSelectArray[j] = iRandom(3);
			  aarray[j] = (lod<<SST_TEXTURE_BIG_LOD4_COMPRESSED_SHIFT) |
			    ((umin+u) << SST_TEXTURE_BIG_S4_COMPRESSED_SHIFT) |
			    ((vmin+v) << SST_TEXTURE_BIG_T4_COMPRESSED_SHIFT) |
			    (wordSelectArray[j] << SST_TEXTURE_BIG_WORDSELECT4_COMPRESSED_SHIFT);
			  tarray[j] = iRandom(0xFFFFFFFF);
			}
		      else if(SST_T8BIT_COMPRESSED(t->tex->tMode))
			{
			  wordSelectArray[j] = iRandom(3);
			  aarray[j] = (lod<<SST_TEXTURE_BIG_LOD8_COMPRESSED_SHIFT) |
			    ((umin+u) << SST_TEXTURE_BIG_S8_COMPRESSED_SHIFT) |
			    ((vmin+v) << SST_TEXTURE_BIG_T8_COMPRESSED_SHIFT) |
			    (wordSelectArray[j] << SST_TEXTURE_BIG_WORDSELECT8_COMPRESSED_SHIFT);
			  tarray[j] = iRandom(0xFFFFFFFF);
			}
		      else
			{
			  GDBG_ERROR("texmem", "God damn shizam! %s(%d)\n", __FILE__, __LINE__);
			  DIAG_FAIL();
			}
		    }
		  else
		    {
		      if (depth == 1) 
			aarray[j] = (lod<<SST_TEXTURE_LOD8_SHIFT) | ((umin+u)<<SST_TEXTURE_S8_SHIFT) | ((vmin+v)<<SST_TEXTURE_T8_SHIFT);
		      else if (depth == 2)
			aarray[j] = (lod<<SST_TEXTURE_LOD16_SHIFT) | ((umin+u)<<SST_TEXTURE_S16_SHIFT) | ((vmin+v)<<SST_TEXTURE_T16_SHIFT);
		      else if (depth == 4)
			aarray[j] = (lod<<SST_TEXTURE_LOD32_SHIFT) | ((umin+u)<<SST_TEXTURE_S32_SHIFT) | ((vmin+v)<<SST_TEXTURE_T32_SHIFT);
		      else if(SST_T4BIT_COMPRESSED(t->tex->tMode))
			{
			  wordSelectArray[j] = iRandom(3);
			  aarray[j] = (lod<<SST_TEXTURE_LOD4_COMPRESSED_SHIFT) |
			    ((umin+u) << SST_TEXTURE_S4_COMPRESSED_SHIFT) |
			    ((vmin+v) << SST_TEXTURE_T4_COMPRESSED_SHIFT) |
			    (wordSelectArray[j] << SST_TEXTURE_WORDSELECT4_COMPRESSED_SHIFT);
			  tarray[j] = iRandom(0xFFFFFFFF);
			}
		      else if(SST_T8BIT_COMPRESSED(t->tex->tMode))
			{
			  wordSelectArray[j] = iRandom(3);
			  aarray[j] = (lod<<SST_TEXTURE_LOD8_COMPRESSED_SHIFT) |
			    ((umin+u) << SST_TEXTURE_S8_COMPRESSED_SHIFT) |
			    ((vmin+v) << SST_TEXTURE_T8_COMPRESSED_SHIFT) |
			    (wordSelectArray[j] << SST_TEXTURE_WORDSELECT8_COMPRESSED_SHIFT);
			  tarray[j] = iRandom(0xFFFFFFFF);
			}
		      else
			{
			  GDBG_ERROR("texmem", "God damn shizam! %s(%d)\n", __FILE__, __LINE__);
			  DIAG_FAIL();
			}
		    }
		}
	      }
	      
	      //Count the number of texels we're going to render
	      for(j=0, v=0; v<= vmax-vmin; v++)
		for(u=0; u<= umax-umin; u+=du, j++)

	      if (iRandom(3)==0) {
		gdbg_info(3,"scrambling order\n");
		scrambleRandom(j, scramble);
	      }
	      
	      // download texture data thru the texture aperture	      		  	      
	      SET(SST_TREX(sst,trex)->textureMode,t->tex->tMode);
	      SET(SST_TREX(sst,trex)->texBaseAddr,mungedBase);
	      if (diago.halInfo->hsim & HSIM_TREX_BACKDOOR_TEXWRITES)
		sst_idle_really(sst);

	      startAddr8  =  (FxU8 *)(texturePort);
	      startAddr16 = (FxU16 *)(texturePort);
	      startAddr32 = (FxU32 *)(texturePort);

	      for (j=0, v=0; v<= vmax-vmin; v++) {
		for (u=0; u<= umax-umin; u+=du, j++) {
		  k = scramble[j];
		  GDBG_INFO(3,"texel[%d]:  (u,v)=%d,%d  a=0x%x  d=0x%x\n",k,umin+u,vmin+v,aarray[k],tarray[k]);
		  if ( sz == 1 )
		    SET8(startAddr8[aarray[k]],(FxU8)tarray[k]);	// download it
		  else if ( sz == 2 )
		    SET16(startAddr16[aarray[k]>>1],(FxU16)tarray[k]);
		  else if (sz == 4)
		    SET(startAddr32[aarray[k]>>2],(FxU32)tarray[k]);
		  else if(sz == 16)  //For compressed textures
		    SET(startAddr32[aarray[k]>>2], (FxU32)tarray[k]);
		  else
		    assert(0);
		}
	      }

	      // wait for the command to complete
	      sst_idle_really(sst);

	      // read back texels in order
	      startAddr8 = (FxU8 *)(SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET);
	      startAddr16 = (FxU16 *)(SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET);
	      startAddr32 = (FxU32 *)(SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET);
	      baseAddr = SST_TEXTURE_UNMUNGE_ADDRESS(mungedBase);

	      for (j=0, v=0; v <= vmax-vmin; v++) {
		for (u=0; u <= umax-umin; u+=du, j++) {
		  int uu, vv;
		  
		  uu = umin + u + mipmap.uoff;
		  vv = vmin + v + mipmap.voff;
		  		  
		  a = tiledAddress(baseAddr,tStride,depth,uu,vv);

		  if ( sz == 1 )
		    got = GET8(startAddr8[a]);
		  else if ( sz == 2 )
		    got = GET16(startAddr16[a>>1]);
		  else if (sz == 4)
		    got = GET(startAddr32[a>>2]);
		  else if(sz == 16)
		    got = GET(startAddr32[(a>>2) + wordSelectArray[j]]);
		  else 
		    assert(0);

		  if (got != tarray[j]) {
		    gdbg_printf("ERROR: expecting 0x%x, got 0x%x @ x0%x (texel[%d] u,v=%d,%d lod=%d a=0x%x)\n",
				tarray[j],got,startAddr8+a,j,umin+u,vmin+v,lod,a);
		    DIAG_INCERROR();
		  }
		}
	      }
	    }
	    //----------------------------------------------------------
	    // LINEAR MODE
	    else {
	      FxU32 textureSize;
	      FxU32 textureDownloadSize;
	      FxI32 mipmapOffset;
	      FxI32 startOffset;  //Determines what part of the texture is downloaded
	      // download random number of words (10*tsize) at a random location
	      // and then check them by reading them back

	      //Allocate the texture
	      textureSize = sstLinearMipMapSize(lod, logAspectRatio, bitsPerTexel, bigAssTexture, 
						wideTexture, compressedTexture);
	      mipmapAddress = allocate(textureSize + 64 + 8192, "linear", randomPlacement);
	      unallocateByName("linear");  //We don't need to preserve the space
	      mipmapAddress += 8192;   //Allow for some slop
	      mipmapAddress &= ~0xF;			// make it 16 byte aligned
	      
	      GDBG_INFO(2, "linear mipmapAddress = 0x%x\n", mipmapAddress);

	      //Calculate the base address of the texture
	      mipmapOffset = sstLinearMipMapOffset(lod, logAspectRatio, bitsPerTexel, bigAssTexture, 
					     wideTexture, compressedTexture, FXFALSE);
	      baseAddr = mipmapAddress - mipmapOffset;
	      baseAddr &= SST_TEXTURE_FULL_ADDRESS;	

	      GDBG_INFO(2, "mipmapOffset = %d\n", mipmapOffset);
	      
	      GDBG_INFO(2, "linear baseAddress = 0x%x\n", baseAddr);

	      //Figure out how much to download
	      textureDownloadSize = iRandom(diago.tsize) * bitsPerTexel;
	      if(textureDownloadSize > textureSize)
		{
		  textureDownloadSize = textureSize;
		  startOffset = 0;
		}
	      else
		{
		  startOffset = rRandom(0, textureSize - textureDownloadSize);
		  startOffset &= ~0x3;
		}
	      startOffset += mipmapOffset;

	      if(textureDownloadSize/4 > MAX_WORDS)
		{
		  textureDownloadSize = MAX_WORDS * 4 - 4;
		  assert(textureDownloadSize > 0);
		}
	      GDBG_INFO(2, "textureDownloadSize=%d  textureSize=%d  startOffset=%d\n", 
			textureDownloadSize, textureSize, startOffset);
	      	      
	      startAddr = (FxU32 *)(texturePort + startOffset -				    
				    sstLinearMipMapOffset(0, logAspectRatio, bitsPerTexel, bigAssTexture,
							  wideTexture, compressedTexture, FXFALSE));
	      
	      mungedBase = SST_TEXTURE_MUNGE_ADDRESS(baseAddr);
	      
	      gdbg_info(2,"downloading %d bytes at offset 0x%x, texBaseAddr=0x%x (Linear)\n",
			textureDownloadSize, startOffset, mungedBase);

	      for (j=0; j < (FxI32)(textureDownloadSize/4); j++) {
		scramble[j] = j;
	      }
	      if (iRandom(4)==0) {
		gdbg_info(3,"scrambling order\n");
		scrambleRandom(textureDownloadSize/4,scramble);
	      }
	      
	      SET(SST_TREX(sst,trex)->textureMode,t->tex->tMode);	// set texture mode just for fun
	      SET(SST_TREX(sst,trex)->texBaseAddr,mungedBase);
	      if (diago.halInfo->hsim & HSIM_TREX_BACKDOOR_TEXWRITES)
		sst_idle_really(sst);

	      baseAddr = SST_TEXTURE_UNMUNGE_ADDRESS(mungedBase);
	      // download texture data thru the texture port
	      for (j=0; j < (FxI32)(textureDownloadSize/4); j++) {
		k = iRandom(0xFFFFFFFF);	// gen a random word
		tarray[scramble[j]] = k;	// save it for checking
		SET(startAddr[scramble[j]],k);	// download it
	      }
	      
	      startAddr = (FxU32 *)(SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET 
				    + ((baseAddr+startOffset)&SST_TEXTURE_FULL_ADDRESS));

	      sst_idle_really(sst);			// wait for the command to complete
	      // now read back the data thru the non-modal LFB port!!!
	      for (j=0; j < (FxI32)textureDownloadSize/4; j++) {
		FxU32 got = GET(startAddr[j]);		// fetch the downloaded data
		if (got != tarray[j]) {
		  gdbg_printf("ERROR: expecting texel[%d] to be 0x%x but read 0x%x @ 0x%x\n",
			      j, tarray[j], got, startAddr+j);
		  DIAG_INCERROR();
		}
	      }
	    }
	}
    }
    DIAG_PASS(0);
}

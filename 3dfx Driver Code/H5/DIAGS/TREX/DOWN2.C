/* -*-c++-*- */

#include <assert.h>
#include <stdlib.h>

#include "allocate.h"
#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

#define TEXEL_COUNT   4   //The number of texels that are tested with each texture download

//
//
// This is an ad hoc diag to test compressed tiled downloads
//
//

void testTiledTexturePort(SstRegs *sst);

int main(int argc, char **argv)
{
  SstRegs *sst;

  GDBG_INFO(0, "************************************************************\n");
  GDBG_INFO(0, "***                                                      ***\n");
  GDBG_INFO(0, "***                                                      ***\n");
  GDBG_INFO(0, "***                                                      ***\n");
  GDBG_INFO(0, "***         Remember to use -J                           ***\n");
  GDBG_INFO(0, "***                                                      ***\n");
  GDBG_INFO(0, "***                                                      ***\n");
  GDBG_INFO(0, "***                                                      ***\n");
  GDBG_INFO(0, "***                                                      ***\n");
  GDBG_INFO(0, "************************************************************\n");

  sst = SST_BEGIN(argc,argv);

  while (DIAG_STARTPASS()) 
    {			// for each pass
      //Test tiled downloads now
      testTiledTexturePort(sst);
    }
  
  DIAG_PASS(-1);

  return(0);
}


void testTiledTexturePort(SstRegs *sst)
{
  FxI32 microTileU, microTileV, lod, wordSelect, bitsPerTexel;
  FxI32 log2AspectRatio, wideTexture, texturePort, tileStride;
  FxU32 textureBaseAddress, textureTiledAddress, writeAddress, texturePortAddress;
  FxU32 texelAddress[4];
  FxU32 texelData[4], readData;
  FxU32 *rawLFB;
  SstIORegs *sstio;
  tiledStruct mipmap;

  //Make rawlfb all linear
  sstio = (SstIORegs *)(SST_IO_ADDRESS(sst));  
  SET(sstio->lfbMemoryConfig, ((0x1FFF)<<SST_RAW_LFB_TILE_BEGIN_PAGE_SHIFT) | (3<<23));  
  rawLFB = (FxU32 *)(((int)sst) - SST_3D_OFFSET + SST_RAW_LFB_OFFSET);

  tileStride = 16;
  GDBG_INFO(0, "texture tileStride = %d\n", tileStride);

  for(bitsPerTexel=4; bitsPerTexel<8; bitsPerTexel+=4)
    {
      if(bitsPerTexel == 4)
	SET(sst->textureMode, SST_3DFX_COMPRESSED | SST_COMPRESSED_TEXTURES);
      else
	SET(sst->textureMode, SST_DXT2 | SST_COMPRESSED_TEXTURES);
      
      for(wideTexture=0; wideTexture<2; wideTexture++)
	{
	  for(log2AspectRatio=0; log2AspectRatio<4; log2AspectRatio++)
	    {
	      SET(sst->tLOD, (wideTexture ? SST_LOD_S_IS_WIDER : 0) |
		  (log2AspectRatio << SST_LOD_ASPECT_SHIFT));
	      
	      for(lod=0; lod<=8; lod++)
		{
		  textureBaseAddress = iRandom(63 * 1024 * 1024) & (~0xF);

		  textureBaseAddress = 0;
		  
		  SET(sst->texBaseAddr, SST_TEXTURE_MUNGE_ADDRESS(textureBaseAddress) | 
		      SST_TEXTURE_IS_TILED | 
		      (tileStride << SST_TEXTURE_TILESTRIDE_SHIFT));
		  
		  microTileU = iRandom(255);
		  microTileV = iRandom(255);

		  microTileU >>= lod;
		  microTileV >>= lod;

		  if(wideTexture)
		    microTileV >>= log2AspectRatio;
		  else
		    microTileU >>= log2AspectRatio;
		  
		  mipmap = sstTiledMipMapOffset2(lod, shadowRegisters3D[0][1].tLOD, shadowRegisters3D[0][1].textureMode);

		  convertToMicroTile(shadowRegisters3D[0][1].textureMode, &mipmap.uoff, &mipmap.voff);		  
		  convertToMicroTile(shadowRegisters3D[0][1].textureMode, &microTileU, &microTileV);
		  
		  //Pick a texturePort
		  texturePort=iRandom(2);
		  if(texturePort == 0)		  
		    texturePortAddress = ((FxU32)sst) - SST_3D_OFFSET + SST_TEX0_OFFSET;
		  else if(texturePort == 1)
		    texturePortAddress = ((FxU32)sst) - SST_3D_OFFSET + SST_TEX1_OFFSET;
		  else if(texturePort == 2)
		    texturePortAddress = ((FxU32)sst) - SST_3D_OFFSET + SST_TEX2_OFFSET;
		  
		  //Do the texture port writes
		  for(wordSelect=0; wordSelect<4; wordSelect++)
		    {
		      textureTiledAddress = calculateTiledTexturePortAddress(shadowRegisters3D[0][1].textureMode,
								      shadowRegisters3D[0][1].tLOD,
								      lod, microTileU, microTileV, wordSelect);
		      writeAddress = textureTiledAddress + texturePortAddress;		      
		      
		      GDBG_INFO(0, "\n");
		      GDBG_INFO(0, "lod=%d U=%d V=%d wordSelect=%d log2AspectRatio=%d wideTexture=%d\n",
				lod, microTileU, microTileV, wordSelect, log2AspectRatio, wideTexture);
		      GDBG_INFO(0, "textureTiledAddress=0x%x texturePort=%d writeAddress=0x%x\n", 
				textureTiledAddress, texturePort, writeAddress);		      
		      
		      texelAddress[wordSelect] = tiledAddress(textureBaseAddress, tileStride, 16, 
							      microTileU + mipmap.uoff, 
							      microTileV + mipmap.voff) + wordSelect*4;
		      
		      texelData[wordSelect] = iRandom(0xFFFFFFFF);

		      GDBG_INFO(0, "0x%x should be written to address 0x%x\n",
				texelData[wordSelect], texelAddress[wordSelect]);
		      diagStore32((void *)writeAddress, texelData[wordSelect]);
		    }

		  //Check the writes
		  sst_idle_really(sst);

		  for(wordSelect=0; wordSelect<4; wordSelect++)
		    {		      
		      FxU32 linearAddress;
		      FxU32 tileU, tileV;
		      
		      linearAddress = texelAddress[wordSelect] & (~(SST_TILE_SIZE - 1));
		      tileU = texelAddress[wordSelect] & SST_TILE_WIDTH_MASK;
		      tileV = (texelAddress[wordSelect] >> SST_TILE_WIDTH_BITS) & SST_TILE_HEIGHT_MASK;
		      
		      linearAddress += 128 * tileV + tileU;
		      
		      assert((texelAddress[wordSelect] & 3) == 0);
		      readData = GET(rawLFB[linearAddress>>2]);
		      
		      if(readData != texelData[wordSelect])
			GDBG_ERROR("", "lod=%d U=%d V=%d wordSelect=%d log2AspectRatio=%d wideTexture=%d failed\n",
				   lod, microTileU, microTileV, wordSelect, log2AspectRatio, wideTexture);
		    }
		}
	    }
	}
    }
}

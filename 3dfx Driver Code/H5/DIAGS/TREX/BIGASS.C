/* -*-c++-*- */

#include <assert.h>
#include <stdlib.h>

#include "allocate.h"
#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

#define TEXEL_COUNT   4   //The number of texels that are tested with each texture download

FxU32 lodColor(FxU32 lod);
void testLinearLargeTexturePort(SstRegs *sst);
void testTiledLargeTexturePort(SstRegs *sst);

//Global variable helpful in debugging
FxU32 texelDownloadCounter=0;

int main (int argc, char **argv)
{
  SstRegs *sst;
  FxI32 i;

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
  
  if(!diago.bigAssTextures)
    {
      GDBG_INFO(0, "Forcing --bigAssTextures!\n");
      diago.bigAssTextures=1;
    }
  
  while (DIAG_STARTPASS()) 
    {			// for each pass
      for(i=0; i<10; i++)
	{
	  //Test the linear download port    
	  //testLinearLargeTexturePort(sst);
	  
	  //Test tiled downloads now
	  testTiledLargeTexturePort(sst);
	}    

    }
  
  DIAG_PASS(0);
  
  return(0);
}

//This picks a unique 32 bit ARGB color for a given lod
FxU32 lodColor(FxU32 lod)
{
  FxU32 color;
  
  switch(lod)
    {
    case 0:
      color=0xff00ff00;
      break;
    case 1:
      color=0x80ff00ff;
      break;
    case 2:
      color=0xffffff00;
      break;
    case 3:
      color=0xff00ffff;
      break;
    case 4:
      color=0xff008000;
      break;
    case 5:
      color=0x8000ff00;
      break;
    case 6:
      color=0x80008000;
      break;
    case 7:
      color=0xff008080;
      break;
    case 8:
      color=0x80808080;
      break;
    case 9:
      color=0x80808000;
      break;
    case 10:
      color=0x80008080;
      break;
    case 11:
      color=0x80000080;
      break;
    }

  return(color);
}

//This downloads fragments of a texture into linear space
//It only downloads a small percentage of the whole texture
//in a random order
void testLinearLargeTexturePort(SstRegs *sst)
{
  FxI32 i, baseAddress, texBaseAddrBaseAddress;
  FxI32 texelIndex, texelValue, texelOffset;
  FxI32 width, height, lod;
  FxI32 logWidth, logHeight;   //Log base 2 of texture width and height
  FxI32 logAspectRatio;
  FxI32 bytesPerTexel, tFormat;
  FxI32 numberTexels;  //Number of texels in texture
  FxBool wideTexture;  //Is wider than tall?
      
  //Pick a texture size
  logWidth = iRandom(11);
  logHeight = logWidth + rRandom(-3, 3);
  if(logHeight > 11)
    logHeight = 11;
  else if(logHeight < 0)
    logHeight = 0;

  width  = 1 << logWidth;
  height = 1 << logHeight;

  //Calculate the aspect ratio
  if(logWidth > logHeight)
    {
      logAspectRatio = logWidth - logHeight;
      wideTexture=FXTRUE;
      lod = 11 - logWidth;
    }
  else if(logWidth < logHeight)
    {
      logAspectRatio = logHeight - logWidth;
      wideTexture=FXFALSE;
      lod = 11 - logHeight;
    }
  else
    {
      logAspectRatio=0;
      wideTexture=iRandom(1);
      lod = 11 - logWidth;
    }  
  GDBG_INFO(5, "width = %d   height = %d  lod = %d   logAspectRatio=%d\n", 
	    width, height, lod, logAspectRatio);

  //Figure out how many texels are in the texture
  numberTexels = width * height;
  GDBG_INFO(205, "numberTexels = %d\n", numberTexels);

  //Pick a texture format
  if(numberTexels<4)  //Make sure the texture is at least 4 bytes
    i=2;
  else
    i=iRandom(2);

  switch(i)
    {
    case 0:
      bytesPerTexel=1;
      tFormat = SST_RGB332;
      break;

    case 1:
      bytesPerTexel=2;
      tFormat = SST_ARGB4444;
      break;

    case 2:
      bytesPerTexel=4;
      tFormat = SST_ARGB8888;
      break;
    }
  GDBG_INFO(5, "bytesPerTexel = %d\n", bytesPerTexel);

  //Allocate space in the framebuffer
  baseAddress = allocate(numberTexels * bytesPerTexel, "Texture", randomPlacement);
  GDBG_INFO(5, "testLinearLargeTexturePort:  baseAddress = 0x%x\n", baseAddress);

  //Free the texture space immediately, we don't really need it long term
  unallocate(baseAddress);

  //Set up for download
  SET(sst->textureMode, tFormat);
  SET(sst->tLOD, ((lod<<SST_LOD_FRACBITS)<<SST_LODMIN_SHIFT) | ((lod<<SST_LOD_FRACBITS)<<SST_LODMAX_SHIFT) | SST_TBIG |
      ((logAspectRatio << SST_LOD_ASPECT_SHIFT) & SST_LOD_ASPECT) | (wideTexture ? SST_LOD_S_IS_WIDER : 0));
      
  
  //Figure out where to point texBaseAddr
  texBaseAddrBaseAddress = baseAddress - sstLinearMipMapOffset2(lod, shadowRegisters3D[0][1].tLOD, 
								shadowRegisters3D[0][1].textureMode);
  
  //Wrap texBaseAddr around if necessary
  if(texBaseAddrBaseAddress < 0)
    texBaseAddrBaseAddress += 64*1024*1024;
  else if(texBaseAddrBaseAddress >= 64*1024*1024)
    texBaseAddrBaseAddress -= 64*1024*1024;

  GDBG_INFO(208, "texBaseAddrBaseAddress=0x%x  baseAddress=0x%x\n",
	    texBaseAddrBaseAddress, baseAddress);
  SET(sst->texBaseAddr, SST_TEXTURE_MUNGE_ADDRESS(texBaseAddrBaseAddress));
  

  //Download and check a bunch of texels
  for(i=0; i<TEXEL_COUNT; i++)
    {
      FxU32 address;
      FxU32 downloadAddress;

      //Pick a texel to write to
      texelIndex = iRandom(numberTexels-1);  //pick a texel for the given LOD
      texelOffset = texelIndex * bytesPerTexel;
      texelValue = iRandom(0xFFFFFFFF);      
      GDBG_INFO(206, "texelIndex = 0x%x   texelValue = 0x%x\n", texelIndex, texelValue);
            
      //Calculate the download texel index relative to lod 0
      downloadAddress = texelOffset + sstLinearMipMapOffset2(lod, shadowRegisters3D[0][1].tLOD, shadowRegisters3D[0][1].textureMode)
	- sstLinearMipMapOffset2(0, shadowRegisters3D[0][1].tLOD, shadowRegisters3D[0][1].textureMode);

      GDBG_INFO(208, "Writing to texture port offset = 0x%x\n", downloadAddress);

      //Download the mofo
      texelDownloadCounter++;
      switch(bytesPerTexel)
	{
	case 1:
	  diagStore8((void *)(SST_TEX2_ADDRESS(sst) + downloadAddress), (FxU8)texelValue);	  
	  texelValue = texelValue & 0x000000FF;
	  GDBG_INFO(210, "bigass diagStore8(0x%x, 0x%x)\n", (SST_TEX2_ADDRESS(sst) + downloadAddress),
		    texelValue);
	  break;
	case 2:
	  diagStore16((void *)(SST_TEX2_ADDRESS(sst) + downloadAddress), (FxU16)texelValue);
	  texelValue = texelValue & 0x0000FFFF;
	  GDBG_INFO(210, "bigass diagStore16(0x%x, 0x%x)\n", (SST_TEX2_ADDRESS(sst) + downloadAddress),
		    texelValue);
	  break;
	case 4:
	  diagStore32((void *)(SST_TEX2_ADDRESS(sst) + downloadAddress), texelValue);
	  GDBG_INFO(210, "bigass diagStore32(0x%x, 0x%x)\n", (SST_TEX2_ADDRESS(sst) + downloadAddress),
		    texelValue);
	  break;
	}

      //Idle the chip to force the download to complete
      sst_idle_really(sst);

      //Check the mofo      
      //Calculate the linear address
      address = baseAddress + texelIndex*bytesPerTexel;

      if(address < 0)
	address += 64*1024*1024;
      else if(address >= 64*1024*1024)
	address -= 64*1024*1024;

      GDBG_INFO(208, "Linear check address = 0x%x\n", address);
      GDBG_INFO(208, "texelIndex = 0x%x\n", texelIndex);
	
      DIAG_TEST_MEM(address, texelValue, bytesPerTexel);
		    
    }  
}

//This downloads fragments of a texture into tiled space
//It only downloads a small percentage of the whole texture
void testTiledLargeTexturePort(SstRegs *sst)
{
  FxI32 i, baseAddress;
  FxI32 texelValue;
  FxI32 width, height, lod;
  FxI32 logWidth, logHeight;   //Log base 2 of texture width and height
  FxI32 logAspectRatio;
  FxI32 bytesPerTexel, tFormat;
  FxBool wideTexture;  //Is wider than tall?
  FxI32 texBaseAddrBaseAddress, tileStride;
  FxI32 du, dv;
  tiledStruct mipmap;
  
      
  //Pick a texture size
  logWidth = iRandom(10);   //We can only download up to 1024x1024 textures in tiled space
  logHeight = logWidth + rRandom(-3, 3);
  if(logHeight > 10)
    logHeight = 10;
  else if(logHeight < 0)
    logHeight = 0;

  width  = 1 << logWidth;
  height = 1 << logHeight;

  //Calculate the aspect ratio
  if(logWidth > logHeight)
    {
      logAspectRatio = logWidth - logHeight;
      wideTexture=FXTRUE;
      lod = 11 - logWidth;
    }
  else if(logWidth < logHeight)
    {
      logAspectRatio = logHeight - logWidth;
      wideTexture=FXFALSE;
      lod = 11 - logHeight;
    }
  else
    {
      logAspectRatio=0;
      wideTexture=iRandom(1);
      lod = 11 - logWidth;
    }  
  GDBG_INFO(5, "width = %d   height = %d  lod = %d   logAspectRatio=%d\n", 
	    width, height, lod, logAspectRatio);

  switch(width >= 8 ? iRandom(2) : 2)
    {
    case 0:
      bytesPerTexel=1;
      tFormat = SST_RGB332;
      break;

    case 1:
      bytesPerTexel=2;
      tFormat = SST_ARGB4444;
      break;

    case 2:
      bytesPerTexel=4;
      tFormat = SST_ARGB8888;
      break;
    }
  GDBG_INFO(205, "bytesPerTexel = %d\n", bytesPerTexel);

  //Allocate space in the framebuffer
  //Just allocate a humungo amount of space
  baseAddress = allocate((2048 + 1024) * 2048 * bytesPerTexel , "Texture", randomPlacement);
  //Free the texture space immediately, we don't really need it long term
  unallocate(baseAddress);

  GDBG_INFO(205, "testTiledLargeTexturePort:  baseAddress = 0x%x\n", baseAddress);

  //Make sure the baseAddress is tile aligned because we're lazy
  baseAddress += (SST_TILE_SIZE-1);
  baseAddress &= ~(SST_TILE_SIZE-1);

  //Set up for download
  SET(sst->textureMode, tFormat);
  SET(sst->tLOD, ((lod<<SST_LOD_FRACBITS)<<SST_LODMIN_SHIFT) | ((lod<<SST_LOD_FRACBITS)<<SST_LODMAX_SHIFT) | SST_TBIG |
      ((logAspectRatio << SST_LOD_ASPECT_SHIFT) & SST_LOD_ASPECT) | (wideTexture ? SST_LOD_S_IS_WIDER : 0));
  
  
  //Need to figure out where to set texBaseAddr
  mipmap = sstTiledMipMapOffset2(lod, shadowRegisters3D[0][1].tLOD, shadowRegisters3D[0][2].textureMode);
  du = -mipmap.uoff;
  dv = -mipmap.voff;
  tileStride = ((2048 + 1024) * bytesPerTexel)/SST_TILE_WIDTH;
  texBaseAddrBaseAddress = tiledAddress(baseAddress, tileStride, bytesPerTexel, du, dv);
  GDBG_INFO(205, "tileStride=%d du=%d dv=%d  texBaseAddrBaseAddress=0x%x\n", 
	    tileStride, du, dv, texBaseAddrBaseAddress);
  
  
  SET(sst->texBaseAddr, SST_TEXTURE_MUNGE_ADDRESS(texBaseAddrBaseAddress) | SST_TEXTURE_IS_TILED |
      ((tileStride<<SST_TEXTURE_TILESTRIDE_SHIFT) & SST_TEXTURE_TILESTRIDE));

  
  //Download and check a bunch of texels
  for(i=0; i<TEXEL_COUNT; i++)
    {      
      FxU32 texturePortAddress, texelAddress;
      FxU32 s, t;
            
      //Pick a texel to write to
      s=iRandom(width-1);
      t=iRandom(height-1);

      //Pick a value
      texelValue = iRandom(0xFFFFFFFF);      
      
      //Figure out the texture port address	
      if(bytesPerTexel == 1)
	texturePortAddress = ((s<<SST_TEXTURE_BIG_S8_SHIFT) & SST_TEXTURE_BIG_S8) |
	  ((t<<SST_TEXTURE_BIG_T8_SHIFT) & SST_TEXTURE_BIG_T8) |
	  ((lod<<SST_TEXTURE_BIG_LOD8_SHIFT) & SST_TEXTURE_BIG_LOD8);
      else if(bytesPerTexel == 2)
	texturePortAddress = ((s<<SST_TEXTURE_BIG_S16_SHIFT) & SST_TEXTURE_BIG_S16) |
	  ((t<<SST_TEXTURE_BIG_T16_SHIFT) & SST_TEXTURE_BIG_T16) |
	  ((lod<<SST_TEXTURE_BIG_LOD16_SHIFT) & SST_TEXTURE_BIG_LOD16);
      else if(bytesPerTexel == 4)
	texturePortAddress = ((s<<SST_TEXTURE_BIG_S32_SHIFT) & SST_TEXTURE_BIG_S32) |
	  ((t<<SST_TEXTURE_BIG_T32_SHIFT) & SST_TEXTURE_BIG_T32) |
	  ((lod<<SST_TEXTURE_BIG_LOD32_SHIFT) & SST_TEXTURE_BIG_LOD32);
            
      GDBG_INFO(206, "s=%d t=%d lod=%d texturePortAddress= 0x%x\n", s, t, lod, texturePortAddress);
            
      //Download the mofo
      texelDownloadCounter++;
      switch(bytesPerTexel)
	{
	case 1:
	  diagStore8((void *)(SST_TEX2_ADDRESS(sst) + texturePortAddress), (FxU8)texelValue);
	  texelValue = texelValue & 0x000000FF;
	  break;
	case 2:
	  diagStore16((void *)(SST_TEX2_ADDRESS(sst) + texturePortAddress), (FxU16)texelValue);
	  texelValue = texelValue & 0x0000FFFF;
	  break;
	case 4:
	  diagStore32((void *)(SST_TEX2_ADDRESS(sst) + texturePortAddress), texelValue);
	  break;
	}
      
      //Idle the chip to force the download to complete
      sst_idle_really(sst);

      //Check the mofo
      texelAddress = tiledAddress(baseAddress, tileStride, bytesPerTexel, s, t);

      GDBG_INFO(205, "texelAddress=0x%x  s=%d  t=%d\n", texelAddress, s, t);

      DIAG_TEST_MEM(texelAddress, texelValue, bytesPerTexel);
    }  
}

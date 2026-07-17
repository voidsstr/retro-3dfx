#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "udiag.h"
#include "sstdiag.h"

#define BLOCK_WIDTH  2
#define BLOCK_HEIGHT 2

typedef struct 
{
  FxU32 r;
  FxU32 g;
  FxU32 b;
} PaletteEntry;

void downloadRandomPalette(SstRegs *sst);
void downloadTexture(SstRegs *sst);
void drawQuad(SstRegs *sst, FxU32 tileWidth, FxU32 tileHeight);
void readPixels(SstRegs *sst, FxU32 tileWidth, FxU32 tileHeight);

PaletteEntry palette[256];

int main(int argc, char **argv)
{
  SstRegs *sst;
  SstIORegs *sstio;
  FxI32 tmu;

  sst = SST_BEGIN(argc, argv);
  sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
  
  while(DIAG_STARTPASS()) 
    {
      //download the texture
      downloadTexture(sst);
      
      //Download a random palette
      downloadRandomPalette(sst);
      
      //Set up shit so the pixel value is just the TMU texel value
      if(diago.trex < 0) 
	tmu = iRandom(1);
      else 
	tmu = diago.trex;

      GDBG_INFO(0, "Testing tmu %d\n", tmu);

      if(tmu == 0)
	{
	  SET_0(sst->textureMode, SST_TC_REPLACE | SST_TCA_REPLACE | SST_P8);
	  SET_0(sst->textureMode, SST_TC_REPLACE | SST_TCA_REPLACE | SST_P8);
	}
      else
	{
	  SET_0(sst->textureMode, SST_TC_PASS);
	  SET_1(sst->textureMode, SST_TC_REPLACE | SST_TCA_REPLACE | SST_P8);
	  SET_1(sst->textureMode, SST_TC_REPLACE | SST_TCA_REPLACE | SST_P8);
	}
      
      SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT | SST_ENTEXTUREMAP |
	  SST_PARMADJUST);
      SET(sst->fbzMode, SST_RGBWRMASK);	   
      
      SET_0(sst->textureMode, SST_TC_REPLACE | SST_TCA_REPLACE | SST_P8);
      SET_0(sst->textureMode, SST_TC_REPLACE | SST_TCA_REPLACE | SST_P8);
      
      //Force lod to 16x16 size
      SET(sst->tLOD, ((4<<2) << SST_LODMIN_SHIFT) | ((4<<2) << SST_LODMAX_SHIFT));
	  
      //Set lfbMemoryConfig so tiled memory starts high up
      SET(sstio->lfbMemoryConfig, SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE(4096));

      //Set up 32bpp
      SET(sst->renderMode, SST_RM_32BPP | SST_RM_RGBA_WMASK);
      SET(sst->colBufferStride, 640*4);
      
      //Draw a 16x16 pattern that uses all palette entries
      drawQuad(sst, BLOCK_WIDTH, BLOCK_HEIGHT);

      //Read back the rendered pixels from the framebuffer
      readPixels(sst, BLOCK_WIDTH, BLOCK_HEIGHT);
    }

  DIAG_PASS(0);

  return(0);
}

void downloadRandomPalette(SstRegs *sst)
{
  FxU32 r, g, b;
  FxI32 index, value;

  GDBG_INFO(0, "downloadRandomPalette begin\n");

  fxHalIdle(sst);

  for(index=0; index<4; index++)
    {
      //Write even palette entries
      r = iRandom(255);
      g = iRandom(255);
      b = iRandom(255);
      
      palette[index].r = r;
      palette[index].g = g;
      palette[index].b = b;

      value = 0x80000000 | ((index & 0xFE)<<23) | (r << 16) | (g << 8) | b;	

      if(index & 1)
	SET(sst->nccTable0[5], value);
      else
	SET(sst->nccTable0[4], value);

      GDBG_INFO(0, "Palette[0x%02x] = %02x %02x %02x\n", index, r, g, b);
    }  

  //Idle again for good measure
  fxHalIdle(sst);

  GDBG_INFO(0, "downloadRandomPalette end\n");
}

void downloadTexture(SstRegs *sst)
{
  FxU32 lodOffset, address, index;

  GDBG_INFO(0, "downloadTexture begin\n");

  //Flush the texture cache
  SET(sst->texBaseAddr, ~0);  //Flush
  SET(sst->texBaseAddr, 0);
  
  //Set the real texture base address
  SET(sst->texBaseAddr, 1024*1024);

  //Calculate the offset to the 16x16 lod
  lodOffset = 256*256 + 128*128 + 64*64 + 32*32;
  
  address = ((FxU32)sst) - SST_3D_OFFSET + SST_TEX0_OFFSET;

  for(index=0; index<256; index++)
    halStore8((void *)(address + lodOffset + index), (FxU8)index);

  GDBG_INFO(0, "downloadTexture end\n");
}

void drawQuad(SstRegs *sst, FxU32 tileWidth, FxU32 tileHeight)
{
  GDBG_INFO(0, "drawQuad begin\n");

  SET(sst->sSetupMode, SST_SETUP_Wfbi | SST_SETUP_ST0 | SST_SETUP_FAN);
  
  SETF(sst->sVx, 0);
  SETF(sst->sVy, 0);
  SETF(sst->sSow0, 0);
  SETF(sst->sTow0, 0);
  SETF(sst->sOowfbi, 1);
  SET(sst->sBeginTriCMD, 0);    	
  
  SETF(sst->sVx, (float)(tileWidth * 16));
  SETF(sst->sVy, 0);
  SETF(sst->sSow0, 256);
  SETF(sst->sTow0, 0);
  SETF(sst->sOowfbi, 1);
  SET(sst->sDrawTriCMD, 0);    	
  
  SETF(sst->sVx, (float)(tileWidth * 16));
  SETF(sst->sVy, (float)(tileHeight * 16));
  SETF(sst->sSow0, 256);
  SETF(sst->sTow0, 256);
  SETF(sst->sOowfbi, 1);
  SET(sst->sDrawTriCMD, 0);
  
  SETF(sst->sVx, 0);
  SETF(sst->sVy, (float)(tileHeight * 16));
  SETF(sst->sSow0, 0);
  SETF(sst->sTow0, 256);
  SETF(sst->sOowfbi, 1);
  SET(sst->sDrawTriCMD, 0);

  //Idle for fun
  fxHalIdle(sst);

  GDBG_INFO(0, "drawQuad end\n");
}

void readPixels(SstRegs *sst, FxU32 tileWidth, FxU32 tileHeight)
{
  FxU32 address, baseAddress;
  FxU32 i,j, x, y;
  FxU32 color, blockColor, r, g, b;
  FxBool badEntry;
  FxU32 nInvalidEntries;
  FxU32 nIncorrectEntries;

  GDBG_INFO(0, "readPixels begin\n");

  baseAddress = ((FxU32)sst) - SST_3D_OFFSET + SST_RAW_LFB_OFFSET;

  nInvalidEntries = 0;
  nIncorrectEntries = 0;
  for(i=0; i<16; i++)
    for(j=0; j<16; j++)
      {
	badEntry = FXFALSE;

	for(y=0; y<tileHeight; y++)
	  for(x=0; x<tileWidth; x++)
	    {
	      address = baseAddress + ((i*tileHeight + y) * 640 * 4) + 
		((j*tileWidth + x) * 4);
	      
	      color = halLoad32((void *)address);

	      r = (color >> 16) & 0xFF;
	      g = (color >> 8)  & 0xFF;
	      b = (color >> 0)  & 0xFF;
	      GDBG_INFO_MORE(1, "%02x %02x %02x\n", r, g, b);
	     
	      if(x == 0 && y == 0)
		blockColor = color;
       
	      if(blockColor != color)
		badEntry = FXTRUE;
	    }

	GDBG_INFO(0, "Palette[0x%02x] = ", j + i * 16);
	if(badEntry)
	  {
	    GDBG_INFO_MORE(0, "f'ed up\n");
	    nInvalidEntries++;
	  }
	else
	  {
	    r = (blockColor >> 16) & 0xFF;
	    g = (blockColor >> 8)  & 0xFF;
	    b = (blockColor >> 0)  & 0xFF;
	    GDBG_INFO_MORE(0, "%02x %02x %02x\n", r, g, b);

	    if((palette[i*16 + j].r != r) ||
	       (palette[i*16 + j].g != g) ||
	       (palette[i*16 + j].b != b))
	      {
		GDBG_ERROR("readPixels", "wrote palette[%3d] = %02x %02x %02x  got %02x %02x %02x\n",
			   i*16 + j , palette[i*16 + j].r, palette[i*16 + j].g, palette[i*16 + j].b, 
			   r, g, b);
		
		nIncorrectEntries++;
	      }
	  }	   
      }

  if(nInvalidEntries)
    GDBG_ERROR("readPixels", "Shizit! There are %d invalid palette entries\n", nInvalidEntries);

  if(nIncorrectEntries)
    GDBG_ERROR("readPixels", "Shizit! There are %d incorrect palette entries\n", nIncorrectEntries);

  GDBG_INFO(0, "readPixels end\n");  
}


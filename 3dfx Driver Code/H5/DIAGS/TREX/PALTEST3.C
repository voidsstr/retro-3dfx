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

void downloadPaletteEntry(SstRegs *sst, FxU32 paletteEntry, 
			  FxU32 r, FxU32 g, FxU32 b);
void downloadTexture(SstRegs *sst, FxU32 paletteEntry);
void drawQuad(SstRegs *sst, FxU32 tileWidth, FxU32 tileHeight, 
	      FxU32 xOffset, FxU32 yOffset, FxU32 paletteEntry);
float readQuad(SstRegs *sst, FxU32 *red, FxU32 *green, FxU32 *blue,
	       FxU32 tileWidth, FxU32 tileHeight, FxU32 xOffset, FxU32 yOffset, 	     
	       FxU32 expectedR, FxU32 expectedG, FxU32 expectedB);
FxU32 reverseBits(FxU32 value, FxU32 width);
void setup(SstRegs *sst);

PaletteEntry palette[256];

int main(int argc, char **argv)
{
  SstRegs *sst;
  FxU32 i, j, x, y, yOffset;
  FxU32 r, g, b;
  FxU32 red[BLOCK_WIDTH * BLOCK_HEIGHT];
  FxU32 green[BLOCK_WIDTH * BLOCK_HEIGHT];
  FxU32 blue[BLOCK_WIDTH * BLOCK_HEIGHT];
  FxU32 paletteEntry;
  float percentageIncorrect[2];
  PaletteEntry colors[]={{0xff,0xff,0xff},
			 {0x00,0x00,0x00},
			 {0xff,0x00,0x00},
			 {0x00,0xff,0x00},
			 {0x00,0x00,0xff},
			 {0x00,0xff,0xff},
			 {0xff,0x00,0xff},
			 {0xff,0xff,0x00}};
  FxU32 nColors = 8;
  FxI32 singleIndex = -1;

  //Scan through args for --index <number>
  for(i=1; i<(FxU32)argc; i++)
    {
      if(!strcmp(argv[i], "--index"))
	{
	  if(i+1 >= (FxU32)argc)
	    {
	      GDBG_ERROR("paltest2::main", "--index <number>: number not included\n");
	      exit(-1);
	    }

	  singleIndex = atoi(argv[i+1]);
	  
	  for(j=i+2; j<(FxU32)argc; j++)
	    {
	      argv[j-2] = argv[j];
	    }
	  argc -= 2;
	  i--;
	}
    }
  
  
  sst = SST_BEGIN(argc, argv);
  
  while(DIAG_STARTPASS()) 
    {
      //Set up the TMUs and fbi
      setup(sst);
      
      for(j=0; j<1; j++)
	{
	  r = colors[j].r;
	  g = colors[j].g;
	  b = colors[j].b;
	  
	  for(i=0; i<4; i++)
	    {
	      downloadPaletteEntry(sst, reverseBits(i, 8), r, g, b);
	      fxHalIdle(sst);
	    }
	  
	  for(yOffset=0; yOffset < 100; yOffset += 100)
	    {		  	      
	      for(i=0; i<4; i++)
		{	  
		  if(singleIndex >= 0)
		    i = singleIndex;
		  
		  x = (i%16)*BLOCK_WIDTH + j * 80;
		  y = (i/16)*BLOCK_HEIGHT + yOffset;
		  paletteEntry = reverseBits(i, 8);		  		  
		  
		  GDBG_INFO(0, "i=%08x, paletteEntry=%08x\n", i, paletteEntry);
		  downloadTexture(sst, paletteEntry);
		  		  
		  drawQuad(sst, BLOCK_WIDTH, BLOCK_HEIGHT, x, y, paletteEntry);		   	  
		  
		  percentageIncorrect[0] = readQuad(sst, red, green, blue, BLOCK_WIDTH, BLOCK_HEIGHT, 
						    x, y, r, g, b);

		  if(singleIndex >= 0)
		    break;

		  //analyzePixels(red, green, blue, r, g, b);
		}
	    }
	}
    }
   
  if(diago.errorCount > 0)
    DIAG_FAIL();

  DIAG_PASS(0);

  return(0);
}

void downloadPaletteEntry(SstRegs *sst, FxU32 paletteEntry, 
			  FxU32 r, FxU32 g, FxU32 b)
{
  FxI32 value;
  
  GDBG_INFO(1, "downloadPaletteEntry begin\n");
  
  //Idle for fun
  fxHalIdle(sst);
  
  assert(r >= 0 && r < 256);
  assert(g >= 0 && g < 256);
  assert(b >= 0 && b < 256);
  assert(paletteEntry >= 0 && paletteEntry < 256);

  palette[paletteEntry].r = r;
  palette[paletteEntry].g = g;
  palette[paletteEntry].b = b;

  value = 0x80000000 | ((paletteEntry & 0xFE) << 23) | (r << 16) | (g << 8) | b;	

  if(paletteEntry & 1)
    SET(sst->nccTable0[5], value);  //Write the odd entry
  else
    SET(sst->nccTable0[4], value);  //Write the even entry

  GDBG_INFO(2, "Palette[0x%02x] = %02x %02x %02x\n", paletteEntry, r, g, b);
  
  //Idle again for good measure
  fxHalIdle(sst);

  GDBG_INFO(1, "downloadPaletteEntry end\n");
}

void downloadTexture(SstRegs *sst, FxU32 paletteEntry)
{
  FxU32 lodOffset, address, index;

  GDBG_INFO(1, "downloadTexture begin\n");

  //Flush the texture cache
  SET(sst->texBaseAddr, ~0);  //Flush
  SET(sst->texBaseAddr, 0);
  
  //Set the real texture base address
  SET(sst->texBaseAddr, 4*1024*1024);

  //Calculate the offset to the 2x2 lod
  lodOffset = 256*256 + 128*128 + 64*64 + 32*32 + 16*16 + 8*8 + 4*4;
  
  address = ((FxU32)sst) - SST_3D_OFFSET + SST_TEX0_OFFSET;

  for(index=0; index<2*2; index++)
    halStore8((void *)(address + lodOffset + index), (FxU8)paletteEntry);

  GDBG_INFO(1, "downloadTexture end\n");
}

void drawQuad(SstRegs *sst, FxU32 tileWidth, FxU32 tileHeight, 
	      FxU32 xOffset, FxU32 yOffset, FxU32 paletteEntry)
{
  float s, t;

  GDBG_INFO(1, "drawQuad begin\n");

  s = (float)(16 * (paletteEntry % 16));
  t = (float)(16 * (paletteEntry / 16));

  SET(sst->sSetupMode, SST_SETUP_Wfbi | SST_SETUP_ST0 | SST_SETUP_FAN);
  
  SETF(sst->sVx, (float)xOffset);
  SETF(sst->sVy, (float)yOffset);
  SETF(sst->sSow0, 0);
  SETF(sst->sTow0, 0);
  SETF(sst->sOowfbi, 1);
  SET(sst->sBeginTriCMD, 0);    	
  
  SETF(sst->sVx, (float)(xOffset + tileWidth));
  SETF(sst->sVy, (float)yOffset);
  SETF(sst->sSow0, 255);
  SETF(sst->sTow0, 0);
  SETF(sst->sOowfbi, 1);
  SET(sst->sDrawTriCMD, 0);    	
  
  SETF(sst->sVx, (float)(xOffset + tileWidth));
  SETF(sst->sVy, (float)(yOffset + tileHeight));
  SETF(sst->sSow0, 255);
  SETF(sst->sTow0, 255);
  SETF(sst->sOowfbi, 1);
  SET(sst->sDrawTriCMD, 0);
  
  SETF(sst->sVx, (float)xOffset);
  SETF(sst->sVy, (float)(yOffset + tileHeight));
  SETF(sst->sSow0, 0);
  SETF(sst->sTow0, 255);
  SETF(sst->sOowfbi, 1);
  SET(sst->sDrawTriCMD, 0);

  //Idle for fun
  fxHalIdle(sst);

  GDBG_INFO(1, "drawQuad end\n");
}

float readQuad(SstRegs *sst, FxU32 *red, FxU32 *green, FxU32 *blue,
	       FxU32 tileWidth, FxU32 tileHeight, FxU32 xOffset, FxU32 yOffset, 		
	       FxU32 expectedR, FxU32 expectedG, FxU32 expectedB)
{
  FxU32 address, baseAddress;
  FxU32 i,j, x, y, index;
  FxU32 color;
  FxU32 nIncorrect;
  float percentageIncorrect;

  GDBG_INFO(1, "readQuad begin\n");

  baseAddress = ((FxU32)sst) - SST_3D_OFFSET + SST_RAW_LFB_OFFSET;
  nIncorrect = 0;

  for(i=0; i<tileHeight; i++)
    for(j=0; j<tileWidth; j++)
      {
	x = xOffset + i;
	y = yOffset + j;

	index = BLOCK_WIDTH * i + j;
	
	address = baseAddress + (y * diago.width * 4) + x * 4;
	color = halLoad32((void *)address);
	
	red[index]   = (color >> 16) & 0xFF;
	green[index] = (color >> 8)  & 0xFF;
	blue[index]  = (color >> 0)  & 0xFF;
	GDBG_INFO(4, "%3d,%3d: %02x %02x %02x\n", x, y, red[index], green[index], blue[index]);

	if((red[index]   != expectedR) || 
	   (green[index] != expectedG) || 
	   (blue[index]  != expectedB))
	  {
	    GDBG_INFO(5, "expected %02x %02x %02x but got %02x %02x %02x\n",
		      expectedR, expectedG, expectedB,
		      red[index], green[index], blue[index]);
	    
	    nIncorrect++;
	  }
      }
  
  if(nIncorrect)
    {
      percentageIncorrect = (float)nIncorrect / (float)(tileWidth * tileWidth);
      GDBG_INFO(0, "%d pixels of %d pixels (%3.1f%%) are incorrect\n",
		nIncorrect, tileWidth*tileHeight, 100.f * percentageIncorrect);	       
    }
  else
    percentageIncorrect = 0;


  GDBG_INFO(1, "readQuad end\n");  

  return(percentageIncorrect);
}

FxU32 reverseBits(FxU32 value, FxU32 width)
{
  FxU32 result=0;
  FxU32 i;

  for(i=0; i<width; i++)
    if((1<<i) & value)
      result |= (1<<(width-1-i));	

  return(result);
}

void setup(SstRegs *sst)
{
  SstIORegs *sstio;
  FxI32 tmu;

  sstio = (SstIORegs *)SST_IO_ADDRESS(sst);

  //Set up shit so the pixel value is just the TMU texel value
  if(diago.trex < 0) 
    tmu = iRandom(1);
  else 
    tmu = diago.trex;
  
  GDBG_INFO(3, "Testing tmu %d\n", tmu);

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
  
  //Force lod to 2x2 size
  SET(sst->tLOD, ((7<<2) << SST_LODMIN_SHIFT) | ((7<<2) << SST_LODMAX_SHIFT));
  
  //Set lfbMemoryConfig so tiled memory starts high up
  SET(sstio->lfbMemoryConfig, SST_RAW_LFB_TILE_BEGIN_PAGE);
  
  //Set up 32bpp
  SET(sst->renderMode, SST_RM_32BPP | SST_RM_RGBA_WMASK);
  SET(sst->colBufferStride, diago.width*4);  

  if(diago.rgb != 32)
    {
      GDBG_ERROR("paltest2::setup", "Must run in f'n 32bpp (-7 option)\n");
      DIAG_FAIL();
    }
}

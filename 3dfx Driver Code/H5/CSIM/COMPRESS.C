/*-*-c++-*-*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

#ifdef HWC_BUILD_SST2
#include <fxhwc.h>
#include <sst2.h>
#include "sst2csim.h"
#include "hwccb.h"
#else
#include <h3.h>
#include "h3sim.h"
#endif

#include "compress.h"

#define FIXED_POINT 12

//Private function declarations
//3Dfx decompression stuff
static FxU32 getBitRanges(CCBlock *block, FxU32 nPairs, ...);
static void decompress3Dfx(FxI32 *pixelsOut, CCBlock blockIn);
static void decompress3DfxAlpha(FxI32 *pixelsOut, CCBlock blockIn);
static void decompress3DfxChroma(FxI32 *pixelsOut, CCBlock blockIn);
static void decompress3DfxMixed(FxI32 *pixelsOut, CCBlock blockIn);
static void decompress3DfxHi(FxI32 *pixelsOut, CCBlock blockIn);

typedef struct
{
  FxU32 r, g, b, a;
} Color;

static void decompressDXT1(FxI32 *pixelsOut, CCBlock blockIn);
static void decompressFourByFourDXT1(FxI32 *pixelsOut, FxU32 highWord, FxU32 lowWord, FxU32 format);
static void decompressDXT2(FxI32 *pixelsOut, CCBlock blockIn);  //DXT3 is the same 
static void decompressDXT4(FxI32 *pixelsOut, CCBlock blockIn);  //DXT5 is the same
static FxU32 divideByThree(FxU32 n);
static FxU32 divideByFive(FxU32 n);
static FxU32 divideBySix(FxU32 n);
static FxU32 divideBySeven(FxU32 n);
static void expand565To8888(FxU16 color565, Color *result);
static void expand565To108108(FxU16 color565, Color *result);



//Public Functions
#ifdef HWC_BUILD_SST2
FX_EXPORT void FX_CSTYLE
sst2CSimDecodeCompressed(Sst2CSimTaCache *pCache)
{
  FxU32 decompressedTexels[32];
  FxU32 index, numTexels;
  CCBlock block;       	
  
  //Pack the texelBlock into a CCBlock structure;
  block.w[0] = pCache->rawData[0];
  block.w[1] = pCache->rawData[1];
  block.w[2] = pCache->rawData[2];
  block.w[3] = pCache->rawData[3];
  
  GDBG_INFO(201, "sst2CSimDecodeCompressed block = 0x%08x_%08x_%08x_%08x\n",
	    block.w[3], block.w[2], block.w[1], block.w[0]);

  switch ( pCache->format ) {
  case SST_TA_FXT1:
    decompress3Dfx((FxI32*) decompressedTexels, block);
    break;
  case SST_TA_DXT1:
      decompressFourByFourDXT1((FxI32*) decompressedTexels, block.w[1], block.w[0], 1);
      break;
  case SST_TA_DXT2:
  case SST_TA_DXT3:
      decompressDXT2((FxI32*) decompressedTexels, block);
      break;
  case SST_TA_DXT4:
  case SST_TA_DXT5:
      decompressDXT4((FxI32*) decompressedTexels, block);
      break;
  default:
      break;
  }

  numTexels = pCache->texelInfo->minWidth*pCache->texelInfo->minHeight; 

  for ( index = 0; index < numTexels; index++) {
    Sst2Color *tColor = & pCache->colorData[index];
    tColor->r = (FxU8)((decompressedTexels[index] >> 16) & 0xFF);  //red
    tColor->g = (FxU8)((decompressedTexels[index] >> 8)  & 0xFF);  //green
    tColor->b = (FxU8)((decompressedTexels[index] >> 0)  & 0xFF);  //blue
    tColor->a = (FxU8)((decompressedTexels[index] >> 24) & 0xFF);  //alpha

    GDBG_INFO(201, "decompressTexels[0x%2x] = 0x%08x\n", index, decompressedTexels[index]);
  }
  pCache->colorValid = FXTRUE;
}
#else
FX_EXPORT FxBool FX_CSTYLE csimDecompressTexel(FxU32 *texelBlock, FxU32 tMode, FxU32 s, FxU32 t, FxU8 *result)
{
  FxU32 decompressTexels[32];
  FxU32 index, i;
  FxU32 nDecompressedTexels;
  FxU32 sBlock, tBlock;
  FxU32 sOriginal, tOriginal;
  FxU32 width, height;
  
  sOriginal = s;
  tOriginal = t;

  //Decompress the whole chunk of texels
  csimDecompressTexels(texelBlock, tMode, decompressTexels);
  
  //Select the proper texel to return
  switch(tMode & SST_TFORMAT)
    {
    case SST_3DFX_COMPRESSED:      
    case SST_DXT1:
      width = 8;
      height = 4;

      sBlock = s & ~7;
      tBlock = t & ~3;
      
      s = s & 7;
      t = t & 3;

      if(s >= 4)
	{
	  s -= 4;
	  index = 16 + (t<<2) + s;
	}
      else
	{
	  index = (t<<2) + s;
	}
      nDecompressedTexels = 32;
      break;

    case SST_DXT2:
    case SST_DXT4:
      width = 4;
      height = 4;

      sBlock = s & ~3;
      tBlock = t & ~3;

      s = s & 3;
      t = t & 3;
      
      index = (t<<2) + s;     
 
      nDecompressedTexels = 16;
      break;
      
    default:
      assert(0);
    }
  
  result[0] = (FxU8)((decompressTexels[index] >> 16) & 0xFF);  //red
  result[1] = (FxU8)((decompressTexels[index] >> 8)  & 0xFF);  //green
  result[2] = (FxU8)((decompressTexels[index] >> 0)  & 0xFF);  //blue
  result[3] = (FxU8)((decompressTexels[index] >> 24) & 0xFF);  //alpha

  GDBG_INFO(201, "decompressTexels block result (ARGB format)\n");
  for(t=0; t<height; t++)
    {    
      GDBG_INFO_MORE(201, "      ");        
      for(s=0; s<width; s++)
	{
	  if(s >= 4)
	    i = 16 + (s-4) + t*4;
	  else
	    i = s + t*4;
	  
	  GDBG_INFO_MORE(201, "%08x ", decompressTexels[i]);
	}      
      GDBG_INFO_MORE(201, "\n");
    }

  GDBG_INFO(201, "decompressTexels block s,t values in hex\n");
  for(t=0; t<height; t++)
    {            
      for(s=0; s<width; s++)
	{	  
	  GDBG_INFO_MORE(201, "%3x,%-03x ", s+sBlock, t+tBlock);
	}      
      GDBG_INFO_MORE(201, "\n");
    }

  GDBG_INFO(201, "decompressTexels: Selecting texel 0x%x(%d) = %08x  (s=0x%x,t=0x%x)\n",
	    index, index, decompressTexels[index], sOriginal, tOriginal);

  return(FXTRUE);
}

FxBool csimDecompressTexels(FxU32 *texelBlock, FxU32 tMode, FxU32 *decompressedTexels)
{
  CCBlock block;       	
  
  assert(tMode & SST_COMPRESSED_TEXTURES);  //Make sure we have a compressed format
  
  
  //Pack the texelBlock into a CCBlock structure;
  block.w[0] = texelBlock[0];
  block.w[1] = texelBlock[1];
  block.w[2] = texelBlock[2];
  block.w[3] = texelBlock[3];
  
  GDBG_INFO(201, "csimDecompressTexels block = 0x%08x_%08x_%08x_%08x\n",
	    block.w[3], block.w[2], block.w[1], block.w[0]);

  switch(tMode & SST_TFORMAT)
    {
    case SST_3DFX_COMPRESSED:
      GDBG_INFO(201, "3dfx compressed format\n");
      decompress3Dfx(decompressedTexels, block);
      break;

    case SST_DXT1:
      GDBG_INFO(201, "DXT1 compressed format\n");
      decompressDXT1(decompressedTexels, block);
      break;

    case SST_DXT2:
      GDBG_INFO(201, "DXT2/DXT3 compressed format\n");
      decompressDXT2(decompressedTexels, block);
      break;

    case SST_DXT4:
      GDBG_INFO(201, "DXT4/DXT5 compressed format\n");
      decompressDXT4(decompressedTexels, block);
      break;

    default:
      assert(0);
    }
  
  return(FXTRUE);
}
#endif


//Private data
static int channelShift[3] = {0, 8, 16};


void decompressDXT1(FxI32 *pixelsOut, CCBlock blockIn)
{
  decompressFourByFourDXT1(pixelsOut, blockIn.w[1], blockIn.w[0], 1);
  decompressFourByFourDXT1(pixelsOut+16, blockIn.w[3], blockIn.w[2], 1);
}

static void decompressFourByFourDXT1(FxI32 *pixelsOut, FxU32 highWord, FxU32 lowWord, FxU32 format)
{
  Color color[4];
  FxU16 color0_565, color1_565;
  FxI32 i, index;

  color0_565 = (FxU16)(lowWord & 0xFFFF);
  color1_565 = (FxU16)((lowWord >> 16) & 0xFFFF);

  expand565To108108(color0_565, &color[0]);
  expand565To108108(color1_565, &color[1]);

  GDBG_INFO(201, "DXT1 block color0_565 = 0x%x   color1_565 = 0x%x\n", color0_565, color1_565);
  
  //Select if 3 or 4 color mode should be used
  //In non-DXT1 modes, the 4 color mode is always used
  if((color0_565 > color1_565) || (format != 1))
    {
      GDBG_INFO(201,  "color0_565 > color1_565\n");
      //This is the 4-color case      
      //The (2<<10) business is for rounding
      color[2].r = divideByThree((color[0].r<<1) + color[1].r);
      color[2].g = divideByThree((color[0].g<<1) + color[1].g);
      color[2].b = divideByThree((color[0].b<<1) + color[1].b);
      color[2].a = 0xFF;
      assert(color[2].r >= 0 && color[2].r < 1024);
      assert(color[2].g >= 0 && color[2].g < 256);
      assert(color[2].b >= 0 && color[2].b < 1024);

      color[3].r = divideByThree((color[1].r<<1) + color[0].r);
      color[3].g = divideByThree((color[1].g<<1) + color[0].g);
      color[3].b = divideByThree((color[1].b<<1) + color[0].b);
      color[3].a = 0xFF;      
      assert(color[3].r >= 0 && color[3].r < 1024);
      assert(color[3].g >= 0 && color[3].g < 256);
      assert(color[3].b >= 0 && color[3].b < 1024);
    }
  else
    {
      GDBG_INFO(201,  "color0_565 <= color1_565\n");
      //This is the 3-color with transparency case
      color[2].r = (color[0].r + color[1].r) >> 1;
      color[2].g = (color[0].g + color[1].g) >> 1;
      color[2].b = (color[0].b + color[1].b) >> 1;
      color[2].a = 0xFF;
      assert(color[2].r >= 0 && color[2].r < 1024);
      assert(color[2].g >= 0 && color[2].g < 256);
      assert(color[2].b >= 0 && color[2].b < 1024);
      
      //Transparent color
      color[3].r = 0;
      color[3].g = 0;
      color[3].b = 0;
      color[3].a = 0;
    }

  for(i=0; i<4; i++)
    GDBG_INFO(201,  "  RGBA color[%d] = 0x%x_%x_%x_%x\n", i, color[i].r, color[i].g, color[i].b, color[i].a); 
	  

  //Convert color from 108108 to 8888
  for(i=0; i<4; i++)
    {
      color[i].r >>= 2;
      color[i].b >>= 2;
    }
  
  //Check the palette
  for(i=0; i<4; i++)
    {
      assert(color[i].r < 256);
      assert(color[i].g < 256);
      assert(color[i].b < 256);
      assert(color[i].a < 256);
    }

  for(i=0; i<16; i++)
    {
      index = GETBIT(highWord, i<<1) | (GETBIT(highWord, (i<<1) + 1) << 1);

      pixelsOut[i] = (color[index].r << 16) | (color[index].g << 8) |
	(color[index].b << 0) | (color[index].a << 24);
    }
}

void decompressDXT2(FxI32 *pixelsOut, CCBlock blockIn)
{
  //This is the explicitly coded 4bpt alpha format (DXT2 and DXT3)
  //DXT2 is for premultiplied alpha
  //DXT3 is for normal alpha
  //For decompression, DXT2 and DXT3 are identical
  FxI32 i, alpha;

  GDBG_INFO(201, "DXT2/DXT3 format\n");

  //Decompress the color information
  decompressFourByFourDXT1(pixelsOut, blockIn.w[3], blockIn.w[2], 2);

  //Now decompress the alpha information
  for(i=0; i<16; i++)
    {
      alpha = CC_GETBIT(blockIn, i*4) | 
	(CC_GETBIT(blockIn, i*4 + 1) << 1) |
	(CC_GETBIT(blockIn, i*4 + 2) << 2) | 
	(CC_GETBIT(blockIn, i*4 + 3) << 3);
      
      //Convert to 8 bits
      alpha = alpha | (alpha << 4);
	  
      //Put the alpha in the output
      pixelsOut[i] = (pixelsOut[i] & 0xFFFFFF) | ((alpha & 0xFF) << 24);
    }
}

void decompressDXT4(FxI32 *pixelsOut, CCBlock blockIn) 
{
  //This is the interpolated alpha format (DXT4 and DXT5)
  //DXT4 is for premultiplied alpha
  //DXT% is for normal alpha
  //For decompression, DXT4 and DXT5 are identical
  FxU32 alpha[8];
  FxI32 i, index;

  GDBG_INFO(201, "DXT4/DXT5 format\n");

  //Decompress the color information
  decompressFourByFourDXT1(pixelsOut, blockIn.w[3], blockIn.w[2], 4);
  
  //Now decompress the alpha information
  alpha[0] = blockIn.w[0] & 0xFF;
  alpha[1] = (blockIn.w[0] >> 8) & 0xFF;


  //Calculate the alpha table
  if(alpha[0] > alpha[1])
    {
      //8 alpha case
      for(i=2; i<8; i++)
	alpha[i] = divideBySeven((8-i) * alpha[0] + (i-1) * alpha[1]);
    }
  else
    {
      //6 alpha case
      for(i=2; i<6; i++)
	alpha[i] = divideByFive((6-i) * alpha[0] + (i-1) * alpha[1]);
      
      alpha[6] = 0;
      alpha[7] = 255;
    }

  //Make sure the alpha table looks ok
  for(i=0; i<8; i++)
    GDBG_INFO(201, "alpha[%d] = 0x%x\n", i, alpha[i]);

  for(i=0; i<8; i++)
      assert(alpha[i] >= 0 && alpha[i] < 256);

  //Fill in the alpha channel
  for(i=0; i<16; i++)
    {
      index = CC_GETBIT(blockIn, i*3 + 16) | 
	(CC_GETBIT(blockIn, i*3 + 1 + 16) << 1) |
	(CC_GETBIT(blockIn, i*3 + 2 + 16) << 2) ;
      
      //Put the alpha in the output
      pixelsOut[i] = (pixelsOut[i] & 0xFFFFFF) | (alpha[index] << 24);	
    }
}


void expand565To8888(FxU16 color565, Color *result)
{  
  //Find channels
  //Add the MSB to the bottom of each channel
  result->r = ((color565 >> 8) & 0xF8) | ((color565 >> 13) & 0x7);
  result->g = ((color565 >> 3) & 0xFC) | ((color565 >> 9) & 0x3);
  result->b = ((color565 << 3) & 0xF8) | ((color565 >> 2) & 0x7);  
  result->a = 0xFF;  //Set opaque as default
}

void expand565To108108(FxU16 color565, Color *result)
{  
  FxU32 r, b;

  r = (color565 & 0xF800) >> 11;
  b = (color565 & 0x001F);

  //Find channels
  //Add the MSB to the bottom of each channel
  result->r = (r<<5) | r;
  result->g = ((color565 >> 3) & 0xFC) | ((color565 >> 9) & 0x3);
  result->b = (b<<5) | b;
  result->a = 0xFF;  //Set opaque as default
}


void decompress3Dfx(FxI32 *pixelsOut, CCBlock blockIn)
{
  FxU32 mode;
  
  GDBG_INFO(201, "decompress3Dfx blockIn = 0x%08x_%08x_%08x_%08x\n",
	    blockIn.w[3], blockIn.w[2], blockIn.w[1], blockIn.w[0]);

  //Figure out which mode this block uses
  mode = getBitRanges(&blockIn, 1, 127,125);

  switch(mode)
    {
    case 0:
    case 1:
      decompress3DfxHi(pixelsOut, blockIn);
      break;
      
    case 2:
      decompress3DfxChroma(pixelsOut, blockIn);
      break;
      
    case 3:
      decompress3DfxAlpha(pixelsOut, blockIn);
      break;
      
    case 4:
    case 5:
    case 6:
    case 7:
      decompress3DfxMixed(pixelsOut, blockIn);
      break;
    }
}


//************************************************************
//
//                       alpha mode
//
//************************************************************
static void decompress3DfxAlpha(FxI32 *pixelsOut, CCBlock blockIn)
{
  FxU32 lerpBit, block, i, paletteIndex;
  Color color[3];   //The four colors contained in the 128 bit datum
  Color palette[4]; //The four colors that the 2-bit texel selects choose between

  GDBG_INFO(201, "3dfx compressed texture: alpha mode\n");
  
  lerpBit = CC_GETBIT(blockIn, 124);

  //Extract the 3 colors from the 128 bit block
  //Look in the SST2 VTA Cache document for info about this shit
  color[2].r = getBitRanges(&blockIn, 2, 108,104, 108,104);
  color[2].g = getBitRanges(&blockIn, 2, 103,99,  103,101);
  color[2].b = getBitRanges(&blockIn, 2, 98,94,   98,94);
  color[2].a = getBitRanges(&blockIn, 2, 123,119, 123,121);

  color[1].r = getBitRanges(&blockIn, 2, 93,89,   93,89);
  color[1].g = getBitRanges(&blockIn, 2, 88,84,   88,86);
  color[1].b = getBitRanges(&blockIn, 2, 83,79,   83,79);
  color[1].a = getBitRanges(&blockIn, 2, 118,114, 118,116); 

  color[0].r = getBitRanges(&blockIn, 2, 78,74,   78,74);
  color[0].g = getBitRanges(&blockIn, 2, 73,69,   73,71);
  color[0].b = getBitRanges(&blockIn, 2, 68,64,   68,64);
  color[0].a = getBitRanges(&blockIn, 2, 113,109, 113,111);

  for(block=0; block<2; block++)
    {
      //Fill the palette
      if(lerpBit)
	{	  
	  if(block == 0)
	    { //texels 0-15
	      memcpy(&palette[0], &color[0], sizeof(Color));
	      memcpy(&palette[3], &color[1], sizeof(Color));
	    }
	  else
	    { //texels 16-31
	      memcpy(&palette[0], &color[2], sizeof(Color));
	      memcpy(&palette[3], &color[1], sizeof(Color));
	    }
	  
	  palette[1].r = divideByThree((palette[0].r<<1) + palette[3].r);
	  palette[1].g = divideByThree((palette[0].g<<1) + palette[3].g);
	  palette[1].b = divideByThree((palette[0].b<<1) + palette[3].b);
	  palette[1].a = divideByThree((palette[0].a<<1) + palette[3].a);
	  
	  palette[2].r = divideByThree(palette[0].r + (palette[3].r<<1));
	  palette[2].g = divideByThree(palette[0].g + (palette[3].g<<1));
	  palette[2].b = divideByThree(palette[0].b + (palette[3].b<<1));
	  palette[2].a = divideByThree(palette[0].a + (palette[3].a<<1));
	}
      else
	{ //lerpBit = 0:  3 colors + transparent black
	  memcpy(&palette[0], &color[0], sizeof(Color));
	  memcpy(&palette[1], &color[1], sizeof(Color));
	  memcpy(&palette[2], &color[2], sizeof(Color));
	  palette[3].r = palette[3].g = palette[3].b = palette[3].a = 0;
	}

      //Convert palette from 108108 to 8888
      for(i=0; i<4; i++)
	{
	  palette[i].r >>= 2;
	  palette[i].b >>= 2;
	}

      //Check the palette
      for(i=0; i<4; i++)
	{
	  assert(palette[i].r < 256);
	  assert(palette[i].g < 256);
	  assert(palette[i].b < 256);
	  assert(palette[i].a < 256);
	}

      //Fill the output texels
      for(i=0; i<16; i++)
	{
	  paletteIndex = getBitRanges(&blockIn, 1, 32 * block + i * 2 + 1, 32 * block + i * 2);


	  pixelsOut[i + 16 * block] = (palette[paletteIndex].a << 24) |
	    (palette[paletteIndex].r << 16) | (palette[paletteIndex].g << 8) |
	    (palette[paletteIndex].b);
	}
    }
}

//************************************************************
//
//                       cc-chroma
//
//************************************************************
void decompress3DfxChroma(FxI32 *pixelsOut, CCBlock blockIn)
{
  Color color[4];   //The four colors contained in the 128 bit datum
  FxU32 i, index;

  GDBG_INFO(201, "3dfx compressed texture: cc-chroma mode\n");
  
  color[3].r = getBitRanges(&blockIn, 2, 123,119, 123,121);
  color[3].g = getBitRanges(&blockIn, 2, 118,114, 118,116);
  color[3].b = getBitRanges(&blockIn, 2, 113,109, 113,111);
  color[3].a = 0xFF;

  color[2].r = getBitRanges(&blockIn, 2, 108,104, 108,106);
  color[2].g = getBitRanges(&blockIn, 2, 103, 99, 103,101);
  color[2].b = getBitRanges(&blockIn, 2,  98, 94,  98, 96);
  color[2].a = 0xFF;

  color[1].r = getBitRanges(&blockIn, 2,  93, 89,  93, 91);
  color[1].g = getBitRanges(&blockIn, 2,  88, 84,  88, 86);
  color[1].b = getBitRanges(&blockIn, 2,  83, 79,  83, 81);
  color[1].a = 0xFF;

  color[0].r = getBitRanges(&blockIn, 2,  78, 74,  78, 76);
  color[0].g = getBitRanges(&blockIn, 2,  73, 69,  73, 71);
  color[0].b = getBitRanges(&blockIn, 2,  68, 64,  68, 66);
  color[0].a = 0xFF;

  //Translate the colors and generate the output
  for(i=0; i<32; i++)
    {
      index = getBitRanges(&blockIn, 1, 2*i + 1, 2*i);
      pixelsOut[i] = (color[index].r << 16) | (color[index].g << 8) | (color[index].b) | (color[index].a << 24);
    }
}


//************************************************************
//
//                       mixed mode
//
//************************************************************
void decompress3DfxMixed(FxI32 *pixelsOut, CCBlock blockIn)
{
  int i, paletteIndex, alphaBit, block;
  Color color[4];   //The four colors contained in the 128 bit datum
  Color palette[4]; //The four colors that the 2-bit texel selects choose between

  GDBG_INFO(201, "3dfx compressed texture: mixed mode\n");

  alphaBit = CC_GETBIT(blockIn, 124);

  //Extract the 4 colors from the 128 bit block
  //Look in the SST2 VTA Cache document for info about this shit
  color[3].r = getBitRanges(&blockIn, 2, 123,119, 123,119);
  color[3].g = getBitRanges(&blockIn, 3, 118,114, 126,126, 118,117);
  color[3].b = getBitRanges(&blockIn, 2, 113,109, 113,109);
  color[3].a = 0xFF;
  
  color[2].r = getBitRanges(&blockIn, 2, 108,104, 108,104);
  if(alphaBit)
    color[2].g = getBitRanges(&blockIn, 2, 103,99, 103,101);
  else
    {
      color[2].g = getBitRanges(&blockIn, 3, 103,99, 33,33, 103,102);
      color[2].g ^= CC_GETBIT(blockIn, 126) << 2;
    }
  color[2].b = getBitRanges(&blockIn, 2, 98,94, 98,94);
  color[2].a = 0xFF;

  color[1].r = getBitRanges(&blockIn, 2, 93,89, 93,89);
  color[1].g = getBitRanges(&blockIn, 3, 88,84, 125,125, 88,87);
  color[1].b = getBitRanges(&blockIn, 2, 83,79, 83,79);
  color[1].a = 0xFF;

  color[0].r = getBitRanges(&blockIn, 2, 78,74, 78,74);
  if(alphaBit)
    color[0].g = getBitRanges(&blockIn, 2, 73,69, 73,71);
  else
    {
      color[0].g = getBitRanges(&blockIn, 3, 73,69, 1,1, 73,72);
      color[0].g ^= CC_GETBIT(blockIn, 125) << 2;
    }
  color[0].b = getBitRanges(&blockIn, 2, 68,64, 68,64);
  color[0].a = 0xFF;  

  for(block = 0; block<2; block++)
    {
      //block 0: texels 0-15,   block 1: texels 16-31
      //Fill the palette
      if(alphaBit)
	{  //1 Intertoplated color + 1 transparent black
	  memcpy(&palette[0], &color[0 + 2*block], sizeof(Color));
	  memcpy(&palette[2], &color[1 + 2*block], sizeof(Color));
	  
	  palette[1].r = (color[0 + 2*block].r + color[1 + 2*block].r) >> 1;
	  palette[1].g = (color[0 + 2*block].g + color[1 + 2*block].g) >> 1;
	  palette[1].b = (color[0 + 2*block].b + color[1 + 2*block].b) >> 1;
	  palette[1].a = 0xFF;
	  
	  palette[3].r = palette[3].g = palette[3].b = palette[3].a = 0;      
	}
      else
	{  //2 Interpolated colors
	  memcpy(&palette[0], &color[0 + 2*block], sizeof(Color));
	  memcpy(&palette[3], &color[1 + 2*block], sizeof(Color));
	  
	  palette[1].r = divideByThree((color[0 + 2*block].r<<1) + color[1 + 2*block].r);
	  palette[1].g = divideByThree((color[0 + 2*block].g<<1) + color[1 + 2*block].g);
	  palette[1].b = divideByThree((color[0 + 2*block].b<<1) + color[1 + 2*block].b);
	  palette[1].a = 0xFF;
	  
	  palette[2].r = divideByThree(color[0 + 2*block].r + (color[1 + 2*block].r<<1));
	  palette[2].g = divideByThree(color[0 + 2*block].g + (color[1 + 2*block].g<<1));
	  palette[2].b = divideByThree(color[0 + 2*block].b + (color[1 + 2*block].b<<1));
	  palette[2].a = 0xFF;
	}

      //Convert palette from 108108 to 8888
      for(i=0; i<4; i++)
	{
	  palette[i].r >>= 2;
	  palette[i].b >>= 2;
	}

      //Check the palette
      for(i=0; i<4; i++)
	{
	  assert(palette[i].r < 256);
	  assert(palette[i].g < 256);
	  assert(palette[i].b < 256);
	  assert(palette[i].a < 256);
	}

      //Fill the output texels
      for(i=0; i<16; i++)
	{
	  paletteIndex = getBitRanges(&blockIn, 1, 32 * block + i * 2 + 1, 32 * block + i * 2);
	  pixelsOut[i + 16 * block] = (palette[paletteIndex].a << 24) |
	    (palette[paletteIndex].r << 16) | (palette[paletteIndex].g << 8) |
	    (palette[paletteIndex].b);
	}
    }    
}


//************************************************************
//
//                        cc-hi
//
//************************************************************
void decompress3DfxHi(FxI32 *pixelsOut, CCBlock blockIn)
{
  FxU32 i, index;
  Color color[8];

  GDBG_INFO(201, "3dfx compressed texture: cc-hi mode\n");

  //Set up the colors
  color[0].r = getBitRanges(&blockIn, 2, 110,106, 110,106);
  color[0].g = getBitRanges(&blockIn, 2, 105,101, 105,103);
  color[0].b = getBitRanges(&blockIn, 2, 100, 96, 100, 96);
  color[0].a = 0xFF;

  color[6].r = getBitRanges(&blockIn, 2, 125,121, 125,121);
  color[6].g = getBitRanges(&blockIn, 2, 120,116, 120,118);
  color[6].b = getBitRanges(&blockIn, 2, 115,111, 115,111);
  color[6].a = 0xFF;

  //Interpolate colors
  for(i=1; i<6; i++)
    {
      color[i].r = divideBySix((color[0].r * (6-i)) + (color[6].r * (i)));
      color[i].g = divideBySix((color[0].g * (6-i)) + (color[6].g * (i)));
      color[i].b = divideBySix((color[0].b * (6-i)) + (color[6].b * (i)));
      color[i].a = 0xFF;
    }

  //Set color 7 to transparent black
  color[7].r = 0;
  color[7].g = 0;
  color[7].b = 0;
  color[7].a = 0;

  //Convert color from 108108 to 8888
  for(i=0; i<8; i++)
    {
      color[i].r >>= 2;
      color[i].b >>= 2;
    }
  
  //Check the colors
  for(i=0; i<8; i++)
    {
      assert(color[i].r < 256);
      assert(color[i].g < 256);
      assert(color[i].b < 256);
      assert(color[i].a < 256);
    }
  
  //Translate the colors and generate the output
  for(i=0; i<32; i++)
    {
      index = getBitRanges(&blockIn, 1, 3*i + 2, 3*i);
      pixelsOut[i] = (color[index].r << 16) | (color[index].g << 8) | (color[index].b) | (color[index].a << 24);
    }
}

//This function returns a set of bit ranges concatenated
//getBitRanges(block, 2, 21,18, 3,2) returns {block[21:18], block[3:2]}
//The format is (block, msb0,lsb0, msb1,lsb1, ...)
//Each msb value must be >= its lsb value
FxU32 getBitRanges(CCBlock *block, FxU32 nPairs, ...)
{
  va_list argPtr;
  FxU32 lsb, msb, lsbWord, msbWord, mask, result, thisResult, i, nBits;
  
  result = 0;
  nBits = 0;

  /*
  GDBG_INFO(0, "getBitRanges 0x%08x_%08x_%08x_%08x\n",
	    block->w[3], block->w[2], block->w[1], block->w[0]);
  */

  va_start(argPtr, nPairs);
  for(i=0; i<nPairs; i++)
    {
      msb = va_arg(argPtr, FxU32);
      lsb = va_arg(argPtr, FxU32);

      //GDBG_INFO(0, "getBitRanges  msb=%d lsb=%d\n", msb, lsb);

      assert(msb >= lsb);

      result <<= (msb-lsb + 1);

      nBits += msb - lsb + 1;
      assert(nBits <= 32);

      if(msb - lsb < 31)
	mask = (2<<(msb-lsb))-1;
      else
	mask = 0xFFFFFFFF;
      
      msbWord = msb >> 5;
      lsbWord = lsb >> 5;
      
      //GDBG_INFO(0, "msbWord=%d lsbWord=%d\n", msbWord, lsbWord);
      //GDBG_INFO(0, "mask 0x%x  lsb&31=%d\n", mask, lsb&31);

      if(msbWord == lsbWord)
	thisResult = (block->w[lsbWord] >> (lsb & 31)) & mask;
      else
	thisResult = ((block->w[lsbWord] >> (lsb & 31)) |
		      (block->w[msbWord] << (32 - (lsb & 31)))) & mask;      

		      //GDBG_INFO(0, "getBitRanges  thisResult =0x%x\n", thisResult);

      result |= thisResult;
    }    
  va_end(argPtr);


  //GDBG_INFO(0, "getBitRanges  result =0x%x\n", result);
  
  return(result);
}


//Bit-accurate divide
FxU32 divideByThree(FxU32 n)
{
  return((n+1)/3);
}

//Bit-accurate divide
FxU32 divideByFive(FxU32 n)
{
  return((n+2)/5);
}

//Bit-accurate divide
FxU32 divideBySix(FxU32 n)
{
  return((n+2)/6);
}

//Bit-accurate divide
FxU32 divideBySeven(FxU32 n)
{
  return((n+3)/7);
}

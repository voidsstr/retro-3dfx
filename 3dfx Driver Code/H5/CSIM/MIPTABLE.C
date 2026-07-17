#include <assert.h>
#include <h3.h>

#include "miptable.h"



//Given various information about a linear texture, this function
//returns the offset of the desired mipmap. The offset is a 
//byte offset relative to the 256x256 mipmap starting at 0.
FX_EXPORT FxI32 FX_CSTYLE sstLinearMipMapOffset(FxU32 lod, FxU32 logAspectRatio, FxU32 bitsPerTexel, 
						FxBool bigAssTexture, FxBool wideTexture, FxBool compressedTexture,
						FxBool splitTextures)
{
  FxI32 offset;

  //Check validity of inputs
  if(bigAssTexture)
    assert(lod <= 11);
  else
    assert(lod <= 8);

  assert(logAspectRatio < 4);
  assert(bitsPerTexel == 4 || bitsPerTexel == 8 || bitsPerTexel == 16 || bitsPerTexel == 32);
  assert(wideTexture == 0 || wideTexture == 1);

  //Figure out which table to use and return the value
  if(bigAssTexture)
    { //2048x2048 textures
      if(compressedTexture)
	{
	  if(bitsPerTexel == 4)
	    {
	      if(splitTextures)
		offset = _sstMipMapOffset_BigCompressedFourBitsPerTexelTextures_Tsplit[wideTexture][logAspectRatio][lod];
	      else
		offset = _sstMipMapOffset_BigCompressedFourBitsPerTexelTextures[wideTexture][logAspectRatio][lod];
	    }
	  else if(bitsPerTexel == 8)
	    {
	      if(splitTextures)
		offset = _sstMipMapOffset_BigCompressedEightBitsPerTexelTextures_Tsplit[wideTexture][logAspectRatio][lod];
	      else
		offset = _sstMipMapOffset_BigCompressedEightBitsPerTexelTextures[wideTexture][logAspectRatio][lod];
	    }
	  else
	    assert(0);
	}
      else //Non-compressed Textures
	{
	  //There's no such thing as a non-compressed 4bpt texture
	  if(bitsPerTexel == 4)
	    assert(0);
	  
	  if(splitTextures)
	    offset = _sstMipMapOffset_BigTextures_Tsplit[logAspectRatio][lod] * (FxI32)bitsPerTexel / 8;
	  else
	    offset = _sstMipMapOffset_BigTextures[logAspectRatio][lod] * (FxI32)bitsPerTexel / 8;
	}
    }
  else
    { //256x256 textures
      if(compressedTexture)
	{
	  if(bitsPerTexel == 4)
	    {
	      if(splitTextures)
		offset = _sstMipMapOffset_NormalCompressedFourBitsPerTexelTextures_Tsplit[wideTexture][logAspectRatio][lod];
	      else
		offset = _sstMipMapOffset_NormalCompressedFourBitsPerTexelTextures[wideTexture][logAspectRatio][lod];
	    }
	  else if(bitsPerTexel == 8)
	    {
	      if(splitTextures)
		offset = _sstMipMapOffset_NormalCompressedEightBitsPerTexelTextures_Tsplit[wideTexture][logAspectRatio][lod];
	      else
		offset = _sstMipMapOffset_NormalCompressedEightBitsPerTexelTextures[wideTexture][logAspectRatio][lod];
	    }
	  else
	    assert(0);
	}
      else //Non-compressed Textures
	{
	  //There's no such thing as a non-compressed 4bpt texture
	  if(bitsPerTexel == 4)
	    assert(0);
	  
	  if(splitTextures)
	    offset = _sstMipMapOffset_NormalTextures_Tsplit[logAspectRatio][lod] * (FxI32)bitsPerTexel / 8;
	  else
	    offset = _sstMipMapOffset_NormalTextures[logAspectRatio][lod] * (FxI32)bitsPerTexel / 8;
	}      
    }

  return(offset);
}

//This is a convenience function based on sstLinearMipMapOffset(). It's too bad
//that c sucks so much that you can't overload functions.
FX_EXPORT FxI32 FX_CSTYLE sstLinearMipMapOffset2(FxU32 lod, FxU32 tLOD, FxU32 textureMode)
{
  FxBool bigAssTexture, wideTexture, compressedTexture, splitTextures;
  FxU32 bitsPerTexel, logAspectRatio;

  if(tLOD & SST_TBIG)
    bigAssTexture = FXTRUE;
  else
    bigAssTexture = FXFALSE;

  if(tLOD & SST_LOD_S_IS_WIDER)
    wideTexture = FXTRUE;
  else
    wideTexture = FXFALSE;
  
  if(textureMode & SST_COMPRESSED_TEXTURES)
    compressedTexture = FXTRUE;
  else
    compressedTexture = FXFALSE;

  if(tLOD & SST_LOD_TSPLIT)
    splitTextures = FXTRUE;
  else
    splitTextures = FXFALSE;

  if(compressedTexture)
    {
      if(SST_T4BIT_COMPRESSED(textureMode))
	bitsPerTexel = 4;
      else if(SST_T8BIT_COMPRESSED(textureMode))
	bitsPerTexel = 8;
      else
	assert(0);
    }
  else
    {
      if(SST_T8BIT(textureMode))
	bitsPerTexel = 8;
      else if(SST_T16BIT(textureMode))
	bitsPerTexel = 16;
      else if(SST_T32BIT(textureMode))
	bitsPerTexel = 32;
      else
	assert(0);
    }

  logAspectRatio = (tLOD & SST_LOD_ASPECT) >> SST_LOD_ASPECT_SHIFT;

  return(sstLinearMipMapOffset(lod, logAspectRatio, bitsPerTexel,
			       bigAssTexture, wideTexture, compressedTexture,
			       splitTextures));
}


//Given various information about a linear texture, this function
//returns the size in bytes of the desired mipmap.
FX_EXPORT FxI32 FX_CSTYLE sstLinearMipMapSize(FxU32 lod, FxU32 logAspectRatio, FxU32 bitsPerTexel, 
					      FxBool bigAssTexture, FxBool wideTexture, FxBool compressedTexture)
{
  FxI32 size;

  //Check validity of inputs
  if(bigAssTexture)
    assert(lod <= 11);
  else
    assert(lod <= 8);

  assert(logAspectRatio < 4);
  assert(bitsPerTexel == 4 || bitsPerTexel == 8 || bitsPerTexel == 16 || bitsPerTexel == 32);
  assert(wideTexture == 0 || wideTexture == 1);

  //Figure out which table to use and return the value
  if(bigAssTexture)
    { //2048x2048 textures
      if(compressedTexture)
	{
	  if(bitsPerTexel == 4)
	    size = _sstMipMapSize_BigCompressedFourBitsPerTexelTextures[wideTexture][logAspectRatio][lod];
	  else if(bitsPerTexel == 8)	    
	    size = _sstMipMapSize_BigCompressedEightBitsPerTexelTextures[wideTexture][logAspectRatio][lod];
	  else
	    assert(0);
	}
      else //Non-compressed Textures
	{
	  //There's no such thing as a non-compressed 4bpt texture
	  if(bitsPerTexel == 4)
	    assert(0);
	  
	  size = _sstMipMapSize_BigTextures[logAspectRatio][lod] * bitsPerTexel / 8;
	}
    }
  else
    { //256x256 textures
      if(compressedTexture)
	{
	  if(bitsPerTexel == 4)
	    size = _sstMipMapSize_NormalCompressedFourBitsPerTexelTextures[wideTexture][logAspectRatio][lod];
	  else if(bitsPerTexel == 8)
	    size = _sstMipMapSize_NormalCompressedEightBitsPerTexelTextures[wideTexture][logAspectRatio][lod];
	  else
	    assert(0);
	}
      else //Non-compressed Textures
	{
	  //There's no such thing as a non-compressed 4bpt texture
	  if(bitsPerTexel == 4)
	    assert(0);
	  
	  size = _sstMipMapSize_NormalTextures[logAspectRatio][lod] * bitsPerTexel / 8;
	}      
    }

  return(size);  
}

//This is a convenience function based on sstLinearMipMapSize(). It's too bad
//that c sucks so much that you can't overload functions.
FX_EXPORT FxI32 FX_CSTYLE sstLinearMipMapSize2(FxU32 lod, FxU32 tLOD, FxU32 textureMode)
{
  FxBool bigAssTexture, wideTexture, compressedTexture;
  FxU32 bitsPerTexel, logAspectRatio;

  if(tLOD & SST_TBIG)
    bigAssTexture = FXTRUE;
  else
    bigAssTexture = FXFALSE;

  if(tLOD & SST_LOD_S_IS_WIDER)
    wideTexture = FXTRUE;
  else
    wideTexture = FXFALSE;
  
  if(textureMode & SST_COMPRESSED_TEXTURES)
    compressedTexture = FXTRUE;
  else
    compressedTexture = FXFALSE;


  if(compressedTexture)
    {
      if(SST_T4BIT_COMPRESSED(textureMode))
	bitsPerTexel = 4;
      else if(SST_T8BIT_COMPRESSED(textureMode))
	bitsPerTexel = 8;
      else
	assert(0);
    }
  else
    {
      if(SST_T8BIT(textureMode))
	bitsPerTexel = 8;
      else if(SST_T16BIT(textureMode))
	bitsPerTexel = 16;
      else if(SST_T32BIT(textureMode))
	bitsPerTexel = 32;
      else
	assert(0);
    }

  logAspectRatio = (tLOD & SST_LOD_ASPECT) >> SST_LOD_ASPECT_SHIFT;

  return(sstLinearMipMapSize(lod, logAspectRatio, bitsPerTexel,
			     bigAssTexture, wideTexture, compressedTexture));
}


//Given various information about a tiled texture, this function
//returns the appropriate tiledStruct. The tiledStruct contains
//information about the u,v offset of the mipmap and the total
//width and height of a mipmap and all its children mipmaps

FX_EXPORT tiledStruct FX_CSTYLE sstTiledMipMapOffset(FxU32 lod, FxU32 logAspectRatio, FxU32 bitsPerTexel, 
						     FxBool bigAssTexture, FxBool wideTexture, FxBool compressedTexture)
{
  tiledStruct result;

  //Check validity of inputs
  if(bigAssTexture)
    assert(lod <= 11);
  else
    assert(lod <= 8);

  assert(logAspectRatio < 4);
  assert(wideTexture == 0 || wideTexture == 1);

  //Figure out which table to use and return the value
  if(bigAssTexture)
    { //2048x2048 textures
      if(compressedTexture)
	{
	  if(bitsPerTexel == 4)
	    result = _sstMipMapOffset_BigCompressedFourBitsPerTexelTextures_Tiled[wideTexture][logAspectRatio][lod];
	  else if(bitsPerTexel == 8)
	    result = _sstMipMapOffset_BigCompressedEightBitsPerTexelTextures_Tiled[wideTexture][logAspectRatio][lod];
	  else
	    assert(0);
	}
      else //Non-compressed Textures
	result = _sstMipMapOffset_BigTextures_Tiled[wideTexture][logAspectRatio][lod];
    }
  else
    { //256x256 textures
      if(compressedTexture)
	{
	  if(bitsPerTexel == 4)
	    result = _sstMipMapOffset_NormalCompressedFourBitsPerTexelTextures_Tiled[wideTexture][logAspectRatio][lod];
	  else if(bitsPerTexel == 8)
	    result = _sstMipMapOffset_NormalCompressedEightBitsPerTexelTextures_Tiled[wideTexture][logAspectRatio][lod];
	  else
	    assert(0);
	}
      else //Non-compressed Textures
	result = _sstMipMapOffset_NormalTextures_Tiled[wideTexture][logAspectRatio][lod];
    }
  
  return(result);  
}

//This is a convenience function based on sstTiledMipMapOffset(). It's too bad
//that c sucks so much that you can't overload functions.
FX_EXPORT tiledStruct FX_CSTYLE sstTiledMipMapOffset2(FxU32 lod, FxU32 tLOD, FxU32 textureMode)
{
  FxBool bigAssTexture, wideTexture, compressedTexture;
  FxU32 logAspectRatio, bitsPerTexel;

  if(tLOD & SST_TBIG)
    bigAssTexture = FXTRUE;
  else
    bigAssTexture = FXFALSE;

  if(tLOD & SST_LOD_S_IS_WIDER)
    wideTexture = FXTRUE;
  else
    wideTexture = FXFALSE;
  
  if(textureMode & SST_COMPRESSED_TEXTURES)
    compressedTexture = FXTRUE;
  else
    compressedTexture = FXFALSE;

  logAspectRatio = (tLOD & SST_LOD_ASPECT) >> SST_LOD_ASPECT_SHIFT;

  if(compressedTexture)
    {
      if(SST_T4BIT_COMPRESSED(textureMode))
	bitsPerTexel = 4;
      else if(SST_T8BIT_COMPRESSED(textureMode))
	bitsPerTexel = 8;
      else
	assert(0);
    }
  else
    {
      if(SST_T8BIT(textureMode))
	bitsPerTexel = 8;
      else if(SST_T16BIT(textureMode))
	bitsPerTexel = 16;
      else if(SST_T32BIT(textureMode))
	bitsPerTexel = 32;
      else
	assert(0);
    }

  return(sstTiledMipMapOffset(lod, logAspectRatio, bitsPerTexel, bigAssTexture, wideTexture, compressedTexture));
}



//The structures in this file were generated with genStruc.pl
//**********************************************************************
//**********************************************************************
//
//                      Linear Mipmap Size tables
//
//**********************************************************************
//**********************************************************************
FxI32 _sstMipMapSize_NormalTextures[4][16] = 
{
  {  // 1:1 aspect ratio
    0x00010000,    // (      65536) LOD 0  :  256x256   =>  ( 256x256 ) 
    0x00004000,    // (      16384) LOD 1  :  128x128   =>  ( 128x128 ) 
    0x00001000,    // (       4096) LOD 2  :   64x64    =>  (  64x64  ) 
    0x00000400,    // (       1024) LOD 3  :   32x32    =>  (  32x32  ) 
    0x00000100,    // (        256) LOD 4  :   16x16    =>  (  16x16  ) 
    0x00000040,    // (         64) LOD 5  :    8x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 6  :    4x4     =>  (   4x4   ) 
    0x00000004,    // (          4) LOD 7  :    2x2     =>  (   2x2   ) 
    0x00000001,    // (          1) LOD 8  :    1x1     =>  (   1x1   ) 
  },
  {  // 2:1 aspect ratio
    0x00008000,    // (      32768) LOD 0  :  256x128   =>  ( 256x128 ) 
    0x00002000,    // (       8192) LOD 1  :  128x64    =>  ( 128x64  ) 
    0x00000800,    // (       2048) LOD 2  :   64x32    =>  (  64x32  ) 
    0x00000200,    // (        512) LOD 3  :   32x16    =>  (  32x16  ) 
    0x00000080,    // (        128) LOD 4  :   16x8     =>  (  16x8   ) 
    0x00000020,    // (         32) LOD 5  :    8x4     =>  (   8x4   ) 
    0x00000008,    // (          8) LOD 6  :    4x2     =>  (   4x2   ) 
    0x00000002,    // (          2) LOD 7  :    2x1     =>  (   2x1   ) 
    0x00000001,    // (          1) LOD 8  :    1x1     =>  (   1x1   ) 
  },
  {  // 4:1 aspect ratio
    0x00004000,    // (      16384) LOD 0  :  256x64    =>  ( 256x64  ) 
    0x00001000,    // (       4096) LOD 1  :  128x32    =>  ( 128x32  ) 
    0x00000400,    // (       1024) LOD 2  :   64x16    =>  (  64x16  ) 
    0x00000100,    // (        256) LOD 3  :   32x8     =>  (  32x8   ) 
    0x00000040,    // (         64) LOD 4  :   16x4     =>  (  16x4   ) 
    0x00000010,    // (         16) LOD 5  :    8x2     =>  (   8x2   ) 
    0x00000004,    // (          4) LOD 6  :    4x1     =>  (   4x1   ) 
    0x00000002,    // (          2) LOD 7  :    2x1     =>  (   2x1   ) 
    0x00000001,    // (          1) LOD 8  :    1x1     =>  (   1x1   ) 
  },
  {  // 8:1 aspect ratio
    0x00002000,    // (       8192) LOD 0  :  256x32    =>  ( 256x32  ) 
    0x00000800,    // (       2048) LOD 1  :  128x16    =>  ( 128x16  ) 
    0x00000200,    // (        512) LOD 2  :   64x8     =>  (  64x8   ) 
    0x00000080,    // (        128) LOD 3  :   32x4     =>  (  32x4   ) 
    0x00000020,    // (         32) LOD 4  :   16x2     =>  (  16x2   ) 
    0x00000008,    // (          8) LOD 5  :    8x1     =>  (   8x1   ) 
    0x00000004,    // (          4) LOD 6  :    4x1     =>  (   4x1   ) 
    0x00000002,    // (          2) LOD 7  :    2x1     =>  (   2x1   ) 
    0x00000001,    // (          1) LOD 8  :    1x1     =>  (   1x1   ) 
  },
};

FxI32 _sstMipMapSize_BigTextures[4][16] = 
{
  {  // 1:1 aspect ratio
    0x00400000,    // (    4194304) LOD 0  : 2048x2048  =>  (2048x2048) 
    0x00100000,    // (    1048576) LOD 1  : 1024x1024  =>  (1024x1024) 
    0x00040000,    // (     262144) LOD 2  :  512x512   =>  ( 512x512 ) 
    0x00010000,    // (      65536) LOD 3  :  256x256   =>  ( 256x256 ) 
    0x00004000,    // (      16384) LOD 4  :  128x128   =>  ( 128x128 ) 
    0x00001000,    // (       4096) LOD 5  :   64x64    =>  (  64x64  ) 
    0x00000400,    // (       1024) LOD 6  :   32x32    =>  (  32x32  ) 
    0x00000100,    // (        256) LOD 7  :   16x16    =>  (  16x16  ) 
    0x00000040,    // (         64) LOD 8  :    8x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 9  :    4x4     =>  (   4x4   ) 
    0x00000004,    // (          4) LOD 10 :    2x2     =>  (   2x2   ) 
    0x00000001,    // (          1) LOD 11 :    1x1     =>  (   1x1   ) 
  },
  {  // 2:1 aspect ratio
    0x00200000,    // (    2097152) LOD 0  : 2048x1024  =>  (2048x1024) 
    0x00080000,    // (     524288) LOD 1  : 1024x512   =>  (1024x512 ) 
    0x00020000,    // (     131072) LOD 2  :  512x256   =>  ( 512x256 ) 
    0x00008000,    // (      32768) LOD 3  :  256x128   =>  ( 256x128 ) 
    0x00002000,    // (       8192) LOD 4  :  128x64    =>  ( 128x64  ) 
    0x00000800,    // (       2048) LOD 5  :   64x32    =>  (  64x32  ) 
    0x00000200,    // (        512) LOD 6  :   32x16    =>  (  32x16  ) 
    0x00000080,    // (        128) LOD 7  :   16x8     =>  (  16x8   ) 
    0x00000020,    // (         32) LOD 8  :    8x4     =>  (   8x4   ) 
    0x00000008,    // (          8) LOD 9  :    4x2     =>  (   4x2   ) 
    0x00000002,    // (          2) LOD 10 :    2x1     =>  (   2x1   ) 
    0x00000001,    // (          1) LOD 11 :    1x1     =>  (   1x1   ) 
  },
  {  // 4:1 aspect ratio
    0x00100000,    // (    1048576) LOD 0  : 2048x512   =>  (2048x512 ) 
    0x00040000,    // (     262144) LOD 1  : 1024x256   =>  (1024x256 ) 
    0x00010000,    // (      65536) LOD 2  :  512x128   =>  ( 512x128 ) 
    0x00004000,    // (      16384) LOD 3  :  256x64    =>  ( 256x64  ) 
    0x00001000,    // (       4096) LOD 4  :  128x32    =>  ( 128x32  ) 
    0x00000400,    // (       1024) LOD 5  :   64x16    =>  (  64x16  ) 
    0x00000100,    // (        256) LOD 6  :   32x8     =>  (  32x8   ) 
    0x00000040,    // (         64) LOD 7  :   16x4     =>  (  16x4   ) 
    0x00000010,    // (         16) LOD 8  :    8x2     =>  (   8x2   ) 
    0x00000004,    // (          4) LOD 9  :    4x1     =>  (   4x1   ) 
    0x00000002,    // (          2) LOD 10 :    2x1     =>  (   2x1   ) 
    0x00000001,    // (          1) LOD 11 :    1x1     =>  (   1x1   ) 
  },
  {  // 8:1 aspect ratio
    0x00080000,    // (     524288) LOD 0  : 2048x256   =>  (2048x256 ) 
    0x00020000,    // (     131072) LOD 1  : 1024x128   =>  (1024x128 ) 
    0x00008000,    // (      32768) LOD 2  :  512x64    =>  ( 512x64  ) 
    0x00002000,    // (       8192) LOD 3  :  256x32    =>  ( 256x32  ) 
    0x00000800,    // (       2048) LOD 4  :  128x16    =>  ( 128x16  ) 
    0x00000200,    // (        512) LOD 5  :   64x8     =>  (  64x8   ) 
    0x00000080,    // (        128) LOD 6  :   32x4     =>  (  32x4   ) 
    0x00000020,    // (         32) LOD 7  :   16x2     =>  (  16x2   ) 
    0x00000008,    // (          8) LOD 8  :    8x1     =>  (   8x1   ) 
    0x00000004,    // (          4) LOD 9  :    4x1     =>  (   4x1   ) 
    0x00000002,    // (          2) LOD 10 :    2x1     =>  (   2x1   ) 
    0x00000001,    // (          1) LOD 11 :    1x1     =>  (   1x1   ) 
  },
};

FxI32 _sstMipMapSize_NormalCompressedFourBitsPerTexelTextures[2][4][16] = 
{
 {  // tallTexture
  {  // 1:1 aspect ratio
    0x00008000,    // (      32768) LOD 0  :  256x256   =>  ( 256x256 ) 
    0x00002000,    // (       8192) LOD 1  :  128x128   =>  ( 128x128 ) 
    0x00000800,    // (       2048) LOD 2  :   64x64    =>  (  64x64  ) 
    0x00000200,    // (        512) LOD 3  :   32x32    =>  (  32x32  ) 
    0x00000080,    // (        128) LOD 4  :   16x16    =>  (  16x16  ) 
    0x00000020,    // (         32) LOD 5  :    8x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 6  :    4x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 7  :    2x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   8x4   ) 
  },
  {  // 1:2 aspect ratio
    0x00004000,    // (      16384) LOD 0  :  128x256   =>  ( 128x256 ) 
    0x00001000,    // (       4096) LOD 1  :   64x128   =>  (  64x128 ) 
    0x00000400,    // (       1024) LOD 2  :   32x64    =>  (  32x64  ) 
    0x00000100,    // (        256) LOD 3  :   16x32    =>  (  16x32  ) 
    0x00000040,    // (         64) LOD 4  :    8x16    =>  (   8x16  ) 
    0x00000020,    // (         32) LOD 5  :    4x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 6  :    2x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 7  :    1x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   8x4   ) 
  },
  {  // 1:4 aspect ratio
    0x00002000,    // (       8192) LOD 0  :   64x256   =>  (  64x256 ) 
    0x00000800,    // (       2048) LOD 1  :   32x128   =>  (  32x128 ) 
    0x00000200,    // (        512) LOD 2  :   16x64    =>  (  16x64  ) 
    0x00000080,    // (        128) LOD 3  :    8x32    =>  (   8x32  ) 
    0x00000040,    // (         64) LOD 4  :    4x16    =>  (   8x16  ) 
    0x00000020,    // (         32) LOD 5  :    2x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 6  :    1x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 7  :    1x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   8x4   ) 
  },
  {  // 1:8 aspect ratio
    0x00001000,    // (       4096) LOD 0  :   32x256   =>  (  32x256 ) 
    0x00000400,    // (       1024) LOD 1  :   16x128   =>  (  16x128 ) 
    0x00000100,    // (        256) LOD 2  :    8x64    =>  (   8x64  ) 
    0x00000080,    // (        128) LOD 3  :    4x32    =>  (   8x32  ) 
    0x00000040,    // (         64) LOD 4  :    2x16    =>  (   8x16  ) 
    0x00000020,    // (         32) LOD 5  :    1x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 6  :    1x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 7  :    1x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   8x4   ) 
  },
 },
 {  // wideTexture
  {  // 1:1 aspect ratio
    0x00008000,    // (      32768) LOD 0  :  256x256   =>  ( 256x256 ) 
    0x00002000,    // (       8192) LOD 1  :  128x128   =>  ( 128x128 ) 
    0x00000800,    // (       2048) LOD 2  :   64x64    =>  (  64x64  ) 
    0x00000200,    // (        512) LOD 3  :   32x32    =>  (  32x32  ) 
    0x00000080,    // (        128) LOD 4  :   16x16    =>  (  16x16  ) 
    0x00000020,    // (         32) LOD 5  :    8x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 6  :    4x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 7  :    2x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   8x4   ) 
  },
  {  // 2:1 aspect ratio
    0x00004000,    // (      16384) LOD 0  :  256x128   =>  ( 256x128 ) 
    0x00001000,    // (       4096) LOD 1  :  128x64    =>  ( 128x64  ) 
    0x00000400,    // (       1024) LOD 2  :   64x32    =>  (  64x32  ) 
    0x00000100,    // (        256) LOD 3  :   32x16    =>  (  32x16  ) 
    0x00000040,    // (         64) LOD 4  :   16x8     =>  (  16x8   ) 
    0x00000010,    // (         16) LOD 5  :    8x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 6  :    4x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 7  :    2x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   8x4   ) 
  },
  {  // 4:1 aspect ratio
    0x00002000,    // (       8192) LOD 0  :  256x64    =>  ( 256x64  ) 
    0x00000800,    // (       2048) LOD 1  :  128x32    =>  ( 128x32  ) 
    0x00000200,    // (        512) LOD 2  :   64x16    =>  (  64x16  ) 
    0x00000080,    // (        128) LOD 3  :   32x8     =>  (  32x8   ) 
    0x00000020,    // (         32) LOD 4  :   16x4     =>  (  16x4   ) 
    0x00000010,    // (         16) LOD 5  :    8x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 6  :    4x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 7  :    2x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   8x4   ) 
  },
  {  // 8:1 aspect ratio
    0x00001000,    // (       4096) LOD 0  :  256x32    =>  ( 256x32  ) 
    0x00000400,    // (       1024) LOD 1  :  128x16    =>  ( 128x16  ) 
    0x00000100,    // (        256) LOD 2  :   64x8     =>  (  64x8   ) 
    0x00000040,    // (         64) LOD 3  :   32x4     =>  (  32x4   ) 
    0x00000020,    // (         32) LOD 4  :   16x2     =>  (  16x4   ) 
    0x00000010,    // (         16) LOD 5  :    8x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 6  :    4x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 7  :    2x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   8x4   ) 
  },
 }
};

FxI32 _sstMipMapSize_BigCompressedFourBitsPerTexelTextures[2][4][16] = 
{
 {  // tallTexture
  {  // 1:1 aspect ratio
    0x00200000,    // (    2097152) LOD 0  : 2048x2048  =>  (2048x2048) 
    0x00080000,    // (     524288) LOD 1  : 1024x1024  =>  (1024x1024) 
    0x00020000,    // (     131072) LOD 2  :  512x512   =>  ( 512x512 ) 
    0x00008000,    // (      32768) LOD 3  :  256x256   =>  ( 256x256 ) 
    0x00002000,    // (       8192) LOD 4  :  128x128   =>  ( 128x128 ) 
    0x00000800,    // (       2048) LOD 5  :   64x64    =>  (  64x64  ) 
    0x00000200,    // (        512) LOD 6  :   32x32    =>  (  32x32  ) 
    0x00000080,    // (        128) LOD 7  :   16x16    =>  (  16x16  ) 
    0x00000020,    // (         32) LOD 8  :    8x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 9  :    4x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 10 :    2x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   8x4   ) 
  },
  {  // 1:2 aspect ratio
    0x00100000,    // (    1048576) LOD 0  : 1024x2048  =>  (1024x2048) 
    0x00040000,    // (     262144) LOD 1  :  512x1024  =>  ( 512x1024) 
    0x00010000,    // (      65536) LOD 2  :  256x512   =>  ( 256x512 ) 
    0x00004000,    // (      16384) LOD 3  :  128x256   =>  ( 128x256 ) 
    0x00001000,    // (       4096) LOD 4  :   64x128   =>  (  64x128 ) 
    0x00000400,    // (       1024) LOD 5  :   32x64    =>  (  32x64  ) 
    0x00000100,    // (        256) LOD 6  :   16x32    =>  (  16x32  ) 
    0x00000040,    // (         64) LOD 7  :    8x16    =>  (   8x16  ) 
    0x00000020,    // (         32) LOD 8  :    4x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 9  :    2x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 10 :    1x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   8x4   ) 
  },
  {  // 1:4 aspect ratio
    0x00080000,    // (     524288) LOD 0  :  512x2048  =>  ( 512x2048) 
    0x00020000,    // (     131072) LOD 1  :  256x1024  =>  ( 256x1024) 
    0x00008000,    // (      32768) LOD 2  :  128x512   =>  ( 128x512 ) 
    0x00002000,    // (       8192) LOD 3  :   64x256   =>  (  64x256 ) 
    0x00000800,    // (       2048) LOD 4  :   32x128   =>  (  32x128 ) 
    0x00000200,    // (        512) LOD 5  :   16x64    =>  (  16x64  ) 
    0x00000080,    // (        128) LOD 6  :    8x32    =>  (   8x32  ) 
    0x00000040,    // (         64) LOD 7  :    4x16    =>  (   8x16  ) 
    0x00000020,    // (         32) LOD 8  :    2x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 9  :    1x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 10 :    1x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   8x4   ) 
  },
  {  // 1:8 aspect ratio
    0x00040000,    // (     262144) LOD 0  :  256x2048  =>  ( 256x2048) 
    0x00010000,    // (      65536) LOD 1  :  128x1024  =>  ( 128x1024) 
    0x00004000,    // (      16384) LOD 2  :   64x512   =>  (  64x512 ) 
    0x00001000,    // (       4096) LOD 3  :   32x256   =>  (  32x256 ) 
    0x00000400,    // (       1024) LOD 4  :   16x128   =>  (  16x128 ) 
    0x00000100,    // (        256) LOD 5  :    8x64    =>  (   8x64  ) 
    0x00000080,    // (        128) LOD 6  :    4x32    =>  (   8x32  ) 
    0x00000040,    // (         64) LOD 7  :    2x16    =>  (   8x16  ) 
    0x00000020,    // (         32) LOD 8  :    1x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 9  :    1x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 10 :    1x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   8x4   ) 
  },
 },
 {  // wideTexture
  {  // 1:1 aspect ratio
    0x00200000,    // (    2097152) LOD 0  : 2048x2048  =>  (2048x2048) 
    0x00080000,    // (     524288) LOD 1  : 1024x1024  =>  (1024x1024) 
    0x00020000,    // (     131072) LOD 2  :  512x512   =>  ( 512x512 ) 
    0x00008000,    // (      32768) LOD 3  :  256x256   =>  ( 256x256 ) 
    0x00002000,    // (       8192) LOD 4  :  128x128   =>  ( 128x128 ) 
    0x00000800,    // (       2048) LOD 5  :   64x64    =>  (  64x64  ) 
    0x00000200,    // (        512) LOD 6  :   32x32    =>  (  32x32  ) 
    0x00000080,    // (        128) LOD 7  :   16x16    =>  (  16x16  ) 
    0x00000020,    // (         32) LOD 8  :    8x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 9  :    4x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 10 :    2x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   8x4   ) 
  },
  {  // 2:1 aspect ratio
    0x00100000,    // (    1048576) LOD 0  : 2048x1024  =>  (2048x1024) 
    0x00040000,    // (     262144) LOD 1  : 1024x512   =>  (1024x512 ) 
    0x00010000,    // (      65536) LOD 2  :  512x256   =>  ( 512x256 ) 
    0x00004000,    // (      16384) LOD 3  :  256x128   =>  ( 256x128 ) 
    0x00001000,    // (       4096) LOD 4  :  128x64    =>  ( 128x64  ) 
    0x00000400,    // (       1024) LOD 5  :   64x32    =>  (  64x32  ) 
    0x00000100,    // (        256) LOD 6  :   32x16    =>  (  32x16  ) 
    0x00000040,    // (         64) LOD 7  :   16x8     =>  (  16x8   ) 
    0x00000010,    // (         16) LOD 8  :    8x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 9  :    4x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 10 :    2x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   8x4   ) 
  },
  {  // 4:1 aspect ratio
    0x00080000,    // (     524288) LOD 0  : 2048x512   =>  (2048x512 ) 
    0x00020000,    // (     131072) LOD 1  : 1024x256   =>  (1024x256 ) 
    0x00008000,    // (      32768) LOD 2  :  512x128   =>  ( 512x128 ) 
    0x00002000,    // (       8192) LOD 3  :  256x64    =>  ( 256x64  ) 
    0x00000800,    // (       2048) LOD 4  :  128x32    =>  ( 128x32  ) 
    0x00000200,    // (        512) LOD 5  :   64x16    =>  (  64x16  ) 
    0x00000080,    // (        128) LOD 6  :   32x8     =>  (  32x8   ) 
    0x00000020,    // (         32) LOD 7  :   16x4     =>  (  16x4   ) 
    0x00000010,    // (         16) LOD 8  :    8x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 9  :    4x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 10 :    2x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   8x4   ) 
  },
  {  // 8:1 aspect ratio
    0x00040000,    // (     262144) LOD 0  : 2048x256   =>  (2048x256 ) 
    0x00010000,    // (      65536) LOD 1  : 1024x128   =>  (1024x128 ) 
    0x00004000,    // (      16384) LOD 2  :  512x64    =>  ( 512x64  ) 
    0x00001000,    // (       4096) LOD 3  :  256x32    =>  ( 256x32  ) 
    0x00000400,    // (       1024) LOD 4  :  128x16    =>  ( 128x16  ) 
    0x00000100,    // (        256) LOD 5  :   64x8     =>  (  64x8   ) 
    0x00000040,    // (         64) LOD 6  :   32x4     =>  (  32x4   ) 
    0x00000020,    // (         32) LOD 7  :   16x2     =>  (  16x4   ) 
    0x00000010,    // (         16) LOD 8  :    8x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 9  :    4x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 10 :    2x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   8x4   ) 
  },
 }
};

FxI32 _sstMipMapSize_NormalCompressedEightBitsPerTexelTextures[2][4][16] = 
{
 {  // tallTexture
  {  // 1:1 aspect ratio
    0x00010000,    // (      65536) LOD 0  :  256x256   =>  ( 256x256 ) 
    0x00004000,    // (      16384) LOD 1  :  128x128   =>  ( 128x128 ) 
    0x00001000,    // (       4096) LOD 2  :   64x64    =>  (  64x64  ) 
    0x00000400,    // (       1024) LOD 3  :   32x32    =>  (  32x32  ) 
    0x00000100,    // (        256) LOD 4  :   16x16    =>  (  16x16  ) 
    0x00000040,    // (         64) LOD 5  :    8x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 6  :    4x4     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 7  :    2x2     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   4x4   ) 
  },
  {  // 1:2 aspect ratio
    0x00008000,    // (      32768) LOD 0  :  128x256   =>  ( 128x256 ) 
    0x00002000,    // (       8192) LOD 1  :   64x128   =>  (  64x128 ) 
    0x00000800,    // (       2048) LOD 2  :   32x64    =>  (  32x64  ) 
    0x00000200,    // (        512) LOD 3  :   16x32    =>  (  16x32  ) 
    0x00000080,    // (        128) LOD 4  :    8x16    =>  (   8x16  ) 
    0x00000020,    // (         32) LOD 5  :    4x8     =>  (   4x8   ) 
    0x00000010,    // (         16) LOD 6  :    2x4     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 7  :    1x2     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   4x4   ) 
  },
  {  // 1:4 aspect ratio
    0x00004000,    // (      16384) LOD 0  :   64x256   =>  (  64x256 ) 
    0x00001000,    // (       4096) LOD 1  :   32x128   =>  (  32x128 ) 
    0x00000400,    // (       1024) LOD 2  :   16x64    =>  (  16x64  ) 
    0x00000100,    // (        256) LOD 3  :    8x32    =>  (   8x32  ) 
    0x00000040,    // (         64) LOD 4  :    4x16    =>  (   4x16  ) 
    0x00000020,    // (         32) LOD 5  :    2x8     =>  (   4x8   ) 
    0x00000010,    // (         16) LOD 6  :    1x4     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 7  :    1x2     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   4x4   ) 
  },
  {  // 1:8 aspect ratio
    0x00002000,    // (       8192) LOD 0  :   32x256   =>  (  32x256 ) 
    0x00000800,    // (       2048) LOD 1  :   16x128   =>  (  16x128 ) 
    0x00000200,    // (        512) LOD 2  :    8x64    =>  (   8x64  ) 
    0x00000080,    // (        128) LOD 3  :    4x32    =>  (   4x32  ) 
    0x00000040,    // (         64) LOD 4  :    2x16    =>  (   4x16  ) 
    0x00000020,    // (         32) LOD 5  :    1x8     =>  (   4x8   ) 
    0x00000010,    // (         16) LOD 6  :    1x4     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 7  :    1x2     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   4x4   ) 
  },
 },
 {  // wideTexture
  {  // 1:1 aspect ratio
    0x00010000,    // (      65536) LOD 0  :  256x256   =>  ( 256x256 ) 
    0x00004000,    // (      16384) LOD 1  :  128x128   =>  ( 128x128 ) 
    0x00001000,    // (       4096) LOD 2  :   64x64    =>  (  64x64  ) 
    0x00000400,    // (       1024) LOD 3  :   32x32    =>  (  32x32  ) 
    0x00000100,    // (        256) LOD 4  :   16x16    =>  (  16x16  ) 
    0x00000040,    // (         64) LOD 5  :    8x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 6  :    4x4     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 7  :    2x2     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   4x4   ) 
  },
  {  // 2:1 aspect ratio
    0x00008000,    // (      32768) LOD 0  :  256x128   =>  ( 256x128 ) 
    0x00002000,    // (       8192) LOD 1  :  128x64    =>  ( 128x64  ) 
    0x00000800,    // (       2048) LOD 2  :   64x32    =>  (  64x32  ) 
    0x00000200,    // (        512) LOD 3  :   32x16    =>  (  32x16  ) 
    0x00000080,    // (        128) LOD 4  :   16x8     =>  (  16x8   ) 
    0x00000020,    // (         32) LOD 5  :    8x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 6  :    4x2     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 7  :    2x1     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   4x4   ) 
  },
  {  // 4:1 aspect ratio
    0x00004000,    // (      16384) LOD 0  :  256x64    =>  ( 256x64  ) 
    0x00001000,    // (       4096) LOD 1  :  128x32    =>  ( 128x32  ) 
    0x00000400,    // (       1024) LOD 2  :   64x16    =>  (  64x16  ) 
    0x00000100,    // (        256) LOD 3  :   32x8     =>  (  32x8   ) 
    0x00000040,    // (         64) LOD 4  :   16x4     =>  (  16x4   ) 
    0x00000020,    // (         32) LOD 5  :    8x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 6  :    4x1     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 7  :    2x1     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   4x4   ) 
  },
  {  // 8:1 aspect ratio
    0x00002000,    // (       8192) LOD 0  :  256x32    =>  ( 256x32  ) 
    0x00000800,    // (       2048) LOD 1  :  128x16    =>  ( 128x16  ) 
    0x00000200,    // (        512) LOD 2  :   64x8     =>  (  64x8   ) 
    0x00000080,    // (        128) LOD 3  :   32x4     =>  (  32x4   ) 
    0x00000040,    // (         64) LOD 4  :   16x2     =>  (  16x4   ) 
    0x00000020,    // (         32) LOD 5  :    8x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 6  :    4x1     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 7  :    2x1     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 8  :    1x1     =>  (   4x4   ) 
  },
 }
};

FxI32 _sstMipMapSize_BigCompressedEightBitsPerTexelTextures[2][4][16] = 
{
 {  // tallTexture
  {  // 1:1 aspect ratio
    0x00400000,    // (    4194304) LOD 0  : 2048x2048  =>  (2048x2048) 
    0x00100000,    // (    1048576) LOD 1  : 1024x1024  =>  (1024x1024) 
    0x00040000,    // (     262144) LOD 2  :  512x512   =>  ( 512x512 ) 
    0x00010000,    // (      65536) LOD 3  :  256x256   =>  ( 256x256 ) 
    0x00004000,    // (      16384) LOD 4  :  128x128   =>  ( 128x128 ) 
    0x00001000,    // (       4096) LOD 5  :   64x64    =>  (  64x64  ) 
    0x00000400,    // (       1024) LOD 6  :   32x32    =>  (  32x32  ) 
    0x00000100,    // (        256) LOD 7  :   16x16    =>  (  16x16  ) 
    0x00000040,    // (         64) LOD 8  :    8x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 9  :    4x4     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 10 :    2x2     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   4x4   ) 
  },
  {  // 1:2 aspect ratio
    0x00200000,    // (    2097152) LOD 0  : 1024x2048  =>  (1024x2048) 
    0x00080000,    // (     524288) LOD 1  :  512x1024  =>  ( 512x1024) 
    0x00020000,    // (     131072) LOD 2  :  256x512   =>  ( 256x512 ) 
    0x00008000,    // (      32768) LOD 3  :  128x256   =>  ( 128x256 ) 
    0x00002000,    // (       8192) LOD 4  :   64x128   =>  (  64x128 ) 
    0x00000800,    // (       2048) LOD 5  :   32x64    =>  (  32x64  ) 
    0x00000200,    // (        512) LOD 6  :   16x32    =>  (  16x32  ) 
    0x00000080,    // (        128) LOD 7  :    8x16    =>  (   8x16  ) 
    0x00000020,    // (         32) LOD 8  :    4x8     =>  (   4x8   ) 
    0x00000010,    // (         16) LOD 9  :    2x4     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 10 :    1x2     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   4x4   ) 
  },
  {  // 1:4 aspect ratio
    0x00100000,    // (    1048576) LOD 0  :  512x2048  =>  ( 512x2048) 
    0x00040000,    // (     262144) LOD 1  :  256x1024  =>  ( 256x1024) 
    0x00010000,    // (      65536) LOD 2  :  128x512   =>  ( 128x512 ) 
    0x00004000,    // (      16384) LOD 3  :   64x256   =>  (  64x256 ) 
    0x00001000,    // (       4096) LOD 4  :   32x128   =>  (  32x128 ) 
    0x00000400,    // (       1024) LOD 5  :   16x64    =>  (  16x64  ) 
    0x00000100,    // (        256) LOD 6  :    8x32    =>  (   8x32  ) 
    0x00000040,    // (         64) LOD 7  :    4x16    =>  (   4x16  ) 
    0x00000020,    // (         32) LOD 8  :    2x8     =>  (   4x8   ) 
    0x00000010,    // (         16) LOD 9  :    1x4     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 10 :    1x2     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   4x4   ) 
  },
  {  // 1:8 aspect ratio
    0x00080000,    // (     524288) LOD 0  :  256x2048  =>  ( 256x2048) 
    0x00020000,    // (     131072) LOD 1  :  128x1024  =>  ( 128x1024) 
    0x00008000,    // (      32768) LOD 2  :   64x512   =>  (  64x512 ) 
    0x00002000,    // (       8192) LOD 3  :   32x256   =>  (  32x256 ) 
    0x00000800,    // (       2048) LOD 4  :   16x128   =>  (  16x128 ) 
    0x00000200,    // (        512) LOD 5  :    8x64    =>  (   8x64  ) 
    0x00000080,    // (        128) LOD 6  :    4x32    =>  (   4x32  ) 
    0x00000040,    // (         64) LOD 7  :    2x16    =>  (   4x16  ) 
    0x00000020,    // (         32) LOD 8  :    1x8     =>  (   4x8   ) 
    0x00000010,    // (         16) LOD 9  :    1x4     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 10 :    1x2     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   4x4   ) 
  },
 },
 {  // wideTexture
  {  // 1:1 aspect ratio
    0x00400000,    // (    4194304) LOD 0  : 2048x2048  =>  (2048x2048) 
    0x00100000,    // (    1048576) LOD 1  : 1024x1024  =>  (1024x1024) 
    0x00040000,    // (     262144) LOD 2  :  512x512   =>  ( 512x512 ) 
    0x00010000,    // (      65536) LOD 3  :  256x256   =>  ( 256x256 ) 
    0x00004000,    // (      16384) LOD 4  :  128x128   =>  ( 128x128 ) 
    0x00001000,    // (       4096) LOD 5  :   64x64    =>  (  64x64  ) 
    0x00000400,    // (       1024) LOD 6  :   32x32    =>  (  32x32  ) 
    0x00000100,    // (        256) LOD 7  :   16x16    =>  (  16x16  ) 
    0x00000040,    // (         64) LOD 8  :    8x8     =>  (   8x8   ) 
    0x00000010,    // (         16) LOD 9  :    4x4     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 10 :    2x2     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   4x4   ) 
  },
  {  // 2:1 aspect ratio
    0x00200000,    // (    2097152) LOD 0  : 2048x1024  =>  (2048x1024) 
    0x00080000,    // (     524288) LOD 1  : 1024x512   =>  (1024x512 ) 
    0x00020000,    // (     131072) LOD 2  :  512x256   =>  ( 512x256 ) 
    0x00008000,    // (      32768) LOD 3  :  256x128   =>  ( 256x128 ) 
    0x00002000,    // (       8192) LOD 4  :  128x64    =>  ( 128x64  ) 
    0x00000800,    // (       2048) LOD 5  :   64x32    =>  (  64x32  ) 
    0x00000200,    // (        512) LOD 6  :   32x16    =>  (  32x16  ) 
    0x00000080,    // (        128) LOD 7  :   16x8     =>  (  16x8   ) 
    0x00000020,    // (         32) LOD 8  :    8x4     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 9  :    4x2     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 10 :    2x1     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   4x4   ) 
  },
  {  // 4:1 aspect ratio
    0x00100000,    // (    1048576) LOD 0  : 2048x512   =>  (2048x512 ) 
    0x00040000,    // (     262144) LOD 1  : 1024x256   =>  (1024x256 ) 
    0x00010000,    // (      65536) LOD 2  :  512x128   =>  ( 512x128 ) 
    0x00004000,    // (      16384) LOD 3  :  256x64    =>  ( 256x64  ) 
    0x00001000,    // (       4096) LOD 4  :  128x32    =>  ( 128x32  ) 
    0x00000400,    // (       1024) LOD 5  :   64x16    =>  (  64x16  ) 
    0x00000100,    // (        256) LOD 6  :   32x8     =>  (  32x8   ) 
    0x00000040,    // (         64) LOD 7  :   16x4     =>  (  16x4   ) 
    0x00000020,    // (         32) LOD 8  :    8x2     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 9  :    4x1     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 10 :    2x1     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   4x4   ) 
  },
  {  // 8:1 aspect ratio
    0x00080000,    // (     524288) LOD 0  : 2048x256   =>  (2048x256 ) 
    0x00020000,    // (     131072) LOD 1  : 1024x128   =>  (1024x128 ) 
    0x00008000,    // (      32768) LOD 2  :  512x64    =>  ( 512x64  ) 
    0x00002000,    // (       8192) LOD 3  :  256x32    =>  ( 256x32  ) 
    0x00000800,    // (       2048) LOD 4  :  128x16    =>  ( 128x16  ) 
    0x00000200,    // (        512) LOD 5  :   64x8     =>  (  64x8   ) 
    0x00000080,    // (        128) LOD 6  :   32x4     =>  (  32x4   ) 
    0x00000040,    // (         64) LOD 7  :   16x2     =>  (  16x4   ) 
    0x00000020,    // (         32) LOD 8  :    8x1     =>  (   8x4   ) 
    0x00000010,    // (         16) LOD 9  :    4x1     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 10 :    2x1     =>  (   4x4   ) 
    0x00000010,    // (         16) LOD 11 :    1x1     =>  (   4x4   ) 
  },
 }
};


//**********************************************************************
//**********************************************************************
//
//                     Linear Mipmap offset tables
//
//**********************************************************************
//**********************************************************************
FxI32 _sstMipMapOffset_NormalTextures[4][16] = 
{
  {  // 1:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x256   =>  ( 256x256 )
    0x00010000,    // (      65536) LOD 1  :  128x128   =>  ( 128x128 )
    0x00014000,    // (      81920) LOD 2  :   64x64    =>  (  64x64  )
    0x00015000,    // (      86016) LOD 3  :   32x32    =>  (  32x32  )
    0x00015400,    // (      87040) LOD 4  :   16x16    =>  (  16x16  )
    0x00015500,    // (      87296) LOD 5  :    8x8     =>  (   8x8   )
    0x00015540,    // (      87360) LOD 6  :    4x4     =>  (   4x4   )
    0x00015550,    // (      87376) LOD 7  :    2x2     =>  (   2x2   )
    0x00015554,    // (      87380) LOD 8  :    1x1     =>  (   1x1   )
  },
  {  // 2:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x128   =>  ( 256x128 )
    0x00008000,    // (      32768) LOD 1  :  128x64    =>  ( 128x64  )
    0x0000a000,    // (      40960) LOD 2  :   64x32    =>  (  64x32  )
    0x0000a800,    // (      43008) LOD 3  :   32x16    =>  (  32x16  )
    0x0000aa00,    // (      43520) LOD 4  :   16x8     =>  (  16x8   )
    0x0000aa80,    // (      43648) LOD 5  :    8x4     =>  (   8x4   )
    0x0000aaa0,    // (      43680) LOD 6  :    4x2     =>  (   4x2   )
    0x0000aaa8,    // (      43688) LOD 7  :    2x1     =>  (   2x1   )
    0x0000aaaa,    // (      43690) LOD 8  :    1x1     =>  (   1x1   )
  },
  {  // 4:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x64    =>  ( 256x64  )
    0x00004000,    // (      16384) LOD 1  :  128x32    =>  ( 128x32  )
    0x00005000,    // (      20480) LOD 2  :   64x16    =>  (  64x16  )
    0x00005400,    // (      21504) LOD 3  :   32x8     =>  (  32x8   )
    0x00005500,    // (      21760) LOD 4  :   16x4     =>  (  16x4   )
    0x00005540,    // (      21824) LOD 5  :    8x2     =>  (   8x2   )
    0x00005550,    // (      21840) LOD 6  :    4x1     =>  (   4x1   )
    0x00005554,    // (      21844) LOD 7  :    2x1     =>  (   2x1   )
    0x00005556,    // (      21846) LOD 8  :    1x1     =>  (   1x1   )
  },
  {  // 8:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x32    =>  ( 256x32  )
    0x00002000,    // (       8192) LOD 1  :  128x16    =>  ( 128x16  )
    0x00002800,    // (      10240) LOD 2  :   64x8     =>  (  64x8   )
    0x00002a00,    // (      10752) LOD 3  :   32x4     =>  (  32x4   )
    0x00002a80,    // (      10880) LOD 4  :   16x2     =>  (  16x2   )
    0x00002aa0,    // (      10912) LOD 5  :    8x1     =>  (   8x1   )
    0x00002aa8,    // (      10920) LOD 6  :    4x1     =>  (   4x1   )
    0x00002aac,    // (      10924) LOD 7  :    2x1     =>  (   2x1   )
    0x00002aae,    // (      10926) LOD 8  :    1x1     =>  (   1x1   )
  },
};

FxI32 _sstMipMapOffset_NormalTextures_Tsplit[4][16] = 
{
  {  // 1:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x256   =>  ( 256x256 )
    0x00000000,    // (          0) LOD 1  :  128x128   =>  ( 128x128 )
    0x00010000,    // (      65536) LOD 2  :   64x64    =>  (  64x64  )
    0x00004000,    // (      16384) LOD 3  :   32x32    =>  (  32x32  )
    0x00011000,    // (      69632) LOD 4  :   16x16    =>  (  16x16  )
    0x00004400,    // (      17408) LOD 5  :    8x8     =>  (   8x8   )
    0x00011100,    // (      69888) LOD 6  :    4x4     =>  (   4x4   )
    0x00004440,    // (      17472) LOD 7  :    2x2     =>  (   2x2   )
    0x00011110,    // (      69904) LOD 8  :    1x1     =>  (   1x1   )
  },
  {  // 2:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x128   =>  ( 256x128 )
    0x00000000,    // (          0) LOD 1  :  128x64    =>  ( 128x64  )
    0x00008000,    // (      32768) LOD 2  :   64x32    =>  (  64x32  )
    0x00002000,    // (       8192) LOD 3  :   32x16    =>  (  32x16  )
    0x00008800,    // (      34816) LOD 4  :   16x8     =>  (  16x8   )
    0x00002200,    // (       8704) LOD 5  :    8x4     =>  (   8x4   )
    0x00008880,    // (      34944) LOD 6  :    4x2     =>  (   4x2   )
    0x00002220,    // (       8736) LOD 7  :    2x1     =>  (   2x1   )
    0x00008888,    // (      34952) LOD 8  :    1x1     =>  (   1x1   )
  },
  {  // 4:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x64    =>  ( 256x64  )
    0x00000000,    // (          0) LOD 1  :  128x32    =>  ( 128x32  )
    0x00004000,    // (      16384) LOD 2  :   64x16    =>  (  64x16  )
    0x00001000,    // (       4096) LOD 3  :   32x8     =>  (  32x8   )
    0x00004400,    // (      17408) LOD 4  :   16x4     =>  (  16x4   )
    0x00001100,    // (       4352) LOD 5  :    8x2     =>  (   8x2   )
    0x00004440,    // (      17472) LOD 6  :    4x1     =>  (   4x1   )
    0x00001110,    // (       4368) LOD 7  :    2x1     =>  (   2x1   )
    0x00004444,    // (      17476) LOD 8  :    1x1     =>  (   1x1   )
  },
  {  // 8:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x32    =>  ( 256x32  )
    0x00000000,    // (          0) LOD 1  :  128x16    =>  ( 128x16  )
    0x00002000,    // (       8192) LOD 2  :   64x8     =>  (  64x8   )
    0x00000800,    // (       2048) LOD 3  :   32x4     =>  (  32x4   )
    0x00002200,    // (       8704) LOD 4  :   16x2     =>  (  16x2   )
    0x00000880,    // (       2176) LOD 5  :    8x1     =>  (   8x1   )
    0x00002220,    // (       8736) LOD 6  :    4x1     =>  (   4x1   )
    0x00000888,    // (       2184) LOD 7  :    2x1     =>  (   2x1   )
    0x00002224,    // (       8740) LOD 8  :    1x1     =>  (   1x1   )
  },
};

FxI32 _sstMipMapOffset_BigTextures[4][16] = 
{
  {  // 1:1 aspect ratio
    0xffac0000,    // (   -5505024) LOD 0  : 2048x2048  =>  (2048x2048)
    0xffec0000,    // (   -1310720) LOD 1  : 1024x1024  =>  (1024x1024)
    0xfffc0000,    // (    -262144) LOD 2  :  512x512   =>  ( 512x512 )
    0x00000000,    // (          0) LOD 3  :  256x256   =>  ( 256x256 )
    0x00010000,    // (      65536) LOD 4  :  128x128   =>  ( 128x128 )
    0x00014000,    // (      81920) LOD 5  :   64x64    =>  (  64x64  )
    0x00015000,    // (      86016) LOD 6  :   32x32    =>  (  32x32  )
    0x00015400,    // (      87040) LOD 7  :   16x16    =>  (  16x16  )
    0x00015500,    // (      87296) LOD 8  :    8x8     =>  (   8x8   )
    0x00015540,    // (      87360) LOD 9  :    4x4     =>  (   4x4   )
    0x00015550,    // (      87376) LOD 10 :    2x2     =>  (   2x2   )
    0x00015554,    // (      87380) LOD 11 :    1x1     =>  (   1x1   )
  },
  {  // 2:1 aspect ratio
    0xffd60000,    // (   -2752512) LOD 0  : 2048x1024  =>  (2048x1024)
    0xfff60000,    // (    -655360) LOD 1  : 1024x512   =>  (1024x512 )
    0xfffe0000,    // (    -131072) LOD 2  :  512x256   =>  ( 512x256 )
    0x00000000,    // (          0) LOD 3  :  256x128   =>  ( 256x128 )
    0x00008000,    // (      32768) LOD 4  :  128x64    =>  ( 128x64  )
    0x0000a000,    // (      40960) LOD 5  :   64x32    =>  (  64x32  )
    0x0000a800,    // (      43008) LOD 6  :   32x16    =>  (  32x16  )
    0x0000aa00,    // (      43520) LOD 7  :   16x8     =>  (  16x8   )
    0x0000aa80,    // (      43648) LOD 8  :    8x4     =>  (   8x4   )
    0x0000aaa0,    // (      43680) LOD 9  :    4x2     =>  (   4x2   )
    0x0000aaa8,    // (      43688) LOD 10 :    2x1     =>  (   2x1   )
    0x0000aaaa,    // (      43690) LOD 11 :    1x1     =>  (   1x1   )
  },
  {  // 4:1 aspect ratio
    0xffeb0000,    // (   -1376256) LOD 0  : 2048x512   =>  (2048x512 )
    0xfffb0000,    // (    -327680) LOD 1  : 1024x256   =>  (1024x256 )
    0xffff0000,    // (     -65536) LOD 2  :  512x128   =>  ( 512x128 )
    0x00000000,    // (          0) LOD 3  :  256x64    =>  ( 256x64  )
    0x00004000,    // (      16384) LOD 4  :  128x32    =>  ( 128x32  )
    0x00005000,    // (      20480) LOD 5  :   64x16    =>  (  64x16  )
    0x00005400,    // (      21504) LOD 6  :   32x8     =>  (  32x8   )
    0x00005500,    // (      21760) LOD 7  :   16x4     =>  (  16x4   )
    0x00005540,    // (      21824) LOD 8  :    8x2     =>  (   8x2   )
    0x00005550,    // (      21840) LOD 9  :    4x1     =>  (   4x1   )
    0x00005554,    // (      21844) LOD 10 :    2x1     =>  (   2x1   )
    0x00005556,    // (      21846) LOD 11 :    1x1     =>  (   1x1   )
  },
  {  // 8:1 aspect ratio
    0xfff58000,    // (    -688128) LOD 0  : 2048x256   =>  (2048x256 )
    0xfffd8000,    // (    -163840) LOD 1  : 1024x128   =>  (1024x128 )
    0xffff8000,    // (     -32768) LOD 2  :  512x64    =>  ( 512x64  )
    0x00000000,    // (          0) LOD 3  :  256x32    =>  ( 256x32  )
    0x00002000,    // (       8192) LOD 4  :  128x16    =>  ( 128x16  )
    0x00002800,    // (      10240) LOD 5  :   64x8     =>  (  64x8   )
    0x00002a00,    // (      10752) LOD 6  :   32x4     =>  (  32x4   )
    0x00002a80,    // (      10880) LOD 7  :   16x2     =>  (  16x2   )
    0x00002aa0,    // (      10912) LOD 8  :    8x1     =>  (   8x1   )
    0x00002aa8,    // (      10920) LOD 9  :    4x1     =>  (   4x1   )
    0x00002aac,    // (      10924) LOD 10 :    2x1     =>  (   2x1   )
    0x00002aae,    // (      10926) LOD 11 :    1x1     =>  (   1x1   )
  },
};

FxI32 _sstMipMapOffset_BigTextures_Tsplit[4][16] = 
{
  {  // 1:1 aspect ratio
    0xffbc0000,    // (   -4456448) LOD 0  : 2048x2048  =>  (2048x2048)
    0xfff00000,    // (   -1048576) LOD 1  : 1024x1024  =>  (1024x1024)
    0xfffc0000,    // (    -262144) LOD 2  :  512x512   =>  ( 512x512 )
    0x00000000,    // (          0) LOD 3  :  256x256   =>  ( 256x256 )
    0x00000000,    // (          0) LOD 4  :  128x128   =>  ( 128x128 )
    0x00010000,    // (      65536) LOD 5  :   64x64    =>  (  64x64  )
    0x00004000,    // (      16384) LOD 6  :   32x32    =>  (  32x32  )
    0x00011000,    // (      69632) LOD 7  :   16x16    =>  (  16x16  )
    0x00004400,    // (      17408) LOD 8  :    8x8     =>  (   8x8   )
    0x00011100,    // (      69888) LOD 9  :    4x4     =>  (   4x4   )
    0x00004440,    // (      17472) LOD 10 :    2x2     =>  (   2x2   )
    0x00011110,    // (      69904) LOD 11 :    1x1     =>  (   1x1   )
  },
  {  // 2:1 aspect ratio
    0xffde0000,    // (   -2228224) LOD 0  : 2048x1024  =>  (2048x1024)
    0xfff80000,    // (    -524288) LOD 1  : 1024x512   =>  (1024x512 )
    0xfffe0000,    // (    -131072) LOD 2  :  512x256   =>  ( 512x256 )
    0x00000000,    // (          0) LOD 3  :  256x128   =>  ( 256x128 )
    0x00000000,    // (          0) LOD 4  :  128x64    =>  ( 128x64  )
    0x00008000,    // (      32768) LOD 5  :   64x32    =>  (  64x32  )
    0x00002000,    // (       8192) LOD 6  :   32x16    =>  (  32x16  )
    0x00008800,    // (      34816) LOD 7  :   16x8     =>  (  16x8   )
    0x00002200,    // (       8704) LOD 8  :    8x4     =>  (   8x4   )
    0x00008880,    // (      34944) LOD 9  :    4x2     =>  (   4x2   )
    0x00002220,    // (       8736) LOD 10 :    2x1     =>  (   2x1   )
    0x00008888,    // (      34952) LOD 11 :    1x1     =>  (   1x1   )
  },
  {  // 4:1 aspect ratio
    0xffef0000,    // (   -1114112) LOD 0  : 2048x512   =>  (2048x512 )
    0xfffc0000,    // (    -262144) LOD 1  : 1024x256   =>  (1024x256 )
    0xffff0000,    // (     -65536) LOD 2  :  512x128   =>  ( 512x128 )
    0x00000000,    // (          0) LOD 3  :  256x64    =>  ( 256x64  )
    0x00000000,    // (          0) LOD 4  :  128x32    =>  ( 128x32  )
    0x00004000,    // (      16384) LOD 5  :   64x16    =>  (  64x16  )
    0x00001000,    // (       4096) LOD 6  :   32x8     =>  (  32x8   )
    0x00004400,    // (      17408) LOD 7  :   16x4     =>  (  16x4   )
    0x00001100,    // (       4352) LOD 8  :    8x2     =>  (   8x2   )
    0x00004440,    // (      17472) LOD 9  :    4x1     =>  (   4x1   )
    0x00001110,    // (       4368) LOD 10 :    2x1     =>  (   2x1   )
    0x00004444,    // (      17476) LOD 11 :    1x1     =>  (   1x1   )
  },
  {  // 8:1 aspect ratio
    0xfff78000,    // (    -557056) LOD 0  : 2048x256   =>  (2048x256 )
    0xfffe0000,    // (    -131072) LOD 1  : 1024x128   =>  (1024x128 )
    0xffff8000,    // (     -32768) LOD 2  :  512x64    =>  ( 512x64  )
    0x00000000,    // (          0) LOD 3  :  256x32    =>  ( 256x32  )
    0x00000000,    // (          0) LOD 4  :  128x16    =>  ( 128x16  )
    0x00002000,    // (       8192) LOD 5  :   64x8     =>  (  64x8   )
    0x00000800,    // (       2048) LOD 6  :   32x4     =>  (  32x4   )
    0x00002200,    // (       8704) LOD 7  :   16x2     =>  (  16x2   )
    0x00000880,    // (       2176) LOD 8  :    8x1     =>  (   8x1   )
    0x00002220,    // (       8736) LOD 9  :    4x1     =>  (   4x1   )
    0x00000888,    // (       2184) LOD 10 :    2x1     =>  (   2x1   )
    0x00002224,    // (       8740) LOD 11 :    1x1     =>  (   1x1   )
  },
};

FxI32 _sstMipMapOffset_NormalCompressedFourBitsPerTexelTextures[2][4][16] = 
{
 {  // tallTexture
  {  // 1:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x256   =>  ( 256x256 )
    0x00008000,    // (      32768) LOD 1  :  128x128   =>  ( 128x128 )
    0x0000a000,    // (      40960) LOD 2  :   64x64    =>  (  64x64  )
    0x0000a800,    // (      43008) LOD 3  :   32x32    =>  (  32x32  )
    0x0000aa00,    // (      43520) LOD 4  :   16x16    =>  (  16x16  )
    0x0000aa80,    // (      43648) LOD 5  :    8x8     =>  (   8x8   )
    0x0000aaa0,    // (      43680) LOD 6  :    4x4     =>  (   8x4   )
    0x0000aab0,    // (      43696) LOD 7  :    2x2     =>  (   8x4   )
    0x0000aac0,    // (      43712) LOD 8  :    1x1     =>  (   8x4   )
  },
  {  // 1:2 aspect ratio
    0x00000000,    // (          0) LOD 0  :  128x256   =>  ( 128x256 )
    0x00004000,    // (      16384) LOD 1  :   64x128   =>  (  64x128 )
    0x00005000,    // (      20480) LOD 2  :   32x64    =>  (  32x64  )
    0x00005400,    // (      21504) LOD 3  :   16x32    =>  (  16x32  )
    0x00005500,    // (      21760) LOD 4  :    8x16    =>  (   8x16  )
    0x00005540,    // (      21824) LOD 5  :    4x8     =>  (   8x8   )
    0x00005560,    // (      21856) LOD 6  :    2x4     =>  (   8x4   )
    0x00005570,    // (      21872) LOD 7  :    1x2     =>  (   8x4   )
    0x00005580,    // (      21888) LOD 8  :    1x1     =>  (   8x4   )
  },
  {  // 1:4 aspect ratio
    0x00000000,    // (          0) LOD 0  :   64x256   =>  (  64x256 )
    0x00002000,    // (       8192) LOD 1  :   32x128   =>  (  32x128 )
    0x00002800,    // (      10240) LOD 2  :   16x64    =>  (  16x64  )
    0x00002a00,    // (      10752) LOD 3  :    8x32    =>  (   8x32  )
    0x00002a80,    // (      10880) LOD 4  :    4x16    =>  (   8x16  )
    0x00002ac0,    // (      10944) LOD 5  :    2x8     =>  (   8x8   )
    0x00002ae0,    // (      10976) LOD 6  :    1x4     =>  (   8x4   )
    0x00002af0,    // (      10992) LOD 7  :    1x2     =>  (   8x4   )
    0x00002b00,    // (      11008) LOD 8  :    1x1     =>  (   8x4   )
  },
  {  // 1:8 aspect ratio
    0x00000000,    // (          0) LOD 0  :   32x256   =>  (  32x256 )
    0x00001000,    // (       4096) LOD 1  :   16x128   =>  (  16x128 )
    0x00001400,    // (       5120) LOD 2  :    8x64    =>  (   8x64  )
    0x00001500,    // (       5376) LOD 3  :    4x32    =>  (   8x32  )
    0x00001580,    // (       5504) LOD 4  :    2x16    =>  (   8x16  )
    0x000015c0,    // (       5568) LOD 5  :    1x8     =>  (   8x8   )
    0x000015e0,    // (       5600) LOD 6  :    1x4     =>  (   8x4   )
    0x000015f0,    // (       5616) LOD 7  :    1x2     =>  (   8x4   )
    0x00001600,    // (       5632) LOD 8  :    1x1     =>  (   8x4   )
  },
 },
 {  // wideTexture
  {  // 1:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x256   =>  ( 256x256 )
    0x00008000,    // (      32768) LOD 1  :  128x128   =>  ( 128x128 )
    0x0000a000,    // (      40960) LOD 2  :   64x64    =>  (  64x64  )
    0x0000a800,    // (      43008) LOD 3  :   32x32    =>  (  32x32  )
    0x0000aa00,    // (      43520) LOD 4  :   16x16    =>  (  16x16  )
    0x0000aa80,    // (      43648) LOD 5  :    8x8     =>  (   8x8   )
    0x0000aaa0,    // (      43680) LOD 6  :    4x4     =>  (   8x4   )
    0x0000aab0,    // (      43696) LOD 7  :    2x2     =>  (   8x4   )
    0x0000aac0,    // (      43712) LOD 8  :    1x1     =>  (   8x4   )
  },
  {  // 2:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x128   =>  ( 256x128 )
    0x00004000,    // (      16384) LOD 1  :  128x64    =>  ( 128x64  )
    0x00005000,    // (      20480) LOD 2  :   64x32    =>  (  64x32  )
    0x00005400,    // (      21504) LOD 3  :   32x16    =>  (  32x16  )
    0x00005500,    // (      21760) LOD 4  :   16x8     =>  (  16x8   )
    0x00005540,    // (      21824) LOD 5  :    8x4     =>  (   8x4   )
    0x00005550,    // (      21840) LOD 6  :    4x2     =>  (   8x4   )
    0x00005560,    // (      21856) LOD 7  :    2x1     =>  (   8x4   )
    0x00005570,    // (      21872) LOD 8  :    1x1     =>  (   8x4   )
  },
  {  // 4:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x64    =>  ( 256x64  )
    0x00002000,    // (       8192) LOD 1  :  128x32    =>  ( 128x32  )
    0x00002800,    // (      10240) LOD 2  :   64x16    =>  (  64x16  )
    0x00002a00,    // (      10752) LOD 3  :   32x8     =>  (  32x8   )
    0x00002a80,    // (      10880) LOD 4  :   16x4     =>  (  16x4   )
    0x00002aa0,    // (      10912) LOD 5  :    8x2     =>  (   8x4   )
    0x00002ab0,    // (      10928) LOD 6  :    4x1     =>  (   8x4   )
    0x00002ac0,    // (      10944) LOD 7  :    2x1     =>  (   8x4   )
    0x00002ad0,    // (      10960) LOD 8  :    1x1     =>  (   8x4   )
  },
  {  // 8:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x32    =>  ( 256x32  )
    0x00001000,    // (       4096) LOD 1  :  128x16    =>  ( 128x16  )
    0x00001400,    // (       5120) LOD 2  :   64x8     =>  (  64x8   )
    0x00001500,    // (       5376) LOD 3  :   32x4     =>  (  32x4   )
    0x00001540,    // (       5440) LOD 4  :   16x2     =>  (  16x4   )
    0x00001560,    // (       5472) LOD 5  :    8x1     =>  (   8x4   )
    0x00001570,    // (       5488) LOD 6  :    4x1     =>  (   8x4   )
    0x00001580,    // (       5504) LOD 7  :    2x1     =>  (   8x4   )
    0x00001590,    // (       5520) LOD 8  :    1x1     =>  (   8x4   )
  },
 }
};

FxI32 _sstMipMapOffset_NormalCompressedFourBitsPerTexelTextures_Tsplit[2][4][16] = 
{
 {  // tallTexture
  {  // 1:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x256   =>  ( 256x256 )
    0x00000000,    // (          0) LOD 1  :  128x128   =>  ( 128x128 )
    0x00008000,    // (      32768) LOD 2  :   64x64    =>  (  64x64  )
    0x00002000,    // (       8192) LOD 3  :   32x32    =>  (  32x32  )
    0x00008800,    // (      34816) LOD 4  :   16x16    =>  (  16x16  )
    0x00002200,    // (       8704) LOD 5  :    8x8     =>  (   8x8   )
    0x00008880,    // (      34944) LOD 6  :    4x4     =>  (   8x4   )
    0x00002220,    // (       8736) LOD 7  :    2x2     =>  (   8x4   )
    0x00008890,    // (      34960) LOD 8  :    1x1     =>  (   8x4   )
  },
  {  // 1:2 aspect ratio
    0x00000000,    // (          0) LOD 0  :  128x256   =>  ( 128x256 )
    0x00000000,    // (          0) LOD 1  :   64x128   =>  (  64x128 )
    0x00004000,    // (      16384) LOD 2  :   32x64    =>  (  32x64  )
    0x00001000,    // (       4096) LOD 3  :   16x32    =>  (  16x32  )
    0x00004400,    // (      17408) LOD 4  :    8x16    =>  (   8x16  )
    0x00001100,    // (       4352) LOD 5  :    4x8     =>  (   8x8   )
    0x00004440,    // (      17472) LOD 6  :    2x4     =>  (   8x4   )
    0x00001120,    // (       4384) LOD 7  :    1x2     =>  (   8x4   )
    0x00004450,    // (      17488) LOD 8  :    1x1     =>  (   8x4   )
  },
  {  // 1:4 aspect ratio
    0x00000000,    // (          0) LOD 0  :   64x256   =>  (  64x256 )
    0x00000000,    // (          0) LOD 1  :   32x128   =>  (  32x128 )
    0x00002000,    // (       8192) LOD 2  :   16x64    =>  (  16x64  )
    0x00000800,    // (       2048) LOD 3  :    8x32    =>  (   8x32  )
    0x00002200,    // (       8704) LOD 4  :    4x16    =>  (   8x16  )
    0x00000880,    // (       2176) LOD 5  :    2x8     =>  (   8x8   )
    0x00002240,    // (       8768) LOD 6  :    1x4     =>  (   8x4   )
    0x000008a0,    // (       2208) LOD 7  :    1x2     =>  (   8x4   )
    0x00002250,    // (       8784) LOD 8  :    1x1     =>  (   8x4   )
  },
  {  // 1:8 aspect ratio
    0x00000000,    // (          0) LOD 0  :   32x256   =>  (  32x256 )
    0x00000000,    // (          0) LOD 1  :   16x128   =>  (  16x128 )
    0x00001000,    // (       4096) LOD 2  :    8x64    =>  (   8x64  )
    0x00000400,    // (       1024) LOD 3  :    4x32    =>  (   8x32  )
    0x00001100,    // (       4352) LOD 4  :    2x16    =>  (   8x16  )
    0x00000480,    // (       1152) LOD 5  :    1x8     =>  (   8x8   )
    0x00001140,    // (       4416) LOD 6  :    1x4     =>  (   8x4   )
    0x000004a0,    // (       1184) LOD 7  :    1x2     =>  (   8x4   )
    0x00001150,    // (       4432) LOD 8  :    1x1     =>  (   8x4   )
  },
 },
 {  // wideTexture
  {  // 1:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x256   =>  ( 256x256 )
    0x00000000,    // (          0) LOD 1  :  128x128   =>  ( 128x128 )
    0x00008000,    // (      32768) LOD 2  :   64x64    =>  (  64x64  )
    0x00002000,    // (       8192) LOD 3  :   32x32    =>  (  32x32  )
    0x00008800,    // (      34816) LOD 4  :   16x16    =>  (  16x16  )
    0x00002200,    // (       8704) LOD 5  :    8x8     =>  (   8x8   )
    0x00008880,    // (      34944) LOD 6  :    4x4     =>  (   8x4   )
    0x00002220,    // (       8736) LOD 7  :    2x2     =>  (   8x4   )
    0x00008890,    // (      34960) LOD 8  :    1x1     =>  (   8x4   )
  },
  {  // 2:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x128   =>  ( 256x128 )
    0x00000000,    // (          0) LOD 1  :  128x64    =>  ( 128x64  )
    0x00004000,    // (      16384) LOD 2  :   64x32    =>  (  64x32  )
    0x00001000,    // (       4096) LOD 3  :   32x16    =>  (  32x16  )
    0x00004400,    // (      17408) LOD 4  :   16x8     =>  (  16x8   )
    0x00001100,    // (       4352) LOD 5  :    8x4     =>  (   8x4   )
    0x00004440,    // (      17472) LOD 6  :    4x2     =>  (   8x4   )
    0x00001110,    // (       4368) LOD 7  :    2x1     =>  (   8x4   )
    0x00004450,    // (      17488) LOD 8  :    1x1     =>  (   8x4   )
  },
  {  // 4:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x64    =>  ( 256x64  )
    0x00000000,    // (          0) LOD 1  :  128x32    =>  ( 128x32  )
    0x00002000,    // (       8192) LOD 2  :   64x16    =>  (  64x16  )
    0x00000800,    // (       2048) LOD 3  :   32x8     =>  (  32x8   )
    0x00002200,    // (       8704) LOD 4  :   16x4     =>  (  16x4   )
    0x00000880,    // (       2176) LOD 5  :    8x2     =>  (   8x4   )
    0x00002220,    // (       8736) LOD 6  :    4x1     =>  (   8x4   )
    0x00000890,    // (       2192) LOD 7  :    2x1     =>  (   8x4   )
    0x00002230,    // (       8752) LOD 8  :    1x1     =>  (   8x4   )
  },
  {  // 8:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x32    =>  ( 256x32  )
    0x00000000,    // (          0) LOD 1  :  128x16    =>  ( 128x16  )
    0x00001000,    // (       4096) LOD 2  :   64x8     =>  (  64x8   )
    0x00000400,    // (       1024) LOD 3  :   32x4     =>  (  32x4   )
    0x00001100,    // (       4352) LOD 4  :   16x2     =>  (  16x4   )
    0x00000440,    // (       1088) LOD 5  :    8x1     =>  (   8x4   )
    0x00001120,    // (       4384) LOD 6  :    4x1     =>  (   8x4   )
    0x00000450,    // (       1104) LOD 7  :    2x1     =>  (   8x4   )
    0x00001130,    // (       4400) LOD 8  :    1x1     =>  (   8x4   )
  },
 }
};

FxI32 _sstMipMapOffset_BigCompressedFourBitsPerTexelTextures[2][4][16] = 
{
 {  // tallTexture
  {  // 1:1 aspect ratio
    0xffd60000,    // (   -2752512) LOD 0  : 2048x2048  =>  (2048x2048)
    0xfff60000,    // (    -655360) LOD 1  : 1024x1024  =>  (1024x1024)
    0xfffe0000,    // (    -131072) LOD 2  :  512x512   =>  ( 512x512 )
    0x00000000,    // (          0) LOD 3  :  256x256   =>  ( 256x256 )
    0x00008000,    // (      32768) LOD 4  :  128x128   =>  ( 128x128 )
    0x0000a000,    // (      40960) LOD 5  :   64x64    =>  (  64x64  )
    0x0000a800,    // (      43008) LOD 6  :   32x32    =>  (  32x32  )
    0x0000aa00,    // (      43520) LOD 7  :   16x16    =>  (  16x16  )
    0x0000aa80,    // (      43648) LOD 8  :    8x8     =>  (   8x8   )
    0x0000aaa0,    // (      43680) LOD 9  :    4x4     =>  (   8x4   )
    0x0000aab0,    // (      43696) LOD 10 :    2x2     =>  (   8x4   )
    0x0000aac0,    // (      43712) LOD 11 :    1x1     =>  (   8x4   )
  },
  {  // 1:2 aspect ratio
    0xffeb0000,    // (   -1376256) LOD 0  : 1024x2048  =>  (1024x2048)
    0xfffb0000,    // (    -327680) LOD 1  :  512x1024  =>  ( 512x1024)
    0xffff0000,    // (     -65536) LOD 2  :  256x512   =>  ( 256x512 )
    0x00000000,    // (          0) LOD 3  :  128x256   =>  ( 128x256 )
    0x00004000,    // (      16384) LOD 4  :   64x128   =>  (  64x128 )
    0x00005000,    // (      20480) LOD 5  :   32x64    =>  (  32x64  )
    0x00005400,    // (      21504) LOD 6  :   16x32    =>  (  16x32  )
    0x00005500,    // (      21760) LOD 7  :    8x16    =>  (   8x16  )
    0x00005540,    // (      21824) LOD 8  :    4x8     =>  (   8x8   )
    0x00005560,    // (      21856) LOD 9  :    2x4     =>  (   8x4   )
    0x00005570,    // (      21872) LOD 10 :    1x2     =>  (   8x4   )
    0x00005580,    // (      21888) LOD 11 :    1x1     =>  (   8x4   )
  },
  {  // 1:4 aspect ratio
    0xfff58000,    // (    -688128) LOD 0  :  512x2048  =>  ( 512x2048)
    0xfffd8000,    // (    -163840) LOD 1  :  256x1024  =>  ( 256x1024)
    0xffff8000,    // (     -32768) LOD 2  :  128x512   =>  ( 128x512 )
    0x00000000,    // (          0) LOD 3  :   64x256   =>  (  64x256 )
    0x00002000,    // (       8192) LOD 4  :   32x128   =>  (  32x128 )
    0x00002800,    // (      10240) LOD 5  :   16x64    =>  (  16x64  )
    0x00002a00,    // (      10752) LOD 6  :    8x32    =>  (   8x32  )
    0x00002a80,    // (      10880) LOD 7  :    4x16    =>  (   8x16  )
    0x00002ac0,    // (      10944) LOD 8  :    2x8     =>  (   8x8   )
    0x00002ae0,    // (      10976) LOD 9  :    1x4     =>  (   8x4   )
    0x00002af0,    // (      10992) LOD 10 :    1x2     =>  (   8x4   )
    0x00002b00,    // (      11008) LOD 11 :    1x1     =>  (   8x4   )
  },
  {  // 1:8 aspect ratio
    0xfffac000,    // (    -344064) LOD 0  :  256x2048  =>  ( 256x2048)
    0xfffec000,    // (     -81920) LOD 1  :  128x1024  =>  ( 128x1024)
    0xffffc000,    // (     -16384) LOD 2  :   64x512   =>  (  64x512 )
    0x00000000,    // (          0) LOD 3  :   32x256   =>  (  32x256 )
    0x00001000,    // (       4096) LOD 4  :   16x128   =>  (  16x128 )
    0x00001400,    // (       5120) LOD 5  :    8x64    =>  (   8x64  )
    0x00001500,    // (       5376) LOD 6  :    4x32    =>  (   8x32  )
    0x00001580,    // (       5504) LOD 7  :    2x16    =>  (   8x16  )
    0x000015c0,    // (       5568) LOD 8  :    1x8     =>  (   8x8   )
    0x000015e0,    // (       5600) LOD 9  :    1x4     =>  (   8x4   )
    0x000015f0,    // (       5616) LOD 10 :    1x2     =>  (   8x4   )
    0x00001600,    // (       5632) LOD 11 :    1x1     =>  (   8x4   )
  },
 },
 {  // wideTexture
  {  // 1:1 aspect ratio
    0xffd60000,    // (   -2752512) LOD 0  : 2048x2048  =>  (2048x2048)
    0xfff60000,    // (    -655360) LOD 1  : 1024x1024  =>  (1024x1024)
    0xfffe0000,    // (    -131072) LOD 2  :  512x512   =>  ( 512x512 )
    0x00000000,    // (          0) LOD 3  :  256x256   =>  ( 256x256 )
    0x00008000,    // (      32768) LOD 4  :  128x128   =>  ( 128x128 )
    0x0000a000,    // (      40960) LOD 5  :   64x64    =>  (  64x64  )
    0x0000a800,    // (      43008) LOD 6  :   32x32    =>  (  32x32  )
    0x0000aa00,    // (      43520) LOD 7  :   16x16    =>  (  16x16  )
    0x0000aa80,    // (      43648) LOD 8  :    8x8     =>  (   8x8   )
    0x0000aaa0,    // (      43680) LOD 9  :    4x4     =>  (   8x4   )
    0x0000aab0,    // (      43696) LOD 10 :    2x2     =>  (   8x4   )
    0x0000aac0,    // (      43712) LOD 11 :    1x1     =>  (   8x4   )
  },
  {  // 2:1 aspect ratio
    0xffeb0000,    // (   -1376256) LOD 0  : 2048x1024  =>  (2048x1024)
    0xfffb0000,    // (    -327680) LOD 1  : 1024x512   =>  (1024x512 )
    0xffff0000,    // (     -65536) LOD 2  :  512x256   =>  ( 512x256 )
    0x00000000,    // (          0) LOD 3  :  256x128   =>  ( 256x128 )
    0x00004000,    // (      16384) LOD 4  :  128x64    =>  ( 128x64  )
    0x00005000,    // (      20480) LOD 5  :   64x32    =>  (  64x32  )
    0x00005400,    // (      21504) LOD 6  :   32x16    =>  (  32x16  )
    0x00005500,    // (      21760) LOD 7  :   16x8     =>  (  16x8   )
    0x00005540,    // (      21824) LOD 8  :    8x4     =>  (   8x4   )
    0x00005550,    // (      21840) LOD 9  :    4x2     =>  (   8x4   )
    0x00005560,    // (      21856) LOD 10 :    2x1     =>  (   8x4   )
    0x00005570,    // (      21872) LOD 11 :    1x1     =>  (   8x4   )
  },
  {  // 4:1 aspect ratio
    0xfff58000,    // (    -688128) LOD 0  : 2048x512   =>  (2048x512 )
    0xfffd8000,    // (    -163840) LOD 1  : 1024x256   =>  (1024x256 )
    0xffff8000,    // (     -32768) LOD 2  :  512x128   =>  ( 512x128 )
    0x00000000,    // (          0) LOD 3  :  256x64    =>  ( 256x64  )
    0x00002000,    // (       8192) LOD 4  :  128x32    =>  ( 128x32  )
    0x00002800,    // (      10240) LOD 5  :   64x16    =>  (  64x16  )
    0x00002a00,    // (      10752) LOD 6  :   32x8     =>  (  32x8   )
    0x00002a80,    // (      10880) LOD 7  :   16x4     =>  (  16x4   )
    0x00002aa0,    // (      10912) LOD 8  :    8x2     =>  (   8x4   )
    0x00002ab0,    // (      10928) LOD 9  :    4x1     =>  (   8x4   )
    0x00002ac0,    // (      10944) LOD 10 :    2x1     =>  (   8x4   )
    0x00002ad0,    // (      10960) LOD 11 :    1x1     =>  (   8x4   )
  },
  {  // 8:1 aspect ratio
    0xfffac000,    // (    -344064) LOD 0  : 2048x256   =>  (2048x256 )
    0xfffec000,    // (     -81920) LOD 1  : 1024x128   =>  (1024x128 )
    0xffffc000,    // (     -16384) LOD 2  :  512x64    =>  ( 512x64  )
    0x00000000,    // (          0) LOD 3  :  256x32    =>  ( 256x32  )
    0x00001000,    // (       4096) LOD 4  :  128x16    =>  ( 128x16  )
    0x00001400,    // (       5120) LOD 5  :   64x8     =>  (  64x8   )
    0x00001500,    // (       5376) LOD 6  :   32x4     =>  (  32x4   )
    0x00001540,    // (       5440) LOD 7  :   16x2     =>  (  16x4   )
    0x00001560,    // (       5472) LOD 8  :    8x1     =>  (   8x4   )
    0x00001570,    // (       5488) LOD 9  :    4x1     =>  (   8x4   )
    0x00001580,    // (       5504) LOD 10 :    2x1     =>  (   8x4   )
    0x00001590,    // (       5520) LOD 11 :    1x1     =>  (   8x4   )
  },
 }
};

FxI32 _sstMipMapOffset_BigCompressedFourBitsPerTexelTextures_Tsplit[2][4][16] = 
{
 {  // tallTexture
  {  // 1:1 aspect ratio
    0xffde0000,    // (   -2228224) LOD 0  : 2048x2048  =>  (2048x2048)
    0xfff80000,    // (    -524288) LOD 1  : 1024x1024  =>  (1024x1024)
    0xfffe0000,    // (    -131072) LOD 2  :  512x512   =>  ( 512x512 )
    0x00000000,    // (          0) LOD 3  :  256x256   =>  ( 256x256 )
    0x00000000,    // (          0) LOD 4  :  128x128   =>  ( 128x128 )
    0x00008000,    // (      32768) LOD 5  :   64x64    =>  (  64x64  )
    0x00002000,    // (       8192) LOD 6  :   32x32    =>  (  32x32  )
    0x00008800,    // (      34816) LOD 7  :   16x16    =>  (  16x16  )
    0x00002200,    // (       8704) LOD 8  :    8x8     =>  (   8x8   )
    0x00008880,    // (      34944) LOD 9  :    4x4     =>  (   8x4   )
    0x00002220,    // (       8736) LOD 10 :    2x2     =>  (   8x4   )
    0x00008890,    // (      34960) LOD 11 :    1x1     =>  (   8x4   )
  },
  {  // 1:2 aspect ratio
    0xffef0000,    // (   -1114112) LOD 0  : 1024x2048  =>  (1024x2048)
    0xfffc0000,    // (    -262144) LOD 1  :  512x1024  =>  ( 512x1024)
    0xffff0000,    // (     -65536) LOD 2  :  256x512   =>  ( 256x512 )
    0x00000000,    // (          0) LOD 3  :  128x256   =>  ( 128x256 )
    0x00000000,    // (          0) LOD 4  :   64x128   =>  (  64x128 )
    0x00004000,    // (      16384) LOD 5  :   32x64    =>  (  32x64  )
    0x00001000,    // (       4096) LOD 6  :   16x32    =>  (  16x32  )
    0x00004400,    // (      17408) LOD 7  :    8x16    =>  (   8x16  )
    0x00001100,    // (       4352) LOD 8  :    4x8     =>  (   8x8   )
    0x00004440,    // (      17472) LOD 9  :    2x4     =>  (   8x4   )
    0x00001120,    // (       4384) LOD 10 :    1x2     =>  (   8x4   )
    0x00004450,    // (      17488) LOD 11 :    1x1     =>  (   8x4   )
  },
  {  // 1:4 aspect ratio
    0xfff78000,    // (    -557056) LOD 0  :  512x2048  =>  ( 512x2048)
    0xfffe0000,    // (    -131072) LOD 1  :  256x1024  =>  ( 256x1024)
    0xffff8000,    // (     -32768) LOD 2  :  128x512   =>  ( 128x512 )
    0x00000000,    // (          0) LOD 3  :   64x256   =>  (  64x256 )
    0x00000000,    // (          0) LOD 4  :   32x128   =>  (  32x128 )
    0x00002000,    // (       8192) LOD 5  :   16x64    =>  (  16x64  )
    0x00000800,    // (       2048) LOD 6  :    8x32    =>  (   8x32  )
    0x00002200,    // (       8704) LOD 7  :    4x16    =>  (   8x16  )
    0x00000880,    // (       2176) LOD 8  :    2x8     =>  (   8x8   )
    0x00002240,    // (       8768) LOD 9  :    1x4     =>  (   8x4   )
    0x000008a0,    // (       2208) LOD 10 :    1x2     =>  (   8x4   )
    0x00002250,    // (       8784) LOD 11 :    1x1     =>  (   8x4   )
  },
  {  // 1:8 aspect ratio
    0xfffbc000,    // (    -278528) LOD 0  :  256x2048  =>  ( 256x2048)
    0xffff0000,    // (     -65536) LOD 1  :  128x1024  =>  ( 128x1024)
    0xffffc000,    // (     -16384) LOD 2  :   64x512   =>  (  64x512 )
    0x00000000,    // (          0) LOD 3  :   32x256   =>  (  32x256 )
    0x00000000,    // (          0) LOD 4  :   16x128   =>  (  16x128 )
    0x00001000,    // (       4096) LOD 5  :    8x64    =>  (   8x64  )
    0x00000400,    // (       1024) LOD 6  :    4x32    =>  (   8x32  )
    0x00001100,    // (       4352) LOD 7  :    2x16    =>  (   8x16  )
    0x00000480,    // (       1152) LOD 8  :    1x8     =>  (   8x8   )
    0x00001140,    // (       4416) LOD 9  :    1x4     =>  (   8x4   )
    0x000004a0,    // (       1184) LOD 10 :    1x2     =>  (   8x4   )
    0x00001150,    // (       4432) LOD 11 :    1x1     =>  (   8x4   )
  },
 },
 {  // wideTexture
  {  // 1:1 aspect ratio
    0xffde0000,    // (   -2228224) LOD 0  : 2048x2048  =>  (2048x2048)
    0xfff80000,    // (    -524288) LOD 1  : 1024x1024  =>  (1024x1024)
    0xfffe0000,    // (    -131072) LOD 2  :  512x512   =>  ( 512x512 )
    0x00000000,    // (          0) LOD 3  :  256x256   =>  ( 256x256 )
    0x00000000,    // (          0) LOD 4  :  128x128   =>  ( 128x128 )
    0x00008000,    // (      32768) LOD 5  :   64x64    =>  (  64x64  )
    0x00002000,    // (       8192) LOD 6  :   32x32    =>  (  32x32  )
    0x00008800,    // (      34816) LOD 7  :   16x16    =>  (  16x16  )
    0x00002200,    // (       8704) LOD 8  :    8x8     =>  (   8x8   )
    0x00008880,    // (      34944) LOD 9  :    4x4     =>  (   8x4   )
    0x00002220,    // (       8736) LOD 10 :    2x2     =>  (   8x4   )
    0x00008890,    // (      34960) LOD 11 :    1x1     =>  (   8x4   )
  },
  {  // 2:1 aspect ratio
    0xffef0000,    // (   -1114112) LOD 0  : 2048x1024  =>  (2048x1024)
    0xfffc0000,    // (    -262144) LOD 1  : 1024x512   =>  (1024x512 )
    0xffff0000,    // (     -65536) LOD 2  :  512x256   =>  ( 512x256 )
    0x00000000,    // (          0) LOD 3  :  256x128   =>  ( 256x128 )
    0x00000000,    // (          0) LOD 4  :  128x64    =>  ( 128x64  )
    0x00004000,    // (      16384) LOD 5  :   64x32    =>  (  64x32  )
    0x00001000,    // (       4096) LOD 6  :   32x16    =>  (  32x16  )
    0x00004400,    // (      17408) LOD 7  :   16x8     =>  (  16x8   )
    0x00001100,    // (       4352) LOD 8  :    8x4     =>  (   8x4   )
    0x00004440,    // (      17472) LOD 9  :    4x2     =>  (   8x4   )
    0x00001110,    // (       4368) LOD 10 :    2x1     =>  (   8x4   )
    0x00004450,    // (      17488) LOD 11 :    1x1     =>  (   8x4   )
  },
  {  // 4:1 aspect ratio
    0xfff78000,    // (    -557056) LOD 0  : 2048x512   =>  (2048x512 )
    0xfffe0000,    // (    -131072) LOD 1  : 1024x256   =>  (1024x256 )
    0xffff8000,    // (     -32768) LOD 2  :  512x128   =>  ( 512x128 )
    0x00000000,    // (          0) LOD 3  :  256x64    =>  ( 256x64  )
    0x00000000,    // (          0) LOD 4  :  128x32    =>  ( 128x32  )
    0x00002000,    // (       8192) LOD 5  :   64x16    =>  (  64x16  )
    0x00000800,    // (       2048) LOD 6  :   32x8     =>  (  32x8   )
    0x00002200,    // (       8704) LOD 7  :   16x4     =>  (  16x4   )
    0x00000880,    // (       2176) LOD 8  :    8x2     =>  (   8x4   )
    0x00002220,    // (       8736) LOD 9  :    4x1     =>  (   8x4   )
    0x00000890,    // (       2192) LOD 10 :    2x1     =>  (   8x4   )
    0x00002230,    // (       8752) LOD 11 :    1x1     =>  (   8x4   )
  },
  {  // 8:1 aspect ratio
    0xfffbc000,    // (    -278528) LOD 0  : 2048x256   =>  (2048x256 )
    0xffff0000,    // (     -65536) LOD 1  : 1024x128   =>  (1024x128 )
    0xffffc000,    // (     -16384) LOD 2  :  512x64    =>  ( 512x64  )
    0x00000000,    // (          0) LOD 3  :  256x32    =>  ( 256x32  )
    0x00000000,    // (          0) LOD 4  :  128x16    =>  ( 128x16  )
    0x00001000,    // (       4096) LOD 5  :   64x8     =>  (  64x8   )
    0x00000400,    // (       1024) LOD 6  :   32x4     =>  (  32x4   )
    0x00001100,    // (       4352) LOD 7  :   16x2     =>  (  16x4   )
    0x00000440,    // (       1088) LOD 8  :    8x1     =>  (   8x4   )
    0x00001120,    // (       4384) LOD 9  :    4x1     =>  (   8x4   )
    0x00000450,    // (       1104) LOD 10 :    2x1     =>  (   8x4   )
    0x00001130,    // (       4400) LOD 11 :    1x1     =>  (   8x4   )
  },
 }
};

FxI32 _sstMipMapOffset_NormalCompressedEightBitsPerTexelTextures[2][4][16] = 
{
 {  // tallTexture
  {  // 1:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x256   =>  ( 256x256 )
    0x00010000,    // (      65536) LOD 1  :  128x128   =>  ( 128x128 )
    0x00014000,    // (      81920) LOD 2  :   64x64    =>  (  64x64  )
    0x00015000,    // (      86016) LOD 3  :   32x32    =>  (  32x32  )
    0x00015400,    // (      87040) LOD 4  :   16x16    =>  (  16x16  )
    0x00015500,    // (      87296) LOD 5  :    8x8     =>  (   8x8   )
    0x00015540,    // (      87360) LOD 6  :    4x4     =>  (   4x4   )
    0x00015550,    // (      87376) LOD 7  :    2x2     =>  (   4x4   )
    0x00015560,    // (      87392) LOD 8  :    1x1     =>  (   4x4   )
  },
  {  // 1:2 aspect ratio
    0x00000000,    // (          0) LOD 0  :  128x256   =>  ( 128x256 )
    0x00008000,    // (      32768) LOD 1  :   64x128   =>  (  64x128 )
    0x0000a000,    // (      40960) LOD 2  :   32x64    =>  (  32x64  )
    0x0000a800,    // (      43008) LOD 3  :   16x32    =>  (  16x32  )
    0x0000aa00,    // (      43520) LOD 4  :    8x16    =>  (   8x16  )
    0x0000aa80,    // (      43648) LOD 5  :    4x8     =>  (   4x8   )
    0x0000aaa0,    // (      43680) LOD 6  :    2x4     =>  (   4x4   )
    0x0000aab0,    // (      43696) LOD 7  :    1x2     =>  (   4x4   )
    0x0000aac0,    // (      43712) LOD 8  :    1x1     =>  (   4x4   )
  },
  {  // 1:4 aspect ratio
    0x00000000,    // (          0) LOD 0  :   64x256   =>  (  64x256 )
    0x00004000,    // (      16384) LOD 1  :   32x128   =>  (  32x128 )
    0x00005000,    // (      20480) LOD 2  :   16x64    =>  (  16x64  )
    0x00005400,    // (      21504) LOD 3  :    8x32    =>  (   8x32  )
    0x00005500,    // (      21760) LOD 4  :    4x16    =>  (   4x16  )
    0x00005540,    // (      21824) LOD 5  :    2x8     =>  (   4x8   )
    0x00005560,    // (      21856) LOD 6  :    1x4     =>  (   4x4   )
    0x00005570,    // (      21872) LOD 7  :    1x2     =>  (   4x4   )
    0x00005580,    // (      21888) LOD 8  :    1x1     =>  (   4x4   )
  },
  {  // 1:8 aspect ratio
    0x00000000,    // (          0) LOD 0  :   32x256   =>  (  32x256 )
    0x00002000,    // (       8192) LOD 1  :   16x128   =>  (  16x128 )
    0x00002800,    // (      10240) LOD 2  :    8x64    =>  (   8x64  )
    0x00002a00,    // (      10752) LOD 3  :    4x32    =>  (   4x32  )
    0x00002a80,    // (      10880) LOD 4  :    2x16    =>  (   4x16  )
    0x00002ac0,    // (      10944) LOD 5  :    1x8     =>  (   4x8   )
    0x00002ae0,    // (      10976) LOD 6  :    1x4     =>  (   4x4   )
    0x00002af0,    // (      10992) LOD 7  :    1x2     =>  (   4x4   )
    0x00002b00,    // (      11008) LOD 8  :    1x1     =>  (   4x4   )
  },
 },
 {  // wideTexture
  {  // 1:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x256   =>  ( 256x256 )
    0x00010000,    // (      65536) LOD 1  :  128x128   =>  ( 128x128 )
    0x00014000,    // (      81920) LOD 2  :   64x64    =>  (  64x64  )
    0x00015000,    // (      86016) LOD 3  :   32x32    =>  (  32x32  )
    0x00015400,    // (      87040) LOD 4  :   16x16    =>  (  16x16  )
    0x00015500,    // (      87296) LOD 5  :    8x8     =>  (   8x8   )
    0x00015540,    // (      87360) LOD 6  :    4x4     =>  (   4x4   )
    0x00015550,    // (      87376) LOD 7  :    2x2     =>  (   4x4   )
    0x00015560,    // (      87392) LOD 8  :    1x1     =>  (   4x4   )
  },
  {  // 2:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x128   =>  ( 256x128 )
    0x00008000,    // (      32768) LOD 1  :  128x64    =>  ( 128x64  )
    0x0000a000,    // (      40960) LOD 2  :   64x32    =>  (  64x32  )
    0x0000a800,    // (      43008) LOD 3  :   32x16    =>  (  32x16  )
    0x0000aa00,    // (      43520) LOD 4  :   16x8     =>  (  16x8   )
    0x0000aa80,    // (      43648) LOD 5  :    8x4     =>  (   8x4   )
    0x0000aaa0,    // (      43680) LOD 6  :    4x2     =>  (   4x4   )
    0x0000aab0,    // (      43696) LOD 7  :    2x1     =>  (   4x4   )
    0x0000aac0,    // (      43712) LOD 8  :    1x1     =>  (   4x4   )
  },
  {  // 4:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x64    =>  ( 256x64  )
    0x00004000,    // (      16384) LOD 1  :  128x32    =>  ( 128x32  )
    0x00005000,    // (      20480) LOD 2  :   64x16    =>  (  64x16  )
    0x00005400,    // (      21504) LOD 3  :   32x8     =>  (  32x8   )
    0x00005500,    // (      21760) LOD 4  :   16x4     =>  (  16x4   )
    0x00005540,    // (      21824) LOD 5  :    8x2     =>  (   8x4   )
    0x00005560,    // (      21856) LOD 6  :    4x1     =>  (   4x4   )
    0x00005570,    // (      21872) LOD 7  :    2x1     =>  (   4x4   )
    0x00005580,    // (      21888) LOD 8  :    1x1     =>  (   4x4   )
  },
  {  // 8:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x32    =>  ( 256x32  )
    0x00002000,    // (       8192) LOD 1  :  128x16    =>  ( 128x16  )
    0x00002800,    // (      10240) LOD 2  :   64x8     =>  (  64x8   )
    0x00002a00,    // (      10752) LOD 3  :   32x4     =>  (  32x4   )
    0x00002a80,    // (      10880) LOD 4  :   16x2     =>  (  16x4   )
    0x00002ac0,    // (      10944) LOD 5  :    8x1     =>  (   8x4   )
    0x00002ae0,    // (      10976) LOD 6  :    4x1     =>  (   4x4   )
    0x00002af0,    // (      10992) LOD 7  :    2x1     =>  (   4x4   )
    0x00002b00,    // (      11008) LOD 8  :    1x1     =>  (   4x4   )
  },
 }
};

FxI32 _sstMipMapOffset_NormalCompressedEightBitsPerTexelTextures_Tsplit[2][4][16] = 
{
 {  // tallTexture
  {  // 1:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x256   =>  ( 256x256 )
    0x00000000,    // (          0) LOD 1  :  128x128   =>  ( 128x128 )
    0x00010000,    // (      65536) LOD 2  :   64x64    =>  (  64x64  )
    0x00004000,    // (      16384) LOD 3  :   32x32    =>  (  32x32  )
    0x00011000,    // (      69632) LOD 4  :   16x16    =>  (  16x16  )
    0x00004400,    // (      17408) LOD 5  :    8x8     =>  (   8x8   )
    0x00011100,    // (      69888) LOD 6  :    4x4     =>  (   4x4   )
    0x00004440,    // (      17472) LOD 7  :    2x2     =>  (   4x4   )
    0x00011110,    // (      69904) LOD 8  :    1x1     =>  (   4x4   )
  },
  {  // 1:2 aspect ratio
    0x00000000,    // (          0) LOD 0  :  128x256   =>  ( 128x256 )
    0x00000000,    // (          0) LOD 1  :   64x128   =>  (  64x128 )
    0x00008000,    // (      32768) LOD 2  :   32x64    =>  (  32x64  )
    0x00002000,    // (       8192) LOD 3  :   16x32    =>  (  16x32  )
    0x00008800,    // (      34816) LOD 4  :    8x16    =>  (   8x16  )
    0x00002200,    // (       8704) LOD 5  :    4x8     =>  (   4x8   )
    0x00008880,    // (      34944) LOD 6  :    2x4     =>  (   4x4   )
    0x00002220,    // (       8736) LOD 7  :    1x2     =>  (   4x4   )
    0x00008890,    // (      34960) LOD 8  :    1x1     =>  (   4x4   )
  },
  {  // 1:4 aspect ratio
    0x00000000,    // (          0) LOD 0  :   64x256   =>  (  64x256 )
    0x00000000,    // (          0) LOD 1  :   32x128   =>  (  32x128 )
    0x00004000,    // (      16384) LOD 2  :   16x64    =>  (  16x64  )
    0x00001000,    // (       4096) LOD 3  :    8x32    =>  (   8x32  )
    0x00004400,    // (      17408) LOD 4  :    4x16    =>  (   4x16  )
    0x00001100,    // (       4352) LOD 5  :    2x8     =>  (   4x8   )
    0x00004440,    // (      17472) LOD 6  :    1x4     =>  (   4x4   )
    0x00001120,    // (       4384) LOD 7  :    1x2     =>  (   4x4   )
    0x00004450,    // (      17488) LOD 8  :    1x1     =>  (   4x4   )
  },
  {  // 1:8 aspect ratio
    0x00000000,    // (          0) LOD 0  :   32x256   =>  (  32x256 )
    0x00000000,    // (          0) LOD 1  :   16x128   =>  (  16x128 )
    0x00002000,    // (       8192) LOD 2  :    8x64    =>  (   8x64  )
    0x00000800,    // (       2048) LOD 3  :    4x32    =>  (   4x32  )
    0x00002200,    // (       8704) LOD 4  :    2x16    =>  (   4x16  )
    0x00000880,    // (       2176) LOD 5  :    1x8     =>  (   4x8   )
    0x00002240,    // (       8768) LOD 6  :    1x4     =>  (   4x4   )
    0x000008a0,    // (       2208) LOD 7  :    1x2     =>  (   4x4   )
    0x00002250,    // (       8784) LOD 8  :    1x1     =>  (   4x4   )
  },
 },
 {  // wideTexture
  {  // 1:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x256   =>  ( 256x256 )
    0x00000000,    // (          0) LOD 1  :  128x128   =>  ( 128x128 )
    0x00010000,    // (      65536) LOD 2  :   64x64    =>  (  64x64  )
    0x00004000,    // (      16384) LOD 3  :   32x32    =>  (  32x32  )
    0x00011000,    // (      69632) LOD 4  :   16x16    =>  (  16x16  )
    0x00004400,    // (      17408) LOD 5  :    8x8     =>  (   8x8   )
    0x00011100,    // (      69888) LOD 6  :    4x4     =>  (   4x4   )
    0x00004440,    // (      17472) LOD 7  :    2x2     =>  (   4x4   )
    0x00011110,    // (      69904) LOD 8  :    1x1     =>  (   4x4   )
  },
  {  // 2:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x128   =>  ( 256x128 )
    0x00000000,    // (          0) LOD 1  :  128x64    =>  ( 128x64  )
    0x00008000,    // (      32768) LOD 2  :   64x32    =>  (  64x32  )
    0x00002000,    // (       8192) LOD 3  :   32x16    =>  (  32x16  )
    0x00008800,    // (      34816) LOD 4  :   16x8     =>  (  16x8   )
    0x00002200,    // (       8704) LOD 5  :    8x4     =>  (   8x4   )
    0x00008880,    // (      34944) LOD 6  :    4x2     =>  (   4x4   )
    0x00002220,    // (       8736) LOD 7  :    2x1     =>  (   4x4   )
    0x00008890,    // (      34960) LOD 8  :    1x1     =>  (   4x4   )
  },
  {  // 4:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x64    =>  ( 256x64  )
    0x00000000,    // (          0) LOD 1  :  128x32    =>  ( 128x32  )
    0x00004000,    // (      16384) LOD 2  :   64x16    =>  (  64x16  )
    0x00001000,    // (       4096) LOD 3  :   32x8     =>  (  32x8   )
    0x00004400,    // (      17408) LOD 4  :   16x4     =>  (  16x4   )
    0x00001100,    // (       4352) LOD 5  :    8x2     =>  (   8x4   )
    0x00004440,    // (      17472) LOD 6  :    4x1     =>  (   4x4   )
    0x00001120,    // (       4384) LOD 7  :    2x1     =>  (   4x4   )
    0x00004450,    // (      17488) LOD 8  :    1x1     =>  (   4x4   )
  },
  {  // 8:1 aspect ratio
    0x00000000,    // (          0) LOD 0  :  256x32    =>  ( 256x32  )
    0x00000000,    // (          0) LOD 1  :  128x16    =>  ( 128x16  )
    0x00002000,    // (       8192) LOD 2  :   64x8     =>  (  64x8   )
    0x00000800,    // (       2048) LOD 3  :   32x4     =>  (  32x4   )
    0x00002200,    // (       8704) LOD 4  :   16x2     =>  (  16x4   )
    0x00000880,    // (       2176) LOD 5  :    8x1     =>  (   8x4   )
    0x00002240,    // (       8768) LOD 6  :    4x1     =>  (   4x4   )
    0x000008a0,    // (       2208) LOD 7  :    2x1     =>  (   4x4   )
    0x00002250,    // (       8784) LOD 8  :    1x1     =>  (   4x4   )
  },
 }
};

FxI32 _sstMipMapOffset_BigCompressedEightBitsPerTexelTextures[2][4][16] = 
{
 {  // tallTexture
  {  // 1:1 aspect ratio
    0xffac0000,    // (   -5505024) LOD 0  : 2048x2048  =>  (2048x2048)
    0xffec0000,    // (   -1310720) LOD 1  : 1024x1024  =>  (1024x1024)
    0xfffc0000,    // (    -262144) LOD 2  :  512x512   =>  ( 512x512 )
    0x00000000,    // (          0) LOD 3  :  256x256   =>  ( 256x256 )
    0x00010000,    // (      65536) LOD 4  :  128x128   =>  ( 128x128 )
    0x00014000,    // (      81920) LOD 5  :   64x64    =>  (  64x64  )
    0x00015000,    // (      86016) LOD 6  :   32x32    =>  (  32x32  )
    0x00015400,    // (      87040) LOD 7  :   16x16    =>  (  16x16  )
    0x00015500,    // (      87296) LOD 8  :    8x8     =>  (   8x8   )
    0x00015540,    // (      87360) LOD 9  :    4x4     =>  (   4x4   )
    0x00015550,    // (      87376) LOD 10 :    2x2     =>  (   4x4   )
    0x00015560,    // (      87392) LOD 11 :    1x1     =>  (   4x4   )
  },
  {  // 1:2 aspect ratio
    0xffd60000,    // (   -2752512) LOD 0  : 1024x2048  =>  (1024x2048)
    0xfff60000,    // (    -655360) LOD 1  :  512x1024  =>  ( 512x1024)
    0xfffe0000,    // (    -131072) LOD 2  :  256x512   =>  ( 256x512 )
    0x00000000,    // (          0) LOD 3  :  128x256   =>  ( 128x256 )
    0x00008000,    // (      32768) LOD 4  :   64x128   =>  (  64x128 )
    0x0000a000,    // (      40960) LOD 5  :   32x64    =>  (  32x64  )
    0x0000a800,    // (      43008) LOD 6  :   16x32    =>  (  16x32  )
    0x0000aa00,    // (      43520) LOD 7  :    8x16    =>  (   8x16  )
    0x0000aa80,    // (      43648) LOD 8  :    4x8     =>  (   4x8   )
    0x0000aaa0,    // (      43680) LOD 9  :    2x4     =>  (   4x4   )
    0x0000aab0,    // (      43696) LOD 10 :    1x2     =>  (   4x4   )
    0x0000aac0,    // (      43712) LOD 11 :    1x1     =>  (   4x4   )
  },
  {  // 1:4 aspect ratio
    0xffeb0000,    // (   -1376256) LOD 0  :  512x2048  =>  ( 512x2048)
    0xfffb0000,    // (    -327680) LOD 1  :  256x1024  =>  ( 256x1024)
    0xffff0000,    // (     -65536) LOD 2  :  128x512   =>  ( 128x512 )
    0x00000000,    // (          0) LOD 3  :   64x256   =>  (  64x256 )
    0x00004000,    // (      16384) LOD 4  :   32x128   =>  (  32x128 )
    0x00005000,    // (      20480) LOD 5  :   16x64    =>  (  16x64  )
    0x00005400,    // (      21504) LOD 6  :    8x32    =>  (   8x32  )
    0x00005500,    // (      21760) LOD 7  :    4x16    =>  (   4x16  )
    0x00005540,    // (      21824) LOD 8  :    2x8     =>  (   4x8   )
    0x00005560,    // (      21856) LOD 9  :    1x4     =>  (   4x4   )
    0x00005570,    // (      21872) LOD 10 :    1x2     =>  (   4x4   )
    0x00005580,    // (      21888) LOD 11 :    1x1     =>  (   4x4   )
  },
  {  // 1:8 aspect ratio
    0xfff58000,    // (    -688128) LOD 0  :  256x2048  =>  ( 256x2048)
    0xfffd8000,    // (    -163840) LOD 1  :  128x1024  =>  ( 128x1024)
    0xffff8000,    // (     -32768) LOD 2  :   64x512   =>  (  64x512 )
    0x00000000,    // (          0) LOD 3  :   32x256   =>  (  32x256 )
    0x00002000,    // (       8192) LOD 4  :   16x128   =>  (  16x128 )
    0x00002800,    // (      10240) LOD 5  :    8x64    =>  (   8x64  )
    0x00002a00,    // (      10752) LOD 6  :    4x32    =>  (   4x32  )
    0x00002a80,    // (      10880) LOD 7  :    2x16    =>  (   4x16  )
    0x00002ac0,    // (      10944) LOD 8  :    1x8     =>  (   4x8   )
    0x00002ae0,    // (      10976) LOD 9  :    1x4     =>  (   4x4   )
    0x00002af0,    // (      10992) LOD 10 :    1x2     =>  (   4x4   )
    0x00002b00,    // (      11008) LOD 11 :    1x1     =>  (   4x4   )
  },
 },
 {  // wideTexture
  {  // 1:1 aspect ratio
    0xffac0000,    // (   -5505024) LOD 0  : 2048x2048  =>  (2048x2048)
    0xffec0000,    // (   -1310720) LOD 1  : 1024x1024  =>  (1024x1024)
    0xfffc0000,    // (    -262144) LOD 2  :  512x512   =>  ( 512x512 )
    0x00000000,    // (          0) LOD 3  :  256x256   =>  ( 256x256 )
    0x00010000,    // (      65536) LOD 4  :  128x128   =>  ( 128x128 )
    0x00014000,    // (      81920) LOD 5  :   64x64    =>  (  64x64  )
    0x00015000,    // (      86016) LOD 6  :   32x32    =>  (  32x32  )
    0x00015400,    // (      87040) LOD 7  :   16x16    =>  (  16x16  )
    0x00015500,    // (      87296) LOD 8  :    8x8     =>  (   8x8   )
    0x00015540,    // (      87360) LOD 9  :    4x4     =>  (   4x4   )
    0x00015550,    // (      87376) LOD 10 :    2x2     =>  (   4x4   )
    0x00015560,    // (      87392) LOD 11 :    1x1     =>  (   4x4   )
  },
  {  // 2:1 aspect ratio
    0xffd60000,    // (   -2752512) LOD 0  : 2048x1024  =>  (2048x1024)
    0xfff60000,    // (    -655360) LOD 1  : 1024x512   =>  (1024x512 )
    0xfffe0000,    // (    -131072) LOD 2  :  512x256   =>  ( 512x256 )
    0x00000000,    // (          0) LOD 3  :  256x128   =>  ( 256x128 )
    0x00008000,    // (      32768) LOD 4  :  128x64    =>  ( 128x64  )
    0x0000a000,    // (      40960) LOD 5  :   64x32    =>  (  64x32  )
    0x0000a800,    // (      43008) LOD 6  :   32x16    =>  (  32x16  )
    0x0000aa00,    // (      43520) LOD 7  :   16x8     =>  (  16x8   )
    0x0000aa80,    // (      43648) LOD 8  :    8x4     =>  (   8x4   )
    0x0000aaa0,    // (      43680) LOD 9  :    4x2     =>  (   4x4   )
    0x0000aab0,    // (      43696) LOD 10 :    2x1     =>  (   4x4   )
    0x0000aac0,    // (      43712) LOD 11 :    1x1     =>  (   4x4   )
  },
  {  // 4:1 aspect ratio
    0xffeb0000,    // (   -1376256) LOD 0  : 2048x512   =>  (2048x512 )
    0xfffb0000,    // (    -327680) LOD 1  : 1024x256   =>  (1024x256 )
    0xffff0000,    // (     -65536) LOD 2  :  512x128   =>  ( 512x128 )
    0x00000000,    // (          0) LOD 3  :  256x64    =>  ( 256x64  )
    0x00004000,    // (      16384) LOD 4  :  128x32    =>  ( 128x32  )
    0x00005000,    // (      20480) LOD 5  :   64x16    =>  (  64x16  )
    0x00005400,    // (      21504) LOD 6  :   32x8     =>  (  32x8   )
    0x00005500,    // (      21760) LOD 7  :   16x4     =>  (  16x4   )
    0x00005540,    // (      21824) LOD 8  :    8x2     =>  (   8x4   )
    0x00005560,    // (      21856) LOD 9  :    4x1     =>  (   4x4   )
    0x00005570,    // (      21872) LOD 10 :    2x1     =>  (   4x4   )
    0x00005580,    // (      21888) LOD 11 :    1x1     =>  (   4x4   )
  },
  {  // 8:1 aspect ratio
    0xfff58000,    // (    -688128) LOD 0  : 2048x256   =>  (2048x256 )
    0xfffd8000,    // (    -163840) LOD 1  : 1024x128   =>  (1024x128 )
    0xffff8000,    // (     -32768) LOD 2  :  512x64    =>  ( 512x64  )
    0x00000000,    // (          0) LOD 3  :  256x32    =>  ( 256x32  )
    0x00002000,    // (       8192) LOD 4  :  128x16    =>  ( 128x16  )
    0x00002800,    // (      10240) LOD 5  :   64x8     =>  (  64x8   )
    0x00002a00,    // (      10752) LOD 6  :   32x4     =>  (  32x4   )
    0x00002a80,    // (      10880) LOD 7  :   16x2     =>  (  16x4   )
    0x00002ac0,    // (      10944) LOD 8  :    8x1     =>  (   8x4   )
    0x00002ae0,    // (      10976) LOD 9  :    4x1     =>  (   4x4   )
    0x00002af0,    // (      10992) LOD 10 :    2x1     =>  (   4x4   )
    0x00002b00,    // (      11008) LOD 11 :    1x1     =>  (   4x4   )
  },
 }
};

FxI32 _sstMipMapOffset_BigCompressedEightBitsPerTexelTextures_Tsplit[2][4][16] = 
{
 {  // tallTexture
  {  // 1:1 aspect ratio
    0xffbc0000,    // (   -4456448) LOD 0  : 2048x2048  =>  (2048x2048)
    0xfff00000,    // (   -1048576) LOD 1  : 1024x1024  =>  (1024x1024)
    0xfffc0000,    // (    -262144) LOD 2  :  512x512   =>  ( 512x512 )
    0x00000000,    // (          0) LOD 3  :  256x256   =>  ( 256x256 )
    0x00000000,    // (          0) LOD 4  :  128x128   =>  ( 128x128 )
    0x00010000,    // (      65536) LOD 5  :   64x64    =>  (  64x64  )
    0x00004000,    // (      16384) LOD 6  :   32x32    =>  (  32x32  )
    0x00011000,    // (      69632) LOD 7  :   16x16    =>  (  16x16  )
    0x00004400,    // (      17408) LOD 8  :    8x8     =>  (   8x8   )
    0x00011100,    // (      69888) LOD 9  :    4x4     =>  (   4x4   )
    0x00004440,    // (      17472) LOD 10 :    2x2     =>  (   4x4   )
    0x00011110,    // (      69904) LOD 11 :    1x1     =>  (   4x4   )
  },
  {  // 1:2 aspect ratio
    0xffde0000,    // (   -2228224) LOD 0  : 1024x2048  =>  (1024x2048)
    0xfff80000,    // (    -524288) LOD 1  :  512x1024  =>  ( 512x1024)
    0xfffe0000,    // (    -131072) LOD 2  :  256x512   =>  ( 256x512 )
    0x00000000,    // (          0) LOD 3  :  128x256   =>  ( 128x256 )
    0x00000000,    // (          0) LOD 4  :   64x128   =>  (  64x128 )
    0x00008000,    // (      32768) LOD 5  :   32x64    =>  (  32x64  )
    0x00002000,    // (       8192) LOD 6  :   16x32    =>  (  16x32  )
    0x00008800,    // (      34816) LOD 7  :    8x16    =>  (   8x16  )
    0x00002200,    // (       8704) LOD 8  :    4x8     =>  (   4x8   )
    0x00008880,    // (      34944) LOD 9  :    2x4     =>  (   4x4   )
    0x00002220,    // (       8736) LOD 10 :    1x2     =>  (   4x4   )
    0x00008890,    // (      34960) LOD 11 :    1x1     =>  (   4x4   )
  },
  {  // 1:4 aspect ratio
    0xffef0000,    // (   -1114112) LOD 0  :  512x2048  =>  ( 512x2048)
    0xfffc0000,    // (    -262144) LOD 1  :  256x1024  =>  ( 256x1024)
    0xffff0000,    // (     -65536) LOD 2  :  128x512   =>  ( 128x512 )
    0x00000000,    // (          0) LOD 3  :   64x256   =>  (  64x256 )
    0x00000000,    // (          0) LOD 4  :   32x128   =>  (  32x128 )
    0x00004000,    // (      16384) LOD 5  :   16x64    =>  (  16x64  )
    0x00001000,    // (       4096) LOD 6  :    8x32    =>  (   8x32  )
    0x00004400,    // (      17408) LOD 7  :    4x16    =>  (   4x16  )
    0x00001100,    // (       4352) LOD 8  :    2x8     =>  (   4x8   )
    0x00004440,    // (      17472) LOD 9  :    1x4     =>  (   4x4   )
    0x00001120,    // (       4384) LOD 10 :    1x2     =>  (   4x4   )
    0x00004450,    // (      17488) LOD 11 :    1x1     =>  (   4x4   )
  },
  {  // 1:8 aspect ratio
    0xfff78000,    // (    -557056) LOD 0  :  256x2048  =>  ( 256x2048)
    0xfffe0000,    // (    -131072) LOD 1  :  128x1024  =>  ( 128x1024)
    0xffff8000,    // (     -32768) LOD 2  :   64x512   =>  (  64x512 )
    0x00000000,    // (          0) LOD 3  :   32x256   =>  (  32x256 )
    0x00000000,    // (          0) LOD 4  :   16x128   =>  (  16x128 )
    0x00002000,    // (       8192) LOD 5  :    8x64    =>  (   8x64  )
    0x00000800,    // (       2048) LOD 6  :    4x32    =>  (   4x32  )
    0x00002200,    // (       8704) LOD 7  :    2x16    =>  (   4x16  )
    0x00000880,    // (       2176) LOD 8  :    1x8     =>  (   4x8   )
    0x00002240,    // (       8768) LOD 9  :    1x4     =>  (   4x4   )
    0x000008a0,    // (       2208) LOD 10 :    1x2     =>  (   4x4   )
    0x00002250,    // (       8784) LOD 11 :    1x1     =>  (   4x4   )
  },
 },
 {  // wideTexture
  {  // 1:1 aspect ratio
    0xffbc0000,    // (   -4456448) LOD 0  : 2048x2048  =>  (2048x2048)
    0xfff00000,    // (   -1048576) LOD 1  : 1024x1024  =>  (1024x1024)
    0xfffc0000,    // (    -262144) LOD 2  :  512x512   =>  ( 512x512 )
    0x00000000,    // (          0) LOD 3  :  256x256   =>  ( 256x256 )
    0x00000000,    // (          0) LOD 4  :  128x128   =>  ( 128x128 )
    0x00010000,    // (      65536) LOD 5  :   64x64    =>  (  64x64  )
    0x00004000,    // (      16384) LOD 6  :   32x32    =>  (  32x32  )
    0x00011000,    // (      69632) LOD 7  :   16x16    =>  (  16x16  )
    0x00004400,    // (      17408) LOD 8  :    8x8     =>  (   8x8   )
    0x00011100,    // (      69888) LOD 9  :    4x4     =>  (   4x4   )
    0x00004440,    // (      17472) LOD 10 :    2x2     =>  (   4x4   )
    0x00011110,    // (      69904) LOD 11 :    1x1     =>  (   4x4   )
  },
  {  // 2:1 aspect ratio
    0xffde0000,    // (   -2228224) LOD 0  : 2048x1024  =>  (2048x1024)
    0xfff80000,    // (    -524288) LOD 1  : 1024x512   =>  (1024x512 )
    0xfffe0000,    // (    -131072) LOD 2  :  512x256   =>  ( 512x256 )
    0x00000000,    // (          0) LOD 3  :  256x128   =>  ( 256x128 )
    0x00000000,    // (          0) LOD 4  :  128x64    =>  ( 128x64  )
    0x00008000,    // (      32768) LOD 5  :   64x32    =>  (  64x32  )
    0x00002000,    // (       8192) LOD 6  :   32x16    =>  (  32x16  )
    0x00008800,    // (      34816) LOD 7  :   16x8     =>  (  16x8   )
    0x00002200,    // (       8704) LOD 8  :    8x4     =>  (   8x4   )
    0x00008880,    // (      34944) LOD 9  :    4x2     =>  (   4x4   )
    0x00002220,    // (       8736) LOD 10 :    2x1     =>  (   4x4   )
    0x00008890,    // (      34960) LOD 11 :    1x1     =>  (   4x4   )
  },
  {  // 4:1 aspect ratio
    0xffef0000,    // (   -1114112) LOD 0  : 2048x512   =>  (2048x512 )
    0xfffc0000,    // (    -262144) LOD 1  : 1024x256   =>  (1024x256 )
    0xffff0000,    // (     -65536) LOD 2  :  512x128   =>  ( 512x128 )
    0x00000000,    // (          0) LOD 3  :  256x64    =>  ( 256x64  )
    0x00000000,    // (          0) LOD 4  :  128x32    =>  ( 128x32  )
    0x00004000,    // (      16384) LOD 5  :   64x16    =>  (  64x16  )
    0x00001000,    // (       4096) LOD 6  :   32x8     =>  (  32x8   )
    0x00004400,    // (      17408) LOD 7  :   16x4     =>  (  16x4   )
    0x00001100,    // (       4352) LOD 8  :    8x2     =>  (   8x4   )
    0x00004440,    // (      17472) LOD 9  :    4x1     =>  (   4x4   )
    0x00001120,    // (       4384) LOD 10 :    2x1     =>  (   4x4   )
    0x00004450,    // (      17488) LOD 11 :    1x1     =>  (   4x4   )
  },
  {  // 8:1 aspect ratio
    0xfff78000,    // (    -557056) LOD 0  : 2048x256   =>  (2048x256 )
    0xfffe0000,    // (    -131072) LOD 1  : 1024x128   =>  (1024x128 )
    0xffff8000,    // (     -32768) LOD 2  :  512x64    =>  ( 512x64  )
    0x00000000,    // (          0) LOD 3  :  256x32    =>  ( 256x32  )
    0x00000000,    // (          0) LOD 4  :  128x16    =>  ( 128x16  )
    0x00002000,    // (       8192) LOD 5  :   64x8     =>  (  64x8   )
    0x00000800,    // (       2048) LOD 6  :   32x4     =>  (  32x4   )
    0x00002200,    // (       8704) LOD 7  :   16x2     =>  (  16x4   )
    0x00000880,    // (       2176) LOD 8  :    8x1     =>  (   8x4   )
    0x00002240,    // (       8768) LOD 9  :    4x1     =>  (   4x4   )
    0x000008a0,    // (       2208) LOD 10 :    2x1     =>  (   4x4   )
    0x00002250,    // (       8784) LOD 11 :    1x1     =>  (   4x4   )
  },
 }
};


//**********************************************************************
//**********************************************************************
//
//                     Tiled Mipmap offset tables
//
//**********************************************************************
//**********************************************************************
tiledStruct _sstMipMapOffset_NormalTextures_Tiled[2][4][16] = {
  {   // tall and thin (s_is_wider==0)
    {   // 1:1 aspect ratio
      {   0,    0,  384,  256},   //  0:  256x256
      { 256,    0,  128,  192},   //  1:  128x128
      { 256,  128,  112,   64},   //  2:   64x64
      { 320,  128,   48,   32},   //  3:   32x32
      { 352,  128,   16,   31},   //  4:   16x16
      { 352,  144,    8,   15},   //  5:    8x8
      { 352,  152,    4,    7},   //  6:    4x4
      { 352,  156,    2,    3},   //  7:    2x2
      { 352,  158,    1,    1},   //  8:    1x1
    },
    {   // 1:2 aspect ratio
      {   0,    0,  192,  256},   //  0:  128x256
      { 128,    0,   64,  192},   //  1:   64x128
      { 128,  128,   56,   64},   //  2:   32x64
      { 160,  128,   24,   32},   //  3:   16x32
      { 176,  128,    8,   31},   //  4:    8x16
      { 176,  144,    4,   15},   //  5:    4x8
      { 176,  152,    2,    7},   //  6:    2x4
      { 176,  156,    1,    3},   //  7:    1x2
      { 176,  158,    1,    1},   //  8:    1x1
    },
    {   // 1:4 aspect ratio
      {   0,    0,   96,  256},   //  0:   64x256
      {  64,    0,   32,  192},   //  1:   32x128
      {  64,  128,   28,   64},   //  2:   16x64
      {  80,  128,   12,   32},   //  3:    8x32
      {  88,  128,    4,   31},   //  4:    4x16
      {  88,  144,    2,   15},   //  5:    2x8
      {  88,  152,    1,    7},   //  6:    1x4
      {  88,  156,    1,    3},   //  7:    1x2
      {  88,  158,    1,    1},   //  8:    1x1
    },
    {   // 1:8 aspect ratio
      {   0,    0,   48,  256},   //  0:   32x256
      {  32,    0,   16,  192},   //  1:   16x128
      {  32,  128,   14,   64},   //  2:    8x64
      {  40,  128,    6,   32},   //  3:    4x32
      {  44,  128,    2,   31},   //  4:    2x16
      {  44,  144,    1,   15},   //  5:    1x8
      {  44,  152,    1,    7},   //  6:    1x4
      {  44,  156,    1,    3},   //  7:    1x2
      {  44,  158,    1,    1},   //  8:    1x1
    }
  },
  {   // short and wide (s_is_wider==1)
    {   // 1:1 aspect ratio
      {   0,    0,  384,  256},   //  0:  256x256
      { 256,    0,  128,  192},   //  1:  128x128
      { 256,  128,  112,   64},   //  2:   64x64
      { 320,  128,   48,   32},   //  3:   32x32
      { 352,  128,   16,   31},   //  4:   16x16
      { 352,  144,    8,   15},   //  5:    8x8
      { 352,  152,    4,    7},   //  6:    4x4
      { 352,  156,    2,    3},   //  7:    2x2
      { 352,  158,    1,    1},   //  8:    1x1
    },
    {   // 2:1 aspect ratio
      {   0,    0,  384,  128},   //  0:  256x128
      { 256,    0,  128,   96},   //  1:  128x64
      { 256,   64,  112,   32},   //  2:   64x32
      { 320,   64,   48,   16},   //  3:   32x16
      { 352,   64,   16,   16},   //  4:   16x8
      { 352,   72,    8,    8},   //  5:    8x4
      { 352,   76,    4,    4},   //  6:    4x2
      { 352,   78,    2,    2},   //  7:    2x1
      { 352,   79,    1,    1},   //  8:    1x1
    },
    {   // 4:1 aspect ratio
      {   0,    0,  384,   64},   //  0:  256x64
      { 256,    0,  128,   48},   //  1:  128x32
      { 256,   32,  112,   16},   //  2:   64x16
      { 320,   32,   48,    9},   //  3:   32x8
      { 352,   32,   16,    9},   //  4:   16x4
      { 352,   36,    8,    5},   //  5:    8x2
      { 352,   38,    4,    3},   //  6:    4x1
      { 352,   39,    2,    2},   //  7:    2x1
      { 352,   40,    1,    1},   //  8:    1x1
    },
    {   // 8:1 aspect ratio
      {   0,    0,  384,   32},   //  0:  256x32
      { 256,    0,  128,   24},   //  1:  128x16
      { 256,   16,  112,    8},   //  2:   64x8
      { 320,   16,   48,    6},   //  3:   32x4
      { 352,   16,   16,    6},   //  4:   16x2
      { 352,   18,    8,    4},   //  5:    8x1
      { 352,   19,    4,    3},   //  6:    4x1
      { 352,   20,    2,    2},   //  7:    2x1
      { 352,   21,    1,    1},   //  8:    1x1
    }
  }
};

tiledStruct _sstMipMapOffset_BigTextures_Tiled[2][4][16] = {
  {   // tall and thin (s_is_wider==0)
    {   // 1:1 aspect ratio
      {-2560, -1024, 3072, 2048},   //  0: 2048x2048
      {-512, -1024, 1024, 1536},   //  1: 1024x1024
      {-512,    0,  896,  512},   //  2:  512x512
      {   0,    0,  384,  256},   //  3:  256x256
      { 256,    0,  128,  192},   //  4:  128x128
      { 256,  128,  112,   64},   //  5:   64x64
      { 320,  128,   48,   32},   //  6:   32x32
      { 352,  128,   16,   31},   //  7:   16x16
      { 352,  144,    8,   15},   //  8:    8x8
      { 352,  152,    4,    7},   //  9:    4x4
      { 352,  156,    2,    3},   // 10:    2x2
      { 352,  158,    1,    1},   // 11:    1x1
    },
    {   // 1:2 aspect ratio
      {-1280, -1024, 1536, 2048},   //  0: 1024x2048
      {-256, -1024,  512, 1536},   //  1:  512x1024
      {-256,    0,  448,  512},   //  2:  256x512
      {   0,    0,  192,  256},   //  3:  128x256
      { 128,    0,   64,  192},   //  4:   64x128
      { 128,  128,   56,   64},   //  5:   32x64
      { 160,  128,   24,   32},   //  6:   16x32
      { 176,  128,    8,   31},   //  7:    8x16
      { 176,  144,    4,   15},   //  8:    4x8
      { 176,  152,    2,    7},   //  9:    2x4
      { 176,  156,    1,    3},   // 10:    1x2
      { 176,  158,    1,    1},   // 11:    1x1
    },
    {   // 1:4 aspect ratio
      {-640, -1024,  768, 2048},   //  0:  512x2048
      {-128, -1024,  256, 1536},   //  1:  256x1024
      {-128,    0,  224,  512},   //  2:  128x512
      {   0,    0,   96,  256},   //  3:   64x256
      {  64,    0,   32,  192},   //  4:   32x128
      {  64,  128,   28,   64},   //  5:   16x64
      {  80,  128,   12,   32},   //  6:    8x32
      {  88,  128,    4,   31},   //  7:    4x16
      {  88,  144,    2,   15},   //  8:    2x8
      {  88,  152,    1,    7},   //  9:    1x4
      {  88,  156,    1,    3},   // 10:    1x2
      {  88,  158,    1,    1},   // 11:    1x1
    },
    {   // 1:8 aspect ratio
      {-320, -1024,  384, 2048},   //  0:  256x2048
      { -64, -1024,  128, 1536},   //  1:  128x1024
      { -64,    0,  112,  512},   //  2:   64x512
      {   0,    0,   48,  256},   //  3:   32x256
      {  32,    0,   16,  192},   //  4:   16x128
      {  32,  128,   14,   64},   //  5:    8x64
      {  40,  128,    6,   32},   //  6:    4x32
      {  44,  128,    2,   31},   //  7:    2x16
      {  44,  144,    1,   15},   //  8:    1x8
      {  44,  152,    1,    7},   //  9:    1x4
      {  44,  156,    1,    3},   // 10:    1x2
      {  44,  158,    1,    1},   // 11:    1x1
    }
  },
  {   // short and wide (s_is_wider==1)
    {   // 1:1 aspect ratio
      {-2560, -1024, 3072, 2048},   //  0: 2048x2048
      {-512, -1024, 1024, 1536},   //  1: 1024x1024
      {-512,    0,  896,  512},   //  2:  512x512
      {   0,    0,  384,  256},   //  3:  256x256
      { 256,    0,  128,  192},   //  4:  128x128
      { 256,  128,  112,   64},   //  5:   64x64
      { 320,  128,   48,   32},   //  6:   32x32
      { 352,  128,   16,   31},   //  7:   16x16
      { 352,  144,    8,   15},   //  8:    8x8
      { 352,  152,    4,    7},   //  9:    4x4
      { 352,  156,    2,    3},   // 10:    2x2
      { 352,  158,    1,    1},   // 11:    1x1
    },
    {   // 2:1 aspect ratio
      {-2560, -512, 3072, 1024},   //  0: 2048x1024
      {-512, -512, 1024,  768},   //  1: 1024x512
      {-512,    0,  896,  256},   //  2:  512x256
      {   0,    0,  384,  128},   //  3:  256x128
      { 256,    0,  128,   96},   //  4:  128x64
      { 256,   64,  112,   32},   //  5:   64x32
      { 320,   64,   48,   16},   //  6:   32x16
      { 352,   64,   16,   16},   //  7:   16x8
      { 352,   72,    8,    8},   //  8:    8x4
      { 352,   76,    4,    4},   //  9:    4x2
      { 352,   78,    2,    2},   // 10:    2x1
      { 352,   79,    1,    1},   // 11:    1x1
    },
    {   // 4:1 aspect ratio
      {-2560, -256, 3072,  512},   //  0: 2048x512
      {-512, -256, 1024,  384},   //  1: 1024x256
      {-512,    0,  896,  128},   //  2:  512x128
      {   0,    0,  384,   64},   //  3:  256x64
      { 256,    0,  128,   48},   //  4:  128x32
      { 256,   32,  112,   16},   //  5:   64x16
      { 320,   32,   48,    9},   //  6:   32x8
      { 352,   32,   16,    9},   //  7:   16x4
      { 352,   36,    8,    5},   //  8:    8x2
      { 352,   38,    4,    3},   //  9:    4x1
      { 352,   39,    2,    2},   // 10:    2x1
      { 352,   40,    1,    1},   // 11:    1x1
    },
    {   // 8:1 aspect ratio
      {-2560, -128, 3072,  256},   //  0: 2048x256
      {-512, -128, 1024,  192},   //  1: 1024x128
      {-512,    0,  896,   64},   //  2:  512x64
      {   0,    0,  384,   32},   //  3:  256x32
      { 256,    0,  128,   24},   //  4:  128x16
      { 256,   16,  112,    8},   //  5:   64x8
      { 320,   16,   48,    6},   //  6:   32x4
      { 352,   16,   16,    6},   //  7:   16x2
      { 352,   18,    8,    4},   //  8:    8x1
      { 352,   19,    4,    3},   //  9:    4x1
      { 352,   20,    2,    2},   // 10:    2x1
      { 352,   21,    1,    1},   // 11:    1x1
    }
  }
};


//**********************************************************************
//**********************************************************************
//
//           Tiled Mipmap offset tables for 4 bit Compressed Textures
//
//**********************************************************************
//**********************************************************************
tiledStruct _sstMipMapOffset_NormalCompressedFourBitsPerTexelTextures_Tiled[2][4][16] = {
  {   // tall and thin (s_is_wider==0)
    {   // 1:1 aspect ratio
      {   0,    0,  384,  256},   //  0:  256x256
      { 256,    0,  128,  192},   //  1:  128x128
      { 256,  128,  112,   64},   //  2:   64x64
      { 320,  128,   48,   36},   //  3:   32x32
      { 352,  128,   16,   36},   //  4:   16x16
      { 352,  144,    8,   20},   //  5:    8x8
      { 352,  152,    8,   12},   //  6:    4x4
      { 352,  156,    8,    8},   //  7:    2x2
      { 352,  160,    8,    4},   //  8:    1x1
    },
    {   // 1:2 aspect ratio
      {   0,    0,  192,  256},   //  0:  128x256
      { 128,    0,   64,  192},   //  1:   64x128
      { 128,  128,   56,   64},   //  2:   32x64
      { 160,  128,   24,   36},   //  3:   16x32
      { 176,  128,    8,   36},   //  4:    8x16
      { 176,  144,    8,   20},   //  5:    4x8
      { 176,  152,    8,   12},   //  6:    2x4
      { 176,  156,    8,    8},   //  7:    1x2
      { 176,  160,    8,    4},   //  8:    1x1
    },
    {   // 1:4 aspect ratio
      {   0,    0,   96,  256},   //  0:   64x256
      {  64,    0,   32,  192},   //  1:   32x128
      {  64,  128,   32,   64},   //  2:   16x64
      {  80,  128,   16,   36},   //  3:    8x32
      {  88,  128,    8,   36},   //  4:    4x16
      {  88,  144,    8,   20},   //  5:    2x8
      {  88,  152,    8,   12},   //  6:    1x4
      {  88,  156,    8,    8},   //  7:    1x2
      {  88,  160,    8,    4},   //  8:    1x1
    },
    {   // 1:8 aspect ratio
      {   0,    0,   56,  256},   //  0:   32x256
      {  32,    0,   24,  192},   //  1:   16x128
      {  32,  128,   24,   64},   //  2:    8x64
      {  40,  128,   16,   36},   //  3:    4x32
      {  48,  128,    8,   36},   //  4:    2x16
      {  48,  144,    8,   20},   //  5:    1x8
      {  48,  152,    8,   12},   //  6:    1x4
      {  48,  156,    8,    8},   //  7:    1x2
      {  48,  160,    8,    4},   //  8:    1x1
    }
  },
  {   // short and wide (s_is_wider==1)
    {   // 1:1 aspect ratio
      {   0,    0,  384,  256},   //  0:  256x256
      { 256,    0,  128,  192},   //  1:  128x128
      { 256,  128,  112,   64},   //  2:   64x64
      { 320,  128,   48,   36},   //  3:   32x32
      { 352,  128,   16,   36},   //  4:   16x16
      { 352,  144,    8,   20},   //  5:    8x8
      { 352,  152,    8,   12},   //  6:    4x4
      { 352,  156,    8,    8},   //  7:    2x2
      { 352,  160,    8,    4},   //  8:    1x1
    },
    {   // 2:1 aspect ratio
      {   0,    0,  384,  128},   //  0:  256x128
      { 256,    0,  128,   96},   //  1:  128x64
      { 256,   64,  112,   32},   //  2:   64x32
      { 320,   64,   48,   24},   //  3:   32x16
      { 352,   64,   16,   24},   //  4:   16x8
      { 352,   72,    8,   16},   //  5:    8x4
      { 352,   76,    8,   12},   //  6:    4x2
      { 352,   80,    8,    8},   //  7:    2x1
      { 352,   84,    8,    4},   //  8:    1x1
    },
    {   // 4:1 aspect ratio
      {   0,    0,  384,   64},   //  0:  256x64
      { 256,    0,  128,   52},   //  1:  128x32
      { 256,   32,  112,   20},   //  2:   64x16
      { 320,   32,   48,   20},   //  3:   32x8
      { 352,   32,   16,   20},   //  4:   16x4
      { 352,   36,    8,   16},   //  5:    8x2
      { 352,   40,    8,   12},   //  6:    4x1
      { 352,   44,    8,    8},   //  7:    2x1
      { 352,   48,    8,    4},   //  8:    1x1
    },
    {   // 8:1 aspect ratio
      {   0,    0,  384,   36},   //  0:  256x32
      { 256,    0,  128,   36},   //  1:  128x16
      { 256,   16,  112,   20},   //  2:   64x8
      { 320,   16,   48,   20},   //  3:   32x4
      { 352,   16,   16,   20},   //  4:   16x2
      { 352,   20,    8,   16},   //  5:    8x1
      { 352,   24,    8,   12},   //  6:    4x1
      { 352,   28,    8,    8},   //  7:    2x1
      { 352,   32,    8,    4},   //  8:    1x1
    }
  }
};

tiledStruct _sstMipMapOffset_BigCompressedFourBitsPerTexelTextures_Tiled[2][4][16] = {
  {   // tall and thin (s_is_wider==0)
    {   // 1:1 aspect ratio
      {-2560, -1024, 3072, 2048},   //  0: 2048x2048
      {-512, -1024, 1024, 1536},   //  1: 1024x1024
      {-512,    0,  896,  512},   //  2:  512x512
      {   0,    0,  384,  256},   //  3:  256x256
      { 256,    0,  128,  192},   //  4:  128x128
      { 256,  128,  112,   64},   //  5:   64x64
      { 320,  128,   48,   36},   //  6:   32x32
      { 352,  128,   16,   36},   //  7:   16x16
      { 352,  144,    8,   20},   //  8:    8x8
      { 352,  152,    8,   12},   //  9:    4x4
      { 352,  156,    8,    8},   // 10:    2x2
      { 352,  160,    8,    4},   // 11:    1x1
    },
    {   // 1:2 aspect ratio
      {-1280, -1024, 1536, 2048},   //  0: 1024x2048
      {-256, -1024,  512, 1536},   //  1:  512x1024
      {-256,    0,  448,  512},   //  2:  256x512
      {   0,    0,  192,  256},   //  3:  128x256
      { 128,    0,   64,  192},   //  4:   64x128
      { 128,  128,   56,   64},   //  5:   32x64
      { 160,  128,   24,   36},   //  6:   16x32
      { 176,  128,    8,   36},   //  7:    8x16
      { 176,  144,    8,   20},   //  8:    4x8
      { 176,  152,    8,   12},   //  9:    2x4
      { 176,  156,    8,    8},   // 10:    1x2
      { 176,  160,    8,    4},   // 11:    1x1
    },
    {   // 1:4 aspect ratio
      {-640, -1024,  768, 2048},   //  0:  512x2048
      {-128, -1024,  256, 1536},   //  1:  256x1024
      {-128,    0,  224,  512},   //  2:  128x512
      {   0,    0,   96,  256},   //  3:   64x256
      {  64,    0,   32,  192},   //  4:   32x128
      {  64,  128,   32,   64},   //  5:   16x64
      {  80,  128,   16,   36},   //  6:    8x32
      {  88,  128,    8,   36},   //  7:    4x16
      {  88,  144,    8,   20},   //  8:    2x8
      {  88,  152,    8,   12},   //  9:    1x4
      {  88,  156,    8,    8},   // 10:    1x2
      {  88,  160,    8,    4},   // 11:    1x1
    },
    {   // 1:8 aspect ratio
      {-320, -1024,  384, 2048},   //  0:  256x2048
      { -64, -1024,  128, 1536},   //  1:  128x1024
      { -64,    0,  120,  512},   //  2:   64x512
      {   0,    0,   56,  256},   //  3:   32x256
      {  32,    0,   24,  192},   //  4:   16x128
      {  32,  128,   24,   64},   //  5:    8x64
      {  40,  128,   16,   36},   //  6:    4x32
      {  48,  128,    8,   36},   //  7:    2x16
      {  48,  144,    8,   20},   //  8:    1x8
      {  48,  152,    8,   12},   //  9:    1x4
      {  48,  156,    8,    8},   // 10:    1x2
      {  48,  160,    8,    4},   // 11:    1x1
    }
  },
  {   // short and wide (s_is_wider==1)
    {   // 1:1 aspect ratio
      {-2560, -1024, 3072, 2048},   //  0: 2048x2048
      {-512, -1024, 1024, 1536},   //  1: 1024x1024
      {-512,    0,  896,  512},   //  2:  512x512
      {   0,    0,  384,  256},   //  3:  256x256
      { 256,    0,  128,  192},   //  4:  128x128
      { 256,  128,  112,   64},   //  5:   64x64
      { 320,  128,   48,   36},   //  6:   32x32
      { 352,  128,   16,   36},   //  7:   16x16
      { 352,  144,    8,   20},   //  8:    8x8
      { 352,  152,    8,   12},   //  9:    4x4
      { 352,  156,    8,    8},   // 10:    2x2
      { 352,  160,    8,    4},   // 11:    1x1
    },
    {   // 2:1 aspect ratio
      {-2560, -512, 3072, 1024},   //  0: 2048x1024
      {-512, -512, 1024,  768},   //  1: 1024x512
      {-512,    0,  896,  256},   //  2:  512x256
      {   0,    0,  384,  128},   //  3:  256x128
      { 256,    0,  128,   96},   //  4:  128x64
      { 256,   64,  112,   32},   //  5:   64x32
      { 320,   64,   48,   24},   //  6:   32x16
      { 352,   64,   16,   24},   //  7:   16x8
      { 352,   72,    8,   16},   //  8:    8x4
      { 352,   76,    8,   12},   //  9:    4x2
      { 352,   80,    8,    8},   // 10:    2x1
      { 352,   84,    8,    4},   // 11:    1x1
    },
    {   // 4:1 aspect ratio
      {-2560, -256, 3072,  512},   //  0: 2048x512
      {-512, -256, 1024,  384},   //  1: 1024x256
      {-512,    0,  896,  128},   //  2:  512x128
      {   0,    0,  384,   64},   //  3:  256x64
      { 256,    0,  128,   52},   //  4:  128x32
      { 256,   32,  112,   20},   //  5:   64x16
      { 320,   32,   48,   20},   //  6:   32x8
      { 352,   32,   16,   20},   //  7:   16x4
      { 352,   36,    8,   16},   //  8:    8x2
      { 352,   40,    8,   12},   //  9:    4x1
      { 352,   44,    8,    8},   // 10:    2x1
      { 352,   48,    8,    4},   // 11:    1x1
    },
    {   // 8:1 aspect ratio
      {-2560, -128, 3072,  256},   //  0: 2048x256
      {-512, -128, 1024,  192},   //  1: 1024x128
      {-512,    0,  896,   64},   //  2:  512x64
      {   0,    0,  384,   36},   //  3:  256x32
      { 256,    0,  128,   36},   //  4:  128x16
      { 256,   16,  112,   20},   //  5:   64x8
      { 320,   16,   48,   20},   //  6:   32x4
      { 352,   16,   16,   20},   //  7:   16x2
      { 352,   20,    8,   16},   //  8:    8x1
      { 352,   24,    8,   12},   //  9:    4x1
      { 352,   28,    8,    8},   // 10:    2x1
      { 352,   32,    8,    4},   // 11:    1x1
    }
  }
};


//**********************************************************************
//**********************************************************************
//
//           Tiled Mipmap offset tables for 8 bit Compressed Textures
//
//**********************************************************************
//**********************************************************************
tiledStruct _sstMipMapOffset_NormalCompressedEightBitsPerTexelTextures_Tiled[2][4][16] = {
  {   // tall and thin (s_is_wider==0)
    {   // 1:1 aspect ratio
      {   0,    0,  384,  256},   //  0:  256x256
      { 256,    0,  128,  192},   //  1:  128x128
      { 256,  128,  112,   64},   //  2:   64x64
      { 320,  128,   48,   36},   //  3:   32x32
      { 352,  128,   16,   36},   //  4:   16x16
      { 352,  144,    8,   20},   //  5:    8x8
      { 352,  152,    4,   12},   //  6:    4x4
      { 352,  156,    4,    8},   //  7:    2x2
      { 352,  160,    4,    4},   //  8:    1x1
    },
    {   // 1:2 aspect ratio
      {   0,    0,  192,  256},   //  0:  128x256
      { 128,    0,   64,  192},   //  1:   64x128
      { 128,  128,   56,   64},   //  2:   32x64
      { 160,  128,   24,   36},   //  3:   16x32
      { 176,  128,    8,   36},   //  4:    8x16
      { 176,  144,    4,   20},   //  5:    4x8
      { 176,  152,    4,   12},   //  6:    2x4
      { 176,  156,    4,    8},   //  7:    1x2
      { 176,  160,    4,    4},   //  8:    1x1
    },
    {   // 1:4 aspect ratio
      {   0,    0,   96,  256},   //  0:   64x256
      {  64,    0,   32,  192},   //  1:   32x128
      {  64,  128,   28,   64},   //  2:   16x64
      {  80,  128,   12,   36},   //  3:    8x32
      {  88,  128,    4,   36},   //  4:    4x16
      {  88,  144,    4,   20},   //  5:    2x8
      {  88,  152,    4,   12},   //  6:    1x4
      {  88,  156,    4,    8},   //  7:    1x2
      {  88,  160,    4,    4},   //  8:    1x1
    },
    {   // 1:8 aspect ratio
      {   0,    0,   48,  256},   //  0:   32x256
      {  32,    0,   16,  192},   //  1:   16x128
      {  32,  128,   16,   64},   //  2:    8x64
      {  40,  128,    8,   36},   //  3:    4x32
      {  44,  128,    4,   36},   //  4:    2x16
      {  44,  144,    4,   20},   //  5:    1x8
      {  44,  152,    4,   12},   //  6:    1x4
      {  44,  156,    4,    8},   //  7:    1x2
      {  44,  160,    4,    4},   //  8:    1x1
    }
  },
  {   // short and wide (s_is_wider==1)
    {   // 1:1 aspect ratio
      {   0,    0,  384,  256},   //  0:  256x256
      { 256,    0,  128,  192},   //  1:  128x128
      { 256,  128,  112,   64},   //  2:   64x64
      { 320,  128,   48,   36},   //  3:   32x32
      { 352,  128,   16,   36},   //  4:   16x16
      { 352,  144,    8,   20},   //  5:    8x8
      { 352,  152,    4,   12},   //  6:    4x4
      { 352,  156,    4,    8},   //  7:    2x2
      { 352,  160,    4,    4},   //  8:    1x1
    },
    {   // 2:1 aspect ratio
      {   0,    0,  384,  128},   //  0:  256x128
      { 256,    0,  128,   96},   //  1:  128x64
      { 256,   64,  112,   32},   //  2:   64x32
      { 320,   64,   48,   24},   //  3:   32x16
      { 352,   64,   16,   24},   //  4:   16x8
      { 352,   72,    8,   16},   //  5:    8x4
      { 352,   76,    4,   12},   //  6:    4x2
      { 352,   80,    4,    8},   //  7:    2x1
      { 352,   84,    4,    4},   //  8:    1x1
    },
    {   // 4:1 aspect ratio
      {   0,    0,  384,   64},   //  0:  256x64
      { 256,    0,  128,   52},   //  1:  128x32
      { 256,   32,  112,   20},   //  2:   64x16
      { 320,   32,   48,   20},   //  3:   32x8
      { 352,   32,   16,   20},   //  4:   16x4
      { 352,   36,    8,   16},   //  5:    8x2
      { 352,   40,    4,   12},   //  6:    4x1
      { 352,   44,    4,    8},   //  7:    2x1
      { 352,   48,    4,    4},   //  8:    1x1
    },
    {   // 8:1 aspect ratio
      {   0,    0,  384,   36},   //  0:  256x32
      { 256,    0,  128,   36},   //  1:  128x16
      { 256,   16,  112,   20},   //  2:   64x8
      { 320,   16,   48,   20},   //  3:   32x4
      { 352,   16,   16,   20},   //  4:   16x2
      { 352,   20,    8,   16},   //  5:    8x1
      { 352,   24,    4,   12},   //  6:    4x1
      { 352,   28,    4,    8},   //  7:    2x1
      { 352,   32,    4,    4},   //  8:    1x1
    }
  }
};

tiledStruct _sstMipMapOffset_BigCompressedEightBitsPerTexelTextures_Tiled[2][4][16] = {
  {   // tall and thin (s_is_wider==0)
    {   // 1:1 aspect ratio
      {-2560, -1024, 3072, 2048},   //  0: 2048x2048
      {-512, -1024, 1024, 1536},   //  1: 1024x1024
      {-512,    0,  896,  512},   //  2:  512x512
      {   0,    0,  384,  256},   //  3:  256x256
      { 256,    0,  128,  192},   //  4:  128x128
      { 256,  128,  112,   64},   //  5:   64x64
      { 320,  128,   48,   36},   //  6:   32x32
      { 352,  128,   16,   36},   //  7:   16x16
      { 352,  144,    8,   20},   //  8:    8x8
      { 352,  152,    4,   12},   //  9:    4x4
      { 352,  156,    4,    8},   // 10:    2x2
      { 352,  160,    4,    4},   // 11:    1x1
    },
    {   // 1:2 aspect ratio
      {-1280, -1024, 1536, 2048},   //  0: 1024x2048
      {-256, -1024,  512, 1536},   //  1:  512x1024
      {-256,    0,  448,  512},   //  2:  256x512
      {   0,    0,  192,  256},   //  3:  128x256
      { 128,    0,   64,  192},   //  4:   64x128
      { 128,  128,   56,   64},   //  5:   32x64
      { 160,  128,   24,   36},   //  6:   16x32
      { 176,  128,    8,   36},   //  7:    8x16
      { 176,  144,    4,   20},   //  8:    4x8
      { 176,  152,    4,   12},   //  9:    2x4
      { 176,  156,    4,    8},   // 10:    1x2
      { 176,  160,    4,    4},   // 11:    1x1
    },
    {   // 1:4 aspect ratio
      {-640, -1024,  768, 2048},   //  0:  512x2048
      {-128, -1024,  256, 1536},   //  1:  256x1024
      {-128,    0,  224,  512},   //  2:  128x512
      {   0,    0,   96,  256},   //  3:   64x256
      {  64,    0,   32,  192},   //  4:   32x128
      {  64,  128,   28,   64},   //  5:   16x64
      {  80,  128,   12,   36},   //  6:    8x32
      {  88,  128,    4,   36},   //  7:    4x16
      {  88,  144,    4,   20},   //  8:    2x8
      {  88,  152,    4,   12},   //  9:    1x4
      {  88,  156,    4,    8},   // 10:    1x2
      {  88,  160,    4,    4},   // 11:    1x1
    },
    {   // 1:8 aspect ratio
      {-320, -1024,  384, 2048},   //  0:  256x2048
      { -64, -1024,  128, 1536},   //  1:  128x1024
      { -64,    0,  112,  512},   //  2:   64x512
      {   0,    0,   48,  256},   //  3:   32x256
      {  32,    0,   16,  192},   //  4:   16x128
      {  32,  128,   16,   64},   //  5:    8x64
      {  40,  128,    8,   36},   //  6:    4x32
      {  44,  128,    4,   36},   //  7:    2x16
      {  44,  144,    4,   20},   //  8:    1x8
      {  44,  152,    4,   12},   //  9:    1x4
      {  44,  156,    4,    8},   // 10:    1x2
      {  44,  160,    4,    4},   // 11:    1x1
    }
  },
  {   // short and wide (s_is_wider==1)
    {   // 1:1 aspect ratio
      {-2560, -1024, 3072, 2048},   //  0: 2048x2048
      {-512, -1024, 1024, 1536},   //  1: 1024x1024
      {-512,    0,  896,  512},   //  2:  512x512
      {   0,    0,  384,  256},   //  3:  256x256
      { 256,    0,  128,  192},   //  4:  128x128
      { 256,  128,  112,   64},   //  5:   64x64
      { 320,  128,   48,   36},   //  6:   32x32
      { 352,  128,   16,   36},   //  7:   16x16
      { 352,  144,    8,   20},   //  8:    8x8
      { 352,  152,    4,   12},   //  9:    4x4
      { 352,  156,    4,    8},   // 10:    2x2
      { 352,  160,    4,    4},   // 11:    1x1
    },
    {   // 2:1 aspect ratio
      {-2560, -512, 3072, 1024},   //  0: 2048x1024
      {-512, -512, 1024,  768},   //  1: 1024x512
      {-512,    0,  896,  256},   //  2:  512x256
      {   0,    0,  384,  128},   //  3:  256x128
      { 256,    0,  128,   96},   //  4:  128x64
      { 256,   64,  112,   32},   //  5:   64x32
      { 320,   64,   48,   24},   //  6:   32x16
      { 352,   64,   16,   24},   //  7:   16x8
      { 352,   72,    8,   16},   //  8:    8x4
      { 352,   76,    4,   12},   //  9:    4x2
      { 352,   80,    4,    8},   // 10:    2x1
      { 352,   84,    4,    4},   // 11:    1x1
    },
    {   // 4:1 aspect ratio
      {-2560, -256, 3072,  512},   //  0: 2048x512
      {-512, -256, 1024,  384},   //  1: 1024x256
      {-512,    0,  896,  128},   //  2:  512x128
      {   0,    0,  384,   64},   //  3:  256x64
      { 256,    0,  128,   52},   //  4:  128x32
      { 256,   32,  112,   20},   //  5:   64x16
      { 320,   32,   48,   20},   //  6:   32x8
      { 352,   32,   16,   20},   //  7:   16x4
      { 352,   36,    8,   16},   //  8:    8x2
      { 352,   40,    4,   12},   //  9:    4x1
      { 352,   44,    4,    8},   // 10:    2x1
      { 352,   48,    4,    4},   // 11:    1x1
    },
    {   // 8:1 aspect ratio
      {-2560, -128, 3072,  256},   //  0: 2048x256
      {-512, -128, 1024,  192},   //  1: 1024x128
      {-512,    0,  896,   64},   //  2:  512x64
      {   0,    0,  384,   36},   //  3:  256x32
      { 256,    0,  128,   36},   //  4:  128x16
      { 256,   16,  112,   20},   //  5:   64x8
      { 320,   16,   48,   20},   //  6:   32x4
      { 352,   16,   16,   20},   //  7:   16x2
      { 352,   20,    8,   16},   //  8:    8x1
      { 352,   24,    4,   12},   //  9:    4x1
      { 352,   28,    4,    8},   // 10:    2x1
      { 352,   32,    4,    4},   // 11:    1x1
    }
  }
};


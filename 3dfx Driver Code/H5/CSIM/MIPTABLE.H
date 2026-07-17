#ifndef MIPTABLE_H
#define MIPTABLE_H

#include <h3.h>
#include "csim.h"

//Linear stuff
extern FxI32 _sstMipMapSize_NormalTextures[4][16]; 
extern FxI32 _sstMipMapSize_BigTextures[4][16]; 
extern FxI32 _sstMipMapSize_NormalCompressedFourBitsPerTexelTextures[2][4][16]; 
extern FxI32 _sstMipMapSize_BigCompressedFourBitsPerTexelTextures[2][4][16]; 
extern FxI32 _sstMipMapSize_NormalCompressedEightBitsPerTexelTextures[2][4][16]; 
extern FxI32 _sstMipMapSize_BigCompressedEightBitsPerTexelTextures[2][4][16]; 
extern FxI32 _sstMipMapOffset_NormalTextures[4][16]; 
extern FxI32 _sstMipMapOffset_NormalTextures_Tsplit[4][16]; 
extern FxI32 _sstMipMapOffset_BigTextures[4][16]; 
extern FxI32 _sstMipMapOffset_BigTextures_Tsplit[4][16]; 
extern FxI32 _sstMipMapOffset_NormalCompressedFourBitsPerTexelTextures[2][4][16]; 
extern FxI32 _sstMipMapOffset_NormalCompressedFourBitsPerTexelTextures_Tsplit[2][4][16]; 
extern FxI32 _sstMipMapOffset_BigCompressedFourBitsPerTexelTextures[2][4][16]; 
extern FxI32 _sstMipMapOffset_BigCompressedFourBitsPerTexelTextures_Tsplit[2][4][16]; 
extern FxI32 _sstMipMapOffset_NormalCompressedEightBitsPerTexelTextures[2][4][16]; 
extern FxI32 _sstMipMapOffset_NormalCompressedEightBitsPerTexelTextures_Tsplit[2][4][16]; 
extern FxI32 _sstMipMapOffset_BigCompressedEightBitsPerTexelTextures[2][4][16]; 
extern FxI32 _sstMipMapOffset_BigCompressedEightBitsPerTexelTextures_Tsplit[2][4][16]; 

extern tiledStruct _sstMipMapOffset_NormalTextures_Tiled[2][4][16];
extern tiledStruct _sstMipMapOffset_BigTextures_Tiled[2][4][16];
extern tiledStruct _sstMipMapOffset_NormalCompressedFourBitsPerTexelTextures_Tiled[2][4][16];
extern tiledStruct _sstMipMapOffset_BigCompressedFourBitsPerTexelTextures_Tiled[2][4][16];
extern tiledStruct _sstMipMapOffset_NormalCompressedEightBitsPerTexelTextures_Tiled[2][4][16];
extern tiledStruct _sstMipMapOffset_BigCompressedEightBitsPerTexelTextures_Tiled[2][4][16];

#endif

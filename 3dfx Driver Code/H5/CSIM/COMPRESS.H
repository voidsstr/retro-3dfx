#ifndef _COMP_H
#define _COMP_H


extern FxBool csimDecompressTexels(FxU32 *texelBlock, FxU32 tMode, FxU32 *decompressedTexels);

/*************************************************************
 *************************************************************
 *          Private stuff from compressed textures
 *************************************************************
 *************************************************************/


#define CCCLEAR(cc) \
(cc).w[0] = (cc).w[1] = (cc).w[2] = (cc).w[3] = 0

#define SETBIT(var, which, val) \
{						\
  (var) &= ~(1 << (which)); 			\
  (var) |= (val) << (which);			\
}

#define GETBIT(var, which) (((var) >> (which)) & 1)

#define CC_SETBIT(cc, which, val) \
SETBIT((cc).w[(which) >> 5], (which) & 31, val)

#define CC_GETBIT(cc, which) \
  GETBIT((cc).w[(which) >> 5], (which) & 31)

#define CC_MIXED_MODE      127
#define CC_MIXED_SEL_1     126
#define CC_MIXED_SEL_0     125
#define CC_MIXED_ALPHA     124
#define CC_MIXED_COLOR1     94
#define CC_MIXED_COLOR0     64
#define CC_MIXED_TEXELS_HI  32
#define CC_MIXED_TEXELS_LO   0

#define CC_HI_MODE_1 127
#define CC_HI_MODE_0 126
#define CC_HI_COLOR1 111
#define CC_HI_COLOR0  96
#define CC_HI_TEXELS   0

#define CC_CHROMA_MODE_2 127
#define CC_CHROMA_MODE_1 126
#define CC_CHROMA_MODE_0 125
#define CC_CHROMA_COLOR3 109
#define CC_CHROMA_COLOR2  94
#define CC_CHROMA_COLOR1  79
#define CC_CHROMA_COLOR0  64
#define CC_CHROMA_TEXELS   0



typedef struct {
  FxU32 w[4];
} CCBlock;
#endif



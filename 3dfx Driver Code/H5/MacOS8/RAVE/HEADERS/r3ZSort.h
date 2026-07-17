#pragma once

typedef struct CacheTriangle {
	UInt8		triType;
	GrVertex	vert[3];
	TQATexture* baseTexture;
//	TQATexture* multiTexture;
	UInt32		stateIdx;
#if !CUSTOM_GLIDE_STATE_DEF
	GrAlphaBlendFnc_t rgb_sf;
	GrAlphaBlendFnc_t rgb_df;
#endif
} CacheTriangle;


typedef struct CacheIndex
{
	float			fAvgZ;
	float			fMaxZ;
	UInt32			index;
} CacheIndex;


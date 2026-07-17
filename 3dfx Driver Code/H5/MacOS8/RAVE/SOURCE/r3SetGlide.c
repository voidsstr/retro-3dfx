//: sst1Glide.c
//: alt.drivers inc.
//: Glenn Nissen


#include "r3Core.h"
extern TRvInfo gRvEngInfo;

void SetGlide_Z( TQADrawPrivate* dp)
{
	FxU32	oldBytesPerPixel = 0;
	UInt32 currZFunc = dp->tState[kQATag_ZFunction].i;
	
	switch(currZFunc) {
	case kQAZFunction_None:
		DebugStr("Z Func None, CMP_ALWAYS ");
		dp->glDepthFunc = GR_CMP_ALWAYS;
		break;

	case kQAZFunction_LT:
		DebugStr("Z Func RAVE Less Then, ");
		DebugStr("Glide Depth Func GR_CMP_LESS ");
		dp->glDepthFunc = GR_CMP_LESS;
		break;

	case kQAZFunction_True:
		DebugStr("Z Func True, CMP_ALWAYS ");
		dp->glDepthFunc = GR_CMP_ALWAYS;
		break;

#if TNSL_OPENGL
	case kQAZFunction_EQ:
		DebugStr("Z Func Equal");
		dp->glDepthFunc = GR_CMP_EQUAL;
		break;

	case kQAZFunction_LE:
		DebugStr("Z Func Less Then or Equal");
		dp->glDepthFunc = GR_CMP_LEQUAL;
		break;

	case kQAZFunction_GT:
		DebugStr("Z Func Greater Than, ");
		DebugStr("Glide Depth Func GR_CMP_GREATER ");
		dp->glDepthFunc = GR_CMP_GREATER;
		break;

	case kQAZFunction_NE:
		DebugStr("Z Func Not Equal");
		dp->glDepthFunc = GR_CMP_NOTEQUAL;
		break;

	case kQAZFunction_GE:
		DebugStr("Z Func Greater Than or Equal");
		dp->glDepthFunc = GR_CMP_GEQUAL;
		break;

#endif

	default:
		DebugStr("*** unexpected or OpenGL ZFunction\n");
		dp->glDepthFunc = GR_CMP_LESS;
		break;
	}
	DebugStr("\n");
	grDepthMask(dp->tState[kQATag_ZFunction].i != kQAZFunction_None);
	grDepthBufferFunction(dp->glDepthFunc);
	#if TNSL_DEBUG
	if (!dp->bFullScreen && dp->auxSurface)
	{
		GrSurfaceDesc_t		theSfcDesc;
		grSurfaceGetDescExt(dp->auxSurface, &theSfcDesc);
		oldBytesPerPixel = theSfcDesc.bytesPerPixel;
		assert( oldBytesPerPixel == dp->auxSurfaceDesc.bytesPerPixel );
	}
	#endif
}

#pragma mark -


void SetGlide_DitherMode( TQADrawPrivate* dp)
{

	grDitherMode( (dp->flags & kQAContext_NoDither) ? GR_DITHER_DISABLE : GR_DITHER_4x4);
	
}

// consequence of kQATag_ChannelMask
void SetGlide_Mask( TQADrawPrivate* dp)
{
	#pragma unused( dp )
	// Cannot be implemented because Glide doesn't support
	// separate channels enabling.
}

// consequence of kQATag_Blend

void SetGlide_Alpha( TQADrawPrivate* dp)
{
	UInt32	currBlend = dp->tState[kQATag_Blend].i;
	
	GrAlphaBlendFnc_t		rgb_sf;
	GrAlphaBlendFnc_t		rgb_df;
		
	rgb_df 		= GR_BLEND_ONE_MINUS_SRC_ALPHA;

	DebugStr("SetGlide_Alpha ");
	switch(currBlend) 
	{
	// This is used in Nanosaur and Bugdom.
	case kQABlend_PreMultiply:
		DebugStr("Blend PreMultiply ");
		rgb_sf = GR_BLEND_ONE;
		break;
		
	case kQABlend_Interpolate:
		DebugStr("Blend Interpolate ");
		rgb_sf = GR_BLEND_SRC_ALPHA;
		break;

	case kQABlend_OpenGL:
		DebugStr("*** Blend (OpenGL) handled elsewhere ");
	// This case is handled by SetRenderState after calls to BlendSrc, etc.
		return;
	default:
		DebugStr("*** unexpected Blend");
		return;
	}
	DebugStr("\n");
	dp->rgb_sf = rgb_sf;
	dp->rgb_df = rgb_df;

	grAlphaBlendFunction(rgb_sf, rgb_df, GR_BLEND_ONE, GR_BLEND_ZERO);
	DebugStr("grAlphaBlendFunction ");
}


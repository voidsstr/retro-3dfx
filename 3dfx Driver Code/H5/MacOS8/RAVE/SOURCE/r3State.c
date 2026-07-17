//: r3State.c
//: alt.drivers inc.
//: Glenn Nissen
//: Vadim Kochubievski
//: Rohit Gundecha

#include "r3Core.h"
#include "r3Context.h"
#include "r3VertexMacros.h"

extern TRvInfo gRvEngInfo;

////////////////////
//                //
//  RvSetFloat  //
//                //
////////////////////

void 
RvSetFloat(	TQADrawContext* drawContext,
				TQATagFloat     tag,
				float           newValue)
{
	TQADrawPrivate* dp = drawContext->drawPrivate;
	
	AppTimerEnd();
	DrvTimerStart();
	
	DebugStr("--> RvSetFloat - ");
	DebugNum(tag);
	DebugFloat(newValue);
	
	if ( tag <= kRaveMaxTag )
	if ( dp->tState[tag].f == newValue )	// nothing to do
	{
		DebugStr(" redundant call\n" );
		return;
	}

	// assert( tag < kQATag_EngineSpecific_Minimum );
		
	switch(tag) 
	{
	case kQATag_ColorBG_a:
		dp->tState[kQATag_ColorBG_a].f = max(min(newValue, 1.0f), 0.0f);
		DebugStr("Background Alpha: ");
		DebugNum((unsigned long) CONVERT_A(dp->tState[kQATag_ColorBG_a].f));
		break;
		
	case kQATag_ColorBG_r:
		dp->tState[kQATag_ColorBG_r].f = max(min(newValue, 1.0f), 0.0f);
		// dp->tState[kQATag_ColorBG_r].f = 0.74;
		DebugStr("Background Red: ");
		DebugNum((unsigned long) CONVERT_R(dp->tState[kQATag_ColorBG_r].f));
		break;
		
	case kQATag_ColorBG_g:
		dp->tState[kQATag_ColorBG_g].f = max(min(newValue, 1.0f), 0.0f);
		// dp->tState[kQATag_ColorBG_g].f = 0.05;
		DebugStr("Background Green: ");
		DebugNum((unsigned long) CONVERT_G(dp->tState[kQATag_ColorBG_g].f));
		break;
		
	case kQATag_ColorBG_b:
		dp->tState[kQATag_ColorBG_b].f = max(min(newValue, 1.0f), 0.0f);
		// dp->tState[kQATag_ColorBG_b].f = 0.06;
		DebugStr("Background Blue: ");
		DebugNum((unsigned long) CONVERT_B(dp->tState[kQATag_ColorBG_b].f));
		break;
		
	case kQATag_FogColor_a:
	case 1006:				// ATI-specific
		dp->tState[kQATag_FogColor_a].f = max(min(newValue, 1.0f), 0.0f);
		dp->tChangeMask		|= STATE_FOG;
    	dp->tFogChangeMask 	|= STATE_FOG_COLOR;
		DebugStr("Fog Alpha: ");
		DebugNum((unsigned long) CONVERT_A(dp->tState[kQATag_FogColor_a].f));
		break;
		
	case kQATag_FogColor_r:
	case 1003:
		dp->tState[kQATag_FogColor_r].f = max(min(newValue, 1.0f), 0.0f);
		dp->tChangeMask		|= STATE_FOG;
    	dp->tFogChangeMask 	|= STATE_FOG_COLOR;
		DebugStr("Fog Red: ");
		DebugNum((unsigned long) CONVERT_R(dp->tState[kQATag_FogColor_r].f));
		break;
		
	case kQATag_FogColor_g:
	case 1004:
		dp->tState[kQATag_FogColor_g].f = max(min(newValue, 1.0f), 0.0f);
		dp->tChangeMask		|= STATE_FOG;
    	dp->tFogChangeMask 	|= STATE_FOG_COLOR;
		DebugStr("Fog Green: ");
		DebugNum((unsigned long) CONVERT_G(dp->tState[kQATag_FogColor_g].f));
		break;
		
	case kQATag_FogColor_b:
	case 1005:
		dp->tState[kQATag_FogColor_b].f = max(min(newValue, 1.0f), 0.0f);
		dp->tChangeMask		|= STATE_FOG;
    	dp->tFogChangeMask 	|= STATE_FOG_COLOR;
		DebugStr("Fog Blue: ");
		DebugNum((unsigned long) CONVERT_B(dp->tState[kQATag_FogColor_b].f));
		break;

	// used only with FogMode linear		
    case kQATag_FogStart:
    case 1008:		// ATI-specific
		if( newValue < 0.0 )
			newValue = 0.0;
		
		dp->tState[kQATag_FogStart].f 	= newValue;
		dp->tChangeMask 				|= STATE_FOG;
		dp->tFogChangeMask 				|= STATE_FOG_START;
		DebugStr("Fog Start: ");
		DebugFloat(dp->tState[kQATag_FogStart].f);
    	break;
        
	// used only with FogMode linear		
    case kQATag_FogEnd:
    case 1009:
    	if( newValue < 0.0001 )
			newValue = 0.0001;

		dp->tState[kQATag_FogEnd].f 		= newValue;
		dp->tChangeMask 				|= STATE_FOG;
		dp->tFogChangeMask 				|= STATE_FOG_END;
		DebugStr("Fog End: ");
		DebugFloat(dp->tState[kQATag_FogEnd].f);
    	break;
 
 	// used only with FogMode exp & exp2       	
    case kQATag_FogDensity:
    case 1007:
		if( newValue < 0.0 )
			newValue = 0.0;
		
		dp->tState[kQATag_FogDensity].f 	= newValue;
		dp->tChangeMask 				|= STATE_FOG;
		dp->tFogChangeMask 				|= STATE_FOG_DENSITY;
		DebugStr("Fog Density: ");
		DebugFloat(dp->tState[kQATag_FogDensity].f);
    	break;

	// for use with internal fog tables
	case kQATag_FogMaxDepth:
		dp->tChangeMask 					|= STATE_FOG;
		dp->tFogChangeMask 					|= STATE_FOG_MAX_DEPTH; 
		dp->tState[kQATag_FogMaxDepth].f = newValue;
		DebugStr("Fog MaxDepth: ");
		DebugFloat(dp->tState[kQATag_FogMaxDepth].f);
		break;
		
	case kQATag_AlphaTestRef:   
    	dp->tState[kQATag_AlphaTestRef].f 	= newValue;
    	dp->tChangeMask 					|= STATE_ALPHA_TST_REF;
    	DebugStr("Alpha Test Ref: ");
    	DebugFloat(dp->tState[kQATag_AlphaTestRef].f );
    	break;
 


	case kQATag_Width:
		dp->tState[kQATag_Width].f = max(min(newValue, kQAMaxWidth), 0.0f);
		DebugStr("Line Width: ");
		dp->tChangeMask |= STATE_PT_WIDTH;
		DebugFloat( dp->tState[kQATag_Width].f);
		break;

	case kQATag_ZMinOffset:
		dp->tState[kQATag_ZMinOffset].f  = newValue;
		DebugStr("Z Min Offset: ");
		DebugFloat( dp->tState[kQATag_ZMinOffset].f );
		break;

	case kQATag_ZMinScale:
		dp->tState[kQATag_ZMinScale].f = newValue;
		DebugStr("Z Min Scale: ");
		DebugFloat( dp->tState[kQATag_ZMinScale].f);
		break;
		
	case kQATag_MultiTextureFactor:
		dp->tState[kQATag_MultiTextureFactor].f = newValue;
		dp->tChangeMask 				|= STATE_COMPOSITE;
		dp->tTex2ChangeMask 			|= STATE_COMP_TEX_OP;
		DebugStr("MultiTextureFactor =");
		DebugFloat( dp->tState[kQATag_MultiTextureFactor].f );
		break;

#if TNSL_GL	
	case kQATagGL_DepthBG:
		break;
		
	case kQATagGL_TextureBorder_a:
	case kQATagGL_TextureBorder_r:
	case kQATagGL_TextureBorder_g:
	case kQATagGL_TextureBorder_b:
		DebugStr("*** unsupported tag (OpenGL)");
		goto bail;
#endif /* TNSL_GL	*/

	case kQATag_Chromakey_r:
	case kQATag_Chromakey_g:
	case kQATag_Chromakey_b:
		dp->tState[tag].f = newValue;
		dp->tChangeMask 				|= STATE_CHROMAKEY;
		DebugStr("Chromakey ");
		break;
		 			    	
	case kQATag_BitmapScale_x:
		dp->tState[kQATag_BitmapScale_x].f = newValue;
		DebugStr("kQATag_BitmapScale_x ");
		break;
		
	case kQATag_BitmapScale_y:
		dp->tState[kQATag_BitmapScale_y].f = newValue;
		DebugStr("kQATag_BitmapScale_y ");
		break;

	default:
		DebugStr("*** unexpected tag");
		goto bail;
	}

bail:
	DebugStr("\n");
	
	DrvTimerEnd();
	AppTimerStart();
}


//////////////////
//              //
//  RvSetInt  //
//              //
//////////////////

void 
RvSetInt(		TQADrawContext* drawContext,
				TQATagInt       tag,
				unsigned long   newValue)
{
	TQADrawPrivate* dp = drawContext->drawPrivate;
	
	AppTimerEnd();
	DrvTimerStart();
	
	DebugStr("--> RvSetInt - ");
	DebugNum(tag);
	DebugNum(newValue);
	
	if ( tag <= kRaveMaxTag )
		if (dp->tState[tag].i == newValue )	// nothing to do
		{
			DebugStr(" redundant call\n" );
			return;
		}
	// assert( tag < kQATag_EngineSpecific_Minimum );
	
	
	switch(tag) 
	{
	// ATI ZWriteEnable
	case 1022:
		DebugStr("ATI EnableZWrite ");
		newValue = max( 0, newValue);
		if ( newValue == 0 )
			grDepthMask( FXFALSE );
		else
			grDepthMask(dp->tState[kQATag_ZFunction].i != kQAZFunction_None);
		break;
		
	case kQATag_PerspectiveZ:
		// ati driver does nothing with this.
		dp->tState[kQATag_PerspectiveZ].i = newValue;
		// 2/15/00 not going to deal with it 
		DebugStr("PerspectiveZ  -- changes allowed, but no effect ");
		DebugNum( newValue );
		break;
		
	case kQATag_ZFunction:
		if((dp->flags & kQAContext_NoZBuffer) && (newValue != kQAZFunction_None)) 
		{
			DebugStr("*** NoZBuffer && ZFunction: ");
			dp->tState[tag].i = newValue = kQAZFunction_None;
		}
		else
		{
			DebugStr("ZFunc ");
			dp->tState[tag].i = newValue;
        	// dp->tChangeMask |= (STATE_Z_FUNC | STATE_ALPHASORTING | STATE_DRAW_DESTINATION);
		}
		dp->tChangeMask |= (STATE_Z_FUNC | STATE_ALPHASORTING);	//+ may not be needed
		break;

	case 1002:			// ATI-specific; map to standard Rave values.
		DebugStr("FogMode (ATI) ");
		DebugNum( newValue );
		dp->atiFog = FXTRUE;
		switch (newValue) {
			case 0:		newValue = 0; break;
			case 1:		newValue = 3; break;
			case 2: 	newValue = 4; break;
			case 3: 	newValue = 1; break;
			case 4:		newValue = 2; break;
		}
		DebugStr("FogMode (RAVE) ");
		DebugNum( newValue );
		dp->tState[kQATag_FogMode].i = max(min(newValue, 4), 0);
		dp->tChangeMask 			|= STATE_FOG;
		dp->tFogChangeMask 			|= STATE_FOG_MODE;
		break;
		// no break, fall thru
	case kQATag_FogMode:
		if (!dp->atiFog)
		{
			DebugStr("FogMode (RAVE) ");
			DebugNum( newValue );
			dp->tState[kQATag_FogMode].i = max(min(newValue, 4), 0);
			DebugStr("FogMode as stored ");
			DebugNum( dp->tState[kQATag_FogMode].i );
			dp->tChangeMask 			|= STATE_FOG;
			dp->tFogChangeMask 			|= STATE_FOG_MODE;
		}
		break;
	
	case kQATag_Blend:
		dp->tState[tag].i = newValue;
		dp->tChangeMask |= STATE_BLEND;
		if (newValue == kQABlend_OpenGL)
			dp->tChangeMask |= STATE_GL_BLEND_MODE;
		DebugStr("Blend ");
		break;

	case kQATag_TextureFilter:
		dp->tState[tag].i = newValue;
		dp->tChangeMask |= STATE_TEXTURE;
		dp->tTex1ChangeMask |= STATE_TEX_FILTER;
		DebugStr("TextureFilter: ");
		break;

	case kQATag_TextureOp:
		DebugStr("TextureOp: ");
		if (newValue & kQATextureOp_Modulate) 
			DebugStr("Modulate ");
		if (newValue & kQATextureOp_Highlight)
			DebugStr("Highlight ");
		if (newValue & kQATextureOp_Decal) 
			DebugStr("Decal ");
		if (newValue & kQATextureOp_Shrink) 
			DebugStr("Shrink ");
		
    	if( newValue == kQATextureOp_None )
    	{
    		dp->tState[tag].i = newValue;
 			DebugStr("kQATextureOp_None ");
	   	}
    	else if( !(dp->tState[tag].i & newValue) )
        {
        	dp->tState[tag].i |= newValue;
        }
    	dp->tChangeMask |= STATE_TEXTURE;
    	dp->tTex1ChangeMask |= STATE_TEX_OP;
		break;

	case kQATag_Antialias:
		if(!(NAPALM_AND_BEYOND)) {
			DebugStr("*** Antialias not implemented: ");
			newValue = kQAAntiAlias_Off;
		}
		
		switch(newValue) 
		{
		case kQAAntiAlias_Off:
			DebugStr("Anti Alias Off");
			break;
		case kQAAntiAlias_Fast:
			newValue = kQAAntiAlias_Off;
			DebugStr("Anti Alias Fast");
			break;
		case kQAAntiAlias_Mid:
			newValue = kQAAntiAlias_Off;
			DebugStr("Anti Alias Mid");
			break;
		case kQAAntiAlias_Best:
			DebugStr("Anti Alias Best");
			break;
		default:
			DebugStr("*** unexpected Antialias\n");
			goto bail;
		}
		dp->tState[tag].i = newValue;
		dp->tChangeMask |= STATE_ANTIALIAS;
		break;

	case kQATag_CSGTag:
		DebugStr("*** unsupported tag (CSGTag)");
		goto bail;
		break;
	case kQATag_CSGEquation:
		DebugStr("*** unsupported tag (CSGEquation)");
		goto bail;
		break;
	case kQATag_BufferComposite:
		DebugStr("*** unsupported tag (BufferComposite)");
		goto bail;
		break;


	case kQATag_ChannelMask:
		dp->tState[tag].i = newValue;
		dp->tChangeMask |= STATE_RGB_MASK;
		break;
	
	case kQATag_ZBufferMask:
		DebugStr("ZBufferMask");
		dp->tState[tag].i = newValue;
		dp->tChangeMask |= (STATE_Z_FUNC | STATE_DRAW_DESTINATION | STATE_ALPHASORTING);
		break;
	
	// Checked at every RenderStart & SetRenderState
	case kQATag_ZSortedHint:
	case 1000:		// ATI-specific
		DebugStr("ZSortedHint ");
		dp->tState[kQATag_ZSortedHint].i = newValue;
		dp->tChangeMask			|= STATE_ALPHASORTING;
		break;
	
	case kQATag_ChromakeyEnable:
		DebugStr("ChromakeyEnable ");
		dp->tState[tag].i = newValue;
		dp->tChangeMask |= STATE_CHROMAKEY;
		break;
	
	case kQATag_AlphaTestFunc:
		dp->tState[tag].i = (newValue & 0x07);	// Fly is sending 518,519
		dp->tChangeMask |= STATE_ALPHA_TST_FUNC;
		DebugStr("kQATag_AlphaTestFunc ");
		break;
	
	case kQATag_DontSwap:
		DebugStr(" DontSwap ");
		dp->tState[tag].i = newValue;
		break;
	
	case kQATag_MultiTextureEnable:
		dp->tState[kQATag_MultiTextureEnable].i 	= newValue;
		dp->tChangeMask 				|= STATE_COMPOSITE;
		dp->tTex2ChangeMask 			|= STATE_COMP_MODE;
		DebugStr("kQATag_MultiTextureEnable");
		break;
	
	// In engines with two texture units, this is always zero.
	case kQATag_MultiTextureCurrent:
		dp->tState[kQATag_MultiTextureCurrent].i 	= 0;
		dp->tChangeMask 				|= STATE_COMPOSITE;
		DebugStr("kQATag_MultiTextureCurrent");
		break;
	
	// blend
	case kQATag_MultiTextureOp:
		dp->tState[kQATag_MultiTextureOp].i 	= newValue;
		dp->tChangeMask 				|= STATE_COMPOSITE;
		dp->tTex2ChangeMask 			|= STATE_COMP_TEX_OP;
		DebugStr("kQATag_MultiTextureOp");
		break;
	
	case kQATag_MultiTextureFilter:
		dp->tState[kQATag_MultiTextureFilter].i 	= newValue;
		dp->tChangeMask 				|= STATE_COMPOSITE;
		dp->tTex2ChangeMask 			|= STATE_COMP_TEX_FILTER;
		DebugStr("kQATag_MultiTextureFilter");
		break;
	
	case kQATag_MultiTextureWrapU:
		dp->tState[kQATag_MultiTextureWrapU].i 	= newValue;
		dp->tChangeMask 				|= STATE_COMPOSITE;
		dp->tTex2ChangeMask 			|= STATE_COMP_TEX_WRAPU;
		DebugStr("kQATag_MultiTextureWrapU");
		break;
	
	
	case kQATag_MultiTextureWrapV:
		dp->tState[kQATag_MultiTextureWrapV].i 	= newValue;
		dp->tChangeMask 				|= STATE_COMPOSITE;
		dp->tTex2ChangeMask 			|= STATE_COMP_TEX_WRAPV;
		DebugStr("kQATag_MultiTextureWrapV");
		break;
	
	
	case kQATag_MultiTextureMagFilter:
		dp->tState[kQATag_MultiTextureMagFilter].i 	= newValue;
		dp->tChangeMask 				|= STATE_COMPOSITE;
		dp->tTex2ChangeMask 			|= STATE_COMP_TEX_MAG;
		DebugStr("kQATag_MultiTextureMagFilter");
		break;
	
	
	case kQATag_MultiTextureMinFilter:
		dp->tState[kQATag_MultiTextureMinFilter].i 	= newValue;
		dp->tChangeMask 				|= STATE_COMPOSITE;
		dp->tTex2ChangeMask 			|= STATE_COMP_TEX_MIN;
		DebugStr("kQATag_MultiTextureMinFilter");
		break;
	
	
	case kQATag_BitmapFilter:

	case kQATag_DrawContextFilter:
		dp->tState[tag].i = newValue;
		break;


	case kQATagGL_TextureWrapU:
		dp->tState[tag].i = newValue;
		dp->tChangeMask 				|= STATE_TEXTURE;
		dp->tTex1ChangeMask				|= STATE_TEX_WRAPU;
		DebugStr("kQATagGL_TextureWrapU");
		break;
		
	case kQATagGL_TextureWrapV:
		dp->tState[tag].i = newValue;
		dp->tChangeMask 				|= STATE_TEXTURE;
		dp->tTex1ChangeMask				|= STATE_TEX_WRAPV;
		DebugStr("kQATagGL_TextureWrapV");
	
		break;

	case kQATagGL_TextureMagFilter:
		dp->tState[tag].i = newValue;
		dp->tChangeMask 				|= STATE_TEXTURE;
		dp->tTex1ChangeMask				|= ( STATE_TEX_FILTER | STATE_TEX_MAG );
		DebugStr("kQATagGL_TextureMagFilter");
		break;
	
	case kQATagGL_TextureMinFilter:
		dp->tState[tag].i = newValue;
		dp->tChangeMask 				|= STATE_TEXTURE;
		dp->tTex1ChangeMask				|= ( STATE_TEX_FILTER | STATE_TEX_MIN );
		DebugStr("kQATagGL_TextureMinFilter");
		break;


	case kQATagGL_BlendSrc:
		dp->tState[tag].i = newValue;
		dp->tChangeMask |= STATE_GL_BLEND_MODE;
		DebugStr("Blend Src "); DebugNum( newValue );
		break;
	
	case kQATagGL_BlendDst:
		dp->tState[tag].i = newValue;
		dp->tChangeMask |= STATE_GL_BLEND_MODE;
		DebugStr("Blend Dst"); DebugNum( newValue );
		break;
		

// for more opengl code, see OGLsetint.c
	default:
#if TNSL_GL
		if((tag >= kQATagGL_AreaPattern0) && (tag <= kQATagGL_AreaPattern31)) 
		{
			dp->tState[tag].i = newValue;
			DebugStr("*** unsupported tag (OpenGL area pattern): ");
			goto bail;
		}
#endif /* TNSL_GL */

		DebugStr("*** unexpected tag");
		goto bail;
	}

bail:
	DebugStr("\n");
	
	DrvTimerEnd();
	AppTimerStart();
	
	return;
}


//////////////////
//              //
//  RvSetPtr  //
//              //
//////////////////

void 
RvSetPtr(TQADrawContext* drawContext,
					 TQATagPtr       tag,
					 const void*     newValue)
{
	TQADrawPrivate* dp = drawContext->drawPrivate;
	
	AppTimerEnd();
	TexTimerStart();
	
	DebugStr("--> RvSetPtr - ");

	if ( tag <= kRaveMaxTag && dp->tState[tag].p == newValue )	// nothing to do
	{
		DebugStr(" redundant call\n" );
		return;
	}

	assert( tag < kQATag_EngineSpecific_Minimum );

	switch(tag) {
	case kQATag_Texture:
		DebugStr("Tag Texture ");
		DebugHex((unsigned long)newValue);
	
		dp->tState[kQATag_Texture].p = newValue;
		if(!newValue) 
		{
			DebugStr("NONE");
			dp->currTriType = TNSL_NONE;
			goto bail;
		}
		
		dp->tChangeMask |= (STATE_TEXTURE | STATE_ALPHA_TST | STATE_ALPHA_TST_FUNC);
		dp->tTex1ChangeMask |= (STATE_PRIMARY_TEX | STATE_TEX_OP);  
		break;

    case kQATag_MultiTexture:
		dp->tState[kQATag_MultiTexture].p 	= newValue;
     	dp->tChangeMask 					|= STATE_COMPOSITE;
		dp->tTex2ChangeMask 				|= STATE_COMP_TEX;
		break;
			
	default:
		DebugStr("*** unexpected tag");
		goto bail;
	}

bail:
	DebugStr("\n");
	
	TexTimerEnd();
	AppTimerStart();
	
	return;
}


////////////////////
//                //
//  RvGetFloat  //
//                //
////////////////////

float RvGetFloat(const TQADrawContext* drawContext, TQATagFloat tag)
{
	assert( tag <= kRaveMaxTag );
	return ( drawContext->drawPrivate->tState[tag].f );
}


//////////////////
//              //
//  RvGetInt  //
//              //
//////////////////

unsigned long RvGetInt(const TQADrawContext* drawContext, TQATagInt tag)
{
	TQADrawPrivate *dp = drawContext->drawPrivate;
	
	// Handle ATI-specific tags as best as possible
	if ( tag > kRaveMaxTag )
	{
		switch (tag) {
			case 1002:	
				switch (dp->tState[kQATag_FogMode].i) {
					case 1: 	return 3;
					case 2:		return 4;
					case 3: 	return 1;
					case 4: 	return 2;
					default:	return 0;
				}
				break;
			case 1000:		tag = kQATag_ZSortedHint; break;
			// ATI chip id; fake it with Rage128
			case 1011:			return 0x500;	break;
			default: DebugStr("? GetInt Unknown tag"); DebugNum(tag); DebugStr("\n");
		}
	}
			
	return ( dp->tState[tag].i );
}


//////////////////
//              //
//  RvGetPtr  //
//              //
//////////////////

void* RvGetPtr(const TQADrawContext* drawContext, TQATagPtr tag)
{
	assert( tag <= kRaveMaxTag );
	return ( (void *)drawContext->drawPrivate->tState[tag].p );
}

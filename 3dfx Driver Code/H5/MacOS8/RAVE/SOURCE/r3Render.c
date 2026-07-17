//: r3Render.c
//: alt.drivers inc.
//: Glenn Nissen
//: Vadim Kochubievski
//: Rohit Gundecha


#include "r3Tweaks.h"
#include "r3Core.h"
#include "r3VertexMacros.h"

extern TRvInfo gRvEngInfo;
extern Boolean bQuakeRunning;

static UInt32 gMyFrameCount = 0;

#if TNSL_PROFILING
// This is a wrapper for Profiling.

static void WrapFSBIClear( TQADrawPrivate *dp)
{
		if (!(dp->flags & kQAContext_NoZBuffer))
		{
			DevTimerStart();
			grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
			{
					// only clear the z-buffer
				grColorMask(FXFALSE, FXFALSE);
				
				grBufferClear(CONVERT_COLOR(dp->tState[kQATag_ColorBG_r].f, dp->tState[kQATag_ColorBG_g].f, 
						dp->tState[kQATag_ColorBG_b].f), 
                       0, 0xFFFF);
				grColorMask(FXTRUE, FXFALSE);
				
				if (dp->tState[kQATag_ZFunction].i == kQAZFunction_None) grDepthMask(FXFALSE);
			}
			DevTimerEnd();
		}

}

static void WrapDoLocked( TQADrawContext *drawContext, GrLfbInfo_t *lfb, const TQARect *dirtyRect )
{
	TQADevice device;
	TQADrawPrivate *dp = drawContext->drawPrivate;
	
			device.deviceType                    = kQADeviceMemory;
			device.device.memoryDevice.pixelType = dp->targetPixelBits == 16 ? kQAPixel_ARGB16 : kQAPixel_ARGB32;
			device.device.memoryDevice.rowBytes  = (long)(*lfb).strideInBytes; 
			device.device.memoryDevice.width     = (long)dp->screenWidth;
			device.device.memoryDevice.height    = dp->screenHeight;
			device.device.memoryDevice.baseAddr  = (*lfb).lfbPtr;
			
			DebugStr(" rowbytes="); DebugNum( device.device.memoryDevice.rowBytes );
			DebugStr(" pixelType="); DebugNum( device.device.memoryDevice.pixelType );
			DebugStr(" baseAddr="); DebugHex( device.device.memoryDevice.baseAddr ); DebugStr("\n");
		
			DrvTimerEnd();
			AppTimerStart();
			(*dp->completionCallBack[kQAMethod_ImageBufferInitialize].bufferNoticeMethod)(
				drawContext, &device, dirtyRect, dp->callBackRefCon[kQAMethod_ImageBufferInitialize]);
			grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);

			if (!NAPALM_AND_BEYOND && dp->targetPixelBits == 16)
			{
				GrVertex verts[4];
				DebugStr(" fullscreen 565 hack ");
				grDisableAllEffects();
				grDitherMode(GR_DITHER_DISABLE);
				grColorCombine(
					GR_COMBINE_FUNCTION_LOCAL, 
					GR_COMBINE_FACTOR_NONE,
					GR_COMBINE_LOCAL_ITERATED, 
					GR_COMBINE_OTHER_NONE, 
					FXFALSE
					);
					
				grAlphaBlendFunction(GR_BLEND_DST_COLOR, GR_BLEND_ZERO, GR_BLEND_ZERO, GR_BLEND_ZERO);
				grAlphaTestFunction(GR_CMP_ALWAYS);
					
				verts[0].x = 0;
				verts[0].y = 0;
				verts[0].argb = 0xffffff7f;
				
				verts[1].x = dp->renderPort->portRect.right;
				verts[1].y = 0;
				verts[1].argb = 0xffffff7f;
				
				verts[2].x = dp->renderPort->portRect.right;
				verts[2].y = dp->renderPort->portRect.bottom;
				verts[2].argb = 0xffffff7f;
				
				verts[3].x = 0;
				verts[3].y = dp->renderPort->portRect.bottom;
				verts[3].argb = 0xffffff7f;
				
				grDrawTriangle(verts, verts + 1, verts + 2);
				grDrawTriangle(verts, verts + 2, verts + 3);
				
				grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
			}

}

static void WrapFSBufferInitCallback( TQADrawContext *drawContext, const TQARect *dirtyRect )
{
	TQADrawPrivate *dp = drawContext->drawPrivate;
		
		WrapFSBIClear(dp);
		
		DebugStr("\ndoing fullscreen kQAMethod_ImageBufferInitialize fullscreen \n");
		{
			GrLfbInfo_t LfbInfo;
			FxBool	bLocked;
			GrLfbWriteMode_t theWriteMode;
			
			if (dp->targetPixelBits == 32)
				theWriteMode = GR_LFBWRITEMODE_8888;
			else
			{
				if (NAPALM_AND_BEYOND)
					theWriteMode = GR_LFBWRITEMODE_1555;
				else
					theWriteMode = GR_LFBWRITEMODE_565;
			}
					
			DebugStr(" theWriteMode = "); DebugNum(theWriteMode);
				
			LfbInfo.size = sizeof (GrLfbInfo_t);
			bLocked = grLfbLock((GR_LFB_WRITE_ONLY | GR_LFB_NOIDLE), GR_BUFFER_BACKBUFFER, theWriteMode,
							                 GR_ORIGIN_ANY, FXFALSE, &LfbInfo);
			if (bLocked)
			{
				WrapDoLocked( (TQADrawContext *)drawContext, &LfbInfo, dirtyRect );
			}
			AppTimerEnd();
		}
			
}
#endif

#if TNSL_PROFILING
// This is a wrapper for Profiling.
static void WrapFinish( void )
{
			grFinish();
}
#endif

#pragma mark -
#pragma mark /*   RAVE 1.6 functions */
///////////////////////
//                   //
//  RvRenderStart  //
//                   //
///////////////////////

void 
RvRenderStart(const TQADrawContext* drawContext,
                const TQARect*        dirtyRect,
                const TQADrawContext* initialContext)
{
	TQADrawPrivate* dp;
	

	MemoryCheck();
	AppTimerEnd();
	DrvTimerStart();

	DebugStr("\n--> RvRenderStart - RENDER START **********************************");
#ifdef TNSL_TIMER
	DebugNum(gRvEngInfo.drvTimer.frameCount);
#endif

	assert( drawContext );
	assert( dirtyRect == NULL );
	dp = drawContext->drawPrivate;
	// dp->bBetweenRStartAndREnd = true;

	DebugStr( "\n dp ptr = "); DebugHex(dp); DebugStr(" Ctx ID= "); DebugNum(dp->ctxID);
	
	if (initialContext) 	// TBFL
	{
		DebugStr("*** ? bailing because initial context");
		assert(0);
		goto bail;
	}
	
 #ifdef TNSL_LOG
	{
		long tempMem, sysMem, mem;
		tempMem = TempFreeMem();
		mem = FreeMem();
		sysMem = FreeMemSys();
		DebugStr(" freeMem="); DebugNum(mem);
		DebugStr(" freeMemSys="); DebugNum(sysMem);
		DebugStr(" tempMem="); DebugNum(tempMem);
	}
#endif
	DebugStr("\n");

	if (gRvEngInfo.currentDrawContext != drawContext)
	{
		grFinish();
		DebugStr(" Switching into new draw context ... ");
		/* Docs are not clear whether grSelectContext works on a windowed context.
		 * TBFL
		 */
		if (dp->bFullScreen)
		{
			// grSelectContext actives a Glide context
			if (!grSelectContext( dp->glideContext ))
			{
				FreeAllTexturesVRAM();
				DebugStr(" error in grSelectContext, RvRenderAbort ");
			}
			DebugStr("grSelectContext ");
		}
		else	// windowed
		{
			if (dp->renderSurface)
				grSurfaceSetRenderingSurfaceExt( dp->renderSurface );
			if (dp->auxSurface)
				grSurfaceSetAuxSurfaceExt( dp->auxSurface );
			DebugStr("windowed Select Context\n");
		}
		// inform globals that this is the current context
		gRvEngInfo.currentDrawContext = (TQADrawContext *)drawContext;
		ResetContextStateFlags((TQADrawContext *)drawContext);
	}
	
	// RenderAbort frees this, so we might need to reallocate
	if (dp->pAlphaGlideStates == NULL)
	{
#if CUSTOM_GLIDE_STATE_DEF
		dp->pCurrAlphaGlideState = dp->pAlphaGlideStates = (TRaveRenderGlideState *)AllocPtr( dp->maxCachedStates * sizeof(TRaveRenderGlideState) );
#else
		dp->pCurrAlphaGlideState = dp->pAlphaGlideStates = (void *)AllocPtr( dp->maxCachedStates * gRvEngInfo.sizeOfGlideState );
		DebugStr("\n reallocated dp->pAlphaGlideStates\n");
#endif
	}
	
	++gMyFrameCount;
	
	
	if (dp->completionCallBack[kQAMethod_ImageBufferInitialize].bufferNoticeMethod) 
	{
		assert( (!(dp->flags & kQAContext_NoZBuffer) && (( dp->bFullScreen ) || dp->auxSurface)) ||
				(dp->flags & kQAContext_NoZBuffer));
		
		// handle z buffer clear, if there is a z buffer, but don't clear the color buffers,
		// because that's what the callback does.
		if (!(dp->flags & kQAContext_NoZBuffer))
		{
			grDepthBufferMode( GR_DEPTHBUFFER_ZBUFFER );
			grDepthMask(FXTRUE);
			grColorMask(FXFALSE, FXFALSE);
			grBufferClear(CONVERT_COLOR(dp->tState[kQATag_ColorBG_r].f, dp->tState[kQATag_ColorBG_g].f, 
							dp->tState[kQATag_ColorBG_b].f),  0, 0xFFFF);
			grColorMask(FXTRUE, FXFALSE);
			
			if (dp->tState[kQATag_ZFunction].i == kQAZFunction_None)
			{
				grDepthMask(FXFALSE);
			}
			DebugStr("\nZ Buffer Cleared, but not Draw Buffer\n");
		}
	
		/*
		 * FullScreen
		 */
		 
		if (dp->bFullScreen)
		{
			// handle image buffer initialize
			GrLfbInfo_t LfbInfo;
			FxBool	bLocked;
			GrLfbWriteMode_t theWriteMode;
			
			DebugStr("\ndoing fullscreen kQAMethod_ImageBufferInitialize fullscreen; dp->z_bits =");
			if (NAPALM_AND_BEYOND)
			{
				if (dp->targetPixelBits == 32)
					theWriteMode = GR_LFBWRITEMODE_8888;
				else
					theWriteMode = GR_LFBWRITEMODE_1555;
			}
			else
			{
					theWriteMode = GR_LFBWRITEMODE_565;
			}				
				
			DebugStr(" theWriteMode = "); DebugNum(theWriteMode);
				
			LfbInfo.size = sizeof (GrLfbInfo_t);
			bLocked = grLfbLock((GR_LFB_WRITE_ONLY | GR_LFB_NOIDLE), GR_BUFFER_BACKBUFFER, theWriteMode,
							                 GR_ORIGIN_ANY, FXFALSE, &LfbInfo);
			if (bLocked)
			{
				TQADevice device;
				device.deviceType                    = kQADeviceMemory;
				device.device.memoryDevice.pixelType = dp->targetPixelBits == 16 ? kQAPixel_ARGB16 : kQAPixel_ARGB32;
				device.device.memoryDevice.rowBytes  = (long)LfbInfo.strideInBytes; 
				device.device.memoryDevice.width     = dp->screenWidth;
				device.device.memoryDevice.height    = dp->screenHeight;
				device.device.memoryDevice.baseAddr  = LfbInfo.lfbPtr;
				
				DebugStr(" rowbytes="); DebugNum( device.device.memoryDevice.rowBytes );
				DebugStr(" pixelType="); DebugNum( device.device.memoryDevice.pixelType );
				DebugStr(" baseAddr="); DebugHex( device.device.memoryDevice.baseAddr ); DebugStr("\n");
			
				DrvTimerEnd();
				AppTimerStart();
				(*dp->completionCallBack[kQAMethod_ImageBufferInitialize].bufferNoticeMethod)(
					drawContext, &device, dirtyRect, dp->callBackRefCon[kQAMethod_ImageBufferInitialize]);
				grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);
			}
			AppTimerEnd();
		}
		/*
		 * Windowed
		 */
		else
		{
			TQADevice device;
			DebugStr("\ndoing surface kQAMethod_ImageBufferInitialize\n");
		
			device.deviceType                    = kQADeviceMemory;
			device.device.memoryDevice.rowBytes  = dp->renderSurfaceDesc.pitch; 
			device.device.memoryDevice.pixelType = dp->targetPixelBits == 16 ? kQAPixel_ARGB16 : kQAPixel_ARGB32;
			device.device.memoryDevice.width     = dp->renderSurfaceDesc.width;	
			device.device.memoryDevice.height    = dp->renderSurfaceDesc.height; 
			device.device.memoryDevice.baseAddr  = (FxU8 *)dp->renderSurfaceDesc.surface;
			
			DebugStr(" rowbytes="); DebugNum( dp->renderSurfaceDesc.pitch );
			DebugStr(" pixelType="); DebugNum( device.device.memoryDevice.pixelType );
			DebugStr(" baseAddr="); DebugHex( dp->renderSurfaceDesc.surface ); DebugStr("\n");
		
			DrvTimerEnd();
			AppTimerStart();
			grDepthMask( FXFALSE );
			dp->tChangeMask |= STATE_Z_FUNC;
			// temp 9/14 grDepthBufferMode( GR_DEPTHBUFFER_DISABLE );
			(*dp->completionCallBack[kQAMethod_ImageBufferInitialize].bufferNoticeMethod)(
				drawContext, &device, dirtyRect, dp->callBackRefCon[kQAMethod_ImageBufferInitialize]);
		}
		if (!(dp->flags & kQAContext_NoZBuffer))
			grDepthMask( FXTRUE );
		if (!NAPALM_AND_BEYOND && dp->targetPixelBits == 16)
		{
			GrVertex verts[4];
			grDisableAllEffects();
			grDitherMode(GR_DITHER_DISABLE);
			grColorCombine(
				GR_COMBINE_FUNCTION_LOCAL, 
				GR_COMBINE_FACTOR_NONE,
				GR_COMBINE_LOCAL_ITERATED, 
				GR_COMBINE_OTHER_NONE, 
				FXFALSE
				);
				
			grAlphaBlendFunction(GR_BLEND_DST_COLOR, GR_BLEND_ZERO, GR_BLEND_ZERO, GR_BLEND_ZERO);
			grAlphaTestFunction(GR_CMP_ALWAYS);
				
			verts[0].x = 0;
			verts[0].y = 0;
			verts[0].argb = 0xffffff7f;
			
			verts[1].x = dp->renderPort->portRect.right;
			verts[1].y = 0;
			verts[1].argb = 0xffffff7f;
			
			verts[2].x = dp->renderPort->portRect.right;
			verts[2].y = dp->renderPort->portRect.bottom;
			verts[2].argb = 0xffffff7f;
			
			verts[3].x = 0;
			verts[3].y = dp->renderPort->portRect.bottom;
			verts[3].argb = 0xffffff7f;
			
			grDrawTriangle(verts, verts + 1, verts + 2);
			grDrawTriangle(verts, verts + 2, verts + 3);
			
			// temp 9/15 grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
		}
	}
	else		// no callback
	{
		// clear color and z buffers
		// grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);	// temp
		grDepthMask(FXTRUE);
			
		grBufferClear(CONVERT_COLOR(dp->tState[kQATag_ColorBG_r].f, dp->tState[kQATag_ColorBG_g].f, 
				dp->tState[kQATag_ColorBG_b].f),  0, 0xFFFF);
		if ((dp->flags & kQAContext_NoZBuffer)) grDepthMask(FXFALSE);
		
		DebugStr("Color Buffer cleared; Z buffer cleared to "); DebugHex(0xFFFF); DebugStr("\n");
	}
	
	// tex surf might have been created after Glide ctx was created.
	if (gRvEngInfo.texture_surface)
	{
		grSurfaceSetTextureSurfaceExt( GR_TMU0, gRvEngInfo.texture_surface );
		grSurfaceSetTextureSurfaceExt( GR_TMU1, gRvEngInfo.texture_surface );
		ResetContextStateFlags( (TQADrawContext *)drawContext );
	}
	dp->bStateInCacheChanged 	= true;	
	dp->currTriType 			= TNSL_NONE;
	SetGlide_Z(dp);
	SetAlphaSorting( dp );  
    DebugStr("Tri Cache "); DebugNum( dp->bSortingAlphaTri );
	// since we mucked with z buffer settings
	dp->tChangeMask |= STATE_Z_FUNC;  
	    
bail:
	AppTimerStart();
	return;
}

/////////////////////
//                 //
//  RvRenderEnd  //
//                 //
/////////////////////

TQAError 
RvRenderEnd(const TQADrawContext* drawContext,
              const TQARect*        modifiedRect)
{
	const static RGBColor rgbBlack = { 0x0000, 0x0000, 0x0000 };
	const static RGBColor rgbWhite = { 0xFFFF, 0xFFFF, 0xFFFF };
	TQADrawPrivate* dp = drawContext->drawPrivate;
	
	MemoryCheck();
	DebugStr("\n--> RvRenderEnd!!! RENDER END ****************************** - \n");
	DebugStr("RenderEnd. Frame: ");
	DebugNum( gMyFrameCount );
	
	
#ifdef TNSL_TIMER
  {
    AbsoluteTime absTmp1;
    Duration durTotal;
    Duration durDev;
    Duration durDrv;
    Duration durApp;
    Duration durTex;
    float perDev;
    float perDrv;
    float perApp;
    float perTex;
    UInt32 secondChanged = 0;

    DebugNum(gRvEngInfo.drvTimer.frameCount);

    AppTimerEnd();
    DrvTimerStart();
	
    gRvEngInfo.drvTimer.frameCount++;
    gRvEngInfo.drvTimer.frames++;
    gRvEngInfo.drvTimer.numFrame++;
    gRvEngInfo.drvTimer.absTotal = gRvEngInfo.drvTimer.absEnd;
    gRvEngInfo.drvTimer.absEnd   = UpTime();
    gRvEngInfo.drvTimer.absTotal = SubAbsoluteFromAbsolute(gRvEngInfo.drvTimer.absEnd, 
                                                          gRvEngInfo.drvTimer.absTotal);
    absTmp1 = SubAbsoluteFromAbsolute(gRvEngInfo.drvTimer.absEnd, gRvEngInfo.drvTimer.absStart);
	
    durTotal = AbsoluteToDuration(absTmp1);
    if((durTotal >= 1000) || (durTotal <= -1000000)) {
      gRvEngInfo.drvTimer.fps = gRvEngInfo.drvTimer.numFrame;
      gRvEngInfo.drvTimer.numFrame = 0;
      gRvEngInfo.drvTimer.absStart = gRvEngInfo.drvTimer.absEnd;
      gRvEngInfo.drvTimer.seconds++;
      secondChanged = 1;
    }
	
    txtDisplayLabel(dp->screenWidth, dp->screenHeight);
	
    txtDisplayNum(gRvEngInfo.drvTimer.fps,         dp->screenWidth - 24, dp->screenHeight - 8, 3);
    txtDisplayNum(gRvEngInfo.drvTimer.numTriangle, dp->screenWidth - 60, dp->screenHeight - 8, 5);
    txtDisplayNum(gRvEngInfo.drvTimer.numVertex,   dp->screenWidth - 96, dp->screenHeight - 8, 5);
	
    txtDisplayNum(gRvEngInfo.pList->textureCount, dp->screenWidth / 2 - 42, dp->screenHeight - 8, 4);
    txtDisplayNum(gRvEngInfo.pList->textureTotal, dp->screenWidth / 2 - 12, dp->screenHeight - 8, 9);
    txtDisplayNum(gRvEngInfo.drvTimer.frameCount, dp->screenWidth / 2, 4, 5);

    durTotal = AbsoluteToDuration(gRvEngInfo.drvTimer.absTotal);
    durDev   = AbsoluteToDuration(gRvEngInfo.drvTimer.absDev);
    durDrv   = AbsoluteToDuration(gRvEngInfo.drvTimer.absDrv);
    durApp   = AbsoluteToDuration(gRvEngInfo.drvTimer.absApp);
    durTex   = AbsoluteToDuration(gRvEngInfo.drvTimer.absTex);

    perDev = (float) (durDev * 100) / durTotal;
    perDrv = (float) ((durDrv - durDev) * 100) / durTotal;
    perApp = (float) (durApp * 100) / durTotal;
    perTex = (float) (durTex * 100) / durTotal;
	
    gRvEngInfo.drvTimer.perDev1 += perDev;
    gRvEngInfo.drvTimer.perDrv1 += perDrv;
    gRvEngInfo.drvTimer.perApp1 += perApp;
    gRvEngInfo.drvTimer.perTex1 += perTex;

    if(secondChanged && (!(gRvEngInfo.drvTimer.seconds % 10))) {
      gRvEngInfo.drvTimer.perDev2 = gRvEngInfo.drvTimer.perDev1 / gRvEngInfo.drvTimer.frames;
      gRvEngInfo.drvTimer.perDrv2 = gRvEngInfo.drvTimer.perDrv1 / gRvEngInfo.drvTimer.frames;
      gRvEngInfo.drvTimer.perApp2 = gRvEngInfo.drvTimer.perApp1 / gRvEngInfo.drvTimer.frames;
      gRvEngInfo.drvTimer.perTex2 = gRvEngInfo.drvTimer.perTex1 / gRvEngInfo.drvTimer.frames;
      gRvEngInfo.drvTimer.perDev1 = 0.0f;
      gRvEngInfo.drvTimer.perDrv1 = 0.0f;
      gRvEngInfo.drvTimer.perApp1 = 0.0f;
      gRvEngInfo.drvTimer.perTex1 = 0.0f;
      gRvEngInfo.drvTimer.frames = 0;
    }
	
    txtDisplayNum((UInt32) perDev,  6, dp->screenHeight - 8, 2);
    txtDisplayNum((UInt32) perDrv, 24, dp->screenHeight - 8, 2);
    txtDisplayNum((UInt32) perApp, 42, dp->screenHeight - 8, 2);
    txtDisplayNum((UInt32) perTex, 60, dp->screenHeight - 8, 2);
	
    txtDisplayNum((UInt32) gRvEngInfo.drvTimer.perDev2,  6, dp->screenHeight - 15, 2);
    txtDisplayNum((UInt32) gRvEngInfo.drvTimer.perDrv2, 24, dp->screenHeight - 15, 2);
    txtDisplayNum((UInt32) gRvEngInfo.drvTimer.perApp2, 42, dp->screenHeight - 15, 2);
    txtDisplayNum((UInt32) gRvEngInfo.drvTimer.perTex2, 60, dp->screenHeight - 15, 2);
	
    gRvEngInfo.drvTimer.absDev.hi = 0;
    gRvEngInfo.drvTimer.absDev.lo = 0;
    gRvEngInfo.drvTimer.absDrv.hi = 0;
    gRvEngInfo.drvTimer.absDrv.lo = 0;
    gRvEngInfo.drvTimer.absApp.hi = 0;
    gRvEngInfo.drvTimer.absApp.lo = 0;
    gRvEngInfo.drvTimer.absTex.hi = 0;
    gRvEngInfo.drvTimer.absTex.lo = 0;
	
    gRvEngInfo.drvTimer.numTriangle = 0;
    gRvEngInfo.drvTimer.numVertex   = 0;
  }
#endif
	


	DevTimerStart();
	// Flush the cache of transparent triangles, if any, with z-buffer write disabled.
	FlushTriCache( (TQADrawContext *)drawContext );

	if(!dp->bFullScreen && dp->targetPort) 
	{	
		Rect srcSwapRect, dstSwapRect;
		GrVertex verts[4];
		
		DrvTimerEnd();
		AppTimerStart();
		
		if ( dp->completionCallBack[kQAMethod_ImageBuffer2DComposite].bufferNoticeMethod ) 
		{
			TQADevice device;
			
			DebugStr("\ndoing surface kQAMethod_ImageBuffer2DComposite\n");
	
			grFinish();
			device.deviceType                    = kQADeviceMemory;
			device.device.memoryDevice.rowBytes  = dp->renderSurfaceDesc.pitch;
			device.device.memoryDevice.pixelType = dp->targetPixelBits == 16 ? kQAPixel_ARGB16 : kQAPixel_ARGB32;
			device.device.memoryDevice.width     = dp->renderSurfaceDesc.width;
			device.device.memoryDevice.height    = dp->renderSurfaceDesc.height;
			device.device.memoryDevice.baseAddr  = (FxU8 *)dp->renderSurfaceDesc.surface;
			
			(*dp->completionCallBack[kQAMethod_ImageBuffer2DComposite].bufferNoticeMethod)
			      (drawContext, 
			       &device, 
			       modifiedRect, 
			       dp->callBackRefCon[kQAMethod_ImageBuffer2DComposite]);
			SetRenderState((TQADrawContext *)drawContext, FXFALSE);
		}
		
		
		AppTimerEnd();
		DrvTimerStart();
		
		dp->bSortingAlphaTri = 0;	// to prevent the 565 hack triangle from being cached.
		
		assert( dp->targetPort != NULL );
		
		srcSwapRect = dp->renderPort->portRect;
		
		if (!NAPALM_AND_BEYOND && dp->targetPixelBits == 16)
		{
			grDisableAllEffects();
			grDitherMode(GR_DITHER_DISABLE);
			grColorCombine(
				GR_COMBINE_FUNCTION_LOCAL, 
				GR_COMBINE_FACTOR_NONE,
				GR_COMBINE_LOCAL_ITERATED, 
				GR_COMBINE_OTHER_NONE, 
				FXFALSE
				);
				
			grAlphaBlendFunction(GR_BLEND_DST_COLOR, GR_BLEND_ZERO, GR_BLEND_ZERO, GR_BLEND_ZERO);
			grAlphaTestFunction(GR_CMP_ALWAYS);
				
			verts[0].x = 0;
			verts[0].y = 0;
			verts[0].argb = 0xff7f7fff;
			
			verts[1].x = srcSwapRect.right;
			verts[1].y = 0;
			verts[1].argb = 0xff7f7fff;
			
			verts[2].x = srcSwapRect.right;
			verts[2].y = srcSwapRect.bottom;
			verts[2].argb = 0xff7f7fff;
			
			verts[3].x = 0;
			verts[3].y = srcSwapRect.bottom;
			verts[3].argb = 0xff7f7fff;
			
			grDrawTriangle(verts, verts + 1, verts + 2);
			grDrawTriangle(verts, verts + 2, verts + 3);			
		}
			

		// we are rendering in a windowed or offscreen PixMap, we need to blit
#if TNSL_PROFILING
		WrapFinish();
#else
		grFinish();
#endif
		
		{
			CGrafPtr savePort;
			GDHandle saveGDevice;			
			GDHandle targetGDevice = dp->device.device.gDevice;
			
			GetGWorld(&savePort, &saveGDevice);
			SetGWorld(dp->targetPort, targetGDevice);
							
			dstSwapRect.top = dp->targetRect.top;
			dstSwapRect.right = dp->targetRect.right;
			dstSwapRect.bottom = dp->targetRect.bottom;
			dstSwapRect.left = dp->targetRect.left;
          
          // sluce says don't lock
			assert( dp->renderPort->portPixMap );
			assert( *(dp->device.device.gDevice));
			assert( (**dp->device.device.gDevice).gdPMap );
					CopyBits(	
				  			(const BitMap*)*(dp->renderPort->portPixMap),
							(const BitMap*)*(dp->targetPort->portPixMap),
							&srcSwapRect,
							&dstSwapRect,
							srcCopy | ditherCopy, dp->maskRgn );

			SetGWorld(savePort, saveGDevice);
		}			

	//+ end of new back-buffer to front blit.		
	}	// if targetPort
	
	
	else if (dp->bFullScreen)
	{
		if( dp->completionCallBack[kQAMethod_ImageBuffer2DComposite].bufferNoticeMethod) 
		{
			FxBool	bLocked;
			GrLfbInfo_t LfbInfo;
			TQADevice device;
			GrLfbWriteMode_t theWriteMode;
			
			if (NAPALM_AND_BEYOND)
			{
				if (dp->targetPixelBits == 32)
					theWriteMode = GR_LFBWRITEMODE_8888;
				else
					theWriteMode = GR_LFBWRITEMODE_1555;
			}
			else
			{
					theWriteMode = GR_LFBWRITEMODE_565;
			}				
				
			DebugStr(" theWriteMode = "); DebugNum(theWriteMode);
			
			DebugStr("\ndoing fullscreen kQAMethod_ImageBuffer2DComposite\n");
			grFinish();
			LfbInfo.size = sizeof (GrLfbInfo_t);
			bLocked = grLfbLock((GR_LFB_WRITE_ONLY | GR_LFB_NOIDLE), GR_BUFFER_BACKBUFFER, theWriteMode,
	               GR_ORIGIN_UPPER_LEFT, FXFALSE, &LfbInfo);
			if (bLocked)
			{
				device.deviceType                    = kQADeviceMemory;
				device.device.memoryDevice.rowBytes  = (long)LfbInfo.strideInBytes;
				device.device.memoryDevice.pixelType = dp->targetPixelBits == 16 ? kQAPixel_ARGB16 : kQAPixel_ARGB32;
				device.device.memoryDevice.width     = dp->screenWidth;	//(long)LfbInfo.size;	// 
				device.device.memoryDevice.height    = dp->screenHeight;// 1; 	// 
				device.device.memoryDevice.baseAddr  = LfbInfo.lfbPtr;
				
				DebugStr(" rowbytes="); DebugNum( device.device.memoryDevice.rowBytes );
				DebugStr(" pixelType="); DebugNum( device.device.memoryDevice.pixelType );
				DebugStr(" baseAddr="); DebugHex( device.device.memoryDevice.baseAddr ); DebugStr("\n");
			
				// 9/14 grDepthBufferMode( GR_DEPTHBUFFER_DISABLE );
				grDepthMask( FXFALSE );
				(*dp->completionCallBack[kQAMethod_ImageBuffer2DComposite].bufferNoticeMethod)
					      (drawContext, 
					       &device, 
					       modifiedRect, 
					       dp->callBackRefCon[kQAMethod_ImageBuffer2DComposite]);
				grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);
				if (!NAPALM_AND_BEYOND && dp->targetPixelBits == 16)
				{
					GrVertex verts[4];
					DebugStr(" fullscreen 565 hack ");
					grDisableAllEffects();
					grDitherMode(GR_DITHER_DISABLE);
					grColorCombine(
						GR_COMBINE_FUNCTION_LOCAL, 
						GR_COMBINE_FACTOR_NONE,
						GR_COMBINE_LOCAL_ITERATED, 
						GR_COMBINE_OTHER_NONE, 
						FXFALSE
						);
						
					grAlphaBlendFunction(GR_BLEND_DST_COLOR, GR_BLEND_ZERO, GR_BLEND_ZERO, GR_BLEND_ZERO);
					grAlphaTestFunction(GR_CMP_ALWAYS);
						
					verts[0].x = 0;
					verts[0].y = 0;
					verts[0].argb = 0xffffff7f;
					
					verts[1].x = dp->renderPort->portRect.right;
					verts[1].y = 0;
					verts[1].argb = 0xffffff7f;
					
					verts[2].x = dp->renderPort->portRect.right;
					verts[2].y = dp->renderPort->portRect.bottom;
					verts[2].argb = 0xffffff7f;
					
					verts[3].x = 0;
					verts[3].y = dp->renderPort->portRect.bottom;
					verts[3].argb = 0xffffff7f;
					
					grDrawTriangle(verts, verts + 1, verts + 2);
					grDrawTriangle(verts, verts + 2, verts + 3);
					
					grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
				}
			}
		}
		
		
		if (dp->tState[kQATag_DontSwap].i == 0)							// Diablo2 sets this.
		{
			grBufferSwap(1);
			DebugStr("\n\ngrBufferSwap \n\n");
		}
	}
	
	
	// dp->bBetweenRStartAndREnd = false;

	DevTimerEnd();
	
	DebugStr("\n");
	DrvTimerEnd();
	AppTimerStart();

	if(dp->completionCallBack[kQAMethod_RenderCompletion].standardNoticeMethod) 
	{
		(*dp->completionCallBack[kQAMethod_RenderCompletion].standardNoticeMethod)(
			drawContext, dp->callBackRefCon[kQAMethod_RenderCompletion]);
	}

	return kQANoErr;
}


///////////////////////
//                   //
//  RvRenderAbort  //
//                   //
///////////////////////

TQAError 
RvRenderAbort(const TQADrawContext* drawContext)
{
	TQADrawPrivate *dp = drawContext->drawPrivate;
	
	DebugStr("\n--> RvRenderAbort - ");
	grFlush();
	
	// initialize alpha tri cache vars
	dp->countAlphaTri = 0;
	dp->idxAlphaTriStatesList = 0;
	if (dp->pAlphaGlideStates)
	{
		FreePtr((void *)dp->pAlphaGlideStates);
		dp->pAlphaGlideStates = NULL;
		dp->pCurrAlphaGlideState = dp->pAlphaGlideStates;
	}

	dp->bStateInCacheChanged = false;
	// dp->bBetweenRStartAndREnd = false;

	DebugStr("\n\n");
	return kQAError;
}


/////////////////
//             //
//  RvFlush  //
//             //
/////////////////

TQAError 
RvFlush(const TQADrawContext* drawContext)
{
#pragma unused(drawContext)
	DebugStr("\n--> RvFlush - ");
	grFlush();
	DebugStr("\n");
	return kQANoErr;
}


////////////////
//            //
//  RvSync  //
//            //
////////////////

TQAError 
RvSync(const TQADrawContext* drawContext)
{	
#pragma unused (drawContext)

	DebugStr("\n--> RvSync - ");
	AppTimerEnd();

	DevTimerStart();
	
	grFinish();
	
	DevTimerEnd();

	DebugStr("\n");
	AppTimerStart();
	return kQANoErr;
}

///////////////////
//               //
//   RvBusy    //
//               //
///////////////////
/*
 * limitation: return value is global, not specific to context.
 */
TQABoolean RvBusy( const TQADrawContext *drawContext )
{
	#pragma unused( drawContext )
	
	SInt32	response = 0;
	
	DebugStr( "RvBusy" );
	
	grGet( GR_IS_BUSY, 4, &response );
	return( (TQABoolean)response );
}

//////////////////////////
//           		    //
//   RvSwapBuffer   	//
//               		//
/////////////////////////

TQAError RvSwapBuffers( const TQADrawContext *drawContext, const TQARect *modifiedRect )
{
	#pragma unused( modifiedRect )
	
	TQADrawPrivate *dp = drawContext->drawPrivate;
	
	if (dp->bFullScreen)
	{
		FlushTriCache( (TQADrawContext *)drawContext );
		grBufferSwap(1);
		ResetContextStateFlags((TQADrawContext *)drawContext);
		DebugStr("\n\n Rave 1.6 swapBuffers \n");
		return kQANoErr;
	}
	else
	{	
	return( kQANotSupported );
}
}

//////////////////////////////////
//								//
//    RvClearDrawBuffer()   	//
//								//
//////////////////////////////////
/*
 * TBFL implement rect, initialContext
 */
TQAError RvClearDrawBuffer(const TQADrawContext *drawContext, const TQARect *rect, const TQADrawContext *initialContext)
{
#pragma unused (rect)

	TQADrawPrivate *dp = drawContext->drawPrivate;
	
	DebugStr("\n Rave 1.6 ClearDrawBuffer initialContext="); DebugHex(initialContext);
	DebugStr(" rect->top="); DebugNum(rect->top);
	DebugStr(" rect->left="); DebugNum(rect->left);
	DebugStr(" rect->right="); DebugNum(rect->right);
	DebugStr(" rect->bottom="); DebugNum(rect->bottom);
	if (1 || initialContext == NULL)
	{
		grDepthMask(FXFALSE);
		grBufferClear(CONVERT_COLOR(dp->tState[kQATag_ColorBG_r].f, dp->tState[kQATag_ColorBG_g].f, 
				dp->tState[kQATag_ColorBG_b].f), 0, 0xFFFF);
		if (dp->tState[kQATag_ZFunction].i != kQAZFunction_None) grDepthMask(FXTRUE);
	}
	DebugStr("\n");
	return ( kQANoErr );
	
}

//////////////////////////////////
//								//
//    RvClearZBuffer()   		//
//								//
//////////////////////////////////

/*
 * TBFL implement rect, initialContext
 */
TQAError RvClearZBuffer(const TQADrawContext *drawContext, const TQARect *rect, const TQADrawContext *initialContext)
{
#pragma unused (rect)

	TQADrawPrivate *dp = drawContext->drawPrivate;
	
	DebugStr("\n Rave 1.6 ClearZBuffer initialContext="); DebugHex(initialContext);
	DebugStr(" rect->top="); DebugNum(rect->top);
	DebugStr(" rect->left="); DebugNum(rect->left);
	DebugStr(" rect->right="); DebugNum(rect->right);
	DebugStr(" rect->bottom="); DebugNum(rect->bottom);
	if ( 1 || initialContext == NULL)
	{
		grColorMask( FXFALSE, FXFALSE );		// prevents clearing the color buffer
		grDepthMask( FXTRUE );
		
		grBufferClear(CONVERT_COLOR(dp->tState[kQATag_ColorBG_r].f, dp->tState[kQATag_ColorBG_g].f, 
				dp->tState[kQATag_ColorBG_b].f), 0, 0xFFFF);
		if (dp->tState[kQATag_ZFunction].i == kQAZFunction_None) grDepthMask(FXFALSE);
		grColorMask( FXTRUE, FXFALSE );		
		if (dp->tState[kQATag_ZFunction].i == kQAZFunction_None)
		{
			grDepthMask(FXFALSE);
		}
		DebugStr("Z buffer cleared to 0xFFFF\n"); 
		DebugStr("Z func Glide {7=ALWAYS; 4=GREATER; 1=LESS} = "); DebugNum( dp->glDepthFunc ); DebugStr("\n");
	}
	DebugStr("\n");
	return (kQANoErr);
}



//////////////////////////////////
//								//
//    RvAccessDrawBuffer()   	//
//								//
//////////////////////////////////
//
// This method could be used to copy the rendered image, e.g., for printing.

TQAError RvAccessDrawBuffer(  
	const TQADrawContext	*drawContext,					/* Draw context */
	TQAPixelBuffer			*pixelBuffer )
{
	#pragma unused( drawContext )
	
	FxBool	bLocked;
	GrLfbInfo_t LfbInfo;
	GrLfbWriteMode_t theWriteMode;
	
	TQADrawPrivate *dp = drawContext->drawPrivate;
		
	DebugStr( "-->RvAccessDrawBuffer" );
	
	if( !pixelBuffer )
		return( kQAParamErr );
	grFinish();
	if (!dp->bFullScreen)
	{
		pixelBuffer->rowBytes				= dp->renderSurfaceDesc.pitch;
		pixelBuffer->pixelType				= dp->targetPixelBits == 16 ? kQAPixel_ARGB16 : kQAPixel_ARGB32; // kQAPixel_RGB16 : kQAPixel_RGB32
		pixelBuffer->width     				= dp->renderSurfaceDesc.width;
		pixelBuffer->height    				= dp->renderSurfaceDesc.height;
		pixelBuffer->baseAddr  				= (FxU8 *)dp->renderSurfaceDesc.surface;
		return( kQANoErr );
	}
	else
	{
		if (NAPALM_AND_BEYOND)
		{
			if (dp->targetPixelBits == 32)
				theWriteMode = GR_LFBWRITEMODE_8888;
			else
				theWriteMode = GR_LFBWRITEMODE_1555;
		}
		else
		{
				theWriteMode = GR_LFBWRITEMODE_565;
		}				
				
		DebugStr(" theWriteMode = "); DebugNum(theWriteMode);
		
		DebugStr("\ndoing fullscreen RvAccessDrawBuffer\n");
		grFinish();
		LfbInfo.size = sizeof (GrLfbInfo_t);
		bLocked = grLfbLock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER, theWriteMode,
               GR_ORIGIN_UPPER_LEFT, FXFALSE, &LfbInfo);
		if (bLocked)
		{
			pixelBuffer->rowBytes		= (long)LfbInfo.strideInBytes;
			pixelBuffer->pixelType		= dp->targetPixelBits == 16 ? kQAPixel_ARGB16 : kQAPixel_ARGB32;
			pixelBuffer->width     		= dp->screenWidth;
			pixelBuffer->height    		= dp->screenHeight;
			pixelBuffer->baseAddr  		= LfbInfo.lfbPtr;
			
			dp->tChangeMask |= STATE_Z_FUNC;
			return( kQANoErr );
		}
		else
			return( kQAError );
	}
}

////////////////////////////////////
//								  //
//    RvAccessDrawBufferEnd()   //
//								  //
////////////////////////////////////

TQAError RvAccessDrawBufferEnd(  
	const TQADrawContext	*drawContext,					/* Draw context */
	const TQARect			*dirtyRect  )
{
	#pragma unused( dirtyRect )
	TQADrawPrivate *dp = drawContext->drawPrivate;
	
	DebugStr( "RvAccessDrawBufferEnd" );
	if (dp->bFullScreen)
	{
		grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);
	}
	return( kQANoErr );	
}

///////////////////////////////
//							//
//    RvAccessZBuffer()   //
//							//
//////////////////////////////

TQAError RvAccessZBuffer( 
	const TQADrawContext 	*drawContext, 
	TQAZBuffer 				*zBuffer)
{
	#pragma unused( drawContext )
	
	TQADrawPrivate *dp = drawContext->drawPrivate;
		
	DebugStr( "RvAccessZBuffer" );
	
	if( !zBuffer )
		return( kQAParamErr );

	if (!dp->bFullScreen)
	{
	
		grFinish();
		zBuffer->width     				= dp->auxSurfaceDesc.width;
		zBuffer->height    				= dp->auxSurfaceDesc.height;
		zBuffer->rowBytes				= dp->auxSurfaceDesc.pitch;
		zBuffer->zbuffer  				= (FxU8 *)dp->auxSurfaceDesc.surface;
		zBuffer->zDepth					= dp->auxSurfaceDesc.bytesPerPixel * 8;
		// not sure
		zBuffer->isBigEndian			= true;
	
		return( kQANoErr );
	}
	else
		return( kQAError );
}

//////////////////////////////////
//								//
//    RvAccessZBufferEnd()    //
//								//
/////////////////////////////////

TQAError RvAccessZBufferEnd(
	const TQADrawContext 	*drawContext, 
	const TQARect 			*dirtyRect)
{
	#pragma unused( drawContext )
	#pragma unused( dirtyRect )
	
	DebugStr( "RvAccessZBufferEnd" );

	return( kQANoErr );
}



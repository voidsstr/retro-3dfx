//: r3Private.c
//: alt.drivers inc.
//: Glenn Nissen
//: Vadim Kochubievski
//: Rohit Gundecha


#include "r3Core.h"
#include "r3Tweaks.h"
#include "r3Context.h"
#include <GLIConstants.h>
#include <gl.h>
#include <Profiler.h>

UInt32 			rvDeviceListCount = 0;
static unsigned long glideInitCount = 0;
GrDeviceInfo_t 	tnslDeviceList[MAX_GLIDE_DEVICES];

extern Boolean bNanosaurRunning;
extern Boolean bBugdomRunning;
extern Boolean bATVRunning;
extern Boolean bCombatRunning;
extern Boolean bDVRunning;
extern Boolean bQuakeRunning;
extern Boolean bFlyRunning;
extern Boolean bNightfallRunning;
extern TRvInfo gRvEngInfo;


// prototypes
static UInt32 	IntersectingRects(Rect *r1, Rect *r2);
float CalcFogNone(float invW, float a, TQADrawPrivate* dp);

static void ContextInit(TQADrawContext*  newDrawContext,
		                   const TQADevice* device,
		                   const TQARect*   rect,
		                   const TQAClip*   clip,
		                   unsigned long    flags);

#define ONE_MEGABYTE 0x00100000
#define SHOW_OVERDRAW 0
// These symbols belong in Rave.h, but Apple never added them.
#define kQAGL_Nearest		0
#define	kQAGL_Linear		1


//////////////////////////
//                      //
//  RvDrawPrivateNew  //
//                      //
//////////////////////////

TQAError 
RvDrawPrivateNew(TQADrawContext*  newDrawContext,
                   const TQADevice* device,
                   const TQARect*   rect,
                   const TQAClip*   clip,
                   unsigned long    flags)
{
	TQADrawContext* dc;
	TQADrawPrivate* dp;
	UInt32	theResult;
	long	tmp = -1;
	
	TQAError err = kQANoErr;
	
	DebugStr("\n\n\n\n--> ********** RvDrawPrivateNew - ");
	Debug4Num(rect->left, rect->top, rect->right, rect->bottom);
	DebugStr(" flags="); DebugHex(flags);
	
	if (rect->right - rect->left == 0)
		return kQAParamErr;
	if (rect->bottom - rect->top == 0)
		return kQAParamErr;
		
#if TNSL_DEBUG
	if (flags & kQAContext_Scale)
		DebugStr(" kQAContext_Scale ");
	if (flags & kQAContext_NoDither)
		DebugStr(" kQAContext_NoDither ");
	if (flags & kQAContext_NonRelocatable)
		DebugStr(" kQAContext_NonRelocatable ");
	if (flags & kQAContext_NoZBuffer)
		DebugStr(" kQAContext_NoZBuffer ");
#endif

	if(flags & (kQAContext_Cache | kQAContext_EngineSpecific1 | kQAContext_EngineSpecific2 | kQAContext_EngineSpecific3 | kQAContext_EngineSpecific4))
	{
		DebugStr("\n*** context caching not supported **************************\n");
		err = kQANotSupported;
		goto bail;
	}
	
	// not supporting 8-bit screen buffers, because we don't have an 
	// accelerated blit for it.
	if ( (device->deviceType == kQADeviceGDevice) && ((**(*device->device.gDevice)->gdPMap).pixelSize < 16))
	{
		DebugStr("*** pixelSize < 16, not supported *** \n");
		err = kQANotSupported;
		goto bail;
	}
		
	newDrawContext->drawPrivate = (void*) AllocPtr(sizeof (TQADrawPrivate));
	if (newDrawContext->drawPrivate == NULL) {
		DebugStr("*** ? AllocPtr failed");
		err = kQAOutOfMemory;
		goto bail;
	}
	DebugStr("\n");
	
	memset(newDrawContext->drawPrivate, 0, sizeof(TQADrawPrivate)); 

	dc = newDrawContext;
	dp = newDrawContext->drawPrivate;
	dp->ctxID = ++glideInitCount;		// assigns each ctx a unique number
	
	gRvEngInfo.numDrawContexts++;
	// for now, all contexts will be double buffered

	if(flags & kQAContext_NoZBuffer) 
	{
		DebugStr("No Z Buffer ");
		dp->z_bits = 0;
	} 
	else if ((flags & kQAContext_DeepZ) && NAPALM_AND_BEYOND) 
		{
		DebugStr("Deep Z Buffer ");
		dp->z_bits = 24;
		}
	else
		{
		dp->z_bits = 16;
		DebugStr("Shallow Z Buffer ");
		}
	
	ContextInit(dc, device, rect, clip, flags);
	
	DebugStr( "dp ptr = "); DebugHex( dp );

	dc->setFloat              = RvSetFloat;
	dc->setInt                = RvSetInt;
	dc->setPtr                = RvSetPtr;
	dc->getFloat              = RvGetFloat;
	dc->getInt                = RvGetInt;
	dc->getPtr                = RvGetPtr;
	dc->drawPoint             = RvDrawPoint;
	dc->drawLine              = RvDrawLine;
	dc->drawTriGouraud        = RvDrawTriGouraud;
	dc->drawTriTexture        = RvDrawTriTexture;
	dc->drawVGouraud          = RvDrawVGouraud;
	dc->drawVTexture          = RvDrawVTexture;
	dc->drawBitmap            = RvDrawBitmap;
	dc->renderStart           = RvRenderStart;
	dc->renderEnd             = RvRenderEnd;
	dc->renderAbort           = RvRenderAbort;
	dc->flush                 = RvFlush;
	dc->sync                  = RvSync;
	dc->drawTriMeshGouraud    = RvDrawTriMeshGouraud;
	dc->submitVerticesGouraud = RvSubmitVerticesGouraud;
	dc->submitVerticesTexture = RvSubmitVerticesTexture;
	dc->drawTriMeshTexture    = RvDrawTriMeshTexture;  //  RvDrawTriMeshTextureVertexArray;		// 
	
	dc->setNoticeMethod       = RvSetNoticeMethod;
	dc->getNoticeMethod       = RvGetNoticeMethod;
	
    if( dc->version >= kQAVersion_1_6 )
    {
    	//+ TBFL for Rave 1.6 support
    	dc->submitMultiTextureParams = NULL;	// RvSubmitMultiTextureParams;
	    dc->accessDrawBuffer 		= RvAccessDrawBuffer;
	    dc->accessDrawBufferEnd 	= RvAccessDrawBufferEnd;
	    dc->accessZBuffer			= RvAccessZBuffer;
	    dc->accessZBufferEnd		= RvAccessZBufferEnd;
	    dc->clearDrawBuffer			= RvClearDrawBuffer;	
	    dc->clearZBuffer			= RvClearZBuffer;	
	    dc->textureFromContext		= NULL;		
	    dc->bitmapFromContext		= NULL;		
	    dc->busy					= RvBusy;	
	    dc->swapBuffers				= RvSwapBuffers;
    }

	dp->width = rect->right - rect->left;
	dp->height = rect->bottom - rect->top;
	
#if ALLOW_GLIDE_FULLSCREEN
	if (dp->bFullScreen)
	{
		UInt32	low, high;
		theResult = InitializeFullscreenGlide( newDrawContext );
		grViewport( rect->left, rect->top, (long)dp->screenWidth, (long)dp->screenHeight );
		grClipWindow( 0, 0, (FxU32)dp->screenWidth, (FxU32)dp->screenHeight);
    	high = grTexMaxAddress(GR_TMU0);
    	low  = grTexMinAddress(GR_TMU0);
    	gRvEngInfo.surface_size	 = high - low;	
    	// V3 sometimes returns zero from grTex*Address
    	if (gRvEngInfo.surface_size <= 0)
    	{
    		// TBFL
    		// work around Glide failure
    		FxI32	nonTexVRAM, safetyMargin;
			if (NAPALM_AND_BEYOND)
				safetyMargin = 6 * ONE_MEGABYTE;
			else
				safetyMargin = ONE_MEGABYTE;
			
    		nonTexVRAM = gRvEngInfo.VRAMusedForNonTextures;
    		if (nonTexVRAM < 640*480*2)		// anything less than this is probably not right
    		{
    			FxI32 backBufSize = 0, frontBufSize = 0, zBufSize = 0;
    			frontBufSize = (dp->targetPixelBits >> 3) * dp->screenWidth * dp->screenHeight;
    			backBufSize  = (dp->targetPixelBits >> 3) * dp->width * dp->height;
    			if (!(flags & kQAContext_NoZBuffer))
    			{
    				if ((flags & kQAContext_DeepZ) && (NAPALM_AND_BEYOND))
    					zBufSize = 4 * dp->width * dp->height;
    				else
     					zBufSize = 2 * dp->width * dp->height;
    			}
    			nonTexVRAM = frontBufSize + backBufSize + zBufSize;
    		}
    		gRvEngInfo.surface_size = gRvEngInfo.totalVRAMcard - nonTexVRAM - safetyMargin;
    		DebugStr("\nFullscreen surface size determined by algorithm = "); DebugNum(gRvEngInfo.surface_size);
    	}
    	assert(gRvEngInfo.surface_size);
    	assert(gRvEngInfo.surface_size < 33000000);

	}
	else
#endif
	{
		// Sets device
		theResult = SetupGlideSurfaceBlit( dp, rect );
		if ( theResult == 0 )
		{
			tmp = dp->targetPixelBits;
			theResult = InitializeWindowedGlideRenderingSurface( newDrawContext, (rect->right - rect->left), 
				(rect->bottom - rect->top), tmp);
			grViewport( rect->left, rect->top, dp->width, dp->height );
			grClipWindow( 0, 0, (FxU32)dp->width, (FxU32)dp->height);
			DebugStr("Viewport "); 
			DebugNum( rect->left); DebugNum( rect->top );
			DebugNum( rect->right - rect->left); DebugNum (rect->bottom - rect->top );
		}
	}

	assert( glideInitCount );
bail:
	DebugStr("\n");
	return err;
}


/////////////////////////////
//                         //
//  RvDrawPrivateDelete  //
//                         //
/////////////////////////////

void 
RvDrawPrivateDelete(TQADrawPrivate* drawPrivate)
{
	
	DebugStr("--> RvDrawPrivateDelete - ");
	DebugHex( drawPrivate );
		
	if (drawPrivate == NULL)
		return;
		
	// Textures are owned at the engine level.
	// Make sure all textures are no longer marked resident
	FreeAllTexturesVRAM();
	
// temp 10/25
	if (drawPrivate->grVerticesTexture) FreePtr((void*) drawPrivate->grVerticesTexture);
	if (drawPrivate->gFogTable) FreePtr((void *) drawPrivate->gFogTable);
	if (drawPrivate->maskRgn) DisposeRgn( drawPrivate->maskRgn );
	if (drawPrivate->pAlphaGlideStates)
		FreePtr((void *)drawPrivate->pAlphaGlideStates);
	if(drawPrivate->targetPort){
		CloseCPort(drawPrivate->targetPort);
		FreePtr((void *)drawPrivate->targetPort);
		drawPrivate->targetPort = NULL;
	}

	// 10/23 added to fix modeling apps that create new ctx every frame.
	if (gRvEngInfo.currentDrawContext == drawPrivate->parent)
		gRvEngInfo.currentDrawContext = NULL;
		
	/*

	grSurfaceSetTextureSurfaceExt(GR_TMU0,0);
	grSurfaceSetTextureSurfaceExt(GR_TMU1,0);
	*/
	
	// Textures are owned at the engine level.
	// This ignores textures that are not known to client; i.e., bitmap pieces.
	// This marks each as non-resident and then frees cached storage, too.
	
	// rave terminate eradicates all textures.
	
	/* temp 10/16
	if(gRvEngInfo.texture_surface && gRvEngInfo.numDrawContexts == 1) 
	{
		grSurfaceReleaseExt(gRvEngInfo.texture_surface);
		gRvEngInfo.texture_surface = 0;
		gRvEngInfo.surface_size = 0;
		DebugStr(" texture_surface released and deleted ");
	}
	*/
	// temp 10/23 grSurfaceSetRenderingSurfaceExt(NULL);
	// temp 10/23 grSurfaceSetAuxSurfaceExt(NULL);
	
	if(drawPrivate->auxSurface) 
	{
		grSurfaceReleaseExt(drawPrivate->auxSurface);
		drawPrivate->auxSurface = 0;
		gRvEngInfo.VRAMusedForNonTextures -= drawPrivate->auxSurfaceDesc.pitch * drawPrivate->auxSurfaceDesc.height;
		gRvEngInfo.VRAMusedForNonTextures = max( 0, gRvEngInfo.VRAMusedForNonTextures );
	}
	if(drawPrivate->renderSurface) 
	{
		grSurfaceReleaseExt(drawPrivate->renderSurface);
		drawPrivate->renderSurface = 0;
		gRvEngInfo.VRAMusedForNonTextures -= drawPrivate->renderSurfaceDesc.pitch * drawPrivate->renderSurfaceDesc.height;
		gRvEngInfo.VRAMusedForNonTextures = max( 0, gRvEngInfo.VRAMusedForNonTextures );
	}
	DebugStr(" gRvEngInfo.VRAMusedForNonTextures="); DebugNum(gRvEngInfo.VRAMusedForNonTextures);
	
	if(drawPrivate->glideContext) 
	{
		if (drawPrivate->bFullScreen)
		{
			grSstWinClose(drawPrivate->glideContext);
		}
		else
		{
			// temp 10/25 to avoid crashes				grSurfaceReleaseContextExt(drawPrivate->glideContext);
		}
		// drawPrivate->glideContext = 0;
	}

	FreePtr((void*) drawPrivate);

	assert( gRvEngInfo.numDrawContexts > 0 );
	gRvEngInfo.numDrawContexts--;
	DebugStr("\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	DebugStr("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	

	return;	
	
	
}

/*
________________________________________________________________________________________

      SetupGlideSurfaceBlit -- note: this doesn't actually do anthing
________________________________________________________________________________________

*/

static UInt32
SetupGlideSurfaceBlit( TQADrawPrivate * inContext, const TQARect* inRect)
{
	UInt32				theResult = GLI_BAD_DRAWABLE;
	
	// DEBUG_ENTRY( SetupGlideSurfaceBlit );	

	//+ might need a InitializeGlide() 
	// if ( !gls_GlideInitialized ) glrInitializeGlide();

	if ( inContext->glideContext )
	{
		long           win_w, win_h; //, buf_w, buf_h;
		Rect            global_rect;
		Rect            device_rect;
//		Rect            local_rect;

		/* Get the window rect in global device coords */
		win_w = RvAbs(inRect->right  - inRect->left);
		win_h = RvAbs(inRect->bottom - inRect->top);
		
		// global_rect.top    = -(**inTargetGrafPort->portPixMap).bounds.top  + inTargetGrafPort->portRect.top;
		global_rect.top		= inRect->top;
		// global_rect.left   = -(**inTargetGrafPort->portPixMap).bounds.left + inTargetGrafPort->portRect.left;
		global_rect.left	= inRect->left;
		global_rect.bottom = global_rect.top  + win_h;
		global_rect.right  = global_rect.left + win_w;

		/* Get device rect */
		device_rect = (*(inContext->device).device.gDevice)->gdRect;

	}
	else
	{
		long i = 0;
		long theBoardID = -1;
		long           win_w, win_h; //, buf_w, buf_h;
		Rect            global_rect;
		UInt32          bestArea;
		//+ DEBUG_VERBOSE( SetupGlideSurfaceBlit, "setting up a new glide context...\n" );
 		/* Get the window rect in global device coords */
		win_w = RvAbs(inRect->right  - inRect->left);
		win_h = RvAbs(inRect->bottom - inRect->top);
		
		// global_rect.top    = -(**inTargetGrafPort->portPixMap).bounds.top  + inTargetGrafPort->portRect.top;
		global_rect.top		= inRect->top;
		// global_rect.left   = -(**inTargetGrafPort->portPixMap).bounds.left + inTargetGrafPort->portRect.left;
		global_rect.left	= inRect->left;
		global_rect.bottom = global_rect.top  + win_h;
		global_rect.right  = global_rect.left + win_w;
        bestArea = 0;
        
		theBoardID = 0;
		
	}
	//+ RES inContext->targetPort = inTargetGrafPort;
	//GetCWMgrPort( &inContext->targetPort );	// get global port
	
	theResult = 0;

ErrorExit:
	return theResult;
}

void ContextInit(TQADrawContext*  dc,
		                   const TQADevice* device,
		                   const TQARect*   rect,
		                   const TQAClip*   clip,
		                   unsigned long    flags)
{
	Rect		deviceRect;
	Rect		quizRect;
	int j;
	RgnHandle 	theClipRgn = NULL;

	TQADrawPrivate * dp = dc->drawPrivate;
	
	dp->parent			= dc;
	// See if this is full-screen context
	dp->device.deviceType 		= device->deviceType;
	dp->device.device.gDevice 	= device->device.gDevice;
	if (dp->device.deviceType == kQADeviceGDevice)
	{
		deviceRect = (*dp->device.device.gDevice)->gdRect;
		quizRect = (**(*dp->device.device.gDevice)->gdPMap).bounds;
		dp->bFullScreen = (rect->top == 0) 		&&
						  (rect->bottom == deviceRect.bottom - deviceRect.top)	&&
						  (rect->left == 0)		&&
						  (rect->right == deviceRect.right - deviceRect.left);

		/* Quake must run in windowed mode, because when the Mac dialogs come up, 
		   the context is not destroyed. So, in fullscreen mode, you don't see
		   the dialogs.
		*/
		if (bNanosaurRunning || bBugdomRunning || bQuakeRunning ) 
			dp->bFullScreen = false;

		// There cannot be any other contexts when you go fullscreen.
		if (gRvEngInfo.numDrawContexts != 1)
			dp->bFullScreen = false;

		DebugStr(" bFullScreen="); DebugNum( dp->bFullScreen );
		#if !ALLOW_GLIDE_FULLSCREEN
		dp->bFullScreen = false;
		#endif
	}
	else
	{
		deviceRect.top = 0; deviceRect.left = 0;
		deviceRect.right = device->device.memoryDevice.width;
		deviceRect.bottom = device->device.memoryDevice.height;
		dp->bFullScreen = false;
	}
	
	// SJL code starts here
	{
		GDHandle saveDevice;
		CGrafPtr savePort;
		
		GetGWorld(&savePort, &saveDevice);		// get the current CGrafPort and GDevice to restore later
		
		SetGDevice(dp->device.device.gDevice);	// set to our gdevice
		
		dp->targetPort = (void *)AllocPtr(sizeof (CGrafPort));
		
		assert(dp->targetPort != NULL); // failed allocation is fatal for this draw context
		
		OpenCPort(dp->targetPort);			// create a new port for us to draw into...
				
		SetGWorld(savePort, saveDevice);		// restore port and device
		
		
	}
	DebugStr( "\n");
	DebugStr( " global numDrawContexts="); DebugNum( gRvEngInfo.numDrawContexts );
	DebugStr( "\n global textureCount="); DebugNum( gRvEngInfo.textureCount);
	DebugStr( "\n global totalTextureSize="); DebugNum( gRvEngInfo.totalTextureSize);
	DebugStr( "\n gloabl avail tex mem on card="); DebugNum( gRvEngInfo.totalVRAMcard);
	DebugStr( "\n global avail alloc tex mem="); DebugNum( gRvEngInfo.surface_size - gRvEngInfo.totalTextureSize);
	DebugStr( "\n");
	// dp->bBetweenRStartAndREnd = false;
	dp->screenWidth = deviceRect.right - deviceRect.left;
	dp->screenHeight = deviceRect.bottom - deviceRect.top;
	DebugStr("Screen resolution "); DebugNum(dp->screenWidth); DebugNum(dp->screenHeight);
	// the clip region can be passed to copybits
	if (clip)
	{
		dp->maskRgn		= NewRgn();
		if (dp->maskRgn)
			MacCopyRgn( clip->clip.clipRgn, dp->maskRgn );
	}
	else
		dp->maskRgn = NULL;
		
	dp->tChangeMask				= 0xFFFFFFFF;
	dp->tTex1ChangeMask			= 0xFFFFFFFF;
	dp->tTex2ChangeMask			= 0x0;
	dp->tFogChangeMask			= 0xFFFFFFFF;
	dp->bStateInCacheChanged 	= true;	

	dp->fogFunc					= CalcFogNone;
	dp->gFogTable				= NULL;
	
	dp->flags      = flags;
	dp->targetRect = *rect;		//+
	dp->deviceRect.top			= deviceRect.top;
	dp->deviceRect.left			= deviceRect.left;
	dp->deviceRect.right		= deviceRect.right;
	dp->deviceRect.bottom		= deviceRect.bottom;
	
	dp->tState[kQATag_Texture].p			= NULL;
	dp->tState[kQATag_MultiTexture].p		= NULL;
	dp->tState[kQATag_MultiTextureCurrent].i 	= 0;

    /*
     * Set the default context state variables
     */
     
    /*
     * DepthBias. These values are from the RAVE spec, for fixed-point z buffers.
     */
    dp->tState[kQATag_ZMinOffset].f     	= -1.0f / 65536.0f;
    dp->tState[kQATag_ZMinScale].f      	= 1.0f;

    /*
     * Color for buffer clears
     */
    dp->tState[kQATag_ColorBG_a].f			= 0.0f;
    dp->tState[kQATag_ColorBG_r].f      	= 0.0f;
    dp->tState[kQATag_ColorBG_g].f      	= 0.0f;
    dp->tState[kQATag_ColorBG_b].f      	= 0.0f;
    //+ SetBgColor();
    
    /*
     * Color for fog
   	 */
    dp->tState[kQATag_FogColor_r].f 		= 1.0f;
    dp->tState[kQATag_FogColor_g].f			= 1.0f;
    dp->tState[kQATag_FogColor_b].f			= 1.0f;
    dp->tState[kQATag_FogColor_a].f			= 1.0f;

    /*
     * Lines and points of variable width
     */
    dp->tState[kQATag_Width].f			= 1.0f;
    
    /*
     * Alpha Blending Control
     */
    dp->tState[kQATag_Blend].i			= kQABlend_PreMultiply;	// RAVE default
    dp->tState[kQATagGL_BlendSrc].i		= GL_ONE;
    dp->tState[kQATagGL_BlendDst].i		= GL_ZERO;
    // dp->rgb_sf							= GR_BLEND_SRC_ALPHA; implies Interpolate
    dp->rgb_sf							= GR_BLEND_ONE;
    dp->rgb_df							= GR_BLEND_ONE_MINUS_SRC_ALPHA;
    
    /*
     * Alpha Testing
     */
	dp->tState[kQATag_AlphaTestFunc].i = kQAAlphaTest_GT;
    dp->tState[kQATag_AlphaTestRef].f	= 0.0;
    // hack temp RES (sent mail to E. Drumbor)
    if (bATVRunning)
	     dp->tState[kQATag_AlphaTestRef].f	= 1.0;
    
    /*
     * Fog Control
     */  
	dp->tState[kQATag_FogMode].i		= kQAFogMode_None;
	dp->atiFog							= FXFALSE;
   	dp->tState[kQATag_FogStart].f		= 0.5f;
   	dp->tState[kQATag_FogEnd].f			= 1.0f;
   	dp->tState[kQATag_FogDensity].f		= 1.0f;
 	dp->tState[kQATag_FogMaxDepth].f	= 1.0f;
     
    /*
     * Texture Control
     */
    dp->tState[kQATag_TextureFilter].i	= kQATextureFilter_Fast;
    // dubious hack, but makes Nightfall look a lot better
    if (bNightfallRunning)
    	dp->tState[kQATag_TextureFilter].i	= kQATextureFilter_Mid;
    	
    dp->tState[kQATag_TextureOp].i		= kQATextureOp_None;
    dp->tState[kQATag_Texture].p		= NULL; 
	dp->tState[kQATagGL_TextureWrapU].i = kQAGL_Repeat;
	dp->tState[kQATagGL_TextureWrapV].i = kQAGL_Repeat;
	dp->tState[kQATagGL_TextureMinFilter].i = kQAGL_Nearest;
	dp->tState[kQATagGL_TextureMagFilter].i = kQAGL_Nearest;

    /*
     * Texture Compositing
     */
	dp->tState[kQATag_MultiTextureEnable].i 	= 0;
	dp->tState[kQATag_MultiTextureBorder_a].f	= 1.0;
	dp->tState[kQATag_MultiTextureBorder_r].f	= 1.0;
	dp->tState[kQATag_MultiTextureBorder_g].f	= 1.0;
	dp->tState[kQATag_MultiTextureBorder_b].f	= 1.0;
	dp->tState[kQATag_MultiTextureFactor].f 	= 1.0;
	dp->tState[kQATag_MultiTextureMipmapBias].f	= 1.0;

	dp->tState[kQATag_BitmapScale_x].f	= 1.0;
	dp->tState[kQATag_BitmapScale_y].f 	= 1.0;


    dp->tState[kQATag_PerspectiveZ].i   = kQAPerspectiveZ_Off;
    // This should not be "none", otherwise, the z buffer clear will not work.
    dp->tState[kQATag_ZFunction].i 		= kQAZFunction_LT;

	dp->glDepthFunc						= GR_CMP_ALWAYS;
	
    dp->tState[kQATag_Antialias].i      = kQAAntiAlias_Off;
        
    dp->tState[kQATag_DontSwap].i 		= false;
    dp->tState[kQATagGL_DepthBG].f		= 1.0;
    	
	dp->tState[kQATag_ChannelMask].i	= kQAChannelMask_r | kQAChannelMask_g | kQAChannelMask_b | kQAChannelMask_a;
    if (bATVRunning)
	     dp->tState[kQATag_ZBufferMask].i	= kQAZBufferMask_Disable; // When default was _Enable, Kawasaki didn't work
	else 
		dp->tState[kQATag_ZBufferMask].i	= kQAZBufferMask_Enable;	
	/*
	 * Chromakey
	 */
	dp->tState[kQATag_ChromakeyEnable].i	= 0;
	dp->tState[kQATag_Chromakey_r].f		= 0.0;
	dp->tState[kQATag_Chromakey_g].f		= 0.0;
	dp->tState[kQATag_Chromakey_b].f		= 0.0;

	dp->nVerticesGouraud		= 0;
	dp->countGouraud			= 0;
	dp->gouraudVertexList		= NULL;
	dp->nVerticesTexture		= 0;
	dp->countTexture			= 0;
	dp->textureVertexList		= NULL;
	dp->grVerticesTexture		= NULL;
	dp->nMultiTexParams			= 0;
	dp->multiTexParams			= NULL;
	dp->bSortingAlphaTri		= 1;	// user can turn off with zsorthint
	dp->tState[kQATag_ZSortedHint].i = 1;

	// transparent triangle cache

	dp->idxAlphaTri				= 0;
	dp->idxAlphaTriStatesList	= 0;
	dp->countAlphaTri			= 0;
	dp->maxCachedTri			= kMaxAlphaTriInCache;	// currently at 2304
	dp->maxCachedStates			= kMaxAlphaTriStates;		// TBFL adjust during testing
	// temp TBFL; fix for general case
	if (bDVRunning)
		dp->maxCachedStates		= 230;
	// preallocate the memory; optimize later
#if CUSTOM_GLIDE_STATE_DEF
	dp->pAlphaGlideStates	= (TRaveRenderGlideState *)AllocPtr( dp->maxCachedStates * sizeof(TRaveRenderGlideState) );
#else
	dp->pAlphaGlideStates	= (void *)AllocPtr( dp->maxCachedStates * gRvEngInfo.sizeOfGlideState );
#endif
	if (dp->pAlphaGlideStates == NULL)
	{
		dp->bSortingAlphaTri = 0;
		dp->tState[kQATag_ZSortedHint].i = 0;	// this will turn off the feature; see RenderStart
	}
	    
	dp->pCurrAlphaGlideState	= dp->pAlphaGlideStates;
	dp->currTriType				= TNSL_NONE;
	dp->usingVertexAlpha		= 0;
	dp->usingTextureAlpha[0]	= 0;
	dp->usingTextureAlpha[1]	= 0;
	dp->currColorTable 			= NULL;
	// dp->currAlphaBits			= 0xDEADBEEF;
	dp->lastBaseTextureProcessed			= NULL;
	dp->currChromakeyValue		= 0xDEADBEEF;
	
		//+ initialize notice (callback) methods
	for ( j = 0; j < kQAMethod_NumSelectors; j++ )
	{
		dp->completionCallBack[j].standardNoticeMethod	= NULL;
		dp->callBackRefCon[j] = NULL;
	}
		
	switch (dp->device.deviceType )
	{
        case kQADeviceGDevice:
    	{
	        PixMap *devicePixMap 	= *(*(dp->device).device.gDevice)->gdPMap;
	        dp->targetPixelBits		= devicePixMap->pixelSize;
	        dp->screenRowBytes   	= devicePixMap->rowBytes & 0x3fff;
	        /* this gives a pointer to back buffer VRAM. */
			dp->pBaseAddr			= (void *)dp->renderSurfaceDesc.surface;
			DebugStr(" screen color depth="); DebugNum(dp->targetPixelBits);
			break;
		}		
		default:
		{
			dp->screenRowBytes = 0;
			dp->pBaseAddr = NULL;
			break;
		}
	}
	
	
#if TNSL_WINDOW
	dp->blitDevice = NULL;
	dp->blitPort = NULL;

#endif 
	/* TNSL_WINDOW */
	
}


void ResetContextStateFlags( TQADrawContext *inContext )
{
	TQADrawPrivate *dp = inContext->drawPrivate;
	
	dp->tChangeMask				= 0xFFFFFFFF;
	// Since multitexuring is not supported, save a few if's by turning off the flag here
	dp->tChangeMask 			&= ~STATE_COMPOSITE;
	dp->tFogChangeMask			= 0xFFFFFFFF;
	dp->tTex1ChangeMask			= 0xFFFFFFFF;
	// dp->tTex2ChangeMask		= 0xFFFFFFFF;
	dp->bStateInCacheChanged 	= true;	
	dp->currTriType 			= TNSL_NONE;
	DebugStr("\ndp->tChangeMask ="); DebugHex(dp->tChangeMask); DebugStr("\n");
}

FxI32 ConvertRaveAlphaTestFunc( TQADrawPrivate *dp )
{
	switch (dp->tState[kQATag_AlphaTestFunc].i) 
	{
	case kQAAlphaTest_None: 
		DebugStr( " GR_CMP_ALWAYS ");
		return (GR_CMP_ALWAYS);
		break;
	case kQAAlphaTest_LT:
		DebugStr( " GR_CMP_LESS ");
		return (GR_CMP_LESS);
		break;
	case kQAAlphaTest_EQ:
		DebugStr( " GR_CMP_EQUAL ");
		return (GR_CMP_EQUAL);
		break;
	case kQAAlphaTest_LE:
		DebugStr( " GR_CMP_LEQUAL ");
		return (GR_CMP_LEQUAL);
		break;
	case kQAAlphaTest_GT:
		DebugStr( " GR_CMP_GREATER ");
		return (GR_CMP_GREATER);
		break;
	case kQAAlphaTest_NE:
		DebugStr( " GR_CMP_NOTEQUAL ");
		return (GR_CMP_NOTEQUAL);
		break;
	case kQAAlphaTest_GE:
		DebugStr( " GR_CMP_GEQUAL ");
		return (GR_CMP_GEQUAL);
		break;
	case kQAAlphaTest_True:
		DebugStr( " GR_CMP_ALWAYS ");
		return (GR_CMP_ALWAYS);
		break;
	}
	return 0;
}

/*
 * SetRenderState
 * Sets Glide state based on RAVE state variables.
 * Just before drawing, SetRenderState will be called, followed by either SetTextureModeAndCheckTexAlpha or SetGouraudMode.
 * Some additional Glide states are set in SetTextureModeAndCheckTexAlpha & SetGouraudMode, but those are
 * not based on RAVE state.
 */

void SetRenderState( TQADrawContext* drawContext, Boolean bTexturing)
{
	TQADrawPrivate *dp = drawContext->drawPrivate;
	
	if (dp->tChangeMask == 0) return;
	
	DebugStr("SetRenderState tChangeMask = "); DebugHex(dp->tChangeMask); DebugStr("\n");
	
 	if( dp->tChangeMask & STATE_BLEND )
 	{
		SetGlide_Alpha( dp );		// PreMultiply or Interpolate, sets AlphaBlendFunction
		DebugStr(" SetGlide_Alpha ");
	}
	
	if( (dp->tChangeMask & STATE_ALPHA_TST_FUNC) && (dp->currBaseTexture->alphaBits != 1) )
	{
		grAlphaTestFunction( ConvertRaveAlphaTestFunc(dp) );
		DebugStr(" grAlphaTestFunction ");
	}
	
	if( (dp->tChangeMask & STATE_ALPHA_TST_REF) && (dp->currBaseTexture->alphaBits != 1) )	
	{
		grAlphaTestReferenceValue((FxU8)(dp->tState[kQATag_AlphaTestRef].f * 255.9f));
		DebugStr(" grAlphaTestReferenceValue= "); DebugFloat(dp->tState[kQATag_AlphaTestRef].f * 255.9f);
		DebugNum((FxU8)(dp->tState[kQATag_AlphaTestRef].f * 255.9f));
	}

			
	if( dp->tChangeMask & STATE_TEXTURE )
	{
		if (!bTexturing)
			dp->lastBaseTextureProcessed = (TQATexture *)0xDEADBEEF;	// makes LoadTextureAndTableToVRAM call grTexSource; maybe unnecessary

		SetAndLoadTextureMap(dp);
		/* might be needed if multitexturing is ever supported.
	    if( dp->tTex1ChangeMask & STATE_PRIMARY_TEX )
	    { }
		*/
		if( (dp->tTex1ChangeMask & STATE_TEX_WRAPU) || (dp->tTex1ChangeMask & STATE_TEX_WRAPV) )
		{
			SetTexWrapClamp(dp);
		}
		if( dp->tTex1ChangeMask & STATE_TEX_FILTER )
            SetBaseTextureFilter(dp);

       	if( dp->tTex1ChangeMask & STATE_TEX_OP )
       	{
 			SetTexGlideCombinesBasedOnState( dp, bTexturing);
 			if (bTexturing)
 			{
 				dp->lastBaseTextureProcessed = dp->currBaseTexture;
 			}
 		}
		
 		/* not supported 
 		if( dp->tTex1ChangeMask & STATE_LOD_BIAS )
 		{ NULL; } */
					
       	dp->tTex1ChangeMask 	= 0;
 	}

	if( dp->tChangeMask & STATE_COMPOSITE )
	{	
		SetRenderStateComposite( dp, bTexturing );
	}
	
	if( dp->tChangeMask & STATE_Z_FUNC )
	{
      	SetGlide_Z( dp );				// sets grDepthMask & grDepthBufferFunction
		// temp experi. Never disable it.
		// SetDepthBufferMode( dp );		// sets grDepthBufferMode
		// grDepthBufferMode( GR_DEPTHBUFFER_ZBUFFER );		// temp
      	if (dp->tChangeMask & STATE_DRAW_DESTINATION)
      	{
      		grDepthMask( dp->tState[kQATag_ZBufferMask].i == kQAZBufferMask_Enable );
      		DebugStr( " Glide grDepthMask(f(kQATag_ZBufferMask)) ");
      		DebugNum(dp->tState[kQATag_ZBufferMask].i);
      	}
	}
      	
    if ( dp->tChangeMask & STATE_ALPHASORTING )
    {
	    SetAlphaSorting( dp );
	}

	if( (dp->tState[kQATag_Blend].i == kQABlend_OpenGL) && 
		 dp->tChangeMask & STATE_GL_BLEND_MODE )
	{
		SetOpenGLBlending(dp);
	}
	
    if ( dp->tChangeMask & STATE_FOG )
	{    	
		SetFog( dp );
	}
    	
	if( dp->tChangeMask & STATE_RGB_MASK )
	{
		SetGlide_Mask( dp );
	}
	
	if( dp->tChangeMask & STATE_DITHER_MODE )
	{
		SetGlide_DitherMode( dp );
	}

	if (dp->tChangeMask & STATE_CHROMAKEY)
	{
		if (dp->tState[kQATag_ChromakeyEnable].i > 0)
			grChromakeyMode( GR_CHROMAKEY_ENABLE );
		else
			grChromakeyMode( GR_CHROMAKEY_DISABLE );
	}

	dp->bStateInCacheChanged 	= true;	    
    dp->tChangeMask 			= 0;
    DebugStr("\n");
}

void SetGouraudMode( TQADrawPrivate *dp )
{
	DebugStr(" * SetGouraudMode * ");
#if VARI_VERT_LAYOUT
		grVertexLayout(GR_PARAM_ST0, 40, GR_PARAM_DISABLE);
		grVertexLayout(GR_PARAM_Q0,  48, GR_PARAM_DISABLE);
		grVertexLayout(GR_PARAM_ST1, 52, GR_PARAM_DISABLE);
		grVertexLayout(GR_PARAM_Q1,  60, GR_PARAM_DISABLE);
		DebugStr(" grVertexLayout disable Q, ST0, Q0 * ");
#endif
		DebugStr("\n");
		DebugStr("^^^^ grAlphaCombine(LOCAL_ALPHA, FACTOR_NONE, LOCAL_ITERATED, OTHER_NONE)\n");
		DebugStr("^^^^ grColorCombine FUNCTION_LOCAL FACTOR_NONE LOCAL_ITERATED OTHER_NONE \n");

#if CUSTOM_GLIDE_STATE_DEF
		dp->currAlphaCombine_function	= GR_COMBINE_FUNCTION_LOCAL_ALPHA;
		dp->currAlphaCombine_factor		= GR_COMBINE_FACTOR_NONE;
		dp->currAlphaCombine_local		= GR_COMBINE_LOCAL_ITERATED;
		dp->currAlphaCombine_other		= GR_COMBINE_OTHER_NONE;
#endif	
		grAlphaCombine(
			GR_COMBINE_FUNCTION_LOCAL_ALPHA,        // combine function
			GR_COMBINE_FACTOR_NONE,					// combine factor
			GR_COMBINE_LOCAL_ITERATED, 				// local color
			GR_COMBINE_OTHER_NONE, 				    // other color
			FXFALSE									// invert
			);
			
#if CUSTOM_GLIDE_STATE_DEF
		dp->currColorCombineFunction 	= GR_COMBINE_FUNCTION_LOCAL;
		dp->currColorCombineFactor		= GR_COMBINE_FACTOR_NONE;
		dp->currColorCombineLocal		= GR_COMBINE_LOCAL_ITERATED;
		dp->currColorCombineOther		= GR_COMBINE_OTHER_NONE;
#endif	

		grColorCombine(
			GR_COMBINE_FUNCTION_LOCAL, 
			GR_COMBINE_FACTOR_NONE,
			GR_COMBINE_LOCAL_ITERATED, 
			GR_COMBINE_OTHER_NONE, 
			FXFALSE
			);

		dp->bStateInCacheChanged = true;
		dp->lastBaseTextureProcessed = NULL;
		// dp->currAlphaBits = 0xDEADBEEF;
		dp->usingTextureAlpha[GR_TMU0] = false;
		dp->currTriType = TNSL_GOURAUD;
}


void SetTextureModeAndCheckTexAlpha( TQADrawPrivate *dp )
{
	FxU32 			numActiveTextureUnits = 1;
	
	/*
	 * Note carefully.
	 ¥ This should be called only when texturing.
	 * Do not set RAVE state variables here. Only use direct Glide calls to affect rendering state.
	 * Reason: SetTextureModeAndCheckTexAlpha is always called after SetRenderState.
	 */
	if (numActiveTextureUnits)
	{
		DebugStr( "\nSetTextureMode: ");
		if (dp->currBaseTexture != dp->lastBaseTextureProcessed ) 
			SetTexGlideCombinesBasedOnState( dp, FXTRUE);
		DebugStr("currBaseTexture = "); DebugHex((int) dp->currBaseTexture );
		
		if(dp->currBaseTexture) 
		{
				DebugStr("alphaBits == "); DebugNum(dp->currBaseTexture->alphaBits); 
			// DebugStr("currAlphaBits == "); DebugNum(dp->currAlphaBits);
						
						if (dp->currBaseTexture->glideTextureInfo.format == GR_TEXFMT_ALPHA_8)
						{
							// this format often used for text overlays with transparent background
							// same as either modulate or no lighting
							grAlphaCombine(				
									GR_COMBINE_FUNCTION_BLEND_OTHER,
									GR_COMBINE_FACTOR_LOCAL,
									GR_COMBINE_LOCAL_ITERATED,
									GR_COMBINE_OTHER_TEXTURE,
									FXFALSE );
							// unlike any of the standard texture ops
							grColorCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_ONE,
							         GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);
			}
			
			// if ((dp->currBaseTexture->alphaBits != dp->currAlphaBits)) 
			// if (dp->currBaseTexture != dp->lastBaseTextureProcessed )
			{
				// dp->currAlphaBits = dp->currBaseTexture->alphaBits;
				switch (dp->currBaseTexture->alphaBits) 
				{
					case 0:
						grAlphaTestFunction(GR_CMP_ALWAYS);
						dp->tChangeMask |= (STATE_ALPHA_TST_FUNC);
						dp->usingTextureAlpha[GR_TMU0] = false;
						break;
					case 1:
						DebugStr(", alpha test > 0x0 ");
						grAlphaTestReferenceValue(0);
						grAlphaTestFunction(GR_CMP_GREATER);
						// dp->tChangeMask |= (STATE_ALPHA_TST_FUNC | STATE_ALPHA_TST_REF);
						dp->usingTextureAlpha[GR_TMU0] = false;	// prevents caching 1555 textures.
						DebugStr("not usingTextureAlpha ");
						break;

					case 4:
					case 8: 
						DebugStr(", alpha test always ");
						grAlphaTestFunction(GR_CMP_ALWAYS);
						dp->tChangeMask |= (STATE_ALPHA_TST_FUNC);
						dp->usingTextureAlpha[GR_TMU0] = true;
						DebugStr("usingTextureAlpha ");
						break;
					default:
						assert(0);
						break;
				} 
			} 
			
		#if VARI_VERT_LAYOUT
				DebugStr("\ngrVertexLayout enable Q, ST0, Q0 ");
				grVertexLayout(GR_PARAM_ST0, 20, GR_PARAM_ENABLE);
				grVertexLayout(GR_PARAM_Q0,  28, GR_PARAM_ENABLE); // when enabled, textures are screwed up
		#endif
			} 
			else 
			{
		#if VARI_VERT_LAYOUT
				grVertexLayout(GR_PARAM_ST0, 20, GR_PARAM_DISABLE);
				grVertexLayout(GR_PARAM_Q0,  28, GR_PARAM_DISABLE);
		#endif
			}
			dp->lastBaseTextureProcessed = dp->currBaseTexture;
			  
		}

#if SHOW_OVERDRAW
          grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
                         GR_COMBINE_FACTOR_ONE,
                         GR_COMBINE_LOCAL_CONSTANT,
                         GR_COMBINE_OTHER_NONE,
                         FXFALSE);
          grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                         GR_COMBINE_FACTOR_ONE,
                         GR_COMBINE_LOCAL_CONSTANT,
                         GR_COMBINE_OTHER_NONE,
                         FXFALSE);
          grConstantColorValue(0x20202020);                         
#endif			
		
	dp->bStateInCacheChanged = true;
	dp->currTriType = TNSL_TEXTURE;
}


void SetAndLoadTextureMap( TQADrawPrivate *dp )
{
	UInt32 err = 0;
	DebugStr("\n * SetAndLoadTextureMap old BaseTexture= ");
	DebugHex((int) dp->currBaseTexture ); 
	DebugStr(" last base tex processed="); 
	DebugHex( dp->lastBaseTextureProcessed ); DebugStr("\n");

	dp->currBaseTexture = (TQATexture*)dp->tState[kQATag_Texture].p;
	if (dp->currBaseTexture)
		err = LoadAsBaseTexture(dp, dp->currBaseTexture);
		
	// dp->currMultiTexture = (TQATexture*)dp->tState[kQATag_MultiTexture].p;
		
}
	

void SetTexGlideCombinesBasedOnState( TQADrawPrivate *dp, FxBool bTexturing)
{
	UInt32 texOpFlags = dp->tState[kQATag_TextureOp].i;
	
	if ( bTexturing )
	{
		DebugStr(" * SetTexGlideCombinesBasedOnState true ");
    
    	/*
    	 * DECAL
    	 */
    	 
		if (dp->tState[kQATag_TextureOp].i & kQATextureOp_Decal) 
		{
			DebugStr("? Decal \n");
			  				               
#if CUSTOM_GLIDE_STATE_DEF
			dp->currAlphaCombine_function	= GR_COMBINE_FUNCTION_LOCAL;
			dp->currAlphaCombine_factor		= GR_COMBINE_FACTOR_LOCAL;
			dp->currAlphaCombine_local		= GR_COMBINE_LOCAL_ITERATED;
			dp->currAlphaCombine_other		= GR_COMBINE_OTHER_TEXTURE;
#endif
			  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
			                 GR_COMBINE_FACTOR_LOCAL,
			                 GR_COMBINE_LOCAL_ITERATED,
			                 GR_COMBINE_OTHER_TEXTURE,
			                 FXFALSE);
			  
			DebugStr("^^^^ grColorCombine FUNCTION_BLEND FACTOR_TEXTURE_ALPHA LOCAL_ITERATED OTHER_TEXTURE \n");
				
#if CUSTOM_GLIDE_STATE_DEF
			dp->currColorCombineFunction 	= GR_COMBINE_FUNCTION_BLEND;
			dp->currColorCombineFactor		= GR_COMBINE_FACTOR_TEXTURE_ALPHA;
			dp->currColorCombineLocal		= GR_COMBINE_LOCAL_ITERATED;
			dp->currColorCombineOther		= GR_COMBINE_OTHER_TEXTURE;
#endif			

			  grColorCombine(GR_COMBINE_FUNCTION_BLEND,
			                 GR_COMBINE_FACTOR_TEXTURE_ALPHA,
			                 GR_COMBINE_LOCAL_ITERATED,
			                 GR_COMBINE_OTHER_TEXTURE,
			                 FXFALSE);
			  
		}
		/*  Punt on modulate/highlight for now SJL 3/22/00
		else if ( (dp->tState[kQATag_TextureOp].i & kQATextureOp_Modulate) && 
				  (dp->tState[kQATag_TextureOp].i & kQATextureOp_Highlight) )
		{
			DebugStr("Modulate & Highlight \n");
			
			DebugStr("^^^^ grAlphaCombine FUNCTION_SCALE_OTHER FACTOR_LOCAL LOCAL_ITERATED OTHER_TEXTURE \n");
			grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA,
			             GR_COMBINE_FACTOR_ONE,
			             GR_COMBINE_LOCAL_ITERATED,
			             GR_COMBINE_OTHER_TEXTURE,
			             FXFALSE);

			DebugStr("^^^^ grColorCombine FUNCTION_SCALE_OTHER_ADD_LOCAL FACTOR_ONE_MINUS_LOCAL LOCAL_ITERATED OTHER_TEXTURE \n");
			grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
			             GR_COMBINE_FACTOR_LOCAL,
			             GR_COMBINE_LOCAL_ITERATED,
			             GR_COMBINE_OTHER_TEXTURE,
			             FXFALSE);
		}
		*/

		/*
		 * Modulate -- the only mode used in Nanosaur
		 */
		else if ( dp->tState[kQATag_TextureOp].i & kQATextureOp_Modulate) 
		{
			DebugStr("Modulate only \n");
			DebugStr("^^^^ grAlphaCombine FUNCTION_SCALE_OTHER  FACTOR_LOCAL_ALPHA  LOCAL_ITERATED  OTHER_TEXTURE \n");

#if CUSTOM_GLIDE_STATE_DEF
			dp->currAlphaCombine_function	= GR_COMBINE_FUNCTION_BLEND_OTHER;
			dp->currAlphaCombine_factor		= GR_COMBINE_FACTOR_LOCAL;
			dp->currAlphaCombine_local		= GR_COMBINE_LOCAL_ITERATED;
			dp->currAlphaCombine_other		= GR_COMBINE_OTHER_TEXTURE;
#endif
			grAlphaCombine( GR_COMBINE_FUNCTION_BLEND_OTHER,
					        GR_COMBINE_FACTOR_LOCAL,
					        GR_COMBINE_LOCAL_ITERATED,
					        GR_COMBINE_OTHER_TEXTURE,
					        FXFALSE
			);

			DebugStr("^^^^ grColorCombine FUNCTION_BLEND_OTHER  FACTOR_LOCAL  LOCAL_ITERATED  OTHER_TEXTURE \n");

#if CUSTOM_GLIDE_STATE_DEF
			dp->currColorCombineFunction 	= GR_COMBINE_FUNCTION_BLEND_OTHER;
			dp->currColorCombineFactor		= GR_COMBINE_FACTOR_LOCAL;
			dp->currColorCombineLocal		= GR_COMBINE_LOCAL_ITERATED;
			dp->currColorCombineOther		= GR_COMBINE_OTHER_TEXTURE;
#endif
			grColorCombine(	GR_COMBINE_FUNCTION_BLEND_OTHER,
			             	GR_COMBINE_FACTOR_LOCAL,
			             	GR_COMBINE_LOCAL_ITERATED,
			             	GR_COMBINE_OTHER_TEXTURE,
			             	FXFALSE);
		}
		/*
		else if (dp->tState[kQATag_TextureOp].i & kQATextureOp_Highlight)
		{
			DebugStr("Highlight only \n");

			DebugStr("^^^^ grAlphaCombine FUNCTION_SCALE_OTHER FACTOR_ONE LOCAL_NONE OTHER_TEXTURE \n");

#if CUSTOM_GLIDE_STATE_DEF
			dp->currAlphaCombine_function	= GR_COMBINE_FUNCTION_BLEND_OTHER;
			dp->currAlphaCombine_factor		= GR_COMBINE_FACTOR_ONE;
			dp->currAlphaCombine_local		= GR_COMBINE_LOCAL_NONE;
			dp->currAlphaCombine_other		= GR_COMBINE_OTHER_TEXTURE;
#endif
			grAlphaCombine(GR_COMBINE_FUNCTION_BLEND_OTHER,
			             GR_COMBINE_FACTOR_ONE,
			             GR_COMBINE_LOCAL_NONE,
			             GR_COMBINE_OTHER_TEXTURE,
			             FXFALSE);

			DebugStr("^^^^ grColorCombine FUNCTION_SCALE_OTHER_ADD_LOCAL FACTOR_ONE LOCAL_ITERATED OTHER_TEXTURE \n");

#if CUSTOM_GLIDE_STATE_DEF
			dp->currColorCombineFunction 	= GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL;
			dp->currColorCombineFactor		= GR_COMBINE_FACTOR_ONE;
			dp->currColorCombineLocal		= GR_COMBINE_LOCAL_ITERATED;
			dp->currColorCombineOther		= GR_COMBINE_OTHER_TEXTURE;
#endif
			grColorCombine(	GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, 
							GR_COMBINE_FACTOR_ONE,
							GR_COMBINE_LOCAL_ITERATED, 
							GR_COMBINE_OTHER_TEXTURE, FXFALSE);	
		}
		*/
		else		// no lighting
		{
			DebugStr("no lighting for textures \n");
			// 3/21
			if ( dp->currBaseTexture->alphaBits > 0 )
			{

#if CUSTOM_GLIDE_STATE_DEF
				dp->currAlphaCombine_function	= GR_COMBINE_FUNCTION_BLEND_OTHER;
				dp->currAlphaCombine_factor		= GR_COMBINE_FACTOR_ONE;
				dp->currAlphaCombine_local		= GR_COMBINE_LOCAL_NONE;
				dp->currAlphaCombine_other		= GR_COMBINE_OTHER_TEXTURE;
#endif
				// Consider black smoke in combat mission. Transparent edges thru alpha test, but also blend with vertex alpha.
				DebugStr("^^^^ grAlphaCombine BLEND_OTHER, FACTOR_LOCAL, LOCAL_ITERATED, OTHER_TEXTURE \n");
				grAlphaCombine(
					GR_COMBINE_FUNCTION_BLEND_OTHER,
					GR_COMBINE_FACTOR_LOCAL,
					GR_COMBINE_LOCAL_ITERATED,
					GR_COMBINE_OTHER_TEXTURE,
					FXFALSE
				);
			/*
				DebugStr("^^^^ grAlphaCombine BLEND_OTHER, FACTOR_ONE, LOCAL_NONE, OTHER_TEXTURE \n");
				  grAlphaCombine(GR_COMBINE_FUNCTION_BLEND_OTHER,
				                 GR_COMBINE_FACTOR_ONE,
				                 GR_COMBINE_LOCAL_NONE,
				                 GR_COMBINE_OTHER_TEXTURE,
				                 FXFALSE);
				*/
			}
			else
			{
				DebugStr("^^^^ grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, FACTOR_NONE, LOCAL_ITERATED, OTHER_NONE \n");

#if CUSTOM_GLIDE_STATE_DEF
				dp->currAlphaCombine_function	= GR_COMBINE_FUNCTION_LOCAL;
				dp->currAlphaCombine_factor		= GR_COMBINE_FACTOR_NONE;
				dp->currAlphaCombine_local		= GR_COMBINE_LOCAL_ITERATED;
				dp->currAlphaCombine_other		= GR_COMBINE_OTHER_NONE;
#endif		
				  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
				                 GR_COMBINE_FACTOR_NONE,
				                 GR_COMBINE_LOCAL_ITERATED,
				                 GR_COMBINE_OTHER_NONE,
				                 FXFALSE);
			}
			DebugStr("^^^^ grColorCombine FUNCTION_SCALE_OTHER FACTOR_ONE LOCAL_ITERATED OTHER_TEXTURE \n");

#if CUSTOM_GLIDE_STATE_DEF
			dp->currColorCombineFunction 	= GR_COMBINE_FUNCTION_SCALE_OTHER;
			dp->currColorCombineFactor		= GR_COMBINE_FACTOR_ONE;
			dp->currColorCombineLocal		= GR_COMBINE_LOCAL_NONE;
			dp->currColorCombineOther		= GR_COMBINE_OTHER_TEXTURE;
#endif
			  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			                 GR_COMBINE_FACTOR_ONE,
			                 GR_COMBINE_LOCAL_NONE,
			                 GR_COMBINE_OTHER_TEXTURE,
			                 FXFALSE);
		}
	}
	else{
		DebugStr(" * SetTexGlideCombinesBasedOnState no texturing (nop)");
		
	}
bail:
	DebugStr("\n");
}


void SetSecondTexGlideCombinesBasedOnState( TQADrawPrivate *dp )
{
	UInt32 texOpFlags = dp->tState[kQATag_MultiTextureOp].i;
	
	if (!dp->tState[kQATag_MultiTextureEnable].i)
		return;
		
	DebugStr("SetSecondTexGlideCombinesBasedOnState ");
	/*
	kQAMultiTexture_Add			= 0,	// texels are added to form final pixel 
	kQAMultiTexture_Modulate	= 1,	// texels are multiplied to form final pixel 
	kQAMultiTexture_BlendAlpha	= 2,	// texels are blended according to 2nd texel's alpha 
	kQAMultiTexture_Fixed		= 3		// texels are blended by a fixed factor via kQATag_MultiTextureFactor  
	*/
	    /* Would be nice to do this in some nicer way... */
	    
	    switch (dp->tState[kQATag_MultiTextureOp].i) {
		case kQAMultiTexture_Add:
			switch (dp->tState[kQATag_TextureOp].i) {
				case kQATextureOp_None:
				case kQATextureOp_Highlight:
				case kQATextureOp_Decal:
				case kQATextureOp_Shrink:
				
				case kQATextureOp_Modulate:
				      grTexCombine(GR_TMU1,
						   GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
						   GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
						   FXFALSE,FXFALSE);
						  
				      grTexCombine(GR_TMU0,
						   GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,GR_COMBINE_FACTOR_ONE,
						   GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,GR_COMBINE_FACTOR_ONE,
						   FXFALSE,FXFALSE);
				    
				    if (dp->currBaseTexture->alphaBits == 0)
				      grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
						     GR_COMBINE_FACTOR_NONE,
						     GR_COMBINE_LOCAL_ITERATED,
						     GR_COMBINE_OTHER_NONE,
						     FXFALSE);
				    else
				      grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
						     GR_COMBINE_FACTOR_ONE,
						     GR_COMBINE_LOCAL_ITERATED,
						     GR_COMBINE_OTHER_NONE,
						     FXFALSE);

					DebugStr("^^^^ grColorCombine SCALE_OTHER FACTOR_ONE LOCAL_ITERATED OTHER_TEXTURE \n");

				    grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
						   GR_COMBINE_FACTOR_ONE,
						   GR_COMBINE_LOCAL_ITERATED,
						   GR_COMBINE_OTHER_TEXTURE,
						   FXFALSE);
				
					break;
				
			}
			break;
			
		case kQAMultiTexture_Modulate:
			switch (dp->tState[kQATag_TextureOp].i) {
				case kQATextureOp_None:
						grTexCombine(GR_TMU1,
						   GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
						   GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
						   FXFALSE,FXTRUE);
						  
						grTexCombine(GR_TMU0,
						   GR_COMBINE_FUNCTION_BLEND_OTHER,GR_COMBINE_FACTOR_LOCAL,
						   GR_COMBINE_FUNCTION_BLEND_OTHER,GR_COMBINE_FACTOR_LOCAL,
						   FXFALSE,FXFALSE);

						if(dp->currBaseTexture->alphaBits == 0)
							grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
							     GR_COMBINE_FACTOR_NONE,
							     GR_COMBINE_LOCAL_ITERATED,
							     GR_COMBINE_OTHER_NONE,
							     FXFALSE);
						else
							grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
							     GR_COMBINE_FACTOR_ONE,
							     GR_COMBINE_LOCAL_ITERATED,
							     GR_COMBINE_OTHER_NONE,
							     FXFALSE);

						/* I think this is wrong, and shouldn't be doing the multiply... */
						DebugStr("^^^^ grColorCombine SCALE_OTHER FACTOR_ONE LOCAL_ITERATED OTHER_TEXTURE \n");
						grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
						   GR_COMBINE_FACTOR_ONE,
						   GR_COMBINE_LOCAL_ITERATED,
						   GR_COMBINE_OTHER_TEXTURE,
						   FXFALSE);

					break;
				case kQATextureOp_Highlight:
				case kQATextureOp_Decal:
				case kQATextureOp_Shrink:
				
				case kQATextureOp_Modulate:
						grTexCombine(GR_TMU1,
							   GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
							   GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
							   FXFALSE,FXFALSE);
							  
						grTexCombine(GR_TMU0,
							   GR_COMBINE_FUNCTION_BLEND_OTHER,GR_COMBINE_FACTOR_LOCAL,
							   GR_COMBINE_FUNCTION_BLEND_OTHER,GR_COMBINE_FACTOR_LOCAL,
							   FXFALSE,FXFALSE);

					    if ( dp->currBaseTexture->alphaBits == 0)
					      grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
							     GR_COMBINE_FACTOR_NONE,
							     GR_COMBINE_LOCAL_ITERATED,
							     GR_COMBINE_OTHER_NONE,
							     FXFALSE);
					    else
					      grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
							     GR_COMBINE_FACTOR_ONE,
							     GR_COMBINE_LOCAL_ITERATED,
							     GR_COMBINE_OTHER_NONE,
							     FXFALSE);

						DebugStr("^^^^ grColorCombine SCALE_OTHER FACTOR_ONE LOCAL_ITERATED OTHER_TEXTURE \n");

					    grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
							   GR_COMBINE_FACTOR_ONE,
							   GR_COMBINE_LOCAL_ITERATED,
							   GR_COMBINE_OTHER_TEXTURE,
							   FXFALSE);
								
					break;
				

			}
			break;
		case kQAMultiTexture_BlendAlpha:
			switch (dp->tState[kQATag_TextureOp].i) {
				case kQATextureOp_None:
				
					break;
				case kQATextureOp_Modulate:
				
					break;
				case kQATextureOp_Highlight:
				
					break;
				case kQATextureOp_Decal:
				
					break;
				case kQATextureOp_Shrink:
			
					break;
			}
			break;
		case kQAMultiTexture_Fixed:
			switch (dp->tState[kQATag_TextureOp].i) {
				case kQATextureOp_None:
				
					break;
				case kQATextureOp_Modulate:
				
					break;
				case kQATextureOp_Highlight:
				
					break;
				case kQATextureOp_Decal:
				
					break;
				case kQATextureOp_Shrink:
			
					break;
			}
			break;
	    }
}

void SetTexWrapClamp( TQADrawPrivate *dp )
{
	GrTextureClampMode_t uMode, vMode;
	
  	DebugStr("Tex Wrap: ");
  	
  	if( dp->tState[kQATagGL_TextureWrapU].i == kQAGL_Clamp )
  	{
  		uMode = GR_TEXTURECLAMP_CLAMP;
  		DebugStr("Clamp ");
  	} 
  	else 
  	{
  		uMode = GR_TEXTURECLAMP_WRAP;
  		DebugStr("Wrap ");
  	}
  	
  	
  	if( dp->tState[kQATagGL_TextureWrapV].i == kQAGL_Clamp )
  	{
  		vMode = GR_TEXTURECLAMP_CLAMP;
  		DebugStr("Clamp ");
  	} 
  	else 
  	{
  		vMode = GR_TEXTURECLAMP_WRAP;
  		DebugStr("Wrap ");
  	}
	grTexClampMode( GR_TMU0, uMode, vMode);
}


void SetBaseTextureFilter( TQADrawPrivate *dp )
{
	GrTextureFilterMode_t	rave_min, rave_mag, gl_min, gl_mag;
	
	DebugStr(" SetBaseTextureFilter: grTexMipMapMode(GR_TMU0, GR_MIPMAP_NEAREST, FXFALSE) ");
	// alternative is GR_MIPMAP_NEAREST_DITHER; didn't make a diff in Kawasaki, which needs it.
	grTexMipMapMode(GR_TMU0, GR_MIPMAP_NEAREST, FXFALSE);
    /*
     *		RAVE texture filter settings
     *      kQATextureFilter_Fast   - pick nearest
     *      kQATextureFilter_Mid    - bilinear
     *      kQATextureFilter_Best   - trilinear
     */ 
    switch( dp->tState[kQATag_TextureFilter].i )
    {
        case kQATextureFilter_Mid:
        	rave_min = GR_TEXTUREFILTER_BILINEAR;
        	rave_mag = GR_TEXTUREFILTER_BILINEAR;
            break;

        case kQATextureFilter_Best:
        	rave_min = GR_TEXTUREFILTER_BILINEAR;
        	rave_mag = GR_TEXTUREFILTER_BILINEAR;
        	break;
            	
        case kQATextureFilter_Fast:
       	default:
        	rave_min = GR_TEXTUREFILTER_POINT_SAMPLED;
        	rave_mag = GR_TEXTUREFILTER_POINT_SAMPLED;
            break;
    }
     /*
     *	OpenGL texture filter settings
	 */
	switch( dp->tState[kQATagGL_TextureMinFilter].i )
	{
		case kQAGL_Nearest:			gl_min = GR_TEXTUREFILTER_POINT_SAMPLED;	break;	
		case kQAGL_Linear:			gl_min = GR_TEXTUREFILTER_BILINEAR;			break;
		default:					gl_min = GR_TEXTUREFILTER_BILINEAR;			break;
	}

	switch( dp->tState[kQATagGL_TextureMagFilter].i )
	{
		case kQAGL_Nearest:			gl_mag = GR_TEXTUREFILTER_POINT_SAMPLED;	break;	
		case kQAGL_Linear:			gl_mag = GR_TEXTUREFILTER_BILINEAR;			break;
		default:					gl_mag = GR_TEXTUREFILTER_BILINEAR;			break;
	}
			
	/*
	 * use highest settings of RAVE and OpenGL
	 */
	rave_min		= max( rave_min, gl_min );
	rave_mag		= max( rave_mag, gl_mag );
	DebugStr(" rave_min="); DebugNum(rave_min); DebugStr(" rave_mag="); DebugNum(rave_mag);
	DebugStr("\n");
	grTexFilterMode(GR_TMU0, rave_min, rave_mag );
}

void SetMultiTextureFilter( TQADrawPrivate *dp )
{
	grTexMipMapMode(GR_TMU1, GR_MIPMAP_DISABLE, FXFALSE);	// 3/15 
	
    switch( dp->tState[kQATag_MultiTextureFilter].i )
    {
        case kQATextureFilter_Mid:
			grTexFilterMode(GR_TMU1, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
            break;

        case kQATextureFilter_Best:
			grTexFilterMode(GR_TMU1, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
        	break;
            	
        case kQATextureFilter_Fast:
       	default:
			// grTexFilterMode(GR_TMU0, GR_TEXTURE_POINT_SAMPLED, GR_TEXTURE_POINT_SAMPLED);
			// bilinear is just as fast as point sampled.
			grTexFilterMode(GR_TMU1, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
            break;
    }
}

float CalcFogNone(float invW, float a, TQADrawPrivate* dp)
{
#pragma unused( invW )
#pragma unused( dp )

	return 1.0;
}

static float CalcFogAlpha(float invW, float vAlpha, TQADrawPrivate* dp)
{
	return vAlpha;
}

static float CalcFogLinear(float invW, float vAlpha, TQADrawPrivate* dp)
{
	float fog;
	float depth = 1.0 / invW;
	float fog_start 	= dp->tState[kQATag_FogStart].f;
	float fog_end 		= dp->tState[kQATag_FogEnd].f;
	fog = (fog_end - depth) / (fog_end - fog_start);
	if(fog < 0.0) return 0.0;
	if(fog > 1.0) return 1.0;
	return fog;
}

static float CalcFogExp(float invW, float vAlpha, TQADrawPrivate* dp)
{
	float fog;
	float depth = 1.0 / invW;
	float fog_density	= dp->tState[kQATag_FogDensity].f/dp->tState[kQATag_FogEnd].f;
	
	if (fog_density > 1.0)	fog_density = 1.0;
	
	fog = exp(-fog_density * depth);
	return fog;
}

static float CalcFogExp2(float invW, float vAlpha, TQADrawPrivate* dp)
{
	float fog;
	float depth = 1.0 / invW;
	float fog_density	= dp->tState[kQATag_FogDensity].f/dp->tState[kQATag_FogEnd].f;
	
	if (fog_density > 1.0)	fog_density = 1.0;
	
	fog = exp(-fog_density * depth * fog_density * depth);
	return fog;
}


void SetFog( TQADrawPrivate *dp )
{
	// GR_PARAM_FOG_EXT			see p.176  f/q in range {0..1}
	// when using fog mode GR_FOG_WITH_TABLE_ON_FOGCOORD_EXT
	
	// Fog unit is separate from the alpha blending unit.
	
	GrColor_t 		fogcolor;
	long			numFogTableEntries = 0;
	float			kMaxFogW = 52428.8;
	int					i;
	int fog_mode 		= dp->tState[kQATag_FogMode].i;
	float fog_start 	= dp->tState[kQATag_FogStart].f;
	float fog_den		= dp->tState[kQATag_FogDensity].f;
	float fog_end 		= dp->tState[kQATag_FogEnd].f;
	float fog_maxdepth = dp->tState[kQATag_FogMaxDepth].f;
	
	DebugStr("@@ SetFog @@ ");
	
	if (fog_mode == kQAFogMode_None)
	{
#if VARI_VERT_LAYOUT
		grVertexLayout(GR_PARAM_Q,   12, GR_PARAM_DISABLE);
#endif
		dp->fogFunc = CalcFogNone;
		if (dp->gFogTable)
			FreePtr((void *)dp->gFogTable);
		dp->gFogTable = NULL;
		grFogMode(GR_FOG_DISABLE);
		DebugStr("(None) ");
		return;
	}
	
#if VARI_VERT_LAYOUT
	grVertexLayout(GR_PARAM_Q,   12, GR_PARAM_ENABLE);
#endif
		
	grGet( GR_FOG_TABLE_ENTRIES, 4, &numFogTableEntries);
	if (dp->gFogTable == NULL)
	{
		dp->gFogTable = (GrFog_t *) AllocPtr(numFogTableEntries * sizeof(GrFog_t));
		if (dp->gFogTable == NULL)
		{
			grFogMode(GR_FOG_DISABLE);
			DebugStr(" ?? --------------->    out of memory for FogTable \n");
			return;
		}

		for(i = 0; i < numFogTableEntries; i++){		// flat ramp table
			dp->gFogTable[i] = 255.0 - 255.0 / guFogTableIndexToW(i);
		}
	}	
	
	// avoid recalc of fog table if possible
	if ( dp->tFogChangeMask & ( STATE_FOG_COLOR | STATE_FOG_START | STATE_FOG_END | STATE_FOG_DENSITY | STATE_FOG_MAX_DEPTH ) )
	{
		assert ( (fog_mode >= kQAFogMode_Alpha) && (fog_mode <= kQAFogMode_ExponentialSquared));
		DebugStr( "\nSetFog mode "); DebugNum( fog_mode );
		DebugStr( "Fog Start="); DebugFloat( fog_start );
		DebugStr( "Fog End="); DebugFloat( fog_end );
		
		switch(fog_mode)
		{
			case kQAFogMode_Linear:
				dp->fogFunc = CalcFogLinear;
				break;
			case kQAFogMode_Exponential:
				dp->fogFunc = CalcFogExp;
				break;
			case kQAFogMode_ExponentialSquared:
				dp->fogFunc = CalcFogExp2;
				break;
			case kQAFogMode_Alpha:
				dp->fogFunc = CalcFogAlpha;
				break;
		}
		
	after_modes:

		fogcolor = 0xFF000000;
		fogcolor |= ((UInt32)((dp->tState[kQATag_FogColor_r].f * 255.0)+0.5) & 0x00007FFF) << 16;
		fogcolor |= ((UInt32)((dp->tState[kQATag_FogColor_g].f * 255.0)+0.5) & 0x00007FFF) << 8;
		fogcolor |= ((UInt32)((dp->tState[kQATag_FogColor_b].f * 255.0)+0.5) & 0x00007FFF);
		
		DebugStr(" fog color "); DebugHex(fogcolor);
		// DebugStr(" forcing to white ");
		// fogcolor = 0xFFFFFFFF;
		grFogTable(dp->gFogTable);
		grFogColorValue(fogcolor);
		grFogMode(GR_FOG_WITH_TABLE_ON_Q);
		DebugStr("\n");
	}
}

void SetSecond_Texture( TQADrawPrivate *dp )
{
	#pragma unused( dp )
	
}

void SetCompositing( TQADrawPrivate *dp, FxBool which )
{
#pragma unused( which )
#pragma unused( dp )
	// (simple)
}

void SetRenderStateComposite( TQADrawPrivate *dp, Boolean bTexturing )
{
#pragma unused(bTexturing)

	if( dp->tTex2ChangeMask & STATE_COMP_TEX )
		SetAndLoadTextureMap(dp);
	
	if( dp->tTex2ChangeMask & STATE_COMP_MODE )
   		SetCompositing( dp, dp->tState[kQATag_MultiTextureEnable].i );
	
	if( dp->tTex2ChangeMask & STATE_COMP_TEX_FILTER )
	{
		SetMultiTextureFilter(dp);
	}
		 	
	if( dp->tTex2ChangeMask & STATE_COMP_TEX_OP )
		SetSecondTexGlideCombinesBasedOnState(dp);

	if( dp->tTex2ChangeMask & STATE_COMP_TEX_MIN )
		NULL;
    if( dp->tTex2ChangeMask & STATE_COMP_TEX_MAG )
    	NULL;
			
	if( (dp->tTex2ChangeMask & STATE_COMP_TEX_WRAPU) || (dp->tTex2ChangeMask & STATE_COMP_TEX_WRAPV) )
	{
	  	if( dp->tState[kQATag_MultiTextureWrapU].i == kQAGL_Clamp )
	  		if ( dp->tState[kQATag_MultiTextureWrapV].i == kQAGL_Clamp )
		  		grTexClampMode( GR_TMU1, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
		  	else
		  		grTexClampMode( GR_TMU1, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_WRAP);
		  		
	  	else if( dp->tState[kQATag_MultiTextureWrapU].i == kQAGL_Repeat )
	  		if ( dp->tState[kQATag_MultiTextureWrapV].i == kQAGL_Clamp )
		  		grTexClampMode( GR_TMU1, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_CLAMP);
		  	else
		  		grTexClampMode( GR_TMU1, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);
	}

	if( dp->tTex1ChangeMask & STATE_COMP_GL_BORDER_COLOR )
	{ /* not supported */ }
	
	dp->tTex2ChangeMask = 0;
}


void SetOpenGLBlending( TQADrawPrivate *dp )
{
	GrAlphaBlendFnc_t rgb_sf;
	GrAlphaBlendFnc_t rgb_df;
	GrAlphaBlendFnc_t alpha_sf;
	GrAlphaBlendFnc_t alpha_df;
	
	switch( dp->tState[kQATagGL_BlendSrc].i)
	{
		case GL_ZERO:
			rgb_sf = GR_BLEND_ZERO;
			alpha_sf = GR_BLEND_ZERO;
		break;
		case GL_ONE:
			rgb_sf = GR_BLEND_ONE;
			alpha_sf = GR_BLEND_ONE;
		break;
		case GL_DST_COLOR:
			rgb_sf = GR_BLEND_DST_COLOR;
			alpha_sf = GR_BLEND_DST_COLOR;
		break;
		case GL_ONE_MINUS_DST_COLOR:
			rgb_sf = GR_BLEND_ONE_MINUS_DST_COLOR;
			alpha_sf = GR_BLEND_ONE_MINUS_DST_COLOR;
		break;
		case GL_SRC_ALPHA_SATURATE:
			// Alpha dest is not support with depth buffer
			// rgb_sf = GR_BLEND_ALPHA_SATURATE;
			// alpha_sf = GR_BLEND_ALPHA_SATURATE;
			
			rgb_sf = GR_BLEND_ONE;
			alpha_sf = GR_BLEND_ONE;
		break;
		case GL_SRC_ALPHA:
			rgb_sf = GR_BLEND_SRC_ALPHA;
			alpha_sf = GR_BLEND_SRC_ALPHA;
		break;
		case GL_ONE_MINUS_SRC_ALPHA:
			rgb_sf = GR_BLEND_ONE_MINUS_SRC_ALPHA;
			alpha_sf = GR_BLEND_ONE_MINUS_SRC_ALPHA;
		break;
		case GL_DST_ALPHA:
			// Alpha dest is not support with depth buffer
			// rgb_sf = GR_BLEND_DST_ALPHA;
			// alpha_sf = GR_BLEND_DST_ALPHA;
			
			rgb_sf = GR_BLEND_ONE;
			alpha_sf = GR_BLEND_ONE;
		break;
		default: /*case GL_ONE_MINUS_DST_ALPHA:*/
			// Alpha dest is not support with depth buffer
			// rgb_sf = GR_BLEND_ONE_MINUS_DST_ALPHA;
			// alpha_sf = GR_BLEND_ONE_MINUS_DST_ALPHA;
			
			rgb_sf = GR_BLEND_ONE;
			alpha_sf = GR_BLEND_ONE;
		break;
	}
	
	switch( dp->tState[kQATagGL_BlendDst].i)
	{
		case GL_ZERO:
			rgb_df = GR_BLEND_ZERO;
			alpha_df = GR_BLEND_ZERO;
		break;
		case GL_ONE:
			rgb_df = GR_BLEND_ONE;
			alpha_df = GR_BLEND_ONE;
		break;
		case GL_SRC_COLOR:
			rgb_df = GR_BLEND_SRC_COLOR;
			alpha_df = GR_BLEND_SRC_COLOR;
		break;
		case GL_ONE_MINUS_SRC_COLOR:
			rgb_df = GR_BLEND_ONE_MINUS_SRC_COLOR;
			alpha_df = GR_BLEND_ONE_MINUS_SRC_COLOR;
		break;
		case GL_SRC_ALPHA:
			rgb_df = GR_BLEND_SRC_ALPHA;
			alpha_df = GR_BLEND_SRC_ALPHA;
		break;
		case GL_ONE_MINUS_SRC_ALPHA:
			rgb_df = GR_BLEND_ONE_MINUS_SRC_ALPHA;
			alpha_df = GR_BLEND_ONE_MINUS_SRC_ALPHA;
		break;
		case GL_DST_ALPHA:
			// Alpha dest is not support with depth buffer
			// rgb_df = GR_BLEND_DST_ALPHA;
			// alpha_df = GR_BLEND_DST_ALPHA;
			
			rgb_df = GR_BLEND_ZERO;
			alpha_df = GR_BLEND_ZERO;
		break;
		default: /*case GL_ONE_MINUS_DST_ALPHA:*/
			// Alpha dest is not support with depth buffer
			// rgb_df = GR_BLEND_ONE_MINUS_DST_ALPHA;
			// alpha_df = GR_BLEND_ONE_MINUS_DST_ALPHA;
			
			rgb_df = GR_BLEND_ZERO;
			alpha_df = GR_BLEND_ZERO;
		break;
	}
	dp->rgb_sf = rgb_sf;
	dp->rgb_df = rgb_df;
	DebugStr(" OpenGL blend "); DebugNum(rgb_sf); DebugNum(rgb_df);
	grAlphaBlendFunction(rgb_sf, rgb_df, GR_BLEND_ONE, GR_BLEND_ZERO);
}
	
void SetDepthBufferMode( TQADrawPrivate *dp )
{
	UInt32 currZFunc = 			dp->tState[kQATag_ZFunction].i;
	
	if (currZFunc == kQAZFunction_None)
	{
		// dp->glDepthFunc = GR_CMP_ALWAYS;
		DebugStr("DepthBuffer Disabled ");
		grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
	}
	else
	{
		DebugStr("DepthBuffer Enabled ");
		grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
	}
	DebugStr("\n");
	
	return;
}

void SetAlphaSorting( TQADrawPrivate *dp )
{
   
    if ((dp->tState[kQATag_ZSortedHint].i == 1) && 
        (dp->tState[kQATag_ZFunction].i != kQAZFunction_None ))
    	dp->bSortingAlphaTri = 1;
    else
    	dp->bSortingAlphaTri = 0;
}

static UInt32 IntersectingRects(Rect *r1, Rect *r2)
{
	SInt32 t, l, r, b, w, h;
	
	/* Init rect 1 edges */
	t = r1->top;
	l = r1->left;
	r = r1->right;
	b = r1->bottom;
	
	/* Trim to rect 2 */
	if(t < r2->top)    t = r2->top;
	if(b > r2->bottom) b = r2->bottom;
	if(l < r2->left)   l = r2->left;
	if(r > r2->right)  r = r2->right;
	
	h = b - t;
	w = r - l;
	
    /* Minor mod, return intersecting area */
	if(w > 0 && h > 0) return w*h;
	
	return 0;
}


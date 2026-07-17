/*________________________________________________________________________________________
** 
** Copyright (c) 1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
**________________________________________________________________________________________
**
**
** 
**
*/

#include <string.h> /* for memset */
#include <Windows.h>
#include <QuickDraw.h>
#include <Devices.h>
#include <Displays.h>
#include <Video.h>
#include <Events.h>
#include "r3Tweaks.h"
#include "r3Core.h"


extern TRvInfo gRvEngInfo;

Boolean tnsl_GlideInitialized = false;
Boolean gls_pix_ext = false;
Boolean gls_combine_ext = false;
Boolean tnsl_32bit_texture = false;
Boolean gls_2k_texture_size = false;
Boolean tnsl_uma_texture = false;
Boolean tnsl_CMDTransport = false;
Boolean tnsl_surface_ext = false;

/* Glide Extensions */
grSurfaceCreateContextFuncPtr grSurfaceCreateContextExt = NULL;
grSurfaceReleaseContextFuncPtr grSurfaceReleaseContextExt = NULL;
grSurfaceSetRenderingSurfaceFuncPtr grSurfaceSetRenderingSurfaceExt = NULL;
grSurfaceCalcTextureWHDFuncPtr grSurfaceCalcTextureWHDExt = NULL;
grSurfaceSetAuxSurfaceFuncPtr grSurfaceSetAuxSurfaceExt = NULL;
grSurfaceSetTextureSurfaceFuncPtr grSurfaceSetTextureSurfaceExt = NULL;
grDeviceQueryFuncPtr grDeviceQueryExt = NULL;

grSstWinOpenFuncPtr grSstWinOpenExt = NULL;
grColorMaskFuncPtr grColorMaskExt = NULL;
grStencilFuncPtr grStencilFuncExt = NULL;
grStencilMaskFuncPtr grStencilMaskExt = NULL;
grStencilOpFuncPtr grStencilOpExt = NULL;
grBufferClearFuncPtr grBufferClearExt = NULL;
grLfbConstantStencilFuncPtr grLfbConstantStencilExt = NULL;
grTBufferWriteMaskFuncPtr grTBufferWriteMaskExt = NULL;
grColorCombineFuncPtr grColorCombineExt = NULL;
grAlphaCombineFuncPtr grAlphaCombineExt = NULL;
grTexColorCombineFuncPtr grTexColorCombineExt = NULL;
grTexAlphaCombineFuncPtr grTexAlphaCombineExt = NULL;

grCommandTransportInfoPtr grCommandTransportInfoExt = NULL;
grCommandTransportMakeRoomPtr grCommandTransportMakeRoomExt = NULL;

/* MacOS specific Glide Extensions */
grSurfaceCreateFuncPtr grSurfaceCreateExt = NULL;
grSurfaceReleaseFuncPtr grSurfaceReleaseExt = NULL;
grSurfaceGetDescFuncPtr grSurfaceGetDescExt = NULL;

/* Device List */
GrDeviceInfo_t rvDeviceList[MAX_GLIDE_DEVICES];
extern FxU32 rvDeviceListCount;

/* local definition */

void    		SetupGlideContext( TQADrawPrivate * inContext);
//+ GrScreenResolution_t    glrGetGlideResolution( const GLsizei inWidth, const GLsizei inHeight);
GrScreenRefresh_t 	GetGlideRefresh( TQADrawPrivate	*dp, const GrScreenResolution_t);
GrPixelFormat_t		RvGetGlidePixelFormat( UInt32 inColorMode );

#ifdef DEBUG_VERBOSE
#undef DEBUG_VERBOSE
#endif
#define DEBUG_VERBOSE( s, t )
#define DEBUG_ENTRY( p )

/*
________________________________________________________________________________________

      InitializeGlide
________________________________________________________________________________________

*/

#define containstr( src, target ) \
  ( strstr( src, target ) != NULL) 
  
UInt32
InitializeGlide()
{
	const char *theExtension;
	SInt32	numBoards = 0;

  if((long)grGlideInit == (long)kUnresolvedCFragSymbolAddress) 
  {
    //+ glr_debug_printf("Glide Library not present.\n");
		return 1;
	}

	// Do glide stuff	
	grGlideInit();
	
  // Get extensions we want to use.
  theExtension = grGetString( GR_EXTENSION );
  
  if ( containstr( theExtension, " SURFACE " ) )
  {
	grSurfaceCreateContextExt = (void *)grGetProcAddress("grSurfaceCreateContextExt");
	grSurfaceReleaseContextExt = (void *)grGetProcAddress("grSurfaceReleaseContextExt");
	grSurfaceSetRenderingSurfaceExt = (void *)grGetProcAddress("grSurfaceSetRenderingSurfaceExt");
	grSurfaceCalcTextureWHDExt = (void *)grGetProcAddress("grSurfaceCalcTextureWHDExt");
	grSurfaceSetAuxSurfaceExt = (void *)grGetProcAddress("grSurfaceSetAuxSurfaceExt");
	grSurfaceSetTextureSurfaceExt = (void *)grGetProcAddress("grSurfaceSetTextureSurfaceExt");
	grSurfaceCreateExt = (void *)grGetProcAddress("grSurfaceCreateExt");
	grSurfaceReleaseExt = (void *)grGetProcAddress("grSurfaceReleaseExt");
	grSurfaceGetDescExt = (void *)grGetProcAddress("grSurfaceGetDescExt");
	grDeviceQueryExt = (void *)grGetProcAddress("grDeviceQueryExt");

	if(grSurfaceCreateContextExt && grSurfaceReleaseContextExt &&
       grSurfaceSetRenderingSurfaceExt && grSurfaceCalcTextureWHDExt &&
       grSurfaceSetAuxSurfaceExt && grSurfaceSetTextureSurfaceExt &&
       grSurfaceCreateExt && grSurfaceReleaseExt && 
       grSurfaceGetDescExt && grDeviceQueryExt) {
      tnsl_surface_ext = FXTRUE;
    }
       
  }

  if ( containstr( theExtension, " PIXEXT " ) )
  {
    grSstWinOpenExt = (void *)grGetProcAddress("grSstWinOpenExt");
    grColorMaskExt = (void *)grGetProcAddress("grColorMaskExt");
    grStencilFuncExt = (void *)grGetProcAddress("grStencilFuncExt");
    grStencilMaskExt = (void *)grGetProcAddress("grStencilMaskExt");
    grStencilOpExt = (void *)grGetProcAddress("grStencilOpExt");
    grBufferClearExt = (void *)grGetProcAddress("grBufferClearExt");
    grLfbConstantStencilExt = (void *)grGetProcAddress("grLfbConstantStencilExt");
    grTBufferWriteMaskExt = (void *)grGetProcAddress("grTBufferWriteMaskExt");

    if (  grSstWinOpenExt && grColorMaskExt && grStencilFuncExt && grStencilMaskExt
          && grStencilOpExt && grBufferClearExt && grLfbConstantStencilExt
          && grTBufferWriteMaskExt )
    {
		DEBUG_VERBOSE( InitializeGlide, "pix extensions are available...\n" );
        gls_pix_ext = FXTRUE;
    }
  }
  
  
  if ( containstr( theExtension, " COMBINE " ) )
  {
    grColorCombineExt = (void *)grGetProcAddress("grColorCombineExt");
    grAlphaCombineExt = (void *)grGetProcAddress("grAlphaCombineExt");
    grTexColorCombineExt = (void *)grGetProcAddress("grTexColorCombineExt");
    grTexAlphaCombineExt = (void *)grGetProcAddress("grTexAlphaCombineExt");

    if (  grColorCombineExt && grAlphaCombineExt && grTexColorCombineExt && grTexAlphaCombineExt )
    {
		DEBUG_VERBOSE( InitializeGlide, "combine extensions are available...\n" );
        gls_combine_ext = true;
    }
  }  


  if ( containstr( theExtension, " TEXFMT " ) )
  {
    tnsl_32bit_texture = gls_2k_texture_size = true;
	DEBUG_VERBOSE( InitializeGlide, "textures extensions are available...\n" );
  }  
  
 
  if ( containstr( theExtension, " TEXUMA " ) )
  {
    tnsl_uma_texture = true;
	DEBUG_VERBOSE( InitializeGlide, "uma textures are available...\n" );
  }  

  if ( containstr( theExtension, " COMMAND_TRANSPORT " ) )
  {
    tnsl_CMDTransport = true;
    grCommandTransportInfoExt = (void *)grGetProcAddress("grCommandTransportInfoExt2");
    grCommandTransportMakeRoomExt = (void *)grGetProcAddress("grCommandTransportMakeRoomExt2");
    //+ DEBUG_VERBOSE( glrInitializeGlide, "command_transport is available...\n");
  }

if (!ValidateGlideEnvironment())
	return 1;
	
	tnsl_GlideInitialized = true;
	
	return noErr;
}



UInt32 ValidateGlideEnvironment(void)
{
  
  /* Returns the # of glide compatible devices in the system. If devList
	 * is non-NULL and listCount is non-zero, devList is filled in w/
	 * information about the current glide device to system device
	 * mapping. If there is not enough space in the client passed list to
	 * fill in all of the device information a partial list is returned to
	 * the client along w/ the full count.  
	 */

	rvDeviceListCount = grDeviceQueryExt(rvDeviceList, MAX_GLIDE_DEVICES);
 
	return rvDeviceListCount;
}

/*
________________________________________________________________________________________

      InitializeWindowedGlideRenderingSurface
________________________________________________________________________________________

*/

UInt32 InitializeWindowedGlideRenderingSurface(
	TQADrawContext *	inContext,
	long				inWidth,
	long				inHeight,
	long				inRenderDepth)
{
	UInt32				theResult = noErr;
	
	// DEBUG_ENTRY( InitializeWindowedGlideRenderingSurface );	
	TQADrawPrivate *dp = inContext->drawPrivate;

	//+ SetupGlideSurfaceBlit is called before this.
	
	if ( !tnsl_GlideInitialized ) InitializeGlide();
	
	assert( inRenderDepth == 16 || inRenderDepth == 32 || inRenderDepth == 8 );
	
	dp->renderPort = NULL;
	dp->renderSurface = NULL;
    dp->auxSurface = NULL;

	if(dp->glideBoardSelectID == -1)
	{
		// DEBUG_ERROR( InitializeWindowedGlideRenderingSurface, "Can't render on target device.\n" );
		goto ErrorExit;
	}
	
    grSstSelect( dp->glideBoardSelectID );

	dp->glideContext = grSurfaceCreateContextExt( GR_SURFACECONTEXT_WINDOWED );
	if ( dp->glideContext == NULL )
	{
		// DEBUG_VERBOSE( InitializeWindowedGlideRenderingSurface, "No glide context could be created...\n" );
		goto ErrorExit;
	}	
	
    if( grSelectContext( dp->glideContext ) )
    {

		GrSurfaceDesc_t		theSfcDesc;

		// DEBUG_VERBOSE( InitializeWindowedGlideRenderingSurface, "glide windowed context created...\n" );

		/* initialize the rendering surface */
		memset( &theSfcDesc,0,sizeof(theSfcDesc) );
		theSfcDesc.width = inWidth;
		theSfcDesc.height = inHeight;
		if (NAPALM_AND_BEYOND)
			theSfcDesc.bytesPerPixel = inRenderDepth >> 3;
		else
			theSfcDesc.bytesPerPixel = 2;	// old voodoo was always 16-bit rendering
	    theSfcDesc.notifyCallback = RvSurfaceNotify;		//+ TBFL
	    theSfcDesc.userData = inContext;

		dp->renderSurface = grSurfaceCreateExt( &theSfcDesc );

		if (dp->renderSurface == NULL)
		{
			// blow away texture surface to get more memory
			grSurfaceSetTextureSurfaceExt(GR_TMU0,0);
			grSurfaceSetTextureSurfaceExt(GR_TMU1,0);
			grSurfaceReleaseExt(gRvEngInfo.texture_surface);
			gRvEngInfo.texture_surface = 0;
			gRvEngInfo.surface_size = 0;
			dp->renderSurface = grSurfaceCreateExt( &theSfcDesc );
		}

		if( dp->renderSurface )
		{
			// DEBUG_VERBOSE( InitializeWindowedGlideRenderingSurface, "glide rendering surface allocated...\n" );

			grSurfaceSetRenderingSurfaceExt( dp->renderSurface );
			grSurfaceGetDescExt( dp->renderSurface, &dp->renderSurfaceDesc );
			dp->renderPort = dp->renderSurfaceDesc.systemPortId;
			assert( dp->renderPort);
			DebugStr("\nrenderPort set to "); DebugHex((int)dp->renderPort);
			assert( dp->renderPort->portPixMap );
			assert( (*dp->renderPort->portPixMap)->baseAddr );
			
			gRvEngInfo.VRAMusedForNonTextures += dp->renderSurfaceDesc.pitch * dp->renderSurfaceDesc.height;
			/* the gdhandle is also passed */

			if(dp->z_bits > 0) {
				/* initialize the rendering surface */
				memset( &theSfcDesc,0,sizeof(theSfcDesc) );
				theSfcDesc.width = inWidth;
				theSfcDesc.height = inHeight;
				if (NAPALM_AND_BEYOND)
				{
					if (inRenderDepth == 32 ) 	// && dp->z_bits > 16)
						theSfcDesc.bytesPerPixel = 4;
					else
					{
						theSfcDesc.bytesPerPixel = 2;
						dp->z_bits = 16;
					}
				}
				else
					theSfcDesc.bytesPerPixel = 2;

                theSfcDesc.notifyCallback = RvSurfaceNotify;
	            theSfcDesc.userData = inContext;

				dp->auxSurface = grSurfaceCreateExt( &theSfcDesc );
				if (dp->auxSurface == NULL)
				{
					// blow away texture surface to get more memory
					grSurfaceSetTextureSurfaceExt(GR_TMU0,0);
					grSurfaceSetTextureSurfaceExt(GR_TMU1,0);
					grSurfaceReleaseExt(gRvEngInfo.texture_surface);
					gRvEngInfo.texture_surface = 0;
					gRvEngInfo.surface_size = 0;
					dp->auxSurface = grSurfaceCreateExt( &theSfcDesc );
				}

				if( dp->auxSurface )
				{   
				    grSurfaceGetDescExt(dp->auxSurface, &dp->auxSurfaceDesc);
			        grSurfaceSetAuxSurfaceExt(dp->auxSurface);
        			gRvEngInfo.VRAMusedForNonTextures += dp->auxSurfaceDesc.pitch * dp->auxSurfaceDesc.height;

			    }
			}

			SetupGlideContext( dp );
			
			theResult = 0;
		}
		else
		{
			// DEBUG_VERBOSE( InitializeWindowedGlideRenderingSurface, "glide surface for the window could not be allocated...\n" );
			// DEBUG_VERBOSE( InitializeWindowedGlideRenderingSurface, "   - theSfcDesc.width = %d ; height = %d ; bytesPerPixel = %d\n", theSfcDesc.width, theSfcDesc.height, theSfcDesc.bytesPerPixel  );
			goto ErrorExit;
		}
	}
	else
	{
		// DEBUG_VERBOSE( InitializeWindowedGlideRenderingSurface, "glide context could not be selected...\n" );
		goto ErrorExit;
	}

	assert( dp->renderPort );
	assert( dp->renderSurface );
    assert( ((dp->z_bits == 0) && (dp->auxSurface == NULL)) || ((dp->z_bits > 0) && dp->auxSurface != NULL));


ErrorExit:

	return theResult;
}

GrScreenRefresh_t
GetGlideRefresh( TQADrawPrivate	*dp, const GrScreenResolution_t)
{
	GrScreenRefresh_t			theRate = GR_REFRESH_NONE;
	GLsizei				        inFreq;
	
	/* Try to determine frequency automatically based on the graphics
	   device the window is on. */
	  GDHandle mainDeviceHandle;
	  short mainDeviceRefNum;
	  ParamBlockRec theParamBlock;
	  VDResolutionInfoRec resolutionInfo;
	  OSErr theSuccess;
	  
      /* Find device for our window */
      mainDeviceHandle = dp->device.device.gDevice;
      mainDeviceRefNum = (*mainDeviceHandle)->gdRefNum;
           
      memset(&theParamBlock,0,sizeof(theParamBlock));
      theParamBlock.cntrlParam.ioCompletion = 0;
      theParamBlock.cntrlParam.ioVRefNum = 0;
      theParamBlock.cntrlParam.ioCRefNum = mainDeviceRefNum;
      *((void **) &(theParamBlock.cntrlParam.csParam)) = &resolutionInfo;
      resolutionInfo.csPreviousDisplayModeID = kDisplayModeIDCurrent;
      theParamBlock.cntrlParam.csCode = cscGetNextResolution;

      theSuccess = (OSErr) PBStatusSync(&theParamBlock);
      if(theSuccess == noErr) {
        /* Round up resolution to next integer value */
        inFreq = (resolutionInfo.csRefreshRate + 32768) >> 16;
	  } else {
	    inFreq = 60;
	  }  
	
	if ( inFreq <= 60 )
		theRate = GR_REFRESH_60Hz;
	else if ( inFreq <= 70 )
		theRate = GR_REFRESH_70Hz;
	else if ( inFreq <= 72 )
		theRate = GR_REFRESH_72Hz;
	else if ( inFreq <= 75 )
		theRate = GR_REFRESH_75Hz;
	else if ( inFreq <= 80 )
		theRate = GR_REFRESH_80Hz;
	else if ( inFreq <= 85 )
		theRate = GR_REFRESH_85Hz;
	else if ( inFreq <= 90 )
		theRate = GR_REFRESH_90Hz;
	else if ( inFreq <= 100 )
		theRate = GR_REFRESH_90Hz;
	else if ( inFreq <= 120 )
		theRate = GR_REFRESH_120Hz;

	return theRate;
}


// #if ALLOW_GLIDE_FULLSCREEN
/*
________________________________________________________________________________________

      InitializeFullscreenGlide
________________________________________________________________________________________

*/

TQAError
InitializeFullscreenGlide( TQADrawContext	*inContext)
{
	int					theNumberOfBuffers;
	GrScreenRefresh_t	theRefreshRate;
	GrScreenResolution_t theScreenResolution;
	GrPixelFormat_t		thePixelFormat;
	TQAError 			err = kQANoErr;
	TQADrawPrivate		*dp = inContext->drawPrivate;
	CGrafPtr			theWindowHandle;
	DEBUG_ENTRY( InitializeFullscreenGlide );	

	if ( !tnsl_GlideInitialized ) InitializeGlide();

	dp->renderPort = NULL;
	dp->renderSurface = NULL;
	dp->auxSurface = NULL;

	if ( dp->glideContext )
	{
		DebugStr( "InitializeFullscreenGlide a glide context is already open...\n" );
		goto ErrorExit;
	}	

	if(dp->glideBoardSelectID == -1)
	{
		// DEBUG_ERROR( InitializeWindowedGlideRenderingSurface, "Can't render on target device.\n" );
		goto ErrorExit;
	}
	
	if (dp->screenWidth == 832)
	{
		err = kQADisplayModeUnsupported;
		DebugStr(" \n\n Glide doesn't support 832x624 fullscreen \n\n");
		goto ErrorExit;
	}
		
    grSstSelect( dp->glideBoardSelectID );
		
		switch(dp->screenWidth) {
		case 512:
			theScreenResolution = GR_RESOLUTION_512x384;
			break;
			
		case 640:
			theScreenResolution = GR_RESOLUTION_640x480;
			break;
			
		case 800:
			theScreenResolution = GR_RESOLUTION_800x600;
			break;
			
		case 960:
			theScreenResolution = GR_RESOLUTION_960x720;
			break;
			
		case 1024:
			theScreenResolution = GR_RESOLUTION_1024x768;
			break;
			
		case 1152:
			theScreenResolution = GR_RESOLUTION_1152x864;
			break;
			
		case 1280:
			theScreenResolution = GR_RESOLUTION_1280x960;
			break;
			
		case 1600:
			theScreenResolution = GR_RESOLUTION_1600x1024;
			break;
			
		case 1792:
			theScreenResolution = GR_RESOLUTION_1792x1344;
			break;
			
		case 1856:
			theScreenResolution = GR_RESOLUTION_1856x1392;
			break;
			
		case 1920:
			theScreenResolution = GR_RESOLUTION_1920x1440;
			break;
			
		case 2048:
			theScreenResolution = GR_RESOLUTION_2048x1536;
			break;
			
		default:
			theScreenResolution = GR_RESOLUTION_NONE;
			err = kQADisplayModeUnsupported;
			goto ErrorExit;
			break;
		}
	
		/* validate input data */
		
		theWindowHandle = (CGrafPtr)1;	// bogus, but it makes Glide happy
		// DebugStr( "InitializeFullscreenGlide front window = 0x%08x\n", theWindowHandle );
		
		if( theScreenResolution == GR_RESOLUTION_NONE )
		{
			// DEBUG_ERROR( InitializeFullscreenGlide, "cannot handle the requested resolution: w = %d, h = %d\n", theGLIFullScreen->width, theGLIFullScreen->height);
			goto ErrorExit;
		}
		// DEBUG_VERBOSE( InitializeFullscreenGlide, "glide resolution = %d\n", theScreenResolution );
	
		theRefreshRate = GetGlideRefresh( dp, theScreenResolution );
		if( theScreenResolution == GR_REFRESH_NONE )
		{
			// DEBUG_ERROR( InitializeFullscreenGlide, "cannot handle the requested refresh rate: %dHz at the requested resolution\n", theGLIFullScreen->freq );
			goto ErrorExit;
		}
		// DEBUG_VERBOSE( InitializeFullscreenGlide, "glide refresh rate = %d\n", theRefreshRate );
	
		
		theNumberOfBuffers = 2;	// does not count the aux buffer
		// if( dp->z_bits > 0 ) theNumberOfBuffers++;
	
	    // TBFL
	    /* Figure out which board to use. 
	    if(tnsl_surface_ext) {
	      for(i = 0; i < rvDeviceListCount; i++) {
	        if(rvDeviceList[i].systemDeviceId == inContext->device) {
	          break;          
	        }
	      }
	      // Hmm.. just pick board 0 then.
	      if(i == rvDeviceListCount) i = 0;
	        grSstSelect(i);
	     
	    } else {
	      grSstSelect(0);
	    }
	*/
	      
		if(theWindowHandle == NULL){
			theWindowHandle = (CGrafPtr)1;
		}
	
		switch (dp->targetPixelBits) {
			case 32:
				thePixelFormat = GR_PIXFMT_ARGB_8888; break;
			case 16:
				thePixelFormat = GR_PIXFMT_ARGB_1555; break;
			default:
				thePixelFormat = GR_PIXFMT_ARGB_8888;
		}
		
		if ( grSstWinOpenExt ) {
			// DEBUG_VERBOSE( InitializeFullscreenGlide, "grSstWinOpenExt()\n" );
			dp->glideContext = grSstWinOpenExt(	(FxU32)theWindowHandle,
														theScreenResolution,
														theRefreshRate,
														GR_COLORFORMAT_ARGB,
														GR_ORIGIN_UPPER_LEFT,
														thePixelFormat,
														theNumberOfBuffers,
														dp->z_bits > 0 ? 1 : 0);
	    }
	    else {
			// DEBUG_VERBOSE( InitializeFullscreenGlide, "grSstWinOpen()\n" );
			dp->glideContext = grSstWinOpen(		(FxU32)theWindowHandle,
														theScreenResolution,
														theRefreshRate,
														GR_COLORFORMAT_ARGB,
														GR_ORIGIN_UPPER_LEFT,
														theNumberOfBuffers,
														1);
		}

	if ( dp->glideContext == 0 )
	{
		// DEBUG_VERBOSE( InitializeFullscreenGlide, "No glide context could be created...\n" );
		err = (TQAError)1;
		goto ErrorExit;
	}
	else
	{
	    GDHandle mainDeviceHandle;
	    short mainDeviceRefNum;
	    ParamBlockRec theParamBlock;
	    GLuint i;
	    FxU32 red[256], green[256], blue[256];
        FxU8 *r, *g, *b;
	    VDGammaRecord gammaRec;
	    
	    GammaTbl *gammaTable;
	    OSErr theSuccess;
	    
		/* TBFL
		dp->viewPort.size.w = theGLIFullScreen->width;
		dp->viewPort.size.h = theGLIFullScreen->height;
		dp->viewPortDirty = GL_TRUE;
		*/
		
        /* Enable UMA texture pool on Voodoo3/Napalm */
        if(tnsl_uma_texture) {
			grEnable(GR_TEXTURE_UMA_EXT);
         }
         
	 	if ( !grSelectContext( dp->glideContext ))
		{
			return kQAError;
		}
		
		grRenderBuffer( GR_BUFFER_BACKBUFFER );
		// for debugging grRenderBuffer( GR_BUFFER_FRONTBUFFER );
		
        /* Do the gamma correction thing.  Set up Glide's gamma table to be the same as the gamma table */
        /* from the window we're using.  Really we need some nice of just getting the GDevice from Glide, */
        /* but on a Voodoo2 there's no such mechanism.  We still need some way of setting the gamma on  */
        /* a Voodoo2 while the game is running.  Need to talk with Carmack about this.  For now users */
        /* can use the init file to change gamma settings as a fallback. */
        // TBFL
        
        if (tnsl_uma_texture) { /* Hack... this would fail on a Banshee */
        
          /* Find device (GDhandle) for our window */
          mainDeviceHandle = dp->device.device.gDevice; //GetMaxDevice(&((CGrafPtr)theWindowHandle)->portRect);
          
          /* Make sure the device is in direct color mode, otherwise this will not be very useful... */
          if (TestDeviceAttribute(mainDeviceHandle,RGBDirect)) 
          {
            /* Make sure it's a real screen device */
            if(TestDeviceAttribute(mainDeviceHandle,screenDevice)) {         
              /* Verify it actually has an associated driver... */
              if(!TestDeviceAttribute(mainDeviceHandle,noDriver)) {  
                mainDeviceRefNum = (*mainDeviceHandle)->gdRefNum;
           
                memset(&theParamBlock,0,sizeof(theParamBlock));
                theParamBlock.cntrlParam.ioCompletion = 0;
                theParamBlock.cntrlParam.ioVRefNum = 0;
                theParamBlock.cntrlParam.ioCRefNum = mainDeviceRefNum;
                *((void **) &(theParamBlock.cntrlParam.csParam)) = &gammaRec;
                theParamBlock.cntrlParam.csCode = cscGetGamma;

                theSuccess = (OSErr) PBStatusSync(&theParamBlock);
                if(theSuccess == noErr) {
                  
                  /* Okay, convert the gamma table to what Glide wants and send it off. */
                  gammaTable = (GammaTblPtr)gammaRec.csGTable;
                  r = (FxU8 *) &gammaTable->gFormulaData + gammaTable->gFormulaSize;
                  if(1 == gammaTable->gChanCnt) {
                    g = r;
                    b = r;
                  }
                  else 
                  {
                    g = &r[gammaTable->gDataCnt];
                    b = &g[gammaTable->gDataCnt];
                  }
                  /* I know my "source" data is 8 bit, so I don't have to do any weird shifting. */
                  for(i = 0; i < 256; i++) {                   
                    red[i] = r[i];
                    green[i] = g[i];
                    blue[i] = b[i];
                  }
                  grLoadGammaTable(256,red,green,blue);
                }
              }
            }
          }
        }
        
		SetupGlideContext( dp );
			
	}

ErrorExit:


	return err;
}

/*
________________________________________________________________________________________

      SetupGlideContext
________________________________________________________________________________________

 * Sets up depth of z-buffer, 
*/

void SetupGlideContext( TQADrawPrivate * inContext)
{
#pragma unused(inContext)

	grBufferClear(0, 0, 0xFFFF);
	//+ This should be a constant layout for RAVE.
	grVertexLayout(GR_PARAM_XY,  0, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Z,   8, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Q,   12, GR_PARAM_ENABLE);	// 1/invW	used for fog
	grVertexLayout(GR_PARAM_PARGB,16,GR_PARAM_ENABLE);

	grVertexLayout(GR_PARAM_RGB, 20, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_A,   32, GR_PARAM_DISABLE);

#if VARI_VERT_LAYOUT
	grVertexLayout(GR_PARAM_ST0, 20, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_Q0,  28, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_ST1, 32, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_Q1,  40, GR_PARAM_DISABLE);
#else
	grVertexLayout(GR_PARAM_ST0, 20, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Q0,  28, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_ST1, 32, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Q1,  40, GR_PARAM_ENABLE);
#endif

	DebugStr(" SetupGlideContext ");
	/* Set up Render State - gouraud shading */
	
	grTexCombine(GR_TMU0, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
           GR_COMBINE_FUNCTION_LOCAL_ALPHA, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE);
    grTexCombine(GR_TMU1, GR_COMBINE_FUNCTION_NONE, GR_COMBINE_FACTOR_NONE,
	             GR_COMBINE_FUNCTION_NONE, GR_COMBINE_FACTOR_NONE, FXFALSE,FXFALSE);
	grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
	grTexLodBiasValue( GR_TMU0, 0.5f );
	grTexMipMapMode(GR_TMU0, GR_MIPMAP_NEAREST, FXFALSE);
	grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);
	grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ONE, GR_BLEND_ZERO);
	
	DebugStr(" SetupGlideContext ");
	grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
	             GR_COMBINE_FACTOR_NONE,
	             GR_COMBINE_LOCAL_ITERATED,
	             GR_COMBINE_OTHER_NONE,
	             FXFALSE);

	grColorCombine(	GR_COMBINE_FUNCTION_LOCAL,
					GR_COMBINE_FACTOR_NONE,
					GR_COMBINE_LOCAL_ITERATED,
					GR_COMBINE_OTHER_NONE,
					FXFALSE );

	grCullMode(GR_CULL_DISABLE);			
    grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
    // 3/20
	grColorMask(FXTRUE, FXFALSE);
	grConstantColorValue(0xFF000000);
	
	grAlphaTestReferenceValue(255);
	grAlphaTestFunction(GR_CMP_EQUAL);
	
   
#if 0    
    if(PerfMonAvailable()) {
      SetMMCR0((1 << MMCR0_PMC1SELECT_SHIFT) |
               (2 << MMCR0_PMC2SELECT_SHIFT));
      SetMMCR1((15 << MMCR1_PMC3SELECT_SHIFT) |
               (5 << MMCR1_PMC4SELECT_SHIFT));         
    }
#endif    
}


/*
________________________________________________________________________________________

      RvSurfaceNotify
________________________________________________________________________________________

*/

void
RvSurfaceNotify(GrSurface_t sfc, void *userData, unsigned long code)
{
	TQADrawContext *inContext = userData;
	TQADrawPrivate *dp = inContext->drawPrivate;
	
	DebugStr("\nRvSurfaceNotify ?????? \n");
	DebugHex(code);
	if(code == GR_SURFACE_NOTIFY_LOST)
	{
		/* Figure out which surface is gone */
		if(sfc == dp->renderSurface) 
		{
			DebugStr(" rendering surface lost.\n");
			grSurfaceSetRenderingSurfaceExt(0);
			//+ TBFL glrNopDispatchTable(dp);
			dp->renderSurface = NULL;
			dp->surfaceLost = true;
		}
		else if(sfc == dp->auxSurface)
		{
			DebugStr(" aux surface lost.\n");
			grSurfaceSetAuxSurfaceExt(0);
			//+ TBFL glrNopDispatchTable(dp);
			dp->auxSurface = NULL;
			dp->surfaceLost = true;
		}
		else if(sfc == gRvEngInfo.texture_surface)
		{
			DebugStr(" texture surface lost.\n");
			FreeAllTexturesVRAM();	// seems like we already freed 'em
			gRvEngInfo.texture_surface = NULL;
			gRvEngInfo.surface_size = 0;
			DebugStr("\n RvSurfaceNotify complete\n");
			/* Technically we lost a surface, but we don't care. The
			 texture management code will lazily grab a new one when needed. */
			
		}

	}
  ResetContextStateFlags( inContext );	// resets change flags
}



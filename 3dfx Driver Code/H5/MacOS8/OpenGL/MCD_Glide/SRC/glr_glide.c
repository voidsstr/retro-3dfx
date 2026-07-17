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



#include "glr.h"
#include "glr_glide.h"
#include "glr_drawing.h"
#include "hrm_3d_envvars.h"

#include <string.h> /* for memset */
#include <Windows.h>
#include <QuickDraw.h>
#include <Devices.h>
#include <Displays.h>
#include <Video.h>
#include <DCon.h>
//#include "PerfMon.h"


GLboolean gls_GlideInitialized = GL_FALSE;
GLboolean gls_surface_ext = GL_FALSE;
#if 0
GLboolean gls_pix_ext = GL_FALSE;
GLboolean gls_combine_ext = GL_FALSE;
GLboolean gls_32bit_texture = GL_FALSE;
GLboolean gls_2k_texture_size = GL_FALSE;
GLboolean gls_uma_texture = GL_FALSE;
GLboolean gls_CMDTransport = GL_FALSE;
#endif

/* Glide Extensions */
grSurfaceCreateContextFuncPtr grSurfaceCreateContextExt = 0;
grSurfaceReleaseContextFuncPtr grSurfaceReleaseContextExt = 0;
grSurfaceSetRenderingSurfaceFuncPtr grSurfaceSetRenderingSurfaceExt = 0;
grSurfaceCalcTextureWHDFuncPtr grSurfaceCalcTextureWHDExt = 0;
grSurfaceSetAuxSurfaceFuncPtr grSurfaceSetAuxSurfaceExt = 0;
grSurfaceSetTextureSurfaceFuncPtr grSurfaceSetTextureSurfaceExt = 0;
grDeviceQueryFuncPtr grDeviceQueryExt = 0;

grSstWinOpenFuncPtr grSstWinOpenExt = 0;
grColorMaskFuncPtr grColorMaskExt = 0;
grStencilFuncPtr grStencilFuncExt = 0;
grStencilMaskFuncPtr grStencilMaskExt = 0;
grStencilOpFuncPtr grStencilOpExt = 0;
grBufferClearFuncPtr grBufferClearExt = 0;
grLfbConstantStencilFuncPtr grLfbConstantStencilExt = 0;
grTBufferWriteMaskFuncPtr grTBufferWriteMaskExt = 0;
grColorCombineFuncPtr grColorCombineExt = 0;
grAlphaCombineFuncPtr grAlphaCombineExt = 0;
grTexColorCombineFuncPtr grTexColorCombineExt = 0;
grTexAlphaCombineFuncPtr grTexAlphaCombineExt = 0;
grConstantColorValueFuncPtr grConstantColorValueExt = 0;

grCommandTransportInfoPtr grCommandTransportInfoExt = 0;
grCommandTransportMakeRoomPtr grCommandTransportMakeRoomExt = 0;

/* MacOS specific Glide Extensions */
grSurfaceCreateFuncPtr grSurfaceCreateExt = 0;
grSurfaceReleaseFuncPtr grSurfaceReleaseExt = 0;
grSurfaceGetDescFuncPtr grSurfaceGetDescExt = 0;

/* Device List */
GrDeviceInfo_t glrDeviceList[MAX_GLIDE_DEVICES];
glrGlideExtensions_t glideExtensions[MAX_GLIDE_DEVICES];  

FxU32 glrDeviceListCount;

/* local definition */

void    glrSetupGlideContext( GLDContext inContext);
GrScreenResolution_t    glrGetGlideResolution( const GLsizei inWidth, const GLsizei inHeight);
GrScreenRefresh_t    glrGetGlideRefresh( const CGrafPtr inWindow, GLsizei inFreq, const GrScreenResolution_t inRes);
GrPixelFormat_t    glrGetGlidePixelFormat( GLenum inColorMode );


/*
________________________________________________________________________________________

      glrInitializeGlide
________________________________________________________________________________________

*/

#define containstr( src, target ) \
  ( strstr( src, target ) != NULL) 
  
static void glrGetBoardExtensions(glrGlideExtensions_t *glideExtensions)
{
  // Get extensions we want to use.
  const char *theExtension = grGetString( GR_EXTENSION );

  DEBUG_ENTRY(glrGetBoardExtensions);	
  
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
      gls_surface_ext = FXTRUE;
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
		DEBUG_VERBOSE( glrGetBoardExtensions, "pix extensions are available...\n" );
        glideExtensions->gls_pix_ext = FXTRUE;
    }
  }
  
  
  if ( containstr( theExtension, " COMBINE " ) )
  {
    grColorCombineExt = (void *)grGetProcAddress("grColorCombineExt");
    grAlphaCombineExt = (void *)grGetProcAddress("grAlphaCombineExt");
    grTexColorCombineExt = (void *)grGetProcAddress("grTexColorCombineExt");
    grTexAlphaCombineExt = (void *)grGetProcAddress("grTexAlphaCombineExt");
    grConstantColorValueExt = (void *)grGetProcAddress("grConstantColorValueExt");
    
    if (  grColorCombineExt && grAlphaCombineExt && grTexColorCombineExt && grTexAlphaCombineExt && grConstantColorValueExt)
    {
		DEBUG_VERBOSE( glrGetBoardExtensions, "combine extensions are available...\n" );
        glideExtensions->gls_combine_ext = FXTRUE;
    }
  }  


  if ( containstr( theExtension, " TEXFMT " ) )
  {
    glideExtensions->gls_32bit_texture = glideExtensions->gls_2k_texture_size = GL_TRUE;
	DEBUG_VERBOSE( glrGetBoardExtensions, "textures extensions are available...\n" );
  }  


  if ( containstr( theExtension, " TEXUMA " ) )
  {
    glideExtensions->gls_uma_texture = GL_TRUE;
	DEBUG_VERBOSE( glrGetBoardExtensions, "uma textures are available...\n" );
  }  

  if ( containstr( theExtension, " COMMAND_TRANSPORT " ) )
  {
    glideExtensions->gls_CMDTransport = GL_TRUE;
    grCommandTransportInfoExt = (void *)grGetProcAddress("grCommandTransportInfoExt2");
    grCommandTransportMakeRoomExt = (void *)grGetProcAddress("grCommandTransportMakeRoomExt2");
    DEBUG_VERBOSE( glrGetBoardExtensions, "command_transport is available...\n");
  }
  //gls_GlideInitialized = GL_TRUE;
  
}

long glrValidateEnvironment()
{
  FxI32 numBoards, i;

  DEBUG_ENTRY( glrValidateEnvironment );
  
  /* Register this API with the HRM (to make sure it's not confused with a pure Glide client) */
  hrm_Register3DClient( kHRM_OpenGL_Client );
  
  /* Make sure Glideis around. */
  if((long)grGlideInit == (long)kUnresolvedCFragSymbolAddress) {
    glr_debug_printf("Glide Library not present.\n");
    return 0;
  }

  /* Make sure we have at least one detected board */
  grGet(GR_NUM_BOARDS,sizeof(numBoards),&numBoards);
  if(!numBoards) {
    return 0;
  }
  
  /* Now that we know we have at least one board we can pretty safely
     do our global Glide init stuff.  If we try to do this first without
     checking for how many boards we have, Glide will throw up a message
     box telling the user that no hardware was found. */
  if(!gls_GlideInitialized) {
    grGlideInit();
    gls_GlideInitialized = FXTRUE;
  }

  //if(gls_surface_ext) {
  //  /* Grab list of all known boards we can use. */
  //  glrDeviceListCount = grDeviceQueryExt(glrDeviceList, MAX_GLIDE_DEVICES);
  //} else {
  //  /* Do it the "old fasioned way". */
  glrDeviceListCount = numBoards;
  //}
  
  for(i = 0; i < numBoards; i++)
  {
    grSstSelect(i);
    glrGetBoardExtensions(&glideExtensions[i]);    
  }  
  grSstSelect(0);
  
  /* Now see if the surface extensions are present for at least the first board. */
  if(gls_surface_ext) {
    /* Grab system specific infor for board/display matching. */
    grDeviceQueryExt(glrDeviceList, MAX_GLIDE_DEVICES);
  }
  
  return glrDeviceListCount;
}

/*
________________________________________________________________________________________

      glrSetupHardwareFuncs
________________________________________________________________________________________

*/

void glrSetupHardwareFuncs(GLDContext inContext)
{
  /* Set up texture/color combine function */
#if !SHOW_OVERDRAW
  if(inContext->glideExtensions.gls_combine_ext)
    inContext->configureCombineUnits = glrConfigureCombineUnitsNapalm;
  else
#endif  
    inContext->configureCombineUnits = glrConfigureCombineUnitsSST1;
}

/*
________________________________________________________________________________________

      glrInitializeWindowedGlideRenderingSurface
________________________________________________________________________________________

*/

GLenum
glrInitializeWindowedGlideRenderingSurface(
	GLDContext			inContext,
	GLint				inWidth,
	GLint				inHeight,
	GLint				inRowbytes)
{
	GLenum				theResult = GLI_BAD_DRAWABLE;
	
	DEBUG_ENTRY( glrInitializeWindowedGlideRenderingSurface );	

	//if ( !gls_GlideInitialized ) glrInitializeGlide();
	
	inContext->renderPort = 0;
	inContext->renderSurface = 0;
    inContext->auxSurface = 0;

	if ( inContext->targetPort == 0 )
	{
		DEBUG_ERROR( glrInitializeWindowedGlideRenderingSurface, "Target grafport has not been set yet. Cannot identify hardware...\n" );
		goto ErrorExit;
	}
	
	if(inContext->glideBoardSelectID == -1)
	{
		DEBUG_ERROR( glrInitializeWindowedGlideRenderingSurface, "Can't render on target device.\n" );
		goto ErrorExit;
	}
	
    grSstSelect( inContext->glideBoardSelectID );
    inContext->glideExtensions = glideExtensions[inContext->glideBoardSelectID];
    glrSetupHardwareFuncs(inContext);

	inContext->glideContext = grSurfaceCreateContextExt( GR_SURFACECONTEXT_WINDOWED );
	if ( inContext->glideContext == 0 )
	{
		DEBUG_VERBOSE( glrInitializeWindowedGlideRenderingSurface, "No glide context could be created...\n" );
		goto ErrorExit;
	}	
	
    if( grSelectContext( inContext->glideContext ) )
    {

		GrSurfaceDesc_t		theSfcDesc;

		DEBUG_VERBOSE( glrInitializeWindowedGlideRenderingSurface, "glide windowed context created...\n" );

		/* initialize the rendering surface */
		memset( &theSfcDesc,0,sizeof(theSfcDesc) );
		theSfcDesc.width = inWidth;
		theSfcDesc.height = inHeight;
		if(inContext->glideExtensions.gls_pix_ext) {
		  theSfcDesc.bytesPerPixel = inRowbytes / inWidth;
		  if(theSfcDesc.bytesPerPixel == 4) {
		    inContext->grPixelFormat = GR_PIXFMT_ARGB_8888;
		  } else {
		    inContext->grPixelFormat = GR_PIXFMT_ARGB_1555;
		  }
		} else {
		  theSfcDesc.bytesPerPixel = 2;
		  inContext->grPixelFormat = GR_PIXFMT_RGB_565;
		}
	    theSfcDesc.notifyCallback = glrSurfaceNotify;
	    theSfcDesc.userData = inContext;

		inContext->renderSurface = grSurfaceCreateExt( &theSfcDesc );

		if( inContext->renderSurface )
		{
			DEBUG_VERBOSE( glrInitializeWindowedGlideRenderingSurface, "glide rendering surface allocated...\n" );

			grSurfaceSetRenderingSurfaceExt( inContext->renderSurface );
			grSurfaceGetDescExt( inContext->renderSurface, &inContext->renderSurfaceDesc );
			inContext->renderPort = inContext->renderSurfaceDesc.systemPortId;
			/* the gdhandle is also passed */

			inContext->viewPort.size.w = inWidth;
			inContext->viewPort.size.h = inHeight;
			inContext->viewPortDirty = GL_TRUE;
			inContext->isUMA = FXTRUE;
			
			if(inContext->has_depth) {
				/* initialize the rendering surface */
				memset( &theSfcDesc,0,sizeof(theSfcDesc) );
				theSfcDesc.width = inWidth;
				theSfcDesc.height = inHeight;
                if(inContext->glideExtensions.gls_pix_ext) {
		          theSfcDesc.bytesPerPixel = inRowbytes / inWidth;
		        } else {
		          theSfcDesc.bytesPerPixel = 2;
		          inContext->grPixelFormat = GR_PIXFMT_RGB_565;
		        }
                theSfcDesc.notifyCallback = glrSurfaceNotify;
	            theSfcDesc.userData = inContext;

				inContext->auxSurface = grSurfaceCreateExt( &theSfcDesc );

				if( inContext->auxSurface )
				{   
				    grSurfaceGetDescExt(inContext->auxSurface, &inContext->auxSurfaceDesc);
			        grSurfaceSetAuxSurfaceExt(inContext->auxSurface);
			    }
			}

			glrSetupGlideContext( inContext );
			
			theResult = GLD_DRAWABLE_NONE;

		}
		else
		{
			DEBUG_VERBOSE( glrInitializeWindowedGlideRenderingSurface, "glide surface for the window could not be allocated...\n" );
			DEBUG_VERBOSE( glrInitializeWindowedGlideRenderingSurface, "   - theSfcDesc.width = %d ; height = %d ; bytesPerPixel = %d\n", theSfcDesc.width, theSfcDesc.height, theSfcDesc.bytesPerPixel  );
			goto ErrorExit;
		}
		
		
	}
	else
	{
		DEBUG_VERBOSE( glrInitializeWindowedGlideRenderingSurface, "glide context could not be selected...\n" );
		goto ErrorExit;
	}


ErrorExit:

	return theResult;
}


/*
________________________________________________________________________________________

      glrInitializeFullscreenGlide
________________________________________________________________________________________

*/

GLenum
glrInitializeFullscreenGlide(
	GLDContext			inContext,
	const GLIFullScreen * inDrawable,
	CGrafPtr            theWindowHandle)
{
	GLint				theNumberOfBuffers;
	GrScreenRefresh_t	theRefreshRate;
	GrScreenResolution_t theScreenResolution;
	GLIFullScreen *		theGLIFullScreen = (GLIFullScreen *) inDrawable;
    FxU32				thePixelformat, i;
	GLenum				theResult = GLI_BAD_DRAWABLE;
	
	DEBUG_ENTRY( glrInitializeFullscreenGlide );	

	//if ( !gls_GlideInitialized ) glrInitializeGlide();

	inContext->renderPort = 0;
	inContext->renderSurface = 0;
	inContext->auxSurface = 0;

	if ( inContext->glideContext )
	{
		DEBUG_VERBOSE( glrInitializeFullscreenGlide, "a glide context is already open...\n" );
		goto ErrorExit;
	}	


	/* validate input data */
	
	//theWindowHandle = (CGrafPtr)FrontWindow();
	DEBUG_VERBOSE( glrInitializeFullscreenGlide, "front window = 0x%08x\n", theWindowHandle );
	
	theScreenResolution = glrGetGlideResolution( theGLIFullScreen->width, theGLIFullScreen->height );
	if( theScreenResolution == GR_RESOLUTION_NONE )
	{
		DEBUG_ERROR( glrInitializeFullscreenGlide, "cannot handle the requested resolution: w = %d, h = %d\n", theGLIFullScreen->width, theGLIFullScreen->height);
		goto ErrorExit;
	}
	DEBUG_VERBOSE( glrInitializeFullscreenGlide, "glide resolution = %d\n", theScreenResolution );

	theRefreshRate = glrGetGlideRefresh( theWindowHandle, theGLIFullScreen->freq, theScreenResolution );
	if( theScreenResolution == GR_REFRESH_NONE )
	{
		DEBUG_ERROR( glrInitializeFullscreenGlide, "cannot handle the requested refresh rate: %dHz at the requested resolution\n", theGLIFullScreen->freq );
		goto ErrorExit;
	}
	DEBUG_VERBOSE( glrInitializeFullscreenGlide, "glide refresh rate = %d\n", theRefreshRate );

	thePixelformat = glrGetGlidePixelFormat( inContext->gliPixelFormat.color_mode );
	if( thePixelformat == 0 )
	{
		DEBUG_ERROR( glrInitializeFullscreenGlide, "the GLIPixelFormat is not supported : 0x%08x\n", inContext->gliPixelFormat.color_mode);
		goto ErrorExit;
	}
	DEBUG_VERBOSE( glrInitializeFullscreenGlide, "glide pixel format = %d\n", thePixelformat );

	theNumberOfBuffers = 1;
	if( inContext->gliPixelFormat.buffer_mode & GLI_DOUBLEBUFFER_BIT ) theNumberOfBuffers++;
	// fix me...  single buffered full screen fails if I ask for just one color buffer
	theNumberOfBuffers = 2;

    /* Figure out which board to use. */
    if(gls_surface_ext) {
      for(i = 0; i < glrDeviceListCount; i++) {
        if((GLint)glrDeviceList[i].systemDeviceId == (GLint)(**inContext->device).gdRefNum) {
          break;          
        }
      }
      // Hmm.. just pick board 0 then.  Sigh.
      if(i == glrDeviceListCount) i = 0;
    } else {
      i = 0;
    }
    
    grSstSelect(i);
    inContext->glideExtensions = glideExtensions[i];
    glrSetupHardwareFuncs(inContext);
    
	if(theWindowHandle == NULL){
		theWindowHandle = (CGrafPtr)1;
	}

	if ( grSstWinOpenExt) {
		DEBUG_VERBOSE( glrInitializeFullscreenGlide, "grSstWinOpenExt()\n" );
		inContext->glideContext = grSstWinOpenExt(	(FxU32)theWindowHandle,
													theScreenResolution,
													theRefreshRate,
													GR_COLORFORMAT_ARGB,
													GR_ORIGIN_UPPER_LEFT,
													thePixelformat,
													theNumberOfBuffers,
													inContext->has_depth ? 1 : 0);
    }
    else {
		DEBUG_VERBOSE( glrInitializeFullscreenGlide, "grSstWinOpen()\n" );
		inContext->glideContext = grSstWinOpen(		(FxU32)theWindowHandle,
													theScreenResolution,
													theRefreshRate,
													GR_COLORFORMAT_ARGB,
													GR_ORIGIN_UPPER_LEFT,
													theNumberOfBuffers,
													inContext->has_depth ? 1 : 0);
	}
		
	if ( inContext->glideContext == 0 )
	{
		DEBUG_VERBOSE( glrInitializeFullscreenGlide, "No glide context could be created...\n" );
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
	
        inContext->grPixelFormat = thePixelformat;
	    
		inContext->viewPort.size.w = theGLIFullScreen->width;
		inContext->viewPort.size.h = theGLIFullScreen->height;
		inContext->viewPortDirty = GL_TRUE;

        /* Enable UMA texture pool on Voodoo3/Napalm */
        if(inContext->glideExtensions.gls_uma_texture) {
          grEnable(GR_TEXTURE_UMA_EXT);
          inContext->isUMA = GL_TRUE;
        }
 
        /* Do the gamma correction thing.  Set up Glide's gamma table to be the same as the gamma table */
        /* from the window we're using.  Really we need some nice of just getting the GDevice from Glide, */
        /* but on a Voodoo2 there's no such mechanism.  We still need some way of setting the gamma on  */
        /* a Voodoo2 while the game is running.  Need to talk with Carmack about this.  For now users */
        /* can use the init file to change gamma settings as a fallback. */
        
        if(inContext->glideExtensions.gls_uma_texture) { /* Hack... this would fail on a Banshee */
        
          /* Find device for our window */
          mainDeviceHandle = inContext->device; //GetMaxDevice(&((CGrafPtr)theWindowHandle)->portRect);
          
          /* Make sure the device is in direct color mode, otherwise this will not be very useful... */
          if(TestDeviceAttribute(mainDeviceHandle,RGBDirect)) {
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
        
		glrSetupGlideContext( inContext );
		
		theResult = GLD_DRAWABLE_NONE;
			
	}


ErrorExit:

#if GLR_DEBUG
	if ( theResult == GLD_DRAWABLE_NONE )
		DEBUG_VERBOSE( glrInitializeFullscreenGlide, "new drawable attached successfully\n" );	
	else
		DEBUG_VERBOSE( glrInitializeFullscreenGlide, "failed to attach drawable (%d)\n", theResult );	
#endif

	return theResult;
}




/*
________________________________________________________________________________________

      glrSetupGlideContext
________________________________________________________________________________________

*/

void
glrSetupGlideContext(
	GLDContext			inContext)
{
	//inContext->device = inContext->targetPort->device;

    inContext->cmdTransportInfo = grCommandTransportInfoExt();
    
	grGet(GR_WDEPTH_MIN_MAX, sizeof(inContext->wRange), (FxI32*)inContext->wRange);  

	grClipWindow(0, 0, inContext->viewPort.size.w, inContext->viewPort.size.h);
	grBufferClear(0, 0, inContext->wRange[0]);

	grVertexLayout(GR_PARAM_XY,  0, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Z,   8, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Q,   12, GR_PARAM_DISABLE); // Q param is only used for fog
	grVertexLayout(GR_PARAM_PARGB,16,GR_PARAM_ENABLE);

	grVertexLayout(GR_PARAM_RGB, 20, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_A,   32, GR_PARAM_DISABLE);

	grVertexLayout(GR_PARAM_ST0, 40, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_Q0,  48, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_ST1, 52, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_Q1,  60, GR_PARAM_DISABLE);

    if(inContext->glideExtensions.gls_2k_texture_size)
      inContext->max_texture_lod = GR_LOD_LOG2_2048;
    else
      inContext->max_texture_lod = GR_LOD_LOG2_256;  
      
	/* Set up Render State - gouraud shading */
	#if 0
	grColorCombine(	GR_COMBINE_FUNCTION_LOCAL,
					GR_COMBINE_FACTOR_NONE,
					GR_COMBINE_LOCAL_ITERATED,
					GR_COMBINE_OTHER_NONE,
					FXFALSE );
    #endif
    
	grCullMode(GR_CULL_DISABLE);			
    grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
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

      glrPixelSize
________________________________________________________________________________________

*/

GLint
glrPixelSize(
	GLbitfield			inColorMode)
{
	if( inColorMode & GLI_COLOR_8BIT_BITS)
		return  8;
	if( inColorMode & GLI_COLOR_16BIT_BITS)
		return 16;
	if( inColorMode & GLI_COLOR_32BIT_BITS)
		return 32;
	return 0;
}



/*
________________________________________________________________________________________

      glrDepthSize
________________________________________________________________________________________

*/

GLint
glrDepthSize(
	GLbitfield				inDepth_mode)
{
	if( inDepth_mode & GLI_32_BIT)
		return 32;
	if( inDepth_mode & GLI_16_BIT)
		return 16;
	return 0;
}


/*
________________________________________________________________________________________

      glrColorSizes
________________________________________________________________________________________

*/

GLboolean
glrColorSizes(
	GLbitfield				inColorMode,
	GLRColorSizes *			outSizes)
{
	outSizes->red_size    = 0;
	outSizes->green_size  = 0;
	outSizes->blue_size   = 0;
	outSizes->alpha_size  = 0;
	outSizes->buffer_size = 0;

	if(inColorMode & (GLI_COLOR_INVERSE_BITS | GLI_COLOR_BGR233_BITS | GLI_COLOR_RGB332_BITS))
	{
		outSizes->red_size    =  3;
		outSizes->green_size  =  3;
		outSizes->blue_size   =  2;
	}
	else if(inColorMode & GLI_COLOR_RGB444_BITS)
	{
		outSizes->red_size    =  4;
		outSizes->green_size  =  4;
		outSizes->blue_size   =  4;
	}
	else if(inColorMode & GLI_COLOR_RGB555_BITS)
	{
		outSizes->red_size    =  5;
		outSizes->green_size  =  5;
		outSizes->blue_size   =  5;
	}
	else if(inColorMode & GLI_COLOR_RGB565_BITS)
	{
		outSizes->red_size    =  5;
		outSizes->green_size  =  6;
		outSizes->blue_size   =  5;
	}
	else if(inColorMode & GLI_COLOR_RGB888_BITS)
	{
		outSizes->red_size    =  8;
		outSizes->green_size  =  8;
		outSizes->blue_size   =  8;
	}
	else if(inColorMode & GLI_COLOR_RGB101010_BITS)
	{
		outSizes->red_size    = 10;
		outSizes->green_size  = 10;
		outSizes->blue_size   = 10;
	}
	else if(inColorMode & GLI_COLOR_RGB121212_BITS)
	{
		outSizes->red_size    = 12;
		outSizes->green_size  = 12;
		outSizes->blue_size   = 12;
	}
	else if(inColorMode & GLI_COLOR_RGB161616_BITS)
	{
		outSizes->red_size    = 16;
		outSizes->green_size  = 16;
		outSizes->blue_size   = 16;
	}
	else if(inColorMode & GLI_INDEX8_BIT)
	{
		outSizes->buffer_size =  8;
	}
	else if(inColorMode & GLI_INDEX16_BIT)
	{
		outSizes->buffer_size = 16;
	}
	else
	{
		return GL_FALSE;
	}
	
	if(inColorMode & GLI_ARGB1555_BIT)
	{
		outSizes->alpha_size = 1;
	}
	else if(inColorMode & GLI_ARGB2101010_BIT)
	{
		outSizes->alpha_size = 2;
	}
	else if(inColorMode & GLI_ARGB4444_BIT)
	{
		outSizes->alpha_size = 4;
	}
	else if(inColorMode & (GLI_RGB8_A8_BIT | GLI_BGR233_A8_BIT | GLI_RGB332_A8_BIT | GLI_RGB444_A8_BIT | GLI_RGB555_A8_BIT | GLI_RGB565_A8_BIT | GLI_ARGB8888_BIT | GLI_RGB888_A8_BIT | GLI_RGB101010_A8_BIT))
	{
		outSizes->alpha_size = 8;
	}
	else if(inColorMode & GLI_ARGB12121212_BIT)
	{
		outSizes->alpha_size = 12;
	}
	else if(inColorMode & GLI_ARGB16161616_BIT)
	{
		outSizes->alpha_size = 16;
	}
	
	return GL_TRUE;
}





/*
________________________________________________________________________________________

      glrCompareFunc
________________________________________________________________________________________

*/

GrCmpFnc_t
glrCompareFunc(
	GLenum					inMode)
{
	GrCmpFnc_t				theFunction;
	
	switch( inMode )
	{
		case GL_NEVER:
			theFunction = GR_CMP_NEVER;
			break;
		
		case GL_LESS:
			theFunction = GR_CMP_LESS;
			break;
		
		case GL_EQUAL:
			theFunction = GR_CMP_EQUAL;
			break;
		
		case GL_LEQUAL:
			theFunction = GR_CMP_LEQUAL;
			break;
		
		case GL_GREATER:
			theFunction = GR_CMP_GREATER;
			break;
		
		case GL_NOTEQUAL:
			theFunction = GR_CMP_NOTEQUAL;
			break;
		
		case GL_GEQUAL:
			theFunction = GR_CMP_GEQUAL;
			break;
		
		default: /*case GL_ALWAYS:*/
			theFunction = GR_CMP_ALWAYS;
			break;
	}
	
	return theFunction;
}


/*
________________________________________________________________________________________

      glrStencilOp
________________________________________________________________________________________

*/

GrStencilOp_t
glrStencilOp(
	GLenum					inOp)
{
	GrCmpFnc_t				theFunction;
	
	switch( inOp )
	{
		case GL_KEEP:
			theFunction = GR_STENCILOP_KEEP;
			break;
		
		case GL_ZERO:
			theFunction = GR_STENCILOP_ZERO;
			break;
		
		case GL_REPLACE:
			theFunction = GR_STENCILOP_REPLACE;
			break;
		
		case GL_INCR:
			theFunction = GR_STENCILOP_INCR_CLAMP;
			break;
		
		case GL_DECR:
			theFunction = GR_STENCILOP_DECR_CLAMP;
			break;
		
		case GL_INVERT:
			theFunction = GR_STENCILOP_INVERT;
			break;		
	}
	
	return theFunction;
}




/*
________________________________________________________________________________________

      glrCompareFuncReversed
________________________________________________________________________________________

*/

GrCmpFnc_t
glrCompareFuncReversed(
	GLenum					inMode)
{
	GrCmpFnc_t				theFunction;
	
	switch( inMode )
	{
		case GL_NEVER:
			theFunction = GR_CMP_NEVER;
			break;
		
		case GL_LESS:
			theFunction = GR_CMP_GREATER;
			break;
		
		case GL_EQUAL:
			theFunction = GR_CMP_EQUAL;
			break;
		
		case GL_LEQUAL:
			theFunction = GR_CMP_GEQUAL;
			break;
		
		case GL_GREATER:
			theFunction = GR_CMP_LESS;
			break;
		
		case GL_NOTEQUAL:
			theFunction = GR_CMP_NOTEQUAL;
			break;
		
		case GL_GEQUAL:
			theFunction = GR_CMP_LEQUAL;
			break;
		
		default: /*case GL_ALWAYS:*/
			theFunction = GR_CMP_ALWAYS;
			break;
	}
	
	return theFunction;
}



/*
________________________________________________________________________________________

      glrGetGlideResolution
________________________________________________________________________________________

*/

GrScreenResolution_t
glrGetGlideResolution(
	const GLsizei				inWidth,
	const GLsizei				inHeight)
{
	
	typedef struct GlideResolutionData {
		GLsizei					width;
		GLsizei					height;
		GrScreenResolution_t	res;
	} GlideResolutionData;
	
	// this table MUST be sort with width growing and height growing!
	
	GlideResolutionData theGlideResolutionList[] =
	{
		{ 320, 240,  GR_RESOLUTION_320x240 },
		{ 512, 384,  GR_RESOLUTION_512x384 },
		{ 640, 400,  GR_RESOLUTION_640x400 },
		{ 640, 480,  GR_RESOLUTION_640x480 },
		{ 800, 600,  GR_RESOLUTION_800x600 },
		{ 960, 720,  GR_RESOLUTION_960x720 },
		{ 1024, 768,  GR_RESOLUTION_1024x768 },
		{ 1280, 960,  GR_RESOLUTION_1280x960 },
		{ 1280, 1024,  GR_RESOLUTION_1280x1024 },
		{ 1600, 1024,  GR_RESOLUTION_1600x1024 },
		{ 1600, 1200,  GR_RESOLUTION_1600x1200 },
		{ 1792, 1344,  GR_RESOLUTION_1792x1344 },
		{ 1856, 1392,  GR_RESOLUTION_1856x1392 },
		{ 1920, 1440,  GR_RESOLUTION_1920x1440 },
		{ 2048, 1536,  GR_RESOLUTION_2048x1536 },
		{ 2048, 2048,  GR_RESOLUTION_2048x2048 }
	};

	GrScreenResolution_t	theRes = GR_RESOLUTION_NONE;
	long					theLastEntry = sizeof( theGlideResolutionList )
												/ sizeof( GlideResolutionData );
	long					theEntry = 0;
	
	while ( theEntry < theLastEntry && theRes == GR_RESOLUTION_NONE ) {
		if ( inWidth <= theGlideResolutionList[ theEntry ].width
				&& inHeight <= theGlideResolutionList [ theEntry ].height ) {
			theRes = theGlideResolutionList[ theEntry ].res;
		}
		theEntry++;
	}

	return theRes;	
}
							


/*
________________________________________________________________________________________

      glrGetGlideRefresh
________________________________________________________________________________________

*/

GrScreenRefresh_t
glrGetGlideRefresh(
    const CGrafPtr              inDrawableWindow,
	GLsizei				        inFreq,
	const GrScreenResolution_t	/*inRes*/)
{
	GrScreenRefresh_t			theRate = GR_REFRESH_NONE;
	
	/* Try to determine frequency automatically based on the graphics
	   device the window is on. */
	if(inFreq == 0 && inDrawableWindow) {
	  GDHandle mainDeviceHandle;
	  short mainDeviceRefNum;
	  ParamBlockRec theParamBlock;
	  VDResolutionInfoRec resolutionInfo;
	  OSErr theSuccess;
	  
      /* Find device for our window */
      mainDeviceHandle = GetMaxDevice(&((CGrafPtr)inDrawableWindow)->portRect);
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




/*
________________________________________________________________________________________

      glrGetGlideRefresh
________________________________________________________________________________________

*/

GrPixelFormat_t
glrGetGlidePixelFormat(
	GLenum						inColorMode )
{
	GrPixelFormat_t				thePixelFormat = 0;

		 if ( inColorMode == GLI_INDEX8_BIT )	thePixelFormat = GR_PIXFMT_I_8;
	else if ( inColorMode == GLI_RGB565_BIT )	thePixelFormat = GR_PIXFMT_RGB_565;
	else if ( inColorMode == GLI_RGB555_BIT )	thePixelFormat = GR_PIXFMT_ARGB_1555;
	else if ( inColorMode == GLI_ARGB1555_BIT )	thePixelFormat = GR_PIXFMT_ARGB_1555;
	else if ( inColorMode == GLI_RGB888_BIT )	thePixelFormat = GR_PIXFMT_ARGB_8888;
	else if ( inColorMode == GLI_ARGB8888_BIT )	thePixelFormat = GR_PIXFMT_ARGB_8888;


/*
	Fix me: handling of AA stuff
	
#define GR_PIXFMT_AA_2_RGB_565                  0x0006
#define GR_PIXFMT_AA_2_ARGB_1555                0x0007
#define GR_PIXFMT_AA_2_ARGB_8888                0x0008
#define GR_PIXFMT_AA_4_RGB_565                  0x0009
#define GR_PIXFMT_AA_4_ARGB_1555                0x000a
#define GR_PIXFMT_AA_4_ARGB_8888                0x000b
*/
	return thePixelFormat;
}

/*
________________________________________________________________________________________

      glrNotImplemented
________________________________________________________________________________________

*/

/* THis is whacked... this used to live in another file, but I was
   getting link errors if was there even though it's public! */
void
glrNotImplemented(void)
{
	DEBUG_ENTRY( glrNotImplemented );
	// not much here!
}

void 
glrNopDispatchTable(GLDContext inContext)
{
GLuint *		theFuncPtr;
GLint			i;

  /* clear all the dispatch table (by filling it with glrNotImplemented() methods ) */
  theFuncPtr = (GLuint *) inContext->dispatch_table;
  if(!theFuncPtr) {
    return;
  }
  i = (sizeof(GLDRenderDispatch) / sizeof(void*) ) - 1;
  while ( i >= 0 ) {
    theFuncPtr[i--] = (GLuint) glrNotImplemented;
  }
}

/*
________________________________________________________________________________________

      glrSurfaceNotify
________________________________________________________________________________________

*/


void
glrSurfaceNotify(GrSurface_t sfc, void *userData, unsigned long code)
{
  if(code == GR_SURFACE_NOTIFY_LOST)
  {
    GLDContext inContext = userData;
    /* Figure out which surface is gone */
    if(sfc == inContext->renderSurface) 
    {
      DEBUG_PRINTF("rendering surface lost.\n");
      grSurfaceSetRenderingSurfaceExt(0);
      glrNopDispatchTable(inContext);
      inContext->renderSurface = 0;
      inContext->surfaceLost = GL_TRUE;
    }
    else if(sfc == inContext->auxSurface)
    {
      DEBUG_PRINTF("aux surface lost.\n");
      grSurfaceSetAuxSurfaceExt(0);
      glrNopDispatchTable(inContext);
      inContext->auxSurface = 0;
      inContext->surfaceLost = GL_TRUE;
    }
    else if(sfc == inContext->texture_surface)
    {
      DEBUG_PRINTF("texture surface lost.\n");
      glrFreeAllTexturesVRAM(inContext);
      inContext->texture_surface = 0;
      inContext->surface_size[0] = 0;
      inContext->surface_size[1] = 0;
      /* Technically we lost a surface, but we don't care. The
         texture management code will lazily grab a new one when needed. */
    }   
  }
}

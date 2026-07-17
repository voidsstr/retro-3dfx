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
**  Description: 
**
** 
**
*/


#include "glr.h"
#include "glr_drawing.h"
#include "glr_glide.h"
#include "glp.h"
#include <cstdlib>
#include <MacWindows.h>

/* local method declaration: */

static GLenum					glrAttachOffscreenDrawable(
										GLDContext			inContext,
										const GLIDrawable	inDrawable);

static GLenum					glrAttachWindowedDrawable(
										GLDContext			inContext,
										const GLIDrawable	inDrawable);

static GLenum					glrAttachFullScreenDrawable(
										GLDContext			inContext,
										const GLIDrawable	inDrawable,
										const CGrafPtr      inDrawableWindow);

static void						glrDetachDrawable(
										GLDContext			inContext);

static GLenum					glrSetupGlideSurfaceBlit(
										GLDContext			inContext,
										const CGrafPtr		inTargetGrafPort);

static GLenum					glrIsDrawableFullScreen(
										GLDContext			inContext,
										const CGrafPtr		inDrawable);




static GLuint glrIntersectingRects(Rect *r1, Rect *r2)
{
	GLint t, l, r, b, w, h;
	
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

/*
________________________________________________________________________________________

      gldAttachDrawable
________________________________________________________________________________________

*/

GLenum
gldAttachDrawable(
	GLDContext			inContext,
	GLenum				inType,
	const GLIDrawable	inDrawable )
{
	GLenum				theSuccess = GLI_BAD_DRAWABLE;
	
	DEBUG_ENTRY( gldAttachDrawable );

    DEBUG_PRINTF("gldAttachDrawable(%08lx,%d,%08lx)\n",inContext,inType,inDrawable);
    
	if( !inContext )
	{
		DEBUG_ERROR( gldAttachDrawable, "Invalid context pointer : 0x%08x\n", inContext);
		theSuccess = GLI_BAD_POINTER;
		goto ErrorExit;
	}
	
	if ( inType != inContext->drawableType && inType != GLI_NONE && inContext->drawableType != GLI_NONE)
	{
		DEBUG_ERROR( gldAttachDrawable, "current drawable (%d) is not the same type as drawable to attach (%d) attempting to detatch/reattach\n", inContext->drawableType, inType);
		glrDetachDrawable( inContext );
		}
		
		switch ( inType )
		{
			
		case GLI_NONE :
			if(inDrawable){
				DEBUG_ERROR(gldAttachDrawable, "warning: drawable type is GLI_NONE, unexpected non NULL drawable(%08lx)\n", inDrawable);
			}
			glrDetachDrawable( inContext );
			theSuccess = GLD_DRAWABLE_NULL;
			break;
		
			case GLI_OFFSCREEN :
				theSuccess = glrAttachOffscreenDrawable( inContext, inDrawable );
				break;
			
			case GLI_WINDOW :
				theSuccess = glrAttachWindowedDrawable( inContext, inDrawable );
				break;
			
			case GLI_FULLSCREEN :
				theSuccess = glrAttachFullScreenDrawable( inContext, inDrawable, NULL );
				break;
			
			default:
				DEBUG_ERROR( gldAttachDrawable, "unknown drawable type 0x%08x\n", inType);
				glrDetachDrawable( inContext );
				theSuccess = GLI_BAD_DRAWABLE;
				break;
			
		}

	
ErrorExit:	

	if ( theSuccess >= GLD_DRAWABLE_NONE && theSuccess < GLD_DRAWABLE_LAST )
	{
		char * theComment;
		
		inContext->drawable = inDrawable;
		
		switch ( theSuccess )
		{
			case GLD_DRAWABLE_NONE:
				theComment = "no changes to the current drawable... (GLD_DRAWABLE_NONE)";
				break;
			
			case GLD_DRAWABLE_NULL:
				theComment = "drawable detached (GLD_DRAWABLE_NULL)";
				break;
			
			case GLD_DRAWABLE_NEW:
				theComment = "a new drawable was attached (GLD_DRAWABLE_NEW)";
				break;
			
			case GLD_DRAWABLE_RESIZE:
				theComment = "the current drawable was resized (GLD_DRAWABLE_RESIZE)";
				break;
			
			case GLD_DRAWABLE_CTAB:
				theComment = "??? (GLD_DRAWABLE_CTAB)";
				break;
			
			default:
				theComment = "*** ERROR *** unknown return...";
				break;
		}

		DEBUG_VERBOSE( gldAttachDrawable, "%s\n", theComment );
	}
	else
	{
		DEBUG_ERROR( gldAttachDrawable, "failed to attach drawable!!! (%d)\n", theSuccess);
	}

	return theSuccess;
}


/*
________________________________________________________________________________________

      glrAttachOffscreenDrawable
________________________________________________________________________________________

*/

GLenum
glrAttachOffscreenDrawable(
	GLDContext			inContext,
	const GLIDrawable	inDrawable)
{
	GLIOffScreen *		theDrawable = (GLIOffScreen*) inDrawable;
	GLenum				theResult = GLD_DRAWABLE_NULL;

	DEBUG_ENTRY( glrAttachOffscreenDrawable );	
	DEBUG_NOT_YET_IMPLEMENTED( "glrAttachOffscreenDrawable()\h" );	

	theResult = glrInitializeWindowedGlideRenderingSurface( inContext, theDrawable->width, theDrawable->height, theDrawable->rowbytes );

	/*
	
	Fix me: 
	
	make up a fake offscreen buffer before we setup the blit
	glrSetupGlideSurfaceBlit( inContext, theDrawableWindow );
	*/
	
	inContext->drawableType = GLI_OFFSCREEN;
	theResult = GLD_DRAWABLE_RESIZE;

	return GLD_DRAWABLE_NONE;
}


/*
________________________________________________________________________________________

      glrAttachWindowedDrawable
________________________________________________________________________________________

*/

GLenum
glrAttachWindowedDrawable(
	GLDContext			inContext,
	const GLIDrawable	inDrawable)
{
	CGrafPtr			theDrawableWindow = (GLIWindow) inDrawable;
	PixMapPtr			theDrawableScreenPixMap = *theDrawableWindow->portPixMap;
	GLint				theWidth = theDrawableWindow->portRect.right - theDrawableWindow->portRect.left;
	GLint				theHeight = theDrawableWindow->portRect.bottom - theDrawableWindow->portRect.top;	
	GLenum				theResult = GLD_DRAWABLE_NULL;


	DEBUG_ENTRY( glrAttachWindowedDrawable );	

	DEBUG_VERBOSE( glrAttachWindowedDrawable, "window width = %d, height = %d\n", theWidth, theHeight, theDrawableWindow->portRect.top, theDrawableWindow->portRect.left  );	

	// if ( !gls_GlideInitialized ) glrInitializeGlide();

    /* Always force fullscreen on Voodoo2.  We have no choice really. */
	if ( !gls_surface_ext || glrIsDrawableFullScreen( inContext, theDrawableWindow ) )
	{
					
		GLIFullScreen	theGLIFullScreen;
		GLenum			theSuccess;
		Rect			winGlobalRect;

		if(inContext->drawableType == GLI_WINDOW){
			// context was previously attached to a window
			if ( inContext->renderSurface ) {
				// previous attachment was NOT fullscreen
				// so, detach from window then procede as normal
		
				DEBUG_VERBOSE( glrAttachWindowedDrawable, "Context being demoted from fullscreen...\n" );
				
				glrDetachDrawable(inContext);
			} else {
				// previous attachment WAS fullscreen...
				// ...still fullscreen
				// so, no change needed
				return GLD_DRAWABLE_NONE;
			}
		}

		DEBUG_VERBOSE( glrAttachWindowedDrawable, "Converting window to full screen...\n" );

		theGLIFullScreen.width = theWidth;
		theGLIFullScreen.height = theHeight;
		theGLIFullScreen.freq = 0;
		theGLIFullScreen.device = 0;
		
		glrGetWindowGlobalRect(theDrawableWindow, &winGlobalRect);
		
		theSuccess = glrAttachFullScreenDrawable( inContext, &theGLIFullScreen, theDrawableWindow);
		inContext->drawableType = GLI_WINDOW;
		
		MacMoveWindow((WindowPtr)theDrawableWindow, winGlobalRect.left, winGlobalRect.top, 0);

		return theSuccess;
	};

	if ( inContext->drawableType == GLI_WINDOW && !inContext->renderSurface ) {
		// the context was previously converted to full screen and now no longer fits
		// so, detach from full screen then procede as normal
		glrDetachDrawable(inContext);
	}

    /* A user-supplied buffer rect overrides the drawable dimensions */
    if(inContext->buffer_rect_enabled) {
      /* Have to clamp the width and height to the proper value. */
      GLint l, r, t, b;
      l = inContext->buffer_rect.x;
      r = l + inContext->buffer_rect.w;
      b = theHeight - inContext->buffer_rect.y;
      t = b - inContext->buffer_rect.h;
      
      /* Clamp it */
      if(l < 0) l = 0;
      if(t < 0) t = 0;
      if(r > theWidth) r = theWidth;
      if(b > theHeight) b = theHeight;
      
      /* Save off final window rect */
      inContext->window_rect.x = l;
      inContext->window_rect.y = t;
      inContext->window_rect.w = r - l;
      inContext->window_rect.h = b - t;

      /* Now recalc width & height */
      theWidth = r - l;
      theHeight = b - t;
    }
	if ( theWidth == 0 || theHeight == 0 )
	{			
		DEBUG_VERBOSE( glrAttachWindowedDrawable, "the drawable is empty (w = %d, h = %d)\n", theWidth, theHeight );
		glrDetachDrawable( inContext );		
		theResult = GLD_DRAWABLE_NULL;
	}		
	
	if ( inContext->viewPort.size.w != 0 )
	{
		if (!inContext->surfaceLost && inContext->viewPort.size.w == theWidth && inContext->viewPort.size.h == theHeight )
		{
			DEBUG_VERBOSE( glrAttachWindowedDrawable, "the drawable may have changed position...\n" );
			theResult = glrSetupGlideSurfaceBlit( inContext, theDrawableWindow );			
		}
		else
		{
			DEBUG_VERBOSE( glrAttachWindowedDrawable, "the drawable has changed size...\n" );		
			glrDetachDrawable( inContext );
			inContext->surfaceLost = GL_FALSE;
			
			theResult = glrSetupGlideSurfaceBlit( inContext, theDrawableWindow );

			if ( theResult == 0 )
			{
				theResult = glrInitializeWindowedGlideRenderingSurface( inContext, theWidth, theHeight, theWidth * theDrawableScreenPixMap->pixelSize >> 3 );
				if ( theResult == 0 )
				{				
					inContext->drawableType = GLI_WINDOW;
					theResult = GLD_DRAWABLE_RESIZE;
				}
			}
		}
	}
	else
	{
		theResult = glrSetupGlideSurfaceBlit( inContext, theDrawableWindow );
		if ( theResult == 0 )
		{
			theResult = glrInitializeWindowedGlideRenderingSurface( inContext, theWidth, theHeight, theWidth * theDrawableScreenPixMap->pixelSize >> 3 );
			if ( theResult == 0 )
			{
				inContext->drawableType = GLI_WINDOW;
				theResult = GLD_DRAWABLE_NEW;
			}
		}
	}

	return theResult;
}


/*
________________________________________________________________________________________

      glrAttachFullScreenDrawable
________________________________________________________________________________________

*/

GLenum
glrAttachFullScreenDrawable(
	GLDContext			inContext,
	const GLIDrawable	inDrawable,
	const CGrafPtr      inDrawableWindow)
{
	GLIFullScreen *		theGLIFullScreen = (GLIFullScreen *) inDrawable;
	GLenum				theResult = GLD_DRAWABLE_NULL;
	
	DEBUG_ENTRY( glrAttachFullScreenDrawable );	
	DEBUG_ENTRY_INPUT( glrAttachFullScreenDrawable, "width = %d, height = %d, Freq = %d, Device = %d\n", theGLIFullScreen->width, theGLIFullScreen->height, theGLIFullScreen->freq, theGLIFullScreen->device );


	if ( inContext->glideContext )
	{
		DEBUG_VERBOSE( glrAttachFullScreenDrawable, "a glide context is already open...\n" );
		goto ErrorExit;
	}	

	theResult = glrInitializeFullscreenGlide( inContext, theGLIFullScreen, inDrawableWindow );
	if (theResult == 0) {
		inContext->drawableType = GLI_FULLSCREEN;
		theResult = GLD_DRAWABLE_NEW;
	}

ErrorExit:
	return theResult;
}


/*
________________________________________________________________________________________

      glrDetachDrawable
________________________________________________________________________________________

fix me: we are not really detaching anything here

*/

static void
glrDetachDrawable(
	GLDContext			inContext)
{
// check if there is a context to detach!!!
    DEBUG_ENTRY( glrDetachDrawable );
	DEBUG_ENTRY_INPUT( glrDetachDrawable, "glrDetachDrawable(): drawable = %08lx, drawableType = %d\n",
	  inContext->drawable,inContext->drawableType);

    // If we're not attached, bail now. */
    if(inContext->drawableType == 0)
      return;
    
    // Make sure no rendering calls get made since everything is going away...
    glrNopDispatchTable(inContext);
    inContext->dispatch_table = 0;
    
	// Clear window data
	inContext->viewPort.size.w = 0;
	inContext->viewPort.size.h = 0;
	inContext->viewPort.offset.x = 0;
	inContext->viewPort.offset.y = 0;
	inContext->viewPort.deadband.w = 0;
	inContext->viewPort.deadband.h = 0;

	inContext->viewPortDirty = GL_TRUE;

	inContext->targetPort = 0;
	
	/* Make sure all textures are no longer marked resident */
	glrFreeAllTexturesVRAM(inContext);
	
	if(inContext->drawableType == GLI_FULLSCREEN) {
	  if(inContext->glideContext) {
        grSstWinClose(inContext->glideContext);
        inContext->glideContext = 0;
      }
	} else if(inContext->drawableType == GLI_WINDOW) {
	  if(!inContext->renderSurface) {
		// drawable is a full screen window
	  	Rect winGlobalRect;
	  
		glrGetWindowGlobalRect(inContext->drawable, &winGlobalRect);
				
		if(inContext->glideContext) {
			grSstWinClose(inContext->glideContext);
			inContext->glideContext = 0;
		}
		
		MacMoveWindow((WindowPtr)inContext->drawable, winGlobalRect.left, winGlobalRect.top, 0);

	  } else {
	  grSurfaceSetRenderingSurfaceExt(0);
	  grSurfaceSetAuxSurfaceExt(0);
	  grSurfaceSetTextureSurfaceExt(GR_TMU0,0);
	  grSurfaceSetTextureSurfaceExt(GR_TMU1,0);
	  
	  if(inContext->renderSurface) {
        grSurfaceReleaseExt(inContext->renderSurface);
        inContext->renderSurface = 0;
      }
	  if(inContext->auxSurface) {
        grSurfaceReleaseExt(inContext->auxSurface);
        inContext->auxSurface = 0;
      }
	  if(inContext->texture_surface) {
        grSurfaceReleaseExt(inContext->texture_surface);
        inContext->texture_surface = 0;
        inContext->surface_size[0] = 0;
        inContext->surface_size[1] = 0;        
      }
      if(inContext->glideContext) {
        grSurfaceReleaseContextExt(inContext->glideContext);
        inContext->glideContext = 0;
      }
	}
	}
	inContext->drawableType = 0;
	
	DEBUG_ENTRY_INPUT( glrDetachDrawable, "glrDetachDrable completed\n");
}


/*
________________________________________________________________________________________

      glrSetupGlideSurfaceBlit
________________________________________________________________________________________

*/

static GLenum
glrSetupGlideSurfaceBlit(
	GLDContext			inContext,
	const CGrafPtr		inTargetGrafPort)
{
	GLenum				theResult = GLI_BAD_DRAWABLE;
	
	DEBUG_ENTRY( glrSetupGlideSurfaceBlit );	

	//if ( !gls_GlideInitialized ) glrInitializeGlide();

	if ( inContext->glideContext )
	{
		// see if the window has moved to another screen
		
		if(inContext->device != glrGetWindowDevice(inTargetGrafPort))
	    {
	      // FIXME -- Something is still horribly broken with this.
	      
	      //glr_debug_printf("Detaching drawable since it's off of our device\n");
	      
	      glrDetachDrawable(inContext);
	      
	      //glr_debug_printf("drawable detached\n");
	      
	      if(inTargetGrafPort != (CGrafPtr)inContext->drawable)
	        theResult = GLD_DRAWABLE_NEW;
	      else
	        theResult = GLD_DRAWABLE_NONE;

          inContext->glideBoardSelectID = -1;

	    }
	    
		//if ( inTargetGrafPort->device != inContext->device )
		//{
		//	DEBUG_ERROR( glrSetupGlideSurfaceBlit, "the target device for this context has changed\n" );
		//	goto ErrorExit;
		//}
	}
	else
	{
		GDHandle theDevice;
		long theBoardID;

		DEBUG_VERBOSE( glrSetupGlideSurfaceBlit, "setting up a new glide context...\n" );

		theDevice = glrGetWindowDevice(inTargetGrafPort);
		theBoardID = glrDeviceToBoardID(theDevice);

		inContext->glideBoardSelectID = theBoardID;
        inContext->device = theDevice;
        
		if ( theBoardID < 0 )
		{
			DEBUG_ERROR( glrSetupGlideSurfaceBlit, "the target window is not on our hardware\n" );
			goto ErrorExit;
		}
		
		DEBUG_VERBOSE( glrSetupGlideSurfaceBlit, "glideBoardSelectID = %d\n", inContext->glideBoardSelectID );

		/* Fix me: check if the target offscreen is on one of our boards */
		DEBUG_NOT_YET_IMPLEMENTED( "glrInitializeOffscreenGlide() : check if the target offscreen is on one of our boards\n" );	
	}
	
	inContext->targetPort = inTargetGrafPort;
	
	theResult = GLD_DRAWABLE_NONE;

ErrorExit:
	return theResult;
}





/*
________________________________________________________________________________________

      glrIsDrawableFullScreen
________________________________________________________________________________________

*/

#define DEBUG_PRINT_RECT(r) DEBUG_PRINTF("(%d, %d, %d, %d)", (r)->left, (r)->top, (r)->right, (r)->bottom)


#include <stdio.h>
static GLenum
glrIsDrawableFullScreen(
	GLDContext			inContext,    
	const CGrafPtr		inDrawable)
{
	Rect				screenRect;
	Rect				windowRect;
	GLenum              theResult = GL_FALSE;
	GDHandle			theGDev;
	
	GLint h, w;

#if SUPPORT_CONFIG_FILE
    if(getenv("FX_DISABLE_FULLSCREEN")) {
      return GL_FALSE;
    }
    
    if(getenv("FX_FORCE_FULLSCREEN")) {
      return GL_TRUE;
    }
#endif

	theGDev = glrGetWindowDevice(inDrawable);
	
	if(theGDev == NULL) return GL_FALSE;
	
	screenRect = (**theGDev).gdRect;
	
	glrGetWindowGlobalRect(inDrawable, &windowRect);
	theResult = MacEqualRect(&screenRect, &windowRect);
	
//	DEBUG_PRINT_RECT(&screenRect);
//	DEBUG_PRINT_RECT(&windowRect);

	if (theResult && inDrawable->visRgn != 0){
		// check the vis region...
		if(!MacEqualRect(&inDrawable->portRect, &(*inDrawable->visRgn)->rgnBBox)){
			theResult = GL_FALSE;				
		}
	}
	
	h = inDrawable->portRect.bottom - inDrawable->portRect.top;
	w = inDrawable->portRect.right - inDrawable->portRect.left;

    /* If there is a buffer rect enabled it has to match the full window */
    if(theResult && inContext->buffer_rect_enabled) {
      theResult = ((inContext->buffer_rect.x == 0) &&
                   (inContext->buffer_rect.y == 0) &&
                   (inContext->buffer_rect.w == w) &&
                   (inContext->buffer_rect.h == h));
                   
    }

    /* Likewise for an enabled swap rect */
    if(theResult && inContext->swap_rect_enabled) {
      theResult = ((inContext->swap_rect.x == 0) &&
                   (inContext->swap_rect.y == 0) &&
                   (inContext->swap_rect.w == w) &&
                   (inContext->swap_rect.h == h));
                   
    }
    	
	DEBUG_PRINTF("glrIsDrawableFullScreen: theResult = %d\n", theResult);

	return theResult;
}


GLint glrDeviceToBoardID(GDHandle gd)
{
	GLint i;
	
	if(!gls_surface_ext){
		// running on voodoo2
		return 0;
	}
	
	for(i = 0; i < glrDeviceListCount; i++){
		if(((GLint)glrDeviceList[i].systemDeviceId) == (GLint)(**gd).gdRefNum){
			return i;
		}
	}

	return -1;
}

void glrGetWindowGlobalRect(CWindowPtr window, Rect * globalRect)
{

	GrafPtr savePort;
	
	*globalRect = window->portRect;
	// port rect is local... convert to global
	GetPort(&savePort);
	SetPort((GrafPtr)window);
	LocalToGlobal((Point*)globalRect);
	LocalToGlobal(((Point*)globalRect) + 1);
	SetPort(savePort);	
}




// returns the GDevice that the given window mostly resides on
GDHandle glrGetWindowDevice(CWindowPtr win)
{
	Rect globalWindowRect;
	Rect deviceRect;
	GDHandle bestGD = NULL;
	GDHandle gd;
	
	GLint area, bestArea;
	bestArea = 0;
	
	glrGetWindowGlobalRect(win, &globalWindowRect);
	
	gd = GetDeviceList();
	while(gd){
		// make sure its an active screen device...
		if(TestDeviceAttribute(gd, screenDevice) && TestDeviceAttribute(gd, screenActive)){
			deviceRect = (**gd).gdRect;
			area = glrIntersectingRects(&deviceRect, &globalWindowRect);
			if(area > bestArea){
				bestArea = area;
				bestGD = gd;
			}
		}
		gd = GetNextDevice(gd);
	}
	
	return bestGD;
}










/* 
** Copyright (c) 1997, 3Dfx Interactive, Inc. 
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
** 
** 
*/ 
#include <windows.h>
#include <glide.h>
#include <math.h>
#include <memory.h>
#include <GL/gl.h>
#include "glint.h"

#include <stdio.h>

#define numPixelFormats 4

PIXELFORMATDESCRIPTOR PixelFormats[numPixelFormats] =
{
  // Double buffered RGB - No Z
  {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_MAIN_PLANE|PFD_DOUBLEBUFFER|PFD_DRAW_TO_WINDOW|
      PFD_SUPPORT_OPENGL|PFD_SWAP_EXCHANGE,
    PFD_TYPE_RGBA,
    24,
    8,0,8,8,8,16,0,0,
    0,0,0,0,0,
    0,
    0,
    0,
    0,
    0,
    0,0,0
  },
  // Single Buffered RGB - No Z
  {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_MAIN_PLANE|PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL,
    PFD_TYPE_RGBA,
    24,
    8,0,8,8,8,16,0,0,
    0,0,0,0,0,
    0,
    0,
    0,
    0,
    0,
    0,0,0
  },
  // Double buffered RGB - Z buffer
  {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_MAIN_PLANE|PFD_DOUBLEBUFFER|PFD_DRAW_TO_WINDOW|
      PFD_SUPPORT_OPENGL|PFD_SWAP_EXCHANGE,
    PFD_TYPE_RGBA,
    24,
    8,0,8,8,8,16,0,0,
    0,0,0,0,0,
    16,
    0,
    0,
    0,
    0,
    0,0,0
  },
  // Single Buffered RGB - Z buffer
  {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_MAIN_PLANE|PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL,
    PFD_TYPE_RGBA,
    24,
    8,0,8,8,8,16,0,0,
    0,0,0,0,0,
    16,
    0,
    0,
    0,
    0,
    0,0,0
  }
};

int APIENTRY wglChoosePixelFormat(HDC hdc, CONST PIXELFORMATDESCRIPTOR * ppfd)
{
  if(ppfd->iPixelType != PFD_TYPE_RGBA)
    // Only RGB supported
    return(0);

//  if(ppfd->iLayerType != PFD_MAIN_PLANE)
//    // No Overlays
//    return(0);

  if(ppfd->dwFlags&PFD_DOUBLEBUFFER)
    {
      // Double Buffer
      if(ppfd->dwFlags&PFD_DEPTH_DONTCARE)
        {
          // No Z-buffer, Double Buffer
          return(1);
        } else {
          // Z-buffer, Double Buffer
          return(3);
        }
    } else {
      // Single Buffer
      if(ppfd->dwFlags&PFD_DEPTH_DONTCARE)
        {
          // No Z-buffer, Single Buffer
          return(2);
        } else {
          // Z-buffer, Single Buffer
          return(4);
        }
    }

  return(0);
}

int APIENTRY wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes,
                                    LPPIXELFORMATDESCRIPTOR ppfd)
{
  // Check for in range
  if((iPixelFormat < 1) || (iPixelFormat > 4))
    return(0);

  // Copy pixel format information
  memcpy(ppfd,&PixelFormats[iPixelFormat-1],sizeof(PIXELFORMATDESCRIPTOR));

  // Number of pixel formats
  return(4);
}

int APIENTRY wglGetPixelFormat(HDC hdc)
{
  return(curPixelFormat);
}

/*--------------------------------------------------------------------------
  HACK FOR CGDC DEMO PURPOSES
  --------------------------------------------------------------------------*/
#include "banner.inc"
static void renderBanner( void ) {
    static GrVertex vtxA, vtxB, vtxC, vtxD;
    static GrState state;
    int baseAddr;
    grGlideGetState( &state );
    grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
                    GR_COMBINE_FACTOR_ONE,
                    GR_COMBINE_LOCAL_NONE,
                    GR_COMBINE_OTHER_TEXTURE,
                    FXFALSE );

    grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
                    GR_COMBINE_FACTOR_ONE,
                    GR_COMBINE_LOCAL_NONE,
                    GR_COMBINE_OTHER_TEXTURE,
                    FXFALSE );
    grAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_ZERO,
                          GR_BLEND_ZERO, GR_BLEND_ZERO );
    grDepthMask( 0 );
    grDepthBufferMode( GR_DEPTHBUFFER_DISABLE );
    grColorMask( 1, 0 );
    grAlphaTestFunction( GR_CMP_ALWAYS );
    grChromakeyMode( GR_CHROMAKEY_DISABLE );
    grRenderBuffer(GR_BUFFER_BACKBUFFER);
    grCullMode(GR_CULL_DISABLE);
    grTexCombine( GR_TMU0,
                  GR_COMBINE_FUNCTION_LOCAL,
                  GR_COMBINE_FACTOR_NONE,
                  GR_COMBINE_FUNCTION_NONE,
                  GR_COMBINE_FACTOR_NONE,
                  FXFALSE, FXFALSE );
    grTexFilterMode( GR_TMU0,
                     GR_TEXTUREFILTER_BILINEAR,
                     GR_TEXTUREFILTER_BILINEAR );
    grTexMipMapMode( GR_TMU0,
                     GR_MIPMAP_NEAREST,
                     FXFALSE );
    grClipWindow( 0, 0, 
                 pglCurContext->WindowWidth, 
                 pglCurContext->WindowHeight );
    baseAddr = 
        grTexMaxAddress( GR_TMU0 ) - 
        grTexTextureMemRequired( GR_MIPMAPLEVELMASK_BOTH,
                                 &bannerInfo );
    grTexDownloadMipMap( GR_TMU0,
                         baseAddr,
                         GR_MIPMAPLEVELMASK_BOTH,
                         &bannerInfo );
    grTexSource( GR_TMU0,
                 baseAddr,
                 GR_MIPMAPLEVELMASK_BOTH,
                 &bannerInfo );
    
    /*---- 
      A-B
      |\|
      C-D
      -----*/
    vtxA.oow = vtxA.tmuvtx[0].oow = 1.0f;
    vtxB = vtxC = vtxD = vtxA;

    vtxA.x = vtxC.x = pglCurContext->WindowWidth  * 0.7f;
    vtxB.x = vtxD.x = pglCurContext->WindowWidth  * 1.0f;
    vtxA.y = vtxB.y = pglCurContext->WindowHeight * 0.1f;
    vtxC.y = vtxD.y = pglCurContext->WindowHeight * 0.0f;

    vtxA.tmuvtx[0].sow = vtxC.tmuvtx[0].sow =   0.0f;
    vtxB.tmuvtx[0].sow = vtxD.tmuvtx[0].sow = 255.0f;
    vtxA.tmuvtx[0].tow = vtxB.tmuvtx[0].tow =   0.0f;
    vtxC.tmuvtx[0].tow = vtxD.tmuvtx[0].tow =  32.0f;

    grDrawTriangle( &vtxA, &vtxD, &vtxC );
    grDrawTriangle( &vtxA, &vtxB, &vtxD );
    grGlideSetState( &state );
}

BOOL APIENTRY wglSwapBuffers(HDC hdc)
{


    if((curPixelFormat&1)&(pglCurContext != NULL)) {
        // Double buffered
//        renderBanner();
        grBufferSwap( 1 );
    }

  return(GL_TRUE);
}

BOOL APIENTRY wglSetPixelFormat(HDC hdc, int iPixelFormat,
                                CONST PIXELFORMATDESCRIPTOR *ppfd)
{
  if(iPixelFormat > 4)
      return(GL_FALSE);

  // DEBUG - Disable Double Buffer 
//  curPixelFormat = iPixelFormat&0xfe;
  curPixelFormat = iPixelFormat;

  return(GL_TRUE);
}

void APIENTRY wglGetDefaultProcAddress(DWORD dwd)
{
}

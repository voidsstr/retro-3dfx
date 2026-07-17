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
#include <GL/gl.h>
#include "glint.h"

#include <stdio.h>

static ATOM _glAtom;

HGLRC APIENTRY wglCreateContext(HDC hdc)
{
  pglContext pglNewContext;

  pglNewContext = (pglContext)malloc(sizeof(glContext));

  pglNewContext->hdc = NULL;

  glIntInitContext(pglNewContext);

  return((HGLRC)pglNewContext);
}

BOOL  APIENTRY wglDeleteContext(HGLRC hglrc)
{
  if(hglrc == (HGLRC)pglCurContext) {
//    SetProp( pglCurContext->hwnd, (LPCTSTR)_glAtom, (HANDLE)NULL);
    free((void *)hglrc);
    pglCurContext = NULL;
  }

  return(GL_TRUE);
}

HGLRC APIENTRY wglGetCurrentContext(void)
{
  return((HGLRC)pglCurContext);
}

HDC   APIENTRY wglGetCurrentDC(void)
{
  if(pglCurContext == NULL)
    return((HDC)NULL);
  else
    return((HDC)pglCurContext->hdc);
}

/* MARK DELTAS */

static void
BindWindow(void *hWnd, pglContext ctx) {
    static int first = 1;
 
    if ( first ) {
        first = 0;
        _glAtom = GlobalAddAtom("3DFX GL Atom");
    }

    SetProp( hWnd, (LPCTSTR)_glAtom, (HANDLE)ctx);
}

static pglContext 
WndToCtx(HWND hWnd) {
    pglContext ctx = NULL;

    ctx = (pglContext) GetProp(hWnd, (LPCTSTR)_glAtom );

    return ctx;
}

static LONG WINAPI
WinProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
  pglContext ctx;
  
  if ((ctx = WndToCtx(hWnd)) == NULL ) {
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
  }
  
  switch( uMsg ) {
  case WM_CLOSE:
    break;
  case WM_ACTIVATE:
    if(ctx) {
      int fActive = LOWORD(wParam);
      
      if(fActive == WA_ACTIVE)  {
        grSstControl(GR_CONTROL_ACTIVATE);
      }
      if(fActive == WA_INACTIVE) {
        grSstControl(GR_CONTROL_DEACTIVATE);
      }
    }
    break;
  case WM_ACTIVATEAPP:
    grSstControl(wParam?GR_CONTROL_ACTIVATE:GR_CONTROL_DEACTIVATE);
    break;
  case WM_MOVE:
    if ( !ctx->FullScreen ) {
      grSstControl(GR_CONTROL_MOVE);
    }
    break;
  case WM_SIZE:
    ctx->WindowWidth = LOWORD(lParam);
    ctx->WindowHeight = HIWORD(lParam);
    if ( !ctx->FullScreen ) {
      grSstControl(GR_CONTROL_RESIZE);
    } else {
      if(ctx->WindowWidth > 640) ctx->WindowWidth = 640;
      if(ctx->WindowHeight > 480) ctx->WindowHeight = 480;
      
      grColorMask(FXTRUE,FXFALSE);
      grClipWindow(0,0,640,480);
      grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      grBufferClear(0x0,0x0,0xffff);
      if(curPixelFormat&1) {
        grRenderBuffer(GR_BUFFER_BACKBUFFER);
        grBufferClear(0x0,0x0,0xffff);
      }
      pglCurContext->ContextDirty = TRUE;
    }
    break;
  default:
    break ;
  }
  
  return ((WNDPROC)curWinProc)(hWnd, uMsg, wParam, lParam);
}

static GrHwConfiguration HwConfig;
static int vid_init;
static int vid_status;

int gl_vid_init( void ) {
  HWND hWnd;

  hWnd = GetActiveWindow();

  if ( !vid_init ) {
    if(grSstWinOpen((FxU32)hWnd,
                    GR_RESOLUTION_NONE,
                    GR_REFRESH_60Hz,
                    GR_COLORFORMAT_ABGR,
                    GR_ORIGIN_LOWER_LEFT,
                    2,1)) {
      grSstQueryHardware(&HwConfig);
      if(HwConfig.SSTs[0].type == GR_SSTTYPE_VOODOO)
        vid_status = GL_TRUE;
      else
        vid_status = GL_FALSE;
    } else {
      if(grSstWinOpen(0,
                      GR_RESOLUTION_640x480,
                      GR_REFRESH_60Hz,
                      GR_COLORFORMAT_ABGR,
                      GR_ORIGIN_LOWER_LEFT,
                      2,1)) {
        vid_status = GL_TRUE;
      } else {
        vid_status = -1;
      }
    }
    vid_init = 1;
  }
  return vid_status;
}


void gl_vid_shutdown( void ) {
  if (vid_init) {
    grSstWinClose();
    vid_init = 0;
  }
}  

BOOL  APIENTRY wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
  RECT rc;
  HWND hWnd;

  hWnd = WindowFromDC(hdc);

  if((hdc == NULL) && (hglrc == NULL)) {
    if(pglCurContext != NULL) {
      gl_vid_shutdown();
    }
    pglCurContext = NULL;
    return(GL_TRUE);
  }

  if(pglCurContext != NULL) {

    if(pglCurContext->hdc == hdc)
      return(GL_TRUE);
    else
      gl_vid_shutdown();
  }

  pglCurContext = (pglContext)hglrc; 

  pglCurContext->hdc = hdc;
  pglCurContext->hwnd = hWnd;

  pglCurContext->FullScreen = gl_vid_init();
  if ( pglCurContext->FullScreen == -1 ) {
    pglCurContext = 0;
    return( GL_FALSE );
  }

  if(GetProp(hWnd, (LPCTSTR)_glAtom ) == NULL) {
    BindWindow(hWnd, pglCurContext);
    curWinProc = (void *)SetWindowLong( hWnd, GWL_WNDPROC, (LONG)WinProc);
    GetClientRect(hWnd, &rc);
    pglCurContext->WindowWidth = rc.right;
    pglCurContext->WindowHeight = rc.bottom;
    if (pglCurContext->FullScreen) {
      if(pglCurContext->WindowWidth > 640) pglCurContext->WindowWidth = 640;
      if(pglCurContext->WindowHeight > 480) pglCurContext->WindowHeight = 480;
    }
  }
  
  // Setup Buffering
  if((curPixelFormat&1)&(pglCurContext != NULL))
    {
      // Double Buffer
      grColorMask(FXTRUE,FXFALSE);
      grClipWindow(0,0,640,480);
      grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      grBufferClear(0x0,0x0,0xffff);
      grRenderBuffer(GR_BUFFER_BACKBUFFER);
      grBufferClear(0x0,0x0,0xffff);
      pglCurContext->ContextDirty = TRUE;
    } else {
      // Single Buffer
      grColorMask(FXTRUE,FXFALSE);
      grClipWindow(0,0,640,480);
      grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      grBufferClear(0x0,0x0,0xffff);
      pglCurContext->ContextDirty = TRUE;
    }

  pglCurContext->ContextDirty = TRUE;

  return(GL_TRUE);
}

BOOL APIENTRY wglCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask)
{
  return(GL_FALSE);
}

BOOL  APIENTRY wglShareLists(HGLRC hglrc1, HGLRC hglrc2)
{
  return(GL_FALSE);
}

PROC  APIENTRY wglGetProcAddress(LPCSTR lpszProc)
{
  PROC rv;

  rv = (PROC)0;
  
  if (!strcmp(lpszProc,"gl3DfxNullExt")) {
    rv = (PROC)0;
  }
  
  return(rv);
}

HGLRC APIENTRY wglCreateLayerContext(HDC hdc, int iLayerPlane)
{
  return((HGLRC)0);
}

BOOL APIENTRY wglDescribeLayerPlane(HDC hdc , int iPixelFormat,
                                    int iLayerPlane, UINT nBytes,
                                    LPLAYERPLANEDESCRIPTOR plpd)
{
  return(GL_FALSE);
}

int  APIENTRY wglSetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart,
                                        int cEntries,
                                        CONST COLORREF *pcr)
{
  return(0);
}

int  APIENTRY wglGetLayerPaletteEntries(HDC hdc, int iLayerPlane,
                                        int iStart, int cEntries,
                                        COLORREF *pcr)
{
  return(0);
}

BOOL APIENTRY wglRealizeLayerPalette(HDC hdc, int iLayerPlane, BOOL bRealize)
{
  return(GL_FALSE);
}

BOOL APIENTRY wglSwapLayerBuffers(HDC hdc, UINT fuPlanes)
{
  return(GL_FALSE);
}






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
#include "fgl.h"

#include <stdio.h>

FGL_hwInfo tdfxinfo[] = {
  "3DFX.DRV",
  2048*1024,
  -1,
  2048*1024
};

FGL_modeInfo tdfxmode[] = { 
  640,
  480,
  16,
  1024*2,
  2,
  {0x5,0xB,0x6,0x5,0x5,0x0,0x0,0x0}
};

#define NUM_MODES (sizeof(tdfxmode)/sizeof(tdfxmode[0]))

static int fgl_video_mode = 0;
static int fgl_buffer_mode = 0;

bool    WINAPI fglDetect(int *numModes,
                         FGL_hwInfo *hwInfo,
                         FGL_modeInfo *modes)
{
  *numModes = NUM_MODES;
  *hwInfo = tdfxinfo[0];
  *modes = tdfxmode[0];

  return(TRUE);
}

bool    WINAPI fglSetVideoMode(int mode)
{
  if(mode >= NUM_MODES)
    return(FALSE);

  fgl_video_mode = mode;

  gl_vid_init();
  
  return(TRUE);
}

void    WINAPI fglSetFocus(bool active)
{
  if(active) {
    grSstControl( GR_CONTROL_ACTIVATE );
  } else {
    grSstControl( GR_CONTROL_DEACTIVATE );
  }
}

void    WINAPI fglRestoreMode(void)
{
//    grSstPassthruMode(0);
}

void    * WINAPI fglBeginDirectAccess(int buffer)
{
  GrLfbInfo_t info;

  switch(buffer) {
  case FGL_FRONT_BUFFER:
    if ( !grLfbLock(GR_LFB_WRITE_ONLY, GR_BUFFER_FRONTBUFFER,
                    GR_LFBWRITEMODE_565, GR_ORIGIN_UPPER_LEFT, FXTRUE,
                    &info ) ) {
      return(NULL);
    } else {
      grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_FRONTBUFFER,
                GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXTRUE,
                &info );
      fgl_buffer_mode = buffer;
      return((void *)info.lfbPtr);
    }
    break;
  case FGL_BACK_BUFFER:
    if ( !grLfbLock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER,
                    GR_LFBWRITEMODE_565, GR_ORIGIN_UPPER_LEFT, FXTRUE,
                    &info ) ) {
      return(NULL);
    } else {
      grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER,
                GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXTRUE,
                &info );
      fgl_buffer_mode = buffer;
      return((void *)info.lfbPtr);
    }
    break;
  case FGL_Z_BUFFER:
    if ( !grLfbLock(GR_LFB_WRITE_ONLY, GR_BUFFER_DEPTHBUFFER,
                    GR_LFBWRITEMODE_ZA16, GR_ORIGIN_UPPER_LEFT, FXTRUE,
                    &info ) ) {
      return(NULL);
    } else {
      grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_DEPTHBUFFER,
                GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXTRUE,
                &info );
      fgl_buffer_mode = buffer;
      return((void *)info.lfbPtr);
    }
    break;
  default:
    break;
  }
  return(NULL);
  
}

void    WINAPI fglEndDirectAccess(void)
{
  switch(fgl_buffer_mode) {
  case FGL_FRONT_BUFFER:
    grLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_FRONTBUFFER);
    grLfbUnlock( GR_LFB_READ_ONLY, GR_BUFFER_FRONTBUFFER);
    break;
  case FGL_BACK_BUFFER:
    grLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);
    grLfbUnlock( GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER);
    break;
  case FGL_Z_BUFFER:
    grLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_DEPTHBUFFER);
    grLfbUnlock( GR_LFB_READ_ONLY, GR_BUFFER_DEPTHBUFFER);
    break;
  default:
    break;
  }
}

HGLRC   WINAPI fglCreateContext(HDC hdc)
{
  return(wglCreateContext(hdc));
}

void    WINAPI fglDeleteContext(HGLRC rc)
{
  wglDeleteContext(rc);
}

bool    WINAPI fglMakeCurrent(HDC dc, HGLRC rc)
{
  return(wglMakeCurrent(dc, rc));
}

bool    WINAPI fglSwapBuffers(bool waitVRT)
{
  if((curPixelFormat&1)&(pglCurContext != NULL)) {
    // Double buffered 
    if(waitVRT)
      grBufferSwap( 1 );
    else
      grBufferSwap( 0 );
  }
  return(TRUE);
}

void    WINAPI fglRealizePalette(palette_t *pal,
                                 int numColors,
                                 int startIndex,
                                 int waitVRT)
{
}


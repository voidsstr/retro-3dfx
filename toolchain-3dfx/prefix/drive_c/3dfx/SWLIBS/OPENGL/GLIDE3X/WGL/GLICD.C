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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 2$ 
** $Date: 10/11/00 7:58:23 PM$ 
**
*/


#define WNDOBJ   void
#define XLATEOBJ void

#include "wgllib.h"
#include "g_xproto.h"

#include "gldrv.h"

/* crash-robust debug logging to C:\3dfxogl.log (defined in wglcmds.c) */
extern void OGLLOG( const char *fmt, ... );


GLCLTPROCTABLE procTable = {
  /* Number of function entries in the table */
  sizeof(GLDISPATCHTABLE) / sizeof(PROC),

  /* gl Entry points */
  {
    glNewList                ,
    glEndList                ,
    glCallList               ,
    glCallLists              ,
    glDeleteLists            ,
    glGenLists               ,
    glListBase               ,
    glBegin                  ,
    glBitmap                 ,
    glColor3b                ,
    glColor3bv               ,
    glColor3d                ,
    glColor3dv               ,
    glColor3f                ,
    glColor3fv               ,
    glColor3i                ,
    glColor3iv               ,
    glColor3s                ,
    glColor3sv               ,
    glColor3ub               ,
    glColor3ubv              ,
    glColor3ui               ,
    glColor3uiv              ,
    glColor3us               ,
    glColor3usv              ,
    glColor4b                ,
    glColor4bv               ,
    glColor4d                ,
    glColor4dv               ,
    glColor4f                ,
    glColor4fv               ,
    glColor4i                ,
    glColor4iv               ,
    glColor4s                ,
    glColor4sv               ,
    glColor4ub               ,
    glColor4ubv              ,
    glColor4ui               ,
    glColor4uiv              ,
    glColor4us               ,
    glColor4usv              ,
    glEdgeFlag               ,
    glEdgeFlagv              ,
    glEnd                    ,
    glIndexd                 ,
    glIndexdv                ,
    glIndexf                 ,
    glIndexfv                ,
    glIndexi                 ,
    glIndexiv                ,
    glIndexs                 ,
    glIndexsv                ,
    glNormal3b               ,
    glNormal3bv              ,
    glNormal3d               ,
    glNormal3dv              ,
    glNormal3f               ,
    glNormal3fv              ,
    glNormal3i               ,
    glNormal3iv              ,
    glNormal3s               ,
    glNormal3sv              ,
    glRasterPos2d            ,
    glRasterPos2dv           ,
    glRasterPos2f            ,
    glRasterPos2fv           ,
    glRasterPos2i            ,
    glRasterPos2iv           ,
    glRasterPos2s            ,
    glRasterPos2sv           ,
    glRasterPos3d            ,
    glRasterPos3dv           ,
    glRasterPos3f            ,
    glRasterPos3fv           ,
    glRasterPos3i            ,
    glRasterPos3iv           ,
    glRasterPos3s            ,
    glRasterPos3sv           ,
    glRasterPos4d            ,
    glRasterPos4dv           ,
    glRasterPos4f            ,
    glRasterPos4fv           ,
    glRasterPos4i            ,
    glRasterPos4iv           ,
    glRasterPos4s            ,
    glRasterPos4sv           ,
    glRectd                  ,
    glRectdv                 ,
    glRectf                  ,
    glRectfv                 ,
    glRecti                  ,
    glRectiv                 ,
    glRects                  ,
    glRectsv                 ,
    glTexCoord1d             ,
    glTexCoord1dv            ,
    glTexCoord1f             ,
    glTexCoord1fv            ,
    glTexCoord1i             ,
    glTexCoord1iv            ,
    glTexCoord1s             ,
    glTexCoord1sv            ,
    glTexCoord2d             ,
    glTexCoord2dv            ,
    glTexCoord2f             ,
    glTexCoord2fv            ,
    glTexCoord2i             ,
    glTexCoord2iv            ,
    glTexCoord2s             ,
    glTexCoord2sv            ,
    glTexCoord3d             ,
    glTexCoord3dv            ,
    glTexCoord3f             ,
    glTexCoord3fv            ,
    glTexCoord3i             ,
    glTexCoord3iv            ,
    glTexCoord3s             ,
    glTexCoord3sv            ,
    glTexCoord4d             ,
    glTexCoord4dv            ,
    glTexCoord4f             ,
    glTexCoord4fv            ,
    glTexCoord4i             ,
    glTexCoord4iv            ,
    glTexCoord4s             ,
    glTexCoord4sv            ,
    glVertex2d               ,
    glVertex2dv              ,
    glVertex2f               ,
    glVertex2fv              ,
    glVertex2i               ,
    glVertex2iv              ,
    glVertex2s               ,
    glVertex2sv              ,
    glVertex3d               ,
    glVertex3dv              ,
    glVertex3f               ,
    glVertex3fv              ,
    glVertex3i               ,
    glVertex3iv              ,
    glVertex3s               ,
    glVertex3sv              ,
    glVertex4d               ,
    glVertex4dv              ,
    glVertex4f               ,
    glVertex4fv              ,
    glVertex4i               ,
    glVertex4iv              ,
    glVertex4s               ,
    glVertex4sv              ,
    glClipPlane              ,
    glColorMaterial          ,
    glCullFace               ,
    glFogf                   ,
    glFogfv                  ,
    glFogi                   ,
    glFogiv                  ,
    glFrontFace              ,
    glHint                   ,
    glLightf                 ,
    glLightfv                ,
    glLighti                 ,
    glLightiv                ,
    glLightModelf            ,
    glLightModelfv           ,
    glLightModeli            ,
    glLightModeliv           ,
    glLineStipple            ,
    glLineWidth              ,
    glMaterialf              ,
    glMaterialfv             ,
    glMateriali              ,
    glMaterialiv             ,
    glPointSize              ,
    glPolygonMode            ,
    glPolygonStipple         ,
    glScissor                ,
    glShadeModel             ,
    glTexParameterf          ,
    glTexParameterfv         ,
    glTexParameteri          ,
    glTexParameteriv         ,
    glTexImage1D             ,
    glTexImage2D             ,
    glTexEnvf                ,
    glTexEnvfv               ,
    glTexEnvi                ,
    glTexEnviv               ,
    glTexGend                ,
    glTexGendv               ,
    glTexGenf                ,
    glTexGenfv               ,
    glTexGeni                ,
    glTexGeniv               ,
    glFeedbackBuffer         ,
    glSelectBuffer           ,
    glRenderMode             ,
    glInitNames              ,
    glLoadName               ,
    glPassThrough            ,
    glPopName                ,
    glPushName               ,
    glDrawBuffer             ,
    glClear                  ,
    glClearAccum             ,
    glClearIndex             ,
    glClearColor             ,
    glClearStencil           ,
    glClearDepth             ,
    glStencilMask            ,
    glColorMask              ,
    glDepthMask              ,
    glIndexMask              ,
    glAccum                  ,
    glDisable                ,
    glEnable                 ,
    glFinish                 ,
    glFlush                  ,
    glPopAttrib              ,
    glPushAttrib             ,
    glMap1d                  ,
    glMap1f                  ,
    glMap2d                  ,
    glMap2f                  ,
    glMapGrid1d              ,
    glMapGrid1f              ,
    glMapGrid2d              ,
    glMapGrid2f              ,
    glEvalCoord1d            ,
    glEvalCoord1dv           ,
    glEvalCoord1f            ,
    glEvalCoord1fv           ,
    glEvalCoord2d            ,
    glEvalCoord2dv           ,
    glEvalCoord2f            ,
    glEvalCoord2fv           ,
    glEvalMesh1              ,
    glEvalPoint1             ,
    glEvalMesh2              ,
    glEvalPoint2             ,
    glAlphaFunc              ,
    glBlendFunc              ,
    glLogicOp                ,
    glStencilFunc            ,
    glStencilOp              ,
    glDepthFunc              ,
    glPixelZoom              ,
    glPixelTransferf         ,
    glPixelTransferi         ,
    glPixelStoref            ,
    glPixelStorei            ,
    glPixelMapfv             ,
    glPixelMapuiv            ,
    glPixelMapusv            ,
    glReadBuffer             ,
    glCopyPixels             ,
    glReadPixels             ,
    glDrawPixels             ,
    glGetBooleanv            ,
    glGetClipPlane           ,
    glGetDoublev             ,
    glGetError               ,
    glGetFloatv              ,
    glGetIntegerv            ,
    glGetLightfv             ,
    glGetLightiv             ,
    glGetMapdv               ,
    glGetMapfv               ,
    glGetMapiv               ,
    glGetMaterialfv          ,
    glGetMaterialiv          ,
    glGetPixelMapfv          ,
    glGetPixelMapuiv         ,
    glGetPixelMapusv         ,
    glGetPolygonStipple      ,
    glGetString              ,
    glGetTexEnvfv            ,
    glGetTexEnviv            ,
    glGetTexGendv            ,
    glGetTexGenfv            ,
    glGetTexGeniv            ,
    glGetTexImage            ,
    glGetTexParameterfv      ,
    glGetTexParameteriv      ,
    glGetTexLevelParameterfv ,
    glGetTexLevelParameteriv ,
    glIsEnabled              ,
    glIsList                 ,
    glDepthRange             ,
    glFrustum                ,
    glLoadIdentity           ,
    glLoadMatrixf            ,
    glLoadMatrixd            ,
    glMatrixMode             ,
    glMultMatrixf            ,
    glMultMatrixd            ,
    glOrtho                  ,
    glPopMatrix              ,
    glPushMatrix             ,
    glRotated                ,
    glRotatef                ,
    glScaled                 ,
    glScalef                 ,
    glTranslated             ,
    glTranslatef             ,
    glViewport               ,
    // OpenGL version 1.0 entries end here

    // OpenGL version 1.1 entries begin here
    glArrayElement           ,
    glBindTexture            ,
    glColorPointer           ,
    glDisableClientState     ,
    glDrawArrays             ,
    glDrawElements           ,
    glEdgeFlagPointer        ,
    glEnableClientState      ,
    glIndexPointer           ,
    glIndexub                ,
    glIndexubv               ,
    glInterleavedArrays      ,
    glNormalPointer          ,
    glPolygonOffset          ,
    glTexCoordPointer        ,
    glVertexPointer          ,
    glAreTexturesResident    ,
    glCopyTexImage1D         ,
    glCopyTexImage2D         ,
    glCopyTexSubImage1D      ,
    glCopyTexSubImage2D      ,
    glDeleteTextures         ,
    glGenTextures            ,
    glGetPointerv            ,
    glIsTexture              ,
    glPrioritizeTextures     ,
    glTexSubImage1D          ,
    glTexSubImage2D          ,
    glPopClientAttrib        ,
    glPushClientAttrib       
  }
};

BOOL APIENTRY DrvCopyContext( DHGLRC dhrcSource, DHGLRC dhrcDest, UINT fuMask ) {
//  GL_BEGIN( "DrvCopyContext", 55 );
//  GL_END( (BOOL)0 );
        return (BOOL)0;
}

DHGLRC APIENTRY DrvCreateContext( HDC hdc ) {
//  GL_BEGIN( "DrvCreateContext", 55 );
//  GL_END( (DHGLRC)0 );
  OGLLOG( "DrvCreateContext hdc=0x%x -> 0 (unsupported, use LayerContext)",
          (unsigned)hdc );
        return (DHGLRC)0;
}

HGLRC WINAPI
wglCreateContext(HDC hDC);

DHGLRC APIENTRY DrvCreateLayerContext( HDC hdc, INT iLayerPlane ) {
  DHGLRC rv = 0;
//  GL_BEGIN( "DrvCreateLayerContext", 55);
//  GL_INFO(( 55, "hdc 0x%.08x\n", hdc ));
//  GL_INFO(( 55, "ilp 0x%.08x\n", iLayerPlane ));
//  GL_END( (DHGLRC)0xbeeff00d );
  OGLLOG( "DrvCreateLayerContext hdc=0x%x plane=%d", (unsigned)hdc,
          (int)iLayerPlane );
  rv = (DHGLRC)wglCreateContext( hdc );
  OGLLOG( "DrvCreateLayerContext -> 0x%x%s", (unsigned)rv,
          rv ? "" : "  ** FAILED **" );

        return rv;
}

BOOL WINAPI
wglDeleteContext(HGLRC hGLRC);

BOOL APIENTRY DrvDeleteContext( DHGLRC dhglrc ) {
  BOOL rv;
//  GL_BEGIN( "DrvDeleteContext", 55);
//  GL_END( (BOOL)0 );
  OGLLOG( "DrvDeleteContext dhglrc=0x%x", (unsigned)dhglrc );
  rv = wglDeleteContext( (HGLRC) dhglrc );
        return rv;
}

BOOL APIENTRY DrvDescribeLayerPlane(HDC hdc,
                                    INT iPixelFormat,
                                    INT iLayerPlane,
                                    UINT nBytes,
                                    LPLAYERPLANEDESCRIPTOR plpd ) {
//  GL_BEGIN( "DrvDescribeLayerPlane", 55);
//  GL_END( (BOOL)0 );
        return (BOOL)0;
}

INT APIENTRY DrvGetLayerPaletteEntries( HDC hdc,
                                        INT iLayerPlane,
                                        INT iStart,
                                        INT cEntries,
                                        COLORREF* pcr ) {
//  GL_BEGIN( "DrvGetLayerPalettEntries", 55);
//  GL_END( (INT)0 );
        return (INT)0;
}

PROC APIENTRY DrvGetProcAddress ( LPCSTR lpszProc ) {
//  GL_BEGIN( "DrvGetProcAddress", 55);
//  GL_END( (PROC)0 );
  /* OPT 0.1.4: resolve extension entry points (GL_ARB_multitexture etc.)
  ** from the shared wgl extension-proc table.  MS opengl32.dll forwards
  ** app wglGetProcAddress calls here for names it doesn't know. */
  {
    extern PROC __wglFindExtProc( LPCSTR );
    PROC p = __wglFindExtProc( lpszProc );
    /* cap the spam: apps probe many extension entry points */
    static int nGPA = 0;
    if ( nGPA < 32 ) {
      nGPA++;
      OGLLOG( "DrvGetProcAddress('%s') -> 0x%x",
              lpszProc ? lpszProc : "(null)", (unsigned)p );
    }
    return p;
  }
}

INT APIENTRY DrvRealizeLayerPalette( HDC hdc, INT iLayerPlane, BOOL bRealize ) {
//  GL_BEGIN( "DrvRealizeLayerPalette", 55);
//  GL_END( (INT)0 );
        return (INT)0;
}


BOOL WINAPI
wglMakeCurrent(HDC hDC, HGLRC hGLRC);

HDC WINAPI
wglGetCurrentDC(void);

BOOL APIENTRY DrvReleaseContext( DHGLRC dhglrc ) {
  BOOL rv;
//  GL_BEGIN( "DrvReleaseContext", 55);
//  GL_END( (BOOL)0 );
  rv = wglMakeCurrent( wglGetCurrentDC(),0 );
        return rv;
}

VOID APIENTRY DrvSetCallbackProcs ( INT nProcs, PROC* pProcs ) {
//  GL_BEGIN( "DrvSetCallbackProcs", 55);
//  GL_END( ; );
        return;
}

BOOL WINAPI
wglMakeCurrent(HDC hDC, HGLRC hGLRC);

PGLCLTPROCTABLE APIENTRY DrvSetContext( HDC    hdc,
                                        DHGLRC dhglrc,
                                        PFN_SETPROCTABLE pfnSetProcTable ) {
  BOOL mcrv;
//  GL_BEGIN( "DrvSetContext", 55);
//  GL_INFO((55, "hdc    0x%.08x\n", hdc ));
//  GL_INFO((55, "dhglrc 0x%.08x\n", dhglrc ));
//  GL_INFO((55, "pfnSPT 0x%.08x\n", pfnSetProcTable ));
//  GL_END( &procTable );
  OGLLOG( "DrvSetContext hdc=0x%x dhglrc=0x%x", (unsigned)hdc,
          (unsigned)dhglrc );
  mcrv = wglMakeCurrent( hdc, (HGLRC)dhglrc );
  OGLLOG( "DrvSetContext: wglMakeCurrent -> %d%s", (int)mcrv,
          mcrv ? "" : "  ** FAILED **" );

  return &procTable;
}

INT APIENTRY DrvSetLayerPaletteEntries( HDC hdc,
                                        INT iLayerPlane,
                                        INT iStart,
                                        INT cEntries,
                                        CONST COLORREF* pcr ) {
//  GL_BEGIN( "DrvsetLayerPaletteEntries", 55);
//  GL_END( (INT)0 );
        return (INT)0;
}

BOOL APIENTRY DrvShareLists ( DHGLRC dhglrc1,
                              DHGLRC dhglrc2 ) {
//  GL_BEGIN( "DrvShareLists", 55);
//  GL_END( (BOOL)0 );
        return (BOOL)0;
}

BOOL APIENTRY DrvSwapLayerBuffers( HDC  hdc,
                                   UINT fuPlanes ) {
//  GL_BEGIN( "DrvSwapLayerBuffers", 55);
//  GL_END( (BOOL)0 );
        return (BOOL)0;
}

BOOL APIENTRY DrvValidateVersion( ULONG ulVersion ) {
  BOOL rv = FALSE;
//  GL_BEGIN( "DrvValidateVersion", 55);
  if ( ulVersion == 1 ) rv = TRUE;
//  GL_END( rv );
  OGLLOG( "DrvValidateVersion(%lu) -> %d", ulVersion, (int)rv );
  return rv;
}

BOOL WINAPI
wglSwapBuffers(HDC hDC);

BOOL APIENTRY DrvSwapBuffers( HDC hdc ) {
  BOOL rv;
  static int firstSwap = 1;
  //  GL_BEGIN( "DrvSwapBuffers", 55);
  //  GL_END( (BOOL)0 );
  if ( firstSwap )
    OGLLOG( "DrvSwapBuffers first call hdc=0x%x", (unsigned)hdc );
  rv = wglSwapBuffers( hdc );
  if ( firstSwap ) {
    firstSwap = 0;
    OGLLOG( "DrvSwapBuffers first call -> %d (further swaps not logged)",
            (int)rv );
  }
        return rv;
}

int WINAPI wglDescribePixelFormat(HDC hDC, int iPixelFormat,
                                  UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd);


PIXELFORMATDESCRIPTOR pfds[] = {
        { sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
          1,                              // version number
          PFD_DRAW_TO_WINDOW              // support window
          |  PFD_SUPPORT_OPENGL           // support OpenGL
          |  PFD_DOUBLEBUFFER,            // double buffered
          PFD_TYPE_RGBA,                  // RGBA Color
          24,                             // 24-bit color depth
          0, 0, 0, 0, 0, 0,               // color bits ignored
          0,                              // no alpha buffer
          0,                              // shift bit ignored
          0,                              // no accumulation buffer
          0, 0, 0, 0,                     // accum bits ignored
          32,                             // 32-bit z-buffer      
          0,                              // no stencil buffer
          0,                              // no auxiliary buffer
          PFD_MAIN_PLANE,                 // main layer
          0,                              // reserved
          0, 0, 0                         // layer masks ignored
        },
        { sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
          1,                              // version number
          PFD_DRAW_TO_WINDOW              // support window
          |  PFD_SUPPORT_OPENGL,          // support OpenGL
          PFD_TYPE_RGBA,                  // RGBA Color
          24,                             // 24-bit color depth
          0, 0, 0, 0, 0, 0,               // color bits ignored
          0,                              // no alpha buffer
          0,                              // shift bit ignored
          0,                              // no accumulation buffer
          0, 0, 0, 0,                     // accum bits ignored
          32,                             // 32-bit z-buffer      
          0,                              // no stencil buffer
          0,                              // no auxiliary buffer
          PFD_MAIN_PLANE,                 // main layer
          0,                              // reserved
          0, 0, 0                         // layer masks ignored
        },
        { sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
          1,                              // version number
          PFD_DRAW_TO_WINDOW              // support window
          |  PFD_SUPPORT_OPENGL           // support OpenGL
          |  PFD_DOUBLEBUFFER,            // double buffered
          PFD_TYPE_RGBA,                  // RGBA Color
          24,                             // 24-bit color depth
          0, 0, 0, 0, 0, 0,               // color bits ignored
          0,                              // no alpha buffer
          0,                              // shift bit ignored
          64,                              // no accumulation buffer
          16, 16, 16, 16,                     // accum bits ignored
          32,                             // 32-bit z-buffer      
          0,                              // no stencil buffer
          0,                              // no auxiliary buffer
          PFD_MAIN_PLANE,                 // main layer
          0,                              // reserved
          0, 0, 0                         // layer masks ignored
        },
        { sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
          1,                              // version number
          PFD_DRAW_TO_WINDOW              // support window
          |  PFD_SUPPORT_OPENGL,          // support OpenGL
          PFD_TYPE_RGBA,                  // RGBA Color
          24,                             // 24-bit color depth
          0, 0, 0, 0, 0, 0,               // color bits ignored
          0,                              // no alpha buffer
          0,                              // shift bit ignored
          64,                              // no accumulation buffer
          16, 16, 16, 16,                     // accum bits ignored
          32,                             // 32-bit z-buffer      
          0,                              // no stencil buffer
          0,                              // no auxiliary buffer
          PFD_MAIN_PLANE,                 // main layer
          0,                              // reserved
          0, 0, 0                         // layer masks ignored
        }
};

LONG APIENTRY DrvDescribePixelFormat( HDC hdc, 
                                      LONG ipfd, 
                                      ULONG cjpfd, 
                                      PIXELFORMATDESCRIPTOR *ppfd ) {
  int rv;
  static int nDPF = 0;

  rv = 4;

  if ( nDPF < 24 ) {
    nDPF++;
    OGLLOG( "DrvDescribePixelFormat hdc=0x%x ipfd=%ld cjpfd=%lu -> count=%d",
            (unsigned)hdc, ipfd, cjpfd, rv );
  }

  if ( ipfd && ppfd && cjpfd ) {
    if ( ipfd < 1 || ipfd > (LONG)(sizeof(pfds)/sizeof(pfds[0])) )
      OGLLOG( "DrvDescribePixelFormat ** ipfd=%ld OUT OF RANGE (1..4) **",
              ipfd );
    memset( ppfd, 0, sizeof( PIXELFORMATDESCRIPTOR ) );
	memcpy( ppfd, &pfds[ipfd-1], sizeof( PIXELFORMATDESCRIPTOR ) );
  }

  return rv;
}

BOOL WINAPI
wglSetPixelFormat(HDC hDC, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd);

int WINAPI
wglChoosePixelFormat(HDC hDC, const PIXELFORMATDESCRIPTOR *ppfd);


BOOL APIENTRY DrvSetPixelFormat( HDC hdc, LONG ipfd ) {
    static PIXELFORMATDESCRIPTOR pfd;
	int    pixelFormat;
	BOOL   sprv;

	OGLLOG( "DrvSetPixelFormat hdc=0x%x ipfd=%ld", (unsigned)hdc, ipfd );
	if ( ipfd < 1 || ipfd > (LONG)(sizeof(pfds)/sizeof(pfds[0])) )
	  OGLLOG( "DrvSetPixelFormat ** ipfd=%ld OUT OF RANGE (1..4) **", ipfd );

	pfd = pfds[ipfd-1];

	OGLLOG( "DrvSetPixelFormat pfd: flags=0x%lx type=%d color=%d depth=%d "
	        "stencil=%d accum=%d",
	        pfd.dwFlags, (int)pfd.iPixelType, (int)pfd.cColorBits,
	        (int)pfd.cDepthBits, (int)pfd.cStencilBits, (int)pfd.cAccumBits );

    pixelFormat = wglChoosePixelFormat(hdc, &pfd );
    OGLLOG( "DrvSetPixelFormat: wglChoosePixelFormat -> %d", pixelFormat );
    sprv = wglSetPixelFormat( hdc, pixelFormat, &pfd );
    OGLLOG( "DrvSetPixelFormat: wglSetPixelFormat -> %d%s", (int)sprv,
            sprv ? "" : "  ** FAILED **" );
    return TRUE;
}

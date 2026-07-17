/*
** Copyright 1991-1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
*/
#include <ddraw.h>
#include "context.h"
#include "imports.h"
#include "global.h"
#include "g_imfncs.h"
#include "types.h"
#include "namesint.h"
#include "pixel.h"
#include "image.h"
#include "glmath.h"
#include "texfmt.h"
#include "texmgr.h"
#include "texture.h"
#include "gldevice.h"

#include "ddtexmgr.h"

//#define __DEBUG_PRINT
#include "dbg.h"

void __glDDTexMgrSlurpLuminanceImage(__GLDDrawTexture *tex,
                                     __GLDDrawMipMapLevel *lp)
{ 
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpLuminanceImage width, height %d %d lp->buffer = %x\r\n",
             lp->level.width,lp->level.height, lp->lpMipBuffer);
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = (*srcP | (*srcP<<8) | (*srcP<<16) | (0xff000000));
     }
}


void __glDDTexMgrSlurpLuminanceSubImage(__GLDDrawTexture *tex, 
					__GLDDrawMipMapLevel *lp,
					GLint x, GLint y, GLint w, GLint h)
{
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpLuminanceSubImage\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+x, 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + w, dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = (*srcP | (*srcP<<8) | (*srcP<<16) | (0xff000000));
         }
     }
}


void __glDDTexMgrSlurpLuminanceAlphaImage(__GLDDrawTexture *tex,
                                          __GLDDrawMipMapLevel *lp)
{ 
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpLuminanceAlphaImage width, height %d %d\r\n",
             lp->level.width,lp->level.height);
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(2*lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP+=2, dstP++ ) {
          *dstP = (*srcP | (*srcP<<8) | (*srcP<<16) | (srcP[1]<<24));
     }
}


void __glDDTexMgrSlurpLuminanceAlphaSubImage(__GLDDrawTexture *tex,
					     __GLDDrawMipMapLevel *lp,
					     GLint x, GLint y, GLint w, GLint h)
{ 
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpLuminanceAlphaSubImage\r\n");
#endif

     srcstride = 2*lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(2*x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (3*w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP += 2, dstxP++ ) {
              *dstP = (*srcP | (*srcP<<8) | (*srcP<<16) | (srcP[1]<<24));
         }
     }
}


void __glDDTexMgrSlurpAlphaImage(__GLDDrawTexture *tex,
				 __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpAlphaImage\r\n");
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = (*srcP<<24);
     }
}


void __glDDTexMgrSlurpAlphaSubImage(__GLDDrawTexture *tex,
				    __GLDDrawMipMapLevel *lp,
				    GLint x, GLint y, GLint w, GLint h)
{ 
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpAlphaSubImage\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (4*w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = (*srcP<<24);
         }
     }
}


void __glDDTexMgrSlurpRGBImage(__GLDDrawTexture *tex,
			       __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGBImage width, height %d %d\r\n",
             lp->level.width,lp->level.height);
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(3*lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP += 3, dstP++ ) {
//          *dstP = (*srcP | (srcP[1]<<8) | (srcP[2]<<16) | (0xff000000));
          *dstP = (*srcP | (srcP[1]<<8) | (srcP[2]<<16) | (0xff000000));
     }
}


void __glDDTexMgrSlurpRGBSubImage(__GLDDrawTexture *tex,
				  __GLDDrawMipMapLevel *lp,
				  GLint x, GLint y, GLint w, GLint h)
{ 
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGBSubImage\r\n");
#endif

     srcstride = 3*lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(3*x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (3*w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP += 3, dstxP++ ) {
              *dstP = (*srcP | (srcP[1]<<8) | (srcP[2]<<16) | (0xff000000));
         }
     }
}


void __glDDTexMgrSlurpRGB332Image(__GLDDrawTexture *tex,
				  __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGB332SubImage\r\n");
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = ((*srcP&0x7) | ((*srcP&0x38)<<8) | ((*srcP*0xc)<<16) | (0xff000000));
     }
}


void __glDDTexMgrSlurpRGB332SubImage(__GLDDrawTexture *tex,
                                     __GLDDrawMipMapLevel *lp,
				     GLint x, GLint y, GLint w, GLint h)
{ 
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGB332SubImage\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = ((*srcP&0x7) | ((*srcP&0x38)<<8) | ((*srcP*0xc)<<16) | (0xff000000));
         }
     }
}


void __glDDTexMgrSlurpRGBAImage(__GLDDrawTexture *tex,
				__GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGBAImage width, height %d %d\r\n",
             lp->level.width,lp->level.height);
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(4*lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP += 4, dstP++ ) {
          *dstP = (*srcP | (srcP[1]<<8) | (srcP[2]<<16) | (srcP[3]<<24));
     }
}


void __glDDTexMgrSlurpRGBASubImage(__GLDDrawTexture *tex,
				   __GLDDrawMipMapLevel *lp,
				   GLint x, GLint y, GLint w, GLint h)
{
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGBASubImage\r\n");
#endif

     srcstride = 4*lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(4*x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (4*w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP += 4, dstxP++ ) {
              *dstP = (*srcP | (srcP[1]<<8) | (srcP[2]<<16) | (srcP[3]<<24));
         }
     }
}


void __glDDTexMgrSlurpRGBA4Image(__GLDDrawTexture *tex,
				 __GLDDrawMipMapLevel *lp)
{ 
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGBA4Image\r\n");
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(2*lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP += 2, dstP++ ) {
          *dstP = ( (*srcP&0xf)<<4 | ((*srcP&0xf0)<<12) | ((srcP[1]&0xf)<<20) 
                                   | ((*srcP&0xf0)<<28) );
     }
}


void __glDDTexMgrSlurpRGBA4SubImage(__GLDDrawTexture *tex,
				    __GLDDrawMipMapLevel *lp,
				    GLint x, GLint y, GLint w, GLint h)
{ 
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGBA4SubImage\r\n");
#endif

     srcstride = 2*lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(2*x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (3*w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP += 2, dstxP++ ) {
              *dstP = ( ((*srcP&0xf)<<4) | ((*srcP&0xf0)<<8) | ((srcP[1]&0xf)<<20) 
                                         | ((srcP[1]&0xf0)<<24) );
         }
     }
}


void __glDDTexMgrSlurpARGB4Image(__GLDDrawTexture *tex,
				 __GLDDrawMipMapLevel *lp)
{ 
    GLushort *srcP, *srcLimP;
    GLuint  *dstP;

#ifdef DEBUG1
ErrorF("SlurpARGB4Image\r\n");
#endif

     for (srcP = (GLushort *)lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
         *dstP = ( ((*srcP&0xf)<<4) | ((*srcP&0xf0)<<8) | ((*srcP&0xf00)<<20) 
                                    | ((*srcP&0xf000)<<24) );
     }
}



void __glDDTexMgrSlurpARGB4SubImage(__GLDDrawTexture *tex,
                                    __GLDDrawMipMapLevel *lp,
				    GLint x, GLint y, GLint w, GLint h)
{
    GLushort *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint  *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpARGB4SubImage\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLushort *)lp->level.buffer+(srcstride*y)+x, 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + w, dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = ( ((*srcP&0xf)<<4) | ((*srcP&0xf0)<<8) | ((*srcP&0xf00)<<20) 
                                         | ((*srcP&0xf000)<<24));
         }
     }
}


void __glDDTexMgrSlurpRGB5Image(__GLDDrawTexture *tex,
				__GLDDrawMipMapLevel *lp)
{
    GLushort *srcP, *srcLimP;
    GLuint  *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGB5Image\r\n");
#endif

     for (srcP = (GLushort *)lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = ( ((*srcP&0x1f)<<3) | ((*srcP&0x7e0)<<10) | ((*srcP&0xf800)<<16) 
                                          | 0xff000000 );
     }
}


void __glDDTexMgrSlurpRGB5SubImage(__GLDDrawTexture *tex,
				   __GLDDrawMipMapLevel *lp,
				   GLint x, GLint y, GLint w, GLint h)
{
    GLushort *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGB5SubImage\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLushort *)lp->level.buffer+(srcstride*y)+x, 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + w, dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = ( ((*srcP&0x1f)<<3) | ((*srcP&0x7e0)<<10) | ((*srcP&0xf800)<<16) 
                                          | 0xff000000 );
         }
     }
}


void __glDDTexMgrSlurpRGB565Image(__GLDDrawTexture *tex,
				  __GLDDrawMipMapLevel *lp)
{
    GLushort *srcP, *srcLimP;
    GLuint  *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGB565Image\r\n");
#endif

     for (srcP = (GLushort *)lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = ( ((*srcP&0x1f)<<3) | ((*srcP&0x7e0)<<10) | ((*srcP&0xf800)<<16) 
                                          | 0xff000000 );
    }
}


void __glDDTexMgrSlurpRGB565SubImage(__GLDDrawTexture *tex,
                                     __GLDDrawMipMapLevel *lp,
				     GLint x, GLint y, GLint w, GLint h)
{
    GLushort *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGB565SubImage\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLushort *)lp->level.buffer+(srcstride*y)+(x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = ( ((*srcP&0x1f)<<3) | ((*srcP&0x7e0)<<10) | ((*srcP&0xf800)<<16) 
                                          | 0xff000000 );
         }
     }
}


void __glDDTexMgrSlurpRGBA5_1Image(__GLDDrawTexture *tex,
				   __GLDDrawMipMapLevel *lp)
{ 
    GLushort *srcP, *srcLimP;
    GLuint  *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGBA5_1Image\r\n");
#endif

     for (srcP = (GLushort *)lp->level.buffer, 
          srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = ( ((*srcP&0x1f)<<3) | ((*srcP&0x3e0)<<10) | ((*srcP&0x7c00)<<16) 
                                          | ((*srcP&0x8000)? 0xff000000 : 0x0));
     }
}


void __glDDTexMgrSlurpRGBA5_1SubImage(__GLDDrawTexture *tex,
				      __GLDDrawMipMapLevel *lp,
				      GLint x, GLint y, GLint w, GLint h)
{
    GLushort *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGBA5_1SubImage\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLushort *)lp->level.buffer+(srcstride*y)+(x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = ( ((*srcP&0x1f)<<3) | ((*srcP&0x3e0)<<10) | ((*srcP&0x7c00)<<16) 
                                          | ((*srcP&0x8000)? 0xff000000 : 0x0));
         }
     }
}


void __glDDTexMgrSlurpARGB5_1Image(__GLDDrawTexture *tex,
				   __GLDDrawMipMapLevel *lp)
{
    GLushort *srcP, *srcLimP;
    GLuint  *dstP;

#ifdef DEBUG1
ErrorF("SlurpARGB5_1Image\r\n");
#endif

     for (srcP = (GLushort *)lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = ( ((*srcP&0x3e)<<2) | ((*srcP&0x7c0)<<9) | ((*srcP&0xf800)<<16) 
                                          | ((*srcP&0x1000)? 0xff000000 : 0x0));
     }
}


void __glDDTexMgrSlurpARGB5_1SubImage(__GLDDrawTexture *tex,
				      __GLDDrawMipMapLevel *lp,
				      GLint x, GLint y, GLint w, GLint h)
{
    GLushort *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint  *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGBA5_1SubImage\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLushort *)lp->level.buffer+(srcstride*y)+(x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = ( ((*srcP&0x3e)<<2) | ((*srcP&0x7c0)<<9) | ((*srcP&0xf800)<<16) 
                                          | ((*srcP&0x1000)? 0xff000000 : 0x0));
         }
     }
}


void __glDDTexMgrSlurpIntensityImage(__GLDDrawTexture *tex,
                                     __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpIntensityImage width, height %d %d. lp->buffer = %x\r\n",
             lp->level.width,lp->level.height, lp->lpMipBuffer);
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = (*srcP | (*srcP<<8) | (*srcP<<16) | (*srcP<<24));
     }
}


void __glDDTexMgrSlurpIntensitySubImage(__GLDDrawTexture *tex,
					__GLDDrawMipMapLevel *lp,
					GLint x, GLint y, GLint w, GLint h)
{
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint  *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpIntensitySubImage\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+x, 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + w, dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = (*srcP | (*srcP<<8) | (*srcP<<16) | (*srcP<<24));
         }
     }
}


void __glDDTexMgrSlurpColorIndex8Image(__GLDDrawTexture *tex,
				       __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint  *dstP;
    GLubyte *table = tex->texture.CT.table;
    GLint idx;

#ifdef DEBUG1
ErrorF("SlurpColorIndex8Image\r\n");
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          idx = *srcP << 2;
          *dstP = (table[idx] | 
                   (table[idx+1]<<8) | (table[idx+2]<<16) | (table[idx+3]<<24));
     }
}


void __glDDTexMgrSlurpColorIndex8SubImage(__GLDDrawTexture *tex,
					  __GLDDrawMipMapLevel *lp,
					  GLint x, GLint y, GLint w, GLint h)
{
    GLubyte  *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint  *dstP, *dstxP;
    GLint dststride, srcstride;
    GLubyte *table = tex->texture.CT.table;
    GLint idx;

#ifdef DEBUG1
ErrorF("SlurpColorIndex8SubImage\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+x, 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + w, dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
             idx = *srcP << 2;
             *dstP = (table[idx] | 
                     (table[idx+1]<<8) | (table[idx+2]<<16) | (table[idx+3]<<24));
         }
     }
}


void __glDDTexMgrSlurpColorIndex16Image(__GLDDrawTexture *tex,
					__GLDDrawMipMapLevel *lp)
{
    GLushort *srcP, *srcLimP;
    GLuint  *dstP;
    GLubyte *table = tex->texture.CT.table;
    GLint idx;

#ifdef DEBUG1
ErrorF("SlurpColorIndex16Image\r\n");
#endif

     for (srcP = (GLushort *)lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          idx = *srcP << 2;
          *dstP = (table[idx] | 
                   (table[idx+1]<<8) | (table[idx+2]<<16) | (table[idx+3]<<24));
     }
}


void __glDDTexMgrSlurpColorIndex16SubImage(__GLDDrawTexture *tex,
					   __GLDDrawMipMapLevel *lp,
					   GLint x, GLint y, GLint w, GLint h)
{
    GLushort *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint  *dstP, *dstxP;
    GLubyte *table = tex->texture.CT.table;
    GLint dststride, srcstride;
    GLint idx;

#ifdef DEBUG1
ErrorF("SlurpColorIndex16SubImage\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLushort *)lp->level.buffer+(srcstride*y)+(x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
             idx = *srcP << 2;
             *dstP = (table[idx] | 
                      (table[idx+1]<<8) | (table[idx+2]<<16) | (table[idx+3]<<24));
         }
     }
}


void __glDDTexMgrSlurpLuminanceImageBGRA8(__GLDDrawTexture *tex,
					  __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpLuminanceImageBGRA8 width, height %d %d lp->buffer = %x\r\n",
             lp->level.width,lp->level.height, lp->lpMipBuffer);
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = ((*srcP<<8) | (*srcP<<16) | (*srcP<<24) | 0xff000000 );
     }
}


void __glDDTexMgrSlurpLuminanceSubImageBGRA8(__GLDDrawTexture *tex,
					     __GLDDrawMipMapLevel *lp,
					     GLint x, GLint y, GLint w, GLint h)
{
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpLuminanceSubImage\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+x, 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + w, dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = ((*srcP<<8) | (*srcP<<16) | (*srcP<<24) | 0xff000000 );
         }
     }
}


void __glDDTexMgrSlurpLuminanceAlphaImageBGRA8(__GLDDrawTexture *tex,
					       __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpLuminanceAlphaImage width, height %d %d\r\n",
             lp->level.width,lp->level.height);
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(2*lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP+=2, dstP++ ) {
          *dstP = (*srcP | (*srcP<<8) | (*srcP<<16) | (srcP[1]<<24));
     }
}


void __glDDTexMgrSlurpLuminanceAlphaSubImageBGRA8(__GLDDrawTexture *tex,
						  __GLDDrawMipMapLevel *lp,
						  GLint x, GLint y, 
						  GLint w, GLint h)
{
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpLuminanceAlphaSubImage\r\n");
#endif

     srcstride = 2*lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(2*x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (2*w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP += 2, dstxP++ ) {
              *dstP = (*srcP | (*srcP<<8) | (*srcP<<16) | (srcP[1]<<24));
         }
     }
}


void __glDDTexMgrSlurpAlphaImageBGRA8(__GLDDrawTexture *tex,
				      __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpAlphaImageBGRA8\r\n");
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = *srcP<<24;
     }
}


void __glDDTexMgrSlurpAlphaSubImageBGRA8(__GLDDrawTexture *tex,
					 __GLDDrawMipMapLevel *lp,
					 GLint x, GLint y, GLint w, GLint h)
{
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpAlphaSubImageBGRA8\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = *srcP<<24;
         }
     }
}


void __glDDTexMgrSlurpRGBImageBGRA8(__GLDDrawTexture *tex,
				    __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGBImageBGRA8 width, height %d %d\r\n",
             lp->level.width,lp->level.height);
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(3*lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP += 3, dstP++ ) {
         *dstP = (*srcP << 16) | (srcP[1] << 8) | srcP[2] | 0xff000000 ;
     }
}


void __glDDTexMgrSlurpRGBSubImageBGRA8(__GLDDrawTexture *tex,
				       __GLDDrawMipMapLevel *lp,
				       GLint x, GLint y, GLint w, GLint h)
{
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGBSubImageBGRA8\r\n");
#endif

     srcstride = 3*lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(3*x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (3*w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP += 3, dstxP++ ) {
             *dstP = (*srcP << 16) | (srcP[1] << 8) | srcP[2] | 0xff000000 ;
         }
     }
}


void __glDDTexMgrSlurpRGB332ImageBGRA8(__GLDDrawTexture *tex,
				       __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGB332SubImageBGRA8\r\n");
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
         *dstP = (((*srcP&0x7)<<21) |((*srcP&0x38)<<10)|((*srcP*0xc)<<16)|0xff000000);
     }
}


void __glDDTexMgrSlurpRGB332SubImageBGRA8(__GLDDrawTexture *tex,
					  __GLDDrawMipMapLevel *lp,
					  GLint x, GLint y, GLint w, GLint h)
{
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGB332SubImageBGRA8\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
             *dstP = (((*srcP&0x7)<<21) |((*srcP&0x38)<<10)|((*srcP*0xc)<<16)|0xff000000);
         }
     }
}


void __glDDTexMgrSlurpRGBAImageBGRA8(__GLDDrawTexture *tex,
                                     __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGBAImageBGRA8 width, height %d %d\r\n",
             lp->level.width,lp->level.height);
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(4*lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP += 4, dstP++ ) {
          *dstP = (*srcP << 16) | (srcP[1] << 8) | srcP[2] | (srcP[3] << 24) ;
     }
}


void __glDDTexMgrSlurpRGBASubImageBGRA8(__GLDDrawTexture *tex,
					__GLDDrawMipMapLevel *lp,
					GLint x, GLint y, GLint w, GLint h)
{
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGBASubImageBGRA8\r\n");
#endif

     srcstride = 4*lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(4*x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (4*w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP += 4, dstxP++ ) {
             *dstP = (*srcP << 16) | (srcP[1] << 8) | srcP[2] | (srcP[3] << 24) ;
         }
     }
}


void __glDDTexMgrSlurpRGBA4ImageBGRA8(__GLDDrawTexture *tex,
				      __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGBA4ImageBGRA8\r\n");
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(2*lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP += 2, dstP++ ) {
          *dstP = ((*srcP&0x0f)<<20) | ((*srcP&0xf0) << 8) | 
                  ((srcP[1]&0xf)<<4) | ((srcP[1]&0xf0)<<24);
     }
}


void __glDDTexMgrSlurpRGBA4SubImageBGRA8(__GLDDrawTexture *tex,
					 __GLDDrawMipMapLevel *lp,
					 GLint x, GLint y, GLint w, GLint h)
{
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGBA4SubImageBGRA8\r\n");
#endif

     srcstride = 2*lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(2*x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (3*w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP += 2, dstxP++ ) {
              *dstP = ((*srcP&0x0f)<<20) | ((*srcP&0xf0) << 8) | 
                      ((srcP[1]&0xf)<<4) | ((srcP[1]&0xf0)<<24);
         }
     }
}


void __glDDTexMgrSlurpARGB4ImageBGRA8(__GLDDrawTexture *tex,
				      __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpARGB4ImageBGRA8\r\n");
#endif

     for (srcP = (GLubyte *)lp->level.buffer, srcLimP = srcP+(2*lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP+=2, dstP++ ) {
         *dstP = ((*srcP&0x0f0)<<16) | ((srcP[1]&0x0f) << 12) | 
                 (srcP[1]&0xf0)      | ((*srcP&0x0f)<<28);
     }
}


void __glDDTexMgrSlurpARGB4SubImageBGRA8(struct __GLDDrawTextureRec *tex,
                                     struct __GLDDrawMipMapLevelRec *lp,
					 GLint x, GLint y, GLint w, GLint h)
{
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint  *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpARGB4SubImageBGRA8\r\n");
#endif

     srcstride = lp->level.width*2;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+(2*x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (w*2), dstxP = dstP;
              srcxP < srcxLim;
              srcxP+=2, dstxP++ ) {
              *dstP = ((*srcP&0x0f0)<<16) | ((srcP[1]&0x0f) << 12) | 
                       (srcP[1]&0xf0)      | ((*srcP&0x0f)<<28);
         }
     }
}


void __glDDTexMgrSlurpRGB5ImageBGRA8(__GLDDrawTexture *tex,
                                     __GLDDrawMipMapLevel *lp)
{
    GLushort *srcP, *srcLimP;
    GLuint  *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGB5ImageBGRA8\r\n");
#endif

     for (srcP = (GLushort *)lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = ((*srcP&0x1f)<<19)  | ((*srcP&0x3e0)<<6) |
                  ((*srcP&0x7c00)>>7) | 0xff000000;
     }
}


void __glDDTexMgrSlurpRGB5SubImageBGRA8(__GLDDrawTexture *tex,
					__GLDDrawMipMapLevel *lp,
					GLint x, GLint y, GLint w, GLint h)
{
    GLushort *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGB5SubImageBGRA8\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLushort *)lp->level.buffer+(srcstride*y)+x, 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + w, dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = ((*srcP&0x1f)<<19)  | ((*srcP&0x3e0)<<6) |
                      ((*srcP&0x7c00)>>7) | 0xff000000;
         }
     }
}


void __glDDTexMgrSlurpRGB565ImageBGRA8(__GLDDrawTexture *tex,
				       __GLDDrawMipMapLevel *lp)
{
    GLushort *srcP, *srcLimP;
    GLuint  *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGB565ImageBGRA8\r\n");
#endif

     for (srcP = (GLushort *)lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
         *dstP = ((*srcP&0x1f)<<19)  | ((*srcP&0x7e0)<<5) |
                 ((*srcP&0xf800)>>8) | 0xff000000;
    }
}


void __glDDTexMgrSlurpRGB565SubImageBGRA8(__GLDDrawTexture *tex,
					  __GLDDrawMipMapLevel *lp,
					  GLint x, GLint y, GLint w, GLint h)
{
    GLushort *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGB565SubImageBGRA8\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLushort *)lp->level.buffer+(srcstride*y)+(x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = ((*srcP&0x1f)<<19)  | ((*srcP&0x7e0)<<5) |
                      ((*srcP&0xf800)>>8) | 0xff000000;
         }
     }
}


void __glDDTexMgrSlurpRGBA5_1ImageBGRA8(__GLDDrawTexture *tex,
					__GLDDrawMipMapLevel *lp)
{
    GLushort *srcP, *srcLimP;
    GLuint  *dstP;

#ifdef DEBUG1
ErrorF("SlurpRGBA5_1ImageBGRA8\r\n");
#endif

     for (srcP = (GLushort *)lp->level.buffer, 
          srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = ((*srcP&0x1f)<<19)  | ((*srcP&0x3e0)<<6) |
                  ((*srcP&0x7c00)>>7) | ((*srcP&0x8000)? 0xff000000 : 0x0);
     }
}


void __glDDTexMgrSlurpRGBA5_1SubImageBGRA8(__GLDDrawTexture *tex,
					   __GLDDrawMipMapLevel *lp,
					   GLint x, GLint y, GLint w, GLint h)
{
    GLushort *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGBA5_1SubImageBGRA8\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLushort *)lp->level.buffer+(srcstride*y)+(x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = ((*srcP&0x1f)<<19)  | ((*srcP&0x3e0)<<6) |
                      ((*srcP&0x7c00)>>7) | ((*srcP&0x8000)? 0xff000000 : 0x0);
         }
     }
}


void __glDDTexMgrSlurpARGB5_1ImageBGRA8(__GLDDrawTexture *tex,
					__GLDDrawMipMapLevel *lp)
{
    GLushort *srcP, *srcLimP;
    GLuint  *dstP;

#ifdef DEBUG1
ErrorF("SlurpARGB5_1ImageBGRA8\r\n");
#endif

     for (srcP = (GLushort *)lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
         *dstP = ((*srcP&0x3e)<<18)  | ((*srcP&0x7c0)<<5) |
                 ((*srcP&0xf800)>>8) | ((*srcP&0x1)? 0xff000000 : 0x0);
     }
}


void __glDDTexMgrSlurpARGB5_1SubImageBGRA8(__GLDDrawTexture *tex,
					   __GLDDrawMipMapLevel *lp,
					   GLint x, GLint y, GLint w, GLint h)
{
    GLushort *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint  *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpRGBA5_1SubImageBGRA8\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLushort *)lp->level.buffer+(srcstride*y)+(x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
             *dstP = ((*srcP&0x3e)<<18)  | ((*srcP&0x7c0)<<5) |
                     ((*srcP&0xf800)>>8) | ((*srcP&0x1)? 0xff000000 : 0x0);
         }
     }
}


void __glDDTexMgrSlurpIntensityImageBGRA8(__GLDDrawTexture *tex,
					  __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint *dstP;

#ifdef DEBUG1
ErrorF("SlurpIntensityImageBGRA8 width, height %d %d. lp->buffer = %x\r\n",
             lp->level.width,lp->level.height, lp->lpMipBuffer);
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          *dstP = ((*srcP) | (*srcP<<8) | (*srcP<<16) | (*srcP<<24));
     }
}


void __glDDTexMgrSlurpIntensitySubImageBGRA8(__GLDDrawTexture *tex,
					     __GLDDrawMipMapLevel *lp,
					     GLint x, GLint y, GLint w, GLint h)
{
    GLubyte *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint  *dstP, *dstxP;
    GLint dststride, srcstride;

#ifdef DEBUG1
ErrorF("SlurpIntensitySubImageBGRA8\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+x, 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + w, dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
              *dstP = (*srcP | (*srcP<<8) | (*srcP<<16) | (*srcP<<24));
         }
     }
}


void __glDDTexMgrSlurpColorIndex8ImageBGRA8(__GLDDrawTexture *tex,
					    __GLDDrawMipMapLevel *lp)
{
    GLubyte *srcP, *srcLimP;
    GLuint  *dstP;
    GLubyte *table = tex->texture.CT.table;
    GLint idx;

#ifdef DEBUG1
ErrorF("SlurpColorIndex8ImageBGRA8\r\n");
#endif

     for (srcP = lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
          idx = *srcP << 2;
          *dstP = (table[idx+2] | (table[idx+1]<<8) |  (table[idx]<<16) | (table[idx+3]<<24));
     }
}


void __glDDTexMgrSlurpColorIndex8SubImageBGRA8(__GLDDrawTexture *tex,
					       __GLDDrawMipMapLevel *lp,
					       GLint x, GLint y, 
					       GLint w, GLint h)
{
    GLubyte  *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint  *dstP, *dstxP;
    GLint dststride, srcstride;
    GLubyte *table = tex->texture.CT.table;
    GLint idx;

#ifdef DEBUG1
ErrorF("SlurpColorIndex8SubImageBGRA8\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLubyte *)lp->level.buffer+(srcstride*y)+x, 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + w, dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
             idx = *srcP << 2;
             *dstP = (table[idx+2] | (table[idx+1]<<8) |  (table[idx]<<16) | (table[idx+3]<<24));
         }
     }
}


void __glDDTexMgrSlurpColorIndex16ImageBGRA8(__GLDDrawTexture *tex,
					     __GLDDrawMipMapLevel *lp)
{
    GLushort *srcP, *srcLimP;
    GLuint  *dstP;
    GLubyte *table = tex->texture.CT.table;
    GLint idx;

#ifdef DEBUG1
ErrorF("SlurpColorIndex16ImageBGRA8\r\n");
#endif

     for (srcP = (GLushort *)lp->level.buffer, srcLimP = srcP+(lp->level.width*lp->level.height),
          dstP = lp->lpMipBuffer;
          srcP < srcLimP;
          srcP++, dstP++ ) {
         idx = *srcP << 2;
         *dstP = (table[idx+2] | (table[idx+1]<<8) |  (table[idx]<<16) | (table[idx+3]<<24));
     }
}


void __glDDTexMgrSlurpColorIndex16SubImageBGRA8(__GLDDrawTexture *tex,
						__GLDDrawMipMapLevel *lp,
						GLint x, GLint y, 
						GLint w, GLint h)
{
    GLushort *srcP, *srcxP, *srcLimP, *srcxLim;
    GLuint  *dstP, *dstxP;
    GLubyte *table = tex->texture.CT.table;
    GLint dststride, srcstride;
    GLint idx;

#ifdef DEBUG1
ErrorF("SlurpColorIndex16SubImageBGRA8\r\n");
#endif

     srcstride = lp->level.width;
     dststride = lp->level.width;
     for (srcP = (GLushort *)lp->level.buffer+(srcstride*y)+(x), 
          srcLimP = srcP+(h*srcstride),
          dstP = (GLuint *)lp->lpMipBuffer+(dststride*y)+x;
          srcP < srcLimP;
          srcP += srcstride, dstP += dststride ) {
         for (srcxP = srcP, srcxLim = srcP + (w), dstxP = dstP;
              srcxP < srcxLim;
              srcxP++, dstxP++ ) {
             idx = *srcP << 2;
             *dstP = (table[idx+2] | (table[idx+1]<<8) |  (table[idx]<<16) | (table[idx+3]<<24));
         }
     }
}


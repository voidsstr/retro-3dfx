/*
** Copyright 1991-1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
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
*/
/****************************************************************************
*
*                       MegaGraph Graphics Library
*
*                   Copyright (C) 1996 SciTech Software.
*                           All rights reserved.
*
* Filename:     $Workfile: cosmomgl.h$
* Version:      $Revision: 4$
*
* Language:     ANSI C
* Environment:  IBM PC (MS DOS)
*
* Description:  Header file for the interface between the SciTech MGL
*               and Cosmo OpenGL.
*
* $Date: 10/11/00 7:52:39 PM$ $Author: Brent$
*
****************************************************************************/

#ifndef __COSMOMGL_H
#define __COSMOMGL_H

#include <windows.h>

#pragma pack(1)

typedef unsigned char   uchar;
typedef unsigned int    uint;
typedef int             ibool;

/* Palette entry structure  */

typedef struct {
    uchar   blue;               /* Blue component of color              */
    uchar   green;              /* Green component of color             */
    uchar   red;                /* Blue component of color              */
    uchar   alpha;              /* Alpha or alignment byte              */
    } palette_t;

/* Pixel format structure */

typedef struct {
    uint    redMask,greenMask;      /* Mask values for pixels           */
    uint    blueMask,rsvdMask;
    int     redPos,redAdjust;       /* Red position and adjustment      */
    int     greenPos,greenAdjust;   /* Green position and adjustment    */
    int     bluePos,blueAdjust;     /* Blue position and adjustment     */
    int     rsvdPos,rsvdAdjust;     /* Reserved position and adjustment */
    } pixel_format_t;

/* The following structure is used for MGL's OpenGL to both choose an
 * appropriate visual that is supported by the software implementation and to
 * pass in the information for the visual when a rendering context is created.
 * Application code will fill in the structure and call MGL_glChooseVisual
 * to find out a valid set of visual information that is close to what
 * they requested, then call MGL_glSetVisual to make that the current
 * visual for a specific MGL device context.
 */

typedef struct {
    ibool       rgb_flag;       /* True for an RGB mode                 */
    ibool       alpha_flag;     /* True for alpha buffers               */
    ibool       db_flag;        /* True for double buffered             */
    int         depth_size;     /* Size of depth buffer in bits         */
    int         stencil_size;   /* Size of stencil buffer in bits       */
    int         accum_size;     /* Size of accumulation buffer in bits  */
    } MGLVisual;

/* The following structure is used to communicate with Cosmo OpenGL to let
 * it know about our surface information for fullscreen modes.
 */

typedef struct {
    void            *frontSurface;  /* Pointer to start of front buffer         */
    void            *backSurface;   /* Pointer to start of back buffer          */
    int             width;          /* Surface width in pixels                  */
    int             height;         /* Surface height in pixels                 */
    int             bytesPerLine;   /* Scanline pitch for the surface           */
    int             bitsPerPixel;   /* Pixel bits for mode (15 = 5:5:5 HiColor) */
    pixel_format_t  pf;             /* Pixel format information                 */
    palette_t       *pal;       	/* Hardware palette to be used              */

    /* Buffer arbitration functions */
    void            (_cdecl *BeginDirectAccess)(void);
    void            (_cdecl *EndDirectAccess)(void);
    void            (_cdecl *Fill)(int x,int y,int w,int h,uint color);
    } __MGLCosmoSurfaceInfo;

#pragma pack()

/* Cosmo OpenGL fullscreen MGL bindings */

typedef struct __MGLCosmoSurface *MGLCosmoSurface;
typedef struct __MGLCosmoContext *MGLCosmoContext;

void WINAPI mglCosmoChooseVisual(__MGLCosmoSurfaceInfo *info, MGLVisual *visual);
BOOL WINAPI mglCosmoCheckVisual(__MGLCosmoSurfaceInfo *info, MGLVisual *visual);
MGLCosmoContext WINAPI mglCosmoCreateContext(__MGLCosmoSurfaceInfo *info, MGLVisual *visual);
MGLCosmoSurface WINAPI mglCosmoCreateSurface(__MGLCosmoSurfaceInfo *info, MGLVisual *visual);
BOOL WINAPI mglCosmoMakeCurrent(MGLCosmoContext ctx, MGLCosmoSurface surface);
BOOL WINAPI mglCosmoDeleteSurface(MGLCosmoSurface surface);
BOOL WINAPI mglCosmoDeleteContext(MGLCosmoContext ctx);
void WINAPI mglCosmoSetSurfacePtr(MGLCosmoSurface surface,void *frontSurface, void *backSurface);

#endif  /* __COSMOMGL_H */


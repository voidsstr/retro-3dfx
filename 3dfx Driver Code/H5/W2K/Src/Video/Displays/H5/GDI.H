/******************************Module*Header**********************************\
*
*                           *******************
*                           * GDI SAMPLE CODE *
*                           *******************
*
* Module Name: gdi.h
*
* Contains all the gdi related stuff
*
* Copyright (c) 1999 3dfx Inc. All rights reserved.
* Copyright (c) 1995-1999 Microsoft Corporation.  All rights reserved.
\*****************************************************************************/
#ifndef __GDI__H__
#define __GDI__H__

//
//-----------------------Function***Prototypes--------------------------------
//
// Low-level blt function prototypes
//
typedef struct _GFNPB
{
    VOID (*pgfn)(struct _GFNPB *); // pointer to graphics function

    PDEV *      ppdev;      // driver ppdev
    
    DSURF *     pdsurfDst;  // destination surface
    RECTL *     prclDst;    // original unclipped destination rectangle
    
    DSURF *     pdsurfSrc;  // source surface
    RECTL *     prclSrc;    // original unclipped source rectangle
    POINTL *    pptlSrc;    // original unclipped source point
                            // NOTE: pdsurfSrc must be null if
                            // there is no source.  If there is
                            // a source either (pptlSrc must be
                            // valid) or (pptlSrc is NULL and prclSrc
                            // is valid).
    
    RECTL *     pRects;     // rectangle list
    LONG        lNumRects;  // number of rectangles in list
    ULONG       colorKey;   // colorKey for transparent operations
    ULONG       solidColor; // solid color used in fills
    // RBrush *    prbrush;    // pointer to brush
    // POINTL *    pptlBrush;  // brush origin
    CLIPOBJ *   pco;        // clipping object
    XLATEOBJ *  pxlo;       // color translatoin object
    POINTL *    pptlMask;   // original uncliped mask origin
    USHORT      usRop4;     // original rop4
    UCHAR       ucFgRop3;   // foreground rop3
    UCHAR       ucBgRop3;   // background rop3
    UCHAR       ucAlpha;    // alpha value for constant blends
    TRIVERTEX * ptvrt;      // verticies used for gradient fills
    ULONG       ulNumTvrt;  // number of verticies
    PVOID       pvMesh;     // connectivity for gradient fills
    ULONG       ulNumMesh;  // number of connectivity elements
    ULONG       ulMode;     // drawing mode
    POINTL *    pptlDitherOrg; // Gradient fill 16bpp crap
    SURFOBJ *   psoSrc;     // GDI managed surface source
    SURFOBJ *   psoDst;     // GDI managed surface destination
} GFNPB;

#define CLIP_LIMIT              50  // We'll take 800 bytes of stack space

typedef struct _ClipEnum
{
    LONG    c;
    RECTL   arcl[CLIP_LIMIT];       // Space for enumerating complex clipping
} ClipEnum;                         /* ce, pce */

void    vGradientFillRect(GFNPB * ppb);

#endif // __GDI__H__



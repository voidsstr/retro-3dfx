/******************************Module*Header**********************************\
 *
 *                           *******************
 *                           * GDI SAMPLE CODE *
 *                           *******************
 *
 * Module Name: clip.c
 *
 * Clipping code.
 *
 * Copyright (c) 1999 3dfx Inc. All rights reserved.
 * Copyright (c) 1995-1999 Microsoft Corporation.  All rights reserved.
 *****************************************************************************/
#include "precomp.h"

#if (_WIN32_WINNT >= 0x0500)

//-----------------------------Public Routine-------------------------------
// VOID vClipAndRender
//
// Clips the destination rectangle calling pfgn (the render function) as
// appropriate.
//
// Argumentes needed from function block (GFNPB)
// 
// pco------pointer to clip object
// prclDst--pointer to destination rectangle
// psurfDst-pointer to destination Surf
// psurfSrc-pointer to destination Surf (NULL if no source)
// pptlSrc--pointer to source point
// prclSrc--pointer to source rectangle (used if pptlSrc == NULL)
// pgfn-----pointer to render function
//
// NOTES:
//
// pptlSrc and prclSrc are only used if psurfSrc == psurfDst.  If there is
// no source psurfSrc must be set to NULL.  If prclSrc is specified, pptlSrc
// is not used.
//
//--------------------------------------------------------------------------

VOID vClipAndRender(GFNPB * ppb)
{
    CLIPOBJ * pco = ppb->pco;
    
    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
        ppb->pRects = ppb->prclDst;
        ppb->lNumRects = 1;
        ppb->pgfn(ppb);
    }
    else if (pco->iDComplexity == DC_RECT)
    {
        RECTL   rcl;

        if (bIntersect(ppb->prclDst, &pco->rclBounds, &rcl))
        {
            ppb->pRects = &rcl;
            ppb->lNumRects = 1;
            ppb->pgfn(ppb);
        }
    }
    else
    {
        ClipEnum    ce;
        LONG        c;
        BOOL        bMore;
        ULONG       ulDir = CD_ANY;

        // determine direction if operation on same surface
        if(ppb->pdsurfDst == ppb->pdsurfSrc)
        {
            LONG   lXSrc, lYSrc, offset;

            if(ppb->pptlSrc != NULL)
            {
                lXSrc = ppb->pptlSrc->x;
                lYSrc = ppb->pptlSrc->y;
            }
            else
            {
                lXSrc = ppb->prclSrc->left;
                lYSrc = ppb->prclSrc->top;
            }

            // NOTE: we can safely shift by 16 because the surface
            //       stride will never be greater than 2^16
            offset = (ppb->prclDst->top - lYSrc) << 16;
            offset += (ppb->prclDst->left - lXSrc);
            if(offset > 0)
                ulDir = CD_LEFTUP;
            else
                ulDir = CD_RIGHTDOWN;
        }


        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, ulDir, 0);

        do
        {
            bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

            c = cIntersect(ppb->prclDst, ce.arcl, ce.c);

            if (c != 0)
            {
                ppb->pRects = ce.arcl;
                ppb->lNumRects = c;
                ppb->pgfn(ppb);
            }

        } while (bMore);
    }
}

#endif

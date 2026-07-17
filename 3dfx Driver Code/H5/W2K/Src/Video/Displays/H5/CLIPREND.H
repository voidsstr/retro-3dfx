/*
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name: cliprend.h
**
** Description: Clipping support for the rendering code for the Software T&L HAL
**
** $Revision: 11$
** $Date: 10/11/00 8:45:07 PM$
**
** $Log: 
**  11   3dfx      1.9.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  10   Napalm Shared1.9         03/14/00 Matt McClure    Modifications for
**       WBuffer and Fog clipping fixes
**  9    Napalm Shared1.8         03/01/00 Matt McClure    Added code to support
**       Interpolation of WFBI
**  8    Napalm Shared1.7         02/24/00 Matt McClure    Modifications to only
**       reverse the perspective calculation.
**  7    Napalm Shared1.6         02/23/00 Matt McClure    Only do perspective
**       divide if we are not wrapping and we are drawing with the perspective
**       state enabled.
**  6    Napalm Shared1.5         02/23/00 Matt McClure    Final modifications for
**       Clip Rendering Code
**  5    Napalm Shared1.4         02/23/00 Matt McClure    More modifications for
**       TLBN clipping routines
**  4    Napalm Shared1.3         02/18/00 Matt McClure    More changes to support
**       TLBN Clipping.
**  3    Napalm Shared1.2         02/17/00 Matt McClure    Added code for Clipping
**       TLBN Triangles.
**  2    Napalm Shared1.1         01/28/00 Scott Kephart   Big T&L Merge: FVF
**       processing changes for T&L
**  1    Napalm Shared1.0         10/25/99 Scott Kephart   
** $
 * 
 * 4     1/25/00 10:16p Skephart
 * BobJ Merge
** 
** 3     10/26/99 12:58a Skephart
** Added #ifdef TnL_HAL
*/
#ifdef TnL_HAL

extern DWORD ClipSingleTriangle(RC *pRc, TLCLIPTRIANGLE *tri, TLCLIPVTX ***clipVertexPointer);
extern DWORD ClipSingleTriangleTLBN(RC *pRc, TLCLIPTRIANGLE *tri, TLCLIPVTX ***clipVertexPointer);
extern DWORD ClipSingleLine( RC *pRc, TLCLIPTRIANGLE *line );


//---------------------------------------------------------------------
// Make clip vertex from D3D vertex
//
// device - DIRECT3DDEVICEI *
// cn    - clipVertex
// pVtx    - a TL vertex
// qwFVF - FVF of the input TL vertex
//---------------------------------------------------------------------
__inline void MakeClipVertexFromFVF( RC *pRc, TLCLIPVTX *cv, LPVOID pVtx,
                            DWORD dwClipFlag, DWORD dwClipMask)
{
    LPBYTE pv = (LPBYTE)pVtx;

    // If the clip flag for this vertex is set, that means that the
    // transformation loop has not computed the screen coordinates for
    // this vertex, it has simply stored the clip coordinates for this
    // vertex


    if (dwClipFlag & dwClipMask)
    {
        // This is a clipped vertex, simply no screen coordinates
        cv->sx  = (D3DVALUE) 0;
        cv->sy  = (D3DVALUE) 0;
        cv->sz  = (D3DVALUE) 0;
        cv->rhw = (D3DVALUE) 0;

        // Since this vertex has been clipped, the transformation loop
        // has put in the clip coordinates instead
        cv->hx  = ((D3DVALUE*)pv)[0];
        cv->hy  = ((D3DVALUE*)pv)[1];
        cv->hz  = ((D3DVALUE*)pv)[2];
        cv->hw  = ((D3DVALUE*)pv)[3];
    }
    else
    {
        // This vertex is not clipped, so its screen coordinates have been
        // computed
        cv->sx  = ((D3DVALUE*)pv)[0];
        cv->sy  = ((D3DVALUE*)pv)[1];
        cv->sz  = ((D3DVALUE*)pv)[2];
        cv->rhw = ((D3DVALUE*)pv)[3];

        // Transform the screen coordinate back to the clipping space
        cv->hw  = 1.0f / cv->rhw;
        cv->hx  = (cv->sx - pRc->tl.ViewData.offsetX) * cv->hw * pRc->tl.ViewData.scaleXi;
        cv->hy  = (cv->sy - pRc->tl.ViewData.offsetY) * cv->hw * pRc->tl.ViewData.scaleYi;
        cv->hz  = (cv->sz - pRc->tl.ViewData.offsetZ) * cv->hw * pRc->tl.ViewData.scaleZi;

    }

    pv += sizeof(D3DVALUE) * 4;
    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_DIFFUSE)
    {
        cv->color   = *(DWORD*)pv;
        pv += sizeof(D3DVALUE);

    }
    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_SPECULAR)
    {
        cv->specular= *(DWORD*)pv;
        pv += sizeof(DWORD);
    }
    memcpy(cv->tex, pv,  pRc->tl.dwTextureCoordSizeTotal);
    pv += (DWORD) pRc->tl.dwTexCoordSize;
    cv->clip = dwClipFlag;
}


//---------------------------------------------------------------------
// Make TL vertex from clip vertex
//
// device - DIRECT3DDEVICEI *
// in    - clipVertex
// out   - TL vertex
//---------------------------------------------------------------------
__inline void MakeFVFVertexFromClip(RC *pRc, LPVOID out, TLCLIPVTX *cv)
{
    LPBYTE pv = (LPBYTE)out;
    ((D3DVALUE*)pv)[0] = cv->sx;
    ((D3DVALUE*)pv)[1] = cv->sy;
    ((D3DVALUE*)pv)[2] = cv->sz;
    ((D3DVALUE*)pv)[3] = D3DVAL(1)/cv->hw;
    pv += sizeof(D3DVALUE)*4;
    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_DIFFUSE)
    {
        *(DWORD*)pv = cv->color;
        pv += sizeof(DWORD);
    }
    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_SPECULAR)
    {
        *(DWORD*)pv = cv->specular;
        pv += sizeof(DWORD);
    }
    memcpy(pv, &cv->tex, pRc->tl.dwTextureCoordSizeTotal);
    pv += pRc->tl.dwTextureCoordSizeTotal;
}

//---------------------------------------------------------------------
// Make clip vertex from TLBN_CLIP Structure
//
// device - DIRECT3DDEVICEI *
// cn    - clipVertex
// pVtx    - a TL vertex
// 
//---------------------------------------------------------------------
__inline void MakeClipVertexFromTLBN( RC *pRc, TLCLIPVTX *cv, LPVOID pVtx,
                            DWORD dwClipFlag, DWORD dwClipMask)
{
    LPBYTE pv = (LPBYTE)pVtx;

    // If the clip flag for this vertex is set, that means that the
    // transformation loop has not computed the screen coordinates for
    // this vertex, it has simply stored the clip coordinates for this
    // vertex

    // Since this vertex has been clipped, the transformation loop
    // has put in the clip coordinates instead
    cv->hx  = ((TLBN_CLIP*)pv)->hx;
    cv->hy  = ((TLBN_CLIP*)pv)->hy;
    cv->hz  = ((TLBN_CLIP*)pv)->hz;
    cv->hw  = ((TLBN_CLIP*)pv)->hw;

    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_DIFFUSE)
    {
        cv->color   = ((TLBN_CLIP*)pv)->diffuse;
    }
    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_SPECULAR)
    {
        cv->specular= ((TLBN_CLIP*)pv)->specular;
    }
	
    if ( (pRc->state & STATE_REQUIRES_PERSPECTIVE) && !pRc->wrapT0 && !pRc->wrapT1 )
	{
        cv->tex[0] = ((TLBN_CLIP*)pv)->tex[0].s * ((TLBN_CLIP*)pv)->hw;
	    cv->tex[1] = ((TLBN_CLIP*)pv)->tex[0].t * ((TLBN_CLIP*)pv)->hw;
	    cv->tex[2] = ((TLBN_CLIP*)pv)->tex[1].s * ((TLBN_CLIP*)pv)->hw;
	    cv->tex[3] = ((TLBN_CLIP*)pv)->tex[1].t * ((TLBN_CLIP*)pv)->hw;
	}
	else
	{
        cv->tex[0] = ((TLBN_CLIP*)pv)->tex[0].s;
	    cv->tex[1] = ((TLBN_CLIP*)pv)->tex[0].t;
	    cv->tex[2] = ((TLBN_CLIP*)pv)->tex[1].s;
	    cv->tex[3] = ((TLBN_CLIP*)pv)->tex[1].t;
	}

    cv->clip = dwClipFlag;
}

//---------------------------------------------------------------------
// Make TLBN vertex from clip vertex
//
// device - DIRECT3DDEVICEI *
// in    - clipVertex
// out   - TL vertex
//---------------------------------------------------------------------
__inline void MakeTLBNVertexFromClip(RC *pRc, LPVOID out, TLCLIPVTX *cv)
{
	const D3DVALUE __HUGE_PWR2 = 1024.0f*1024.0f*2.0f;
    LPBYTE pv = (LPBYTE)out;
    D3DVALUE w;	

    if (!FLOAT_EQZ(cv->hw))
      w = D3DVAL(1)/cv->hw;
    else
      w = __HUGE_PWR2;

    switch ((int)cv->clip & (CLIPPED_LEFT|CLIPPED_RIGHT))
    {
      case CLIPPED_LEFT:  cv->sx = pRc->tl.ViewData.minXgb; break;
      case CLIPPED_RIGHT: cv->sx = pRc->tl.ViewData.maxXgb; break;
      default:
        cv->sx = cv->hx * pRc->tl.ViewData.scaleX * w + pRc->tl.ViewData.offsetX;
        if (cv->sx < pRc->tl.ViewData.minXgb)
          cv->sx = pRc->tl.ViewData.minXgb;
        if (cv->sx > pRc->tl.ViewData.maxXgb)
          cv->sx = pRc->tl.ViewData.maxXgb;
    }

    switch ((int)cv->clip & (CLIPPED_TOP|CLIPPED_BOTTOM))
    {
      case CLIPPED_BOTTOM: cv->sy = pRc->tl.ViewData.maxYgb; break;
      case CLIPPED_TOP:    cv->sy = pRc->tl.ViewData.minYgb; break;
      default:
        cv->sy = cv->hy * pRc->tl.ViewData.scaleY * w + pRc->tl.ViewData.offsetY;
        if (cv->sy < pRc->tl.ViewData.minYgb)
          cv->sy = pRc->tl.ViewData.minYgb;
        if (cv->sy > pRc->tl.ViewData.maxYgb)
          cv->sy = pRc->tl.ViewData.maxYgb;
    }
    cv->sz = cv->hz * pRc->tl.ViewData.scaleZ * w + pRc->tl.ViewData.offsetZ;
 
    ((TLBN*)pv)->x = cv->sx + pRc->sst.pixelOffset;
    ((TLBN*)pv)->y = cv->sy + pRc->sst.pixelOffset;
	((TLBN*)pv)->z = cv->sz;
	((TLBN*)pv)->w = w;
	((TLBN*)pv)->wfbi = ((TLBN*)pv)->w;

	// MUST RECALCULATE WFBI!
    if( pRc->state & STATE_REQUIRES_WBUFFER )
    {
      ((TLBN*)pv)->wfbi = (((((TLBN*)pv)->w) * pRc->aW) + pRc->bW);

      if (pRc->state & STATE_REQUIRES_VERTEXFOG)
      {
          ((TLBN*)pv)->z = (float)((255 - RGBA_GETALPHA(cv->specular)) << 8);
      }
    }
    else
    {
      ((TLBN*)pv)->z = pRc->zScale * cv->sz;

      if (pRc->state & STATE_REQUIRES_VERTEXFOG)
      {
        ((TLBN*)pv)->wfbi = (float)(255 - RGBA_GETALPHA(cv->specular));
      }
      else if (pRc->state & STATE_REQUIRES_HWFOG)
	  {
	   ((TLBN*)pv)->wfbi = ((TLBN*)pv)->w;
      }
    }
  
    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_DIFFUSE)
    {
        ((TLBN*)pv)->diffuse = cv->color;
    }
    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_SPECULAR)
    {
        ((TLBN*)pv)->specular = cv->specular;
    }

    // For the TLBN Rendering code, we must do all the computations for scale,
	// center, and perspective correction.
	// Don't do the perpsective calculation on NON Perspective or Wrapped Textured
	// triangles.
    if ( (pRc->state & STATE_REQUIRES_PERSPECTIVE) && !pRc->wrapT0 && !pRc->wrapT1 )
	{
      ((TLBN*)pv)->tex[0].s = cv->tex[0] * ((TLBN*)pv)->w;
      ((TLBN*)pv)->tex[0].t = cv->tex[1] * ((TLBN*)pv)->w;
      ((TLBN*)pv)->tex[1].s = cv->tex[2] * ((TLBN*)pv)->w;
	  ((TLBN*)pv)->tex[1].t = cv->tex[3] * ((TLBN*)pv)->w;
	}
	else // Else we are drawing triangles without perspective correction, or we are wrapping
	{
      ((TLBN*)pv)->tex[0].s = cv->tex[0];
      ((TLBN*)pv)->tex[0].t = cv->tex[1];
      ((TLBN*)pv)->tex[1].s = cv->tex[2];
	  ((TLBN*)pv)->tex[1].t = cv->tex[3];
    }
//    memcpy(pv, &cv->tex, pRc->tl.dwTextureCoordSizeTotal);
//    pv += pRc->tl.dwTextureCoordSizeTotal;
}

/*-------------------------------------------------------------------
Function Name:  ClipTLBNTriangle
Description:    
Parameters:     
Information:    
Return: DWORD
-------------------------------------------------------------------*/
__inline DWORD ClipTLBNTriangle(RC *pRc, LPDWORD pA, LPDWORD pB, LPDWORD pC, DWORD dwMask )
{
  TLCLIPTRIANGLE newtri;
  TLCLIPVTX cv[3];
  DWORD vert_count;
  TLCLIPVTX **ver;
  LPBYTE p;

     // Do Clipping
     MakeClipVertexFromTLBN( pRc, &cv[0], pA, ((TLBN_CLIP*)pA)->clip_code, dwMask);
     MakeClipVertexFromTLBN( pRc, &cv[1], pB, ((TLBN_CLIP*)pB)->clip_code, dwMask);
     MakeClipVertexFromTLBN( pRc, &cv[2], pC, ((TLBN_CLIP*)pC)->clip_code, dwMask);

     newtri.v[0] = &cv[0]; cv[0].next = &cv[1];
     newtri.v[1] = &cv[1]; cv[1].next = &cv[2];
     newtri.v[2] = &cv[2]; cv[2].next = NULL;
  
     cv[0].clip |= CLIPPED_ENABLE;
     cv[1].clip |= CLIPPED_ENABLE;
     cv[2].clip |= CLIPPED_ENABLE;
            

     if (vert_count = ClipSingleTriangleTLBN( pRc, &newtri, &ver ))
     {
      DWORD i;
   
   
        // BobJ 10/13/199 Note, allocating a fixed 0x200 (4K)
        // Output Clip buffer in tnlinit.c
        // This should be plenty of room for a worst case clipped triangle
        // and largest size Vertex.
        p = (LPVOID) pRc->tl.clipping.ClipBuf.alignedBuf;

        for (i = 0; i < vert_count; i++)
        {
           MakeTLBNVertexFromClip( pRc, p, ver[i]);
           p += TLBN_SIZE;//pRc->tl.TLFVF.dwStride;
        }
     }
	 
	 return (vert_count);

} /* ClipTLBNTriangle */

/*-------------------------------------------------------------------
Function Name:  ClipFVFTriangle
Description:    
Parameters:     
Information:    
Return: DWORD
-------------------------------------------------------------------*/
__inline DWORD ClipFVFTriangle(RC *pRc, LPDWORD pA, LPDWORD pB, LPDWORD pC, DWORD c0, DWORD c1, DWORD c2,
                               DWORD dwMask )
{
  TLCLIPTRIANGLE newtri;
  TLCLIPVTX cv[3];
  DWORD vert_count;
  TLCLIPVTX **ver;
  LPBYTE p;

     // Do Clipping
     MakeClipVertexFromFVF( pRc, &cv[0], pA, c0, dwMask);
     MakeClipVertexFromFVF( pRc, &cv[1], pB, c1, dwMask);
     MakeClipVertexFromFVF( pRc, &cv[2], pC, c2, dwMask);

     newtri.v[0] = &cv[0]; cv[0].next = &cv[1];
     newtri.v[1] = &cv[1]; cv[1].next = &cv[2];
     newtri.v[2] = &cv[2]; cv[2].next = NULL;
  
     cv[0].clip |= CLIPPED_ENABLE;
     cv[1].clip |= CLIPPED_ENABLE;
     cv[2].clip |= CLIPPED_ENABLE;
            

     if (vert_count = ClipSingleTriangle( pRc, &newtri, &ver ))
     {
      DWORD i;
   
   
        // BobJ 10/13/199 Note, allocating a fixed 0x200 (4K)
        // Output Clip buffer in tnlinit.c
        // This should be plenty of room for a worst case clipped triangle
        // and largest size Vertex.
        p = (LPVOID) pRc->tl.clipping.ClipBuf.alignedBuf;

        for (i = 0; i < vert_count; i++)
        {
           MakeFVFVertexFromClip( pRc, p, ver[i]); 
           p += pRc->tl.TLFVF.dwStride;
        }
     }
	 
	 return (vert_count);

} /* ClipFVFTriangle */

/*-------------------------------------------------------------------
Function Name:  ClipFVFLine
Description:    
Parameters:     
Information:    
Return: DWORD
-------------------------------------------------------------------*/
__inline DWORD ClipFVFLine(RC *pRc, LPDWORD pA, LPDWORD pB,  DWORD c0, DWORD c1, DWORD dwMask )
{
  TLCLIPTRIANGLE newline;
  TLCLIPVTX cv[2];
  LPBYTE p;

     // Do Clipping
     MakeClipVertexFromFVF( pRc, &cv[0], pA, c0, dwMask);
     MakeClipVertexFromFVF( pRc, &cv[1], pB, c1, dwMask);

     newline.v[0] = &cv[0]; 
     newline.v[1] = &cv[1]; 
  

     if (ClipSingleLine( pRc, &newline ))
     {
   
        // BobJ 10/13/199 Note, allocating a fixed 0x200 (4K)
        // Output Clip buffer in tnlinit.c
        // This should be plenty of room for a worst case clipped triangle
        // and largest size Vertex.
        p = (LPVOID) pRc->tl.clipping.ClipBuf.alignedBuf;

        MakeFVFVertexFromClip( pRc, p, newline.v[0]); 
        p += pRc->tl.TLFVF.dwStride;
        MakeFVFVertexFromClip( pRc, p, newline.v[1]); 
        return 1;
     }
	 else
	   return 0;

} /* ClipFVFLine */


#endif //TnL_HAL
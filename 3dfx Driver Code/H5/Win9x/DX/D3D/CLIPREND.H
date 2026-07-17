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
** $Revision: 19$
** $Date: 10/11/00 8:49:27 PM$
**
** $Log: 
**  19   3dfx      1.9.1.8     10/11/00 Brent           Forced check in to enforce
**       branching.
**  18   3dfx      1.9.1.7     10/10/00 Allen Hansen    added vertex blends to the
**       optimized paths
**  17   3dfx      1.9.1.6     10/03/00 Allen Hansen    fixed PRS 15644 (Silex
**       screen saver): bug in clipping fog, changed TLCLIPVTX.color to diffuse
**  16   3dfx      1.9.1.5     09/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  15   3dfx      1.9.1.4     09/15/00 Allen Hansen    Got rid of the TLBN_CLIP
**       structure (with homogenous coords), all output verts, clipped or not, use
**       the TLBN structure.  MakeClipVertexFromTLBN() now re-transfroms the source
**       vert to generate clip coords
**  14   3dfx      1.9.1.3     08/31/00 Allen Hansen    fixed bug when wfbi was
**       being used for Z, only interpolate wfbi if it's really fog, then if it's
**       being used for Z we calculate "wfbi = (w * pRc->aW) + pRc->bW" when
**       rebuilding a TLBN vertex (found this with Messiah)
**  13   3dfx      1.9.1.2     08/21/00 Allen Hansen    look for "SPECULARFIX" -
**       non-textured specular tri's are now handled the same as textured specular
**       tri's (both are 2 pass), caught in WHQL in one of the clipping tests, the
**       problem was when a vert has a diffuse + spec > 255, we clamp to 255 and
**       the hardware then interpolates along that edge.  What really should happen
**       is the hardware will add diffuse & spec at each pixel and clamp.  This is
**       not a T&L bug, it's just something that has always been wrong.
**  12   3dfx      1.9.1.1     07/29/00 Allen Hansen    fixed several WHQL
**       problems, mostly with fog, simplified some of the code
**  11   3dfx      1.9.1.0     07/24/00 Allen Hansen    whql bug fixes (T&L tests)
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
        cv->diffuse = *(DWORD*)pv;
        pv += sizeof(D3DVALUE);

    }
    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_SPECULAR)
    {
        cv->specular= *(DWORD*)pv;
#if 0	// SPECULARFIX!!!
		/*
		* Specular w/o textures:  This makes the a clipped tri match a non-clipped
		* tri in the current rendering code.  It's not really correct but it's consistant.
		* We pre-add the specular to the diffuse, and then set specular to zero.  
		*/
		if ((pRc->texture == 0) && (pRc->specular))
		{
			CLAMP888( cv->diffuse, cv->diffuse, cv->specular );
			cv->specular &= 0xff000000;		// don't throw the fog away!
	    }
#endif
        pv += sizeof(DWORD);
	}

    memcpy(cv->tex, pv,  pRc->tl.dwTextureCoordSizeTotal);
    pv += (DWORD) pRc->tl.dwTexCoordSize;
    cv->clip_code = dwClipFlag;

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
        *(DWORD*)pv = cv->diffuse;
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

/*-------------------------------------------------------------------
Function Name:  MakeClipVertexFromTLBN
Description:    Make clip vertex from TLBN Structure
Parameters:     pRc - rendering context
				pv - source TLBN vertex
				pvut - untransformed vertex in
				cv - destination TLCLIPVTX vertex
Information:    
Return:
-------------------------------------------------------------------*/
__inline void MakeClipVertexFromTLBN( RC *pRc, TLCLIPVTX *cv, lpTLBN pv,
                            D3DVERTEX *pvut, DWORD dwClipFlag, DWORD dwClipMask)
{
	int numVertexBlends = pRc->tl.numVertexBlends;

	// re-transform the input vertex into clip space
	if(numVertexBlends == 0)
	{
	    cv->hw = 1.0f / pv->w;  // this is a little faster than transforming w
    	cv->hx = (pvut->x*pRc->tl.lpxfmCurrent[0]->_11 + pvut->y*pRc->tl.lpxfmCurrent[0]->_21 + pvut->z*pRc->tl.lpxfmCurrent[0]->_31 + pRc->tl.lpxfmCurrent[0]->_41);
	    cv->hy = (pvut->x*pRc->tl.lpxfmCurrent[0]->_12 + pvut->y*pRc->tl.lpxfmCurrent[0]->_22 + pvut->z*pRc->tl.lpxfmCurrent[0]->_32 + pRc->tl.lpxfmCurrent[0]->_42);
    	cv->hz = (pvut->x*pRc->tl.lpxfmCurrent[0]->_13 + pvut->y*pRc->tl.lpxfmCurrent[0]->_23 + pvut->z*pRc->tl.lpxfmCurrent[0]->_33 + pRc->tl.lpxfmCurrent[0]->_43);
	    //cv->hw = (pin->x*pRc->tl.lpxfmCurrent[0]->_14 + pin->y*pRc->tl.lpxfmCurrent[0]->_24 + pin->z*pRc->tl.lpxfmCurrent[0]->_34 + pRc->tl.lpxfmCurrent[0]->_44);
	}
	else
	{
		int j;
		float *pBlendFactors = (float*)((LPBYTE)pvut + sizeof( D3DVALUE )*3);
		float cumulBlend = 0;	// Blend accumulated so far

		cv->hx = cv->hy = cv->hz = 0;
	    cv->hw = 1.0f / pv->w;  // this is a little faster than transforming w

  		for( j=0; j<=numVertexBlends; j++)
  		{
			float blend;
			if( j == numVertexBlends )	 
			{
				blend = 1.0f - cumulBlend;
			}
			else
			{
				blend = pBlendFactors[j];
				cumulBlend += pBlendFactors[j];
			}
			cv->hx += (pvut->x*pRc->tl.lpxfmCurrent[j]->_11 + pvut->y*pRc->tl.lpxfmCurrent[j]->_21 + pvut->z*pRc->tl.lpxfmCurrent[j]->_31 + pRc->tl.lpxfmCurrent[j]->_41) * blend;
			cv->hy += (pvut->x*pRc->tl.lpxfmCurrent[j]->_12 + pvut->y*pRc->tl.lpxfmCurrent[j]->_22 + pvut->z*pRc->tl.lpxfmCurrent[j]->_32 + pRc->tl.lpxfmCurrent[j]->_42) * blend;
			cv->hz += (pvut->x*pRc->tl.lpxfmCurrent[j]->_13 + pvut->y*pRc->tl.lpxfmCurrent[j]->_23 + pvut->z*pRc->tl.lpxfmCurrent[j]->_33 + pRc->tl.lpxfmCurrent[j]->_43) * blend;
		}
	}


	if (pRc->tl.KniRC.RfBits & BIT_RC_VERTEX_FOG)
	{
	  if(! (pRc->tl.KniRC.RfBits & BIT_RC_REQUIRES_WBUFFER))
        cv->vfog = pv->wfbi;
      else
        cv->vfog = pv->z;
	}

    cv->diffuse = pv->diffuse;
    cv->specular = pv->specular;


    if ( (pRc->state & STATE_REQUIRES_PERSPECTIVE) && !pRc->wrapT0 && !pRc->wrapT1 )
	{
		cv->tex[0] = pv->tex[0].s * cv->hw;
		cv->tex[1] = pv->tex[0].t * cv->hw;
		cv->tex[2] = pv->tex[1].s * cv->hw;
		cv->tex[3] = pv->tex[1].t * cv->hw;
	}
	else
	{
		cv->tex[0] = pv->tex[0].s;
		cv->tex[1] = pv->tex[0].t;
		cv->tex[2] = pv->tex[1].s;
		cv->tex[3] = pv->tex[1].t;
	}

    cv->clip_code = dwClipFlag;
}

/*-------------------------------------------------------------------
Function Name:  MakeTLBNVertexFromClip
Description:    Makes a TLBN vertex from a TLCLIPVTX vertex
Parameters:     pRc - rendering context
				pv - destination TLBN vertex
				cv - source TLCLIPVTX vertex
Information:    
Return:
-------------------------------------------------------------------*/
__inline void MakeTLBNVertexFromClip(RC *pRc, lpTLBN pv, TLCLIPVTX *cv)
{
	const D3DVALUE __HUGE_PWR2 = 1024.0f*1024.0f*2.0f;
    D3DVALUE w;	

    if (!FLOAT_EQZ(cv->hw))
      w = D3DVAL(1)/cv->hw;
    else
      w = __HUGE_PWR2;

    switch ((int)cv->clip_code & (CLIPPED_LEFT|CLIPPED_RIGHT))
    {
      case CLIPPED_LEFT:  cv->sx = pRc->tl.ViewData.minXgb; break;
      case CLIPPED_RIGHT: cv->sx = pRc->tl.ViewData.maxXgb; break;
      default:
        cv->sx = (cv->hx * pRc->tl.ViewData.scaleX * w) + pRc->tl.ViewData.offsetX;
        if (cv->sx < pRc->tl.ViewData.minXgb)
          cv->sx = pRc->tl.ViewData.minXgb;
        if (cv->sx > pRc->tl.ViewData.maxXgb)
          cv->sx = pRc->tl.ViewData.maxXgb;
    }

    switch ((int)cv->clip_code & (CLIPPED_TOP|CLIPPED_BOTTOM))
    {
      case CLIPPED_BOTTOM: cv->sy = pRc->tl.ViewData.maxYgb; break;
      case CLIPPED_TOP:    cv->sy = pRc->tl.ViewData.minYgb; break;
      default:
        cv->sy = (cv->hy * pRc->tl.ViewData.scaleY * w) + pRc->tl.ViewData.offsetY;
        if (cv->sy < pRc->tl.ViewData.minYgb)
          cv->sy = pRc->tl.ViewData.minYgb;
        if (cv->sy > pRc->tl.ViewData.maxYgb)
          cv->sy = pRc->tl.ViewData.maxYgb;
    }

    pv->x = cv->sx + pRc->sst.pixelOffset;
    pv->y = cv->sy + pRc->sst.pixelOffset;
	pv->w = w;
    cv->sz = ((cv->hz * pRc->tl.ViewData.scaleZ * w) + pRc->tl.ViewData.offsetZ) * pRc->zScale;

	// All the goofy fog stuff
    if(! (pRc->tl.KniRC.RfBits & BIT_RC_REQUIRES_WBUFFER))
	{
	  pv->z = cv->sz;
      if( pRc->tl.KniRC.RfBits & BIT_RC_VERTEX_FOG )
	    pv->wfbi = cv->vfog;
	  else
	    pv->wfbi = w;
	}
	else
	{
      if( pRc->tl.KniRC.RfBits & BIT_RC_VERTEX_FOG )
	    pv->z = cv->vfog;
	  else
	    pv->z = cv->sz;
	  pv->wfbi = (w * pRc->aW) + pRc->bW;
	}

    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_DIFFUSE)
      pv->diffuse = cv->diffuse;
    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_SPECULAR)
      pv->specular = cv->specular;

    // For the TLBN Rendering code, we must do all the computations for scale,
	// center, and perspective correction.
	// Don't do the perpsective calculation on NON Perspective or Wrapped Textured
	// triangles.
    if ( (pRc->state & STATE_REQUIRES_PERSPECTIVE) && !pRc->wrapT0 && !pRc->wrapT1 )
	{
      pv->tex[0].s = cv->tex[0] * pv->w;
      pv->tex[0].t = cv->tex[1] * pv->w;
      pv->tex[1].s = cv->tex[2] * pv->w;
	  pv->tex[1].t = cv->tex[3] * pv->w;
	}
	else // Else we are drawing triangles without perspective correction, or we are wrapping
	{
      pv->tex[0].s = cv->tex[0];
      pv->tex[0].t = cv->tex[1];
      pv->tex[1].s = cv->tex[2];
	  pv->tex[1].t = cv->tex[3];
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
__inline DWORD ClipTLBNTriangle(RC *pRc, lpTLBN pA, lpTLBN pB, lpTLBN pC, 
	D3DVERTEX *pAin, D3DVERTEX *pBin, D3DVERTEX *pCin, DWORD dwMask )
{
  TLCLIPTRIANGLE newtri;
  TLCLIPVTX cv[3];
  DWORD vert_count;
  TLCLIPVTX **ver;
  lpTLBN p;

     // Do Clipping
     MakeClipVertexFromTLBN( pRc, &cv[0], pA, pAin, pA->clip_code, dwMask);
     MakeClipVertexFromTLBN( pRc, &cv[1], pB, pBin, pB->clip_code, dwMask);
     MakeClipVertexFromTLBN( pRc, &cv[2], pC, pCin, pC->clip_code, dwMask);

     newtri.v[0] = &cv[0];		cv[0].next = &cv[1];
     newtri.v[1] = &cv[1];		cv[1].next = &cv[2];
     newtri.v[2] = &cv[2];		cv[2].next = NULL;
  
     cv[0].clip_code |= CLIPPED_ENABLE;
     cv[1].clip_code |= CLIPPED_ENABLE;
     cv[2].clip_code |= CLIPPED_ENABLE;
            

     if (vert_count = ClipSingleTriangleTLBN( pRc, &newtri, &ver ))
     {
      DWORD i;
   
   
        // BobJ 10/13/199 Note, allocating a fixed 0x200 (4K)
        // Output Clip buffer in tnlinit.c
        // This should be plenty of room for a worst case clipped triangle
        // and largest size Vertex.
        p = (lpTLBN) pRc->tl.clipping.ClipBuf.alignedBuf;

        for (i = 0; i < vert_count; i++)
        {
           MakeTLBNVertexFromClip( pRc, p, ver[i]);
		   ++p;
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
  
     cv[0].clip_code |= CLIPPED_ENABLE;
     cv[1].clip_code |= CLIPPED_ENABLE;
     cv[2].clip_code |= CLIPPED_ENABLE;
            

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
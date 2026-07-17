/*
** Copyright (c) 2000, 3Dfx Interactive, Inc.
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
** File name: procprim.c
**
** DescriptioMain body of ProcessPrimitive for T & L
**
** $Revision: 47$
** $Date: 10/19/00 1:30:50 PM$
**
** $Log: 
**  47   3dfx      1.18.1.5.1.2110/19/00 Allen Hansen    Added check for change in
**       VB start in indexed tris, caused failure in Evolva
**  46   3dfx      1.18.1.5.1.2010/18/00 Allen Hansen    Optimized when T&L
**       parameters (matricies, material, lights, etc) are dirty, now we don't
**       reload these unless they actually change.  This saves the processing of
**       the dirty params.
**  45   3dfx      1.18.1.5.1.1910/11/00 Brent           Forced check in to enforce
**       branching.
**  44   3dfx      1.18.1.5.1.1810/10/00 Allen Hansen    added vertex blends to the
**       optimized paths
**  43   3dfx      1.18.1.5.1.1710/05/00 Allen Hansen    fixed texgen bug
**       (villagemark - PRS15680
**  42   3dfx      1.18.1.5.1.1610/03/00 Allen Hansen    optimization - change
**       unsigned to signed before doing float to int conversions, always
**       interpolate diffuse on clipped tris
**  41   3dfx      1.18.1.5.1.1509/28/00 Allen Hansen    started vertex blend
**       support in fastpath (still ifdef'd out)
**  40   3dfx      1.18.1.5.1.1409/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  39   3dfx      1.18.1.5.1.1309/22/00 Allen Hansen    implemented texgen and
**       texture transformation to the fastpaths
**  38   3dfx      1.18.1.5.1.1209/15/00 Allen Hansen    Got rid of the TLBN_CLIP
**       structure (with homogenous coords), all output verts, clipped or not, use
**       the TLBN structure.
**  37   3dfx      1.18.1.5.1.1109/13/00 Allen Hansen    Fixed bug when using
**       colorvertex; if diffuse comes from the vertex, then the alpha must come
**       from that source.  Was failing the D3DIM sample app "Shadow Volume 2"
**  36   3dfx      1.18.1.5.1.1009/07/00 Allen Hansen    fixed prs 15434 (no
**       textures in villagemark): Corrected several problems with texgen, texture
**       transformation, and texture coordinate indexes.  This code will be much
**       less messy in the fastpath.  The real nasty part is the non-T&L rendering
**       code (which is used by the slow path) selects the imput texture based on
**       the texture coordinate index.  If the texture coordinate are generated,
**       the index is really invalid.  So rather than change all the rendering
**       paths I made sure the index was always valid.
**  35   3dfx      1.18.1.5.1.909/03/00 Allen Hansen    tlglobal.h, xform.c,
**       procprim.c
**  34   3dfx      1.18.1.5.1.808/29/00 Allen Hansen    added colorvertex support
**       for sse path, removed dead code in ProcessPrimitive()
**  33   3dfx      1.18.1.5.1.708/27/00 Allen Hansen    moved the dynamic guardband
**       code to UpdateXformData() in xform.c, now don't call UpdateXformData()
**       unless we need to
**  32   3dfx      1.18.1.5.1.608/23/00 Allen Hansen    fixed bug in the dynamic
**       guardband code (thanks Matt!)
**  31   3dfx      1.18.1.5.1.508/21/00 Allen Hansen    early-out of
**       ProcessVerticies() if the VB is already T&L'd and the RC hasn't changed,
**       start on fast-path color vertex support, fixed SOA lighting bug so
**       spec.alpha is always diff.alpha
**  30   3dfx      1.18.1.5.1.407/29/00 Allen Hansen    fixed bug in
**       ComputeClipCodes(), wrote ConvertePrimitiveToIndexTriList
**  29   3dfx      1.18.1.5.1.307/21/00 Allen Hansen    code cleanup, some
**       optimizations with vertex blends
**  28   3dfx      1.18.1.5.1.207/03/00 Allen Hansen    Changed processor specific
**       check from == to &
**  27   3dfx      1.18.1.5.1.106/29/00 Allen Hansen    Took the KNI prefetch out
**       of process verticies, this path is supposed to be for any cpu
**  26   3dfx      1.18.1.5.1.006/25/00 Allen Hansen    added support for 3DNow
**       codepath
**  25   3dfx      1.18.1.5    06/04/00 Allen Hansen    SW T&L Only:
**       ProcessPrimitive(), now non-kni cpu's go to the slow path
**  24   3dfx      1.18.1.4    06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  23   3dfx      1.18.1.3    05/22/00 Allen Hansen    Implemented 2-Pass outer
**       loop for small primitives, it's marginally faster so the code is currently
**       disabled.
** 
**  22   3dfx      1.18.1.2    05/16/00 Bob Johnston    Consolodating VERT_BUFF and
**       TnL_HAL defines to just use TnL_HAL
**  21   3dfx      1.18.1.1    05/11/00 Allen Hansen    Changed prefetch from
**       Prefetch0 to PrefetchNTA
**  20   3dfx      1.18.1.0    05/05/00 Bob Johnston    Implimented Dynamic
**       Guardband clipper for SW T&L HAL
**  19   Napalm Shared1.18        04/23/00 Matt McClure    Fixed compiler Error
**       when using SW_TNL and Z_ACCESS_OPT
**  18   Napalm Shared1.17        03/29/00 Bob Johnston    Created User Memory
**       Split T&L Path and added index pre calculations.
**  17   Napalm Shared1.16        03/23/00 Bob Johnston    Scott and Bob's changes
**       to split up the tranformation and lighting in the vertex processing loop
**       for improved VB primitive perfromance.
**  16   Napalm Shared1.15        03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  15   Napalm Shared1.14        03/16/00 Bob Johnston    Got Vertex Buffers
**       working correctly
**  14   Napalm Shared1.13        03/14/00 Bob Johnston    Changed User Mem
**       vertices to only use a min amount of memory in the SOAFVF_UM buff. Called
**       the setDX6State() from the FP branch in procprim.  Fixed
**       CanCreateExecBuff32 problem.  Decided to force VB creation to punt until I
**       fix all VB problems.
**  13   Napalm Shared1.12        03/14/00 Scott Kephart   Fixed numerous specular
**       alpha bugs
**  12   Napalm Shared1.11        03/06/00 Bob Johnston    Changes to make Vertex
**       Buffer handleing much more robust and accurate in terms of size.
**  11   Napalm Shared1.10        03/04/00 Bob Johnston    Increased max vertices
**       for VB and User Mem to 8K.  This fixes 3DMark2K's problems.  Also but a
**       fail over check to punt to the slow path  when the driver sees more than
**       8K of vertices inthe T&L HAL.  This number can be easily adjusted
**  10   Napalm Shared1.9         02/22/00 Bob Johnston    Fastpath rendering with
**       non-clipped vertices.  Texture coordinate handleing bug fixes, lots of bug
**       fixes.  Still showing some bugs in the fastpath.
**  9    Napalm Shared1.8         02/11/00 Bob Johnston    Fixed user mem vertex
**       pass/fail bug
**  8    Napalm Shared1.7         02/10/00 Bob Johnston    Added better clipping
**       support for the test code.  It will now use HAL assembly when okay.
**  7    Napalm Shared1.6         02/04/00 Bob Johnston    Changes to support User
**       Memory vertex buffers for the fastpath.  Also cleaned up the vertex
**       processing loops with neater macros for better readablity.
**  6    Napalm Shared1.5         01/28/00 Scott Kephart   Big T&L Merge: Carve out
**       place for fastpath, FVF changes
**  5    Napalm Shared1.4         12/13/99 Scott Kephart   Big T&L Update:
**       1. Improved T&L profiling code
**       2. Optimizations to scalar transformation and lighting code
**       3. Changes to the vertex buffer code to allow functionality under Windows
**       2000.
**  4    Napalm Shared1.3         11/22/99 Scott Kephart   More vertex buffer
**       support changes. Fix for hang in 3D Mark 2000.
**  3    Napalm Shared1.2         11/15/99 Scott Kephart   Added initial vertex
**       buffer support (more changes to follow)
**       Added support for Intel C Compiler for T&L code
**  2    Napalm Shared1.1         11/10/99 Scott Kephart   Added profiling changes
**       for T&L
**  1    Napalm Shared1.0         10/25/99 Scott Kephart   
** $
** 
** 8     3/12/00 5:21p Skephart
** Fixed fogging problems, added more comments
** 
** 7     3/10/00 4:39p Skephart
** Add logic to select Diffuse + specular RGB + A
** 
** 6     3/08/00 9:31p Skephart
** Fixed bug in SetupFVF -- had TLBN.dwFVFFlags &= !TL_FVFFLAG_CLIPPED,
** instead of TLBN.dwFVFFlags &= ~TL_FVFFLAG_CLIPPED
** 
** 5     3/08/00 9:24p Skephart
 * 
 * 10    1/27/00 12:29p Skephart
 * 
 * 9     1/27/00 12:43a Skephart
 * New changes from Bob, Joe, Scott
 * 
 * 8     1/25/00 10:17p Skephart
 * BobJ Merge
 * 
 * 6   1/24/00 4:13p Skephart
 * Beginnings of the fast path
 * 
 * 5   1/20/00 2:25p Skephart
 * Update from BobJ
** 
** 2   1/17/00 9:55p Skephart
** Bob J's FVF_COMP changes
** 
** 1   11/12/99 4:27p Skephart
** 
** 31  10/26/99 12:48a Skephart
** Added #ifdef TnL_HAL
** 
** 30  10/21/99 12:04a Skephart
** Force specular and diffuse components to be stored in every vertex
** 
** 29  10/20/99 3:16p Skephart
** 
** 28  10/20/99 1:43p Skephart
** Added TLPV_VALIDCLIPBUFFER, re-org of FVF setup
** 
** 27  10/19/99 1:40p Skephart
** Fixes for Flare
** 
** 26  10/11/99 1:24p Skephart
** Bob's clipping changes.
** SK - Fixed bug in bend - specular was disabled when it should have been
** enabled in SetupFVFData
** 
** 24  10/06/99 10:27p Skephart
** Cleanup
** 
** 23  10/06/99 3:13p Skephart
** Cleanup
** 
** 22  10/01/99 4:26p Skephart
** Fog Table fixes
** 
** 21  9/28/99 3:13p Skephart
** Fixes from BobJ
** 
** 20  9/27/99 8:32p Skephart
** Move render context variables for TL HAL out of d3global.h, 
** and into tlglobal.h. All TL related variables are gathered under 
** pRc->tl.<variable>
** 
** 19  9/27/99 7:04p Skephart
** Save Primitive Data update from BobJ
** 
** 11  9/16/99 10:59a Skephart
** Clean up gpClipBuf
*/
 
#include "precomp.h"

#if( DX >= 7 )
#ifdef TnL_HAL

#ifndef WINNT
#include <d3dhal.h>
#include "d6fvf.h"
#include "fxglobal.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "fifomgr.h"
#include "d3tri.h"
#include "d6global.h"
#include "d3contxt.h"
#endif

#include "dxins.h"
#include "d7fvfext.h"

#ifdef VCPP
#include "xmmintrin.h"  // intrinsics file
#endif

const D3DVALUE __HUGE_PWR2 = 1024.0f*1024.0f*2.0f;

extern DWORD setDX6state( RC *pRc, DWORD primitiveType, DWORD count );



/***
 ** SavePrimitiveData
 *
 *  FILENAME: C:\project\NAPALM\d3d\procprim.c
 *
 *  PARAMETERS: 
 *          DWORD dwFVFIn    -- Input FVF type
 *
 *  DESCRIPTION:
 *        Initializes important context information for the T&L HAL
 *        to use during ProcessPrimitive.  Called on NON indexed 
 *        Primitives.
 *
 *  RETURNS: Nothing
 *
***/
void SavePrimitiveData( RC *pRc, DWORD dwFVFIn, 
            LPVOID pVtx, DWORD dwVStart, UINT cVertices, 
            D3DPRIMITIVETYPE PrimType, LPVBSURFACEDATA lpVBSurfData )
{
  //
  // 1) Save the incoming information
  //
  pRc->tl.primType = PrimType;
  pRc->tl.InFVF.lpvData = pVtx;

  // Force some state changes if the FVF is different
  if( dwFVFIn != (DWORD) pRc->tl.InFVF.dwFVFType )
    pRc->tl.dwDirtyFlags |= TLPV_DIRTY_LIGHTING;

  pRc->tl.InFVF.dwFVFType = dwFVFIn;
  pRc->tl.dwNumVertices = cVertices;

  // No indices to work with
  pRc->tl.dwNumIndices = 0;
  pRc->tl.pIndices = NULL;
  pRc->tl.InFVF.dwVStart = dwVStart;
  pRc->tl.lpVBSurfData = lpVBSurfData;


} /* SavePrimitiveData */

/***
 ** SaveIdxPrimitiveData
 *
 *  FILENAME: C:\project\NAPALM\d3d\procprim.c
 *
 *  PARAMETERS: 
 *          DWORD dwFVFIn    -- Input FVF type
 *
 *  DESCRIPTION:
 *        Initializes important context information for the T&L HAL
 *        to use during ProcessPrimitive.  Called on Indexed Primitives.
 *
 *  RETURNS: Nothing
 *
***/
void SaveIdxPrimitiveData(    RC *pRc, DWORD dwFVFIn,
                LPVOID pVtx, DWORD dwVStart, UINT cVertices,
                D3DPRIMITIVETYPE PrimType,
                LPWORD pIndices, UINT cIndices, LPVBSURFACEDATA lpVBSurfData  )
 
{
  //
  // 1) Save the incoming information
  //
  pRc->tl.primType = PrimType;
  pRc->tl.InFVF.lpvData = pVtx;

  // Force some state changes if the FVF is different
  if( dwFVFIn != (DWORD) pRc->tl.InFVF.dwFVFType )
    pRc->tl.dwDirtyFlags |= TLPV_DIRTY_LIGHTING;

  pRc->tl.InFVF.dwFVFType = dwFVFIn;
  pRc->tl.dwNumVertices = cVertices - dwVStart;
  if (pRc->tl.prevIdxPrimVStart != dwVStart)
  {
    pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VB;
    pRc->tl.prevIdxPrimVStart = dwVStart;
  }

  pRc->tl.dwNumIndices = cIndices;
  pRc->tl.pIndices = pIndices;
  pRc->tl.InFVF.dwVStart = dwVStart;
  pRc->tl.lpVBSurfData = lpVBSurfData;

} /* SaveIdxPrimitiveData */


/***
 ** MakeRRCOLOR
 *
 *  FILENAME: C:\project\NAPALM\d3d\procprim.c
 *
 *  PARAMETERS: 
 *          TLCOLOR *out   -- where float color result is to be stored
 *          DWORD inputColor  -- input 32 bit RGBA color
 *
 *  DESCRIPTION:
 *        Convert a color from integer RGBA to 4 floats.
 *
 *  RETURNS:
 *
***/
__inline void MakeTLCOLOR( TLCOLOR *out, DWORD inputColor )
{
  out->r = (D3DVALUE)((int)RGBA_GETRED(inputColor));
  out->g = (D3DVALUE)((int)RGBA_GETGREEN(inputColor));
  out->b = (D3DVALUE)((int)RGBA_GETBLUE(inputColor));
}

/***
 ** ComputeClipCodes
 *
 *  FILENAME: C:\project\NAPALM\d3d\procprim.c
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
***/
TLCLIPCODE ComputeClipCodes(RC *pRc, TLCLIPCODE* pclipIntersection, TLCLIPCODE* pclipUnion, LPPRVCLIPPLANES lpClip)
{
  DWORD j;
  /* if true, need to deal with point size for clipping */
  D3DVALUE xx = lpClip->w - lpClip->x;
  D3DVALUE yy = lpClip->w - lpClip->y;
  D3DVALUE zz = lpClip->w - lpClip->z;

  /*  if (x < 0)  clip_code |= RRCLIP_LEFTBIT;    */
  /*  if (x >= we) clip_code |= RRCLIP_RIGHTBIT;  */
  /*  if (y < 0)  clip_code |= RRCLIP_BOTTOMBIT;  */
  /*  if (y >= we) clip_code |= RRCLIP_TOPBIT;    */
  /*  if (z < 0)    clip_code |= RRCLIP_FRONTBIT; */
  /*  if (z >= we) clip_code |= RRCLIP_BACKBIT;   */
  TLCLIPCODE clip_code = ((AS_INT32(lpClip->x)  & 0x80000000) >>  (32-TLCLIP_LEFTBIT))   |
                         ((AS_INT32(lpClip->y)  & 0x80000000) >>  (32-TLCLIP_BOTTOMBIT)) |
                         ((AS_INT32(lpClip->z)  & 0x80000000) >>  (32-TLCLIP_FRONTBIT))  |
                         ((AS_INT32(xx)         & 0x80000000) >>  (32-TLCLIP_RIGHTBIT))  |
                         ((AS_INT32(yy)         & 0x80000000) >>  (32-TLCLIP_TOPBIT))    |
                         ((AS_INT32(zz)         & 0x80000000) >>  (32-TLCLIP_BACKBIT));

  TLCLIPCODE clipBit = TLCLIP_USERCLIPPLANE0;
  for( j=0; j<TLMAX_USER_CLIPPLANES; j++)
  {
    if( pRc->tl.xfmUserClipPlanes[j].bActive )
    {
      TLVECTOR4 *plane = &pRc->tl.xfmUserClipPlanes[j].plane;
      FLOAT fComp = 0.0f;
      if( (lpClip->x*plane->x + lpClip->y*plane->y + lpClip->z*plane->z + lpClip->w*plane->w) < fComp )
        clip_code |= clipBit;
    }
    clipBit <<= 1;
  }

  if (clip_code == 0)
  {
    *pclipIntersection = 0;
    return clip_code;
  }
  else
  {
    if (pRc->tl.dwTLState & TLPV_GUARDBAND)
    {
      // We do guardband check in the projection space, so
      // we transform X and Y of the vertex there
      D3DVALUE xnew = lpClip->x * pRc->tl.ViewData.gb11 + lpClip->w * pRc->tl.ViewData.gb41;
      D3DVALUE ynew = lpClip->y * pRc->tl.ViewData.gb22 + lpClip->w * pRc->tl.ViewData.gb42;
      D3DVALUE xx = lpClip->w - xnew;
      D3DVALUE yy = lpClip->w - ynew;
      clip_code |= ((AS_INT32(xnew) & 0x80000000) >> (32-TLCLIPGB_LEFTBIT))   |
                   ((AS_INT32(ynew) & 0x80000000) >> (32-TLCLIPGB_BOTTOMBIT)) |
                   ((AS_INT32(xx)   & 0x80000000) >> (32-TLCLIPGB_RIGHTBIT))  |
                   ((AS_INT32(yy)   & 0x80000000) >> (32-TLCLIPGB_TOPBIT));
    }
	//if (fPointSize > 1.0f)
    //else
    {
      *pclipIntersection &= clip_code;
    }
    *pclipUnion |= clip_code;
    return clip_code;
  }
}


///////////////////////////////////////////////////////////////////////////////
// Process primitives implementation:
// 1) Compute FVF info
// 2) Grow buffers to the requisite size
// 3) Initialize clipping state
// 4) Update T&L state
// 5) Transform, Light and compute clipping for vertices
// 6) Clip and Draw the primitives
//
///////////////////////////////////////////////////////////////////////////////

TLCLIPCODE ProcessVertices(RC *pRc)
{
  D3DVERTEX		*pin   = (D3DVERTEX*)pRc->tl.InFVF.lpvData;
  D3DTLVERTEX	*pout  = (D3DTLVERTEX*)pRc->tl.TLFVF.lpvData;
  DWORD	in_size	= pRc->tl.InFVF.dwStride;
  DWORD	out_size=  pRc->tl.TLFVF.dwStride;
  DWORD	inFVF	= (DWORD) pRc->tl.InFVF.dwFVFType;
  DWORD	outFVF	= pRc->tl.TLFVF.dwFVFType;
  DWORD	flags	= pRc->tl.dwTLState;
  DWORD dwNumVertices = pRc->tl.dwNumVertices;
  DWORD *pOut;
  DWORD i;
  int j;
  TLCLIPCODE *pclip_code = pRc->tl.pClipBuf;
  TLCLIPCODE  clipIntersection = ~0;
  TLCLIPCODE  clipUnion = 0;
  D3DLIGHTINGELEMENT le;
  BOOL bVertexInEyeSpace = FALSE;
  FLOAT fPointSize = 0.0f;
  NT9XDEVICEDATA *ppdev = pRc->ppdev;
  DWORD dwNumActiveTex;

  //
  // Number of vertices to blend. i.e number of blend-matrices to
  // use is numVertexBlends+1.
  //
  int numVertexBlends = pRc->tl.numVertexBlends;

  if( HW_STATE_CHANGED )
    setDX6state( pRc, 0, 0);        // pRc, primitiveType, #primitives

  // If we've already T&L'd this we can just return
  if(! (pRc->tl.dwDirtyFlags & TLPV_DIRTY_VB))
    return pRc->tl.clipIntersection;

  pRc->tl.lighting.dwDiffuse = TL_DEFAULT_DIFFUSE;
  pRc->tl.lighting.dwSpecular = TL_DEFAULT_SPECULAR;

  dwNumActiveTex = 0;
  if(pRc->state & STATE_REQUIRES_ST_TMU0)
  {
    if(pRc->state & STATE_REQUIRES_ST_TMU1)
      dwNumActiveTex = 2;
	else
      dwNumActiveTex = 1;
  }


  //
  // The main transform loop
  //
  for (i=dwNumVertices; i; i--)
  {
    const D3DVECTOR *pNormal = (D3DVECTOR *)((LPBYTE)pin + pRc->tl.InFVF.dwNormalOffset);

#ifdef VCPP
    __declspec(align(16)) PRVCLIPPLANES Clip;  
#else
    PRVCLIPPLANES Clip;
#endif
    LPPRVCLIPPLANES lpClip = (LPPRVCLIPPLANES)&Clip;
    float inv_w_clip=0.0f;
    float *pBlendFactors = (float *)((LPBYTE)pin + sizeof( D3DVALUE )*3);


    if(!numVertexBlends)
    { // Special case for no vertex blends (by far the most common case)
      if (flags & (TLPV_DOLIGHTING | TLPV_DOFOG | TLPV_DOTEXGEN))
      {
        le.dvPosition.x = (pin->x*pRc->tl.lpxfmToEye[0]->_11 + pin->y*pRc->tl.lpxfmToEye[0]->_21 + pin->z*pRc->tl.lpxfmToEye[0]->_31 + pRc->tl.lpxfmToEye[0]->_41);
        le.dvPosition.y = (pin->x*pRc->tl.lpxfmToEye[0]->_12 + pin->y*pRc->tl.lpxfmToEye[0]->_22 + pin->z*pRc->tl.lpxfmToEye[0]->_32 + pRc->tl.lpxfmToEye[0]->_42);
        le.dvPosition.z = (pin->x*pRc->tl.lpxfmToEye[0]->_13 + pin->y*pRc->tl.lpxfmToEye[0]->_23 + pin->z*pRc->tl.lpxfmToEye[0]->_33 + pRc->tl.lpxfmToEye[0]->_43);

        // Transform vertex normal to the eye space, We use inverse transposed matrix
        le.dvNormal.x = (pNormal->x*pRc->tl.lpxfmToEyeInv[0]->_11 + pNormal->y*pRc->tl.lpxfmToEyeInv[0]->_12 + pNormal->z*pRc->tl.lpxfmToEyeInv[0]->_13);
        le.dvNormal.y = (pNormal->x*pRc->tl.lpxfmToEyeInv[0]->_21 + pNormal->y*pRc->tl.lpxfmToEyeInv[0]->_22 + pNormal->z*pRc->tl.lpxfmToEyeInv[0]->_23);
        le.dvNormal.z = (pNormal->x*pRc->tl.lpxfmToEyeInv[0]->_31 + pNormal->y*pRc->tl.lpxfmToEyeInv[0]->_32 + pNormal->z*pRc->tl.lpxfmToEyeInv[0]->_33);
      }

      // Apply WORLD
      Clip.x = (pin->x*pRc->tl.lpxfmCurrent[0]->_11 + pin->y*pRc->tl.lpxfmCurrent[0]->_21 + pin->z*pRc->tl.lpxfmCurrent[0]->_31 + pRc->tl.lpxfmCurrent[0]->_41);
      Clip.y = (pin->x*pRc->tl.lpxfmCurrent[0]->_12 + pin->y*pRc->tl.lpxfmCurrent[0]->_22 + pin->z*pRc->tl.lpxfmCurrent[0]->_32 + pRc->tl.lpxfmCurrent[0]->_42);
      Clip.z = (pin->x*pRc->tl.lpxfmCurrent[0]->_13 + pin->y*pRc->tl.lpxfmCurrent[0]->_23 + pin->z*pRc->tl.lpxfmCurrent[0]->_33 + pRc->tl.lpxfmCurrent[0]->_43);
      Clip.w = (pin->x*pRc->tl.lpxfmCurrent[0]->_14 + pin->y*pRc->tl.lpxfmCurrent[0]->_24 + pin->z*pRc->tl.lpxfmCurrent[0]->_34 + pRc->tl.lpxfmCurrent[0]->_44);
    }
    else
    {
       float cumulBlend = 0; // Blend accumulated so far

       if (flags & TLPV_DOLIGHTING)
         ZeroMemory( &le, sizeof(D3DLIGHTINGELEMENT) );
       Clip.x = Clip.y = Clip.z = Clip.w = 0.0f;

       for( j=0; j<=numVertexBlends; j++)
       {
         float blend;

         if( j == numVertexBlends )	 blend = 1.0f - cumulBlend;
         else						 blend = pBlendFactors[j];

         cumulBlend += pBlendFactors[j];

         if (flags & (TLPV_DOLIGHTING | TLPV_DOFOG | TLPV_DOTEXGEN))
         {
           le.dvPosition.x += (pin->x*pRc->tl.lpxfmToEye[j]->_11 + pin->y*pRc->tl.lpxfmToEye[j]->_21 + pin->z*pRc->tl.lpxfmToEye[j]->_31 + pRc->tl.lpxfmToEye[j]->_41) * blend;
           le.dvPosition.y += (pin->x*pRc->tl.lpxfmToEye[j]->_12 + pin->y*pRc->tl.lpxfmToEye[j]->_22 + pin->z*pRc->tl.lpxfmToEye[j]->_32 + pRc->tl.lpxfmToEye[j]->_42) * blend;
           le.dvPosition.z += (pin->x*pRc->tl.lpxfmToEye[j]->_13 + pin->y*pRc->tl.lpxfmToEye[j]->_23 + pin->z*pRc->tl.lpxfmToEye[j]->_33 + pRc->tl.lpxfmToEye[j]->_43) * blend;

           // Transform vertex normal to the eye space, we use inverse transposed matrix
           le.dvNormal.x += (pNormal->x*pRc->tl.lpxfmToEyeInv[j]->_11 + pNormal->y*pRc->tl.lpxfmToEyeInv[j]->_12 + pNormal->z*pRc->tl.lpxfmToEyeInv[j]->_13) * blend;
           le.dvNormal.y += (pNormal->x*pRc->tl.lpxfmToEyeInv[j]->_21 + pNormal->y*pRc->tl.lpxfmToEyeInv[j]->_22 + pNormal->z*pRc->tl.lpxfmToEyeInv[j]->_23) * blend;
           le.dvNormal.z += (pNormal->x*pRc->tl.lpxfmToEyeInv[j]->_31 + pNormal->y*pRc->tl.lpxfmToEyeInv[j]->_32 + pNormal->z*pRc->tl.lpxfmToEyeInv[j]->_33) * blend;
         }

         // Apply WORLDj
         Clip.x += (pin->x*pRc->tl.lpxfmCurrent[j]->_11 + pin->y*pRc->tl.lpxfmCurrent[j]->_21 + pin->z*pRc->tl.lpxfmCurrent[j]->_31 + pRc->tl.lpxfmCurrent[j]->_41) * blend;
         Clip.y += (pin->x*pRc->tl.lpxfmCurrent[j]->_12 + pin->y*pRc->tl.lpxfmCurrent[j]->_22 + pin->z*pRc->tl.lpxfmCurrent[j]->_32 + pRc->tl.lpxfmCurrent[j]->_42) * blend;
         Clip.z += (pin->x*pRc->tl.lpxfmCurrent[j]->_13 + pin->y*pRc->tl.lpxfmCurrent[j]->_23 + pin->z*pRc->tl.lpxfmCurrent[j]->_33 + pRc->tl.lpxfmCurrent[j]->_43) * blend;
         Clip.w += (pin->x*pRc->tl.lpxfmCurrent[j]->_14 + pin->y*pRc->tl.lpxfmCurrent[j]->_24 + pin->z*pRc->tl.lpxfmCurrent[j]->_34 + pRc->tl.lpxfmCurrent[j]->_44) * blend;
       }
    }


    //
    // Transform vertex to the clipping space, and position and normal
    // into eye space, if needed.
    //
    if ((flags & TLPV_NORMALIZENORMALS) && (flags & TLPV_DOLIGHTING))
      Normalize(&le.dvNormal);


    //
    // Compute clip codes if needed
    //
    if (flags & TLPV_DOCLIPPING)
    {
      TLCLIPCODE clip_code = ComputeClipCodes(pRc, &clipIntersection, &clipUnion, lpClip);
      if (clip_code == 0)
      {
        *pclip_code++ = 0;
        inv_w_clip = D3DVAL(1)/Clip.w;
      }
      else
      {
        if (flags & TLPV_GUARDBAND)
        {
          if ((clip_code & ~TLCLIP_INGUARDBAND) == 0)
          {
            // If vertex is inside the guardband we have to compute
            // screen coordinates
            inv_w_clip = D3DVAL(1)/Clip.w;
            *pclip_code++ = (TLCLIPCODE)clip_code;
            goto l_DoScreenCoord;
          }
        }
        *pclip_code++ = (TLCLIPCODE)clip_code;
        // If vertex is outside the frustum we can not compute screen
        // coordinates, hence store the clip coordinates
        pout->sx  = Clip.x;
        pout->sy  = Clip.y;
        pout->sz  = Clip.z;
        pout->rhw = Clip.w;
        goto l_DoLighting;
      }
    }
    else
    {
      // We have to check this only for DONOTCLIP case, because otherwise
      // the vertex with "we = 0" will be clipped and screen coordinates
      // will not be computed
      // "clip_code" is not zero, if "we" is zero.
      if (!FLOAT_EQZ(Clip.w))
        inv_w_clip = D3DVAL(1)/Clip.w;
      else
        inv_w_clip = __HUGE_PWR2;
    }


l_DoScreenCoord:
    pout->sx = Clip.x * inv_w_clip * pRc->tl.ViewData.scaleX + pRc->tl.ViewData.offsetX;
    pout->sy = Clip.y * inv_w_clip * pRc->tl.ViewData.scaleY + pRc->tl.ViewData.offsetY;
    pout->sz = Clip.z * inv_w_clip * pRc->tl.ViewData.scaleZ + pRc->tl.ViewData.offsetZ;
    pout->rhw = inv_w_clip;

l_DoLighting:
    pOut = (DWORD*)((char*)pout + 4*sizeof(D3DVALUE));


    if (flags & TLPV_DOLIGHTING)
    {
      bVertexInEyeSpace = TRUE;

      //
      // If Diffuse color is needed, extract it for color vertex.
      //
      if (flags & TLPV_VERTEXDIFFUSENEEDED)
      {
        const DWORD color = *(DWORD*)((char*)pin + pRc->tl.InFVF.dwDiffuseOffset);
        MakeTLCOLOR((TLCOLOR *)&pRc->tl.lighting.vertexDiffuse, color);
        pRc->tl.lighting.vertexDiffAlpha = color & 0xff000000;
      }

      //
      // If Specular color is needed and provided, extract it for color vertex.
      //
      if (flags & TLPV_VERTEXSPECULARNEEDED)
      {
        const DWORD color = *(DWORD*)((char*)pin + pRc->tl.InFVF.dwSpecularOffset);
        MakeTLCOLOR((TLCOLOR *)&pRc->tl.lighting.vertexSpecular, color);
        pRc->tl.lighting.vertexSpecAlpha = color & 0xff000000;
      }

      //
      // Light the vertex
      //
      LightVertex( pRc, &le );
    }
    else if (inFVF & (D3DFVF_DIFFUSE | D3DFVF_SPECULAR))
    {
      if (inFVF & D3DFVF_DIFFUSE)
        pRc->tl.lighting.dwDiffuse = *(DWORD*)((char*)pin + pRc->tl.InFVF.dwDiffuseOffset);
      if (inFVF & D3DFVF_SPECULAR)
        pRc->tl.lighting.dwSpecular = *(DWORD*)((char*)pin + pRc->tl.InFVF.dwSpecularOffset);
    }

    //
    // Compute Vertex Fog if needed
    //
    if (flags & TLPV_DOFOG)
      FogVertex( pRc, (D3DVECTOR*)(pin), &le, numVertexBlends, pBlendFactors, bVertexInEyeSpace );

    if (outFVF & D3DFVF_DIFFUSE)
      *pOut++ = pRc->tl.lighting.dwDiffuse;
    if (outFVF & D3DFVF_SPECULAR)
      *pOut++ = pRc->tl.lighting.dwSpecular;;

    // Texgen and Texture Transform code out of the DX8 refrast
    if(! (flags & (TLPV_DOTEXGEN | TLPV_DOTEXXFORM)))
    {
      // Copy the textures over if there is no TexGen or TexTransform
      memcpy(pOut, (char*)pin + pRc->tl.InFVF.dwTexOffset, pRc->tl.InFVF.dwTexCoordSize);
    }
    else
    {
	  DWORD dwTexStage;
	  DWORD dwOutputTextures = (DWORD) pOut;	// address of the output vertex's texture coordinates
	  TLVECTOR4 v4;
	  AOS_UV *texOut;

      for( dwTexStage = 0; dwTexStage < dwNumActiveTex; dwTexStage++ )
      {
        //DWORD TexXfmFlags = m_pDev->GetTextureStageState(dwTexStage)[D3DTSS_TEXTURETRANSFORMFLAGS];
        //DWORD TCI = m_pDev->GetTextureStageState(dwTexStage)[D3DTSS_TEXCOORDINDEX];
        DWORD TCI = pRc->textureStage[dwTexStage].texCoordIndex;
		DWORD TexCordIn = TCI & 0x0000ffff;
		texOut = (AOS_UV*)(dwOutputTextures + (TexCordIn * sizeof(AOS_UV)));

        // Perform TexGen
        switch( TCI & 0xffff0000 )
        {
          case D3DTSS_TCI_CAMERASPACENORMAL:
		  { /*
            Vout.m_tex[dwTexStage].x = le.dvNormal.x;
            Vout.m_tex[dwTexStage].y = le.dvNormal.y;
            Vout.m_tex[dwTexStage].z = le.dvNormal.z;
            Vout.m_tex[dwTexStage].w = 1.0f; */
		    v4.x = le.dvNormal.x;
		    v4.y = le.dvNormal.y;
		    v4.z = le.dvNormal.z;
		    v4.w = 1.0f;
            break;
		  }
          case D3DTSS_TCI_CAMERASPACEPOSITION:
		  { /*
            Vout.m_tex[dwTexStage].x = le.dvPosition.x;
            Vout.m_tex[dwTexStage].y = le.dvPosition.y;
            Vout.m_tex[dwTexStage].z = le.dvPosition.z;
            Vout.m_tex[dwTexStage].w = 1.0f; */
		    v4.x = le.dvPosition.x;
		    v4.y = le.dvPosition.y;
		    v4.z = le.dvPosition.z;
		    v4.w = 1.0f;
            break;
		  }
          case D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR:
          {
            //if( m_pDev->GetRenderState()[D3DRENDERSTATE_LOCALVIEWER] == TRUE )
			if( flags & TLPV_LOCALVIEWER )
            { /*
              D3DVALUE fX = le.dvPosition.x;
              D3DVALUE fY = le.dvPosition.y;
              D3DVALUE fZ = le.dvPosition.z;
                        
               have to normalize before we reflect, result will be normalized
              D3DVALUE fDist = (D3DVALUE)sqrt(fX*fX + fY*fY + fZ*fZ);
              if( FLOAT_NEZ( fDist ) ) fNorm = 1.0f/fDist;  else  fNorm = 0;
              fX *= fNorm;  fY *= fNorm;  fZ *= fNorm;
              fDot2 = 2.0f * (fX*le.dvNormal.x + fY*le.dvNormal.y + fZ*le.dvNormal.z);
              Vout.m_tex[dwTexStage].x = fX - (le.dvNormal.x * fDot2);
              Vout.m_tex[dwTexStage].y = fY - (le.dvNormal.y * fDot2);
              Vout.m_tex[dwTexStage].z = fZ - (le.dvNormal.z * fDot2);  */

			  D3DVECTOR v = le.dvPosition;
			  D3DVALUE fDot2;
              // have to normalize before we reflect, result will be normalized
			  Normalize( &v );
              fDot2 = 2.0f * (v.x*le.dvNormal.x + v.y*le.dvNormal.y + v.z*le.dvNormal.z);
 		      v4.x = v.x - (le.dvNormal.x * fDot2);
 		      v4.y = v.y - (le.dvNormal.y * fDot2);
 		      v4.z = v.z - (le.dvNormal.z * fDot2);
 		      v4.w = 1.0f;
            }
            else
            { /*
              fDot2 = 2.0f * le.dvNormal.z;
              Vout.m_tex[dwTexStage].x = -le.dvNormal.x * fDot2;
              Vout.m_tex[dwTexStage].y = -le.dvNormal.y * fDot2;
              Vout.m_tex[dwTexStage].z = 1.f - le.dvNormal.z*fDot2; */
			  D3DVALUE fDot2 = 2.0f * le.dvNormal.z;
 		      v4.x = 0.0f - (le.dvNormal.x * fDot2);
 		      v4.y = 0.0f - (le.dvNormal.y * fDot2);
 		      v4.z = 1.0f - (le.dvNormal.z * fDot2);
 		      v4.w = 1.0f;
            }
            //Vout.m_tex[dwTexStage].w = 1.0f;
            break;
          }
          case D3DTSS_TCI_PASSTHRU: // No TexGen
          { /*
            // Copy the tex coordinate for this stage
            DWORD n = GetTexCoordDim( inFVF, TCI );  = inFVF >> ((TCI&0xffff)*2+16)
            float *pCoord = (float *)&Vout.m_tex[dwTexStage];
            TCI &= 0xFFFF;
            for( DWORD j = 0; j < n; j++ )
               pCoord[j] = pTex[TCI][j];
            if( n < 4 ) 
              pCoord[n] = 1.0f;
            for( j = n+1; j < 4; j++ ) 
              pCoord[j] = 0.0f; */

            // Copy the tex coordinate for this stage.  This can use any of the 
			// source textures
            if( TexCordIn < pRc->tl.InFVF.dwNumTexCoords )
			{
              TLVECTOR4 *texIn = (TLVECTOR4*)((char*)pin + pRc->tl.dwTexOffsetIn[TexCordIn]);
			  switch( (inFVF >> (TexCordIn*2 + 16)) & 0x3 )
			  {
			    case D3DFVF_TEXTUREFORMAT1:	// 1d Texture coordinates
 		          v4.x = texIn->x;	v4.y = 1.0f;		v4.z = 0.0f;		v4.w = 0.0f;
				  break;
     			case D3DFVF_TEXTUREFORMAT2:	// 2d Texture coordinates
 		          v4.x = texIn->x;    v4.y = texIn->y;	v4.z = 1.0f;		v4.w = 0.0f;
				  break;
			    case D3DFVF_TEXTUREFORMAT3:	// 3d Texture coordinates
 		          v4.x = texIn->x;	v4.y = texIn->y;	v4.z = texIn->z;	v4.w = 1.0f;
				  break;
			    case D3DFVF_TEXTUREFORMAT4:	// 4d Texture coordinates
 		          v4.x = texIn->x;	v4.y = texIn->y;	v4.z = texIn->z;	v4.w = texIn->w;
				  break;
			  }
			}
			else
			{
			  // with no input texture we load zero's 
			  // (ref D3DTEXTURESTAGESTATETYPE in the dx7 sdk spec)
			  v4.x = v4.y = v4.z = v4.w = 0;
			}
            break;
          }
          default:
          {
            D3DPRINT(0, "Unknown TexGen mode" ); 
			// I guess loading zero is the safest bet here
 		    v4.x = v4.y = v4.z = v4.w = 0;
            break;
          }
        }

        // Perform TexTransform
        //DWORD TexXfmFlags = m_pDev->GetTextureStageState(dwTexStage)[D3DTSS_TEXTURETRANSFORMFLAGS];
        //if( ( TexXfmFlags & ~D3DTTFF_PROJECTED ) != D3DTTFF_DISABLE )
        if( (pRc->textureStage[dwTexStage].texXformFlags & (D3DTTFF_PROJECTED-1)) != D3DTTFF_DISABLE )
        { /*
          LPD3DMATRIX pM = &m_xfmTex[dwTexStage];
          FLOAT fX = Vout.m_tex[dwTexStage].x;
          FLOAT fY = Vout.m_tex[dwTexStage].y;
          FLOAT fZ = Vout.m_tex[dwTexStage].z;
          FLOAT fW = Vout.m_tex[dwTexStage].w;
          FLOAT fXout = fX*pM->_11 + fY*pM->_21 + fZ*pM->_31 + fW*pM->_41;
          FLOAT fYout = fX*pM->_12 + fY*pM->_22 + fZ*pM->_32 + fW*pM->_42;
          FLOAT fZout = fX*pM->_13 + fY*pM->_23 + fZ*pM->_33 + fW*pM->_43;
          FLOAT fWout = fX*pM->_14 + fY*pM->_24 + fZ*pM->_34 + fW*pM->_44;
          Vout.m_tex[dwTexStage].x = fXout;
          Vout.m_tex[dwTexStage].y = fYout;
          Vout.m_tex[dwTexStage].z = fZout;
          Vout.m_tex[dwTexStage].w = fWout; */
          LPD3DMATRIX pM = &pRc->tl.xfmTxtr[dwTexStage];
	      texOut->u = v4.x*pM->_11 + v4.y*pM->_21 + v4.z*pM->_31 + v4.w*pM->_41;
	      texOut->v = v4.x*pM->_12 + v4.y*pM->_22 + v4.z*pM->_32 + v4.w*pM->_42;
	      //texOut->z = v4.x*pM->_13 + v4.y*pM->_23 + v4.z*pM->_33 + v4.w*pM->_43;
	      //texOut->w = v4.x*pM->_14 + v4.y*pM->_24 + v4.z*pM->_34 + v4.w*pM->_44;

		  // This wasn't in the procprim.c in the refrast, but the sdk says for projected
		  // you divide the elements passed in by the lash element
		  if( pRc->textureStage[dwTexStage].texXformFlags & D3DTTFF_PROJECTED )
		  {
		    D3DVALUE rz = v4.x*pM->_13 + v4.y*pM->_23 + v4.z*pM->_33 + v4.w*pM->_43;
            if (!FLOAT_EQZ(rz))
		      rz = 1.0f/rz;
	        else
			  rz = __HUGE_PWR2;
		    texOut->u *= rz;
		    texOut->v *= rz;
          }
		}
		else
		{
		  texOut->u = v4.x;
		  texOut->v = v4.y;
		  //texOut->z = v4.z;
		  //texOut->w = v4.w;

		  // This wasn't in the procprim.c in the refrast, but the sdk says for projected
		  // you divide the elements passed in by the last element
		  if( pRc->textureStage[dwTexStage].texXformFlags & D3DTTFF_PROJECTED )
		  {
		    D3DVALUE rz = v4.z;
            if (!FLOAT_EQZ(rz))
		      rz = 1.0f/rz;
	        else
			  rz = __HUGE_PWR2;
		    texOut->u *= rz;
		    texOut->v *= rz;
          }
		}
      }
    }

    pin = (D3DVERTEX*) ((char*) pin + in_size);
    pout = (D3DTLVERTEX*) ((char*) pout + out_size);
  }


  if (flags & TLPV_DOCLIPPING)
  {
    pRc->tl.clipIntersection = clipIntersection;
    pRc->tl.clipUnion = clipUnion;
  }
  else
  {
    pRc->tl.clipIntersection = 0;
    pRc->tl.clipUnion = 0;
  }

  if (!pRc->tl.clipUnion) 
  {
    pRc->tl.dwTLState &= ~TLPV_VALIDCLIPBUFFER;
  }

  pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_VB;	// this VB has been T&L'd

  // Returns whether all the vertices were off screen
  return pRc->tl.clipIntersection;
}

//---------------------------------------------------------------------
// This function should be called every time FVF ID is changed
// All pv flags, input and output FVF id should be set before calling the
// function.
//---------------------------------------------------------------------
_inline void UpdateComponentOffsets (
			   DWORD dwFVFIn,
               LPDWORD pNormalOffset,
               LPDWORD pDiffOffset,
               LPDWORD pSpecOffset,
               LPDWORD pTexOffset)
{
  DWORD dwOffset = 0;

  switch( dwFVFIn & D3DFVF_POSITION_MASK )
  {
  case D3DFVF_XYZ:
    dwOffset = sizeof(D3DVECTOR);
    break;
  case D3DFVF_XYZB1:
    dwOffset = sizeof(D3DVECTOR) + sizeof(D3DVALUE);
    break;
  case D3DFVF_XYZB2:
    dwOffset = sizeof(D3DVECTOR) + 2*sizeof(D3DVALUE);
    break;
  case D3DFVF_XYZB3:
    dwOffset = sizeof(D3DVECTOR) + 3*sizeof(D3DVALUE);
    break;
  case D3DFVF_XYZB4:
    dwOffset = sizeof(D3DVECTOR) + 4*sizeof(D3DVALUE);
    break;
  case D3DFVF_XYZB5:
    dwOffset = sizeof(D3DVECTOR) + 5*sizeof(D3DVALUE);
    break;
  default:
    D3DPRINT(0, "Unable to compute offsets, strange FVF bits set");
  }

  *pNormalOffset = dwOffset;

  if (dwFVFIn & D3DFVF_NORMAL)
    dwOffset += sizeof(D3DVECTOR);
  if (dwFVFIn & D3DFVF_RESERVED1)
    dwOffset += sizeof(D3DVALUE);

  // Offset to the diffuse color
  *pDiffOffset = dwOffset;

  if (dwFVFIn & D3DFVF_DIFFUSE)
    dwOffset += sizeof(DWORD);

  // Offset to the specular color
  *pSpecOffset = dwOffset;

  if (dwFVFIn & D3DFVF_SPECULAR)
    dwOffset += sizeof(DWORD);

  // Offset to the texture data
  *pTexOffset = dwOffset;
} /* UpdateComponentOffsets  */


//---------------------------------------------------------------------
// SetupFVFData() : This function is called when passed FVF types 
//          cahnge.  It's sole purpose is to compute values
//          needed later in the FVF_COMP structure for both
//          the Slowpath and FastPath
//---------------------------------------------------------------------
void SetupFVFData(RC *pRc)
{
  // This function will set values for all four of the following FVF_COMP
  // structures:
  //    InFVF:  The passed untransformed and lit FVF from D3D to DP2 in the HAL.
  //    SOAFVF: The optimized and swizzled SOA untransformed and lit FVF that
  //        we create!
  //    LTFVF:  The D3DFVF compatible Output FVF that the slowpath rendering code 
  //        uses.
  //    LTBN: The Fastpath optimized AOS vertex format. Comes in two flavors, clipped
  //        and unclipped.  This format contains NO FVF info at all, but the 
  //        info required can be housed in the FVF_COMP struture anyway.  
  //        Just ignore all FVF related members of the FVF_COMP structure 
  //        for this pointer.
  int dwTexStage;


  // Some of the members of InFVF have already been set earlier in SaveXXXPrimitiveData
  // Now just update some of the informaiton to other FVF_COMP members.
  pRc->tl.SOAFVF.dwFVFType = pRc->tl.InFVF.dwFVFType;
  pRc->tl.SOAFVF.dwFVFFlags = TL_FVFFLAG_SOA;

  // FYI, SOAFVF.lpvData will be assinged later in ProcessVerticesFast() if
  // this primitive can be routed to the fast path.  Allocation has 
  // to occur this way since we had no Vertex info at the original vertex 
  // buffer creation time and we can't put it here since this code is only
  // run when a different FVF type is passed.

  //*****************************************************************
  // Compute number of texture coordinates for each FVF_COMP pointer
  //*****************************************************************
  pRc->tl.InFVF.dwNumTexCoords = FVF_TEXCOORD_NUMBER((DWORD)pRc->tl.InFVF.dwFVFType);
  pRc->tl.SOAFVF.dwNumTexCoords = pRc->tl.InFVF.dwNumTexCoords;
  pRc->tl.TLFVF.dwNumTexCoords = pRc->tl.InFVF.dwNumTexCoords;

  // Handle cases where we generate more textures that we get
  if((pRc->tl.TLFVF.dwNumTexCoords < 1) && (pRc->state & STATE_REQUIRES_ST_TMU0))
    pRc->tl.TLFVF.dwNumTexCoords = 1;
  if((pRc->tl.TLFVF.dwNumTexCoords < 2) && (pRc->state & STATE_REQUIRES_ST_TMU1))
    pRc->tl.TLFVF.dwNumTexCoords = 2;

  //************************************************************
  // Compute output FVF type for the Slowpath's TLFVF pointer
  //************************************************************
  // Amazing as it seems, we always want to output diffuse and specular components
  // for each vertex. The setup engine is *always* setup to accept at least a diffuse
  // vertex component.
  pRc->tl.TLFVF.dwFVFType = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR);
  
  // Set up number of texture coordinates and copy texture formats
  pRc->tl.TLFVF.dwFVFType |= (DWORD)((pRc->tl.TLFVF.dwNumTexCoords << D3DFVF_TEXCOUNT_SHIFT)  |
           ((DWORD)pRc->tl.InFVF.dwFVFType & 0xFFFFF000));

  //************************************************************
  // Compute size of texture coordinates for all FVF_COMP
  //************************************************************
  pRc->tl.dwTextureCoordSizeTotal = 0;
  ComputeTextureCoordSize((DWORD)pRc->tl.InFVF.dwFVFType, pRc->tl.dwTexCoordSize, 
               &(pRc->tl.dwTextureCoordSizeTotal));
  pRc->tl.InFVF.dwTexCoordSize = pRc->tl.dwTextureCoordSizeTotal; 
  pRc->tl.SOAFVF.dwTexCoordSize = pRc->tl.dwTextureCoordSizeTotal * SOA_SIZE;
  if( pRc->tl.InFVF.dwNumTexCoords == pRc->tl.TLFVF.dwNumTexCoords )
  {
    pRc->tl.TLFVF.dwTexCoordSize = pRc->tl.dwTextureCoordSizeTotal;
  }
  else
  {
    ComputeTextureCoordSize((DWORD)pRc->tl.TLFVF.dwFVFType, pRc->tl.dwTexCoordSize, 
               &(pRc->tl.dwTextureCoordSizeTotal));
    pRc->tl.TLFVF.dwTexCoordSize = pRc->tl.dwTextureCoordSizeTotal; 
  }
  pRc->tl.TLBN.dwTexCoordSize = (sizeof(AOS_ST) * NAPALM_MAX_TEXTURES);

    
  //  Compute output size
  pRc->tl.InFVF.dwStride = GetFVFVertexSize( pRc->tl.InFVF.dwFVFType );
  pRc->tl.TLFVF.dwStride  = GetFVFVertexSize( pRc->tl.TLFVF.dwFVFType );
  pRc->tl.SOAFVF.dwStride = pRc->tl.InFVF.dwStride * SOA_SIZE;  


  //************************************************************
  // Update the component offsets for all Input FVF_COMP 
  // strutures
  //************************************************************
  // Now compute the InFVF dependent offsets used by the Geometry loop
  UpdateComponentOffsets ((DWORD)pRc->tl.InFVF.dwFVFType, &(pRc->tl.InFVF.dwNormalOffset),
              &(pRc->tl.InFVF.dwDiffuseOffset), &(pRc->tl.InFVF.dwSpecularOffset),
              &(pRc->tl.InFVF.dwTexOffset));

  // Update the SOAFVF component offsets
  pRc->tl.SOAFVF.dwNormalOffset = pRc->tl.InFVF.dwNormalOffset * SOA_SIZE;
  pRc->tl.SOAFVF.dwDiffuseOffset = pRc->tl.InFVF.dwDiffuseOffset * SOA_SIZE;
  pRc->tl.SOAFVF.dwSpecularOffset = pRc->tl.InFVF.dwSpecularOffset * SOA_SIZE;
  pRc->tl.SOAFVF.dwTexOffset = pRc->tl.InFVF.dwTexOffset * SOA_SIZE;

  // after we calculate the size of the current texture,  
  // we can store the starting offset of the next texture
  pRc->tl.dwTLState &= ~TLPV_NON_2D_TEXTURE;
  pRc->tl.dwTexOffsetIn[0] = pRc->tl.InFVF.dwTexOffset;		// we already know 0's offset
  for( dwTexStage=0; dwTexStage < (int)(pRc->tl.InFVF.dwNumTexCoords-1); ++dwTexStage )
  {
	DWORD dwTexSize;
    switch( (pRc->tl.InFVF.dwFVFType >> (dwTexStage*2 + 16)) & 0x3 )
	{
	  case D3DFVF_TEXTUREFORMAT1:	
	    dwTexSize = 1 * sizeof(DWORD);
		pRc->tl.dwTLState |= TLPV_NON_2D_TEXTURE;	// handled in slow path only
	    break;
	  case D3DFVF_TEXTUREFORMAT2:	
	    dwTexSize = 2 * sizeof(DWORD);
	    break;
	  case D3DFVF_TEXTUREFORMAT3:	
	    dwTexSize = 3 * sizeof(DWORD);
		pRc->tl.dwTLState |= TLPV_NON_2D_TEXTURE;	// handled in slow path only
	    break;
	  case D3DFVF_TEXTUREFORMAT4:	
	    dwTexSize = 4 * sizeof(DWORD);
		pRc->tl.dwTLState |= TLPV_NON_2D_TEXTURE;	// handled in slow path only
	    break;
	}
	pRc->tl.dwTexOffsetIn[dwTexStage+1] = pRc->tl.dwTexOffsetIn[dwTexStage] + dwTexSize;
  }

  // figure out the output texture offsets, we'll need 
  // to unfudge the texture offsets after texgen in the slowpath
  pRc->tl.dwTexOffsetOut[0] = 4*sizeof(D3DVALUE);
  if (pRc->tl.TLFVF.dwFVFType & D3DFVF_DIFFUSE)
    pRc->tl.dwTexOffsetOut[0] += sizeof(D3DVALUE);
  if (pRc->tl.TLFVF.dwFVFType & D3DFVF_SPECULAR)
    pRc->tl.dwTexOffsetOut[0] += sizeof(D3DVALUE);
  for( dwTexStage=0; dwTexStage < (D3DDP_MAXTEXCOORD-1); ++dwTexStage )
	pRc->tl.dwTexOffsetOut[dwTexStage+1] = pRc->tl.dwTexOffsetOut[dwTexStage] + sizeof(AOS_UV);


  //************************************************************
  // Clear our Dirty flags before leaving our update!
  //************************************************************
  pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_FVFOUT;
  pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_FVFIN;

  return;

} /* SetupFVFData */


HRESULT UpdateTLState(RC *pRc)
{
  HRESULT hr = D3D_OK;
  int numVertexBlends;
  DWORD dwXfmBlendMask;
  TL_SOATMP *pTlTmp = pRc->tl.pTL;
  NT9XDEVICEDATA *ppdev = pRc->ppdev;

  //
  // Compute Input and Output FVF and the size of output vertices
  //
  if ((pRc->tl.dwDirtyFlags & TLPV_DIRTY_FVFOUT) ||
      (pRc->tl.dwDirtyFlags & TLPV_DIRTY_FVFIN))
    SetupFVFData(pRc);

  if( HW_STATE_CHANGED )
    setDX6state( pRc, 0, 0);        // pRc, primitiveType, #primitives

  //
  // Update Geometry Loop flags based on the current state set
  //
  pRc->tl.dwTLState &= ~TLPV_FASTPATH_OK;


  /*-------------------------------------------------------------**
  **  Fog or not:                                                **
  **  Compute fog if: 1) Fogging is enabled                      **
  **          2) VertexFog mode is not FOG_NONE                  **
  **          3) TableFog mode is FOG_NONE                       **
  **  If both table and vertex fog are not FOG_NONE, table fog   **
  **  is applied.                                                **
  **-------------------------------------------------------------*/
  if (pRc->useFogTable == FOGTABLE_DISABLED && pRc->useFog && pRc->tl.lighting.fog_mode)
    pRc->tl.dwTLState |= TLPV_DOFOG;
  else
    pRc->tl.dwTLState &= ~TLPV_DOFOG;


  // Need to compute the Min of what is in the FVF and the renderstate.
  numVertexBlends = min( pRc->tl.StateVertexBlends, (((DWORD)pRc->tl.InFVF.dwFVFType  & D3DFVF_POSITION_MASK) >> 1) - 2 );
  if(numVertexBlends != pRc->tl.numVertexBlends)
  {
    // We don't update unused world transforms unless they're used
	// This is here so if they change the number of blends without 
	// changing the world matricies, we'll update them
    pRc->tl.numVertexBlends = numVertexBlends;
	pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLDXFM_ALL;
	pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_EYEXFM_ALL;
  }

  /* Check if we need to do texgen or texture transform */
  pRc->tl.dwTLState &= ~TLPV_DOTEXGEN;
  pRc->tl.dwTLState &= ~TLPV_DOTEXXFORM;
  if(pRc->state & STATE_REQUIRES_ST_TMU0)
  {
    DWORD TCI0 = pRc->textureStage[0].texCoordIndex;
    if( (TCI0 & 0xffff0000) != D3DTSS_TCI_PASSTHRU )
	  pRc->tl.dwTLState |= TLPV_DOTEXGEN;
    if( (pRc->textureStage[0].texXformFlags & (D3DTTFF_PROJECTED-1)) != D3DTTFF_DISABLE )
	  pRc->tl.dwTLState |= TLPV_DOTEXXFORM;

    if(pRc->state & STATE_REQUIRES_ST_TMU1)
    {
	  DWORD TCI1 = pRc->textureStage[1].texCoordIndex;
      if( (TCI1 & 0xffff0000) != D3DTSS_TCI_PASSTHRU )
	    pRc->tl.dwTLState |= TLPV_DOTEXGEN;
      if( (pRc->textureStage[1].texXformFlags & (D3DTTFF_PROJECTED-1)) != D3DTTFF_DISABLE )
	    pRc->tl.dwTLState |= TLPV_DOTEXXFORM;

      if(pRc->tl.dwTLState & (TLPV_DOTEXGEN | TLPV_DOTEXXFORM))
      {
        // The rendering code grabs the texture from TCI & 0xffff.  
        // This isn't really correct.  If the texture is generated the
	    // lower word can be anything.  We'll force valid values into 
	    // the lower word if they're doing texgen.
	    if((TCI0 & 0x0000ffff) == (TCI1 & 0x0000ffff))
	    {
	      // both are getting their imputs from the same place, it may be a problem
	      if( TCI0 & 0xffff0000 )
          {
		    if( TCI1 & 0xffff0000 )
		    {
		      // both are doing texgen and trying to store in the same place
		      // so we'll move texture #1
		      if( (TCI0 & 0x0000ffff) == 0 )
			    pRc->textureStage[1].texCoordIndex = (TCI1 & 0xffff0000) | 1;
              else
			    pRc->textureStage[1].texCoordIndex = (TCI1 & 0xffff0000) | 0;
              pRc->textureStage[1].changed = TRUE;
              UPDATE_HW_STATE( SC_SOMETHING );
		    }
		    else
		    {
		      // tex0 doing texgen and tex is not, but trying to store
		      // in tex1's spot so we'll move texture 0's output
		      if ( TCI1 & 0x0000ffff == 1 )
			    pRc->textureStage[0].texCoordIndex = (TCI0 & 0xffff0000) | 0;
		      else
			    pRc->textureStage[0].texCoordIndex = (TCI0 & 0xffff0000) | 1;
              pRc->textureStage[0].changed = TRUE;
              UPDATE_HW_STATE( SC_SOMETHING );
		    }
	      }
          else if( TCI1 & 0xffff0000 )
	      {
		    // tex0 not doing texgen, tex1 doing texgen but trying to 
		    // store in tex0's spot so we'll move texture 1's output
		    if( (TCI0 & 0x0000ffff) == 0 )
			  pRc->textureStage[1].texCoordIndex = (TCI1 & 0xffff0000) | 1;
		    else
			  pRc->textureStage[1].texCoordIndex = (TCI1 & 0xffff0000) | 0;
            pRc->textureStage[1].changed = TRUE;
            UPDATE_HW_STATE( SC_SOMETHING );
	      }
		}
	  } //TLPV_DOTEXGEN | TLPV_DOTEXXFORM
    } //STATE_REQUIRES_ST_TMU1
  } //STATE_REQUIRES_ST_TMU0


  /*   Find out if something changed in the transformation state  */
  /*   Recompute digested transform state                         */

  // Figure out which blends are enabled so we only update transform and lighting for those matricies
  switch (numVertexBlends)
  {
    case 0:
      dwXfmBlendMask = TLPV_DIRTY_WORLDXFM | TLPV_DIRTY_EYEXFM;
      break;
    case 1:
      dwXfmBlendMask = TLPV_DIRTY_WORLDXFM | TLPV_DIRTY_WORLD1XFM | TLPV_DIRTY_EYEXFM | TLPV_DIRTY_EYE1XFM;
      break;
    case 2:
      dwXfmBlendMask = TLPV_DIRTY_WORLDXFM | TLPV_DIRTY_WORLD1XFM | TLPV_DIRTY_WORLD2XFM |
      				   TLPV_DIRTY_EYEXFM | TLPV_DIRTY_EYE1XFM | TLPV_DIRTY_EYE2XFM;
      break;
    case 3:
      dwXfmBlendMask = TLPV_DIRTY_WORLDXFM | TLPV_DIRTY_WORLD1XFM | TLPV_DIRTY_WORLD2XFM | TLPV_DIRTY_WORLD3XFM |
      				   TLPV_DIRTY_EYEXFM | TLPV_DIRTY_EYE1XFM | TLPV_DIRTY_EYE2XFM | TLPV_DIRTY_EYE3XFM;
      break;
	default:	
	  dwXfmBlendMask = 0;
	  break;	// bad case
  }

  if( (pRc->tl.dwDirtyFlags & (TLPV_DIRTY_ZRANGE | TLPV_DIRTY_VIEWRECT | TLPV_DIRTY_PROJXFM | TLPV_DIRTY_VIEWXFM | TLPV_DIRTY_DYNGB | TLPV_DIRTY_TXTRXFM)) ||
      (pRc->tl.dwDirtyFlags & dwXfmBlendMask & TLPV_DIRTY_WORLDXFM_ALL) || 
      ((pRc->tl.dwDirtyFlags & dwXfmBlendMask & TLPV_DIRTY_EYEXFM_ALL) && 
         (pRc->tl.dwTLState & (TLPV_DOLIGHTING | TLPV_DOFOG | TLPV_DOTEXGEN))) )
  {
	// Some transform changed, let's figure out which one it is and update it
    HR_RET(UpdateXformData(pRc, dwXfmBlendMask));
  }

  /*  Something changed in the lighting state */
  if ((pRc->tl.dwTLState & TLPV_DOLIGHTING) && (pRc->tl.dwDirtyFlags & TLPV_DIRTY_LIGHTING))
  {
    /*                                                            */
    /*  Compute Colorvertex flags only if the lighting is enabled */
    /*                                                            */
    pRc->tl.dwTLState &= ~TLPV_COLORVERTEXFLAGS;
    pRc->tl.lighting.pAmbientSrc = &pRc->tl.lighting.matAmb;
    pRc->tl.lighting.pDiffuseSrc = &pRc->tl.lighting.matDiff;
    pRc->tl.lighting.pSpecularSrc = &pRc->tl.lighting.matSpec;
    pRc->tl.lighting.pEmissiveSrc = &pRc->tl.lighting.matEmis;
    pRc->tl.lighting.pDiffuseAlphaSrc = &pRc->tl.lighting.materialDiffAlpha;
    pRc->tl.lighting.pSpecularAlphaSrc = &pRc->tl.lighting.materialSpecAlpha;

	// Fastpath colorvertex values
	if (pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE) 
	{
	  // load default colovertex offsets in the TL_SOATMP struct
	  pRc->tl.lighting.pSOACvAmbientSrc = (DWORD)&pTlTmp->fSOAmatAmbient;
      pRc->tl.lighting.pSOACvDiffuseSrc = (DWORD)&pTlTmp->fSOAmatDiffuse;
      pRc->tl.lighting.pSOACvSpecularSrc = (DWORD)&pTlTmp->fSOAmatSpecular;
      pRc->tl.lighting.pSOACvEmissiveSrc = (DWORD)&pTlTmp->fSOAmatEmissive;
	  pRc->tl.lighting.dSOAOffsetDiffAlphaSrc = 0;
	}
	else if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
	{
	  // load default colovertex offsets in a TLBN (0 means use the material)
      pRc->tl.lighting.dwK3dOffsetCvAmbientSrc = 0;
      pRc->tl.lighting.dwK3dOffsetCvDiffuseSrc = 0;
      pRc->tl.lighting.dwK3dOffsetCvSpecularSrc = 0;
      pRc->tl.lighting.dwK3dOffsetCvEmissiveSrc = 0;
	}

    if (pRc->tl.dwTLState & TLPV_COLORVERTEXNEEDED)
    {
      switch( pRc->tl.lighting.AmbMaterialSrc )
      {
      case D3DMCS_MATERIAL:
        if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
        {
    	  pRc->tl.lighting.dwMaterialAmbient = 
    		( ((FTOI(pRc->tl.lighting.Material.ambient.a * 255.0f)) << 24) |
			  ((FTOI(pRc->tl.lighting.Material.ambient.r * 255.0f)) << 16) |
			  ((FTOI(pRc->tl.lighting.Material.ambient.g * 255.0f)) <<  8) |
			  ((FTOI(pRc->tl.lighting.Material.ambient.b * 255.0f)) <<  0) );
		}
        break;
      case D3DMCS_COLOR1:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_DIFFUSE)
          {
		    lpTLBN tmpTLBN = 0;
            pRc->tl.dwTLState |= (TLPV_VERTEXDIFFUSENEEDED | TLPV_COLORVERTEXAMB);
            pRc->tl.lighting.pAmbientSrc = &pRc->tl.lighting.vertexDiffuse;
			pRc->tl.lighting.pSOACvAmbientSrc = (DWORD)&pTlTmp->fSOADiffuseIn;
            pRc->tl.lighting.dwK3dOffsetCvAmbientSrc = (DWORD)&tmpTLBN->diffuse;
          }
        }
        break;
      case D3DMCS_COLOR2:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_SPECULAR)
          {
		    lpTLBN tmpTLBN = 0;
            pRc->tl.dwTLState |= (TLPV_VERTEXSPECULARNEEDED | TLPV_COLORVERTEXAMB);
            pRc->tl.lighting.pAmbientSrc = &pRc->tl.lighting.vertexSpecular;
			pRc->tl.lighting.pSOACvAmbientSrc = (DWORD)&pTlTmp->fSOASpecularIn;
            pRc->tl.lighting.dwK3dOffsetCvAmbientSrc = (DWORD)&tmpTLBN->specular;
          }
        }
        break;
      }

      switch( pRc->tl.lighting.DfusMaterialSrc )
      {
      case D3DMCS_MATERIAL:
        if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
        {
    	  pRc->tl.lighting.dwMaterialDiffuse =
    		(      pRc->tl.lighting.materialDiffAlpha                    |
			((FTOI(pRc->tl.lighting.Material.diffuse.r * 255.0f)) << 16) |
			((FTOI(pRc->tl.lighting.Material.diffuse.g * 255.0f)) <<  8) |
			((FTOI(pRc->tl.lighting.Material.diffuse.b * 255.0f)) <<  0) );
		}
        break;
      case D3DMCS_COLOR1:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_DIFFUSE)
          {
		    lpTLBN tmpTLBN = 0;
            pRc->tl.dwTLState |= (TLPV_VERTEXDIFFUSENEEDED | TLPV_COLORVERTEXDIFF);
            pRc->tl.lighting.pDiffuseSrc = &pRc->tl.lighting.vertexDiffuse;
            pRc->tl.lighting.pDiffuseAlphaSrc = &pRc->tl.lighting.vertexDiffAlpha;
			pRc->tl.lighting.pSOACvDiffuseSrc = (DWORD)&pTlTmp->fSOADiffuseIn;
			pRc->tl.lighting.dSOAOffsetDiffAlphaSrc = pRc->tl.SOAFVF.dwDiffuseOffset;
            pRc->tl.lighting.dwK3dOffsetCvDiffuseSrc = (DWORD)&tmpTLBN->diffuse;
          }
        }
        break;
      case D3DMCS_COLOR2:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_SPECULAR)
          {
		    lpTLBN tmpTLBN = 0;
            pRc->tl.dwTLState |= (TLPV_VERTEXSPECULARNEEDED | TLPV_COLORVERTEXDIFF);
            pRc->tl.lighting.pDiffuseSrc = &pRc->tl.lighting.vertexSpecular;
            pRc->tl.lighting.pDiffuseAlphaSrc = &pRc->tl.lighting.vertexSpecAlpha;
			pRc->tl.lighting.pSOACvDiffuseSrc = (DWORD)&pTlTmp->fSOASpecularIn;
			pRc->tl.lighting.dSOAOffsetDiffAlphaSrc = pRc->tl.SOAFVF.dwSpecularOffset;
            pRc->tl.lighting.dwK3dOffsetCvDiffuseSrc = (DWORD)&tmpTLBN->specular;
          }
        }
        break;
      }

      switch( pRc->tl.lighting.SpecMaterialSrc )
      {
      case D3DMCS_MATERIAL:
        if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
        {
    	  pRc->tl.lighting.dwMaterialSpecular =
			(      pRc->tl.lighting.materialSpecAlpha                     |
			((FTOI(pRc->tl.lighting.Material.specular.r * 255.0f)) << 16) |
			((FTOI(pRc->tl.lighting.Material.specular.g * 255.0f)) <<  8) |
			((FTOI(pRc->tl.lighting.Material.specular.b * 255.0f)) <<  0) );
		}
        break;
      case D3DMCS_COLOR1:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_DIFFUSE)
          {
		    lpTLBN tmpTLBN = 0;
            pRc->tl.dwTLState |= (TLPV_VERTEXDIFFUSENEEDED | TLPV_COLORVERTEXSPEC);
            pRc->tl.lighting.pSpecularSrc = &pRc->tl.lighting.vertexDiffuse;
            pRc->tl.lighting.pSpecularAlphaSrc = &pRc->tl.lighting.vertexDiffAlpha;
			pRc->tl.lighting.pSOACvSpecularSrc = (DWORD)&pTlTmp->fSOADiffuseIn;
            pRc->tl.lighting.dwK3dOffsetCvSpecularSrc = (DWORD)&tmpTLBN->diffuse;
          }
        }
        break;
      case D3DMCS_COLOR2:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_SPECULAR)
          {
		    lpTLBN tmpTLBN = 0;
            pRc->tl.dwTLState |= (TLPV_VERTEXSPECULARNEEDED | TLPV_COLORVERTEXSPEC);
            pRc->tl.lighting.pSpecularSrc = &pRc->tl.lighting.vertexSpecular;
            pRc->tl.lighting.pSpecularAlphaSrc = &pRc->tl.lighting.vertexSpecAlpha;
			pRc->tl.lighting.pSOACvSpecularSrc = (DWORD)&pTlTmp->fSOASpecularIn;
            pRc->tl.lighting.dwK3dOffsetCvSpecularSrc = (DWORD)&tmpTLBN->specular;
          }
        }
        break;
      }

      switch( pRc->tl.lighting.EmisMaterialSrc )
      {
      case D3DMCS_MATERIAL:
        if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
		{
		  pRc->tl.lighting.dwMaterialEmissive =
			( ((FTOI(pRc->tl.lighting.Material.emissive.a * 255.0f)) << 24) |
		      ((FTOI(pRc->tl.lighting.Material.emissive.r * 255.0f)) << 16) |
			  ((FTOI(pRc->tl.lighting.Material.emissive.g * 255.0f)) <<  8) |
			  ((FTOI(pRc->tl.lighting.Material.emissive.b * 255.0f)) <<  0) );
		}
        break;
      case D3DMCS_COLOR1:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_DIFFUSE)
          {
		    lpTLBN tmpTLBN = 0;
            pRc->tl.dwTLState |= (TLPV_VERTEXDIFFUSENEEDED | TLPV_COLORVERTEXEMIS);
            pRc->tl.lighting.pEmissiveSrc = &pRc->tl.lighting.vertexDiffuse;
			pRc->tl.lighting.pSOACvEmissiveSrc = (DWORD)&pTlTmp->fSOADiffuseIn;
            pRc->tl.lighting.dwK3dOffsetCvEmissiveSrc = (DWORD)&tmpTLBN->diffuse;
          }
        }
        break;
      case D3DMCS_COLOR2:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_SPECULAR)
          {
		    lpTLBN tmpTLBN = 0;
            pRc->tl.dwTLState |= (TLPV_VERTEXSPECULARNEEDED | TLPV_COLORVERTEXEMIS);
            pRc->tl.lighting.pEmissiveSrc = &pRc->tl.lighting.vertexSpecular;
			pRc->tl.lighting.pSOACvEmissiveSrc = (DWORD)&pTlTmp->fSOASpecularIn;
            pRc->tl.lighting.dwK3dOffsetCvEmissiveSrc = (DWORD)&tmpTLBN->specular;
          }
        }
        break;
      }
    }


    /*  If specular is needed in the output and has been provided */
    /*  in the input, force the copy of specular data             */
    if ((pRc->tl.InFVF.dwFVFType & D3DFVF_SPECULAR) && (pRc->specular == FALSE))
    {
      //pRc->tl.dwTLState |= TLPV_VERTEXSPECULARNEEDED;
      pRc->tl.dwTLState |= TLPV_SPEC_SRC_VTX;
    }

    /*  Update the remaining light state */
    HR_RET(UpdateLightingData(pRc));
  }

  if ((pRc->tl.dwTLState & TLPV_DOFOG) && (pRc->tl.dwDirtyFlags & TLPV_DIRTY_FOG))
  {
    HR_RET(UpdateFogData(pRc));
  }

  if (pRc->tl.dwTLState & TLPV_DOCLIPPING)
  {
    /*  Figure out which pieces need to be interpolated in new vertices. */
    pRc->tl.clipping.dwInterpolate = 0;
    pRc->tl.clipping.dwInterpolate |= TLCLIP_INTERPOLATE_COLOR;
    if ((pRc->tl.TLFVF.dwFVFType & D3DFVF_SPECULAR) ||  (pRc->fogEnable))
      pRc->tl.clipping.dwInterpolate |= TLCLIP_INTERPOLATE_SPECULAR;
    if (FVF_TEXCOORD_NUMBER(pRc->tl.InFVF.dwFVFType) != 0)
      pRc->tl.clipping.dwInterpolate |= TLCLIP_INTERPOLATE_TEXTURE;

    /*  Clear clip union and intersection flags */
    pRc->tl.clipIntersection = 0;
    pRc->tl.clipUnion = 0;

    if( pRc->tl.dwDirtyFlags & TLPV_DIRTY_CLIPPLANES )
      HR_RET( UpdateClippingData( pRc, pRc->tl.clipPlaneEnable ));
  }


  // ************************************************************************************************  
  // Figure out where Diffuse RGB , Specular RGB, Diffuse Alpha, and Specular Alpha come from
  //
  // RGB colors and alpha for vertices are essentially a big MUX. The RGB color for diffuse and
  // specular can come from different places, depending on the render state and input vertex.
  //
  // DIFFUSE:
  // RGB - Source is one of the following:
  // 0              -     No diffuse
  // Input Vertex   -     Diffuse is given in vertex
  // Lighting       -     Diffuse is computed by lighting code
  //
  // Alpha - Source is one of the following:
  // 0              -     No diffuse
  // Input Vertex   -     Alpha is given in vertex
  // Material       -     Alpha is given in material
  // 
  //
  //
  // SPECULAR:
  // RGB - source is one of the following:
  // 0              -     No specular
  // Input vertex   -     Specular is given in vertex
  // Lighting       -     specular is computed
  //
  // Alpha - source is one of the following:
  // 0              -     No Alpha
  // Input vertex   -     alpha is given in vertex
  // Material       -     alpha is given in material
  // Fog            -     alpha is computed by fog routine
  //
  // When fog is enabled, there are some additional complications.
  // Napalm needs the fog factor given as a float when vertex fog is
  // enabled. The fog is available as a float when;
  // Alpha src = Material, Alpha Src = Fog. When Alpha Src = Input vertex,
  // we have a problem -- the input vertex gets it's fog from the integer
  // alpha portion of the specular RGBA color. So we have to convert this 
  // to a float.

  pRc->tl.dwTLDevFlags = 0;
  pRc->tl.dwTLState &= ~(TLPV_DIFF_SRC_VTX | TLPV_SPEC_SRC_VTX) ;
   
#if defined(FASTPATH) && defined(VCPP)
  // Update Vertex diffuse color and alpha info
  if (pRc->tl.dwTLState & TLPV_DOLIGHTING)  
  { // If lighting , copy diffuse from computed LIGHT
    pRc->tl.dwTLDevFlags |= TLPV_DEV_FTOI_DIFF_RGB | TLPV_DEV_COMBINE_DIFF;
    pRc->tl.lighting.pdSOADiffRGBSrc = &pTlTmp->dSOADiffuse;
    if (pRc->tl.lighting.dSOAOffsetDiffAlphaSrc)
      pRc->tl.lighting.pdSOADiffAlphaSrc = &pTlTmp->dSOADiffAlpha;
	else
      pRc->tl.lighting.pdSOADiffAlphaSrc = &pTlTmp->dSOAMatDiffAlpha;
  }
  else  
  {
    pRc->tl.dwTLDevFlags |= TLPV_DEV_COPY_DIFF;
    if (pRc->tl.SOAFVF.dwFVFType & D3DFVF_DIFFUSE)  
    { // if not lighting, and input vtx has diffuse, copy that
      pRc->tl.dwTLState |= TLPV_DIFF_SRC_VTX;
      pRc->tl.lighting.pdSOADiffRGBSrc = (SOA_DWORD *) &TL_maskLow24;   // initialize to something to insure 
      pRc->tl.lighting.pdSOADiffAlphaSrc = (SOA_DWORD *) &TL_maskHigh8; // a valid pointer - cheap insurance in case this is accessed incorrectly.
      if (pRc->tl.lighting.dSOAOffsetDiffAlphaSrc)
        pRc->tl.lighting.pdSOADiffAlphaSrc = &pTlTmp->dSOADiffAlpha;
	  else
        pRc->tl.lighting.pdSOADiffAlphaSrc = &pTlTmp->dSOAMatDiffAlpha;
    }
    else  
    { // else copy in zeros
      pRc->tl.dwTLDevFlags |= TLPV_DEV_COPY_SPEC;	// we didn't create diff and it wasn't passed in, so why do this???
      pRc->tl.lighting.pdSOADiffRGBSrc = (SOA_DWORD *) &TL_maskLow24;
      pRc->tl.lighting.pdSOADiffAlphaSrc = (SOA_DWORD *) &TL_maskHigh8;
      if (pRc->tl.lighting.dSOAOffsetDiffAlphaSrc)
        pRc->tl.lighting.pdSOADiffAlphaSrc = (SOA_DWORD*)&TL_soa_0;
	  else
        pRc->tl.lighting.pdSOADiffAlphaSrc = &pTlTmp->dSOAMatDiffAlpha;
    }
  }


  if (pRc->state & STATE_REQUIRES_VERTEXFOG)
      pRc->tl.dwTLDevFlags |= TLPV_DEV_VERTEX_FOG;

  // Update Vertex specular color and alpha info
  if ((pRc->tl.dwTLState & TLPV_DOLIGHTING) && (pRc->tl.dwTLState & TLPV_DOSPECULAR))  {
    // If lighting & specular specular color from computed light
    pRc->tl.dwTLDevFlags |= TLPV_DEV_FTOI_SPEC_RGB | TLPV_DEV_COMBINE_SPEC;
    pRc->tl.lighting.pdSOASpecRGBSrc = &pTlTmp->dSOASpecular;

    if (pRc->tl.dwTLState & TLPV_DOFOG)  
    {   // Alpha from Fog
        pRc->tl.lighting.pdSOASpecAlphaSrc = &pTlTmp->dSOAFog;
        pRc->tl.lighting.pfSOASpecAlphaSrc = &pTlTmp->fSOAFog;
    } 
    else 
    {	// Alpha from Material
        pRc->tl.lighting.pdSOASpecAlphaSrc = &pTlTmp->dSOAMatSpecAlpha;
        pRc->tl.lighting.pfSOASpecAlphaSrc = &pTlTmp->fSOAMatSpecAlpha;
    }
  }
  else if (pRc->tl.SOAFVF.dwFVFType & D3DFVF_SPECULAR) // no lighting, specular passed in
  {
    // We have vertex specular
    pRc->tl.dwTLState |= TLPV_SPEC_SRC_VTX;
    // Color Source is from vertex
    pRc->tl.lighting.pdSOASpecRGBSrc   = (SOA_DWORD *) &TL_soa_0;	// initialize to something to insure 
    pRc->tl.lighting.pdSOASpecAlphaSrc = (SOA_DWORD *) &TL_soa_0;	// a valid pointer - cheap insurance 
    pRc->tl.lighting.pfSOASpecAlphaSrc = (SOA_FLOAT *) &TL_soa_0;	// in case this is accessed incorrectly.

    // Vertex fog computed?
    if (pRc->tl.dwTLState & TLPV_DOFOG)  
    { // Combine computed vertex fog and RGB from vertex
      pRc->tl.dwTLDevFlags |= TLPV_DEV_COMBINE_SPEC;
      pRc->tl.lighting.pdSOASpecAlphaSrc = &pTlTmp->dSOAFog;
      pRc->tl.lighting.pfSOASpecAlphaSrc = &pTlTmp->fSOAFog;
    } 
    else 
    { // Specular, but no computed vertex fog.  Just copy the specular
      pRc->tl.dwTLDevFlags |= TLPV_DEV_COPY_SPEC;
      // But wait, what if vertex fog is enabled in the RENDER state?
      if (pRc->state & STATE_REQUIRES_VERTEXFOG)  
      { // We must convert specular alpha (passed in vtx) to float for fog!
        pRc->tl.dwTLDevFlags |= TLPV_DEV_ITOF_SPEC_ALPHA;
        pRc->tl.lighting.pfSOASpecAlphaSrc = &pTlTmp->fSOAFog;		// i dunno about this one!!!
      }
    }
  }
  else // Lighting, but specular not enabled!
  { // Lighting, no specular highlights either computed or passed in
    // Copy Zero's for specular!
    pRc->tl.lighting.pdSOASpecRGBSrc = (SOA_DWORD *) &TL_soa_0;
    pRc->tl.lighting.pdSOASpecAlphaSrc = (SOA_DWORD *) &TL_soa_0;
    pRc->tl.lighting.pfSOASpecAlphaSrc = (SOA_FLOAT *) &TL_soa_0;

    if (pRc->tl.dwTLState & TLPV_DOFOG)  
    {
      // Combine computed vertex fog and RGB from vertex
      pRc->tl.dwTLDevFlags |= TLPV_DEV_COMBINE_SPEC;
      pRc->tl.lighting.pdSOASpecAlphaSrc = &pTlTmp->dSOAFog;
      pRc->tl.lighting.pfSOASpecAlphaSrc = &pTlTmp->fSOAFog;
    }
    else  
    {
      pRc->tl.dwTLDevFlags |= TLPV_DEV_COPY_SPEC;
    }
  }
#endif    
  // ************************************************************************************************  

  if (!(pRc->tl.dwTLState & pRc->tl.dwTLBadFlags))
    pRc->tl.dwTLState |= TLPV_FASTPATH_OK;

  return hr;
}



/**********************************************************************************
* Figure out what kind of primitive we have.  The T&L path only handles 
* IndextedTriangleList2.  So if it's any kind of triangle we turn it
* into an IndextedTriangleList2.  Lines and points go down the slow
* path, at least for now.
**********************************************************************************/
BOOL ConvertPrimitiveToIndexTriList( RC *pRc )
{
	switch(pRc->tl.primType)
	{
		case D3DPT_TRIANGLELIST:
		{
			if(pRc->tl.pIndices)	// D3DDP2OP_INDEXEDTRIANGLELIST2 default case
			{	// INDEXEDTRIANGLELIST2
				return TRUE;	// it's already done
			}
			else
			{	// TRIANGLELIST
				DWORD dwNumVertices = pRc->tl.dwNumVertices;
				LPWORD pSynthIndices = (LPWORD)pRc->tl.pSynthIndices.alignedBuf;
                DWORD idx;
				*pSynthIndices++ = 0;	// skip the first indice
				for(idx=0; idx<dwNumVertices; ++idx)
					*pSynthIndices++ = (unsigned short) idx;
				pRc->tl.dwNumIndices = (DWORD)idx;
				pRc->tl.pIndices = (LPWORD)pRc->tl.pSynthIndices.alignedBuf;
				return TRUE;
			}
		 }

		case D3DPT_TRIANGLESTRIP:
		{
            DWORD idx, dwNumTris;
			WORD nextA, nextB;
			LPWORD pSynthIndices = (LPWORD)pRc->tl.pSynthIndices.alignedBuf;
			if(pRc->tl.pIndices)	// D3DDP2OP_INDEXEDTRIANGLESTRIP
			{	// INDEXEDTRIANGLESTRIP
				LPWORD pSrcIndices = pRc->tl.pIndices;
				dwNumTris = pRc->tl.dwNumIndices-2;
				*pSynthIndices++ = 0;	pSrcIndices++;	// skip the first indices
				nextA = *pSrcIndices++;
				nextB = *pSrcIndices++;
				if ( pRc->shadeMode == D3DSHADE_FLAT )
				{	// flat won't be stripable, but we can't stip flat-shaded tri's anyway
					// just need to get the order right (vertex "A" suppies color)
					for(idx=0; idx<dwNumTris; idx+=2)
					{
						pSynthIndices[0] = nextA;					// A1
						pSynthIndices[1] = nextA = nextB;			// B1
						pSynthIndices[2] = nextB = pSrcIndices[0];	// C1
						pSynthIndices[3] = nextA;					// A2
						pSynthIndices[5] = nextA = nextB;			// C2
						pSynthIndices[4] = nextB = pSrcIndices[1];	// B2
						pSynthIndices += 6;
						pSrcIndices += 2;
					}
				}
				else
				{
					for(idx=0; idx<dwNumTris; idx+=2)
					{
						pSynthIndices[0] = nextA;					// A1
						pSynthIndices[1] = nextA = nextB;			// B1
						pSynthIndices[2] = nextB = pSrcIndices[0];	// C1
						pSynthIndices[4] = nextA;					// B2
						pSynthIndices[3] = nextA = nextB;			// A2
						pSynthIndices[5] = nextB = pSrcIndices[1];	// C2
						pSynthIndices += 6;
						pSrcIndices += 2;
					}
				}
			}
			else
			{	// TRIANGLESTRIP
				dwNumTris = pRc->tl.dwNumVertices-2;
				*pSynthIndices++ = 0;					// skip the first indice
				nextA = 0;
				nextB = 1;
				if ( pRc->shadeMode == D3DSHADE_FLAT )
				{	// flat won't be stripable, but we can't stip flat-shaded tri's anyway
					for(idx=0; idx<dwNumTris; idx+=2)
					{
						pSynthIndices[0] = nextA;			// A1
						pSynthIndices[1] = nextA = nextB;	// B1
						pSynthIndices[2] = ++nextB;			// C1
						pSynthIndices[3] = nextA;		 	// A2
						pSynthIndices[5] = nextA = nextB;	// C2
						pSynthIndices[4] = ++nextB;			// B2
						pSynthIndices += 6;
					}
				}
				else
				{
					for(idx=0; idx<dwNumTris; idx+=2)
					{
						pSynthIndices[0] = nextA;			// A1
						pSynthIndices[1] = nextA = nextB;	// B1
						pSynthIndices[2] = ++nextB;			// C1
						pSynthIndices[4] = nextA;			// B2
						pSynthIndices[3] = nextA = nextB;	// A2
						pSynthIndices[5] = ++nextB;			// C2
						pSynthIndices += 6;
					}
				}
			}
			pRc->tl.dwNumIndices = dwNumTris*3;
			pRc->tl.pIndices = (LPWORD)pRc->tl.pSynthIndices.alignedBuf;
			return TRUE;
		}

		case D3DPT_TRIANGLEFAN:
		{
            DWORD idx, dwNumTris;
			WORD center, lastC;
			LPWORD pSynthIndices = (LPWORD)pRc->tl.pSynthIndices.alignedBuf;
			if(pRc->tl.pIndices)	// D3DDP2OP_INDEXEDTRIANGLEFAN
			{	// Turn INDEXEDTRIANGLEFAN into INDEXEDTRIANGLELIST2
				LPWORD pSrcIndices = pRc->tl.pIndices;
				dwNumTris = pRc->tl.dwNumIndices-2;
				*pSynthIndices++ = 0;	pSrcIndices++;	// skip the first indices
				center = *pSrcIndices++;
				lastC = *pSrcIndices++;
				if ( pRc->shadeMode == D3DSHADE_FLAT )
				{	// flat won't be stripable, but we can't stip flat-shaded tri's anyway
					// just need to get the order right (vertex "B" suppies color)
					for(idx=0; idx<dwNumTris; idx++)
					{
						pSynthIndices[0] = lastC;					// A
						pSynthIndices[1] = lastC = *pSrcIndices++;	// B
						pSynthIndices[2] = center;					// C
						pSynthIndices += 3;
					}
				}
				else
				{
					for(idx=0; idx<dwNumTris; idx++)
					{
						pSynthIndices[0] = center;					// A
						pSynthIndices[1] = lastC;					// B
						pSynthIndices[2] = lastC = *pSrcIndices++;	// C
						pSynthIndices += 3;
					}
				}
			}
			else					// D3DDP2OP_TRIANGLEFAN
			{	// Turn TRIANGLEFAN into INDEXEDTRIANGLELIST2
				dwNumTris = pRc->tl.dwNumVertices-2;
				*pSynthIndices++ = 0;				// skip the first indice
				lastC = 1;
				if ( pRc->shadeMode == D3DSHADE_FLAT )
				{	// flat won't be stripable, but we can't stip flat-shaded tri's anyway
					for(idx=0; idx<dwNumTris; idx++)
					{
						pSynthIndices[0] = lastC;		// A
						pSynthIndices[1] = ++lastC;		// B
						pSynthIndices[2] = 0;	   		// C
						pSynthIndices += 3;
					}
				}
				else
				{
					for(idx=0; idx<dwNumTris; idx++)
					{
						pSynthIndices[0] = 0;			// A
						pSynthIndices[1] = lastC;		// B
						pSynthIndices[2] = ++lastC;		// C
						pSynthIndices += 3;
					}
				}
			}
			pRc->tl.dwNumIndices = dwNumTris*3;
			pRc->tl.pIndices = (LPWORD)pRc->tl.pSynthIndices.alignedBuf;
			return TRUE;
		}

		// Everything else gets the slow path
		case D3DPT_POINTLIST:		// D3DDP2OP_POINTS
		case D3DPT_LINELIST:		// D3DDP2OP_INDEXEDLINELIST2, D3DDP2OP_LINELIST, D3DDP2OP_LINESTRIP
		default:
		{
			return FALSE;
		}
	}
}


HRESULT ProcessPrimitive( RC *pRc )
{
  SETUP_PPDEV(pRc)
  HRESULT ret = D3D_OK;
  DWORD dwVertexPoolSize = 0;

  //
  // Update T&L state (must be before FVFData is set up)
  //

  // Update Lighting and related state and flags
  if ((ret = UpdateTLState(pRc)) != D3D_OK)
    return ret;  // There must be a serious problem!  Abort and skip over this Primitive!

  // Adjust the pointer to the input vertices if there is 
  // an offset passed in by DP2
  if (pRc->tl.InFVF.dwVStart)
  {
    pRc->tl.InFVF.lpvData = (LPVOID)((LPBYTE)pRc->tl.InFVF.lpvData + 
               (pRc->tl.InFVF.dwVStart * pRc->tl.InFVF.dwStride));
    D3DPRINT(32, "dwVStart is %d, marking VB as dirty", pRc->tl.InFVF.dwVStart);
    pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VB;	// now the vertex buffer is hosed!
  }


#if defined(FASTPATH) 
#if defined(VCPP)
  if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
  {

    // This is the Big Check for the FASTPATH
    // If we don't have acccess to a valid Vertex Buffer,
    // we can't do squat!  We also need to qualify the VB too

    // First, determine if our lpVBSurfData pointer is valid
	// Won't be doing vertex blends in this path (no apps w/blends with
	// a valid lpVBSurfData yet so I can't verify that the blend code works)
    if(pRc->tl.lpVBSurfData)
    {
      LPVBSURFACEDATA lpVBSD = pRc->tl.lpVBSurfData;

      // We can really only work with Vertex Data that's 
      // marked as write only.  Also, if the VB is 
      // animated or is being updated frequenmtly by the 
      // app, we should just let the SLOWPATH take it
      if((lpVBSD->dwSrcFlags & VBSURF_WRITEONLY) && 
        !(lpVBSD->dwSrcFlags & VBSURF_NOTQUALIFY))
      {

        // If this is the first time seeing this VB
        // Check it out to see if it is too big
        if(!(lpVBSD->pOptAllocAddr)) 
        {
		  // 319 = 288+31 (extra memory added for swizzling)
          DWORD VertCnt = ((lpVBSD->dwSrcAllocSize - 319) / pRc->tl.InFVF.dwStride);
          if( VertCnt > TLMAXNUMVERTICES )
          {
            lpVBSD->dwSrcFlags |= VBSURF_NOTQUALIFY;
            pRc->tl.dwTLState &= ~TLPV_FASTPATH_OK;
          }
          else
          {
            D3DPRINT(32, "VB VertCount = %d", VertCnt);
            lpVBSD->dwNumVerts = VertCnt;
          }
        }

        // We need a more elegant way of handling this...
        // Check if is OK to use fast path 
        if ( (pRc->tl.primType == D3DPT_TRIANGLELIST) && pRc->tl.pIndices &&	// indexed tri's only
             (pRc->tl.dwTLState & TLPV_FASTPATH_OK) &&
             (pRc->tl.numVertexBlends == 0) && 					// vertex blends implemented in the UM path
             1 ) 
        {
          if( HW_STATE_CHANGED )
             setDX6state( pRc, 0, 0);         // pRc, primitiveType, #primitives

          return FP_IndexedTriangleList2_SOA_Split(pRc);
        }
      }
    } //if lpVBSurfData

    if ( (pRc->tl.dwNumVertices <= TLMAXNUMVERTICES) && 
         (pRc->tl.dwTLState & TLPV_FASTPATH_OK ) &&
         (pRc->tl.primType == D3DPT_TRIANGLELIST) )
    {
	  if ( ConvertPrimitiveToIndexTriList(pRc) )
      {
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 0, 0);        // pRc, primitiveType, #primitives

        return FP_IndexedTriangleList2_SOA_UM_Split(pRc);
      }
    }
  } //if TL_CODEPATH_SSE
  else
#endif // VCPP
  if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOW)
  {
    if ((pRc->tl.dwNumVertices <= TLMAXNUMVERTICES) && (pRc->tl.dwTLState & TLPV_FASTPATH_OK ))
	{
	  if ( ConvertPrimitiveToIndexTriList(pRc) )
      {
        if( HW_STATE_CHANGED )
          setDX6state( pRc, 0, 0);        // pRc, primitiveType, #primitives

        return FP_IndexedTriangleList2_K3D(pRc);
      }
    }
  } //if TL_CODEPATH_3DNOW
#endif // FASTPATH

  // Set the ClipCode Buffer Valid flag if clipping is on
  if (pRc->tl.dwTLState & TLPV_DOCLIPPING) 
    pRc->tl.dwTLState |=  TLPV_VALIDCLIPBUFFER;
  else								    
    pRc->tl.dwTLState &= ~TLPV_VALIDCLIPBUFFER;

  // Transform, Light and compute clipping for vertices
  if (ProcessVertices(pRc))
  {
    // If the entire primitive lies outside the view frustum, quit
    // without drawing anything.  Tell DP2 that we need to skip over
    // this whole primitive
    ret = !D3D_OK;
  }


  return ret;
}

#endif //TnL_HAL
#endif //DX >= 7


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
** File name: clip.c
**
** Description: Clipping code for T&L HAL
**
** $Revision: 23$
** $Date: 10/11/00 8:49:24 PM$
**
** $Log: 
**  23   3dfx      1.9.1.1.1.1010/11/00 Brent           Forced check in to enforce
**       branching.
**  22   3dfx      1.9.1.1.1.9 10/10/00 Allen Hansen    simplified color
**       interpolation
**  21   3dfx      1.9.1.1.1.8 10/03/00 Allen Hansen    fixed PRS 15644 (Silex
**       screen saver): bug in clipping fog, changed TLCLIPVTX.color to diffuse
**  20   3dfx      1.9.1.1.1.7 09/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  19   3dfx      1.9.1.1.1.6 09/02/00 Allen Hansen    fixed texCoordIndex bug,
**       texgen type must be masked from upper word
**  18   3dfx      1.9.1.1.1.5 08/31/00 Allen Hansen    fixed bug when wfbi was
**       being used for Z, only interpolate wfbi if it's really fog, then if it's
**       being used for Z we calculate "wfbi = (w * pRc->aW) + pRc->bW" when
**       rebuilding a TLBN vertex (found this with Messiah)
**  17   3dfx      1.9.1.1.1.4 08/27/00 Allen Hansen    removed
**       InterpolateColorsAsmKNI() (everyone goes through the mmx code now)
**  16   3dfx      1.9.1.1.1.3 08/21/00 Allen Hansen    whql fog bug fix, don't
**       copy colors unless we need them, in ClipSingleTriangleXXX() don't save
**       colors for verts B & C on flat shaded tri's (a ref driver error)
**  15   3dfx      1.9.1.1.1.2 07/29/00 Allen Hansen    moved g128pt0 to soaconst,
**       wrote InterpolateColorsAsmMMX for all fast-path cpu's (KNI version won't
**       be used anymore), checked for vertex fog before interpolating wfbi
**  14   3dfx      1.9.1.1.1.1 07/24/00 Allen Hansen    whql bug fixes (T&L tests)
**  13   3dfx      1.9.1.1.1.0 07/03/00 Allen Hansen    Changed processor specific
**       check from == to &
**  12   3dfx      1.9.1.1     06/11/00 Allen Hansen    Fixed bug in
**       InterpolateColorsAsmKNI (my bad)
**  11   3dfx      1.9.1.0     06/04/00 Allen Hansen    InterpolateColorsAsmKNI()
**       now only runs on KNI cpu's
**  10   Napalm Shared1.9         04/17/00 Allen Hansen    Wrote asm version of the
**       color interpolation functions, can be disabled by commenting out "#ifdef
**       INTERPOLATE_COLORS_ASM"
**  9    Napalm Shared1.8         03/14/00 Matt McClure    Modifications for
**       WBuffer and Fog Fast Path clipping bugs
**  8    Napalm Shared1.7         03/01/00 Matt McClure    Added code to support
**       WFBI interpolation
**  7    Napalm Shared1.6         02/23/00 Matt McClure    Modifications to choose
**       the correct wrap flags
**  6    Napalm Shared1.5         02/09/00 Scott Kephart   Fix for clip mirror --
**       user clip planes were never enabled.
**  5    Napalm Shared1.4         01/28/00 Scott Kephart   Big T&L Merge: changes
**       to FVF handling
**  4    Napalm Shared1.3         12/13/99 Scott Kephart   Big T&L Update:
**       1. Improved T&L profiling code
**       2. Optimizations to scalar transformation and lighting code
**       3. Changes to the vertex buffer code to allow functionality under Windows
**       2000.
**  3    Napalm Shared1.2         10/27/99 Russ Lind       added #ifdef TnL_HAL
**       right after the #if (DX >= 7)
**  2    Napalm Shared1.1         10/26/99 Scott Kephart   
**  1    Napalm Shared1.0         10/25/99 Scott Kephart   
** $
 * 
 * 4     1/25/00 10:17p Skephart
 * BobJ Merge
** 
** 7     10/19/99 12:20p Skephart
** Add support for User Clipping Planes
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

#define INTERPOLATE_COLORS_ASMKNI

const DWORD CLIPPED_USERCLIPPLANE0 = TLCLIP_USERCLIPPLANE5 << 8;
const DWORD CLIPPED_USERCLIPPLANE1 = TLCLIP_USERCLIPPLANE5 << 9;
const DWORD CLIPPED_USERCLIPPLANE2 = TLCLIP_USERCLIPPLANE5 << 10;
const DWORD CLIPPED_USERCLIPPLANE3 = TLCLIP_USERCLIPPLANE5 << 11;
const DWORD CLIPPED_USERCLIPPLANE4 = TLCLIP_USERCLIPPLANE5 << 12;
const DWORD CLIPPED_USERCLIPPLANE5 = TLCLIP_USERCLIPPLANE5 << 13;


#define GET_NEW_CLIP_VERTEX \
&pRc->tl.clipping.clip_vertices[pRc->tl.clipping.clip_vertices_used++];

void Interpolate(RC *pRc, TLCLIPVTX *out, TLCLIPVTX *p1, TLCLIPVTX *p2,
                 int code, D3DVALUE num, D3DVALUE denom);


/*-------------------------------------------------------------------
Function Name: 	InterpolateColor
Description:    Interpolates RGBA values in the cliped vertex
Parameters:		
Information:    
Return: void
-------------------------------------------------------------------*/
__inline void InterpolateColor(TLCLIPVTX *result, TLCLIPVTX *p1,
                               TLCLIPVTX *p2, D3DVALUE num_denom )
{
  int r1, g1, b1, a1;
  int r2, g2, b2, a2;

  r1 = RGBA_GETRED(p1->diffuse);
  g1 = RGBA_GETGREEN(p1->diffuse);
  b1 = RGBA_GETBLUE(p1->diffuse);
  a1 = RGBA_GETALPHA(p1->diffuse);
  r2 = RGBA_GETRED(p2->diffuse);
  g2 = RGBA_GETGREEN(p2->diffuse);
  b2 = RGBA_GETBLUE(p2->diffuse);
  a2 = RGBA_GETALPHA(p2->diffuse);
  result->diffuse = RGBA_MAKE((WORD)(r1 + (r2 - r1) * num_denom),
      (WORD)(g1 + (g2 - g1) * num_denom),
      (WORD)(b1 + (b2 - b1) * num_denom),
      (WORD)(a1 + (a2 - a1) * num_denom));
} /* InterpolateColor */

/*-------------------------------------------------------------------
Function Name: 	InterpolateSpecular
Description:    Interpolates the specular value in the cliped vertex
Parameters:		
Information:    
Return: void
-------------------------------------------------------------------*/
__inline void InterpolateSpecular(TLCLIPVTX *result, TLCLIPVTX *p1,
                                  TLCLIPVTX *p2, D3DVALUE num_denom )
{
  int r1, g1, b1, a1;
  int r2, g2, b2, a2;

  r1 = RGBA_GETRED(p1->specular);
  g1 = RGBA_GETGREEN(p1->specular);
  b1 = RGBA_GETBLUE(p1->specular);
  a1 = RGBA_GETALPHA(p1->specular);
  r2 = RGBA_GETRED(p2->specular);
  g2 = RGBA_GETGREEN(p2->specular);
  b2 = RGBA_GETBLUE(p2->specular);
  a2 = RGBA_GETALPHA(p2->specular);
  result->specular = RGBA_MAKE((WORD)(r1 + (r2 - r1) * num_denom),
      (WORD)(g1 + (g2 - g1) * num_denom),
      (WORD)(b1 + (b2 - b1) * num_denom),
      (WORD)(a1 + (a2 - a1) * num_denom));
} /* InterpolateSpecular */ 


/*-------------------------------------------------------------------
This is quite a bit faster than the C version because the colors are 
in a packed RGBA format so what happenes is:
  - read the packed RGBA value into a register
  - 4x:
    - mask off the byte you need
    - shift to the lsb byte
    - store to the stack as a dword
    - read from the stack by the fpu and convert from integer to a float
    - do the interpolation computations
    - convert from float to integer (calls the dreaded _ftol() function)
    - load the dword from the stack into an integer register
  - pack the 4 values into an RGBA format

Since num_denum is between 0.0 and 1.0, the assembly version premultiplies 
it by 128 and stores it as an unsigned int.  So now num_denum is an int 
between 0 and 128.  We can't set the range to 0-256 because the intermediate 
component c2-c1 has the range -255 - +255.  That takes 9 bits so our multiplier
is 7 bits, so the result fits in a 16-bit word.  This is not quite as 
accurate as the floating point version but is much faster.
-------------------------------------------------------------------*/

// This path should be safe by any cpu doing T&L since we require 
// MMX already (in dumpspecularbuffer()).
__inline void InterpolateColorsAsmMMX( TLCLIPVTX *result, TLCLIPVTX *p1, TLCLIPVTX *p2, 
                                    int num_denom_x128 , DWORD dwInterpolate )
{
  __asm {
	mov			eax, [p1]
	movd		mm6, [num_denom_x128]		//             0               d*128	signed DWORD
	mov			edx, [p2]
	movd		mm0, [eax]TLCLIPVTX.diffuse	// 0   0   0   0   d1a d1r d1g d1b		4 unsigned bytes
	movd		mm1, [edx]TLCLIPVTX.diffuse	// 0   0   0   0   d2a d2r d2g d2b
	pxor		mm7, mm7					// 0   0   0   0   0   0   0   0
	packssdw	mm6, mm6					//     0       0       0       d*128	signed word
	movd		mm2, [eax]TLCLIPVTX.specular// 0   0   0   0   s1a s1r s1g s1b
	movd		mm3, [edx]TLCLIPVTX.specular// 0   0   0   0   s2a s2r s2g s2b
	punpcklbw	mm0, mm7					// 0   d1a 0   d1r 0   d1g 0   d1b
	punpcklbw	mm1, mm7					// 0   d2a 0   d2r 0   d2g 0   d2b
	punpcklwd	mm6, mm6					//     0       0       d*128   d*128	2 signed words
	mov			eax, [result]

	// expand the components from unsigned bytes to positive signed words
	// difference is a signed word, range is +/-255
	punpckldq	mm6, mm6					//     d*128   d*128   d*128   d*128	4 signed words
	psubsw		mm1, mm0					//   d2a-d1a d2r-d1r d2g-d1g d2b-d1b
	pmullw		mm1, mm6					//     (c2-c1) * (num_denom*128)
	// immediate result has the range range is now +/-32K, so divide by 128
	mov			ecx, [dwInterpolate]
	psraw		mm1, 7						//     (c2-c1) * num_denom
	// now the range is +/-255
	paddsw		mm0, mm1					// c1 + ((c2-c1) * num_denom)
	// result is a signed word with the range 0-255, can be treated as unsigned
	packuswb	mm0, mm0					// 0   0   0   0   da  db  dr  dg
	test		ecx, TLCLIP_INTERPOLATE_SPECULAR
	movd		[eax]TLCLIPVTX.diffuse, mm0

	jz			InterpolateColorsAsmMMX_SpecularDone
//InterpolateColorsAsmMMX_Specular:
	punpcklbw	mm2, mm7					// 0   s1a 0   s1r 0   s1g 0   s1b
	punpcklbw	mm3, mm7					// 0   s2a 0   s2r 0   s2g 0   s2b
	psubsw		mm3, mm2			        //   s2a-s1a s2r-s1r s2g-s1g s2b-s1b
	pmullw		mm3, mm6					//     (c2-c1) * (num_denom*128)
	psraw		mm3, 7						//     (c2-c1) * num_denom
	paddsw		mm2, mm3					// c1 + ((c2-c1) * num_denom)
	packuswb	mm2, mm2					// 0   0   0   0   sa  sb  sr  sg
	movd		[eax]TLCLIPVTX.specular, mm2
InterpolateColorsAsmMMX_SpecularDone:

	emms
  }
} /* InterpolateColorsAsmMMX */


/*-------------------------------------------------------------------
Function Name: 	TextureDiff
Description:    Used by InterpolateTexture
Parameters:		
Information:    
Return: FLOAT
-------------------------------------------------------------------*/
 __inline FLOAT TextureDiff(FLOAT fTb, FLOAT fTa, INT iMode)
{
  FLOAT fDiff1 = fTb - fTa;

  if (iMode == 0)
  {
    // Wrap not set, return plain difference.
    return fDiff1;
  }
  else
  {
    FLOAT fDiff2;

    // Wrap set, compute shortest distance of plain difference
    // and wrap difference.

    fDiff2 = fDiff1;
    if (FLOAT_LTZ(fDiff1))
    {
      fDiff2 += g_fOne;
    }
    else if (FLOAT_GTZ(fDiff1))
    {
      fDiff2 -= g_fOne;
    }
    if (ABSF(fDiff1) < ABSF(fDiff2))
    {
      return fDiff1;
    }
    else
    {
      return fDiff2;
    }
  }
} /* TextureDiff */

/*-------------------------------------------------------------------
Function Name: 	InterpolateTexture
Description:    Interpolates the Texture coords in the clipped 
				vertex
Parameters:		
Information:    
Return: D3DVALUE
-------------------------------------------------------------------*/
__inline D3DVALUE InterpolateTexture(D3DVALUE t1, D3DVALUE t2,
                                     D3DVALUE num_denom, DWORD bWrap)
{
  if (!bWrap)
  {
    return ((t2 - t1) * num_denom + t1);
  }
  else
  {
    D3DVALUE t = (TextureDiff(t2, t1, 1) * num_denom + t1);
    if (t > 1.0f) t -= 1.0f;
    return t;
  }
} /* InterpolateTexture */

//
// Clipping a triangle by a plane
//
// Returns number of vertices in the clipped triangle
//
/*-------------------------------------------------------------------
Function Name: 	ClipByPlane 
Description:    Clips a triangle by a plane
Parameters:		
Information:    
Return: int - Returns number of vertices in the clipped triangle
-------------------------------------------------------------------*/
int ClipByPlane( RC *pRc, TLCLIPVTX **inv, TLCLIPVTX **outv, TLVECTOR4 *plane,
                 DWORD dwClipFlag, int count )
{
  int i;
  int out_count = 0;
  TLCLIPVTX *curr, *prev;
  D3DVALUE curr_inside;
  D3DVALUE prev_inside;

  prev = inv[count-1];
  curr = *inv++;
  prev_inside = prev->hx*plane->x + prev->hy*plane->y +
      prev->hz*plane->z + prev->hw*plane->w;
  for (i = count; i; i--)
  {
    curr_inside = curr->hx*plane->x + curr->hy*plane->y +
        curr->hz*plane->z + curr->hw*plane->w;
    // We interpolate always from the inside vertex to the outside vertex
    // to reduce precision problems
    if (FLOAT_LTZ(prev_inside))
    { // first point is outside
      if (FLOAT_GEZ(curr_inside))
      { // second point is inside
        // Find intersection and insert in into the output buffer
        outv[out_count] = GET_NEW_CLIP_VERTEX;
        Interpolate( pRc, outv[out_count],
            curr, prev,
            (prev->clip_code & CLIPPED_ENABLE) | dwClipFlag,
            curr_inside, curr_inside - prev_inside);
        out_count++;
      }
    }
    else
    { // first point is inside - put it to the output buffer first
      outv[out_count++] = prev;
      if (FLOAT_LTZ(curr_inside))
      { // second point is outside
        // Find intersection and put it to the output buffer
        outv[out_count] = GET_NEW_CLIP_VERTEX;
        Interpolate( pRc, outv[out_count],
            prev, curr,
            dwClipFlag,
            prev_inside, prev_inside - curr_inside);
        out_count++;
      }
    }
    prev = curr;
    curr = *inv++;
    prev_inside = curr_inside;
  }
  return out_count;
} /* ClipByPlane */

/*-------------------------------------------------------------------
Function Name: 	ClipLineByPlane 
Description:    Clips a line by a plane
Parameters:		
Information:    
Return: int - Returns 1 if the line is outside the frustum, 
              0 otherwise 
-------------------------------------------------------------------*/
int ClipLineByPlane(RC *pRc, TLCLIPTRIANGLE *line, TLVECTOR4 *plane,
                    DWORD dwClipBit)
{
  D3DVALUE in1, in2;
  TLCLIPVTX outv;
  in1 = line->v[0]->hx * plane->x +
      line->v[0]->hy * plane->y +
      line->v[0]->hz * plane->z +
      line->v[0]->hw * plane->w;
  in2 = line->v[1]->hx * plane->x +
      line->v[1]->hy * plane->y +
      line->v[1]->hz * plane->z +
      line->v[1]->hw * plane->w;
  if (in1 < 0)
  {
    if (in2 < 0)
      return 1;
    Interpolate( pRc, &outv, line->v[0], line->v[1],
        dwClipBit, in1, in1 - in2);
    *line->v[0] = outv;
  }
  else
  {
    if (in2 < 0)
    {
      Interpolate( pRc, &outv, line->v[0], line->v[1],
          dwClipBit, in1, in1 - in2);
      *line->v[1] = outv;
    }
  }
  return 0;
} /* ClipLineByPlane */

/*-------------------------------------------------------------------
Function Name: 	ComputeScreenCoordinates 
Description:    Calculate the screen coords for any new vertices
                introduced into the polygon.
Parameters:		
Information:    
Return: void
-------------------------------------------------------------------*/
void ComputeScreenCoordinates(const TLVIEWPORTDATA VData,
                              TLCLIPVTX **inv, DWORD count)
{
  DWORD i;

  for (i = 0; i < count; i++)
  {
    TLCLIPVTX *p;
    p = inv[i];

    /*
    * Catch any vertices that need screen co-ordinates generated.
    * There are two possibilities
    *      1) Vertices generated during interpolation
    *      2) Vertices marked for clipping by the transform but
    *              not clipped here due to the finite precision
    *              of the floating point unit.
    */

    if (p->clip_code & ~CLIPPED_ENABLE)
    {
      D3DVALUE w;

      w = D3DVAL(1.0)/p->hw;
      switch ((int)p->clip_code & (CLIPPED_LEFT|CLIPPED_RIGHT))
      {
      case CLIPPED_LEFT:  p->sx = VData.minXgb; break;
      case CLIPPED_RIGHT: p->sx = VData.maxXgb; break;
      default:
        p->sx = p->hx * w * VData.scaleX + VData.offsetX;
        if (p->sx < VData.minXgb)
          p->sx = VData.minXgb;
        if (p->sx > VData.maxXgb)
          p->sx = VData.maxXgb;
      }
      switch ((int)p->clip_code & (CLIPPED_TOP|CLIPPED_BOTTOM))
      {
      case CLIPPED_BOTTOM: p->sy = VData.maxYgb; break;
      case CLIPPED_TOP:    p->sy = VData.minYgb; break;
      default:
        p->sy = p->hy * w * VData.scaleY + VData.offsetY;
        if (p->sy < VData.minYgb)
          p->sy = VData.minYgb;
        if (p->sy > VData.maxYgb)
          p->sy = VData.maxYgb;
      }
      p->sz = p->hz * w * VData.scaleZ + VData.offsetZ;
      p->rhw = w;
    }
  }
} /* ComputeScreenCoordinates */

/*-------------------------------------------------------------------
Function Name: 	Interpolate
Description:    Master Interpolatorfor all Vertex sub components
Parameters:		
Information:    
Return: void
-------------------------------------------------------------------*/
void Interpolate(RC *pRc, TLCLIPVTX *out, TLCLIPVTX *p1, TLCLIPVTX *p2,
                 int code, D3DVALUE num, D3DVALUE denom)
{
  DWORD dwInterpolate = pRc->tl.clipping.dwInterpolate;
  D3DVALUE num_denom = num / denom;
  DWORD i, j;

  out->clip_code = (((int)p1->clip_code & (int)p2->clip_code) & ~CLIPPED_ENABLE) | code;
  out->hx = p1->hx + (p2->hx - p1->hx) * num_denom;
  out->hy = p1->hy + (p2->hy - p1->hy) * num_denom;
  out->hz = p1->hz + (p2->hz - p1->hz) * num_denom;
  out->hw = p1->hw + (p2->hw - p1->hw) * num_denom;

  if( pRc->tl.KniRC.RfBits & BIT_RC_VERTEX_FOG )
    out->vfog = p1->vfog + (p2->vfog - p1->vfog) * num_denom;

  /*
  * Interpolate any other color model or quality dependent values.
  */
  // Any cpu we allow to do T&L should be okay with this path
  InterpolateColorsAsmMMX(out, p1, p2, FTOI(num_denom*128.0f), dwInterpolate );

  if (dwInterpolate & TLCLIP_INTERPOLATE_TEXTURE)
  {
    // Assume that D3DRENDERSTATE_WRAPi are sequential
    D3DVALUE *pTexture1 = p1->tex;
    D3DVALUE *pTexture2 = p2->tex;
    D3DVALUE *pTexture = out->tex;

    for ( i = 0; i < pRc->tl.InFVF.dwNumTexCoords; i++)
    {
      //DWORD wrapState = ((ReferenceRasterizer *)this)->GetRenderState()[D3DRENDERSTATE_WRAP0 + i];

      // BobJ, 10/08/1999 I know this is not accurate because it does NOT check
      // each Texture's real WRAP renderstate.  I'm unsure of how to get 
      // access to the TS[n].state table.
	  //       DWORD wrapState =  TS[i].wrap;
	  // Matt M.  Make sure we access the proper wrap coordinates
	  DWORD wrapState = TS[TS[i].texCoordIndex & 0xffff].wrap;
      DWORD n = (DWORD)(pRc->tl.dwTexCoordSize[i] >> 2);
      DWORD dwWrapBit = 1;
      for ( j=0; j < n; j++)
      {
        *pTexture = InterpolateTexture(*pTexture1, *pTexture2, num_denom, wrapState & dwWrapBit);
        dwWrapBit <<= 1;
        pTexture++;
        pTexture1++;
        pTexture2++;
      }
    }
  }
} /* Interpolate */

//------------------------------------------------------------------------------
// Functions for clipping by frustum window
//
#define __CLIP_NAME ClipLeft
#define __CLIP_LINE_NAME ClipLineLeft
#define __CLIP_FLAG CLIPPED_LEFT
#define __CLIP_COORD hx
#include "clipfuncs.h"

#define __CLIP_NAME ClipRight
#define __CLIP_LINE_NAME ClipLineRight
#define __CLIP_W
#define __CLIP_FLAG CLIPPED_RIGHT
#define __CLIP_COORD hx
#include "clipfuncs.h"

#define __CLIP_NAME ClipBottom
#define __CLIP_LINE_NAME ClipLineBottom
#define __CLIP_FLAG CLIPPED_BOTTOM
#define __CLIP_COORD hy
#include "clipfuncs.h"

#define __CLIP_NAME ClipTop
#define __CLIP_LINE_NAME ClipLineTop
#define __CLIP_W
#define __CLIP_FLAG CLIPPED_TOP
#define __CLIP_COORD hy
#include "clipfuncs.h"

#define __CLIP_NAME ClipBack
#define __CLIP_LINE_NAME ClipLineBack
#define __CLIP_W
#define __CLIP_FLAG CLIPPED_BACK
#define __CLIP_COORD hz
#include "clipfuncs.h"

#define __CLIP_NAME ClipFront
#define __CLIP_LINE_NAME ClipLineFront
#define __CLIP_FLAG CLIPPED_FRONT
#define __CLIP_COORD hz
#include "clipfuncs.h"
//------------------------------------------------------------------------------
// Functions for guard band clipping
//
#define __CLIP_GUARDBAND
#define __CLIP_NAME ClipLeftGB
#define __CLIP_LINE_NAME ClipLineLeftGB
#define __CLIP_FLAG CLIPPED_LEFT
#define __CLIP_COORD hx
#define __CLIP_SIGN -
#define __CLIP_GBCOEF Kgbx1
#include "clipfuncs.h"

#define __CLIP_NAME ClipRightGB
#define __CLIP_LINE_NAME ClipLineRightGB
#define __CLIP_FLAG CLIPPED_RIGHT
#define __CLIP_COORD hx
#define __CLIP_GBCOEF Kgbx2
#define __CLIP_SIGN +
#include "clipfuncs.h"

#define __CLIP_NAME ClipBottomGB
#define __CLIP_LINE_NAME ClipLineBottomGB
#define __CLIP_FLAG CLIPPED_BOTTOM
#define __CLIP_COORD hy
#define __CLIP_SIGN -
#define __CLIP_GBCOEF Kgby1
#include "clipfuncs.h"

#define __CLIP_NAME ClipTopGB
#define __CLIP_LINE_NAME ClipLineTopGB
#define __CLIP_FLAG CLIPPED_TOP
#define __CLIP_COORD hy
#define __CLIP_GBCOEF Kgby2
#define __CLIP_SIGN +
#include "clipfuncs.h"

#undef __CLIP_GUARDBAND

/*-------------------------------------------------------------------
Function Name: 	ComputeClipCodeGB
Description:    
Parameters:		
Information:    
Return: DWORD
-------------------------------------------------------------------*/
__inline DWORD ComputeClipCodeGB(const TLVIEWPORTDATA VData, TLCLIPVTX *p)
{
  DWORD clip_code = 0;
  if (p->hx < p->hw * VData.Kgbx1)
    clip_code |= TLCLIPGB_LEFT;
  if (p->hx > p->hw * VData.Kgbx2)
    clip_code |= TLCLIPGB_RIGHT;
  if (p->hy < p->hw * VData.Kgby1)
    clip_code |= TLCLIPGB_BOTTOM;
  if (p->hy > p->hw * VData.Kgby2)
    clip_code |= TLCLIPGB_TOP;
  if (p->hz > p->hw)
    clip_code |= TLCLIP_BACK;
  p->clip_code = (p->clip_code & (CLIPPED_ENABLE | CLIPPED_FRONT)) | clip_code;
  return clip_code;
} /* ComputeClipCodeGB */ 

/*-------------------------------------------------------------------
Function Name: 	ComputeClipCode
Description:    
Parameters:		
Information:    
Return: DWORD
-------------------------------------------------------------------*/
__inline DWORD ComputeClipCode(TLCLIPVTX *p)
{
  DWORD clip_code = 0;
  if (FLOAT_LTZ(p->hx))
    clip_code |= TLCLIP_LEFT;
  if (p->hx > p->hw)
    clip_code |= TLCLIP_RIGHT;
  if (FLOAT_LTZ(p->hy))
    clip_code |= TLCLIP_BOTTOM;
  if (p->hy > p->hw)
    clip_code |= TLCLIP_TOP;
  if (p->hz > p->hw)
    clip_code |= TLCLIP_BACK;
  p->clip_code = (p->clip_code & (CLIPPED_ENABLE | CLIPPED_FRONT)) | clip_code;
  return clip_code;
} /* ComputeClipCode */

/*-------------------------------------------------------------------
Function Name: 	UpdateClippingData
Description:    Updates clipping data used by ProcessVertices
Parameters:		
Information:    
Return: HRESULT
-------------------------------------------------------------------*/
HRESULT UpdateClippingData(RC *pRc, DWORD dwClipPlanesEnable)
{
  HRESULT hr = D3D_OK;
  DWORD i;

  pRc->tl.dwTLState &= ~TLPV_USERCLIPPLANES;
  // Update the user defined clip plane data
  for( i=0; i<TLMAX_USER_CLIPPLANES; i++ )
  {
    // Figure out if it is active
    pRc->tl.xfmUserClipPlanes[i].bActive = (BOOL)(dwClipPlanesEnable & 0x1);
    dwClipPlanesEnable >>= 1;

    // If it is active, transform it into eye-space using the
    // view transform. The clip planes are defined in the
    // world space.
    if( pRc->tl.xfmUserClipPlanes[i].bActive )
    {
      pRc->tl.dwTLState |= TLPV_USERCLIPPLANES;
      XformPlaneBy4x4Transposed( &pRc->tl.UserClipPlanes[i],
                                 &pRc->tl.TransformData.m_VPSInv,
                                 &pRc->tl.xfmUserClipPlanes[i].plane );
    }
  }
  pRc->tl.dwDirtyFlags &= ~(TLPV_DIRTY_CLIPPLANES);
  return hr;

} /* UpdateClippingData */ 

/*-------------------------------------------------------------------
Function Name: 	ClipSingleTriangleTLBN
Description:    
Parameters:		
Information:    
Return: DWORD
-------------------------------------------------------------------*/
DWORD ClipSingleTriangleTLBN(RC *pRc, TLCLIPTRIANGLE *tri, TLCLIPVTX ***clipVertexPointer)
{
  int accept;
  DWORD i;
  int j;
  DWORD count;
  TLCLIPVTX **inv;
  TLCLIPVTX **outv;
  //TLCLIPVTX *p;
  ULONG_PTR swapv;
  DWORD dwClipBit;
  DWORD dwClippedBit;
#ifdef TNL_PROFILE
  DWORD actualAccept = 0L;
#endif

  if ( pRc->shadeMode == D3DSHADE_FLAT )
  {
    // It is easier to set all vertices to the same color here
    tri->v[1]->diffuse  = tri->v[2]->diffuse  = tri->v[0]->diffuse;
    tri->v[1]->specular = tri->v[2]->specular = tri->v[0]->specular;
	tri->v[1]->vfog     = tri->v[2]->vfog     = tri->v[0]->vfog;
  }
  accept = (tri->v[0]->clip_code | tri->v[1]->clip_code | tri->v[2]->clip_code);

  inv = tri->v;
  count = 3;
  outv = pRc->tl.clipping.clip_vbuf1;
  // need these for flat shading
//  pRc->tl.clipping.clip_color = tri->v[0]->diffuse;
//  pRc->tl.clipping.clip_specular = tri->v[0]->specular;

  /*
  * XXX assbpxumes sizeof(void*) == sizeof(unsigned long)
  */
  {
    ULONG_PTR tmp1;
    ULONG_PTR tmp2;

    tmp1 = (ULONG_PTR)pRc->tl.clipping.clip_vbuf1;
    tmp2 = (ULONG_PTR)pRc->tl.clipping.clip_vbuf2;

    swapv = tmp1 + tmp2;
  }
  pRc->tl.clipping.clip_vertices_used = 0;

#define CLIP_SWAP(inv, outv)     \
  inv = outv;             \
  outv = (TLCLIPVTX**) (swapv - (ULONG_PTR) outv)

  if (accept & TLCLIP_FRONT)
  {
    count = ClipFront( pRc, inv, outv, count );
#ifdef TNL_PROFILE
    actualAccept |= TLCLIP_FRONT;
#endif
    if (count < 3)
      goto out_of_here;
    CLIP_SWAP(inv, outv);
  }

  if (pRc->tl.dwTLState & TLPV_GUARDBAND)
  {
    // If there was clipping by the front plane it is better to
    // compute clip code for new vertices and re-compute accept.
    // Otherwise we will try to clip by sides when it is not necessary
    if (accept & TLCLIP_FRONT)
    {
      accept = 0;
      for (i = 0; i < count; i++)
      {
        TLCLIPVTX *p;
        p = inv[i];
        if (p->clip_code & CLIPPED_FRONT)
          accept |= ComputeClipCodeGB(pRc->tl.ViewData, p);
        else
          accept |= p->clip_code;
      }
    }
    if (accept & TLCLIP_BACK)
    {
      count = ClipBack( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIP_BACK;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIPGB_LEFT)
    {
      count = ClipLeftGB( pRc, inv, outv, count );
#ifdef TNL_PROFILE
   actualAccept |= TLCLIPGB_LEFT;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIPGB_RIGHT)
    {
      count = ClipRightGB( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIPGB_RIGHT;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIPGB_BOTTOM)
    {
      count = ClipBottomGB( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIPGB_BOTTOM;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIPGB_TOP)
    {
      count = ClipTopGB( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIPGB_TOP;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
  }
  else
  {
    // If there was clipping by the front plane it is better to
    // compute clip code for new vertices and re-compute accept.
    // Otherwise we will try to clip by sides when it is not necessary
    if (accept & TLCLIP_FRONT)
    {
      accept = 0;
      for (i = 0; i < count; i++)
      {
        TLCLIPVTX *p;
        p = inv[i];
        if (p->clip_code & (CLIPPED_FRONT))
          accept |= ComputeClipCode( p );
        else
          accept |= p->clip_code;
      }
    }
    
    if (accept & TLCLIP_BACK)
    {
      count = ClipBack( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIP_BACK;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIP_LEFT)
    {
      count = ClipLeft( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIP_LEFT;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIP_RIGHT)
    {
      count = ClipRight( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIP_RIGHT;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIP_BOTTOM)
    {
      count = ClipBottom( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIP_BOTTOM;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIP_TOP)
    {
      count = ClipTop( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIP_TOP;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
  }

  dwClipBit = TLCLIP_USERCLIPPLANE0;
  dwClippedBit = CLIPPED_USERCLIPPLANE0;
  // User Clip Planes
  for( j=0; j<TLMAX_USER_CLIPPLANES; j++)
  {
    if (accept & dwClipBit)
    {
      count = ClipByPlane( pRc, inv, outv, &pRc->tl.xfmUserClipPlanes[j].plane,
          dwClippedBit, count);
#ifdef TNL_PROFILE
      actualAccept |= (TLCLIP_USERCLIPPLANE0 << j);
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    dwClipBit <<= 1;
    dwClippedBit <<= 1;
  }

#undef CLIP_SWAP

  *clipVertexPointer = inv;
  pRc->tl.clipping.current_vbuf = inv;
#ifdef TNL_PROFILE
  TriClipStats(actualAccept);
#endif
  return count;

out_of_here:

  *clipVertexPointer = NULL;
#ifdef TNL_PROFILE
  TriClipStats(actualAccept);
#endif
  return 0;

} /* ClipSingleTriangleTLBN */

/*-------------------------------------------------------------------
Function Name: 	ClipSingleTriangle
Description:    
Parameters:		
Information:    
Return: DWORD
-------------------------------------------------------------------*/
DWORD ClipSingleTriangle(RC *pRc, TLCLIPTRIANGLE *tri, TLCLIPVTX ***clipVertexPointer)
{
  int accept;
  DWORD i;
  int j;
  DWORD count;
  TLCLIPVTX **inv;
  TLCLIPVTX **outv;
  //TLCLIPVTX *p;
  ULONG_PTR swapv;
//  D3DCOLOR diffuse1;          // Original colors
//  D3DCOLOR specular1;
//  D3DCOLOR diffuse2;
//  D3DCOLOR specular2;
  DWORD dwClipBit;
  DWORD dwClippedBit;
#ifdef TNL_PROFILE
  DWORD actualAccept = 0L;
#endif

  if ( pRc->shadeMode == D3DSHADE_FLAT )
  {
    // It is easier to set all vertices to the same color here
    tri->v[1]->diffuse  = tri->v[2]->diffuse  = tri->v[0]->diffuse;
    tri->v[1]->specular = tri->v[2]->specular = tri->v[0]->specular;
  }
  accept = (tri->v[0]->clip_code | tri->v[1]->clip_code | tri->v[2]->clip_code);

  inv = tri->v;
  count = 3;
  outv = pRc->tl.clipping.clip_vbuf1;
//  pRc->tl.clipping.clip_color = tri->v[0]->diffuse;
//  pRc->tl.clipping.clip_specular = tri->v[0]->specular;

  /*
  * XXX assbpxumes sizeof(void*) == sizeof(unsigned long)
  */
  {
    ULONG_PTR tmp1;
    ULONG_PTR tmp2;

    tmp1 = (ULONG_PTR)pRc->tl.clipping.clip_vbuf1;
    tmp2 = (ULONG_PTR)pRc->tl.clipping.clip_vbuf2;

    swapv = tmp1 + tmp2;
  }
  pRc->tl.clipping.clip_vertices_used = 0;

#define CLIP_SWAP(inv, outv)     \
  inv = outv;             \
  outv = (TLCLIPVTX**) (swapv - (ULONG_PTR) outv)

  if (accept & TLCLIP_FRONT)
  {
    count = ClipFront( pRc, inv, outv, count );
#ifdef TNL_PROFILE
   actualAccept |= TLCLIP_FRONT;
#endif
    if (count < 3)
      goto out_of_here;
    CLIP_SWAP(inv, outv);
  }
  if (pRc->tl.dwTLState & TLPV_GUARDBAND)
  {
    // If there was clipping by the front plane it is better to
    // compute clip code for new vertices and re-compute accept.
    // Otherwise we will try to clip by sides when it is not necessary
    if (accept & TLCLIP_FRONT)
    {
      accept = 0;
      for (i = 0; i < count; i++)
      {
        TLCLIPVTX *p;
        p = inv[i];
        if (p->clip_code & CLIPPED_FRONT)
          accept |= ComputeClipCodeGB(pRc->tl.ViewData, p);
        else
          accept |= p->clip_code;
      }
    }
    if (accept & TLCLIP_BACK)
    {
      count = ClipBack( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIP_BACK;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIPGB_LEFT)
    {
      count = ClipLeftGB( pRc, inv, outv, count );
#ifdef TNL_PROFILE
   actualAccept |= TLCLIPGB_LEFT;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIPGB_RIGHT)
    {
      count = ClipRightGB( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIPGB_RIGHT;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIPGB_BOTTOM)
    {
      count = ClipBottomGB( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIPGB_BOTTOM;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIPGB_TOP)
    {
      count = ClipTopGB( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIPGB_TOP;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
  }
  else
  {
    // If there was clipping by the front plane it is better to
    // compute clip code for new vertices and re-compute accept.
    // Otherwise we will try to clip by sides when it is not necessary
    if (accept & TLCLIP_FRONT)
    {
      accept = 0;
      for (i = 0; i < count; i++)
      {
        TLCLIPVTX *p;
        p = inv[i];
        if (p->clip_code & (CLIPPED_FRONT))
          accept |= ComputeClipCode( p );
        else
          accept |= p->clip_code;
      }
    }
    
    if (accept & TLCLIP_BACK)
    {
      count = ClipBack( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIP_BACK;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIP_LEFT)
    {
      count = ClipLeft( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIP_LEFT;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIP_RIGHT)
    {
      count = ClipRight( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIP_RIGHT;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIP_BOTTOM)
    {
      count = ClipBottom( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIP_BOTTOM;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    if (accept & TLCLIP_TOP)
    {
      count = ClipTop( pRc, inv, outv, count );
#ifdef TNL_PROFILE
      actualAccept |= TLCLIP_TOP;
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
  }

  dwClipBit = TLCLIP_USERCLIPPLANE0;
  dwClippedBit = CLIPPED_USERCLIPPLANE0;
  // User Clip Planes
  for( j=0; j<TLMAX_USER_CLIPPLANES; j++)
  {
    if (accept & dwClipBit)
    {
      count = ClipByPlane( pRc, inv, outv, &pRc->tl.xfmUserClipPlanes[j].plane,
          dwClippedBit, count);
#ifdef TNL_PROFILE
      actualAccept |= (TLCLIP_USERCLIPPLANE0 << j);
#endif
      if (count < 3)
        goto out_of_here;
      CLIP_SWAP(inv, outv);
    }
    dwClipBit <<= 1;
    dwClippedBit <<= 1;
  }

#undef CLIP_SWAP

  ComputeScreenCoordinates( pRc->tl.ViewData, inv, count );

  *clipVertexPointer = inv;
  pRc->tl.clipping.current_vbuf = inv;
#ifdef TNL_PROFILE
  TriClipStats(actualAccept);
#endif
  return count;

out_of_here:

  *clipVertexPointer = NULL;
#ifdef TNL_PROFILE
  TriClipStats(actualAccept);
#endif
  return 0;

} /* ClipSingleTriangle */

/*-------------------------------------------------------------------
Function Name: 	ClipSingleLine
Description:    
Parameters:		
Information:    
Return: DWORD
-------------------------------------------------------------------*/
DWORD ClipSingleLine( RC *pRc, TLCLIPTRIANGLE *line )
{
  int         accept;
  int         j;
  DWORD dwClipBit;
  DWORD dwClippedBit;

  accept = (line->v[0]->clip_code | line->v[1]->clip_code);

//  pRc->tl.clipping.clip_color = line->v[0]->diffuse;
//  pRc->tl.clipping.clip_specular = line->v[0]->specular;

  if (accept & D3DCLIP_FRONT)
    if (ClipLineFront(pRc, line))
      goto out_of_here;

  if (pRc->tl.dwTLState & TLPV_GUARDBAND)
  {
    // If there was clipping by the front plane it is better to
    // compute clip code for new vertices and re-compute accept.
    // Otherwise we will try to clip by sides when it is not necessary
    if (accept & D3DCLIP_FRONT)
    {
      TLCLIPVTX * p;
      accept = 0;
      p = line->v[0];
      if (p->clip_code & CLIPPED_FRONT)
        accept |= ComputeClipCodeGB(pRc->tl.ViewData, p);
      else
        accept |= p->clip_code;
      p = line->v[1];
      if (p->clip_code & CLIPPED_FRONT)
        accept |= ComputeClipCodeGB(pRc->tl.ViewData, p);
      else
        accept |= p->clip_code;
    }
    if (accept & D3DCLIP_BACK)
      if (ClipLineBack( pRc, line ))
        goto out_of_here;

    if (accept & TLCLIPGB_LEFT)
      if (ClipLineLeftGB( pRc, line ))
        goto out_of_here;

    if (accept & TLCLIPGB_RIGHT)
      if (ClipLineRightGB( pRc, line ))
        goto out_of_here;

    if (accept & TLCLIPGB_TOP)
      if (ClipLineTopGB( pRc, line ))
        goto out_of_here;

    if (accept & TLCLIPGB_BOTTOM)
      if (ClipLineBottomGB( pRc, line ))
        goto out_of_here;
  }
  else
  {
    // If there was clipping by the front plane it is better to
    // compute clip code for new vertices and re-compute accept.
    // Otherwise we will try to clip by sides when it is not necessary
    if (accept & D3DCLIP_FRONT)
    {
      TLCLIPVTX * p;
      accept = 0;
      p = line->v[0];
      if (p->clip_code & CLIPPED_FRONT)
        accept |= ComputeClipCode( p );
      else
        accept |= p->clip_code;
      p = line->v[1];
      if (p->clip_code & CLIPPED_FRONT)
        accept |= ComputeClipCode( p );
      else
        accept |= p->clip_code;
    }
    if (accept & D3DCLIP_BACK)
      if (ClipLineBack( pRc, line ))
        goto out_of_here;

    if (accept & D3DCLIP_LEFT)
      if (ClipLineLeft( pRc, line ))
        goto out_of_here;

    if (accept & D3DCLIP_RIGHT)
      if (ClipLineRight( pRc, line ))
        goto out_of_here;

    if (accept & D3DCLIP_TOP)
      if (ClipLineTop( pRc, line ))
        goto out_of_here;

    if (accept & D3DCLIP_BOTTOM)
      if (ClipLineBottom( pRc, line ))
        goto out_of_here;
  }

  // User Clip Planes
  dwClipBit = TLCLIP_USERCLIPPLANE0;
  dwClippedBit = CLIPPED_USERCLIPPLANE0;
  for( j=0; j<TLMAX_USER_CLIPPLANES; j++)
  {
    if (accept & dwClipBit)
    {
      if( ClipLineByPlane( pRc, line, &pRc->tl.xfmUserClipPlanes[j].plane,
          dwClippedBit ))
        goto out_of_here;
    }
    dwClipBit <<= 1;
    dwClippedBit <<= 1;
  }

  ComputeScreenCoordinates(pRc->tl.ViewData, line->v, 2);

  return 1;
out_of_here:
  return 0;
} /* ClipSingleLine */


#endif
#endif


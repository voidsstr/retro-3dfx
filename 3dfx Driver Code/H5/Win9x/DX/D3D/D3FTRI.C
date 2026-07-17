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
** $Log: 
**  2    3dfx      1.0.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 1     6/02/99 6:40a Michael
** Branch from H3
** 
** 11    12/09/98 7:23a Russ
** NT5 D3D changes for Banshee
**
** 10    10/16/98 4:33p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
**
** 9     8/04/98 10:13a Adrians
** Added backface removal to wireframe and point triangle fill modes.
**
** 8     7/30/98 4:33p Adrians
** Fix for DX5 line drawing.
**
** 7     7/24/98 1:37p Hohn
 *
 * 6     10/14/97 11:22a Adrians
 * Added Miriam's 4M texture support.  Added build environment for H3.
 *
 * 5     10/10/97 10:42a Adrians
 * Removed all references to DIRECTX5.
 *
 * 4     10/02/97 8:38p Adrians
 * Include init code into build. Enable Write Combining.  Inline system
 * functions.  Change optimisations.  Some code tidy up.
 *
 * 3     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/

#include "precomp.h"

#ifndef WINNT
#include "d3dhal.h"
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h"
#include "fifomgr.h"
#endif

//----------------------------
//
//  draw points at each vertex
//
//----------------------------
void __stdcall fpFillTrianglePt( RC *pRc, WORD count, LPD3DTRIANGLE tri, LPD3DTLVERTEX vertices )
{
  D3DTLVERTEX   *pA, *pB, *pC;
  int           sign;

  //---------------
  //
  // every triangle
  //
  //---------------
  for (; count > 0; --count)
  {
    pA    = &vertices[tri->v1];
    pB    = &vertices[tri->v2];
    pC    = &vertices[tri->v3];

    // draw each vertex
    ((float*)&sign)[0] = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) -
                         ((pB->sx - pC->sx) * (pA->sy - pB->sy));

    sign &= 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      fpDrawPoint(pA->sx, pA->sy, pA, pRc, pA->color);
      fpDrawPoint(pB->sx, pB->sy, pB, pRc, pA->color);
      fpDrawPoint(pC->sx, pC->sy, pC, pRc, pA->color);
    }

    ++tri;

  } // every triangle
} // floating point triangle


//-------------------------------------------------------------------
//----------------------------
//
//  draw points at each vertex
//
//----------------------------
void __stdcall fpFillTriangleLine( RC *pRc, WORD count, LPD3DTRIANGLE tri, LPD3DTLVERTEX vertices )
{
  D3DTLVERTEX   *pA, *pB, *pC;
  int           sign;

  //---------------
  //
  // every triangle
  //
  //---------------
  for (; count > 0; --count)
  {
    pA    = &vertices[tri->v1];
    pB    = &vertices[tri->v2];
    pC    = &vertices[tri->v3];

    ((float*)&sign)[0] = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) -
                         ((pB->sx - pC->sx) * (pA->sy - pB->sy));

    sign &= 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      // draw each edge
      if (tri->wFlags & D3DTRIFLAG_EDGEENABLE1)
      {
        fpDrawLine( pA, pB, pRc, pA->color );
      }

      if (tri->wFlags & D3DTRIFLAG_EDGEENABLE2)
      {
        fpDrawLine( pB, pC, pRc, pA->color );
      }

      if (tri->wFlags & D3DTRIFLAG_EDGEENABLE3)
      {
        fpDrawLine( pC, pA, pRc, pA->color );
      }
    }

    ++tri;

  } // every triangle
} // floating point triangle


//-------------------------------------------------------------------

#ifndef WINNT
// fill indexed draw primitive triangles
void __stdcall fpFillIDrawTriangle( RC *pRc, LPD3DHAL_DRAWONEINDEXEDPRIMITIVEDATA lpdoipd )
{
  WORD             *tri = lpdoipd->lpwIndices;
  int               count = lpdoipd->dwNumIndices, sign;
  LPD3DTLVERTEX     vertices = (LPD3DTLVERTEX)lpdoipd->lpvVertices;
  D3DTLVERTEX      *pA, *pB, *pC;
  int               index = 0;
  DWORD             fillMode = pRc->fillMode;
  unsigned int      grab = 1;

  switch (lpdoipd->PrimitiveType)
  {
    case D3DPT_TRIANGLELIST:
      for (; count > 0; count-=3)
      {
        pA = vertices + tri[index++];
        pB = vertices + tri[index++];
        pC = vertices + tri[index++];

        ((float*)&sign)[0] = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) -
                             ((pB->sx - pC->sx) * (pA->sy - pB->sy));

        sign &= 0x80000000;

        if ((sign ^ pRc->cullMask) != 0x80000000)
        {
          if (fillMode == D3DFILL_WIREFRAME)
          {
            fpDrawLine( pA, pB, pRc, pA->color );
            fpDrawLine( pB, pC, pRc, pA->color );
            fpDrawLine( pC, pA, pRc, pA->color );
          }
          else
          {
            fpDrawPoint( pA->sx, pA->sy, pA, pRc, pA->color );
            fpDrawPoint( pB->sx, pB->sy, pB, pRc, pA->color );
            fpDrawPoint( pC->sx, pC->sy, pC, pRc, pA->color );
          }
        }
      } // every triangle
      break;

    case D3DPT_TRIANGLESTRIP:
      count -=2 ;
      for (; count > 0; --count)
      {
        if (grab)
        {
          pA = vertices + tri[index++];
          pB = vertices + tri[index++];
          pC = vertices + tri[index++];
        }
        else
        {
          pA = pC;
          pB = pB;
          pC = vertices + tri[index--];
        }
        grab ^= 1;

        ((float*)&sign)[0] = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) -
                             ((pB->sx - pC->sx) * (pA->sy - pB->sy));

        sign &= 0x80000000;

        if ((sign ^ pRc->cullMask) != 0x80000000)
        {
          if (fillMode == D3DFILL_WIREFRAME)
          {
            fpDrawLine( pA, pB, pRc, pA->color );
            fpDrawLine( pB, pC, pRc, pA->color );
            fpDrawLine( pC, pA, pRc, pA->color );
          }
          else
          {
            fpDrawPoint( pA->sx, pA->sy, pA, pRc, pA->color );
            fpDrawPoint( pB->sx, pB->sy, pB, pRc, pA->color );
            fpDrawPoint( pC->sx, pC->sy, pC, pRc, pA->color );
          }
        }
      } // every triangle
      break;

  case D3DPT_TRIANGLEFAN:
      pA = vertices + tri[index++];
      pC = vertices + tri[index++];
      count -=2 ;
      for (; count > 0; --count)
      {
        pB = pC;
        pC = vertices + tri[index++];

        ((float*)&sign)[0] = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) -
                             ((pB->sx - pC->sx) * (pA->sy - pB->sy));

        sign &= 0x80000000;

        if ((sign ^ pRc->cullMask) != 0x80000000)
        {
          if (fillMode == D3DFILL_WIREFRAME)
          {
            fpDrawLine( pA, pB, pRc, pA->color );
            fpDrawLine( pB, pC, pRc, pA->color );
            fpDrawLine( pC, pA, pRc, pA->color );
          }
          else
          {
            fpDrawPoint( pA->sx, pA->sy, pA, pRc, pA->color );
            fpDrawPoint( pB->sx, pB->sy, pB, pRc, pA->color );
            fpDrawPoint( pC->sx, pC->sy, pC, pRc, pA->color );
          }
        }
      } // every triangle
      break;

  } // switch
}

//-------------------------------------------------------------------

// fill indexed draw primitive triangles
void __stdcall fpFillDrawTriangle( RC *pRc, LPD3DHAL_DRAWONEPRIMITIVEDATA lpdopd )
{
  int               count = lpdopd->dwNumVertices, sign;
  LPD3DTLVERTEX     vertices = (LPD3DTLVERTEX)lpdopd->lpvVertices;
  D3DTLVERTEX      *pA, *pB, *pC;
  DWORD             fillMode = pRc->fillMode;
  unsigned int      grab = 1;

  switch (lpdopd->PrimitiveType)
  {
    case D3DPT_TRIANGLELIST:
      for (; count > 0; count-=3)
      {
        pA = vertices++;
        pB = vertices++;
        pC = vertices++;

        ((float*)&sign)[0] = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) -
                             ((pB->sx - pC->sx) * (pA->sy - pB->sy));

        sign &= 0x80000000;

        if ((sign ^ pRc->cullMask) != 0x80000000)
        {
          if (fillMode == D3DFILL_WIREFRAME)
          {
            fpDrawLine( pA, pB, pRc, pA->color );
            fpDrawLine( pB, pC, pRc, pA->color );
            fpDrawLine( pC, pA, pRc, pA->color );
          }
          else
          {
            fpDrawPoint( pA->sx, pA->sy, pA, pRc, pA->color );
            fpDrawPoint( pB->sx, pB->sy, pB, pRc, pA->color );
            fpDrawPoint( pC->sx, pC->sy, pC, pRc, pA->color );
          }
        }
      } // every triangle
      break;

    case D3DPT_TRIANGLESTRIP:
      count -=2 ;
      for (; count > 0; --count)
      {
        if (grab)
        {
          pA = vertices++;
          pB = vertices++;
          pC = vertices++;
        }
        else
        {
          pA = pC;
          pB = pB;
          pC = vertices--;
        }
        grab ^= 1;

        ((float*)&sign)[0] = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) -
                             ((pB->sx - pC->sx) * (pA->sy - pB->sy));

        sign &= 0x80000000;

        if ((sign ^ pRc->cullMask) != 0x80000000)
        {
          if (fillMode == D3DFILL_WIREFRAME)
          {
            fpDrawLine( pA, pB, pRc, pA->color );
            fpDrawLine( pB, pC, pRc, pA->color );
            fpDrawLine( pC, pA, pRc, pA->color );
          }
          else
          {
            fpDrawPoint( pA->sx, pA->sy, pA, pRc, pA->color );
            fpDrawPoint( pB->sx, pB->sy, pB, pRc, pA->color );
            fpDrawPoint( pC->sx, pC->sy, pC, pRc, pA->color );
          }
        }
      } // every triangle
      break;

    case D3DPT_TRIANGLEFAN:
      pA = vertices++;
      pC = vertices++;
      count -=2 ;
      for (; count > 0; --count)
      {
        pB = pC;
        pC = vertices++;

        ((float*)&sign)[0] = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) -
                             ((pB->sx - pC->sx) * (pA->sy - pB->sy));

        sign &= 0x80000000;

        if ((sign ^ pRc->cullMask) != 0x80000000)
        {
          if (fillMode == D3DFILL_WIREFRAME)
          {
            fpDrawLine( pA, pB, pRc, pA->color );
            fpDrawLine( pB, pC, pRc, pA->color );
            fpDrawLine( pC, pA, pRc, pA->color );
          }
          else
          {
            fpDrawPoint( pA->sx, pA->sy, pA, pRc, pA->color );
            fpDrawPoint( pB->sx, pB->sy, pB, pRc, pA->color );
            fpDrawPoint( pC->sx, pC->sy, pC, pRc, pA->color );
          }
        }
      } // every triangle
      break;

  } // switch
}
#endif

//-------------------------------------------------------------------

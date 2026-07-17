/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Revision: 6$
*/
#include "d3dhal.h"
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h" 
#include "fifomgr.h"

#define MAXFLOATS 256
static  float _int2float[MAXFLOATS];

//------------------------------------------------------------------------------
//
//  Triangle Primitive Processing - NO support for specular, fog and alpha blending
//
//------------------------------------------------------------------------------
void __stdcall fpDrawTriangleAllSW(RC           *pRc,
                                   WORD          count,
                                   LPD3DTRIANGLE tri,
                                   LPD3DTLVERTEX vertices) 
{
  float             dxAB, dxBC, dyAB, dyBC;
  float             dzdx, dzdy;
  D3DTLVERTEX      *pA,  *pB,   *pC;
  float             area;                        
  register float    ooarea;
  float             drdx, drdy, dgdx, dgdy, dbdx, dbdy;
  float             ax, ay, bx, by, cx, cy;
  BOOL              reverse;
  D3DCOLOR          Acolor, Bcolor, Ccolor, AScolor, BScolor, CScolor;
  float             rAB, rBC, gAB, gBC, bAB, bBC;
                    // reloads all the time
  float             s1, t1, s2, t2, s3, t3;
  float             ac, bc, cc; 

  // we may sort and rearrrange the order so save the color of the first
  // vertex so we can flat shade with the correct color
  D3DTLVERTEX   *pFlat;
  CMDFIFO_PROLOG( cmdFifo );
  
  //---------------
  //
  // every triangle        
  //
  //---------------
  for (; count > 0; --count)
  {
    pFlat = pA = &vertices[tri->v1];
    pB    = &vertices[tri->v2];
    pC    = &vertices[tri->v3];

    //-------------------
    //   
    // Setup for a sort
    //
    //-------------------
    //------------------------------------
    //
    //    Snap floating point vertices...
    //
    // Before we can do anything just snap x and y to x.4 precision. The
    // hardware performs its calculations with this so the driver should
    // sort and compute slope with the same precision.
    // Gary said that you don't want to mess around with s and t.
    //
    // must snap in local variables - can not change the execute buffer.
    #define SNAP_BIAS 524288.f  // (float) 1 << (23 - SST_XY_FRACBITS)
    #define SNAPIT(dest,src)  dest = D3DVAL(src) + SNAP_BIAS 

    {
      unsigned long sign;
      SNAPIT((volatile float)ax,pA->sx);
      SNAPIT((volatile float)ay,pA->sy);
      SNAPIT((volatile float)bx,pB->sx);
      SNAPIT((volatile float)by,pB->sy);
      SNAPIT((volatile float)cx,pC->sx);
      SNAPIT((volatile float)cy,pC->sy);

      // cull when area=0 
      area = ((ax - bx) * (by - cy)) - ((bx - cx) * (ay - by));
      sign = *(unsigned long *)&area;
      if ((sign & 0x7fffffff) == 0)
        goto Continue;
      
      // cull
      sign &= 0x80000000;
      if ((sign ^ pRc->cullMask) == 0x80000000)
        goto Continue;
    }


    //----------------------------------------------------------------
    //
    // Sort the vertices and associated colors where A.y <= B.y <= C.y
    //
    //----------------------------------------------------------------
    #define reverseValue 0x80000000
    reverse = 0;
    {
      D3DTLVERTEX *tmp; 
      float tx = ax; 
      float ty = ay;

      #define f2l(flt) *(long *)&flt 
      if (f2l(ay) < f2l(by))
      {
        if (f2l(by) > f2l(cy))
        {
          if (f2l(ay) < f2l(cy))
          {
            SWAP(pB, pC, void *);
            SWAP(by, cy, float);
            SWAP(bx, cx, float);
            reverse = reverseValue;    // reversed order of triangle
            area = -area;
          }
          else
          {
            //a = c
            SWAP(pA, pC, void *);
            SWAP(ay, cy, float);
            SWAP(ax, cx, float);
            //c = b
            SWAP(pB, pC, void *);
            SWAP(by, cy, float);
            SWAP(bx, cx, float);
            //b = a
            SWAP(by, ty, float);
            SWAP(bx, tx, float);
          } // else
        } // by > cy

        // else its already sorted
      } // ay < by
      else
      {
        if (f2l(by) < f2l(cy))
        {
          if (f2l(ay) < f2l(cy))
          {
            SWAP(pA, pB, void *);
            SWAP(ay, by, float);
            SWAP(ax, bx, float);
            reverse = reverseValue;    // reversed order of triangle
            area = -area;
          }
          else
          {
            tmp = pA;
            tx  = ax; 
            ty  = ay;
            // a = b
            SWAP(pA, pB, void *);
            SWAP(ay, by, float);
            SWAP(ax, bx, float);
            // b = c
            SWAP(pB, pC, void *);
            SWAP(by, cy, float);
            SWAP(bx, cx, float);
            // c = a
            SWAP(pC, tmp, void *);
            SWAP(cy, ty, float);
            SWAP(cx, tx, float);
          }
        }
        else
        {
          SWAP(pA, pC, void *);
          SWAP(ay, cy, float);
          SWAP(ax, cx, float);
          reverse = reverseValue;        // reversed order of triangle
          area = -area;
        }
      } 
    } //sort

    //---------------------------------------- 
    //
    // Compute the RGBAZ slopes for a triangle
    //
    //----------------------------------------
    ooarea = 1.0f / area;

    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 6 );
    
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(6, 1, FvA.x, 0xF ) );
    SETFPD( cmdFifo, ghw->FvA.x, ax );
    SETFPD( cmdFifo, ghw->FvA.y, ay );
    SETFPD( cmdFifo, ghw->FvB.x, bx );
    SETFPD( cmdFifo, ghw->FvB.y, by );
    SETFPD( cmdFifo, ghw->FvC.x, cx );
    SETFPD( cmdFifo, ghw->FvC.y, cy );

    if ((count - 1) > 0)
    {
      s1 = vertices[(tri+1)->v1].sx;
      s2 = vertices[(tri+1)->v2].sx;
      s3 = vertices[(tri+1)->v3].sx;
    }

    dxAB = ax - bx; 
    dyAB = ay - by; 
    dxBC = bx - cx; 
    dyBC = by - cy; 
    dxAB *= ooarea; 
    dyAB *= ooarea; 
    dxBC *= ooarea; 
    dyBC *= ooarea; 

    //----------
    //
    // Shade 
    //
    //----------
    if ( pRc->shadeMode == D3DSHADE_FLAT )
    {
#if 0	// SPECULARFIX!!!
      if ( (pRc->specular) && (pRc->texture == 0) )
        CLAMP888( Acolor, pFlat->color, pFlat->specular );
      else
#endif
	  {
        Acolor = pFlat->color;

        // Flat shaded specular should also work
		AScolor = pA->specular;
		CScolor = BScolor = AScolor;
	  }

      Bcolor = (Acolor & 0x00FFFFFF) | (pB->color & 0xFF000000);
      Ccolor = (Acolor & 0x00FFFFFF) | (pC->color & 0xFF000000);
    }
    else
    {
#if 0	// SPECULARFIX!!!
      if ( (pRc->specular) && (pRc->texture == 0) )
      {
        CLAMP888(Acolor, pA->color, pA->specular);
        CLAMP888(Bcolor, pB->color, pB->specular);
        CLAMP888(Ccolor, pC->color, pC->specular);
      }
      else
#endif
      {
        Acolor = pA->color;
        Bcolor = pB->color;
        Ccolor = pC->color;

        AScolor = pA->specular;
		BScolor = pB->specular;
		CScolor = pC->specular;
      }
    }

    CMDFIFO_CHECKROOM( cmdFifo, 3 * (( PH4_SIZE + 2 ) + ( PH1_SIZE + 1 )) );
    
    ac   = _int2float[RGBA_GETRED(Acolor)];
    bc   = _int2float[RGBA_GETRED(Bcolor)];
    cc   = _int2float[RGBA_GETRED(Ccolor)];
    rAB = (ac - bc);
    rBC = (bc - cc);
    drdx = ((rAB * dyBC) - (rBC * dyAB));
    drdy = ((rBC * dxAB) - (rAB * dxBC));
    SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R8, Fr, 0xF ) );
    SETFPD( cmdFifo, ghw->Fr, ac );
    SETFPD( cmdFifo, ghw->Fdrdx, drdx );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdrdy, 0xF ) );
    SETFPD( cmdFifo, ghw->Fdrdy, drdy );

    ac   = _int2float[RGBA_GETGREEN(Acolor)];
    bc   = _int2float[RGBA_GETGREEN(Bcolor)];
    cc   = _int2float[RGBA_GETGREEN(Ccolor)];
    gAB = (ac - bc);
    gBC = (bc - cc);
    dgdx = ((gAB * dyBC) - (gBC * dyAB));
    dgdy = ((gBC * dxAB) - (gAB * dxBC));
    SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R8, Fg, 0xF ) );
    SETFPD( cmdFifo, ghw->Fg, ac );
    SETFPD( cmdFifo, ghw->Fdgdx, dgdx );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdgdy, 0xF ) );
    SETFPD( cmdFifo, ghw->Fdgdy, dgdy );

    ac   = _int2float[RGBA_GETBLUE(Acolor)];
    bc   = _int2float[RGBA_GETBLUE(Bcolor)];
    cc   = _int2float[RGBA_GETBLUE(Ccolor)];
    bAB = (ac - bc);
    bBC = (bc - cc);
    dbdx = ((bAB * dyBC) - (bBC * dyAB));
    dbdy = ((bBC * dxAB) - (bAB * dxBC));
    SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R8, Fb, 0xF ) );
    SETFPD( cmdFifo, ghw->Fb, ac );
    SETFPD( cmdFifo, ghw->Fdbdx, dbdx );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdbdy, 0xF ) );
    SETFPD( cmdFifo, ghw->Fdbdy, dbdy );

    //----------
    //
    // Z 
    //
    //----------
    if (pRc->zEnable)
    {
      float dzAB, dzBC;
            
      //
      // D3D z values range from -1 to 1. Our hardware precision is 20.12 where the
      // high 4 bits are used for overflow. So, we scale z by 2**16.
      //
      CMDFIFO_CHECKROOM( cmdFifo, 3 * (PH1_SIZE + 1) );
      
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fz, 0xF ) );
      SETFPD( cmdFifo, ghw->Fz, ZSCALE(D3DVAL(pA->sz)) );
      dzAB =  D3DVAL(pA->sz)-D3DVAL(pB->sz);
      dzBC =  D3DVAL(pB->sz)-D3DVAL(pC->sz);
      dzdx =  ((dzAB * dyBC) - (dzBC * dyAB));
      dzdx =  ZSCALE(dzdx);
      dzdy =  ((dzBC * dxAB) - (dzAB * dxBC));
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdzdx, 0xF ) );
      SETFPD( cmdFifo, ghw->Fdzdx, dzdx );
      dzdy =  ZSCALE(dzdy);
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdzdy, 0xF ) );
      SETFPD( cmdFifo, ghw->Fdzdy, dzdy );
    }
    
    //----------
    //
    // Alpha 
    //
    //----------
    if (pRc->state & STATE_REQUIRES_IT_ALPHA)
    {
      float daAB, daBC, dadx, dady;

      CMDFIFO_CHECKROOM( cmdFifo, 3 * (PH1_SIZE + 1) );
      
      daAB = ((float)RGBA_GETALPHA(pA->color) - (float)RGBA_GETALPHA(pB->color)) ;
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fa, 0xF ) );
      SETFPD( cmdFifo, ghw->Fa, (float)RGBA_GETALPHA(pA->color) ) ;
      daBC = ((float)RGBA_GETALPHA(pB->color) - (float)RGBA_GETALPHA(pC->color)) ;
      dadx = ((daAB * dyBC) - (daBC * dyAB));
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdadx, 0xF ) );
      SETFPD( cmdFifo, ghw->Fdadx, dadx );
      dady = ((daBC * dxAB) - (daAB * dxBC));
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdady, 0xF ) );
      SETFPD( cmdFifo, ghw->Fdady, dady );
    }
    
    //-------------------------
    //
    // Vertex style Fog
    //
    //-------------------------
    if (pRc->state & STATE_REQUIRES_VERTEXFOG)
    {
      float daAB, daBC, dadx, dady;
      float fogA, fogB, fogC;

      // need to put the alpha component of specular into iterated alpha
      // and use as Afog.
      //D3DPRINT(0,"Fog: Specular A=0x%x B=0x%x C=0x%x  fog1=%d fog2=%d fog3=%d",
      //    pA->specular, pB->specular, pC->specular,    
      //    RGBA_GETALPHA(pA->specular),RGBA_GETALPHA(pB->specular),RGBA_GETALPHA(pC->specular));
      fogA = (float)(255 - RGBA_GETALPHA(pA->specular));
      fogB = (float)(255 - RGBA_GETALPHA(pB->specular));
      fogC = (float)(255 - RGBA_GETALPHA(pC->specular));
      
      daAB = fogA - fogB;
      daBC = fogB - fogC;
      dadx = ((daAB * dyBC) - (daBC * dyAB));
      dady = ((daBC * dxAB) - (daAB * dxBC));
      
      CMDFIFO_CHECKROOM( cmdFifo, 3 * (PH1_SIZE + 1) );
      
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fw, 1 ) );
      SETFPD( cmdFifo, SST_CHIP(ghw0, 1)->Fw, fogA );
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdwdx, 1 ) );
      SETFPD( cmdFifo, SST_CHIP(ghw0, 1)->Fdwdx, dadx );
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdwdy, 1 ) );
      SETFPD( cmdFifo, SST_CHIP(ghw0, 1)->Fdwdy, dady );
    }
    
    if (pRc->state & STATE_REQUIRES_HWFOG)
    {
      float dwAB, dwBC, dwdx, dwdy;
      // Per pixel fog needs depth information and usually this is 
      // computed above when we are texture mapping. If we are not
      // texture mapping then use w.
      dwAB = D3DVAL(pA->rhw) - D3DVAL(pB->rhw) ;
      dwBC = D3DVAL(pB->rhw) - D3DVAL(pC->rhw) ;
      dwdx = ((dwAB * dyBC) - (dwBC * dyAB));
      dwdy = ((dwBC * dxAB) - (dwAB * dxBC));
      
      CMDFIFO_CHECKROOM( cmdFifo, 3 * (PH1_SIZE + 1) );
      
      SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R8, Fw, 1 ) );
      SETFPD( cmdFifo, SST_CHIP(ghw0, 1)->Fw, D3DVAL(pA->rhw) );
      SETFPD( cmdFifo, SST_CHIP(ghw0, 1)->Fdwdx, dwdx );
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdwdy, 1 ) );
      SETFPD( cmdFifo, SST_CHIP(ghw0, 1)->Fdwdy, dwdy );
    }

    //----------------
    //
    // Texture Mapping 
    //
    //----------------
    // NOTE: texture processing must occur after we process fog, chroma etc. because
    //       trilinear processing draws two triangles and assumes everything is setup
    //       by now.     
    if ((pRc->texture != 0))
    {
      float dsdx, dsdy, dtdx, dtdy, dwdx, dwdy;
      float lscaleS, lscaleT;
       
      s1 = D3DVAL(pA->tu) ;
      t1 = D3DVAL(pA->tv) ;
      s2 = D3DVAL(pB->tu) ;
      t2 = D3DVAL(pB->tv) ;
      s3 = D3DVAL(pC->tu) ;
      t3 = D3DVAL(pC->tv) ;

      // two ways to texture. D3D wraps its textures going around the other direction
      // so we need to adjust S and T in order to get the correct result
      GET_ST(s, t, pRc->wrapU, pRc->wrapV);

      // compute the slope of s t and w
      // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
      if (pRc->texturePerspective)
      {
        float dwAB, dwBC;
        float dt1, dt2, ds1, ds2;
  
        CMDFIFO_CHECKROOM( cmdFifo, 9 * (PH1_SIZE + 1) );
      
        lscaleS = pRc->sst.scaleS;       
        s1 = s1 * D3DVAL(pA->rhw) * lscaleS;
        s2 = s2 * D3DVAL(pB->rhw) * lscaleS;
        s3 = s3 * D3DVAL(pC->rhw) * lscaleS;
        ds1 = s1-s2;
        ds2 = s2-s3;
        dsdx = ( (ds1 * dyBC) - (ds2 * dyAB) );
        dsdy = ( (ds2 * dxAB) - (ds1 * dxBC) );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R8, Fs, 0xF ) );
        SETFPD( cmdFifo, ghw->Fs, s1 );
        SETFPD( cmdFifo, ghw->Fdsdx, dsdx );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdsdy, 0xF ) );
        SETFPD( cmdFifo, ghw->Fdsdy, dsdy );
        
        lscaleT = pRc->sst.scaleT;
        t1 = t1 * D3DVAL(pA->rhw) * lscaleT;
        t2 = t2 * D3DVAL(pB->rhw) * lscaleT;
        t3 = t3 * D3DVAL(pC->rhw) * lscaleT;
        dt1 = t1-t2;
        dt2 = t2-t3;
        dtdx = ( (dt1 * dyBC) - (dt2 * dyAB) );
        dtdy = ( (dt2 * dxAB) - ((dt1) * dxBC) );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R8, Ft, 0xF ) );
        SETFPD( cmdFifo, ghw->Ft, t1 );
        SETFPD( cmdFifo, ghw->Fdtdx, dtdx );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdtdy, 0xF ) );
        SETFPD( cmdFifo, ghw->Fdtdy, dtdy );

        dwAB = D3DVAL(pA->rhw)-D3DVAL(pB->rhw) ;
        dwBC = D3DVAL(pB->rhw)-D3DVAL(pC->rhw) ;
        dwdx = ((dwAB * dyBC) - (dwBC * dyAB));
        dwdy = ((dwBC * dxAB) - (dwAB * dxBC));
        SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R8, Fw, 2 ) );
        SETFPD( cmdFifo, SST_CHIP(ghw0, 2)->Fw, D3DVAL(pA->rhw) );
        SETFPD( cmdFifo, SST_CHIP(ghw0, 2)->Fdwdx, dwdx );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdwdy, 2 ) );
        SETFPD( cmdFifo, SST_CHIP(ghw0, 2)->Fdwdy, dwdy );
      }
      // if the texture is not prespective correct then the w is effectively equal to 1
      else
      {
        CMDFIFO_CHECKROOM( cmdFifo, 9 * (PH1_SIZE + 1) );
        
        s1 = s1 * pRc->sst.scaleS;
        s2 = s2 * pRc->sst.scaleS;
        s3 = s3 * pRc->sst.scaleS;
        dsdx = ( ((s1-s2) * dyBC) - ((s2-s3) * dyAB) );
        dsdy = ( ((s2-s3) * dxAB) - ((s1-s2) * dxBC) );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R8, Fs, 0xF ) );
        SETFPD( cmdFifo, ghw->Fs, s1 );
        SETFPD( cmdFifo, ghw->Fdsdx, dsdx );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdsdy, 0xF ) );
        SETFPD( cmdFifo, ghw->Fdsdy, dsdy );
        
        t1 = t1 * pRc->sst.scaleT;
        t2 = t2 * pRc->sst.scaleT;
        t3 = t3 * pRc->sst.scaleT;
        dtdx = ( ((t1-t2) * dyBC) - ((t2-t3) * dyAB) );
        dtdy = ( ((t2-t3) * dxAB) - ((t1-t2) * dxBC) );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R8, Ft, 0xF ) );
        SETFPD( cmdFifo, ghw->Ft, t1 );
        SETFPD( cmdFifo, ghw->Fdtdx, dtdx );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdtdy, 0xF ) );
        SETFPD( cmdFifo, ghw->Fdtdy, dtdy );
        
        SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R8, Fw, 2 ) );
        SETFPD( cmdFifo, SST_CHIP(ghw0, 2)->Fw, D3DVAL(pA->rhw) );
        SETFPD( cmdFifo, SST_CHIP(ghw0, 2)->Fdwdx, 0.0f );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, Fdwdy, 2 ) );
        SETFPD( cmdFifo, SST_CHIP(ghw0, 2)->Fdwdy, 0.0f );
      }
    } // texture
    //------------------------------------------------ 
    // Per pixel Fog table Fog with no texture mapping
    //------------------------------------------------ 
    
    //----------------
    //
    // Draw Triangle 
    //
    //----------------
 
    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, FtriangleCMD, 0xF ) );
    SETFPD( cmdFifo, ghw->FtriangleCMD, area );
    
    //---------------------
    //
    // Specular on Textures
    //
    //---------------------
    // specular highlights on texture where the specular color is not black
#if 0	// SPECULARFIX!!!
    if (   (pRc->specular) && (pRc->texture != 0) &&
#else
    if (   (pRc->specular) &&
#endif
      (CI_MASKALPHA(pA->specular | pB->specular | pC->specular) != 0))
    {
      // if specular is turned on then we need to add in the specular color
      // and this means re-render the triangle gouraud shaded using the specular
      // color information and then add it to the textured triangle we just rendered.
      // We can't just add it to the triangle color because this color would be
      // blended or replaced with the texture and highlights are added into textures.
      //
      ULONG fbzMode;
      ULONG combineModeFBI;
      ULONG fbzColorPath; 

      float ac, bc, cc;

      // Where FOG() = fog function
      // FOG(T1 + T2) = AlphaFog * FogColor + (1 - AlphaFog)[T1 +T2]   
      //    Pass 1    = AlphaFog * FogColor + (1 - AlphaFog)T1 
      //    Pass 2    =                       (1 - AlphaFog)T2
      if (pRc->fogEnable)
      {
        SET( cmdFifo, ghw->fogMode,
        (pRc->sst.fogMode | SST_FOGADD) & ~SST_ENFOGGING ) ;
      }
    
      // let's just add in the color ignoring the effect on alpha blending for
      // the time being
      // (Src * 1 + Dst * 1)
      SET( cmdFifo, ghw->alphaMode,
        (SST_ENALPHABLEND | (SST_A_ONE << SST_RGBSRCFACT_SHIFT)
        | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
        
      Acolor = AScolor;
      Bcolor = BScolor;
      Ccolor = CScolor;

      SETF( cmdFifo, ghw->vA.x, ax );
      SETF( cmdFifo, ghw->vA.y, ay );
      SETF( cmdFifo, ghw->vB.x, bx );
      SETF( cmdFifo, ghw->vB.y, by );
      SETF( cmdFifo, ghw->vC.x, cx );
      SETF( cmdFifo, ghw->vC.y, cy );

      ac   = _int2float[RGBA_GETRED(Acolor)];
      bc   = _int2float[RGBA_GETRED(Bcolor)];
      cc   = _int2float[RGBA_GETRED(Ccolor)];
      rAB = (ac - bc);
      rBC = (bc - cc);
      drdx = ((rAB * dyBC) - (rBC * dyAB));
      drdy = ((rBC * dxAB) - (rAB * dxBC));

      SETF( cmdFifo, ghw->Fr, ac );
      SETF( cmdFifo, ghw->Fdrdx, drdx );
      SETF( cmdFifo, ghw->Fdrdy, drdy );

      ac   = _int2float[RGBA_GETGREEN(Acolor)];
      bc   = _int2float[RGBA_GETGREEN(Bcolor)];
      cc   = _int2float[RGBA_GETGREEN(Ccolor)];
      gAB = (ac - bc);
      gBC = (bc - cc);
      dgdx = ((gAB * dyBC) - (gBC * dyAB));
      SETF( cmdFifo, ghw->Fg, ac);
      dgdy = ((gBC * dxAB) - (gAB * dxBC) );
      SETF( cmdFifo, ghw->Fdgdx, dgdx );

      ac   = _int2float[RGBA_GETBLUE(Acolor)];
      bc   = _int2float[RGBA_GETBLUE(Bcolor)];
      cc   = _int2float[RGBA_GETBLUE(Ccolor)];
      SETF( cmdFifo, ghw->Fdgdy, dgdy );
      bAB = (ac - bc);
      SETF( cmdFifo, ghw->Fb, ac );
      bBC = (bc - cc);
      dbdx = ((bAB * dyBC) - (bBC * dyAB) );
      SETF( cmdFifo, ghw->Fdbdx, dbdx );
      dbdy = ((bBC * dxAB) - (bAB * dxBC) );
      SETF( cmdFifo, ghw->Fdbdy, dbdy );

      if (pRc->zEnable) 
      {
        // if z-buffering then this triangle z values equals the values written on pass 1
        fbzMode  =  pRc->sst.fbzMode & ~(SST_ZFUNC_LT | SST_ZFUNC_EQ | SST_ZFUNC_GT) ;
        fbzMode |= SST_ZFUNC_EQ ;

        SET( cmdFifo, ghw->fbzMode, fbzMode );
     
        // if subpixel correct then must reset values or will subpixel incorrectly
        SETF( cmdFifo, ghw->Fz, ZSCALE(D3DVAL(pA->sz)) );
        SETF( cmdFifo, ghw->Fdzdx, dzdx );
        SETF( cmdFifo, ghw->Fdzdy, dzdy );
      }

      // texture mapping off and texture blending off
      SET( cmdFifo, ghw->nopCMD, 0 );
      if (IS_NAPALM) {//NAPALM_CU
            combineModeFBI = ( SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | 
                               SST_CM_DISABLE_CHROMA_SUBSTITUTION | 
                               SST_CM_USE_COMBINE_MODE ) & pRc->sst.combineModeFBI; 
        SET( cmdFifo, ghw->combineMode, combineModeFBI );
      }
      else {
        fbzColorPath = DEFAULT_FBZCOLORPATH; 
        if (pRc->subPixel == TRUE)
            fbzColorPath |= SST_PARMADJUST;
        SET( cmdFifo, ghw->fbzColorPath, fbzColorPath );
      }
      // if subpixel is on then must resend vertices
      // I interlaced this above to spread out the PCI writes.

      SETF( cmdFifo, ghw->FtriangleCMD, area );

      // restore everything back to the way it was
      if (IS_NAPALM) { //NAPALM_CU
        SET( cmdFifo, ghw->combineMode, pRc->sst.combineModeFBI );
      }
      else {
        SET( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath ) ;
      }
      SET( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode ) ;

      if (pRc->fogEnable)
      {
        SET( cmdFifo, ghw->fogMode, pRc->sst.fogMode ) ;         
      }
    
      SET( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );

    } // specular

Continue: ;
    ++tri ;

  } // every triangle

  CMDFIFO_EPILOG( cmdFifo );
  
} // floating point triangle

void __stdcall d3triInit( void )
{
  int cnt;
  
  for (cnt = 0; cnt < MAXFLOATS; ++cnt)
     _int2float[cnt] = (float) cnt;
}
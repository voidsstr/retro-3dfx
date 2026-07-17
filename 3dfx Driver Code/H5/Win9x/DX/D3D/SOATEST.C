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
** File name: soatest.c 
**
** Description: -- Testcode for proving out SOA fastpath
**
*/

#include "precomp.h"

#if( DX >= 7 )
#ifdef TnL_HAL
#if defined(VCPP)

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


DWORD SwizzleSOA2AOSTEST ( RC *pRc,          // Pointer to the rendering context structure, core of all structures
                          DWORD index )     // Index number of the first element in the group of four elements in TL_SOATMP
{
  int  i = 0;
  int  j = 0;
  TL_SOATMP   *tltmp_ptr = pRc->tl.pTL;
  D3DTLVERTEX *TLBuff_Ptr;
  DWORD       *TLBuff_Out;
//  float       *TLBuff_FOut;
//  float       *fTmpSrc;
  DWORD       *InFVFPtr;
  TLCLIPCODE *clipbuf_ptr;
  DWORD          ret_val = 0;
//  float          f1_tmp, f2_tmp, f3_tmp, f4_tmp;

  DWORD dwMask = TLCLIP_LEFT  | TLCLIP_RIGHT | TLCLIP_TOP | TLCLIP_BOTTOM  | TLCLIP_FRONT | TLCLIP_BACK | TLCLIP_USERPLANES_ALL;
  if (pRc->tl.dwTLState & TLPV_GUARDBAND) {
    dwMask = TLCLIPGB_LEFT | TLCLIPGB_RIGHT | TLCLIPGB_TOP | TLCLIPGB_BOTTOM |      
        TLCLIP_FRONT | TLCLIP_BACK | TLCLIP_USERPLANES_ALL; 
  }


  // Loop through all 4 vertices in the SOA format.
  clipbuf_ptr = ((TLCLIPCODE *)pRc->tl.pClipBuf) + index;
  for ( i = 0; i < 4; i ++ ) {
    TLBuff_Ptr = (D3DTLVERTEX *)((char *)pRc->tl.TLFVF.lpvData + ( pRc->tl.TLFVF.dwStride * (index + i)));

    *clipbuf_ptr++ = tltmp_ptr->dSOAclip_code.d[i];

    // If we are clipping, then send down the homogenous vertex
    // a.k.a. the "Transformed to viewer" vertices
    if (pRc->tl.dwTLState & TLPV_DOCLIPPING) {
      if ( ( tltmp_ptr->dSOAclip_code.d[i] & dwMask ) != 0 ) {
        TLBuff_Ptr->sx  = tltmp_ptr->SOAhv.x.f[i];
        TLBuff_Ptr->sy  = tltmp_ptr->SOAhv.y.f[i];
        TLBuff_Ptr->sz  = tltmp_ptr->SOAhv.z.f[i];
        TLBuff_Ptr->rhw = tltmp_ptr->SOAhv.w.f[i];
        ret_val = 1;
      }
      else // We are not crossing a clipping plane
      {

        // Adjust the screen coordinates back to 
        // Non-Computed State
        TLBuff_Ptr->sx = tltmp_ptr->sv.x.f[i] - pRc->sst.pixelOffset;
        TLBuff_Ptr->sy = tltmp_ptr->sv.y.f[i] - pRc->sst.pixelOffset;
        TLBuff_Ptr->sz = tltmp_ptr->sv.z.f[i] / pRc->zScale;
        TLBuff_Ptr->rhw = tltmp_ptr->sv.w.f[i];
      }   
    }
    else {
      TLBuff_Ptr->sx = tltmp_ptr->sv.x.f[i] - pRc->sst.pixelOffset;
      TLBuff_Ptr->sy = tltmp_ptr->sv.y.f[i] - pRc->sst.pixelOffset;
      TLBuff_Ptr->sz = tltmp_ptr->sv.z.f[i] / pRc->zScale;
      TLBuff_Ptr->rhw = tltmp_ptr->sv.w.f[i];
    }

    TLBuff_Out = (DWORD*)((char*)TLBuff_Ptr + 4*sizeof(D3DVALUE));


    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_DIFFUSE) {
      *TLBuff_Out = tltmp_ptr->dSOAdiff.d[i];

      TLBuff_Out++;           
    }
    if (pRc->tl.TLFVF.dwFVFType & D3DFVF_SPECULAR) {
      *TLBuff_Out = tltmp_ptr->dSOAspec.d[i];

      TLBuff_Out++;
    }

    // For unknown reasons, we need to fill in the original InFVFto the TLFVF,
    // Maybe for initialization purposes
    InFVFPtr = (DWORD *)((char *)pRc->tl.InFVF.lpvData + ( pRc->tl.InFVF.dwStride * (index + i)));
    memcpy(TLBuff_Out, ((char*)InFVFPtr + pRc->tl.InFVF.dwTexOffset), pRc->tl.InFVF.dwTexCoordSize);


#if 0

    // Now we overwrite one or both texture coordinates by backing out perspective divide,
    // offset or scale.  This tests out our Device Coordinate Transform.
    fTmpSrc = (float *)&tltmp_ptr->SOAtex[0];

    if( pRc->state & STATE_REQUIRES_ST_TMU0 ) {
      f1_tmp = fTmpSrc[i*2];
      f2_tmp = fTmpSrc[(i*2)+1];
      if((pRc->state & STATE_REQUIRES_PERSPECTIVE) && (pRc->wrapT0 == 0) && (pRc->wrapT1 == 0)) {
        f1_tmp /= tltmp_ptr->sv.w.f[i];
        f2_tmp /= tltmp_ptr->sv.w.f[i];
      }
      f1_tmp -= pRc->sst.centerS;
      f2_tmp -= pRc->sst.centerT;
      f1_tmp /= pRc->sst.scaleS;
      f2_tmp /= pRc->sst.scaleT;

      TLBuff_FOut = (float *)(TLBuff_Out + pRc->t0CoordIndex);

      *TLBuff_FOut++ = f1_tmp;
      *TLBuff_FOut++ = f2_tmp;

    } 
    if( pRc->state & STATE_REQUIRES_ST_TMU1 ) {
      f3_tmp = fTmpSrc[(i*2)+8];
      f4_tmp = fTmpSrc[(i*2)+9];

      if((pRc->state & STATE_REQUIRES_PERSPECTIVE) && (pRc->wrapT0 == 0) && (pRc->wrapT1 == 0)) {
        f3_tmp /= tltmp_ptr->sv.w.f[i];
        f4_tmp /= tltmp_ptr->sv.w.f[i];
      }
      f3_tmp -= pRc->sst.centerS1;
      f4_tmp -= pRc->sst.centerT1;
      f3_tmp /= pRc->sst.scaleS1;
      f4_tmp /= pRc->sst.scaleT1;

      TLBuff_FOut = (float *)(TLBuff_Out + pRc->t1CoordIndex);

      *TLBuff_FOut++ = f3_tmp;
      *TLBuff_FOut++ = f4_tmp;

    }
#endif

  } // For i = 0; i < 4; i ++

  return ret_val;

}





#endif 
#endif
#endif

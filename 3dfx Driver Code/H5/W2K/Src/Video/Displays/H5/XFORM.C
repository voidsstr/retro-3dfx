/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** File name:   xform.c
**
** Description: transformation code 
**
**
** 1     8/10/99 Ping Zheng
** Created
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
void SetXfrm(RC *pRc, D3DTRANSFORMSTATETYPE xfrmType, D3DMATRIX *pMat )
{
  // BUGBUG is there a define for 0x80000000?
  BOOL bSetIdentity = (xfrmType & 0x80000000) != 0;
  DWORD dwxfrmType = (DWORD)xfrmType & (~0x80000000);
  
#ifdef TNL_PROFILE
  MatrixStats(dwxfrmType);
#endif //TNL_PROFILE
  
  switch (dwxfrmType)
  {
    case D3DTRANSFORMSTATE_WORLD:
      memcpy(&(pRc->tl.xfmWorld[0]), pMat, sizeof(D3DMATRIX));
      pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLDXFM;
      break;
    case D3DTRANSFORMSTATE_VIEW:
      memcpy(&(pRc->tl.xfmView), pMat, sizeof(D3DMATRIX));
      pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VIEWXFM;
      break;
    case D3DTRANSFORMSTATE_PROJECTION:
      memcpy(&(pRc->tl.xfmProj), pMat, sizeof(D3DMATRIX));
      pRc->tl.dwDirtyFlags |= TLPV_DIRTY_PROJXFM;
      break;
    case D3DTRANSFORMSTATE_WORLD1:
      memcpy(&(pRc->tl.xfmWorld[1]), pMat, sizeof(D3DMATRIX));
      pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLD1XFM;
      break;
    case D3DTRANSFORMSTATE_WORLD2:
      memcpy(&(pRc->tl.xfmWorld[2]), pMat, sizeof(D3DMATRIX));
      pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLD2XFM;
      break;
    case D3DTRANSFORMSTATE_WORLD3:
      memcpy(&(pRc->tl.xfmWorld[3]), pMat, sizeof(D3DMATRIX));
      pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLD3XFM;
      break;

    case D3DTRANSFORMSTATE_TEXTURE0:
    case D3DTRANSFORMSTATE_TEXTURE1:
    case D3DTRANSFORMSTATE_TEXTURE2:
    case D3DTRANSFORMSTATE_TEXTURE3:
    case D3DTRANSFORMSTATE_TEXTURE4:
    case D3DTRANSFORMSTATE_TEXTURE5:
    case D3DTRANSFORMSTATE_TEXTURE6:
    case D3DTRANSFORMSTATE_TEXTURE7:
/*
      {
        DWORD dwStage = xfrmType - D3DTRANSFORMSTATE_TEXTURE0;
        if (bSetIdentity)
        {
          memcpy(&pRc->TextureStageState[dwStage].dwVal[D3DTSSI_MATRIX], &matIdent, sizeof(D3DMATRIX));
        }
        else
        {
          memcpy(&pRc->TextureStageState[dwStage].dwVal[D3DTSSI_MATRIX], pMat, sizeof(D3DMATRIX));
        }
      }
      break;
*/
    default:
      break;
  }
}

//---------------------------------------------------------------------
// This function uses Cramer's Rule to calculate the matrix inverse.
// See nt\private\windows\opengl\serever\soft\so_math.c
//
// Returns:
//    0 - if success
//   -1 - if input matrix is singular
//
int Inverse4x4(D3DMATRIX *src, D3DMATRIX *inverse)
{
    double x00, x01, x02;
    double x10, x11, x12;
    double x20, x21, x22;
    double rcp;
    double x30, x31, x32;
    double y01, y02, y03, y12, y13, y23;
    double z02, z03, z12, z13, z22, z23, z32, z33;

#define x03 x01
#define x13 x11
#define x23 x21
#define x33 x31
#define z00 x02
#define z10 x12
#define z20 x22
#define z30 x32
#define z01 x03
#define z11 x13
#define z21 x23
#define z31 x33

    /* read 1st two columns of matrix into registers */
    x00 = src->_11;
    x01 = src->_12;
    x10 = src->_21;
    x11 = src->_22;
    x20 = src->_31;
    x21 = src->_32;
    x30 = src->_41;
    x31 = src->_42;

    /* compute all six 2x2 determinants of 1st two columns */
    y01 = x00*x11 - x10*x01;
    y02 = x00*x21 - x20*x01;
    y03 = x00*x31 - x30*x01;
    y12 = x10*x21 - x20*x11;
    y13 = x10*x31 - x30*x11;
    y23 = x20*x31 - x30*x21;

    /* read 2nd two columns of matrix into registers */
    x02 = src->_13;
    x03 = src->_14;
    x12 = src->_23;
    x13 = src->_24;
    x22 = src->_33;
    x23 = src->_34;
    x32 = src->_43;
    x33 = src->_44;

    /* compute all 3x3 cofactors for 2nd two columns */
    z33 = x02*y12 - x12*y02 + x22*y01;
    z23 = x12*y03 - x32*y01 - x02*y13;
    z13 = x02*y23 - x22*y03 + x32*y02;
    z03 = x22*y13 - x32*y12 - x12*y23;
    z32 = x13*y02 - x23*y01 - x03*y12;
    z22 = x03*y13 - x13*y03 + x33*y01;
    z12 = x23*y03 - x33*y02 - x03*y23;
    z02 = x13*y23 - x23*y13 + x33*y12;

    /* compute all six 2x2 determinants of 2nd two columns */
    y01 = x02*x13 - x12*x03;
    y02 = x02*x23 - x22*x03;
    y03 = x02*x33 - x32*x03;
    y12 = x12*x23 - x22*x13;
    y13 = x12*x33 - x32*x13;
    y23 = x22*x33 - x32*x23;

    /* read 1st two columns of matrix into registers */
    x00 = src->_11;
    x01 = src->_12;
    x10 = src->_21;
    x11 = src->_22;
    x20 = src->_31;
    x21 = src->_32;
    x30 = src->_41;
    x31 = src->_42;

    /* compute all 3x3 cofactors for 1st column */
    z30 = x11*y02 - x21*y01 - x01*y12;
    z20 = x01*y13 - x11*y03 + x31*y01;
    z10 = x21*y03 - x31*y02 - x01*y23;
    z00 = x11*y23 - x21*y13 + x31*y12;

    /* compute 4x4 determinant & its reciprocal */
    rcp = x30*z30 + x20*z20 + x10*z10 + x00*z00;
    if (rcp == (float)0)
    return -1;
    rcp = (float)1/rcp;

    /* compute all 3x3 cofactors for 2nd column */
    z31 = x00*y12 - x10*y02 + x20*y01;
    z21 = x10*y03 - x30*y01 - x00*y13;
    z11 = x00*y23 - x20*y03 + x30*y02;
    z01 = x20*y13 - x30*y12 - x10*y23;

    /* multiply all 3x3 cofactors by reciprocal */
    inverse->_11 = (float)(z00*rcp);
    inverse->_21 = (float)(z01*rcp);
    inverse->_12 = (float)(z10*rcp);
    inverse->_31 = (float)(z02*rcp);
    inverse->_13 = (float)(z20*rcp);
    inverse->_41 = (float)(z03*rcp);
    inverse->_14 = (float)(z30*rcp);
    inverse->_22 = (float)(z11*rcp);
    inverse->_32 = (float)(z12*rcp);
    inverse->_23 = (float)(z21*rcp);
    inverse->_42 = (float)(z13*rcp);
    inverse->_24 = (float)(z31*rcp);
    inverse->_33 = (float)(z22*rcp);
    inverse->_43 = (float)(z23*rcp);
    inverse->_34 = (float)(z32*rcp);
    inverse->_44 = (float)(z33*rcp);
    return 0;
}

//---------------------------------------------------------------------
#define MATRIX_PRODUCT(res, a, b)                                           \
res->_11 = a->_11*b->_11 + a->_12*b->_21 + a->_13*b->_31 + a->_14*b->_41;   \
res->_12 = a->_11*b->_12 + a->_12*b->_22 + a->_13*b->_32 + a->_14*b->_42;   \
res->_13 = a->_11*b->_13 + a->_12*b->_23 + a->_13*b->_33 + a->_14*b->_43;   \
res->_14 = a->_11*b->_14 + a->_12*b->_24 + a->_13*b->_34 + a->_14*b->_44;   \
                                                                            \
res->_21 = a->_21*b->_11 + a->_22*b->_21 + a->_23*b->_31 + a->_24*b->_41;   \
res->_22 = a->_21*b->_12 + a->_22*b->_22 + a->_23*b->_32 + a->_24*b->_42;   \
res->_23 = a->_21*b->_13 + a->_22*b->_23 + a->_23*b->_33 + a->_24*b->_43;   \
res->_24 = a->_21*b->_14 + a->_22*b->_24 + a->_23*b->_34 + a->_24*b->_44;   \
                                                                            \
res->_31 = a->_31*b->_11 + a->_32*b->_21 + a->_33*b->_31 + a->_34*b->_41;   \
res->_32 = a->_31*b->_12 + a->_32*b->_22 + a->_33*b->_32 + a->_34*b->_42;   \
res->_33 = a->_31*b->_13 + a->_32*b->_23 + a->_33*b->_33 + a->_34*b->_43;   \
res->_34 = a->_31*b->_14 + a->_32*b->_24 + a->_33*b->_34 + a->_34*b->_44;   \
                                                                            \
res->_41 = a->_41*b->_11 + a->_42*b->_21 + a->_43*b->_31 + a->_44*b->_41;   \
res->_42 = a->_41*b->_12 + a->_42*b->_22 + a->_43*b->_32 + a->_44*b->_42;   \
res->_43 = a->_41*b->_13 + a->_42*b->_23 + a->_43*b->_33 + a->_44*b->_43;   \
res->_44 = a->_41*b->_14 + a->_42*b->_24 + a->_43*b->_34 + a->_44*b->_44;
//---------------------------------------------------------------------
// result = a*b
// result is the same as a or b
//
void MatrixProduct2(D3DMATRIX *result, D3DMATRIX *a, D3DMATRIX *b)
{
    D3DMATRIX res;
    MATRIX_PRODUCT((&res), a, b);
    *(D3DMATRIX*)result = res;
}
//---------------------------------------------------------------------
// result = a*b.
// "result" pointer  could be equal to "a" or "b"
//
void MatrixProduct(D3DMATRIX *result, D3DMATRIX *a, D3DMATRIX *b)
{
    if (result == a || result == b)
    {
        MatrixProduct2(result, a, b);
        return;
    }
    MATRIX_PRODUCT(result, a, b);
}


/*
 ** UpdateXformData
 *
 *  FILENAME: C:\project\NAPALM\d3d\xform.c
 *
 *  PARAMETERS:	RC *pRC 
 *
 *  DESCRIPTION: Updates transform data used by ProcessVertices
 *
 *  RETURNS: D3D_OK
 *
 */

HRESULT UpdateXformData(RC *pRc)
{
	DWORD i;
	HRESULT hr = D3D_OK;
	TLVIEWPORTDATA  *VData = &pRc->tl.ViewData;
	TLTRANSFORMDATA *TData = &pRc->tl.TransformData;

#ifdef TNL_PROFILE
    MatrixStats(0L); // Count this as an update JJP
#endif //TNL_PROFILE
	/* Update viewport information */
	if (pRc->tl.dwDirtyFlags & TLPV_DIRTY_ZRANGE)
	{
		VData->scaleZ  = pRc->tl.Viewport.dvMaxZ - pRc->tl.Viewport.dvMinZ;
		VData->offsetZ = pRc->tl.Viewport.dvMinZ;
		/* BUGBUG: This could be a Divide by Zero here if */
		/* the dvMaxZ == dvMinZ. Fix it later.            */
		VData->scaleZi = D3DVAL(1) / VData->scaleZ;
	}

	if (pRc->tl.dwDirtyFlags & TLPV_DIRTY_VIEWRECT)
	{
		// Bail if we are going to cause any divide by zero exceptions.
		// The likely reason is that we have a bogus viewport set by
		// TLVertex execute buffer app.
		if(pRc->tl.Viewport.dwWidth == 0 || pRc->tl.Viewport.dwHeight == 0 )
			return DDERR_GENERIC;

		VData->dvX = D3DVAL(pRc->tl.Viewport.dwX);
		VData->dvY = D3DVAL(pRc->tl.Viewport.dwY);
		VData->dvWidth = D3DVAL(pRc->tl.Viewport.dwWidth);
		VData->dvHeight = D3DVAL(pRc->tl.Viewport.dwHeight);

		// Coefficients to compute screen coordinates from normalized window
		// coordinates
		VData->scaleX  = VData->dvWidth;
		VData->scaleY  = - VData->dvHeight;
		VData->offsetX = VData->dvX;
		VData->offsetY = VData->dvY + VData->dvHeight;

#if 0
				// Small offset is added to prevent generation of negative screen
				// coordinates (this could happen because of precision errors).
				// Not needed (or wanted) for devices which do guardband.
				VData->offsetX += SMALL_NUMBER;
		VData->offsetY += SMALL_NUMBER;
#endif

		VData->scaleXi = D3DVAL(1) / VData->scaleX;
		VData->scaleYi = D3DVAL(1) / VData->scaleY;

		VData->minX = VData->dvX;
		VData->maxX = VData->dvX + VData->dvWidth;
		VData->minY = VData->dvY;
		VData->maxY = VData->dvY + VData->dvHeight;

		if (pRc->tl.dwTLState & TLPV_GUARDBAND)
		{
		    
			// Because we clip by guard band window we have to use its extents
			D3DVALUE w = 2.0f / VData->dvWidth;
			D3DVALUE h = 2.0f / VData->dvHeight;
// 			D3DVALUE ax1 = -(H5_GB_LEFT)   * w + 1.0f;
// 			D3DVALUE ax2 =  H5_GB_RIGHT  * w - 1.0f;
// 			D3DVALUE ay1 =  H5_GB_BOTTOM * h - 1.0f;
// 			D3DVALUE ay2 = -(H5_GB_TOP) * h + 1.0f;
			D3DVALUE ax1 = -(VData->minXgb - VData->dvX)  * w + 1.0f;
			D3DVALUE ax2 =  (VData->maxXgb - VData->dvX)  * w - 1.0f;
			D3DVALUE ay1 =  (VData->maxYgb - VData->dvY)  * h - 1.0f;
			D3DVALUE ay2 = -(VData->minYgb - VData->dvY)  * h + 1.0f;
			VData->gb11 = 2.0f / (ax1 + ax2);
			VData->gb41 = VData->gb11 * (ax1 - 1.0f) * 0.5f;
			VData->gb22 = 2.0f / (ay1 + ay2);
			VData->gb42 = VData->gb22 * (ay1 - 1.0f) * 0.5f;

			VData->Kgbx1 = 0.5f * (1.0f - ax1);
			VData->Kgbx2 = 0.5f * (1.0f + ax2);
			VData->Kgby1 = 0.5f * (1.0f - ay1);
			VData->Kgby2 = 0.5f * (1.0f + ay2);

      for (i = 0; i < SOA_SIZE; i++) {
          pRc->tl.pTL->gb11.f[i] = VData->gb11;
          pRc->tl.pTL->gb22.f[i] = VData->gb22;
          pRc->tl.pTL->gb41.f[i] = VData->gb41;
          pRc->tl.pTL->gb42.f[i] = VData->gb42;
      }
		}
		else
		{
			VData->minXgb = VData->minX;
			VData->maxXgb = VData->maxX;
			VData->minYgb = VData->minY;
			VData->maxYgb = VData->maxY;
		}
	}

	// Update Mproj*Mclip
	if( pRc->tl.dwDirtyFlags &  TLPV_DIRTY_PROJXFM )
	{
		D3DMATRIX MShift;
		ZeroMemory (&MShift, sizeof(D3DMATRIX));
		MShift._11 = 0.5f;
		MShift._22 = 0.5f;
		MShift._41 = 0.5f;
		MShift._42 = 0.5f;
		MShift._44 = 1.0f;
		MShift._33 = 1.0f;

		MatrixProduct(&TData->m_PS, &pRc->tl.xfmProj, &MShift);
	}

	// Update Mview*Mproj*Mclip
	if( pRc->tl.dwDirtyFlags & (TLPV_DIRTY_VIEWXFM | TLPV_DIRTY_PROJXFM) )
	{
		MatrixProduct(&TData->m_VPS, &pRc->tl.xfmView, &TData->m_PS);
		Inverse4x4( (D3DMATRIX *)&TData->m_VPS, (D3DMATRIX *)&TData->m_VPSInv );
	}

	for( i=0; i< TLMAX_WORLD_MATRICES; i++)
	{
		MatrixProduct(pRc->tl.lpxfmCurrent[i], &pRc->tl.xfmWorld[i],   &TData->m_VPS);
	}

#if defined(SSECPP)
  SwizzleMatrix(pRc->tl.lpxfmCurrent[0], pRc->tl.lpxfmCurrentSOA);
#endif

	// Compute xfmToEye (world*view) matrix (needed for lighting and fog)
	// if needed
	if (pRc->tl.dwDirtyFlags & (TLPV_DIRTY_VIEWXFM  |
														TLPV_DIRTY_WORLDXFM |
														TLPV_DIRTY_WORLD1XFM |
														TLPV_DIRTY_WORLD2XFM |
														TLPV_DIRTY_WORLD3XFM ))
	{
		pRc->tl.dwDirtyFlags |= TLPV_DIRTY_INVERSEWORLDVIEW;
	}

	if (pRc->tl.dwTLState & (TLPV_DOLIGHTING | TLPV_DOFOG | 
												TLPV_DOPASSEYENORMAL | TLPV_DOPASSEYEXYZ) &&
												(pRc->tl.dwDirtyFlags & TLPV_DIRTY_INVERSEWORLDVIEW))
	{
		for( i=0; i< TLMAX_WORLD_MATRICES; i++)
		{
			MatrixProduct(pRc->tl.lpxfmToEye[i], &pRc->tl.xfmWorld[i], &pRc->tl.xfmView);
			Inverse4x4((D3DMATRIX *)pRc->tl.lpxfmToEye[i],
								 (D3DMATRIX *)pRc->tl.lpxfmToEyeInv[i]);
			pRc->tl.dwDirtyFlags |= TLPV_DIRTY_NEEDXFMLIGHT;
		}
    // Create Transpose of matrix
    pRc->tl.lpxfmToEyeInvT->_11 = pRc->tl.lpxfmToEyeInv[0]->_11;
    pRc->tl.lpxfmToEyeInvT->_22 = pRc->tl.lpxfmToEyeInv[0]->_22;
    pRc->tl.lpxfmToEyeInvT->_33 = pRc->tl.lpxfmToEyeInv[0]->_33;
    pRc->tl.lpxfmToEyeInvT->_44 = pRc->tl.lpxfmToEyeInv[0]->_44;
    pRc->tl.lpxfmToEyeInvT->_12 = pRc->tl.lpxfmToEyeInv[0]->_21;
    pRc->tl.lpxfmToEyeInvT->_21 = pRc->tl.lpxfmToEyeInv[0]->_12;
    pRc->tl.lpxfmToEyeInvT->_13 = pRc->tl.lpxfmToEyeInv[0]->_31;
    pRc->tl.lpxfmToEyeInvT->_31 = pRc->tl.lpxfmToEyeInv[0]->_13;
    pRc->tl.lpxfmToEyeInvT->_14 = pRc->tl.lpxfmToEyeInv[0]->_41;
    pRc->tl.lpxfmToEyeInvT->_41 = pRc->tl.lpxfmToEyeInv[0]->_14;
    pRc->tl.lpxfmToEyeInvT->_23 = pRc->tl.lpxfmToEyeInv[0]->_32;
    pRc->tl.lpxfmToEyeInvT->_32 = pRc->tl.lpxfmToEyeInv[0]->_23;
    pRc->tl.lpxfmToEyeInvT->_24 = pRc->tl.lpxfmToEyeInv[0]->_42;
    pRc->tl.lpxfmToEyeInvT->_42 = pRc->tl.lpxfmToEyeInv[0]->_24;
    pRc->tl.lpxfmToEyeInvT->_34 = pRc->tl.lpxfmToEyeInv[0]->_43;
    pRc->tl.lpxfmToEyeInvT->_43 = pRc->tl.lpxfmToEyeInv[0]->_34;

#if defined(SSECPP)
    SwizzleMatrix(pRc->tl.lpxfmToEye[0], pRc->tl.lpxfmToEyeSOA);
    SwizzleMatrix(pRc->tl.lpxfmToEyeInvT, pRc->tl.lpxfmToEyeInvTSOA);
#endif

		pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_INVERSEWORLDVIEW;
	}

	// Clear the dirty transform flags
	pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_XFORM;
	return hr;
}

#endif
#endif

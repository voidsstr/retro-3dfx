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
#endif //WINNT

#define NEW_ASM_XFORM_CODE

#include "dxins.h"

//**********************************
// Profiling code -- see if the 
// matrices are sparse!
//*********************************
//#define PROFILE_SPARSE_MATRIX
#ifdef PROFILE_SPARSE_MATRIX

// the global variable mprofctrl controls the accumulation and display of output
// mprofctrl = 0 means reset all accumulated data and start accumulating new data. 
// mprofctrl = 2 means to dump all currently accumulated data, and reset back to zero.

// The way you use this is set profctrl to zero in the debugger. Run the benchmark. 
// After the benchmark run ends, break into the debugger, and set profctrl = 2. Then start the benchmark again.
// hit the escape key to terminate the benchmark, and save the log file in the winice symbol loader app. 
// When you want to start profiling again, simply set profctrl to 0 prior to running the app.
// 

DWORD mprofctrl = 0;

DWORD sparse_curr_matrix0[65536];
DWORD sparse_eye_matrix0[512];
DWORD sparse_eyeinv_matrix0[512];

DWORD sparse_curr_matrix1[65536];
DWORD sparse_eye_matrix1[512];
DWORD sparse_eyeinv_matrix1[512];

void mat_gather_stats(D3DMATRIX *curr, D3DMATRIX *eye, D3DMATRIX *inveye);
#endif //PROFILE_SPARSE_MATRIX


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
	{
      if (FALSE == pRc->bSBRecMode)
	  {
	    if(memcmp(pMat, &(pRc->tl.xfmWorld[0]), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
        {
	      D3DPRINT(D3DDBGLVL," loading new world transform");
      	  memcpy(&(pRc->tl.xfmWorld[0]), pMat, sizeof(D3DMATRIX));
      	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLDXFM;
		}
	  }
	  else
	  {
	    D3DPRINT(D3DDBGLVL,"Storing world transform from %lXh", (DWORD)pMat );
      	memcpy(&pRc->pCurrSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD], pMat, sizeof(D3DMATRIX));
		SET_STATEBLOCK_XFORM_FLAG( pRc->pCurrSB, TLTRANSFORMSTATE_WORLD );
	  }
      break;
	}

    case D3DTRANSFORMSTATE_VIEW:
	{
      if (FALSE == pRc->bSBRecMode)
      {
	    if(memcmp(pMat, &(pRc->tl.xfmView), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
		{
	      D3DPRINT(D3DDBGLVL," loading new viewport transform");
      	  memcpy(&(pRc->tl.xfmView), pMat, sizeof(D3DMATRIX));
      	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VIEWXFM;\
		}
	  }
	  else
	  {
	    D3DPRINT(D3DDBGLVL,"Storing viewport transform from %lXh", (DWORD)pMat );
      	memcpy(&pRc->pCurrSB->uc.TransformationStates[TLTRANSFORMSTATE_VIEW], pMat, sizeof(D3DMATRIX));
		SET_STATEBLOCK_XFORM_FLAG( pRc->pCurrSB, TLTRANSFORMSTATE_VIEW );
	  }
      break;
	}

    case D3DTRANSFORMSTATE_PROJECTION:
	{
      if (FALSE == pRc->bSBRecMode)
      {
	    if(memcmp(pMat, &(pRc->tl.xfmProj), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
		{
		  D3DPRINT(D3DDBGLVL," loading new projection transform");
      	  memcpy(&(pRc->tl.xfmProj), pMat, sizeof(D3DMATRIX));
      	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_PROJXFM;
		}
	  }
	  else
	  {
	    D3DPRINT(D3DDBGLVL,"Storing projection transform from %lXh", (DWORD)pMat );
      	memcpy(&pRc->pCurrSB->uc.TransformationStates[TLTRANSFORMSTATE_PROJ], pMat, sizeof(D3DMATRIX));
		SET_STATEBLOCK_XFORM_FLAG( pRc->pCurrSB, TLTRANSFORMSTATE_PROJ );
	  }
      break;
	}

    case D3DTRANSFORMSTATE_WORLD1:
	{
      if (FALSE == pRc->bSBRecMode)
      {
	    if(memcmp(pMat, &(pRc->tl.xfmWorld[1]), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
        {
	      D3DPRINT(D3DDBGLVL," loading new world1 transform");
      	  memcpy(&(pRc->tl.xfmWorld[1]), pMat, sizeof(D3DMATRIX));
      	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLD1XFM;
		}
	  }
	  else
	  {
	    D3DPRINT(D3DDBGLVL,"Storing world1 transform from %lXh", (DWORD)pMat );
      	memcpy(&pRc->pCurrSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD1], pMat, sizeof(D3DMATRIX));
		SET_STATEBLOCK_XFORM_FLAG( pRc->pCurrSB, TLTRANSFORMSTATE_WORLD1 );
	  }
      break;
	}

    case D3DTRANSFORMSTATE_WORLD2:
	{
      if (FALSE == pRc->bSBRecMode)
      {
	    if(memcmp(pMat, &(pRc->tl.xfmWorld[2]), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
        {
	      D3DPRINT(D3DDBGLVL," loading new world2 transform");
      	  memcpy(&(pRc->tl.xfmWorld[2]), pMat, sizeof(D3DMATRIX));
      	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLD2XFM;
		}
	  }
	  else
	  {
	    D3DPRINT(D3DDBGLVL,"Storing world2 transform from %lXh", (DWORD)pMat );
      	memcpy(&pRc->pCurrSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD2], pMat, sizeof(D3DMATRIX));
		SET_STATEBLOCK_XFORM_FLAG( pRc->pCurrSB, TLTRANSFORMSTATE_WORLD2 );
	  }
      break;
	}

    case D3DTRANSFORMSTATE_WORLD3:
	{
      if (FALSE == pRc->bSBRecMode)
      {
	    if(memcmp(pMat, &(pRc->tl.xfmWorld[3]), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
        {
	      D3DPRINT(D3DDBGLVL," loading new world3 transform");
      	  memcpy(&(pRc->tl.xfmWorld[3]), pMat, sizeof(D3DMATRIX));
      	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLD3XFM;
		}
	  }
	  else
	  {
	    D3DPRINT(D3DDBGLVL,"Storing world3 transform from %lXh", (DWORD)pMat );
      	memcpy(&pRc->pCurrSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD3], pMat, sizeof(D3DMATRIX));
		SET_STATEBLOCK_XFORM_FLAG( pRc->pCurrSB, TLTRANSFORMSTATE_WORLD3 );
	  }
      break;
	}

    case D3DTRANSFORMSTATE_TEXTURE0:
    case D3DTRANSFORMSTATE_TEXTURE1:
    case D3DTRANSFORMSTATE_TEXTURE2:
    case D3DTRANSFORMSTATE_TEXTURE3:
    case D3DTRANSFORMSTATE_TEXTURE4:
    case D3DTRANSFORMSTATE_TEXTURE5:
    case D3DTRANSFORMSTATE_TEXTURE6:
    case D3DTRANSFORMSTATE_TEXTURE7:
	{
	  DWORD xfmTxtrNum = dwxfrmType - D3DTRANSFORMSTATE_TEXTURE0;	// should be 0-7
      if (FALSE == pRc->bSBRecMode)
      {
	    if(memcmp(pMat, &(pRc->tl.xfmTxtr[xfmTxtrNum]), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
        {
	      D3DPRINT(D3DDBGLVL," loading new texture transform %d", xfmTxtrNum);
      	  memcpy(&(pRc->tl.xfmTxtr[xfmTxtrNum]), pMat, sizeof(D3DMATRIX));
      	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_TXTRXFM;
		  pRc->tl.dwDirtyXfmTxtr |= (1 << xfmTxtrNum);
		}
	  }
	  else
	  {
	    D3DPRINT(D3DDBGLVL,"Storing texture transform %d from %lXh", xfmTxtrNum, (DWORD)pMat );
      	memcpy(&pRc->pCurrSB->uc.TransformationStates[TLTRANSFORMSTATE_TEX0+xfmTxtrNum], pMat, sizeof(D3DMATRIX));
		SET_STATEBLOCK_XFORM_FLAG( pRc->pCurrSB, TLTRANSFORMSTATE_TEX0+xfmTxtrNum );
	  }
      break;
	}

    default:
	  D3DPRINT(D3DDBGLVL,"ERROR: Ignoring Unknown TransformType %lXh", dwxfrmType );
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
int Inverse4x4(D3DMATRIX *src, D3DMATRIX *dst)
{
    float rcp;
    float a11, a12, a13, a22, a23, a33;
    float b11, b12, b13, b22, b23, b33;
	D3DMATRIX	d, s;


    /* read 1st two columns of matrix into registers */
    s._11 = src->_11;
    s._12 = src->_12;
    s._21 = src->_21;
    s._22 = src->_22;
    s._31 = src->_31;
    s._32 = src->_32;
    s._41 = src->_41;
    s._42 = src->_42;

    /* compute all six 2x2 determinants of 1st two columns */
    a11 = s._11*s._22 - s._12*s._21;
    a12 = s._11*s._32 - s._12*s._31;
    a13 = s._11*s._42 - s._12*s._41;
    a22 = s._21*s._32 - s._22*s._31;
    a23 = s._21*s._42 - s._22*s._41;
    a33 = s._31*s._42 - s._32*s._41;

    /* read 2nd two columns of matrix into registers */
    s._13 = src->_13;
    s._14 = src->_14;
    s._23 = src->_23;
    s._24 = src->_24;
    s._33 = src->_33;
    s._34 = src->_34;
    s._43 = src->_43;
    s._44 = src->_44;

    /* compute all 3x3 cofactors for 4th column */
    d._44 = s._13*a22 - s._23*a12 + s._33*a11;		// _11, _12, _13, _21, _22, _23, _31, _32, _33
    d._43 = s._23*a13 - s._43*a11 - s._13*a23;		// _11, _12, _13, _21, _22, _23, _41, _42, _43
    d._42 = s._13*a33 - s._33*a13 + s._43*a12;		// _11, _12, _13, _31, _32, _33, _41, _42, _43
    d._41 = s._33*a23 - s._43*a22 - s._23*a33;		// _21, _22, _23, _31, _32, _33, _41, _42, _43

    /* compute all 3x3 cofactors for 3rd column */
    d._34 = s._24*a12 - s._34*a11 - s._14*a22;		// _11, _12, _14, _21, _22, _24, _31, _32, _34
    d._33 = s._14*a23 - s._24*a13 + s._44*a11;		// _11, _12, _14, _21, _22, _24, _41, _42, _44
    d._32 = s._34*a13 - s._44*a12 - s._14*a33;		// _11, _12, _14, _31, _32, _34, _41, _42, _44
    d._31 = s._24*a33 - s._34*a23 + s._44*a22;		// _21, _22, _24, _31, _32, _34, _41, _42, _44

    /* compute all six 2x2 determinants of 2nd two columns */
    b11 = s._13*s._24 - s._14*s._23;
    b12 = s._13*s._34 - s._14*s._33;
    b13 = s._13*s._44 - s._14*s._43;
    b22 = s._23*s._34 - s._24*s._33;
    b23 = s._23*s._44 - s._24*s._43;
    b33 = s._33*s._44 - s._34*s._43;

    /* compute all 3x3 cofactors for 1st column */
    d._14 = s._22*b12 - s._32*b11 - s._12*b22;		// _12, _13, _14, _22, _23, _24, _32, _33, _34
    d._13 = s._12*b23 - s._22*b13 + s._42*b11;		// _12, _13, _14, _22, _23, _24, _42, _43, _44
    d._12 = s._32*b13 - s._42*b12 - s._12*b33;		// _12, _13, _14, _32, _33, _34, _42, _43, _44
    d._11 = s._22*b33 - s._32*b23 + s._42*b22;		// _22, _23, _24, _32, _33, _34, _42, _43, _44

    /* compute 4x4 determinant & its reciprocal */
    rcp = s._41*d._14 + s._31*d._13 + s._21*d._12 + s._11*d._11;
    if (rcp == 0)
	    return -1;
    rcp = 1.0f/rcp;

    /* multiply 1st column 3x3 cofactors by reciprocal */
    dst->_11 = d._11*rcp;
    dst->_12 = d._12*rcp;
    dst->_13 = d._13*rcp;
    dst->_14 = d._14*rcp;

    /* compute all 3x3 cofactors for 2nd column */
    d._24 = s._11*b22 - s._21*b12 + s._31*b11;		// _11, _13, _14, _21, _23, _24, _31, _33, _34
    d._23 = s._21*b13 - s._41*b11 - s._11*b23;		// _11, _13, _14, _21, _23, _24, _41, _43, _44
    d._22 = s._11*b33 - s._31*b13 + s._41*b12;		// _11, _13, _14, _31, _33, _34, _41, _43, _44
    d._21 = s._31*b23 - s._41*b22 - s._21*b33;		// _21, _23, _24, _31, _33, _34, _41, _43, _44

    /* multiply 2nd, 3rd, and 4th column 3x3 cofactors by reciprocal */
    dst->_21 = d._21*rcp;
    dst->_22 = d._22*rcp;
    dst->_23 = d._23*rcp;
    dst->_24 = d._24*rcp;
    dst->_31 = d._31*rcp;
    dst->_32 = d._32*rcp;
    dst->_33 = d._33*rcp;
    dst->_34 = d._34*rcp;
    dst->_41 = d._41*rcp;
    dst->_42 = d._42*rcp;
    dst->_43 = d._43*rcp;
    dst->_44 = d._44*rcp;

    return 0;
}

//---------------------------------------------------------------------
#define MATRIX_PRODUCT(a, b, res)                                           \
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


_inline void TransposeMatrix_C(D3DMATRIX *src, D3DMATRIX *dst)
{
	dst->_11 = src->_11;	dst->_21 = src->_12;	dst->_31 = src->_13;	dst->_41 = src->_14;
	dst->_12 = src->_21;	dst->_22 = src->_22;	dst->_32 = src->_23;	dst->_42 = src->_24;
	dst->_13 = src->_31;	dst->_23 = src->_32;	dst->_33 = src->_33;	dst->_43 = src->_34;
	dst->_14 = src->_41;	dst->_24 = src->_42;	dst->_34 = src->_43;	dst->_44 = src->_44;
}
_inline void TransposeMatrix_KNI_Asm(D3DMATRIX *src, D3DMATRIX *dst)
{
  _asm {
	mov		eax, src
	mov		edx, dst
	movlps	xmm0, [eax+0x00]	//	---  ---  s12  s11
	movhps	xmm0, [eax+0x10]	//	s22  s21  s12  s11
	movlps	xmm1, [eax+0x08]	//	---  ---  s14  s13
	movhps	xmm1, [eax+0x18]	//	s24  s23  s14  s13
	movlps	xmm2, [eax+0x20]	//	---  ---  s32  s31
	movhps	xmm2, [eax+0x30]	//	s42  s41  s32  s31
	shufps	xmm0, xmm0, 0xd8	//	s22  s12  s21  s11
	movlps	xmm3, [eax+0x28]	//	---  ---  s34  s33
	shufps	xmm1, xmm1, 0xd8	//	s24  s14  s23  s13
	movhps	xmm3, [eax+0x38]	//	s44  s43  s34  s33
	movlps	[edx+0x00], xmm0	//	d12=s21,  d11=s11
	movhps	[edx+0x10], xmm0	//	d32=s22,  d31=s12
	shufps	xmm2, xmm2, 0xd8	//	s42  s32  s41  s31
	movlps	[edx+0x20], xmm1	//	d22=s23,  d21=s13
	movhps	[edx+0x30], xmm1	//	d42=s24,  d41=s14
	shufps	xmm3, xmm3, 0xd8	//	s44  s34  s43  s33
	movlps	[edx+0x08], xmm2	//	d14=s41,  d13=s31
	movhps	[edx+0x18], xmm2	//	d34=s42,  d33=s32
	movlps	[edx+0x28], xmm3	//	d24=s43,  d23=s33
	movhps	[edx+0x38], xmm3	//	d44=s44,  d43=s34
  }
}


//---------------------------------------------------------------------
// result = a*b
// result is the same as a or b
void MatrixProduct2(D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX *result)
{
    D3DMATRIX res;	// needs to be 8-byte aligned
    MATRIX_PRODUCT(a, b, (&res));
    *(D3DMATRIX*)result = res;
}

//---------------------------------------------------------------------
// result = a*b.
// check "result" pointer - could be equal to "a" or "b"
void MatrixProduct(D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX *result)
{
    if (result == a || result == b)
    {
        MatrixProduct2(a, b, result);
        return;
    }
    MATRIX_PRODUCT(a, b, result);
}


//---------------------------------------------------------------------
// result = a*b
// result is the same as a or b
#if defined(VCPP)
void MatrixProduct2_KNI(D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX *result)
{
    D3DMATRIX res;	// needs to be 8-byte aligned
	MatrixProduct_KNI_Asm(a, b, (&res));
    *(D3DMATRIX*)result = res;
}

//---------------------------------------------------------------------
// result = a*b.
// check "result" pointer - could be equal to "a" or "b"
_inline void MatrixProduct_KNI(D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX *result)
{
	if (result == a)	// only check "a", "b" is kept in KNI registers
        MatrixProduct2_KNI(a, b, result);
	else
        MatrixProduct_KNI_Asm(a, b, result);
}
#endif


//---------------------------------------------------------------------
// result = a*b
// result is the same as a or b
void MatrixProduct2_K3d(D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX *result)
{
    D3DMATRIX res;	// needs to be 8-byte aligned
	MatrixProduct_K3d_Asm(a, b, (&res));
    *(D3DMATRIX*)result = res;
}

//---------------------------------------------------------------------
// result = a*b.
// check "result" pointer - could be equal to "a" or "b"
_inline void MatrixProduct_K3d(D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX *result)
{
	if (result == a)	// only check "a", "b" is kept in KNI registers
        MatrixProduct2_K3d(a, b, result);
	else
        MatrixProduct_K3d_Asm(a, b, result);
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

HRESULT UpdateXformData(RC *pRc, DWORD dwXfmBlendMask)
{
	HRESULT hr = D3D_OK;
	TLVIEWPORTDATA  *VData = &pRc->tl.ViewData;
	TLTRANSFORMDATA *TData = &pRc->tl.TransformData;
	DWORD dwDirtyFlags = pRc->tl.dwDirtyFlags;


#ifdef TNL_PROFILE
    MatrixStats(0L); // Count this as an update JJP
#endif //TNL_PROFILE

	// If Dirty Dynamic GB flag, look at the current number of outstanding queued frames
	// that have yet to be rendered. Decide to adjust the T&L HAL's clip area
	// depending on the number of outstanding flips.
	if((dwDirtyFlags & (TLPV_DIRTY_DYNGB | TLPV_DIRTY_VIEWRECT)) && (pRc->tl.dwTLState & TLPV_GUARDBAND))
	{
	  switch ( pRc->tl.dwFramesQueued )
	  {
		// We are not HW bound, set GB to Napalm MAX
	    case 0:
		   pRc->tl.ViewData.minXgb = -4095.0f;
           pRc->tl.ViewData.maxXgb =  4095.0f;
           pRc->tl.ViewData.minYgb = -4095.0f;
           pRc->tl.ViewData.maxYgb =  4095.0f;
		   break;
	    // Got 1 outstanding, shorten down to a 200 pixel GB
		case 1:
		   pRc->tl.ViewData.minXgb = (D3DVALUE)((int)pRc->tl.Viewport.dwX) - 200.0f;
           pRc->tl.ViewData.maxXgb = (D3DVALUE)((int)(pRc->tl.Viewport.dwX + pRc->tl.Viewport.dwWidth)) + 200.0f;
           pRc->tl.ViewData.minYgb = (D3DVALUE)((int)pRc->tl.Viewport.dwY) - 200.0f;
           pRc->tl.ViewData.maxYgb = (D3DVALUE)((int)(pRc->tl.Viewport.dwY + pRc->tl.Viewport.dwHeight)) + 200.0f;
		   break;
	    // Got 2 outstanding, shorten down to a 100 pixel GB
		case 2:
		   pRc->tl.ViewData.minXgb = (D3DVALUE)((int)pRc->tl.Viewport.dwX) - 100.0f;
           pRc->tl.ViewData.maxXgb = (D3DVALUE)((int)(pRc->tl.Viewport.dwX + pRc->tl.Viewport.dwWidth)) + 100.0f;
           pRc->tl.ViewData.minYgb = (D3DVALUE)((int)pRc->tl.Viewport.dwY) - 100.0f;
           pRc->tl.ViewData.maxYgb = (D3DVALUE)((int)(pRc->tl.Viewport.dwY + pRc->tl.Viewport.dwHeight)) + 100.0f;
		   break;
	    // Got 3 or more outstanding, shorten down to a 25 pixel GB
		default:
		   pRc->tl.ViewData.minXgb = (D3DVALUE)((int)pRc->tl.Viewport.dwX) - 25.0f;
           pRc->tl.ViewData.maxXgb = (D3DVALUE)((int)(pRc->tl.Viewport.dwX + pRc->tl.Viewport.dwWidth)) + 25.0f;
           pRc->tl.ViewData.minYgb = (D3DVALUE)((int)pRc->tl.Viewport.dwY) - 25.0f;
           pRc->tl.ViewData.maxYgb = (D3DVALUE)((int)(pRc->tl.Viewport.dwY + pRc->tl.Viewport.dwHeight)) + 25.0f;
	  }
	}


	/* Update from viewport information */
	if (dwDirtyFlags & TLPV_DIRTY_ZRANGE)
	{
		VData->scaleZ  = pRc->tl.Viewport.dvMaxZ - pRc->tl.Viewport.dvMinZ;
		VData->offsetZ = pRc->tl.Viewport.dvMinZ;
		/* BUGBUG: This could be a Divide by Zero here if */
		/* the dvMaxZ == dvMinZ. Fix it later.            */
		VData->scaleZi = D3DVAL(1) / VData->scaleZ;
	}

	/* Update viewport information */
	if (dwDirtyFlags & TLPV_DIRTY_VIEWRECT)
	{
		// Bail if we are going to cause any divide by zero exceptions.
		// The likely reason is that we have a bogus viewport set by
		// TLVertex execute buffer app.
		if(pRc->tl.Viewport.dwWidth == 0 || pRc->tl.Viewport.dwHeight == 0 )
			hr = DDERR_GENERIC;

		VData->dvX = (D3DVALUE)((int)pRc->tl.Viewport.dwX);
		VData->dvY = (D3DVALUE)((int)pRc->tl.Viewport.dwY);
		VData->dvWidth = (D3DVALUE)((int)pRc->tl.Viewport.dwWidth);
		VData->dvHeight = (D3DVALUE)((int)pRc->tl.Viewport.dwHeight);

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
			D3DVALUE ax1 = -(VData->minXgb - VData->dvX) * w + 1.0f;
			D3DVALUE ax2 =  (VData->maxXgb - VData->dvX) * w - 1.0f;
			D3DVALUE ay1 =  (VData->maxYgb - VData->dvY) * h - 1.0f;
			D3DVALUE ay2 = -(VData->minYgb - VData->dvY) * h + 1.0f;
			VData->gb11 = 2.0f / (ax1 + ax2);
			VData->gb41 = VData->gb11 * (ax1 - 1.0f) * 0.5f;
			VData->gb22 = 2.0f / (ay1 + ay2);
			VData->gb42 = VData->gb22 * (ay1 - 1.0f) * 0.5f;

			VData->Kgbx1 = 0.5f * (1.0f - ax1);
			VData->Kgbx2 = 0.5f * (1.0f + ax2);
			VData->Kgby1 = 0.5f * (1.0f - ay1);
			VData->Kgby2 = 0.5f * (1.0f + ay2);

#if defined(VCPP)
			if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
			{
				TL_SOATMP *pTL = pRc->tl.pTL;
				SwizzleScaler2SOAFLOAT( VData->gb11, &pTL->fSOAgb11 );
				SwizzleScaler2SOAFLOAT( VData->gb22, &pTL->fSOAgb22 );
				SwizzleScaler2SOAFLOAT( VData->gb41, &pTL->fSOAgb41 );
				SwizzleScaler2SOAFLOAT( VData->gb42, &pTL->fSOAgb42 );
			}
			else
#endif
			if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOW)
			{
				// K3D_TMP copies are guaranteed to be 8-byte aligned
		        TL_K3DTMP *pTLK = pRc->tl.pTLK;
			    pTLK->gb11 = VData->gb11;
			    pTLK->gb22 = VData->gb22;
			    pTLK->gb41 = VData->gb41;
			    pTLK->gb42 = VData->gb42;
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
	if( dwDirtyFlags &  TLPV_DIRTY_PROJXFM )
	{
#if 0
		D3DMATRIX MShift;	// needs to be 8-byte aligned
		MShift._11 = 0.5f;	MShift._12 = 0.0f;	MShift._13 = 0.0f;	MShift._14 = 0.0f;
		MShift._21 = 0.0f;	MShift._22 = 0.5f;	MShift._23 = 0.0f;	MShift._24 = 0.0f;
		MShift._31 = 0.0f;	MShift._32 = 0.0f;	MShift._33 = 1.0f;	MShift._34 = 0.0f;
		MShift._41 = 0.5f;	MShift._42 = 0.5f;	MShift._43 = 0.0f;	MShift._44 = 1.0f;
		MatrixProduct( &pRc->tl.xfmProj, &MShift, &TData->m_PS );
#else
		// Since MShift is hard-coded we'll hard-code the transform.  I tried just using
		// the MATRIX_PRODUCT macro but the compiler wouldn't take out the *0 and *1 multiplies.
		pRc->tl.TransformData.m_PS._11 = (pRc->tl.xfmProj._11 + pRc->tl.xfmProj._14) * 0.5f;
		pRc->tl.TransformData.m_PS._12 = (pRc->tl.xfmProj._12 + pRc->tl.xfmProj._14) * 0.5f;
		pRc->tl.TransformData.m_PS._13 =  pRc->tl.xfmProj._13; 
		pRc->tl.TransformData.m_PS._14 =  pRc->tl.xfmProj._14;
		pRc->tl.TransformData.m_PS._21 = (pRc->tl.xfmProj._21 + pRc->tl.xfmProj._24) * 0.5f;
		pRc->tl.TransformData.m_PS._22 = (pRc->tl.xfmProj._22 + pRc->tl.xfmProj._24) * 0.5f;
		pRc->tl.TransformData.m_PS._23 =  pRc->tl.xfmProj._23; 
		pRc->tl.TransformData.m_PS._24 =  pRc->tl.xfmProj._24;
		pRc->tl.TransformData.m_PS._31 = (pRc->tl.xfmProj._31 + pRc->tl.xfmProj._34) * 0.5f;
		pRc->tl.TransformData.m_PS._32 = (pRc->tl.xfmProj._32 + pRc->tl.xfmProj._34) * 0.5f;
		pRc->tl.TransformData.m_PS._33 =  pRc->tl.xfmProj._33; 
		pRc->tl.TransformData.m_PS._34 =  pRc->tl.xfmProj._34;
		pRc->tl.TransformData.m_PS._41 = (pRc->tl.xfmProj._41 + pRc->tl.xfmProj._44) * 0.5f;
		pRc->tl.TransformData.m_PS._42 = (pRc->tl.xfmProj._42 + pRc->tl.xfmProj._44) * 0.5f;
		pRc->tl.TransformData.m_PS._43 =  pRc->tl.xfmProj._43; 
		pRc->tl.TransformData.m_PS._44 =  pRc->tl.xfmProj._44;
#endif
	}

	// Update Mview*Mproj*Mclip
	if( dwDirtyFlags & (TLPV_DIRTY_VIEWXFM | TLPV_DIRTY_PROJXFM) )
	{
#if defined(VCPP)
        if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
		{
			MatrixProduct_KNI( &pRc->tl.xfmView, &TData->m_PS, &TData->m_VPS );
			Inverse4x4_KNI_Asm( &TData->m_VPS, &TData->m_VPSInv );	// used by clipping code
		}
		else
#endif
		{
			if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOW)
 			  	MatrixProduct_K3d( &pRc->tl.xfmView, &TData->m_PS, &TData->m_VPS );
			else
 			  	MatrixProduct( &pRc->tl.xfmView, &TData->m_PS, &TData->m_VPS );
			Inverse4x4( &TData->m_VPS, &TData->m_VPSInv );	// used by clipping code
		}
		dwDirtyFlags |= TLPV_DIRTY_WORLDXFM_ALL;
	}

	// Update Mview*Mproj*Mclip*World[i]
	if( dwDirtyFlags & (TLPV_DIRTY_VIEWXFM | TLPV_DIRTY_PROJXFM | TLPV_DIRTY_WORLDXFM_ALL) )
	{
	    DWORD dwDirtyWorldXforms = dwDirtyFlags & dwXfmBlendMask;	// we don't care about blends that we're not doing
#if defined(VCPP)
		if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
		{
			if ( dwDirtyWorldXforms & TLPV_DIRTY_WORLDXFM )
			{
				dwDirtyFlags |= TLPV_DIRTY_EYEXFM;
				MatrixProduct_KNI( &pRc->tl.xfmWorld[0], &TData->m_VPS, pRc->tl.lpxfmCurrent[0] );
				SwizzleMatrix(pRc->tl.lpxfmCurrent[0], pRc->tl.lpSOAxfmCurrent[0]);
			}
			if ( dwDirtyWorldXforms & TLPV_DIRTY_WORLD1XFM )
			{
				dwDirtyFlags |= TLPV_DIRTY_EYE1XFM;											   
				MatrixProduct_KNI( &pRc->tl.xfmWorld[1], &TData->m_VPS, pRc->tl.lpxfmCurrent[1] );
				SwizzleMatrix(pRc->tl.lpxfmCurrent[1], pRc->tl.lpSOAxfmCurrent[1]);
			}
			if ( dwDirtyWorldXforms & TLPV_DIRTY_WORLD2XFM )
			{
				dwDirtyFlags |= TLPV_DIRTY_EYE2XFM;
				MatrixProduct_KNI( &pRc->tl.xfmWorld[2], &TData->m_VPS, pRc->tl.lpxfmCurrent[2] );
				SwizzleMatrix(pRc->tl.lpxfmCurrent[2], pRc->tl.lpSOAxfmCurrent[2]);
			}
			if ( dwDirtyWorldXforms & TLPV_DIRTY_WORLD3XFM )
			{
				dwDirtyFlags |= TLPV_DIRTY_EYE3XFM;
				MatrixProduct_KNI( &pRc->tl.xfmWorld[3], &TData->m_VPS, pRc->tl.lpxfmCurrent[3] );
				SwizzleMatrix(pRc->tl.lpxfmCurrent[3], pRc->tl.lpSOAxfmCurrent[3]);
			}
		}
		else
#endif
		if (pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOW)
		{
		    TL_K3DTMP *pTLK = pRc->tl.pTLK;
			if ( dwDirtyWorldXforms & TLPV_DIRTY_WORLDXFM )
			{
				dwDirtyFlags |= TLPV_DIRTY_EYEXFM;
				MatrixProduct_K3d( &pRc->tl.xfmWorld[0], &TData->m_VPS, pRc->tl.lpxfmCurrent[0] );
				TransposeMatrix_C( pRc->tl.lpxfmCurrent[0], &pTLK->xfmCurrentT[0]);
			}
			if ( dwDirtyWorldXforms & TLPV_DIRTY_WORLD1XFM )
			{
				dwDirtyFlags |= TLPV_DIRTY_EYE1XFM;
				MatrixProduct_K3d( &pRc->tl.xfmWorld[1], &TData->m_VPS, pRc->tl.lpxfmCurrent[1] );
				TransposeMatrix_C( pRc->tl.lpxfmCurrent[1], &pTLK->xfmCurrentT[1]);
			}
			if ( dwDirtyWorldXforms & TLPV_DIRTY_WORLD2XFM )
			{
				dwDirtyFlags |= TLPV_DIRTY_EYE2XFM;
				MatrixProduct_K3d( &pRc->tl.xfmWorld[2], &TData->m_VPS, pRc->tl.lpxfmCurrent[2] );
				TransposeMatrix_C( pRc->tl.lpxfmCurrent[2], &pTLK->xfmCurrentT[2]);
			}
			if ( dwDirtyWorldXforms & TLPV_DIRTY_WORLD3XFM )
			{
				dwDirtyFlags |= TLPV_DIRTY_EYE3XFM;
				MatrixProduct_K3d( &pRc->tl.xfmWorld[3], &TData->m_VPS, pRc->tl.lpxfmCurrent[3] );
				TransposeMatrix_C( pRc->tl.lpxfmCurrent[3], &pTLK->xfmCurrentT[3]);
			}
		}
		else
		{
			// Default C path
			if ( dwDirtyWorldXforms & TLPV_DIRTY_WORLDXFM )
			{
				dwDirtyFlags |= TLPV_DIRTY_EYEXFM;
				MatrixProduct( &pRc->tl.xfmWorld[0], &TData->m_VPS, pRc->tl.lpxfmCurrent[0] );
			}
			if ( dwDirtyWorldXforms & TLPV_DIRTY_WORLD1XFM )
			{
				dwDirtyFlags |= TLPV_DIRTY_EYE1XFM;
				MatrixProduct( &pRc->tl.xfmWorld[1], &TData->m_VPS, pRc->tl.lpxfmCurrent[1] );
			}
			if ( dwDirtyWorldXforms & TLPV_DIRTY_WORLD2XFM )
			{
				dwDirtyFlags |= TLPV_DIRTY_EYE2XFM;
				MatrixProduct( &pRc->tl.xfmWorld[2], &TData->m_VPS, pRc->tl.lpxfmCurrent[2] );
			}
			if ( dwDirtyWorldXforms & TLPV_DIRTY_WORLD3XFM )
			{
				dwDirtyFlags |= TLPV_DIRTY_EYE3XFM;
				MatrixProduct( &pRc->tl.xfmWorld[3], &TData->m_VPS, pRc->tl.lpxfmCurrent[3] );
			}
		}
	}


	// We only update these if we have lighting/fog/etc and even then only update the dirty ones
	if ( (pRc->tl.dwTLState & (TLPV_DOLIGHTING | TLPV_DOFOG | TLPV_DOTEXGEN)) &&
		 (dwDirtyFlags & (TLPV_DIRTY_VIEWXFM | TLPV_DIRTY_EYEXFM_ALL)) )
	{
	    DWORD dwDirtyEyeXforms = dwDirtyFlags & dwXfmBlendMask;	// we don't care about blends that we're not doing
#if defined(VCPP)
		if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
		{
			if ( dwDirtyEyeXforms & TLPV_DIRTY_EYEXFM )
			{
				MatrixProduct_KNI( &pRc->tl.xfmWorld[0], &pRc->tl.xfmView, pRc->tl.lpxfmToEye[0] );
	        	Inverse4x4_KNI_Asm( pRc->tl.lpxfmToEye[0], pRc->tl.lpxfmToEyeInv[0] );
				TransposeMatrix_KNI_Asm( pRc->tl.lpxfmToEyeInv[0], pRc->tl.lpxfmToEyeInvT[0] );
	    		SwizzleMatrix( pRc->tl.lpxfmToEye[0], pRc->tl.lpSOAxfmToEye[0] );
    			SwizzleMatrix( pRc->tl.lpxfmToEyeInvT[0], pRc->tl.lpSOAxfmToEyeInvT[0] );
			}
			if ( dwDirtyEyeXforms & TLPV_DIRTY_EYE1XFM )
			{
				MatrixProduct_KNI( &pRc->tl.xfmWorld[1], &pRc->tl.xfmView, pRc->tl.lpxfmToEye[1] );
	        	Inverse4x4_KNI_Asm( pRc->tl.lpxfmToEye[1], pRc->tl.lpxfmToEyeInv[1] );
				TransposeMatrix_KNI_Asm( pRc->tl.lpxfmToEyeInv[1], pRc->tl.lpxfmToEyeInvT[1] );
	    		SwizzleMatrix( pRc->tl.lpxfmToEye[1], pRc->tl.lpSOAxfmToEye[1] );
    			SwizzleMatrix( pRc->tl.lpxfmToEyeInvT[1], pRc->tl.lpSOAxfmToEyeInvT[1] );
			}
			if ( dwDirtyEyeXforms & TLPV_DIRTY_EYE2XFM )
			{
				MatrixProduct_KNI( &pRc->tl.xfmWorld[2], &pRc->tl.xfmView, pRc->tl.lpxfmToEye[2] );
	        	Inverse4x4_KNI_Asm( pRc->tl.lpxfmToEye[2], pRc->tl.lpxfmToEyeInv[2] );
				TransposeMatrix_KNI_Asm( pRc->tl.lpxfmToEyeInv[2], pRc->tl.lpxfmToEyeInvT[2] );
	    		SwizzleMatrix( pRc->tl.lpxfmToEye[2], pRc->tl.lpSOAxfmToEye[2] );
    			SwizzleMatrix( pRc->tl.lpxfmToEyeInvT[2], pRc->tl.lpSOAxfmToEyeInvT[2] );
			}
			if ( dwDirtyEyeXforms & TLPV_DIRTY_EYE3XFM )
			{
				MatrixProduct_KNI( &pRc->tl.xfmWorld[3], &pRc->tl.xfmView, pRc->tl.lpxfmToEye[3] );
	        	Inverse4x4_KNI_Asm( pRc->tl.lpxfmToEye[3], pRc->tl.lpxfmToEyeInv[3] );
				TransposeMatrix_KNI_Asm( pRc->tl.lpxfmToEyeInv[3], pRc->tl.lpxfmToEyeInvT[3] );
	    		SwizzleMatrix( pRc->tl.lpxfmToEye[3], pRc->tl.lpSOAxfmToEye[3] );
    			SwizzleMatrix( pRc->tl.lpxfmToEyeInvT[3], pRc->tl.lpSOAxfmToEyeInvT[3] );
			}
		}
		else
#endif
		if (pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOW)
		{
		    TL_K3DTMP *pTLK = pRc->tl.pTLK;
			if ( dwDirtyEyeXforms & TLPV_DIRTY_EYEXFM )
			{
				MatrixProduct_K3d( &pRc->tl.xfmWorld[0], &pRc->tl.xfmView, pRc->tl.lpxfmToEye[0] );
				TransposeMatrix_C( pRc->tl.lpxfmToEye[0], &pRc->tl.pTLK->xfmToEyeT[0] );		// this one we only do for 3dnow
        		Inverse4x4( pRc->tl.lpxfmToEye[0], pRc->tl.lpxfmToEyeInv[0] );
				CopyMatrix( pRc->tl.lpxfmToEyeInv[0], &pTLK->lpxfmToEyeInv[0] );
			}
			if ( dwDirtyEyeXforms & TLPV_DIRTY_EYE1XFM )
			{
				MatrixProduct_K3d( &pRc->tl.xfmWorld[1], &pRc->tl.xfmView, pRc->tl.lpxfmToEye[1] );
				TransposeMatrix_C( pRc->tl.lpxfmToEye[1], &pRc->tl.pTLK->xfmToEyeT[1] );
        		Inverse4x4( pRc->tl.lpxfmToEye[1], pRc->tl.lpxfmToEyeInv[1] );
				CopyMatrix( pRc->tl.lpxfmToEyeInv[1], &pTLK->lpxfmToEyeInv[1] );
			}
			if ( dwDirtyEyeXforms & TLPV_DIRTY_EYE2XFM )
			{
				MatrixProduct_K3d( &pRc->tl.xfmWorld[2], &pRc->tl.xfmView, pRc->tl.lpxfmToEye[2] );
				TransposeMatrix_C( pRc->tl.lpxfmToEye[2], &pRc->tl.pTLK->xfmToEyeT[2] );
        		Inverse4x4( pRc->tl.lpxfmToEye[2], pRc->tl.lpxfmToEyeInv[2] );
				CopyMatrix( pRc->tl.lpxfmToEyeInv[2], &pTLK->lpxfmToEyeInv[2] );
			}
			if ( dwDirtyEyeXforms & TLPV_DIRTY_EYE3XFM )
			{
				MatrixProduct_K3d( &pRc->tl.xfmWorld[3], &pRc->tl.xfmView, pRc->tl.lpxfmToEye[3] );
				TransposeMatrix_C( pRc->tl.lpxfmToEye[3], &pRc->tl.pTLK->xfmToEyeT[3] );
        		Inverse4x4( pRc->tl.lpxfmToEye[3], pRc->tl.lpxfmToEyeInv[3] );
				CopyMatrix( pRc->tl.lpxfmToEyeInv[3], &pTLK->lpxfmToEyeInv[3] );
			}
		}
		else
		{
			// Default C path
			if ( dwDirtyEyeXforms & TLPV_DIRTY_EYEXFM )
			{
				MatrixProduct( &pRc->tl.xfmWorld[0], &pRc->tl.xfmView, pRc->tl.lpxfmToEye[0] );
        		Inverse4x4( pRc->tl.lpxfmToEye[0], pRc->tl.lpxfmToEyeInv[0] );
			}
			if ( dwDirtyEyeXforms & TLPV_DIRTY_EYE1XFM )
			{
				MatrixProduct( &pRc->tl.xfmWorld[1], &pRc->tl.xfmView, pRc->tl.lpxfmToEye[1] );
        		Inverse4x4( pRc->tl.lpxfmToEye[1], pRc->tl.lpxfmToEyeInv[1] );
			}
			if ( dwDirtyEyeXforms & TLPV_DIRTY_EYE2XFM )
			{
				MatrixProduct( &pRc->tl.xfmWorld[2], &pRc->tl.xfmView, pRc->tl.lpxfmToEye[2] );
        		Inverse4x4( pRc->tl.lpxfmToEye[2], pRc->tl.lpxfmToEyeInv[2] );
			}
			if ( dwDirtyEyeXforms & TLPV_DIRTY_EYE3XFM )
			{
				MatrixProduct( &pRc->tl.xfmWorld[3], &pRc->tl.xfmView, pRc->tl.lpxfmToEye[3] );
        		Inverse4x4( pRc->tl.lpxfmToEye[3], pRc->tl.lpxfmToEyeInv[3] );
			}
		}

		dwDirtyFlags &= ~TLPV_DIRTY_EYEXFM_ALL;
		dwDirtyFlags |= TLPV_DIRTY_NEEDXFMLIGHT;
	}

	if( dwDirtyFlags & TLPV_DIRTY_TXTRXFM )
	{
#if defined(VCPP)
		if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
		{
			DWORD i;
		  	for(i=0; i<D3DDP_MAXTEXCOORD; ++i)
		  	{
				if(pRc->tl.dwDirtyXfmTxtr & (1 << i))
				{
		  			SwizzleMatrix(&pRc->tl.xfmTxtr[i], pRc->tl.lpxfmTxtrSOA[i]);
					pRc->tl.dwDirtyXfmTxtr &= ~(1 << i);
					if(pRc->tl.dwDirtyXfmTxtr == 0)
						i = D3DDP_MAXTEXCOORD;	// early out
				}
			}
		}
		else 
#endif
		if (pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOW)
		{
			DWORD i;
		  	for(i=0; i<D3DDP_MAXTEXCOORD; ++i)
		  	{
				if(pRc->tl.dwDirtyXfmTxtr & (1 << i))
				{
					TransposeMatrix_C(&pRc->tl.xfmTxtr[i], pRc->tl.lpxfmTxtrT[i]);
					pRc->tl.dwDirtyXfmTxtr &= ~(1 << i);
					if(pRc->tl.dwDirtyXfmTxtr == 0)
						i = D3DDP_MAXTEXCOORD;	// early out
				}
			}
		}	
	}


#ifdef PROFILE_SPARSE_MATRIX
    mat_gather_stats(pRc->tl.lpxfmCurrent[0], pRc->tl.lpxfmToEye[0], pRc->tl.lpxfmToEyeInv[0]);
#endif

	// Clear the dirty transform flags, all except the world transforms
	pRc->tl.dwDirtyFlags = dwDirtyFlags & ~TLPV_DIRTY_XFORM;
	pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VB;	// VB is hosed, we have to re-xform
	return hr;
}


//**********************************
// Profiling code -- see if the 
// matrices are sparse!
//*********************************
#ifdef PROFILE_SPARSE_MATRIX

/*-------------------------------------------------------------------
Function Name:  InitMatrixStats
Description:    Initialize Matrix statistic counters
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
void InitMatrixStats()
{
  int i;

  for (i = 0; i < 65536; i++)  {
    sparse_curr_matrix0[i] = 0;
    sparse_curr_matrix1[i] = 0;
  }

  for (i = 0; i < 512; i++)  {
    sparse_eye_matrix0[i] = 0;
    sparse_eyeinv_matrix0[i] = 0;
    sparse_eye_matrix1[i] = 0;
    sparse_eyeinv_matrix1[i] = 0;
  }

}  

/*-------------------------------------------------------------------
Function Name:  AccumulateMatrixStats
Description:    Gather statistics on matrices
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/

void AccumulateMatrixStats(float val, D3DMATRIX *curr, D3DMATRIX *eye, D3DMATRIX *inveye, DWORD *currstat, DWORD *eyestat, DWORD *inveyestat)
{
  DWORD bits;

  bits = 0;
  if (curr->_11 == val) bits |= 1<<0;  
  if (curr->_12 == val) bits |= 1<<1;  
  if (curr->_13 == val) bits |= 1<<2;  
  if (curr->_14 == val) bits |= 1<<3;  
  if (curr->_21 == val) bits |= 1<<4;  
  if (curr->_22 == val) bits |= 1<<5;  
  if (curr->_23 == val) bits |= 1<<6;  
  if (curr->_24 == val) bits |= 1<<7;  
  if (curr->_31 == val) bits |= 1<<8;  
  if (curr->_32 == val) bits |= 1<<9;  
  if (curr->_33 == val) bits |= 1<<10;  
  if (curr->_34 == val) bits |= 1<<11;  
  if (curr->_41 == val) bits |= 1<<12;  
  if (curr->_42 == val) bits |= 1<<13;  
  if (curr->_43 == val) bits |= 1<<14;  
  if (curr->_44 == val) bits |= 1<<15;  
  currstat[bits]++;

  bits = 0;
  if (eye->_11 == val) bits |= 1<<0;  
  if (eye->_12 == val) bits |= 1<<1;  
  if (eye->_13 == val) bits |= 1<<2;  
  if (eye->_21 == val) bits |= 1<<3;  
  if (eye->_22 == val) bits |= 1<<4;  
  if (eye->_23 == val) bits |= 1<<5;  
  if (eye->_31 == val) bits |= 1<<6;  
  if (eye->_32 == val) bits |= 1<<7;  
  if (eye->_33 == val) bits |= 1<<8;  
  eyestat[bits]++;

  bits = 0;
  if (inveye->_11 == val) bits |= 1<<0;  
  if (inveye->_12 == val) bits |= 1<<1;  
  if (inveye->_13 == val) bits |= 1<<2;  
  if (inveye->_21 == val) bits |= 1<<3;  
  if (inveye->_22 == val) bits |= 1<<4;  
  if (inveye->_23 == val) bits |= 1<<5;  
  if (inveye->_31 == val) bits |= 1<<6;  
  if (inveye->_32 == val) bits |= 1<<7;  
  if (inveye->_33 == val) bits |= 1<<8;  
  inveyestat[bits]++;

}  

/*-------------------------------------------------------------------
Function Name:  MatrixResults
Description:    Display special case matrices
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
void MatrixResults(float val, DWORD *curr, DWORD *eye, DWORD *eyeinv)
{
  int i;
  char buf[132];
  D3DPRINT(0, "\n*****************************************************************************************\n");
  D3DPRINT(0, "*****************************************************************************************\n");
  sprintf(buf, "%f", val);
  D3DPRINT(0, "Value: %s\n", buf);  
  D3DPRINT(0, "Current Matrix\n");
  for (i = 0; i < 65536; i++)  {
    if (curr[i])  {
      D3DPRINT(0, "Count: %08d -- ", curr[i]);
      if (i & (1<<0)) D3DPRINT(0, "_11 ");
      if (i & (1<<1)) D3DPRINT(0, "_12 ");
      if (i & (1<<2)) D3DPRINT(0, "_13 ");
      if (i & (1<<3)) D3DPRINT(0, "_14 ");
      if (i & (1<<4)) D3DPRINT(0, "_21 ");
      if (i & (1<<5)) D3DPRINT(0, "_22 ");
      if (i & (1<<6)) D3DPRINT(0, "_23 ");
      if (i & (1<<7)) D3DPRINT(0, "_24 ");
      if (i & (1<<8)) D3DPRINT(0, "_31 ");
      if (i & (1<<9)) D3DPRINT(0, "_32 ");
      if (i & (1<<10)) D3DPRINT(0, "_33 ");
      if (i & (1<<11)) D3DPRINT(0, "_34 ");
      if (i & (1<<12)) D3DPRINT(0, "_41 ");
      if (i & (1<<13)) D3DPRINT(0, "_42 ");
      if (i & (1<<14)) D3DPRINT(0, "_43 ");
      if (i & (1<<15)) D3DPRINT(0, "_44 ");
      D3DPRINT(0, "\n");
    }
  }

  D3DPRINT(0, "------------------------------------------------------------------------------------\n");
  sprintf(buf, "%f", val);
  D3DPRINT(0, "Value: %s\n", buf);  
  D3DPRINT(0, "Eye Matrix\n");
  for (i = 0; i < 512; i++)  {
    if (eye[i])  {
      D3DPRINT(0, "Count: %08d -- ", eye[i]);
      if (i & (1<<0)) D3DPRINT(0, "_11 ");
      if (i & (1<<1)) D3DPRINT(0, "_12 ");
      if (i & (1<<2)) D3DPRINT(0, "_13 ");
      if (i & (1<<3)) D3DPRINT(0, "_21 ");
      if (i & (1<<4)) D3DPRINT(0, "_22 ");
      if (i & (1<<5)) D3DPRINT(0, "_23 ");
      if (i & (1<<6)) D3DPRINT(0, "_31 ");
      if (i & (1<<7)) D3DPRINT(0, "_32 ");
      if (i & (1<<8)) D3DPRINT(0, "_33 ");
      D3DPRINT(0, "\n");
    }
  }

  D3DPRINT(0, "------------------------------------------------------------------------------------\n");
  sprintf(buf, "%f", val);
  D3DPRINT(0, "Value: %s\n", buf);  
  D3DPRINT(0, "Inv. Eye Matrix\n");
  for (i = 0; i < 512; i++)  {
    if (eyeinv[i])  {
      D3DPRINT(0, "Count: %08d -- ", eyeinv[i]);
      if (i & (1<<0)) D3DPRINT(0, "_11 ");
      if (i & (1<<1)) D3DPRINT(0, "_12 ");
      if (i & (1<<2)) D3DPRINT(0, "_13 ");
      if (i & (1<<3)) D3DPRINT(0, "_21 ");
      if (i & (1<<4)) D3DPRINT(0, "_22 ");
      if (i & (1<<5)) D3DPRINT(0, "_23 ");
      if (i & (1<<6)) D3DPRINT(0, "_31 ");
      if (i & (1<<7)) D3DPRINT(0, "_32 ");
      if (i & (1<<8)) D3DPRINT(0, "_33 ");
      D3DPRINT(0, "\n");
    }
  }

}  



/*-------------------------------------------------------------------
Function Name:  mat_gather_stats
Description:    Driver for the matrix profiling code
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/


void mat_gather_stats(D3DMATRIX *curr, D3DMATRIX *eye, D3DMATRIX *inveye)
{
  if (mprofctrl == 0) {
    InitMatrixStats();
    mprofctrl = 1;
  }
  else if (mprofctrl == 1) {
    AccumulateMatrixStats(0.0f, curr, eye, inveye, sparse_curr_matrix0, sparse_eye_matrix0, sparse_eyeinv_matrix0);
    AccumulateMatrixStats(1.0f, curr, eye, inveye, sparse_curr_matrix1, sparse_eye_matrix1, sparse_eyeinv_matrix1);
  }
  else if (mprofctrl == 2) {
    MatrixResults(0.0, sparse_curr_matrix0, sparse_eye_matrix0, sparse_eyeinv_matrix0);
    MatrixResults(1.0, sparse_curr_matrix1, sparse_eye_matrix1, sparse_eyeinv_matrix1);
    mprofctrl = 0;
  }
}  

#endif //PROFILE_SPARSE_MATRIX


#endif //TnL_HAL
#endif //(DX >= 7)

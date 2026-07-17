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
** File name: soageom.h
**
** Description: SOA Transformation code
**
*/

#ifdef TnL_HAL


/*-------------------------------------------------------------------------------
*	Xform_4_SOA_MAC( SOA_XYZW *pD , SOA_XYZ *pV, SOA_MATRIX *pSOAMat4x4x4, RC* pRc )
*	Prefetches the current c.z and h.y cachelines, increments src prt
-------------------------------------------------------------------------------*/
#define Xform_4_SOA_MAC( pRc, pMat, pSrc, pDst )						\
    _asm movaps    xmm7, [pSrc]_SV3.x	/* x[i] */						\
    _asm movaps    xmm3, [pSrc]_SV3.y	/* y[i] */						\
	_asm prefetchnta [pSrc+0x20]		/* z[i] */						\
    _asm movaps    xmm4, xmm7											\
    _asm mulps     xmm4, [pMat]_SM._11	/* x[i] * _11 */				\
    _asm movaps    xmm0, xmm3											\
    _asm mulps     xmm0, [pMat]_SM._21	/* y[i] * _21 */				\
    _asm movaps    xmm5, xmm7											\
    _asm mulps     xmm5, [pMat]_SM._12	/* x[i] * _12 */				\
    _asm movaps    xmm1, xmm3											\
    _asm mulps     xmm1, [pMat]_SM._22	/* y[i] * _22 */				\
    _asm addps     xmm4, xmm0			/* x[i] * _11 + y[i] * _21 */	\
    _asm movaps    xmm6, xmm7											\
    _asm mulps     xmm6, [pMat]_SM._13	/* x[i] * _13 */				\
    _asm movaps    xmm2, xmm3											\
    _asm mulps     xmm2, [pMat]_SM._23	/* y[i] * _23 */				\
    _asm addps     xmm5, xmm1			/* x[i] * _12 + y[i] * _22 */	\
    _asm mulps     xmm7, [pMat]_SM._14	/* x[i] * _14 */				\
    _asm movaps    xmm1, [pSrc]_SV3.z	/* z[i] */						\
    _asm addps     xmm4, [pMat]_SM._41	/* x[i] * _11 + y[i] * _21 +  _41 */	\
    _asm mulps     xmm3, [pMat]_SM._24	/* y[i] * _24 */						\
    _asm movaps    xmm0, xmm1													\
    _asm addps     xmm5, [pMat]_SM._42	/* x[i] * _12 + y[i] * _22 +  _42 */	\
    _asm mulps     xmm1, [pMat]_SM._31	/* z[i] * _31 */						\
    _asm addps     xmm6, xmm2			/* x[i] * _13 + y[i] * _23 */			\
	_asm prefetchnta [pSrc+0x40]		/* y of the normals (or 1st blend) */	\
    _asm movaps    xmm2, xmm0													\
	_asm add       pSrc, [pRc]RC.tl.SOAFVF.dwStride	/* pPos += SOAFVF stride */	\
    _asm addps     xmm7, xmm3			/* x[i] * _14 + y[i] * _24 */			\
    _asm mulps     xmm2, [pMat]_SM._32	/* z[i] * _32 */						\
    _asm movaps    xmm3, xmm0													\
    _asm addps     xmm6, [pMat]_SM._43	/* x[i] * _13 + y[i] * _23 +  _43 */	\
    _asm mulps     xmm3, [pMat]_SM._33	/* z[i] * _33 */						\
    _asm addps     xmm7, [pMat]_SM._44	/* x[i] * _14 + y[i] * _24 +  _44 */	\
    _asm mulps     xmm0, [pMat]_SM._34	/* z[i] * _34 */						\
    _asm addps     xmm4, xmm1        	/* x[i] * _11 + y[i] * _21 + z[i] * _31 + _41 */	\
	_asm prefetchnta [pSrc]_SV3.x		/* z[i] */						\
	_asm addps     xmm5, xmm2        	/* x[i] * _12 + y[i] * _22 + z[i] * _32 + _42 */	\
    _asm movaps    [pDst]_SV4.x, xmm4	/* pD->x */											\
    _asm addps     xmm6, xmm3        	/* x[i] * _13 + y[i] * _23 + z[i] * _33 + _43 */	\
    _asm movaps    [pDst]_SV4.y, xmm5	/* pD->y */											\
    _asm addps     xmm7, xmm0			/* x[i] * _14 + y[i] * _24 + z[i] * _34 + _44 */	\
    _asm movaps    [pDst]_SV4.z, xmm6	/* pD->z */											\
    _asm movaps    [pDst]_SV4.w, xmm7	/* pD->w */											


// Same as Xform_4_SOA_MAC but doesn't store to memory, leaves results in xmm4-xmm7
#define Xform_4_NOSTORE_SOA_MAC( pRc, pMat, pSrc  )						\
    _asm movaps    xmm7, [pSrc]_SV3.x	/* x[i] */						\
    _asm movaps    xmm3, [pSrc]_SV3.y	/* y[i] */						\
	_asm prefetchnta [pSrc+0x20]		/* z[i] */						\
    _asm movaps    xmm4, xmm7											\
    _asm mulps     xmm4, [pMat]_SM._11	/* x[i] * _11 */				\
    _asm movaps    xmm0, xmm3											\
    _asm mulps     xmm0, [pMat]_SM._21	/* y[i] * _21 */				\
    _asm movaps    xmm5, xmm7											\
    _asm mulps     xmm5, [pMat]_SM._12	/* x[i] * _12 */				\
    _asm movaps    xmm1, xmm3											\
    _asm mulps     xmm1, [pMat]_SM._22	/* y[i] * _22 */				\
    _asm addps     xmm4, xmm0			/* x[i] * _11 + y[i] * _21 */	\
    _asm movaps    xmm6, xmm7											\
    _asm mulps     xmm6, [pMat]_SM._13	/* x[i] * _13 */				\
    _asm movaps    xmm2, xmm3											\
    _asm mulps     xmm2, [pMat]_SM._23	/* y[i] * _23 */				\
    _asm addps     xmm5, xmm1			/* x[i] * _12 + y[i] * _22 */	\
    _asm mulps     xmm7, [pMat]_SM._14	/* x[i] * _14 */				\
    _asm movaps    xmm1, [pSrc]_SV3.z	/* z[i] */						\
    _asm addps     xmm4, [pMat]_SM._41	/* x[i] * _11 + y[i] * _21 +  _41 */	\
    _asm mulps     xmm3, [pMat]_SM._24	/* y[i] * _24 */						\
    _asm movaps    xmm0, xmm1													\
    _asm addps     xmm5, [pMat]_SM._42	/* x[i] * _12 + y[i] * _22 +  _42 */	\
    _asm mulps     xmm1, [pMat]_SM._31	/* z[i] * _31 */						\
    _asm addps     xmm6, xmm2			/* x[i] * _13 + y[i] * _23 */			\
	_asm prefetchnta [pSrc+0x40]		/* y of the normals (or 1st blend) */	\
    _asm movaps    xmm2, xmm0													\
	_asm add       pSrc, [pRc]RC.tl.SOAFVF.dwStride	/* pPos += SOAFVF stride */	\
    _asm addps     xmm7, xmm3			/* x[i] * _14 + y[i] * _24 */			\
    _asm mulps     xmm2, [pMat]_SM._32	/* z[i] * _32 */						\
    _asm movaps    xmm3, xmm0													\
    _asm addps     xmm6, [pMat]_SM._43	/* x[i] * _13 + y[i] * _23 +  _43 */	\
    _asm mulps     xmm3, [pMat]_SM._33	/* z[i] * _33 */						\
    _asm addps     xmm7, [pMat]_SM._44	/* x[i] * _14 + y[i] * _24 +  _44 */	\
    _asm mulps     xmm0, [pMat]_SM._34	/* z[i] * _34 */						\
    _asm addps     xmm4, xmm1        	/* x[i] * _11 + y[i] * _21 + z[i] * _31 + _41 */	\
	_asm prefetchnta [pSrc]_SV3.x		/* z[i] */						\
	_asm addps     xmm5, xmm2        	/* x[i] * _12 + y[i] * _22 + z[i] * _32 + _42 */	\
    _asm addps     xmm6, xmm3        	/* x[i] * _13 + y[i] * _23 + z[i] * _33 + _43 */	\
    _asm addps     xmm7, xmm0			/* x[i] * _14 + y[i] * _24 + z[i] * _34 + _44 */

// Similar to Xform_4_NOSTORE_SOA_MAC, but for vertex blending
// First blend doesn't need to add to the last blend and pcumulBlend = 1.0f - pBlend
#define Xform_4_SOA_MAC_BLEND_FIRST( pRc, pMat, pSrc, pDst, pcumulBlend, pBlend )	\
    _asm movaps    xmm7, [pSrc]_SV3.x	/* x[i] */						\
    _asm movaps    xmm3, [pSrc]_SV3.y	/* y[i] */						\
    _asm movaps    xmm4, xmm7											\
    _asm mulps     xmm4, [pMat]_SM._11	/* x[i] * _11 */				\
    _asm movaps    xmm0, xmm3											\
    _asm mulps     xmm0, [pMat]_SM._21	/* y[i] * _21 */				\
    _asm movaps    xmm5, xmm7											\
    _asm mulps     xmm5, [pMat]_SM._12	/* x[i] * _12 */				\
    _asm movaps    xmm1, xmm3											\
    _asm mulps     xmm1, [pMat]_SM._22	/* y[i] * _22 */				\
    _asm addps     xmm4, xmm0			/* x[i] * _11 + y[i] * _21 */	\
    _asm movaps    xmm6, xmm7											\
    _asm mulps     xmm6, [pMat]_SM._13	/* x[i] * _13 */				\
    _asm movaps    xmm2, xmm3											\
    _asm mulps     xmm2, [pMat]_SM._23	/* y[i] * _23 */				\
    _asm addps     xmm5, xmm1			/* x[i] * _12 + y[i] * _22 */	\
    _asm mulps     xmm7, [pMat]_SM._14	/* x[i] * _14 */				\
    _asm movaps    xmm1, [pSrc]_SV3.z	/* z[i] */						\
    _asm addps     xmm4, [pMat]_SM._41	/* x[i] * _11 + y[i] * _21 +  _41 */	\
    _asm mulps     xmm3, [pMat]_SM._24	/* y[i] * _24 */						\
    _asm movaps    xmm0, xmm1													\
    _asm addps     xmm5, [pMat]_SM._42	/* x[i] * _12 + y[i] * _22 +  _42 */	\
    _asm mulps     xmm1, [pMat]_SM._31	/* z[i] * _31 */						\
    _asm addps     xmm6, xmm2			/* x[i] * _13 + y[i] * _23 */			\
    _asm movaps    xmm2, xmm0													\
    _asm addps     xmm7, xmm3			/* x[i] * _14 + y[i] * _24 */			\
    _asm mulps     xmm2, [pMat]_SM._32	/* z[i] * _32 */						\
    _asm movaps    xmm3, xmm0													\
    _asm addps     xmm6, [pMat]_SM._43	/* x[i] * _13 + y[i] * _23 +  _43 */	\
    _asm mulps     xmm3, [pMat]_SM._33	/* z[i] * _33 */						\
    _asm addps     xmm7, [pMat]_SM._44	/* x[i] * _14 + y[i] * _24 +  _44 */	\
    _asm mulps     xmm0, [pMat]_SM._34	/* z[i] * _34 */						\
    _asm addps     xmm4, xmm1        	/* x[i] * _11 + y[i] * _21 + z[i] * _31 + _41 */	\
	_asm addps     xmm5, xmm2        	/* x[i] * _12 + y[i] * _22 + z[i] * _32 + _42 */	\
	_asm movaps	   xmm1, pBlend			/* blend */											\
    _asm addps     xmm6, xmm3        	/* x[i] * _13 + y[i] * _23 + z[i] * _33 + _43 */	\
	_asm movaps	   xmm2, [TL_one]		/* cumulBlend */ 									\
    _asm addps     xmm7, xmm0			/* x[i] * _14 + y[i] * _24 + z[i] * _34 + _44 */	\
	_asm add       pMat, SIZE SOA_MATRIX /* setup next blend */			\
	_asm mulps     xmm4, xmm1			/* x*blend */					\
	_asm prefetchnta [pSrc+0x20]		/* z[i] */						\
	_asm subps     xmm2, xmm1			/* cumulBlend - current blend */\
	_asm prefetchnta [pSrc+0x40]		/* y of the normals (or 1st blend) */	\
	_asm mulps     xmm5, xmm1			/* y*blend */					\
    _asm movaps    [pDst]_SV4.x, xmm4	/* pD->x */						\
	_asm mulps     xmm6, xmm1			/* z*blend */					\
    _asm movaps    pcumulBlend, xmm2	/* cumulBlend */				\
	_asm mulps     xmm7, xmm1			/* w*blend */					\
    _asm movaps    [pDst]_SV4.y, xmm5	/* pD->y */						\
    _asm movaps    [pDst]_SV4.z, xmm6	/* pD->z */						\
    _asm movaps    [pDst]_SV4.w, xmm7	/* pD->w */						\

// Intermediate blends must add to previous, pcumulBlend -= pBlend
#define Xform_4_SOA_MAC_BLEND_MIDDLE( pRc, pMat, pSrc, pDst, pcumulBlend, pBlend )	\
    _asm movaps    xmm7, [pSrc]_SV3.x	/* x[i] */						\
    _asm movaps    xmm3, [pSrc]_SV3.y	/* y[i] */						\
    _asm movaps    xmm4, xmm7											\
    _asm mulps     xmm4, [pMat]_SM._11	/* x[i] * _11 */				\
    _asm movaps    xmm0, xmm3											\
    _asm mulps     xmm0, [pMat]_SM._21	/* y[i] * _21 */				\
    _asm movaps    xmm5, xmm7											\
    _asm mulps     xmm5, [pMat]_SM._12	/* x[i] * _12 */				\
    _asm movaps    xmm1, xmm3											\
    _asm mulps     xmm1, [pMat]_SM._22	/* y[i] * _22 */				\
    _asm addps     xmm4, xmm0			/* x[i] * _11 + y[i] * _21 */	\
    _asm movaps    xmm6, xmm7											\
    _asm mulps     xmm6, [pMat]_SM._13	/* x[i] * _13 */				\
    _asm movaps    xmm2, xmm3											\
    _asm mulps     xmm2, [pMat]_SM._23	/* y[i] * _23 */				\
    _asm addps     xmm5, xmm1			/* x[i] * _12 + y[i] * _22 */	\
    _asm mulps     xmm7, [pMat]_SM._14	/* x[i] * _14 */				\
    _asm movaps    xmm1, [pSrc]_SV3.z	/* z[i] */						\
    _asm addps     xmm4, [pMat]_SM._41	/* x[i] * _11 + y[i] * _21 +  _41 */	\
    _asm mulps     xmm3, [pMat]_SM._24	/* y[i] * _24 */						\
    _asm movaps    xmm0, xmm1													\
    _asm addps     xmm5, [pMat]_SM._42	/* x[i] * _12 + y[i] * _22 +  _42 */	\
    _asm mulps     xmm1, [pMat]_SM._31	/* z[i] * _31 */						\
    _asm addps     xmm6, xmm2			/* x[i] * _13 + y[i] * _23 */			\
    _asm movaps    xmm2, xmm0													\
    _asm addps     xmm7, xmm3			/* x[i] * _14 + y[i] * _24 */			\
    _asm mulps     xmm2, [pMat]_SM._32	/* z[i] * _32 */						\
    _asm movaps    xmm3, xmm0													\
    _asm addps     xmm6, [pMat]_SM._43	/* x[i] * _13 + y[i] * _23 +  _43 */	\
    _asm mulps     xmm3, [pMat]_SM._33	/* z[i] * _33 */						\
    _asm addps     xmm7, [pMat]_SM._44	/* x[i] * _14 + y[i] * _24 +  _44 */	\
    _asm mulps     xmm0, [pMat]_SM._34	/* z[i] * _34 */						\
    _asm addps     xmm4, xmm1        	/* x[i] * _11 + y[i] * _21 + z[i] * _31 + _41 */	\
	_asm addps     xmm5, xmm2        	/* x[i] * _12 + y[i] * _22 + z[i] * _32 + _42 */	\
	_asm movaps	   xmm1, pBlend			/* blend */											\
    _asm addps     xmm6, xmm3        	/* x[i] * _13 + y[i] * _23 + z[i] * _33 + _43 */	\
	_asm movaps	   xmm2, pcumulBlend	/* cumulBlend */ 									\
    _asm addps     xmm7, xmm0			/* x[i] * _14 + y[i] * _24 + z[i] * _34 + _44 */	\
	_asm add       pMat, SIZE SOA_MATRIX /* setup next blend */			\
	_asm mulps     xmm4, xmm1			/* x*blend */					\
	_asm subps     xmm2, xmm1			/* cumulBlend - current blend */\
	_asm mulps     xmm5, xmm1			/* y*blend */					\
    _asm addps     xmm4, [pDst]_SV4.x	/* x += pD->x */				\
	_asm mulps     xmm6, xmm1			/* z*blend */					\
    _asm addps     xmm5, [pDst]_SV4.y	/* y += pD->y */				\
	_asm mulps     xmm7, xmm1			/* w*blend */					\
    _asm addps     xmm6, [pDst]_SV4.z	/* z += pD->z */				\
    _asm addps     xmm7, [pDst]_SV4.w	/* w += pD->w */				\
    _asm movaps    pcumulBlend, xmm2	/* cumulBlend */				\
    _asm movaps    [pDst]_SV4.x, xmm4	/* pD->x */						\
    _asm movaps    [pDst]_SV4.y, xmm5	/* pD->y */						\
    _asm movaps    [pDst]_SV4.z, xmm6	/* pD->z */						\
    _asm movaps    [pDst]_SV4.w, xmm7	/* pD->w */						\

// The last blend uses pcumulBlend for the blend factor
#define Xform_4_SOA_MAC_BLEND_LAST( pRc, pMat, pSrc, pDst, pcumulBlend )	\
    _asm movaps    xmm7, [pSrc]_SV3.x	/* x[i] */						\
    _asm movaps    xmm3, [pSrc]_SV3.y	/* y[i] */						\
    _asm movaps    xmm4, xmm7											\
    _asm mulps     xmm4, [pMat]_SM._11	/* x[i] * _11 */				\
    _asm movaps    xmm0, xmm3											\
    _asm mulps     xmm0, [pMat]_SM._21	/* y[i] * _21 */				\
    _asm movaps    xmm5, xmm7											\
    _asm mulps     xmm5, [pMat]_SM._12	/* x[i] * _12 */				\
    _asm movaps    xmm1, xmm3											\
    _asm mulps     xmm1, [pMat]_SM._22	/* y[i] * _22 */				\
    _asm addps     xmm4, xmm0			/* x[i] * _11 + y[i] * _21 */	\
    _asm movaps    xmm6, xmm7											\
    _asm mulps     xmm6, [pMat]_SM._13	/* x[i] * _13 */				\
    _asm movaps    xmm2, xmm3											\
    _asm mulps     xmm2, [pMat]_SM._23	/* y[i] * _23 */				\
    _asm addps     xmm5, xmm1			/* x[i] * _12 + y[i] * _22 */	\
    _asm mulps     xmm7, [pMat]_SM._14	/* x[i] * _14 */				\
    _asm movaps    xmm1, [pSrc]_SV3.z	/* z[i] */						\
    _asm addps     xmm4, [pMat]_SM._41	/* x[i] * _11 + y[i] * _21 +  _41 */	\
    _asm mulps     xmm3, [pMat]_SM._24	/* y[i] * _24 */						\
    _asm movaps    xmm0, xmm1													\
    _asm addps     xmm5, [pMat]_SM._42	/* x[i] * _12 + y[i] * _22 +  _42 */	\
    _asm mulps     xmm1, [pMat]_SM._31	/* z[i] * _31 */						\
    _asm addps     xmm6, xmm2			/* x[i] * _13 + y[i] * _23 */			\
    _asm movaps    xmm2, xmm0													\
    _asm addps     xmm7, xmm3			/* x[i] * _14 + y[i] * _24 */			\
    _asm mulps     xmm2, [pMat]_SM._32	/* z[i] * _32 */						\
    _asm movaps    xmm3, xmm0													\
    _asm addps     xmm6, [pMat]_SM._43	/* x[i] * _13 + y[i] * _23 +  _43 */	\
    _asm mulps     xmm3, [pMat]_SM._33	/* z[i] * _33 */						\
    _asm addps     xmm7, [pMat]_SM._44	/* x[i] * _14 + y[i] * _24 +  _44 */	\
    _asm mulps     xmm0, [pMat]_SM._34	/* z[i] * _34 */						\
    _asm addps     xmm4, xmm1        	/* x[i] * _11 + y[i] * _21 + z[i] * _31 + _41 */	\
	_asm movaps	   xmm1, pcumulBlend	/* last blend = cumulBlend */ 						\
	_asm addps     xmm5, xmm2        	/* x[i] * _12 + y[i] * _22 + z[i] * _32 + _42 */	\
	_asm add       pSrc, [pRc]RC.tl.SOAFVF.dwStride	/* pPos += SOAFVF stride */	\
    _asm addps     xmm6, xmm3        	/* x[i] * _13 + y[i] * _23 + z[i] * _33 + _43 */	\
    _asm addps     xmm7, xmm0			/* x[i] * _14 + y[i] * _24 + z[i] * _34 + _44 */	\
	_asm prefetchnta [pSrc]_SV3.x		/* z[i] */						\
	_asm mulps     xmm4, xmm1			/* x*blend */					\
	_asm mulps     xmm5, xmm1			/* y*blend */					\
    _asm addps     xmm4, [pDst]_SV4.x	/* x += pD->x */				\
	_asm mulps     xmm6, xmm1			/* z*blend */					\
    _asm addps     xmm5, [pDst]_SV4.y	/* y += pD->y */				\
	_asm mulps     xmm7, xmm1			/* w*blend */					\
    _asm addps     xmm6, [pDst]_SV4.z	/* z += pD->z */				\
    _asm addps     xmm7, [pDst]_SV4.w	/* w += pD->w */				\
    _asm movaps    [pDst]_SV4.x, xmm4	/* pD->x */						\
    _asm movaps    [pDst]_SV4.y, xmm5	/* pD->y */						\
    _asm movaps    [pDst]_SV4.z, xmm6	/* pD->z */						\
    _asm movaps    [pDst]_SV4.w, xmm7	/* pD->w */						\


/**************************************************************************
*	Xform_3_SOA_NOSTORE_MAC ( SOA_MATRIX *pSOAMat4x4x4, SOA_XYZ *pV  )
**************************************************************************/
#define Xform_3_SOA_NOSTORE_MAC( pMat, pSrc ) 				\
    _asm movaps    xmm2, [pSrc]_SV3.x	/* x[i] */			\
    _asm movaps    xmm5, [pSrc]_SV3.y	/* y[i] */			\
    _asm movaps    xmm7, [pSrc]_SV3.z	/* z[i] */			\
    _asm movaps    xmm0, xmm2								\
    _asm mulps     xmm0, [pMat]_SM._11	/* x[i] * _11 */	\
    _asm movaps    xmm1, xmm2								\
    _asm mulps     xmm1, [pMat]_SM._12	/* x[i] * _12 */	\
    _asm movaps    xmm3, xmm5								\
    _asm mulps     xmm2, [pMat]_SM._13	/* x[i] * _13 */	\
    _asm addps     xmm0, [pMat]_SM._41	/* x[i] * _11 +  _41 */	\
    _asm movaps    xmm4, xmm5									\
    _asm mulps     xmm3, [pMat]_SM._21	/* y[i] * _21 */		\
    _asm addps     xmm1, [pMat]_SM._42	/* x[i] * _12 +  _42 */	\
    _asm mulps     xmm4, [pMat]_SM._22	/* y[i] * _22 */		\
    _asm movaps    xmm6, xmm7									\
    _asm addps     xmm2, [pMat]_SM._43	/* x[i] * _13 +  _43 */	\
    _asm mulps     xmm5, [pMat]_SM._23	/* y[i] * _23 */		\
    _asm addps     xmm0, xmm3       	/* x[i] * _11 + y[i] * _21 */	\
    _asm mulps     xmm6, [pMat]_SM._31	/* z[i] * _31 */				\
    _asm movaps    xmm3, xmm7											\
    _asm addps     xmm1, xmm4       	/* x[i] * _12 + y[i] * _22 */	\
    _asm mulps     xmm3, [pMat]_SM._32	/* z[i] * _32 */				\
    _asm addps     xmm0, xmm6       	/* x[i] * _11 + y[i] * _21 + z[i] * _31 */	\
    _asm mulps     xmm7, [pMat]_SM._33	/* z[i] * _33 */							\
    _asm addps     xmm2, xmm5       	/* x[i] * _13 + y[i] * _23 */				\
    _asm addps     xmm1, xmm3       	/* x[i] * _12 + y[i] * _22 + z[i] * _32 */	\
    _asm addps     xmm2, xmm7       	/* x[i] * _13 + y[i] * _23 + z[i] * _33 */


/**************************************************************************
*	Xform_3_SOA_MAC ( SOA_MATRIX *pSOAMat4x4x4, SOA_XYZ *pV, SOA_XYZ *pD )
**************************************************************************/
#define Xform_3_SOA_MAC( pMat, pSrc, pDst )					\
	Xform_3_SOA_NOSTORE_MAC( pMat, pSrc  )					\
    _asm movaps    [pDst]_SV3.x, xmm0	/* pD->x */			\
    _asm movaps    [pDst]_SV3.y, xmm1	/* pD->y */			\
    _asm movaps    [pDst]_SV3.z, xmm2	/* pD->z */			


// Similar to Xform_3_SOA_MAC, but for vertex blending
/********************************************************************************************************************************
*	Xform_3_SOA_MAC_BLEND_FIRST ( SOA_MATRIX *pMat, SOA_XYZ *pSrc, SOA_XYZ *pDst, SOA_FLOAT *pcumulBlend, SOA_FLOAT *pBlend )
********************************************************************************************************************************/
// First blend doesn't need to add to the last blend and pcumulBlend = 1.0f - pBlend
#define Xform_3_SOA_MAC_BLEND_FIRST( pMat, pSrc, pDst, pcumulBlend, pBlend )		\
    _asm movaps    xmm0, [pSrc]_SV3.x	/* x[i] */			\
    _asm movaps    xmm5, [pSrc]_SV3.y	/* y[i] */			\
    _asm movaps    xmm7, [pSrc]_SV3.z	/* z[i] */			\
    _asm movaps    xmm2, xmm0								\
    _asm mulps     xmm0, [pMat]_SM._11	/* x[i] * _11 */	\
    _asm movaps    xmm1, xmm2								\
    _asm mulps     xmm1, [pMat]_SM._12	/* x[i] * _12 */	\
    _asm movaps    xmm3, xmm5								\
    _asm mulps     xmm2, [pMat]_SM._13	/* x[i] * _13 */	\
    _asm addps     xmm0, [pMat]_SM._41	/* x[i] * _11 +  _41 */	\
    _asm movaps    xmm4, xmm5									\
    _asm mulps     xmm3, [pMat]_SM._21	/* y[i] * _21 */		\
    _asm addps     xmm1, [pMat]_SM._42	/* x[i] * _12 +  _42 */	\
    _asm mulps     xmm4, [pMat]_SM._22	/* y[i] * _22 */		\
    _asm movaps    xmm6, xmm7									\
    _asm addps     xmm2, [pMat]_SM._43	/* x[i] * _13 +  _43 */	\
    _asm mulps     xmm5, [pMat]_SM._23	/* y[i] * _23 */		\
    _asm addps     xmm0, xmm3       	/* x[i] * _11 + y[i] * _21 */	\
    _asm mulps     xmm6, [pMat]_SM._31	/* z[i] * _31 */				\
    _asm movaps    xmm3, xmm7											\
    _asm addps     xmm1, xmm4       	/* x[i] * _12 + y[i] * _22 */	\
    _asm mulps     xmm3, [pMat]_SM._32	/* z[i] * _32 */				\
    _asm addps     xmm0, xmm6       	/* x[i] * _11 + y[i] * _21 + z[i] * _31 */	\
    _asm mulps     xmm7, [pMat]_SM._33	/* z[i] * _33 */							\
    _asm addps     xmm2, xmm5       	/* x[i] * _13 + y[i] * _23 */				\
	_asm movaps	   xmm4, pBlend			/* blend */									\
    _asm addps     xmm1, xmm3       	/* x[i] * _12 + y[i] * _22 + z[i] * _32 */	\
	_asm movaps	   xmm6, [TL_one]		/* cumulBlend */ 							\
    _asm addps     xmm2, xmm7       	/* x[i] * _13 + y[i] * _23 + z[i] * _33 */	\
	_asm mulps	   xmm0, xmm4			/* x * blend */								\
	_asm add       pMat, SIZE SOA_MATRIX /* setup next blend */						\
	_asm subps     xmm6, xmm4			/* cumulBlend - current blend */			\
	_asm mulps	   xmm1, xmm4			/* y * blend */								\
    _asm movaps    pDst+0x00, xmm0	/* pD->x */									\
	_asm mulps	   xmm2, xmm4			/* z * blend */								\
    _asm movaps    pcumulBlend, xmm6	/* cumulBlend */							\
    _asm movaps    pDst+0x10, xmm1	/* pD->y */									\
    _asm movaps    pDst+0x20, xmm2	/* pD->z */									\

/********************************************************************************************************************************
*	Xform_3_SOA_MAC_BLEND_MIDDLE ( SOA_MATRIX *pMat, SOA_XYZ *pSrc, SOA_XYZ *pDst, SOA_FLOAT *pcumulBlend, SOA_FLOAT *pBlend )
********************************************************************************************************************************/
// Intermediate blends must add to previous, pcumulBlend -= pBlend
#define Xform_3_SOA_MAC_BLEND_MIDDLE( pMat, pSrc, pDst, pcumulBlend, pBlend )		\
    _asm movaps    xmm0, [pSrc]_SV3.x	/* x[i] */			\
    _asm movaps    xmm5, [pSrc]_SV3.y	/* y[i] */			\
    _asm movaps    xmm7, [pSrc]_SV3.z	/* z[i] */			\
    _asm movaps    xmm2, xmm0								\
    _asm mulps     xmm0, [pMat]_SM._11	/* x[i] * _11 */	\
    _asm movaps    xmm1, xmm2								\
    _asm mulps     xmm1, [pMat]_SM._12	/* x[i] * _12 */	\
    _asm movaps    xmm3, xmm5								\
    _asm mulps     xmm2, [pMat]_SM._13	/* x[i] * _13 */	\
    _asm addps     xmm0, [pMat]_SM._41	/* x[i] * _11 +  _41 */	\
    _asm movaps    xmm4, xmm5									\
    _asm mulps     xmm3, [pMat]_SM._21	/* y[i] * _21 */		\
    _asm addps     xmm1, [pMat]_SM._42	/* x[i] * _12 +  _42 */	\
    _asm mulps     xmm4, [pMat]_SM._22	/* y[i] * _22 */		\
    _asm movaps    xmm6, xmm7									\
    _asm addps     xmm2, [pMat]_SM._43	/* x[i] * _13 +  _43 */	\
    _asm mulps     xmm5, [pMat]_SM._23	/* y[i] * _23 */		\
    _asm addps     xmm0, xmm3       	/* x[i] * _11 + y[i] * _21 */	\
    _asm mulps     xmm6, [pMat]_SM._31	/* z[i] * _31 */				\
    _asm movaps    xmm3, xmm7											\
    _asm addps     xmm1, xmm4       	/* x[i] * _12 + y[i] * _22 */	\
    _asm mulps     xmm3, [pMat]_SM._32	/* z[i] * _32 */				\
    _asm addps     xmm0, xmm6       	/* x[i] * _11 + y[i] * _21 + z[i] * _31 */	\
    _asm mulps     xmm7, [pMat]_SM._33	/* z[i] * _33 */							\
    _asm addps     xmm2, xmm5       	/* x[i] * _13 + y[i] * _23 */				\
	_asm movaps	   xmm4, pBlend			/* blend */									\
    _asm addps     xmm1, xmm3       	/* x[i] * _12 + y[i] * _22 + z[i] * _32 */	\
	_asm movaps	   xmm6, pcumulBlend	/* cumulBlend */ 							\
    _asm addps     xmm2, xmm7       	/* x[i] * _13 + y[i] * _23 + z[i] * _33 */	\
	_asm mulps	   xmm0, xmm4			/* x * blend */								\
	_asm add       pMat, SIZE SOA_MATRIX /* setup next blend */						\
	_asm subps     xmm6, xmm4			/* cumulBlend - current blend */			\
	_asm mulps	   xmm1, xmm4			/* y * blend */								\
	_asm addps	   xmm0, pDst+0x00		/* pD->x += x * blend */					\
	_asm mulps	   xmm2, xmm4			/* z * blend */								\
	_asm addps	   xmm1, pDst+0x10		/* pD->x += x * blend */					\
	_asm addps	   xmm2, pDst+0x20		/* pD->x += x * blend */					\
    _asm movaps    pcumulBlend, xmm6	/* cumulBlend */							\
    _asm movaps    pDst+0x00, xmm0		/* pD->x */									\
    _asm movaps    pDst+0x10, xmm1		/* pD->y */									\
    _asm movaps    pDst+0x20, xmm2		/* pD->z */									\

/********************************************************************************************************************************
*	Xform_3_SOA_MAC_BLEND_LAST ( SOA_MATRIX *pMat, SOA_XYZ *pSrc, SOA_XYZ *pDst, SOA_FLOAT *pcumulBlend )
********************************************************************************************************************************/
// The last blend uses pcumulBlend for the blend factor
#define Xform_3_SOA_MAC_BLEND_LAST( pMat, pSrc, pDst, pcumulBlend )		\
    _asm movaps    xmm0, [pSrc]_SV3.x	/* x[i] */			\
    _asm movaps    xmm5, [pSrc]_SV3.y	/* y[i] */			\
    _asm movaps    xmm7, [pSrc]_SV3.z	/* z[i] */			\
    _asm movaps    xmm2, xmm0								\
    _asm mulps     xmm0, [pMat]_SM._11	/* x[i] * _11 */	\
    _asm movaps    xmm1, xmm2								\
    _asm mulps     xmm1, [pMat]_SM._12	/* x[i] * _12 */	\
    _asm movaps    xmm3, xmm5								\
    _asm mulps     xmm2, [pMat]_SM._13	/* x[i] * _13 */	\
    _asm addps     xmm0, [pMat]_SM._41	/* x[i] * _11 +  _41 */	\
    _asm movaps    xmm4, xmm5									\
    _asm mulps     xmm3, [pMat]_SM._21	/* y[i] * _21 */		\
    _asm addps     xmm1, [pMat]_SM._42	/* x[i] * _12 +  _42 */	\
    _asm mulps     xmm4, [pMat]_SM._22	/* y[i] * _22 */		\
    _asm movaps    xmm6, xmm7									\
    _asm addps     xmm2, [pMat]_SM._43	/* x[i] * _13 +  _43 */	\
    _asm mulps     xmm5, [pMat]_SM._23	/* y[i] * _23 */		\
    _asm addps     xmm0, xmm3       	/* x[i] * _11 + y[i] * _21 */	\
    _asm mulps     xmm6, [pMat]_SM._31	/* z[i] * _31 */				\
    _asm movaps    xmm3, xmm7											\
    _asm addps     xmm1, xmm4       	/* x[i] * _12 + y[i] * _22 */	\
    _asm mulps     xmm3, [pMat]_SM._32	/* z[i] * _32 */				\
    _asm addps     xmm0, xmm6       	/* x[i] * _11 + y[i] * _21 + z[i] * _31 */	\
    _asm mulps     xmm7, [pMat]_SM._33	/* z[i] * _33 */							\
    _asm addps     xmm2, xmm5       	/* x[i] * _13 + y[i] * _23 */				\
	_asm movaps	   xmm4, pcumulBlend	/* blend = cumulBlend */					\
    _asm addps     xmm1, xmm3       	/* x[i] * _12 + y[i] * _22 + z[i] * _32 */	\
    _asm addps     xmm2, xmm7       	/* x[i] * _13 + y[i] * _23 + z[i] * _33 */	\
	_asm mulps	   xmm0, xmm4			/* x * blend */								\
	_asm mulps	   xmm1, xmm4			/* y * blend */								\
	_asm addps	   xmm0, pDst+0x00		/* pD->x += x * blend */					\
	_asm mulps	   xmm2, xmm4			/* z * blend */								\
	_asm addps	   xmm1, pDst+0x10		/* pD->x += x * blend */					\
	_asm addps	   xmm2, pDst+0x20		/* pD->x += x * blend */					\



/*-------------------------------------------------------------------
*	DotProduct3SOA_MAC ( SOA_XYZ *v1, SOA_XYZ *v2 )
*
*	Description:	Perform dot product on 2 3D vectors in SOA form
*	Input Params:	eax = v1, ebx = v2
*	Return:			xmm0
-------------------------------------------------------------------*/
#define DotProduct3SOA_MAC( pV1, pV2 )		\
    _asm movaps  xmm0, [pV1]_SV3.x			\
    _asm movaps  xmm1, [pV1]_SV3.y			\
    _asm movaps  xmm2, [pV1]_SV3.z			\
    _asm mulps   xmm0, [pV2]_SV3.x			\
    _asm mulps   xmm1, [pV2]_SV3.y			\
    _asm addps   xmm0, xmm1					\
    _asm mulps   xmm2, [pV2]_SV3.z			\
    _asm addps   xmm0, xmm2


/*-------------------------------------------------------------------
*	Normalize3SOA_MAC (SOA_XYZ *pV)
*
*	Description:    Normalize 4 vectors with X, Y, Z components
*	Input Params:	eax = pV
-------------------------------------------------------------------*/
#define Normalize3SOA_MAC( pV )							\
    _asm movaps  xmm3, [pV]_SV3.x						\
    _asm movaps  xmm4, [pV]_SV3.y						\
    _asm movaps  xmm5, [pV]_SV3.z						\
    _asm mulps   xmm3, xmm3		/* x*x */				\
    _asm mulps   xmm4, xmm4		/* y*y */				\
    _asm movaps  xmm0, [pV]_SV3.x						\
    _asm mulps   xmm5, xmm5		/* z*z */				\
    _asm addps   xmm3, xmm4		/* x*x + y*y */			\
    _asm movaps  xmm1, [pV]_SV3.y						\
    _asm addps   xmm3, xmm5		/* x*x + y*y + z*z */	\
    _asm movaps  xmm2, [pV]_SV3.z						\
    _asm rsqrtps xmm3, xmm3		/* 1.0 / sqrt(x*x + y*y + z*z) */	\
    _asm mulps   xmm0, xmm3		/* normalize X */	\
    _asm mulps   xmm1, xmm3		/* normalize Y */	\
    _asm mulps   xmm2, xmm3		/* normalize Z */	\
    _asm movaps  [pV]_SV3.x, xmm0					\
    _asm movaps  [pV]_SV3.y, xmm1					\
    _asm movaps  [pV]_SV3.z, xmm2					


/*-------------------------------------------------------------------
*	Normalize3SOA_REG_MAC
*
*	Description:    Normalize 4 vectors with X, Y, Z components
*	Input Params:	xmm0 = x, xmm1 = y, xmm2 = z
*	Return:			xmm0 = x/|V|, xmm1 = y/|V|, xmm2 = z/|V|
-------------------------------------------------------------------*/
#define Normalize3SOA_REG_MAC							\
    _asm movaps  xmm3, xmm0								\
    _asm mulps   xmm3, xmm3		/* x*x */				\
    _asm movaps  xmm4, xmm1								\
    _asm mulps   xmm4, xmm4		/* y*y */				\
    _asm movaps  xmm5, xmm2								\
    _asm mulps   xmm5, xmm5		/* z*z */				\
    _asm addps   xmm3, xmm4		/* x*x + y*y */			\
    _asm addps   xmm3, xmm5		/* x*x + y*y + z*z */	\
    _asm rsqrtps xmm3, xmm3		/* 1.0 / sqrt(x*x + y*y + z*z) */	\
    _asm mulps   xmm0, xmm3		/* normalize X */	\
    _asm mulps   xmm1, xmm3		/* normalize Y */	\
    _asm mulps   xmm2, xmm3		/* normalize Z */	


#endif //TnL_HAL

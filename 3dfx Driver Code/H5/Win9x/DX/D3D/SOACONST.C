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
** File name: soaconst.c
**
** Description: SOA Constant Data
**
** $Revision: 14$
** $Date: 10/11/00 8:50:19 PM$
**
** $Log: 
**  14   3dfx      1.2.1.0.1.9 10/11/00 Brent           Forced check in to enforce
**       branching.
**  13   3dfx      1.2.1.0.1.8 09/22/00 Allen Hansen    added TL_soa_neg_2
**  12   3dfx      1.2.1.0.1.7 08/31/00 Allen Hansen    added global constant
**       exp_X16
**  11   3dfx      1.2.1.0.1.6 08/27/00 Allen Hansen    added TL_maskHigh16QW and
**       exp_X64K (used by 3dnow colorvertex)
**  10   3dfx      1.2.1.0.1.5 08/21/00 Allen Hansen    changed compiler check from
**       SSECPP to VCPP 
**  9    3dfx      1.2.1.0.1.4 07/29/00 Allen Hansen    added gbl_f128p0
**  8    3dfx      1.2.1.0.1.3 07/21/00 Allen Hansen    added a couple of masks
**  7    3dfx      1.2.1.0.1.2 07/05/00 Allen Hansen    support for asm clip code
**       generation to 3dnow code
**  6    3dfx      1.2.1.0.1.1 07/03/00 Allen Hansen    added 1 value
**  5    3dfx      1.2.1.0.1.0 06/29/00 Allen Hansen    Added a couple of values
**       needed for 3dnow
**  4    3dfx      1.2.1.0     06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  3    Napalm    1.2         03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  2    Napalm    1.1         03/14/00 Scott Kephart   Added TL_soa_0
**  1    Napalm    1.0         03/08/00 Scott Kephart   
** $
** 
** 2     3/10/00 4:39p Skephart
** Add TL_soa_0
** 
** 1     3/08/00 9:26p Skephart
*/

#ifdef TnL_HAL
#ifdef VCPP
#define __SOACONST 

#include "precomp.h"

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

__declspec(align(8)) const DWORD exp_mul256[2] = { (8 << 23), (8 << 23) };		// add this as a dword to a float, it multiplies the number by 256
__declspec(align(8)) const DWORD TL_sign_mask_low[2] = {0x80000000, 0x00000000};
__declspec(align(8)) const DWORD TL_sign_mask_high[2]  = {0x00000000, 0x80000000};
__declspec(align(16)) const DWORD TL_clip_mask[2] = {0x80000000, 0x80000000};
__declspec(align(16)) const DWORD TL_userclip_mask[2] = {TLCLIP_USERCLIPPLANE0, TLCLIP_USERCLIPPLANE0};
__declspec(align(16)) const float TL_one[4] = {   1.0f,   1.0f,   1.0f,   1.0f };
__declspec(align(16)) const float TL_255[4] = { 255.0f, 255.0f, 255.0f, 255.0f };
__declspec(align(16)) const float TL_256[4] = { 256.0f, 256.0f, 256.0f, 256.0f };

// Constants

__declspec(align(16)) const float TL_soa_0[4] =  {0.0f, 0.0f, 0.0f, 0.0f};
__declspec(align(16)) const float TL_soa_1[4] =  {1.0f, 1.0f, 1.0f, 1.0f};
__declspec(align(16)) const float TL_soa_2[4] =  {2.0f, 2.0f, 2.0f, 2.0f};
__declspec(align(16)) const float TL_soa_neg_1[4] =  {-1.0f, -1.0f, -1.0f, -1.0f};
__declspec(align(16)) const float TL_soa_neg_2[4] =  {-2.0f, -2.0f, -2.0f, -2.0f};
__declspec(align(16)) const float TL_soa_0pt5[4] = {0.5f, 0.5f, 0.5f, 0.5f}; 
__declspec(align(16)) const float TL_soa_neg_3[4] = {-3.0f, -3.0f, -3.0f, -3.0f}; 
__declspec(align(16)) const float TL_soa_255[4] =  {255.0f, 255.0f, 255.0f, 255.0f};



// Reciprocals

__declspec(align(16)) const float TL_soa_r2[4] = {(1.0f/2.0f), (1.0f/2.0f), (1.0f/2.0f), (1.0f/2.0f)}; 
__declspec(align(16)) const float TL_soa_r3[4] = {(1.0f/3.0f), (1.0f/3.0f), (1.0f/3.0f), (1.0f/3.0f)}; 
__declspec(align(16)) const float TL_soa_r4[4] = {(1.0f/4.0f), (1.0f/4.0f), (1.0f/4.0f), (1.0f/4.0f)}; 
__declspec(align(16)) const float TL_soa_r5[4] = {(1.0f/5.0f), (1.0f/5.0f), (1.0f/5.0f), (1.0f/5.0f)};
__declspec(align(16)) const float TL_soa_r6[4] = {(1.0f/6.0f), (1.0f/6.0f), (1.0f/6.0f), (1.0f/6.0f)}; 
__declspec(align(16)) const float TL_soa_r7[4] = {(1.0f/7.0f), (1.0f/7.0f), (1.0f/7.0f), (1.0f/7.0f)};
__declspec(align(16)) const float TL_soa_r8[4] = {(1.0f/8.0f), (1.0f/8.0f), (1.0f/8.0f), (1.0f/8.0f)}; 
__declspec(align(16)) const float TL_soa_r9[4] = {(1.0f/9.0f), (1.0f/9.0f), (1.0f/9.0f), (1.0f/9.0f)};

// Reciprocal factorials

__declspec(align(16)) const float TL_soa_r2f[4] = {(1.0f/2.0f), (1.0f/2.0f), (1.0f/2.0f), (1.0f/2.0f)}; 
__declspec(align(16)) const float TL_soa_r3f[4] = {(1.0f/6.0f), (1.0f/6.0f), (1.0f/6.0f), (1.0f/6.0f)}; 
__declspec(align(16)) const float TL_soa_r4f[4] = {(1.0f/24.0f), (1.0f/24.0f), (1.0f/24.0f), (1.0f/24.0f)}; 
__declspec(align(16)) const float TL_soa_r5f[4] = {(1.0f/120.0f), (1.0f/120.0f), (1.0f/120.0f), (1.0f/120.0f)};



__declspec(align(16)) const DWORD TL_hi_bits[4] =  {0x80000000, 0x80000000, 0x80000000, 0x80000000};

__declspec(align(16)) const DWORD TL_maskLow24[4] =  {0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff};
__declspec(align(16)) const DWORD TL_maskHigh8[4] =  {0xff000000, 0xff000000, 0xff000000, 0xff000000};

// 3DNow constants
__declspec(align(8)) const DWORD TLCLIPMASK_FRONT_0[2]		=  { (1 << (TLCLIP_FRONTBIT-1)), (0) };								// 	(z <  0) (  0   )
__declspec(align(8)) const DWORD TLCLIPMASK_BACK_0[2]		=  { (1 << (TLCLIP_BACKBIT -1)), (0) };								// 	(z >= w) (  0   )
__declspec(align(8)) const DWORD TLCLIPMASK_FRONT_BACK[2] 	=  { (1 << (TLCLIP_FRONTBIT-1)), (1 << (TLCLIP_BACKBIT -1)) };		// 	(z <  0) (z >= w)
__declspec(align(8)) const DWORD TLCLIPMASK_LEFT_BOTTOM[2]	=  { (1 << (TLCLIP_LEFTBIT -1)), (1 << (TLCLIP_BOTTOMBIT-1)) }; 	// 	(x <  0) (y <  0)
__declspec(align(8)) const DWORD TLCLIPMASK_RIGHT_TOP[2]	=  { (1 << (TLCLIP_RIGHTBIT-1)), (1 << (TLCLIP_TOPBIT-1)) };   		// 	(x >= w) (y >= w)
__declspec(align(8)) const DWORD TLCLIPGBMASK_LEFT_BOTTOM[2]=  { (1 << (TLCLIPGB_LEFTBIT -1)), (1 << (TLCLIPGB_BOTTOMBIT-1)) }; // 	(x <  0) (y <  0)
__declspec(align(8)) const DWORD TLCLIPGBMASK_RIGHT_TOP[2]	=  { (1 << (TLCLIPGB_RIGHTBIT-1)), (1 << (TLCLIPGB_TOPBIT-1)) };   	// 	(x >= w) (y >= w)


__declspec(align(8)) const __int64 TL_maskHigh16QW = 0xffff000000000000;
__declspec(align(8)) const DWORD exp_X16[2]  = { ( 4 << 23), ( 4 << 23) };
__declspec(align(8)) const DWORD exp_X4K[2]  = { (12 << 23), (12 << 23) };
__declspec(align(8)) const DWORD exp_X64K[2] = { (16 << 23), (16 << 23) };
__declspec(align(4)) const float gbl_f128p0 = 128.0;

#endif
#endif
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
** File name: soaconst.h
**
** Description: SOA Constant Data definitions
**
** $Revision: 13$
** $Date: 10/11/00 8:50:19 PM$
**
** $Log: 
**  13   3dfx      1.2.1.9     10/11/00 Brent           Forced check in to enforce
**       branching.
**  12   3dfx      1.2.1.8     09/22/00 Allen Hansen    added TL_soa_neg_2
**  11   3dfx      1.2.1.7     08/31/00 Allen Hansen    added global constant
**       exp_X16
**  10   3dfx      1.2.1.6     08/27/00 Allen Hansen    added TL_maskHigh16QW and
**       exp_X64K (used by 3dnow colorvertex)
**  9    3dfx      1.2.1.5     08/21/00 Allen Hansen    changed compiler check from
**       SSECPP to VCPP 
**  8    3dfx      1.2.1.4     07/29/00 Allen Hansen    added gbl_f128p0
**  7    3dfx      1.2.1.3     07/21/00 Allen Hansen    added a couple of masks
**  6    3dfx      1.2.1.2     07/05/00 Allen Hansen    support for asm clip code
**       generation to 3dnow code
**  5    3dfx      1.2.1.1     07/03/00 Allen Hansen    added exp_X4K (for 3DNow)
**  4    3dfx      1.2.1.0     06/29/00 Allen Hansen    Added a couple of values
**       needed for 3dnow
**  3    Napalm    1.2         03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  2    Napalm    1.1         03/14/00 Scott Kephart   Added TL_soa_0
**  1    Napalm    1.0         03/08/00 Scott Kephart   
** $
** 
** 2     3/10/00 4:43p Skephart
** Add TL_soa_0
** 
** 1     3/08/00 9:26p Skephart
*/

#ifdef TnL_HAL
#ifdef VCPP

#ifndef __SOACONST

extern const DWORD exp_mul256[2];
extern const DWORD TL_sign_mask_low[2];
extern const DWORD TL_sign_mask_high[2];

extern const DWORD TL_clip_mask[2];
extern const DWORD TL_userclip_mask[2];
extern const float TL_one[4];
extern const float TL_255[4];
extern const float TL_256[4];

// Constants

extern const float TL_soa_0[4];
extern const float TL_soa_1[4];
extern const float TL_soa_2[4];
extern const float TL_soa_neg_1[4];
extern const float TL_soa_neg_2[4];
extern const float TL_soa_0pt5[4];
extern const float TL_soa_neg_3[4];
extern const float TL_soa_255[4];



// Reciprocals

extern const float TL_soa_r2[4];
extern const float TL_soa_r3[4];
extern const float TL_soa_r4[4];
extern const float TL_soa_r5[4];
extern const float TL_soa_r6[4];
extern const float TL_soa_r7[4];
extern const float TL_soa_r8[4];
extern const float TL_soa_r9[4];

// Reciprocal factorials

extern const float TL_soa_r2f[4];
extern const float TL_soa_r3f[4];
extern const float TL_soa_r4f[4];
extern const float TL_soa_r5f[4];


extern const DWORD TL_hi_bits[4];

extern const DWORD TL_maskLow24[4];
extern const DWORD TL_maskHigh8[4];


// 3DNow constants
extern const DWORD TLCLIPMASK_FRONT_0[2];
extern const DWORD TLCLIPMASK_BACK_0[2];
extern const DWORD TLCLIPMASK_FRONT_BACK[2];
extern const DWORD TLCLIPMASK_LEFT_BOTTOM[2];
extern const DWORD TLCLIPMASK_RIGHT_TOP[2];
extern const DWORD TLCLIPGBMASK_LEFT_BOTTOM[2];
extern const DWORD TLCLIPGBMASK_RIGHT_TOP[2];
extern const __int64 TL_maskHigh16QW;


extern const DWORD exp_X16[2];
extern const DWORD exp_X4K[2];
extern const DWORD exp_X64K[2];
extern const float gbl_f128p0;


#endif

#endif
#endif
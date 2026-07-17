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
** File name: d3tri.h
**
** Description: Misc defines for Wrap variations,swapping,clamp,replicate macro,Triangle variations.
**
** $Revision: 3$
** $Date: 10/11/00 8:48:35 PM$
**
**
** $Log: 
**  3    3dfx      1.0.1.1     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.0.1.0     08/21/00 Allen Hansen    added processor pack (VCPP)
**       support for inline 3dnow assembly (instead of macros)
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 1     6/02/99 6:44a Michael
** Branch from H3
** 
** 8     1/29/99 10:49a Cshaw
** Added unified headers.
** 
** 7     12/09/98 6:40a Russ
** NT5 D3D changes for Banshee
**
** 6     11/04/98 4:09p Adrians
** Updated changes to K6-2.
**
** 5     7/24/98 1:37p Hohn
 *
 * 4     9/16/97 5:13p Adrians
 * Flat shading now uses RGB iterator rather than c0/c1.
 *
 * 3     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/
#ifndef __D3TRI_H__
#define __D3TRI_H__

//---------------------------------------------------
//
// A handful of macros use by the triangle setup code
//
//---------------------------------------------------
#define XY_ONE (1.0)

#ifndef WINNT
// handy swap macro
#undef  SWAP
#define SWAP(a, b, type)\
{                       \
    type tmp = (type)a; \
    (type)a = (type)b;  \
    (type)b = tmp;      \
}
#endif


//------------------------------------------------------------------
// Handy macro to add two colors togethor and clamp without alot of
// messy checking.
//
// This loses one bit of accuracy, but adds and clamps without ifs.
// We first mask all channels with 0xfe.  This leaves the lsb of
// each channel clear, so when the terms are added, any carry goes
// into the new highest bit.  Now all we have to do is generate a
// mask for any channels that have overflowed.  So we shift it right
// and eliminate everything but the overflow bits, so each channel
// contains either 0x00 or 0x01.  Subtracting each channel from 0x80
// produces 0x80 or 0x7f.  We just shift this left once and mask to
// give 0x00 or 0xfe.  (We could eliminate the final mask here, but
//it would introduce noise into the low-bit of every channel..)
//------------------------------------------------------------------

#define CLAMP888(result, color, specular) \
  do { \
     result = (color & 0xfefefefe) + (specular & 0x00fefefe); \
     result |= ((0x808080 - ((result >> 8) & 0x010101)) & 0x7f7f7f) << 1; \
  } while (0)

//-------------------------------------------------------------------------
// Macros received from Rendermorphics to "fixup" texture wraping. D3D does
// not wrap like SST-1. Concern: this is slow.
//-------------------------------------------------------------------------
float _inline d3absval(float val)
{
  if (val < 0)
    return(-val);
 else return(val);
}

#define ITOVALP(val, precision)  ((float)val)
#define DTOVALP(val, precision)  val

#define WRAP(uv, wrap)                                                      \
{                                                                           \
    if (wrap) {                                                             \
        /*                                                                  \
         * Calculate the three edge diffs to determine whether this         \
         * triangle crosses the boundary.  If it does, exactly two edge     \
         * diffs should have magnitude greater than 0.5.                    \
         */                                                                 \
        float  el, er, em;                                                  \
        el = d3absval(uv##3 - uv##1);                                       \
        er = d3absval(uv##2 - uv##3);                                       \
        em = d3absval(uv##1 - uv##2);                                       \
        /*                                                                  \
         * If we have two edges crossing the boundary, then we can adjust   \
         * the one vertex which is shared by those two edges by +-1 to      \
         * perform the wrapping operation.                                  \
         */                                                                 \
        if (el > DTOVALP(0.5, 24) && er > DTOVALP(0.5, 24)) {               \
            /*                                                              \
             * Shared vertex is uv##3.                                      \
             */                                                             \
            if (uv##3 < uv##2)                                              \
                uv##3 += ITOVALP(1, 24);                                    \
            else                                                            \
                { uv##2 += ITOVALP(1, 24); uv##1 += ITOVALP(1, 24); }       \
        } else if (er > DTOVALP(0.5, 24) && em > DTOVALP(0.5, 24)) {        \
            /*                                                              \
             * Shared vertex is uv##2.                                      \
             */                                                             \
            if (uv##2 < uv##1)                                              \
                uv##2 += ITOVALP(1, 24);                                    \
            else                                                            \
                { uv##1 += ITOVALP(1, 24); uv##3 += ITOVALP(1, 24); }       \
        } else if (em > DTOVALP(0.5, 24) && el > DTOVALP(0.5, 24)) {        \
            /*                                                              \
             * Shared vertex is uv##1.                                      \
             */                                                             \
            if (uv##1 < uv##3)                                              \
                uv##1 += ITOVALP(1, 24);                                    \
            else                                                            \
                { uv##3 += ITOVALP(1, 24); uv##2 += ITOVALP(1, 24); }       \
        }                                                                   \
    }                                                                       \
}

#ifdef K6_2

//these are now in d3global.c
//const float const_m1=-1.0f;
//const float const_5=0.5f;
//const float const_1=1.0f;

#ifdef VCPP
#define WRAP_3DNow(uv, wrap)                                            \
{                                                                       \
  if (wrap) {                                                           \
    int  c_el, c_er, c_em;                                              \
    DWORD  el, er, em;                                                  \
    __asm {  movd      mm0, uv##1 }                                     \
    __asm {  movd      mm1, uv##2 }                                     \
    __asm {  movd      mm2, uv##3 }                                     \
    __asm {  movq      mm3, mm0   }                                     \
    __asm {  movq      mm5, mm1   }                                     \
    __asm {  movq      mm6, mm2   }                                     \
    __asm {  movq      mm4, const_m1 }                                  \
    __asm {  pfsub     mm3, mm1 }  /* mm3: uv1 - uv2 (em) */            \
    __asm {  pfsub     mm1, mm2 }  /* mm1: uv2 - uv3 (er) */            \
    __asm {  pfsub     mm2, mm0 }  /* mm2: uv3 - uv1 (el) */            \
    __asm {  movd      dword ptr em, mm3 }                              \
    __asm {  movd      dword ptr er, mm1 }                              \
    __asm {  movd      dword ptr el, mm2 }                              \
    if ( *((int*)&em) & 0x80000000L ) { __asm { pfmul mm3, mm4 } }      \
    if ( *((int*)&er) & 0x80000000L ) { __asm { pfmul mm1, mm4 } }      \
    if ( *((int*)&el) & 0x80000000L ) { __asm { pfmul mm2, mm4 } }      \
    /* let's make comparision to el, er, em */                          \
    __asm {  punpckldq mm2, mm1 } /* mm2: er | el */                    \
    __asm {  movd      mm7, const_5}                                    \
    __asm {  punpckldq mm7, mm7}  /* mm7: 0.5 | 0.5 */                  \
    __asm {  pfcmpgt   mm2, mm7  }                                      \
    __asm {  movd      dword ptr c_el, mm2 }                            \
    __asm {  punpckhdq mm2, mm2}                                        \
    __asm {  movd      dword ptr c_er, mm2 }                            \
    __asm {  pfcmpgt   mm3, mm7  }                                      \
    __asm {  movd      dword ptr c_em, mm3 }                            \
    __asm {  movd       mm7, const_1 }                                  \
    /* mm0: uv1, mm5: uv2, mm6: uv3  */                                 \
    /*if (el > DTOVALP(0.5, 24) && er > DTOVALP(0.5, 24)) {*/           \
    if ( c_el && c_er) {                                                \
      if ( !(*((int*)&er) & 0x80000000L) )                              \
        { __asm { pfadd mm6, mm7 }  }                                   \
      else                                                              \
        { __asm { pfadd mm5, mm7 } __asm { pfadd mm0, mm7 } }           \
    }                                                                   \
    else                                                                \
      if (c_er && c_em ) {                                              \
        if ( !(*((int*)&em) & 0x80000000L) ) \
          { __asm { pfadd mm5, mm7 }  }                                 \
        else                                                            \
          { __asm { pfadd mm0, mm7 } __asm { pfadd mm6, mm7 } }         \
      }                                                                 \
      else                                                              \
        if (c_em && c_el) {                                             \
          if ( !(*((int*)&el) & 0x80000000L) ) \
            { __asm { pfadd mm0, mm7 }  }                               \
          else                                                          \
            { __asm { pfadd mm6, mm7 } __asm { pfadd mm5, mm7 } }       \
        }                                                               \
    __asm {  movd      uv##1, mm0 }                                     \
    __asm {  movd      uv##2, mm5 }                                     \
    __asm {  movd      uv##3, mm6 }                                     \
  }                                                                     \
}
#else // not VCPP (uses AMD's inline macro's
#define WRAP_3DNow(uv, wrap)                                            \
{                                                                       \
  if (wrap) {                                                           \
    int  c_el, c_er, c_em;                                              \
    float  el, er, em;                                                  \
    __asm {  movd      mm0, uv##1 }                                     \
    __asm {  movd      mm1, uv##2 }                                     \
    __asm {  movd      mm2, uv##3 }                                     \
    __asm {  movq      mm3, mm0   }                                     \
    __asm {  movq      mm5, mm1   }                                     \
    __asm {  movq      mm6, mm2   }                                     \
    __asm {  movq      mm4, const_m1 }                                  \
    __asm {  pfsub(mm3_mm1) }  /* mm3: uv1 - uv2 (em) */                \
    __asm {  pfsub(mm1_mm2) }  /* mm1: uv2 - uv3 (er) */                \
    __asm {  pfsub(mm2_mm0) }  /* mm2: uv3 - uv1 (el) */                \
    __asm {  movd      dword ptr em, mm3 }                              \
    __asm {  movd      dword ptr er, mm1 }                              \
    __asm {  movd      dword ptr el, mm2 }                              \
    if ( *((int*)&em) & 0x80000000L ) { __asm { pfmul(mm3_mm4) } }      \
    if ( *((int*)&er) & 0x80000000L ) { __asm { pfmul(mm1_mm4) } }      \
    if ( *((int*)&el) & 0x80000000L ) { __asm { pfmul(mm2_mm4) } }      \
    /* let's make comparision to el, er, em */                          \
    __asm {  punpckldq mm2, mm1 } /* mm2: er | el */                    \
    __asm {  movd      mm7, const_5}                                    \
    __asm {  punpckldq mm7, mm7}  /* mm7: 0.5 | 0.5 */                  \
    __asm {  pfcmpgt(mm2_mm7)  }                                        \
    __asm {  movd      dword ptr c_el, mm2 }                            \
    __asm {  punpckhdq mm2, mm2}                                        \
    __asm {  movd      dword ptr c_er, mm2 }                            \
    __asm {  pfcmpgt(mm3_mm7)  }                                        \
    __asm {  movd      dword ptr c_em, mm3 }                            \
    __asm {  movd       mm7, const_1 }                                  \
    /* mm0: uv1, mm5: uv2, mm6: uv3  */                                 \
    /*if (el > DTOVALP(0.5, 24) && er > DTOVALP(0.5, 24)) {*/           \
    if ( c_el && c_er) {                                                \
      if ( !(*((int*)&er) & 0x80000000L) )                              \
        { __asm { pfadd(mm6_mm7) }  }                                   \
      else                                                              \
        { __asm { pfadd(mm5_mm7) } __asm { pfadd(mm0_mm7) } }           \
    }                                                                   \
    else                                                                \
      if (c_er && c_em ) {                                              \
        if ( !(*((int*)&em) & 0x80000000L) ) \
          { __asm { pfadd(mm5_mm7) }  }                                 \
        else                                                            \
          { __asm { pfadd(mm0_mm7) } __asm { pfadd(mm6_mm7) } }         \
      }                                                                 \
      else                                                              \
        if (c_em && c_el) {                                             \
          if ( !(*((int*)&el) & 0x80000000L) ) \
            { __asm { pfadd(mm0_mm7) }  }                               \
          else                                                          \
            { __asm { pfadd(mm6_mm7) } __asm { pfadd(mm5_mm7) } }       \
        }                                                               \
    __asm {  movd      uv##1, mm0 }                                     \
    __asm {  movd      uv##2, mm5 }                                     \
    __asm {  movd      uv##3, mm6 }                                     \
  }                                                                     \
}
#endif //VCPP

#endif // K6_2

#define GET_ST(s, t, wrap_s, wrap_t)     \
        do {                             \
            WRAP(s, wrap_s);             \
            WRAP(t, wrap_t);             \
        } while (0)


//--------------------------------------------------------------------------
// _imgMSBReplicate
// Replicates the msb's of the input value to fill in an 8-bit color channel;
//--------------------------------------------------------------------------
#define _imgMSBReplicate( COLOR, LEFT_SHIFT, RIGHT_SHIFT ) (COLOR << LEFT_SHIFT) | (COLOR >> RIGHT_SHIFT)

// Optimizated triangle routines - if this changes then the corresponding table
// in d3trii.c must change!
typedef void (__stdcall *TRIFXN)(VOID *, DWORD, VOID *, LPD3DTLVERTEX);
#define INDEX_ALL      0  // all renderstates
#define INDEX_C        1  // flat shaded
#define INDEX_CSTW     2  // flat texture mapped
#define INDEX_CSTWA    3  // flat texture mapped alpha blended
#define INDEX_RGBSTWZ  4  // flat texture mapped alpha blended Z buffered
#define INDEX_RGBSTWZA 5  // gouraud texture mapped alpha blended
#define INDEX_RGBSTW   6  // gouraud texture mapped
#define INDEX_RGBSTWA  7  // gouraud texture mapped alpha blended
#define INDEX_MAX_OPTD 8

#endif //__D3TRI_H__

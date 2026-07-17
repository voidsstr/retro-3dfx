#if 1
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
 ** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
 ** rights reserved under the Copyright Laws of the United States.
 **
 ** $Header: xdraw_ppc.c, 2, 10/11/00 8:22:25 PM, Brent$
 ** $Log: 
 **  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
 **       branching.
 **  1    3dfx      1.0         09/12/99 StarTeam VTS Administrator 
 ** $
** 
** 2     8/23/99 2:47p Kcd
** Changes to make code compatible with AltiVec.
** 
** 1     7/02/99 4:37p Kcd
** Somewhat faster PowerPC trisetup code.
** 
** 63    8/24/98 6:45p Peter
** perf things for theo
** 
** 62    7/24/98 1:40p Hohn
** 
** 61    3/17/98 3:00p Peter
** removed vertex sorting
 * 
 * 60    12/01/97 6:13p Peter
 * non-packet3 tsu triangles ooz vs z
 * 
 * 59    11/21/97 3:20p Peter
 * direct writes tsu registers
 * 
 * 58    11/18/97 4:36p Peter
 * chipfield stuff cleanup and w/ direct writes
 * 
 * 57    11/17/97 4:55p Peter
 * watcom warnings/chipfield stuff
 * 
 * 56    11/12/97 9:54p Peter
 * fixed all the fuckage from new config
 * 
 * 55    11/03/97 4:38p Peter
 * yapc fix
 * 
 * 54    11/01/97 10:01a Peter
 * tri dispatch stuff
 * 
 * 53    10/29/97 2:45p Peter
 * C version of Taco's packing code
 * 
 * 52    10/27/97 5:59p Peter
 * removed some debugging code
 * 
 * 51    10/21/97 3:22p Peter
 * hand pack rgb
 * 
 * 50    10/19/97 12:51p Peter
 * no tsu happiness
 * 
 * 49    10/19/97 10:59a Peter
 * fixed p1 tsu writes
 * 
 * 48    10/17/97 3:15p Peter
 * removed unused addr field from datalist
 * 
 * 47    10/17/97 10:15a Peter
 * packed rgb state cleanup
 * 
 * 46    10/16/97 5:33p Peter
 * argb != rgba
 * 
 * 45    10/16/97 3:40p Peter
 * packed rgb
 * 
 * 44    10/16/97 10:31a Peter
 * fixed hoopti tsu-subtractor unsorted
 * 
 * 43    10/15/97 5:53p Peter
 * hoopti tri compare code
 * 
 * 42    10/10/97 4:33p Peter
 * non-packet3 tsu triangles
 * 
 * 41    10/08/97 5:19p Peter
 * optinally clamp only texture params
 * 
 * 40    10/08/97 11:32a Peter
 * pre-computed packet headers for packet 3
 * 
 * 39    9/20/97 4:42p Peter
 * tri_setf fixup/big fifo
 * 
 * 38    9/16/97 2:50p Peter
 * fixed watcom unhappiness w/ static initializers
 * 
 * 37    9/15/97 7:31p Peter
 * more cmdfifo cleanup, fixed normal buffer clear, banner in the right
 * place, lfb's are on, Hmmmm.. probably more
 * 
 * 36    9/10/97 10:13p Peter
 * fifo logic from GaryT, non-normalized fp first cut
 * 
 * 35    9/03/97 2:11p Peter
 * start gdbg_info cleanup, fixed zero area no-tsu triangle fuckage
 * 
 * 34    9/01/97 3:19p Peter
 * no-tsu w from vertex not tmuvtx
 * 
 * 33    8/31/97 4:06p Peter
 * no tsu fix
 * 
 * 32    8/31/97 12:04p Peter
 * hacked no-tsu code
 * 
 * 31    7/25/97 11:40a Peter
 * removed dHalf, change field name to match real use for cvg
 * 
 * 30    6/30/97 3:22p Peter
 * cmd fifo sanity
 * 
 * 29    6/24/97 4:02p Peter
 * proper cmd fifo placement
 * 
 * 28    6/23/97 4:43p Peter
 * cleaned up #defines etc for a nicer tree
 **
 */

#include <3dfx.h>
#include <glidesys.h>

#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include "fxglide.h"

#ifdef GDBG_INFO_ON
/* Some debugging information */
static char *indexNames[] = {  
  "GR_VERTEX_X_OFFSET",         /* 0 */
  "GR_VERTEX_Y_OFFSET",         /* 1 */
  "GR_VERTEX_Z_OFFSET",         /* 2 */
  "GR_VERTEX_R_OFFSET",         /* 3 */
  "GR_VERTEX_G_OFFSET",         /* 4 */
  "GR_VERTEX_B_OFFSET",         /* 5 */
  "GR_VERTEX_OOZ_OFFSET",       /* 6 */
  "GR_VERTEX_A_OFFSET",         /* 7 */
  "GR_VERTEX_OOW_OFFSET",       /* 8 */
  "GR_VERTEX_SOW_TMU0_OFFSET",  /* 9 */
  "GR_VERTEX_TOW_TMU0_OFFSET",  /* 10 */
  "GR_VERTEX_OOW_TMU0_OFFSET",  /* 11 */
  "GR_VERTEX_SOW_TMU1_OFFSET",  /* 12 */
  "GR_VERTEX_TOW_TMU1_OFFSET",  /* 13 */
  "GR_VERTEX_OOW_TMU1_OFFSET"	/* 14 */
};  
#endif

/*
 **  _trisetup_nogradients
 **
 **  This routine does all the setup needed for drawing a triangle.  It
 **  is intended to be an exact specification for the mechanisim used
 **  to pass vertices to the assembly language triangle setup code, and
 **  as such has no optimizations at all.  Whenever a 'shortcut'
 **  routine to draw triangles (such as for antialiasing, fast lines,
 **  fast spans, polygons, etc) is needed, this code should be used as
 **  the starting point.
 **
 */
GR_DDFUNC(_trisetup_nogradients,
          FxI32,
          (const GrVertex *va, const GrVertex *vb, const GrVertex *vc))
{
#define FN_NAME "_trisetup_nogradients"
  GR_DCL_GC;

  _GlideRoot.stats.trisProcessed++;

#if GLIDE_TRI_CULLING || GLIDE_TRI_DIST
  {
    const float *fa = &va->x;
    const float *fb = &vb->x;
    const float *fc = &vc->x;
    float dxAB, dxBC, dyAB, dyBC;
    
    /* Compute Area */
    dxAB = fa[GR_VERTEX_X_OFFSET] - fb[GR_VERTEX_X_OFFSET];
    dxBC = fb[GR_VERTEX_X_OFFSET] - fc[GR_VERTEX_X_OFFSET];
    
    dyAB = fa[GR_VERTEX_Y_OFFSET] - fb[GR_VERTEX_Y_OFFSET];
    dyBC = fb[GR_VERTEX_Y_OFFSET] - fc[GR_VERTEX_Y_OFFSET];
    
    /* Stash the area in the float pool for easy access */
    _GlideRoot.pool.ftemp1 = dxAB * dyBC - dxBC * dyAB;
    
#define FloatVal(__f) (((__f) < 786432.875) ? (__f) : ((__f) - 786432.875))
    {
      const FxI32 j = *(FxI32*)&_GlideRoot.pool.ftemp1;
      const FxU32 culltest = (gc->state.cull_mode << 31UL);
      
      /* Zero-area triangles are BAD!! */
      if ((j & 0x7FFFFFFF) == 0) {
        GDBG_INFO(291, FN_NAME": Culling (%g %g) (%g %g) (%g %g) : (%g : 0x%X : 0x%X)\n",
                  FloatVal(fa[0]), FloatVal(fa[1]), 
                  FloatVal(fb[0]), FloatVal(fb[1]), 
                  FloatVal(fc[0]), FloatVal(fc[1]), 
                  _GlideRoot.pool.ftemp1, gc->state.cull_mode, culltest);
        
        return 0;
      }
      
      /* Backface culling, use sign bit as test */
      if ((gc->state.cull_mode != GR_CULL_DISABLE) && (((FxI32)(j ^ culltest)) >= 0)) {
        GDBG_INFO(291, FN_NAME": Culling (%g %g) (%g %g) (%g %g) : (%g : 0x%X : 0x%X)\n",
                  FloatVal(fa[0]), FloatVal(fa[1]), 
                  FloatVal(fb[0]), FloatVal(fb[1]), 
                  FloatVal(fc[0]), FloatVal(fc[1]), 
                  _GlideRoot.pool.ftemp1, gc->state.cull_mode, culltest);
        
        return -1;
      }

    }
  }
#endif /* GLIDE_TRI_CULLING || GLIDE_TRI_DIST */

  /* Stuff for using hw tsu */
  {
    const float* vectorArray[3];

    /* Load up the real vertices */
    vectorArray[0] = &va->x;
    vectorArray[1] = &vb->x;
    vectorArray[2] = &vc->x;

    GR_SET_EXPECTED_SIZE(_GlideRoot.curTriSize, 1);

  { 
    union {
      FxU32 buffer[2];
      double buffer_double;
    } buff;
    int vectorIndex = 0, dataIndex;
    const float* vec = vectorArray[0];
    const int* dataList;

    FxU32* tPackPtr = gc->cmdTransportInfo.fifoPtr; 
    TRI_ASSERT_DECL(3, _GlideRoot.curVertexSize, gc->cmdTransportInfo.triPacketHdr);
    
    /* This is REALLY nasty to do in C... */
    if((FxU32)tPackPtr & 7) { 
    
      /* Destination not 8-byte aligned, so write the packet header directly
         to the command fifo */     
      SET(*tPackPtr++, gc->cmdTransportInfo.triPacketHdr);
      goto vertex_begin_empty;
    } else {
      /* Destination is aligned, so store packet header to command fifo */
      SET(buff.buffer[0], gc->cmdTransportInfo.triPacketHdr);
      goto vertex_begin_half;
    }

    /* Buffer is empty, so store X and Y and then flush it to memory */
vertex_begin_empty:
    SETF(buff.buffer[0],vec[GR_VERTEX_X_OFFSET]);
    SETF(buff.buffer[1],vec[GR_VERTEX_Y_OFFSET]);
    *(double *)tPackPtr = *(double *)&buff.buffer[0];
    tPackPtr += 2;
    dataList = gc->tsuDataList;
    goto vertex_loop_empty;
    
    /* Buffer is 1/2 full, so store X into second half and flush it to memory. */
vertex_begin_half:
    SETF(buff.buffer[1],vec[GR_VERTEX_X_OFFSET]);
    *(double *)tPackPtr = *(double *)&buff.buffer[0];
    tPackPtr += 2;
    SETF(buff.buffer[0],vec[GR_VERTEX_Y_OFFSET]);
    dataList = gc->tsuDataList;
    goto vertex_loop_half;
    
    /* Buffer is empty, next data item goes into entry 0. */
vertex_loop_empty:
    dataIndex = *dataList++;
    if(dataIndex == 0)
      goto vertex_end_empty;
    SETF(buff.buffer[0],FARRAY(vec, dataIndex));
    
    /* Buffer is half-full, next data item goes into entry 1, then we flush. */
vertex_loop_half:
    dataIndex = *dataList++;
    if(dataIndex == 0)
      goto vertex_end_half;
      
    SETF(buff.buffer[1],FARRAY(vec, dataIndex));
    *(double *)tPackPtr = *(double *)&buff.buffer[0];
    tPackPtr += 2;
    goto vertex_loop_empty;
    
    /* Buffer is empty, increment to next vector and continue */    
vertex_end_empty:
    vectorIndex++;
    if(vectorIndex < sizeof(vectorArray) / sizeof(float*)) {
      vec = vectorArray[vectorIndex];
      goto vertex_begin_empty;
    }
    goto vertex_end;
    
    /* Buffer is 1/2 full, increment to next vector and continue to unaligned read above. */
vertex_end_half:
    vectorIndex++;
    if(vectorIndex < sizeof(vectorArray) / sizeof(float*)) {
      vec = vectorArray[vectorIndex];
      goto vertex_begin_half;
    }
    
    /* Buffer is 1/2 full, must flush entry */
    *tPackPtr++ = buff.buffer[0];
    
vertex_end:
    /* TRI_ASSERT(); */
    gc->cmdTransportInfo.fifoRoom -= ((FxU32)tPackPtr - (FxU32)gc->cmdTransportInfo.fifoPtr);
    gc->cmdTransportInfo.fifoPtr = tPackPtr;
    GDBG_INFO(gc->myLevel + 200, "\tTriEnd: (0x%X : 0x%X)\n", tPackPtr, gc->cmdTransportInfo.fifoRoom);
    FIFO_ASSERT();
  }
   /* GR_CHECK_SIZE(); */
    goto __triDrawn;
  }

__triDrawn:
  /* If we made it this far then we drew the triangle */
  _GlideRoot.stats.trisDrawn++;
  /* GR_CHECK_SIZE(); */
  
  return 1;
    
#undef FN_NAME
} /* _trisetup_nogradients */
#endif

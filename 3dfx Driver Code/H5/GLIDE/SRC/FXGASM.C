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
** $Revision: 2$
** $Date: 10/11/00 8:21:21 PM$
*/

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <3dfx.h>
#include <glidesys.h>

#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include "fxglide.h"

/*----------------------------------------------------------------------
 * macros for creating assembler offset files
 *----------------------------------------------------------------------*/

#define NEWLINE printf("\n")
#define COMMENT printf(";----------------------------------------------------------------------\n")

#define HEADER(str)     NEWLINE; COMMENT; \
                        printf("; Assembler offsets for %s struct\n",str);\
                        COMMENT; NEWLINE

#define OFFSET(p,o,pname) if (hex) \
        printf("%s\t= %08xh\n",pname,((int)&p.o)-(int)&p); \
    else printf("%s\t= %10d\n",pname,((int)&p.o)-(int)&p)

#define OFFSET2(p,o,pname) if (hex) \
        printf("%s\t= %08xh\n",pname,((int)&o)-(int)&p); \
    else printf("%s\t= %10d\n",pname,((int)&o)-(int)&p)

#define SIZEOF(p,pname) if (hex) \
        printf("SIZEOF_%s\t= %08xh\n",pname,sizeof(p)); \
    else printf("SIZEOF_%s\t= %10d\n",pname,sizeof(p))

int
main (int argc, char **argv)
{
    int hex=0;          /* default is print in decimal  */
    static struct _GlideRoot_s gr;
    static GrGC gc;

#if !GLIDE_HW_TRI_SETUP
    static Sstregs sst;
    static struct dataList_s dl;
#endif /* !GLIDE_HW_TRI_SETUP */

    if (argc > 1) {
      if (strcmp("-inline", argv[1]) == 0) {
        SstRegs dummyRegs = { 0x00UL };

        printf("#ifndef __FX_INLINE_H__\n");
        printf("#define __FX_INLINE_H__\n");
        printf("\n");

#if GLIDE_DISPATCH_SETUP
        printf("#define kCurGCOffset   0x%XUL\n",
               offsetof(struct _GlideRoot_s, curGC));

        printf("#define kTriProcOffset 0x%XUL\n",
               offsetof(struct GrGC_s, archDispatchProcs.triSetupProc));
#endif /* GLIDE_DISPATCH_SETUP */
        
        printf("/* The # of 2-byte entries in the hw fog table */\n");
        printf("#define kInternalFogTableEntryCount 0x%XUL\n",
               sizeof(dummyRegs.fogTable) >> 1);

        printf("\n");
        printf("#endif /* __FX_INLINE_H__ */\n");

        return 0;
      }

      hex = 1;
    }

#if !GLIDE_HW_TRI_SETUP
    printf("SST_CHIP_MASK = 0%xh\n",SST_CHIP_MASK);
#endif

    HEADER ("SSTREGS");

    NEWLINE;
    HEADER ("GC");
    OFFSET (gc,base_ptr,"base_ptr\t");
    OFFSET (gc,reg_ptr,"reg_ptr\t\t");
    OFFSET (gc,lfb_ptr,"lfb_ptr\t\t");
    OFFSET (gc,tex_ptr,"tex_ptr\t\t");
#ifdef GLIDE_DEBUG
    OFFSET (gc,checkPtr,"checkPtr\t\t");
#endif
    OFFSET (gc,state.cull_mode,"cull_mode\t");
    OFFSET (gc, regDataList,"regDataList\t");
    OFFSET (gc, tsuDataList,"tsuDataList\t");
    OFFSET (gc, cmdTransportInfo.triPacketHdr, "triPacketHdr");
    OFFSET (gc, cmdTransportInfo.cullStripHdr, "cullStripHdr");
    OFFSET (gc, cmdTransportInfo.paramMask, "paramMask");

    OFFSET (gc, cmdTransportInfo.fifoStart, "fifoStart");
    OFFSET (gc, cmdTransportInfo.fifoEnd, "fifoEnd");
    OFFSET (gc, cmdTransportInfo.fifoOffset, "fifoOffset");
    OFFSET (gc, cmdTransportInfo.fifoSize, "fifoSize");
    OFFSET (gc, cmdTransportInfo.fifoJmpHdr, "fifoJmpHdr");
    OFFSET (gc, cmdTransportInfo.fifoPtr, "fifoPtr");
    OFFSET (gc, cmdTransportInfo.fifoRead, "fifoRead");

    OFFSET (gc, cmdTransportInfo.fifoRoom, "fifoRoom");
    OFFSET (gc, cmdTransportInfo.roomToReadPtr, "roomToReadPtr");
    OFFSET (gc, cmdTransportInfo.roomToEnd, "roomToEnd");

    OFFSET (gc, cmdTransportInfo.lfbLockCount, "lfbLockCount");

    HEADER("Chris's GC Stuff");
    OFFSET(gc, totBuffers, "gc_totBuffers");
    OFFSET(gc,     strideInTiles, "    gc_strideInTiles");
    OFFSET(gc, heightInTiles, "gc_heightInTiles");
    OFFSET(gc, bufferStride, "gc_tileStride");
    OFFSET(gc, bufSizeInTiles, "gc_bufSizeInTiles");
    OFFSET(gc, bufSize, "gc_bufSize");
    OFFSET(gc, fbOffset, "gc_fbOffset");
    OFFSET(gc, tramOffset, "gc_tramOffset");
    OFFSET(gc, base_ptr, "gc_base_ptr");
    OFFSET(gc, reg_ptr, "gc_reg_ptr");
    OFFSET(gc, tex_ptr, "gc_tex_ptr");
    OFFSET(gc, lfb_ptr, "gc_lfb_ptr");
    OFFSET(gc, lostcontext, "lostcontext");
  
#ifdef GLIDE_INIT_HWC
    OFFSET(gc, bInfo, "gc_bInfo");
#endif

#if GLIDE_MULTIPLATFORM
    OFFSET(gc, gcFuncs, "gc_gcFuncs");
#endif  
  
#if defined(GLIDE3) && defined(GLIDE3_ALPHA)
    OFFSET(gc, oemInit, "gc_oemInit");
#endif
  
    OFFSET(gc, cmdTransportInfo.fifoStart, "gc_cmdTransportInfo_fifoStart");
    OFFSET(gc, cmdTransportInfo.fifoEnd, "gc_cmdTransportInfo_fifoEnd");
    OFFSET(gc, cmdTransportInfo.fifoOffset, "gc_cmdTransportInfo_fifoOffset");
    OFFSET(gc, cmdTransportInfo.fifoSize, "gc_cmdTransportInfo_fifoSize");
    OFFSET(gc, cmdTransportInfo.fifoJmpHdr, "gc_cmdTransportInfo_fifoJmpHdr");
    
    OFFSET(gc, cmdTransportInfo.fifoPtr, "gc_cmdTransportInfo_fifoPtr");
    OFFSET(gc, cmdTransportInfo.fifoRead, "gc_cmdTransportInfo_fifoRead");
    
#if GLIDE_USE_DEBUG_FIFO && !HAL_CSIM
    OFFSET(gc, cmdTransportInfo.fifoShadowBase, "gc_cmdTransportInfo_fifoShadowBase");
    OFFSET(gc, cmdTransportInfo.fifoShadowPtr, "gc_cmdTransportInfo_fifoShadowPtr");
#endif /* GLIDE_USE_DEBUG_FIFO */
    
    /* Fifo checking information. In units of usuable bytes until
     * the appropriate condition.
     */
    OFFSET(gc, cmdTransportInfo.fifoRoom, "gc_cmdTransportInfo_fifoRoom");
    OFFSET(gc, cmdTransportInfo.roomToReadPtr, "gc_cmdTransportInfo_roomToReadPtr");
    OFFSET(gc, cmdTransportInfo.roomToEnd, "gc_cmdTransportInfo_roomToEnd");

    OFFSET(gc, cmdTransportInfo.lfbLockCount, "gc_cmdTransportInfo_lfbLockCount");
    
    OFFSET(gc, ioRegs, "gc_ioRegs");
    OFFSET(gc, cRegs, "gc_cRegs");
    OFFSET(gc, gRegs, "gc_gRegs");
    OFFSET(gc, sstRegs, "gc_sstRegs");

    OFFSET(gc, rawLfb, "gc_rawLfb");
    OFFSET(gc, nBuffers, "gc_nBuffers");
    OFFSET(gc, curBuffer, "gc_curBuffer");
    OFFSET(gc, frontBuffer, "gc_frontBuffer");
    OFFSET(gc, backBuffer, "gc_backBuffer");
    OFFSET(gc, buffers0, "gc_buffers");
  
    OFFSET(gc, counter, "gc_counter");

    OFFSET(gc, expected_counter, "gc_expected_counter");

    OFFSET(gc, checkCounter, "gc_checkCounter");
    OFFSET(gc, checkPtr, "gc_checkPtr");
   


    SIZEOF (gr.GCs[0].state,"GrState\t");
    SIZEOF (gr.hwConfig,"GrHwConfiguration");
    SIZEOF (gr.GCs[0],"GC\t");
    NEWLINE;

    HEADER ("GlideRoot");
    OFFSET (gr,p6Fencer,"p6Fencer\t");
    OFFSET (gr,current_sst,"current_sst\t");
    OFFSET (gr,CPUType,"CPUType\t\t");
    OFFSET (gr,curGC,"curGC\t\t");
    OFFSET (gr,curTriSize,"curTriSize\t");

    OFFSET (gr,stats.trisProcessed,"trisProcessed\t");
    OFFSET (gr,stats.trisDrawn,"trisDrawn\t");

#if GLIDE_PACKED_RGB
    OFFSET (gr, pool.ftemp1, "fTemp1");
    OFFSET (gr, pool.ftemp2, "fTemp2");
    OFFSET (gr, pool.fBiasHi,"fBiasHi");
    OFFSET (gr, pool.fBiasLo,"fBiasLo");
#endif /* (GLIDE_PLATFORM & GLIDE_HW_CVG) && GLIDE_PACKED_RGB */

    SIZEOF (gr,"GlideRoot");
    NEWLINE;

    HEADER ("GrVertex");
    {
      GrVertex v;

      OFFSET(v, x, "x");
      OFFSET(v, y, "y");
      OFFSET(v, r, "r");
      OFFSET(v, g, "g");
      OFFSET(v, b, "b");
      OFFSET(v, a, "a");
      SIZEOF(v, "GrVertex");
    }
    NEWLINE;

#if !GLIDE_HW_TRI_SETUP
    HEADER ("dataList");
    OFFSET (dl,i,"dl_i\t\t");
    OFFSET (dl,addr,"dl_addr\t\t");
    SIZEOF (dl,"dataList\t");
    NEWLINE;
#endif /* !GLIDE_HW_TRI_SETUP */

    return 0;
}

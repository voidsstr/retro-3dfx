/*-*-c++-*- */
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
** $Date: 10/11/00 8:10:25 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int n;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    while (DIAG_STARTPASS())                    // for each pass
    for (n=0; n<1; n++) {                       // do 1 test
        gdbg_info(2,"test32 fbzColorPath\n");
        register32test(&sst->fbzColorPath, 0xFFFFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 fogMode\n");
        register32test(&sst->fogMode, 0x00000FFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 alphaMode\n");
        register32test(&sst->alphaMode, 0xFFFFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 fbzMode\n");
        register32test(&sst->fbzMode, 0xFFFFFFFF, 0x003F3FFF & (diago.sliEnabled ? (~SST_YORIGIN) : (~0)));
        gdbg_info(2,"test32 lfbMode\n");
        register32test(&sst->lfbMode, 0xFFFFFFFF, 0x0001FFCF & (diago.sliEnabled ? (~SST_LFB_YORIGIN) : (~0)));
        gdbg_info(2,"test32 clipLeftRight\n");
        register32test(&sst->clipLeftRight,0x0FFF0FFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 clipBottomTop\n");
        register32test(&sst->clipBottomTop,0x0FFF0FFF, 0xFFFFFFFF);

	//For multi-chip shit, quit out early
	if(diago.chipCount > 1)
	  {
	    DIAG_PASS(-1);
	    exit(0);
	  }

        gdbg_info(2,"test32 stipple\n");
        register32test(&sst->stipple,0xFFFFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 c0\n");
        register32test(&sst->c0,0xFFFFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 c1\n");
        register32test(&sst->c1,0xFFFFFFFF, 0xFFFFFFFF);

	// new Avenger+ registers
        gdbg_info(2,"test32 renderMode\n");
        register32test(&sst->renderMode,0xFFFFFFFF, 0xFFFFFFFF);
#if 0	// combineMode is spec'd as write-only
        gdbg_info(2,"test32 combineMode\n");
        register32test(&sst->combineMode,0xFFFFFFFF, 0xFFFFFFFF);
#endif
        gdbg_info(2,"test32 stencilMode\n");
        register32test(&sst->stencilMode,0x0FFFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 stencilOp\n");
        register32test(&sst->stencilOp,0x00000FFF, 0xFFFFFFFF);
#ifdef CVG
        gdbg_info(2,"test32 bltSrcBaseAddr\n");
        register32test(&sst->bltSrcBaseAddr,0x003FFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 bltDstBaseAddr\n");
        register32test(&sst->bltDstBaseAddr,0x003FFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 bltXYstrides\n");
        register32test(&sst->bltXYstrides,0x0FFF0FFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 bltSrcChromaRange\n");
        register32test(&sst->bltSrcChromaRange,0xFFFFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 bltDstChromaRange\n");
        register32test(&sst->bltDstChromaRange,0xFFFFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 bltClipX\n");
        register32test(&sst->bltClipX,0x0FFF0FFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 bltClipY\n");
        register32test(&sst->bltClipY,0x0FFF0FFF, 0xFFFFFFFF);

        gdbg_info(2,"test32 bltSrcXY\n");
        register32test(&sst->bltSrcXY,0x07FF07FF, 0xFFFFFFFF);
        gdbg_info(2,"test32 bltDstXY\n");
        register32test(&sst->bltDstXY,0x87FF07FF, ~SSTG_GO);
        gdbg_info(2,"test32 bltSize\n");
        register32test(&sst->bltSize,0x8FFF0FFF, ~SSTG_GO);
        gdbg_info(2,"test32 bltRop\n");
        register32test(&sst->bltRop,0x0000FFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 bltColor\n");
        register32test(&sst->bltColor,0xFFFFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 bltCommand\n");
        register32test(&sst->bltCommand,0x8003FFFF, ~SSTG_GO);
#else
        {
          // save and restore col/aux buffer base/stride so 
          // DIAG_DIFFSCREEN() doesn't generate illegal addresses

          FxU32 colBufferAddr = GET(sst->colBufferAddr);
          FxU32 colBufferStride = GET(sst->colBufferStride);
          FxU32 auxBufferAddr = GET(sst->auxBufferAddr);
          FxU32 auxBufferStride = GET(sst->auxBufferStride);
          
          gdbg_info(2,"test32 colBufferAddr\n");
          register32test(&sst->colBufferAddr,0x00FFFFFF, 0x7FFFFFFF);
          gdbg_info(2,"test32 colBufferStride\n");
          register32test(&sst->colBufferStride,0x0000BFFF, 0xFFFFFFFF);
          gdbg_info(2,"test32 auxBufferAddr\n");
          register32test(&sst->auxBufferAddr,0x00FFFFFF, 0x7FFFFFFF);
          gdbg_info(2,"test32 auxBufferStride\n");
          register32test(&sst->auxBufferStride,0x0000BFFF, 0xFFFFFFFF);
          
          SET(sst->colBufferAddr, colBufferAddr);
          SET(sst->colBufferStride, colBufferStride);
          SET(sst->auxBufferAddr, auxBufferAddr);
          SET(sst->auxBufferStride, auxBufferStride);
        }
        gdbg_info(2,"test32 clipLeftRight1\n");
        register32test(&sst->clipLeftRight1,0x8FFF0FFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 clipBottomTop1\n");
        register32test(&sst->clipBottomTop1,0x8FFF0FFF, 0xFFFFFFFF);
#endif
        // NOTE: we cannot write pixel counter registers so we cannot
        //       test them here
    }
    DIAG_PASS(-1);
}

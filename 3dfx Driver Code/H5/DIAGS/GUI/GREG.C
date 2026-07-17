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
** $Revision: 2$
** $Date: 10/11/00 8:11:13 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
    int i,n;
    SstRegs *sst;
    SstGRegs *sstg;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);

    while (DIAG_STARTPASS())                    // for each pass
    for (n=0; n<1; n++) {                       // do 1 test
        gdbg_info(2,"test32 clip0min\n");
        register32test(&sstg->clip0min,0x0FFF0FFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 clip0max\n");
        register32test(&sstg->clip0max,0x0FFF0FFF, 0xFFFFFFFF);

        gdbg_info(2,"test32 dstBaseAddr\n");
        register32test(&sstg->dstBaseAddr,0x80FFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 dstFormat\n");
        register32test(&sstg->dstFormat,0x00073FFF, 0xFFFFFFFF);

        gdbg_info(2,"test32 srcColorkeyMin\n");
        register32test(&sstg->srcColorkeyMin,0x00FFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 srcColorkeyMax\n");
        register32test(&sstg->srcColorkeyMax,0x00FFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 dstColorkeyMin\n");
        register32test(&sstg->dstColorkeyMin,0x00FFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 dstColorkeyMax\n");
        register32test(&sstg->dstColorkeyMax,0x00FFFFFF, 0xFFFFFFFF);

        gdbg_info(2,"test32 bresError0\n");
        register32test(&sstg->bresError0,0x8000FFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 bresError1\n");
        register32test(&sstg->bresError1,0x8000FFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 rop\n");
        register32test(&sstg->rop,0x00FFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 srcBaseAddr\n");
        register32test(&sstg->srcBaseAddr,0x80FFFFFF, 0xFFFFFFFF);

        gdbg_info(2,"test32 commandEx\n");
        register32test(&sstg->commandEx,0x00000007, 0xFFFFFFFF);
        gdbg_info(2,"test32 lineStipple\n");
        register32test(&sstg->lineStipple,0xFFFFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 lineStyle\n");
        register32test(&sstg->lineStyle,0x1FFF1FFF, 0xFFFFFFFF);

        gdbg_info(2,"test32 pattern0alias\n");
        register32test(&sstg->pattern0alias,0xFFFFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 pattern1alias\n");
        register32test(&sstg->pattern1alias,0xFFFFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 clip1min\n");
        register32test(&sstg->clip1min,0x0FFF0FFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 clip1max\n");
        register32test(&sstg->clip1max,0x0FFF0FFF, 0xFFFFFFFF);

        gdbg_info(2,"test32 srcSize\n");
        register32test(&sstg->srcSize,0x1FFF1FFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 srcXY\n");
        register32test(&sstg->srcXY,0x1FFF1FFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 colorBack\n");
        register32test(&sstg->colorBack,0xFFFFFFFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 colorFore\n");
        register32test(&sstg->colorFore,0xFFFFFFFF, 0xFFFFFFFF);

        gdbg_info(2,"test32 dstSize\n");
        register32test(&sstg->dstSize,0x1FFF1FFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 dstXY\n");
        register32test(&sstg->dstXY,0x1FFF1FFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 srcFormat\n");
        register32test(&sstg->srcFormat,0x00FF3FFF, 0xFFFFFFFF);
        gdbg_info(2,"test32 command\n");
        register32test(&sstg->command,0xFFFFFF0F, ~SSTG_GO);

        for (i=0; i<64; i++) {
            gdbg_info(2,"test32 colorPattern[%d]\n",i);
            register32test(&sstg->colorPattern[i],0xFFFFFFFF, 0xFFFFFFFF);
        }
#if COLORTRANSLUT
        for (i=0; i<256; i++) {
            gdbg_info(2,"test32 colorTransLut[%d]\n",i);
            register32test(&sstg->colorTransLut[i],0x00FFFFFF, 0xFFFFFFFF);
        }
#endif
    }

    SET(sstg->dstBaseAddr,diago.minTrashMem);           // redefine a default surface
    SET(sstg->dstFormat,SSTG_PIXFMT_32BPP | diago.xmaxscreen*4);
    DIAG_PASS(-1);
}

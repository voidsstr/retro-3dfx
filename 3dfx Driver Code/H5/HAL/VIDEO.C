#include "vxd.h"

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
** $Revision: 3$
** $Date: 10/11/00 8:31:16 PM$
*/

#include <stdlib.h>

#include <h3.h>
#include "../csim/csim.h"

#ifdef HAL_HW
#include <h3cinit.h>
#endif

FxBool FX_EXPORT FX_CSTYLE
fxHalInitVideo( SstRegs *sst, FxU32 resolution, FxU32 refresh,
                FxVideoTimingInfo *vti)
{
#ifndef KERNEL
    FxU32 ioBase;
#endif

    FxU32 bn;
    FxDeviceInfo *info;

    GDBG_INFO(1,"fxHalInitVideo(0x%x,0x%x,0x%x)\n",sst,resolution,refresh);
    if (fxHalVaddrToBoardNumber( sst, &bn ))    // find the board
        info = &halInfo.boardInfo[bn];
    else
        return FXFALSE;

    switch(resolution) {
        case(GR_RESOLUTION_400x300):
            info->fbiVideoWidth = 400;
            info->fbiVideoHeight = 300;
            break;
        case(GR_RESOLUTION_512x256):
            info->fbiVideoWidth = 512;
            info->fbiVideoHeight = 256;
            break;
        case(GR_RESOLUTION_512x384):
            info->fbiVideoWidth = 512;
            info->fbiVideoHeight = 384;
            break;
        case(GR_RESOLUTION_640x400):
            info->fbiVideoWidth = 640;
            info->fbiVideoHeight = 400;
            break;
        case(GR_RESOLUTION_640x480):
            info->fbiVideoWidth = 640;
            info->fbiVideoHeight = 480;
            break;
        case(GR_RESOLUTION_800x600):
            info->fbiVideoWidth = 800;
            info->fbiVideoHeight = 600;
            break;
        case(GR_RESOLUTION_856x480):
            info->fbiVideoWidth = 856;
            info->fbiVideoHeight = 480;
            break;
        case(GR_RESOLUTION_960x720):
            info->fbiVideoWidth = 960;
            info->fbiVideoHeight = 720;
            break;
        case(GR_RESOLUTION_1024x768):
            info->fbiVideoWidth = 1024;
            info->fbiVideoHeight = 768;
            break;
        case(GR_RESOLUTION_1280x1024):
            info->fbiVideoWidth = 1280;
            info->fbiVideoHeight = 1024;
            break;
        case(GR_RESOLUTION_1600x1200):
            info->fbiVideoWidth = 1600;
            info->fbiVideoHeight = 1200;
            break;
        case(GR_RESOLUTION_1792x1344):
            info->fbiVideoWidth = 1792;
            info->fbiVideoHeight = 1344;
            break;
        case(GR_RESOLUTION_1856x1392):
            info->fbiVideoWidth = 1836;
            info->fbiVideoHeight = 1392;
            break;
        case(GR_RESOLUTION_1920x1440):
            info->fbiVideoWidth = 1920;
            info->fbiVideoHeight = 1440;
            break;
        case(GR_RESOLUTION_2048x1536):
            info->fbiVideoWidth = 2048;
            info->fbiVideoHeight = 1536;
            break;
        default:
            GDBG_ERROR("fxHalInitVideo", "Unsupported Resolution.\n");
            return(FXFALSE);
            break;
    }

    switch(refresh) {
        case(GR_REFRESH_60Hz):
            info->fbiVideoRefresh = 60;
            break;
        case(GR_REFRESH_70Hz):
            info->fbiVideoRefresh = 70;
            break;
        case(GR_REFRESH_72Hz):
            info->fbiVideoRefresh = 72;
            break;
        case(GR_REFRESH_75Hz):
            info->fbiVideoRefresh = 75;
            break;
        case(GR_REFRESH_80Hz):
            info->fbiVideoRefresh = 80;
            break;
        case(GR_REFRESH_85Hz):
            info->fbiVideoRefresh = 85;
            break;
        case(GR_REFRESH_90Hz):
            info->fbiVideoRefresh = 90;
            break;
        case(GR_REFRESH_100Hz):
            info->fbiVideoRefresh = 100;
            break;
        case(GR_REFRESH_120Hz):
            info->fbiVideoRefresh = 120;
            break;
        default:
            GDBG_ERROR("fxHalInitVideo", "Unsupported Refresh Rate.\n");
            return(FXFALSE);
            break;
    }

#ifndef KERNEL
    pciGetConfigData(PCI_IO_BASE_ADDRESS, info->deviceNumber, &ioBase);

    ioBase &= ~1;
#endif

#ifdef HAL_HW
    if (halInfo.hw)
    {
	h3InitVideoProc(ioBase, SST_VIDEO_PROCESSOR_EN);
    }
#endif

    {  
        SstIORegs *sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
        FxU32 fbSize = info->fbiVideoWidth * info->fbiVideoHeight * 2;

        fxHalIdleNoNop(sst);
#if 0
// GMT: we now use a bit in the renderMode 3D register to do this
    {
	FxU32 temp; 
        /* Initialize Y-Origin */
        temp = IGET(sstio->miscInit0) & ~SST_YORIGIN_TOP;
        ISET(sstio->miscInit0,
             temp | (info->fbiVideoHeight - 1) << SST_YORIGIN_TOP_SHIFT); 
        fxHalIdleNoNop(sst);
    }
#endif

        // init base and stride in a linear fashion
        // base and stride must be 16-byte aligned
        SET(sst->colBufferAddr,0);
        SET(sst->colBufferStride,info->fbiVideoWidth * 2);
        SET(sst->auxBufferAddr,fbSize*2);               // skip over backbuffer
        SET(sst->auxBufferStride,info->fbiVideoWidth * 2);
    }

#ifdef HAL_HW
    if (halInfo.hw)
    {
	h3InitSetVideoMode(
	    (FxU16) ioBase,
	    info->fbiVideoWidth,
	    info->fbiVideoHeight, 
	    info->fbiVideoRefresh,
	    1);				// initialize CLUT entries
    }
#endif /* #ifdef HAL_HW */

    csimFbiSetBuffers(halInfo.boardInfo[bn].sstCSIM);   // init the front,back,aux buffers

    if(bn == 0)
      guiNewViewWindow(bn,CSIM_VIEW_WINDOW_NAME);

    return FXTRUE;
} // fxHalInitVideo


FxBool FX_EXPORT FX_CSTYLE
#ifndef NO_FLOAT
fxHalInitGamma( SstRegs *sst, FxFloat gamma )
#else
fxHalInitGamma( SstRegs *sst, FxU32 gamma )
#endif /* #ifndef NO_FLOAT */
{
    GDBG_INFO(1,"fxHalInitGamma(0x%x,%g)\n",sst,gamma);
    if(!sst)
        return(FXFALSE);
    // GMT: implement gamma by saving the info away in SST somewhere
    // and then offering a video display option
    return FXTRUE;
}




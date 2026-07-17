/* -*-c++-*- */
/* */
/*
** Copyright (c) 1997-1998, 3Dfx Interactive, Inc.
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

Module Name:

	h3modeset.c

Abstract:

	This module performs the refresh rate and mode set functions. We use this
	because the effort and rewrite required to make the old code support ULONG
	ops is hardly worth the trouble.

Environment:

	Kernel mode

Revision History:


--*/

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "h3.h"
#include "localpci.h"
#include "cmdcnst.h"

#define TVOUT_SUPPORTED 1
#ifdef TVOUT_SUPPORTED
extern int BT868_Enable (PHW_DEVICE_EXTENSION HwDeviceExtension, int vgaMode);
#include "dfp.h"
#endif //def TVOUT_SUPPORTED

UCHAR vgaAttributes[20] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0F, 0x00
};

	
// translate a bits per pixel into a video processor
// desktop pixel format setting.

ULONG
bppToPixfmt(ULONG bpp)
{
	switch (bpp)
	{
	case 8:
		return SST_DESKTOP_PIXEL_PAL8;
	case 16:
		return SST_DESKTOP_PIXEL_RGB565;
	case 24:
		return SST_DESKTOP_PIXEL_RGB24;
	case 32:
		return SST_DESKTOP_PIXEL_RGB32;
	default:
		VideoDebugPrint((0, "bppToPixfmt: bad bpp value: %d\n", bpp));
		return 0;
	}
}

VOID
LockVgaTimingRegisters(PHW_DEVICE_EXTENSION HwDeviceExtension,
                       PH3_MEMBASE0         RegisterMap)
{
	ULONG vgainit1;
	
	// lock VGA video timing registers
	//
	vgainit1 = RegisterMap->vgaInit1;
	vgainit1 |= (BIT(21) | BIT(22) | BIT(23) | BIT(24)
			 |	 BIT(25) | BIT(26) | BIT(27) | BIT(28));
	RegisterMap->vgaInit1 = vgainit1;
}


VOID
UnlockVgaTimingRegisters(PHW_DEVICE_EXTENSION HwDeviceExtension,
                         PH3_MEMBASE0         RegisterMap)
{
	ULONG vgainit1;
	
	// unlock VGA video timing registers
	//
	vgainit1 = RegisterMap->vgaInit1;
	vgainit1 &= ~(BIT(21) | BIT(22) | BIT(23) | BIT(24)
			 |	  BIT(25) | BIT(26) | BIT(27) | BIT(28));
	RegisterMap->vgaInit1 = vgainit1;
}

VP_STATUS
H3SetRefreshRate(PHW_DEVICE_EXTENSION   HwDeviceExtension,
                 PH3_MEMBASE0           RegisterMap,
                 PUCHAR                 pIO,
                 PH3_VIDEO_FREQUENCIES  FrequencyEntry)
/*++

Routine Description:

	Takes refresh rate information and programs proper registers in Banshee.
	This routine only gets called during BIOS mode sets.

Arguments:

	HwDeviceExtension - Pointer to the miniport driver's device extension.

Return Value:

	NO_ERROR, or ERROR_INVALID_PARAMETER

--*/
{
	VP_STATUS status = NO_ERROR;

	PUCHAR reg = FrequencyEntry->VideoData;
	ULONG temp, i, j;

	VideoDebugPrint((2, "H3SetRefreshRate - "));

	//
	// Scan the video frequency settings for correctness,
	// Invalid settings are all zero.
	//
	j = 0;
	for (i = 0; i < 21; i++)
	{
		j += reg[i];
	}

	if (j == 0)
	{
		VideoDebugPrint((0, "H3SetRefreshRate - invalid frequency settings\n"));
		return ERROR_INVALID_PARAMETER;
	}

	//
	// unlock the CRTC, and then program it
	//
	VideoPortWritePortUchar((PUCHAR)(pIO + 0x0c2), (UCHAR)(reg[16] | BIT(0)));

	// mystical VGA magic
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)0x0011);
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[0] << 8) | 0x00));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[1] << 8) | 0x01));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[2] << 8) | 0x02));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[3] << 8) | 0x03));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[4] << 8) | 0x04));

	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[5] << 8) | 0x05));

	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[6] << 8) | 0x06));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[7] << 8) | 0x07));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[8] << 8) | 0x09));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[9] << 8) | 0x10));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[10] << 8) | 0x11));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[11] << 8) | 0x12));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[12] << 8) | 0x15));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[13] << 8) | 0x16));

	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[14] << 8) | 0x1a));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[15] << 8) | 0x1b));

	// Enable sync outputs
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)0x8017);

	// set the video clock to the proper frequency
	RegisterMap->pllCtrl0 = (ULONG)((reg[19] << 8) | reg[18]); //VideoPortWritePortUlong((PULONG)(pIO + PLLCTRL0), (USHORT)((reg[19] << 8) | reg[18]));

	// set up 1x or 2x mode for the DAC
	RegisterMap->dacMode = (ULONG)reg[20]; //VideoPortWritePortUlong((PULONG)(pIO + DACMODE), (ULONG)reg[20]);

	// Sequencer registers (set run mode, not reset)
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xc4), (USHORT)((reg[17] & 0xdf) << 8 | 0x1));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xc4), (USHORT)0x0300);

	// make sure the attribute register is initialized
	temp = VideoPortReadPortUchar((PUCHAR)(pIO + 0x0da));

	// set attribute registers
	for (i = 0; i < 20; i++)
	{
		VideoPortWritePortUchar((PUCHAR)(pIO + 0xc0), (UCHAR) i);
		VideoPortWritePortUchar((PUCHAR)(pIO + 0xc0), vgaAttributes[i]);
	}

	VideoPortWritePortUchar((PUCHAR)(pIO + 0xc0), (UCHAR)0x34);
	VideoPortWritePortUchar((PUCHAR)(pIO + 0xda), (UCHAR)0x00);

	//
	// turn off VGA's screen refresh, as this function only sets extended
	// video modes, and the VGA screen refresh eats up performance
	// (10% difference in screen to screen blits!).   This code is not in
	// the perl, but should stay here unless specifically decided otherwise
	//
	temp = RegisterMap->vgaInit0; //VideoPortReadPortUlong((PULONG)(pIO + VGAINIT0));
	temp |= BIT(12);
	RegisterMap->vgaInit0 = temp; //VideoPortWritePortUlong((PULONG)(pIO + VGAINIT0), temp);

	//
	// write the CLUT with the ramp
	//

	for (i = 0; i < 0x100; i++)
	{
		VideoPortWritePortUlong((PULONG)(pIO + DACADDR), i);
		temp = VideoPortReadPortUlong((PULONG)(pIO + DACADDR));

		j = i & 0xff;
		temp = ((j << 16) | (j << 8) | j);

		VideoPortWritePortUlong((PULONG)(pIO + DACDATA), temp);
		temp = VideoPortReadPortUlong((PULONG)(pIO + DACADDR));
	}

	H3UpdateGamma(HwDeviceExtension, RegisterMap, pIO, HwDeviceExtension->GammaTable);

	// again, per andy
	//
	// clear bit 10 and 11 in the vidProcCfg register so that the CLUT
	// is not bypassed
	//
	temp = RegisterMap->vidProcCfg; //VideoPortReadPortUlong((PULONG)(pIO + VIDPROCCFG));

	temp &= ~SST_BYPASS_CLUT;

	RegisterMap->vidProcCfg = temp; //VideoPortWritePortUlong((PULONG)(pIO + VIDPROCCFG), temp);

	VideoDebugPrint((2, "H3SetRefreshRate - done\n"));

	return status;
}

VOID
H3InitDesktopSurface(PHW_DEVICE_EXTENSION   HwDeviceExtension,
                     PH3_MEMBASE0           RegisterMap,
                     PUCHAR                 pIO,
                     PH3_VIDEO_FREQUENCIES  FrequencyEntry,
                     PVID_PROC_CONFIG       pvidInfo)
{
	ULONG doStride;
	ULONG vidProcCfg;

	UNREFERENCED_PARAMETER(FrequencyEntry);

	VideoDebugPrint((2, "H3InitDesktopSurface - \n"));

	vidProcCfg = RegisterMap->vidProcCfg;

	vidProcCfg &= ~ (SST_DESKTOP_EN
				| SST_DESKTOP_TILED_EN
				| SST_DESKTOP_PIXEL_FORMAT
				| SST_DESKTOP_CLUT_BYPASS
				| SST_DESKTOP_CLUT_SELECT);

	if (pvidInfo->dtSurface.enable)
		vidProcCfg |= SST_DESKTOP_EN;

	if (pvidInfo->dtSurface.tiled)
		vidProcCfg |= ~SST_DESKTOP_TILED_EN;

	vidProcCfg |= pvidInfo->dtSurface.pixFmt;

	if (pvidInfo->dtSurface.clutBypass)
		vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;

	if (pvidInfo->dtSurface.clutSelect)
		vidProcCfg |= SST_DESKTOP_CLUT_SELECT;

	RegisterMap->vidProcCfg = vidProcCfg;

	RegisterMap->vidDesktopStartAddr = 0; // ((ULONG)HwDeviceExtension->MappedAddress[SST_FB_INDEX] & SST_VIDEO_START_ADDR) << SST_VIDEO_START_ADDR_SHIFT;

	// change only the desktop portion of the vidDesktopOverlayStride register
	//
	doStride = RegisterMap->vidDesktopOverlayStride;
	doStride &= ~(SST_DESKTOP_LINEAR_STRIDE | SST_DESKTOP_TILE_STRIDE);
	pvidInfo->dtSurface.stride <<= SST_DESKTOP_STRIDE_SHIFT;
	if (pvidInfo->dtSurface.tiled)
		pvidInfo->dtSurface.stride &= SST_DESKTOP_TILE_STRIDE;
	else
		pvidInfo->dtSurface.stride &= SST_DESKTOP_LINEAR_STRIDE;
	doStride |= pvidInfo->dtSurface.stride;

	RegisterMap->vidDesktopOverlayStride = doStride;
}

VOID
H3InitOverlaySurface(PHW_DEVICE_EXTENSION   HwDeviceExtension,
                     PH3_MEMBASE0           RegisterMap,
                     PUCHAR                 pIO,
                     PH3_VIDEO_FREQUENCIES  FrequencyEntry,
                     PVID_PROC_CONFIG       pvidInfo)
{
  //PH3_3D_REGISTERS RegisterMap3D = (PH3_3D_REGISTERS)((PUCHAR)RegisterMap + 0x200000);
	ULONG doStride;
	ULONG vidProcCfg = RegisterMap->vidProcCfg;

	UNREFERENCED_PARAMETER(FrequencyEntry);

	VideoDebugPrint((2, "H3InitOverlaySurface - \n"));

	//
	// Somebody may set these so we need to reset them.
	//
	RegisterMap->miscInit0 = 0;
	RegisterMap->lfbMemoryConfig = 0x3fff;

	vidProcCfg &= ~ (SST_OVERLAY_EN
			   |	 SST_OVERLAY_TILED_EN
			   |	 SST_OVERLAY_STEREO_EN
			   |	 SST_OVERLAY_HORIZ_SCALE_EN
			   |	 SST_OVERLAY_VERT_SCALE_EN
			   |	 SST_OVERLAY_TILED_EN
			   |	 SST_OVERLAY_PIXEL_FORMAT
			   |	 SST_OVERLAY_CLUT_BYPASS
			   |	 SST_OVERLAY_CLUT_SELECT
			   |	 SST_OVERLAY_FILTER_MODE);

	if (pvidInfo->ovSurface.enable)
		vidProcCfg |= SST_OVERLAY_EN;

	if (pvidInfo->ovSurface.stereo)
		vidProcCfg |= SST_OVERLAY_STEREO_EN;

	if (pvidInfo->ovSurface.horizScaling)
		vidProcCfg |= SST_OVERLAY_HORIZ_SCALE_EN;

	if (pvidInfo->ovSurface.verticalScaling)
		vidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;

	if (pvidInfo->ovSurface.tiled)
		vidProcCfg |= SST_OVERLAY_TILED_EN;

	vidProcCfg |= pvidInfo->ovSurface.pixFmt;

	if (pvidInfo->ovSurface.clutBypass)
		vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;

	if (pvidInfo->ovSurface.clutSelect)
		vidProcCfg |= SST_OVERLAY_CLUT_SELECT;

	if (pvidInfo->ovSurface.horizScaling)
	{
		vidProcCfg |= SST_OVERLAY_HORIZ_SCALE_EN;
		RegisterMap->vidOverlayDudx = pvidInfo->ovSurface.dudx;
	}

	//
	// XXX: assumes that stride is in bytes, otherwise this breaks
	// if stride is ever specified in tiles
	//
	RegisterMap->vidOverlayDudxOffsetSrcWidth =
	   (0 | (pvidInfo->ovSurface.stride << SST_OVERLAY_FETCH_SIZE_SHIFT));

	if (pvidInfo->ovSurface.verticalScaling)
	{
		vidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;
		RegisterMap->vidOverlayDvdy = pvidInfo->ovSurface.dvdy;
		RegisterMap->vidOverlayDvdyOffset = 0;
	}

	//RegisterMap3D->leftOverlayBuf = 0x10400;	// Really the driver should update this.
	//RegisterMap3D->swapbufferCMD = 0x01;

	RegisterMap->vidProcCfg = vidProcCfg;

	//
	// change only the overlay portion of the vidDesktopOverlayStride register
	//
	doStride = RegisterMap->vidDesktopOverlayStride;
	doStride &= ~(SST_OVERLAY_LINEAR_STRIDE | SST_OVERLAY_TILE_STRIDE);
	pvidInfo->ovSurface.stride <<= SST_OVERLAY_STRIDE_SHIFT;
	if (pvidInfo->ovSurface.tiled)
		pvidInfo->ovSurface.stride &= SST_OVERLAY_TILE_STRIDE;
	else
		pvidInfo->ovSurface.stride &= SST_OVERLAY_LINEAR_STRIDE;
	doStride |= pvidInfo->ovSurface.stride;

	RegisterMap->vidDesktopOverlayStride = doStride;

}

VP_STATUS
H3InitSetMode(PHW_DEVICE_EXTENSION  HwDeviceExtension,
              PH3_MEMBASE0          RegisterMap,
              PUCHAR                pIO,
              PH3_VIDEO_FREQUENCIES FrequencyEntry,
              PVID_PROC_CONFIG      pvidInfo)
{
	PUCHAR reg = FrequencyEntry->VideoData;
	ULONG status = ERROR_INVALID_PARAMETER;
	ULONG temp, i, j;
#ifdef TVOUT_SUPPORTED
	ULONG dacMode;
#endif
#if defined ( GTF_TIMINGS ) || ( DMT_ENABLED )
	UCHAR tempVideoData[CRTC_TABLE_SIZE];
#endif

	VideoDebugPrint((2, "H3InitSetMode - \n"));
	VideoDebugPrint((2,"            %04d x %04d x %02d @ %03d\n",
		FrequencyEntry->ScreenWidth,
		FrequencyEntry->ScreenHeight,
		FrequencyEntry->BitsPerPel,
		FrequencyEntry->ScreenFrequency
		));

// fix for PRS 14785
// don't recompute crtc settings here
#if 0
// what the hell are we doing this for?
// we already computed the friggin' crtc table at FindAdapter time!
// recomputing the crtc settings here screws up all low res modes on the Mitsubishi Diamond Pro 1000
#ifdef GTF_TIMINGS
	if ( HwDeviceExtension->MonitorIsGTF )
	{
	TIMING_PARAMS GTF_Params;	 


		GTF_Params.width = FrequencyEntry->ScreenWidth;
		GTF_Params.height = FrequencyEntry->ScreenHeight;
		GTF_Params.refresh = FrequencyEntry->ScreenFrequency;
		GTF_Params.CharWidth = DEFAULT_CHAR_WIDTH;
		GTF_Params.CRTCflags = DEFAULT_CRTC_FLAGS;
		di_GetGTF_Timing( &GTF_Params);
		ds_Calc_CRTC_table( HwDeviceExtension, &GTF_Params, FrequencyEntry->ScreenWidth, FrequencyEntry->ScreenHeight, tempVideoData);
        reg = tempVideoData;
	}
#endif // GTF_TIMINGS
#endif

#ifdef DMT_ENABLED
	// if DFP is active use calculated timings in several cases.
	if ( isPanelActive( HwDeviceExtension ) &&
        (FrequencyEntry->ScreenFrequency == 60))
    {
        if (isPanelScaling( HwDeviceExtension ))
        {
            //  if setting flat panel to it's native resolution then calculate timing params from
            //  EDID values.
            if (( HwDeviceExtension->centeredDfpCurrentTimings.width == FrequencyEntry->ScreenWidth) &&
                    ( HwDeviceExtension->centeredDfpCurrentTimings.height == FrequencyEntry->ScreenHeight))
            {
                // set timings to native resolution using values loaded from EDID.
                ds_Calc_CRTC_table( HwDeviceExtension,
                        &HwDeviceExtension->centeredDfpCurrentTimings,
                        FrequencyEntry->ScreenWidth, 
                        FrequencyEntry->ScreenHeight,
                        tempVideoData);
                reg = tempVideoData;
            }
            // else for other resolutions just use the timings passed in.
        }
        else
        {
            // if setting flat panel to a resolution it can handle.
            if ( HwDeviceExtension->centeredDfpCurrentTimings.width >= FrequencyEntry->ScreenWidth)
            {
                // set timings to force centering, if a "non-scaling" DFP is active.
                ds_Calc_CRTC_table( HwDeviceExtension,
                        &HwDeviceExtension->centeredDfpCurrentTimings,
                        FrequencyEntry->ScreenWidth, 
                        FrequencyEntry->ScreenHeight,
                        tempVideoData);
                reg = tempVideoData;
            }
        }
    }
#endif

	//
	// Scan the video frequency settings for correctness,
	// Invalid settings are all zero.
	//
	for (i = 0, j = 0; i < 21; i++)
		j += reg[i];

	if (j == 0)
	{
		VideoDebugPrint((0, "H3InitSetMode - invalid frequency settings\n"));
#if DBG
		DbgBreakPoint();
#endif
		return ERROR_INVALID_PARAMETER;
	}

	//
	// unlock the CRTC, and then program it
	//
	VideoPortWritePortUchar((PUCHAR)(pIO + 0x0c2), (UCHAR)(reg[16] | BIT(0)));

	// mystical VGA magic
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)0x0011);
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[0] << 8) | 0x00));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[1] << 8) | 0x01));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[2] << 8) | 0x02));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[3] << 8) | 0x03));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[4] << 8) | 0x04));

	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[5] << 8) | 0x05));

	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[6] << 8) | 0x06));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[7] << 8) | 0x07));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[8] << 8) | 0x09));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[9] << 8) | 0x10));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[10] << 8) | 0x11));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[11] << 8) | 0x12));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[12] << 8) | 0x15));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[13] << 8) | 0x16));

	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[14] << 8) | 0x1a));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)((reg[15] << 8) | 0x1b));

	// Enable sync outputs
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT)0x8017);

	// set the video clock to the proper frequency
	RegisterMap->pllCtrl0 = ((reg[19] << 8) | reg[18]);

	// set up 1x or 2x mode for the DAC
	RegisterMap->dacMode = (ULONG)reg[20];

	//
	// the 1x / 2x bit must also be set in vidProcConfig to properly
	// enable 1x / 2x mode
	//
	temp = RegisterMap->vidProcCfg;
	temp &= ~(SST_VIDEO_2X_MODE_EN | SST_HALF_MODE);
	if (reg[20])
		temp |= SST_VIDEO_2X_MODE_EN;
	if (pvidInfo->dtSurface.scanlinedouble)
	   temp |= SST_HALF_MODE;
	RegisterMap->vidProcCfg = temp;

	// Sequencer registers (set run mode, not reset)
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xc4), (USHORT)((reg[17] & (~BIT(5))) << 8 | 0x01));
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xc4), (USHORT)((0x03 << 8) | 0x00));

	// make sure the attribute register is initialized
	temp = VideoPortReadPortUchar((PUCHAR)(pIO + 0x0da));

	// set attribute registers
	for (i = 0; i < 20; i++)
	{
		VideoPortWritePortUchar((PUCHAR)(pIO + 0xc0), (UCHAR) i);
		VideoPortWritePortUchar((PUCHAR)(pIO + 0xc0), vgaAttributes[i]);
	}

	VideoPortWritePortUchar((PUCHAR)(pIO + 0xc0), (UCHAR)0x34);
	VideoPortWritePortUchar((PUCHAR)(pIO + 0xda), (UCHAR)0x00);

	//
	// Initialize VIDEO res & proc mode info...
	//
	if (pvidInfo->dtSurface.scanlinedouble)
		RegisterMap->vidScreenSize = ((FrequencyEntry->ScreenHeight << 13)
								   |  (FrequencyEntry->ScreenWidth & 0xfff));
	else
  {
    ULONG screenWidth = FrequencyEntry->ScreenWidth;
    if ((IS_VOODOO3) && (2048 == screenWidth))
      screenWidth = 2046;
		RegisterMap->vidScreenSize = ((FrequencyEntry->ScreenHeight << 12)
								   |  (screenWidth & 0xfff));
  }

	//
	// set vidOverlayStartCoords
	//
	RegisterMap->vidOverlayStartCoords = 0;

	//
	// set vidOverlayEndScreenCoords
	//
	RegisterMap->vidOverlayEndScreenCoords = (((FrequencyEntry->ScreenHeight - 1) << 12)
										   |  ((FrequencyEntry->ScreenWidth  - 1) & 0xfff));
	//
	// write the CLUT with the ramp
	//
	if (pvidInfo->ovSurface.clutBypass == 0)
	{
		for (i = 0; i < 0x100; i++)
		{
			VideoPortWritePortUlong((PULONG)(pIO + DACADDR), i);
			temp = VideoPortReadPortUlong((PULONG)(pIO + DACADDR));

			j = i & 0xff;
			temp = ((j << 16) | (j << 8) | j);

			VideoPortWritePortUlong((PULONG)(pIO + DACDATA), temp);
			temp = VideoPortReadPortUlong((PULONG)(pIO + DACADDR));
		}
	}

	H3UpdateGamma(HwDeviceExtension, RegisterMap, pIO, HwDeviceExtension->GammaTable);

	//
	// turn off VGA's screen refresh, as this function only sets extended
	// video modes, and the VGA screen refresh eats up performance
	// (10% difference in screen to screen blits!).   This code is not in
	// the perl, but should stay here unless specifically decided otherwise
	//
	temp = RegisterMap->vgaInit0;
	temp |= BIT(12);
	RegisterMap->vgaInit0 = temp;

	// again, per andy
	//
	// clear bit 10 and 11 in the vidProcCfg register so that the CLUT
	// is not bypassed
	//
	temp = RegisterMap->vidProcCfg;
	VideoDebugPrint((2, "vidProcCfg(before) = 0x%08lx\n", temp));

	temp &= ~SST_BYPASS_CLUT;

	//
	// Check dacMode bit 0 to see if we are in a clock doubled mode and
	// we need to set the 2 pixels per clock option in bit 26 of vidProcCfg
	//
	if (reg[20] & 0x01)
		temp |= SST_VIDEO_2X_MODE_EN;
	else
		temp &= ~SST_VIDEO_2X_MODE_EN;

	//
	// If we're a "regular" desktop mode, clear the half mode bit...
	//
	if ((FrequencyEntry->ScreenHeight < 400) && (FrequencyEntry->BitsPerPel <= 16))
	{
		temp |= SST_HALF_MODE;

		RegisterMap->vidScreenSize = ((FrequencyEntry->ScreenHeight << 13)
								   |  ((FrequencyEntry->ScreenWidth) & 0xfff));
	}
	else if (FrequencyEntry->ScreenHeight >= 400)
		temp &= ~SST_HALF_MODE;

	//
	// Now set bit 0 of vidProcCfg to 1 to enable the new mode and timing settings
	//
	temp |= SST_VIDEO_PROCESSOR_EN | SST_CURSOR_MICROSOFT;

	if (HwDeviceExtension->ActiveFrequencyEntry->ScreenHeight >= 400)
		temp |= SST_DESKTOP_EN;

  // Set Bit 28 and 29 on Napalm boards
  // This fixes a problem we were seeing with high-res modes in a heated environment.
  if ((IS_NAPALM) /*&& (66 == HwDeviceExtension->PciSpeed)*/)
    temp |= (BIT(28) | BIT(29));
  else
    temp &= ~(BIT(28) | BIT(29));

	// set vidProcCfg with the modified values
	RegisterMap->vidProcCfg = temp;

	temp = RegisterMap->vidProcCfg;

#ifdef TVOUT_SUPPORTED
        //  if TV Out is active
        if (HwDeviceExtension->tvOutActive && HwDeviceExtension->tvOutCapable)
	{
            BT868_Enable(HwDeviceExtension, -1);
	}

        // turn monitor back off if it is not supposed to be active.
        if (!HwDeviceExtension->monitorActive)
	{
	    dacMode = VideoPortReadPortUlong((PULONG)(pIO + DACMODE));
	    dacMode |= SST_DACMODE_DPMS_SUSPEND;
            VideoPortWritePortUlong((PULONG)(pIO + DACMODE), dacMode);
	}
#endif  //def TVOUT_SUPPORTED

	return NO_ERROR;
}

VP_STATUS
H3SetModeOverlayDouble(PHW_DEVICE_EXTENSION   HwDeviceExtension,
                       PH3_MEMBASE0           RegisterMap,
                       PUCHAR                 pIO,
                       PH3_VIDEO_FREQUENCIES  FrequencyEntry,
                       PVID_PROC_CONFIG       pvidInfo)
{
  VP_STATUS status = ERROR_INVALID_PARAMETER;

  pvidInfo->changeVideoMode = TRUE;
  pvidInfo->changeDesktop = TRUE;
  pvidInfo->changeOverlay = TRUE;

  pvidInfo->dtSurface.enable = 0;
  pvidInfo->dtSurface.tiled = 0;
  pvidInfo->dtSurface.pixFmt = 0;
  pvidInfo->dtSurface.clutBypass = 0;
  pvidInfo->dtSurface.clutSelect = 0;
  pvidInfo->dtSurface.stride = 0;
  pvidInfo->dtSurface.scanlinedouble = 0;

  pvidInfo->ovSurface.enable = TRUE;
  pvidInfo->ovSurface.stereo = 0; 		// no stereo right now...
  pvidInfo->ovSurface.horizScaling = 1;	// no scaling right now...
  pvidInfo->ovSurface.dudx = STRETCH_BY_TWO;
  pvidInfo->ovSurface.verticalScaling = 1;	// no scaling right now...
  pvidInfo->ovSurface.dvdy = STRETCH_BY_TWO;
  pvidInfo->ovSurface.filterMode = 0;
  pvidInfo->ovSurface.tiled = 0;			// no tiled modes right now...
  pvidInfo->ovSurface.pixFmt = SST_OVERLAY_PIXEL_RGB565D;
  pvidInfo->ovSurface.clutBypass = 0; 	// Bypass clut for OS if required
  pvidInfo->ovSurface.clutSelect = 0; 		// use the lower clut
  pvidInfo->ovSurface.stride =
  	FrequencyEntry->ModeEntry->ModeInformation.ScreenStride;

  pvidInfo->width  = FrequencyEntry->ScreenWidth * 2;
  pvidInfo->height = FrequencyEntry->ScreenHeight * 2;

  status = H3InitSetMode(HwDeviceExtension,
                         RegisterMap,
                         pIO,
                         FrequencyEntry,
                         pvidInfo);

  return status;
}



VP_STATUS
H3SetMode(PHW_DEVICE_EXTENSION  HwDeviceExtension,
          PH3_MEMBASE0          RegisterMap,
          PUCHAR                pIO)
/*++

Routine Description:

	Takes the mode set information and programs the proper registers in the Banshee

Arguments:

	HwDeviceExtension - Pointer to the miniport driver's device extension.

Return Value:

	NO_ERROR, or ERROR_INVALID_PARAMETER

--*/
{
	PH3_VIDEO_FREQUENCIES FrequencyEntry = HwDeviceExtension->ActiveFrequencyEntry;
	VP_STATUS status = ERROR_INVALID_PARAMETER;
	// defaults for mode and desktop stuff
	VID_PROC_CONFIG vidInfo;
	ULONG scratch;
	ULONG scanlinedoubling = FALSE;

	UnlockVgaTimingRegisters(HwDeviceExtension, RegisterMap);

	if (FrequencyEntry->ModeEntry->ModeInformation.DriverSpecificAttributeFlags & CAPS_SCAN_LINE_DOUBLED)
	{
		//
		// Double the pixel size, but do not use the overlay surface
		//
		scanlinedoubling = TRUE;
	}

	//
	// for now, always enable this mode as the desktop, and disable the
	// overlay
	//
	vidInfo.changeDesktop = TRUE;
	vidInfo.dtSurface.enable = TRUE;
	vidInfo.dtSurface.tiled = 0;
	vidInfo.dtSurface.pixFmt = bppToPixfmt(FrequencyEntry->BitsPerPel);
	vidInfo.dtSurface.clutBypass = 0;
	if (FrequencyEntry->BitsPerPel == 8)
		vidInfo.dtSurface.clutSelect = 0;
	else
		vidInfo.dtSurface.clutSelect = 1;
	vidInfo.dtSurface.stride =
		FrequencyEntry->ModeEntry->ModeInformation.ScreenStride;
	vidInfo.dtSurface.scanlinedouble = scanlinedoubling;

	//
	// convert bytes to tiles by dividing by 128, even though we don't support
	// tiled modes yet???
	//
	if (vidInfo.dtSurface.tiled)
		vidInfo.dtSurface.stride >>= 7;

	//
	// right now, disable the overlay surface
	//
	vidInfo.changeOverlay = TRUE;
	vidInfo.ovSurface.enable = 0;
	vidInfo.ovSurface.stereo = 0;
	vidInfo.ovSurface.horizScaling = 0;
	vidInfo.ovSurface.dudx = 0;
	vidInfo.ovSurface.verticalScaling = 0;
	vidInfo.ovSurface.dvdy = 0;
	vidInfo.ovSurface.filterMode = 0;
	vidInfo.ovSurface.tiled = 0;
	vidInfo.ovSurface.pixFmt = 0;
	vidInfo.ovSurface.clutBypass = 0;
	vidInfo.ovSurface.clutSelect = 0;
	vidInfo.ovSurface.stride = 0;

	vidInfo.width  = FrequencyEntry->ScreenWidth;
	vidInfo.height = FrequencyEntry->ScreenHeight;

  H3InitDesktopSurface(HwDeviceExtension,
                       RegisterMap,
                       pIO,
                       FrequencyEntry,
                       &vidInfo);

  H3InitOverlaySurface(HwDeviceExtension,
                       RegisterMap,
                       pIO,
                       FrequencyEntry,
                       &vidInfo);

  status = H3InitSetMode(HwDeviceExtension,
                         RegisterMap,
                         pIO,
                         FrequencyEntry,
                         &vidInfo);

	//
	// If there's been a BIOS call to a DOS mode, we may have to restore
	// a bit in dramInit1 (bit 0) if bit 0 in vidProcCfg if 0 (for refresh enable)
	//
	scratch = RegisterMap->vidProcCfg;
	if ((scratch & BIT(0)) == 0)
		scratch = RegisterMap->dramInit1 & (~BIT(0));
	else
		scratch = RegisterMap->dramInit1 | BIT(0);

	RegisterMap->dramInit1 = scratch;

	return status;
}


// The following routine is called when a switch to an alternate display device (DFP) might require that
// the CRTC timings be changed.
void H3SetModeResetTiming(PHW_DEVICE_EXTENSION  HwDeviceExtension)
{
    VID_PROC_CONFIG vidInfo;

    vidInfo.changeVideoMode = TRUE;
    vidInfo.changeDesktop = FALSE;
    vidInfo.changeOverlay = FALSE;

    vidInfo.dtSurface.scanlinedouble = FALSE;
    if (HwDeviceExtension->ActiveFrequencyEntry->ModeEntry->ModeInformation.DriverSpecificAttributeFlags & CAPS_SCAN_LINE_DOUBLED)
    {
        vidInfo.dtSurface.scanlinedouble = TRUE;
    }
    vidInfo.ovSurface.clutBypass = 0;

    vidInfo.width  = HwDeviceExtension->ActiveFrequencyEntry->ScreenWidth;
    vidInfo.height = HwDeviceExtension->ActiveFrequencyEntry->ScreenHeight;


    if (H3InitSetMode(HwDeviceExtension,
            (PH3_MEMBASE0)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX],
            (PUCHAR)HwDeviceExtension->MappedAddress[SST_IO_INDEX],
            HwDeviceExtension->ActiveFrequencyEntry,
            &vidInfo))

    {
        VideoDebugPrint((0, "Couldn't set mode!\n"));
    }

}

//=======================================================================================
// DYNAMIC MODE TABLE begin
//=======================================================================================

#if defined ( GTF_TIMINGS ) || ( DMT_ENABLED )

ULONG _fltused;		// Added so we might be able to link the simpler floating point operatons
	
#define FPU_FUNCTION_SAVE		0
#define FPU_FUNCTION_RESTORE	1
#define FPU_STATE_SAVE_SIZE		108

//----------------------------------------------------------------------
// Function name:  FPU_State
//
// Description:    Save/Restore the state of the Floating Processor.
// 
// Information:
// 
// Return:         VOID
// ----------------------------------------------------------------------
void FPU_State( int function )
{
static unsigned char SaveArea[FPU_STATE_SAVE_SIZE];


	switch ( function )
	{
		case FPU_FUNCTION_SAVE:
			_asm { FSAVE SaveArea };
			break;
		case FPU_FUNCTION_RESTORE:
			_asm { FRSTOR SaveArea };
			break;
	}
}

//----------------------------------------------------------------------
// Function name:  Round
//
// Description:    round a double to the nearest integer.
//
// Information:
//
// Return:         double  representing nearest whole number
//----------------------------------------------------------------------
double Round( double x )
{
double wholepart;
double fraction;
int wholeint;


	_asm { FLD x };					// Read in the number to be truncated
	_asm { FISTP wholeint };		// Write it to a memory location

	wholepart = (double)wholeint;
	fraction = ( x - wholepart );

	if ( x >= 0.0 )					// Perform rounding for positive values
	{
		if ( fraction >= 0.5 )
			return ( wholepart + 1.0 );
		else
			return ( wholepart );
	}
	else						   	// Perform rounding for negative values
	{
		if ( fraction <= -0.5 )
			return ( wholepart - 1.0 );
		else
			return ( wholepart );
	}
}

//----------------------------------------------------------------------
// Function name:  FloatToInt
//
// Description:    truncate a double to an integer.
//
// Information:
//
// Return:         int representing whole number part of a double
//----------------------------------------------------------------------
int FloatToInt( double x )
{
int wholepart;


	_asm { FLD x };					// Read in the number to be truncated
	_asm { FISTP wholepart };		// Write it to a memory location

	return ( wholepart );
}

//              r[0]  r[1]  r[2]  r[3]  r[4]  r[5]  r[6]  r[7]  r[8]  r[9]  r[10] r[11] r[12] r[13] r[14] r[15] r[16] r[17] r[18] r[19] r[20]
//   x  y  rr,    0     1     2     3     4     5     6     7     9    10     11    12    15    16    1a    1b    c2   SR1  pllctrl0  dacmode
//   x  y  rr, Htotl HDEnE HBlSt HBlEn HSySt HSyEn Vtotl Ovflw MxSLn VSySt  VSyEn VDEnE VBlSt VBlEn HExtn VExtn MiscO  SR1  pllctrl0  dacmode 
//
//----------------------------------------------------------------------
// Function name:  ds_Calc_CRTC_table
//
// Description:    Translate the VBE 3 based timing parameters into CRTC timing values.
//
// Information:
//
// Return:         void
//----------------------------------------------------------------------
VOID ds_Calc_CRTC_table( PHW_DEVICE_EXTENSION HwDeviceExtension, TIMING_PARAMS *pVprm, FxU32 actualWidth, FxU32 actualHeight, PUCHAR crtc_table )
{
UCHAR byHTotal, byHorDispEnEnd, byHBlankStart;
UCHAR byHBlankEnd, byHSyncStart, byHSyncEnd;
UCHAR byVTotal, byOverflow, byMaxLineScan;
UCHAR byVSyncStart, byVSyncEnd, byVertDispEnEnd;
UCHAR byVBlankStart, byVBlankEnd;
UCHAR byHExtensions, byVExtensions;
UCHAR byMiscOutput, byDacMode;
UCHAR byScanLineDoubled;
UCHAR byHSyncPolarity;
UCHAR byVSyncPolarity;
UCHAR byCRTCflags;
UCHAR bySeqDotClk;
USHORT wHVisible;
USHORT wHTotal;
USHORT wHBlankStart;
USHORT wHBlankTime;
USHORT wHSyncStart;
USHORT wHSyncTime;
USHORT wVVisible;
USHORT wVTotal;
USHORT wVBlankStart;
USHORT wVBlankTime;
USHORT wVSyncStart;
USHORT wVSyncTime;
ULONG  bClockDouble;
double dbPixelClock;
// variables used for finding the best pll #s
int m, k, bestm, bestn, bestk;
double test, rndtest, newoverflow, oldoverflow;
USHORT wPllCtrl0;
#ifdef DEBUG
float answer;
USHORT wPossiblePllCtrl0values[256];
USHORT wPossiblePllCount = 0;
int iGoodTimings = 0;
#endif

	FPU_State( FPU_FUNCTION_SAVE );

	byCRTCflags = (UCHAR)pVprm->CRTCflags;
	byScanLineDoubled = ( byCRTCflags & 1 );
	byHSyncPolarity = ( ( byCRTCflags << 4 ) & 0x40 );
	byVSyncPolarity = ( ( byCRTCflags << 4 ) & 0x80 );

	wHVisible = (USHORT)( pVprm->width / pVprm->CharWidth ); // HVisible same as Hor Addr Time
	wHBlankStart = wHVisible; 	// We can assume this because there are no borders
	wHTotal = (USHORT)( pVprm->HTotal / pVprm->CharWidth ); 
	wHBlankTime = ( wHTotal - wHVisible );
	wHSyncStart	= (USHORT)( pVprm->HSyncStart / pVprm->CharWidth );
	wHSyncTime	= (USHORT)( ( pVprm->HSyncEnd - pVprm->HSyncStart ) / pVprm->CharWidth );

	wVVisible = (USHORT)pVprm->height;	 // VVisible same as Ver Addr Time
	if ( byScanLineDoubled )
		wVVisible *= 2;
	wVBlankStart = wVVisible;	// We can assume this because there are no borders
	wVTotal = (USHORT)pVprm->VTotal;
	wVBlankTime = ( wVTotal - wVBlankStart );
	wVSyncStart	= (USHORT)pVprm->VSyncStart;
	wVSyncTime  = (USHORT)( pVprm->VSyncEnd - pVprm->VSyncStart );

	dbPixelClock = (double)pVprm->PixelClock;   
	dbPixelClock = ( dbPixelClock / 1000000 );	// Convert PixelClock from Hz to double

    if(actualWidth != pVprm->width)  //Test for a dfp centered mode
    {
        FxU16 H2, V2;

        H2           = (FxU16)(((pVprm->width - actualWidth) / 2) / pVprm->CharWidth);
        wHVisible    = (FxU16)( actualWidth / pVprm->CharWidth );
        wHBlankStart -= H2;
        wHSyncStart  -= H2;

        V2           = (FxU16)((pVprm->height - actualHeight) / 2);
        wVVisible    = (FxU16)actualHeight;
        wVBlankStart -= V2;
        wVSyncStart  -= V2;
    }

	// Dacmode Bit0 set to 0 for 1:1 mode, 1 for 2:1 mode
	byDacMode = 0;

	// Test for DoubleDACRate
  bClockDouble = FALSE;
  if (IS_NAPALM)
  {
    // The last or clause is needed since HBlank End is only 6 bits and it would need to be 9
    // this cause problem when switch between DOS and Hi-Rez
    if(((dbPixelClock > 262.0) && ( pVprm->width >= 1280)) || (wHTotal > 261))
      bClockDouble = TRUE;
  }
  else
  {
    if((dbPixelClock > 160.0) && ( pVprm->width >= 1280))
      bClockDouble = TRUE;
  }

  if (bClockDouble)
	{
		byDacMode = 1;
		wHVisible >>= 1;		
		wHBlankTime	= (wHBlankTime & 0x1) + (wHBlankTime>>1);
		wHSyncStart	= (wHSyncStart & 0x1) + (wHSyncStart>>1);
		wHSyncTime	= (wHSyncTime & 0x1) + (wHSyncTime>>1);
		wHTotal		= wHVisible + wHBlankTime;
    wHBlankStart = (wHBlankStart & 0x1) + (wHBlankStart>>1);
	}

	// Lower 8 bits of Horizontal Total
	byHTotal = 0xff & (wHTotal - 5);

	// Lower 8 bits of the horizontal display enable end
	byHorDispEnEnd = 0xff & (wHVisible - 1);

	// Lower 8 bits of the horizontal Blanking start
	byHBlankStart = 0xff & (wHBlankStart-1);

	// Finishing the Horizontal Blank End time
	// Assuming DisplayEnableSkew is 0 and Compatibility Read is on
  byHBlankEnd = 0x1f&(wHBlankTime + ((wHBlankStart-1) &0x3f)) | 0x80;
  byHSyncEnd = (0x20&(wHBlankTime + ((wHBlankStart-1)&0x3f)))<<2;

	// Filling in the lower 8 bits of the Horizontal Sync start value
	byHSyncStart = 0xff & (wHSyncStart-1);

	// Finishing the Horzontal Sync End time
	// Assuming HorizontalSyncSkew is 0
	byHSyncEnd |= (0x1f&(wHSyncTime + wHSyncStart-1));

	// Getting the lower 8 bits of the total # of vertical lines
	byVTotal = 0xff & (wVTotal - 2);

	// Filling in the Overflow register
	// Assuming the LineComp bit 8 is 1
	byOverflow = ((0x100 & (wVTotal-2))>>8) | ((0x100 & (wVVisible-1))>>7) |
		((0x100 & (wVSyncStart-1))>>6) | ((0x100 & (wVBlankStart-1))>>5) |
		0x10 | ((0x200 & (wVTotal-2))>>4) | ((0x200 & (wVVisible-1))>>3) |
		((0x200 & (wVSyncStart-1))>>2);

	// Filling in the MaxScanLine register
	// Assuming the LineComp bit 9 is 1
	byMaxLineScan = ((0x200 & (wVBlankStart-1))>>4) | 0x40 | 
		(byScanLineDoubled ? 0x80 : 0x0);

	// Filling in the lower 8 bits of Vertical Sync Start
	byVSyncStart = 0xff & (wVSyncStart-1);

	// Filling in the Vertical Sync End time
	// Assuming that we are enabling the Vertical Interrupt, allowing access to CR0-7,
	// and not clearing the interrupt
	byVSyncEnd = (0x0f & (wVSyncTime + wVSyncStart-1)) | 0x20;

	// Filling in the in lower 8 bits of the visible vertical lines
	byVertDispEnEnd = 0xff & (wVVisible-1);

	// Filling in the lower 8 bits of Vertical Blank Start
	byVBlankStart = 0xff & (wVBlankStart - 1);

	// Filling in the Vertical Blank End time
	byVBlankEnd = 0xff & (wVBlankTime + wVBlankStart - 1);

	// Filling in the Horizontal Extensions
	byHExtensions = ((0x100 & (wHTotal-5))>>8) |
		((0x100 & (wHVisible-1))>>6) | ((0x100 & (wHBlankStart-1))>>4) |
	  ((0x40&(wHBlankTime + ((wHBlankStart-1)&0x3f)))>>1) |
		((0x100 & (wHSyncStart-1))>>2) |
		((0x20&(wHSyncTime + wHSyncStart-1))<<2);

	// Filling in the Vertical Extensions
	byVExtensions = ((0x400 & (wVTotal-2))>>10) |
		((0x400 & (wVVisible-1))>>8) | ((0x400 & (wVBlankStart-1))>>6) | 
		((0x400 & (wVSyncStart-1))>>4);

	// Filling in Miscellaneous Output register
	// Asuming Clock Select is dictated by the Programmable PLL,
	// RAM is enabled, and using color mode CRTC addressing
	byMiscOutput = 0xf | byHSyncPolarity | byVSyncPolarity;

	if ( pVprm->CharWidth == 9 )
		bySeqDotClk = 0x20;
	else
		bySeqDotClk = 0x21;

	oldoverflow = 1000.0;  // big number
	bestn = bestm = bestk = 0;

	if(dbPixelClock > 150.0)
		k=1;
	else if (dbPixelClock > 65.0)
		k=2;
	else
		k=3;

	// Find the correct pllTable value for the pixel clock
	for(m=1 ; m<64; m++)
	{
		// m should not start at 0 (Found empirically that m=0 will cause the
		// equation to be 14.31818*(n+2)/(1*2^k) So m adds 1 but not 2 here.)
		// As per Yancy's email on Feb. 8, 1999, use only m values >=10
		if ( (dbPixelClock > 36.0) && (m>10))
			break;
		if ( (dbPixelClock > 200.0) && (m>5))
			break;

		test = (dbPixelClock) * ((double) m + 2.0) * 
			((double) (8>>(3-k))) / 14.31818;

		if(test>257.0)
			continue;

		rndtest = Round(test);

		newoverflow = dbPixelClock - 14.31818*rndtest/(((double) m + 2.0) * ((double) (8>>(3-k))));
		if(newoverflow < 0.0)
			newoverflow *= -1.0;

#ifdef DEBUG
		if((newoverflow / dbPixelClock < 0.005) && (wPossiblePllCount < 256))
		{
			wPossiblePllCtrl0values[wPossiblePllCount++] = ( (FloatToInt(rndtest-2.0)) <<8) | (m<<2) | k;
		}

		if(newoverflow == oldoverflow)
			iGoodTimings++;
#endif
			
		// The first values found will give better results than the older values
		if(newoverflow < oldoverflow)
		{
#ifdef DEBUG
			if(newoverflow != oldoverflow)
				iGoodTimings = 0;
#endif
			bestm = m;
			bestn = FloatToInt(rndtest - 2.0);
			bestk = k;
			oldoverflow = newoverflow;
		}
	}


#ifdef DEBUG
	answer = (14.31818f * ( ((float) bestn) + 2.0f)) / 
		( (((float) bestm) + 2.0f) * ((float) (8>>(3-bestk))) );
#endif

	wPllCtrl0 = (bestn<<8) | (bestm<<2) | bestk;

//  Define this if you want to see mode/pll information
//	Deubg_Printf("\n%dx%d @ %d Hz\n", pVpc->width, pVpc->height, pVpc->refresh);
//	Deubg_Printf("0x%04x, n=%d, m=%d, k=%d, 10*VCO=%d\n", wPllCtrl0, bestn, bestm, bestk, ((1431818*(bestn+2))/(bestm+2))/10000 );
	
	crtc_table[0]  = byHTotal;
	crtc_table[1]  = byHorDispEnEnd;
	crtc_table[2]  = byHBlankStart;
	crtc_table[3]  = byHBlankEnd;
	crtc_table[4]  = byHSyncStart;
	crtc_table[5]  = byHSyncEnd;
	crtc_table[6]  = byVTotal;
	crtc_table[7]  = byOverflow;
	crtc_table[8]  = byMaxLineScan;
	crtc_table[9]  = byVSyncStart;
	crtc_table[10] = byVSyncEnd;
	crtc_table[11] = byVertDispEnEnd;
	crtc_table[12] = byVBlankStart;
	crtc_table[13] = byVBlankEnd;
	crtc_table[14] = byHExtensions;
	crtc_table[15] = byVExtensions;
	crtc_table[16] = byMiscOutput;
	crtc_table[17] = bySeqDotClk;	// SR1
	crtc_table[18] = wPllCtrl0 & 0xff;
	crtc_table[19] = wPllCtrl0 >> 8;
	crtc_table[20] = byDacMode;

#if DBG
#define CRTC_DUMP_LEVEL   2
{
  H3_VIDEO_FREQUENCIES  *pH3VidFreq;

  VideoDebugPrint((CRTC_DUMP_LEVEL, "\ncomputed CRTC table for %ldx%ld @ %ldHz\n", pVprm->width, pVprm->height, pVprm->refresh));
  VideoDebugPrint((CRTC_DUMP_LEVEL, "  0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X,\n",
                   crtc_table[0], crtc_table[1], crtc_table[2], crtc_table[3], crtc_table[4],
                   crtc_table[5], crtc_table[6], crtc_table[7], crtc_table[8], crtc_table[9]));
  VideoDebugPrint((CRTC_DUMP_LEVEL, "  0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
                   crtc_table[10], crtc_table[11], crtc_table[12], crtc_table[13], crtc_table[14],
                   crtc_table[15], crtc_table[16], crtc_table[17], crtc_table[18], crtc_table[19], crtc_table[20]));

  for (pH3VidFreq = &H4GenericFrequencyTable[0];
       //pH3VidFreq < &H4GenericFrequencyTable[sizeof(H4GenericFrequencyTable)/sizeof(H4GenericFrequencyTable[0])];
       pH3VidFreq < &H4GenericInt10FrequencyTable[0];
       pH3VidFreq++)
  {
    if ((pH3VidFreq->ScreenWidth     == pVprm->width)  &&
        (pH3VidFreq->ScreenHeight    == pVprm->height) &&
        (pH3VidFreq->ScreenFrequency == pVprm->refresh))
    {
      UCHAR *pVidData = pH3VidFreq->VideoData;

      for (m = 0; m < 21; m++)
      {
        if (pVidData[m] != crtc_table[m])
        {
          VideoDebugPrint((CRTC_DUMP_LEVEL, "    crtc_table[%ld]=0x%02lX  differs from  H3_Reference_Freq_%ldx%ldx%ld[%ld]=0x%02lX\n",
                           m, crtc_table[m],
                           pH3VidFreq->ScreenWidth, pH3VidFreq->ScreenHeight, pH3VidFreq->ScreenFrequency, m, pVidData[m]));
        }
      }
      break;
    }
  }
}
#endif

	FPU_State( FPU_FUNCTION_RESTORE );
}

//----------------------------------------------------------------------
// Function name:  di_GetGTF_Timing
//
// Description:    Calculate Generalized Timing values into VBE 3 based timing parameters
//				   based on inputs of width, height, refresh and character width.
// Information:    
//
// Return:         VOID
//----------------------------------------------------------------------
VOID di_GetGTF_Timing( TIMING_PARAMS *pVprm )
{
double H_pixels_rnd;
double V_lines_rnd;
double V_frame_rate;
double V_field_rate;
double V_field_rate_rqd;
double V_field_rate_est;
double cell_gran;
double H_blank;
double H_sync_time;
double H_sync_start;
double H_period;
double H_period_est;
double V_back_porch;
double total_V_lines;
double total_pixels;
double total_active_pixels;
double ideal_duty_cycle;
double pixel_clock;
double vsync_plus_bp;
double min_vsync_plus_bp = 550;
double min_porch_rnd = 1;
double V_sync_rnd = 3;
double interlace = 0;
double sync_width = 0.08;
double M =  300;
double C =  30;


	V_field_rate_rqd = (double)pVprm->refresh;

	cell_gran = (double)pVprm->CharWidth;

	H_pixels_rnd = Round((double)pVprm->width / cell_gran ) * cell_gran;

	V_lines_rnd	= (double)pVprm->height;

	// half the number of vertical lines if interlace is requested
	if ( pVprm->CRTCflags & 2 )
	{
		V_lines_rnd = V_lines_rnd / 2;
		V_field_rate_rqd *= 2;
		interlace = 0.5;
	}

	if ( pVprm->CRTCflags & 1 )	// Double scanned value
	{
		V_lines_rnd = V_lines_rnd * 2;
	}	

	H_period_est = (( 1 / V_field_rate_rqd ) - min_vsync_plus_bp / 1000000 ) / 
					( V_lines_rnd + min_porch_rnd + interlace) * 1000000;

	vsync_plus_bp = Round( min_vsync_plus_bp / H_period_est );

	V_back_porch = vsync_plus_bp - V_sync_rnd;

	total_V_lines = V_lines_rnd + vsync_plus_bp + interlace + min_porch_rnd;

	V_field_rate_est = 1 / H_period_est / total_V_lines * 1000000;

	H_period = H_period_est	/ ( V_field_rate_rqd / V_field_rate_est );

	V_field_rate = 1 / H_period / total_V_lines	* 1000000;

	V_frame_rate = V_field_rate;

	// half the number of vertical lines if interlace is requested
	if ( pVprm->CRTCflags & 2 )
		V_frame_rate /= 2;
	
	total_active_pixels = H_pixels_rnd;
	
	ideal_duty_cycle = C - ( M * H_period / 1000 );

	H_blank = Round(( total_active_pixels * ideal_duty_cycle / (100 - ideal_duty_cycle ) / 
	                ( 2 * cell_gran ))) * ( 2 * cell_gran );

	total_pixels = total_active_pixels + H_blank; 

	pixel_clock = total_pixels / H_period;


	H_sync_time = Round( sync_width * ( total_pixels / cell_gran )) * cell_gran;

	H_sync_start = total_active_pixels + (( H_blank / 2 ) - H_sync_time );

	pVprm->HTotal = FloatToInt( total_pixels );
	pVprm->HSyncStart = FloatToInt( H_sync_start );
	pVprm->HSyncEnd = FloatToInt( H_sync_start + H_sync_time );
	pVprm->VTotal = FloatToInt( total_V_lines );
	pVprm->VSyncStart = FloatToInt( V_lines_rnd + min_porch_rnd );
	pVprm->VSyncEnd = FloatToInt( V_lines_rnd + min_porch_rnd + V_sync_rnd );
	pVprm->PixelClock = FloatToInt( pixel_clock * 1000000 );

}

//----------------------------------------------------------------------
// Function name:  di_IsMonitorGTF
//
// Description:    Parse the monitor EDID block to determine if it claims to be GTF.
//
// Information:    
//
// Return:         BOOLEAN - TRUE  if monitor indicates it is GTF
//							 FALSE if monitor does not indicate GTF
//----------------------------------------------------------------------
BOOLEAN di_IsMonitorGTF( PHW_DEVICE_EXTENSION HwDeviceExtension )
{
UCHAR EdidBlock[EDID_BLOCK_MAX_SIZE];
USHORT Version;
ULONG Descriptor;
int i;


	if ( !GetDdcInformation( HwDeviceExtension, EdidBlock, sizeof( EdidBlock ) ) )
		return FALSE;

	Version = (USHORT)EdidBlock[18];	// Get EDID Version number
	Version <<= 8;
	Version |= (USHORT)EdidBlock[19];  	// Get EDID Revision number

	if ( Version < 0x101 )				// EDID version/rev must be 1.1 or higher to support GTF
		return FALSE;

	if ( !( EdidBlock[24] & BIT(0) ) ) 	// Bit 0 of Feature Support will be set if monitor is GTF
		return FALSE;

	// We must also have a timing range descriptor in order to be truly GTF
	for ( i = 54; i <= 108; i += 18 )
	{
		Descriptor = ((ULONG)EdidBlock[i]) << 24;
		Descriptor |= ((ULONG)EdidBlock[i+1]) << 16;
		Descriptor |= ((ULONG)EdidBlock[i+2]) << 8;
		Descriptor |= ((ULONG)EdidBlock[i+3]);

		if ( Descriptor == 0x000000FD )
			return TRUE;
	}

	return FALSE;
}

#endif // ( GTF_TIMINGS ) || ( DMT_ENABLED )

//=======================================================================================
// DYNAMIC MODE TABLE end
//=======================================================================================

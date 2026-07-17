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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
**
*/
#pragma once

#ifndef __HRM_OVERLAY_SURFACE_H__
#define __HRM_OVERLAY_SURFACE_H__

#include "hrm_priv.h"
#include "hdwr_res_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*hrmSetVideoOverlayPtr)(
	hrmBoard_t	*theBoard,				// the board in question
	UInt32		enable,					// true to turn on overlay, false to turn it off
	UInt32		stereo,					// true for stereo display
	UInt32		leftSrcBaseAddr,		// the on-board base address
	UInt32		rightSrcBaseAddr,		// set this to the on-board base address as well
	UInt32		srcStride,
	UInt32		srcWidth,
	UInt32		srcHeight,
	UInt32		dstWidth,
	UInt32		dstHeight,
	SInt32		dstTop,
	SInt32		dstLeft,
	UInt32		filterMode,				/* one of: 
												SST_OVERLAY_FILTER_POINT
												SST_OVERLAY_FILTER_2X2
												SST_OVERLAY_FILTER_4X4
												SST_OVERLAY_FILTER_BILINEAR 
										*/
	UInt32		tiled,					// set to false for now
	UInt32		pixFmt,					/* one of:
												SST_OVERLAY_PIXEL_RGB565U
												SST_OVERLAY_PIXEL_YUV411
												SST_OVERLAY_PIXEL_YUYV422
												SST_OVERLAY_PIXEL_UYVY422
												SST_OVERLAY_PIXEL_RGB565D	
										*/
	UInt32		clutBypass,				// set to true for now
	UInt32		clutSelect,				// set to 0 for now
	UInt32		colorKey);				// the color key value to use (as a pixel value, not replicated)
	
void hrmSetVideoOverlay(
	hrmBoard_t	*theBoard,				// the board in question
	UInt32		enable,					// true to turn on overlay, false to turn it off
	UInt32		stereo,					// true for stereo display
	UInt32		leftSrcBaseAddr,		// the on-board base address
	UInt32		rightSrcBaseAddr,		// set this to the on-board base address as well
	UInt32		srcStride,
	UInt32		srcWidth,
	UInt32		srcHeight,
	UInt32		dstWidth,
	UInt32		dstHeight,
	SInt32		dstTop,
	SInt32		dstLeft,
	UInt32		filterMode,				/* one of: 
												SST_OVERLAY_FILTER_POINT
												SST_OVERLAY_FILTER_2X2
												SST_OVERLAY_FILTER_4X4
												SST_OVERLAY_FILTER_BILINEAR 
										*/
	UInt32		tiled,					// set to false for now
	UInt32		pixFmt,					/* one of:
												SST_OVERLAY_PIXEL_RGB565U
												SST_OVERLAY_PIXEL_YUV411
												SST_OVERLAY_PIXEL_YUYV422
												SST_OVERLAY_PIXEL_UYVY422
												SST_OVERLAY_PIXEL_RGB565D	
										*/
	UInt32		clutBypass,				// set to true for now
	UInt32		clutSelect,				// set to 0 for now
	UInt32		colorKey);				// the color key value to use (as a pixel value, not replicated)

#ifdef __cplusplus
}
#endif

#endif /* __HRM_OVERLAY_SURFACE_H__ */
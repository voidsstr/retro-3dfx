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
** $Header: HDWR_Res_Mgr.h, 5, 10/11/00 8:38:05 PM, Brent$
** $Log: 
**  5    3dfx      1.2.1.1     10/11/00 Brent           Forced check in to enforce
**       branching.
**  4    3dfx      1.2.1.0     07/11/00 Critical Path   new source drop
**  3    MacOS Dev Tree1.2         02/07/00 Kenneth Dyke    Added SLI/AA support.
**  2    MacOS Dev Tree1.1         02/01/00 Kenneth Dyke    Changed hrmBoardInfo_t
**       to return driver ref num rather than GDHandle.  Fixed CR/LF effage.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 4     8/23/99 2:38p Kcd
** 
** 3     7/08/99 1:27p Kcd
** C++ support.
** 
** 2     7/02/99 3:28p Kcd
** New extension stuff.
**
*/
#pragma once

#ifndef __HARDWARE_RESOURCE_MANAGER_H__
#define __HARDWARE_RESOURCE_MANAGER_H__

// Includes
#ifdef macintosh
# include <Types.h>
# include <Quickdraw.h>
#endif

//#include "DynamicPatches.h"
//#include "hrm_fifo.h"

#ifdef __cplusplus
extern "C" {
#endif

// Types

typedef enum {
  HRM_OVERLAY_SRC_PIXEL_RGB565U = 1000,
  HRM_OVERLAY_SRC_PIXEL_YUV411,
  HRM_OVERLAY_SRC_PIXEL_YUYV422,
  HRM_OVERLAY_SRC_PIXEL_UYVY422,
  HRM_OVERLAY_SRC_PIXEL_RGB565D
} hrmOvlSrcPixFmt_e;

typedef enum {
  HRM_OVERLAY_DST_PIXEL_PAL8 = 2000,
  HRM_OVERLAY_DST_PIXEL_RGB565,
  HRM_OVERLAY_DST_PIXEL_RGB555,
  HRM_OVERLAY_DST_PIXEL_RGB24,
  HRM_OVERLAY_DST_PIXEL_RGB32
} hrmOvlDstPixFmt_e;

typedef enum {
  HRM_OVERLAY_FILTER_POINT = 3000,
  HRM_OVERLAY_FILTER_2X2,
  HRM_OVERLAY_FILTER_4X4,
  HRM_OVERLAY_FILTER_BILINEAR
} hrmOvlFilter_e;

typedef struct hrmMacOverlaySurface_t
{
  short
    inRefNum,
    inEnable,
    inIsStereo,
    inUseColorKey;
  long
    inSrcBaseAddr,
    inSrcRightBaseAddr,
    inSrcStride,
    inSrcWidth,
    inSrcHeight;
  hrmOvlSrcPixFmt_e
    inSrcPixFormat;
  Rect
    inDstRect;
  hrmOvlDstPixFmt_e
    inDstPixFormat;
  hrmOvlFilter_e
    inDstFilter;
  RGBColor
    inColorKey;
} hrmMacOverlaySurface_t;

typedef struct hrmDeviceConfig_s
{
  unsigned long
    lfbBase,
    pciStride,
    hwStride,
    tileMark;
  GDHandle
    device;
} hrmDeviceConfig_t;

typedef struct hrmVersionInfo_s
{
  unsigned char
    major,
    minor;
} hrmVersionInfo_t;

typedef struct hrmBoardInfo_s
{
  unsigned long size;
  signed   long driverRefNum;
  unsigned long h3Mem;
  unsigned long deviceRev;
  unsigned long vendorID;
  unsigned long deviceID;
  unsigned long pciBaseAddr[4];
  unsigned long devNum;
  unsigned long isMaster;
  unsigned long numChips;
  unsigned long swizzleOffset[4];
  unsigned long is66MHz;			// TRUE if in 66MHz PCI slot
} hrmBoardInfo_t;

typedef struct hrmSLIAAChipInfo_s
{
  unsigned long size;
  unsigned long numChips;       // dwChips: Number of chips in multi-chip configuration (1-4)
  unsigned long sliEnable;      // dwsliEn: Sli is to be enabled (0,1)
  unsigned long aaEnable;       // dwaaEn: Anti-aliasing is to be enabled (0,1)
  unsigned long aaSampleHigh;   // dwaaSampleHigh: 0->Enable 2-sample AA, 1->Enable 4-sample AA
  unsigned long sliAaAnalog;    // dwsliAaAnalog: 0->Enable digital SLI/AA, 1->Enable analog Sli/AA
  unsigned long sli_nlines;     // dwsli_nLines: Number of lines owned by each chip in SLI (2-128)
  unsigned long swapAlgorithm;  // Swap Buffer Algorithm
} hrmSLIAAChipInfo_t;

typedef struct hrmSLIAAMemInfo_s
{
  unsigned long size;
  unsigned long totalMemory;       // In MB
  unsigned long tileMark;          // In MB
  unsigned long tileCmpMark;       // In MB
  unsigned long aaSecondaryColorBufBegin;    // In MB
  unsigned long aaSecondaryDepthBufBegin;    // In MB
  unsigned long aaSecondaryDepthBufEnd;    // In MB
  unsigned long bpp;                        // Bits Per Pixel
} hrmSLIAAMemInfo_t;

/* Opaque data types */
typedef struct hrmBoard_s hrmBoard_t;
 
/* Permanent exported functions (for convenience) */
void		*hrmGetExtension(const char *extensionName);
long		hrmGetNumTargets( void );
hrmBoard_t	*hrmGetTargetAtIndex( long index );
void hrmGetTargetBoardInfoExt(hrmBoard_t *board, hrmBoardInfo_t *boardInfo);

typedef void *(*hrmGetExtensionPtr) (const char *extName);
typedef void (*hrmGetVersionInfoPtr)(hrmVersionInfo_t *   outVersion);
typedef short (*hrmGetDeviceConfigPtr)(hrmBoard_t *board, hrmDeviceConfig_t *config);
typedef void (*hrmGetTargetBoardInfoExtPtr)(hrmBoard_t *board, hrmBoardInfo_t *boardInfo);

typedef struct hrmFifoUpdate_s
{
  hrmBoard_t *board;
  void    (*setLfb)(volatile unsigned long *d, unsigned long s);
  void    (*setLfbHost)(volatile unsigned long *d, unsigned long s);
} hrmFifoUpdate_t;

#ifdef __cplusplus
}
#endif

#endif /* __HARDWARE_RESOURCE_MANAGER_H__ */
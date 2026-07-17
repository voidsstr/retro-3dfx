/*
** Copyright (c) 1999, 3dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 16$
** $Date: 10/11/00 8:44:15 PM$
*/
#ifndef __FXIOCTL_H__
#define __FXIOCTL_H__

// comment from winioctl.h just prior to the define for CTL_CODE:
//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.

//
// This is a private IOCTL for use by the display driver to unmap membase0
// and membase1 for Glide.
//
#define IOCTL_VIDEO_QUERY_GLIDE_ACCESS_RANGES \
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfd3, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// This is a private IOCTL for use by the display driver to map membase0
// and membase1 for Glide.
//
#define IOCTL_VIDEO_FREE_GLIDE_ACCESS_RANGES \
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfd4, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct
{
#ifdef SLI_AA
  VIDEO_MEMORY VideoMemory[3 + HWINFO_SST_MAX_NUM_CHIPS * HWINFO_SST_MAX_CHIP_INDEX];
#else
  VIDEO_MEMORY VideoMemory[3];
#endif
  HANDLE  hProcess;
} GLIDE_BASE_INFO;

//
// This is a private IOCTL for use by the display driver to query a value
// from the registry
//
#define IOCTL_3DFX_QUERY_REGISTRY_VALUE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfd5, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// This is a private IOCTL for use by the display driver to set a value
// in the registry
//
#define IOCTL_3DFX_SET_REGISTRY_VALUE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfd6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define TDFX_MAX_VALUE_NAME_LENGTH  64
#define TDFX_MAX_DATA_LENGTH        1024

// Structure used by IOCTL_3DFX_QUERY_REGISTRY_VALUE

typedef struct _TDFX_QUERY_VALUE_INFO
{
  ULONG Type;
  ULONG DataLength;
  UCHAR Data[TDFX_MAX_DATA_LENGTH];
} TDFX_QUERY_VALUE_INFO;

// Structure used by IOCTL_3DFX_SET_REGISTRY_VALUE

typedef struct _TDFX_SET_VALUE_INFO
{
  ULONG ValueNameLength;
  UCHAR ValueName[TDFX_MAX_VALUE_NAME_LENGTH];
  ULONG Type;
  ULONG DataLength;
  UCHAR Data[TDFX_MAX_DATA_LENGTH];
} TDFX_SET_VALUE_INFO;

#if ENABLE_LOG_FILE
//
// This is a private IOCTL for use by the display driver to write a buffer
// to a log file
//
#define IOCTL_3DFX_WRITE_LOG_FILE \
  CTL_CODE(FILE_DEVICE_VIDEO, 0xfd7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

// Structure and IOCTL used to collect vendor and device IDs from the
// miniport. If in the future it becomes necessary to collect more info
// this IOCTL should be used to pass the data.

typedef struct _TDFX_IDENTITY_INFO
{
	USHORT	VendorID;
	USHORT	DeviceID;
	ULONG	ChipRevision;
	ULONG	CpuType;
	ULONG   CustomerNumber;
    ULONG   usSubSystemID;     // Subsystem ID's
} TDFX_IDENTITY_INFO, *PTDFX_IDENTITY_INFO;

#define IOCTL_3DFX_IDENTITY_INFO \
	  CTL_CODE(FILE_DEVICE_VIDEO, 0xfd8, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// This are private IOCTLs for use by the display driver to save and restore
// the gamma table in the registry.
//
#define IOCTL_VIDEO_REG_SAVE_GAMMA_LUT \
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfd9, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT \
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfda, METHOD_BUFFERED, FILE_ANY_ACCESS)

#if (_WIN32_WINNT < 0x0500)
typedef struct _TDFX_LINEARADDR_INFO
{
	PULONG	ulAddress;
	PULONG	ulLinearBase;
} TDFX_LINEARADDR_INFO, *PTDFX_LINEARADDR_INFO;

#define IOCTL_3DFX_LINEARBASE_INFO \
	  CTL_CODE(FILE_DEVICE_VIDEO, 0xfdb, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

//
// This ioctl requests the bios version string from the miniport
//
typedef struct _TDFX_BIOSVERSION_INFO
{
	PUCHAR	ucString;
} TDFX_BIOS_VERSION_INFO, *PTDFX_BIOS_VERSION_INFO;

#define IOCTL_3DFX_GET_BIOS_VERSION \
	  CTL_CODE(FILE_DEVICE_VIDEO, 0xfdc, METHOD_BUFFERED, FILE_ANY_ACCESS)

// for whatever reason, the display driver uses AGP_CMDFIFO and the miniport
// uses AGP_CMD_FIFO so we need to check both here
#if (_WIN32_WINNT >= 0x0500) && (defined (AGP_CMDFIFO) || defined(AGP_CMD_FIFO))
//
// This ioctl passes the AGP fifo info structure to the display driver.
//
#define IOCTL_3DFX_GET_AGP_FIFO_INFO \
	  CTL_CODE(FILE_DEVICE_VIDEO, 0xffc, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif

#define TVOUT_SUPPORTED 1
#ifdef TVOUT_SUPPORTED
// TV Out and LCD support

#define IOCTL_TV_TRANSACTION \
	  CTL_CODE(FILE_DEVICE_VIDEO, 0xfdd, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_QUERY_MONITOR \
	  CTL_CODE(FILE_DEVICE_VIDEO, 0xfde, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Structure used by IOCTL_QUERY_MONITOR
// Must match driver's QGETSET_MONITOR_CTL in qmodes.h
typedef struct _TDFX_QUERY_MONITOR_INFO {
	ULONG dwSubFunc;
	ULONG monitorStatus;
	ULONG monitorControl;
} TDFX_QUERY_MONITOR_INFO;
// values for use in above structure
#define ENABLE_MONITOR         BIT(0)
#define DISABLE_MONITOR        BIT(1)
#define MONITOR_IS_ENABLED     BIT(2)


#define IOCTL_SET_ALTERNATE_DISPLAYS \
	  CTL_CODE(FILE_DEVICE_VIDEO, 0xfe1, METHOD_BUFFERED, FILE_ANY_ACCESS)
// the following definitions are duplicates of those in funcapi.h and need to be kept in sync with that file and
// 3dfx Tools API.
#define STB_FUNCTION_VGA				0x01
#define STB_FUNCTION_TV					0x02
#define STB_FUNCTION_DFP				0x04


#define IOCTL_GET_MISC_VALUES \
	  CTL_CODE(FILE_DEVICE_VIDEO, 0xfe2, METHOD_BUFFERED, FILE_ANY_ACCESS)
typedef struct _TDFX_MISC_INFO
{
	ULONG	dwDefaultClock;
	ULONG	dwMinClock;
	ULONG   dwMaxClock;
} TDFX_MISC_INFO, *PTDFX_MISC_INFO;


#define IOCTL_DFP_TRANSACTION \
	  CTL_CODE(FILE_DEVICE_VIDEO, 0xfdf, METHOD_BUFFERED, FILE_ANY_ACCESS)

#if (_WIN32_WINNT < 0x500)
// the following IOCTL is defined for Win2K in "ntddvdeo.h". It is defined to a different value
// here for WinNT4 use.
#define IOCTL_VIDEO_HANDLE_VIDEOPARAMETERS \
      CTL_CODE(FILE_DEVICE_VIDEO, 0xfe0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif
#endif  //def TVOUT_SUPPORTED

// Note: IOCTL_3DFX_GET_AGP_FIFO_INFO uses value 0xffc

// This is a private IOCTL for use by the display driver to allocate non-cached memory
//
#define IOCTL_ALLOCATE_NONCACHED_MEMORY \
	CTL_CODE(FILE_DEVICE_VIDEO, 0xffd, METHOD_BUFFERED, FILE_ANY_ACCESS)

// this is a private IOCTL for use by the display driver to map memory into the current
// process
#define IOCTL_MAP_MEMORY_TO_CURRENT_PROCESS \
	CTL_CODE(FILE_DEVICE_VIDEO, 0xffe, METHOD_BUFFERED, FILE_ANY_ACCESS)

// this is a private IOCTL for use by the display driver to allocate
// KMVTBUFF ,which shares betwee miniport and display driver
#define IOCTL_ALLOC_KMVT_MEMORY \
	CTL_CODE(FILE_DEVICE_VIDEO, 0xfff, METHOD_BUFFERED, FILE_ANY_ACCESS)

// at this point we've burned up everything between 0xfd3 - 0xfff
// (except 0xfec, for whatever reason)
// in order to stay within the 2048 - 4095 range,
// we need to start going backward from 0xfd3

#ifdef SLI_AA
//
// This is a private IOCTL for use by the display driver to enable SLI/AA mode
//
#define IOCTL_3DFX_SLI_AA_ENABLE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfd2, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// This is a private IOCTL for use by the display driver to disable SLI/AA mode
//
#define IOCTL_3DFX_SLI_AA_DISABLE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfd1, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _sli_aa_chipinfo
{
  ULONG dwChips;            // dwChips: Number of chips in multi-chip configuration (1-4)
  ULONG dwsliEn;            // dwsliEn: Sli is to be enabled (0,1)
  ULONG dwaaEn;             // dwaaEn: Anti-aliasing is to be enabled (0,1)
  ULONG dwaaSampleHigh;     // dwaaSampleHigh: 0->Enable 2-sample AA, 1->Enable 4-sample AA
  ULONG dwsliAaAnalog;      // dwsliAaAnalog: 0->Enable digital SLI/AA, 1->Enable analog Sli/AA
  ULONG dwsli_nlines;       // dwsli_nLines: Number of lines owned by each chip in SLI (2-128)
  ULONG dwCfgSwapAlgorithm; // Swap Buffer Algorithm
} SLI_AA_CHIPINFO, *PSLI_AA_CHIPINFO;

typedef struct _sli_aa_meminfo
{
  ULONG dwTotalMemory;                // In MB
  ULONG dwTileMark;                   // In MB
  ULONG dwTileCmpMark;                // In MB
  ULONG dwaaSecondaryColorBufBegin;   // In MB
  ULONG dwaaSecondaryDepthBufBegin;   // In MB
  ULONG dwaaSecondaryDepthBufEnd;     // In MB
  ULONG dwBpp;                        // Bits Per Pixel
} SLI_AA_MEMINFO, *PSLI_AA_MEMINFO;

typedef struct sli_aa_request
{
  SLI_AA_CHIPINFO ChipInfo;
  SLI_AA_MEMINFO  MemInfo;
} SLI_AA_REQUEST, *PSLI_AA_REQUEST;

typedef struct _TDFX_SLI_AA_INFO
{
  ULONG numUnits;
  ULONG sliMappedAddress[HWINFO_SST_MAX_NUM_CHIPS][HWINFO_SST_MAX_CHIP_INDEX];
} TDFX_SLI_AA_INFO, *PTDFX_SLI_AA_INFO;

// ioctl for display driver to get info about num sli chips and
// addesses for mmio regs, lfb and io space for each chip
// 0xfd0 was used for IOCTL_VMI_FUNCTION so skip from 0xfd1 to 0xfcf
#define IOCTL_3DFX_SLI_AA_INFO \
	  CTL_CODE(FILE_DEVICE_VIDEO, 0xfcf, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define H3G_PCI_READ    0x00
#define H3G_PCI_WRITE   0x01
typedef struct _hwpciop
{
  ULONG dwOp;             // Operation Read or Write?
  ULONG dwFunc;           // Which Unit?
  ULONG dwOffset;         // Offset [0..255]
  ULONG dwValue;          // Value to Read into or Write from
} HWPCIOP;

#define IOCTL_3DFX_PCI_OP \
	  CTL_CODE(FILE_DEVICE_VIDEO, 0xfce, METHOD_BUFFERED, FILE_ANY_ACCESS)

#if (_WIN32_WINNT >= 0x0500)
// only define this for w2k
//#define RD_ABORT_ERROR
#endif

#ifdef RD_ABORT_ERROR
#define IOCTL_3DFX_ENABLE_SLI_READ\
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfcc, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_3DFX_DISABLE_SLI_READ\
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfcb, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif  // RD_ABORT_ERROR
#endif  // SLI_AA

//
// This is a private IOCTL for use by the display driver to
// communicate with VMI or PLD
#define IOCTL_VMI_FUNCTION \
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfd0, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// This is a private IOCTL for use by the display driver to
// communicate scaling rectangles to Micronas V3TV chip
#define IOCTL_V3TV_SCALE_FUNCTION \
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfcd, METHOD_BUFFERED, FILE_ANY_ACCESS)

#if (_WIN32_WINNT >= 0x0500)
#define IOCTL_3DFX_GET_CURRENT_PROCESS_ID \
    CTL_CODE(FILE_DEVICE_VIDEO, 0xfca, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

#endif /* __FXIOCTL_H__ */


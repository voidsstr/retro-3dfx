/* -*-c++-*- */
/* $Header: devbits.h, 4, 10/11/00 8:50:55 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** Portions Copyright (C) 1995 Microsoft Corporation.  All Rights Reserved.
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
** File name:   devbits.h   
**
** Description: Structures, macros, etc. for device bitmaps.
**
** $Revision: 4$
** $Date: 10/11/00 8:50:55 PM$
**
** $History: devbits.h $
** 
** *****************  Version 9  *****************
** User: Michael      Date: 12/28/98   Time: 11:23a
** Updated in $/devel/h3/Win95/dx/dd16
** Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
** files where this was missing.
** 
** 
*/

#ifndef _DEVBITS_H_
#define _DEVBITS_H_

#include <dqueue.h>

// Default for the filter function
#define MIN_DEVBIT_X (8)
#define MIN_DEVBIT_Y (8)
#define MAX_DEVBIT_X (1600)
#define MAX_DEVBIT_Y (1200)

#define STR_ENABLE ("DevBitEnable")
#define STR_FILTERENABLE ("DevBitFilterEnable")
#define STR_MIN_X ("DevBitMinX")
#define STR_MIN_Y ("DevBitMinY")
#define STR_MAX_X ("DevBitMaxX")
#define STR_MAX_Y ("DevBitMaxY")

typedef struct reginfo {
   WORD FAR * lpValue;
   WORD nDefault;
   char FAR * lpStr;
   } REGINFO, * PREGINFO, FAR * LPREGINFO;

typedef struct devfilter {
   WORD nMinX;       //Minimum size to discard runts
   WORD nMinY;       //Minimum size to discard runts
   WORD nMaxX;       //Size that is to large
   WORD nMaxY;       //Size that is to large
   WORD nDevBitsEnabled;    //Should we do Device Bitmaps
   WORD nFilterEnabled;    //Use the filter
} DEVFILTER, * PDEVFILTER, FAR * LPDEVFILTER;
#define DV_NOTINUSE (0x00)
#define DV_ONHOST (0x01)
#define DV_OFFSCREEN (0x02)

#ifdef PERF_NEWMM
typedef struct devinfo {
   DWORD deBitsOffset;     // fword offset to bits of dib
   WORD  wBitsSel;         // word selector of dib
   WORD  wDeltaScan;      
   WORD  wWidthBytes;
   DWORD pNode;            // Pointer to the pNode associated with this device bitmap
} DEVINFO, * PDEVINFO, FAR * LPDEVINFO;
#else
typedef struct devinfo {
   DWORD dwAddr;           // Offscreen Address of Device BitMap
   DWORD deBitsOffset;     // fword offset to bits of dib
   WORD  wHlpPDevice;      // Handle to lpPDevice or Handle to Host Stage 1 Memory
   WORD  wDevPitch;        // Pitch of Device
   WORD  wHostPitch;       // Pitch of the Host
   WORD  wHeight;          // Height of the host
   WORD  wStatus;          // Where the Device Bit map is
} DEVINFO, * PDEVINFO, FAR * LPDEVINFO;
#endif

typedef struct devqueue {
   struct devqueue * pLeft;     // Pointer to the left
   struct devqueue * pRight;    // Pointer to the Right
   DEVINFO DevInfo;           // Device Bit Map Information
   } DEVQUEUE, * PDEVQUEUE, FAR * FPDEVQUEUE;

#define RANGE(val,lo,hi) ((lo) <= (val) && (val) <= (hi))

// The number of DEVQUEUE structures in the DevBitPool

#ifdef IFB
#define NUMBER_DEVBIT (200 * 3)   // big array for infinite frame buffer test
#else
#define NUMBER_DEVBIT (500)
#endif


//Stolen from windowsx.h
#define     GlobalPtrHandle(lp)         \
                ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

// extern functions
void DoAllHost(void);
void _loadds WINAPI DoHostStage1(DWORD dwAddr);
void _loadds DoHostStage2(LPDIBENGINE lpPDevice);

#endif

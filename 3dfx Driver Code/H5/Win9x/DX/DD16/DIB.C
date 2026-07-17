/* -*-c++-*- */
/* $Header: dib.c, 2, 10/11/00 8:50:58 PM, Brent$ */
/*
** Copyright (c) 1995-1998, 3Dfx Interactive, Inc.
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
** File name:   dib.h   
**
** Description: DIB functions.  Appears to no longer be used since
**              functionality was moved to the 32-bit side.
**
** $Revision: 2$
** $Date: 10/11/00 8:50:58 PM$
**
** $History: dib.c $
** 
** *****************  Version 10  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 6:51a
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 9  *****************
** User: Andrew       Date: 1/09/99    Time: 11:29a
** Updated in $/devel/h3/Win95/dx/dd16
** Added back in the call to DoHostStage2
** 
** *****************  Version 8  *****************
** User: Michael      Date: 12/28/98   Time: 11:24a
** Updated in $/devel/h3/Win95/dx/dd16
** Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
** files where this was missing.
** 
** *****************  Version 7  *****************
** User: Andrew       Date: 9/23/98    Time: 11:11p
** Updated in $/devel/h3/Win95/dx/dd16
** removed DoHostStage2
** 
** *****************  Version 6  *****************
** User: Michael      Date: 6/22/98    Time: 7:22a
** Updated in $/devel/h3/Win95/dx/dd16
** ChrisE's (IGX) addition of code for FxStretchDIBits.  Fixes 1893.
** 
** *****************  Version 5  *****************
** User: Michael      Date: 6/03/98    Time: 3:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Mark L's (IGX) code to fix a hole in the device bitmap consistency
** checking in StretchDIBits.  Fixes #1695.
**
*/

#include "header.h"

#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif

extern void DoHostStage2(LPDIBENGINE lpPDevice);

extern WORD hwAll;
extern WORD hwDibBlt;

int dbgdibblt=0;
int dbgdibdev=0;
int dbgdibstrdi=0;

unsigned long dibbltcount=0;
unsigned long dibdevcount=0;

typedef struct _histstruct{
	unsigned long ten;
	unsigned long hun;
	unsigned long thou;
	unsigned long tenk;
	unsigned long twentyk;
	unsigned long big;
	
} HIST_STRUCT;
typedef HIST_STRUCT far * lpHIST_STRUCT;
HIST_STRUCT dibdev_hist, dibblt_hist;
void init_hist_struct(lpHIST_STRUCT);

unsigned long dibdev10    =0;
unsigned long dibdev100   =0;
unsigned long dibdev1000  =0;
unsigned long dibdev10000 =0;
unsigned long dibdevbig   =0;

unsigned long dibdevbpp1   =0;
unsigned long dibdevbpp4   =0;
unsigned long dibdevbpp8   =0;
unsigned long dibdevbpp16   =0;
unsigned long dibdevbpp24   =0;
unsigned long dibdevbpp32   =0;
unsigned long dibdevbppx   =0;
unsigned long dibdevrle4   =0;
unsigned long dibdevrle8   =0;

unsigned long dibblt10    =0;
unsigned long dibblt100   =0;
unsigned long dibblt1000  =0;
unsigned long dibblt10000 =0;
unsigned long dibbltbig   =0;

unsigned long dibbltbpp1   =0;
unsigned long dibbltbpp4   =0;
unsigned long dibbltbpp8   =0;
unsigned long dibbltbpp16   =0;
unsigned long dibbltbpp24   =0;
unsigned long dibbltbpp32   =0;
unsigned long dibbltbppx   =0;
unsigned long dibbltrle4   =0;
unsigned long dibbltrle8   =0;

unsigned long dibstrdi10    =0;
unsigned long dibstrdi100   =0;
unsigned long dibstrdi1000  =0;
unsigned long dibstrdi10000 =0;
unsigned long dibstrdibig   =0;

unsigned long dibstrdibpp1   =0;
unsigned long dibstrdibpp4   =0;
unsigned long dibstrdibpp8   =0;
unsigned long dibstrdibpp16   =0;
unsigned long dibstrdibpp24   =0;
unsigned long dibstrdibpp32   =0;
unsigned long dibstrdibppx   =0;
unsigned long dibstrdirle4   =0;
unsigned long dibstrdirle8   =0;

int histdibblt=0;
int histdibdev=0;
int histdibstrdi=0;


int hwdib_blt( DIBENGINE FAR * lpBitmap,   // device dib
			WORD iStart,
			WORD cScans,
			LPVOID lpDIBits,           // buffer
			LPBITMAPINFO lpBitmapInfo );

WORD FAR PASCAL _loadds zzStretchDIBits(
				DIBENGINE FAR * lpBitmap,
				WORD fGet,
				WORD DstX,
				WORD DstY,
				WORD DstXE,
				WORD DstYE,
				WORD SrcX,
				WORD SrcY,
				WORD SrcXE,
				WORD SrcYE,
				VOID * lpBits,
				LPBITMAPINFO lpBitmapInfo,
				LPINT lpTranslate,
				DWORD dwROP,
				DIB_Brush8 far  * lpBrush,
				LPDRAWMODE lpDrawMode,
				LPRECT lpClipRect)

{
#ifndef SKIP_STAGE2
    // Ensure bitmap is synced up if DevBitmap was hostified
    if ( (lpBitmap->deType == TYPE_DIBENG) &&
        ((lpBitmap->deFlags & (VRAM|OFFSCREEN)) == (VRAM|OFFSCREEN))) {
        DoHostStage2(lpBitmap);
    }
#endif

	return (DIB_StretchDIBits( lpBitmap, fGet, DstX,  DstY, DstXE,
				DstYE, SrcX, SrcY, SrcXE, SrcYE, lpBits,
				lpBitmapInfo, lpTranslate, dwROP,
				lpBrush, lpDrawMode, lpClipRect));
}



int hwdib_blt( DIBENGINE FAR * lpBitmap,   // device dib
			WORD iStart,
			WORD cScans,
			LPVOID lpDIBits,           // buffer
			LPBITMAPINFO lpBitmapInfo )
{
DWORD  dwords_per_scan;

	// calc dword to write	
	dwords_per_scan=(lpBitmapInfo->bmiHeader.biWidth*
					lpBitmapInfo->bmiHeader.biBitCount);
	//mono
	dwords_per_scan+=31;
	dwords_per_scan>>=5; //divide by 32
	
	
	// 4 bpp
	dwords_per_scan+=7;
	dwords_per_scan>>=3; //divide by 8

	// 8 bpp
	dwords_per_scan+=3;
	dwords_per_scan>>=2; //divide by 4

	// 24 bpp
	dwords_per_scan+=31;
	dwords_per_scan>>=5; //divide by 32
return 1;
}

void init_hist_struct(lpHIST_STRUCT generic_hist)
{

	generic_hist->ten=0;
	generic_hist->hun=0;
	generic_hist->thou=0;
	generic_hist->tenk=0;
	generic_hist->twentyk=0;
	generic_hist->big=0;
}

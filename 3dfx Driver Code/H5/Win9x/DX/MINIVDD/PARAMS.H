/* -*-c++-*- */
/* $Header: params.h, 2, 10/11/00 8:54:51 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File name:   params.h
**
** Description: Structures of stack variables coming across in the
**              16- 32-bit thunking.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:51 PM$
**
** $History: params.h $
** 
** *****************  Version 12  *****************
** User: Michael      Date: 1/12/99    Time: 9:22a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 11  *****************
** User: Michael      Date: 6/22/98    Time: 7:15a
** Updated in $/devel/h3/Win95/dx/minivdd
** ChrisE's (IGX) addition of FxStretchDIBits and supporting
** functionality.  Fixes 1893.
** 
** *****************  Version 10  *****************
** User: Ken          Date: 4/27/98    Time: 5:35p
** Updated in $/devel/h3/win95/dx/minivdd
** complete (well, almost) enter/leave implementation for gdi32
** 
** *****************  Version 9  *****************
** User: Ken          Date: 4/24/98    Time: 1:33p
** Updated in $/devel/h3/win95/dx/minivdd
** first installment of FXENTER / FXLEAVE display driver drawing function
** work.  gdi32 only, no gdi16 yet.   works, but is slow, don't measure
** performance until all work is done, in about a week.
** also renamed fields in some blit parameter functions to be a bit more
** meaningful
** 
** *****************  Version 8  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

//: params.h


#ifndef __PARAMS_H__
#define __PARAMS_H__


#pragma pack( 2 )

typedef struct struct_BitBlt
{
    DWORD lpDrawMode;
    DWORD lpPBrush;
    DWORD Rop;
    WORD  height;
    WORD  width;
    WORD  SrcY;
    WORD  SrcX;
    DWORD lpSrcDev;
    WORD  DstY;
    WORD  DstX;
    DWORD lpDstDev;
}
BitBltParams;

typedef struct struct_ColorInfo
{
	DWORD lpPColor;
	DWORD dwColorin;
	DWORD lpDestDev;
}
ColorInfoParams;

typedef struct struct_ExtTextOut
{
    WORD  wOptions;
    DWORD lpOpaqueRect;
    DWORD lpCharWidths;
    DWORD lpTextXForm;
    DWORD lpDrawMode;
    DWORD lpFontInfo;
    WORD  wCount;
    DWORD lpString;
    DWORD lpClipRect;
    WORD  wDestYOrg;
    WORD  wDestXOrg;
    DWORD lpDst;
}
ExtTextOutParams;


typedef struct struct_Output
{
	DWORD lpClipRect;
	DWORD lpDrawMode;
	DWORD lpPBrush;
	DWORD lpPPen;
	DWORD lpPoints;
	SHORT wCount;
	WORD  wStyle;
	DWORD lpDestDev;
}
OutputParams;


typedef struct struct_DeviceBitmapBits
{
    DWORD  lpTranslate;
    DWORD  lpDrawMode;
    DWORD  lpBitmapInfo;
    DWORD  lpDIBits;
    WORD   cScans;
    WORD   iStart;
    WORD   fGet;
    DWORD  lpBitmap;
}
DeviceBitmapBitsParams;


typedef struct struct_SetDIBitsToDevice
{
    DWORD  lpTranslate;
    DWORD  lpBitmapInfo;
    DWORD  lpDIBits;
    DWORD  lpDrawMode;
	DWORD  lpClipRect;
    WORD   cScans;
    WORD   iScan;
    WORD   Y;
    WORD   X;
    DWORD  lpDestDev;
}
SetDIBitsToDeviceParams;


typedef struct struct_StretchDIBits
{
    DWORD  lpClipRect;
    DWORD  lpDrawMode;
    DWORD  lpPBrush;
    DWORD  Rop;
    DWORD  lpTranslate;
    DWORD  lpBitmapInfo;
    DWORD  lpBits;
    WORD   srcHeight;
    WORD   srcWidth;
    WORD   srcY;
    WORD   srcX;
    WORD   dstHeight;
    WORD   dstWidth;
    WORD   dstY;
    WORD   dstX;
    WORD   fGet;
    DWORD  lpDestDev;
}
StretchDIBitsParams;


typedef struct struct_StretchBlt
{
	DWORD  lpClipRect;
	DWORD  lpDrawMode;
	DWORD  lpPBrush;
	DWORD  Rop;
	WORD   srcHeight;
	WORD   srcWidth;
	WORD   SrcY;
	WORD   SrcX;
	DWORD  lpSrcDev;
	WORD   dstHeight;
	WORD   dstWidth;
	WORD   DstY;
	WORD   DstX;
	DWORD  lpDestDev;
}
StretchBltParams;

typedef struct struct_SelectBitmap
{
    DWORD fFlags;
    DWORD lpBitmap;
    DWORD lpPrevBitmap;
    DWORD lpDevice;
}
SelectBitmapParams;

typedef struct struct_BitmapBits
{
	DWORD lpBits;
	DWORD dwCount;
	DWORD fFlags;
	DWORD lpDevice;
}
BitmapBitsParams;

typedef struct struct_BeginAccess
{
    WORD  Flags;
    WORD  Bottom;
    WORD  Right;
    WORD  Top;
    WORD  Left;
    DWORD lpDevice;
}
BeginAccessParams;

typedef struct struct_EndAccess
{
    WORD  Flags;
    DWORD lpDevice;
}
EndAccessParams;

typedef struct struct_MapSharedData
{
    DWORD lpDriverData;
}
MapSharedDataParams;

typedef struct struct_initFifoParams
{
    DWORD start;
    DWORD size;
}
initFifoParams;


#pragma pack()

#define SRCCOPYROP 0XCC000000L
#endif

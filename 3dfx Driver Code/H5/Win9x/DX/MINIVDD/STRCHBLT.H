/* -*-c++-*- */
/* $Header: strchblt.h, 3, 10/11/00 8:55:09 PM, Brent$ */
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
** File name:   strchblt.h
**
** Description: StretchBlt function definitions and externs.
**
** $Revision: 3$
** $Date: 10/11/00 8:55:09 PM$
**
** $History: strchblt.h $
** 
** *****************  Version 6  *****************
** User: Michael      Date: 1/15/99    Time: 6:35a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 5  *****************
** User: Ken          Date: 4/24/98    Time: 1:33p
** Updated in $/devel/h3/win95/dx/minivdd
** first installment of FXENTER / FXLEAVE display driver drawing function
** work.  gdi32 only, no gdi16 yet.   works, but is slow, don't measure
** performance until all work is done, in about a week.
** also renamed fields in some blit parameter functions to be a bit more
** meaningful
** 
** *****************  Version 4  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
** 
** *****************  Version 3  *****************
** User: Ken          Date: 4/10/98    Time: 3:16p
** Updated in $/devel/h3/win95/dx/minivdd
** ken's fixes to chris e's fixes to stretch blit clipping
**
*/

/*
 extern WORD hwAll;
 extern WORD hwBlt; hwBitBltSS, hwBitBltPS, hwBitBltHS ;
 extern WORD hwStretchBlt, hwStretchBltSS;
 extern WORD hwText, hwAltpolygon, hwWindpolygon;hwPolyline; 
 extern WORD hwRect, hwOutput ;

*/

/***************************************************************************
 *
 * functions in StretchBlt.c
 *
 ***************************************************************************/

BOOL HWStretchBltScreen 
(
   StretchBltParams * lpParams,
   DIBENGINE * lpDst,
   DIBENGINE * lpSrc
);

int
HWStretchBltPattern 
(
   StretchBltParams * lpParams,
   DIBENGINE * lpDst,
   DIBENGINE * lpSrc
);

HWStretchBltMemory 
(
   StretchBltParams * lpParams,
   DIBENGINE * lpDst,
   DIBENGINE * lpSrc
);

int StretchBltBufferToScreen
(
   LPDIBENGINE lpDst,
   WORD DstX, 
   WORD dstScanY, 
   WORD dstWidth,
   WORD srcWidth,
   DWORD bufFormatData,
   DWORD clipSelect
);

int LoadColorBuffer
( 
   DIBENGINE FAR * Src, 
   WORD SrcX,
   WORD SrcY, 
   WORD width,
   WORD height,
   DWORD bufFormatData
);


int LoadMonoBuffer
(
   DIBENGINE FAR * Src, 
   WORD SrcX,
   WORD SrcY, 
   WORD width,
   WORD height,
   DWORD bufFormatData,
   DRAWMODE FAR *lpDrawMode
) ;
extern void decodebrushflag( DIB_Brush8 * lpBrush);
extern void decodebrushstyle (WORD style);


extern DWORD HandleSrcFormat
(
   DIBENGINE FAR * Src, 
   short width, 
   short height
);


int LoadShrinkBuffer(DIBENGINE FAR * Src, WORD SrcX, WORD SrcY, WORD srcWidth, WORD dstWidth, WORD height, DWORD bufFormatData);
int BltBufferScreenToScreen(LPDIBENGINE lpDst, WORD DstX, WORD dstScanY, WORD dstWidth, DWORD bufFormatData, DWORD clipSelect);

extern WORD hwStretchBlt;
extern WORD hwStretchBltSS;
extern WORD hwStretchBltHS;
extern WORD hwStretchBltPS;

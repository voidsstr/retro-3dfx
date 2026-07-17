/* -*-c++-*- */
/* $Header: bitblt.h, 2, 10/11/00 8:53:51 PM, Brent$ */
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
** File name:   bitblt.h
**
** Description: forward referenced functions and #defines for
**              BitBlt functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:53:51 PM$
**
** $History: bitblt.h $
** 
** *****************  Version 7  *****************
** User: Michael      Date: 1/04/99    Time: 1:20p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 6  *****************
** User: Michael      Date: 6/18/98    Time: 7:03a
** Updated in $/devel/h3/Win95/dx/minivdd
** Rewrote HandleSpecialBrush().  Put missing CMDFIFO_SAVE/_RELOAD around
** call to HandleSpecialBrush().  Fixes 1901.
** 
** *****************  Version 4  *****************
** User: Edwin        Date: 5/09/98    Time: 5:38p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added WriteBMPColorSrc function prototype and WRITECOLORSRCFUNC typedef
** to support non TYPE_DIBENG bitmap source.
** 
** *****************  Version 3  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

/***************************************************************************
 *
 * type definitions in bitblt.c
 *
 ***************************************************************************/

// pointer to WriteColorSrc function
typedef LONG (
    (* WRITECOLORSRCFUNC) (LPDIBENGINE, DWORD, DWORD, DWORD)
    );


/***************************************************************************
 *
 * functions in bitblt.c
 *
 ***************************************************************************/

LONG HWBitBltScreen(BitBltParams *, LPDIBENGINE, LPDIBENGINE); 

LONG HWBitBltMemory(BitBltParams *, LPDIBENGINE, LPDIBENGINE); 

LONG  HWBitBltPattern(BitBltParams *, LPDIBENGINE, LPDIBENGINE); 

extern void decodebrushflag( DIB_Brush8 * lpBrush);
extern void decodebrushstyle (WORD style);

LONG WriteBMPColorSrc(LPDIBENGINE, DWORD, DWORD, DWORD);

LONG WriteColorSrc(LPDIBENGINE Src,
			DWORD SrcOffset ,
			DWORD NumberOfScans,
			DWORD DwordsInScanline);

WriteMonoSrc( LPBITMAP Src,
			DWORD SrcOffset ,
			DWORD NumberOfScans,
			DWORD DwordsInScanline);


DWORD HandleBrush(DIB_Brush8 FAR * lpBrush, DWORD Rop, 
					DRAWMODE  FAR * lpDrawMode);

DWORD HandleFormat(DIBENGINE FAR * Src);

void ExpandMonoBrush(short DstBytePix, DIB_Brush8 FAR * lpBrush, 	
					DRAWMODE FAR * lpDrawMode, BYTE * patbuf);

DWORD HandleSpecialBrush(DIB_Brush8 FAR * lpBrush, DWORD Rop,
                    DRAWMODE FAR * lpDrawMode);





// 4k byte page
#define MAXCOLORDWORDS 1024
#define MAXMONODWORDS  1024

//  NUM_SCANS defines the number of scanlines to request from Makeroom 
//  function.

#define NUM_SCANS 4


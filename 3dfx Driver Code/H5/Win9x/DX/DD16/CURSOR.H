/* $Header: cursor.h, 2, 10/11/00 8:50:52 PM, Brent$ */
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
** File name:   cursor.h
**
** Description: Structures, macros, etc. for cursor functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:50:52 PM$
**
** $History: cursor.h $
** 
** *****************  Version 11  *****************
** User: Andrew       Date: 6/04/99    Time: 4:09p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added some new function prototypes
** 
** *****************  Version 9  *****************
** User: Michael      Date: 12/28/98   Time: 11:23a
** Updated in $/devel/h3/Win95/dx/dd16
** Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
** files where this was missing.
**
*/

#ifndef _CURSOR_H_

#define _CURSOR_H_

#define TRANSLATE(X) ((X) & 0x7FFFFFF)
#define AND_MASK_STRIDE   4UL

typedef struct pointerinfo1 {
   WORD csHotX;       
	WORD csHotY;       
	WORD csWidth;      
	WORD csHeight;     
	WORD csWidthBytes; 
	WORD csColor;           // 0101h = mono, 0801h = 8bpp, etc.
	BYTE csANDBits[4*32];   // 32x32 bit mask = 4 bytes/row 
	BYTE csXORBits[4*32];   // 32x32 color data = 4 bytes/row 
} POINTERINFO1, *PPOINTERINFO1, FAR *LPPOINTERINFO1;

typedef struct pointerinfo4{
   WORD csHotX;       
	WORD csHotY;       
	WORD csWidth;      
	WORD csHeight;     
	WORD csWidthBytes; 
	WORD csColor;           // 0101h = mono, 0801h = 8bpp, etc.
	BYTE csANDBits[4*32];   // 32x32 bit mask = 4 bytes/row 
	BYTE csXORBits[16*32];  // 16x32x1 color data 
} POINTERINFO4, *PPOINTERINFO4, FAR *LPPOINTERINFO4;

typedef struct pointerinfo8{
   WORD csHotX;       
	WORD csHotY;       
	WORD csWidth;      
	WORD csHeight;     
	WORD csWidthBytes; 
	WORD csColor;           // 0101h = mono, 0801h = 8bpp, etc.
	BYTE csANDBits[4*32];   // 32x32 bit mask = 4 bytes/row 
	BYTE csXORBits[32*32];  // 32x32x1 color data 
} POINTERINFO8, *PPOINTERINFO8, FAR *LPPOINTERINFO8;

typedef struct pointerinfo16 {
   WORD csHotX;       
	WORD csHotY;       
	WORD csWidth;      
	WORD csHeight;     
	WORD csWidthBytes; 
	WORD csColor;           // 0101h = mono, 0801h = 8bpp, etc.
	BYTE csANDBits[4*32];   // 32x32 bit mask = 4 bytes/row 
	BYTE csXORBits[64*32];  // 32x32x2 color data
} POINTERINFO16, *PPOINTERINFO16, FAR *LPPOINTERINFO16;

typedef struct pointerinfo24 {
   WORD csHotX;       
	WORD csHotY;       
	WORD csWidth;      
	WORD csHeight;     
	WORD csWidthBytes; 
	WORD csColor;           // 0101h = mono, 0801h = 8bpp, etc.
	BYTE csANDBits[4*32];   // 32x32 bit mask = 4 bytes/row 
	BYTE csXORBits[96*32];  // 32x32x3 color data
} POINTERINFO24, *PPOINTERINFO24, FAR *LPPOINTERINFO24;

typedef struct pointerinfo {
   WORD csHotX;       
	WORD csHotY;       
	WORD csWidth;      
	WORD csHeight;     
	WORD csWidthBytes; 
	WORD csColor;           // 0101h = mono, 0801h = 8bpp, etc.
	BYTE csANDBits[4*32];   // 32x32 bit mask = 4 bytes/row 
	BYTE csXORBits[128*32]; // 32x32x4 color data
} POINTERINFO, *PPOINTERINFO, FAR *LPPOINTERINFO;

int DebugBltCursor(int x, int y);
int DebugBltExclusion(int x, int y);
int ClearCursorRect(int x, int y);
int RestoreCursorExclude(int x, int y);
int SaveCursorExclude(int x, int y);
int DrawCursor(int x, int y);
int DIBMoveCursor(SHORT x, SHORT y);
#ifdef DEBUG
int GottaCursor(void);
#endif
DWORD GetCursorFormat (CURSORSHAPE FAR *lpCursor);
BOOL SetCursorBusy( WORD FAR * lpcursorBusySemaphore);
BOOL ClearCursorBusy( WORD FAR * lpcursorBusySemaphore);
//
// External DIB Engine Functions
//
#if 0
extern UINT PASCAL FAR DIB_SetCursorExt(LPVOID, LPDIBENGINE);
extern UINT PASCAL FAR DIB_MoveCursorExt(WORD, WORD, LPDIBENGINE);
#else
extern void PASCAL FAR DIB_CheckCursorExt(LPDIBENGINE);
#endif


#define CURSOR_DBL_X (1)
#define CURSOR_DBL_Y (2)
#define CURSOR_DBL_BOTH (3)

  
typedef int (FNSAVECURSOREXCLUDE)(int x, int y);
typedef int (FNRESTORECURSOREXCLUDE)(int x, int y);
typedef int (FNDRAWCURSOR)(int x, int y);

#endif

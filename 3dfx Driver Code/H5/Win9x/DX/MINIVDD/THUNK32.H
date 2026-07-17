/* -*-c++-*- */
/* $Header: thunk32.h, 3, 10/11/00 8:55:18 PM, Brent$ */
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
** File name:   thunk32.h
**
** Description: support for 32-bit side of the 16->32 thunker.
**
**
** $Revision: 3$
** $Date: 10/11/00 8:55:18 PM$
**
** $History: thunk32.h $
** 
** *****************  Version 27  *****************
** User: Michael      Date: 1/15/99    Time: 9:25a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 26  *****************
** User: Ken          Date: 7/24/98    Time: 10:38p
** Updated in $/devel/h3/win95/dx/minivdd
** changes to allow 2d driver to run properly synchronized with an AGP
** command fifo (although video memory fifo is still used when the desktop
** has the focus, e.g., a fullscreen 3d app isn't in the foreground)
** 
** *****************  Version 25  *****************
** User: Andrew       Date: 7/23/98    Time: 12:26p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added some flags for BitmapBits
** 
** *****************  Version 24  *****************
** User: Ken          Date: 7/15/98    Time: 4:10p
** Updated in $/devel/h3/win95/dx/minivdd
** optimized FarToFlat() for pentium II
** 
** *****************  Version 23  *****************
** User: Michael      Date: 6/22/98    Time: 7:19a
** Updated in $/devel/h3/Win95/dx/minivdd
** ChrisE's (IGX) addition of code for FxStretchDIBits.  Fixes 1893.
** Also, modifies THUNK_PROLOG to use EBX instead of EDX.
** 
** *****************  Version 22  *****************
** User: Andrew       Date: 6/01/98    Time: 6:57a
** Updated in $/devel/h3/Win95/dx/minivdd
**  Added a macro to convert a stack offset to a 16:16 pointer that can be
** used in FartoFlat to get a 16:32 pointer
** 
** *****************  Version 21  *****************
** User: Artg         Date: 5/01/98    Time: 4:25p
** Updated in $/devel/h3/Win95/dx/minivdd
** sw cursor stuff
** 
** *****************  Version 20  *****************
** User: Ken          Date: 4/27/98    Time: 5:35p
** Updated in $/devel/h3/win95/dx/minivdd
** complete (well, almost) enter/leave implementation for gdi32
** 
** *****************  Version 19  *****************
** User: Ken          Date: 4/27/98    Time: 7:47a
** Updated in $/devel/h3/win95/dx/minivdd
** fixed multi-defined launch_reg_1 warning
** 
** *****************  Version 18  *****************
** User: Ken          Date: 4/24/98    Time: 9:43p
** Updated in $/devel/h3/win95/dx/minivdd
** cleaned up some functions, added R32 macros, 
** 
** *****************  Version 17  *****************
** User: Ken          Date: 4/24/98    Time: 1:33p
** Updated in $/devel/h3/win95/dx/minivdd
** first installment of FXENTER / FXLEAVE display driver drawing function
** work.  gdi32 only, no gdi16 yet.   works, but is slow, don't measure
** performance until all work is done, in about a week.
** also renamed fields in some blit parameter functions to be a bit more
** meaningful
** 
** *****************  Version 16  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

//: thunk32.h


#ifndef __THUNK32_H__
#define __THUNK32_H__


#define FAR
#define WINAPI __stdcall

#define FALSE (0 > 1)
#define TRUE  (0 < 1)

typedef          char  CHAR;
typedef   signed char  SCHAR;
typedef unsigned char  UCHAR;

typedef          short SHORT;
typedef unsigned short USHORT;

typedef          long  LONG;
typedef unsigned long  ULONG;

typedef unsigned char  BYTE,  FAR* LPBYTE;
typedef unsigned short WORD,  FAR* LPWORD;
typedef unsigned long  DWORD, FAR* LPDWORD;
typedef          void  VOID,  FAR* LPVOID;
typedef          int   BOOL;


typedef int (FAR WINAPI *FARPROC)();


#pragma pack( 1 )
    #include "gdidefs.h"
    #include "dibeng.h"
#pragma pack()

#include "3dfx.h"
#include "shared.h"
#include "params.h"
#include "cmclient.h"
#include "hardware.h"
#include "misc.h"

extern GLOBALDATA * lpDriverData;


#define USES_PAT(r) (((r) ^ ((r) >> 4)) & 0x0F)
#define USES_SRC(r) (((r) ^ ((r) >> 2)) & 0x33)
#define USES_DST(r) (((r) ^ ((r) >> 1)) & 0x55)

/* Dword access to rects */
#define RECT_TOP_LEFT(r)        (((DWORD *)(r))[0])
#define RECT_BOTTOM_RIGHT(r)    (((DWORD *)(r))[1])


/*
**  On entry to 16-bit entry points:
**
**      Stack:
**
**  +--------+----------------------+
**  | [SP+4] | params               |
**  +--------+----------------------+
**  | [SP+0] | 16:16 return address |
**  +--------+----------------------+
**
**  On entry to 32-bit entry points:
**
**      AX = flat data selector
**      ES = flat data selector
**
**      High order word of EBP is clear
**
**      Stack:
**
**  +---------+------------------------+
**  | [BP+12] | dwDevNode/params       |
**  +---------+------------------------+
**  | [BP+08] | 16:16 return address   |
**  +---------+------------------------+
**  | [BP+06] | caller's DS            |
**  +---------+------------------------+
**  | [BP+04] | caller's SI            |
**  +---------+------------------------+
**  | [BP+02] | caller's DI            |
**  +---------+------------------------+
**  | [BP+00] | caller's BP            |
**  +---------+------------------------+
**  |   0-3   | stack alignment        |
**  +---------+------------------------+
**  | [SP+12] | extra params           |
**  +---------+------------------------+
**  | [SP+10] | zero                   |
**  +---------+------------------------+
**  | [SP+08] | garbage                |
**  +---------+------------------------+
**  | [SP+06] | zero                   |
**  +---------+------------------------+
**  | [SP+00] | 16:32 return address   |
**  +---------+------------------------+
**
**  Once the 32-bit prolog has executed:
**
**      Stack:
**
**  +----------+------------------------+
**  |    4     | params                 |
**  +----------+------------------------+
**  |    4     | 16:16 return address   |
**  +----------+------------------------+
**  |    2     | caller's DS            |
**  +----------+------------------------+
**  |    2     | caller's SI            |
**  +----------+------------------------+
**  |    2     | caller's DI            |
**  +----------+------------------------+
**  |    2     | caller's BP            |
**  +----------+------------------------+
**  |   0-3    | stack alignment        |
**  +----------+------------------------+
**  | [EBP+16] | extra params           |
**  +----------+------------------------+
**  | [EBP+12] | flat pointer to params | on exit, holds return value on exit
**  +----------+------------------------+
**  | [EBP+10] | zero                   | on exit, holds punt flag in msb
**  +----------+------------------------+
**  | [EBP+08] | return addr selector   |
**  +----------+------------------------+
**  | [EBP+04] | return addr offset     |
**  +----------+------------------------+
**  | [EBP+00] | pointer to caller's BP |
**  +----------+------------------------+
**  | [ESP+00] | locals                 |
**  +----------+------------------------+
**
**  The 32-bit Visual C++ compiler will see the stack thus:
**
**  +----------+-----------------------+
**  | [EBP+12] | pointer to params     |
**  +----------+-----------------------+
**  | [EBP+08] | dummy first parameter |
**  +----------+-----------------------+
**  | [EBP+04] | not used (naked)      |
**  +----------+-----------------------+
**  | [EBP+00] | not used (naked)      |
**  +----------+-----------------------+
**  | [ESP+00] | locals                |
**  +----------+-----------------------+
*/


#define THUNK_PROLOG                    \
{                                       \
    __asm   push    ebp                 \
    __asm   movzx   ebp, sp             \
    __asm   sub     esp, __LOCAL_SIZE   \
                                        \
    __asm   mov     ds, ax              \
                                        \
    __asm   mov     edx, dwLdtPtr       \
    __asm   mov     eax, pParams        \
    __asm   mov     dwDevNode, eax      \
    __asm   mov     ax, ss              \
    __asm   and     eax, 0FFF8h         \
    __asm   add     edx, eax            \
    __asm   mov     al, [edx][4]        \
    __asm   mov     ah, [edx][7]        \
    __asm   shl     eax, 16             \
    __asm   mov     ax, [edx][2]        \
    __asm   add     eax, [ebp]          \
    __asm   add     eax, 12             \
    __asm   mov     pParams, eax        \
}


#define THUNK_EPILOG                    \
{                                       \
    __asm   mov     eax, dwDummy        \
    __asm   mov     esp, ebp            \
    __asm   pop     ebp                 \
    __asm   retf                        \
}


#define DEBUG_FIX                       \
{                                       \
    __asm   movzx   ebp, bp             \
    __asm   mov     eax, [ebp]          \
    __asm   movzx   eax, ax             \
    __asm   mov     [ebp], eax          \
}


#define SelToFlat(sl,flt)               \
{                                       \
    DWORD ts = sl;                      \
    __asm   mov     ebx, dwLdtPtr       \
    __asm   movzx   eax, WORD PTR ts    \
    __asm   and     eax, NOT 7          \
    __asm   add     ebx, eax            \
    __asm   mov     al, [ebx][4]        \
    __asm   mov     ah, [ebx][7]        \
    __asm   shl     eax, 16             \
    __asm   mov     ax, [ebx][2]        \
    __asm   mov     flt, eax            \
}


//
// pentium II optimized version of Vadim's original FarToFlat
// -KMW
//
#define FarToFlat(fr,flt)               \
{                                       \
    DWORD tf = (DWORD)(fr);             \
    __asm   mov     ebx, dwLdtPtr       \
\
    __asm   mov     eax, tf             \
    __asm   shr     eax, 16             \
\
    __asm   and     eax, NOT 7          \
    __asm   add     ebx, eax            \
\
    __asm   mov     eax, [ebx][4]	\
    __asm   mov     ecx, eax		\
    __asm   and     eax, 0xff000000	\
    __asm   shl     ecx, 16		\
    __asm   and     ecx, 0x00ff0000	\
    __asm   or      eax, ecx		\
\
    __asm   mov     ecx, [ebx][2]	\
    __asm   and     ecx, 0xffff		\
    __asm   or      eax, ecx		\
\
    __asm   movzx   ebx, WORD PTR tf	\
    __asm   add     eax, ebx            \
    __asm   mov     flt, eax            \
}

#define STACKFP(OurObject, Our1616)\
{\
   __asm  mov   eax, OurObject\
   __asm  rol   eax, 16\
   __asm  mov   ax, ss\
   __asm  rol   eax, 16\
   __asm  mov   Our1616, eax\
}

#include "cmddefs.h"
#include "h3g.h"
#include "fifomgr.h"

#ifndef LAUNCH_REG_1
#define LAUNCH_REG_1	0x80 >> 2
#endif // #ifndef LAUNCH_REG_1

#define PATTERN_REG_1   0x100 >> 2

#define PUNT_BIT 0x80000000

#define R32(p1, p2) ((DWORD)(p1) << 16 | ((DWORD)(p2) & 0xffff))

extern LPDIBENGINE lpDestDev;
extern LPDIBENGINE lpSrcDev;

extern LPCMINFO lpDestInfo;
extern LPCMINFO lpSrcInfo;

extern DIB_Brush1* lpPBrush;

extern DWORD dwRop;
extern DWORD dwCmd;

extern DWORD dwLdtPtr;
extern BOOL  bPunt;

void FXWAITFORIDLE(void);

extern DWORD UseDibEng;
extern DWORD hwAll;	
extern DWORD hwBlt; hwBitBltSS, hwBitBltPS, hwBitBltHS ;
extern DWORD hwText, hwAltPolygon, hwWindPolygon, hwPolyline; 
extern DWORD hwRect, hwOutput ;
extern DWORD hwScanline,hwPolyScanline;
extern DWORD hwbreakStall, hwpollStall, hwpciFree, pollCount;

extern FxU32 CurrentDstAddr, CurrentDstFormat;

extern DWORD hwBitmapBits, hwBitmapBitsHS, hwBitmapBitsSS, hwBitmapBitsSH;

BOOL FxBitBlt           ( BitBltParams*             pParams );
BOOL FxStretchBlt       ( StretchBltParams*         pParams );
LONG FxOutput           ( OutputParams*             pParams );
BOOL FxExtTextOut       ( ExtTextOutParams*         pParams );
BOOL FxDeviceBitmapBits ( DeviceBitmapBitsParams*   pParams );
BOOL FxSetDIBitsToDevice( SetDIBitsToDeviceParams*  pParams );
BOOL FxStretchDIBits    ( StretchDIBitsParams*      pParams );
BOOL FxSelectBitmap     ( SelectBitmapParams*       pParams );
BOOL FxBitmapBits       ( BitmapBitsParams*         pParams );
DWORD FxBeginAccess32   ( DWORD flags);
DWORD FxEndAccess32      ( DWORD flags);
BOOL LoadPattern        ( VOID );
VOID InitRegs           ( VOID );
VOID BreakStall();
VOID PollStall();
BOOL PCIFree();
void SendMonoBltData(DWORD xs, DWORD xd, DWORD w, DWORD h, LPBITMAP lpSrc);
#endif

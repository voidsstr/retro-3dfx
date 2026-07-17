//: altgdi.h
//: Copyright (C) alt.drivers inc. 1997
//: Glenn Nissen

#define NOGDI
#include <windows.h>
#include <i86.h>
#include <ver.h>

#define NOPTRC
#define PTTYPE POINT
#include <gdidefs.inc>
#include <dibeng.inc>

#include "general.h"
//#include "alliance.h"

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  Macros                                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

VOID BREAKPOINT( WORD );
#pragma aux BREAKPOINT =    \
    "int 3"                 \
    parm    [ax]            \
    modify  [ax];

VOID BREAK( DWORD );
#pragma aux BREAK =         \
    "int 3"                 \
    parm    [dx ax]         \
    modify  [dx ax];

#define SAVEFUNC(d)                                                         \
    *((BYTE *) (d).lpSave + 0) = *((BYTE FAR *) (d).lpTargetCode + 0);      \
    *((BYTE *) (d).lpSave + 1) = *((BYTE FAR *) (d).lpTargetCode + 1);      \
    *((BYTE *) (d).lpSave + 2) = *((BYTE FAR *) (d).lpTargetCode + 2);      \
    *((BYTE *) (d).lpSave + 3) = *((BYTE FAR *) (d).lpTargetCode + 3);      \
    *((BYTE *) (d).lpSave + 4) = *((BYTE FAR *) (d).lpTargetCode + 4);

#define PATCHFUNC(d)                                                        \
    *((BYTE FAR *) (d).lpTargetCode) = 0xEA;                                \
    *((WORD FAR *) ((d).lpTargetCode + 1)) = OFFSETOF( (d).lpNewFunc );     \
    *((WORD FAR *) ((d).lpTargetCode + 3)) = SELECTOROF( (d).lpNewFunc );

#define UNPATCHFUNC(d)                                                      \
    *((BYTE FAR *) (d).lpTargetCode + 0) = *((BYTE *) (d).lpSave + 0);      \
    *((BYTE FAR *) (d).lpTargetCode + 1) = *((BYTE *) (d).lpSave + 1);      \
    *((BYTE FAR *) (d).lpTargetCode + 2) = *((BYTE *) (d).lpSave + 2);      \
    *((BYTE FAR *) (d).lpTargetCode + 3) = *((BYTE *) (d).lpSave + 3);      \
    *((BYTE FAR *) (d).lpTargetCode + 4) = *((BYTE *) (d).lpSave + 4);

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  Typedefs                                                               //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

typedef BOOL (FAR PASCAL *FARENTRY_GDIBITBLT)( HDC, int, int, int, int, HDC, int, int, DWORD );
typedef BOOL (FAR PASCAL *FARENTRY_GDIPATBLT)( HDC, int, int, int, int, DWORD );
typedef int  (FAR PASCAL *FARENTRY_GDIESCAPE)( HDC, int, int, LPCSTR, LPVOID );

typedef DWORD (FAR PASCAL *FARENTRY_DRVCONTROL)      ( LPVOID, WORD, LPVOID, LPVOID );
typedef BOOL  (FAR PASCAL *FARENTRY_DRVBITBLT)       ( LPVOID, WORD, WORD, LPVOID, WORD, WORD, WORD, WORD, long, LPVOID, LPVOID );
typedef WORD  (FAR PASCAL *FARENTRY_DRVREALIZEOBJECT)( LPVOID, WORD, LPVOID, LPVOID, LPVOID );

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  Structures                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

typedef struct
{
    WORD      wSel;
    BYTE      lpSave[5];
    BYTE FAR *lpTargetCode;
    VOID FAR *lpTargetFunc;
    VOID FAR *lpNewFunc;
}
PATCHINFO;

typedef struct
{
    PATCHINFO piEscape;
    PATCHINFO piGdiBitBlt;
    PATCHINFO piGdiPatBlt;
    PATCHINFO piDrvControl;
    PATCHINFO piDrvBitBlt;

    FARENTRY_DRVBITBLT lpDrvBitBlt;

    WORD wGdiSel;
    WORD wDrvSel;

    LPVOID lpDevice;
}
APPINFO;

typedef struct
{
    WORD   wUnknown00;          // 0x00
    WORD   wDcType;             // 0x02
    WORD   wUnknown04;          // 0x04
    WORD   wUnknown06;          // 0x06
    WORD   wUnknown08;          // 0x08
    WORD   wUnknown0A;          // 0x0A
    WORD   wUnknown0C;          // 0x0C
    WORD   wUnknown0E;          // 0x0E
    WORD   wUnknown10;          // 0x10
    WORD   hMetaFile;           // 0x12
    WORD   hrgnClip;            // 0x14
    WORD   hPDevice;            // 0x16
    WORD   hLPen;               // 0x18
    WORD   hLBrush;             // 0x1A
    WORD   hLFont;              // 0x1C
    WORD   hBitmap;             // 0x1E
    WORD   dchPal;              // 0x20
    WORD   hLDevice;            // 0x22
    WORD   hRaoClip;            // 0x24
    WORD   hPDeviceBlock;       // 0x26
    WORD   hPPen;               // 0x28
    WORD   hPBrush;             // 0x2A
    WORD   hPFontTrans;         // 0x2C
    WORD   hPFont;              // 0x2E
    LPVOID lpPDevice;           // 0x30
    WORD   pLDevice;            // 0x34
    WORD   pRaoClip;            // 0x36
    WORD   pPDeviceBlock;       // 0x38
    LPVOID lpPPen;              // 0x3A
    LPVOID lpPBrush;            // 0x3E
    WORD   pPFontTrans;         // 0x42
    LPVOID lpPFont;             // 0x44
    WORD   nPFTIndex;           // 0x48
    LPVOID Transform;           // 0x4A
    WORD   wRop2;               // 0x4E
    WORD   wBkMode;             // 0x50
    DWORD  dwBkColor;           // 0x52
    DWORD  dwTextColor;         // 0x56
    WORD   nTBreakExtra;        // 0x5A
    WORD   nBreakExtra;         // 0x5C
    WORD   wBreakError;         // 0x5E
    WORD   nBreakRem;           // 0x60
    WORD   nBreakCount;         // 0x62
    WORD   nCharExtra;          // 0x64
    DWORD  crLbkColor;          // 0x66
    DWORD  crLTextColor;        // 0x6A
    WORD   wUnknown6E;          // 0x6E
    WORD   wUnknown70;          // 0x70
    WORD   wUnknown72;          // 0x72
    WORD   wUnknown74;          // 0x74
    WORD   wUnknown76;          // 0x76
    WORD   wUnknown78;          // 0x78
    WORD   wUnknown7A;          // 0x7A
    WORD   wUnknown7C;          // 0x7C
    WORD   wUnknown7E;          // 0x7E
    WORD   wUnknown80;          // 0x80
    WORD   wUnknown82;          // 0x82
    WORD   wUnknown84;          // 0x84
    WORD   wUnknown86;          // 0x86
    WORD   wUnknown88;          // 0x88
    WORD   wUnknown8A;          // 0x8A
    WORD   wUnknown8C;          // 0x8C
    WORD   wUnknown8E;          // 0x8E
    WORD   wUnknown90;          // 0x90
    WORD   wUnknown92;          // 0x92
    WORD   wMapMode;            // 0x94
    WORD   wUnknown96;          // 0x96
    WORD   wUnknown98;          // 0x98
    WORD   wUnknown9A;          // 0x9A
    BYTE   byPlane;             // 0x9C
    BYTE   byBitsPix;           // 0x9D
    WORD   wUnknown9E;          // 0x9E
    WORD   wUnknownA0;          // 0xA0
    WORD   wUnknownA2;          // 0xA2
    WORD   wUnknownA4;          // 0xA4
    WORD   wUnknownA6;          // 0xA6
    WORD   wUnknownA8;          // 0xA8
    WORD   wUnknownAA;          // 0xAA
    WORD   wUnknownAC;          // 0xAC
    WORD   wUnknownAE;          // 0xAE
    WORD   wUnknownB0;          // 0xB0
    WORD   wUnknownB2;          // 0xB2
    WORD   wUnknownB4;          // 0xB4
    WORD   wUnknownB6;          // 0xB6
    WORD   wUnknownB8;          // 0xB8
    WORD   wUnknownBA;          // 0xBA
    WORD   wUnknownBC;          // 0xBC
    WORD   wUnknownBE;          // 0xBE
    WORD   wUnknownC0;          // 0xC0
    WORD   wUnknownC2;          // 0xC2
    WORD   wUnknownC4;          // 0xC4
    WORD   wUnknownC6;          // 0xC6
    WORD   wDCOrgX;             // 0xC8
    WORD   wDCOrgY;             // 0xCA
}
DC;

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  Externals                                                              //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

extern APPINFO appInfo;

extern BOOL FAR PASCAL __loadds NewGdiBitBlt( HDC, int, int, int, int, HDC, int, int, DWORD );
extern BOOL FAR PASCAL __loadds NewGdiPatBlt( HDC, int, int, int, int, DWORD );
extern int  FAR PASCAL __loadds NewEscape( HDC, int, int, LPCSTR, LPVOID );

extern DWORD FAR PASCAL __loadds NewDrvControl      ( LPVOID, WORD, LPVOID, LPVOID );
extern BOOL  FAR PASCAL __loadds NewDrvBitBlt       ( LPVOID, WORD, WORD, LPVOID, WORD, WORD, WORD, WORD, long, LPVOID, LPVOID );
extern WORD  FAR PASCAL __loadds NewDrvRealizeObject( LPVOID, WORD, LPVOID, LPVOID, LPVOID );

extern HFILE hFile;
extern OFSTRUCT ofstruct;

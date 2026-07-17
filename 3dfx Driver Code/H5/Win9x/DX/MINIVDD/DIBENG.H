/* -*-c++-*- */
/* $Header: dibeng.h, 2, 10/11/00 8:54:15 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** Portions Copyright (c) 1992 Microsoft Corporation.
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
** File name:   dibeng.c
**
** Description: Dib Engine Interface Definitions.
**
**              The DIB Engine is non-palettized from GDI's perspective.
**              When an app selects a DIB into a memory DC, GDI will
**              create a DIB Engine PDevice (see definition below) and
**              will stuff in a 'DI' in the deType field.  Subsequent
**              operations on this DC will result in calls to the DIB
**              Engine with this PDevice.  Device drivers can also use
**              the DIB Engine to handle most, if not all, of their
**              rendering work.  A device driver exports the DIB Engine
**              PDevice as it's own PDevice to GDI.  This PDevice
**              contains a pointer to a BitmapInfo header in the
**              driver's data segment. Immediately following this is
**              an optional color table for devices less than 16 bpp.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:15 PM$
**
** $History: dibeng.h $
** 
** *****************  Version 3  *****************
** User: Michael      Date: 1/05/99    Time: 7:51a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#ifndef __DIBENG_H__
#define __DIBENG_H__


//;----------------------------------------------------------------------------
//; E Q U A T E S
//;----------------------------------------------------------------------------

#define BRUSHSIZE   8       // height and width in pixels
#define VER_DIBENG  0x400   // version = 4.0
#define TYPE_DIBENG 0x5250  // deType

//;----------------------------------------------------------------------------
//; S T R U C T U R E S
//;----------------------------------------------------------------------------

//;----------------------------------------------------------------------------
//; PDevice Structure for the DIB Engine.  deType will contain 'DI' when GDI
//; calls the DIB Engine to perform graphics operations on the dib.  deType
//; will contain a 0 or a Selector if a mini-driver is calling the DIB Engine
//; to do graphics operations.
//;----------------------------------------------------------------------------

#define deCursorExclude   deBeginAccess
#define deCursorUnexclude deEndAccess

typedef struct
{
    WORD         deType;                // contains TYPE_DIBENG or 0
    WORD         deWidth;               // Width of dib in pixels
    WORD         deHeight;              // Height of dib in pixels
    WORD         deWidthBytes;          // #bytes per scan line
    BYTE         dePlanes;              // # of planes in bitmap
    BYTE         deBitsPixel;           // # of bits per pixel
    DWORD        deReserved1;           // cannot be used.
    DWORD        deDeltaScan;           // + or -. Displacement to next scan.
    LPBYTE       delpPDevice;           // Pointer to associated PDevice
    DWORD        deBitsOffset;          // fword offset to bits of dib
    WORD         deBitsSelector;
    WORD         deFlags;               // additional flags
    WORD         deVersion;             // lsb=minor, msb=major (0400h = 4.0)
    LPBITMAPINFO deBitmapInfo;          // pointer to the bitmapinfo header
    void         (FAR *deBeginAccess)();// Begin surface access call back
    void         (FAR *deEndAccess)();  // End surface access call back
    DWORD        deDriverReserved;      // Reserved for Minidriver use.
}
DIBENGINE, FAR *LPDIBENGINE;

//;----------------------------------------------------------------------------
//; Definitions for DIBEngine.deFlags
//;----------------------------------------------------------------------------

#define MINIDRIVER      0x0001  // display driver
#define PALETTIZED      0x0002  // paletized device
#define SELECTEDDIB     0x0004  // DIB Section
#define OFFSCREEN       0x0008  // offscreen surface (use with VRAM)
#define DISABLED        0x0010  // going away -- please use BUSY instead
#define BUSY            0x0010
#define NOT_FRAMEBUFFER 0x0020  // example: 8514/a
#define FIVE6FIVE       0x0040  // 16 bpp, 565 color format.
#define NON64KBANK      0x0080  // bank size is not 64K
#define VRAM            0x8000  // physical surface (video memory)
#define BANKEDVRAM      0x4000  // VFlatD simulated
#define BANKEDSCAN      0x2000  // VFlatD simulated (broken rasters)
#define PALETTE_XLAT    0x1000  // background palette xlat
#define VGADITHER       0x0800  // dither to VGA colors (first 8, and last 8)
#define CTCHANGE        0x0400  // color table has been changed
#define DITHER256       0x0200  // dither to 256 fixed colors
#define HWCURSOR        0x0100  // driver is using a HWCURSOR right now.
#define FREE2           0x0100  // Free.

#define BUSY_BIT        0x0004  // bit number to test for BUSY

//;----------------------------------------------------------------------------
//; Definitions for DIBEngine.deBeginAccess flags
//;----------------------------------------------------------------------------

#define FB_ACCESS       0x0001
#define CURSOREXCLUDE   0x0008

//;----------------------------------------------------------------------------
//; Definitions for most significant byte of a physical color.
//;----------------------------------------------------------------------------

#define GREY_BIT        0x40    // color is grey (r=g=b)

//;----------------------------------------------------------------------------
//; DIB Engine Color Table entry structure. This structure is used by device
//; drivers that are using DIB Engine services for rendering.  This structure
//; is identical to the RGBQuad structure except for some bit definitions
//; in the 4th byte.
//;----------------------------------------------------------------------------

typedef struct
{
    BYTE dceBlue;
    BYTE dceGreen;
    BYTE dceRed;
    BYTE dceFlags;
}
DIBColorEntry;

//;----------------------------------------------------------------------------
//; Definitions for DIBColorEntry.dceFlags
//;----------------------------------------------------------------------------

#define NONSTATIC       0x80    // Inhibits color matching to this entry.
#define MAPTOWHITE      0x01    // 0=Black, 1=White

//;----------------------------------------------------------------------------
//; DIB Engine Physical Object Definitions
//;----------------------------------------------------------------------------

typedef struct
{
    WORD  dpPenStyle;
    BYTE  dpPenFlags;   // currently none undefined.
    BYTE  dpPenBpp;
    DWORD dpPenMono;
    DWORD dpPenColor;
}
DIB_Pen;

typedef struct
{
    BYTE  dp1BrushFlags;                // Accelerator for solids
    BYTE  dp1BrushBpp;                  // Brush Bits per pixel format
    WORD  dp1BrushStyle;                // Style of the brush
    DWORD dp1FgColor;                   // Physical fg color
    WORD  dp1Hatch;                     // Hatching style
    DWORD dp1BgColor;                   // Physical bg color
    BYTE dp1BrushMono [BRUSHSIZE*4];    // Mono portion
    BYTE dp1BrushMask [BRUSHSIZE*4];    // transparency mask (hatch pattern)
    BYTE dp1BrushBits [BRUSHSIZE*4];    // 8 rows, 8 columns of 1 bit/pixel
}
DIB_Brush1;

typedef struct
{
    BYTE  dp4BrushFlags;                // Accelerator for solids
    BYTE  dp4BrushBpp;                  // Brush Bits per pixel format
    WORD  dp4BrushStyle;                // Style of the brush
    DWORD dp4FgColor;                   // Physical fg color
    WORD  dp4Hatch;                     // Hatching style
    DWORD dp4BgColor;                   // Physical bg color
    BYTE dp4BrushMono [BRUSHSIZE*4];    // Mono portion
    BYTE dp4BrushMask [BRUSHSIZE*4];    // transparency mask (hatch pattern)
    BYTE dp4BrushBits [BRUSHSIZE*4];    // 8 rows, 8 columns of 4 bit/pixel
}
DIB_Brush4;

typedef struct
{
    BYTE  dp8BrushFlags;                // Accelerator for solids
    BYTE  dp8BrushBpp;                  // Brush Bits per pixel format
    WORD  dp8BrushStyle;                // Style of the brush
    DWORD dp8FgColor;                   // Physical fg color
    WORD  dp8Hatch;                     // Hatching style
    DWORD dp8BgColor;                   // Physical bg color
    BYTE dp8BrushMono [BRUSHSIZE*4];    // Mono portion
    BYTE dp8BrushMask [BRUSHSIZE*4];    // transparency mask (hatch pattern)
    BYTE dp8BrushBits [BRUSHSIZE*8];    // 8 rows,8 columns of 8 bit/pixel
}
DIB_Brush8;

typedef struct
{
    BYTE  dp16BrushFlags;               // Accelerator for solids
    BYTE  dp16BrushBpp;                 // Brush Bits per pixel format
    WORD  dp16BrushStyle;               // Style of the brush
    DWORD dp16FgColor;                  // Physical fg color
    WORD  dp16Hatch;                    // Hatching style
    DWORD dp16BgColor;                  // Physical bg color
    BYTE dp16BrushMono [BRUSHSIZE*4];   // Mono portion
    BYTE dp16BrushMask [BRUSHSIZE*4];   // transparency mask (hatch pattern)
    BYTE dp16BrushBits [BRUSHSIZE*16];  // 8 rows,8 columns of 16 bit/pixel
}
DIB_Brush16;

typedef struct
{
    BYTE  dp24BrushFlags;               // Accelerator for solids
    BYTE  dp24BrushBpp;                 // Brush Bits per pixel format
    WORD  dp24BrushStyle;               // Style of the brush
    DWORD dp24FgColor;                  // Physical fg color
    WORD  dp24Hatch;                    // Hatching style
    DWORD dp24BgColor;                  // Physical bg color
    BYTE dp24BrushMono [BRUSHSIZE*4];   // Mono portion
    BYTE dp24BrushMask [BRUSHSIZE*4];   // transparency mask (hatch pattern)
    BYTE dp24BrushBits [BRUSHSIZE*24];  // 8 rows,8 columns of 24 bit/pixel
}
DIB_Brush24;

typedef struct
{
    BYTE  dp32BrushFlags;               // Accelerator for solids
    BYTE  dp32BrushBpp;                 // Brush Bits per pixel format
    WORD  dp32BrushStyle;               // Style of the brush
    DWORD dp32FgColor;                  // Physical fg color
    WORD  dp32Hatch;                    // Hatching style
    DWORD dp32BgColor;                  // Physical bg color
    BYTE dp32BrushMono [BRUSHSIZE*4];   // Mono portion
    BYTE dp32BrushMask [BRUSHSIZE*4];   // transparency mask (hatch pattern)
    BYTE dp32BrushBits [BRUSHSIZE*32];  // 8 rows,8 columns of 32 bit/pixel
}
DIB_Brush32;

//;----------------------------------------------------------------------------
//; Definitions for DIB_Brushxx.dpxxBrushFlags
//;----------------------------------------------------------------------------

#define COLORSOLID      0x01    // Color part is solid.
#define MONOSOLID       0x02    // Mono part is solid.
#define PATTERNMONO     0x04    // Pattern brush came from a mono bitmap.
#define MONOVALID       0x08    // Mono part is valid.
#define MASKVALID       0x10    // Transparency Mask part is valid.
#define PRIVATEDATA     0x20    // Vendor specific bit for Pens, Brushes
#define BRUSH40         0x40    //
#define DIBENGBRUSH     0x80    // the DIBENG created this brush

//;----------------------------------------------------------------------------
//; ColorToMono
//; Entry: red, green, blue
//; Exit:  blue = intensity.
//;----------------------------------------------------------------------------

#if 0
ColorToMono     macro   red, green, blue
        add     blue, red       ; R+B
        rcr     blue, 1         ; (R+B)/2
        add     blue, green     ; pitch in Green
        rcr     blue, 1         ; G/2 + (R+B)/4
endm    ColorToMono
#endif

//;----------------------------------------------------------------------------
//; ColorToMonoBit
//; Entry: red, green, blue
//; Exit:  blue = 0 if color maps to black
//;        blue = 1 if color maps to white
//;----------------------------------------------------------------------------

#if 0
ColorToMonoBit  macro   red, green, blue
        ; Call ColorToMono to derive intensity.
        ColorToMono red, green, blue
        cmp     blue,127
        setnc   blue
endm ColorToMonoBit
#endif


#endif

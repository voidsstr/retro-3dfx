/* -*-c++-*- */
/* $Header: gdidefs.h, 2, 10/11/00 8:54:18 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
** Portions Copyright (C) 1993-95  Microsoft Corporation.
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
** File name:   gdidefs.h
**
** Description: GDI Definitions for Device Drivers.
**              Since most of the routines only need a portion of
**              these definitions, conditional assembly flags have
**              been defined in the various files to only include
**              portions as needed (as opposed to having a lot of
**              include files to mess with).  The flags are as follows:
**
**              incFont         include font definitions
**              incDevice       include device definitions
**              incLogical      include logical object definitions
**              incDrawmode     include DrawMode structure definition
**              incOutput       include Output definitions
**              incControl      include Control definitions
**              incPolyScanline include Scan definition
**
** $Revision: 2$
** $Date: 10/11/00 8:54:18 PM$
**
** $History: gdidefs.h $
** 
** *****************  Version 5  *****************
** User: Adrians      Date: 3/30/99    Time: 3:17p
** Updated in $/devel/h3/Win95/dx/minivdd
** Remove an annoying warning when compiling.
** 
** *****************  Version 4  *****************
** User: Michael      Date: 1/05/99    Time: 8:44a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 3  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#ifndef __GDIDEFS_H__
#define __GDIDEFS_H__

//;----------------------------------------------------------------------------
//; General definitions that almost everyone will use.
//;----------------------------------------------------------------------------

typedef struct tagRECTL
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
}
RECTL;

// Physical Bitmap Structure

// Bitmap data structure passed to OEM routines.  Defines the location and
// size of a main memory bitmap.

typedef struct
{
    SHORT  bmType;          // 0 means main memory bitmap.  Non-zero
                            // is number of physical display and format
                            // of the rest of the structure known only
                            // to device driver
    USHORT bmWidth;         // Width of bitmap in pixels
    USHORT bmHeight;        // Height of bitmap in pixels
    USHORT bmWidthBytes;    // # of bytes per scan line
    BYTE   bmPlanes;        // # of planes in bitmap
    BYTE   bmBitsPixel;     // # of bits per pixel
    LPBYTE bmBits;          // Far pointer to bits of main memory bitmap
    ULONG  bmWidthPlanes;   // Product of bmWidthBytes and bmHeight
    LPBYTE bmlpPDevice;     // Pointer to associated PDevice
    USHORT bmSegmentIndex;  // Index to planes next segment if non-zero
    USHORT bmScanSegment;   // Number of scans per segment
    USHORT bmFillBytes;     // Number of unused bytes per segment
    USHORT futureUse4;
    USHORT futureUse5;
}
BITMAP, FAR* LPBITMAP;

// structures used for Device Independent Bitmap (DIB) processing.
// all taken out of Presentation Manager's documentation

typedef struct
{
    DWORD bcSize;
    WORD  bcWidth;
    WORD  bcHeight;
    WORD  bcPlanes;
    WORD  bcBitCount;
}
BITMAPCOREHEADER, FAR* LPBITMAPCOREHEADER;

// new format bitmap structure based on PM2.0 format DCR.

typedef struct
{
    DWORD biSize;
    DWORD biWidth;
    DWORD biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    DWORD biXPelsPerMeter;
    DWORD biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
}
BITMAPINFOHEADER, FAR* LPBITMAPINFOHEADER;

// triple used in PM1.1 (BitmapCoreInfo) format color table

typedef struct
{
    BYTE rgbtBlue;
    BYTE rgbtGreen;
    BYTE rgbtRed;
}
RGBTRIPLE;

// RGB DWORD used in PM2.0 format color table

typedef struct
{
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
}
RGBQUAD;

// ICM Color Definitions
// The following two structures are used for defining RGB's in terms of
// CIEXYZ. The values are fixed point 16.16.

typedef long FXPT16DOT16, FAR* LPFXPT16DOT16;
typedef long FXPT2DOT30, FAR* LPFXPT2DOT30;

typedef struct tagCIEXYZ
{
    FXPT2DOT30 ciexyzX;
    FXPT2DOT30 ciexyzY;
    FXPT2DOT30 ciexyzZ;
}
CIEXYZ, FAR* LPCIEXYZ;

typedef struct tagICEXYZTRIPLE
{
    CIEXYZ  ciexyzRed;
    CIEXYZ  ciexyzGreen;
    CIEXYZ  ciexyzBlue;
}
CIEXYZTRIPLE, FAR* LPCIEXYZTRIPLE;

typedef struct
{
    BITMAPCOREHEADER bmciHeader;
    RGBTRIPLE        bmciColors[1]; // array of RGBTRIPLES
}
BITMAPCOREINFO, FAR* LPBITMAPCOREINFO;

typedef struct
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];  // array of RGBQUADS
}
BITMAPINFO, FAR* LPBITMAPINFO;

typedef struct
{
    DWORD        bV4Size;
    LONG         bV4Width;
    LONG         bV4Height;
    WORD         bV4Planes;
    WORD         bV4BitCount;
    DWORD        bV4V4Compression;
    DWORD        bV4SizeImage;
    LONG         bV4XPelsPerMeter;
    LONG         bV4YPelsPerMeter;
    DWORD        bV4ClrUsed;
    DWORD        bV4ClrImportant;
    DWORD        bV4RedMask;
    DWORD        bV4GreenMask;
    DWORD        bV4BlueMask;
    DWORD        bV4AlphaMask;
    DWORD        bV4CSType;
    CIEXYZTRIPLE bV4Endpoints;
    DWORD        bV4GammaRed;
    DWORD        bV4GammaGreen;
    DWORD        bV4GammaBlue;
}
BITMAPV4HEADER, FAR* LPBITMAPV4HEADER;

typedef struct
{
    BITMAPV4HEADER bmv4Header;
    RGBQUAD        bmv4Colors[1];
}
BITMAPV4INFO, FAR* LPBITMAPV4INFO;

// currently, if the low byte of biCompression is non zero,
// it must be one of following

#define BI_RGB              0x00
#define BI_RLE8             0x01
#define BI_RLE4             0x02
#define BI_BITFIELDS        0x03

#define BITMAP_SELECTED     0x01
#define BITMAP_64K          0x01

#define DIBSIGNATURE        0x4944

#ifndef NOPTRC

typedef struct
{
    SHORT x;   // x coordinate of point
    SHORT y;   // y coordinate of point
}
PTTYPE, FAR* LPPOINT;

#define POINT   PTTYPE

typedef struct
{
    SHORT left;
    SHORT top;
    SHORT right;
    SHORT bottom;
}
RECT, FAR* LPRECT;

#endif

typedef struct
{
    PTTYPE min;     // x,y starting coord
    PTTYPE ext;     // x,y extents
}
BOXTYPE;


// Logical Object Definitions -- incLogical

#define OBJ_PEN   1
#define OBJ_BRUSH 2
#define OBJ_FONT  3

typedef struct
{
    USHORT lbStyle;     // Style of logical BRUSH
    ULONG  lbColor;     // RGB color
    USHORT lbHatch;     // Hatching style
    ULONG lbBkColor;    // Background color for hatched brush
    ULONG lbhcmXform;   // transform for image color matching
}
LOGBRUSH, FAR* LPLOGBRUSH;

// pointer to physical pattern

#define lbPattern lbColor

// Brush styles defined by GDI

#define  BS_SOLID           0
#define  BS_NULL            1
#define  BS_HOLLOW          1
#define  BS_HATCHED         2
#define  BS_PATTERN         3
#define  BS_INDEXED         4
#define  BS_DIBPATTERN      5
#define  BS_DIBPATTERNPT    6
#define  BS_PATTERN8X8      7
#define  BS_DIBPATTERN8X8   8

#define MaxBrushStyle   3

// Hatched Brush hatching styles defined by GDI

#define HS_HORIZONTAL   0   /* ----- */
#define HS_VERTICAL     1   /* ||||| */
#define HS_FDIAGONAL    2   /* ///// */
#define HS_BDIAGONAL    3   /* \\\\\ */
#define HS_CROSS        4   /* +++++ */
#define HS_DIAGCROSS    5   /* xxxxx */

#define MaxHatchStyle   5

// Logical Pen Structure

typedef struct
{
    USHORT lopnStyle;       // (solid, hollow, dashed..)
    PTTYPE lopnWidth;       // This is really a point type
    ULONG  lopnColor;
    USHORT lopnStyle2;      // end caps and joins
    ULONG  lopnhcmXform;    // transform for image color matching
}
LOGPEN, FAR* LPLOGPEN;

// Line Style definitions for lopnStyle

#define LS_SOLID            0
#define LS_DASHED           1
#define LS_DOTTED           2
#define LS_DOTDASHED        3
#define LS_DASHDOTDOT       4
#define LS_NOLINE           5
#define LS_INSIDEFRAME      6
#define MaxLineStyle        LS_NOLINE

// Line Style definitions for lopnStyle2

#define LS_ENDCAP_FLAT      0x01
#define LS_ENDCAP_ROUND     0x02
#define LS_ENDCAP_SQUARE    0x04
#define LS_JOIN_BEVEL       0x08
#define LS_JOIN_MITER       0x10
#define LS_JOIN_ROUND       0x20


// The size to allocate for the lfFaceName field in the logical font.

#ifndef LF_FACESIZE
#define LF_FACESIZE 32
#endif

// Various constants for defining a logical font.

#define OUT_DEFAULT_PRECIS      0
#define OUT_STRING_PRECIS       1
#define OUT_CHARACTER_PRECIS    2
#define OUT_STROKE_PRECIS       3
#define OUT_TT_PRECIS           4
#define OUT_DEVICE_PRECIS       5
#define OUT_RASTER_PRECIS       6
#define OUT_TT_ONLY_PRECIS      7

#define CLIP_DEFAULT_PRECIS     0
#define CLIP_CHARACTER_PRECIS   1
#define CLIP_STROKE_PRECIS      2
#define CLIP_MASK               0x0F
#define CLIP_LH_ANGLES          0x10
#define CLIP_TT_ALWAYS          0x20
#define CLIP_EMBEDDED           0x80

#define DEFAULT_QUALITY         0
#define DRAFT_QUALITY           1
#define PROOF_QUALITY           2

#define DEFAULT_PITCH           0
#define FIXED_PITCH             1
#define VARIABLE_PITCH          2

#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define MAC_CHARSET             77
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255

// GDI font families.

#define FF_DONTCARE     (0 << 4)    // Don't care or don't know.
#define FF_ROMAN        (1 << 4)    // Variable stroke width, serifed.
                                    // Times Roman, Century Schoolbook, etc.
#define FF_SWISS        (2 << 4)    // Variable stroke width, sans-serifed.
                                    // Helvetica, Swiss, etc.
#define FF_MODERN       (3 << 4)    // Constant stroke width, serifed or sans-serifed.
                                    // Pica, Elite, Courier, etc.
#define FF_SCRIPT       (4 << 4)    // Cursive, etc.
#define FF_DECORATIVE   (5 << 4)    // Old English, etc.

// Font weights lightest to darkest.

#define FW_DONTCARE     0
#define FW_THIN         100
#define FW_EXTRALIGHT   200
#define FW_LIGHT        300
#define FW_NORMAL       400
#define FW_MEDIUM       500
#define FW_SEMIBOLD     600
#define FW_BOLD         700
#define FW_EXTRABOLD    800
#define FW_HEAVY        900

#define FW_ULTRALIGHT   FW_EXTRALIGHT
#define FW_REGULAR      FW_NORMAL
#define FW_DEMIBOLD     FW_SEMIBOLD
#define FW_ULTRABOLD    FW_EXTRABOLD
#define FW_BLACK        FW_HEAVY

// Enumeration font types.

#define RASTER_FONTTYPE 1
#define DEVICE_FONTTYPE 2


typedef struct
{
    SHORT lfHeight;
    SHORT lfWidth;
    SHORT lfEscapement;
    SHORT lfOrientation;
    SHORT lfWeight;
    BYTE  lfItalic;
    BYTE  lfUnderline;
    BYTE  lfStrikeOut;
    BYTE  lfCharSet;
    BYTE  lfOutPrecision;
    BYTE  lfClipPrecision;
    BYTE  lfQuality;
    BYTE  lfPitchAndFamily;
    BYTE  lfFaceName[LF_FACESIZE];  // Variable length field for the face name.
}
LOGFONT, FAR* LPLOGFONT;


// Device Definitions - incDevice

#define EnableDevice    0x0000  // Enable Device
#define InquireInfo     0x0001  // Inquire Device GDI Info
#define InfoContext     0x8000  // Inquire/Enable for info context

// Device Technologies

#define DT_PLOTTER      0   // Vector plotter
#define DT_RASDISPLAY   1   // Raster display
#define DT_RASPRINTER   2   // Raster printer
#define DT_RASCAMERA    3   // Raster camera
#define DT_CHARSTREAM   4   // Character-stream, PLP
#define DT_METAFILE     5   // Metafile, VDM
#define DT_DISPFILE     6   // Display-file
#define DT_JUMBO        11  // SPAG LJ cool thing

// Curve Capabilities

#define CC_NONE             0x0000  // Curves not supported
#define CC_CIRCLES          0x0001  // Can do circles
#define CC_PIE              0x0002  // Can do pie wedges
#define CC_CHORD            0x0004  // Can do chord arcs
#define CC_ELLIPSES         0x0008  // Can do ellipese
#define CC_WIDE             0x0010  // Can do wide lines
#define CC_STYLED           0x0020  // Can do styled lines
#define CC_WIDESTYLED       0x0040  // Can do wide styled lines
#define CC_INTERIORS        0x0080  // Can do interiors
#define CC_ROUNDRECT        0x0100  // Can do round rectangles
#define CC_POLYBEZIER       0x0200  // Can do polybeziers

// Line Capabilities

#define LC_NONE             0x0000  // Lines not supported
#define LC_POLYSCANLINE     0x0001  // Poly Scanlines supported
#define LC_POLYLINE         0x0002  // Can do polylines
#define LC_MARKER           0x0004  // Can do markers
#define LC_POLYMARKER       0x0008  // Can do polymarkers
#define LC_WIDE             0x0010  // Can do wide lines
#define LC_STYLED           0x0020  // Can do styled lines
#define LC_WIDESTYLED       0x0040  // Can do wide styled lines
#define LC_INTERIORS        0x0080  // Can do interiors

// Polygonal Capabilities

#define PC_NONE             0x0000  // Polygonals not supported
#define PC_ALTPOLYGON       0x0001  // Can do even odd polygons
#define PC_POLYGON          0x0001  // old name for ALTPOLYGON
#define PC_RECTANGLE        0x0002  // Can do rectangles
#define PC_WINDPOLYGON      0x0004  // Can do winding polygons
#define PC_TRAPEZOID        0x0004  // old name for WINDPOLYGON
#define PC_SCANLINE         0x0008  // Can do scanlines
#define PC_WIDE             0x0010  // Can do wide borders
#define PC_STYLED           0x0020  // Can do styled borders
#define PC_WIDESTYLED       0x0040  // Can do wide styled borders
#define PC_INTERIORS        0x0080  // Can do interiors
#define PC_POLYPOLYGON      0x0100  // Can do PolyPolygons
#define PC_PATHS            0x0200  // Can do Paths

// Clipping Capabilities

#define CP_NONE             0x0000  // no clipping of Output
#define CP_RECTANGLE        0x0001  // Output clipped to Rects
#define CP_REGION           0x0002  // not supported
#define CP_REGION32         0x0004  // Output clipped to regions

// Text Capabilities

#define TC_OP_CHARACTER     0x0001  // Can do OutputPrecision   CHARACTER
#define TC_OP_STROKE        0x0002  // Can do OutputPrecision   STROKE
#define TC_CP_STROKE        0x0004  // Can do ClipPrecision     STROKE
#define TC_CR_90            0x0008  // Can do CharRotAbility    90
#define TC_CR_ANY           0x0010  // Can do CharRotAbility    ANY
#define TC_SF_X_YINDEP      0x0020  // Can do ScaleFreedom      X_YINDEPENDENT
#define TC_SA_DOUBLE        0x0040  // Can do ScaleAbility      DOUBLE
#define TC_SA_INTEGER       0x0080  // Can do ScaleAbility      INTEGER
#define TC_SA_CONTIN        0x0100  // Can do ScaleAbility      CONTINUOUS
#define TC_EA_DOUBLE        0x0200  // Can do EmboldenAbility   DOUBLE
#define TC_IA_ABLE          0x0400  // Can do ItalisizeAbility  ABLE
#define TC_UA_ABLE          0x0800  // Can do UnderlineAbility  ABLE
#define TC_SO_ABLE          0x1000  // Can do StrikeOutAbility  ABLE
#define TC_RA_ABLE          0x2000  // Can do RasterFontAble    ABLE
#define TC_VA_ABLE          0x4000  // Can do VectorFontAble    ABLE
#define TC_RESERVED         0x8000  // Reserved. Must be returned zero.

// Raster Capabilities

#define RC_NONE             0x0000  // No Raster Capabilities
#define RC_BITBLT           0x0001  // Can do bitblt
#define RC_BANDING          0x0002  // Requires banding support
#define RC_SCALING          0x0004  // does scaling while banding
#define RC_BITMAP64         0x0008  // supports >64k bitmaps
#define RC_GDI20_OUTPUT     0x0010  // has 2.0 output calls
#define RC_GDI20_STATE      0x0020  // dc has a state block
#define RC_SAVEBITMAP       0x0040  // can save bitmaps locally
#define RC_DI_BITMAP        0x0080  // can do DIBs
#define RC_PALETTE          0x0100  // can do color pal management
#define RC_DIBTODEV         0x0200  // can do SetDIBitsToDevice
#define RC_BIGFONT          0x0400  // can do BIGFONTs
#define RC_STRETCHBLT       0x0800  // can do StretchBlt
#define RC_FLOODFILL        0x1000  // can do FloodFill
#define RC_STRETCHDIB       0x2000  // can do StretchDIBits
#define RC_OP_DX_OUTPUT     0x4000  // can do smart ExtTextOut w/dx
#define RC_DEVBITS          0x8000  // supports device bitmaps

// DC Management Flags

#define DC_SPDevice         0x0001  // Seperate PDevice required per device/filename
#define DC_1PDevice         0x0002  // Only 1 PDevice allowed per device/filename
#define DC_IgnoreDFNP       0x0004  // Ignore device/filename pairs when matching

// dpCaps1 capability bits

#define C1_TRANSPARENT      0x0001  // supports transparency
#define TC_TT_ABLE          0x0002  // can do TT through DDI or brute
#define C1_TT_CR_ANY        0x0004  // can do rotated TT fonts
#define C1_EMF_COMPLIANT    0x0008  // Win95 - supports metafile spooling
#define C1_DIBENGINE        0x0010  // DIB Engine compliant driver
#define C1_GAMMA_RAMP       0x0020  // supports gamma ramp setting
#define C1_ICM              0x0040  // does some form of ICM support
#define C1_REINIT_ABLE      0x0080  // Driver supports ReEnable
#define C1_GLYPH_INDEX      0x0100  // Driver supports glyph index fonts
#define C1_BIT_PACKED       0x0200  // Supports bit-packed glyphs
#define C1_BYTE_PACKED      0x0400  // Supports byte-packed glyphs
#define C1_COLORCURSOR      0x0800  // Driver supports color_cursors and async SetCursor
#define C1_CMYK_ABLE        0x1000  // Driver supports CMYK ColorRefs
#define C1_SLOW_CARD        0x2000  // Little or no acceleration (VGA, etc.)

// dpCapsFE capability bits

#define FEC_TT_DBCS         0x0020  // can output DBCS TT fonts correctly
#define FEC_WIFE_ABLE       0x0080  // can handle WIFE font as Engine font


typedef struct
{
    SHORT  dpVersion;       // Version = 0100h for now
    SHORT  dpTechnology;    // Device classification
    SHORT  dpHorzSize;      // Horizontal size in millimeters
    SHORT  dpVertSize;      // Vertical   size in millimeters
    SHORT  dpHorzRes;       // Horizontal width in pixels
    SHORT  dpVertRes;       // Vertical   width in pixels
    SHORT  dpBitsPixel;     // Number of bits per pixel
    SHORT  dpPlanes;        // Number of planes
    SHORT  dpNumBrushes;    // Number of brushes the device has
    SHORT  dpNumPens;       // Number of pens the device has
    SHORT  dpCapsFE;        // capability bits for Far East
    SHORT  dpNumFonts;      // Number of fonts the device has
    SHORT  dpNumColors;     // Number of colors in color table
    SHORT  dpDEVICEsize;    // Size required for the device descriptor
    USHORT dpCurves;        // Curves    capabilities
    USHORT dpLines;         // Line      capabilities
    USHORT dpPolygonals;    // Polygonal capabilities
    USHORT dpText;          // Text      capabilities
    USHORT dpClip;          // Clipping  capabilities
    USHORT dpRaster;        // Bitblt    capabilities
    SHORT  dpAspectX;       // Length of X leg
    SHORT  dpAspectY;       // Length of Y leg
    SHORT  dpAspectXY;      // Length of hypotenuse
    SHORT  dpStyleLen;      // Length of segment for line styles
    PTTYPE dpMLoWin;        // Metric  WinX,WinY Lo res
    PTTYPE dpMLoVpt;        // Metric  VptX,VptY Lo res
    PTTYPE dpMHiWin;        // Metric  WinX,WinY Hi res
    PTTYPE dpMHiVpt;        // Metric  VptX,VptY Hi res
    PTTYPE dpELoWin;        // English WinX,WinY Lo res
    PTTYPE dpELoVpt;        // English VptX,VptY Lo res
    PTTYPE dpEHiWin;        // English WinX,WinY Hi res
    PTTYPE dpEHiVpt;        // English VptX,VptY Hi res
    PTTYPE dpTwpWin;        // Twips   WinX,WinY
    PTTYPE dpTwpVpt;        // Twips   VptX,VptY
    SHORT  dpLogPixelsX;    // Logical pixels/inch in X
    SHORT  dpLogPixelsY;    // Logical pixels/inch in Y
    SHORT  dpDCManage;      // DC Management flags
    USHORT dpCaps1;         // more capability bits
    SHORT  futureuse4;      // dpSpotSizeX
    SHORT  futureuse5;
    SHORT  futureuse6;      // dpSpotSizeY
    SHORT  futureuse7;

    // start of entries in version 3.0 of this structure

    WORD   dpNumPalReg;     // Number of entries in device's palette
    WORD   dpPalReserved;   // Number of reserved entries palette
    WORD   dpColorRes;      // bits of color resolution (total)
}
GDIINFO, FAR* LPGDIINFO;


// Font Definitions -- incFont

// flags for extended textout

#define ETO_GRAYED      0x0001
#define ETO_OPAQUE      0x0002
#define ETO_CLIPPED     0x0004
#define ETO_GLYPH_INDEX 0x0010
#define ETO_RTL         0x0080
#define ETO_BYTE_PACKED 0x0100
#define ETO_BIT_PACKED  0x0200
#define ETO_LEVEL_MODE  0x1000

// This bit in the dfType field signals that the dfBitsOffset field is an
// absolute memory address and should not be altered.

#define PF_BITS_IS_ADDRESS  4

// This bit in the dfType field signals that the font is device realized.

#define PF_DEVICE_REALIZED  0x80

// These bits in the dfType give the fonttype - raster, vector, other1, other2.

#define PF_RASTER_TYPE      0x00
#define PF_VECTOR_TYPE      0x01
#define PF_OTHER1_TYPE      0x02
#define PF_OTHER2_TYPE      0x03
#define PF_GLYPH_INDEX      0x20
#define PF_WIFE_TYPE        0x08

// Glyph types for EngineGetGlyphBmp

#define EGB_BITMAP          1
#define EGB_OUTLINE         2
#define EGB_GRAY2_BITMAP    8
#define EGB_GRAY4_BITMAP    9
#define EGB_GRAY8_BITMAP    10


// The size to allocate for the dfMaps field in the physical font.
#ifndef DF_MAPSIZE
#define DF_MAPSIZE 1
#endif


// Font data structure passed to OEM routines.  Refer to chapters 12 and 13 of
// the OEM adaptation guide for a complete description.

typedef struct
{
    SHORT  dfType;              // Type field for the font.
    SHORT  dfPoints;            // Point size of font.
    SHORT  dfVertRes;           // Vertical digitization.
    SHORT  dfHorizRes;          // Horizontal digitization.
    SHORT  dfAscent;            // Baseline offset from char cell top.
    SHORT  dfInternalLeading;   // Internal leading included in font
    SHORT  dfExternalLeading;   // Prefered extra space between lines
    BYTE   dfItalic;            // Flag specifying if italic.
    BYTE   dfUnderline;         // Flag specifying if underlined.
    BYTE   dfStrikeOut;         // Flag specifying if struck out.
    SHORT  dfWeight;            // Weight of font.
    BYTE   dfCharSet;           // Character set of font.
    SHORT  dfPixWidth;          // Width field for the font.
    SHORT  dfPixHeight;         // Height field for the font.
    BYTE   dfPitchAndFamily;    // Flag specifying variable pitch, family.
    SHORT  dfAvgWidth;          // Average character width.
    SHORT  dfMaxWidth;          // Maximum character width.
    BYTE   dfFirstChar;         // First character in the font.
    BYTE   dfLastChar;          // Last character in the font.
    BYTE   dfDefaultChar;       // Default character for out of range.
    BYTE   dfBreakChar;         // Character to define wordbreaks.
    SHORT  dfWidthBytes;        // Number of bytes in each row.
    ULONG  dfDevice;            // Offset to device name.
    ULONG  dfFace;              // Offset to face name.
    ULONG  dfBitsPointer;       // Bits pointer.
    ULONG  dfBitsOffset;        // Offset to the begining of the bitmap.
                                // On the disk, this is relative to the
                                // begining of the file. In memory this is
                                // relative to the begining of this structure.
    BYTE   dfReservedByte;      // filler byte to WORD-align charoffset
    USHORT dfMaps[DF_MAPSIZE];  // Area for storing the character offsets,
                                // facename, device name (opt), and bitmap.
}
FONTINFO, FAR* LPFONTINFO;

typedef struct
{
    SHORT erType;               // Type field for the font.
    SHORT erPoints;             // Point size of font.
    SHORT erVertRes;            // Vertical digitization.
    SHORT erHorizRes;           // Horizontal digitization.
    SHORT erAscent;             // Baseline offset from char cell top.
    SHORT erInternalLeading;    // Internal leading included in font
    SHORT erExternalLeading;    // Prefered extra space between lines
    BYTE  erItalic;             // Flag specifying if italic.
    BYTE  erUnderline;          // Flag specifying if underlined.
    BYTE  erStrikeOut;          // Flag specifying if struck out.
    SHORT erWeight;             // Weight of font.
    BYTE  erCharSet;            // Character set of font.
    SHORT erPixWidth;           // Width field for the font.
    SHORT erPixHeight;          // Height field for the font.
    BYTE  erPitchAndFamily;     // Flag specifying pitch and family.
    SHORT erAvgWidth;           // Average character width.
    SHORT erMaxWidth;           // Maximum character width.
    BYTE  erFirstChar;          // First character in the font.
    BYTE  erLastChar;           // Last character in the font.
    BYTE  erDefaultChar;        // Default character for out of range.
    BYTE  erBreakChar;          // Character to define wordbreaks.
    SHORT erWidthBytes;         // Number of bytes in each row.
    ULONG erDevice;             // Offset to device name.
    ULONG erFace;               // Offset to face name.
    ULONG erBitsPointer;        // Bits pointer.
    ULONG erBitsOffset;         // Offset to the begining of the bitmap.
    BYTE  erReservedByte;       // flags, and word align the stuff to come
    SHORT erUnderlinePos;       // underline position relative to cell origin
    SHORT erUnderlineThick;     // underline thickness
    SHORT erStrikeoutPos;       // Strikeout position relative to cell origin
    SHORT erStrikeoutThick;     // strikeout thickness
}
SCALABLEFONTINFO, FAR* LPSCALABLEFONTINFO;


//;------------------------------------------------------------------------------
//;--------------------------  4.0 font structures  -----------------------------
//;------------------------------------------------------------------------------
//; This font format has a simple font header. It contains a version field
//; followed by a glyph format field, the total number of glyphs in the
//; entire font, and the dword offset to the array of glyph offsets.
//;------------------------------------------------------------------------------

//;------------------------------------------------------------------------------
//; The NF_LARGE flag indicates that the glyph offset table is
//;  array of DWORDs instead of WORD offsets AND that the glyphs
//;  have LARGExxxGLYPH headers instead of SMALLxxxGLYPH headers.
//;------------------------------------------------------------------------------

#define NF_BYTE_PACKED  0x0001  // Byte aligned row format
#define NF_BIT_PACKED   0x0002  // Bit packed row format
#define NF_FROM_BMP     0x0040  // Derived from bitmap font
#define NF_LARGE        0x0080  // Large offsets and glyph headers

typedef struct
{
    WORD  nfVersion;        // Font version (0x0400)
    WORD  nfFormat;         // Font format flags
    WORD  nfNumGlyphs;      // Number of glyphs in font
    DWORD nfGlyphOffset;    // Offset to glyph offset table
    DWORD nfAWTable;        // Offset to advance width table
    WORD  nfHeight;         // Height of font in pixels
    WORD  nfAscent;         // Distance from baseline to top
    DWORD nfUniqueID;       // Unique identifier for this PFont
}
NewFontSeg;


//;----------------------  Byte-packed glyph structures  ------------------------
//;
//; This structure is the header for a glyph stored in byte-packed
//; row-oriented format. The glyph bitmap is stored immediately after
//; the ROWGLYPH structure.
//;------------------------------------------------------------------------------

typedef struct
{
    BYTE srgOrgX;       // Vector to glyph origin in bitmap
    BYTE srgOrgY;       //  from upper left corner
    BYTE srgWidth;      // Width and height of glyph bmp in
    BYTE srgHeight;     //  pixels
}
SMALLROWGLYPH;

typedef struct
{
    WORD lrgOrgX;       // Vector to glyph origin in bitmap
    WORD lrgOrgY;       //  from upper left corner
    WORD lrgWidth;      // Width and height of glyph bmp in
    WORD lrgHeight;     //  pixels
}
LARGEROWGLYPH;

//;----------------------  Bit-packed glyph structures  -------------------------
//;
//; This structure is the header for a glyph stored in bit-packed row-oriented
//; format. The glyph bitmap is stored immediately after the BITGLYPH structure.
//;
//; Bit-packed format means that the glyph rows are packed together end-to-end.
//; There is no padding out to byte boundaries at the end of a row. This
//; format results in the highest performance on some accelerated cards and
//; the smallest memory footprint.
//;------------------------------------------------------------------------------

typedef struct
{
    BYTE sbgOrgX;       // Vector to glyph origin in bitmap
    BYTE sbgOrgY;       //  from upper left corner
    BYTE sbgWidth;      // Width and height of glyph bmp in
    BYTE sbgHeight;     //  pixels
    WORD sbgPixels;     // Total # of pixels in the glyph
}
SMALLBITGLYPH;

typedef struct
{
    WORD  lbgOrgX;      // Vector to glyph origin in bitmap
    WORD  lbgOrgY;      //  from upper left corner
    WORD  lbgWidth;     // Width and height of glyph bmp in
    WORD  lbgHeight;    //  pixels
    DWORD lbgPixels;    // Total # of pixels in the glyph
}
LARGEBITGLYPH;


typedef struct
{
    SHORT  ftHeight;
    SHORT  ftWidth;
    SHORT  ftEscapement;
    SHORT  ftOrientation;
    SHORT  ftWeight;
    BYTE   ftItalic;
    BYTE   ftUnderline;
    BYTE   ftStrikeOut;
    BYTE   ftOutPrecision;
    BYTE   ftClipPrecision;
    USHORT ftAccelerator;
    SHORT  ftOverhang;
}
TEXTXFORM, FAR* LPTEXTXFORM;

typedef struct
{
    SHORT tmHeight;             // Ascent+Descent
    SHORT tmAscent;             // Pixels above the baseline
    SHORT tmDescent;            // Pixels below the baseline
    SHORT tmInternalLeading;    // Internal leading included in font
    SHORT tmExternalLeading;    // Prefered extra space between lines
    SHORT tmAveCharWidth;       // Of the letter 'X'
    SHORT tmMaxCharWidth;
    SHORT tmWeight;
    BYTE  tmItalic;
    BYTE  tmUnderlined;
    BYTE  tmStruckOut;
    BYTE  tmFirstChar;
    BYTE  tmLastChar;
    BYTE  tmDefaultChar;        // dfDefaultChar+dfFirstChar
    BYTE  tmBreakChar;          // dfBreakChar+dfFirstChar
    BYTE  tmPitchAndFamily;     // Low bit zero if fixed pitch, one if
                                // variable. Family in high nibble.
    BYTE  tmCharSet;
    SHORT tmOverhang;
    SHORT tmDigitizedAspectX;   // Digitization aspect ratio
    SHORT tmDigitizedAspectY;   // in X and Y.
}
TEXTMETRIC, FAR* LPTEXTMETRIC;


// Drawing mode definitions - incDrawMode


typedef struct
{
    SHORT Rop2;             // The 16-bit encoded Logical op
    SHORT bkMode;           // Background Mode (for text only)
    ULONG bkColor;          // Physical background Color
    ULONG TextColor;        // Physical text (forground) color
    SHORT TBreakExtra;      // total pixles to stuff into a line
    SHORT BreakExtra;       // div(TBreakExtra, BreakCount)
    SHORT BreakErr;         // running error term
    SHORT BreakRem;         // mod(TBreakExtra, BreakCount)
    SHORT BreakCount;       // count of breaks in the line
    SHORT CharExtra;        // extra pixles to stuff after each char
                            // (used to space out a font)
    ULONG LbkColor;         // Logical background color
    ULONG LTextColor;       // Logical Text (forground) color
    DWORD ICMCXform;        // Transform for DIC image color matcher
    SHORT StretchBltMode;   // Stretch Blt mode
    DWORD eMiterLimit;      // Miter Limit (single precision IEEE float)
}
DRAWMODE, FAR* LPDRAWMODE;


// Background Mode definitions

#define TRANSPARENT         1
#define OPAQUE              2

#define BKMODE_TRANSPARENT  1
#define BKMODE_OPAQUE       2
#define BKMODE_LEVEL1       3
#define BKMODE_LEVEL2       4
#define BKMODE_LEVEL3       5
#define BKMODE_TRANSLATE    6

// StretchBlt Mode definitions

#define STRETCH_ANDSCANS    1
#define STRETCH_ORSCANS     2
#define STRETCH_DELETESCANS 3
#define STRETCH_HALFTONE    4

#define SBM_BLACKONWHITE    STRETCH_ANDSCANS
#define SBM_WHITEONBLACK    STRETCH_ORSCANS
#define SBM_COLORONCOLOR    STRETCH_DELETESCANS
#define SBM_HALFTONE        STRETCH_HALFTONE


// The SCAN structure used by OS_POLYSCANLINE and DRAWESCAPE.
// The DRAWESCAPE structure is used by the DrawEcape DDI.

// incPolyScanline

typedef struct
{
    SHORT scnPntCnt;    // Count of X coords (not pairs)
    SHORT scnPntTop;    // Top of scan (inclusive)
    SHORT scnPntBottom; // Bottom of scan (exclusive)
    SHORT scnPntX[2];   // Var. length array of X pairs
    SHORT scnPntCntToo; // =scnPntCnt
}
SCAN, FAR* LPSCAN;

typedef struct
{
    DWORD  cbSize;      // sizeof(DRAWESCAPE)
    LPVOID lpDestDev;   // PDEVICE or PBITMAP
    DWORD  nEscape;     // API parameter
    DWORD  cbInput;     // API parameter
    LPVOID lpInput;     // API parameter
    POINT  ptOrigin;    // origin of window
    DWORD  dwUniq;      // region uniqueness value
    RECT   rcBBox;      // bounding box of region
    DWORD  cScans;      // count of scans in region
    LPSCAN lpScan;      // ptr to first scan in region
}
DRAWESCAPE, FAR* LPDRAWESCAPE;

typedef struct
{
    WORD   id;          // if LPREGION 0x8000 else LPRECT
    WORD   cbSize;      // sizeof(REGION)
    LPRECT lprcClip;    // NULL or region walking opt.
    DWORD  dwUniq;      // region uniqueness value
    RECT   rcBBox;      // bounding box of region
    DWORD  cScans;      // count of scans in region
    LPSCAN lpScan;      // ptr to first scan in region
}
REGION, FAR* LPREGION;


// Output Definitions - incOutput


// Output Style definitions used by GDI

#define OS_POLYBEZIER   1
#define OS_ARC          3
#define OS_SCANLINES    4
#define OS_POLYSCANLINE 5
#define OS_RECTANGLE    6
#define OS_ELLIPSE      7
#define OS_MARKER       8
#define OS_POLYLINE     18
#define OS_TRAPEZOID    20  // should really be WINDPOLYGON
#define OS_WINDPOLYGON  OS_TRAPEZOID
#define OS_POLYGON      22  // should really be ALTPOLYGON
#define OS_ALTPOLYGON   OS_POLYGON
#define OS_PIE          23
#define OS_POLYMARKER   24
#define OS_CHORD        39
#define OS_CIRCLE       55

#define OS_POLYPOLYGON  0x4000  // this bit is ORd with OS_WIND/ALTPOLYGON

#define OS_BEGINNSCAN   80
#define OS_ENDNSCAN     81


// incControl

// GDI escape constants

#define OEM_FAILED              0x80000000L

#define NEWFRAME                1
#define ABORTDOC                2
#define NEXTBAND                3
#define SETCOLORTABLE           4
#define GETCOLORTABLE           5
#define FLUSHOUTPUT             6
#define DRAFTMODE               7
#define QUERYESCSUPPORT         8
#define SETPRINTERDC            9   // DDK - between GDI and Driver
#define SETABORTPROC            9   // SDK - between APP and GDI
#define STARTDOC                10
#define ENDDOC                  11
#define GETPHYSPAGESIZE         12
#define GETPRINTINGOFFSET       13
#define GETSCALINGFACTOR        14
#define MFCOMMENT               15
#define GETPENWIDTH             16
#define SETCOPYCOUNT            17
#define SELECTPAPERSOURCE       18
#define DEVICEDATA              19
#define PASSTHROUGH             19
#define GETTECHNOLGY            20
#define GETTECHNOLOGY           20
#define SETLINECAP              21
#define SETLINEJOIN             22
#define SETMITERLIMIT           23
#define BANDINFO                24
#define DRAWPATTERNRECT         25
#define GETVECTORPENSIZE        26
#define GETVECTORBRUSHSIZE      27
#define ENABLEDUPLEX            28
#define GETSETPAPERBINS         29
#define GETSETPRINTORIENT       30
#define ENUMPAPERBINS           31
#define SETDIBSCALING           32
#define EPSPRINTING             33
#define ENUMPAPERMETRICS        34
#define GETSETPAPERMETRICS      35
#define GETVERSION              36  // ADOBE_11_1_88
#define POSTSCRIPT_DATA         37
#define POSTSCRIPT_IGNORE       38
#define MOUSETRAILS             39
#define QUERYROPSUPPORT         40
#define GETDEVICEUNITS          42
#define RESETDEVICE             128
#define GETEXTENDEDTEXTMETRICS  256
#define GETEXTENTTABLE          257
#define GETPAIRKERNTABLE        258
#define GETTRACKKERNTABLE       259
#define EXTTEXTOUT              512
#define GETFACENAME             513
#define DOWNLOADFACE            514 // ADOBE_11_1_88
#define ENABLERELATIVEWIDTHS    768
#define ENABLEPAIRKERNING       769
#define SETKERNTRACK            770
#define SETALLJUSTVALUES        771
#define SETCHARSET              772
#define STRETCHBLT              2048
#define QUERYDIBSUPPORT         3073
#define QDI_SETDIBITS           0x0001
#define QDI_GETDIBITS           0x0002
#define QDI_DIBTOSCREEN         0x0004
#define QDI_STRETCHDIB          0x0008
#define DCICOMMAND              3075
#define BEGIN_PATH              4096
#define CLIP_TO_PATH            4097
#define END_PATH                4098
#define EXT_DEVICE_CAPS         4099
#define RESTORE_CTM             4100
#define SAVE_CTM                4101
#define SET_ARC_DIRECTION       4102
#define SET_BACKGROUND_COLOR    4103
#define SET_POLY_MODE           4104
#define SET_SCREEN_ANGLE        4105
#define SET_SPREAD              4106
#define TRANSFORM_CTM           4107
#define SET_CLIP_BOX            4108
#define SET_BOUNDS              4109
#define OPENCHANNEL             4110
#define DOWNLOADHEADER          4111
#define CLOSECHANNEL            4112
#define SETGDIXFORM             4113
#define RESETPAGE               4114
#define POSTSCRIPT_PASSTHROUGH  4115
#define ENCAPSULATED_POSTSCRIPT 4116


/* Binary raster ops */
#define R2_BLACK            1
#define R2_NOTMERGEPEN      2
#define R2_MASKNOTPEN       3
#define R2_NOTCOPYPEN       4
#define R2_MASKPENNOT       5
#define R2_NOT              6
#define R2_XORPEN           7
#define R2_NOTMASKPEN       8
#define R2_MASKPEN          9
#define R2_NOTXORPEN        10
#define R2_NOP              11
#define R2_MERGENOTPEN      12
#define R2_COPYPEN          13
#define R2_MERGEPENNOT      14
#define R2_MERGEPEN         15
#define R2_WHITE            16

/* Ternary raster operations */
#define SRCCOPY             0x00CC0020L
#define SRCPAINT            0x00EE0086L
#define SRCAND              0x008800C6L
#define SRCINVERT           0x00660046L
#define SRCERASE            0x00440328L
#define NOTSRCCOPY          0x00330008L
#define NOTSRCERASE         0x001100A6L
#define MERGECOPY           0x00C000CAL
#define MERGEPAINT          0x00BB0226L
#define PATCOPY             0x00F00021L
#define PATOR               0x00FA0089L
#define PATPAINT            0x00FB0A09L
#define PATINVERT           0x005A0049L
#define DSTINVERT           0x00550009L
#define BLACKNESS           0x00000042L
#define WHITENESS           0x00FF0062L
#define SRCSTENCIL          0x00B8074AL
#define SRCINVSTENCIL       0x00E20746L
typedef FARPROC FAR* LPFARPROC;


#endif

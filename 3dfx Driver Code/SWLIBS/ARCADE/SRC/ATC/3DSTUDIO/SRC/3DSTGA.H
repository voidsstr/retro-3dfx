#ifndef TGA_H__
#define TGA_H__

/*
 * The following copyright is included from the source code which this
 * code is based:
 */

/* tgatoppm.c - read a TrueVision Targa file and write a portable pixmap
**
** Partially based on tga2rast, version 1.0, by Ian MacPhedran.
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#ifdef __cplusplus
extern "C" {
#endif

/* tga.h - header file for Targa files
*/
 
/* Header definition. */
struct ImageHeader {
    unsigned char IDLength;		/* length of Identifier String */
    unsigned char CoMapType;		/* 0 = no map */
    unsigned char ImgType;		/* image type (see below for values) */
    unsigned char Index_lo, Index_hi;	/* index of first color map entry */
    unsigned char Length_lo, Length_hi;	/* number of entries in color map */
    unsigned char CoSize;		/* size of color map entry (15,16,24,32) */
    unsigned char X_org_lo, X_org_hi;	/* x origin of image */
    unsigned char Y_org_lo, Y_org_hi;	/* y origin of image */
    unsigned char Width_lo, Width_hi;	/* width of image */
    unsigned char Height_lo, Height_hi;	/* height of image */
    unsigned char PixelSize;		/* pixel size (8,16,24,32) */
    unsigned char AttBits;		/* 4 bits, number of attribute bits per pixel */
    unsigned char Rsrvd;		/* 1 bit, reserved */
    unsigned char OrgBit;		/* 1 bit, origin: 0=lower left, 1=upper left */
    unsigned char IntrLve;		/* 2 bits, interleaving flag */
    };

typedef char ImageIDField[256];

/* Definitions for image types. */
#define TGA_Null 0
#define TGA_Map 1
#define TGA_RGB 2
#define TGA_Mono 3
#define TGA_RLEMap 9
#define TGA_RLERGB 10
#define TGA_RLEMono 11
#define TGA_CompMap 32
#define TGA_CompMap4 33

/* Definitions for interleave flag. */
#define TGA_IL_None 0
#define TGA_IL_Two 1
#define TGA_IL_Four 2

typedef struct
{
  unsigned char a, b, g, r;
} TgaPixel;

typedef struct
{
  int width, height;
  TgaPixel *data;
} TgaImage;

int ReadTga( const char *filename, TgaImage *tgaimage_ );

#ifdef __cplusplus
}
#endif


#endif

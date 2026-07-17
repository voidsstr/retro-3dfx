/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
**
** $Revision: 4$ 
** $Date: 10/11/00 7:30:12 PM$ 
**
*/

#include <atscene.h>
#define  _WIN32_LEAN_AND_MEAN_
#include <windows.h>
#include <atinput.h>
#include <atdemo.h>
#include <glide.h>
#include <texus.h>
#include "view.h"
#include "resource.h"

void txYABtoPal256(long *palette, const long* yabTable);

/* 
 * Pn_8 = convert n bits (n <= 6) to 8 bits by replicating the msb's of input
 * into the lsb's of the output.
 */
static  FxU8    P1_8[] = {0x00,0xff};
static  FxU8    P2_8[] = {0x00,0x55,0xaa,0xff};
static  FxU8    P3_8[] = {0x00,0x24,0x49,0x6d,0x92,0xb6,0xdb,0xff};
static  FxU8    P4_8[] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
                                          0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
static  FxU8    P5_8[] = {0x00,0x08,0x10,0x18,0x21,0x29,0x31,0x39,
                                          0x42,0x4a,0x52,0x5a,0x63,0x6b,0x73,0x7b,
                                          0x84,0x8c,0x94,0x9c,0xa5,0xad,0xb5,0xbd,
                                          0xc6,0xce,0xd6,0xde,0xe7,0xef,0xf7,0xff};
static  FxU8    P6_8[] = {0x00,0x04,0x08,0x0c,0x10,0x14,0x18,0x1c,
                                          0x20,0x24,0x28,0x2c,0x30,0x34,0x38,0x3c,
                                          0x41,0x45,0x49,0x4d,0x51,0x55,0x59,0x5d,
                                          0x61,0x65,0x69,0x6d,0x71,0x75,0x79,0x7d,
                                          0x82,0x86,0x8a,0x8e,0x92,0x96,0x9a,0x9e,
                                          0xa2,0xa6,0xaa,0xae,0xb2,0xb6,0xba,0xbe,
                                          0xc3,0xc7,0xcb,0xcf,0xd3,0xd7,0xdb,0xdf,
                                          0xe3,0xe7,0xeb,0xef,0xf3,0xf7,0xfb,0xff};

/* convert 8 bit formats */

static void
YIQ422ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;
    FxU32     pal[256];

    txYABtoPal256((long *)pal, (long *)table);

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU8 r, g, b;
            FxU32 pixel;
    
            pixel = pal[*src++];

            r = (FxU8)(( pixel >> 16 ) & 0xff);
            g = (FxU8)(( pixel >> 8 ) & 0xff);
            b = (FxU8)(pixel & 0xff);

            *dstBits++ = b;
            *dstBits++ = g;
            *dstBits++ = r;
        }
    }
}

static void
P8ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;
    FxU32     *pal = (FxU32 *)table;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU8 r, g, b;
            FxU32 pixel;
    
            pixel = pal[*src++];

            r = (FxU8)(( pixel >> 16 ) & 0xff);
            g = (FxU8)(( pixel >> 8 ) & 0xff);
            b = (FxU8)(pixel & 0xff);

            *dstBits++ = b;
            *dstBits++ = g;
            *dstBits++ = r;
        }
    }
}

static void
RGB332ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU8 r, g, b;
            r = P3_8[(*src>>5)&7];
            g = P3_8[(*src>>2)&7];
            b = P2_8[(*src   )&3];
            src++;
            *dstBits++ = b;
            *dstBits++ = g;
            *dstBits++ = r;
        }
    }
}

static void
A8ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU8 r, g, b;
            r = 0xff;
            g = 0xff;
            b = 0xff;
            *dstBits++ = b;
            *dstBits++ = g;
            *dstBits++ = r;
        }
    }
}

static void
I8ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU8 i = *src++;

            *dstBits++ = i;
            *dstBits++ = i;
            *dstBits++ = i;
        }
    }
}

static void
AI44ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU8 i = P4_8[(*src++ & 0x0F)];

            *dstBits++ = i;
            *dstBits++ = i;
            *dstBits++ = i;
        }
    }
}

/* convert 16 bit formats */

static void
ARGB8332ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU16 c8332;
            FxU8 a, r, g, b;

            c8332 = *(FxU16 *)src; src += 2;

            a = (c8332 >>  8);      
            r = P3_8[(c8332 >>  5) & 0x7];
            g = P3_8[(c8332 >>  2) & 0x7];
            b = P2_8[(c8332      ) & 0x3];

            *dstBits++ = b;
            *dstBits++ = g;
            *dstBits++ = r;
        }
    }
}

static void
RGB565ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU16 c565;
            FxU8 a, r, g, b;

            c565 = *(FxU16 *)src; src += 2;
    
            a = 0xFF;
            r = P5_8[(c565 >> 11)       ];
            g = P6_8[(c565 >>  5) & 0x3f];
            b = P5_8[(c565      ) & 0x1f];

            *dstBits++ = b;
            *dstBits++ = g;
            *dstBits++ = r;
        }
    }
}

static void
ARGB1555ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU16 c1555;
            FxU8 a, r, g, b;

            c1555 = *(FxU16 *)src; src += 2;

            a = P1_8[(c1555 >> 15)       ];
            r = P5_8[(c1555 >> 10) & 0x1f];
            g = P5_8[(c1555 >>  5) & 0x1f];
            b = P5_8[(c1555      ) & 0x1f];
    
            *dstBits++ = b;
            *dstBits++ = g;
            *dstBits++ = r;
        }
    }
}

static void
ARGB4444ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU16 c4444;
            FxU8 a, r, g, b;

            c4444 = *(FxU16 *)src; src += 2;

            a = P4_8[(c4444 >> 12) & 0xf];
            r = P4_8[(c4444 >>  8) & 0xf];
            g = P4_8[(c4444 >>  4) & 0xf];
            b = P4_8[(c4444      ) & 0xf];

            *dstBits++ = b;
            *dstBits++ = g;
            *dstBits++ = r;
        }
    }
}

static void
AI88ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU16 c88;
            FxU8 a, r, g, b;

            c88 = *(FxU16 *)src; src += 2;

            a = c88 >> 8;
            r = g = b = (c88 & 0xff);

            *dstBits++ = b;
            *dstBits++ = g;
            *dstBits++ = r;
        }
    }
}

static void
AYIQ8422ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;
    FxU32     pal[256];

    txYABtoPal256((long *)pal, (long *)table);

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU8 r, g, b;
            FxU32 pixel;
            FxU16 c88 ;

            c88 = *(FxU16 *)src;
            src += 2;
            
            pixel = pal[c88&0xff];

            r = (FxU8)(( pixel >> 16 ) & 0xff);
            g = (FxU8)(( pixel >> 8 ) & 0xff);
            b = (FxU8)(pixel & 0xff);

            *dstBits++ = b;
            *dstBits++ = g;
            *dstBits++ = r;
        }
    }
}

static void
AP88ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;
    FxU32     *pal = (FxU32 *)table;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU8 r, g, b;
            FxU32 pixel;
            FxU16 c88 ;
    
            c88 = *(FxU16 *)src;
            src += 2;
            
            pixel = pal[c88&0xff];

            r = (FxU8)(( pixel >> 16 ) & 0xff);
            g = (FxU8)(( pixel >> 8 ) & 0xff);
            b = (FxU8)(pixel & 0xff);

            *dstBits++ = b;
            *dstBits++ = g;
            *dstBits++ = r;
        }
    }
}

/* convert 24 bit formats */

static void
RGB888ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;
    FxU32     *pal = (FxU32 *)table;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            *dstBits++ = *src++;
            *dstBits++ = *src++;
            *dstBits++ = *src++;
        }
    }
}

/* convert 32 bit formats */

static void
ARGB8888ToDIB(FxU8 *dst, FxU8 *src, FxU32 mipWidth, FxU32 mipHeight, 
         FxU32 dstStride, AtrImgTable *table) {
    FxU32      x, y;
    FxU8      *dstBits, *pDstLine;

    pDstLine = dst;

    for( y = 0; y < mipHeight; y++ ) {
        dstBits = pDstLine;
        pDstLine += dstStride*3;
        for( x = 0; x < mipWidth; x++ ) {
            FxU8 r, g, b;
            b = *src++;
            g = *src++;
            r = *src++;
            src++;
            *dstBits++ = b;
            *dstBits++ = g;
            *dstBits++ = r;
        }
    }
}

/* Walk through all mipmap levels, and convert to DIB format */

HBITMAP
imageToDIB(HWND hWnd, AtrImg *img) {
    HDC        hdc;
    HBITMAP     hBmp;
    FxU32 mipWidth, mipHeight, dstStride;
    BITMAPINFO bmi;
    char       *pBits, *pBase;
    FxU8      *srcBits;
    FxU32      level;
    void (* cnv)(FxU8 *, FxU8 *, FxU32, FxU32, FxU32, AtrImgTable *);
    int bpp;
    
    switch(img->format) {
    case GR_TEXFMT_RGB_332:         
        cnv = RGB332ToDIB;         
        bpp = 1;
        break;
    case GR_TEXFMT_YIQ_422:         
        cnv = YIQ422ToDIB;
        bpp = 1;
        break;
    case GR_TEXFMT_A_8:                     
        cnv = A8ToDIB;             
        bpp = 1;
        break;
    case GR_TEXFMT_I_8:                     
        cnv = I8ToDIB;             
        bpp = 1;
        break;
    case GR_TEXFMT_AI_44:           
        cnv = AI44ToDIB;           
        bpp = 1;
        break;
    case GR_TEXFMT_P_8:                     
        cnv = P8ToDIB;
        bpp = 1;
        break;
    case GR_TEXFMT_ARGB_8332:       
        cnv = ARGB8332ToDIB;       
        bpp = 2;
        break;
    case GR_TEXFMT_AYIQ_8422:       
        cnv = AYIQ8422ToDIB;
        bpp = 2;
        break;
    case GR_TEXFMT_RGB_565:         
        cnv = RGB565ToDIB; 
        bpp = 2;
        break;
    case GR_TEXFMT_ARGB_1555:       
        cnv = ARGB1555ToDIB;       
        bpp = 2;
        break;
    case GR_TEXFMT_ARGB_4444:       
        cnv = ARGB4444ToDIB;       
        bpp = 2;
        break;
    case GR_TEXFMT_AI_88:           
        cnv = AI88ToDIB;           
        bpp = 2;
        break;
    case GR_TEXFMT_AP_88:           
        cnv = AP88ToDIB;
        bpp = 2;
        break;
    case GR_TEXFMT_RGB_888:         
        cnv = RGB888ToDIB;       
        bpp = 3;
        break;
    case GR_TEXFMT_ARGB_8888:       
        cnv = ARGB8888ToDIB;       
        bpp = 4;
        break;
    default: 
       atuError(FXFALSE, "unknow image format %d\n");
       return NULL;
    }

    mipWidth = img->width;
    dstStride = 0;
    for ( level = 0; level < img->nLevels; level++ ) {
        dstStride += mipWidth;
        if ( mipWidth > 1 )
            mipWidth >>= 1;
    }
    dstStride = (dstStride+3)&~0x3;

    hdc = GetDC( hWnd );

    bmi.bmiHeader.biSize        = sizeof( bmi.bmiHeader );
    bmi.bmiHeader.biWidth       = dstStride;
    bmi.bmiHeader.biHeight      = -(int)img->height; // Top Down Bitmap
    bmi.bmiHeader.biPlanes      = 1; // MS Says
    bmi.bmiHeader.biBitCount    = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = img->width*img->height*3;
    bmi.bmiHeader.biXPelsPerMeter = 0; // ?       
    bmi.bmiHeader.biYPelsPerMeter = 0; // ?       
    bmi.bmiHeader.biClrUsed       = 0; // Means use max color
    bmi.bmiHeader.biClrImportant  = 0; // All colors are important

    hBmp = CreateDIBSection( hdc,
                                 &bmi,
                                 DIB_RGB_COLORS,
                                 &pBits,
                                 0,
                                 0 );
    ReleaseDC( hWnd, hdc );

    if ( !hBmp ) {
        MessageBox( 0, "Failed to create bitmap.\n", "Error", 0 );
    }
    
    // Fill in Bitmap Bits
    srcBits = img->data;
    mipWidth = img->width;
    mipHeight = img->height;
    pBase = pBits;
    
    for ( level = 0; level < img->nLevels; level++ ) {
        cnv(pBase, srcBits, mipWidth, mipHeight, dstStride, img->table);
        pBase += mipWidth*3;
        srcBits += bpp*mipWidth*mipHeight;
        if ( mipWidth > 1 )
            mipWidth >>= 1;
        if ( mipHeight > 1 )
            mipHeight >>= 1;
    }

    return hBmp;
}

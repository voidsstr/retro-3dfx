/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** File name: tcutils.h
**
** Description: Texture compression functions.
**
**
*/

// Primary color components (use DirextX byte ordering)
#undef  RED
#define RED   0
#undef  GRN
#define GRN   1
#undef  BLU
#define BLU   2
#undef  ALFA
#define ALFA 3

#define NUM_PRIMARIES   3                                               
#define NUM_COMPONENTS  4
//
// Quantization constants for RGB565
//
#define PRIMARY_BITS    8

#define RED_BITS        5
#define GRN_BITS        6
#define BLU_BITS        5

#define RED_SHIFT       (PRIMARY_BITS-RED_BITS)
#define GRN_SHIFT       (PRIMARY_BITS-GRN_BITS)
#define BLU_SHIFT       (PRIMARY_BITS-BLU_BITS)

//-------------------------------------------------------------------------
// S3 compressed texture formats
//-------------------------------------------------------------------------
// number of pixels in block
#define DXT_BLOCK_PIXELS   16


typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

typedef struct  
{
   BYTE        rgba[4];
} DXT_COLOR;


typedef WORD   RGB565;     // packed color
typedef DWORD  PIXBM;      // 2 BPP bitmap


typedef struct  
{
   RGB565      rgb0;       // color for index 0
   RGB565      rgb1;       // color for index 1
   PIXBM       pixbm;      // pixel bitmap
} DXTBlockRGB;

typedef struct  
{
   WORD        alphabm[4]; // alpha bitmap at 4 BPP
   DXTBlockRGB rgb;        // color block
} DXTBlockAlpha4;

typedef struct  
{
   BYTE        alpha0;     // alpha for index 0
   BYTE        alpha1;     // alpha for index 1
   BYTE        alphabm[6]; // alpha bitmap at 3 BPP
   DXTBlockRGB rgb;        // color block
} DXTBlockAlpha3;



#ifdef __cplusplus
extern "C" {
#endif

void DecodeBlockRGB (DXTBlockRGB *pblockSrc, DXT_COLOR colorDst[DXT_BLOCK_PIXELS]);
void EncodeBlockRGB(DXT_COLOR colorSrc[DXT_BLOCK_PIXELS], DXTBlockRGB *pblockDst);
void DecodeBlockAlpha4(DXTBlockAlpha4 *pblockSrc, DXT_COLOR colorDst[DXT_BLOCK_PIXELS]);
void EncodeBlockAlpha4(DXT_COLOR colorSrc[DXT_BLOCK_PIXELS], DXTBlockAlpha4 *pblockDst);
void DecodeBlockAlpha3(DXTBlockAlpha3 *pblockSrc, DXT_COLOR colorDst[DXT_BLOCK_PIXELS]);
void EncodeBlockAlpha3(DXT_COLOR colorSrc[DXT_BLOCK_PIXELS], DXTBlockAlpha3 *pblockDst);

#ifdef __cplusplus
}
#endif


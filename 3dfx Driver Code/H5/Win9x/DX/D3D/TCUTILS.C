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
** File name: tcutils.c
**
** Description: Texture compression functions.
**
**
*/

#include "precomp.h"

#include "tcutils.h"



static int l_abs(int x)
{
   if (x < 0)
      return -x;
   else
      return x;
}

#define ABS l_abs


//-----------------------------------------------------------------------------
// unpack a fixed point color
//-----------------------------------------------------------------------------
static  void    RGBToColor (RGB565 *prgb, DXT_COLOR *pcolor)
{
   WORD        rgb;
   DXT_COLOR   color;

   rgb = *((WORD *)prgb);

   // pick off bits in groups of 5, 6, and 5
   color.rgba[BLU] = (unsigned char) rgb;
   rgb >>= BLU_BITS;
   color.rgba[GRN] = (unsigned char) rgb;
   rgb >>= GRN_BITS;
   color.rgba[RED] = (unsigned char) rgb;

   // shift primaries into the appropriate LSBs
   color.rgba[BLU] <<= BLU_SHIFT;
   color.rgba[GRN] <<= GRN_SHIFT;
   color.rgba[RED] <<= RED_SHIFT;

   // replicate primaries MSBs into LSBs

   color.rgba[BLU] |= color.rgba[BLU] >> BLU_BITS;
   color.rgba[GRN] |= color.rgba[GRN] >> GRN_BITS;
   color.rgba[RED] |= color.rgba[RED] >> RED_BITS;

   *pcolor = color;
}

//-----------------------------------------------------------------------------
// pack a fixed point color
//-----------------------------------------------------------------------------
static  void ColorToRGB (DXT_COLOR *pcolor, RGB565 *prgb)
{
   WORD        rgb;

   rgb = pcolor->rgba[BLU] >> (BLU_SHIFT);
   rgb |= ((pcolor->rgba[GRN] >> (GRN_SHIFT)) << BLU_BITS);
   rgb |= ((pcolor->rgba[RED] >> (RED_SHIFT)) << (BLU_BITS+GRN_BITS));

   *((WORD *)prgb) = rgb;
}


//-----------------------------------------------------------------------------
// DecodeBlockRGB - decompress a color block
//-----------------------------------------------------------------------------
void DecodeBlockRGB (DXTBlockRGB *pblockSrc, DXT_COLOR colorDst[DXT_BLOCK_PIXELS])
{
   int         lev;
   DXT_COLOR   clut[4];
   PIXBM       pixbm;
   int         pixel;
   int         primary;

   // if source block is invalid, ...
   if (pblockSrc == 0)
      return;

   // determine the number of color levels in the block
   lev = (pblockSrc->rgb0 <= pblockSrc->rgb1) ? 2 : 3;

   // Fill extrema values into pixel code lookup table.
   RGBToColor(&pblockSrc->rgb0, &clut[0]);
   RGBToColor(&pblockSrc->rgb1, &clut[1]);

   clut[0].rgba[ALFA] =
   clut[1].rgba[ALFA] =
   clut[2].rgba[ALFA] = 255;

   if (lev == 3)
   { // No transparency info present, all color info.
      for (primary = 0; primary < NUM_PRIMARIES; ++primary)
      {
         WORD temp0 = clut[0].rgba[primary];   // jvanaken fixed overflow bug
         WORD temp1 = clut[1].rgba[primary];
         clut[2].rgba[primary] = (BYTE)((2*temp0 + temp1 + 1)/3);
         clut[3].rgba[primary] = (BYTE)((temp0 + 2*temp1 + 1)/3);
      }
      clut[3].rgba[ALFA] = 255;
   }
   else
   {  // transparency info.
      for (primary = 0; primary < NUM_PRIMARIES; ++primary)
      {
         WORD temp0 = clut[0].rgba[primary];   // jvanaken fixed overflow bug
         WORD temp1 = clut[1].rgba[primary];
         clut[2].rgba[primary] = (BYTE)((temp0 + temp1)/2);
         clut[3].rgba[primary] = 0;     // jvanaken added this
      }
      clut[3].rgba[ALFA] = 0;
   }

   // munge a local copy
   pixbm = pblockSrc->pixbm;

   // Look up the actual pixel color in the table.
   for (pixel=0; pixel < DXT_BLOCK_PIXELS; ++pixel)
   {
      // lookup color from pixel bitmap
      for (primary = 0; primary < NUM_PRIMARIES; ++primary)
         colorDst[pixel].rgba[primary] = clut[pixbm & 3].rgba[primary];

      colorDst[pixel].rgba[ALFA] = clut[pixbm & 3].rgba[ALFA];

      // prepare to extract next index
      pixbm >>= 2;
   }
}

//-----------------------------------------------------------------------------
// EncodeBlockDXT - compress a DXT block
//-----------------------------------------------------------------------------

static void EncodeBlockRGB_i(DXT_COLOR colorSrc[DXT_BLOCK_PIXELS], DXTBlockRGB *pblockDst, int ignore_alpha)
{
   int         i, j, dr, dg, db, td;
   int         dcolor1=0, dcolor2=0;
   int         cdist=0;
   int         lev;
   int         primary;
   int         nearest,nIdx;
   RGB565      tclr1,tclr2;
   DXT_COLOR   lcolors[4];
   DWORD       pkData=0;
   WORD        temp0,temp1;

   if (colorSrc == 0)
      return;


   lev=3;

   // This is a stupid buble sort/search for the furthest colors,
   // they should be the most extream for compressing.
   for (i=0;i < DXT_BLOCK_PIXELS;i++)
   {
      if (!ignore_alpha)
      {
         if (colorSrc[i].rgba[ALFA] < 128)
         {
            lev=2;
         }
      }

      for(j=i;j < DXT_BLOCK_PIXELS;j++)
      {
         dr = colorSrc[i].rgba[RED] - colorSrc[j].rgba[RED];
         dr *= dr;
         dg = colorSrc[i].rgba[GRN] - colorSrc[j].rgba[GRN];
         dg *= dg;
         db = colorSrc[i].rgba[BLU] - colorSrc[j].rgba[BLU];
         db *= db;
         td = dr+dg+db;
         if (td > cdist)
         {
            dcolor1=i;
            dcolor2=j;
            cdist=td;
         }
      }
   }


   ColorToRGB(&colorSrc[dcolor1],&tclr1);
   ColorToRGB(&colorSrc[dcolor2],&tclr2);

   if (lev == 3)
   {
      if (tclr1 > tclr2)
      {
         lcolors[0]=colorSrc[dcolor1];
         pblockDst->rgb0 = tclr1;
         lcolors[1]=colorSrc[dcolor2];
         pblockDst->rgb1 = tclr2;
      }
      else
      {
         lcolors[0]=colorSrc[dcolor2];
         pblockDst->rgb0 = tclr2;
         lcolors[1]=colorSrc[dcolor1];
         pblockDst->rgb1 = tclr1;
      }

      lcolors[0].rgba[RED] &= 0xf8;
      lcolors[0].rgba[GRN] &= 0xfc;
      lcolors[0].rgba[BLU] &= 0xf8;

      lcolors[1].rgba[RED] &= 0xf8;
      lcolors[1].rgba[GRN] &= 0xfc;
      lcolors[1].rgba[BLU] &= 0xf8;

      for (primary=0; primary < NUM_PRIMARIES; ++primary)
      {
         temp0 = lcolors[0].rgba[primary];
         temp1 = lcolors[1].rgba[primary];
         lcolors[2].rgba[primary] = (BYTE)((2*temp0 + temp1 + 1) / 3);
         lcolors[3].rgba[primary] = (BYTE)((temp0 + 2*temp1 + 1) / 3);
      }
      lcolors[3].rgba[ALFA] = 255;
   }
   else
   {

      if (tclr1 > tclr2)
      {
         lcolors[0]=colorSrc[dcolor2];
         pblockDst->rgb0 = tclr2;
         lcolors[1]=colorSrc[dcolor1];
         pblockDst->rgb1 = tclr1;
      }
      else
      {
         lcolors[0]=colorSrc[dcolor1];
         pblockDst->rgb0 = tclr1;
         lcolors[1]=colorSrc[dcolor2];
         pblockDst->rgb1 = tclr2;
      }

      lcolors[0].rgba[RED] &= 0xf8;
      lcolors[0].rgba[GRN] &= 0xfc;
      lcolors[0].rgba[BLU] &= 0xf8;

      lcolors[1].rgba[RED] &= 0xf8;
      lcolors[1].rgba[GRN] &= 0xfc;
      lcolors[1].rgba[BLU] &= 0xf8;

      for (primary=0;primary < NUM_PRIMARIES;++primary)
      {
         temp0 = lcolors[0].rgba[primary];
         temp1 = lcolors[1].rgba[primary];
         lcolors[2].rgba[primary] = (BYTE)((temp0 + temp1) / 2);
         lcolors[3].rgba[primary] = 0;
      }
      lcolors[3].rgba[ALFA] = 0;
   }


   for (i=DXT_BLOCK_PIXELS-1; i >=0;i--)
   {
      nearest=0x10000000;
      nIdx=0;
      if (lev==2 && colorSrc[i].rgba[ALFA] < 128)
      {  // It is a transparent texel.
         nIdx = 3;
      }
      else
      {  // It has color, lets find the closest match.
         for (j=0;j < (lev+1);j++)
         {
            dr = colorSrc[i].rgba[RED] - lcolors[j].rgba[RED];
            dr *= dr;
            dg = colorSrc[i].rgba[GRN] - lcolors[j].rgba[GRN];
            dg *= dg;
            db = colorSrc[i].rgba[BLU] - lcolors[j].rgba[BLU];
            db *= db;
            td = dr+dg+db;
            if (td < nearest)
            {
               nearest = td;
               nIdx = j;
            }
         }
      }
      pkData <<= 2;
      pkData |= nIdx;
   }

   pblockDst->pixbm = pkData;

}


void EncodeBlockRGB(DXT_COLOR colorSrc[DXT_BLOCK_PIXELS], DXTBlockRGB *pblockDst)
{
   EncodeBlockRGB_i(colorSrc,pblockDst,0);
}

//-----------------------------------------------------------------------------
// DecodeBlockAlpha4 - decompress a block with alpha at 4 BPP
//-----------------------------------------------------------------------------
void DecodeBlockAlpha4(DXTBlockAlpha4 *pblockSrc, DXT_COLOR colorDst[DXT_BLOCK_PIXELS])
{
   int     row, col;
   WORD    alpha;

   DecodeBlockRGB(&pblockSrc->rgb, colorDst);

   for (row = 0; row < 4; ++row)
   {
      alpha = pblockSrc->alphabm[row];

      for (col = 0; col < 4; ++col)
      {
         colorDst[4 * row + col].rgba[ALFA] = ((alpha & 0xf) << 4)
                                              | (alpha & 0xf);
                                              alpha >>= 4;
      }
   }
}


//-----------------------------------------------------------------------------
// EncodeBlockAlpha4 - compress a block with alpha at 4 BPP
//-----------------------------------------------------------------------------
void EncodeBlockAlpha4(DXT_COLOR colorSrc[DXT_BLOCK_PIXELS], DXTBlockAlpha4 *pblockDst)
{
   int   row,col;
   WORD  alpha;

   EncodeBlockRGB_i(colorSrc,&pblockDst->rgb,1);

   for (row=0; row < 4; ++row)
   {
      alpha = 0;
      for (col=3; col >= 0;col--)
      {
         alpha <<= 4;
         alpha |= ((colorSrc[4*row+col].rgba[ALFA] >> 4) & 0xf);
      }
      pblockDst->alphabm[row] = alpha;
   }
}



//-----------------------------------------------------------------------------
// DecodeBlockAlpha3 - decompress a block with alpha at 3 BPP
//-----------------------------------------------------------------------------
void DecodeBlockAlpha3(DXTBlockAlpha3 *pblockSrc, DXT_COLOR colorDst[DXT_BLOCK_PIXELS])
{
   int     pixel;
   int     alpha[8];       // alpha lookup table
   DWORD   dwBM = 0;       // alpha bitmap in DWORD cache

   DecodeBlockRGB(&pblockSrc->rgb, colorDst);

   alpha[0] = pblockSrc->alpha0;
   alpha[1] = pblockSrc->alpha1;

   // if 8 alpha ramp, ...
   if (alpha[0] > alpha[1])
   {
      // interpolate intermediate colors with rounding
      alpha[2] = (    alpha[0] + 6 * alpha[1] + 3) / 7;
      alpha[3] = (2 * alpha[0] + 5 * alpha[1] + 3) / 7;
      alpha[4] = (3 * alpha[0] + 4 * alpha[1] + 3) / 7;
      alpha[5] = (4 * alpha[0] + 3 * alpha[1] + 3) / 7;
      alpha[6] = (5 * alpha[0] + 2 * alpha[1] + 3) / 7;
      alpha[7] = (6 * alpha[0] +     alpha[1] + 3) / 7;
   }
   else
   { // else 6 alpha ramp with 0 and 255
      // interpolate intermediate colors with rounding
      alpha[2] = (    alpha[0] + 4 * alpha[1] + 3) / 5;
      alpha[3] = (2 * alpha[0] + 3 * alpha[1] + 3) / 5;
      alpha[4] = (3 * alpha[0] + 2 * alpha[1] + 3) / 5;
      alpha[5] = (4 * alpha[0] +     alpha[1] + 3) / 5;
      alpha[6] = 0;
      alpha[7] = 255;
   }

   for (pixel=0; pixel < DXT_BLOCK_PIXELS; ++pixel)
   {
      // reload bitmap dword cache every 8 pixels
      if ((pixel & 7) == 0)
      {
         if (pixel == 0)
         {
            // pack 3 bytes into dword
            dwBM  = pblockSrc->alphabm[2];
            dwBM <<= 8;
            dwBM |= pblockSrc->alphabm[1];
            dwBM <<= 8;
            dwBM |= pblockSrc->alphabm[0];
         }
         else
         {  // pixel == 8
            // pack 3 bytes into dword
            dwBM  = pblockSrc->alphabm[5];
            dwBM <<= 8;
            dwBM |= pblockSrc->alphabm[4];
            dwBM <<= 8;
            dwBM |= pblockSrc->alphabm[3];
         }
      }

      // unpack bitmap dword 3 bits at a time
      colorDst[pixel].rgba[ALFA] = (BYTE)alpha[(dwBM & 7)];
      dwBM >>= 3;
   }
}


void EncodeBlockAlpha3(DXT_COLOR colorSrc[DXT_BLOCK_PIXELS], DXTBlockAlpha3 *pblockDst)
{
   int   i,j;
   int   t;
   int   alpha[8];
   int   min = 255;
   int   max = 0;
   int   opaque=0;
   int   transparent=0;
   int   tsize = 8;
   int   nearest;
   int   nearestIdx;
   int   maxset=0, minset=0;
   unsigned long dwBM;

   EncodeBlockRGB_i(colorSrc,&pblockDst->rgb,1);

   for (i=0;i < DXT_BLOCK_PIXELS; i++)
   {
      if(colorSrc[i].rgba[ALFA] == 255)
         opaque = 1;
      else if (colorSrc[i].rgba[ALFA] == 0)
         transparent = 1;

      if(colorSrc[i].rgba[ALFA] >= max)
      {
         if (colorSrc[i].rgba[ALFA] != 255)
         {
            max = colorSrc[i].rgba[ALFA];
            maxset = 1;
         }
      }
      if (colorSrc[i].rgba[ALFA] <= min)
      {
         if (colorSrc[i].rgba[ALFA] != 0)
         {
            min = colorSrc[i].rgba[ALFA];
            minset = 1;
         }
      }
   }
   if ( transparent || opaque )
   {  // use 6 alpha rampiwth 0 and 255

      // This keeps min from being larger max, which indicates that to the
      // decompressor that it uses 8 ramps, and no explicit 0,255.
      // This can happen if every value in the block is transparent.

      if (!minset)
         min=0;

      if (!maxset)
         max=255;

      alpha[0]=min;
      alpha[1]=max;
      alpha[2] = (    alpha[0] + 4 * alpha[1] + 3) / 5;
      alpha[3] = (2 * alpha[0] + 3 * alpha[1] + 3) / 5;
      alpha[4] = (3 * alpha[0] + 2 * alpha[1] + 3) / 5;
      alpha[5] = (4 * alpha[0] +     alpha[1] + 3) / 5;
      alpha[6]=0;
      alpha[7]=255;
   }
   else
   { // use 8 interpolated alphas

      // interpolate intermediate colors with rounding
      alpha[0] = max;
      alpha[1] = min;
      alpha[2] = (    alpha[0] + 6 * alpha[1] + 3) / 7;
      alpha[3] = (2 * alpha[0] + 5 * alpha[1] + 3) / 7;
      alpha[4] = (3 * alpha[0] + 4 * alpha[1] + 3) / 7;
      alpha[5] = (4 * alpha[0] + 3 * alpha[1] + 3) / 7;
      alpha[6] = (5 * alpha[0] + 2 * alpha[1] + 3) / 7;
      alpha[7] = (6 * alpha[0] +     alpha[1] + 3) / 7;
   }

#ifdef WINNT
   // clean up warnings in w2k build
   pblockDst->alpha0 = (BYTE)alpha[0];
   pblockDst->alpha1 = (BYTE)alpha[1];
#else
   pblockDst->alpha0 = alpha[0];
   pblockDst->alpha1 = alpha[1];
#endif

   dwBM=0;
   for (i=DXT_BLOCK_PIXELS-1; i >= 0; i--)
   {
      nearest = 255;
      if (transparent || opaque)
      {
         if (colorSrc[i].rgba[ALFA] == 0)
            nearestIdx = 6;
         else if (colorSrc[i].rgba[ALFA] == 255)
            nearestIdx = 7;
         else
         {
            for (j=0; j < 6; j++)
            {
               t=ABS(colorSrc[i].rgba[ALFA] - alpha[j]);
               if (t < nearest)
               {
                  nearest = t;
                  nearestIdx = j;
               }
            }
         }
      }
      else
      {
         for(j=0; j < 8; j++)
         {
            t = ABS(colorSrc[i].rgba[ALFA] - alpha[j]);
            if (t < nearest)
            {
               nearest = t;
               nearestIdx = j;
            }
         }
      }
      dwBM <<= 3;
      dwBM |= (nearestIdx & 0x7);
      if (i==8)
      {
         pblockDst->alphabm[3] = (unsigned char)(dwBM & 0xff);
         dwBM >>= 8;
         pblockDst->alphabm[4] = (unsigned char)(dwBM & 0xff);
         dwBM >>= 8;
         pblockDst->alphabm[5] = (unsigned char)(dwBM & 0xff);
         dwBM >>= 8;

         dwBM=0;
      }
   }

   pblockDst->alphabm[0] = (unsigned char)(dwBM & 0xff);
   dwBM >>= 8;
   pblockDst->alphabm[1] = (unsigned char)(dwBM & 0xff);
   dwBM >>= 8;
   pblockDst->alphabm[2] = (unsigned char)(dwBM & 0xff);
   dwBM >>= 8;

}


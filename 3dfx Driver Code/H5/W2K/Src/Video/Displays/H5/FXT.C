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

#include "d3global.h"
#include "fxt.h"

#define assert(_bs)		(_bs)
#ifndef NULL
#define NULL	((void *)0)
#endif

/***

	Routines from bitcoder.c

***/


#define REMOVE(d, b, n) { d = (FxU32) (b & ((1 << (n)) - 1)); b >>= n; }
#define INSERT(b, d, n) { b <<= n; b |= ((d) & ((1 << (n)) - 1)); }

#define RAND_TABLE_SIZE 43
int rand_table[RAND_TABLE_SIZE]=
{
   41   ,
   18467,
   6334 ,
   26500,
   19169,
   15724,
   11478,
   29358,
   26962,
   24464,
   5705 ,
   28145,
   23281,
   16827,
   9961 ,
   491  ,
   2995 ,
   11942,
   4827 ,
   5436 ,
   32391,
   14604,
   3902 ,
   153  ,
   292  ,
   12382,
   17421,
   18716,
   19718,
   19895,
   5447 ,
   21726,
   14771,
   11538,
   1869 ,
   19912,
   25667,
   26299,
   17035,
   9894 ,
   28703,
   23811,
   31322,
};

static int local_rand()
{
   static table_loc=0;
   if (table_loc < RAND_TABLE_SIZE-1)
   {
      table_loc++;
   }
   else
   {
      table_loc=0;
   }
   return (rand_table[table_loc]);
}

/* 
 * The following 4 procedures read and write 2 or 3 bit indices into 
 * the bit bucket 
 */
static void encode2(int *index, FxU32 bits[4] )
{
   FxU32 lo = 0, hi = 0; 
   int   i;

   for (i=15; i >= 0; i--) 
   {
      lo = (lo << 2) | (index[i     ] & 3);
      hi = (hi << 2) | (index[i + 16] & 3);
   }
   bits[0] = lo;
   bits[1] = hi;
}

static void encode3(int *index, FxU32 bits[4] )
{
   FxU64 lo = 0, hi = 0;
   int   i;

   for (i=15; i >= 0; i--) 
   {
      lo = (lo << 3) | (index[i     ] & 7);
      hi = (hi << 3) | (index[i + 16] & 7);
   }
   bits[0] = (FxU32) lo;
   bits[1] = (FxU32) ((lo >> 32) | (hi << 16));
   bits[2] = (FxU32) (hi >> 16);
}

static void decode2(FxU32 bits[4], int *index)
{
   FxU32 lo, hi;
   int   i;

   lo = bits[0];
   hi = bits[1];

   for (i=0; i<16; i++) 
   {
      index[i +  0] = lo & 3;  lo >>= 2;
      index[i + 16] = hi & 3;  hi >>= 2;
   }
}

static void decode3(FxU32 bits[4], int *index)
{
   FxU64 lo, hi;
   int   i;

   lo = bits[0] | (((FxU64) bits[1]) << 32);
   hi = (bits[1] >> 16) | (((FxU64) bits[2]) << 16);

   for (i=0; i<16; i++) 
   {
      index[i +  0] = (int) (lo & 7); lo >>= 3;
      index[i + 16] = (int) (hi & 7); hi >>= 3;
   }
}

/* decode variable length mode field */

static int getMode(FxU32 bits3)
{
   int mode;

   bits3 >>= 29;

   if ( bits3 & 4 ) 
   { // 1XX
      mode = TCC_MIXED;
   } 
   else if (( bits3 & 6 ) == 0 ) 
   { // 00X
      mode = TCC_HI;
   } 
   else 
   {
      switch ( bits3 ) 
      {
         case TCC_CHROMA:
         case TCC_ALPHA:
            mode = bits3;
            break;
         default:
//            txError("FXT1 bad mode\n");
         break;
      }
   }
   return mode;
}

int bitDecoder( void *bits128, FxU32 c[4], int index[32], FxU32 *alpha )
{
   FxU32       *bits = (FxU32 *) bits128;
   int         mode = getMode(bits[3]);
   FxU64       d;

   d = bits[3];
   d = (d << 32) | bits[2];

   switch(mode & 3) 
   {
      case TCC_HI:
         {
            d >>= 32;
            REMOVE(c[0], d, 15);
            REMOVE(c[1], d, 15);
            c[2] = 0;
            c[3] = 0;
            decode3( bits, index);
            *alpha = 0;
         }
         break;

      case TCC_MIXED:
         {
            REMOVE(c[0], d, 15);
            REMOVE(c[1], d, 15);
            REMOVE(c[2], d, 15);
            REMOVE(c[3], d, 15);
            REMOVE(*alpha, d, 3);
            decode2( bits, index);
         }
         break;

      case TCC_CHROMA:
         {
            REMOVE(c[0], d, 15);
            REMOVE(c[1], d, 15);
            REMOVE(c[2], d, 15);
            REMOVE(c[3], d, 15);
            *alpha = 0;
            decode2( bits, index);
         }
         break;

      case TCC_ALPHA:
         {
            FxU32 a;
            REMOVE(c[0], d, 15);
            REMOVE(c[1], d, 15);
            REMOVE(c[2], d, 15);
            REMOVE(a, d, 5); c[0] |= a << 15;
            REMOVE(a, d, 5); c[1] |= a << 15;
            REMOVE(a, d, 5); c[2] |= a << 15;
            c[3] = 0;
            REMOVE(*alpha, d, 1);      // interpolate
            decode2( bits, index);
         }
         break;
   }
   return mode;
}

void bitEncoder( int mode, FxU32 c[4], FxU32 alpha, int index[32], void *bits128 )
{
   FxU32 *bits = (FxU32 *) bits128;
   FxU64 d = 0;

   // mode has variable length encoding to fit alpha bits
   switch(mode & 3) 
   {
      case TCC_HI:
         {
            encode3( index, bits);
            INSERT( d, mode, 2);
            INSERT( d, c[1], 15);
            INSERT( d, c[0], 15);
            bits[3] = (FxU32) d;
         }
         break;
      case TCC_MIXED:
         {
            encode2( index, bits);
            INSERT(d, mode, 1);
            INSERT(d, alpha, 3);
            INSERT(d, c[3], 15);
            INSERT(d, c[2], 15);
            INSERT(d, c[1], 15);
            INSERT(d, c[0], 15);
            bits[2] = (FxU32) d;
            bits[3] = (FxU32) (d >> 32);
         }
         break;
      case TCC_CHROMA:
         {
            encode2( index, bits);
            INSERT(d, mode, 3);
            INSERT(d, 0, 1);             // pad
            INSERT(d, c[3], 15);
            INSERT(d, c[2], 15);
            INSERT(d, c[1], 15);
            INSERT(d, c[0], 15);
            bits[2] = (FxU32) d;
            bits[3] = (FxU32) (d >> 32);
         }
         break;
      case TCC_ALPHA:
         {
            encode2( index, bits);
            INSERT(d, mode, 3);
            INSERT(d, alpha, 1);         // interpolate
            INSERT(d, (c[2]>>15), 5);
            INSERT(d, (c[1]>>15), 5);
            INSERT(d, (c[0]>>15), 5);
            INSERT(d, (c[2]&0x7fff), 15);
            INSERT(d, (c[1]&0x7fff), 15);
            INSERT(d, (c[0]&0x7fff), 15);
            bits[2] = (FxU32) d;
            bits[3] = (FxU32) (d >> 32);
         }
         break;
   }
}


/***

	Routines from codec.c

***/


#define SQUARED(x)  ((x)*(x))
#define ABS(x)      (((x) < 0) ? -(x) : (x))

int globalX, globalY;
int     a_color_cc  = -1; // force color coding
int     a_force_cc  = -1; // force color mode
int     a_tolerance = 0;
int     a_lerp = 0; // force interpolation in alpha mode

static int bestColor(const float  *a, 
                     const float  codebook[][3], 
                     int          codesize)
{
   int     i;
   int     bestindex = -1;
   float   bestdist  = 1.0e30F;
   float   dist;

   for (i=0; i < codesize; i++) 
   {
      dist = SQUARED(a[0] - codebook[i][0]) +
      SQUARED(a[1] - codebook[i][1]) +
      SQUARED(a[2] - codebook[i][2]) ;
      if (dist < bestdist) 
      {
         bestdist  = dist;
         bestindex = i;
      }
   }
   return bestindex;
}

// Usable only for the interpolation compression modes.
static int bestColorRGBInterp(const float  a[3], 
                              const float  iv[3],
                              const float  b,
                              const int    codesize)
{
   unsigned t = (unsigned)((a[0]*iv[0] + a[1]*iv[1] + a[2]*iv[2]) + b);
   return (int)( (t >= (unsigned)codesize) ? codesize-1 : t);
}

static int bestColorAlpha(const float  *c, 
                          const float   a, 
                          const float  codebook[][4], 
                          const int    codesize,
                          const FxU32  lerp)
{
   int     i;
   int     bestindex = -1;
   float   bestdist  = 1.0e30F;
   float   d0, d1, d2, dist;

   if (!lerp && ( c[0] == 0.0f ) && ( c[1] == 0.0f ) &&  ( c[2] == 0.0f ) && ( a == 0.0f ))
      return 3;

   for (i=0; i < codesize; i++) 
   {
      if ( a_lerp ) 
      {
         d0 = SQUARED(c[0]*a - codebook[i][0]*codebook[i][3]);
         d1 = SQUARED(c[1]*a - codebook[i][1]*codebook[i][3]);
         d2 = SQUARED(c[2]*a - codebook[i][2]*codebook[i][3]);

         dist = SQUARED(c[0]*a - codebook[i][0]*codebook[i][3]) +
         SQUARED(c[1]*a - codebook[i][1]*codebook[i][3]) +
         SQUARED(c[2]*a - codebook[i][2]*codebook[i][3]);
         dist = d0 + d1 + d2;
      } 
      else 
      {
         dist = SQUARED(c[0] - codebook[i][0]) +
         SQUARED(c[1] - codebook[i][1]) +
         SQUARED(c[2] - codebook[i][2]) +
         SQUARED(a - codebook[i][3]);
      }
      if (dist < bestdist) 
      {
         bestdist  = dist;
         bestindex = i;
      }
   }
   if (( c[0] < 10.f ) && ( codebook[bestindex][0] > 50.f ))
      return bestindex;

   return bestindex;
}

/* Given either end points for the 2 colors, generate intermediate colors */
static void makePalette( FxU32 lo, FxU32 hi, int nlevels, FxU32 *palette, float fpal[][3])
{
   int rlo, glo, blo, alo, rhi, ghi, bhi, ahi, r, g, b, a, i;

   assert((nlevels == 7) || (nlevels == 4) || (nlevels == 3));

   alo = ALF(lo);
   rlo = RED(lo);
   glo = GRN(lo);
   blo = BLU(lo);

   ahi = ALF(hi);
   rhi = RED(hi);
   ghi = GRN(hi);
   bhi = BLU(hi);

   for (i=0; i < nlevels; i++) 
   {
      a = alo + ((ahi - alo) * i) / (nlevels - 1);
      r = rlo + ((rhi - rlo) * i) / (nlevels - 1);
      g = glo + ((ghi - glo) * i) / (nlevels - 1);
      b = blo + ((bhi - blo) * i) / (nlevels - 1);

      /* make sure all values are within 0..255 */
      assert( ((a & ~0xff) == 0) && ((r & ~0xff) == 0) && 
      ((g & ~0xff) == 0) && ((b & ~0xff) == 0) );

      if (palette) 
      {
         palette[i] = ARGB(a, r, g, b);
      }

      if (fpal) 
      {
         fpal[i][0] = (float) r;
         fpal[i][1] = (float) g;
         fpal[i][2] = (float) b;
      }
   }
}

// Returns a vector 'iv' that when dotted with a color and added to an offset 'b', 
// finds the index of the Cartesian-nearest color in the (linear) palette. 
//
// Imagine the set of ncolors-1 planes in color space, each normal to the line through
// the color palette, that partition color space into slab-shaped sets of points, each slab
// belonging to a particular palette entry.  This procedure implements the first phase
// of that mapping by reducing the color palette to a vector 'iv' and offset 'b' for
// later use by bestColorRGBInterp().
static void makeInterpVector( float p[8][3],  // 8 is an upper bound by the design of FXT1
                              int ncolors, 
                              float iv[3],    // RETURN
                              float *b)       // RETURN
{
   float d2 = 0.0f;
   float rd2;
   int i;

   for ( i=0; i<3; i++) 
   {
      iv[i] = p[ncolors-1][i] - p[0][i];  // vector between extrema of palette; may be zero
      d2 += iv[i]*iv[i];                  // accumulate square of Cartesian distance
   }
   rd2 = (float)(ncolors-1) / d2;          // if all iv[] are 0, rd2 is Infinity
   *b = 0.0f;
   for ( i=0; i<3; i++) 
   {
      *b -= iv[i]*p[0][i];
      iv[i] *= rd2;                       // if rd2 is Infinity and iv[i] was 0, result is NaN
   }
   *b = *b * rd2 + 0.5f;
}

/* Given either end points for the 2 colors, generate intermediate colors */
static void makePaletteAlpha( FxU32 lo, FxU32 hi, int nlevels, FxU32 *palette, float fpal[][4])
{
   int rlo, glo, blo, alo, rhi, ghi, bhi, ahi, r, g, b, a, i;

   assert((nlevels == 7) || (nlevels == 4) || (nlevels == 3));

   alo = ALF(lo);
   rlo = RED(lo);
   glo = GRN(lo);
   blo = BLU(lo);

   ahi = ALF(hi);
   rhi = RED(hi);
   ghi = GRN(hi);
   bhi = BLU(hi);

   for (i=0; i < nlevels; i++) 
   {
      a = alo + ((ahi - alo) * i) / (nlevels - 1);
      r = rlo + ((rhi - rlo) * i) / (nlevels - 1);
      g = glo + ((ghi - glo) * i) / (nlevels - 1);
      b = blo + ((bhi - blo) * i) / (nlevels - 1);

      /* make sure all values are within 0..255 */
      assert( ((a & ~0xff) == 0) && ((r & ~0xff) == 0) && 
      ((g & ~0xff) == 0) && ((b & ~0xff) == 0) );

      if (palette) 
      {
         palette[i] = ARGB(a, r, g, b);
      }

      if (fpal) 
      {
         fpal[i][0] = (float) r;
         fpal[i][1] = (float) g;
         fpal[i][2] = (float) b;
         fpal[i][3] = (float) a;
      }
   }
}

/* rgb5555 to 8888 by msb replication */
static FxU32 argb8888( FxU32 rgb5555 )
{
   FxU32 a = (rgb5555 >> 15) & 0x1f;
   FxU32 r = (rgb5555 >> 10) & 0x1f;
   FxU32 g = (rgb5555 >>  5) & 0x1f;
   FxU32 b = (rgb5555      ) & 0x1f;

   a = (a << 3) | (a >> 2);    
   r = (r << 3) | (r >> 2);    
   g = (g << 3) | (g >> 2);
   b = (b << 3) | (b >> 2);

   return ARGB( a, r, g, b);
}

/* rgb555 to 888 by msb replication */
static FxU32 rgb888( FxU32 rgb555 )
{
   FxU32 r = (rgb555 >> 10) & 0x1f;
   FxU32 g = (rgb555 >>  5) & 0x1f;
   FxU32 b = (rgb555      ) & 0x1f;

   r = (r << 3) | (r >> 2);    
   g = (g << 3) | (g >> 2);
   b = (b << 3) | (b >> 2);

   return ARGB( 0xff, r, g, b);
}

/* rgb565 to 888 by msb replication */
static FxU32 rgb565_888( FxU32 rgb565 )
{
   FxU32 r = (rgb565 >> 11) & 0x1f;
   FxU32 g = (rgb565 >>  5) & 0x3f;
   FxU32 b = (rgb565      ) & 0x1f;

   r = (r << 3) | (r >> 2);    
   g = (g << 2) | (g >> 4);
   b = (b << 3) | (b >> 2);

   return ARGB( 0xff, r, g, b);
}

/* rgb888 to 565 by rounding */
static FxU32 rgb565( FxU32 rgb888 )
{
   FxU32 r = (RED(rgb888) + 4) >> 3;
   FxU32 g = (GRN(rgb888) + 2) >> 2;
   FxU32 b = (BLU(rgb888) + 4) >> 3;

   if (r > 0x1f) r = 0x1f;
   if (g > 0x3f) g = 0x3f;
   if (b > 0x1f) b = 0x1f;

   return (r << 11) | (g << 5) | b;
}

/* argb8888 to 5555 by rounding */
static FxU32 argb5555( FxU32 argb8888 )
{
   FxU32 a = (ALF(argb8888) + 4) >> 3;
   FxU32 r = (RED(argb8888) + 4) >> 3;
   FxU32 g = (GRN(argb8888) + 4) >> 3;
   FxU32 b = (BLU(argb8888) + 4) >> 3;

   if (a > 0x1f) a = 0x1f;
   if (r > 0x1f) r = 0x1f;
   if (g > 0x1f) g = 0x1f;
   if (b > 0x1f) b = 0x1f;

   return (a << 15 ) | (r << 10) | (g << 5) | b;
}

/* rgb888 to 555 by rounding */
static FxU32 rgb555( FxU32 rgb888 )
{
   FxU32 r = (RED(rgb888) + 4) >> 3;
   FxU32 g = (GRN(rgb888) + 4) >> 3;
   FxU32 b = (BLU(rgb888) + 4) >> 3;

   if (r > 0x1f) r = 0x1f;
   if (g > 0x1f) g = 0x1f;
   if (b > 0x1f) b = 0x1f;

   return (r << 10) | (g << 5) | b;
}
/*
 * The eigen vector generated may sometimes have endpoints that are outside
 * the rgb color space. We clip it along the line, and move endpoints within
 * the color space.
 */
void clipLine(float lo[3], float hi[3])
{
   int i, j;
   int cclo, cchi;
   int swapped = 0;

again:
   cclo = 0;
   cchi = 0;

   for (i=0; i<3; i++) 
   {
      if (lo[i] <   0.0f) cclo |= (1 <<     i);
      if (hi[i] <   0.0f) cchi |= (1 <<     i);

      if (lo[i] > 255.0f) cclo |= (1 << (3+i));
      if (hi[i] > 255.0f) cchi |= (1 << (3+i));
   }

   if (cclo & cchi) {
      // trivial reject. Bad news.
      // fprintf(stdout, "\nBad   : [%4.0f %4.0f %4.0f][%4.0f %4.0f %4.0f]\n",
      // lo[0], lo[1], lo[2], hi[0], hi[1], hi[2]);

      // Try to fix it directly by clamping (Really bad, this)
      if ((cclo & cchi) & 0x01) lo[0] = hi[0] =   0.0f;
      if ((cclo & cchi) & 0x02) lo[1] = hi[1] =   0.0f;
      if ((cclo & cchi) & 0x04) lo[2] = hi[2] =   0.0f;

      if ((cclo & cchi) & 0x08) lo[0] = hi[0] = 255.0f;
      if ((cclo & cchi) & 0x10) lo[1] = hi[1] = 255.0f;
      if ((cclo & cchi) & 0x20) lo[2] = hi[2] = 255.0f;

      // fprintf(stdout, "\nFixed : [%4.0f %4.0f %4.0f][%4.0f %4.0f %4.0f]\n",
      // lo[0], lo[1], lo[2], hi[0], hi[1], hi[2]);

   } 
   else if ((cclo | cchi) == 0)
   {
      // trivial accept
      return;
   }

	for (i=0; i<3; i++) 
   {
	   float   t;

	   // Travel towards the center, shortening all coordinates.
	   if (lo[i] <   0.0f) 
      {
	      t = (  0.0f - hi[i]) / (lo[i] - hi[i]);
	   } 
      else if (lo[i] > 255.0f) 
      {
	      t = (255.0f - hi[i]) / (lo[i] - hi[i]);
	   }
	   else 
         continue;

	   // Shorten all coordinates by this amount.
	   for (j=0; j<3; j++) 
      {
	      lo[j] = hi[j] + (lo[j] - hi[j]) * t;
	   }

	   // Account for round-off errors.
	   // if (lo[i] < 0.0f) lo[i] = 0.0f;
	   // else if (lo[i] > 255.0f) lo[i] = 255.0f;

	}
	// There might be some roundoff errors, so we fudge.
	for (i=0; i<3; i++) 
   {
		if ((lo[i] <   0.0f) /* && (lo[i] >   -2.0f)*/) lo[i] = 0.0f;
		if ((lo[i] > 255.0f) /* && (lo[i] <  257.0f)*/) lo[i] = 255.0f;

		if ((lo[i] < 0.0f) || (lo[i] > 255.0f)) 
      {
			// fprintf(stderr, "\n Bad color: %4.0f %4.0f %4.0f\n", 
			// lo[0], lo[1], lo[2]);
		}
	}
	if (!swapped) 
   {
		// reverse end points and do it again.
		float  *tmp;
		swapped = 1;
		tmp = lo; lo = hi; hi = tmp;
		goto again;
	}
}

/* 
 * Given that lo and hi differ by less than 16 on all 3 coords, encode it
 * as a midpoint color at 666 resolution, plus a 12 bit signed delta.
 */
static FxU32 encodeDelta( float c0[3], float c1[3])
{
   int   r, g, b, dr, dg, db;

   r = float2int ((c0[0] + c1[0]) * 0.5f);
   g = float2int ((c0[1] + c1[1]) * 0.5f);
   b = float2int ((c0[2] + c1[2]) * 0.5f);


   /* round to rgb666 and back to 888 */
   r = (r + 2) >> 2;
   g = (g + 2) >> 2;
   b = (b + 2) >> 2;

   if (r > 0x3f) r = 0x3f;
   if (g > 0x3f) g = 0x3f;
   if (b > 0x3f) b = 0x3f;

   r <<= 2;
   g <<= 2;
   b <<= 2;

   /* Generate half the delta value */
   dr = float2int ((c0[0] - c1[0]) * 0.5f);
   dg = float2int ((c0[1] - c1[1]) * 0.5f);
   db = float2int ((c0[2] - c1[2]) * 0.5f);

   /* Ensure it's within -8 to +7 */
   if (dr < -8) dr = -8;
   if (dg < -8) dg = -8;
   if (db < -8) db = -8;

   if (dr >  7) dr = 7;
   if (dg >  7) dg = 7;
   if (db >  7) db = 7;

   if (dr < 0) 
   {
      if ((r + dr) <   0) dr =   0 -   r; 
      if ((r - dr) > 255) dr =   r - 255;
   } 
   else 
   {
      if ((r - dr) <   0) dr =   r -   0; 
      if ((r + dr) > 255) dr = 255 -   r;
   }

   if (dg < 0) 
   {
      if ((g + dg) <   0) dg =   0 -   g; 
      if ((g - dg) > 255) dg =   g - 255;
   } 
   else 
   {
      if ((g - dg) <   0) dg =   g -   0; 
      if ((g + dg) > 255) dg = 255 -   g;
   }

   if (db < 0) 
   {
      if ((b + db) <   0) db =   0 -   b; 
      if ((b - db) > 255) db =   b - 255;
   } 
   else 
   {
      if ((b - db) <   0) db =   b -   0; 
      if ((b + db) > 255) db = 255 -   b;
   }

   // printf("Mid pts = [%3d %3d %3d]\n", r, g, b);
   // printf("deltas = %d %d %d\n", dr, dg, db);

   /* So here's the new c0 and c1 values you would use for the palette */
   c0[0] = (float) (r - dr);
   c0[1] = (float) (g - dg);
   c0[2] = (float) (b - db);

   c1[0] = (float) (r + dr);
   c1[1] = (float) (g + dg);
   c1[2] = (float) (b + db);

   // fflush(stdout);


   assert((dr >= -8) && (dr <= 7) && 
          (dg >= -8) && (dg <= 7) && 
          (db >= -8) && (db <= 7));

   assert((c0[0] >= 0.0f) && (c0[1] >= 0.0f) && (c0[2] >= 0.0f));
   assert((c1[0] >= 0.0f) && (c1[1] >= 0.0f) && (c1[2] >= 0.0f));

   assert((c0[0] <= 255.0f) && (c0[1] <= 255.0f) && (c0[2] <= 255.0f));
   assert((c1[0] <= 255.0f) && (c1[1] <= 255.0f) && (c1[2] <= 255.0f));

   /* This will be encoded as an rgb666 + drgb444 */
   r >>= 2;
   g >>= 2;
   b >>= 2;
   r = (r << 12) | (g << 6) | b;
   dr = ((dr & 0xf) << 8) | ((dg & 0xf) << 4) | ((db & 0xf));

   return (r << 12) | dr | (0x1 << 30);        // the delta mode bit  is 30 
}

static void decodeDelta( FxU32 col, FxU32 *lo, FxU32 *hi )
{
   int r, g, b, dr, dg, db;
   int rlo, glo, blo, rhi, ghi, bhi;

   db = col & 0x0f; col >>= 4;
   dg = col & 0x0f; col >>= 4;
   dr = col & 0x0f; col >>= 4;
   b  = col & 0x3f; col >>= 6;
   g  = col & 0x3f; col >>= 6;
   r  = col & 0x3f; col >>= 6;

   /* sign extend the deltas */
   if (dr & 8) dr |= 0xfffffff0;
   if (dg & 8) dg |= 0xfffffff0;
   if (db & 8) db |= 0xfffffff0;

   /* make rgb666 to 888 */
   r <<= 2;
   g <<= 2;
   b <<= 2;

   rlo = r - dr;
   glo = g - dg;
   blo = b - db;

   rhi = r + dr;
   ghi = g + dg;
   bhi = b + db;

   assert((rlo >=   0) && (glo >=   0) && (blo >=   0));
   assert((rlo <= 255) && (glo <= 255) && (blo <= 255));

   assert((rhi >=   0) && (ghi >=   0) && (bhi >=   0));
   assert((rhi <= 255) && (ghi <= 255) && (bhi <= 255));

   *lo = ARGB( 255, rlo, glo, blo);
   *hi = ARGB( 255, rhi, ghi, bhi);
}

static void encodeColors(int mode, int mixmode, int alpha, float c0[3], float c1[3], float c2[3], float c3[3], 
                         float input[][3], FxI32 ainput[], void *bits)
{
   int   i, sel, index[32];
   FxU32 lo, hi, col[4];
   float fpal[8][3];
   float iv[3];
   float b;

	switch(mode) 
   {
      case TCC_HI:
         clipLine(c0, c1);
         lo = ARGB( 255, float2int (c0[0]), float2int (c0[1]), float2int (c0[2]));
         hi = ARGB( 255, float2int (c1[0]), float2int (c1[1]), float2int (c1[2]));

         col[0] = rgb555( lo );
         col[1] = rgb555( hi );

         lo = rgb888( col[0] );
         hi = rgb888( col[1] );
         makePalette( lo, hi, 7, NULL, fpal);
         makeInterpVector( fpal, 7, iv, &b);

         /* Map input colors to closest entry in the palette */
         for (i=0; i<32; i++) 
         {
            if ( alpha && ( ainput[i] == 0 ))
               index[i] = 7;
            else 
               index[i] = bestColorRGBInterp( (float *)&input[i][0], iv, b, 7);
         }

         /* Now encode these into the 128 bits */
         bitEncoder( mode, col, alpha, index, bits);
         break;

      case TCC_MIXED:
         clipLine(c0, c1);
         clipLine(c2, c3);

         /* Deal with even block */
         lo = ARGB( 255, float2int (c0[0]), float2int (c0[1]), float2int (c0[2]));
         hi = ARGB( 255, float2int (c1[0]), float2int (c1[1]), float2int (c1[2]));
         if (alpha) 
         {
            col[0] = rgb555(lo);
            col[1] = rgb555(hi);
         } 
         else 
         {
            col[0] = rgb565(lo);
            col[1] = rgb565(hi);
         }
         makePalette( lo, hi, alpha ? 3 : 4, NULL, fpal);
         makeInterpVector( fpal, alpha ? 3 : 4, iv, &b);

         /* Map input colors to closest entry in the palette */
         for (i=0; i<16; i++) 
         {
            if ( alpha && ( ainput[i] == 0 ))
               index[i] = 3;
            else 
               index[i] = bestColorRGBInterp( (float *)&input[i][0], iv, b, alpha ? 3 : 4);
         }

         sel = alpha;

         // funky encoding for lsb of green
         if (!alpha) 
         {
            sel |= ( (col[1]>>5) & 0x1 )<<1;
            if (( (FxU32)index[0] >> 1 ) != (( (col[0]>>5) & 0x1 ) ^ ( (col[1]>>5) & 0x1 )) ) 
            {
               FxU32 tmp = col[0];
               col[0] = col[1];
               col[1] = tmp;
               for (i=0; i<16; i++) 
               {
                  index[i] ^= 3;
               }
            }
            // remove lsb of green
            col[0] = ((col[0] & 0xFFC0) >> 1) | (col[0] & 0x1F);
            col[1] = ((col[1] & 0xFFC0) >> 1) | (col[1] & 0x1F);
         }

         /* Now deal with odd block */
         lo = ARGB( 255, float2int (c2[0]), float2int (c2[1]), float2int (c2[2]));
         hi = ARGB( 255, float2int (c3[0]), float2int (c3[1]), float2int (c3[2]));
         if (alpha) 
         {
            col[2] = rgb555(lo);
            col[3] = rgb555(hi);
         } 
         else 
         {
            col[2] = rgb565(lo);
            col[3] = rgb565(hi);
         }
         makePalette( lo, hi, alpha ? 3 : 4, NULL, fpal);
         makeInterpVector( fpal, alpha ? 3 : 4, iv, &b);

         /* Map input colors to closest entry in the palette */
         for (i=16; i<32; i++) 
         {
            if ( alpha && ( ainput[i] == 0 ))
               index[i] = 3;
            else 
               index[i] = bestColorRGBInterp( (float *)&input[i][0], iv, b, alpha ? 3 : 4);
         }

         // funky encoding for lsb of green
         if (!alpha) 
         {
            sel |= ( (col[3]>>5) & 0x1 )<<2;
            if (( (FxU32)index[16] >> 1 ) != (( (col[2]>>5) & 0x1 ) ^ ( (col[3]>>5) & 0x1 )) ) 
            {
               FxU32 tmp = col[2];
               col[2] = col[3];
               col[3] = tmp;
               for (i=16; i<32; i++) 
               {
                  index[i] ^= 3;
               }
            }
            // remove lsb of green
            col[2] = ((col[2] & 0xFFC0) >> 1) | (col[2] & 0x1F);
            col[3] = ((col[3] & 0xFFC0) >> 1) | (col[3] & 0x1F);
         }

         /* Now encode these into the 128 bits */
         bitEncoder( mode, col, sel, index, bits);
         break;

      case TCC_CHROMA:
         col[0] = ARGB( 255, float2int (c0[0]), float2int (c0[1]), float2int (c0[2]));
         col[1] = ARGB( 255, float2int (c1[0]), float2int (c1[1]), float2int (c1[2]));
         col[2] = ARGB( 255, float2int (c2[0]), float2int (c2[1]), float2int (c2[2]));
         col[3] = ARGB( 255, float2int (c3[0]), float2int (c3[1]), float2int (c3[2]));

         for (i=0; i < 4; i++) 
         {
            int rgb;

            col[i] = rgb555( col[i] );
            rgb = rgb888( col[i] ); 
            fpal[i][0] = (float) RED(rgb); 
            fpal[i][1] = (float) GRN(rgb);
            fpal[i][2] = (float) BLU(rgb);
         }

         /* Map input colors to closest entry in the palette */
         for (i=0; i<32; i++) 
         {
            index[i] = bestColor((float *) &input[i][0], fpal, 4);
         }

         /* Now encode these into the 128 bits */
         bitEncoder( mode, col, alpha, index, bits);
         break;


      default:
      // printf("NYI in encodeColors\n");
      break; // exit(0);
	}
}

static void decodeColors( void *bits, float output[][4] )
{
   int   i, mode, index[32];
   FxU32 col[4], lo, hi; 
   float fpal[8][3];
   FxU32 alpha, glsb;

   mode = bitDecoder( bits, col, index, &alpha);
   switch(mode) 
   {
      case TCC_HI:
         lo = rgb888(col[0]);
         hi = rgb888(col[1]);
         makePalette(lo, hi, 7, NULL, fpal);

         for (i=0; i<32; i++) 
         {
            int j = index[i];
            if ( j == 7 ) 
            {
               output[i][0] = 
               output[i][1] = 
               output[i][2] = 
               output[i][3] = 0.0f;
            } 
            else 
            {
               output[i][0] = 255.0f;
               output[i][1] = fpal[j][0];
               output[i][2] = fpal[j][1];
               output[i][3] = fpal[j][2];
            }
         }
         break;

      case TCC_MIXED:
         glsb = alpha >> 1;
         alpha &= 0x1;
         if ( alpha ) 
         {
            lo = rgb888( col[0] );
            hi = rgb888( col[1] );
         } 
         else 
         {
            // compute 565 colors
            col[0] = (( col[0] & 0x7fe0 ) << 1 ) | ( col[0] & 0x1f ) |
            (((index[0]>> 1) ^ ( glsb & 0x1)) << 5);
            col[1] = (( col[1] & 0x7fe0 ) << 1 ) | ( col[1] & 0x1f ) |
            (( glsb & 0x1) << 5);
            lo = rgb565_888( col[0] );
            hi = rgb565_888( col[1] );
         }
         makePalette(lo, hi, alpha ? 3 : 4, NULL, fpal);
         for (i=0; i<16; i++) 
         {
            int j = index[i];
            if ( alpha && ( j == 3 )) 
            {
               output[i][0] = 
               output[i][1] = 
               output[i][2] = 
               output[i][3] = 0.0f;
            } 
            else 
            {
               output[i][0] = 255.0f;
               output[i][1] = fpal[j][0];
               output[i][2] = fpal[j][1];
               output[i][3] = fpal[j][2];
            }
         }
         if ( alpha ) 
         {
            lo = rgb888( col[2] );
            hi = rgb888( col[3] );
         } 
         else 
         {
            // compute 565 colors
            col[2] = (( col[2] & 0x7fe0 ) << 1 ) | ( col[2] & 0x1f ) |
                     (((index[16]>> 1) ^ ( glsb >> 1)) << 5);
            col[3] = (( col[3] & 0x7fe0 ) << 1 ) | ( col[3] & 0x1f ) |
                     (( glsb >> 1) << 5);
            lo = rgb565_888( col[2] );
            hi = rgb565_888( col[3] );
         }
         makePalette(lo, hi, alpha ? 3 : 4, NULL, fpal);
         for (i=16; i<32; i++) 
         {
            int j;

            j = index[i];
            if ( alpha && ( j == 3 )) 
            {
               output[i][0] = 
               output[i][1] = 
               output[i][2] = 
               output[i][3] = 0.0f;
            } 
            else 
            {
               output[i][0] = 255.0f;
               output[i][1] = fpal[j][0];
               output[i][2] = fpal[j][1];
               output[i][3] = fpal[j][2];
            }
         }
         break;

      case TCC_CHROMA:
         for (i=0; i<4; i++) 
         {
            int rgb;

            rgb = rgb888( col[i] ); 
            fpal[i][0] = (float) RED(rgb); 
            fpal[i][1] = (float) GRN(rgb);
            fpal[i][2] = (float) BLU(rgb);
         }
         for (i=0; i<32; i++) 
         {
            int j = index[i];
            output[i][0] = 255.0f;
            output[i][1] = fpal[j][0];
            output[i][2] = fpal[j][1];
            output[i][3] = fpal[j][2];
         }
         break;

      case TCC_ALPHA:
         if ( alpha ) 
         { // interpolate colors
            lo = argb8888( col[0] );
            hi = argb8888( col[1] );
            makePalette(lo, hi, 4, NULL, fpal); // XXX  makePaletteAlpha ??
            for (i=0; i<16; i++) 
            {
               int j;

               j = index[i];
               output[i][0] = fpal[j][0];
               output[i][1] = fpal[j][1];
               output[i][2] = fpal[j][2];
               output[i][3] = fpal[j][3];
            }

            lo = argb8888( col[2] );
            hi = argb8888( col[1] );
            makePalette(lo, hi, 4, NULL, fpal);
            for (i=16; i<32; i++) 
            {
               int j;

               j = index[i];
               output[i][0] = fpal[j][0];
               output[i][1] = fpal[j][1];
               output[i][2] = fpal[j][2];
               output[i][3] = fpal[j][3];
            }
         } 
         else 
         { // no interpolation use colors as they are index 3 = transparent black
            FxU32 p[4];
            p[0] = argb8888( col[0] );
            p[1] = argb8888( col[1] );
            p[2] = argb8888( col[2] );
            p[3] = 0; // transparent black
            for (i=0; i<32; i++) 
            {
               int j = index[i];

               output[i][0] = (float)ALF(p[j]);
               output[i][1] = (float)RED(p[j]);
               output[i][2] = (float)GRN(p[j]);
               output[i][3] = (float)BLU(p[j]);
            }
         }
         break;

      default:
         // fprintf(stderr, "NYI in decodeColors\n");
         break; // exit(0);
   }

   if (a_color_cc == -1) return;                          // no color coding.
   if ((a_color_cc != 4) && (a_color_cc != mode)) return; // not this block

   // Do color coding.
   {
      float r, g, b;

      if (mode == TCC_HI) 
      { 
         r = 255.0f; g = 255.0f; b = 0.0f;           // yellow
      } 
      else if (mode == TCC_CHROMA) 
      {
         r = 255.0f; g = 0.0f; b = 0.0f;             // red
      } 
      else if (mode == TCC_ALPHA) 
      {
         r = 255.0f; g = 0.0f; b = 255.0f;           // magenta
      } 
      else 
      {
         // mixed.
         i = 0;
         if ((col[0] >> 30) & 1) i++;
         if ((col[1] >> 30) & 1) i++;

         if (i == 0) 
         { 
            r = 0.0f; g = 0.0f; b = 255.0f; 
         }       // blue
         else if (i == 1) 
         { 
            r = 0.0f; g = 255.0f; b = 255.0f; 
         }     // cyan
         else 
         {
            r = 0.0f; g = 255.0f; b = 0.0f; 
         }       // green
      }

#define NPIXELS_COLORED 1    // 32
      for (i=0; i<NPIXELS_COLORED; i++) 
      {
         output[i][1] = r;
         output[i][2] = g;
         output[i][3] = b;
      }
   }
}

#define NCOLORS  4

static void vqChroma(const float in[][3], int ncolors, float colors[][3])
{
   float   input[32][3];
   float   deltas[NCOLORS][3];
   float   errors[NCOLORS];
   float   counts[NCOLORS];
   float   best[NCOLORS][3];
   float   besterr = 1.0e20f;  // infinity
   float   lasterr = 1.0e20f;

   float   alpha = 1.0f;       // XX no other writes to this !?
   float   oo8 = 1.0f/8.0f;
   float   err = 0.0f;
   int     i, j, k;
   int     repeat = 10;

   assert (ncolors <= NCOLORS);

   memcpy(input, in, 32*3*4);
   // Copy input colors, chopping down to 555
   for (i=0; i<32; i++) 
   {
      input[i][0] = in[i][0] * oo8;
      input[i][1] = in[i][1] * oo8;
      input[i][2] = in[i][2] * oo8;
   }

   // Select ncolors colors arbitrarily
   for (i=0; i<ncolors; i++) 
   {
      int j;
      j = local_rand() & 0x1f;
      colors[ i][0] = input[j][0];
      colors[ i][1] = input[j][1];
      colors[ i][2] = input[j][2];
   }

again:
   // Here's the vector quantizer:
   for (k=0; k<50; k++) 
   {

      // Find closest color, and track deltas.
      for (i=0; i<ncolors; i++) 
      {
         counts[i] = 0.0f;
         deltas[i][0] = deltas[i][1] = deltas[i][2] = 0.0f;
         errors[i]    = 0.0f;
      }
      err = 0.0f;

      for (i=0; i<32; i++) 
      {
         float   e;

         j = bestColor((float *) &input[i][0], colors, ncolors);
         counts[j] += 1.0f;
         deltas[j][0] += (input[i][0] - colors[j][0]) * alpha;
         deltas[j][1] += (input[i][1] - colors[j][1]) * alpha;
         deltas[j][2] += (input[i][2] - colors[j][2]) * alpha;

         e = ( SQUARED(colors[j][0] - input[i][0]) +
         SQUARED(colors[j][1] - input[i][1]) +
         SQUARED(colors[j][2] - input[i][2]) );
         err += e;
         errors[j] += e;
      }

      // Update colors.
      for (i=0; i<ncolors; i++) 
      {
         float   c;

         c = (counts[i] == 0.0f) ? 1.0f : counts[i];
         colors[i][0] += (deltas[i][0] / c);
         colors[i][1] += (deltas[i][1] / c);
         colors[i][2] += (deltas[i][2] / c);
      }

      if ((err == 0.0f) || (ABS(lasterr - err) < 1.0f)) break;
      lasterr = err;
   }

   /*
   * Find worst fitting color and replace any item in the palette.
   * in palette 
   */
   if (err < besterr) 
   {
      besterr = err; 
      memcpy( best, colors, ncolors * 3 * sizeof(float));
   } 
   else 
   {
   }

   if ((err == 0.0f) || (--repeat <= 0)) goto done;

   {
      float worsterr = -1.0f;
      int   worsti;

      for (i=0; i<32; i++) 
      {
         float   dr, dg, db, e;

         j = bestColor((float *) &input[i][0], colors, ncolors);
         dr = ABS( input[i][0] - colors[j][0] );
         dg = ABS( input[i][1] - colors[j][1] );
         db = ABS( input[i][2] - colors[j][2] );
         e = dr;
         if (dg > e) e = dg;
         if (db > e) e = db;
         if (e > worsterr) 
         {
            worsterr = e;
            worsti   = i;
         }
      }

      /* If some palette entry is unused, use it; otherwise, gamble */
      for (i=0; i<ncolors; i++) 
         if (counts[i] == 0.0f) break;

      if (i >= ncolors) i = local_rand() % ncolors;

      /* Replace palette entry, and retry. */
      colors[i][0] = input[worsti][0];
      colors[i][1] = input[worsti][1];
      colors[i][2] = input[worsti][2];
   }
   goto again;

done:

	/* Scale colors back to 888 */
	for (i=0; i<ncolors; i++) 
   {
		colors[i][0] = best[i][0] * 8.0f;
		colors[i][1] = best[i][1] * 8.0f;
		colors[i][2] = best[i][2] * 8.0f;
	}
}

static void vqChromaAlpha(const float in[][3], FxI32 ain[], int ncolors, float colors[][4], FxU32 lerp)
{
   float   input[32][4]; // make alpha 4th comp to minimize code delta
   float   deltas[NCOLORS][4];
   float   errors[NCOLORS];
   float   counts[NCOLORS];
   float   best[NCOLORS][4];
   float   besterr = 1.0e20f;  // infinity
   float   lasterr = 1.0e20f;

   float   alpha = 1.0f;       // XX no other writes to this !?
   float   oo8 = 1.0f/8.0f;
   float   err = 0.0f;
   int     i, j, k;
   int     repeat = 10;

   assert (ncolors <= NCOLORS);

   // Copy input colors, chopping down to 555
   for (i=0; i<32; i++) 
   {
      input[i][0] = in[i][0] * oo8;
      input[i][1] = in[i][1] * oo8;
      input[i][2] = in[i][2] * oo8;
      input[i][3] = ain[i] * oo8;
   }

   // Select ncolors colors arbitrarily
   for (i=0; i<ncolors; i++) 
   {
      int j;
      j = local_rand() & 0x1f;
      colors[ i][0] = input[j][0];
      colors[ i][1] = input[j][1];
      colors[ i][2] = input[j][2];
      colors[ i][3] = input[j][3];
   }

again:
   // Here's the vector quantizer:
   for (k=0; k<50; k++) 
   {

      // Find closest color, and track deltas.
      for (i=0; i<ncolors; i++) 
      {
         counts[i] = 0.0f;
         deltas[i][0] = deltas[i][1] = deltas[i][2] = deltas[i][3] = 0.0f;
         errors[i]    = 0.0f;
      }
      err = 0.0f;

      for (i=0; i<32; i++) 
      {
         float   e0, e1, e2, e;

         j = bestColorAlpha((float *) &input[i][0], input[i][3], colors, ncolors, lerp);
         if ( !lerp && ( j == 3 )) continue; // transparent black handled specially
         counts[j] += 1.0f;
         deltas[j][0] += (input[i][0] - colors[j][0]) * alpha;
         deltas[j][1] += (input[i][1] - colors[j][1]) * alpha;
         deltas[j][2] += (input[i][2] - colors[j][2]) * alpha;
         deltas[j][3] += (input[i][3] - colors[j][3]) * alpha;

         if ( a_lerp ) 
         {
            e0 = SQUARED(colors[j][0]*colors[j][3] - input[i][0]*input[i][3]);
            e1 = SQUARED(colors[j][1]*colors[j][3] - input[i][1]*input[i][3]);
            e2 = SQUARED(colors[j][2]*colors[j][3] - input[i][2]*input[i][3]);

            e0 = SQUARED(colors[j][0] - input[i][0]);
            e1 = SQUARED(colors[j][1] - input[i][1]);
            e2 = SQUARED(colors[j][2] - input[i][2]);

            e = ( SQUARED(colors[j][0]*colors[j][3] - input[i][0]*input[i][3]) +
            SQUARED(colors[j][1]*colors[j][3] - input[i][1]*input[i][3]) +
            SQUARED(colors[j][2]*colors[j][3] - input[i][2]*input[i][3]) );
            e = e0 + e1 + e2;
         } 
         else 
         {
            e = ( SQUARED(colors[j][0] - input[i][0]) +
            SQUARED(colors[j][1] - input[i][1]) +
            SQUARED(colors[j][2] - input[i][2]) +
            SQUARED(colors[j][3] - input[i][3]) );
         }
         err += e;
         errors[j] += e;
      }

      // Update colors.
      for (i=0; i<ncolors; i++) 
      {
         float   c;

         c = (counts[i] == 0.0f) ? 1.0f : counts[i];
         colors[i][0] += (deltas[i][0] / c);
         colors[i][1] += (deltas[i][1] / c);
         colors[i][2] += (deltas[i][2] / c);
         colors[i][3] += (deltas[i][3] / c);
      }

      if ((err == 0.0f) || (ABS(lasterr - err) < 1.0f)) break;

      lasterr = err;
   }

   /*
   * Find worst fitting color and replace any item in the palette.
   * in palette 
   */
   if (err < besterr) 
   {
      besterr = err; 
      memcpy( best, colors, ncolors * 4 * sizeof(float));
   } 
   else 
   {
   }

   if ((err == 0.0f) || (--repeat <= 0)) goto done;

   {
      float worsterr = -1.0f;
      int   worsti;

      for (i=0; i<32; i++) 
      {
         float   dr, dg, db, da, e;

         j = bestColorAlpha((float *) &input[i][0], input[i][3], colors, ncolors, lerp);
         if ( !lerp && ( j == 3 )) continue;
         dr = ABS( input[i][0] - colors[j][0] );
         dg = ABS( input[i][1] - colors[j][1] );
         db = ABS( input[i][2] - colors[j][2] );
         da = ABS( input[i][3] - colors[j][3] );
         e = dr;
         if (dg > e) e = dg;
         if (db > e) e = db;
         if (da > e) e = da;
         if (e > worsterr) 
         {
            worsterr = e;
            worsti   = i;
         }
      }

      /* If some palette entry is unused, use it; otherwise, gamble */
      for (i=0; i<ncolors; i++) 
         if (counts[i] == 0.0f) break;

      if (i >= ncolors) i = local_rand() % ncolors;

      /* Replace palette entry, and retry. */
      colors[i][0] = input[worsti][0];
      colors[i][1] = input[worsti][1];
      colors[i][2] = input[worsti][2];
      colors[i][3] = input[worsti][3];
   }
   goto again;

done:

   /* Scale colors back to 888 */
   for (i=0; i<ncolors; i++) 
   {
      colors[i][0] = best[i][0] * 8.0f;
      colors[i][1] = best[i][1] * 8.0f;
      colors[i][2] = best[i][2] * 8.0f;
      colors[i][3] = best[i][3] * 8.0f;
   }
}

static int _cc_hi = 0;
static int _cc_mixed_3 = 0;
static int _cc_mixed_12 = 0;
static int _cc_mixed_0 = 0;
static int _cc_chroma = 0;
static int _cc_alpha = 0;

static void encodeAlpha( float input[][3], FxI32  ainput[], void *bits, FxU32 lerp)
{
	FxU32   lo, hi, p[3], icol[3];
	float   col[3][4];
	float   fpal[4][4];
	int     i, index[32];

	vqChromaAlpha( input, ainput, 3, col, lerp);

	if ( lerp ) 
   {
		/* Deal with even block */
		lo = ARGB( float2int(col[0][3]), float2int (col[0][0]), float2int (col[0][1]), float2int (col[0][2]));
		hi = ARGB( float2int(col[1][3]), float2int (col[1][0]), float2int (col[1][1]), float2int (col[1][2]));
		makePaletteAlpha( lo, hi, 4, NULL, fpal);
		icol[0] = argb5555( lo );
		icol[1] = argb5555( hi );

		/* Map input colors to closest entry in the palette */
		for (i=0; i<16; i++) 
      {
			index[i] = bestColorAlpha((float *) &input[i][0], (float)ainput[i], fpal, 4, lerp);
		}

		/* Now deal with odd block */
		lo = ARGB( float2int(col[2][3]), float2int (col[2][0]), float2int (col[2][1]), float2int (col[2][2]));
		hi = ARGB( float2int(col[1][3]), float2int (col[1][0]), float2int (col[1][1]), float2int (col[1][2]));
		makePaletteAlpha( lo, hi, 4, NULL, fpal);
		icol[2] = argb5555( hi );

		/* Map input colors to closest entry in the palette */
		for (i=16; i<32; i++) 
      {
			index[i] = bestColorAlpha((float *) &input[i][0], (float)ainput[i], fpal, 4, lerp);
		}
	} 
   else 
   { // no interpolation
		/* Deal with even block */
		p[0] = ARGB( float2int(col[0][3]), float2int (col[0][0]), float2int (col[0][1]), float2int (col[0][2]));
		p[1] = ARGB( float2int(col[1][3]), float2int (col[1][0]), float2int (col[1][1]), float2int (col[1][2]));
		p[2] = ARGB( float2int(col[2][3]), float2int (col[2][0]), float2int (col[2][1]), float2int (col[2][2]));
		icol[0] = argb5555( p[0] );
		icol[1] = argb5555( p[1] );
		icol[2] = argb5555( p[2] );

		/* Map input colors to closest entry in the palette */
		for (i=0; i<32; i++) 
      {
			index[i] = bestColorAlpha((float *) &input[i][0], (float)ainput[i], col, 3, lerp);
		}
	} 

	/* Now encode these into the 128 bits */
	bitEncoder( TCC_ALPHA, icol, lerp, index, bits);
	_cc_alpha++;
}

static void quantize4bpp_block(float input[][3], FxI32  ainput[], void *bits)
{
	float   Eavg[3], Emin[3], Emax[3], Eerr[3];     // even  block
	float   Oavg[3], Omin[3], Omax[3], Oerr[3];     // odd   block
	float   Wavg[3], Wmin[3], Wmax[3], Werr[3];     // whole block

	float   Eflo[3][3], Efhi[3][3];                 // even  block
	float   Oflo[3][3], Ofhi[3][3];                 // odd   block
	float   Wflo[3][3], Wfhi[3][3];                 // whole block

	float   output[32][3];
	float   col[4][3];
	int     submode = 0;
	int     i, alpha = 0;

	// determine alpha properties: 
	//     alpha == 0 => opaque, 
	//     alpha == 1 => bimodal (opaque or transp)
	//     alpha == 2 => partially transparent
	for (i=0; i<32; i++) 
   {
		// if alpha differs from 0 or 255 within tolerance it can still use none alpha blocks.
		if ( ainput[i] >= ( 255 - a_tolerance ) )
			ainput[i] = 255;

		if ( ainput[i] <=  a_tolerance )
			ainput[i] = 0;

		if ( ainput[i] == 0 )
			alpha = 1;
		else if ( ainput[i] != 255 )
			alpha = 2;
	}

	// whole block statistics
	eigenStatistics(32, input, output, Wflo, Wfhi, Wavg /*not used*/, Wmin, Wmax, Werr);

	if (a_force_cc != -1) 
   {
		// int loEven, loOdd;
		switch (a_force_cc) 
      {
		   case TCC_HI:
			   encodeColors( TCC_HI, 0, alpha,
			      &Wflo[0][0], &Wfhi[0][0], NULL, NULL, input, ainput, bits);
			   _cc_hi++;
			   return;

		   case TCC_MIXED:
			   submode = 0;
			   // Even, odd block statistics
			   eigenStatistics(16, (const float(*)[3]) &input[ 0][0], output, 
				     Eflo, Efhi, Eavg /*not used*/, Emin, Emax, Eerr /*not used*/);
			   eigenStatistics(16, (const float(*)[3]) &input[16][0], output, 
				     Oflo, Ofhi, Oavg /*not used*/, Omin, Omax, Oerr /*not used*/);

			   encodeColors( TCC_MIXED, submode, alpha,
			     &Eflo[0][0], &Efhi[0][0], &Oflo[0][0], &Ofhi[0][0], input, ainput, bits);
			   _cc_mixed_0++;
			   return;

		   case TCC_CHROMA:
			   vqChroma( input, alpha ? 3 : 4, col);
			   encodeColors( TCC_CHROMA, 0, 0,
				   &col[0][0], &col[1][0], &col[2][0], &col[3][0], input, ainput, bits);
			   _cc_chroma++;
			   return;

		   case TCC_ALPHA:
			   encodeAlpha( input, ainput, bits, a_lerp );
		}
		return;
	}

	if (( alpha == 2 ) || ((alpha == 1 ) && (Werr[1] >= 20))) 
   {
		// strong alpha component or strong color component with alpha use TCC_ALPHA
		// LOOOK need to determine whether to interpolate or not
		encodeAlpha( input, ainput, bits, a_lerp);
		return;
	}

	if (Werr[1] >= 20) 
   {
		/* 2nd eigenvector is strong, points to a chroma block */
		vqChroma( input, alpha ? 3 : 4, col);
		encodeColors( TCC_CHROMA, 0, alpha,
			&col[0][0], &col[1][0], &col[2][0], &col[3][0], input, ainput, bits);
		_cc_chroma++;
		return;
	}

	// Even, odd block statistics
	eigenStatistics(16, (const float(*)[3]) &input[ 0][0], output, 
		Eflo, Efhi, Eavg /*not used*/, Emin, Emax, Eerr /*not used*/);
	eigenStatistics(16, (const float(*)[3]) &input[16][0], output, 
		Oflo, Ofhi, Oavg /*not used*/, Omin, Omax, Oerr /*not used*/);

	{
		// commented out to get rid of error on VC++ 6.0
		// int loEven, loOdd;

		int skewed;

		/* 
		 * No chrominance, only intensity changes in this block, so we
		 * ignore the 2nd eigenvector 
		 *
		 * Now we decide between coding at 7 levels, or 4 levels
		 */
		skewed = (ABS(ABS(Wmin[0]) - ABS(Wmax[0])) > 32) ||
				 (ABS(ABS(Emin[0]) - ABS(Emax[0])) > 32) ||
				 (ABS(ABS(Omin[0]) - ABS(Omax[0])) > 32) ;

		// Neither half has small variations, split whole block into 8
		if (skewed) 
      {
			encodeColors( TCC_HI, 0, alpha,
			   &Wflo[0][0], &Wfhi[0][0], NULL, NULL, input, ainput, bits);
			_cc_hi++;
			return;
		} 
      else 
      {
			encodeColors( TCC_MIXED, 0, alpha,
			   &Eflo[0][0], &Efhi[0][0], &Oflo[0][0], &Ofhi[0][0], input, ainput, bits);
			_cc_mixed_0++;
			return;
		}
	}
}

void encode4bpp_block(int *pp0, 
                      int *pp1, 
                      int *pp2, 
                      int *pp3, 
                      int *code)
{
	float   input[32][3];
	FxI32   ainput[32];
   int     tcode[4];
	int     i;

	/* Convert input to input vectors */
	for (i=0; i<4; i++) 
   {
		// 1st block of 4x4
		ainput[ 0 + i] = ALF(pp0[i]);
		input[ 0 + i][0] = (float) (RED(pp0[i]));
		input[ 0 + i][1] = (float) (GRN(pp0[i]));
		input[ 0 + i][2] = (float) (BLU(pp0[i]));

		ainput[ 4 + i] = ALF(pp1[i]);
		input[ 4 + i][0] = (float) (RED(pp1[i]));
		input[ 4 + i][1] = (float) (GRN(pp1[i]));
		input[ 4 + i][2] = (float) (BLU(pp1[i]));

		ainput[ 8 + i] = ALF(pp2[i]);
		input[ 8 + i][0] = (float) (RED(pp2[i]));
		input[ 8 + i][1] = (float) (GRN(pp2[i]));
		input[ 8 + i][2] = (float) (BLU(pp2[i]));

		ainput[12 + i] = ALF(pp3[i]);
		input[12 + i][0] = (float) (RED(pp3[i]));
		input[12 + i][1] = (float) (GRN(pp3[i]));
		input[12 + i][2] = (float) (BLU(pp3[i]));

		// 2nd block of 4x4
		ainput[16 + i] = ALF(pp0[i+4]);
		input[16 + i][0] = (float) (RED(pp0[i+4]));
		input[16 + i][1] = (float) (GRN(pp0[i+4]));
		input[16 + i][2] = (float) (BLU(pp0[i+4]));

		ainput[20 + i] = ALF(pp1[i+4]);
		input[20 + i][0] = (float) (RED(pp1[i+4]));
		input[20 + i][1] = (float) (GRN(pp1[i+4]));
		input[20 + i][2] = (float) (BLU(pp1[i+4]));

		ainput[24 + i] = ALF(pp2[i+4]);
		input[24 + i][0] = (float) (RED(pp2[i+4]));
		input[24 + i][1] = (float) (GRN(pp2[i+4]));
		input[24 + i][2] = (float) (BLU(pp2[i+4]));

		ainput[28 + i] = ALF(pp3[i+4]);
		input[28 + i][0] = (float) (RED(pp3[i+4]));
		input[28 + i][1] = (float) (GRN(pp3[i+4]));
		input[28 + i][2] = (float) (BLU(pp3[i+4]));

	}
	quantize4bpp_block(input, ainput, tcode);
   memcpy(code,tcode,sizeof(int)*4);
}

#define FARGB(a, r, g, b)    (ARGB( float2int (a), float2int (r), float2int (g), float2int (b)) )

void decode4bpp_block(int *code,
                      int *pp0, 
                      int *pp1, 
                      int *pp2, 
                      int *pp3)
{
	float output[32][4];  // order AGBR
   int   tcode[4];
	int   i;

   memcpy(tcode,code,sizeof(int)*4);
	decodeColors(tcode, output);

	// Decode and put it back into source array right away!
	for (i=0; i<4; i++) 
   {
		pp0[i+0] = FARGB(output[ 0 + i][0], output[ 0+i][1], output[ 0+i][2], output[ 0+i][3]);
		pp1[i+0] = FARGB(output[ 4 + i][0], output[ 4+i][1], output[ 4+i][2], output[ 4+i][3]);
		pp2[i+0] = FARGB(output[ 8 + i][0], output[ 8+i][1], output[ 8+i][2], output[ 8+i][3]);
		pp3[i+0] = FARGB(output[12 + i][0], output[12+i][1], output[12+i][2], output[12+i][3]);

		pp0[i+4] = FARGB(output[16 + i][0], output[16+i][1], output[16+i][2], output[16+i][3]);
		pp1[i+4] = FARGB(output[20 + i][0], output[20+i][1], output[20+i][2], output[20+i][3]);
		pp2[i+4] = FARGB(output[24 + i][0], output[24+i][1], output[24+i][2], output[24+i][3]);
		pp3[i+4] = FARGB(output[28 + i][0], output[28+i][1], output[28+i][2], output[28+i][3]);
	}
}

void FXT1Encode4bpp(int *data, int width, int height, int* encoded)
{
	int x, y ;
	for (y=0; y < height; y += 4) 
   {
		for (x=0; x < width; x += 8) 
      {
			globalX = x;
			globalY = y;
			encode4bpp_block(&data[x + ((y + 0) * width)],
				              &data[x + ((y + 1) * width)],
				              &data[x + ((y + 2) * width)],
				              &data[x + ((y + 3) * width)],
				              encoded);
			encoded += 4;       // 128 bits per 8x4 block = 4bpp
		}
	}
}

void FXT1Decode4bpp(int *encoded, int width, int height, int *data)
{
	int x, y ;

	for (y=0; y < height; y += 4) 
   {
		for (x=0; x < width; x += 8) 
      {
			globalX = x;
			globalY = y;
			decode4bpp_block(encoded,
                          &data[x + ((y + 0) * width)],
                          &data[x + ((y + 1) * width)],
                          &data[x + ((y + 2) * width)],
                          &data[x + ((y + 3) * width)] );
			encoded += 4;       // 128 bits per 8x4 block = 4bpp
		}
	}
}

/***

	Routines from eigen.c

***/


#define swapValue(K, i, j)  t=K[i], K[i] = K[j], K[j] = (float) t
#define swapVector(U, i, j) swapValue(U[0], i, j); swapValue(U[1], i, j); swapValue(U[2], i, j)

// OffD[] exploits the symmetry of S[][].
static void eigenVectors (const float S[3][3], float U[3][3], float K[3])
{
#define LOSSY_OPTIMIZATIONS 0  // gets about 5%, might affect results
#if LOSSY_OPTIMIZATIONS
	// XX How can we generate the single-precision fabs Intel instruction?
	float OffD[3], sm, g, h, fabsh, fabsOffDi, t, theta;
	float c, s, tau, ta, OffDq, a, b;
#else
	double OffD[3], sm, g, h, fabsh, fabsOffDi, t, theta;
	double c, s, tau, ta, OffDq, a, b;
#endif
	int sweep, i, j, p, q;
	static int ncount = 0;
	static int mod3[]  = { 0, 1, 2, 0, 1, 2};

	for (i = 0; i < 3; i++) 
   {
		U[i][i] = 1.0f;
		K[i] = S[i][i];
		OffD[i] = S[mod3[i + 1]][mod3[i + 2]];
		for (j = i + 1; j < 3; j++) 
      {
			U[i][j] = U[j][i] = 0.0f;
		}
	}
	for (sweep = 25; sweep > 0; sweep--) 
   {
		sm = fabs(OffD[0]) + fabs(OffD[1]) + fabs(OffD[2]);
		if (sm == 0.0f) break;
		for (i = 2; i >= 0; i--) 
      {
			p = mod3[i + 1];
			q = mod3[i + 2];
	 
			fabsOffDi = fabs(OffD[i]);
			g = (float) (100.0 * fabsOffDi);
			if (fabsOffDi > 0.0) 
         {
				h = K[q] - K[p];
				fabsh = fabs(h);
				if (fabsh + g == fabsh) 
            {
					t = OffD[i] / h;
				} 
            else 
            {
					theta = 0.5 * h / OffD[i];
					t = 1.0 / (fabs(theta) + sqrt(theta * theta + 1.0));
					if (theta < 0) t = -t;
				}
				c = 1.0 / sqrt(t * t + 1);
				s = t * c;
				tau = s / (c + 1);
				ta = t * OffD[i];
				OffD[i] = 0.0;
				K[p] -= (float) ta;
				K[q] += (float) ta;
				OffDq = OffD[q];
				OffD[q] -= s * (OffD[p] + tau * OffD[q]);
				OffD[p] += s * (OffDq - tau * OffD[p]);
				for (j = 2; j >= 0; j--) 
            {
					a = U[j][p];
					b = U[j][q];
					U[j][p] -= (float) (s * (b + tau * a));
					U[j][q] += (float) (s * (a - tau * b));
				}
			}
		}
	}

	/* Eigen values can't be negative */
	if (K[0] < 0.0f) K[0] = 0.0f;
	if (K[1] < 0.0f) K[1] = 0.0f;
	if (K[2] < 0.0f) K[2] = 0.0f;

	/* 
	 * Sort eigen values in increasing order. When this is done, make sure
	 * that eigen vectors are swapped as well.
	 */
	if (K[0] < K[1]) { swapValue(K, 0, 1); swapVector(U, 0, 1); }
	if (K[0] < K[2]) { swapValue(K, 0, 2); swapVector(U, 0, 2); }
	if (K[1] < K[2]) { swapValue(K, 1, 2); swapVector(U, 1, 2); }
}

/* Create a 3x3 covariance matrix from a given nx3 data matrix */
void covariance(int n, float data[][3], float cov[3][3])
{
   int   i, j, k;

   /* Now compute cov[3][3] = Transpose(data[n][3]) * data[n][3] */
   for (i=0; i<3; i++) 
   {
      for (j=i; j<3; j++) 
      {
         cov[i][j] = 0.0f;
      }
   }
   for (k=0; k<n; k++) 
   {
      cov[0][0] += data[k][0] * data[k][0];
      cov[0][1] += data[k][0] * data[k][1];
      cov[0][2] += data[k][0] * data[k][2];
      cov[1][1] += data[k][1] * data[k][1];
      cov[1][2] += data[k][1] * data[k][2];
      cov[2][2] += data[k][2] * data[k][2];
   }
   for (i=0; i<3; i++) 
   {
      for (j=i; j<3; j++) 
      {
         cov[j][i] = cov[i][j];
      }
   }
}


void eigenProject(int n, float *idata, float m[3][3], float *odata)
{
   int i, j;

   for (i=0; i<n; i++) 
   {
      float   tdata[3];       // temporary space.

      // XX MSVC doesn't unroll loops!? 
      for (j=0; j<3; j++) 
      {
         tdata[j] =  (idata[0] * m[0][j])
                     + (idata[1] * m[1][j])
                     + (idata[2] * m[2][j]);
      }
      // This is necessary in case idata == odata
      for (j=0; j<3; j++) 
      {
         odata[i*3+j] = tdata[j];
      }
      idata += 3;
   }
}

void eigenSpace(int n, float *data, float evectors[3][3], float evalues[3])
{
   float   cov[3][3];

   covariance(n, (float (*)[3]) data, cov); 
   eigenVectors(cov, evectors, evalues);
}

/*
 * Given an input vector input[n][3], this routine computes various
 * statistics using the eigen Transform.
 */
#define NCOMPONENTS 3
void eigenStatistics(int     n, 
                     const   float input[][3], 

                     float   output[][3],        // after transformation to eigenspace
                     float   lo[3][3],           // lo and hi colors for each evector
                     float   hi[3][3],

                     float   avg[3],             // this is the mean value of this block
                     float   min[3],             // min and max values for each evector
                     float   max[3], 
                     float   errs[3])            // max error incurred when dropping each evector
{
   float   evectors[3][3], evalues[3];
   int     i, j;

   if (n < 1) 
   {
      // fprintf(stderr, "Bad n: %d (File %s)\n", n, __FILE__);
      // exit(0);
   }


   // output[i][j] = mean removed input[i][j]
   for (i=0; i<n; i++) 
   {
      for (j = 0; j < NCOMPONENTS; j++) 
      {
         output[i][j] = input[i][j]  ;       //  - avg[j];
      }
   }

   // Compute eigenValues, eigenVectors
   eigenSpace(n, (float*) output, evectors, (float*) evalues);

   // Project to eigenSpace
   eigenProject(n, (float*) output, evectors, (float *) output);


   // Find min and max values of each component in eigenSpace
   for (j = 0; j < NCOMPONENTS; j++) 
   {
      min[j] = max[j] = output[0][j];
   }
   for (i = 1; i < n; i++) 
   {
      if (min[0] > output[i][0]) min[0] = output[i][0];
      if (max[0] < output[i][0]) max[0] = output[i][0];
      if (min[1] > output[i][1]) min[1] = output[i][1];
      if (max[1] < output[i][1]) max[1] = output[i][1];
      if (min[2] > output[i][2]) min[2] = output[i][2];
      if (max[2] < output[i][2]) max[2] = output[i][2];
   }

   // Find lo and hi values for each eigenVector
   for (i = 0; i < 3; i++) 
   {
      for (j = 0; j < NCOMPONENTS; j++) 
      {
         lo[i][j] = evectors[j][i] * min[i];
         hi[i][j] = evectors[j][i] * max[i];
      }
   }

   // Compute abs(spread) of each color per eigenVector
   for (i=0; i<3; i++) 
   {

      errs[i] = 0.0f;
      for (j = 0; j < NCOMPONENTS; j++) 
      {
         float   e;

         e = lo[i][j] - hi[i][j];
         if (e < 0.0f) e = -e;           // ABS
         if (errs[i] < e) errs[i] = e;   // MAX
      }
   }
}

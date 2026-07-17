/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "conform.h"
#include "vertex.h"
#include "matrix.h"


static char *test_name = "lfb";
static char *test_description = 
 "Test LFB writes/reads of color and aux buffers";


#define DEPTH_FAR  (65535.0f/10.0f)  // for Z tests
#define DEPTH_MID  (65535.0f/50.0f)
#define DEPTH_NEAR (65535.0f/100.0f)

#define ALPHA_SMALL 0x20
#define ALPHA_MID   0x80
#define ALPHA_BIG   0xff

#define TEST_NONE   0x0
#define TEST_DEPTH  0x1
#define TEST_ALPHA  0x2
#define TEST_BOTH   (TEST_DEPTH | TEST_ALPHA)

typedef struct {
  char *name;
  FxU32 val;
} NameValuePair;

typedef struct {
  int test_pass;
  NameValuePair colorformat;
  NameValuePair lfb_writemode;
  NameValuePair pixpipe_flag;
  NameValuePair buffer;
  int debug;
  int x,y, width, height;
  int which_lfb_writemode;
  int test_alpha_depth; // flags for which of these to also test
} test_data;

NameValuePair LFB_Writemodes[] = {
    {"GR_LFBWRITEMODE_565", GR_LFBWRITEMODE_565},
    {"GR_LFBWRITEMODE_555", GR_LFBWRITEMODE_555},
    {"GR_LFBWRITEMODE_1555", GR_LFBWRITEMODE_1555},
    {"GR_LFBWRITEMODE_888", GR_LFBWRITEMODE_888},
    {"GR_LFBWRITEMODE_8888", GR_LFBWRITEMODE_8888},
    {"GR_LFBWRITEMODE_565_DEPTH", GR_LFBWRITEMODE_565_DEPTH},
    {"GR_LFBWRITEMODE_555_DEPTH", GR_LFBWRITEMODE_555_DEPTH},
    {"GR_LFBWRITEMODE_1555_DEPTH", GR_LFBWRITEMODE_1555_DEPTH},
    {"GR_LFBWRITEMODE_ZA16", GR_LFBWRITEMODE_ZA16},
};
#define NUM_LFB_WRITEMODES sizeof(LFB_Writemodes)/sizeof(NameValuePair)

NameValuePair Buffers[] = {
  {"GR_BUFFER_FRONTBUFFER", GR_BUFFER_FRONTBUFFER},
  {"GR_BUFFER_BACKBUFFER", GR_BUFFER_BACKBUFFER},
  {"GR_BUFFER_AUXBUFFER", GR_BUFFER_AUXBUFFER},
};
#define NUM_BUFFERS sizeof(Buffers)/sizeof(NameValuePair)

NameValuePair PixPipe_Flags[] = {
  {"PixPipe ON", FXTRUE},
  {"PixPipe OFF", FXFALSE},
};
#define NUM_PIXPIPE_FLAGS sizeof(PixPipe_Flags)/sizeof(NameValuePair)

FxBool pass(conform_state* state, char *s, int n) {
  char buf[256];
  sprintf(buf, "%s @ line %d", s, n);
  return log_status(state, TEST_PASS, buf);
}

FxBool fail(conform_state* state, char *s, int n) {
  char buf[256];
  sprintf(buf, "%s @ line %d", s, n);
  return log_status(state, TEST_FAIL, buf);
}

/* pack_bits (stolen from lib/lfbutils.c)
**
** Summary: Pack a 32-bit value with a series of bit fields specified in the i
**          format  Value, Width, from MSB to LSB
**
** Arguments: This function takes a variable number of arguments
**            n - number of fields to create in the 32 bit pattern
**            val<x>  - bit pattern to fill field <x> where x [0,n)
**            width<x> - width of field <x>
**
** Return Value: The packed 32 bit pattern
*/
FxU32
pack_bits( FxU32 n, ... /*val1, width1, ..., val<n>, width<n>*/ )
{
  va_list argumentPointer;
  FxU32   bitField = 0;
  FxU32   bitIndex = 0;
    
  va_start( argumentPointer, n );
    
  for ( ; n; n-- )
  {
    FxU32   bitMask;
    FxU32   fieldValue;
    FxU32   fieldWidth;
        
    fieldValue = va_arg( argumentPointer, FxU32 );
    fieldWidth = va_arg( argumentPointer, FxU32 );
        
    if ( bitIndex + fieldWidth > 32 )
    {
        fprintf(stderr,"Error: pack_bits() field width overflow." );
    }
        
    bitMask = ~0u;
    bitMask <<= fieldWidth;
    bitMask = ~bitMask;
        
    fieldValue &= bitMask;
    fieldValue <<= ( 32 - bitIndex - fieldWidth );
    bitField |= fieldValue;
        
    bitIndex += fieldWidth;
  }
    
  va_end( argumentPointer );
  return bitField;
}


/* mangle_color (stolen from lib/lfbutils.cpp)
** 
** Summary: Re-Order the Color Bits into a single 32-bit word according
**          to the lfbWriteMode
**
** Parameters: writeValue - pointer to a 32-bit storage where the mangled color
**                          will be stored.
**             pixelsToWrite - number of pixels to be written
**             lfb_writemode - the LFB write mode
**             colorformat - the color format to use
**             a1,r1,g1,b1,depth1 - data values for first pixel
**             a2,r2,g2,b2,depth2 - data values for first pixel
**             
*/
void
mangle_color( FxU32 *writeValue, FxU32 pixelsToWrite, 
             GrLfbWriteMode_t lfb_writemode, GrColorFormat_t colorformat,
             FxU32 a1, FxU32 r1, FxU32 g1, FxU32 b1, FxU32 depth1,
             FxU32 a2, FxU32 r2, FxU32 g2, FxU32 b2, FxU32 depth2 )
{
  FxU32 z1Width, z2Width;
  FxU32 a1Width, a2Width;
  FxU32 r1Width, r2Width;
  FxU32 g1Width, g2Width;
  FxU32 b1Width, b2Width;
  FxBool swapWords = FXFALSE;  // ?
  FxU32 wordsPerPixel;
  FxU32 words;
    
  // Do color lane swap and setup Colors
  switch (lfb_writemode)
  {
    case GR_LFBWRITEMODE_565:
      z1Width = z2Width = 0;
      depth1 = depth2 = 0;
      a1Width = a2Width = 0;
      a1 = a2 = 0;
      r1Width = r2Width = 5;
      r1 >>= 3;
      r2 >>= 3;
      g1Width = g2Width = 6;
      g1 >>= 2;
      g2 >>= 2;
      b1Width = b2Width = 5;
      b1 >>= 3;
      b2 >>= 3;
      wordsPerPixel = 1;
      break;
    case GR_LFBWRITEMODE_555:
      z1Width = z2Width = 0;
      depth1 = depth2 = 0;
      a1Width = a2Width = 1;
      a1 = a2 = 0;
      r1Width = r2Width = 5;
      r1 >>= 3;
      r2 >>= 3;
      g1Width = 5;
      g2Width = 5;
      g1 >>= 3;
      g2 >>= 3;
      b1Width = 5;
      b2Width = 5;
      b1 >>= 3;
      b2 >>= 3;
      wordsPerPixel = 1;
      break;
    case GR_LFBWRITEMODE_1555:
      z1Width = z2Width = 0;
      depth1 = depth2 = 0;
      a1Width = a2Width = 1;
      a1 = a1 ? 1 : 0;
      a2 = a2 ? 1 : 0;
      r1Width = r2Width = 5;
      r1 >>= 3;
      r2 >>= 3;
      g1Width = g2Width = 5;
      g1 >>= 3;
      g2 >>= 3;
      b1Width = b2Width = 5;
      b1 >>= 3;
      b2 >>= 3;
      wordsPerPixel = 1;
      break;
    case GR_LFBWRITEMODE_888: 
      z1Width = z2Width = 0;
      depth1 = depth2 = 0;
      a1Width = 8;
      a2Width = 0;
      a1 = a2 = 0;
      r1Width = 8;
      r2Width = 0;
      g1Width = 8;
      g2Width = 0;
      b1Width = 8;
      b2Width = 0;
      swapWords = FXFALSE;
      wordsPerPixel = 2;
      break;
    case GR_LFBWRITEMODE_8888:        
      z1Width = z2Width = 0;
      depth1 = depth2 = 0;
      a1Width = 8;
      a2Width = 0;
      r1Width = 8;
      r2Width = 0;
      g1Width = 8;
      g2Width = 0;
      b1Width = 8;
      b2Width = 0;
      swapWords = FXFALSE;
      wordsPerPixel = 2;
      break;
    case GR_LFBWRITEMODE_565_DEPTH:        
      z1Width = 16;
      z2Width = 0;
      a1Width = a2Width = 0;
      a1 = a2 = 0;
      r1Width = 5;
      r2Width = 0;
      r1 >>= 3;
      r2 >>= 3;
      g1Width = 6;
      g2Width = 0;
      g1 >>= 2;
      g2 >>= 2;
      b1Width = 5;
      b2Width = 0;
      b1 >>= 3;
      b2 >>= 3;
      wordsPerPixel = 2;
      break;
    case GR_LFBWRITEMODE_555_DEPTH:        
      z1Width = 16;
      z2Width = 0;
      a1Width = 1;
      a2Width = 0;
      a1 = a2 = 0;
      r1Width = 5;
      r2Width = 0;
      r1 >>= 3;
      r2 >>= 3;
      g1Width = 5;
      g2Width = 0;
      g1 >>= 3;
      g2 >>= 3;
      b1Width = 5;
      b2Width = 0;
      b1 >>= 3;
      b2 >>= 3;
      wordsPerPixel = 2;
      break;
    case GR_LFBWRITEMODE_1555_DEPTH:       
      z1Width = 16;
      z2Width = 0;
      a1Width = 1;
      a2Width = 0;
      a1 = a1 ? 1 : 0;
      a2 = a2 ? 1 : 0;
      r1Width = 5;
      r2Width = 0;
      r1 >>= 3;
      r2 >>= 3;
      g1Width = 5;
      g2Width = 0;
      g1 >>= 3;
      g2 >>= 3;
      b1Width = 5;
      b2Width = 0;
      b1 >>= 3;
      b2 >>= 3;
      wordsPerPixel = 2;
      break; 
    case GR_LFBWRITEMODE_ZA16:  
      z1Width = z2Width = 16;
      a1Width = a2Width = 0;
      a1 = a2 = 0;
      r1Width = r2Width = 0;
      r1 = r2 = 0;
      g1Width = g2Width = 0;
      g1 = g2 = 0;
      b1Width = b2Width = 0;
      b1 = b2 = 0;
      wordsPerPixel = 1;
      break;
    //case GR_LFBWRITEMODE_u1:          
    //case GR_LFBWRITEMODE_u2:  
    //case GR_LFBWRITEMODE_u3:  
    default:
      fprintf(stderr, "Error: mangle_color() unrecognized/reserved lfb write format\n" );
      break;
  }
    
  // This will be removed when I feel better about this code
  if ( z1Width + z2Width +
       a1Width + a2Width +
       r1Width + r2Width +
       g1Width + g2Width +
       b1Width + b2Width != 32 )
  {
      fprintf(stderr, "Error: mangle_color() has an internal consistency error\n" );
  }
    
  words = pixelsToWrite * wordsPerPixel;
    
  switch(colorformat)
  {
    case GR_COLORFORMAT_ARGB:
      *writeValue = pack_bits(10,
                             depth2, z2Width, a2, a2Width,
                             r2, r2Width, g2, g2Width,
                             b2, b2Width, depth1, z1Width,
                             a1, a1Width, r1, r1Width,
                             g1, g1Width, b1, b1Width );
      break;
    case GR_COLORFORMAT_ABGR:
      *writeValue = pack_bits(10,
                             depth2, z2Width, a2, a2Width,
                             b2, b2Width, g2, g2Width,
                             r2, r2Width, depth1, z1Width,
                             a1, a1Width, b1, b1Width,
                             g1, g1Width, r1, r1Width );
      break;
    case GR_COLORFORMAT_RGBA:
      *writeValue = pack_bits(10,
                             depth2, z2Width, r2, r2Width,
                             g2, g2Width, b2, b2Width,
                             a2, a2Width, depth1, z1Width,
                             r1, r1Width, g1, g1Width,
                             b1, b1Width, a1, a1Width );
      break;
    case GR_COLORFORMAT_BGRA:
      *writeValue = pack_bits(10,
                             depth2, z2Width, b2, b2Width,
                             g2, g2Width, r2, r2Width,
                             a2, a2Width, depth1, z1Width,
                             b1, b1Width, g1, g1Width,
                             r1, r1Width, a1, a1Width );
      break;
  }
    
  if ( swapWords )
  {
      if ( words == 2 )
        *writeValue = ( *writeValue << 16 ) | (*writeValue >> 16 );
  }

  return;
}


/**
 * Write one or two pixels, given RGB,A, depth ,location, and an output format.
 * @param local_ptr - the testing state
 * @param num_pixels_to_write - 1 or 2
 * @param r1 - red component of 1st pixel to write
 * @param g1 - green component of 1st pixel to write
 * @param b1 - blue component of 1st pixel to write
 * @param a1 - alpha component of 1st pixel to write
 * @param d1 - depth component of 1st pixel to write
 * @param r1 - red component of 2nd pixel to write
 * @param g1 - green component of 2nd pixel to write
 * @param b1 - blue component of 2nd pixel to write
 * @param a1 - alpha component of 2nd pixel to write
 * @param d1 - depth component of 2nd pixel to write
 * @param x - x location at which to write pixel(s)
 * @param y - y location at which to write pixel(s)
 * @return FXTRUE if pixel written, FXFALSE otherwise
 */
FxBool
write_pixel(conform_state *state, int num_pixels_to_write,
            FxU32 r1, FxU32 g1, FxU32 b1, FxU32 a1, FxU32 d1,
            FxU32 r2, FxU32 g2, FxU32 b2, FxU32 a2, FxU32 d2,
            int x, int y) {

  FxU8 *fbAddr = 0x0;
  FxU32 mangledPixel = 0x0;
  FxBool status = FXFALSE;
  test_data *local_ptr = (test_data *)state->local_data;

  GrLfbInfo_t lfbInfo; 
  lfbInfo.size = sizeof(lfbInfo);

  if (grLfbLock(GR_LFB_WRITE_ONLY,
                local_ptr->buffer.val,
                local_ptr->lfb_writemode.val,
                GR_ORIGIN_UPPER_LEFT,
                local_ptr->pixpipe_flag.val, 
                &lfbInfo ) == FXTRUE) {


    /* As of 10/98, LFB writes are always in ARGB format */
    mangle_color(&mangledPixel, num_pixels_to_write,
                 local_ptr->lfb_writemode.val, GR_COLORFORMAT_ARGB,
                 a1, r1, g1, b1, d1,  
                 a2, r2, g2, b2, d2);

    switch(local_ptr->lfb_writemode.val) {
      // 16 bit writes
      case GR_LFBWRITEMODE_565:
      case GR_LFBWRITEMODE_555:
      case GR_LFBWRITEMODE_ZA16:
      case GR_LFBWRITEMODE_1555: {
        // Compute the address of the pixel(s) we want to write    
        fbAddr = (FxU8*)(((char*)lfbInfo.lfbPtr) + 
                         (y * lfbInfo.strideInBytes) +
                         (x * sizeof(FxU16)));

        if (num_pixels_to_write == 1) {
          // write lower half of mangled pixel
          lfb_write_data(state, fbAddr, (FxU8*)&mangledPixel, sizeof(FxU16));

          if (local_ptr->debug) {
              printf("Writing pix @ (%d, %d) =  0x%04X\n", 
                      x, y, mangledPixel);
          }
        } else { // write 2 packed 16 bit pixels

          lfb_write_data(state, fbAddr, (FxU8*)&mangledPixel, sizeof(FxU32));
          if (local_ptr->debug) {
            printf("Writing 32b pix @ (%d, %d) =  0x%08X\n", 
                    x, y, mangledPixel);
          }
        }
        break;
      }

      // 32 bit writes
      case GR_LFBWRITEMODE_888:
      case GR_LFBWRITEMODE_8888:
      case GR_LFBWRITEMODE_565_DEPTH:
      case GR_LFBWRITEMODE_555_DEPTH:
      case GR_LFBWRITEMODE_1555_DEPTH:
      case GR_LFBWRITEMODE_ANY:
      default: {
        FxU8 *fbAddr = (FxU8*)(((char*)lfbInfo.lfbPtr) + 
                                 (y * lfbInfo.strideInBytes) +
                                 (x * sizeof(FxU32)));

        lfb_write_data(state, fbAddr, (FxU8*)&mangledPixel, sizeof(FxU32));

        if (local_ptr->debug) {
          printf("write32 0x%X @ 0x%X (%d, %d)\n", mangledPixel, fbAddr, x, y); 
        }

        break;  
      }
    }
    grLfbUnlock(GR_LFB_WRITE_ONLY, local_ptr->buffer.val);
    status = FXTRUE;
  } else {
    fprintf(stderr, "Couldn't get lfb lock in write_one_pixel()!\n");
  }
  return status;
}

/**
 * Simple wrapper around write_pixel() for writing 1 pixel.
 * Write one pixel, given RGB,A, depth ,location, and an output format.
 * @param r - red component of pixel to write
 * @param g - green component of pixel to write
 * @param b - blue component of pixel to write
 * @param a - alpha component of pixel to write
 * @param d - depth component of pixel to write
 * @param x - x location at which to write pixel
 * @param y - y location at which to write pixel
 * @param ts - the testing state
 * @return FXTRUE if pixel written, FXFALSE otherwise
 */
FxBool
write_one_pixel(conform_state *state, 
                FxU32 r1, FxU32 g1, FxU32 b1, FxU32 a1, FxU32 d1,
                int x, int y) {

  return write_pixel(state, 1,
                     r1, g1, b1, a1, d1, 
                     0, 0, 0, 0, 0,
                     x, y);
}
/**
 * Read one pixel back from the LFB and return it in it's native
 * format.
 * @param buffer - the buffer from which to read.
 * @param x, y   - the location at which to read.
 * @param pixpipe_flag - if FXTRUE, enable pixel pipeline 
 *                      (ignored by glide, but what the heck)
 * @param pix    - the location to copy the returned pixel into.
 * @return FXTRUE if the read was succesful, FXFALSE otherwise.
 */
FxBool
read_one_pixel(conform_state *state, int x, int y, FxU16 *pix) {

  FxU8 *fbAddr = 0x0;
  FxBool status = FXFALSE;
  test_data *local_ptr = (test_data *)state->local_data;

  GrLfbInfo_t lfbInfo; 
  lfbInfo.size = sizeof(lfbInfo);
  if (grLfbLock(GR_LFB_READ_ONLY,
                local_ptr->buffer.val,
                GR_LFBWRITEMODE_565, //GR_LFBWRITEMODE_ANY,
                GR_ORIGIN_UPPER_LEFT,
                local_ptr->pixpipe_flag.val, // ignored for reads
                &lfbInfo ) == FXTRUE) {

    *pix = (FxU16)0xDDDD; // init to known value before read, just in case

    fbAddr = (FxU8*)(((char*)lfbInfo.lfbPtr) + 
                     (y * lfbInfo.strideInBytes) +
                     (x * sizeof(FxU16)));
    lfb_read_data(state, (FxU8*)pix, fbAddr, sizeof(FxU16));

    grLfbUnlock(GR_LFB_READ_ONLY, local_ptr->buffer.val);

    if (local_ptr->debug) { 
      printf("Read pix @ 0x%X (%d, %d) =  0x%08X\n", fbAddr, x, y, *pix);
    }

    status = FXTRUE;
  } else {
    fprintf(stderr,"read_one_pixel: Couldn't get READ_ONLY lock.\n");
  }

  return status;
}


/**
 * Compare a pixel read from the LFB to some RGB vals. This
 * just does the appropriate shifing & clamping, and return FXTRUE
 * if the pixel has the same RGB. It only compares the 5 most
 * significant bits of each color, since we could've written
 * in 555 mode.
 *
 * NOTE: this comparison assumes truncation, not dithering. 
 *       grDitherMode() must be disabled for these comparisons
 *        to work.
 * 
 * @param colorformat - the colorformat we are expecting the LFB 
 *                      pixel to adhere to.
 * @param pix - a 565 pixel (as from the LFB)
 * @param r,g,b - the colors to which to compare the colors from
 *                pix.
 */
FxBool 
compare_pixel_to_RGB(test_data* local_ptr, 
                  FxU16 pix, FxU32 r, FxU32 g, FxU32 b) {
  FxU32 rPix = 0;
  FxU32 gPix = 0;
  FxU32 bPix = 0;

  // mask off the low bits of the RGB's passed in to match
  // our 555 precision.

  r = (r >> 3) & 0x1F;
  g = (g >> 3) & 0x1F;
  b = (b >> 3) & 0x1F;

  // Now unpack the pixel from the LFB

  /* As of 10/98, LFB reads only return data in ARGB format */

  rPix = (pix >> 11) & 0x1F;
  gPix = (pix >> 6) & 0x1F;
  bPix = (pix & 0x1F);


  if (local_ptr->debug) {
    printf("Comparing RGB to pix: = 0x%02X,0x%02X,0x%02X / 0x%02X,0x%02X,0x%02X\n",
           r, g, b, rPix, gPix, bPix);
  }
  return ((rPix == r) && (gPix == g) && (bPix == b));
}


/**
 * Given a string (from the command line), figure out which LFBWRITEMODE
 * it represents. 
 */
FxBool
get_lfb_writemode(test_data *local_ptr, char *arg)
{
  FxBool status = FXTRUE;

  if (!_stricmp(arg, "565")) {
    local_ptr->lfb_writemode.val = GR_LFBWRITEMODE_565;
    local_ptr->lfb_writemode.name = "GR_LFBWRITEMODE_565";
    local_ptr->which_lfb_writemode = 0;
  } else if (!_stricmp(arg, "555")) {
    local_ptr->lfb_writemode.val = GR_LFBWRITEMODE_555;
    local_ptr->lfb_writemode.name = "GR_LFBWRITEMODE_555";
    local_ptr->which_lfb_writemode = 1;
  } else if (!_stricmp(arg, "1555")) {
    local_ptr->lfb_writemode.val = GR_LFBWRITEMODE_1555;
    local_ptr->lfb_writemode.name = "GR_LFBWRITEMODE_1555";
    local_ptr->which_lfb_writemode = 2;
  } else if (!_stricmp(arg, "888")) {
    local_ptr->lfb_writemode.val = GR_LFBWRITEMODE_888;
    local_ptr->lfb_writemode.name = "GR_LFBWRITEMODE_888";
    local_ptr->which_lfb_writemode = 3;
  } else if (!_stricmp(arg, "8888")) {
    local_ptr->lfb_writemode.val = GR_LFBWRITEMODE_8888;
    local_ptr->lfb_writemode.name = "GR_LFBWRITEMODE_8888";
    local_ptr->which_lfb_writemode = 4;
  } else if (!_stricmp(arg, "565D")) {
    local_ptr->lfb_writemode.val = GR_LFBWRITEMODE_565_DEPTH;
    local_ptr->lfb_writemode.name = "GR_LFBWRITEMODE_565_DEPTH";
    local_ptr->which_lfb_writemode = 5;
  } else if (!_stricmp(arg, "555D")) {
    local_ptr->lfb_writemode.val = GR_LFBWRITEMODE_555_DEPTH;
    local_ptr->lfb_writemode.name = "GR_LFBWRITEMODE_555_DEPTH";
    local_ptr->which_lfb_writemode = 6;
  } else if (!_stricmp(arg, "1555D")) {
    local_ptr->lfb_writemode.val = GR_LFBWRITEMODE_1555_DEPTH;
    local_ptr->lfb_writemode.name = "GR_LFBWRITEMODE_1555_DEPTH";
    local_ptr->which_lfb_writemode = 7;
  } else if (!_stricmp(arg, "ZA16")) {
    local_ptr->lfb_writemode.val = GR_LFBWRITEMODE_ZA16;
    local_ptr->lfb_writemode.name = "GR_LFBWRITEMODE_ZA16";
    local_ptr->which_lfb_writemode = 8;
  } else {
    // default
    local_ptr->lfb_writemode.val = GR_LFBWRITEMODE_888;
    local_ptr->lfb_writemode.name = "GR_LFBWRITEMODE_888";
    local_ptr->which_lfb_writemode = 0;
    status = FXFALSE;
  }
  return status;
}

FxBool
get_colorformat(test_data *local_ptr, int cf) 
{
  FxBool status = FXTRUE;
  
  switch (cf) {
    case GR_COLORFORMAT_ARGB:
      local_ptr->colorformat.val = GR_COLORFORMAT_ARGB;
      local_ptr->colorformat.name = "GR_COLORFORMAT_ARGB";
      break;
    case GR_COLORFORMAT_ABGR:
      local_ptr->colorformat.val = GR_COLORFORMAT_ABGR;
      local_ptr->colorformat.name = "GR_COLORFORMAT_ABGR";
      break;
    case GR_COLORFORMAT_BGRA:
      local_ptr->colorformat.val = GR_COLORFORMAT_BGRA;
      local_ptr->colorformat.name = "GR_COLORFORMAT_BGRA";
      break;
    case GR_COLORFORMAT_RGBA:
      local_ptr->colorformat.val = GR_COLORFORMAT_RGBA;
      local_ptr->colorformat.name = "GR_COLORFORMAT_RGBA";
      break;
    default:
      local_ptr->colorformat.val = GR_COLORFORMAT_ARGB;
      local_ptr->colorformat.name = "GR_COLORFORMAT_ARGB";
      status = FXFALSE;
      break;
  }
  return status;
}

/**
 * Write and Read back one pixel. Compare the read back val to
 * the RGB we wrote (truncating and rearranging it appropriately,
 * etc).
 * @param x, y  - where on the LFB to write/read the pixel.
 * @param ts - the testing state
 * @return FXTRUE if same RGB read back, FXFALSE otherwise.
 */
FxBool 
test_write_and_read(conform_state *state, test_data *local_ptr, int x, int y)
{

  FxBool status = FXFALSE;
  FxU32 r1 = nrand(0xF0) + 0xF;
  FxU32 g1 = nrand(0xF0) + 0xF;
  FxU32 b1 = nrand(0xF0) + 0xF;

  putchar('.');

  // test one pixel per write.
  if (!write_one_pixel(state,
                       r1, g1, b1, 0xa0, 0xd0,
                       x, y)) {
    fail(state, "write pixel", __LINE__);
  } else {
    FxU16 pix = 0;
    if (!read_one_pixel(state, x, y, &pix)) {
      fail(state, "read pixel", __LINE__);
    } else {
      // Compare the read back values to the ones we wrote.
      if (compare_pixel_to_RGB(local_ptr, pix, r1, g1, b1)) {
        status = FXTRUE;
        pass(state, "write and read pixel", __LINE__);
      } else {
        fail(state, "write and read pixel", __LINE__);
      }
    }
  }
  return status;
}
/**
 * Write two pixels (packed in a single 32bit write) and read them back.
 * Compare the read back vals to the ones we wrote (truncating and rearranging
 * them appropriately, etc).
 * @param x, y  - where on the LFB to write/read the pixel.
 * @param ts - the testing state
 * @return FXTRUE if same RGB read back, FXFALSE otherwise.
 */
FxBool 
test_packed_write_and_read(conform_state *state, int x, int y)
{
  test_data *local_ptr = (test_data *)state->local_data;
  FxBool status = FXFALSE;
  FxU32 r1 = nrand(0xF0) + 0xF;
  FxU32 g1 = nrand(0xF0) + 0xF;
  FxU32 b1 = nrand(0xF0) + 0xF;

  FxU32 r2 = nrand(0xF0) + 0xF;
  FxU32 g2 = nrand(0xF0) + 0xF;
  FxU32 b2 = nrand(0xF0) + 0xF;

  putchar('p');

  // Since we're writing 2 pixels, make sure there's room. Also
  // make sure we'll write on a 32bit boundary (even pixel).
  x &= ~0x1; // round down to even pixel

  if (x >= (local_ptr->x + local_ptr->width)) {
    x -= 2;
  }

  // test two pixels per write.
  if (!write_pixel(state, 2,
                   r1, g1, b1, 0xa0, 0xd0,
                   r2, g2, b2, 0xa0, 0xd0,
                   x, y)) {
    fail(state, "write packed pixel", __LINE__);
  } else {
    FxU16 pix1 = 0;
    FxU16 pix2 = 0;
    // Read back what got written to the LFB
    if (!read_one_pixel(state, x, y, &pix1) ||
        !read_one_pixel(state, x+1, y, &pix2)) {
      fail(state, "read pixel", __LINE__);
    } else {
      // Compare the read back values to the ones we wrote.
      if (compare_pixel_to_RGB(local_ptr, pix1, r1, g1, b1) &&
          compare_pixel_to_RGB(local_ptr, pix2, r2, g2, b2)) {
        status = FXTRUE;
        pass(state, "write and read packed pixel", __LINE__);
      } else {
        fail(state, "write and read packed pixel", __LINE__);
      }
    }
  }
  return status;
}


/**
 * Write and Read back one pixel outside a ClipWindow. Compare the read
 * back val to the RGB we wrote (truncating and rearranging them it 
 * appropriately, etc). If the pixel pipeline is ON, we shouldn't have
 * been able to write the pixel (it'd be clipped). If the pipeline is
 * OFF, we should be able to write the pixel.
 *
 * @param x, y  - where on the LFB to write/read the pixel.
 * @param ts - the testing state
 * @return FXTRUE if same RGB read back, FXFALSE otherwise.
 */
FxBool 
test_clipped_write_and_read(conform_state *state, test_data *local_ptr)
{

  FxBool status = FXFALSE;
  FxU32 r1 = nrand(0xF0) + 0xF;  // colors to write
  FxU32 g1 = nrand(0xF0) + 0xF; 
  FxU32 b1 = nrand(0xF0) + 0xF;
  FxU16 pix = 0;                 // read back value

  putchar('c');

  // Set the ClipWindow to be the upper left quarter of our working
  // area.
  grClipWindow(local_ptr->x,
               local_ptr->y,
               local_ptr->x + local_ptr->width/2,
               local_ptr->y + local_ptr->height/2);

  // Pick the location of the pixel we'll try to write. Make sure
  // it's outside of the clip window.
  int x = local_ptr->x + local_ptr->width/2 + 1;
  int y = local_ptr->y + local_ptr->height/2 + 1;

  // test one pixel per write.
  if (!write_one_pixel(state,
                       r1, g1, b1, 0xa0, 0xd0,
                       x, y) ||
      !read_one_pixel(state, x, y, &pix)) {
    fail(state, "write or read clipped pixel", __LINE__);
  } else {
    FxBool was_written = compare_pixel_to_RGB(local_ptr, pix, r1, g1, b1);

    // If the pixel pipeline is ON, the pixel should NOT have been written.
    // If the pixel pipeline if OFF, the pixel should have been written.
    if (((local_ptr->pixpipe_flag.val == FXTRUE) &&  !was_written) ||
        ((local_ptr->pixpipe_flag.val == FXFALSE) && was_written)) {
          status = FXTRUE;
          pass(state, "clipped pixel write", __LINE__);
      } else {
          fail(state, "clipped pixel write", __LINE__);
    }
  }

  // Reset the clip window to the whole working area.
  grClipWindow(local_ptr->x,
               local_ptr->y,
               local_ptr->x + local_ptr->width,
               local_ptr->y + local_ptr->height);
  
  return status;
}
/**
 * A utility for testing z-buffering.
 * Write two pixels at different depths, then read back from
 * the same location. Return value indicates which, if any,
 * of the two pixels ended up in the LFB.
 * 
 * @return the index of the pixel (1 or 2) that remained in
 *         in the LFB after both writes, or 0 if neither 
 *         pixel was written.
 */
FxU32
test_two_writes(conform_state *state,
                FxU32 a1, FxU32 a2,
                float d1, float d2) {  

  test_data *local_ptr = (test_data *)state->local_data;
  int winner = 0; // no winner

  int x = nrand(local_ptr->width-1) + local_ptr->x;
  int y = nrand(local_ptr->height-1) + local_ptr->y;

  FxU32 r1 = 0xbf;
  FxU32 g1 = 0xaf;
  FxU32 b1 = 0xdf;

  FxU32 r2 = 0xaf;
  FxU32 g2 = 0xdf;
  FxU32 b2 = 0xcf;
   
  FxU16 pix;

  if (write_one_pixel(state, r1, g1, b1, a1, (FxU32)d1,
                      x, y) &&
      write_one_pixel(state, r2, g2, b2, a2, (FxU32)d2,
                    x, y) &&
      read_one_pixel(state, x, y, &pix)) {

    // See which one won.
    if (compare_pixel_to_RGB(local_ptr, pix, r1, g1, b1)) {
      winner = 1;
    } else if (compare_pixel_to_RGB(local_ptr, pix, r2, g2, b2)) {
      winner = 2;
    } else {
      printf("Neither pixel was read back!\n");
    }
  } else {
    fprintf(stderr, "test_two_writes(): Couldn't test pixels.\n");
  }
  return winner;
}

/*-----------------------------------------------------------------
@func test_alpha
@arg conform_state *state - the conformance framework state
@arg test_data *local_ptr - the test-specific state
@arg int x,y - the LFB location at which to test a pixel.
@html
Test that alpha testing works with the pixel pipeline ON (and
verify that it doesn't with the pipeline OFF). Test constant
alpha as well as writing pixels with alpha information. 
@end
-------------------------------------------------------------------*/
void
test_alpha(conform_state *state, test_data *local_ptr, int x, int y){

    // Disable depth anything
    grDepthBufferMode(GR_DEPTHBUFFER_DISABLE );
    grDepthMask(FXFALSE);

    grColorMask(FXTRUE, FXTRUE); 

    grAlphaTestFunction(GR_CMP_ALWAYS);

    // Tell the alpha combine unit to use iterated alpha
    grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, 
                   GR_COMBINE_FACTOR_NONE, 
                   GR_COMBINE_LOCAL_ITERATED, 
                   GR_COMBINE_OTHER_NONE, 
                   FXFALSE );

    // Tell the color combine unit to use iterated alpha
    grColorCombine(GR_COMBINE_FUNCTION_LOCAL, 
                   GR_COMBINE_FACTOR_NONE, 
                   GR_COMBINE_LOCAL_ITERATED, 
                   GR_COMBINE_OTHER_NONE, 
                   FXFALSE );

    // clear the buffer to black
    grBufferClear(0, 0, 0); // color, alpha, depth 

    // First test that disabling alpha testing works.
    putchar('a');
  
    // Now write two pixels with two different alphas and see
    // which one ends up staying. Since we've disabled alpha
    // testing, the second pixel should always win. This 
    // should also be true if alpha testing is on and the
    // pixel pipeline is off. When writing without alpha
    // information in the pixel, it'll use the ConstantAlpha
    // value.
  
    grAlphaTestReferenceValue(ALPHA_MID);
    grLfbConstantAlpha(ALPHA_SMALL);

    int winner = -1;
    if (winner = test_two_writes(state, 
                                 ALPHA_BIG, ALPHA_SMALL,  
                                 DEPTH_FAR, DEPTH_NEAR) !=2 ) {
        // if Zbuffering wasn't OFF, FAR [1] would have won.
        fail(state, "alphatest - wrong pixel written", __LINE__);
    }
 
    // Enable alpha testing
    putchar('a');
    grBufferClear(0, 0, 0); // color, alpha, depth 

    grAlphaTestFunction(GR_CMP_LESS); // smaller wins
    grAlphaTestReferenceValue(ALPHA_MID); 
    grLfbConstantAlpha(ALPHA_SMALL);
  
    winner = test_two_writes(state, 
                             ALPHA_SMALL, ALPHA_BIG,  
                             DEPTH_FAR, DEPTH_NEAR);
    // If the pixel pipeline is ON, then pixel #1 (SMALL) should
    // have won. If the pipeline is OFF, then alpha testing is off,
    // and #2 (BIG) should win. Note that for writeFormats with
    // no depth information, the alpha is taken from the ConstantAlpha.
    // This means that both pixels will write at the same depth for
    // this case, and since the test is CMP_LESS, then the second pixel
    // shouldn't draw if the pipeline is ON.
    if ( ((local_ptr->pixpipe_flag.val == FXTRUE) && (winner == 1)) ||
         ((local_ptr->pixpipe_flag.val == FXFALSE) && (winner == 2)) ) {
        pass(state, "alphatest", __LINE__);
    } else {
        fail(state, "alphatest - wrong pixel written", __LINE__);
    }

    // Some more ConstantAlpha testing.
    switch(local_ptr->lfb_writemode.val) {
      case GR_LFBWRITEMODE_1555:
        // Skip this one for the moment.
        break;
      // these modes have alpha information
      case GR_LFBWRITEMODE_8888:

        putchar('a');
        grBufferClear(0, 0, 0); // color, alpha, depth 
        grAlphaTestFunction(GR_CMP_GREATER); // smaller wins
        grAlphaTestReferenceValue(ALPHA_MID); 
        grLfbConstantAlpha(ALPHA_SMALL);

        winner = test_two_writes(state, 
                                 ALPHA_BIG, ALPHA_SMALL,  
                                 DEPTH_MID, DEPTH_MID);

        // If the pixelpipeline is ON, then pixel #1 should 
        // win. If the pipeline is OFF, then pixel #2 should win.
        if (((local_ptr->pixpipe_flag.val==FXTRUE) && (winner==1))||
            ((local_ptr->pixpipe_flag.val==FXFALSE) && (winner==2))){
             pass(state, "alphatest (with alpha data) test", __LINE__);
        } else {
             fail(state, "alphatest - wrong pixel written", __LINE__);
        }

        putchar('a');
        grBufferClear(0, 0, 0); // color, alpha, depth 
        grAlphaTestFunction(GR_CMP_EQUAL); // smaller wins
        grAlphaTestReferenceValue(ALPHA_MID); 
        grLfbConstantAlpha(ALPHA_SMALL);

        winner = test_two_writes(state, 
                                 ALPHA_MID, ALPHA_BIG,  
                                 DEPTH_MID, DEPTH_NEAR);
        // If the pixelpipeline is ON, then pixel #1 should 
        // win. If the pipeline is OFF, then pixel #2 should win.
        if (((local_ptr->pixpipe_flag.val==FXTRUE) && (winner==1))||
            ((local_ptr->pixpipe_flag.val==FXFALSE) && (winner==2))){
             pass(state, "alphatest (with alpha data) test", __LINE__);
        } else {
             fail(state, "alphatest - wrong pixel written", __LINE__);
        }
            
        break;
      default: // no alpha information in pixel, uses ConstantAlpha
        FxU32 r = 0xbf;
        FxU32 g = 0xaf;
        FxU32 b = 0xdf;
        FxU16 pix = 0xffff;
        putchar('a');

        grBufferClear(0, 0, 0); // color, alpha, depth 
        grAlphaTestFunction(GR_CMP_LESS); // smaller wins
        grAlphaTestReferenceValue(ALPHA_MID); 
        grLfbConstantAlpha(ALPHA_BIG);

        grLfbConstantDepth((FxU32)DEPTH_FAR);
        if (!write_one_pixel(state, r, g, b, ALPHA_BIG, (FxU32)DEPTH_NEAR,
                           x, y) ||
            !read_one_pixel(state, x, y, &pix)) {
            fail(state, "write or read failed", __LINE__);
        } else {
            FxBool wroteIt = 
                compare_pixel_to_RGB(local_ptr, pix, r, g, b);

            // If the pipeline is ON, then we shouldn't have
            // written the pixel. If the pipeline is OFF, then
            // we should have.
            if (((local_ptr->pixpipe_flag.val == FXTRUE) && 
                 (wroteIt == FXFALSE)) ||
                ((local_ptr->pixpipe_flag.val == FXFALSE) && 
                 (wroteIt == FXTRUE))) {
                pass(state, "pixel vs. reference alpha", __LINE__);
            } else {
                fail(state, "pixel vs. reference alpha", __LINE__);
            }
        }
        break;
    }
    grAlphaTestFunction(GR_CMP_ALWAYS); // turn off alpha testing 
}

/**
 * Test that ZBuffering works with the pixel pipeline ON (and
 * verify that it doesn't with the pipeline OFF). Test constant
 * depth as well as writing pixels with depth information. 
 * @param x,y - the LFB location at which to test a pixel.
 * @ts - the TestingState
 */
void
test_zbuffering(conform_state *state, test_data *local_ptr, int x, int y){

    // Disable alpha anything
    grAlphaTestFunction(GR_CMP_ALWAYS);
    grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
                         GR_BLEND_ONE, GR_BLEND_ZERO);
    grColorMask(FXTRUE, FXFALSE);

    // Set up Z-buffering. 
    FxU32 zrange[2];
    grGet(GR_ZDEPTH_MIN_MAX, 8, (long*)zrange);  
    grDepthMask(FXTRUE);

    // clear the buffer to black
    grBufferClear(0, 0, zrange[1]); // color, alpha, depth 

    // First test that disabling z-buffering works.
    grDepthBufferMode(GR_DEPTHBUFFER_DISABLE );
    putchar('z');
  
    // Now write two pixels at two different depths and see
    // which one ends up staying. Since we've disabled depth
    // buffering, the second pixel should always win. This 
    // should also be true if depth buffering is on and the
    // pixel pipeline is off. When writing without depth
    // information in the pixel, it'll use the ConstantDepth
    // value.
  
  
    grDepthBufferFunction(GR_CMP_GREATER); // farther wins
    grLfbConstantDepth((FxU32)DEPTH_MID);
    int winner = -1;
    if (winner = test_two_writes(state, 
                                 ALPHA_MID, ALPHA_MID,  
                                 DEPTH_FAR, DEPTH_NEAR) !=2 ) {
        // if Zbuffering wasn't OFF, FAR [1] would have won.
        fail(state, "depth test - wrong pixel written", __LINE__);
    }
  
    // Enable z-buffering.
    putchar('Z');
    grBufferClear(0, 0, zrange[1]); // color, alpha, depth 
    grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER );
    grDepthBufferFunction(GR_CMP_GREATER); // farther wins
  
    grLfbConstantDepth((FxU32)DEPTH_MID);
  
    winner = test_two_writes(state, 
                             ALPHA_MID, ALPHA_MID,  
                             DEPTH_FAR, DEPTH_NEAR);
    // If the pixel pipeline is ON, then pixel #1 (FAR) should
    // have won. If the pipeline is OFF, then Zbuffering is off,
    // and #2 (NEAR) should win. Note that for writeFormats with
    // no depth information, the depth is taken from the ConstantDepth.
    // This means that both pixels will write at the same depth for
    // this case, and since the test is GREATER, then the second pixel
    // shouldn't draw if the pipeline is ON.
    if ( ((local_ptr->pixpipe_flag.val == FXTRUE) && (winner == 1)) ||
         ((local_ptr->pixpipe_flag.val == FXFALSE) && (winner == 2)) ) {
        pass(state, "depth comparison", __LINE__);
    } else {
        fail(state, "depth test - wrong pixel written", __LINE__);
    }

    // Some more ConstantDepth testing.
    switch(local_ptr->lfb_writemode.val) {
      // these modes have depth information
      case GR_LFBWRITEMODE_565_DEPTH:
      case GR_LFBWRITEMODE_555_DEPTH:
      case GR_LFBWRITEMODE_1555_DEPTH:

        putchar('Z');
        grBufferClear(0, 0, (FxU32)DEPTH_MID); // color, alpha, depth 
        grDepthBufferFunction(GR_CMP_GREATER); // farther wins
        winner = test_two_writes(state, 
                                 ALPHA_MID, ALPHA_MID,  
                                 DEPTH_FAR, DEPTH_NEAR);

        // If the pixelpipeline is ON, then pixel #1 should 
        // win. If the pipeline is OFF, then pixel #2 should win.
        if (((local_ptr->pixpipe_flag.val==FXTRUE) && (winner==1))||
            ((local_ptr->pixpipe_flag.val==FXFALSE) && (winner==2))){
             pass(state, "depth comparison (with depth data) test", __LINE__);
        } else {
             fail(state, "depth test - wrong pixel written", __LINE__);
        }

        putchar('Z');
        grBufferClear(0, 0, (FxU32)DEPTH_MID); // color, alpha, depth 
        grDepthBufferFunction(GR_CMP_EQUAL); // same wins
        winner = test_two_writes(state, 
                                 ALPHA_MID, ALPHA_MID,  
                                 DEPTH_MID, DEPTH_NEAR);
        // If the pixelpipeline is ON, then pixel #1 should 
        // win. If the pipeline is OFF, then pixel #2 should win.
        if (((local_ptr->pixpipe_flag.val==FXTRUE) && (winner==1))||
            ((local_ptr->pixpipe_flag.val==FXFALSE) && (winner==2))){
             pass(state, "depth comparison (with depth data) test", __LINE__);
        } else {
             fail(state, "depth test - wrong pixel written", __LINE__);
        }
            
        break;
      default: // no depth information in pixel, uses ConstantDepth
        FxU32 r = 0xbf;
        FxU32 g = 0xaf;
        FxU32 b = 0xdf;
        FxU16 pix = 0xffff;
        putchar('Z');
        grBufferClear(0, 0, (FxU32)DEPTH_MID); // color, alpha, depth 
        grDepthBufferFunction(GR_CMP_LESS); // nearer wins
        grLfbConstantDepth((FxU32)DEPTH_FAR);
        if (!write_one_pixel(state, r, g, b, 0xFF, (FxU32)DEPTH_NEAR,
                           x, y) ||
            !read_one_pixel(state, x, y, &pix)) {
            fail(state, "write or read failed", __LINE__);
        } else {
            FxBool wroteIt = 
                compare_pixel_to_RGB(local_ptr, pix, r, g, b);

            // If the pipeline is ON, then we shouldn't have
            // written the pixel. If the pipeline is OFF, then
            // we should have.
            if (((local_ptr->pixpipe_flag.val == FXTRUE) && 
                 (wroteIt == FXFALSE)) ||
                ((local_ptr->pixpipe_flag.val == FXFALSE) && 
                 (wroteIt == FXTRUE))) {
                pass(state, "pixel vs. clear depth", __LINE__);
            } else {
                fail(state, "pixel vs. clear depth", __LINE__);
            }
        }
        break;
    }
}

/**
 * Write and read some AUXBUFFER values.
 */
void
test_aux_buffer_access(conform_state *state, test_data *local_ptr, int x, int y){
    FxU16 pix;
    FxU16 depthFar = (FxU16)DEPTH_FAR;
    FxU16 depthNear = (FxU16)DEPTH_NEAR;

    grDepthMask(FXTRUE);
    grBufferClear(0, 0, (FxU32)DEPTH_MID); // color, alpha, depth 
    grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER );
    grDepthBufferFunction(GR_CMP_GREATER); // farther wins


    // If the pipeline is OFF, we write the pixel. If the
    // pipeline is ON and working, then we write the pixel.
    putchar('A'); 
    if (!write_one_pixel(state, 0, 0, 0, 0xa, depthFar, x, y) ||
        !read_one_pixel(state, x, y, &pix)) {
        fail(state, "read/write", __LINE__); 
    } else {
        if (pix == depthFar) {
            pass(state, "depth comparison == pass or pixpipe OFF write", __LINE__);
        } else {   
            fail(state, "depth comparison == pass or pixpipe OFF write", __LINE__);
        }
    }

    // Now try a pixel that we shouldn't be able to write if
    // the pipeline is on. Depth at this pixel should currently
    // be depthFar (since we just put it there...)
    putchar('A'); 
    if (!write_one_pixel(state, 0, 0, 0, 0xa, depthNear, x, y) ||
        !read_one_pixel(state, x, y, &pix)) {
        fail(state, "write or read", __LINE__); 
    } else {
        if ( ((local_ptr->pixpipe_flag.val == FXTRUE) &&  // pipe ON, no write
             (pix == depthFar)) ||
            ((local_ptr->pixpipe_flag.val == FXFALSE) && // pipe ON, write
             (pix == depthNear)) ) {
            pass(state, "pixpipe state / depth comparison write", __LINE__);
        } else {
            fail(state, "pixpipe state / depth comparison write", __LINE__);
        }
    }
}

/**
 **  Framework entrypoints
 **/

int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;
  int num_arg;

  state->test_name = test_name;
  state->test_description = test_description;

  if((state->local_data = calloc(1, sizeof(test_data))) == NULL) {
    // Could not allocate data for local memory
    log_perror(state, "Could not allocate memory for local data");
    return(0);
  } 

  // setup image file names
  // this is manditory
  frame_name(test_name,state);
  img_file_type(IMG_P6,state);

  // Set some default local state
  local_ptr = (test_data *)state->local_data;
  local_ptr->debug = 0; 
  local_ptr->test_alpha_depth = TEST_DEPTH; // TEST_BOTH when alpha working

  // default to WRITEMODE_565 if none specified on the command line
  get_lfb_writemode(local_ptr, "565"); 

  // The default test should try to cycle through all of the 
  // LFB writemodes.
  if (state->num_reps == 1) {
    state->num_reps = NUM_LFB_WRITEMODES;
  }

  // handle our test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    if(!_stricmp(*local_argv,"-wm") || // a specific LFB_WRITEMODE specified
      !_stricmp(*local_argv,"/wm")) {
      ++local_argv;
      ++num_arg;
      // if a valid LFB writemode was specified, only do one pass.
      if (get_lfb_writemode(local_ptr, (char *)*local_argv)) {
        state->num_reps = 1;
      }
      --local_argc;
    } else if(!_stricmp(*local_argv,"-debug") || // enable debugging output
              !_stricmp(*local_argv,"/debug")) {
      local_ptr->debug = 1;
    } else if(!_stricmp(*local_argv, "-mode") || // 
              !_stricmp(*local_argv, "/mode")) {
      ++local_argv;
      ++num_arg;
      char tmp_buf[64];
      if(!sscanf(*local_argv, "%s", tmp_buf)) {
        sprintf(tmp_buf, "invalid argument after '-mode'");
        log_message(state, tmp_buf);
      } else {
        if (!_stricmp("alpha", tmp_buf)) {
          local_ptr->test_alpha_depth = TEST_ALPHA;
        } else if (!_stricmp("depth", tmp_buf)) {
          local_ptr->test_alpha_depth = TEST_DEPTH;
        } else if (!_stricmp("both", tmp_buf)) {
          local_ptr->test_alpha_depth = TEST_BOTH;
        } else {
          log_message(state, "unsupported mode specified");
        }
      }
      --local_argc;
    } else {
       fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
       ++local_argv;
       ++num_arg;
    }
    ++local_argv;
    ++num_arg;
  }

  // need an aux buffer 
  if (state->num_aux_bufs < 1) {
    state->num_aux_bufs = 1;  
  }
  
  // we need two color buffers 
  if (state->num_color_bufs < 2) {
    state->num_color_bufs = 2;
  }

  // xycheck and xylimit not supported (fix this when adding self-checking)
  state->xycheck = 0;
  state->xylimit = 0;

  return(-1);
}

void close_conform(conform_state *state)
{
  // Free up local data
  free(state->local_data);
}

int do_conform(conform_state *state)
{
  test_data *local_ptr;
  char tmp_str[128];
  FxFloat    vnear = 0.f, vfar = 1.f;
  int v = 0;

  local_ptr = (test_data *)state->local_data;

  // TODO: This test expects the conform window to be the full window.
  state->conform_x = 0;
  state->conform_y = 0;
  state->conform_width = state->screen_width;
  state->conform_height = state->screen_height;

  // The window we'll be using.
  local_ptr->x = state->conform_x;
  local_ptr->y = state->conform_y;
  local_ptr->width = state->conform_width;
  local_ptr->height = state->conform_height;

  // Set the clip window to the whole working area.
  grClipWindow(local_ptr->x,
               local_ptr->y,
               local_ptr->x + local_ptr->width,
               local_ptr->y + local_ptr->height);
  
  // This test only knows how to test truncated RGB values, so
  // turn dithering off for our purposes.
  grDitherMode(GR_DITHER_DISABLE);
  get_colorformat(local_ptr, state->colorformat);

  // Disable Alpha blending
  grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
                       GR_BLEND_ONE, GR_BLEND_ZERO);

  for (int buf = 0; buf < NUM_BUFFERS; buf++) {
    local_ptr->buffer = Buffers[buf];
    for (int ppf = 0; ppf < NUM_PIXPIPE_FLAGS; ppf++) {
      local_ptr->pixpipe_flag = PixPipe_Flags[ppf];

      sprintf(tmp_str, "%s/%s/%s/%s",        
                       local_ptr->colorformat.name,
                       local_ptr->lfb_writemode.name,
                       local_ptr->buffer.name, 
                       local_ptr->pixpipe_flag.name);
      log_message(state, tmp_str);
      printf("\n%s\n", tmp_str);
      

      // Point to the same buffer that we'll be locking for
      // LFB writes.  (this is because, if the pixel pipeline
      // is ON, then pixels go to the current render buffer
      // regardless of which buffer you tried to lock for
      // writing). If we're the AUX buffer, set it to the
      // front buffer.
      if (local_ptr->buffer.val != GR_BUFFER_AUXBUFFER) {
        grRenderBuffer(local_ptr->buffer.val);
      } else {
        grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      }

      // First, do some simple tests. Write a pixel, read
      // it back, compare the vals. Then do Z and Alpha.
      for (int locs = 0; locs < 5; locs++) {

        // clear the buffer to black, disable Z buffering
        grBufferClear(0, 0, 0); // color, alpha, depth 
        grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
        int x, y;
        switch(locs) { 
           case 0: // UL
             x = local_ptr->x; 
             y = local_ptr->y; 
             break;            
           case 1: // UR
             x = local_ptr->x + local_ptr->width-1; 
             y = local_ptr->y; 
             break;
           case 2: // LR
             x = local_ptr->x + local_ptr->width-1; 
             y = local_ptr->y + local_ptr->height-1; 
             break;
           case 3: //LL
             x = local_ptr->x; 
             y = local_ptr->y + local_ptr->height-1; 
             break;
           case 4:
           default: // arbitrary, but even pixel
             x = local_ptr->x + nrand(local_ptr->width-1) & ~0x1;
             y = local_ptr->y + nrand(local_ptr->height-1) & ~0x1;
             break;
        }

        // This is kind of funky. Skip testing if we're on the 
        // AUXBUFFER or in WRITEMODE_ZA16, unless *both* are true!
        if ((local_ptr->buffer.val != GR_BUFFER_AUXBUFFER) &&
            (local_ptr->lfb_writemode.val != GR_LFBWRITEMODE_ZA16)) {
            grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);

            /** Test a simple write and read back. */
            test_write_and_read(state, local_ptr, x, y); 

            /** If applicable, test two-pixel packed writes. */
            if ((local_ptr->lfb_writemode.val == GR_LFBWRITEMODE_565) ||
                (local_ptr->lfb_writemode.val == GR_LFBWRITEMODE_555) ||
                (local_ptr->lfb_writemode.val == GR_LFBWRITEMODE_1555)) {
              test_packed_write_and_read(state, x, y);
            }

            if (local_ptr->test_alpha_depth & TEST_DEPTH) {
              // Do some Z-buffering tests.  
              test_zbuffering(state, local_ptr, x, y);  
            }

            if ((local_ptr->test_alpha_depth & TEST_ALPHA) &&
                (local_ptr->pixpipe_flag.val == FXFALSE)) {
              /** Do some alpha tests.  */
              test_alpha(state, local_ptr, x, y);  
            }

        } else if ((local_ptr->buffer.val == GR_BUFFER_AUXBUFFER) &&
            (local_ptr->lfb_writemode.val == GR_LFBWRITEMODE_ZA16)) {
            /** Do some AUX-buffer access tests.  */
            test_aux_buffer_access(state, local_ptr, x, y); 
        } else {
            putchar('X'); // case skipped.
        }
      }

      if ((local_ptr->buffer.val != GR_BUFFER_AUXBUFFER) &&
          (local_ptr->lfb_writemode.val != GR_LFBWRITEMODE_ZA16)) {
        // Test a simple clipped write and read back.
        test_clipped_write_and_read(state, local_ptr);
      }
    }
    putchar('\n');
  }

  // End of frame, though there isn't much to do.
  add_data(state,(char *)"TODO\n",5);
  end_frame(END_OP_NOOP, state, NULL, END_BUFFER_FRONTBUFFER);

  // Update some local state
  local_ptr->test_pass++;

  // Increment to the next LFB write mode
  int which_lfb_writemode = local_ptr->which_lfb_writemode+1;
  which_lfb_writemode %= NUM_LFB_WRITEMODES;
  local_ptr->lfb_writemode.val = LFB_Writemodes[which_lfb_writemode].val;
  local_ptr->lfb_writemode.name = LFB_Writemodes[which_lfb_writemode].name;
  local_ptr->which_lfb_writemode = which_lfb_writemode;

  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -mode <m> test depth buffering and/or alpha test\n");
  fprintf(stderr,"        alpha  test alphatest/constant alpha, but not depth\n");
  fprintf(stderr,"        depth  test zbuffering/constant depth, but not alpha (default)\n");
  fprintf(stderr,"        both   test both\n");
  fprintf(stderr,"   -wm <s> set the LFBWRITEMODE to use\n");
  fprintf(stderr,"       565    use GR_LFBWRITEMODE_565\n");
  fprintf(stderr,"       555    use GR_LFBWRITEMODE_555\n");
  fprintf(stderr,"       1555   use GR_LFBWRITEMODE_1555\n");
  fprintf(stderr,"       888    use GR_LFBWRITEMODE_888\n");
  fprintf(stderr,"       8888   use GR_LFBWRITEMODE_8888\n");
  fprintf(stderr,"       565D   use GR_LFBWRITEMODE_565_DEPTH\n");
  fprintf(stderr,"       555D   use GR_LFBWRITEMODE_555_DEPTH\n");
  fprintf(stderr,"       1555D  use GR_LFBWRITEMODE_1555_DEPTH\n");
  fprintf(stderr,"       ZA16   use GR_LFBWRITEMODE_ZA16\n");
}


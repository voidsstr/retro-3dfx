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

#include <math.h>

#include "conform.h"
#include "vertex.h"
#include "matrix.h"

static char *test_name = "lfb-region";
static char *test_description = 
 "test grLfbWriteRegion(), grLfbReadRegion(), grLfbConstantAlpha()";

#define ARGB_1555(_a,_r,_g,_b) \
  (FxU16)((_a<<15) | ((_r & 0xf8) << 7) | ((_g & 0xf8) << 2) | (_b & 0xfF) >> 3)

#define RGB_565(_r,_g,_b) \
  (FxU16)(((_r & 0xf8) << 8) | ((_g & 0xfc) << 3) | (_b & 0xfF) >> 3)

#define RGB_555(_r,_g,_b) \
  (FxU16)(((_r & 0xf8) << 7) | ((_g & 0xf8) << 2) | (_b & 0xfF) >> 3)

#define RGB_888(_r,_g,_b) \
  (FxU32)((_r << 16) | (_g << 8) | _b )

#define ARGB_8888(_a,_r,_g,_b) \
  (FxU32)((_a << 24) | RGB_888(_r, _g, _b))


# define RGBD_565(_r,_g,_b,_d) \
  (FxU32)((_d << 16) | RGB_565(_r,_g,_b))

# define RGBD_555(_r,_g,_b,_d) \
   (FxU32)((_d << 16) | RGB_555(_r,_g,_b))

# define ARGBD_1555(_a,_r,_g,_b,_d) \
   (FxU32)((_d << 16) | ARGB_1555(_a,_r,_g,_b))

#define ZA_16(_da) \
  (FxU16)(_da)

#define IMG_WIDTH  10   // the size of the block of pixels we'll be writing
#define IMG_HEIGHT 20

// Data specific to this test.

typedef struct {
  char *name;
  FxU32 val;
} NameValuePair;

NameValuePair Src_Formats[] = {
  {"GR_LFB_SRC_FMT_565", GR_LFB_SRC_FMT_565}, 
  {"GR_LFB_SRC_FMT_555", GR_LFB_SRC_FMT_555}, 
  {"GR_LFB_SRC_FMT_1555", GR_LFB_SRC_FMT_1555},
  {"GR_LFB_SRC_FMT_888", GR_LFB_SRC_FMT_888},
  {"GR_LFB_SRC_FMT_8888", GR_LFB_SRC_FMT_8888},
  {"GR_LFB_SRC_FMT_565_DEPTH", GR_LFB_SRC_FMT_565_DEPTH},
  {"GR_LFB_SRC_FMT_555_DEPTH", GR_LFB_SRC_FMT_555_DEPTH},
  {"GR_LFB_SRC_FMT_1555_DEPTH", GR_LFB_SRC_FMT_1555_DEPTH},
  {"GR_LFB_SRC_FMT_ZA16", GR_LFB_SRC_FMT_ZA16},
//  {"GR_LFB_SRC_FMT_RLE16", GR_LFB_SRC_FMT_RLE16},  // NOT YET IMPLEMENTED
};
#define NUM_SRC_FORMATS (sizeof(Src_Formats) / sizeof(NameValuePair))

NameValuePair Buffers[] = {
  {"GR_BUFFER_FRONTBUFFER", GR_BUFFER_FRONTBUFFER},
  {"GR_BUFFER_BACKBUFFER", GR_BUFFER_BACKBUFFER},
  {"GR_BUFFER_AUXBUFFER", GR_BUFFER_AUXBUFFER},
};
#define NUM_BUFFERS (sizeof(Buffers)/sizeof(NameValuePair))

NameValuePair PixPipe_Flags[] = {
  {"PixPipe OFF", FXFALSE},
  {"PixPipe ON", FXTRUE},
};
#define NUM_PIXPIPE_FLAGS sizeof(PixPipe_Flags)/sizeof(NameValuePair)


typedef struct {
  NameValuePair *src_format;
  NameValuePair *pixpipe_flag;
  NameValuePair *buffer;
} test_data;


/*----------------------------------------------------------------
@func compare_pix
@arg FxU16 pix
@arg FxU8 r,g,b
@return FXTRUE if they match, FXFALSE otherwise
@html
Compares the 5 high bits of each color of pix (565 from the LFB) to
the 5 high bits of r,g,b to see if they match.
@end
-----------------------------------------------------------------*/
FxBool 
compare_pix(FxU16 pix, FxU8 r, FxU8 g, FxU8 b) {
  FxU8 rPix = 0;
  FxU8 gPix = 0;
  FxU8 bPix = 0;

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

  if (!((rPix == r) && (gPix == g) && (bPix == b))) {
    fprintf(stderr, "Comparing RGB to pix: = 0x%02X,0x%02X,0x%02X / 0x%02X,0x%02X,0x%02X\n",
                    r, g, b, rPix, gPix, bPix); 
    return FXFALSE;
  } else {
    return FXTRUE;
  }
}
/*----------------------------------------------------------------
@func write_and_read32
@arg conform_state *state - the conformance state (used for messages)
@arg test_data *local_ptr - the local test state 
@arg FxU8 a - alpha value to write
@arg FxU8 r - red value to write
@arg FxU8 g - green value to write
@arg FxU8 b - blue value to write
@arg FxU16 d - depth value to write
@arg FxU16 *readback - the value of the pixels read back
@return FXTRUE if succesful, FXFALSE otherwise.
@html
This routine calls grLfbWriteRegion() to write a block of 32bit
pixels in various formats. It then reads the block back and
verifies that all pixels in it match. It checks that the stride
argument works, too. It will return false if all of the pixels
read back aren't the same value, or if it couldn't write or 
read data.
@end
-------------------------------------------------------------------*/
FxBool
write_and_read32(conform_state *state, test_data *local_ptr,
               FxU8 a, FxU8 r, FxU8 g, FxU8 b, FxU16 d, FxU16 *readback)
{
  int x, y;
  char msg_buf[256];
  FxU32 col32 = 0;
  void *data;

  // Alloc some space for an image. Make it bigger than it needs 
  // to be so we can test if stride works.
  data = calloc(1, IMG_WIDTH * 2 * IMG_HEIGHT * sizeof(FxU32));

  if (!data) {
    sprintf(msg_buf, "Couldn't alloc space for pixel data");
    log_perror(state, msg_buf);
    fprintf(stderr, "%s\n", msg_buf);
    return FXFALSE;
  }

  switch(local_ptr->src_format->val) {
    case GR_LFB_SRC_FMT_888:
    default:
      col32 = RGB_888(r, g, b);
      break;
    case GR_LFB_SRC_FMT_8888:
      col32 = ARGB_8888(a, r, g, b);
      break;
    case GR_LFB_SRC_FMT_565_DEPTH:
      col32 = RGBD_565(r, g, b, d);
      break;
    case GR_LFB_SRC_FMT_555_DEPTH:
      col32 = RGBD_555(r, g, b, d);
      break;
    case GR_LFB_SRC_FMT_1555_DEPTH:
      col32 = ARGBD_1555(a, r, g, b, d);
      break;
    case GR_LFB_SRC_FMT_RLE16: 
      col32 = ((IMG_WIDTH*IMG_HEIGHT)<< 16) | RGB_565(r, g, b); 
      break;
  }


  FxU32 *pix = (FxU32*)data;
  for (y =0; y < IMG_HEIGHT;y++) { // copy the color we chose into the image
    for (x =0; x < IMG_WIDTH*2; x++) { 
      if (x  < IMG_WIDTH) {
        *pix++ = col32; 
      } else {
        *pix++ = ~col32;  // after the stride, change the data
      }
    }
  }


  if (!grLfbWriteRegion(local_ptr->buffer->val,
                        5, 5, // where
                        local_ptr->src_format->val,
                        IMG_WIDTH,   // width
                        IMG_HEIGHT,   // height
                        local_ptr->pixpipe_flag->val,
                        IMG_WIDTH * 2 * sizeof(FxU32), // stride
                        data)) {
    sprintf(msg_buf, "grLfbWriteRegion() failed @ line %d.", __LINE__);
    log_message(state, msg_buf);
    fprintf(stderr, "%s\n", msg_buf);
    return FXFALSE;
  } 

  grFinish();

  // At this point, we've written our data. Now read it back and test it.

  memset(data, 0, IMG_WIDTH*IMG_HEIGHT*2*sizeof(FxU32)); // zero it first

  if (!grLfbReadRegion(local_ptr->buffer->val,
                       5, 5,
                       IMG_WIDTH,
                       IMG_HEIGHT,
                       IMG_WIDTH * 2 * sizeof(FxU32), // stride
                       data)) {
    sprintf(msg_buf, "grLfbReadRegion() failed @ line %d.", __LINE__);
    log_message(state, msg_buf);
    fprintf(stderr, "%s\n", msg_buf);
    return FXFALSE;
  }

  grFinish();

  // Check that all the pixels we read back are the same value. We don't
  // care (yet) what the value is, we just want them to be consistent.

  FxU16 *pix16 = (FxU16*)data;
  FxU16 last_pix = *pix16;
  for (y =0; y < IMG_HEIGHT;y++) { 
    for (x =0; x < IMG_WIDTH*2; x++) { 
      if (x < IMG_WIDTH) {
        if (*pix16 != last_pix) {
          sprintf(msg_buf, "different pixel (%d, %d) read back @ line %d (0x%X != 0x%X).",
                           x,y,  __LINE__, *pix16, last_pix);
          log_message(state, msg_buf);
          fprintf(stderr, "%s\n", msg_buf);
          return FXFALSE;
        }
        last_pix = *pix16;
      }
      pix++;
    }
  }

  *readback = last_pix;

  return FXTRUE;
} 
/*----------------------------------------------------------------
@func write_and_read16
@arg conform_state *state - the conformance state (used for messages)
@arg test_data *local_ptr - the local test state 
@arg FxU8 a - alpha value to write
@arg FxU8 r - red value to write
@arg FxU8 g - green value to write
@arg FxU8 b - blue value to write
@arg FxU16 d - depth value to write
@arg FxU16 *readback - the value of the pixels read back
@return FXTRUE if succesful, FXFALSE otherwise.
@html
This routine calls grLfbWriteRegion() to write a block of 16bit
pixels in various formats. It then reads the block back and
verifies that all pixels in it match. It checks that the stride
argument works, too. It will return false if all of the pixels
read back aren't the same value, or if it couldn't write or 
read data.
@end
-------------------------------------------------------------------*/
FxBool
write_and_read16(conform_state *state, test_data *local_ptr,
               FxU8 a, FxU8 r, FxU8 g, FxU8 b, FxU16 d, FxU16 *readback)
{
  int x, y;
  char msg_buf[256];
  FxU16 col16 = 0;
  void *data;

  // Alloc some space for an image. Make it bigger than it needs 
  // to be so we can test if stride works.
  data = calloc(1, IMG_WIDTH * 2 * IMG_HEIGHT * sizeof(FxU16));

  if (!data) {
    sprintf(msg_buf, "Couldn't alloc space for pixel data");
    log_perror(state, msg_buf);
    fprintf(stderr, "%s\n", msg_buf);
    return FXFALSE;
  }

  switch(local_ptr->src_format->val) {
    case GR_LFB_SRC_FMT_555:
      col16 = RGB_555(r, g, b);
      break;
    case GR_LFB_SRC_FMT_1555: 
      col16 = ARGB_1555(a, r, g, b);
      break;
    case GR_LFB_SRC_FMT_ZA16:
      col16 = ZA_16(d); // should only be here on AUX buffer
      break;
    case GR_LFB_SRC_FMT_565: 
    default:
      col16 = RGB_565(r, g, b);
      break;
  }


  FxU16 *pix = (FxU16*)data;
  for (y =0; y < IMG_HEIGHT;y++) { // copy the color we chose into the image
    for (x =0; x < IMG_WIDTH*2; x++) { 
      if (x  < IMG_WIDTH) {
        *pix++ = col16; 
      } else {
        *pix++ = ~col16;  // after the stride, change the data
      }
    }
  }


  if (!grLfbWriteRegion(local_ptr->buffer->val,
                        5, 5, // where
                        local_ptr->src_format->val,
                        IMG_WIDTH,   // width
                        IMG_HEIGHT,   // height
                        local_ptr->pixpipe_flag->val,
                        IMG_WIDTH * 2 * sizeof(FxU16), // stride
                        data)) {
    sprintf(msg_buf, "grLfbWriteRegion() failed @ line %d.", __LINE__);
    log_message(state, msg_buf);
    fprintf(stderr, "%s\n", msg_buf);
    return FXFALSE;
  } 

  grFinish();

  // At this point, we've written our data. Now read it back and test it.

  memset(data, 0, IMG_WIDTH*IMG_HEIGHT*2*sizeof(FxU16)); // zero it first

  if (!grLfbReadRegion(local_ptr->buffer->val,
                       5, 5,
                       IMG_WIDTH,
                       IMG_HEIGHT,
                       IMG_WIDTH * 2 * sizeof(FxU16), // stride
                       data)) {
    sprintf(msg_buf, "grLfbReadRegion() failed @ line %d.", __LINE__);
    log_message(state, msg_buf);
    fprintf(stderr, "%s\n", msg_buf);
    return FXFALSE;
  }

  grFinish();

  // Check that all the pixels we read back are the same value. We don't
  // care (yet) what the value is, we just want them to be consistent.

  pix = (FxU16*)data;
  FxU16 last_pix = *pix;
  for (y =0; y < IMG_HEIGHT;y++) { 
    for (x =0; x < IMG_WIDTH*2; x++) { 
      if (x < IMG_WIDTH) {
        if (*pix != last_pix) {
          sprintf(msg_buf, "different pixel (%d, %d) read back @ line %d (0x%X != 0x%X).",
                           x,y,  __LINE__, *pix, last_pix);
          log_message(state, msg_buf);
          fprintf(stderr, "%s\n", msg_buf);
          return FXFALSE;
        }
        last_pix = *pix;
      }
      pix++;
    }
  }

  *readback = last_pix;

  return FXTRUE;
} 


/*----------------------------------------------------------------
@func write_and_read16
@arg conform_state *state - the conformance state (used for messages)
@arg test_data *local_ptr - the local test state 
@return FXTRUE if the tests passed, FXFALSE otherwise
@html
Calls the appropriate 16 and 32 bit tests to check region writes
and reads on the LFB.
@end
-----------------------------------------------------------------*/
TestResult
test_write_and_read_color(conform_state *state, test_data *local_ptr)
{
  FxU16 col16 = 0;
  FxU32 col32 = 0;
  FxU8 r, g, b, a;
  FxU16 d;
  char msg_buf[256];


  a = 0x1;
  r = nrand(0xFF);
  g = nrand(0xFF);
  b = nrand(0xFF);
  d = 0xDEAD;

  // Point to the same buffer that we'll be locking for
  // LFB writes.  (this is because, if the pixel pipeline
  // is ON, then pixels go to the current render buffer
  // regardless of which buffer you tried to lock for
  // writing). If we're the AUX buffer, set it to the
  // front buffer.
  if (local_ptr->buffer->val == GR_BUFFER_AUXBUFFER) {
    grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  } else {
    grRenderBuffer(local_ptr->buffer->val);
  }


  grColorMask(FXTRUE, FXFALSE);
  grDepthMask(FXTRUE);
  grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
  grBufferClear(rgbacolor(state, 0xff000000), 0x0, 0x0); // clear to red

  grFinish();

  switch(local_ptr->src_format->val) {
    case GR_LFB_SRC_FMT_565:
    case GR_LFB_SRC_FMT_555:
    case GR_LFB_SRC_FMT_1555: 
    case GR_LFB_SRC_FMT_ZA16: {

      FxU16 readback;  
    
      if (local_ptr->src_format->val == GR_LFB_SRC_FMT_ZA16) {
        grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
      }

      if (!write_and_read16(state, local_ptr,
                            a, r, g, b, d,
                            &readback)) {
         sprintf(msg_buf, "test_write_and_read16() @ line %d", __LINE__);
         log_message(state, msg_buf);
         fprintf(stderr, "%s\n", msg_buf);
         return TEST_FAIL;
      }

      // At this point, a block of pixels were written, and a block of
      // consistent pixels was read back. "readback" has the value of the
      // pixels that were read back from the LFB. Check to make sure it
      // is what we expect it to be.

      if (local_ptr->src_format->val == GR_LFB_SRC_FMT_ZA16) {
        if (readback != d) {
          sprintf(msg_buf, "returned depth (0x%X) != sent depth (0x%X) @ line %d", 
                           readback, d, __LINE__);
          log_message(state, msg_buf);
          fprintf(stderr, "%s\n", msg_buf);
          return TEST_FAIL;
        }
      } else if (!compare_pix(readback, r, g, b)) {
         sprintf(msg_buf, "returned pix != sent pix @ line %d", __LINE__);
         log_message(state, msg_buf);
         fprintf(stderr, "%s\n", msg_buf);
         return TEST_FAIL;
      }
    }
    break;

    case GR_LFB_SRC_FMT_888:
    case GR_LFB_SRC_FMT_8888:
    case GR_LFB_SRC_FMT_565_DEPTH:
    case GR_LFB_SRC_FMT_555_DEPTH:
    case GR_LFB_SRC_FMT_1555_DEPTH:
    case GR_LFB_SRC_FMT_RLE16: {

      FxU16 readback;  

      if (!write_and_read32(state, local_ptr,
                            a, r, g, b, d,
                            &readback)) {
         sprintf(msg_buf, "test_write_and_read16() @ line %d", __LINE__);
         log_message(state, msg_buf);
         fprintf(stderr, "%s\n", msg_buf);
         return TEST_FAIL;
      }

      // At this point, a block of pixels were written, and a block of
      // consistent pixels was read back. "readback" has the value of the
      // pixels that were read back from the LFB. Check to make sure it
      // is what we expect it to be.

      if (!compare_pix(readback, r, g, b)) {
         sprintf(msg_buf, "returned pix != sent pix @ line %d", __LINE__);
         log_message(state, msg_buf);
         fprintf(stderr, "%s\n", msg_buf);
         return TEST_FAIL;
      }
    }
    break;
  }

  return TEST_PASS;  
}

int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;
  int num_arg;

  state->test_name = test_name;
  state->test_description = test_description;

  if((state->local_data = calloc(1, sizeof(test_data))) == NULL) {
    // Could not allocate data for local memory
    log_perror(state,"Could not allocate memory for local data");
    return(0);
  } 

  // Set some default local state
  local_ptr = (test_data *)state->local_data;

  // print out test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
    ++local_argv;
    ++num_arg;
  }

  state->tile = 0;
  state->num_reps = 1;

  if (state->num_color_bufs < 2) {
    state->num_color_bufs = 2; // need at least 2 color bufs
  }

  if (state->num_aux_bufs < 1) {
    state->num_aux_bufs = 1; // need at least l aux bufs
  }

  state->conform_width  = 100;
  state->conform_height = 100;

  // setup image file names
  // this is manditory
  char tmp_str[128];
  sprintf(tmp_str,"%s",test_name);

  frame_name(tmp_str,state);
  img_file_type(IMG_P6,state);

  return(-1);
}

void close_conform(conform_state *state)
{
  // Free up local data
  test_data *local_ptr = (test_data*)state->local_data;
  if (local_ptr) {
    free(local_ptr);
  }
}


int do_conform(conform_state *state)
{
  char config_buf[128];
  int which_format, which_buffer, which_flag;

  test_data *local_ptr = (test_data*)state->local_data;

  grCoordinateSpace(GR_WINDOW_COORDS);
  grDitherMode(GR_DITHER_DISABLE);

  for (which_buffer = 0; which_buffer < NUM_BUFFERS; which_buffer++) {
    for (which_format = 0; which_format < NUM_SRC_FORMATS; which_format++) { 
      for (which_flag = 0; which_flag < NUM_PIXPIPE_FLAGS; which_flag++) {
        local_ptr->src_format = &Src_Formats[which_format];  
        local_ptr->buffer = &Buffers[which_buffer];
        local_ptr->pixpipe_flag = &PixPipe_Flags[which_flag];
        sprintf(config_buf, "%s / %s / %s", 
                            local_ptr->buffer->name, 
                            local_ptr->src_format->name, 
                            local_ptr->pixpipe_flag->name);

        if ((local_ptr->buffer->val != GR_BUFFER_AUXBUFFER) &&
            (local_ptr->src_format->val != GR_LFB_SRC_FMT_ZA16)) { 

          fprintf(stderr, "%s\n", config_buf);
          if (!log_status(state, test_write_and_read_color(state, local_ptr),
                          config_buf)) {
            return TEST_FAIL;
          }
  
        } else if ((local_ptr->buffer->val == GR_BUFFER_AUXBUFFER) &&
                   (local_ptr->src_format->val == GR_LFB_SRC_FMT_ZA16) && 
                   (local_ptr->pixpipe_flag->val == FXFALSE)) {  

           // We test ZA16 if a) we're on auxbuffer, b) we're on ZA16, on
           // c) the pixel pipeline is off (as per the ref manual).
          fprintf(stderr, "%s\n", config_buf);
          if (!log_status(state, test_write_and_read_color(state, local_ptr),
                          config_buf)) {
            return TEST_FAIL;
          }

        } else { 
          // ZA_16 and not aux buffer. Skip it.
        }
      }
    }
  }

  return(TEST_PASS); // this value is ignored.
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"    No test specific args\n");
}


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

static char *test_name = "simple-clear";
static char *test_description = "Clears all buffers to known values, "
 "verifies that they are cleared to the expected values, verifies that "
 "clearing pays attention to the clip window";

typedef struct {
  int test_pass;
  float scale_y;
  float scale_x;
  float delta_y;
  float delta_x;
  int origin_x;
  int origin_y;
} test_data;

typedef struct {
  char *name;
  GrBuffer_t val;
} NameValuePair;

NameValuePair buffers[] = {
  {"GR_BUFFER_FRONTBUFFER", GR_BUFFER_FRONTBUFFER},
  {"GR_BUFFER_BACKBUFFER", GR_BUFFER_BACKBUFFER},
  {"GR_BUFFER_AUXBUFFER", GR_BUFFER_AUXBUFFER},
};

#define NUM_BUFFERS (sizeof(buffers) / sizeof(NameValuePair))

void
adjust_origin(conform_state *state, int flip_flag,
              int x, int y, int width, int height,
              int *new_x, int *new_y, int *new_width, int *new_height)
{
  *new_x = x + state->conform_x;
  *new_width = width;
  *new_height = height;

  if (flip_flag && state->origin == GR_ORIGIN_LOWER_LEFT) {
    *new_y = state->screen_height - (height + y + state->conform_y);
  } else {
    *new_y = y + state->conform_y;
  }
}

void
set_clip_window(conform_state *state,
                int x, int y, int width, int height)
{
  int new_x, new_y, new_width, new_height;

  adjust_origin(state, 1,
                x, y, width, height, 
                &new_x, &new_y, &new_width, &new_height);

  grClipWindow(new_x, new_y, new_x + new_width, new_y + new_height);

}

/*----------------------------------------------------------------------
@func verify_buffer_contens
@arg conform_state *state - the framework state
@arg GrBuffer_t buffer - which buffer to examine
@arg x,y,width,height - the window in the buffer to verify
@arg expectedValue - the value to compare to each pixel in
                     the buffer.
@arg int alpha_flag - if true, we only care about the low order 8 bits
@return TEST_PASS if all pixels match, TEST_FAIL otherwise
@html
Read in the contents of a buffer and check the contents. If
grLfbReadRegion() is broken, this routine is broken. 
Compare each pixel value returned from the grLfbReadRegion() 
to expectedValue. 
-----------------------------------------------------------------------*/
TestResult
verify_buffer_contents(conform_state *state, 
                       GrBuffer_t buffer,
                       int x, int y, int width, int height,
                       FxU16 expected_value,
                       int alpha_flag) {
    TestResult status = TEST_FAIL;
    int verbose = 1;

    FxU16 *pix_buf = (FxU16*)calloc(width * height, 2);
    if (!pix_buf) { 
        fprintf(stderr, "calloc() failed!\n");
        log_perror(state, "calloc() failed"); 
        return status;
    }

    int new_x, new_y, new_width, new_height;
    adjust_origin(state, 0, 
                  x, y, width, height, 
                  &new_x, &new_y, &new_width, &new_height);

    if (grLfbReadRegion(buffer, 
                        new_x, new_y,
                        new_width, new_height,
                        new_width * 2, // dst stride
                        pix_buf)){

        FxU16 *pix = pix_buf;
        int num_pixels = width * height;

        status = TEST_PASS;

        // check all the read back data, it's broken if there's a mismatch

        if (alpha_flag) { // we only care about the lower 8 bits
          for (int i = 0; i < num_pixels; i++) {
              if ((*pix & 0xff) != expected_value) {
                  if (verbose) {
                      printf(" reading %dx%d block @ %d, %d\n",
                               new_width, new_height, new_x, new_y);
                      printf(" expected 0x%04X != read back 0x%04X @ pix# %d\n", 
                              expected_value, *pix, i);
                  }
                  status = TEST_FAIL;
                  break;
              }
              ++pix;
          }
        } else { // we care about all 16 bits
          // check all the read back data, it's broken if there's a mismatch
          for (int i = 0; i < num_pixels; i++) {
              if (*pix != expected_value) {
                  if (verbose) {
                      printf(" reading %dx%d block @ %d, %d\n",
                               new_width, new_height, new_x, new_y);
                      printf(" expected 0x%04X != read back 0x%04X @ pix# %d\n", 
                              expected_value, *pix, i);
                  }
                  status = TEST_FAIL;
                  break;
              }
              ++pix;
          }
       }
    } else {
        fprintf(stderr, "grLfbReadRegion() failed.\n");
    }
    free(pix_buf);
    return status;
}

/**
 * Given the current COLORFORMAT and some RGB's, return an
 * appropriate color 'clearColor' to pass into grBufferClear()
 * so that the contents of the buffer when read back will
 * consist of 'readBack'.
 * @param ts - the testing state
 * @param r,g,b - the RGB values of the color to munge
 * @param *clearColor - returned: the value appropriate for passing 
 *                      to grBufferClear() (given the RGB above)
 * @param *readBack - returned: the value that should be returned
 *                    when an LFB read is called after
 *                    grBufferClear(clearColor, 0, 0);
 */
FxBool 
munge_color(GrColorFormat_t color_format,
            FxU8 r, FxU8 g, FxU8 b,
            GrColor_t *clearColor, FxU16 *readBack) {

    GrColor_t col = 0;

    switch (color_format) { 
        case GR_COLORFORMAT_ARGB:
            col = (r & 0xff) << 16 | (g & 0xff)<< 8 | (b & 0xff);
            break;
        case GR_COLORFORMAT_RGBA:
            col = (r & 0xff) << 16 | (g & 0xff)<< 8 | (b & 0xff);
            col <<= 8;
            break;
        case GR_COLORFORMAT_ABGR:
            col = (b & 0xff) << 16 | (g & 0xff)<< 8 | (r & 0xff);
            break;
        case GR_COLORFORMAT_BGRA:
            col = (b & 0xff) << 16 | (g & 0xff)<< 8 | (r & 0xff);
            col <<= 8;
            break;
        default:
            fprintf(stderr, "invalid COLORFORMAT specified\n");
            return FXFALSE;
            break;
    }
    *clearColor = col;
    *readBack = (FxU16)((r & 0xF8) << 8| (g & 0xFC) << 3 | 
                                (b & 0xF8) >> 3);
    return FXTRUE;
}


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
  local_ptr->scale_y = 1.0f;
  local_ptr->scale_x = 1.0f;
  local_ptr->delta_y = 1.0f;
  local_ptr->origin_x = 0;
  local_ptr->origin_y = 0;

  // handle our test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
      fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
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

  // we don't support off-screen conform windows
  state->xycheck = 0;
  state->xylimit = 0;

  state->num_reps = 1; // one pass through

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
  FxFloat    vnear = 0.f, vfar = 1.f;
  int v = 0;

  local_ptr = (test_data *)state->local_data;

  // The window we'll be using.
  int x = 0;
  int y = 0;
  int width = state->conform_width;
  int height = state->conform_height;

  int clip_width  = nrand((int)(width * 0.5f)) + (int)(width * 0.25f);
  int clip_height = nrand((int)(height * 0.5f)) + (int)(height * 0.25f);
  int clip_x      = nrand((int)(width - clip_width));
  int clip_y      = nrand((int)(height - clip_height));

  grDitherMode(GR_DITHER_DISABLE);

  /*
   *  The algorithm (for each buffer) is:
   *   - set the clip window to the full window
   *   - grBufferClear() to val1
   *   - verify that the whole buffer was cleared to val1
   *   - set the clipwindow to a subset
   *   - grBufferClear() to val2
   *   - verify that inside the clip window was cleared to val2
   *   - verify that outside the clip window remains val1
   */
  for (int buf = 0; buf < NUM_BUFFERS; buf++) {

    printf("Testing: %s\n", buffers[buf].name, buffers[buf].val);
    FxI32 buffer = buffers[buf].val;
    FxI32 colorformat = state->colorformat; 

    if (buffer == GR_BUFFER_AUXBUFFER) {

      printf(" depth\n");
      // For the AUX buffer, test depth and Alpha.
      grRenderBuffer(GR_BUFFER_BACKBUFFER); // can't set to AUX

      // Enable depth buffering
      grColorMask(FXTRUE, FXFALSE); // color ON, alpha OFF
      grDepthMask(FXTRUE);                
      grDepthBufferMode( GR_DEPTHBUFFER_ZBUFFER );
      grDepthBufferFunction( GR_CMP_GREATER );


      FxU16 d1= 0xDEAD;
      FxU16 d2= 0xBEEF;

      set_clip_window(state, x, y, width, height); // full window
      grBufferClear(0x0000ff00, 0, (FxU32)d1);   //c, a, d
      end_frame(END_OP_NOOP, state, NULL, buffer);

      if (!log_status(state,
                      verify_buffer_contents(state, buffer, x, y, 
                                             width, height, d1, 0),
                      "full AUX buffer clear (depth)")) {
        return TEST_FAIL;
      }

      // now clear only a window
      set_clip_window(state, clip_x, clip_y, clip_width, clip_height);
      grBufferClear(0x00ff00ff, 0, (FxU32)d2);   //c, a, d
      end_frame(END_OP_NOOP, state, NULL, buffer);

      if (!log_status(state,
                      verify_buffer_contents(state, 
                                             buffer,
                                             clip_x,
                                             clip_y,
                                             clip_width,
                                             clip_height,
                                             d2,
                                             0),
                      "clipped AUX buffer clear (depth)")) {
        return TEST_FAIL; 
      }

      // and verify that it didn't affect outside the clip window
      // check the box from 0,0 to the clip window's origin
      if (!log_status(state,
                      verify_buffer_contents(state, 
                                             buffer, 
                                             x,
                                             y,
                                             clip_x, 
                                             clip_y, 
                                             d1,
                                             0),
                       "clipped AUX buffer clear (depth)")) {
        return TEST_FAIL;
      }
      // Disable Depth Buffering
      grDepthBufferMode( GR_DEPTHBUFFER_DISABLE );

      // 
      // Done with depth, now test alpha.
      //

      printf(" alpha\n");

      grDepthMask(FXFALSE);
      grColorMask(FXTRUE, FXTRUE);
      grDepthBufferMode(GR_DEPTHBUFFER_DISABLE ); // disable depth buffering
      // Enable Alpha blending
      grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ONE,
                           GR_BLEND_ONE, GR_BLEND_ONE ); 

      set_clip_window(state, x, y, width, height); // full window

      GrAlpha_t a1= 0xAC;  // 8 bit values!
      GrAlpha_t a2= 0xBE;

      grBufferClear(0x0000ff00, a1, (FxU32)0xBEEF);   //c, a, d
      end_frame(END_OP_NOOP, state, NULL, buffer);

      if (!log_status(state,
                      verify_buffer_contents(state,
                                             buffer, 
                                             x, y, 
                                             width, 
                                             height, a1, 1),
                      "full AUX buffer clear (alpha)")) {
        return TEST_FAIL;
      }

      // now clear only a window
      set_clip_window(state, clip_x, clip_y, clip_width, clip_height);
      grBufferClear(0x00ff00ff, a2, (FxU32)0xABCD);   //c, a, d
      end_frame(END_OP_NOOP, state, NULL, buffer);

      if (!log_status(state,
                      verify_buffer_contents(state, 
                                             buffer,
                                             clip_x, 
                                             clip_y, 
                                             clip_width,
                                             clip_height,
                                             a2, 1),
                      "clipped AUX buffer clear (alpha)")) {
        return TEST_FAIL;
      }

      // and verify that it didn't affect outside the clip window
      if (!log_status(state,
                      verify_buffer_contents(state, 
                                             buffer, 
                                             x,
                                             y,
                                             clip_x, 
                                             clip_y, 
                                             a1, 1),
                      "clipped AUX buffer clear (alpha)")) {
        return TEST_FAIL;
      }

      // Disable alpha
      grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
                           GR_BLEND_ONE, GR_BLEND_ZERO);
      grColorMask(FXTRUE, FXFALSE);

    } else { // COLOR buffers.

      GrColor_t clearColor1, clearColor2;
      FxU16 readBack1, readBack2;

      grRenderBuffer(buffer); // can't set to AUX
                 
      // For a given RGB, figure out what color we should write
      // and what we should expect back (for our COLORFORMAT)
      munge_color(colorformat, 0xbb, 0xaa, 0xdd, &clearColor1, &readBack1);
      munge_color(colorformat, 0xf0, 0xa0, 0xe0, &clearColor2, &readBack2);

      // At this point, clearColorX is the properly formatted
      // color for our COLORFORMAT to pass to grClearBuffer().
      // readBackX is the color that we'd expect to get doing
      // a grLfbReadRegion() after the grBufferClear()
      
      set_clip_window(state, x, y, width, height); // full window
      grBufferClear(clearColor1, 0, 0);   //c, a, d
      end_frame(END_OP_NOOP, state, NULL, buffer);

      if (!log_status(state,
                      verify_buffer_contents(state, 
                                             buffer, 
                                             x, y, 
                                             width, height,
                                             readBack1, 0),
                      "full color buffer clear")) {
        return TEST_FAIL;
      }


      // now clear only a window
      set_clip_window(state, clip_x, clip_y, clip_width, clip_height);
      grBufferClear(clearColor2, 0, 0); //c, a, d
      end_frame(END_OP_NOOP, state, NULL, buffer);

      if (!log_status(state,
                      verify_buffer_contents(state, 
                                             buffer,  
                                             clip_x,
                                             clip_y,
                                             clip_width,
                                             clip_height,
                                             readBack2,
                                             0),
                      "clipped color buffer clear")) {
        return TEST_FAIL;
      }

      // Now make sure that things were unaffected outside of the
      // clip window.

      // Check to the upper left of the clipwindow
      if (!log_status(state,
                      verify_buffer_contents(state, 
                                             buffer, 
                                             x , y, 
                                             clip_x, 
                                             clip_y,
                                             readBack1, 0),
                      "clipped color buffer (should be unaffected) clear")) {
        return TEST_FAIL;
      }

      // Check below and right of the clip window
      if (!log_status(state,
                      verify_buffer_contents(state, 
                                             buffer, 
                                             clip_x + clip_width,
                                             clip_y + clip_height,
                                             width - (clip_width + clip_x),
                                             height - (clip_height + clip_y),
                                             readBack1, 0),
                      "clipped color buffer (should be unaffected) clear")) {
        return TEST_FAIL;
      }
    }
  }


  return(TEST_PASS); // ignored
}

void help_conform(conform_state *state)
{
}


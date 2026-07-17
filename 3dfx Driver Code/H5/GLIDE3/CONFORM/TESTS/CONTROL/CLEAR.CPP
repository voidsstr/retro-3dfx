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

static char *test_name = "clear";
static char *test_description =
 "Test grBufferClear() on color and aux buffers, with and without clip windows";

#define NUM_PASSES 5

#define DEPTH_FAR    (65535.0f/100.0f)  // for Z tests
#define DEPTH_MID_G  (65535.0f/150.0f)
#define DEPTH_MID    (65535.0f/200.0f)
#define DEPTH_MID_L  (65535.0f/250.0f)
#define DEPTH_NEAR   (65535.0f/300.0f)


// Data specific to this test.
typedef struct {
  int test_pass;
} test_data;


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
  local_ptr->test_pass = 0;

  // print out test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
    ++local_argv;
    ++num_arg;
  }

  state->tile = 0;
  state->num_reps = NUM_PASSES;

  if (state->num_color_bufs < 2) {
    state->num_color_bufs = 2; // need at least 2 color buf
  }

  if (state->num_aux_bufs < 1) {
    state->num_aux_bufs = 1; // need at least 1 color buf
  }

  // disable -xylimit and -xycheck support
  state->xycheck = 0;
  state->xylimit = 0;

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
  FxU32 size = 0;
  FxBool passed = FXTRUE;
  GrVertex verts[3];
  int x = state->conform_x;
  int y = state->conform_y;
  int width = state->conform_width;
  int height = state->conform_height;

  GrColor_t red = rgbacolor(state, 0xff0000ff);
  GrColor_t green = rgbacolor(state, 0x00ff00ff);
  GrColor_t blue = rgbacolor(state, 0x0000ffff);
  GrColor_t cyan = rgbacolor(state, 0x00ffFFff);
  GrColor_t yellow = rgbacolor(state, 0xffFF00FF);
 
  test_data *local_ptr = (test_data*)state->local_data;
  local_ptr = (test_data *)state->local_data;

  grDitherMode(GR_DITHER_DISABLE); // turn dithering OFF

  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);   
  grVertexLayout(GR_PARAM_Z , GR_VERTEX_OOZ_OFFSET << 2, GR_PARAM_ENABLE); 

  verts[0].x = (float)x;
  verts[0].y = (float)y;
  verts[0].ooz = DEPTH_MID_L;

  verts[1].x = (float)x+(width/2);
  verts[1].y = (float)y+height;
  verts[1].ooz = DEPTH_MID;

  verts[2].x = (float)x+width;
  verts[2].y = (float)y;
  verts[2].ooz = DEPTH_MID_G;

  grConstantColorValue(rgbacolor(state, 0xFF00FFFF)); // magenta, full alpha

  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_CONSTANT,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );

  // Start with known state.
  grCoordinateSpace(GR_WINDOW_COORDS);
  grViewport(state->conform_x, state->conform_y, 
             state->conform_width, state->conform_height);
  grClipWindow(x, y, x + width, y + height);

  /**
   ** Bug: If grColormask() sets the alpha flag to true in the line
   ** below, the depth buffer tests are broken. If the alpha flag
   ** is set to false, everything works as expected.
   **/
  grColorMask(FXTRUE, FXTRUE);  // BUG
  //grColorMask(FXTRUE, FXFALSE); // works

  grDepthMask(FXFALSE);
  grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);         // disable depth buffering
  grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,  // disable alpha blending
                       GR_BLEND_ONE, GR_BLEND_ZERO);

  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grBufferClear(yellow, 0, 0);
  grRenderBuffer(GR_BUFFER_BACKBUFFER);
  grBufferClear(yellow, 0, 0);


  switch(local_ptr->test_pass % NUM_PASSES) {
    case 0: // clear whole front/back
      grClipWindow(x, y, x + width, y + height);

      grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      grBufferClear(green, 0, 0);
      grRenderBuffer(GR_BUFFER_BACKBUFFER);
      grBufferClear(blue, 0, 0);

      end_frame(END_OP_SAVE_TO_FILE, state, NULL, 
                END_BUFFER_FRONTBUFFER | END_BUFFER_BACKBUFFER);

      break;
    case 1: // clear front/back w/clip
      grClipWindow(x, y, x + width, y + height);  // clip to full window

      grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      grBufferClear(0, 0, 0);
      grDrawTriangle(&verts[0], &verts[1], &verts[2]);

      grRenderBuffer(GR_BUFFER_BACKBUFFER);
      grBufferClear(0, 0, 0);
      grDrawTriangle(&verts[0], &verts[1], &verts[2]);

      grClipWindow(x+width/4, 
                   y+width/4, 
                   x+width/4 + width/2, 
                   y+height/4 + height/2);

      grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      grBufferClear(green, 0, 0);

      grRenderBuffer(GR_BUFFER_BACKBUFFER);
      grBufferClear(blue, 0, 0);

      end_frame(END_OP_SAVE_TO_FILE, state, NULL, 
                END_BUFFER_FRONTBUFFER | END_BUFFER_BACKBUFFER);
      break;
    case 2: // test that clear pays attention to the grColorMask() color arg
      grClipWindow(x, y, x + width, y + height);  // clip to full window

      grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      grBufferClear(0, 0, 0);
      grDrawTriangle(&verts[0], &verts[1], &verts[2]);

      grRenderBuffer(GR_BUFFER_BACKBUFFER);
      grBufferClear(0, 0, 0);
      grDrawTriangle(&verts[0], &verts[1], &verts[2]);

      grClipWindow(x+width/4, 
                   y+width/4, 
                   x+width/4 + width/2, 
                   y+height/4 + height/2);

      grColorMask(FXFALSE, FXFALSE);
      grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      grBufferClear(green, 0, 0);

      grRenderBuffer(GR_BUFFER_BACKBUFFER);
      grBufferClear(blue, 0, 0);

      end_frame(END_OP_SAVE_TO_FILE, state, NULL, 
                END_BUFFER_FRONTBUFFER | END_BUFFER_BACKBUFFER);
      break;
    case 3:  // clears with depth
      grClipWindow(x, y, x + width, y + height);

      grColorMask(FXTRUE, FXFALSE);
      grDepthMask(FXTRUE);
      grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);

      grDepthBufferFunction(GR_CMP_GREATER);

      grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      grBufferClear(red, 0, (FxU32)DEPTH_MID);

      grClipWindow(x+width/4, 
                   y+width/4, 
                   x+width/4 + width/2, 
                   y+height/4 + height/2);

      grBufferClear(green, 0, (FxU32) DEPTH_FAR);

      grClipWindow(x, y, x + width, y + height);  // clip to full window

      grDrawTriangle(&verts[0], &verts[1], &verts[2]);

      end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER );
      break;
    case 4: // clears with alpha
      grClipWindow(x, y, x + width, y + height);  // clip to full window

      grDepthMask(FXFALSE);
      grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
      grColorMask(FXTRUE, FXTRUE);

      grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      grBufferClear(blue, 0x80, (FxU32)DEPTH_MID); // half alpha

      grClipWindow(x+width/4, 
                   y+width/4, 
                   x+width/4 + width/2, 
                   y+height/4 + height/2);

      grBufferClear(green, 0x40, (FxU32) DEPTH_NEAR); // half alpha

      grClipWindow(x, y, x + width, y + height);  // clip to full window

      grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, 
                           GR_BLEND_DST_ALPHA, 
                           GR_BLEND_ONE, 
                           GR_BLEND_ZERO); 

      grConstantColorValue(rgbacolor(state, 0xFF0000ff)); // red, full alpha

      grDrawTriangle(&verts[0], &verts[1], &verts[2]);

      end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER );
      break;
    default:
      break;
  }

  local_ptr->test_pass++;

  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"    No test specific args\n");
}


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

static char *test_name = "buffer-swap";
static char *test_description = "test GrBufferSwap()";

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

  // tiling not supported
  state->tile = 0;
  state->num_reps = 1;

  // xylimit/xycheck not supported (fix this when adding self-checking)
  state->xylimit = 0;
  state->xycheck = 0;

  if (state->num_color_bufs < 2) {
    state->num_color_bufs = 2; // need at least 2 color bufs
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
  GrColor_t red = rgbacolor(state, 0xff0000ff);
  GrColor_t green = rgbacolor(state, 0x00ff00ff);
  GrColor_t blue = rgbacolor(state, 0x0000ffff);
  GrColor_t yellow = rgbacolor(state, 0xffff00ff);
  GrColor_t cyan = rgbacolor(state, 0x00ffffff);
  
  GrVertex p1, p2, p3;
  char buf[256];
  int i;

  p1.x = 0.0f; p1.y = 0.0f;
  p2.x = 0.0f; p2.y = (float)state->conform_height;
  p3.x = (float)state->conform_width; p3.y = (float)state->conform_height;

  test_data *local_ptr = (test_data*)state->local_data;

  local_ptr = (test_data *)state->local_data;

  grCoordinateSpace(GR_WINDOW_COORDS);

  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE); 

  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_CONSTANT,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );

  // Clear the buffers, do some drawing in each, make sure
  // they swap.
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grBufferClear(0, 0, 0); 
  
  grRenderBuffer(GR_BUFFER_BACKBUFFER);
  grBufferClear(0, 0, 0); 

  grRenderBuffer(GR_BUFFER_BACKBUFFER);
  grConstantColorValue(red);
  grDrawTriangle(&p1, &p2, &p3);

  end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_BACKBUFFER);
  end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);

  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grConstantColorValue(green);
  grDrawTriangle(&p1, &p2, &p3);

  end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_BACKBUFFER);
  end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);

  grBufferSwap(1);

  end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_BACKBUFFER);
  end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);

  grBufferSwap(1);

  end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_BACKBUFFER);
  end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);


  // Now see if swap_interval actually waits for a retrace.  I'm
  // not really sure how to do this, so we'll try the statistical
  // approach. 

  FxI32 pos[2];
  FxI32 total = 0;

#define NUM_FRAMES 200

  for (i = 0; i < NUM_FRAMES; i++) {
    grBufferSwap(1);
    grFinish();
    grGet(GR_VIDEO_POSITION, sizeof(pos), pos);  
    total += pos[0];
  }

  // Now check if the average location of the vertical beam was
  // in the top 10% of the window.

  sprintf(buf, "waiting for vertical retrace may not be working.");
  if ((total/NUM_FRAMES) > (state->screen_height/10)) {
    log_status(state, TEST_FAIL, buf);
  } else {
    log_status(state, TEST_PASS, buf);
  }
  
  // Now check that grGet(SWAP_HISTORY) is working.
  FxI32 hist[20];
  FxI32 entries, num_entries; 
  FxBool passed = FXTRUE;
  

  memset(hist, 0, sizeof(hist));

  grGet(GR_NUM_SWAP_HISTORY_BUFFER, sizeof(num_entries), &num_entries);
  grGet(GR_SWAP_HISTORY, sizeof(hist), hist); //does this clear it?

  for (i = 0; i < num_entries; i++) {
    grBufferSwap(num_entries-i);
  }
  grFinish(); 

  entries = grGet(GR_SWAP_HISTORY, sizeof(hist), hist);
  if ((entries/4) != num_entries) {
    log_status(state, TEST_FAIL, 
               "grGet(GR_SWAP_HISTORY) didn't return correct number of entries.");
    passed = FXFALSE;
  } else {
    log_status(state, TEST_PASS, 
               "grGet(GR_SWAP_HISTORY) returned correct number of entries.");
  }

  for (i = 0; i < num_entries; i++) {
    if (hist[i] != i) {
      sprintf(buf, 
              "swap history [%d] was wrong (%d, should be %d).", i, hist[i], i);
      log_status(state, TEST_FAIL, buf);
      passed = FXFALSE;
    } else {
      log_status(state, TEST_PASS, "swap history matched.");
    }
  }

  return TEST_PASS; // return value ignored
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"    No test specific args\n");
}


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

static char *test_name = "query";
static char *test_description = "Call all flavors of grGet()";

// Data specific to this test.
typedef struct {
  int test_pass;
} test_data;


typedef struct {
  FxU32 which;
  char *name;
  FxU32 expected_size;
} QueryType;

static QueryType Queries[] = {
  { GR_BITS_DEPTH, "GR_BITS_DEPTH", 4, }, 
  { GR_BITS_RGBA, "GR_BITS_RGBA", 16, }, 
  { GR_FIFO_FULLNESS, "GR_FIFO_FULLNESS", 8, }, 
  { GR_FOG_TABLE_ENTRIES, "GR_FOG_TABLE_ENTRIES", 4, }, 
  { GR_GAMMA_TABLE_ENTRIES, "GR_GAMMA_TABLE_ENTRIES", 4, }, 
  { GR_GLIDE_STATE_SIZE, "GR_GLIDE_STATE_SIZE", 4, }, 
  { GR_GLIDE_VERTEXLAYOUT_SIZE, "GR_GLIDE_VERTEXLAYOUT_SIZE", 4, }, 
  { GR_IS_BUSY, "GR_IS_BUSY", 4, }, 
  { GR_LFB_PIXEL_PIPE, "GR_LFB_PIXEL_PIPE", 4, }, 
  { GR_MAX_TEXTURE_SIZE, "GR_MAX_TEXTURE_SIZE", 4, }, 
  { GR_MAX_TEXTURE_ASPECT_RATIO, "GR_MAX_TEXTURE_ASPECT_RATIO", 4, }, 
  { GR_MEMORY_FB, "GR_MEMORY_FB", 4, }, 
  { GR_MEMORY_UMA, "GR_MEMORY_UMA", 4, }, 
  { GR_MEMORY_TMU, "GR_MEMORY_TMU", 4, }, 
  { GR_NUM_BOARDS, "GR_NUM_BOARDS", 4, }, 
  { GR_NON_POWER_OF_TWO_TEXTURES, "GR_NON_POWER_OF_TWO_TEXTURES", 4, }, 
  { GR_NUM_FB, "GR_NUM_FB", 4, }, 
  { GR_NUM_SWAP_HISTORY_BUFFER, "GR_NUM_SWAP_HISTORY_BUFFER", 4, }, 
  { GR_NUM_TMU, "GR_NUM_TMU", 4, }, 
  { GR_PENDING_BUFFERSWAPS, "GR_PENDING_BUFFERSWAPS", 4, }, 
  { GR_REVISION_FB, "GR_REVISION_FB", 4, }, 
  { GR_REVISION_TMU, "GR_REVISION_TMU", 4, }, 
  { GR_STATS_LINES, "GR_STATS_LINES", 4, }, 
  { GR_STATS_PIXELS_AFUNC_FAIL, "GR_STATS_PIXELS_AFUNC_FAIL", 4, }, 
  { GR_STATS_PIXELS_CHROMA_FAIL, "GR_STATS_PIXELS_CHROMA_FAIL", 4, }, 
  { GR_STATS_PIXELS_DEPTHFUNC_FAIL, "GR_STATS_PIXELS_DEPTHFUNC_FAIL", 4, },
  { GR_STATS_PIXELS_IN, "GR_STATS_PIXELS_IN", 4, }, 
  { GR_STATS_PIXELS_OUT, "GR_STATS_PIXELS_OUT", 4, }, 
  { GR_STATS_POINTS, "GR_STATS_POINTS", 4, }, 
  { GR_STATS_TRIANGLES_IN, "GR_STATS_TRIANGLES_IN", 4, }, 
  { GR_STATS_TRIANGLES_OUT, "GR_STATS_TRIANGLES_OUT", 4, }, 
  { GR_SWAP_HISTORY, "GR_SWAP_HISTORY", 4 }, /* 4xN - no CVG */
  { GR_SUPPORTS_PASSTHRU, "GR_SUPPORTS_PASSTHRU", 4, }, 
  { GR_TEXTURE_ALIGN, "GR_TEXTURE_ALIGN", 4, }, 
  { GR_VIDEO_POSITION, "GR_VIDEO_POSITION", 8, }, 
  { GR_VIEWPORT, "GR_VIEWPORT", 16, }, 
  { GR_WDEPTH_MIN_MAX, "GR_WDEPTH_MIN_MAX", 8, }, 
  { GR_ZDEPTH_MIN_MAX, "GR_ZDEPTH_MIN_MAX", 8, }, 
  { GR_BITS_GAMMA, "GR_BITS_GAMMA", 4, }, 
};

#define NUM_QUERIES (sizeof(Queries)/sizeof(QueryType))

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
  state->num_reps = 1;

  if (state->num_color_bufs < 1) {
    state->num_color_bufs = 1; // need at least 1 color buf
  }

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

TestResult
test_viewport(conform_state *state, test_data *local_ptr, QueryType *query)
{
  FxI32 orig[4];
  FxI32 buf[4];

  grGet(query->which, sizeof(orig), orig); // save original

  grViewport(10, 20, 30, 40);
  grGet(query->which, sizeof(buf), buf); 

  grViewport(orig[0], orig[1], orig[2], orig[3]); // restore original

  if ((buf[0] != 10) ||
      (buf[1] != 20) ||
      (buf[2] != 30) ||
      (buf[3] != 40)) {
    return TEST_FAIL;
  }
  return TEST_PASS;
}

TestResult
test_lines(conform_state *state, test_data *local_ptr, QueryType *query)
{
  GrVertex p1, p2;
  FxI32 count;
  p1.x = 0.0f;
  p1.y = 0.0f;
  p1.x = 10.0f;
  p1.y = 20.0f;
  grReset (GR_STATS_LINES);

  int num_lines = nrand(100);
  for (int i = 0; i < num_lines; i++) { // draw some lines
    grDrawLine(&p1, &p2);
  }

  grGet(GR_STATS_LINES, sizeof(count), &count);

  if (count != num_lines) {
    return TEST_FAIL;
  }

  return TEST_PASS;
}

TestResult
test_tris(conform_state *state, test_data *local_ptr, QueryType *query)
{
  GrVertex p1, p2, p3;
  FxI32 count, last_count;
  p1.x = 0.0f;
  p1.y = 0.0f;
  p2.x = 10.0f;
  p2.y = 20.0f;
  p3.x = 0.0f;
  p3.y = 20.0f;
 
  grReset (GR_STATS_TRIANGLES);
  grFinish();

  grGet(query->which, sizeof(last_count), &last_count); // tris_in or tris_out

  for (int i = 0; i < 100; i++) {
    int num_tris = nrand(100)+10;
    for (int n = 0; n < num_tris; n++) { 
      grDrawTriangle(&p1, &p2, &p3);
    }
    grFinish();
    grGet(query->which, sizeof(count), &count);
    if (count != (last_count + num_tris)) {
      fprintf(stderr, "Unexpected value from TRIS_IN or TRIS_OUT\n");
      return TEST_FAIL;
    }
    last_count = count;
  }

  return TEST_PASS;
}

TestResult
test_pix(conform_state *state, test_data *local_ptr, QueryType *query)
{
  GrVertex p1, p2, p3, p4;
  FxI32 count, last_count;
  p1.x = 0.0f;
  p1.y = 0.0f;

  p2.x = 0.0f;
  p2.y = 20.0f;

  p3.x = 20.0f;
  p3.y = 20.0f;

  p4.x = 20.0f;
  p4.y = 0.0f;

 
  grReset (GR_STATS_PIXELS);
  grFinish();

  grGet(query->which, sizeof(last_count), &last_count); // PIX_IN or PIX_OUT

  for (int i = 0; i < 100; i++) {
    int num_tris = nrand(100)+10;
    for (int n = 0; n < num_tris; n++) { 
      grDrawTriangle(&p1, &p2, &p3);
      grDrawTriangle(&p1, &p3, &p4);
    }
    grFinish();
    grGet(query->which, sizeof(count), &count);

    if (count != (last_count + (num_tris*400))) {
      fprintf(stderr, 
        "Unexpected value from PIX_IN or PIX_OUT (count = %d, last_count = %d\n", 
              count, last_count);
      return TEST_FAIL;
    }
    last_count = count;
  }

  return TEST_PASS;
}


int do_conform(conform_state *state)
{
  FxI32 buf[32];
  FxU32 size = 0;
  QueryType *query = NULL;
  TestResult status = TEST_PASS;

  test_data *local_ptr = (test_data*)state->local_data;

  local_ptr = (test_data *)state->local_data;

  grCoordinateSpace(GR_WINDOW_COORDS);
  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE); 
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grConstantColorValue(rgbacolor(state, 0x00FF00FF));
  grBufferClear(0, 0, 0); 

  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_CONSTANT,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );


  query = Queries;
  for (int i = 0; i < NUM_QUERIES; i++) {
    size = grGet(query->which, sizeof(buf), buf);
    if (size == 0) {
      if (!log_status(state, status, "FAIL: 0 returned from GrGet()")) {
        return(TEST_FAIL);
      }
    } else if (size != query->expected_size) {

      if (query->which == GR_SWAP_HISTORY) {
        if ((size % 4) != 0) {
          if (!log_status(state, status, 
                      "FAIL: GrGet(GR_SWAP_HISTORY) returned invalid value")) {
            return(TEST_FAIL);
          }
        }
      } else {
        if (!log_status(state, status, 
                              "FAIL: GrGet() returned unexpected value")) {
          return(TEST_FAIL);
        }
      }
    }

    status = TEST_PASS;

    switch (query->which) {
      case GR_VIEWPORT:
       status = test_viewport(state, local_ptr, query);
       break;
      case GR_STATS_LINES:
       status = test_lines(state, local_ptr, query);
       break;
      case GR_STATS_TRIANGLES_IN:
       status = test_tris(state, local_ptr, query);
       break;
      case GR_STATS_TRIANGLES_OUT:
       status = test_tris(state, local_ptr, query);
       break;
      case GR_STATS_PIXELS_IN:
       status = test_pix(state, local_ptr, query); 
       break;
      case GR_STATS_PIXELS_OUT:
       status = test_pix(state, local_ptr, query);
       //end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
       break;
    }

    if (!log_status(state, status, query->name)) {
      return(TEST_FAIL);
    }
    query++;
  }

  return(TEST_PASS);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"    No test specific args\n");
}


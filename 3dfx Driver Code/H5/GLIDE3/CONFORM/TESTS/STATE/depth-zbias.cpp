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

#include "conform.h"
#include "vertex.h"
#include "vmglide.h"

static char *test_name = "depth-zbias";
static char *test_description = "Test Z buffer bias";

#define RGBA8888_RED    0xff0000ff
#define RGBA8888_GREEN  0x00ff00ff
#define RGBA8888_BLUE   0x0000ffff
#define RGBA8888_WHITE  0xffFFffFF
#define RGBA8888_BLACK  0x000000ff

// local data
typedef struct {
  GrVertex verts[12];  // Temporary vertex storage
  int debug;
  FxU32 *emu_color_buf;
  float *emu_depth_buf;
  FxU32 *screen_color_buf;
} test_data;

// Prototypes
void setup_prim(conform_state *state);
void setup_verts(conform_state *state);
FxBool emu_init(conform_state *state);
void emu_free(conform_state *state);
void emu_buffer_clear(conform_state *state, 
                      FxU32 clearcolor,  // RGBA
                      FxU32 alpha,       // ignored
                      FxU32 depth);
void draw_at_bias(conform_state *state, 
                  GrVertex *v,
                  FxU32 bias);
void emu_draw_at_bias(conform_state *state, 
                      GrVertex *v,
                      FxU32 bias,
                      GrDepthBufferMode_t mode,
                      GrCmpFnc_t func,
                      FxU32 color);
TestResult check_frame(conform_state *state);


int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;
  int num_tmu;

  state->test_name = test_name;
  state->test_description = test_description;

  if((state->local_data = malloc(sizeof(test_data))) == NULL) {
    // Could not allocate data for local memory
    log_perror(state,"Could not allocate memory for local data");
    return(0);
  } 

  local_ptr = (test_data *)state->local_data;

  local_ptr->emu_color_buf = NULL;
  local_ptr->emu_depth_buf = NULL;
  local_ptr->debug = 0;

  // pick a random TMU
  num_tmu = nrand((int)(state->num_tmu));

  // print out test dependent args
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    if(!_stricmp(*local_argv,"-debug") || // enable debug output
              !_stricmp(*local_argv,"/debug")) {
      local_ptr->debug = 1;
    } else {
      fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
      ++local_argv;
    }
  }


  // need an aux buffer for depth buffering
  state->num_aux_bufs = 1;

  // use 100x100 image area
  state->conform_width = 100;
  state->conform_height = 100;

  // tiling not supported
  state->tile = 0;

  // setup image file names
  // this is manditory
  frame_name(test_name, state);
  img_file_type(IMG_P6, state);

  // Initialization complete
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
  GrColor_t clearcolor;
  long depthrange[2];
  TestResult status = TEST_PASS;

  local_ptr = (test_data *)state->local_data;

  // TODO: This test expects to be at (0,0). In order to support
  //       arbitrary locations (ala -xylimit), check_frame() will
  //       need to be tweaked. 
  state->conform_x = 0;
  state->conform_y = 0;

  // clear front buffer to black
  clearcolor = rgbacolor(state, RGBA8888_BLACK);
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grColorMask(FXTRUE,FXFALSE);
  grDepthMask(FXFALSE);
  grCoordinateSpace(GR_WINDOW_COORDS);
  vmgrClipWindow(state,
                 state->conform_x,
                 state->conform_y,
                 state->conform_x+state->conform_width,
                 state->conform_y+state->conform_height);
  
  vmgrViewport(state,
               state->conform_x, state->conform_y, 
               state->conform_width, state->conform_height);
  
  if (!emu_init(state)) {
    log_perror(state, "Couldn't init emulation buffers");
    return 0;
  }

  local_ptr->screen_color_buf = (FxU32*)malloc(state->conform_width *
                                               state->conform_height *  
                                               sizeof(FxU32));
  if (!local_ptr->screen_color_buf) {
    log_perror(state, "Couldn't alloc screen buffer");
    emu_free(state); 
    return 0;
  }

  setup_verts(state);
  
  setup_prim(state);

  // get depth range
  grGet(GR_ZDEPTH_MIN_MAX, 8, depthrange);
  
  // Setup normal Z buffering
  grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
  grDepthBufferFunction(GR_CMP_GREATER);

  // clear the buffer
  grDepthMask(FXTRUE);
  grBufferClear(clearcolor,0x0000,depthrange[1]);
  emu_buffer_clear(state, RGBA8888_BLACK, 0x0000, depthrange[1]);
    
  // Draw squares
  // Init primitive color to red
  grConstantColorValue(rgbacolor(state,RGBA8888_RED));
  
  // set depth bias
  draw_at_bias(state, &local_ptr->verts[0], 0);
  emu_draw_at_bias(state, &local_ptr->verts[0], 0, 
                   GR_DEPTHBUFFER_ZBUFFER, GR_CMP_GREATER, RGBA8888_RED);

  if (!log_status(state, check_frame(state), "zbuffer bias 0 check")) {
      return(TEST_FAIL);
  }

  // Init primitive color to green
  grConstantColorValue(rgbacolor(state,RGBA8888_GREEN));
  
  // set depth bias
  draw_at_bias(state, &local_ptr->verts[4], 1);
  emu_draw_at_bias(state, &local_ptr->verts[4], 1,
                   GR_DEPTHBUFFER_ZBUFFER, GR_CMP_GREATER, RGBA8888_GREEN);

  if (!log_status(state, check_frame(state), "zbuffer bias 1 check")) {
      return(TEST_FAIL);
  }

  // Init primitive color to blue
  grConstantColorValue(rgbacolor(state,RGBA8888_BLUE));
  
  // set depth bias
  draw_at_bias(state, &local_ptr->verts[8], 2);
  emu_draw_at_bias(state, &local_ptr->verts[8], 2,
                   GR_DEPTHBUFFER_ZBUFFER, GR_CMP_GREATER, RGBA8888_BLUE);

  if (!log_status(state, check_frame(state), "zbuffer bias 2 check")) {
      return(TEST_FAIL);
  }
  
  // Init primitive color to white
  grConstantColorValue(rgbacolor(state,RGBA8888_WHITE));
  
  // set depth bias
  grDepthBufferFunction(GR_CMP_EQUAL);
  draw_at_bias(state, &local_ptr->verts[0], 0);
  emu_draw_at_bias(state, &local_ptr->verts[0], 0,
                   GR_DEPTHBUFFER_ZBUFFER, GR_CMP_EQUAL, RGBA8888_WHITE);

  if (!log_status(state, check_frame(state), "zbuffer bias 0 check")) {
      return(TEST_FAIL);
  }

  // Setup biased Z buffering
  grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS);
  grDepthBufferFunction(GR_CMP_GREATER);

  // clear the buffer
  grDepthMask(FXTRUE);
  grBufferClear(clearcolor,0x0000,depthrange[1]);
  emu_buffer_clear(state, RGBA8888_BLACK,0x0000,depthrange[1]);
    
  // Draw squares
  // Init primitive color to red
  grConstantColorValue(rgbacolor(state,RGBA8888_RED));
  
  // set depth bias
  draw_at_bias(state, &local_ptr->verts[0], 0);
  emu_draw_at_bias(state, &local_ptr->verts[0], 0,
                   GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS, 
                   GR_CMP_GREATER, RGBA8888_RED);

  if (!log_status(state, check_frame(state), "zbuffer compare to bias 0 check")) {
      return(TEST_FAIL);
  }

  // Init primitive color to green
  grConstantColorValue(rgbacolor(state,RGBA8888_GREEN));
  
  // set depth bias
  draw_at_bias(state, &local_ptr->verts[4], 1);
  emu_draw_at_bias(state, &local_ptr->verts[4], 1,
                   GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS, 
                   GR_CMP_GREATER, RGBA8888_GREEN);

  if (!log_status(state, check_frame(state), "zbuffer compare to bias 1 check")) {
      return(TEST_FAIL);
  }

  // Init primitive color to blue
  grConstantColorValue(rgbacolor(state,RGBA8888_BLUE));
  
  // set depth bias
  draw_at_bias(state, &local_ptr->verts[8], 2);
  emu_draw_at_bias(state, &local_ptr->verts[8], 2,
                   GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS, 
                   GR_CMP_GREATER, RGBA8888_BLUE);

  if (!log_status(state, check_frame(state), "zbuffer compare to bias 2 check")) {
      return(TEST_FAIL);
  }
  
  // Init primitive color to white
  grConstantColorValue(rgbacolor(state,RGBA8888_WHITE));
  
  // set depth bias
  grDepthBufferFunction(GR_CMP_EQUAL);

  draw_at_bias(state, &local_ptr->verts[0], 0);
  emu_draw_at_bias(state, &local_ptr->verts[0], 0,
                   GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS, 
                   GR_CMP_EQUAL, RGBA8888_WHITE);

  if (!log_status(state, check_frame(state), "zbuffer compare to bias 0 check")) {
      return(TEST_FAIL);
  }

  emu_free(state);
  free((void*)local_ptr->screen_color_buf);

  return(status); // failures are reported in individual check_frame()s
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -tmu <n>   texture unit to test\n");
}


void setup_prim(conform_state *state)
{
  test_data *local_ptr;

  local_ptr = (test_data *)state->local_data;

  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_Q , GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_Z, GR_VERTEX_OOZ_OFFSET << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_A,  GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_RGB,  GR_VERTEX_R_OFFSET << 2, GR_PARAM_ENABLE);  

  // set for max 3 TMUs for now
  grVertexLayout(GR_PARAM_ST0, GR_VERTEX_SOW_TMU0_OFFSET << 2,
		 GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_Q0, GR_VERTEX_OOW_TMU0_OFFSET << 2,
		 GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_ST1, GR_VERTEX_SOW_TMU1_OFFSET << 2,
		 GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_Q1, GR_VERTEX_OOW_TMU1_OFFSET << 2,
		 GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_ST2, GR_VERTEX_SOW_TMU2_OFFSET << 2,
		 GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_Q2, GR_VERTEX_OOW_TMU2_OFFSET << 2,
		 GR_PARAM_ENABLE);

  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
		 GR_COMBINE_FACTOR_NONE,
		 GR_COMBINE_LOCAL_CONSTANT,  
		 GR_COMBINE_OTHER_NONE,
		 FXFALSE);        

  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
		 GR_COMBINE_FACTOR_NONE,
		 GR_COMBINE_LOCAL_CONSTANT,  
		 GR_COMBINE_OTHER_NONE,
		 FXFALSE);        

  grAlphaBlendFunction(GR_BLEND_ONE,
		       GR_BLEND_ZERO,
		       GR_BLEND_ONE,
		       GR_BLEND_ZERO);

  grAlphaTestReferenceValue(0x00);
  grAlphaTestFunction(GR_CMP_NOTEQUAL);
}

void setup_verts(conform_state *state)
{
  test_data *local_ptr;

  local_ptr = (test_data *)state->local_data;

  local_ptr->verts[0].x = 18.0f;
  local_ptr->verts[0].y = 18.0f;
  local_ptr->verts[0].oow = 1.0f;
  local_ptr->verts[0].ooz = 100.0f;
  local_ptr->verts[1].x = 18.0f + 64.0f;
  local_ptr->verts[1].y = 18.0f;
  local_ptr->verts[1].oow = 1.0f;
  local_ptr->verts[1].ooz = 100.0f;
  local_ptr->verts[2].x = 18.0f + 64.0f;
  local_ptr->verts[2].y = 18.0f + 64.0f;
  local_ptr->verts[2].oow = 1.0f;
  local_ptr->verts[2].ooz = 100.0f;
  local_ptr->verts[3].x = 18.0f;
  local_ptr->verts[3].y = 18.0f + 64.0f;
  local_ptr->verts[3].oow = 1.0f;
  local_ptr->verts[3].ooz = 100.0f;

  local_ptr->verts[4].x = 18.0f;
  local_ptr->verts[4].y = 18.0f + 8.0f;
  local_ptr->verts[4].oow = 1.0f;
  local_ptr->verts[4].ooz = 100.0f;
  local_ptr->verts[5].x = 18.0f + 64.0f;
  local_ptr->verts[5].y = 18.0f + 8.0f;
  local_ptr->verts[5].oow = 1.0f;
  local_ptr->verts[5].ooz = 100.0f;
  local_ptr->verts[6].x = 18.0f + 64.0f;
  local_ptr->verts[6].y = 18.0f + 32.0f + 8.0f;
  local_ptr->verts[6].oow = 1.0f;
  local_ptr->verts[6].ooz = 100.0f;
  local_ptr->verts[7].x = 18.0f;
  local_ptr->verts[7].y = 18.0f + 32.0f + 8.0f;
  local_ptr->verts[7].oow = 1.0f;
  local_ptr->verts[7].ooz = 100.0f;

  local_ptr->verts[8].x = 18.0f;
  local_ptr->verts[8].y = 18.0f + 24.0f;
  local_ptr->verts[8].oow = 1.0f;
  local_ptr->verts[8].ooz = 100.0f;
  local_ptr->verts[9].x = 18.0f + 64.0f;
  local_ptr->verts[9].y = 18.0f + 24.0f;
  local_ptr->verts[9].oow = 1.0f;
  local_ptr->verts[9].ooz = 100.0f;
  local_ptr->verts[10].x = 18.0f + 64.0f;
  local_ptr->verts[10].y = 18.0f + 32.0f + 24.0f;
  local_ptr->verts[10].oow = 1.0f;
  local_ptr->verts[10].ooz = 100.0f;
  local_ptr->verts[11].x = 18.0f;
  local_ptr->verts[11].y = 18.0f + 32.0f + 24.0f;
  local_ptr->verts[11].oow = 1.0f;
  local_ptr->verts[11].ooz = 100.0f;
}

/*--------------------------------------------------------------------------
@func check_frame
@arg conform_state *state - the framework state
@return TEST_PASS if the emulated and actual images matched, TEST_FAIL otherwise
@html
Compare the emulated image to the HW generated screen.
@end
---------------------------------------------------------------------------*/
TestResult check_frame(conform_state *state) 
{
  char msg[128];
  int x, y;
  test_data *local_ptr = (test_data *)state->local_data;
  TestResult status = TEST_PASS;
  
  FxU32 *expected_ptr = local_ptr->emu_color_buf;
  FxU32 *screen_ptr = local_ptr->screen_color_buf;
  Color4D expected;
 
  // Take a snapshot of the screen.
  end_frame(END_OP_SAVE_TO_MEM, state, 
            (void**)&local_ptr->screen_color_buf, END_BUFFER_FRONTBUFFER);

  // Compare it to our emulated version.
  for (y = 0; y < state->conform_height; y++) {
    for (x = 0; x < state->conform_width; x++) {
      RGBA8888_TO_Color4D(*expected_ptr, &expected);
      if (!compare_color(state, *screen_ptr, &expected)) {
        sprintf(msg, "@(%d, %d), actual(0x%x) != expected(0x%x 0x%x 0x%x)", x, y,
                *screen_ptr,
                (int)(expected.red * 255.5f),
                (int)(expected.green * 255.5f),
                (int)(expected.blue * 255.5f));
        log_message(state, msg);
        status = TEST_FAIL;  
        //*expected_ptr = 0xffff0000; // debug
      }
      ++expected_ptr;
      ++screen_ptr;
    }
  }

  if (local_ptr->debug) {
    char filename[128];
    sprintf(filename, "%s/emu-%s-%02d", 
            state->output_dir, test_name, state->framecnt-1);
    save_image_to_file(state, 
                       filename,
                       local_ptr->emu_color_buf,
                       state->conform_width,
                       state->conform_height,
                       FXFALSE);
  }

  return status;
}

/*-------------------------------------------------------------------------
@func emu_buffer_clear
@arg conform_state *state - the framework state
@arg FxU32 clearcolor - the color to clear to
@arg FxU32 alpha      - the alpha to clear to (currently ignored)
@arg FxU32 depth      - the depth to clear to (used as a float)
@html
The analog to grBufferClear() for our simulated buffers
@end
--------------------------------------------------------------------------*/
void emu_buffer_clear(conform_state *state, 
                      FxU32 clearcolor,  // RGBA
                      FxU32 alpha,       // ignored
                      FxU32 depth)
{
  test_data *local_ptr = (test_data *)state->local_data;
  FxU32 *c = local_ptr->emu_color_buf;
  float *d = local_ptr->emu_depth_buf;
  int i;
  for (i = 0; i < state->conform_width * state->conform_height; i++) {
    *c++ = clearcolor;
    *d++ = (float)depth;
  }

}
  
/*-------------------------------------------------------------------------
@func emu_draw_at_bias
@arg conform_state *state - the framework state
@arg GrVertex *v      - list of 4 vertices 
@arg FxU32 bias       - the bias level to use for this rectangle
@arg GrDepthBufferMode_t mode - the depth buffer mode to use
@arg GrCmpFnc_t func - the comparison function to use
@arg FxU32           - the RGBA8888 color of these tris
@html
Emulate drawing a rectangle with the given bias level, color, depth buffer 
mode, and comparison function.
@end
--------------------------------------------------------------------------*/
void emu_draw_at_bias(conform_state *state, 
                      GrVertex *v,
                      FxU32 bias,
                      GrDepthBufferMode_t mode,
                      GrCmpFnc_t func,
                      FxU32 color)
{
  int x, y;
  int ystart, yend;
  FxU32 *color_ptr;
  float *depth_ptr;
  FxBool depthmask = FXTRUE;  // always update depth buffer
  test_data *local_ptr = (test_data *)state->local_data;

  // 0-1  We assume two triangles layed out in a square like
  // |/|  this.
  // 3-2
 
  int xloc   = (int)v[0].x;
  int yloc   = (int)v[0].y;
  int width  = (int)(v[1].x - v[0].x);
  int height = (int)(v[3].y - v[0].y);

  // Start filling pixels.
  // N.B. We don't do any bounds checking!

#ifdef ORIGIN_LL
    ystart = state->conform_height - yloc - height;
#else // UL, default
    ystart = yloc;
#endif 

  yend = ystart + height;

  for (y = ystart; y < yend; y++) {
    color_ptr = local_ptr->emu_color_buf;
    color_ptr += ((y * state->conform_width) + xloc);
    depth_ptr = local_ptr->emu_depth_buf;
    depth_ptr += ((y * state->conform_width) + xloc);

    for (x = 0; x < width; x++) {

      float res_depth = 0.0f;
      FxU32 res_color = 0xf0f0f0f0;

      vmgrDepthBufferFunction(func, 
                              mode,
                              *depth_ptr, *color_ptr,
                              v[0].ooz, color,
                              bias,
                              &res_depth, &res_color);
      if (depthmask) {
        *depth_ptr = res_depth;
      }
      *color_ptr = res_color;

      ++color_ptr;
      ++depth_ptr;
    }
  }
}

/*-------------------------------------------------------------------------
@func draw_at_bias
@arg conform_state *state - the framework state
@arg GrVertex *v - a list of 4 vertices
@arg FxU32 bias - the bias level to use
@html
Draw two triangles at bias.
@end
--------------------------------------------------------------------------*/
void draw_at_bias(conform_state *state, 
                  GrVertex *v,
                  FxU32 bias) 
{
  grDepthBiasLevel(bias);

  grDrawTriangle(&v[0], &v[1], &v[2]);
  grDrawTriangle(&v[0], &v[2], &v[3]);
}

/*-------------------------------------------------------------------------
@func emu_free
@arg conform_state *state - the framework state
@html
Free memory allocated by the emulation stuff
@end
--------------------------------------------------------------------------*/
void emu_free(conform_state *state)
{
  test_data *local_ptr = (test_data *)state->local_data;
  if (local_ptr->emu_color_buf) {
     free((void*)local_ptr->emu_color_buf);
  }
  if (local_ptr->emu_depth_buf) {
     free((void*)local_ptr->emu_depth_buf);
  }
}


/*-------------------------------------------------------------------------
@func emu_init
@arg conform_state *state - the framework state
@html
Set up the emulator state. (Just allocs memory at the moment)
@end
--------------------------------------------------------------------------*/
FxBool emu_init(conform_state *state)
{
  test_data *local_ptr = (test_data *)state->local_data;

  local_ptr->emu_color_buf = 
    (FxU32*)malloc(state->conform_width * state->conform_height * sizeof(FxU32));

  if (!local_ptr->emu_color_buf) {
     log_message(state, "Couldn't alloc software color buffer!");
     return FXFALSE;
  }

  local_ptr->emu_depth_buf = 
    (float*)malloc(state->conform_width * state->conform_height * sizeof(float));

  if (!local_ptr->emu_depth_buf) {
     free((void*)local_ptr->emu_color_buf);
     log_message(state, "Couldn't alloc software depth buffer!");
     return FXFALSE;
  }

  return FXTRUE;
}

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

static char *test_name = "depth-wbuffer-sc";
static char *test_description = "Test basic W buffering";


// local data
typedef struct {
  GrVertex verts[4];  // Temporary vertex storage
  int debug;
} test_data;

typedef struct {
  GrCmpFnc_t function;
  FxBool  depthmask;
  char *name;
} ModeType;

static ModeType Modes[] = {
  {GR_CMP_ALWAYS,    FXTRUE,   "GR_CMP_ALWAYS, FXTRUE"},
  {GR_CMP_NEVER,     FXTRUE,   "GR_CMP_NEVER, FXTRUE"},
  {GR_CMP_LESS,      FXTRUE,   "GR_CMP_LESS, FXTRUE"},
  {GR_CMP_EQUAL,     FXTRUE,   "GR_CMP_EQUAL, FXTRUE"},
  {GR_CMP_LEQUAL,    FXTRUE,   "GR_CMP_LEQUAL, FXTRUE"},
  {GR_CMP_GREATER,   FXTRUE,   "GR_CMP_GREATER, FXTRUE"},
  {GR_CMP_GEQUAL,    FXTRUE,   "GR_CMP_GEQUAL, FXTRUE"},
  {GR_CMP_NOTEQUAL,  FXTRUE,   "GR_CMP_NOTEQUAL, FXTRUE"},
  {GR_CMP_EQUAL,     FXFALSE,  "GR_CMP_EQUAL,  FXFALSE"},
};

#define NUM_MODES  (sizeof(Modes) / sizeof(ModeType))


// Prototypes
void setup_verts(conform_state *state);
void draw_at_depth(conform_state *state, float depth);

FxU32* generate_image(conform_state *state,
                      GrCmpFnc_t cmp_function,
                      GrColor_t clearcolor,
                      FxU32 cleardepth,
                      FxU32 box1_color,
                      float box1_depth,
                      FxU32 box2_color,
                      float box2_depth,
                      FxBool depthmask);

TestResult check_frame(conform_state *state,
                       FxU32* emulated_image,
                       FxU32* screen);

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

  // use 40x40 image area
  state->conform_width = 40;
  state->conform_height = 40;

  // tiling not supported
  state->tile = 0;

  // setup image file names
  // this is manditory
  frame_name(test_name, state);
  img_file_type(IMG_P6,state);

  // Initialization complete

  return(-1);
}

void close_conform(conform_state *state)
{
  // Free up local data
  free(state->local_data);
}


#define RGBA8888_BLACK 0x000000ff 
#define RGBA8888_RED   0xff0000ff 
#define RGBA8888_GREEN 0x00ff00ff 

int do_conform(conform_state *state)
{
  test_data *local_ptr;
  long depthrange[2];
  int depth, mode;
  float box1_depth, box2_depth; // the depth at which we draw the 2nd poly
  FxU32 box1_color, box2_color;
  long clear_depth = 0;
  TestResult status = TEST_PASS;

  GrColor_t clear_color = rgbacolor(state, RGBA8888_BLACK); 
  FxBool depthmask = FXTRUE;

  local_ptr = (test_data *)state->local_data;

  // TODO: This test expects to be at (0,0). In order to support
  //       arbitrary locations (ala -xylimit), check_frame() will
  //       need to be tweaked. 
  state->conform_x = 0;
  state->conform_y = 0; 

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
  
  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_Q , GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);

  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
         GR_COMBINE_FACTOR_NONE,
         GR_COMBINE_LOCAL_CONSTANT,  
         GR_COMBINE_OTHER_NONE,
         FXFALSE);        

  setup_verts(state); // init our vertices to be a box @ (10,10)
  
  // get depth range
  grGet(GR_WDEPTH_MIN_MAX, 8, depthrange);
  grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
 
  clear_depth = depthrange[0]; 
  box1_color = RGBA8888_RED;
  box2_color = RGBA8888_GREEN;

  for(mode = 0; mode < NUM_MODES; mode++) { 
    for (depth = 0; depth < 3; depth++) {

      grDepthMask(FXTRUE);
      grBufferClear(clear_color, 0x0000, clear_depth);

      depthmask = Modes[mode].depthmask;

      grDepthMask(depthmask);

      // Always draw first (red) box
      grDepthBufferFunction(GR_CMP_ALWAYS);

      // Set color to red      
      grConstantColorValue(rgbacolor(state, box1_color));
      box1_depth = 1.0f/(float)(depthrange[1]  * 0.5f);
      draw_at_depth(state, box1_depth);

      grDepthBufferFunction(Modes[mode].function);

      // Set color to green
      grConstantColorValue(rgbacolor(state, box2_color));
      switch (depth) {
        case 1: // ==
          box2_depth =  1.0f/(float)(depthrange[1]  * 0.5f);
          break;
        case 2: // <<
          box2_depth =  1.0f/(float)(depthrange[1]  * 0.75f);
          break;
        default: // << 
          box2_depth =  1.0f/(float)(depthrange[1]  * 0.25f);
          break;
      }
      draw_at_depth(state, box2_depth);


      // allocate screen for automated compare
      FxU32 *screen = 0;
      if((screen = (FxU32 *)malloc(state->conform_width*
                                     state->conform_height*
                                     sizeof(FxU32))) == NULL) {
        // fail
        log_message(state,"Could not allocate memory buffer for screen image");
      }

      // Get a memory snapshot of the render buffer
      end_frame(END_OP_SAVE_TO_MEM, state, (void**)&screen, 
                END_BUFFER_FRONTBUFFER);

      // Now create our emulated frame.
      FxU32 *expected =
        generate_image(state,
                       Modes[mode].function,
                       RGBA8888_BLACK,
                       clear_depth,
                       box1_color, // red
                       box1_depth, 
                       box2_color, // green
                       box2_depth,
                       depthmask);

      // Now compare the two images.
      if (!log_status(state, check_frame(state, expected, screen),
                      Modes[mode].name)) {
        free((void*)screen);
      
        if (expected) { 
          free((void*)expected);
        }
        return(TEST_FAIL);
      }

      // free the alloc'd memory
      free((void*)screen);
      
      if (expected) { 
        free((void*)expected);
      }
    }    
  }

  return(status);
}

void help_conform(conform_state *state)
{
  fprintf(stderr, "   -debug   dump debugging images (use with -dump)\n");
}


/*--------------------------------------------------------------------------
@func setup_verts
@arg conform_state *state - the framework state
@return none
@html
Initialize our vertices so we can draw a 10x10 square @ 10,10
@end
---------------------------------------------------------------------------*/
void setup_verts(conform_state *state)
{
  test_data *local_ptr = (test_data *)state->local_data;

  // Though we don't use it, set a Z value to find Z vs. W bugs
  local_ptr->verts[0].x = 10.0f;
  local_ptr->verts[0].y = 10.0f;
  local_ptr->verts[0].ooz = 1.0f;

  local_ptr->verts[1].x = 20.0f;
  local_ptr->verts[1].y = 10.0f;
  local_ptr->verts[1].ooz = 1.0f;

  local_ptr->verts[2].x = 20.0f;
  local_ptr->verts[2].y = 20.0f;
  local_ptr->verts[2].ooz = 1.0f;

  local_ptr->verts[3].x = 10.0f;
  local_ptr->verts[3].y = 20.0f;
  local_ptr->verts[3].ooz = 1.0f;

}

/*--------------------------------------------------------------------------
@func draw_at_depth
@arg conform_state *state - the framework state
@arg float depth - the depth at which to draw
@return none
@html
Draw our two triangles at a given depth.
@end
---------------------------------------------------------------------------*/
void draw_at_depth(conform_state *state, float depth)
{
  test_data *local_ptr = (test_data *)state->local_data;
  for (int i = 0; i < 4; i++) {
    local_ptr->verts[i].oow = depth;
  }

  grDrawTriangle(&local_ptr->verts[0],
                 &local_ptr->verts[1],
                 &local_ptr->verts[2]);

  grDrawTriangle(&local_ptr->verts[0],
                 &local_ptr->verts[2],
                 &local_ptr->verts[3]);
}

/*--------------------------------------------------------------------------
@func generate_image
@arg conform_state *state - the framework state
@arg GrCmpFnc_t cmp_function - 
@arg FxU32 clearcolor - the color to clear the background to
@arg FxU32 cleardepth - the depth at which to initialize the depth buffer
@arg FxU32 box1_color - the color of the 1st box we draw
@arg float box1_depth - the depth of the 1st box we draw
@arg FxU32 box2_color - the color of the 2nd box we try to draw
@arg float box2_depth - the depth of the 2nd box we try to draw
@arg FxBool depthmask - whether or not we update the depth buffer while
                        drawing the first box
@return FxU32 *data - the resulting image in RGBA8888 format
@html
@end
---------------------------------------------------------------------------*/
FxU32* generate_image(conform_state *state,
                      GrCmpFnc_t cmp_function,
                      FxU32 clearcolor,
                      FxU32 cleardepth,
                      FxU32 box1_color,
                      float box1_depth,
                      FxU32 box2_color,
                      float box2_depth,
                      FxBool depthmask)
{
  test_data *local_ptr = (test_data *)state->local_data;
  int x, y;

  // Make our own color and depth buffers.
  float *depth_buffer = 
    (float*)malloc(state->conform_width * state->conform_height * 
                   sizeof(float));

  if (!depth_buffer) {
    log_message(state, "Couldn't alloc depth test buffer");
    return NULL;
  }

  // We'll use 8888 format for our color buffer
  FxU32 *color_buffer = 
    (FxU32*)malloc(state->conform_width * state->conform_height * 
                   sizeof(FxU32));

  if (!color_buffer) {
    free((void*)depth_buffer);
    log_message(state, "Couldn't alloc color test buffer");
    return NULL;
  }

  // Clear the color buffer.
  FxU32 *color_ptr = color_buffer;

  for (y = 0; y < state->conform_height; y++) {
    for (x = 0; x < state->conform_width; x++) {
      *color_ptr++ = clearcolor;
    }
  }

  // Clear the depth buffer.
  float *depth_ptr = (float*)depth_buffer;

  for (y = 0; y < state->conform_height; y++) {
    for (x = 0; x < state->conform_width; x++) {
      *depth_ptr++ = (float)cleardepth;
    }
  }

  // Now draw our 2 boxes.
  // 0 --- 1
  // |     | 
  // 3 --- 2
  //
  GrVertex *v;
  
  int xloc, yloc, width, height;
  int ystart, yend;
  float cur_depth;
  GrCmpFnc_t compare_function;
  FxU32 box_color = 0;

  for (int box = 0; box < 2; box++) {    

    if (box == 0) {
      compare_function = GR_CMP_ALWAYS;  // always draw first box 
      cur_depth = box1_depth;
      box_color = box1_color;
    } else {
      compare_function = cmp_function; 
      cur_depth = box2_depth;
      box_color = box2_color;
    }
    v = &local_ptr->verts[0];

    xloc   = (int)v[0].x;
    yloc   = (int)v[0].y;
    width  = (int)(v[1].x - v[0].x);
    height = (int)(v[3].y - v[0].y);

    // printf("  %d x %d rect @ %d, %d\n", width, height, xloc, yloc);

#ifdef ORIGIN_LL
    ystart = state->conform_height - yloc - height;
#else // UL, default
    ystart = yloc;
#endif 
    yend = ystart + height;

    // Start filling pixels.
    // N.B. We don't do any bounds checking!
    for (y = ystart; y < yend; y++){

      color_ptr = color_buffer + (y * state->conform_width) + xloc;  
      depth_ptr = depth_buffer + (y * state->conform_width) + xloc;  

      for (x = 0; x < width; x++) {

        float res_depth; 
        FxU32 res_color;

        vmgrDepthBufferFunction(compare_function, 
                                GR_DEPTHBUFFER_WBUFFER, // w buffering
                                *depth_ptr, *color_ptr,
                                cur_depth, box_color,
                                0, // bias
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

  free(depth_buffer);
  return color_buffer;
}

/*--------------------------------------------------------------------------
@func check_frame
@arg conform_state *state - the framework state
@arg FxU32 *expected_ptr - pointer to the RGBA8888 emulated color buffer 
@arg FxU32 *screen_ptr - pointer to the RGB565 snapshot of the renderbuffer
@return TEST_PASS if the images matched, TEST_FAIL otherwise
@html
Compare the emulated image to the HW generated screen.
@end
---------------------------------------------------------------------------*/
TestResult check_frame(conform_state *state, 
                       FxU32 *expected_ptr, FxU32 *screen_ptr)
{
  int x, y;
  char msg[256];
  Color4D expected;
  TestResult status = TEST_PASS;
  test_data *local_ptr = (test_data *)state->local_data; 
  FxU32 *sw_image = expected_ptr;

  if (!expected_ptr || !screen_ptr) {

    status = TEST_FAIL;

  } else {
    for (y = 0; y < state->conform_height; y++){
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
          *expected_ptr = 0x0000ffff;  // debug
        }
        ++expected_ptr;
        ++screen_ptr;
      }
    }
  }

  if (local_ptr->debug) {
    char filename[128];
    sprintf(filename, "%s/emu-%s-%02d", 
            state->output_dir, test_name, state->framecnt-1);
    save_image_to_file(state, 
                       filename,
                       sw_image,
                       state->conform_width,
                       state->conform_height,
                       FXFALSE);
  }

  return status;
}


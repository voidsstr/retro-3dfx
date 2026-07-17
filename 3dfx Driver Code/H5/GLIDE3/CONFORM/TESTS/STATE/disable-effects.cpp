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

static char *test_name = "disable-effects";
static char *test_description = "Tests that grDisableAllEffects() works";

// Data specific to this test.
typedef struct {
  int test_pass;
  GrVertex tri1[3];
  GrVertex tri2[3];
  GrColor_t color1;
  GrColor_t color2;
} test_data;

char *PassNames[] = {  // these effects are tested individually
 "Alpha Blending",
 "Fog",
 "Chroma Keying",
 "Depth Buffering",
 "Alpha Testing",
};

#define NUM_PASSES (sizeof(PassNames) / sizeof(char *))

void init_verts(conform_state *state, test_data *local_ptr)
{
  GrVertex *tri1 = local_ptr->tri1;
  GrVertex *tri2 = local_ptr->tri2;

  int x = state->conform_x;
  int y = state->conform_y;
  int width = state->conform_width;
  int height = state->conform_height;

  // Specify a vertex layout
  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_A,  GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_Q,   GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_W,  GR_VERTEX_W_OFFSET << 2, GR_PARAM_ENABLE);  

  tri1[0].x = (float)x;
  tri1[0].y = (float)y;
  tri1[0].a = 128.0f;
  tri1[0].oow = 1.0f/50.0f;
  tri1[0].w = 1.0f;

  tri1[1].x = (float)x + width;
  tri1[1].y = (float)y;
  tri1[1].a = 128.0f;
  tri1[1].oow = 1.0f/50.0f;
  tri1[1].w = 1.0f;

  tri1[2].x = (float)(x + width/2);
  tri1[2].y = (float)(y + height);
  tri1[2].a = 128.0f;
  tri1[2].oow = 1.0f/50.0f;
  tri1[2].w = 1.0f;

  tri2[0].x = (float)x;
  tri2[0].y = (float)(y + height);
  tri2[0].a = 64.0f;
  tri2[0].oow = 1.0f/20.0f;
  tri2[0].w = 1.0f;

  tri2[1].x = (float)(x + width);
  tri2[1].y = (float)(y + height);
  tri2[1].a = 64.0f;
  tri2[1].oow = 1.0f/100.0f;
  tri2[1].w = 1.0f;

  tri2[2].x = (float)(x + width/2);
  tri2[2].y = (float)y;
  tri2[2].a = 64.0f;
  tri2[2].oow = 1.0f/60.0f;
  tri2[2].w = 1.0f;
}

/*--------------------------------------------------------------------
@func draw_triangles
@arg test_data *local_ptr - the test specific state
@html
Draw both triangles, log the vertex data.
@end
---------------------------------------------------------------------*/
void draw_triangles(conform_state *state, test_data *local_ptr)
{ 
  GrVertex *tri1 = local_ptr->tri1;
  GrVertex *tri2 = local_ptr->tri2;

  // Draw first triangle in red
  grConstantColorValue(local_ptr->color1);
  grDrawTriangle(&tri1[0], &tri1[1], &tri1[2]);
  add_command_data(state, "grDrawTriangle()", 0);
  add_vertex_data(state, tri1, 3);

  // Draw second triangle in green
  grConstantColorValue(local_ptr->color2);
  grDrawTriangle(&tri2[0], &tri2[1], &tri2[2]);
  add_command_data(state, "grDrawTriangle()", 0);
  add_vertex_data(state, tri2, 3);
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

  if (state->num_aux_bufs < 1) {
    state->num_aux_bufs = 1; // need at least 1 aux buf
  }

  // xycheck and xylimit not supported (fix this when adding self-checking)
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
  test_data *local_ptr = (test_data*)state->local_data;

  local_ptr = (test_data *)state->local_data;
  local_ptr->color1 = rgbacolor(state, 0xFF0000FF); // red
  local_ptr->color2 = rgbacolor(state, 0x00FF00FF); // green

  grColorMask(FXTRUE,FXFALSE);
  grDepthMask(FXFALSE);
  grCoordinateSpace(GR_WINDOW_COORDS);
  grClipWindow(state->conform_x,
	       state->conform_y,
	       state->conform_x+state->conform_width,
	       state->conform_y+state->conform_height);

  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grBufferClear(0, 0, 0); 

  init_verts(state, local_ptr);

  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_CONSTANT,
                 GR_COMBINE_OTHER_CONSTANT, // for chroma keying
                 FXFALSE );

  for (int pass = 0; pass < NUM_PASSES; pass++) {

    grBufferClear(0, 0, 0); 
    grDisableAllEffects(); 

    switch (pass) {
      case 0:  { // Alpha Blending
        // init alpha blending
        grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
                       GR_COMBINE_FACTOR_NONE,
                       GR_COMBINE_LOCAL_ITERATED,
                       GR_COMBINE_OTHER_NONE,
                       FXFALSE );
  
        grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                             GR_BLEND_ZERO, GR_BLEND_ZERO );
      }
      break;
      case 1: { // Fog
       // Disable alpha blending
       grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, 
                            GR_BLEND_ONE, GR_BLEND_ZERO);

        // Generate a fog table
        long ftsize;
        grGet(GR_FOG_TABLE_ENTRIES, 4, &ftsize);
        GrFog_t *fogtable = (GrFog_t*)malloc(sizeof(GrFog_t)*ftsize);
 
        if (fogtable) {
          grFogMode(GR_FOG_WITH_TABLE_ON_Q);
          grFogColorValue(rgbacolor(state, 0x0000FFFF));
          guFogGenerateExp(fogtable, .01f);
          grFogTable(fogtable);
        } else {
          log_perror(state, "Couldn't allocate memory for fog table!");
          break;
        }
      }
      break;
      case 2: { // Chroma keying
        // enable chroma-keying 
        grChromakeyMode(GR_CHROMAKEY_ENABLE); 
        // set the reference color (the color to discard)
        grChromakeyValue(local_ptr->color1);
      }
      break;
      case 3: { // depth buffering
        FxU32 depth_range[2];
        grDepthMask(FXTRUE);
        grGet(GR_WDEPTH_MIN_MAX, 8, (long*)depth_range);   // min, ma
        grBufferClear(0, 0, depth_range[0]);
        grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER); 
        grDepthBufferFunction(GR_CMP_GREATER); // 1/smaller wins 
      }
      break;
      case 4: { // alpha testing
        grAlphaTestFunction(GR_CMP_GREATER);
        grAlphaTestReferenceValue((GrAlpha_t)96.0f);
      }
      break;
    }

    draw_triangles(state, local_ptr); // draw with state set
    end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);

    grBufferClear(0, 0, 0); 
    grDisableAllEffects(); // should disable effects
    draw_triangles(state, local_ptr); // draw again
    end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);


  }

  local_ptr->test_pass++;

  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"    No test specific args\n");
}


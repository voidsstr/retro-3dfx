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

static char *test_name = "ras-edge";
static char *test_description = "Draw lines and triangles at the edges of the FB using clip and native coords";

#define MAX_VERTS 4      // Max # of verts we'll use

#define CLIP_COORDS_W_VALUE 3.0f // arbitrary

// Data specific to this test.
typedef struct {
  int test_pass;
  GrVertex verts[MAX_VERTS]; // pointer to all of our primitive verts
  int num_verts;       // number of vertices currently in use
  int do_aa;           // enable anti-aliasing
  int use_window_coords; // use window coords instead of clip coords
  int cur_rep;
} test_data;


/*----------------------------------------------------------------
@func init_verts
@arg conform_state *state     - a pointer to the conform state
@arg test_data     *local_ptr - a pointer to our local_data structure 
                          (which holds the vertices we'll be init'ing)
@html
Initialize our vertices to be the 4 edges of the window.
@end
-----------------------------------------------------------------*/
void init_verts(conform_state *state, test_data *local_ptr) {
  GrVertex *verts =  local_ptr->verts;
  float rgb = 1.0f;   // default color
  float alpha = 0.5f; // default alpha
 
  // Specify a vertex layout
  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_A,  GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE);  

  local_ptr->num_verts = 4;

  if (local_ptr->use_window_coords) {
    // from (0, 0) to (screen_width-1, screen_height -1)
    verts[0].x =  (float)state->conform_x;
    verts[0].y =  (float)state->conform_y;
 
    verts[1].x =  (float)state->conform_x + state->conform_width - 1.0f;
    verts[1].y =  (float)state->conform_y;

    verts[2].x =  (float)state->conform_x + state->conform_width - 1.0f;
    verts[2].y =  (float)state->conform_y + state->conform_height - 1.0f;

    verts[3].x =  (float)state->conform_x;
    verts[3].y =  (float)state->conform_y + state->conform_height - 1.0f;

  } else { // clip coords, [-1.0 to +1.0, inclusive]

    grVertexLayout(GR_PARAM_W,  GR_VERTEX_W_OFFSET << 2, GR_PARAM_ENABLE);  
    verts[0].x = -1.0f * CLIP_COORDS_W_VALUE;
    verts[0].y = -1.0f * CLIP_COORDS_W_VALUE;
    verts[0].w = CLIP_COORDS_W_VALUE;

    verts[1].x =  1.0f * CLIP_COORDS_W_VALUE;
    verts[1].y = -1.0f * CLIP_COORDS_W_VALUE;
    verts[1].w = CLIP_COORDS_W_VALUE;

    verts[2].x =  1.0f * CLIP_COORDS_W_VALUE;
    verts[2].y =  1.0f * CLIP_COORDS_W_VALUE;
    verts[2].w = CLIP_COORDS_W_VALUE;

    verts[3].x = -1.0f * CLIP_COORDS_W_VALUE;
    verts[3].y =  1.0f * CLIP_COORDS_W_VALUE;
    verts[3].w = CLIP_COORDS_W_VALUE;
  }

  if (local_ptr->use_window_coords) {
     alpha = 127.5f;
  }

  for (int i = 0; i < local_ptr->num_verts; i++) {
    verts[i].a = alpha;
  }
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
  local_ptr->num_verts = 0;
  local_ptr->do_aa = 0; // aa off
  local_ptr->use_window_coords = 0; // use clip coords by default
  local_ptr->cur_rep = 0;

  // print out test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    if(!_stricmp(*local_argv,"-aa") || // anti aliasing 
              !_stricmp(*local_argv,"/aa")) {
      local_ptr->do_aa = 1;
    } else if(!_stricmp(*local_argv,"-wc") || // use window coords
              !_stricmp(*local_argv,"/wc")) {
      local_ptr->use_window_coords = 1;
    } else {
      fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
      ++local_argv;
      ++num_arg;
    }
    ++local_argv;
    ++num_arg;
  }

  // don't use any visuals with aux buffers
  state->num_aux_bufs = 0;
  
  // we need at least one color buffer
  if (state->num_color_bufs < 1) {
    state->num_color_bufs = 1;
  }

  // xycheck and xylimit not supported (fix this when adding self-checking)
  state->xycheck = 0;
  state->xylimit = 0;

  // setup image file names
  // this is manditory
  char tmp_str[128];
  sprintf(tmp_str,"%s%s%s", 
          test_name,
          (local_ptr->use_window_coords? "-wc" : ""),
          (local_ptr->do_aa? "-aa" : ""));

  frame_name(tmp_str,state);
  img_file_type(IMG_P6,state);

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

  // this test expects to use the whole screen
  state->conform_x = 0;
  state->conform_y = 0;
  state->conform_width = state->screen_width;
  state->conform_height = state->screen_height;

  grClipWindow(state->conform_x,    
               state->conform_y, 
               state->conform_x + state->conform_width, 
               state->conform_y + state->conform_height);

  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grBufferClear(rgbacolor(state, 0x404040ff), 0, 0);
 
  if (!local_ptr->use_window_coords) {
    grCoordinateSpace(GR_CLIP_COORDS);
    grDepthRange( vnear, vfar );
    grViewport(state->conform_x, state->conform_y, 
               state->conform_width, state->conform_height);
  }

  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_CONSTANT,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );

  // Always draw with alpha enabled, so we can look for double pixel
  // writing.
  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_ITERATED,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );

  grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                       GR_BLEND_ZERO, GR_BLEND_ZERO );

  // Turn on AA, if applicable.
  if (local_ptr->do_aa) {
    grEnable(GR_AA_ORDERED);
  } else {
    grDisable(GR_AA_ORDERED);
  }

  init_verts(state, local_ptr); 

  int num_verts = local_ptr->num_verts; // filled in in init_line_verts()
  GrVertex *verts = local_ptr->verts;

  grConstantColorValue(rgbacolor(state, 0x00ff0080)); //green

  // Draw a line for each pair of vertices
  for (v = 0; v < num_verts; v++) {
    add_command_data(state, "grDrawLine()", 0);
    grDrawLine(&verts[v], 
               &verts[(v+1)%num_verts]);
    // record the vertices into the data file
    add_vertex_data(state, &verts[v], 2);
  }

  sprintf(tmp_str,"test data: rep# %d\n", local_ptr->cur_rep);
  add_data(state, tmp_str, strlen(tmp_str));

  if(state->tile) {
    if(state->last_rep) {
      int x,y,width,height;

      // save conform width
      x = state->conform_x;
      y = state->conform_y;
      width = state->conform_width;
      height = state->conform_height;

      // set to full frame
      state->conform_x = 0;
      state->conform_y = 0;
      state->conform_width = state->screen_width;
      state->conform_height = state->screen_height;

      // save tiled frame
      end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);

      // restore conform width
      state->conform_x = x;
      state->conform_y = y;
      state->conform_width = width;
      state->conform_height = height;
    }
  } else {
    end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
  }

  sprintf(tmp_str,"pass = %d, tsx= %d sy = %d wid = %d hei = %d",
    local_ptr->test_pass++,
    state->conform_x,
    state->conform_y,
    state->conform_width,
    state->conform_height);
  log_message(state,tmp_str);
 
  local_ptr->cur_rep++; // can be different than test_pass!

  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr, "   -aa     enable anti-aliasing\n");
  fprintf(stderr, "   -wc     use window coords instead of clip coords\n");
}


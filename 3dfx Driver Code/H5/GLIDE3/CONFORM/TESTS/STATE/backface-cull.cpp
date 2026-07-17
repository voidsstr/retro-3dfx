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
#include <stddef.h>

#include "conform.h"
#include "vertex.h"
#include "vmglide.h"
#include "matrix.h"

static char *test_name = "backface-cull";
static char *test_description = "rotate solid color prims along the Y axis, testing backface culling (grCullMode())";


#define SCALE_TWEAK .35f // scale down rotated prims to keep them in their 1x1 bbox

#define MAX_VERTS 10 // The maximum # of vertices we'll be dealing with

#define CLIP_COORDS_W_VALUE 3.0f // arbitrary

#define PRIM_TRIANGLES         0
#define PRIM_LINES             1
#define PRIM_POINTS            2
#define PRIM_VA_POINTS         3
#define PRIM_VA_TRIANGLES      4
#define PRIM_VA_TRIANGLE_STRIP 5
#define PRIM_VA_TRIANGLE_FAN   6
#define PRIM_VA_POLYGON        7
#define PRIM_VA_LINES          8
#define PRIM_VA_LINE_STRIP     9

#define NUM_PRIMS 10

// The rotations we'll use. Stick in whatever edge case you want...
static float rotations[] = {
    0.0f,
   45.0f,
   90.0f,
  135.0f,
  180.0f,
  225.0f,
  270.0f,
  315.0f,
  360.0f,
};
#define NUM_ROTATIONS  (sizeof(rotations) / sizeof(float))


// The culling modes we'll be testing...
static GrCullMode_t cull_modes[] = {
  GR_CULL_DISABLE,
  GR_CULL_NEGATIVE,
  GR_CULL_POSITIVE,
};
#define  NUM_CULL_MODES (sizeof(cull_modes)/sizeof(FxU32))


// The various modes of grDrawVertexArray()
static FxU32 va_modes[] = { // NOTE: This must be in the same order as the
  GR_POINTS,                // primitives are defined.
  GR_TRIANGLES,
  GR_TRIANGLE_STRIP,
  GR_TRIANGLE_FAN,
  GR_POLYGON,
  GR_LINES,
  GR_LINE_STRIP,
};


// Our local test data
typedef struct {
  int test_pass;
  Vertex verts[MAX_VERTS]; // pointer to all of our primitive verts
  int num_verts;         // number of vertices currently in use
  int cur_rep;           // current pass through do_conform()
  GrCullMode_t cull_mode;// the current cull mode
  float cur_angle;       // the angle (degrees) of this rotation
  float scale_x;         // amount to scale each rotation by in x
  float scale_y;         // amount to scale each rotation by in y
  float offset_x;         // amount to offset each successive rotation in x
  float offset_y;         // amount to offset each successive rotation in y
  int do_aa;             // enable anti-aliasing
  int use_window_coords; // use window coords instead of clip coords
 
} test_data;


/*----------------------------------------------------------------
@func rot_verts_y
@arg Vertex  *out_verts - where to return the xformed verts
@arg Vertex  *in_verts - the source verts
@arg int       num_verts - how many verts to xform
@arg float     y_degrees - the angle (in degrees) to xform to.
@arg float     scale_x - the amount to scale each vertex
@arg float     scale_y - the amount to scale each vertex
@arg float     dx      - the offsets to add to each vertex after
                         rotations and scale.
@arg float     dy      - the offsets to add to each vertex after
                         rotations and scale.
@html
Rotate our vertices around the Y axis, scale, and add
an offset.
@end
-------------------------------------------------------------------*/
void rot_verts_y(Vertex *out_verts, Vertex *in_verts, 
                int num_verts, 
                float y_degrees, 
                float scale_x, 
                float scale_y, 
                float dx, 
                float dy) {

    matrix rotm;       
    mat_make_y_rot(rotm, DEG2RAD(y_degrees));

    for(int i = 0; i < num_verts; i++) {
      mat_vertex_mul(&out_verts[i], &in_verts[i], rotm);
      out_verts[i].x *= scale_x * SCALE_TWEAK;
      out_verts[i].y *= scale_y * SCALE_TWEAK;
      out_verts[i].y += dy;
      out_verts[i].x += dx; 
    }
}


/*---------------------------------------------------------------
@func init_tri_verts
@arg local_ptr -  a pointer to our local_data structure (which holds
                  the vertices we'll be init'ing)
@html
Initialize our triangle vertices.
@end
-----------------------------------------------------------------*/
void init_tri_verts(test_data *local_ptr) {
  Vertex *verts =  local_ptr->verts;
  float rgba = 1.0f; // default color/alpha

  local_ptr->num_verts = 3;

  verts[0].x = -1.0f; verts[0].y = -1.0f; verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].x =  0.0f; verts[1].y =  1.0f; verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].x =  1.0f; verts[2].y = -1.0f; verts[2].w = CLIP_COORDS_W_VALUE;

  if (local_ptr->use_window_coords) {
     rgba = 255.0f;
  }
  for (int i = 0; i < local_ptr->num_verts; i++) {
    verts[i].tmu[0].r = verts[i].tmu[0].g = verts[i].tmu[0].b = 
    verts[i].tmu[0].a = rgba;
  }
}

/*---------------------------------------------------------------
@func init_line_verts
@arg local_ptr -  a pointer to our local_data structure (which holds
                  the vertices we'll be init'ing)
@html
Initialize our triangle vertices.
@end
-----------------------------------------------------------------*/
void init_line_verts(test_data *local_ptr) {
  Vertex *verts =  local_ptr->verts;
  float rgba = 1.0f; // default color/alpha

  local_ptr->num_verts = 8;

  verts[0].x =  0.0f; verts[0].y = -1.0f; verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].x =  0.0f; verts[1].y =  1.0f; verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].x = -1.0f; verts[2].y =  0.0f; verts[2].w = CLIP_COORDS_W_VALUE;
  verts[3].x =  1.0f; verts[3].y =  0.0f; verts[3].w = CLIP_COORDS_W_VALUE;
  verts[4].x = -0.8f; verts[4].y = -0.9f; verts[4].w = CLIP_COORDS_W_VALUE;
  verts[5].x =  0.9f; verts[5].y =  0.9f; verts[5].w = CLIP_COORDS_W_VALUE;
  verts[6].x =  0.9f; verts[6].y = -0.7f; verts[6].w = CLIP_COORDS_W_VALUE;
  verts[7].x = -0.9f; verts[7].y =  0.2f; verts[7].w = CLIP_COORDS_W_VALUE;

  if (local_ptr->use_window_coords) {
     rgba = 255.0f;
  }
  for (int i = 0; i < local_ptr->num_verts; i++) {
    verts[i].tmu[0].r = verts[i].tmu[0].g = verts[i].tmu[0].b = 
    verts[i].tmu[0].a = rgba;
  }
}

/*---------------------------------------------------------------
@func init_point_verts
@arg local_ptr -  a pointer to our local_data structure (which holds
                  the vertices we'll be init'ing)
@html
Initialize our triangle vertices.
@end
-----------------------------------------------------------------*/
void init_point_verts(test_data *local_ptr) {
  Vertex *verts =  local_ptr->verts;
  float rgba = 1.0f; // default color/alpha

  local_ptr->num_verts = 10;

  verts[0].x = -0.9f; verts[0].y = -0.9f; verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].x =  0.0f; verts[1].y =  0.5f; verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].x =  0.4f; verts[2].y = -0.4f; verts[2].w = CLIP_COORDS_W_VALUE;
  verts[3].x =  0.3f; verts[3].y =  0.3f; verts[3].w = CLIP_COORDS_W_VALUE;
  verts[4].x = -0.3f; verts[4].y = -0.1f; verts[4].w = CLIP_COORDS_W_VALUE;
  verts[5].x = -0.6f; verts[5].y = -0.5f; verts[5].w = CLIP_COORDS_W_VALUE;
  verts[6].x = -0.4f; verts[6].y =  0.4f; verts[6].w = CLIP_COORDS_W_VALUE;
  verts[7].x =  0.3f; verts[7].y = -0.3f; verts[7].w = CLIP_COORDS_W_VALUE;
  verts[8].x =  0.0f; verts[8].y = -0.8f; verts[8].w = CLIP_COORDS_W_VALUE;
  verts[9].x = -0.9f; verts[9].y =  0.8f; verts[9].w = CLIP_COORDS_W_VALUE;

  if (local_ptr->use_window_coords) {
     rgba = 255.0f;
  }
  for (int i = 0; i < local_ptr->num_verts; i++) {
    verts[i].tmu[0].r = verts[i].tmu[0].g = verts[i].tmu[0].b = 
    verts[i].tmu[0].a = rgba;
  }
}

/*---------------------------------------------------------------
@func init_va_verts
@arg local_ptr -  a pointer to our local_data structure (which holds
                  the vertices we'll be init'ing)
@html
Initialize our triangle vertices.
@end
-----------------------------------------------------------------*/
void init_va_verts(test_data *local_ptr) {
  Vertex *verts =  local_ptr->verts;
  float rgba = 1.0f; // default color/alpha

  local_ptr->num_verts = 10;

  verts[0].x = -1.0f; verts[0].y = -0.6f; verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].x = -1.0f; verts[1].y =  1.0f; verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].x = -0.7f; verts[2].y = -1.0f; verts[2].w = CLIP_COORDS_W_VALUE;
  verts[3].x = -0.3f; verts[3].y =  0.5f; verts[3].w = CLIP_COORDS_W_VALUE;
  verts[4].x = -0.2f; verts[4].y = -0.8f; verts[4].w = CLIP_COORDS_W_VALUE;
  verts[5].x =  0.0f; verts[5].y =  1.0f; verts[5].w = CLIP_COORDS_W_VALUE;
  verts[6].x =  0.3f; verts[6].y = -0.9f; verts[6].w = CLIP_COORDS_W_VALUE;
  verts[7].x =  0.5f; verts[7].y =  0.3f; verts[7].w = CLIP_COORDS_W_VALUE;
  verts[8].x =  0.8f; verts[8].y = -0.3f; verts[8].w = CLIP_COORDS_W_VALUE;
  verts[9].x =  1.0f; verts[9].y =  0.8f; verts[9].w = CLIP_COORDS_W_VALUE;

  if (local_ptr->use_window_coords) {
     rgba = 255.0f;
  }
  for (int i = 0; i < local_ptr->num_verts; i++) {
    verts[i].tmu[0].r = verts[i].tmu[0].g = verts[i].tmu[0].b = 
    verts[i].tmu[0].a = rgba;
  }
}

/*---------------------------------------------------------------
@func init_va_poly_verts
@arg local_ptr -  a pointer to our local_data structure (which holds
                  the vertices we'll be init'ing)
@html
Initialize our Vertex Array's polygon vertices. Vertex Array
polygons are supposed to be convex, so we don't use the same
data as for the other vertex array prims.
@end
-----------------------------------------------------------------*/
void init_va_poly_verts(test_data *local_ptr) {
  Vertex *verts =  local_ptr->verts;
  float rgba = 1.0f; // default color/alpha

  local_ptr->num_verts = 5;

  verts[0].x = -1.0f; verts[0].y =  0.0f; verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].x = -0.4f; verts[1].y =  0.8f; verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].x =  0.9f; verts[2].y =  0.3f; verts[2].w = CLIP_COORDS_W_VALUE;
  verts[3].x =  0.6f; verts[3].y = -0.2f; verts[3].w = CLIP_COORDS_W_VALUE;
  verts[4].x = -0.2f; verts[4].y = -0.8f; verts[4].w = CLIP_COORDS_W_VALUE;

  if (local_ptr->use_window_coords) {
     rgba = 255.0f;
  }
  for (int i = 0; i < local_ptr->num_verts; i++) {
    verts[i].tmu[0].r = verts[i].tmu[0].g = verts[i].tmu[0].b = 
    verts[i].tmu[0].a = rgba;
  }
}


int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;
  int num_arg;

  vmgrInit(state); // Initialize self checking code
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
  local_ptr->cur_angle = rotations[0];
  local_ptr->cur_rep = 0;
  local_ptr->num_verts = 0;
  local_ptr->scale_x = 1.0f; 
  local_ptr->scale_y = 1.0f;
  local_ptr->offset_x = 0.25f;
  local_ptr->offset_y = 0.25f;
  local_ptr->do_aa = 0; // aa off
  local_ptr->use_window_coords = 0; // use clip coords by default
  local_ptr->cull_mode = cull_modes[0];

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
  
  // default state is cycle through all of our prims
  if (state->num_reps == 1) {
      state->num_reps = NUM_PRIMS * NUM_ROTATIONS * NUM_CULL_MODES;
  }

  // set a small conform window, if none was requested
  if ((state->conform_width == -1) &&
      (state->conform_height == -1)) {
    state->conform_width = 50;
    state->conform_height = 50;
  }
  // setup image file names
  // this is manditory

  char tmp_str[128];
  sprintf(tmp_str,"%s%s%s%s", 
          test_name,
          (state->origin == GR_ORIGIN_UPPER_LEFT? "-ul" : "-ll"),
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
  FxFloat    vnear = 0.f, vfar = 1.f;
  int v = 0;

  local_ptr = (test_data *)state->local_data;

  int cull_mode = cull_modes[local_ptr->cur_rep / (NUM_ROTATIONS * NUM_PRIMS)];
  int cur_prim = (local_ptr->cur_rep / NUM_ROTATIONS) % NUM_PRIMS;

  grCullMode(cull_mode);

  // First, clear the whole window
  vmgrClipWindow(state, 0, 0, state->screen_width, state->screen_height);
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grBufferClear(0, 0, 0);

  // Now set the clip window to just the check_region. (which can be bigger
  // than the conform window).
  vmgrClipWindow(state,
                 state->check_region.min_x,
                 state->check_region.min_y,
                 state->check_region.max_x,
                 state->check_region.max_y);
 
  if (!local_ptr->use_window_coords) {
    vmgrCoordinateSpace(state, GR_CLIP_COORDS);
    vmgrDepthRange( state, vnear, vfar );
  }

  vmgrViewport(state, state->conform_x,state->conform_y, 
              state->conform_width, state->conform_height);

  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_CONSTANT,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );


  // Turn on AA, if applicable.
  if (local_ptr->do_aa) {
    grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
                   GR_COMBINE_FACTOR_NONE,
                   GR_COMBINE_LOCAL_ITERATED,
                   GR_COMBINE_OTHER_NONE,
                   FXFALSE );
    grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                         GR_BLEND_ZERO, GR_BLEND_ZERO );
    grEnable(GR_AA_ORDERED);
  } else {
    grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                   GR_COMBINE_FACTOR_ONE,
                   GR_COMBINE_LOCAL_NONE,
                   GR_COMBINE_OTHER_CONSTANT,
                   FXFALSE);
    grAlphaBlendFunction(GR_BLEND_ONE , GR_BLEND_ZERO, 
                         GR_BLEND_ONE, GR_BLEND_ZERO);
    grDisable(GR_AA_ORDERED);
  }

  // pick a constant color
  if(state->colorformat == GR_COLORFORMAT_ABGR ||
     state->colorformat == GR_COLORFORMAT_ARGB) {
    grConstantColorValue(0x0000ff00);  // green, if ARGB or ABGR
  } else {
    grConstantColorValue(0x00ff0000);  // green, if RGBA or BGRA
  }

  // Adjust layout on screen
  if (local_ptr->use_window_coords) {
    // If we're using window coords, scale ourselves up to the conform
    // window dimensions, and offset appropriately.
    local_ptr->scale_x  = (float)state->conform_width;
    local_ptr->scale_y  = (float)state->conform_height;
    local_ptr->offset_x = (float)(state->conform_x + (state->conform_width / 2.0f));
    local_ptr->offset_y = (float)(state->conform_y + (state->conform_height / 2.0f));
  } else {
    // If we're using clip coords, we need to scale ourselves up by
    // our W value.
    local_ptr->scale_x = 2.0f;                 // -1 to +1
    local_ptr->scale_y = 2.0f;                 // -1 to +1
    local_ptr->scale_x *= CLIP_COORDS_W_VALUE; // to compensate for W
    local_ptr->scale_y *= CLIP_COORDS_W_VALUE; // to compensate for W
    local_ptr->offset_x = 0.0f;
    local_ptr->offset_y = 0.0f;
  }

  // Specify a vertex layout
  vmgrVertexLayout(state, GR_PARAM_XY, offsetof(Vertex, x), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_A, offsetof(Vertex, tmu[0].a), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_W, offsetof(Vertex, w), GR_PARAM_ENABLE);

  Vertex *verts = local_ptr->verts;
  Vertex xformed_verts[MAX_VERTS];

  // Init our vertex data for the current primitive type
  switch (cur_prim) {
    case PRIM_TRIANGLES:           init_tri_verts(local_ptr); break;
    case PRIM_LINES:               init_line_verts(local_ptr); break;
    case PRIM_POINTS:              init_point_verts(local_ptr); break;
    case PRIM_VA_POINTS: 
    case PRIM_VA_LINES:
    case PRIM_VA_TRIANGLES:
    case PRIM_VA_LINE_STRIP:
    case PRIM_VA_TRIANGLE_STRIP:
    case PRIM_VA_TRIANGLE_FAN: init_va_verts(local_ptr); break;
    case PRIM_VA_POLYGON:      init_va_poly_verts(local_ptr); break; 
    default: break;
  }

  // get the next angle
  local_ptr->cur_angle = rotations[local_ptr->cur_rep % NUM_ROTATIONS];

  // transform our verts
  rot_verts_y(xformed_verts, 
              verts, 
              local_ptr->num_verts,       // num verts to rotate
              local_ptr->cur_angle,
              local_ptr->scale_x,
              local_ptr->scale_y,
              local_ptr->offset_x,
              local_ptr->offset_y);

  // figure out which we're doing, and draw them...
  switch (cur_prim) {
    case PRIM_TRIANGLES:
      if (local_ptr->do_aa) {
        add_command_data(state, "grAADrawTriangle()", 6, 
                                NULL, NULL, NULL,
                                FXTRUE, FXTRUE, FXFALSE);
        // make it a more interesting test - don't AA one side.
        grAADrawTriangle(&xformed_verts[0], 
                         &xformed_verts[1], 
                         &xformed_verts[2],
                         FXTRUE, FXTRUE, FXFALSE);
      } else {
        add_command_data(state, "grDrawTriangle()", 0);
        grDrawTriangle(&xformed_verts[0], 
                       &xformed_verts[1], 
                       &xformed_verts[2]);
      }
      break;
    case PRIM_LINES:
      add_command_data(state, "grDrawLine()", 0);
      for (v = 0; v < local_ptr->num_verts; v+=2) {
        grDrawLine(&xformed_verts[v], &xformed_verts[v+1]);
      }
      break;
    case PRIM_POINTS:
      add_command_data(state, "grDrawPoint()", 0);
      for (v = 0; v < local_ptr->num_verts; v++) {
        grDrawPoint(&xformed_verts[v]);
      }
      break;
    case PRIM_VA_LINES:
    case PRIM_VA_LINE_STRIP:  
    case PRIM_VA_POINTS: 
    case PRIM_VA_TRIANGLE_STRIP:
    case PRIM_VA_TRIANGLES:
    case PRIM_VA_TRIANGLE_FAN:
    case PRIM_VA_POLYGON: {
      int mode = va_modes[cur_prim - PRIM_VA_POINTS];
      //printf("Contig mode: %d\n", mode);
      //dump_verts(num_verts, xformed_verts);
      add_command_data(state, "grDrawVertexArrayContiguous()", 4, 
                              mode, local_ptr->num_verts, NULL, sizeof(Vertex));
      grDrawVertexArrayContiguous(mode,
                                  local_ptr->num_verts, 
                                  xformed_verts,
                                  sizeof(Vertex));
      break;
    }
    default: 
      printf("Unknown prim: %d\n", cur_prim);
      break;
  }

  // record the vertices into the data file
  add_vertex_data(state, xformed_verts, local_ptr->num_verts);

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

  local_ptr->cur_rep++;

  return TEST_PASS; // return value ignored
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -aa     enable anti-aliasing\n");
  fprintf(stderr,"   -wc     use window coords instead of clip coords\n");
}


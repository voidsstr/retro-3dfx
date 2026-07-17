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

static char *test_name = "flex-coords";
static char *test_description = "choose a random vertex layout per frame";

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
   42.0f,
   93.0f,
//  137.0f,
//  178.0f,
//  229.0f,
//  273.0f,
//  310.0f,
//  352.0f,
};
#define NUM_ROTATIONS  (sizeof(rotations) / sizeof(float))

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

#define NUM_SLOTS 50 // vertex data can hold 50 floats


// An arbitrarily sized vertex data structure. Vertex params will
// be placed at random locations in it.
typedef struct {
  float slot[NUM_SLOTS];
} VertexData;

#define MAX_VERTEX_LAYOUT_PARAMS 10

// Our local test data
typedef struct {
  int test_pass;
  int num_verts;         // number of vertices currently in use
  float cur_angle;       // the angle (degrees) of this rotation
  float scale_x;         // amount to scale each rotation by in x
  float scale_y;         // amount to scale each rotation by in y
  float offset_x;        // amount to offset each successive rotation in x
  float offset_y;        // amount to offset each successive rotation in y
  int cur_rep;           // which pass we're on
  int do_aa;             // enable anti-aliasing
  int use_window_coords; // use window coords instead of clip coords
  int debug;             // enable debugging output
  int x_offset;          // the offset of x in the VertexData array of floats
  int y_offset;          // the offset of y in the VertexData array of floats
  int w_offset;          // the offset of w in the VertexData array of floats
  int a_offset;          // the offset of a in the VertexData array of floats
  VertexData vert_data[MAX_VERTS]; // our vertex data
  void *vertex_layout;
} test_data;


/*----------------------------------------------------------------
@func find_slot
@arg char slots_taken[]  - an array of flags that indicate whether
                           a slot is taken or not
@arg int  slots_required - the number of contiguous slots required
@html
Find "slots_required" contiguous empty slots at a random location.
@end
-------------------------------------------------------------------*/
int 
find_slot(char *slots_taken, int slots_required)
{ 
  int j, num_tries, slot;
  FxBool got_one;

  got_one = FXFALSE;
  num_tries = 0;

  while(!got_one && (num_tries < NUM_SLOTS)) {
    slot = nrand(NUM_SLOTS);
    got_one = FXTRUE;
    for (j = 0; j < slots_required; j++) {
      if (slots_taken[slot + j] ||
          ((slot + slots_required) >= NUM_SLOTS)) {
        got_one = FXFALSE;
        slot = -1;
        break;
      }
    }
    ++num_tries;
  }

  if (got_one) {
    // mark the slots as filled
    for (j = 0; j < slots_required; j++) {
      slots_taken[slot + j] = 1;
      //printf("slot taken: %d\n", slot + j);
    }
  }

  return slot; // could be -1, if no slots found
}

/*----------------------------------------------------------------
@func gen_layout
@arg conform_state *state - the framework state
@arg test_data    *local_ptr - local data
@html
Generate a random vertex layout for our vertices, and update
the test_data so it knows where to find them.
@end
-------------------------------------------------------------------*/
FxBool
gen_layout(conform_state *state, test_data *local_ptr)
{
  int i, slot;
  char slots_taken[NUM_SLOTS];
 
  // Init all the data slots
  for (i = 0; i < MAX_VERTS; i++) {
    for (slot = 0; slot < NUM_SLOTS; slot++) {
      local_ptr->vert_data[i].slot[slot] = 0.0f;
    }
  }

  for (slot = 0; slot < NUM_SLOTS; slot++) {
    slots_taken[slot] = 0x0;
  }

  // PARAM_XY are always in slots 0 and 1, and are always set

  slots_taken[0] = slots_taken[1] = 1;
  local_ptr->x_offset = 0;
  local_ptr->y_offset = 1;
  local_ptr->a_offset = find_slot(slots_taken, 1);
  local_ptr->w_offset = find_slot(slots_taken, 1);


  if (local_ptr->y_offset == -1 ||
      local_ptr->a_offset == -1 ||
      local_ptr->w_offset == -1) {
      log_perror(state, "Couldn't generate a vertex layout!\n");
      return FXFALSE;
  }

  vmgrVertexLayout(state, GR_PARAM_XY, local_ptr->x_offset << 2, GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_W, local_ptr->w_offset << 2, GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_A, local_ptr->a_offset << 2, GR_PARAM_ENABLE);

  if (local_ptr->debug) {
    printf("GR_PARAM_XY: %d\n", local_ptr->x_offset); 
    printf("GR_PARAM_A: %d\n", local_ptr->a_offset); 
    printf("GR_PARAM_W: %d\n", local_ptr->w_offset); 
  }

  return FXTRUE;
}

/*----------------------------------------------------------------
@func rot_verts_z
@arg VertexData  *out_verts - where to return the xformed verts
@arg VertexData  *in_verts - the source verts
@arg int       num_verts - how many verts to xform
@arg float     y_degrees - the angle (in degrees) to xform to.
@arg float     scale_x - the amount to scale each vertex
@arg float     scale_y - the amount to scale each vertex
@arg float     dx      - the offsets to add to each vertex after
                         rotations and scale.
@arg float     dy      - the offsets to add to each vertex after
                         rotations and scale.
@html
Rotate our vertices around the Z axis, scale, and add
an offset.
@end
-------------------------------------------------------------------*/
void rot_verts_z(test_data *local_ptr,
                 VertexData *out_verts, VertexData *in_verts, 
                 int num_verts, 
                 float z_degrees, 
                 float scale_x, 
                 float scale_y, 
                 float dx, 
                 float dy) {

  int i;
  GrVertex out[MAX_VERTS], in[MAX_VERTS];

  for (i = 0; i < num_verts; i++) {
    in[i].x = in_verts[i].slot[local_ptr->x_offset];
    in[i].y = in_verts[i].slot[local_ptr->y_offset];
    in[i].a = in_verts[i].slot[local_ptr->a_offset];
    in[i].w = in_verts[i].slot[local_ptr->w_offset];
    in[i].z = 0.0f;
  }

  matrix rotm;       
  mat_make_z_rot(rotm, DEG2RAD(z_degrees));

  for (i = 0; i < num_verts; i++) {
    mat_point_mul(&out[i], &in[i], rotm);
    out[i].x *= scale_x * SCALE_TWEAK;
    out[i].y *= scale_y * SCALE_TWEAK;
    out[i].y += dy;
    out[i].x += dx; 
  }

  for (i = 0; i < num_verts; i++) {
    out_verts[i].slot[local_ptr->x_offset] = out[i].x;
    out_verts[i].slot[local_ptr->y_offset] = out[i].y;
    out_verts[i].slot[local_ptr->a_offset] = out[i].a;
    out_verts[i].slot[local_ptr->w_offset] = out[i].w;
  }
}

/**
 * Debug routine. Dump our verts to stdout.
 */
void dump_verts(test_data *local_ptr, int num_verts, VertexData *verts) {
  for (int i = 0; i < num_verts; i++) {
    printf("  [%d] (x,y,a,w) = (%6.3f, %6.3f, %6.3f, %6.3f)\n", i,
            verts[i].slot[local_ptr->x_offset],
            verts[i].slot[local_ptr->y_offset],
            verts[i].slot[local_ptr->a_offset],
            verts[i].slot[local_ptr->w_offset]);
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
  VertexData *verts =  local_ptr->vert_data; 

  local_ptr->num_verts = 3;

  verts[0].slot[local_ptr->x_offset] = -1.0f;
  verts[0].slot[local_ptr->y_offset] = -1.0f;
  verts[0].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[1].slot[local_ptr->x_offset] =  0.0f;
  verts[1].slot[local_ptr->y_offset] =  1.0f;
  verts[1].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[2].slot[local_ptr->x_offset] =  1.0f;
  verts[2].slot[local_ptr->y_offset] = -1.0f;
  verts[2].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;
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
  VertexData *verts =  local_ptr->vert_data; 

  local_ptr->num_verts = 8;

  verts[0].slot[local_ptr->x_offset] =  0.0f;
  verts[0].slot[local_ptr->y_offset] = -1.0f;
  verts[0].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[1].slot[local_ptr->x_offset] =  0.0f;
  verts[1].slot[local_ptr->y_offset] =  1.0f;
  verts[1].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[2].slot[local_ptr->x_offset] = -1.0f;
  verts[2].slot[local_ptr->y_offset] =  0.0f;
  verts[2].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[3].slot[local_ptr->x_offset] =  1.0f;
  verts[3].slot[local_ptr->y_offset] =  0.0f;
  verts[3].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[4].slot[local_ptr->x_offset] = -0.8f;
  verts[4].slot[local_ptr->y_offset] = -0.9f;
  verts[4].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[5].slot[local_ptr->x_offset] =  0.9f;
  verts[5].slot[local_ptr->y_offset] =  0.9f;
  verts[5].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[6].slot[local_ptr->x_offset] =  0.9f;
  verts[6].slot[local_ptr->y_offset] = -0.7f;
  verts[6].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[7].slot[local_ptr->x_offset] = -0.9f;
  verts[7].slot[local_ptr->y_offset] =  0.2f;
  verts[7].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;
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
  VertexData *verts =  local_ptr->vert_data; 

  local_ptr->num_verts = 10;

  verts[0].slot[local_ptr->x_offset] = -0.9f;
  verts[0].slot[local_ptr->y_offset] = -0.9f;
  verts[0].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[1].slot[local_ptr->x_offset] =  0.0f;
  verts[1].slot[local_ptr->y_offset] =  0.5f;
  verts[1].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[2].slot[local_ptr->x_offset] =  0.4f;
  verts[2].slot[local_ptr->y_offset] = -0.4f;
  verts[2].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[3].slot[local_ptr->x_offset] =  0.3f;
  verts[3].slot[local_ptr->y_offset] =  0.3f;
  verts[3].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[4].slot[local_ptr->x_offset] = -0.3f;
  verts[4].slot[local_ptr->y_offset] = -0.1f;
  verts[4].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[5].slot[local_ptr->x_offset] = -0.6f;
  verts[5].slot[local_ptr->y_offset] = -0.5f;
  verts[5].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[6].slot[local_ptr->x_offset] = -0.4f;
  verts[6].slot[local_ptr->y_offset] =  0.4f;
  verts[6].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[7].slot[local_ptr->x_offset] =  0.3f;
  verts[7].slot[local_ptr->y_offset] = -0.3f;
  verts[7].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[8].slot[local_ptr->x_offset] =  0.0f;
  verts[8].slot[local_ptr->y_offset] = -0.8f;
  verts[8].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[9].slot[local_ptr->x_offset] = -0.9f;
  verts[9].slot[local_ptr->y_offset] =  0.8f;
  verts[9].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;
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
  VertexData *verts =  local_ptr->vert_data; 

  local_ptr->num_verts = 10;

  verts[0].slot[local_ptr->x_offset] = -1.0f;
  verts[0].slot[local_ptr->y_offset] = -0.6f;
  verts[0].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[1].slot[local_ptr->x_offset] = -1.0f;
  verts[1].slot[local_ptr->y_offset] =  1.0f;
  verts[1].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[2].slot[local_ptr->x_offset] = -0.7f;
  verts[2].slot[local_ptr->y_offset] = -1.0f;
  verts[2].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[3].slot[local_ptr->x_offset] = -0.3f;
  verts[3].slot[local_ptr->y_offset] =  0.5f;
  verts[3].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[4].slot[local_ptr->x_offset] = -0.2f;
  verts[4].slot[local_ptr->y_offset] = -0.8f;
  verts[4].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[5].slot[local_ptr->x_offset] =  0.0f;
  verts[5].slot[local_ptr->y_offset] =  1.0f;
  verts[5].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[6].slot[local_ptr->x_offset] =  0.3f;
  verts[6].slot[local_ptr->y_offset] = -0.9f;
  verts[6].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[7].slot[local_ptr->x_offset] =  0.5f;
  verts[7].slot[local_ptr->y_offset] =  0.3f;
  verts[7].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[8].slot[local_ptr->x_offset] =  0.8f;
  verts[8].slot[local_ptr->y_offset] = -0.3f;
  verts[8].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[9].slot[local_ptr->x_offset] =  1.0f;
  verts[9].slot[local_ptr->y_offset] =  0.8f;
  verts[9].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;
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
  VertexData *verts =  local_ptr->vert_data; 

  local_ptr->num_verts = 5;

  verts[0].slot[local_ptr->x_offset] = -1.0f;
  verts[0].slot[local_ptr->y_offset] =  0.0f;
  verts[0].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[1].slot[local_ptr->x_offset] = -0.4f;
  verts[1].slot[local_ptr->y_offset] =  0.8f;
  verts[1].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[2].slot[local_ptr->x_offset] =  0.9f;
  verts[2].slot[local_ptr->y_offset] =  0.3f;
  verts[2].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[3].slot[local_ptr->x_offset] =  0.6f;
  verts[3].slot[local_ptr->y_offset] = -0.2f;
  verts[3].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

  verts[4].slot[local_ptr->x_offset] = -0.2f;
  verts[4].slot[local_ptr->y_offset] = -0.8f;
  verts[4].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;

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
  local_ptr->test_pass = -1;
  local_ptr->cur_angle = rotations[0];
  local_ptr->cur_rep = 0;
  local_ptr->num_verts = 0;
  local_ptr->scale_x = 1.0f; 
  local_ptr->scale_y = 1.0f;
  local_ptr->offset_x = 0.25f;
  local_ptr->offset_y = 0.25f;
  local_ptr->do_aa = 0; // aa off
  local_ptr->use_window_coords = 0; // use clip coords by default
  local_ptr->debug = 0;
  local_ptr->x_offset = 0;
  local_ptr->y_offset = 0;
  local_ptr->a_offset = 0;
  local_ptr->w_offset = 0;
  local_ptr->vertex_layout = 0;

  // print out test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    if(!_stricmp(*local_argv,"-sp") || // starting pass
              !_stricmp(*local_argv,"/sp")) {
      ++local_argv;
      ++num_arg; 
      int tmp = 0;
      if(!sscanf(*local_argv,"%d",&tmp)) {
        // bad argument
      } 
      --local_argc;
    } else if(!_stricmp(*local_argv,"-aa") || // anti aliasing 
              !_stricmp(*local_argv,"/aa")) {
      local_ptr->do_aa = 1;
    } else if(!_stricmp(*local_argv,"-wc") || // use window coords
              !_stricmp(*local_argv,"/wc")) {
      local_ptr->use_window_coords = 1;
    } else if(!_stricmp(*local_argv,"-debug") || // enable debug output
              !_stricmp(*local_argv,"/debug")) {
      local_ptr->debug = 1;
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

  // 3 passes through all of combos. The 3rd pass should match the
  // 1st pass.
  if (state->num_reps == 1) {
      state->num_reps = NUM_PRIMS * NUM_ROTATIONS * 3;
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
  if (state->local_data) {
    test_data *td = (test_data*)state->local_data;
    if (td->vertex_layout) {  
      free(td->vertex_layout);
    }
    free(state->local_data);
  }
}

int do_conform(conform_state *state)
{
  test_data *local_ptr;
  FxFloat    vnear = 0.f, vfar = 1.f;
  int i = 0;

  local_ptr = (test_data *)state->local_data;

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

  local_ptr->test_pass = local_ptr->cur_rep / (NUM_ROTATIONS * NUM_PRIMS);

  // On the 1st prim of the 1st pass, generate and save a vertex layout.
  // On the 1st prim of the 3rd pass, restore the 1st pass's layout if
  // it's available.
  // On the middle pass, generate a new layout every time

  if (!(local_ptr->cur_rep % (NUM_ROTATIONS * NUM_PRIMS))) {
    // we're at the first prim of a pass
    switch (local_ptr->test_pass) {
      case 0: {
        gen_layout(state, local_ptr);
        if (!local_ptr->vertex_layout) {
          FxI32 size;
          grGet(GR_GLIDE_VERTEXLAYOUT_SIZE, sizeof(size), &size);
          local_ptr->vertex_layout = calloc(1, size);
        }
        if (local_ptr->vertex_layout) {
          grGlideGetVertexLayout(local_ptr->vertex_layout);
        } else { 
          log_status(state, TEST_FAIL, "save vertex layout");
        }
        break;
      }
      case 1: {
        gen_layout(state, local_ptr);
        break;
      }
      case 2: {
        if (local_ptr->vertex_layout) {
          grGlideSetVertexLayout(local_ptr->vertex_layout);
        } else {
          gen_layout(state, local_ptr);
        }
        break;
      }
    }
  } else if (local_ptr->test_pass == 1) { // if we're the middle pass
     gen_layout(state, local_ptr);        // use a new layout for each prim
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


  VertexData *verts = local_ptr->vert_data;
  VertexData xformed_verts[MAX_VERTS];

  int cur_prim = (local_ptr->cur_rep / NUM_ROTATIONS) % NUM_PRIMS;

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

  int num_verts = local_ptr->num_verts; // filled in in init_X_verts()

  // get the next angle
  local_ptr->cur_angle = rotations[local_ptr->cur_rep % NUM_ROTATIONS];

  // transform our verts
  rot_verts_z(local_ptr,
              xformed_verts, 
              verts, 
              num_verts,       // num verts to rotate
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
      for (i = 0; i < num_verts; i+=2) {
        grDrawLine(&xformed_verts[i], &xformed_verts[i+1]);
      }
      break;
    case PRIM_POINTS:
      add_command_data(state, "grDrawPoint()", 0);
      for (i = 0; i < num_verts; i++) {
        grDrawPoint(&xformed_verts[i]);
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
      add_command_data(state, "grDrawVertexArrayContiguous()", 4, 
                              mode, num_verts, NULL, sizeof(VertexData));
      grDrawVertexArrayContiguous(mode,
                                  num_verts, 
                                  xformed_verts,
                                  sizeof(VertexData));
      break;
    }
    default: 
      printf("Unknown prim: %d\n", cur_prim);
      break;
  }

  // record the vertices into the data file
  GrVertex v;
  for (i = 0; i < num_verts; i++) {
    memset(&v, 0, sizeof(v));
    v.x = xformed_verts[i].slot[local_ptr->x_offset];
    v.y = xformed_verts[i].slot[local_ptr->y_offset];
    v.a = xformed_verts[i].slot[local_ptr->a_offset];
    v.w = xformed_verts[i].slot[local_ptr->w_offset];
    add_vertex_data(state, &v, 1);
  }

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

  return TEST_PASS; // ignored
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -aa     enable anti-aliasing\n");
  fprintf(stderr,"   -wc     use window coords instead of clip coords\n");
  fprintf(stderr,"   -debug  print some diagnostics while running\n");
  
}


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

static char *test_name = "simple-ras";
static char *test_description = "test rasterization by incrementally rotate solid color primitives";


// TODO: Bump up NUM_ROTS_PER_PRIM when self-checking is put in!
#define NUM_ROTS_PER_PRIM  7 // preferred number of rotations of each prim

#define SCALE_TWEAK .35f // scale down rotated prims to keep them in their 1x1 bbox

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

static char *prim_names[] = {
  "triangles",                     // 0
  "lines",                         // 1
  "points",                        // 2
  "vertex array points",           // 3
  "vertex array triangles",        // 4
  "vertex array triangle strip",   // 5
  "vertex array triangle fan",     // 6
  "vertex array polygon",          // 7
  "vertex array lines",            // 8
  "vertex array line strip",       // 9
};

#define  NUM_PRIMS (sizeof(prim_names)/sizeof(char *))

#define MAX_VERTS 10

static FxU32 va_modes[] = { // NOTE: This must be in the same order as the
  GR_POINTS,                // primitives are defined.
  GR_TRIANGLES,
  GR_TRIANGLE_STRIP,
  GR_TRIANGLE_FAN,
  GR_POLYGON,
  GR_LINES,
  GR_LINE_STRIP,
};

typedef struct {
  int test_pass;
  Vertex verts[MAX_VERTS]; // pointer to all of our primitive verts
  int num_rotations;   // number of rotations of each prim per image
  int num_verts;       // number of vertices currently in use
  int cur_prim;        // the current primitive we're drawing
  int cur_rep;         // the index into rot_types that we're applying
  float angle_incr;    // amount to incremement each rotation
  float scale_x;       // amount to scale each rotation by in x
  float scale_y;       // amount to scale each rotation by in y
  float offset_x;       // amount to offset each successive rotation in x
  float offset_y;       // amount to offset each successive rotation in y
  int do_aa;           // enable anti-aliasing
  int use_window_coords; // use window coords instead of clip coords
 
} test_data;


/**
 * Rotate our vertices around the Z-axis, scale, and add
 * an offset.
 * @param out_verts - where to return the xformed verts
 * @param in_verts - the source verts
 * @param num_verts - how many verts to xform
 * @param z_degrees - the angle (in degrees) to xform to.
 * @param scale_x - the amount to scale each vertex
 * @param scale_y - the amount to scale each vertex
 * @param dx      - the offsets to add to each vertex after
 *                  rotations and scale.
 * @param dy      - the offsets to add to each vertex after
 *                  rotations and scale.
 */
void rot_verts_z(Vertex *out_verts, Vertex *in_verts, 
                int num_verts, 
                float z_degrees, 
                float scale_x, 
                float scale_y, 
                float dx, 
                float dy) {

    matrix rotm;       
    mat_make_z_rot(rotm, DEG2RAD(z_degrees));

    for(int i = 0; i < num_verts; i++) {
      mat_vertex_mul(&out_verts[i], &in_verts[i], rotm);
      out_verts[i].x *= scale_x * SCALE_TWEAK;
      out_verts[i].y *= scale_y * SCALE_TWEAK;
      out_verts[i].y += dy;
      out_verts[i].x += dx; 
    }
}

/**
 * Debug routine. Dump our verts to stdout.
 */
void dump_verts(int num_verts, Vertex *verts) {
  for (int i = 0; i < num_verts; i++) {
    printf("  [%d](x,y,w) = (%6.3f, %6.3f, %6.3f)\n", i,
            verts[i].x,
            verts[i].y,
            verts[i].w);
  }
}

/**
 * Initialize our triangle vertices.
 * @param td -  a pointer to our local_data structure (which holds
 *              the vertices we'll be init'ing)
 */
void init_tri_verts(test_data *td) {
  Vertex *verts =  td->verts;
  float rgba = 1.0f; // default color/alpha

  td->num_verts = 3;

  verts[0].x = -1.0f; verts[0].y = -1.0f; verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].x =  0.0f; verts[1].y =  1.0f; verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].x =  1.0f; verts[2].y = -1.0f; verts[2].w = CLIP_COORDS_W_VALUE;

  if (td->use_window_coords) {
     rgba = 255.0f;
  }
  for (int i = 0; i < td->num_verts; i++) {
    verts[i].tmu[0].a = verts[i].tmu[0].r = verts[i].tmu[0].g = verts[i].tmu[0].b = rgba;
  }
}

/**
 * Initialize our line vertices.
 * @param td -  a pointer to our local_data structure (which holds
 *              the vertices we'll be init'ing)
 */
void init_line_verts(test_data *td) {
  Vertex *verts =  td->verts;
  float rgba = 1.0f; // default color/alpha

  td->num_verts = 8;

  verts[0].x =  0.0f; verts[0].y = -1.0f; verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].x =  0.0f; verts[1].y =  1.0f; verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].x = -1.0f; verts[2].y =  0.0f; verts[2].w = CLIP_COORDS_W_VALUE;
  verts[3].x =  1.0f; verts[3].y =  0.0f; verts[3].w = CLIP_COORDS_W_VALUE;
  verts[4].x = -0.8f; verts[4].y = -0.9f; verts[4].w = CLIP_COORDS_W_VALUE;
  verts[5].x =  0.9f; verts[5].y =  0.9f; verts[5].w = CLIP_COORDS_W_VALUE;
  verts[6].x =  0.9f; verts[6].y = -0.7f; verts[6].w = CLIP_COORDS_W_VALUE;
  verts[7].x = -0.9f; verts[7].y =  0.2f; verts[7].w = CLIP_COORDS_W_VALUE;

  if (td->use_window_coords) {
     rgba = 255.0f;
  }
  for (int i = 0; i < td->num_verts; i++) {
    verts[i].tmu[0].a = verts[i].tmu[0].r = verts[i].tmu[0].g = verts[i].tmu[0].b = rgba;
  }
}

/**
 * Initialize our point vertices.
 * @param td -  a pointer to our local_data structure (which holds
 *              the vertices we'll be init'ing)
 */
void init_point_verts(test_data *td) {
  Vertex *verts =  td->verts;
  float rgba = 1.0f; // default color/alpha

  td->num_verts = 10;

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

  if (td->use_window_coords) {
     rgba = 255.0f;
  }
  for (int i = 0; i < td->num_verts; i++) {
    verts[i].tmu[0].a = verts[i].tmu[0].r = verts[i].tmu[0].g = verts[i].tmu[0].b = rgba;
  }
}

/**
 * Initialize our Vertex Array vertices. These vertices are used
 * for all the VA prims except polygons.
 * @param td -  a pointer to our local_data structure (which holds
 *              the vertices we'll be init'ing)
 */
void init_va_verts(test_data *td) {
  Vertex *verts =  td->verts;
  float rgba = 1.0f; // default color/alpha

  td->num_verts = 10;

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

  if (td->use_window_coords) {
     rgba = 255.0f;
  }
  for (int i = 0; i < td->num_verts; i++) {
    verts[i].tmu[0].a = verts[i].tmu[0].r = verts[i].tmu[0].g = verts[i].tmu[0].b = rgba;
  }
}

/**
 * Initialize our Vertex Array's polygon vertices. Vertex Array
 * polygons are supposed to be convex, so we don't use the same
 * data as for the other vertex array prims.
 * @param td -  a pointer to our local_data structure (which holds
 *              the vertices we'll be init'ing)
 */
void init_va_poly_verts(test_data *td) {
  Vertex *verts =  td->verts;
  float rgba = 1.0f; // default color/alpha

  td->num_verts = 5;

  verts[0].x = -1.0f; verts[0].y =  0.0f; verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].x = -0.4f; verts[1].y =  0.8f; verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].x =  0.9f; verts[2].y =  0.3f; verts[2].w = CLIP_COORDS_W_VALUE;
  verts[3].x =  0.6f; verts[3].y = -0.2f; verts[3].w = CLIP_COORDS_W_VALUE;
  verts[4].x = -0.2f; verts[4].y = -0.8f; verts[4].w = CLIP_COORDS_W_VALUE;

  if (td->use_window_coords) {
     rgba = 255.0f;
  }
  for (int i = 0; i < td->num_verts; i++) {
    verts[i].tmu[0].a = verts[i].tmu[0].r = verts[i].tmu[0].g = verts[i].tmu[0].b = rgba;
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
  local_ptr->angle_incr = 0.0f;
  local_ptr->num_rotations = NUM_ROTS_PER_PRIM;
  local_ptr->cur_prim = 0;
  local_ptr->cur_rep = 0;
  local_ptr->num_verts = 0;
  local_ptr->scale_x = 1.0f; 
  local_ptr->scale_y = 1.0f;
  local_ptr->offset_x = 0.25f;
  local_ptr->offset_y = 0.25f;
  local_ptr->do_aa = 0; // aa off
  local_ptr->use_window_coords = 0; // use clip coords by default

  // print out test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    if(!_stricmp(*local_argv,"-nr") || // number of rotations
       !_stricmp(*local_argv,"/nr")) {
      ++local_argv;
      ++num_arg;
      if(!sscanf(*local_argv,"%d",&(local_ptr->num_rotations))) {
        // bad argument
      } 
      --local_argc;
    } else if(!_stricmp(*local_argv,"-aa") || // anti aliasing 
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
      state->num_reps = NUM_PRIMS * local_ptr->num_rotations;
  }

  // set a small conform window, if none was requested
  if ((state->conform_width == -1) &&
      (state->conform_height == -1)) {
    state->conform_width = 50;
    state->conform_height = 50;
  }

  // compute the amount we'll rotate successive frames.  We want
  // to cover a full rotation per rotation type.
  local_ptr->angle_incr = 360.0f/local_ptr->num_rotations;

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
  FxFloat    vnear = 0.f, vfar = 1.f;
  int v = 0;

  vmgrInit(state); // Initialize self checking code

  local_ptr = (test_data *)state->local_data;

  local_ptr->cur_prim = local_ptr->cur_rep / local_ptr->num_rotations;

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

  // pick a constant color
  if(state->colorformat == GR_COLORFORMAT_ABGR ||
     state->colorformat == GR_COLORFORMAT_ARGB) {
    grConstantColorValue(0x0000ff00);  // green, if ARGB or ABGR
  } else {
    grConstantColorValue(0x00ff0000);  // green, if RGBA or BGRA
  }

  // Specify a vertex layout
  vmgrVertexLayout(state, GR_PARAM_XY, offsetof(Vertex, x), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_A, offsetof(Vertex, tmu[0].a), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_W, offsetof(Vertex, w), GR_PARAM_ENABLE);

  Vertex *verts = local_ptr->verts;
  Vertex xformed_verts[MAX_VERTS];

  // Init our vertex data for the current primitive type
  switch (local_ptr->cur_prim) {
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

  rot_verts_z(xformed_verts, 
              verts, 
              num_verts,       // num verts to rotate
              local_ptr->cur_rep * local_ptr->angle_incr,
              local_ptr->scale_x,
              local_ptr->scale_y,
              local_ptr->offset_x,
              local_ptr->offset_y);

  switch (local_ptr->cur_prim) {
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
      for (v = 0; v < num_verts; v+=2) {
        grDrawLine(&xformed_verts[v], &xformed_verts[v+1]);
      }
      break;
    case PRIM_POINTS:
      add_command_data(state, "grDrawPoint()", 0);
      for (v = 0; v < num_verts; v++) {
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
      int mode = va_modes[local_ptr->cur_prim - PRIM_VA_POINTS];
      //printf("Contig mode: %d\n", mode);
      //dump_verts(num_verts, xformed_verts);
      add_command_data(state, "grDrawVertexArrayContiguous()", 4, 
                              mode, num_verts, NULL, sizeof(Vertex));
      grDrawVertexArrayContiguous(mode,
                                  num_verts, 
                                  xformed_verts,
                                  sizeof(Vertex));
      break;
    }
    default: 
      printf("Unknown prim: %d\n", local_ptr->cur_prim);
      break;
  }

  // record the vertices into the data file
  add_vertex_data(state, xformed_verts, num_verts);

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
  fprintf(stderr,"   -nr <n> the number of rotations of each prim\n");
}


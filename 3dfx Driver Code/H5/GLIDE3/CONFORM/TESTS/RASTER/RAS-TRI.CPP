/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;

#endif NOT
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

#undef SHOW_ERRORS
#ifdef SHOW_ERRORS
extern "C" {
  FX_ENTRY void FX_CALL gdPause(void);
  FX_ENTRY void FX_CALL gdWritePixel(GrBuffer_t buffer, FxI32 x, FxI32 y, FxU32 pixel);
}
#endif

static char *test_name = "ras-tri";
static char *test_description = 
  "Incrementally and randomly rotate solid color tris";

#define MAX_VERTS 3      // Number of vertices in our prim

#define SCALE_TWEAK 0.35f // Our prims are from -1 to +1, which means that,
                          // when rotated, they can project outside of their
                          // bounding box. (sqrt(1^2 + 1^2) == 1.4)
                          // So, scale everything by SCALE_TWEAK to make sure
                          // they don't leave their bounding boxes. 
                          // 1.4 * .35 == .49, .49 * 2 == .98

#define CLIP_COORDS_W_VALUE 3.0f // arbitrary

#ifndef BIT
#define BIT(n)  (1UL<<(n))
#endif

// Bit flags for specifying which rotations to do for this pass.
#define ROT_X            BIT(0)
#define ROT_Y            BIT(1)
#define ROT_Z            BIT(2)
#define ROT_INVERT_X     BIT(3)
#define ROT_INVERT_Y     BIT(4)
#define ROT_INVERT_Z     BIT(5)
#define ROT_RANDOM_ORDER BIT(6)
#define ROT_RANDOM_RADS  BIT(8)

// The default number of times to rotate and draw the prim per
// rotation type. 
#define DEFAULT_NUM_TRIS_PER_ROT_TYPE 25

// The combinations of rotations that we'll do.
static int rot_types[] = {
  ROT_X,
  ROT_Y,
  ROT_Z,
  ROT_X | ROT_Y,
  ROT_X | ROT_Z,
  ROT_Y | ROT_Z,
  ROT_X | ROT_Y | ROT_Z,
  ROT_INVERT_X | ROT_Y,
  ROT_INVERT_X | ROT_Z,
  ROT_INVERT_Y | ROT_Z,
  ROT_INVERT_Y | ROT_X,
  ROT_INVERT_Z | ROT_X,
  ROT_INVERT_Z | ROT_Y,
  ROT_INVERT_X | ROT_Y | ROT_Z,
  ROT_INVERT_X | ROT_Y | ROT_INVERT_Z,

  // Now do a bunch of purely random tris.
  ROT_RANDOM_RADS | ROT_RANDOM_ORDER | ROT_X | ROT_Y | ROT_Z,
  ROT_RANDOM_RADS | ROT_RANDOM_ORDER | ROT_X | ROT_Y | ROT_Z,
  ROT_RANDOM_RADS | ROT_RANDOM_ORDER | ROT_X | ROT_Y | ROT_Z,
  ROT_RANDOM_RADS | ROT_RANDOM_ORDER | ROT_X | ROT_Y | ROT_Z,
  ROT_RANDOM_RADS | ROT_RANDOM_ORDER | ROT_X | ROT_Y | ROT_Z,
  ROT_RANDOM_RADS | ROT_RANDOM_ORDER | ROT_X | ROT_Y | ROT_Z,
  ROT_RANDOM_RADS | ROT_RANDOM_ORDER | ROT_X | ROT_Y | ROT_Z,
  ROT_RANDOM_RADS | ROT_RANDOM_ORDER | ROT_X | ROT_Y | ROT_Z,
};
#define NUM_ROT_TYPES (sizeof(rot_types) / sizeof(int)) 

// A list of orders in which to apply our rotations
#define NUM_ROT_ORDERS 6
int rotOrders[NUM_ROT_ORDERS][3] = {
 {0, 1, 2},  // x, y, z
 {0, 2, 1},  // x, z, y
 {1, 2, 0},  // etc...
 {1, 0, 2},
 {2, 1, 0},
 {2, 0, 1},
};

// Data specific to this test.
typedef struct {
  Vertex verts[MAX_VERTS]; // pointer to all of our primitive verts
  Vertex xformed_verts[MAX_VERTS]; // pointer to transformed vers
  int num_verts;       // number of vertices currently in use
  int num_rotations;   // number of rotations of each prim per rot type 
  float angle_incr;    // amount to incremement each rotation
  float scale_x;       // amount to scale each rotation by in x
  float scale_y;       // amount to scale each rotation by in y
  float offset_x;       // amount to offset each successive rotation in x
  float offset_y;       // amount to offset each successive rotation in y
  int do_aa;           // enable anti-aliasing
  int do_clip;         // enable setting of arbitrary clip windows
  int use_window_coords; // use window coords instead of clip coords
  int xform_flags;     // which xforms to apply
  int cur_rep;         // the index into rot_types that we're applying
  int debug;           // enable debugging output
  FxU32 *screen;
  Triangle tri;
  float tolerance;     // pixel tolerance for rasterization check
} test_data;

/*----------------------------------------------------------------
@func save_debug_image
@date 3/12/99
@arg conform_state *state - the framework state
@html
Save a ppm image of local_ptr->screen to disk.
@end
----------------------------------------------------------------*/
void save_debug_image(conform_state *state)
{
  char filename[64];
  test_data *local_ptr = (test_data *)state->local_data;
  int width = state->check_region.max_x - state->check_region.min_x + 1;
  int height = state->check_region.max_y - state->check_region.min_y + 1;
  sprintf(filename, "%s/%s-%02d-dbg", 
          state->output_dir, test_name, state->framecnt-1);
  save_image_to_file(state, 
                     filename,
                     local_ptr->screen, 
                     width, height,
                     FXTRUE);
}

/*----------------------------------------------------------------
@func xform_verts
@date 9/03/98
@arg test_data *local_ptr - local test data
@arg GrVertex  *out_verts - where to return the xformed verts
@arg grVertex  *in_verts - the source verts
@arg int       num_verts - how many verts to xform
@arg float     degrees - the angle (in degrees) to xform to.
@arg float     scale_x - the amount to scale each vertex
@arg float     scale_y - the amount to scale each vertex
@arg float     dx      - the offsets to add to each vertex after
                         rotations and scale.
@arg float     dy      - the offsets to add to each vertex after
                         rotations and scale.
@html
Rotate our vertices around the X, Y, or Z axes, scale, and add
an offset.
@end
-------------------------------------------------------------------*/
void xform_verts(test_data *local_ptr,
                 Vertex *out_verts,   
                 Vertex *in_verts, 
                 int num_verts, 
                 float degrees, 
                 float scale_x, 
                 float scale_y, 
                 float dx, 
                 float dy) 
{

    matrix rotz;       
    matrix rotx;       
    matrix roty;       
    float xRadians = DEG2RAD(degrees);
    float yRadians = DEG2RAD(degrees);
    float zRadians = DEG2RAD(degrees);

    if (local_ptr->xform_flags & ROT_RANDOM_RADS) {
      xRadians = DEG2RAD(nrand(360));
      yRadians = DEG2RAD(nrand(360));
      zRadians = DEG2RAD(nrand(360));
    } 

    // Decide which matrices we'll need.
    if (local_ptr->xform_flags & ROT_X) {
      mat_make_x_rot(rotx, xRadians);
    } else if (local_ptr->xform_flags & ROT_INVERT_X) {
      mat_make_x_rot(rotx, xRadians + M_PI);
    }
    if (local_ptr->xform_flags & ROT_Y) {
      mat_make_y_rot(roty, yRadians);
    } else if (local_ptr->xform_flags & ROT_INVERT_Y) {
      mat_make_y_rot(roty, yRadians + M_PI);
    }
    if (local_ptr->xform_flags & ROT_Z) {
      mat_make_z_rot(rotz, zRadians);
    } else if (local_ptr->xform_flags & ROT_INVERT_Z) {
      mat_make_z_rot(rotz, zRadians + M_PI);
    }

    int whichOrder;
    if (local_ptr->xform_flags & ROT_RANDOM_ORDER) {
      whichOrder = nrand(NUM_ROT_ORDERS); // pick a random order 
    } else {
      whichOrder = 0;                     // apply rots in x,y,z order
    }

    // For each vertex, apply appropriate rotations
    for(int i = 0; i < num_verts; i++) {
      out_verts[i] = in_verts[i];
      for (int rot = 0; rot < 3; rot++) {
        switch(rotOrders[whichOrder][rot]) {
          case 0:
            if (local_ptr->xform_flags & (ROT_X | ROT_INVERT_X)) {
              mat_vertex_mul(&out_verts[i], &out_verts[i], rotx);
            }
            break;
          case 1:
            if (local_ptr->xform_flags & (ROT_Y | ROT_INVERT_Y)) {
              mat_vertex_mul(&out_verts[i], &out_verts[i], roty);
            }
            break;
          case 2:
            if (local_ptr->xform_flags & (ROT_Z | ROT_INVERT_Z)) {
              mat_vertex_mul(&out_verts[i], &out_verts[i], rotz);
            }
            break;
         }
      }

      // Now scale and offset 
      out_verts[i].x *= scale_x * SCALE_TWEAK;
      out_verts[i].y *= scale_y * SCALE_TWEAK;
      out_verts[i].y += dy;
      out_verts[i].x += dx; 
    }
}

/*---------------------------------------------------------------
@func init_tri_verts
@arg test_date *local_ptr -  a pointer to our local_data structure (which holds
                             the vertices we'll be init'ing)
@html
Initialize our triangle vertices.
@end
-----------------------------------------------------------------*/
void init_tri_verts(conform_state *state)
{
  render_state *rstate = &state->rstate;
  test_data *local_ptr = (test_data *)state->local_data;
  Vertex *verts =  local_ptr->verts;
  float rgb = 1.0f;   // default color
  float alpha = 0.5f; // default alpha

  // Specify a vertex layout
  vmgrVertexLayout(state, GR_PARAM_XY, offsetof(Vertex, x), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_A, offsetof(Vertex, tmu[0].a), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_W, offsetof(Vertex, w), GR_PARAM_ENABLE);

  local_ptr->num_verts = 3;

  verts[0].x = -1.0f; verts[0].y = -1.0f; verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].x =  0.4f; verts[1].y =  1.0f; verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].x =  1.0f; verts[2].y = -1.0f; verts[2].w = CLIP_COORDS_W_VALUE;

  if (local_ptr->use_window_coords) {
     rgb = 255.0f;
     alpha = 127.5f;
  }
  for (int i = 0; i < local_ptr->num_verts; i++) {
    verts[i].tmu[0].r = verts[i].tmu[0].g = verts[i].tmu[0].b = rgb;
    verts[i].tmu[0].a = alpha;
  }
}

/*---------------------------------------------------------------
@func set_clip_window
@arg conform_state *state - the framework's state
@html
Choose and apply a random clip window.
@end
-----------------------------------------------------------------*/
void set_clip_window(conform_state *state)
{

  test_data *local_ptr = (test_data *)state->local_data;

  local_ptr = (test_data *)state->local_data;
  // First, compute the max size of the box that this prim
  // is to draw in. We want this in window coords.
  float bbox_x, bbox_y, bbox_w, bbox_h;
  int clip_x, clip_y, clip_width, clip_height; 
  
  bbox_x = (float)state->conform_x;
  bbox_y = (float)state->conform_y;
  bbox_w = (float)state->conform_width;
  bbox_h = (float)state->conform_height;

  // At this point, bbox is the window coordinate box around the 
  // the primitive. Pick an arbitrary clip window inside of it.
  clip_width  = nrand((int)(bbox_w * 0.5f)) + (int)(bbox_w * 0.25f);
  clip_height = nrand((int)(bbox_h * 0.5f)) + (int)(bbox_h * 0.25f);
  clip_x      = nrand((int)(bbox_w - clip_width)) + (int)bbox_x;
  clip_y      = nrand((int)(bbox_h - clip_height)) + (int)bbox_y;

  //printf("bbox = %f, %f, %f, %f\n", bbox_x, bbox_y, bbox_w, bbox_h);
  //printf("clip = %d, %d, %d, %d\n", clip_x, clip_y, clip_width, clip_height);

  vmgrClipWindow(state, clip_x, clip_y, clip_x + clip_width, clip_y + clip_height);

  add_command_data(state, "grClipWindow()", 4, 
                   clip_x, clip_y, clip_x + clip_width, clip_y + clip_height);

  if (local_ptr->debug) {
    //Show the clip window
    grBufferClear(abgrcolor(state, (0xfff00000 | nrand(0xff))), 0, 0);
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
  local_ptr->angle_incr = 0.0f;
  local_ptr->num_rotations = DEFAULT_NUM_TRIS_PER_ROT_TYPE;
  local_ptr->num_verts = 0;
  local_ptr->scale_x = 1.0f; 
  local_ptr->scale_y = 1.0f;
  local_ptr->offset_x = 0.25f;
  local_ptr->offset_y = 0.25f;
  local_ptr->do_aa = 0; // aa off
  local_ptr->do_clip = 0; // clip window off
  local_ptr->use_window_coords = 0; // use clip coords by default
  local_ptr->xform_flags = ROT_Z;
  local_ptr->cur_rep = 0;
  local_ptr->debug = 0;
  local_ptr->tolerance = 1.5f; 
  local_ptr->screen = NULL;

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
    } else if(!_stricmp(*local_argv,"-sp") || // starting pass
              !_stricmp(*local_argv,"/sp")) {
      ++local_argv;
      ++num_arg;
      if(!sscanf(*local_argv,"%d",&(local_ptr->cur_rep))) {
        // bad argument
      } 
      --local_argc;
    } else if(!_stricmp(*local_argv,"-aa") || // anti aliasing 
              !_stricmp(*local_argv,"/aa")) {
      local_ptr->do_aa = 1;
      local_ptr->tolerance = 1.5f; // bump the tolerance for AA tests.
    } else if(!_stricmp(*local_argv,"-cw") || // clip window
              !_stricmp(*local_argv,"/cw")) {
      local_ptr->do_clip = 1;
    } else if(!_stricmp(*local_argv,"-wc") || // use window coords
              !_stricmp(*local_argv,"/wc")) {
      local_ptr->use_window_coords = 1;
    } else if(!_stricmp(*local_argv,"-debug") || // enable debugging output
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
 
  // default state is cycle through all of our prims
  if (state->num_reps == 1) {
      state->num_reps = local_ptr->num_rotations * NUM_ROT_TYPES;
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
  // this is mandatory
  char tmp_str[128];
  sprintf(tmp_str,"%s%s%s%s", 
          test_name,
          (local_ptr->use_window_coords? "-wc" : ""),
          (local_ptr->do_clip? "-cw" : ""),
          (local_ptr->do_aa? "-aa" : ""));

  frame_name(tmp_str,state);
  img_file_type(IMG_P6,state);

  return(-1);
}

void close_conform(conform_state *state)
{
  test_data *local_ptr = (test_data *)state->local_data;
  // Free up local data
  if(local_ptr->screen != NULL )
    free(local_ptr->screen);

  free(state->local_data);
}


TestResult check_triangle(conform_state *state, Triangle *t)
{
  test_data *local_ptr = (test_data *)state->local_data;
  FxU32 * screen = local_ptr->screen;
  FxI32 x, y;
  char outstr[128];
  TestResult result = TEST_PASS;
  BBox *pBox = &state->check_region;

  //printf("clipped bbox: min min, max max = %d, %d, %d, %d\n",
  //        state->check_region.min_x,
  //        state->check_region.min_y,
  //        state->check_region.max_x,
  //        state->check_region.max_y);

  int width  = pBox->max_x - pBox->min_x + 1;
  int height = pBox->max_y - pBox->min_y + 1;

  int xOffset = pBox->min_x;
  int yOffset = pBox->min_y;

  for ( y = 0; y < height; y++ ) {
    for ( x = 0; x < width; x++ ) {
      FxU32 pix = screen[y*width+x];
      float xo = x + xOffset + 0.5f;
      float yo = y + yOffset + 0.5f;

      if ( pix ) {
        if (!vmgrInsideClipWindow(state, xo, yo)) {
           result = TEST_FAIL;
           sprintf(outstr, "lit pixel @(%d, %d) outside clip window", x, y);
           log_message(state,outstr);
        } else if (!vmgrInsideTriangle(xo, yo, t, local_ptr->tolerance)) {
           result = TEST_FAIL;
           sprintf(outstr, "(%d, %d) outside triangle (dAB %f, dAC %f, dBC %f)", 
                   x, y, t->dAB, t->dAC, t->dBC);
           log_message(state,outstr);
        }
      } else { 
        if (vmgrInsideClipWindow(state, xo, yo) && 
            !vmgrOutsideTriangle(xo, yo, t, local_ptr->tolerance)) {
           result = TEST_FAIL;
           sprintf(outstr, "(%d, %d) inside triangle (dAB %f, dAC %f, dBC %f)", 
                   x, y, t->dAB, t->dAC, t->dBC);
           log_message(state,outstr);
        }
      }
    }
  }

  if (local_ptr->debug) { // show pixels that should have been drawn
    for ( y = 0; y < height; y++ ) {
      for ( x = 0; x < width; x++ ) {
        FxU32 * pix = &screen[y * width + x];
        int xo = x + xOffset;
        int yo = y + yOffset;
        if (vmgrInsideTriangle(xo + 0.5f, 
                               yo + y + 0.5f, 
                               t, local_ptr->tolerance)) {
           *pix |= 0x1f;;
        } 
      }
    }
  }
  
#ifdef SHOW_ERRORS // THIS MIGHT BE BROKEN, CHECK THE PIXEL LOCATIONS
  if ( result == 0 ) {
    for ( y = 0; y < height; y++ ) {
      for ( x = 0; x < width; x++ ) {
        FxU32 pix = screen[(height-y-1)*width+x]>>8;
        gdWritePixel(GR_BUFFER_FRONTBUFFER, x, y, pix ? 0x1f : 0);
      }
    }
  
    // gdPause();
  
    for ( y = t->bbox.min_y; y <= t->bbox.max_y; y++ ) {
      for ( x = t->bbox.min_x; x <= t->bbox.max_x; x++ ) {
        // for debugging purposes display pixel if its within tolerance
        if (vmgrInsideTriangle(x+0.5f, y+0.5f, t, local_ptr->tolerance)) {
           gdWritePixel(GR_BUFFER_FRONTBUFFER, x, y, 0x1f<<11);
        }
      }
    }
  
    gdPause();
  }
#endif

  return result;
}

TestResult check_frame(conform_state *state)
{
  test_data *local_ptr = (test_data *)state->local_data;
  Triangle *t = &local_ptr->tri;
  Vertex *tri_verts = local_ptr->xformed_verts;
  TestResult status = TEST_PASS;

  end_frame(END_OP_NOOP, state, (void **)&local_ptr->screen, 
            END_BUFFER_FRONTBUFFER);

  // Get the pixels in the check_region. Since we're passing in a NULL
  // BBox pointer to vmgrGetPixels(), it'll use the one on the conform_state
  // (and adjust it, if necessary).

  if (!vmgrGetPixels(state, local_ptr->screen, GR_BUFFER_FRONTBUFFER, NULL)) {
    log_message(state, "Couldn't read color buffer");
    return TEST_FAIL;
  }

  vmgrMakeTriangle(state, t, &tri_verts[0], &tri_verts[1], &tri_verts[2]);

  status = check_triangle(state, t);

  if (local_ptr->debug) {
    save_debug_image(state);
  }

  return status;
}

int do_conform(conform_state *state)
{
  test_data *local_ptr;
  char tmp_str[128];
  FxFloat    vnear = 0.f, vfar = 1.f;
  int v = 0;
  TestResult status = TEST_PASS;

  local_ptr = (test_data *)state->local_data;

  local_ptr->xform_flags = 
     rot_types[(local_ptr->cur_rep / local_ptr->num_rotations) % NUM_ROT_TYPES];

  // allocate screen for automated compare
  if(local_ptr->screen == NULL ) {
      if((local_ptr->screen = 
          (FxU32 *)malloc(state->screen_width*state->screen_height*sizeof(FxU32))) == NULL) {
        // fail
        log_message(state,"Could not allocate memory buffer for screen image");
      }
  }

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

  // Always draw with alpha enabled, so we can look for double pixel
  // writing. (we send a 0.5 alpha value at the verts)
  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_ITERATED,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );

  grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                       GR_BLEND_ZERO, GR_BLEND_ZERO );

  // Pick a constant color for the tris
  grConstantColorValue(rgbacolor(state, 0x00ff00FF)); // green

  Vertex *verts = local_ptr->verts;
  Vertex *xformed_verts = local_ptr->xformed_verts;

  init_tri_verts(state); 

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

  // Rotate, scale, and offset our verts.
  xform_verts(local_ptr,
              xformed_verts, 
              verts, 
              local_ptr->num_verts,       // num verts to rotate
              local_ptr->cur_rep * local_ptr->angle_incr,
              local_ptr->scale_x, 
              local_ptr->scale_y, 
              local_ptr->offset_x, 
              local_ptr->offset_y);

  // If we're doing a clip test, pick an arbitrary clip window
  if (local_ptr->do_clip) {
    set_clip_window(state);
  }

  // Draw the triangle
  if (local_ptr->do_aa) {
    // make it a more interesting test - don't AA one side.
    grAADrawTriangle(&xformed_verts[0], 
                     &xformed_verts[1], 
                     &xformed_verts[2],
                     FXTRUE, FXTRUE, FXFALSE);
    add_command_data(state, "grAADrawTriangle()", 6, 
                     NULL, NULL, NULL,
                     FXTRUE, FXTRUE, FXFALSE);
  } else {
    grDrawTriangle(&xformed_verts[0], 
                   &xformed_verts[1], 
                   &xformed_verts[2]);
    add_command_data(state, "grDrawTriangle()", 0);

  }
  // record the vertices into the data file
  add_vertex_data(state, xformed_verts, local_ptr->num_verts);

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
    if (!log_status(state, check_frame(state), "triangle check")) {
      return(TEST_FAIL);
    }
  }
 
  local_ptr->cur_rep++; // can be different than test_pass!

  return(status);
}

void help_conform(conform_state *state)
{
  fprintf(stderr, "   -aa     enable anti-aliasing\n");
  fprintf(stderr, "   -wc     use window coords instead of clip coords\n");
  fprintf(stderr, "   -cw     pick a random clip window for each rotation\n");
  fprintf(stderr, "   -nr <n> the number of rotations of each prim\n");
  fprintf(stderr, "   -sp <n> the rep to start with\n");
  fprintf(stderr, "   -debug   enable debugging mode\n");
}


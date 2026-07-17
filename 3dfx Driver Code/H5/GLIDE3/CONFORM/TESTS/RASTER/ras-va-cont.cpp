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

static char *test_name = "ras-va-cont";
static char *test_description = 
 "Incrementally rotate a solid color vertex array, (grDrawVertexArray() or grDrawVertexArrayContiguous()) using the 'continuation' modes: triangle_strip_continue and triangle_fan_continue, optionally applying a clip window and anti-aliasing";

// TODO: Bump this up when self-checking is in!
#define NUM_ROTS  80    // Default number of rotations of each prim

#define MAX_VERTS 10     // Number of vertices in our prim

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

// The combinations of rotations that we'll do.
static int rot_types[] = {
//  ROT_X,
//  ROT_Y,
  ROT_Z,
//  ROT_X | ROT_Y,
//  ROT_X | ROT_Z,
//  ROT_Y | ROT_Z,
//  ROT_X | ROT_Y | ROT_Z,
//  ROT_INVERT_X | ROT_Y,
//  ROT_INVERT_X | ROT_Z,
//  ROT_INVERT_Y | ROT_Z,
//  ROT_INVERT_X | ROT_Y | ROT_Z,
//  ROT_INVERT_X | ROT_Y | ROT_INVERT_Z,
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
  int test_pass;
  Vertex verts[MAX_VERTS]; // pointer to all of our primitive verts
  Vertex xformed_verts[MAX_VERTS]; // pointer to transformed vers
  int num_verts;       // number of vertices currently in use
  int num_verts_0;     // number of vertices in initial prim
  int num_verts_1;     // number of vertices in continued prim
  int num_verts_2;     // number of vertices in next continued prim
  int num_rotations;   // number of rotations of each prim per image
  int rowcol;          // number of rows and columns per image
  float angle_incr;    // amount to incremement each rotation
  float scale_x;       // amount to scale each rotation by in x
  float scale_y;       // amount to scale each rotation by in y
  float offset_x;       // amount to offset each successive rotation in x
  float offset_y;       // amount to offset each successive rotation in y
  int do_aa;           // enable anti-aliasing
  int do_clip;         // enable setting of arbitrary clip windows
  int do_contiguous;   // if set, use grDrawVertexArrayContiguous()
  int use_window_coords; // use window coords instead of clip coords
  int xform_flags;     // which xforms to apply
  int cur_rep;         // the index into rot_types that we're applying
  int debug;           // enable debugging output
  int va_mode;         // the mode with which we'll call grDrawVertexArray()
  char va_mode_name[30];// printable name of the aforementioned va_mode
  float triangle_tolerance;    
  FxU32 *screen;       // where we'll take the snapshot
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
  char filename[128];
  test_data *local_ptr = (test_data *)state->local_data;
  int width = state->check_region.max_x - state->check_region.min_x + 1;
  int height = state->check_region.max_y - state->check_region.min_y + 1;

  sprintf(filename,"%s/%s-%s%s%s%s%s-%04d-dbg", 
          state->output_dir,
          test_name,
          local_ptr->va_mode_name,
          (local_ptr->do_contiguous? "-contig" : ""),
          (local_ptr->use_window_coords? "-wc" : ""),
          (local_ptr->do_clip? "-cw" : ""),
          (local_ptr->do_aa? "-aa" : ""),
          state->framecnt-1);

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
@html
Scale and rotate our vertices around the X, Y, or Z axes, and then
scale, and add an offset to get them back in screen coords, if
necessary.
@end
-------------------------------------------------------------------*/
void xform_verts(test_data *local_ptr, Vertex *out_verts,   Vertex *in_verts)
{

  matrix rotz;       
  matrix rotx;       
  matrix roty;       

  int   num_verts = local_ptr->num_verts;
  float degrees = local_ptr->cur_rep * local_ptr->angle_incr;
  float scale_x = local_ptr->scale_x;
  float scale_y = local_ptr->scale_y;
  float dx = local_ptr->offset_x;
  float dy = local_ptr->offset_y;

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


/*---------------------------------------------------------------
@func init_tri_strip_verts
@arg conform_state *state - the framework's state
@html
Initialize our vertex array's vertices.
@end
-----------------------------------------------------------------*/
void init_tri_strip_verts(conform_state *state) {

  test_data *local_ptr = (test_data *)state->local_data;
  Vertex *verts =  local_ptr->verts;
  float rgb = 1.0f;   // default color
  float alpha = 0.5f; // default alpha
  int i;

  if (local_ptr->use_window_coords) {
     rgb = 255.0f;
     alpha = 127.5f;
  }

  // Specify a vertex layout
  vmgrVertexLayout(state, GR_PARAM_XY, offsetof(Vertex, x), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_A, offsetof(Vertex, tmu[0].a), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_W, offsetof(Vertex, w), GR_PARAM_ENABLE);

  local_ptr->num_verts = MAX_VERTS;

  local_ptr->num_verts_0 = 4; // first prim
  verts[0].x = -1.0f; verts[0].y = -0.6f; verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].x = -1.0f; verts[1].y =  1.0f; verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].x = -0.7f; verts[2].y = -1.0f; verts[2].w = CLIP_COORDS_W_VALUE;
  verts[3].x = -0.3f; verts[3].y =  0.5f; verts[3].w = CLIP_COORDS_W_VALUE;

  local_ptr->num_verts_1 = 3;    // continued prim
  verts[4].x = -0.2f; verts[4].y = -0.8f; verts[4].w = CLIP_COORDS_W_VALUE;
  verts[5].x =  0.0f; verts[5].y =  1.0f; verts[5].w = CLIP_COORDS_W_VALUE;
  verts[6].x =  0.3f; verts[6].y = -0.9f; verts[6].w = CLIP_COORDS_W_VALUE;

  local_ptr->num_verts_2 = 3;    // continued prim
  verts[7].x =  0.5f; verts[7].y =  0.3f; verts[7].w = CLIP_COORDS_W_VALUE;
  verts[8].x =  0.8f; verts[8].y = -0.3f; verts[8].w = CLIP_COORDS_W_VALUE;
  verts[9].x =  1.0f; verts[9].y =  0.8f; verts[9].w = CLIP_COORDS_W_VALUE;

  // Set the color and alpha values
  for (i = 0; i < local_ptr->num_verts; i++) {
    //verts[i].r = verts[i].g = verts[i].b = 0.0f;
    verts[i].tmu[0].a = alpha;
  }

}

/*---------------------------------------------------------------
@func init_tri_fan_verts
@arg conform_state *state - the framework's state
@html
Initialize our vertex array's vertices for a triangle fan
@end
-----------------------------------------------------------------*/
void init_tri_fan_verts(conform_state *state) {
  test_data *local_ptr = (test_data *)state->local_data;
  Vertex *verts =  local_ptr->verts;
  float rgb = 1.0f;   // default color
  float alpha = 0.5f; // default alpha
  int i;

  // Specify a vertex layout
  vmgrVertexLayout(state, GR_PARAM_XY, offsetof(Vertex, x), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_A, offsetof(Vertex, tmu[0].a), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_W, offsetof(Vertex, w), GR_PARAM_ENABLE);

  local_ptr->num_verts = MAX_VERTS;
  local_ptr->num_verts_0 = 5;

  verts[0].x = -0.1f; verts[0].y =  0.1f; verts[0].w = CLIP_COORDS_W_VALUE; // origin

  verts[1].x = -0.9f; verts[1].y = -0.1f; verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].x = -0.5f; verts[2].y =  0.7f; verts[2].w = CLIP_COORDS_W_VALUE;
  verts[3].x =  0.1f; verts[3].y =  0.6f; verts[3].w = CLIP_COORDS_W_VALUE;
  verts[4].x =  0.8f; verts[4].y =  0.1f; verts[4].w = CLIP_COORDS_W_VALUE;

  local_ptr->num_verts_1 = 2;
  verts[5].x =  0.7f; verts[5].y = -0.3f; verts[5].w = CLIP_COORDS_W_VALUE;
  verts[6].x =  0.4f; verts[6].y = -0.6f; verts[6].w = CLIP_COORDS_W_VALUE;

  local_ptr->num_verts_2 = 3;
  verts[7].x =  0.1f; verts[7].y = -0.9f; verts[7].w = CLIP_COORDS_W_VALUE;
  verts[8].x = -0.5f; verts[8].y = -0.5f; verts[8].w = CLIP_COORDS_W_VALUE;
  verts[9].x = -0.8f; verts[9].y = -0.2f; verts[9].w = CLIP_COORDS_W_VALUE;


  if (local_ptr->use_window_coords) {
     rgb = 255.0f;
     alpha = 127.5f;
  }
  for (i = 0; i < local_ptr->num_verts; i++) {
    //verts[i].r = verts[i].g = verts[i].b = rgb;
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

/*----------------------------------------------------------------------------
@func check_triangles
@arg conform_state *state - the framework state
@arg Triangles *tris - pointer to an array of Triangles
@arg int numTris - the number of Triangles in the array
@return TEST_PASS if each lit pixel belongs to a Triangle, TEST_FAIL otherwise
@html
Test that the pixels on screen that have been lit belong to one of the
triangles passed in. Also checks that each unlit pixel is outside of all
the triangles passed in.
@end
-----------------------------------------------------------------------------*/
TestResult check_triangles(conform_state *state, Triangle *tris, int numTris)
{
  test_data *local_ptr = (test_data *)state->local_data;
  FxU32 * screen = local_ptr->screen;
  FxI32 x, y;
  char outstr[128];
  int i;
  TestResult result = TEST_PASS;
  BBox *pBox = &state->check_region;

  int width  = pBox->max_x - pBox->min_x + 1;
  int height = pBox->max_y - pBox->min_y + 1;

  int xOffset = pBox->min_x;
  int yOffset = pBox->min_y;

  // This test checks that lit pixels are on at least one of the 
  // triangles, and that non-lit pixels are outside of all the
  // triangles. It doesn't NOT check that a lit pixel is only in
  // one triangle, which could be added if the triangle fan data
  // was changed so that overlapping triangles weren't produced.

  for ( y = 0; y < height; y++ ) {
    for ( x = 0; x < width; x++ ) {
      FxU32 *pix = &screen[y*width+x];
      float xo = x + xOffset + 0.5f;
      float yo = y + yOffset + 0.5f;

      int pass = 0;
      if ( *pix ) {
        if (!vmgrInsideClipWindow(state, xo, yo)) {
          result = TEST_FAIL;
          sprintf(outstr, "lit pixel @(%d, %d) outside clip window", x, y);
          log_message(state,outstr);
        } else {
          for (i = 0; i < numTris; i++) {
            if (vmgrInsideTriangle(xo, yo,
                                   &tris[i], local_ptr->triangle_tolerance)) {
              pass = 1;
              if (local_ptr->debug) { // show coverage
                *pix |= 0x1f;
              }
              break;
            }
          }
          if (!pass) {
            result = TEST_FAIL;
            sprintf(outstr, "%d %d outside all triangles\n", x, y);
            log_message(state,outstr);
          }
        }
      } else {
        pass = 1;
        for (i = 0; i < numTris; i++) {
          if (vmgrInsideClipWindow(state, xo, yo) && 
              !vmgrOutsideTriangle(xo, yo,
                                   &tris[i], local_ptr->triangle_tolerance)) {
            pass = 0;
            break;
          }
        }
        if (!pass) {
          result = TEST_FAIL;
          sprintf(outstr, "%d %d inside at least one triangle)\n", x, y );
          log_message(state,outstr);
          if (local_ptr->debug) { // show pixels that should have drawn
            *pix |= 0xff00;
          }
        }
      }
    }
  }
  return result;
}


/*----------------------------------------------------------------------------
@func check_frame
@arg conform_state *state - the framework state
@return TEST_PASS if the screen matches what we think it should, TEST_FAIL
        otherwise
@html
Take a snapshot of the screen after rendering and verify that it contains
what we think it should contain. 
@end
-----------------------------------------------------------------------------*/
TestResult check_frame(conform_state *state)
{
  test_data *local_ptr = (test_data *)state->local_data;
  TestResult status = TEST_PASS;

  end_frame(END_OP_NOOP, state, NULL, END_BUFFER_FRONTBUFFER);

  // Get the pixels in the check_region. Since we're passing in a NULL
  // BBox pointer to vmgrGetPixels(), it'll use the one on the conform_state
  // (and adjust it, if necessary).

  if (!vmgrGetPixels(state, local_ptr->screen, GR_BUFFER_FRONTBUFFER, NULL)) {
    log_message(state, "Couldn't read color buffer");
    return TEST_FAIL;
  }

  switch (local_ptr->va_mode) {
    case GR_TRIANGLE_FAN_CONTINUE: {
      Triangle t[8];
      Vertex *tri_verts = local_ptr->xformed_verts;
      vmgrMakeTriangle(state, &t[0], &tri_verts[0], &tri_verts[1], &tri_verts[2]);
      vmgrMakeTriangle(state, &t[1], &tri_verts[0], &tri_verts[2], &tri_verts[3]);
      vmgrMakeTriangle(state, &t[2], &tri_verts[0], &tri_verts[3], &tri_verts[4]);
      vmgrMakeTriangle(state, &t[3], &tri_verts[0], &tri_verts[4], &tri_verts[5]);
      vmgrMakeTriangle(state, &t[4], &tri_verts[0], &tri_verts[5], &tri_verts[6]);
      vmgrMakeTriangle(state, &t[5], &tri_verts[0], &tri_verts[6], &tri_verts[7]);
      vmgrMakeTriangle(state, &t[6], &tri_verts[0], &tri_verts[7], &tri_verts[8]);
      vmgrMakeTriangle(state, &t[7], &tri_verts[0], &tri_verts[8], &tri_verts[9]);
      status = check_triangles(state, t, 8);
      break;
    }
    case GR_TRIANGLE_STRIP_CONTINUE: {
      Triangle t[8];
      Vertex *tri_verts = local_ptr->xformed_verts;
      vmgrMakeTriangle(state, &t[0], &tri_verts[0], &tri_verts[1], &tri_verts[2]);
      vmgrMakeTriangle(state, &t[1], &tri_verts[1], &tri_verts[2], &tri_verts[3]);
      vmgrMakeTriangle(state, &t[2], &tri_verts[2], &tri_verts[3], &tri_verts[4]);
      vmgrMakeTriangle(state, &t[3], &tri_verts[3], &tri_verts[4], &tri_verts[5]);
      vmgrMakeTriangle(state, &t[4], &tri_verts[4], &tri_verts[5], &tri_verts[6]);
      vmgrMakeTriangle(state, &t[5], &tri_verts[5], &tri_verts[6], &tri_verts[7]);
      vmgrMakeTriangle(state, &t[6], &tri_verts[6], &tri_verts[7], &tri_verts[8]);
      vmgrMakeTriangle(state, &t[7], &tri_verts[7], &tri_verts[8], &tri_verts[9]);
      status = check_triangles(state, t, 8);
      break;
    }
    default:
      log_message(state, "Invalid mode passed to check_frame()");
      status = TEST_FAIL;
  }

  if (local_ptr->debug) {
    save_debug_image(state);
  }

  return status;
}


int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;
  int num_arg;
  char tmp_str[128];

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
  local_ptr->num_rotations = NUM_ROTS;
  local_ptr->rowcol = 1;
  local_ptr->num_verts = 0;
  local_ptr->scale_x = 1.0f; 
  local_ptr->scale_y = 1.0f;
  local_ptr->offset_x = 0.25f;
  local_ptr->offset_y = 0.25f;
  local_ptr->do_aa = 0; // aa off
  local_ptr->do_clip = 0; // clip window off
  local_ptr->do_contiguous = 0; // use grDrawVertexArray() by default
  local_ptr->use_window_coords = 0; // use clip coords by default
  local_ptr->xform_flags = ROT_Z;
  local_ptr->cur_rep = 0;
  local_ptr->debug = 0;
  local_ptr->va_mode = -1; // default to no prim specified.
  local_ptr->screen = NULL;
  local_ptr->triangle_tolerance = 0.5f;

  // print out test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    if(!_stricmp(*local_argv, "-nr") || // number of rotations
       !_stricmp(*local_argv, "/nr")) {
      ++local_argv;
      ++num_arg;
      if(!sscanf(*local_argv, "%d", &(local_ptr->num_rotations))) {
        sprintf(tmp_str, "invalid argument after '-nr'");
        log_message(state, tmp_str);
      } 
      --local_argc;
    } else if(!_stricmp(*local_argv, "-mode") || // the VA primitive we'll draw
              !_stricmp(*local_argv, "/mode")) {
      ++local_argv;
      ++num_arg;
      if(!sscanf(*local_argv, "%s", local_ptr->va_mode_name)) {
        sprintf(tmp_str, "invalid argument after '-mode'");
        log_message(state, tmp_str);
      } else {
        if (!_stricmp("strip", local_ptr->va_mode_name)) {
          local_ptr->va_mode = GR_TRIANGLE_STRIP_CONTINUE;
        } else if (!_stricmp("fan", local_ptr->va_mode_name)) {
          local_ptr->va_mode = GR_TRIANGLE_FAN_CONTINUE;
        } else {
          sprintf(tmp_str, "unsupported va mode (%s) after '-mode'", 
                           local_ptr->va_mode_name);
          log_message(state, tmp_str);
        }
      }
      --local_argc;
    } else if(!_stricmp(*local_argv, "-aa") || // anti aliasing 
              !_stricmp(*local_argv, "/aa")) {
      local_ptr->do_aa = 1;
    } else if(!_stricmp(*local_argv, "-cw") || // clip window
              !_stricmp(*local_argv, "/cw")) {
      local_ptr->do_clip = 1;
    } else if(!_stricmp(*local_argv, "-wc") || // use window coords
              !_stricmp(*local_argv, "/wc")) {
      local_ptr->use_window_coords = 1;
    } else if(!_stricmp(*local_argv, "-contig") || // use window coords
              !_stricmp(*local_argv, "/contig") ||
              !_stricmp(*local_argv, "-contiguous") ||
              !_stricmp(*local_argv, "/contiguous")) {
      local_ptr->do_contiguous = 1;
    } else if(!_stricmp(*local_argv, "-debug") || // enable debugging output
              !_stricmp(*local_argv, "/debug")) {
      local_ptr->debug = 1;
    } else {
      fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
      ++local_argv;
      ++num_arg;
    }
    ++local_argv;
    ++num_arg;
  }

  // let the user know that we're defaulting the choice of va mode if
  // they didn't pick any.
  if (local_ptr->va_mode < 0) {
    sprintf(tmp_str, "No mode specified - defaulting to GR_TRIANGLE_STRIP_CONTINUE");
    log_message(state, tmp_str);
    fprintf(stderr, "%s\n", tmp_str);
    local_ptr->va_mode = GR_TRIANGLE_STRIP_CONTINUE;
    strcpy(local_ptr->va_mode_name, "strip");
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
  sprintf(tmp_str,"%s-%s%s%s%s%s", 
          test_name,
          local_ptr->va_mode_name,
          (local_ptr->do_contiguous? "-contig" : ""),
          (local_ptr->use_window_coords? "-wc" : ""),
          (local_ptr->do_clip? "-cw" : ""),
          (local_ptr->do_aa? "-aa" : ""));

  frame_name(tmp_str,state);
  img_file_type(IMG_P6,state);

  return(-1);
}

void close_conform(conform_state *state)
{
  // Free up local data
  test_data *local_ptr = (test_data *)state->local_data;

  if(local_ptr->screen != NULL )
    free(local_ptr->screen);

  free(state->local_data);
}

int do_conform(conform_state *state)
{
  test_data *local_ptr;
  char tmp_str[128];
  FxFloat    vnear = 0.f, vfar = 1.f;
  int i, v = 0;

  vmgrInit(state); // Initialize self checking code

  local_ptr = (test_data *)state->local_data;

  local_ptr->xform_flags = 
     rot_types[(local_ptr->cur_rep / local_ptr->num_rotations) % NUM_ROT_TYPES];

  // allocate screen for automated compare
  if(local_ptr->screen == NULL ) {
      if((local_ptr->screen = 
          (FxU32 *)malloc(state->screen_width*state->screen_height*sizeof(FxU32))) == NULL) {
        // fail
        log_perror(state,"Could not allocate memory buffer for screen image");
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

  // Pick a constant color for the tris
  grConstantColorValue(rgbacolor(state, 0x00ff00FF)); // green

  Vertex *verts = local_ptr->verts;
  Vertex *xformed_verts = local_ptr->xformed_verts;

  // Set up a list of pointers to our xformed_verts. We'll pass these
  // to grDrawVertexArray() later.
  Vertex *vert_ptrs[MAX_VERTS];
  for (i = 0; i < MAX_VERTS; i++) {
    vert_ptrs[i] = &xformed_verts[i];
  }

  if (local_ptr->va_mode == GR_TRIANGLE_STRIP_CONTINUE) {
    init_tri_strip_verts(state); 
  } else {
    init_tri_fan_verts(state); 
  }

  int num_verts = local_ptr->num_verts; // filled in in init_verts()

  // rotate
  xform_verts(local_ptr, xformed_verts, verts);

  // If we're doing a clip test, pick an arbitrary clip window
  if (local_ptr->do_clip) {
    set_clip_window(state);
  }

  FxU32 initial_mode, continue_mode;
      
  if (local_ptr->va_mode == GR_TRIANGLE_STRIP_CONTINUE) {
    initial_mode = GR_TRIANGLE_STRIP;
    continue_mode = GR_TRIANGLE_STRIP_CONTINUE; 
  } else {
    initial_mode = GR_TRIANGLE_FAN;
    continue_mode = GR_TRIANGLE_FAN_CONTINUE;
  }

  if (local_ptr->do_contiguous) { // use grDrawVertexArrayContiguous()

    // Draw the first triangle_strip
    add_command_data(state, "grDrawVertexArrayContiguous()", 4, 
                            initial_mode,
                            local_ptr->num_verts_0,
                            NULL, sizeof(Vertex));
 
    grDrawVertexArrayContiguous(initial_mode,
                                local_ptr->num_verts_0, 
                                xformed_verts,
                                sizeof(Vertex));

    // record the vertices into the data file
    add_vertex_data(state, xformed_verts, local_ptr->num_verts_0);

    // Draw the continued triangle strip
    add_command_data(state, "grDrawVertexArrayContiguous()", 4, 
                            continue_mode,
                            local_ptr->num_verts_1,
                            NULL, sizeof(Vertex));

    grDrawVertexArrayContiguous(continue_mode,
                                local_ptr->num_verts_1, 
                                &xformed_verts[local_ptr->num_verts_0],
                                sizeof(Vertex));

    // record the vertices into the data file
    add_vertex_data(state, &xformed_verts[local_ptr->num_verts_0],
                    local_ptr->num_verts_1);

    // Draw the next continued triangle strip
    add_command_data(state, "grDrawVertexArrayContiguous()", 4, 
                            continue_mode,
                            local_ptr->num_verts_2,
                            NULL, sizeof(Vertex));

    grDrawVertexArrayContiguous(continue_mode,
                                local_ptr->num_verts_2, 
                                &xformed_verts[local_ptr->num_verts_0 + 
                                               local_ptr->num_verts_1],
                                sizeof(Vertex));

    // record the vertices into the data file
    add_vertex_data(state, 
                    &xformed_verts[local_ptr->num_verts_0 +
                                   local_ptr->num_verts_1],
                    local_ptr->num_verts_2);

  } else {  // use grDrawVertexArray()

    // Draw the first triangle_strip
    add_command_data(state, "grDrawVertexArray()", 3, 
                            initial_mode,
                            local_ptr->num_verts_0,
                            NULL);

    grDrawVertexArray(initial_mode,
                      local_ptr->num_verts_0, 
                      vert_ptrs);
  
    // record the vertices into the data file
    add_vertex_data(state, xformed_verts, local_ptr->num_verts_0);

    // Draw the continued triangle strip
    add_command_data(state, "grDrawVertexArray()", 3, 
                            continue_mode,
                            local_ptr->num_verts_1,
                            NULL);

    grDrawVertexArray(continue_mode,
                      local_ptr->num_verts_1, 
                      &vert_ptrs[local_ptr->num_verts_0]);

    // record the vertices into the data file
    add_vertex_data(state, &xformed_verts[local_ptr->num_verts_0],
                    local_ptr->num_verts_1);

    // Draw the next continued triangle strip
    add_command_data(state, "grDrawVertexArray()", 3, 
                            continue_mode,
                            local_ptr->num_verts_2,
                            NULL);
  
    grDrawVertexArray(continue_mode,
                      local_ptr->num_verts_2, 
                      &vert_ptrs[local_ptr->num_verts_0 + 
                                 local_ptr->num_verts_1]);

    // record the vertices into the data file
    add_vertex_data(state, 
                    &xformed_verts[local_ptr->num_verts_0 +
                                   local_ptr->num_verts_1],
                    local_ptr->num_verts_2);
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
    //end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
    if (!log_status(state, check_frame(state), "triangle check")) {
      return TEST_FAIL; // return value ignored
    }
  }

  local_ptr->cur_rep++; // can be different than test_pass!

  return TEST_PASS; // return value ignored
}

void help_conform(conform_state *state)
{
  fprintf(stderr, "   -aa     enable anti-aliasing\n");
  fprintf(stderr, "   -wc     use window coords instead of clip coords\n");
  fprintf(stderr, "   -cw     pick a random clip window for each rotation\n");
  fprintf(stderr, "   -nr <n> the number of rotations of each prim\n");
  fprintf(stderr, "   -mode <s> the vertex array mode to use:\n");
  fprintf(stderr, "          strip - GR_TRIANGLE_STRIP/CONTINUE\n");
  fprintf(stderr, "          fan   - GR_TRIANGLE_FAN/CONTINUE\n");
  fprintf(stderr, "   -contig  use grDrawVertexArrayContiguous()\n");
  fprintf(stderr, "   -debug   enable debugging mode\n");
}


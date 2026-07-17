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

#undef SHOW_ERRORS
#ifdef SHOW_ERRORS
extern "C" {
  FX_ENTRY void FX_CALL gdPause(void);
  FX_ENTRY void FX_CALL gdWritePixel(GrBuffer_t buffer, FxI32 x, FxI32 y, FxU32 pixel);
}
#endif

static char *test_name = "ras-line";
static char *test_description = "Incrementally rotate solid color lines";

#define NUM_ROTS  50    // Default number of rotations of each prim

#define MAX_VERTS 8      // Number of vertices in our prim

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
#define ROT_X         BIT(0)
#define ROT_Y         BIT(1)
#define ROT_Z         BIT(2)
#define ROT_INVERT_X  BIT(3)
#define ROT_INVERT_Y  BIT(4)
#define ROT_INVERT_Z  BIT(5)

// The combinations of rotations that we'll do.
static int rot_types[] = {
  ROT_Z,
  ROT_X | ROT_Y,
  ROT_X | ROT_Z,
  ROT_Y | ROT_Z,
};
#define NUM_ROT_TYPES (sizeof(rot_types) / sizeof(int)) 

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
  Line l;
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
  // printf("\nImage saved. Hit return to continue\n");
  // wait_for_keypress(state);
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
                 float dy) {

    matrix rotz;       
    matrix rotx;       
    matrix roty;       
    float radians = DEG2RAD(degrees);

    // Decide which matrices we'll need.
    if (local_ptr->xform_flags & ROT_X) {
      mat_make_x_rot(rotx, radians);
    } else if (local_ptr->xform_flags & ROT_INVERT_X) {
      mat_make_x_rot(rotx, radians + M_PI);
    }
    if (local_ptr->xform_flags & ROT_Y) {
      mat_make_y_rot(roty, radians);
    } else if (local_ptr->xform_flags & ROT_INVERT_Y) {
      mat_make_y_rot(roty, radians + M_PI);
    }
    if (local_ptr->xform_flags & ROT_Z) {
      mat_make_z_rot(rotz, radians);
    } else if (local_ptr->xform_flags & ROT_INVERT_Z) {
      mat_make_z_rot(rotz, radians + M_PI);
    }

    // For each vertex, apply appropriate rotations
    for(int i = 0; i < num_verts; i++) {
      out_verts[i] = in_verts[i];

      // Apply rotations
      if (local_ptr->xform_flags & (ROT_X | ROT_INVERT_X)) {
        mat_vertex_mul(&out_verts[i], &out_verts[i], rotx);
      }
      if (local_ptr->xform_flags & (ROT_Y | ROT_INVERT_Y)) {
        mat_vertex_mul(&out_verts[i], &out_verts[i], roty);
      }
      if (local_ptr->xform_flags & (ROT_Z | ROT_INVERT_Z)) {
        mat_vertex_mul(&out_verts[i], &out_verts[i], rotz);
      }

      // Now scale and offset 
      out_verts[i].x *= scale_x * SCALE_TWEAK;
      out_verts[i].y *= scale_y * SCALE_TWEAK;
      out_verts[i].y += dy;
      out_verts[i].x += dx; 
    }
}

/*----------------------------------------------------------------
@arg td -  a pointer to our local_data structure (which holds
           the vertices we'll be init'ing)
@html
Initialize our lines vertices.
@end
-----------------------------------------------------------------*/
void init_line_verts(conform_state *state)
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

  local_ptr->num_verts = 8;

  verts[0].x =  0.2f; verts[0].y = -1.0f; verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].x = -0.1f; verts[1].y =  1.0f; verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].x = -1.0f; verts[2].y = -0.3f; verts[2].w = CLIP_COORDS_W_VALUE;
  verts[3].x =  1.0f; verts[3].y =  0.0f; verts[3].w = CLIP_COORDS_W_VALUE;
  verts[4].x = -0.8f; verts[4].y = -0.9f; verts[4].w = CLIP_COORDS_W_VALUE;
  verts[5].x =  0.9f; verts[5].y =  0.9f; verts[5].w = CLIP_COORDS_W_VALUE;
  verts[6].x =  0.9f; verts[6].y = -0.7f; verts[6].w = CLIP_COORDS_W_VALUE;
  verts[7].x = -0.9f; verts[7].y =  0.2f; verts[7].w = CLIP_COORDS_W_VALUE;

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

  vmgrClipWindow(state, clip_x, clip_y,   
                 clip_x + clip_width, clip_y + clip_height);

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
  local_ptr->num_rotations = NUM_ROTS;
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
  local_ptr->tolerance = 1.0f; // end points can be off by upto 1 pixel, 

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

  // compute the amount we'll rotate successive frames.  We want
  // to cover a full rotation per rotation type.
  local_ptr->angle_incr = 360.0f/local_ptr->num_rotations;

  // set a small conform window, if none was requested
  if ((state->conform_width == -1) &&
      (state->conform_height == -1)) {
    state->conform_width = 50;
    state->conform_height = 50;
  }

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

double max_error = 0.f;

int check_line(conform_state *state, Line *l)
{
  test_data *local_ptr = (test_data *)state->local_data;
  FxU32 * screen = local_ptr->screen;
  FxI32 x, y;
  char outstr[128];
  int result = -1;
  BBox *pBox = &state->check_region;

  int width  = pBox->max_x - pBox->min_x + 1;
  int height = pBox->max_y - pBox->min_y + 1;

  int xOffset = pBox->min_x;
  int yOffset = pBox->min_y;

  switch ( l->major ) {
  case  X_MAJOR:
    for ( x = 0; x < width; x++ ) {
      int expected, count = 0;
      int xo = x + xOffset;
      for ( y = 0; y < height; y++ ) {
        FxU32 pix = screen[y*width+x];
        int yo = y + yOffset;
        if ( pix ) {
          if ( l->error > max_error ) max_error = l->error;
          count++;
          if ( !vmgrOnLine(xo+0.5f, yo+0.5f, l, local_ptr->tolerance)) {
            sprintf(outstr, "%d %d pixel not on line (%f)\n", x, y, l->error);
            log_message(state,outstr);
            return 0;
          }
        }
      }
      // if any part of the window is offscreen, this test isn't
      // guaranteed to be correct. 
      if ((width == state->conform_width) && (height == state->conform_height)) {
        expected = (( xo < l->bbox.min_x ) || ( xo > l->bbox.max_x )) ? 0 : 1;
        if ( count > 1 ) {
          sprintf(outstr, "%d %d more than one pixel per minor axis\n", x, y);
          log_message(state,outstr);
          return 0;
        }
        if (( count != expected ) && ( xo > (l->bbox.min_x+1) ) && ( xo < (l->bbox.max_x-1) )) {
          if ( expected == 0 ) {
            sprintf(outstr, "%d %d pixel drawn outside bbox\n", x, y);
            log_message(state,outstr);
            return 0;
           } else {
             sprintf(outstr, "%d %d  has missing pixels per minor axis(x)", x, y);
             log_message(state,outstr);
             return 0;
          }
        }
      }
    }
    break;
  case  Y_MAJOR:
    for ( y = 0; y < height; y++ ) {
      int expected, count = 0;
      int yo = y + yOffset;
      for ( x = 0; x < width; x++ ) {
        int xo = x + xOffset;
        FxU32 pix = screen[y*width+x];
        if ( pix ) {
          if ( l->error > max_error ) max_error = l->error;
          count++;
          if ( !vmgrOnLine(xo+0.5f, yo+0.5f, l, local_ptr->tolerance)) {
           sprintf(outstr, "%d %d pixel not on line (%f)\n", x, y, l->error);
           log_message(state,outstr);
           return 0;
          }
        }
      }
      // if any part of the window is offscreen, this test isn't
      // guaranteed to be correct. 
      if ((width == state->conform_width) && (height == state->conform_height)) {
        expected = (( yo < l->bbox.min_y ) || ( yo > l->bbox.max_y )) ? 0 : 1;
        if ( count > 1 ) {
          sprintf(outstr, "%d %d more than one pixel per minor axis\n", x, y);
          log_message(state,outstr);
          return 0;
        }
        if (( count != expected ) && ( yo > (l->bbox.min_y+1) ) && ( yo < (l->bbox.max_y-1) )) {
          if ( expected == 0 ) {
            sprintf(outstr, "%d %d pixel drawn outside bbox\n", x, y);
            log_message(state,outstr);
            return 0;
           } else {
             sprintf(outstr, "%d %d  has missing pixels per minor axis(y) %d, %d", 
                     x, y, count, expected);
             log_message(state,outstr);
             return 0;
          }
        }
      }
    }
    break;
  }

#ifdef SHOW_ERRORS
  if ( result == 0 ) {
    for ( y = 0; y < state->conform_height; y++ ) {
      for ( x = 0; x < state->conform_width; x++ ) {
        FxU32 pix = screen[(state->conform_height-y-1)*state->conform_width+x]>>8;
  
        gdWritePixel(GR_BUFFER_FRONTBUFFER, x, y, pix ? 0x1f : 0);
      }
    }
  
    // gdPause();
  
    for ( y = t->bbox.min_y; y <= t->bbox.max_y; y++ ) {
      for ( x = t->bbox.min_x; x <= t->bbox.max_x; x++ ) {
        // for debugging purposes display pixel if its within tolerance
        if (vmgrOnLine(x+0.5f, y+0.5f, t, local_ptr->tolerance)) {
           gdWritePixel(GR_BUFFER_FRONTBUFFER, x, y, 0x1f<<11);
        }
      }
    }
  
    gdPause();
  }
#endif

  return result;
}

TestResult draw_check_line(conform_state *state, Vertex *v0, Vertex *v1)
{
  test_data *local_ptr = (test_data *)state->local_data;
  Line *l = &local_ptr->l;
  Vertex *tri_verts = local_ptr->xformed_verts;

  grBufferClear(0, 0, 0);

  grDrawLine(v0, v1);

  end_frame(END_OP_NOOP, state, (void **)&local_ptr->screen, 
            END_BUFFER_FRONTBUFFER);

  // Get the pixels in the check_region. Since we're passing in a NULL
  // BBox pointer to vmgrGetPixels(), it'll use the one on the conform_state
  // (and adjust it, if necessary).

  if (!vmgrGetPixels(state, local_ptr->screen, GR_BUFFER_FRONTBUFFER, NULL)) {
    log_message(state, "Couldn't read color buffer");
    return TEST_FAIL;
  }

  vmgrMakeLine(state, l, v0, v1);
  if ( !check_line(state, l)) {
    vmgrMakeLine(state, l, v0, v1);
    return TEST_FAIL;
  }
  
  return TEST_PASS;
}

TestResult test_diagonals(conform_state *state) 
{
  test_data *local_ptr = (test_data *)state->local_data;
  TestResult status = TEST_PASS;
  Vertex v0, v1;
  int w = state->conform_width/2;
  int h = state->conform_height/2;
  int j;

  for ( j = 0; j < 10; j++ ) {
    v0.x = (float)(state->conform_x+w);
    v0.y = (float)(state->conform_y+h);
    switch ( j % 3 ) {
    case 0:
      v1.x = v0.x;
      break;
    case 1:
      v1.x = v0.x + w;
      break;
    default:
      v1.x = v0.x - w;
      break;
    }
    switch ( j / 3 ) {
    case 0:
      v1.y = v0.y;
      break;
    case 1:
      v1.y = v0.y + h;
      break;
    default:
      v1.y = v0.y - h;
      break;
    }
    // v1.x = v0.x + (float)(nrand(2) ? w : -w);
    // v1.y = v0.y + (float)(nrand(2) ? h : -h);
    v0.w = 1.0f;
    v1.w = 1.0f;
    v0.tmu[0].a = 127.5f;
    v1.tmu[0].a = 127.5f;
    
    if (!local_ptr->use_window_coords) {
      vmgrWindowToClip(state, &v0, &v0);
      vmgrWindowToClip(state, &v1, &v1);
    }
 

    if ( ( status = draw_check_line(state, &v0, &v1)) == TEST_FAIL ) {
      if (local_ptr->debug) {
        save_debug_image(state);
      }
      break;
    }
  }

  if (!log_status(state, status, "diagonal line check")) {
      return(TEST_FAIL);
  }

  // do a bunch of random lines 

  for ( j = 0; j < 5; j++ ) {
    v0.x = (float)(state->conform_x+w);
    v0.y = (float)(state->conform_y+h);
    v1.x = v0.x + (float)(nrand(2) ? nrand(w) : -nrand(w));
    v1.y = v0.y + (float)(nrand(2) ? nrand(h) : -nrand(h));
    v0.w = 1.0f;
    v1.w = 1.0f;
    v0.tmu[0].a = 127.5f;
    v1.tmu[0].a = 127.5f;
    
    if (!local_ptr->use_window_coords) {
      vmgrWindowToClip(state, &v0, &v0);
      vmgrWindowToClip(state, &v1, &v1);
    }
  
    if ( ( status = draw_check_line(state, &v0, &v1)) == TEST_FAIL ) {
      if (local_ptr->debug) {
        save_debug_image(state);
      }
      break;
    }
  }
  
  if (!log_status(state, status, "diagonal line check")) {
    return(TEST_FAIL);
  }

  return TEST_PASS; 
}

int do_conform(conform_state *state)
{
  test_data *local_ptr;
  char tmp_str[128];
  FxFloat    vnear = 0.f, vfar = 1.f;
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

  init_line_verts(state); 

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

  // Draw a line for each pair of vertices
  for (int v = 0; v < local_ptr->num_verts; v+=2) {
    add_command_data(state, "grDrawLine()", 0);
    // record the vertices into the data file
    add_vertex_data(state, &xformed_verts[v], 2);
    if (!log_status(state, draw_check_line(state, &xformed_verts[v], &xformed_verts[v+1]), "3d line check")) {
      return(TEST_FAIL);
    }
  }

  if (!log_status(state, test_diagonals(state), "diagonal line check")) {
    return(TEST_FAIL);
  }

  sprintf(tmp_str,"test data: rep# %d\n", local_ptr->cur_rep);
  add_data(state, tmp_str, strlen(tmp_str));

  local_ptr->cur_rep++; // can be different than test_pass!

  return(status);
}

void help_conform(conform_state *state)
{
  fprintf(stderr, "   -aa     enable anti-aliasing\n");
  fprintf(stderr, "   -wc     use window coords instead of clip coords\n");
  fprintf(stderr, "   -cw     pick a random clip window for each rotation\n");
  fprintf(stderr, "   -nr <n> the number of rotations of each prim\n");
  fprintf(stderr, "   -sp <n> the rep to start with:\n");
  fprintf(stderr, "        0 - Rotation: X\n");
  fprintf(stderr, "        1 - Rotation: Y\n");
  fprintf(stderr, "        2 - Rotation: Z\n");
  fprintf(stderr, "        3 - Rotation: X*Y\n");
  fprintf(stderr, "        4 - Rotation: X*Z\n");
  fprintf(stderr, "        5 - Rotation: Y*Z\n");
  fprintf(stderr, "        6 - Rotation: X*Y*Z\n");
  fprintf(stderr, "        7 - Rotation: Xi*Y\n");
  fprintf(stderr, "        8 - Rotation: Xi*ZZ\n");
  fprintf(stderr, "        9 - Rotation: Yi*Z\n");
  fprintf(stderr, "       10 - Rotation: Xi*Y*Z\n");
  fprintf(stderr, "       11 - Rotation: Xi*Y*Zi\n");
  fprintf(stderr, "   -debug   enable debugging mode\n");
}


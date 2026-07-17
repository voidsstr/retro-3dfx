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

static char *test_name = "simple-interp-depth";
static char *test_description = 
 "Draw varied depth prims and verify their depth is being linearly "
 "interpolated by also drawing constant depth lines on both sides "
 "of them. Compute the depth of the lines ourselves.";

#define PRIM_TRIANGLES         0
#define PRIM_LINES             1
#define PRIM_VA_TRIANGLES      2
#define PRIM_VA_TRIANGLE_STRIP 3
#define PRIM_VA_TRIANGLE_FAN   4
#define PRIM_VA_POLYGON        5
#define PRIM_VA_LINES          6
#define PRIM_VA_LINE_STRIP     7

static char *prim_names[] = {
  "triangles",                     // 0
  "lines",                         // 1
  "vertex array triangles",        // 2
  "vertex array triangle strip",   // 3
  "vertex array triangle fan",     // 4
  "vertex array polygon",          // 5
  "vertex array lines",            // 6
  "vertex array line strip",       // 7
};

#define  NUM_PRIMS (sizeof(prim_names)/sizeof(char *))

#define MAX_VERTS 10

static FxU32 va_modes[] = { // NOTE: This must be in the same order as the
  GR_TRIANGLES,
  GR_TRIANGLE_STRIP,
  GR_TRIANGLE_FAN,
  GR_POLYGON,
  GR_LINES,
  GR_LINE_STRIP,
};

#define NUM_TRI_VERTS 3
#define NUM_LINE_VERTS 4
#define NUM_VA_VERTS 6
#define NUM_VA_POLY_VERTS 4

typedef struct {
  GrVertex line_verts[NUM_LINE_VERTS];
  GrVertex tri_verts[NUM_TRI_VERTS];
  GrVertex va_verts[NUM_VA_VERTS];
  GrVertex va_poly_verts[NUM_VA_POLY_VERTS];
  int rows;
  int num_lines; // number of constant depth lines
  float scale_y;
  float scale_x;
  float delta_y;
  float delta_x;
  int origin_x;
  int origin_y;
  int use_z_depth; 
} test_data;

#define NEAREST   10.0f
#define FARTHEST  400.0f
#define MID ((FARTHEST - NEAREST)/2.0f + NEAREST)
/**
 * Initialize our vertices.
 * @param td -  a pointer to our local_data structure (which holds
 *              the vertices we'll be init'ing
 */
void init_verts(test_data *td, float nearest, float mid, float farthest) {
  GrVertex *verts =  td->tri_verts;
  float rgba = 255.0f; // default color/alpha
  int i;

  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_A,  GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE);  

  if (td->use_z_depth) {
    grVertexLayout(GR_PARAM_Z,   GR_VERTEX_OOZ_OFFSET << 2, GR_PARAM_ENABLE);
  } else {
    grVertexLayout(GR_PARAM_Q,   GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
  }

  // triangle
  verts[0].x = -1.0f; verts[0].y =  0.0f; verts[0].oow = verts[0].ooz = farthest;
  verts[1].x =  1.0f; verts[1].y =  1.0f; verts[1].oow = verts[1].ooz = nearest;
  verts[2].x =  1.0f; verts[2].y = -1.0f; verts[2].oow = verts[2].ooz = nearest;

  for (i = 0; i < NUM_TRI_VERTS; i++) {
    verts[i].a = verts[i].r = verts[i].g = verts[i].b = rgba;
  }

  // lines
  verts =  td->line_verts;
  verts[0].x = -1.0f; verts[0].y = -0.8f; verts[0].oow = verts[0].ooz = farthest;
  verts[1].x =  1.0f; verts[1].y = -0.8f; verts[1].oow = verts[1].ooz = nearest;
  verts[2].x = -1.0f; verts[2].y =  0.8f; verts[2].oow = verts[2].ooz = farthest;
  verts[3].x =  1.0f; verts[3].y =  0.8f; verts[3].oow = verts[3].ooz = nearest;

  for (i = 0; i < NUM_LINE_VERTS; i++) {
    verts[i].a = verts[i].r = verts[i].g = verts[i].b = rgba;
  }

  // vertex array
  verts =  td->va_verts;
  verts[0].x = -1.0f; verts[0].y = -0.6f; verts[0].oow = verts[0].ooz = farthest;
  verts[1].x =  1.0f; verts[1].y = -0.3f; verts[1].oow = verts[1].ooz = nearest;
  verts[2].x = -1.0f; verts[2].y =  0.0f; verts[2].oow = verts[2].ooz = farthest;
  verts[3].x =  1.0f; verts[3].y =  0.4f; verts[3].oow = verts[3].ooz = nearest;
  verts[4].x = -1.0f; verts[4].y =  0.7f; verts[4].oow = verts[4].ooz = farthest;
  verts[5].x =  1.0f; verts[5].y =  0.9f; verts[5].oow = verts[5].ooz = nearest;

  for (i = 0; i < NUM_VA_VERTS; i++) {
    verts[i].a = verts[i].r = verts[i].g = verts[i].b = rgba;
  }

  // vertex array polygon (has to be convex)
  verts =  td->va_poly_verts;
  verts[0].x = -1.0f; verts[0].y = -0.1f; verts[0].oow = verts[0].ooz = farthest;
  verts[1].x =  0.0f; verts[1].y = -0.8f; verts[1].oow = verts[1].ooz = mid;
  verts[2].x =  1.0f; verts[2].y =  0.2f; verts[2].oow = verts[2].ooz = nearest;
  verts[3].x =  0.0f; verts[3].y =  0.7f; verts[3].oow = verts[3].ooz = mid;

  for (i = 0; i < NUM_VA_POLY_VERTS; i++) {
    verts[i].a = verts[i].r = verts[i].g = verts[i].b = rgba;
  }
}

void scale_and_translate(test_data *td, GrVertex *xformed_verts, GrVertex *verts,
                         int num_verts, int row)
{
  for (int v = 0; v < num_verts; v++) {
    xformed_verts[v] = verts[v];
    xformed_verts[v].y *= td->scale_y;
    xformed_verts[v].y += (row * td->delta_y) +
                           td->delta_y/2.0f + td->origin_y;

    xformed_verts[v].x *= td->scale_x;
    xformed_verts[v].x += td->delta_x + td->origin_x;
  }
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
  local_ptr->scale_y = 1.0f;
  local_ptr->scale_x = 1.0f;
  local_ptr->delta_y = 1.0f;
  local_ptr->origin_x = 0;
  local_ptr->origin_y = 0;
  local_ptr->rows = NUM_PRIMS;
  local_ptr->num_lines = 20;
  local_ptr->use_z_depth = 0;

  // handle our test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    if(!_stricmp(*local_argv,"-nl") || // number of vertical lines 
       !_stricmp(*local_argv,"/nl")) {
      ++local_argv;
      ++num_arg;
      if(!sscanf(*local_argv,"%d",&(local_ptr->num_lines))) {
        // bad argument
      } 
      --local_argc;
    } else if(!_stricmp(*local_argv,"-z") || // use Z buffering
       !_stricmp(*local_argv,"/z")) {
      local_ptr->use_z_depth = 1;
    } else {
      fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
      ++local_argv;
      ++num_arg;
    }
    ++local_argv;
    ++num_arg;
  }

  // need a w / z buffer
  if (state->num_aux_bufs < 1) {
    state->num_aux_bufs = 1;
  }
  
  // we need at least one color buffer
  if (state->num_color_bufs < 1) {
    state->num_color_bufs = 1;
  }

  // xycheck and xylimit not supported (fix this when adding self-checking)
  state->xycheck = 0;
  state->xylimit = 0;

  // setup image file names
  // this is mandatory
  sprintf(tmp_str, "%s%s",
          test_name,
          (local_ptr->use_z_depth? "-z" : "-w"));

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

  grClipWindow(state->conform_x,    
               state->conform_y, 
               state->conform_x + state->conform_width, 
               state->conform_y + state->conform_height);



  grViewport(state->conform_x, state->conform_y, 
             state->conform_width, state->conform_height);

  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_CONSTANT,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );

  // z buffering on
  FxU32 depth_range[2];

  if (local_ptr->use_z_depth) {
    grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
    grGet(GR_ZDEPTH_MIN_MAX, 8, (long*)depth_range);   // min, max 
  } else {
    grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
    grGet(GR_WDEPTH_MIN_MAX, 8, (long*)depth_range);   // min, max 
  }
  grDepthMask( FXTRUE );


  grDepthBufferFunction(GR_CMP_GREATER); // 1/smaller wins
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);

  // fill in our initial vertex data 
  if (local_ptr->use_z_depth) {
    init_verts(local_ptr, depth_range[0]/NEAREST, depth_range[0]/MID, 
                          depth_range[0]/FARTHEST);
    grBufferClear(0, 0, depth_range[1]);
  } else {
    init_verts(local_ptr, 1.0f/NEAREST, 1.0f/MID, 1.0f/FARTHEST);
    grBufferClear(0, 0, depth_range[0]);
  }


  // compute the offsets and the scale factors to use.
  local_ptr->scale_y = (float)state->conform_height/local_ptr->rows * 0.4f; 
  local_ptr->delta_y = (float)state->conform_height/local_ptr->rows;
  local_ptr->scale_x = (float)state->conform_width/2.0f;
  local_ptr->delta_x = (float)state->conform_width/2.0f;
  local_ptr->origin_x = state->conform_x;
  local_ptr->origin_y = state->conform_y;

  // pick a constant color
  grConstantColorValue(0x0FFffFFff); // white

  float delta_y = local_ptr->delta_y;
  GrVertex xformed_verts[MAX_VERTS];

  for (int row = 0; row < local_ptr->rows; row++) {

    switch (row) { // one per primitive
      case PRIM_TRIANGLES:
        scale_and_translate(local_ptr, xformed_verts, local_ptr->tri_verts, 
                            NUM_TRI_VERTS, row);
        grDrawTriangle(&xformed_verts[0], 
                       &xformed_verts[1], 
                       &xformed_verts[2]);
        add_command_data(state, "grDrawTriangle()", 0);
        add_vertex_data(state, xformed_verts, 3);
        break;
      case PRIM_LINES:
        scale_and_translate(local_ptr, xformed_verts, local_ptr->line_verts, 
                            NUM_LINE_VERTS, row);
        for (v = 0; v < NUM_LINE_VERTS; v+=2) {
          grDrawLine(&xformed_verts[v], &xformed_verts[v+1]);
          add_command_data(state, "grDrawLine()", 0);
          add_vertex_data(state, &xformed_verts[v], 2);
        }
        break;
      case PRIM_VA_LINES: 
      case PRIM_VA_LINE_STRIP: 
      case PRIM_VA_TRIANGLES:
      case PRIM_VA_TRIANGLE_STRIP:
      case PRIM_VA_TRIANGLE_FAN: {
        int mode = va_modes[row - PRIM_VA_TRIANGLES];
        scale_and_translate(local_ptr, xformed_verts, local_ptr->va_verts, 
                            NUM_VA_VERTS, row);
        grDrawVertexArrayContiguous(mode,
                                    NUM_VA_VERTS, 
                                    xformed_verts,
                                    sizeof(GrVertex));
        add_command_data(state, "grDrawVertexArrayContiguous()", 4,
                                 mode, NUM_VA_VERTS, NULL, 
                                 sizeof(GrVertex));
        add_vertex_data(state, xformed_verts, NUM_VA_VERTS);
        break;
      }
      case PRIM_VA_POLYGON: 
        scale_and_translate(local_ptr, xformed_verts, local_ptr->va_poly_verts, 
                            NUM_VA_POLY_VERTS, row);
        grDrawVertexArrayContiguous(GR_POLYGON,
                                    NUM_VA_POLY_VERTS, 
                                    xformed_verts,
                                    sizeof(GrVertex));
        add_command_data(state, "grDrawVertexArrayContiguous()", 4,
                                GR_POLYGON, NUM_VA_POLY_VERTS, NULL, 
                                sizeof(GrVertex));
        add_vertex_data(state, xformed_verts, NUM_VA_POLY_VERTS);
        break;
      default: 
        break;
    }
  }

  // At this point, our primitives are drawn. Now put in the constant
  // depth vertical lines. Well, each line is at a constant depth, but 
  // each also gets nearer in a linear fashion.

  grConstantColorValue(rgbacolor(state, 0x0000ffFF));  // blue

  float line_x_inc = (float)state->conform_width/local_ptr->num_lines;
  int line_x_origin = state->conform_x + 1;
  int line_y_origin = state->conform_y + 1;
  int line_y_end = state->conform_y + state->conform_height;

  float oowz_near = 1.0f/NEAREST;
  float oowz_far  = 1.0f/FARTHEST;

  if (local_ptr->use_z_depth) {
    oowz_near = depth_range[0]/NEAREST;
    oowz_far = depth_range[0]/FARTHEST;
  }

  float line_depth_inc = (oowz_near - oowz_far) / local_ptr->num_lines;

  // this is the offset that we'll give the constant depth lines
  // on either side of the interpolated depth prims.  We'll draw
  // the constant depth lines alternating, first farther than
  // the interpolated prims, then closer. If it's too small, then
  // we'll run into depth-buffer precision problems.   The end
  // result should be a set of lines alternating on each side of
  // the prims.

  // If we're W-buffering, make it 1/100th of the range of the
  // W buffer (arbitrary).
  float line_depth_offset = 100.0f/65535.0f;

  // If we're Z-buffering, make it 1/4 the depth incremenent. (arbitrary)
  if (local_ptr->use_z_depth) {
    line_depth_offset = line_depth_inc/4.0f;
  }

  GrVertex line_verts[2];
  // Zero out the line_verts to avoid writing large bogus numbers into
  // the data file.
  memset(line_verts, 0, sizeof(line_verts));
  for (int i = 0; i < local_ptr->num_lines; i++) {
    line_verts[0].x   = i * line_x_inc + line_x_origin;
    line_verts[0].y   = 0.0f;
    line_verts[0].oow = line_verts[0].ooz = 
       (float) oowz_far + (line_depth_inc * i) + line_depth_offset; 

    line_verts[1].x   = i * line_x_inc + line_x_origin;
    line_verts[1].y   = (float)line_y_end;
    line_verts[1].oow = line_verts[1].ooz = 
       (float) oowz_far + (line_depth_inc * i) + line_depth_offset; 

    grDrawLine(&line_verts[0], &line_verts[1]);
    add_command_data(state, "grDrawLine()", 0);
    add_vertex_data(state, &line_verts[0], 2);

    line_depth_offset = -line_depth_offset; // flip the offset for next time
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


  sprintf(tmp_str,"sx= %d sy = %d wid = %d hei = %d",
    state->conform_x,
    state->conform_y,
    state->conform_width,
    state->conform_height);
  log_message(state,tmp_str);

  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -nl <n>  draw <n> vertical lines\n");
  fprintf(stderr,"   -z       use Z buffering, not W buffering\n");
}


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

static char *test_name = "flex-coords-depth-color";
static char *test_description = 
 "Using a random vertex layout, draw varied depth prims and verify their depth "
 "is being linearly interpolated by also drawing constant depth lines on both "
 "sides of them. Compute the depth of the lines ourselves. Also iterate on "
 "vertex colors.";

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

#define CLIP_COORDS_W_VALUE 1.1f // give it a w != 1.0 in clip coords mode

#define MAX_VERTS 6

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

#define NUM_SLOTS 50 // vertex data can hold 50 floats

// An arbitrarily sized vertex data structure. Vertex params will
// be placed at random locations in it.
typedef struct {
  float slot[NUM_SLOTS];
} VertexData;

typedef struct {
  int rows;
  int num_lines;       // number of constant depth lines
  float scale_x;       // amount to scale each rotation by in x
  float scale_y;       // amount to scale each rotation by in y
  float delta_x;       // amount to offset each successive rotation in x
  float delta_y;       // amount to offset each successive rotation in y
  float origin_x;
  float origin_y;
  int do_z_depth;      // use z depth instead of w
  int debug;
  int do_vertex_color; // set per vertex colors
  int use_window_coords; // use window coords instead of clip coords

  int x_offset;        // Various offsets into the array of slots in VertexData
  int y_offset;     
  int w_offset;
  int oow_offset;   
  int ooz_offset;   
  int pargb_offset;    

  VertexData line_verts[MAX_VERTS];  // the vertex data for each prim
  VertexData tri_verts[MAX_VERTS]; 
  VertexData va_verts[MAX_VERTS]; 
  VertexData va_poly_verts[MAX_VERTS]; 
} test_data;

#define NEAREST   10.0f
#define FARTHEST  40.0f
#define MID ((FARTHEST - NEAREST)/2.0f + NEAREST)

#define PARGB(_a, _r, _g, _b) (FxU32)(((_a & 0xff) << 24) | \
                                      ((_r & 0xff) << 16) | \
                                      ((_g & 0xff) << 8)  | \
                                      ((_b & 0xff)))

// An array of vertex colors to iterate through
static FxU32 pargb[] = {
 PARGB(255, 255, 000, 000),
 PARGB(255, 000, 255, 000),
 PARGB(255, 255, 000, 255),
 PARGB(255, 255, 255, 000),
 PARGB(255, 255, 000, 255),
 PARGB(255, 000, 255, 255),  
};
#define NUM_PARGB (sizeof(pargb) / sizeof(FxU32))

/**
 * Initialize our vertices.
 * @param local_ptr -  a pointer to our local_data structure (which holds
 *              the vertices we'll be init'ing
 */
void init_verts(test_data *local_ptr, 
                int which, // which prim (one per row)
                int do_vertex_color,
                float nearest, float mid, float farthest) {


  int i;
  float rgba = 255.0f; // default color/alpha
  VertexData *verts =  local_ptr->tri_verts;

  

  switch(which) { 
    case PRIM_TRIANGLES: // triangles
      verts =  local_ptr->tri_verts;
      verts[0].slot[local_ptr->x_offset] = -1.0f; 
      verts[0].slot[local_ptr->y_offset] =  0.0f;
      verts[0].slot[local_ptr->oow_offset] = 
      verts[0].slot[local_ptr->ooz_offset] = farthest;

      verts[1].slot[local_ptr->x_offset] =  1.0f; 
      verts[1].slot[local_ptr->y_offset] =  1.0f; 
      verts[1].slot[local_ptr->oow_offset] = 
      verts[1].slot[local_ptr->ooz_offset] = nearest;

      verts[2].slot[local_ptr->x_offset] =  1.0f; 
      verts[2].slot[local_ptr->y_offset] = -1.0f; 
      verts[2].slot[local_ptr->oow_offset] =
      verts[2].slot[local_ptr->ooz_offset] = nearest;

      for (i = 0; i < NUM_TRI_VERTS; i++) {
        verts[i].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;
        memcpy(&(verts[i].slot[local_ptr->pargb_offset]), &pargb[i % NUM_PARGB], 
               sizeof(FxU32));
      }
      break;

    case PRIM_LINES: // lines
      verts =  local_ptr->line_verts;
    
      verts[0].slot[local_ptr->x_offset] = -1.0f; 
      verts[0].slot[local_ptr->y_offset] = -0.8f; 
      verts[0].slot[local_ptr->oow_offset] = 
      verts[0].slot[local_ptr->ooz_offset] = farthest;
      
      verts[1].slot[local_ptr->x_offset] =  1.0f; 
      verts[1].slot[local_ptr->y_offset] = -0.8f; 
      verts[1].slot[local_ptr->oow_offset] = 
      verts[1].slot[local_ptr->ooz_offset] = nearest;
    
      verts[2].slot[local_ptr->x_offset] = -1.0f; 
      verts[2].slot[local_ptr->y_offset] =  0.8f; 
      verts[2].slot[local_ptr->oow_offset] = 
      verts[2].slot[local_ptr->ooz_offset] = farthest;
    
      verts[3].slot[local_ptr->x_offset] =  1.0f; 
      verts[3].slot[local_ptr->y_offset] =  0.8f; 
      verts[3].slot[local_ptr->oow_offset] = 
      verts[3].slot[local_ptr->ooz_offset] = nearest;
    
      for (i = 0; i < NUM_LINE_VERTS; i++) {
        verts[i].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;
        memcpy(&(verts[i].slot[local_ptr->pargb_offset]), &pargb[i % NUM_PARGB], 
               sizeof(FxU32));
      }
      break;

      case PRIM_VA_TRIANGLES:        // vertex array prims, not polygon
      case PRIM_VA_TRIANGLE_STRIP:
      case PRIM_VA_TRIANGLE_FAN:
      case PRIM_VA_LINES:
      case PRIM_VA_LINE_STRIP:
      verts =  local_ptr->va_verts;
      verts[0].slot[local_ptr->x_offset] = -1.0f; 
      verts[0].slot[local_ptr->y_offset] = -0.6f; 
      verts[0].slot[local_ptr->oow_offset] = 
      verts[0].slot[local_ptr->ooz_offset] = farthest;
    
      verts[1].slot[local_ptr->x_offset] =  1.0f; 
      verts[1].slot[local_ptr->y_offset] = -0.3f; 
      verts[1].slot[local_ptr->oow_offset] = 
      verts[1].slot[local_ptr->ooz_offset] = nearest;
    
      verts[2].slot[local_ptr->x_offset] = -1.0f; 
      verts[2].slot[local_ptr->y_offset] =  0.0f; 
      verts[2].slot[local_ptr->oow_offset] = 
      verts[2].slot[local_ptr->ooz_offset] = farthest;
    
      verts[3].slot[local_ptr->x_offset] =  1.0f; 
      verts[3].slot[local_ptr->y_offset] =  0.4f; 
      verts[3].slot[local_ptr->oow_offset] = 
      verts[3].slot[local_ptr->ooz_offset] = nearest;
    
      verts[4].slot[local_ptr->x_offset] = -1.0f; 
      verts[4].slot[local_ptr->y_offset] =  0.7f; 
      verts[4].slot[local_ptr->oow_offset] = 
      verts[4].slot[local_ptr->ooz_offset] = farthest;
    
      verts[5].slot[local_ptr->x_offset] =  1.0f; 
      verts[5].slot[local_ptr->y_offset] =  0.9f; 
      verts[5].slot[local_ptr->oow_offset] = 
      verts[5].slot[local_ptr->ooz_offset] = nearest;
    
      for (i = 0; i < NUM_VA_VERTS; i++) {
        verts[i].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;
        memcpy(&(verts[i].slot[local_ptr->pargb_offset]), &pargb[i % NUM_PARGB], 
               sizeof(FxU32));
      }
      break;
    
    case PRIM_VA_POLYGON: // vertex array polygon (has to be convex)
      verts =  local_ptr->va_poly_verts;
      verts[0].slot[local_ptr->x_offset] = -1.0f; 
      verts[0].slot[local_ptr->y_offset] = -0.1f; 
      verts[0].slot[local_ptr->oow_offset] = 
      verts[0].slot[local_ptr->ooz_offset] = farthest;
    
      verts[1].slot[local_ptr->x_offset] =  0.0f; 
      verts[1].slot[local_ptr->y_offset] = -0.8f; 
      verts[1].slot[local_ptr->oow_offset] =
      verts[1].slot[local_ptr->ooz_offset] = mid;
    
      verts[2].slot[local_ptr->x_offset] =  1.0f; 
      verts[2].slot[local_ptr->y_offset] =  0.2f; 
      verts[2].slot[local_ptr->oow_offset] =
      verts[2].slot[local_ptr->ooz_offset] = nearest;
    
      verts[3].slot[local_ptr->x_offset] =  0.0f; 
      verts[3].slot[local_ptr->y_offset] =  0.7f; 
      verts[3].slot[local_ptr->oow_offset] = 
      verts[3].slot[local_ptr->ooz_offset] = mid;
    
      for (i = 0; i < NUM_VA_POLY_VERTS; i++) {
        verts[i].slot[local_ptr->w_offset] =  CLIP_COORDS_W_VALUE;
        memcpy(&(verts[i].slot[local_ptr->pargb_offset]), &pargb[i % NUM_PARGB], 
               sizeof(FxU32));
      }
      break;
  }
}

void scale_and_translate(test_data *local_ptr, 
                         VertexData *xformed_verts,
                         VertexData *verts,
                         int num_verts, int row)
{

  for (int v = 0; v < num_verts; v++) {
    xformed_verts[v] = verts[v];
    xformed_verts[v].slot[local_ptr->y_offset] *= local_ptr->scale_y;
    xformed_verts[v].slot[local_ptr->y_offset] += 
                                        (row * local_ptr->delta_y) +
                                        local_ptr->delta_y/2.0f + 
                                        local_ptr->origin_y;

    xformed_verts[v].slot[local_ptr->x_offset] *= local_ptr->scale_x;
    xformed_verts[v].slot[local_ptr->x_offset] += local_ptr->delta_x + 
                                                  local_ptr->origin_x;
  }
}

void 
local_add_vertex_data(conform_state *state, VertexData *in, int num_verts)
{
  GrVertex v;
  test_data *local_ptr = (test_data *)state->local_data;

  // record the vertices into the data file
  for (int i = 0; i < num_verts; i++) {
    memset(&v, 0, sizeof(v));
    v.x = in[i].slot[local_ptr->x_offset];
    v.y = in[i].slot[local_ptr->y_offset];
    v.w = in[i].slot[local_ptr->w_offset];
    add_vertex_data(state, &v, 1);
  }
}


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
 
  // Init all the triangle data slots
  for (i = 0; i < NUM_TRI_VERTS; i++) {
    for (slot = 0; slot < NUM_SLOTS; slot++) {
      local_ptr->tri_verts[i].slot[slot] = 0.0f;
    }
  }
  // Init all the line data slots
  for (i = 0; i < NUM_LINE_VERTS; i++) {
    for (slot = 0; slot < NUM_SLOTS; slot++) {
      local_ptr->line_verts[i].slot[slot] = 0.0f;
    }
  }
  // Init all the VA data slots
  for (i = 0; i < NUM_VA_VERTS; i++) {
    for (slot = 0; slot < NUM_SLOTS; slot++) {
      local_ptr->va_verts[i].slot[slot] = 0.0f;
    }
  }
  // Init all the VA polygon data slots
  for (i = 0; i < NUM_VA_POLY_VERTS; i++) {
    for (slot = 0; slot < NUM_SLOTS; slot++) {
      local_ptr->va_poly_verts[i].slot[slot] = 0.0f;
    }
  }



  for (slot = 0; slot < NUM_SLOTS; slot++) {
    slots_taken[slot] = 0x0;
  }

  // PARAM_XY are always in slots 0 and 1, and are always set

  // Assign slots to all the params
  slots_taken[0] = slots_taken[1] = 1;
  local_ptr->x_offset = 0;
  local_ptr->y_offset = 1;
  local_ptr->w_offset = find_slot(slots_taken, 1);
  local_ptr->oow_offset = find_slot(slots_taken, 1);
  local_ptr->ooz_offset = find_slot(slots_taken, 1);
  local_ptr->pargb_offset = find_slot(slots_taken, 1); // u_int, not float

  // Make sure we got slots for all the params
  if (local_ptr->y_offset == -1 ||
      local_ptr->oow_offset == -1 ||
      local_ptr->w_offset == -1 ||
      local_ptr->ooz_offset == -1 ||
      local_ptr->pargb_offset == -1 ) {
      log_perror(state, "Couldn't generate a vertex layout!\n");
      return FXFALSE;
  }

  grVertexLayout(GR_PARAM_XY, local_ptr->x_offset << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_W, local_ptr->w_offset << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_PARGB, local_ptr->pargb_offset << 2, GR_PARAM_ENABLE);

  if (local_ptr->do_z_depth) {
    grVertexLayout(GR_PARAM_Z,   local_ptr->ooz_offset << 2, GR_PARAM_ENABLE);
  } else {
    grVertexLayout(GR_PARAM_Q,   local_ptr->oow_offset << 2, GR_PARAM_ENABLE);
  }

  if (local_ptr->debug) {
    printf("GR_PARAM_XY: %d\n", local_ptr->x_offset); 
    printf("GR_PARAM_W: %d\n", local_ptr->w_offset); 
    printf("GR_PARAM_Z: %d\n", local_ptr->ooz_offset); 
    printf("GR_PARAM_Q: %d\n", local_ptr->oow_offset); 
    printf("GR_PARAM_PARGB: %d\n", local_ptr->pargb_offset); 
  }

  return FXTRUE;
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
  local_ptr->debug = 0;
  local_ptr->scale_y = 1.0f;
  local_ptr->scale_x = 1.0f;
  local_ptr->delta_y = 1.0f;
  local_ptr->origin_x = 0.0f;
  local_ptr->origin_y = 0.0f;
  local_ptr->rows = NUM_PRIMS;
  local_ptr->num_lines = 20;
  local_ptr->do_z_depth = 0;        // do w depth by default
  local_ptr->do_vertex_color = 0;   // 1st pass doesn't do vertex colors
  local_ptr->use_window_coords = 0; // use clip coords by default
  local_ptr->x_offset = -1;         // bogus default value
  local_ptr->y_offset = -1;         // bogus default value
  local_ptr->oow_offset = -1;       // bogus default value
  local_ptr->ooz_offset = -1;       // bogus default value
  local_ptr->pargb_offset = -1;     // bogus default value

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
      local_ptr->do_z_depth = 1;
    } else if(!_stricmp(*local_argv,"-debug") || // enable debug output
              !_stricmp(*local_argv,"/debug")) {
      local_ptr->debug = 1;
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

  // need a w / z buffer
  if (state->num_aux_bufs < 1) {
    state->num_aux_bufs = 1;
  }
  
  // we need at least one color buffer
  if (state->num_color_bufs < 1) {
    state->num_color_bufs = 1;
  }
   
  state->num_reps = 2;

  // xycheck and xylimit not supported (fix this when adding self-checking)
  state->xycheck = 0;
  state->xylimit = 0;

  // setup image file names
  // this is mandatory
  sprintf(tmp_str, "%s%s%s",
          test_name,
          (local_ptr->use_window_coords? "-wc" : ""),
          (local_ptr->do_z_depth? "-z" : "-w"));

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
  float depth_near, depth_mid, depth_far;
  int v = 0;

  local_ptr = (test_data *)state->local_data;

  grClipWindow(state->conform_x,    
               state->conform_y, 
               state->conform_x + state->conform_width, 
               state->conform_y + state->conform_height);

  if (!local_ptr->use_window_coords) {
    grCoordinateSpace(GR_CLIP_COORDS);
  }

  grViewport(state->conform_x, state->conform_y, 
             state->conform_width, state->conform_height);

  if (local_ptr->do_vertex_color) {
    grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                   GR_COMBINE_FACTOR_NONE,
                   GR_COMBINE_LOCAL_ITERATED,
                   GR_COMBINE_OTHER_NONE,
                   FXFALSE );
  } else {
    grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                   GR_COMBINE_FACTOR_NONE,
                   GR_COMBINE_LOCAL_CONSTANT,
                   GR_COMBINE_OTHER_NONE,
                   FXFALSE );
  }

  // z buffering on
  FxU32 depth_range[2];

  if (local_ptr->do_z_depth) {
    grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
    grGet(GR_ZDEPTH_MIN_MAX, 8, (long*)depth_range);   // min, max 
  } else {
    grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
    grGet(GR_WDEPTH_MIN_MAX, 8, (long*)depth_range);   // min, max 
  }
  grDepthMask( FXTRUE );

  grDepthBufferFunction(GR_CMP_GREATER); // 1/smaller wins
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);

  if (local_ptr->do_z_depth) {
    if (local_ptr->use_window_coords) {
      depth_near = depth_range[0]/NEAREST;
      depth_mid  = depth_range[0]/MID;
      depth_far  = depth_range[0]/FARTHEST;
      grBufferClear(0, 0, depth_range[1]);
    } else { // clip coords
      depth_near = CLIP_COORDS_W_VALUE/NEAREST; // will be divided by w again
      depth_mid  = CLIP_COORDS_W_VALUE/MID;     // will be divided by w again
      depth_far  = CLIP_COORDS_W_VALUE/FARTHEST;// will be divided by w again
      grBufferClear(0, 0, depth_range[1]);
      grDepthRange(0.0f, 1.0f);
    }
  } else {
    depth_near = 1.0f/NEAREST;
    depth_mid  = 1.0f/MID;
    depth_far  = 1.0f/FARTHEST;
    grBufferClear(0, 0, depth_range[0]);
  }

  // compute the offsets and the scale factors to use.
  if (local_ptr->use_window_coords) {
    local_ptr->scale_y = (float)state->conform_height/local_ptr->rows * 0.4f; 
    local_ptr->delta_y = (float)state->conform_height/local_ptr->rows;
    local_ptr->scale_x = (float)state->conform_width/2.0f;
    local_ptr->delta_x = (float)state->conform_width/2.0f;
    local_ptr->origin_x = (float)state->conform_x;
    local_ptr->origin_y = (float)state->conform_y;
  } else { // clip coords
    local_ptr->scale_y = (float)2.0f/local_ptr->rows * 0.4f; 
    local_ptr->delta_y = (float)2.0f/local_ptr->rows;
    local_ptr->scale_x = (float)1.0f;
    local_ptr->delta_x = (float)1.0f;
    local_ptr->origin_x = -1.0f; 
    local_ptr->origin_y = -1.0f; 
   }

  // pick a constant color
  grConstantColorValue(0x0FFffFFff); // white

  float delta_y = local_ptr->delta_y;
  VertexData xformed_verts[MAX_VERTS];

  for (int row = 0; row < local_ptr->rows; row++) {

    // Generate a random vertex layout for each prim
    gen_layout(state, local_ptr);
    init_verts(local_ptr, row, local_ptr->do_vertex_color, 
               depth_near, depth_mid, depth_far);

    memset(xformed_verts, 0, sizeof(xformed_verts));

    switch (row) { // one per primitive
      case PRIM_TRIANGLES:
        scale_and_translate(local_ptr, xformed_verts, local_ptr->tri_verts, 
                            NUM_TRI_VERTS, row);
        grDrawTriangle(&xformed_verts[0], 
                       &xformed_verts[1], 
                       &xformed_verts[2]);
        add_command_data(state, "grDrawTriangle()", 0);
        local_add_vertex_data(state, xformed_verts, 3);
        break;
      case PRIM_LINES:
        scale_and_translate(local_ptr, xformed_verts, local_ptr->line_verts, 
                            NUM_LINE_VERTS, row);
        for (v = 0; v < NUM_LINE_VERTS; v+=2) {
          grDrawLine(&xformed_verts[v], &xformed_verts[v+1]);
          add_command_data(state, "grDrawLine()", 0);
          local_add_vertex_data(state, &xformed_verts[v], 2);
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
                                    sizeof(VertexData));
        add_command_data(state, "grDrawVertexArrayContiguous()", 4,
                                 mode, NUM_VA_VERTS, NULL, 
                                 sizeof(VertexData));
        local_add_vertex_data(state, xformed_verts, NUM_VA_VERTS);
        break;
      }
      case PRIM_VA_POLYGON: 
        scale_and_translate(local_ptr, xformed_verts, local_ptr->va_poly_verts, 
                            NUM_VA_POLY_VERTS, row);
        grDrawVertexArrayContiguous(GR_POLYGON,
                                    NUM_VA_POLY_VERTS, 
                                    xformed_verts,
                                    sizeof(VertexData));
        add_command_data(state, "grDrawVertexArrayContiguous()", 4,
                                GR_POLYGON, NUM_VA_POLY_VERTS, NULL, 
                                sizeof(VertexData));
        local_add_vertex_data(state, xformed_verts, NUM_VA_POLY_VERTS);
        break;
      default: 
        break;
    }
  }

  // At this point, our primitives are drawn. Now put in the constant
  // depth vertical lines. Well, each line is at a constant depth, but 
  // each also gets nearer in a linear fashion.


  // lines are drawn in constant color
  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_CONSTANT,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );

  if (local_ptr->do_vertex_color) {
    grConstantColorValue(rgbacolor(state, 0xffFFffFF));  // white
  } else {
    grConstantColorValue(rgbacolor(state, 0x0000ffFF));  // blue
  }


  // Get depth range for lines
  float oowz_near = 1.0f/NEAREST;
  float oowz_far  = 1.0f/FARTHEST;

  if (local_ptr->do_z_depth) {
    if (local_ptr->use_window_coords) {
      oowz_near = depth_range[0]/NEAREST;
      oowz_far = depth_range[0]/FARTHEST;
    } else { // clip coords
      oowz_near = CLIP_COORDS_W_VALUE/NEAREST;
      oowz_far = CLIP_COORDS_W_VALUE/FARTHEST;
    }
  }

  // Compute the amount to increment the depth of each line
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
  if (local_ptr->do_z_depth) {
    line_depth_offset = line_depth_inc/4.0f;
  }

  // Set up line stepping
  float line_x_inc;
  float line_x_origin, line_y_origin, line_y_end;
 
  if (local_ptr->use_window_coords) { 
    line_x_inc = (float)state->conform_width/local_ptr->num_lines;
    line_x_origin = (float)state->conform_x;
    line_y_origin = (float)state->conform_y;
    line_y_end = (float)(state->conform_y + state->conform_height);
  } else { // clip coords
    line_x_inc = (float)2.0f/local_ptr->num_lines;
    line_x_origin = -1.0f;
    line_y_origin = -1.0f;
    line_y_end = 1.0f;
  }

  VertexData line_verts[2];
  // Zero out the line_verts to avoid writing large bogus numbers into
  // the data file.
  memset(line_verts, 0, sizeof(line_verts));

  for (int i = 0; i < local_ptr->num_lines; i++) {
    line_verts[0].slot[local_ptr->x_offset]   = i * line_x_inc + line_x_origin;
    line_verts[0].slot[local_ptr->y_offset]   = line_y_origin;
    line_verts[0].slot[local_ptr->w_offset]   = CLIP_COORDS_W_VALUE;
    line_verts[0].slot[local_ptr->oow_offset] =
    line_verts[0].slot[local_ptr->ooz_offset] = 
       (float) oowz_far + (line_depth_inc * i) + line_depth_offset; 

    line_verts[1].slot[local_ptr->x_offset]   = i * line_x_inc + line_x_origin;
    line_verts[1].slot[local_ptr->y_offset]   = line_y_end;
    line_verts[1].slot[local_ptr->w_offset]   = CLIP_COORDS_W_VALUE;
    line_verts[1].slot[local_ptr->oow_offset] = 
    line_verts[1].slot[local_ptr->ooz_offset] = 
       (float) oowz_far + (line_depth_inc * i) + line_depth_offset; 

    grDrawLine(&line_verts[0], &line_verts[1]);
    add_command_data(state, "grDrawLine()", 0);
    local_add_vertex_data(state, &line_verts[0], 2);

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

  local_ptr->do_vertex_color = 1;

  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -nl <n>  draw <n> vertical lines\n");
  fprintf(stderr,"   -wc      use window coords and not clip coords\n");
  fprintf(stderr,"   -z       use Z buffering, not W buffering\n");
}


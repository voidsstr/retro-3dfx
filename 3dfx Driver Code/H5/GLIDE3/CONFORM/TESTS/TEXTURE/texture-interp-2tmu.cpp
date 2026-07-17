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

static char *test_name = "texture-interp-2tmu";
static char *test_description = 
  "test 2 tmu texture interpolation across various odd-shaped triangles";

// TODO: Bump up NUM_ROTS when self-checking is in!
#define NUM_ROTS  7    // Default number of rotations of each prim

#define MAX_VERTS 5      // Number of vertices in our prim

#define SCALE_TWEAK 0.35f // Our prims are from -1 to +1, which means that,
                          // when rotated, they can project outside of their
                          // bounding box. (sqrt(1^2 + 1^2) == 1.4)
                          // So, scale everything by SCALE_TWEAK to make sure
                          // they don't leave their bounding boxes. 
                          // 1.4 * .35 == .49, .49 * 2 == .98

#define CLIP_COORDS_W_VALUE 3.0f // arbitrary

// The texture coords we'll use
#define TEXTURE_MIN 0.0f
#define TEXTURE_MAX 256.0f
#define TEXTURE_CENTER 128.0f

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

#define ARGB_1555(_a,_r,_g,_b) \
  (FxU16)((_a<<15) | ((_r & 0x1F) << 10) | ((_g & 0x1F) << 5) | (_b & 0x1F))

// The combinations of rotations that we'll do. Uncomment the rest if you
// want funky shapes.
static int rot_flags[] = {
  ROT_X,
  ROT_Y,
  ROT_Z,
//  ROT_X | ROT_Y,
  ROT_X | ROT_Z,
  ROT_Y | ROT_Z,
  ROT_X | ROT_Y | ROT_Z,
//  ROT_INVERT_X | ROT_Y,
  ROT_INVERT_X | ROT_Z,
  ROT_INVERT_Y | ROT_Z,
  ROT_INVERT_X | ROT_Y | ROT_Z,
  ROT_INVERT_X | ROT_Y | ROT_INVERT_Z,
};

#define NUM_ROT_TYPES (sizeof(rot_flags) / sizeof(int))

// Data specific to this test.
typedef struct {
  int test_pass;
  Vertex verts[MAX_VERTS]; // pointer to all of our primitive verts
  int num_verts;       // number of vertices currently in use
  int num_rotations;   // number of rotations of each prim per image
  int rowcol;          // number of rows and columns per image
  float angle_incr;    // amount to incremement each rotation
  float scale_x;       // amount to scale each rotation by in x
  float scale_y;       // amount to scale each rotation by in y
  float offset_x;       // amount to offset each successive rotation in x
  float offset_y;       // amount to offset each successive rotation in y
  int do_aa;           // enable anti-aliasing
  int do_clip;         // enable setting of arbitrary clip windows
  int do_adj_tex_coords; // enable adjusting the texture coords to clip it
  int use_window_coords; // use window coords instead of clip coords
  int do_clip_texture; // enable 
  int xform_flags;     // which xforms to apply
  int cur_rep;         // the index into rot_flags that we're applying
  int debug;           // enable debugging output
  int texture_wh;      // width and height of texture to make
  GrLOD_t texture_lod; // LOD of texture
  
} test_data;



/*----------------------------------------------------------------
@func xform_verts
@data 9/03/98
@arg test_data *local_ptr - local test data
@arg Vertex  *out_verts - where to return the xformed verts
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
  float divisor;

  if (local_ptr->use_window_coords) {
    divisor = 1.0f;
  } else {
    divisor = 256.0f; // texture coords are 0 - 256
  }

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

    // Adjust texture coords before applying scale and offset,
    // if applicable
    if (local_ptr->do_adj_tex_coords) {
      out_verts[i].tmu[0].s = ((TEXTURE_CENTER * out_verts[i].x) + 
                                TEXTURE_CENTER)/divisor; 
      out_verts[i].tmu[0].t = ((TEXTURE_CENTER * out_verts[i].y) + 
                                TEXTURE_CENTER)/divisor; 
      out_verts[i].tmu[1].s = ((TEXTURE_CENTER * out_verts[i].x) + 
                                TEXTURE_CENTER)/divisor; 
      out_verts[i].tmu[1].t = ((TEXTURE_CENTER * out_verts[i].y) + 
                                TEXTURE_CENTER)/divisor; 
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


FxBool
gen_texture(conform_state *state, test_data *local_ptr, GrChipID_t tmu)
{
  GrTexInfo txinfo;
  FxU16 texture_data[256*256];  // maximum size possible
  FxU32 texture_size, start_address;

  txinfo.smallLodLog2 = local_ptr->texture_lod;
  txinfo.largeLodLog2 = local_ptr->texture_lod;
  txinfo.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
  txinfo.format = GR_TEXFMT_ARGB_1555;
  txinfo.data = texture_data;

  texture_size = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &txinfo);
  start_address = grTexMinAddress(GR_TMU0);

  if ((start_address + texture_size) > grTexMaxAddress(tmu)) {
    log_perror(state, "texture too big to fit in memory");
    return FXFALSE;
  }

  int x_size = local_ptr->texture_wh;
  int y_size = local_ptr->texture_wh;

  // Generate a different texture for each TMU
  if (tmu == GR_TMU0) {
    // This texture has a cyan border on the bottom/left
    // edges, and a yellow one on the top/right. There is a 
    // green 2-pixel wide cross in the center. The rest of the
    // texture is white.
    FxU16 *data_ptr = &texture_data[0];
    for (int y = 0; y < y_size; y++) {
      for (int x = 0; x < x_size; x++) {
        if (x == 0 || y == 0) {                      // cyan bottom and left edges
          *data_ptr++ = ARGB_1555(1, 0, 255, 255);
        } else if (x == x_size-1 || y == y_size-1) { // yeloow top & right
          *data_ptr++ = ARGB_1555(1, 255, 255, 0);
        } else if ((x == x_size/2) || (y == y_size/2) ||
                   (x == x_size/2-1) || (y == y_size/2-1)) {
          *data_ptr++ = ARGB_1555(1, 0, 255, 0);     // green cross in center
        } else {
          *data_ptr++ = ARGB_1555(1, 255, 255, 255); // white center
        }
      }
    }

  } else {  // GR_TMU1

    // This texture has a red border 1-pixel in on the bottom/left, and
    // a magenta border (again, 1-pixel in from the edge) on the top/right.
    // There is a 1-pixel wide blue X in the center of the texture, from
    // corner to corner. The rest of the texture is white.
    FxU16 *data_ptr = &texture_data[0];
    for (int y = 0; y < y_size; y++) {
      for (int x = 0; x < x_size; x++) {
        if (x == 1 || y == 1) {                      // red bottom and left edges
          *data_ptr++ = ARGB_1555(1, 255, 0, 0);
        } else if (x == x_size-2 || y == y_size-2) { // magenta top & right
          *data_ptr++ = ARGB_1555(1, 255, 0, 255);
        } else if ((x == y) || (x == (y_size - y))) { // blue diagonal in center
          *data_ptr++ = ARGB_1555(1, 0, 0, 255); 
        } else {
          *data_ptr++ = ARGB_1555(1, 255, 255, 255); // white center
        }
      }
    }
  }

  grTexDownloadMipMap(tmu, start_address, 
                      GR_MIPMAPLEVELMASK_BOTH, &txinfo);

  grTexSource(tmu, start_address, GR_MIPMAPLEVELMASK_BOTH, &txinfo);

  return FXTRUE;
}

void setup_prim(conform_state *state)
{
  test_data *local_ptr;

  local_ptr = (test_data *)state->local_data;

  vmgrVertexLayout(state, GR_PARAM_XY, offsetof(Vertex, x), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_A, offsetof(Vertex, tmu[0].a), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_W, offsetof(Vertex, w), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_ST0, offsetof(Vertex, tmu[0].s), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_Q0, offsetof(Vertex, tmu[0].q), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_ST1, offsetof(Vertex, tmu[1].s), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_Q1, offsetof(Vertex, tmu[1].q), GR_PARAM_ENABLE);

  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		 GR_COMBINE_FACTOR_ONE,
		 GR_COMBINE_LOCAL_NONE,  
		 GR_COMBINE_OTHER_TEXTURE,
		 FXFALSE);        
 
  // Set up to use both TMUs 
  grTexCombine(GR_TMU0,
               GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
               GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
               FXFALSE, FXFALSE ); 

  grTexCombine(GR_TMU1,
               GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
               GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
               FXFALSE, FXFALSE ); 

  grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);
  grTexClampMode(GR_TMU1, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

  grTexFilterMode(GR_TMU0,
		  GR_TEXTUREFILTER_POINT_SAMPLED,
		  GR_TEXTUREFILTER_POINT_SAMPLED );
  grTexFilterMode(GR_TMU1,
		  GR_TEXTUREFILTER_POINT_SAMPLED,
		  GR_TEXTUREFILTER_POINT_SAMPLED );

  grTexMipMapMode(GR_TMU0,
		  GR_MIPMAP_NEAREST,
		  FXFALSE );
  grTexMipMapMode(GR_TMU1,
		  GR_MIPMAP_NEAREST,
		  FXFALSE );

  grTexLodBiasValue(GR_TMU0, 0.0f);
}


/*----------------------------------------------------------------
@func init_tri_verts
@arg test_data *td - a pointer to our local_data structure (which holds
                     the vertices we'll be init'ing)
@html
 Initialize our triangle vertices, for both clip and window coords
@end
--------------------------------------------------------------------*/
void init_tri_verts(test_data *local_ptr) {
  Vertex *verts =  local_ptr->verts;
  float divisor;  
  int i;

  // Specify a vertex layout

  local_ptr->num_verts = MAX_VERTS;

  if (local_ptr->use_window_coords) {
    divisor = 1.0f;
  } else {
    divisor = 256.0f;   // texture coords are from 0 - 256
  }

  /*
     1-----3
     |\ D /|  
     | \ / | 
     |A 2 C| 
     | / \ | 
     |/ B \|
     0-----4
  */

  // Set up our verts for both clip and window coords.

  verts[0].x = -1.0f; verts[0].y = -1.0f;
  verts[0].tmu[0].s = TEXTURE_MIN/divisor; 
  verts[0].tmu[0].t = TEXTURE_MIN/divisor;
  verts[0].tmu[0].q = 1.0f;

  verts[1].x = -1.0f; verts[1].y =  1.0f;
  verts[1].tmu[0].s = TEXTURE_MIN/divisor; 
  verts[1].tmu[0].t = TEXTURE_MAX/divisor;
  verts[1].tmu[0].q = 1.0f;

  verts[2].x =  0.0f; verts[2].y =  0.0f;
  verts[2].tmu[0].s = TEXTURE_CENTER/divisor;
  verts[2].tmu[0].t = TEXTURE_CENTER/divisor;
  verts[2].tmu[0].q = 1.0f;

  verts[3].x =  1.0f; verts[3].y =  1.0f;
  verts[3].tmu[0].s = TEXTURE_MAX/divisor;
  verts[3].tmu[0].t = TEXTURE_MAX/divisor;
  verts[3].tmu[0].q = 1.0f;

  verts[4].x =  1.0f; verts[4].y = -1.0f;
  verts[4].tmu[0].s = TEXTURE_MAX/divisor; 
  verts[4].tmu[0].t = TEXTURE_MIN/divisor;
  verts[4].tmu[0].q = 1.0f;

  for (i = 0; i < local_ptr->num_verts; i++) {
    verts[i].tmu[1].s = verts[i].tmu[0].s;
    verts[i].tmu[1].t = verts[i].tmu[0].t;
    verts[i].tmu[1].q = verts[i].tmu[0].q;
  }

  if (local_ptr->use_window_coords) {
    divisor = 1.0f;
  } else {
    divisor = 255.0f; // alpha and colors are from 0 - 255
  }

  float rgb = 255.0f;
  float alpha = 255.0f;
  for (i = 0; i < local_ptr->num_verts; i++) {
    verts[i].tmu[0].a = verts[i].tmu[1].a = alpha/divisor;
    verts[i].q = 1.0f;
    verts[i].w = CLIP_COORDS_W_VALUE;
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

/**
 ** Framework entrypoints begin here
 **/

int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;
  int num_arg;
  char tmp_str[128];

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
  local_ptr->angle_incr = 0.0f;
  local_ptr->num_rotations = NUM_ROTS;
  local_ptr->rowcol = 1;
  local_ptr->num_verts = 0;
  local_ptr->scale_x = 1.0f; 
  local_ptr->scale_y = 1.0f;
  local_ptr->offset_x = 0.25f;
  local_ptr->offset_y = 0.25f;
  local_ptr->do_aa = 0; // aa off
  local_ptr->do_clip = 0; // clip windows off
  local_ptr->do_adj_tex_coords = 0; // don't adjust texture coords by default
  local_ptr->use_window_coords = 0; // use clip coords by default
  local_ptr->xform_flags = ROT_Z;
  local_ptr->cur_rep = 0;
  local_ptr->debug = 0;
  local_ptr->texture_lod = GR_LOD_LOG2_16; // default to 16x16 texture
  local_ptr->texture_wh = 16; 


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
    } else if(!_stricmp(*local_argv,"-lod") || // select a lod for the textures
              !_stricmp(*local_argv,"/lod")) {
      ++local_argv;
      ++num_arg;
      int lod;
      if(!sscanf(*local_argv,"%d",&lod)) {
        // bad argument
      } 
      switch(lod) {
        case   1: local_ptr->texture_lod = GR_LOD_LOG2_1; 
                  local_ptr->texture_wh = 1; 
                  break;
        case   2: local_ptr->texture_lod = GR_LOD_LOG2_2; 
                  local_ptr->texture_wh = 2; 
                  break;
        case   4: local_ptr->texture_lod = GR_LOD_LOG2_4; 
                  local_ptr->texture_wh = 4; 
                  break;
        case   8: local_ptr->texture_lod = GR_LOD_LOG2_8; 
                  local_ptr->texture_wh = 8;
                  break;
        case  16: local_ptr->texture_lod = GR_LOD_LOG2_16; 
                  local_ptr->texture_wh = 16; 
                  break;
        case  32: local_ptr->texture_lod = GR_LOD_LOG2_32; 
                  local_ptr->texture_wh = 32; 
                  break;
        case  64: local_ptr->texture_lod = GR_LOD_LOG2_64; 
                  local_ptr->texture_wh = 64; 
                  break;
        case 128: local_ptr->texture_lod = GR_LOD_LOG2_128; 
                  local_ptr->texture_wh = 128; 
                  break;
        case 256: local_ptr->texture_lod = GR_LOD_LOG2_256; 
                  local_ptr->texture_wh = 256;
                  break;
        default: {
          sprintf(tmp_str,"invalid lod: %d - defaulting to 16", lod);
          log_message(state, tmp_str);
          local_ptr->texture_lod = GR_LOD_LOG2_16; 
          local_ptr->texture_wh = 16;
          break;
        }
      }
      --local_argc;
    } else if(!_stricmp(*local_argv,"-aa") || // anti aliasing 
              !_stricmp(*local_argv,"/aa")) {
      local_ptr->do_aa = 1;
    } else if(!_stricmp(*local_argv,"-wc") || // use window coords
              !_stricmp(*local_argv,"/wc")) {
      local_ptr->use_window_coords = 1;
    } else if(!_stricmp(*local_argv,"-cw") || // clip window
              !_stricmp(*local_argv,"/cw")) {
      local_ptr->do_clip = 1;
    } else if(!_stricmp(*local_argv,"-atc") || // adjust texture coords
              !_stricmp(*local_argv,"/atc")) {
      local_ptr->do_adj_tex_coords = 1;
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
  // this is manditory
  sprintf(tmp_str,"%s-lod-%d%s%s%s%s", 
          test_name,
          local_ptr->texture_wh,
          (local_ptr->use_window_coords? "-wc" : ""),
          (local_ptr->do_adj_tex_coords? "-atc" : ""),
          (local_ptr->do_clip? "-cw" : ""),
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

  local_ptr->xform_flags = rot_flags[local_ptr->cur_rep / local_ptr->num_rotations];

  // First, clear the whole window
  vmgrClipWindow(state, 0, 0, state->screen_width, state->screen_height);
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grBufferClear(0, 0, 0);
  grConstantColorValue(rgbacolor(state, 0x0000ffff));

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

  Vertex *verts = local_ptr->verts;
  Vertex xformed_verts[MAX_VERTS];

  setup_prim(state);
  gen_texture(state, local_ptr, GR_TMU0);
  gen_texture(state, local_ptr, GR_TMU1);
  init_tri_verts(local_ptr); 

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

  int num_verts = local_ptr->num_verts; // filled in in init_tri_verts()


  xform_verts(local_ptr,
              xformed_verts, 
              verts, 
              num_verts,       // num verts to rotate
              local_ptr->cur_rep * local_ptr->angle_incr,
              local_ptr->scale_x,
              local_ptr->scale_y,
              local_ptr->offset_x,
              local_ptr->offset_y);

  // If we're doing a clip test, pick an arbitrary clip window
  if (local_ptr->do_clip) {
    set_clip_window(state);
  }

  // Draw our triangles.
  /*
      0-----4
      |\ D /|  
      | \ / | 
      |A 2 C| 
      | / \ | 
      |/ B \|
      1-----3
   */

  if (local_ptr->do_aa) {

    grAADrawTriangle(&xformed_verts[0], 
                     &xformed_verts[1], 
                     &xformed_verts[2],
                     FXTRUE, FXFALSE, FXFALSE);
    add_command_data(state, "grAADrawTriangle()", 6, 
                            NULL, NULL, NULL,
                            FXTRUE, FXFALSE, FXFALSE);
    add_vertex_data(state, &xformed_verts[0], 1);
    add_vertex_data(state, &xformed_verts[1], 1);
    add_vertex_data(state, &xformed_verts[2], 1);

    grAADrawTriangle(&xformed_verts[1], 
                     &xformed_verts[3], 
                     &xformed_verts[2],
                     FXTRUE, FXFALSE, FXFALSE);
    add_command_data(state, "grAADrawTriangle()", 6, 
                            NULL, NULL, NULL,
                            FXTRUE, FXFALSE, FXFALSE);
    add_vertex_data(state, &xformed_verts[1], 1);
    add_vertex_data(state, &xformed_verts[3], 1);
    add_vertex_data(state, &xformed_verts[2], 1);

    grAADrawTriangle(&xformed_verts[3], 
                     &xformed_verts[2], 
                     &xformed_verts[4],
                     FXFALSE, FXFALSE, FXTRUE);
    add_command_data(state, "grAADrawTriangle()", 6, 
                            NULL, NULL, NULL,
                            FXFALSE, FXFALSE, FXTRUE);
    add_vertex_data(state, &xformed_verts[3], 1);
    add_vertex_data(state, &xformed_verts[2], 1);
    add_vertex_data(state, &xformed_verts[4], 1);

    grAADrawTriangle(&xformed_verts[2], 
                     &xformed_verts[4], 
                     &xformed_verts[0],
                     FXFALSE, FXTRUE, FXFALSE);
    add_command_data(state, "grAADrawTriangle()", 6, 
                            NULL, NULL, NULL,
                            FXFALSE, FXTRUE, FXFALSE);
    add_vertex_data(state, &xformed_verts[2], 1);
    add_vertex_data(state, &xformed_verts[4], 1);
    add_vertex_data(state, &xformed_verts[0], 1);

  } else {

    grDrawTriangle(&xformed_verts[0], 
                   &xformed_verts[1], 
                   &xformed_verts[2]);
    add_command_data(state, "grDrawTriangle()", 0);
    add_vertex_data(state, &xformed_verts[0], 1);
    add_vertex_data(state, &xformed_verts[1], 1);
    add_vertex_data(state, &xformed_verts[2], 1);

    grDrawTriangle(&xformed_verts[1], 
                   &xformed_verts[3], 
                   &xformed_verts[2]);
    add_command_data(state, "grDrawTriangle()", 0);
    add_vertex_data(state, &xformed_verts[1], 1);
    add_vertex_data(state, &xformed_verts[3], 1);
    add_vertex_data(state, &xformed_verts[2], 1);


    grDrawTriangle(&xformed_verts[3], 
                   &xformed_verts[2], 
                   &xformed_verts[4]);
    add_command_data(state, "grDrawTriangle()", 0);
    add_vertex_data(state, &xformed_verts[3], 1);
    add_vertex_data(state, &xformed_verts[2], 1);
    add_vertex_data(state, &xformed_verts[4], 1);

    grDrawTriangle(&xformed_verts[2], 
                   &xformed_verts[4], 
                   &xformed_verts[0]);
    add_command_data(state, "grDrawTriangle()", 0);
    add_vertex_data(state, &xformed_verts[2], 1);
    add_vertex_data(state, &xformed_verts[4], 1);
    add_vertex_data(state, &xformed_verts[0], 1);
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

  local_ptr->cur_rep++; // can be different than test_pass!

  return TEST_PASS;  // ignored
}

void help_conform(conform_state *state)
{
  fprintf(stderr, "   -aa     enable anti-aliasing\n");
  fprintf(stderr, "   -wc     use window coords instead of clip coords\n");
  fprintf(stderr, "   -cw     pick a random clip window for each rotation\n");
  fprintf(stderr, "   -atc    adjust the texture coords so each rotation\n");
  fprintf(stderr, "           is a piece of the texture\n");
  fprintf(stderr, "   -nr <n> the number of rotations of each prim\n");
  fprintf(stderr, "   -lod <n> the size of the texture to use.\n");
  fprintf(stderr, "          1 - 1x1 texture\n");
  fprintf(stderr, "          2 - 2x2 texture\n");
  fprintf(stderr, "          4 - 4x4 texture\n");
  fprintf(stderr, "          8 - 8x8 texture\n");
  fprintf(stderr, "         16 - 16x16 texture (default)\n");
  fprintf(stderr, "         32 - 32x32 texture\n");
  fprintf(stderr, "         64 - 64x64 texture\n");
  fprintf(stderr, "        128 - 128x128 texture\n");
  fprintf(stderr, "        256 - 256x256 texture\n");
  fprintf(stderr, "   -debug   enable debugging mode\n");

}


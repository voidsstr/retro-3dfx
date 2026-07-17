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

static char *test_name = "simple-interp-tex";
static char *test_description = 
 "Decal and iterated RGB textures applied to rotated triangles. "
 "Rotated decal and interated textures applied to triangles."
 "All with and without AA.";

// TODO: Bump this up when self-checking is in!
#define NUM_ROTS_PER_PERM  7 // preferred number of rotations of each prim

#define SCALE_TWEAK .35f // scale down rotated prims to keep them in their 1x1 bbox

#define CLIP_COORDS_W_VALUE 3.0f // arbitrary

#define MAX_VERTS 3

#define NUM_PERMUTATIONS 8

// 1) rotated tris, decal texture applied
// 2) tris, rotated decal texture applied
// 3) rotated tris, texture * iterated  RGB applied.
// 4) tris, rotated texture * iterated  RGB applied.
// 5) rotated tris, one texture vertex having a different depth than the 
//    others
// 6) tris, rotated textures w/one texture vertex having a different depth 
//    than the others
// 7) rotated tris, texture * iterated RGB applied, with one texture vertex
//    having a different depth than  the others.
// 9) tris, rotated texture * iterated RGB applied, with one texture vertex
//    having a different depth than  the others.

typedef struct {
  Vertex verts[MAX_VERTS]; // pointer to all of our primitive verts
  int num_verts;           // number of vertices currently in use
  int cur_prim;            // the current primitive we're drawing
  int last_prim;
  int cur_rep; 
  int num_rotations;       // number of rotations of each prim per image
  float angle_incr;        // amount to incremement each rotation
  float scale_x;           // amount to scale each rotation by in x
  float scale_y;           // amount to scale each rotation by in y
  float offset_x;          // amount to offset each successive rotation in x
  float offset_y;          // amount to offset each successive rotation in y
  int use_window_coords;   // use window coords instead of clip coords
  int do_aa;               // enable anti-aliasing
  int do_bilinear_filtering; // enable bilinear filtering/mipmapping
  int flags;               // current test permutation
  float red[3];            // colors per permutation
  float green[3];
  float blue[3]; 
} test_data;

#define DO_ITER_RGB  0x01
#define DO_ROTATE_TEXTURE 0x02
#define DO_TWEAK_DEPTH 0x14
 
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
 * @param dont_normalize_colors - if true, don't divide colors by 255.0f
 */
void rot_verts_z(conform_state *state,
                 Vertex *out_verts, Vertex *in_verts, 
                 int num_verts, 
                 float z_degrees, 
                 float scale_x, 
                 float scale_y, 
                 float dx, 
                 float dy,
                 int dont_normalize_colors) {

    matrix rotm;       
    mat_make_z_rot(rotm, DEG2RAD(z_degrees));
    test_data *local_ptr = (test_data *)state->local_data;

    float color_divisor = 255.0f;

    if (dont_normalize_colors) {
      color_divisor = 1.0f;
    }

    for(int i = 0; i < num_verts; i++) {
      mat_vertex_mul(&out_verts[i], &in_verts[i], rotm);
      out_verts[i].x *= scale_x * SCALE_TWEAK;
      out_verts[i].y *= scale_y * SCALE_TWEAK;
      out_verts[i].y += dy;
      out_verts[i].x += dx; 

      out_verts[i].tmu[0].r = local_ptr->red[i]/color_divisor;
      out_verts[i].tmu[0].g = local_ptr->green[i]/color_divisor;
      out_verts[i].tmu[0].b = local_ptr->blue[i]/color_divisor; 
   }
}

/*----------------------------------------------------------------
@func rot_verts_z
@arg Vertex  *out_verts - where to return the xformed verts
@arg grVertex  *in_verts - the source verts
@arg int       num_verts - how many verts to xform
@arg float     y_degrees - the angle (in degrees) to xform to.
@arg float     scale_x - the amount to scale each vertex
@arg float     scale_y - the amount to scale each vertex
@arg float     dx      - the offsets to add to each vertex after
                         rotations and scale.
@arg float     dy      - the offsets to add to each vertex after
                         rotations and scale.
@arg int dont_normalize_colors - if true, don't normalize colors
@html
Rotate our texture vertices around the Z axis. Scale and add
an offset.
@end
-------------------------------------------------------------------*/
void rot_tex_verts_z(conform_state *state,
                     Vertex *out_verts, Vertex *in_verts, 
                     int numVerts,
                     float z_degrees, 
                     float scale_x, 
                     float scale_y, 
                     float dx, 
                     float dy,
                     int dont_normalize_colors) {
    matrix rotm;       
    mat_make_z_rot(rotm, DEG2RAD(z_degrees));
    Vertex tmp_in, tmp_out;
    test_data *local_ptr = (test_data *)state->local_data;

    float color_divisor = 255.0f;
    float half = 0.5f;
    if (dont_normalize_colors) {
      color_divisor = 1.0f;  
      half = 127.5f;
    }

    for( int i = 0; i < numVerts; i++ ) {
      out_verts[i] = in_verts[i]; // copy over Alpha values, etc
      out_verts[i].x *= scale_x * SCALE_TWEAK;
      out_verts[i].y *= scale_y * SCALE_TWEAK;
      out_verts[i].y += dy;
      out_verts[i].x += dx; 

      out_verts[i].tmu[0].r = local_ptr->red[i]/color_divisor;
      out_verts[i].tmu[0].g = local_ptr->green[i]/color_divisor;
      out_verts[i].tmu[0].b = local_ptr->blue[i]/color_divisor; 

      // normalize, rotate, and re-scale the texture coords
      tmp_in.x = (in_verts[i].tmu[0].s - half)/half;
      tmp_in.y = (in_verts[i].tmu[0].t - half)/half;
      tmp_in.z = 0.0f;

      mat_vertex_mul(&tmp_out, &tmp_in, rotm);            

      out_verts[i].tmu[0].s = tmp_out.x * half + half;
      out_verts[i].tmu[0].t = tmp_out.y * half + half;
    }
}


/*----------------------------------------------------------------
@func init_verts
@arg test_data *local_ptr - the local test data
@html
Set up our vertex layout and initialize our triangle vertices.
@end
----------------------------------------------------------------*/
void init_verts(conform_state *state) 
{
  float divisor;
  float texture_divisor;
  test_data *local_ptr = (test_data *)state->local_data;

  // Specify a vertex layout
  vmgrVertexLayout(state, GR_PARAM_XY, offsetof(Vertex, x), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_A, offsetof(Vertex, tmu[0].a), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_W, offsetof(Vertex, w), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_RGB, offsetof(Vertex, tmu[0].r),  GR_PARAM_ENABLE); 
  vmgrVertexLayout(state, GR_PARAM_ST0, offsetof(Vertex, tmu[0].s), GR_PARAM_ENABLE);
  vmgrVertexLayout(state, GR_PARAM_Q0, offsetof(Vertex, tmu[0].q), GR_PARAM_ENABLE);

  if (local_ptr->use_window_coords) {
    divisor = 1.0f;
    texture_divisor = 1.0f;
  } else {
    divisor = 255.0f;
    texture_divisor = 256.0f;
  }

  Vertex *verts =  local_ptr->verts;
  local_ptr->num_verts = 3;

  verts[0].x = -1.0f; verts[0].y = -1.0f; verts[0].q = 1.0f;
  verts[1].x =  0.0f; verts[1].y =  1.0f; verts[1].q = 1.0f;
  verts[2].x =  1.0f; verts[2].y = -1.0f; verts[2].q = 1.0f;

  verts[0].tmu[0].a = 255.0f/divisor;  verts[0].w = CLIP_COORDS_W_VALUE;
  verts[1].tmu[0].a = 10.0f/divisor;   verts[1].w = CLIP_COORDS_W_VALUE;
  verts[2].tmu[0].a = 255.0f/divisor;  verts[2].w = CLIP_COORDS_W_VALUE;

  verts[0].tmu[0].s =   0.0f/texture_divisor; 
  verts[0].tmu[0].t = 256.0f/texture_divisor;

  verts[1].tmu[0].s = 128.0f/texture_divisor; 
  verts[1].tmu[0].t =   0.0f/texture_divisor;

  verts[2].tmu[0].s = 256.0f/texture_divisor; 
  verts[2].tmu[0].t = 256.0f/texture_divisor;

  verts[0].tmu[0].q = 1.0f; 
  verts[1].tmu[0].q = 1.0f; 
  verts[2].tmu[0].q = 1.0f; 
}

/*----------------------------------------------------------------
@func set_mode
@arg conform_state *state - the framework state
@arg test_data *local_ptr - the local test data
@html
Set up our color/texture/alpha combine state depending on
flags contained in local_ptr.
@end
----------------------------------------------------------------*/
void set_mode(conform_state *state, test_data *local_ptr)
{

  if (local_ptr->flags & DO_ITER_RGB) {
    // interpolated RGB
    grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                   GR_COMBINE_FACTOR_LOCAL,
                   GR_COMBINE_LOCAL_ITERATED,
                   GR_COMBINE_OTHER_TEXTURE,
                   FXFALSE);
  } else {
    // decal
    grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                   GR_COMBINE_FACTOR_ONE,
                   GR_COMBINE_LOCAL_NONE,  
                   GR_COMBINE_OTHER_TEXTURE,
                   FXFALSE);        
  }

  grTexCombine(GR_TMU0,
               GR_COMBINE_FUNCTION_LOCAL,
               GR_COMBINE_FACTOR_NONE,
               GR_COMBINE_FUNCTION_NONE,
               GR_COMBINE_FACTOR_NONE,
               FXFALSE, FXFALSE );

  // Let the texture wrap in both directions
  grTexClampMode( GR_TMU0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

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

  grTexMipMapMode(GR_TMU0,
                  GR_MIPMAP_NEAREST,
                  FXFALSE );

  // turn on bilinear filtering, if applicable
  if (local_ptr->do_bilinear_filtering) {
    /* Enable Bilinear Filtering + Mipmapping */
    grTexFilterMode(GR_TMU0,
                    GR_TEXTUREFILTER_BILINEAR,
                    GR_TEXTUREFILTER_BILINEAR );
    log_message(state, "Enabling bilinear filtering & mipmapping");
  } else {
    grTexFilterMode(GR_TMU0,
                    GR_TEXTUREFILTER_POINT_SAMPLED,
                    GR_TEXTUREFILTER_POINT_SAMPLED );
  }
}


// Convert 8-bit a,r,g,b values into one 32-bit 1555 ARGB word
#define ARGB_1555(_a,_r,_g,_b) \
  (FxU16)((_a<<15) | ((_r & 0x1F) << 10) | ((_g & 0x1F) << 5) | (_b & 0x1F))
/*----------------------------------------------------------------
@func gen_texture
@arg test_data *local_ptr - the local test data
@html
Generate and download a single MIP level grid texture. The texture
is white, with a blue and magenta grid across it. The left and
right sides are red, and the bottom and top are green.
@end
-----------------------------------------------------------------*/
FxBool
gen_texture(test_data *local_ptr)
{
  GrTexInfo txinfo;
  FxU16 texture_data[64*64];  // maximum size possible
  FxU32 texture_size, start_address;

  txinfo.smallLodLog2 = GR_LOD_LOG2_64;
  txinfo.largeLodLog2 = GR_LOD_LOG2_64;
  txinfo.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
  txinfo.format = GR_TEXFMT_ARGB_1555;
  txinfo.data = texture_data;

  texture_size = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &txinfo);
  start_address = grTexMinAddress(GR_TMU0);

  int x_size = 64;
  int y_size = 64;

  FxU16 *data_ptr = &texture_data[0];
  for (int y = 0; y < y_size; y++) {
    for (int x = 0; x < x_size; x++) {
      if (x == 0 || x == x_size-1) { // left and right sides, red
        *data_ptr++ = ARGB_1555(1, 255, 0, 0);
      } else if (y == 0 || y == y_size-1) { // top & bottom, green 
        *data_ptr++ = ARGB_1555(1, 0, 255, 0);
      } else if (!(x % 8)) {
        *data_ptr++ = ARGB_1555(1, 0, 0, 255); 
      } else if (!(y % 8)) {
        *data_ptr++ = ARGB_1555(1, 255, 0, 255); 
      } else {
        *data_ptr++ = ARGB_1555(1, 255, 255, 255); 
      }
    }
  }

  if ((start_address + texture_size) > grTexMaxAddress(GR_TMU0)) {
    printf("texture too big to fit in memory");
    return FXFALSE;
  }

  grTexDownloadMipMap(GR_TMU0, start_address, 
                      GR_MIPMAPLEVELMASK_BOTH, &txinfo);

  grTexSource(GR_TMU0, start_address, GR_MIPMAPLEVELMASK_BOTH, &txinfo);

  return FXTRUE;
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
  local_ptr->cur_rep = 0;
  local_ptr->angle_incr = 0.0f;
  local_ptr->num_rotations = NUM_ROTS_PER_PERM;
  local_ptr->cur_prim = 0;
  local_ptr->cur_rep = 0;
  local_ptr->last_prim = -1;
  local_ptr->num_verts = 0;
  local_ptr->scale_x = 1.0f; 
  local_ptr->scale_y = 1.0f;
  local_ptr->offset_x = 0.25f;
  local_ptr->offset_y = 0.25f;
  local_ptr->use_window_coords = 0; // use clip coords by default
  local_ptr->do_aa = 0; // aa off
  local_ptr->do_bilinear_filtering = 0;
  local_ptr->flags = 0;

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
    } else if(!_stricmp(*local_argv,"-bf") || // use bilinear filtering & mipmapping
              !_stricmp(*local_argv,"/bf")) {
      local_ptr->do_bilinear_filtering = 1;
    } else {
      fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
      ++local_argv;
      ++num_arg;
    }
    ++local_argv;
    ++num_arg;
  }

  // set a small conform window, if none was requested
  if ((state->conform_width == -1) &&
      (state->conform_height == -1)) {
    state->conform_width = 50;
    state->conform_height = 50;
  }

  // don't use any visuals with aux buffers
  state->num_aux_bufs = 0;
  
  // we need at least one color buffer
  if (state->num_color_bufs < 1) {
    state->num_color_bufs = 1;
  }
 
  // default state is cycle through all of our prims
  if (state->num_reps == 1) {
      state->num_reps = NUM_PERMUTATIONS * local_ptr->num_rotations;
  }

  // compute the amount we'll rotate successive frames. 
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

  return(-1); }

void close_conform(conform_state *state)
{
  // Free up local data
  free(state->local_data);
}

int do_conform(conform_state *state)
{
  test_data *local_ptr;
  FxFloat    vnear = 0.0f, vfar = 1.0f;
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

  init_verts(state);

  if (!gen_texture(local_ptr)) {
    log_perror(state, "Couldn't generate texture!");
    return 0;
  }

  // Choose a color once per permutation
  if (local_ptr->last_prim != local_ptr->cur_prim) {
    for (int i = 0; i < 3; i++) {
      local_ptr->red[i]   = (float)nrand(255);
      local_ptr->green[i] = (float)nrand(255);
      local_ptr->blue[i]  = (float)nrand(255);
    }
    local_ptr->last_prim = local_ptr->cur_prim;
  }

  local_ptr->flags = 0; // reset flags
  switch (local_ptr->cur_prim) {
    case 0:
      // decal texture
      // rotated tris
      // constant depth
      break;
    case 1:
      // decal texture
      // rotated textures
      // constant depth
      local_ptr->flags = DO_ROTATE_TEXTURE;
      break;
    case 2:
      // iterated RGB * texture
      // rotated tris
      // constant depth
      local_ptr->flags = DO_ITER_RGB;
      break;
    case 3:
      // iterated RGB * texture
      // rotated textures
      // constant depth
      local_ptr->flags = DO_ITER_RGB | DO_ROTATE_TEXTURE;
      break;
    case 4:
      // decal texture
      // rotated tris
      // texture changes depth
      local_ptr->flags = DO_TWEAK_DEPTH;
      break;
    case 5:
      // decal texture
      // rotated textures
      // texture changes depth
      local_ptr->flags = DO_ROTATE_TEXTURE | DO_TWEAK_DEPTH;
      break;
    case 6:
      // iterated RGB * texture
      // rotated tris
      // texture changes depth
      local_ptr->flags = DO_ITER_RGB | DO_TWEAK_DEPTH;
      break;
    case 7:
      // iterated RGB * texture
      // rotated textures
      // texture changes depth
      local_ptr->flags = DO_ITER_RGB | DO_ROTATE_TEXTURE | DO_TWEAK_DEPTH;
      break;
    default: 
      break;
  }

  // Set the texture/RGB/depth/alpha/bilinear filtering combo for this test.
  set_mode(state, local_ptr);

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


  Vertex *verts = local_ptr->verts;
  Vertex xformed_verts[MAX_VERTS];

  int num_verts = local_ptr->num_verts; // filled in in init_X_verts()

  if (local_ptr->flags & DO_ROTATE_TEXTURE) {
    // leave the prims, rotate the textures
    rot_tex_verts_z(state,
                    xformed_verts, 
                    verts, 
                    num_verts,       // num verts to rotate
                    local_ptr->cur_rep * local_ptr->angle_incr,
                    local_ptr->scale_x,
                    local_ptr->scale_y,
                    local_ptr->offset_x,
                    local_ptr->offset_y,
                    local_ptr->use_window_coords); 
  } else {
    // leave the textures, rotate the prims
    rot_verts_z(state,
                xformed_verts, 
                verts, 
                num_verts,       // num verts to rotate
                local_ptr->cur_rep * local_ptr->angle_incr,
                local_ptr->scale_x,
                local_ptr->scale_y,
                local_ptr->offset_x,
                local_ptr->offset_y,
                local_ptr->use_window_coords);
  }

  if (local_ptr->flags & DO_TWEAK_DEPTH) {
    xformed_verts[1].tmu[0].s *= 0.2f;
    xformed_verts[1].tmu[0].t *= 0.2f;
    xformed_verts[1].tmu[0].q *= 0.2f;
  }
      
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
  add_vertex_data(state, xformed_verts, 3);

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
  fprintf(stderr,"   -bf     use bilinear filtering and mipmapping\n");
  fprintf(stderr,"   -nr <n> the number of rotations of each prim\n");
}


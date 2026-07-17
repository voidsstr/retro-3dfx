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
#include "vmglide.h"

#define ARGB_1555(_a,_r,_g,_b) \
  (FxU16)((_a<<15) | ((_r & 0x1F) << 10) | ((_g & 0x1F) << 5) | (_b & 0x1F))

static char *test_name = "query-state";
static char *test_description =
 "Draw with initial state, check that grGlideGetState()/grGlideSetState() work.";

// Data specific to this test.
typedef struct {
  FxU32 *screen;
  FxU32 checksum[5]; // one for each screen
} test_data;

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

void
init_verts(conform_state *state, GrVertex *verts1, GrVertex *verts2) 
{
  long depthrange[2];
  float max_height = (float)state->conform_height;
  float max_width = (float)state->conform_width;
 
  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE); 
  grVertexLayout(GR_PARAM_A,  GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE); 
  grVertexLayout(GR_PARAM_Q , GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_Z , GR_VERTEX_OOZ_OFFSET << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_RGB , GR_VERTEX_R_OFFSET << 2, GR_PARAM_ENABLE);

  grVertexLayout(GR_PARAM_ST0, GR_VERTEX_SOW_TMU0_OFFSET << 2, GR_PARAM_ENABLE);

  // get depth range
  grGet(GR_ZDEPTH_MIN_MAX, 8, depthrange);

  // Init vertex data
  float zbig = (float)depthrange[0];
  float zsmall = (float)depthrange[1];

  /* 
     3 ------ 2
     |        |
     |        | 
     0 ------ 1
   */

  verts1[0].x = 0.0f;                         
  verts1[0].y = 0.0f;
  verts1[0].oow = 1.0f;
  verts1[0].a = 255.0f;
  verts1[0].tmuvtx[0].sow = 0.0f;
  verts1[0].tmuvtx[0].tow = 0.0f;
  verts1[0].r = 0.0f;
  verts1[0].g = 0.0f;
  verts1[0].b = 255.0f;
  verts1[0].ooz = zbig;


  verts1[1].x = max_width;
  verts1[1].y = 0.0f;
  verts1[1].oow = 1.0f;
  verts1[1].a = 64.0f;
  verts1[1].tmuvtx[0].sow = 512.0f;
  verts1[1].tmuvtx[0].tow = 0.0f;
  verts1[1].r = 0.0f;
  verts1[1].g = 255.0f;
  verts1[1].b = 0.0f;
  verts1[1].ooz = zsmall;

  verts1[2].x = max_width;
  verts1[2].y = max_height;
  verts1[2].oow = 1.0f;
  verts1[2].a = 255.0f;
  verts1[2].tmuvtx[0].sow = 512.0f;
  verts1[2].tmuvtx[0].tow = 512.0f;
  verts1[2].r = 255.0f;
  verts1[2].g = 0.0f;
  verts1[2].b = 0.0f;
  verts1[2].ooz = zsmall;

  verts1[3].x = 0.0f;
  verts1[3].y = max_height;
  verts1[3].oow = 1.0f;
  verts1[3].a = 255.0f;
  verts1[3].tmuvtx[0].sow = 0.0f;
  verts1[3].tmuvtx[0].tow = 512.0f;
  verts1[3].r = 255.0f;
  verts1[3].g = 255.0f;
  verts1[3].b = 0.0f;
  verts1[3].ooz = zbig;

  // contrasting triangle

  verts2[0].x = 0.0f;
  verts2[0].y = max_height/2.0f + max_height/8.0f;
  verts2[0].oow = 1.0f;
  verts2[0].a = 255.0f;
  verts2[0].tmuvtx[0].sow = 0.0f;
  verts2[0].tmuvtx[0].tow = 0.0f;
  verts2[0].r = 0.0f;
  verts2[0].g = 255.0f;
  verts2[0].b = 255.0f;
  verts2[0].ooz = zsmall;

  verts2[1].x = max_width/2.0f;
  verts2[1].y = verts2[0].y + max_height/8.0f;
  verts2[1].oow = 1.0f;
  verts2[1].a = 255.0f;
  verts2[1].tmuvtx[0].sow = 128.0f;
  verts2[1].tmuvtx[0].tow = 128.0f;
  verts2[1].r = 0.0f;
  verts2[1].g = 255.0f;
  verts2[1].b = 255.0f;
  verts2[1].ooz = zbig/2.0f;
  
  verts2[2].x = max_width;
  verts2[2].y = verts2[0].y;
  verts2[2].oow = 1.0f;
  verts2[2].a = 255.0f;
  verts2[2].tmuvtx[0].sow = 256.0f;
  verts2[2].tmuvtx[0].tow = 0.0f;
  verts2[2].r = 0.0f;
  verts2[2].g = 255.0f;
  verts2[2].b = 255.0f;
  verts2[2].ooz = zbig;
}

void *
save_state(conform_state *state)
{
  FxI32 size;
  void *glide_state = 0;
  
  grGet(GR_GLIDE_STATE_SIZE,  sizeof(size), &size);
  glide_state = malloc(size);
  if (!glide_state) {
    log_perror(state, "Error - Couldn't malloc space for state!");
    fprintf(stderr, "Error - Couldn't malloc space for state!\n");
    return 0;
  }
 
  grGlideGetState(glide_state);

  return glide_state;
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

  // print out test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
    ++local_argv;
    ++num_arg;
  }

  state->tile = 0;
  state->num_reps = 1;

  if (state->num_color_bufs < 1) {
    state->num_color_bufs = 1; // need at least 1 color buf
  }

  if (state->num_aux_bufs < 1) {
    state->num_aux_bufs = 1; // need at least 1 color buf
  }

  // use 200x200 image area
  state->conform_width = 200;
  state->conform_height = 200;

  // xycheck/xylimit not supported.
  state->xycheck = 0;
  state->xylimit = 0;

  // setup image file names
  // this is manditory
  char tmp_str[128];
  sprintf(tmp_str,"%s",test_name);

  frame_name(tmp_str,state);
  img_file_type(IMG_P6,state);

  return(-1);
}

void close_conform(conform_state *state)
{
  // Free up local data
  test_data *local_ptr = (test_data*)state->local_data;
  if (local_ptr) {
    if (local_ptr->screen) {
      free(local_ptr->screen);
    }
    free(local_ptr);
  }
}

int do_conform(conform_state *state)
{
  FxU32 size = 0;
  GrVertex verts1[4];
  GrVertex verts2[3];

  test_data *local_ptr = (test_data*)state->local_data;

  local_ptr = (test_data *)state->local_data;

  // allocate screens for automated compare
  if(local_ptr->screen == NULL ) {
      if((local_ptr->screen = 
          (FxU32 *)malloc(state->screen_width*state->screen_height*sizeof(FxU32))) == NULL) {
        // fail
        log_status(state, TEST_FAIL, "Couldn't allocate memory for screen image");
        return TEST_FAIL;
      }
  }

  grClipWindow(state->conform_x,    
               state->conform_y, 
               state->conform_x + state->conform_width, 
               state->conform_y + state->conform_height);

  init_verts(state, verts1, verts2);

  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grBufferClear(0, 0, 0); 

  grDitherMode(GR_DITHER_DISABLE); // turn dithering OFF

  grConstantColorValue(rgbacolor(state, 0x00FF0088));

  grDrawTriangle(&verts1[0], &verts1[1], &verts1[2]);
  grDrawTriangle(&verts1[0], &verts1[2], &verts1[3]);
  grDrawTriangle(&verts2[0], &verts2[1], &verts2[2]);

  // Save an image drawn in the default state.
  end_frame(END_OP_SAVE_TO_MEM, state, (void**)&local_ptr->screen, 
            END_BUFFER_FRONTBUFFER);
  local_ptr->checksum[0] = 
     compute_checksum((FxU8*)local_ptr->screen,
                      state->screen_width * state->screen_height * sizeof(FxU32));
  

  void *state1 = save_state(state); // save the current state

  /** Now muck with the state. **/

  // Generate and download the texture
  gen_texture(local_ptr);

  grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

  grTexFilterMode(GR_TMU0,
                  GR_TEXTUREFILTER_POINT_SAMPLED,
                  GR_TEXTUREFILTER_POINT_SAMPLED );

  grTexMipMapMode(GR_TMU0,
                  GR_MIPMAP_NEAREST,
                  FXFALSE );

  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                 GR_COMBINE_FACTOR_LOCAL,
                 GR_COMBINE_LOCAL_ITERATED,
                 GR_COMBINE_OTHER_TEXTURE, 
                 FXFALSE);

  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_ITERATED,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );

  grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                       GR_BLEND_ZERO, GR_BLEND_ZERO );

  grEnable(GR_AA_ORDERED);

  grTexCombine(GR_TMU0,
               GR_COMBINE_FUNCTION_LOCAL,
               GR_COMBINE_FACTOR_NONE,
               GR_COMBINE_FUNCTION_NONE,
               GR_COMBINE_FACTOR_NONE,
               FXFALSE, FXFALSE );

  grDepthMask(FXTRUE);
  grBufferClear(0, 0, 0);
  grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
  grDepthBufferFunction(GR_CMP_GREATER);

  grDrawTriangle(&verts1[0], &verts1[1], &verts1[2]);
  grDrawTriangle(&verts1[0], &verts1[2], &verts1[3]);
  grDrawTriangle(&verts2[0], &verts2[1], &verts2[2]);
   

  void *state2 = save_state(state); // save the current state
  end_frame(END_OP_SAVE_TO_MEM, state, (void**)&local_ptr->screen, 
            END_BUFFER_FRONTBUFFER);
  local_ptr->checksum[1] = 
     compute_checksum((FxU8*)local_ptr->screen,
                      state->screen_width * state->screen_height * sizeof(FxU32));

  // Muck with the state some more and draw again.
  grBufferClear(0, 0, 0);
  grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
  grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_CLAMP);
  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_CONSTANT,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );

  grDrawTriangle(&verts1[0], &verts1[1], &verts1[2]);
  grDrawTriangle(&verts1[0], &verts1[2], &verts1[3]);
  grDrawTriangle(&verts2[0], &verts2[1], &verts2[2]);

  end_frame(END_OP_SAVE_TO_MEM, state, (void**)&local_ptr->screen, 
            END_BUFFER_FRONTBUFFER);
  local_ptr->checksum[2] = 
     compute_checksum((FxU8*)local_ptr->screen,
                      state->screen_width * state->screen_height * sizeof(FxU32));


  // Reset the initial state and redraw
  grBufferClear(0, 0, 0);
  grGlideSetState(state1);
  grDrawTriangle(&verts1[0], &verts1[1], &verts1[2]);
  grDrawTriangle(&verts1[0], &verts1[2], &verts1[3]);
  grDrawTriangle(&verts2[0], &verts2[1], &verts2[2]);

  end_frame(END_OP_SAVE_TO_MEM, state, (void**)&local_ptr->screen, 
            END_BUFFER_FRONTBUFFER);
  local_ptr->checksum[3] = 
     compute_checksum((FxU8*)local_ptr->screen,
                      state->screen_width * state->screen_height * sizeof(FxU32));

  // Reset the first mucked with state and redraw
  grGlideSetState(state2);
  grDepthMask(FXTRUE);
  grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
  grDepthBufferFunction(GR_CMP_GREATER);
  grBufferClear(0, 0, 0);
  grDrawTriangle(&verts1[0], &verts1[1], &verts1[2]);
  grDrawTriangle(&verts1[0], &verts1[2], &verts1[3]);
  grDrawTriangle(&verts2[0], &verts2[1], &verts2[2]);

  end_frame(END_OP_SAVE_TO_MEM, state, (void**)&local_ptr->screen, 
            END_BUFFER_FRONTBUFFER);
  local_ptr->checksum[4] = 
     compute_checksum((FxU8*)local_ptr->screen,
                      state->screen_width * state->screen_height * sizeof(FxU32));

  // Now we have a checksum for all 5 screens. Make sure that 0 & 3, and 
  // 1 & 4 match.

  if (local_ptr->checksum[0] != local_ptr->checksum[3]) {
    log_status(state, TEST_FAIL, 
               "restored state didn't match initial default state");
  } else {
    log_status(state, TEST_PASS, 
               "restored state matched initial default state");
  }

  if (local_ptr->checksum[1] != local_ptr->checksum[4]) {
    log_status(state, TEST_FAIL, 
               "restored state didn't match modified state");
  } else {
    log_status(state, TEST_PASS, 
               "restored state matched modified state");
  }
  
  // make sure that the ones that are supposed to be different, are.
  if ((local_ptr->checksum[0] == local_ptr->checksum[1]) ||
      (local_ptr->checksum[0] == local_ptr->checksum[2])) {
    log_status(state, TEST_FAIL, 
               "states that should not have matched, did");
  } else {
    log_status(state, TEST_PASS, 
               "states that should not have matched, didn't.");
  }

  return(TEST_PASS);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"    No test specific args\n");
}


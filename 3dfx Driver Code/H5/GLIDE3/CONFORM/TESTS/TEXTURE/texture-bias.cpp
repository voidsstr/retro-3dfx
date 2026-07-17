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

#include "conform.h"
#include "vertex.h"

static char *test_name = "texture-bias";
static char *test_description = "Test MIP mapped texture LOD bias";

// local data
typedef struct {
  int cur_rep;
  float cur_bias;
  float bias_delta;
  GrVertex verts[6]; // two triangles
  GrTexInfo info;
  GrChipID_t curtmu;
} test_data;

// Prototypes
void init_verts(test_data *local_ptr);
void setup_prim(conform_state *state);

int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;
  int num_tmu;

  state->test_name = test_name;
  state->test_description = test_description;

  if((state->local_data = malloc(sizeof(test_data))) == NULL) {
    // Could not allocate data for local memory
    log_perror(state,"Could not allocate memory for local data");
    return(0);
  } 

  local_ptr = (test_data *)state->local_data;
  local_ptr->cur_rep = 0;
  local_ptr->cur_bias = 0.0f;

  // don't use any visuals with aux buffers
  state->num_aux_bufs = 0;

  // use 300x300 image area
  state->conform_width = 300;
  state->conform_height = 300;

  // tiling is not supported
  state->tile = 0;

  // xycheck and xylimit not supported (fix this when adding self-checking)
  state->xycheck = 0;
  state->xylimit = 0;

  // default to 10 reps
  if(state->num_reps == 1) {
    state->num_reps = 10;
  }

  local_ptr->bias_delta = 2.0f/((float)state->num_reps);

  // pick a random TMU
  num_tmu = nrand((int)(state->num_tmu));

  // process test dependent args
  ++local_argv; // skip test name
  while(--local_argc) {
    if(!_stricmp(*local_argv,"-bias") || // use window coords
       !_stricmp(*local_argv,"/bias")) {
      
      // force single rep
      state->num_reps = 1;

      ++local_argv;
      if(!sscanf(*local_argv,"%f",&(local_ptr->cur_bias))) {
        // bad argument - ignore
      } 
      --local_argc;
    } else if(!_stricmp(*local_argv,"-tmu") || // TMU to use
       !_stricmp(*local_argv,"/tmu")) {
      ++local_argv;
      if(!sscanf(*local_argv,"%d",&num_tmu)) {
        // bad argument
      } 
      --local_argc;
    } else {
      fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
      ++local_argv;
    }
    ++local_argv;
  }

  // Convert numeric TMU to argument for glide
  if(num_tmu >= state->num_tmu) {
    num_tmu = 0;
  }
  switch(num_tmu) {
  case 1:
    local_ptr->curtmu = GR_TMU1;
    break;
  case 2:
    local_ptr->curtmu = GR_TMU2;
    break;
  default:
    local_ptr->curtmu = GR_TMU0;
    break;
  }

  // setup image file names
  // this is manditory
  frame_name("texture-bias",state);
  img_file_type(IMG_P6,state);

  // Init vertex data
  init_verts(local_ptr);

  // Initialization complete
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
  GrColor_t clearcolor;

  local_ptr = (test_data *)state->local_data;

  // write record of TMU to loog file
  switch(local_ptr->curtmu) {
  case 1:
    log_message(state,"Using TMU 1");
    break;
  case 2:
    log_message(state,"Using TMU 2");
    break;
  default:
    log_message(state,"Using TMU 0");
    break;
  }

  // clear front buffer to black
  clearcolor = rgbacolor(state, 0x00000000);
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grColorMask(FXTRUE,FXFALSE);
  grDepthMask(FXFALSE);
  grCoordinateSpace(GR_WINDOW_COORDS);
  grClipWindow(state->conform_x,
	       state->conform_y,
	       state->conform_x+state->conform_width,
	       state->conform_y+state->conform_height);

  grBufferClear(clearcolor,0x0000,0x00000000);
	       
  grViewport(state->conform_x, state->conform_y, 
             state->conform_width, state->conform_height);
  
  if(local_ptr->cur_rep == 0) {
    setup_prim(state);
  }

  grTexLodBiasValue(local_ptr->curtmu, local_ptr->cur_bias);
  
  grDrawTriangle(&(local_ptr->verts[0]), 
		 &(local_ptr->verts[1]), 
		 &(local_ptr->verts[2]));

  grDrawTriangle(&(local_ptr->verts[3]), 
		 &(local_ptr->verts[4]), 
		 &(local_ptr->verts[5]));

  end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);

  local_ptr->cur_bias += local_ptr->bias_delta;

  local_ptr->cur_rep++;

  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -bias <n>  force floating point bias value\n");
  fprintf(stderr,"   -tmu <n>   texture unit to test\n");
}

void init_verts(test_data *local_ptr)
{
  int i;

  // initialize vertex structure
  // Triangle # 1
  local_ptr->verts[0].x = 22.0f;
  local_ptr->verts[0].y = 22.0f; 
  local_ptr->verts[0].oow = 1.0f/256.0f;  

  local_ptr->verts[1].x = 256.0f + 22.0f;
  local_ptr->verts[1].y = 22.0f; 
  local_ptr->verts[1].oow = 1.0f;  

  local_ptr->verts[2].x = 256.0f + 22.0f;
  local_ptr->verts[2].y = 128.0f + 22.0f; 
  local_ptr->verts[2].oow = 1.0f;  

  // triangle #2
  local_ptr->verts[3].x = 22.0f;
  local_ptr->verts[3].y = 22.0f; 
  local_ptr->verts[3].oow = 1.0f/256.0f;  

  local_ptr->verts[4].x = 22.0f;
  local_ptr->verts[4].y = 256.0f + 22.0f; 
  local_ptr->verts[4].oow = 1.0f;  

  local_ptr->verts[5].x = 128.0f + 22.0f;
  local_ptr->verts[5].y = 256.0f + 22.0f; 
  local_ptr->verts[5].oow = 1.0f;  

  for(i=0;i<GRVERTEX_NUM_TMU; i++) {
    local_ptr->verts[0].tmuvtx[i].sow = 0.0f;
    local_ptr->verts[0].tmuvtx[i].tow = 0.0f;
    local_ptr->verts[0].tmuvtx[i].oow = 1.0f/256.0f;

    local_ptr->verts[1].tmuvtx[i].sow = 0.0f;
    local_ptr->verts[1].tmuvtx[i].tow = 240.0f;
    local_ptr->verts[1].tmuvtx[i].oow = 1.0f;

    local_ptr->verts[2].tmuvtx[i].sow = 240.0f;
    local_ptr->verts[2].tmuvtx[i].tow = 240.0f;
    local_ptr->verts[2].tmuvtx[i].oow = 1.0f;

    local_ptr->verts[3].tmuvtx[i].sow = 0.0f;
    local_ptr->verts[3].tmuvtx[i].tow = 0.0f;
    local_ptr->verts[3].tmuvtx[i].oow = 1.0f/256.0f;

    local_ptr->verts[4].tmuvtx[i].sow = 0.0f;
    local_ptr->verts[4].tmuvtx[i].tow = 240.0f;
    local_ptr->verts[4].tmuvtx[i].oow = 1.0f;

    local_ptr->verts[5].tmuvtx[i].sow = 240.0f;
    local_ptr->verts[5].tmuvtx[i].tow = 240.0f;
    local_ptr->verts[5].tmuvtx[i].oow = 1.0f;
  }
}

// MIP level colors
FxU16 colors[9] = {
  0x801f, // blue      (1x1)
  0x83e0, // green     (2x2)
  0xfc00, // red       (4x4)

  0x800f, // 1/2 blue  (8x8)
  0x81e0, // 1/2 green (16x16)
  0xbc00, // 1/2 red   (32x32)

  0x8007, // 1/4 blue  (64x64)
  0x80e0, // 1/4 green (128x128)
  0x9c00  // 1/4 red   (256x256)
};

void setup_prim(conform_state *state)
{
  test_data *local_ptr;
  GrChipID_t passtmu;
  FxU16 *buffer;
  FxU16 *tmp;
  long  i,j;

  local_ptr = (test_data *)state->local_data;

  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_A,  GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_Q , GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
  // set for max 3 TMUs for now
  grVertexLayout(GR_PARAM_ST0, GR_VERTEX_SOW_TMU0_OFFSET << 2,
		 GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_Q0, GR_VERTEX_OOW_TMU0_OFFSET << 2,
		 GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_ST1, GR_VERTEX_SOW_TMU1_OFFSET << 2,
		 GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_Q1, GR_VERTEX_OOW_TMU1_OFFSET << 2,
		 GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_ST2, GR_VERTEX_SOW_TMU2_OFFSET << 2,
		 GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_Q2, GR_VERTEX_OOW_TMU2_OFFSET << 2,
		 GR_PARAM_ENABLE);

  if((buffer = (FxU16 *)malloc(256*256*2*2)) == NULL) {
    // fail
    log_message(state,"Could not allocate memory buffer for MIP map");
  }

  // fill buffer with different color for each MIP level
  tmp = buffer;
  for(i=8; i>=0; i--) {
    for(j=0; j < (1<<(i*2)) ; j++) {
      *tmp++ = colors[i];
    }
  }

  local_ptr->info.smallLodLog2 = GR_LOD_LOG2_1;
  local_ptr->info.largeLodLog2 = GR_LOD_LOG2_256;
  local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
  local_ptr->info.format = GR_TEXFMT_ARGB_1555;

  local_ptr->info.data = buffer;

  grTexDownloadMipMap(local_ptr->curtmu,
		      grTexMinAddress(local_ptr->curtmu),
		      GR_MIPMAPLEVELMASK_BOTH,
		      &(local_ptr->info));
  
  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		 GR_COMBINE_FACTOR_ONE,
		 GR_COMBINE_LOCAL_NONE,  
		 GR_COMBINE_OTHER_TEXTURE,
		 FXFALSE);        
  
  // Setup other TMUs to pass thru
  for(i = 0; i< local_ptr->curtmu; i++) {
    switch(i) {
    case 1:
      passtmu = GR_TMU1;
      break;
    case 2:
      passtmu = GR_TMU2;
      break;
    default:
      passtmu = GR_TMU0;
      break;
    }
    grTexCombine(passtmu,
		 GR_COMBINE_FUNCTION_SCALE_OTHER,
		 GR_COMBINE_FACTOR_ONE,
		 GR_COMBINE_FUNCTION_SCALE_OTHER,
		 GR_COMBINE_FACTOR_ONE,
		 FXFALSE, FXFALSE );
  }

  grTexCombine(local_ptr->curtmu,
	       GR_COMBINE_FUNCTION_LOCAL,
	       GR_COMBINE_FACTOR_NONE,
	       GR_COMBINE_FUNCTION_NONE,
	       GR_COMBINE_FACTOR_NONE,
	       FXFALSE, FXFALSE );
  
  grTexSource(local_ptr->curtmu,
	      grTexMinAddress(local_ptr->curtmu),
	      GR_MIPMAPLEVELMASK_BOTH,
	      &(local_ptr->info));

  grTexClampMode(local_ptr->curtmu, 
		 GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

  grTexFilterMode(local_ptr->curtmu,
		  GR_TEXTUREFILTER_POINT_SAMPLED,
		  GR_TEXTUREFILTER_POINT_SAMPLED );

  grTexMipMapMode(local_ptr->curtmu,
		  GR_MIPMAP_NEAREST,
		  FXFALSE );

  free(buffer);
}


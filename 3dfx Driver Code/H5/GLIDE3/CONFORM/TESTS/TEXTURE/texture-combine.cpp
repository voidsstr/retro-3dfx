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

static char *test_name = "texture-combine";
static char *test_description = "Test texture combine functionality";

// local data
typedef struct {
  GrVertex verts[4];  // Temporary vertex storage
  GrTexInfo info;
  GrChipID_t curtmu;
} test_data;

// Prototypes
void init_verts(test_data *local_ptr, int vertindx);
void setup_prim(conform_state *state);
void texture_gen(int format, FxU16 *buffer, test_data *local_ptr);

#define NUM_COMBO 30

struct {
  GrCombineFunction_t function;
  GrCombineFactor_t factor;
  FxBool invert;
} combine[NUM_COMBO] = {
  GR_COMBINE_FUNCTION_LOCAL,
  GR_COMBINE_FACTOR_NONE,
  FXFALSE,

  GR_COMBINE_FUNCTION_LOCAL,
  GR_COMBINE_FACTOR_NONE,
  FXTRUE,

  GR_COMBINE_FUNCTION_ZERO,
  GR_COMBINE_FACTOR_NONE,
  FXFALSE,

  GR_COMBINE_FUNCTION_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_NONE,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL,
  GR_COMBINE_FACTOR_ZERO,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL,
  GR_COMBINE_FACTOR_LOCAL,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL,
  GR_COMBINE_FACTOR_LOCAL_ALPHA,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL,
  GR_COMBINE_FACTOR_ONE,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL,
  GR_COMBINE_FACTOR_ONE_MINUS_LOCAL, 
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL,
  GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA, 
  FXFALSE,

  GR_COMBINE_FUNCTION_BLEND_LOCAL,
  GR_COMBINE_FACTOR_ZERO,
  FXFALSE,

  GR_COMBINE_FUNCTION_BLEND_LOCAL,
  GR_COMBINE_FACTOR_LOCAL,
  FXFALSE,

  GR_COMBINE_FUNCTION_BLEND_LOCAL,
  GR_COMBINE_FACTOR_LOCAL_ALPHA,
  FXFALSE,

  GR_COMBINE_FUNCTION_BLEND_LOCAL,
  GR_COMBINE_FACTOR_ONE,
  FXFALSE,

  GR_COMBINE_FUNCTION_BLEND_LOCAL,
  GR_COMBINE_FACTOR_ONE_MINUS_LOCAL, 
  FXFALSE,

  GR_COMBINE_FUNCTION_BLEND_LOCAL,
  GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA, 
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_ZERO,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_LOCAL,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_LOCAL_ALPHA,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_ONE,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_ONE_MINUS_LOCAL, 
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA, 
  FXFALSE,
      
  GR_COMBINE_FUNCTION_SCALE_OTHER,
  GR_COMBINE_FACTOR_ZERO,
  FXFALSE,

  GR_COMBINE_FUNCTION_BLEND_OTHER,
  GR_COMBINE_FACTOR_ZERO,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
  GR_COMBINE_FACTOR_ZERO,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_ZERO,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
  GR_COMBINE_FACTOR_ZERO,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
  GR_COMBINE_FACTOR_ZERO, 
  FXFALSE,

  GR_COMBINE_FUNCTION_BLEND,
  GR_COMBINE_FACTOR_ZERO,
  FXFALSE,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_ZERO, 
  FXFALSE,
  
};

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

  // pick a random TMU
  num_tmu = nrand((int)(state->num_tmu));

  // print out test dependent args
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    if(!_stricmp(*local_argv,"-tmu") || // TMU to use
       !_stricmp(*local_argv,"/tmu")) {
      ++local_argv;
      if(!sscanf(*local_argv,"%d",&num_tmu)) {
        // bad argument
      } 
      --local_argc;
    } else {
      fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
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

  // don't use any visuals with aux buffers
  state->num_aux_bufs = 0;

  // use 50x50 image area
  state->conform_width = 50;
  state->conform_height = 50;

  // tiling not supported
  state->tile = 0;

  // xycheck and xylimit not supported (fix this when adding self-checking)
  state->xycheck = 0;
  state->xylimit = 0;

  // setup image file names
  // this is manditory
  frame_name("texture-combine",state);
  img_file_type(IMG_P6,state);

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
  int num_tmu;
  int alpha;
  int format;
  FxU16 *buffer;
  FxU32 baseaddr;

  local_ptr = (test_data *)state->local_data;

  // if multiple reps - randomize
  if(state->num_reps != 1) {
    // randomize TMU
    num_tmu = nrand(state->num_tmu);
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
  }

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

  // clear front buffer to grey
  clearcolor = rgbacolor(state,0x7f7f7fff);
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grColorMask(FXTRUE,FXFALSE);
  grDepthMask(FXFALSE);
  grCoordinateSpace(GR_WINDOW_COORDS);
  grClipWindow(state->conform_x,
	       state->conform_y,
	       state->conform_x+state->conform_width,
	       state->conform_y+state->conform_height);

  grViewport(state->conform_x, state->conform_y, 
             state->conform_width, state->conform_height);

  setup_prim(state);

  if((buffer = (FxU16 *)malloc(32*32*2)) == NULL) {
    // fail
    log_message(state,"Could not allocate memory buffer for texture map");
  }

  for(alpha = 0; alpha < 2; alpha++) {
    for(format = 0; format < NUM_COMBO; format++) {
      grBufferClear(clearcolor,0x0000,0x00000000);
      
      local_ptr->info.data = buffer;
      texture_gen(format, buffer, local_ptr);
      
      // get random texture address
      baseaddr = texture_base(state, local_ptr->curtmu, GR_MIPMAPLEVELMASK_BOTH, &(local_ptr->info));
      
      // download texture
      grTexDownloadMipMap(local_ptr->curtmu,
			  baseaddr,
			  GR_MIPMAPLEVELMASK_BOTH,
			  &(local_ptr->info));
      
      grTexSource(local_ptr->curtmu,
		  baseaddr,
		  GR_MIPMAPLEVELMASK_BOTH,
		  &(local_ptr->info));
      
      if(alpha) {
	grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		       GR_COMBINE_FACTOR_OTHER_ALPHA,
		       GR_COMBINE_LOCAL_NONE,  
		       GR_COMBINE_OTHER_TEXTURE,
		       FXFALSE);        

	grTexCombine(local_ptr->curtmu,
		     GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     combine[format].function,
		     combine[format].factor, 
		     FXFALSE,
		     combine[format].invert);
      } else {
	grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		       GR_COMBINE_FACTOR_ONE,
		       GR_COMBINE_LOCAL_NONE,  
		       GR_COMBINE_OTHER_TEXTURE,
		       FXFALSE);        
	
	grTexCombine(local_ptr->curtmu,
		     combine[format].function,
		     combine[format].factor, 
		     GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     combine[format].invert,
		     FXFALSE);
      }

      // Init vertex data
      init_verts(local_ptr, 0);
      
      // Draw square
      grDrawTriangle(&(local_ptr->verts[0]), 
		     &(local_ptr->verts[1]), 
		     &(local_ptr->verts[2]));
      
      grDrawTriangle(&(local_ptr->verts[0]), 
		     &(local_ptr->verts[2]), 
		     &(local_ptr->verts[3]));
      
      
      end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
    }
  }

  free(buffer);
  
  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -tmu <n>   texture unit to test\n");
}


void setup_prim(conform_state *state)
{
  test_data *local_ptr;
  int i;
  GrChipID_t passtmu;

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

  grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		 GR_COMBINE_FACTOR_ONE,
		 GR_COMBINE_LOCAL_NONE,  
		 GR_COMBINE_OTHER_TEXTURE,
		 FXFALSE);        
  
  grAlphaBlendFunction(GR_BLEND_ONE,
		       GR_BLEND_ZERO,
		       GR_BLEND_ONE,
		       GR_BLEND_ZERO);

  grAlphaTestReferenceValue(0x00);
  grAlphaTestFunction(GR_CMP_ALWAYS);

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

  
  grTexClampMode(local_ptr->curtmu, 
		 GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

  grTexFilterMode(local_ptr->curtmu,
		  GR_TEXTUREFILTER_POINT_SAMPLED,
		  GR_TEXTUREFILTER_POINT_SAMPLED );

  grTexMipMapMode(local_ptr->curtmu,
		  GR_MIPMAP_NEAREST,
		  FXFALSE );

  grTexLodBiasValue(local_ptr->curtmu, 0.0f);
}

void texture_gen(int format, FxU16 *buffer, test_data *local_ptr)
{
  int pos;
  int i;
  FxU16 red, grn, blu, alp;

  local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
  local_ptr->info.format = GR_TEXFMT_ARGB_4444;
  local_ptr->info.smallLodLog2 = GR_LOD_LOG2_32;
  local_ptr->info.largeLodLog2 = GR_LOD_LOG2_32;

  // generate ARGB testure
  for(pos = 0; pos < 32*32; pos++) {
    if(pos < 256) {
      // grey ramp
      red = pos;
      grn = pos;
      blu = pos;
    } else if(pos < 512) {
      // red ramp
      red = pos-256;
      grn = 0;
      blu = 0;
    } else if(pos < 768) {
      // green ramp
      red = 0;
      grn = pos-512;
      blu = 0;
    } else {
      // blue ramp
      red = 0;
      grn = 0;
      blu = pos-768;
    }
    // alpha ramp
    alp = pos>>2;

    // reduce colores to 4 bit
    red = red>>4;
    grn = grn>>4;
    blu = blu>>4;
    alp = alp>>4;

    // combine into 4444 ARGB
    *buffer++ = (alp<<12)|(red<<8)|(grn<<4)|blu;
  }

  // setup vertex data for all TMUs
  for(i = 0; i < GRVERTEX_NUM_TMU; i++) {
    local_ptr->verts[0].tmuvtx[i].sow = 0.0f;
    local_ptr->verts[0].tmuvtx[i].tow =0.0f;
    local_ptr->verts[0].oow = 1.0f;  
    local_ptr->verts[0].tmuvtx[i].oow = 1.0f;
    
    local_ptr->verts[1].tmuvtx[i].sow = 256.0f;
    local_ptr->verts[1].tmuvtx[i].tow = 0.0f;
    local_ptr->verts[1].oow = 1.0f;  
    local_ptr->verts[1].tmuvtx[i].oow = 1.0f;
    
    local_ptr->verts[2].tmuvtx[i].sow = 256.0f;
    local_ptr->verts[2].tmuvtx[i].tow = 256.0f;
    local_ptr->verts[2].oow = 1.0f;  
    local_ptr->verts[2].tmuvtx[i].oow = 1.0f;
    
    local_ptr->verts[3].tmuvtx[i].sow = 0.0f;
    local_ptr->verts[3].tmuvtx[i].tow = 256.0f;
    local_ptr->verts[3].oow = 1.0f;  
    local_ptr->verts[3].tmuvtx[i].oow = 1.0f;
  }

}

// vertex list
struct
{
  float x, y;         /* X and Y in screen space */
} vertlist[4] = {
// 256x256 
8.0f, 8.0f,
32.0f + 8.0f, 8.0f,
32.0f + 8.0f, 32.0f + 8.0f,
8.0f, 32.0f + 8.0f,

};

void init_verts(test_data *local_ptr, int vertindx)
{
  local_ptr->verts[0].x = vertlist[(vertindx*4)].x;  
  local_ptr->verts[0].y = vertlist[(vertindx*4)].y;  
  
  local_ptr->verts[1].x = vertlist[(vertindx*4)+1].x;  
  local_ptr->verts[1].y = vertlist[(vertindx*4)+1].y;  
  
  local_ptr->verts[2].x = vertlist[(vertindx*4)+2].x;  
  local_ptr->verts[2].y = vertlist[(vertindx*4)+2].y;  
  
  local_ptr->verts[3].x = vertlist[(vertindx*4)+3].x;  
  local_ptr->verts[3].y = vertlist[(vertindx*4)+3].y;  
}

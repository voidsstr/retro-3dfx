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

static char *test_name = "texture-combine-detail";
static char *test_description = "Test texture combine with detail blending";

// local data
typedef struct {
  GrVertex verts[6];  // Temporary vertex storage
  GrTexInfo info;
  GrChipID_t curtmu;
  GrChipID_t othertmu;
} test_data;

// Prototypes
void init_verts(test_data *local_ptr, int vertindx);
void setup_prim(conform_state *state);
void texture_gen(int num, FxU16 *buffer, test_data *local_ptr, int alpha);

#define NUM_DETAIL 3

struct {
  int bias;
  FxU8 scale;
  float max; 
} detail[NUM_DETAIL] = {
  2, 7, 1.0f,
  4, 6, 1.0f,
  3, 7, 0.25f
};

#define NUM_COMBO 22

struct {
  GrCombineFunction_t function;
  GrCombineFactor_t factor;
} combine[NUM_COMBO] = {
  
  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
  GR_COMBINE_FACTOR_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_BLEND_OTHER,
  GR_COMBINE_FACTOR_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_OTHER,
  GR_COMBINE_FACTOR_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
  GR_COMBINE_FACTOR_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
  GR_COMBINE_FACTOR_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_BLEND,
  GR_COMBINE_FACTOR_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL,
  GR_COMBINE_FACTOR_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_BLEND_LOCAL,
  GR_COMBINE_FACTOR_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
  GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_BLEND_OTHER,
  GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_OTHER,
  GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
  GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
  GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_BLEND,
  GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL,
  GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_BLEND_LOCAL,
  GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,

  GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA,
  GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,

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
  num_tmu = nrand((int)(state->num_tmu-1));

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
  if(num_tmu >= (state->num_tmu-1)) {
    num_tmu = 0;
  }
  switch(num_tmu) {
  case 1:
    local_ptr->curtmu = GR_TMU1;
    local_ptr->othertmu = GR_TMU2;
    break;
  default:
    local_ptr->curtmu = GR_TMU0;
    local_ptr->othertmu = GR_TMU1;
    break;
  }

  // don't use any visuals with aux buffers
  state->num_aux_bufs = 0;

  // use 300x300 image area
  state->conform_width = 300;
  state->conform_height = 300;

  // tiling not supported
  state->tile = 0;

  // xycheck and xylimit not supported (fix this when adding self-checking)
  state->xycheck = 0;
  state->xylimit = 0;

  // setup image file names
  // this is manditory
  frame_name("texture-combine-detail",state);
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
  int alpha;
  int format;
  int cur_detail;
  FxU16 *buffer;
  FxU16 *buffer1;
  FxU32 baseaddr;

  local_ptr = (test_data *)state->local_data;

  // write record of TMU to loog file
  switch(local_ptr->curtmu) {
  case 1:
    log_message(state,"Using TMU 1 and TMU 2");
    break;
  default:
    log_message(state,"Using TMU 0 and TMU 1");
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

  if((buffer = (FxU16 *)malloc(256*256*2*2)) == NULL) {
    // fail
    log_message(state,"Could not allocate memory buffer for texture map");
  }

  if((buffer1 = (FxU16 *)malloc(256*256*2*2)) == NULL) {
    // fail
    log_message(state,"Could not allocate memory buffer for texture map");
  }

  for(alpha = 0; alpha < 2; alpha++) {
    for(format = 0; format < NUM_COMBO; format++) {
      for(cur_detail = 0; cur_detail < NUM_DETAIL; cur_detail++) {
	grBufferClear(clearcolor,0x0000,0x00000000);
	
	texture_gen(0, buffer, local_ptr, alpha);
	
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
	
	texture_gen(1, buffer1, local_ptr, alpha);
	
	// get random texture address
    baseaddr = texture_base(state, local_ptr->othertmu, GR_MIPMAPLEVELMASK_BOTH, &(local_ptr->info));
	
	// download upstream texture
	grTexDownloadMipMap(local_ptr->othertmu,
			    baseaddr,
			    GR_MIPMAPLEVELMASK_BOTH,
			    &(local_ptr->info));
	
	grTexSource(local_ptr->othertmu,
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
		       GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
		       GR_COMBINE_FACTOR_DETAIL_FACTOR,
		       combine[format].function,
		       combine[format].factor, 
		       FXFALSE,
		       FXFALSE);
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
		       FXFALSE,
		       FXFALSE);
	}
	
	// Setup the upstream TMU
	grTexCombine(local_ptr->othertmu,
		     GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     FXFALSE, FXFALSE );
	
	// Setup detail control
	grTexDetailControl(local_ptr->curtmu, 
			   detail[cur_detail].bias,
			   detail[cur_detail].scale,
			   detail[cur_detail].max); 
	
	// Init vertex data
	init_verts(local_ptr, 0);
	
	// Draw two triangles
	grDrawTriangle(&(local_ptr->verts[0]), 
		       &(local_ptr->verts[1]), 
		       &(local_ptr->verts[2]));
	
	grDrawTriangle(&(local_ptr->verts[3]), 
		       &(local_ptr->verts[4]), 
		       &(local_ptr->verts[5]));
	
	
	end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
      }
    }
  }

  free(buffer);
  free(buffer1);
  
  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -tmu <n>   texture unit to test\n");
}

// MIP level colors
FxU16 colors[9] = {
  0x000f, // blue      (1x1)
  0x00f0, // green     (2x2)
  0x0f00, // red       (4x4)

  0x0008, // 1/2 blue  (8x8)
  0x0080, // 1/2 green (16x16)
  0x0800, // 1/2 red   (32x32)

  0x0004, // 1/4 blue  (64x64)
  0x0040, // 1/4 green (128x128)
  0x0400  // 1/4 red   (256x256)
};

// MIP level colors
FxU16 colors1[9] = {
  0x0fff, 
  0x0fff, 
  0x0fff, 
  0x0fff, 
  0x0fff, 
  0x0fff, 
  0x0fff, 
  0x0fff,
  0x0fff 
};

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
  grTexClampMode(local_ptr->othertmu, 
		 GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

  grTexFilterMode(local_ptr->curtmu,
		  GR_TEXTUREFILTER_POINT_SAMPLED,
		  GR_TEXTUREFILTER_POINT_SAMPLED );
  grTexFilterMode(local_ptr->othertmu,
		  GR_TEXTUREFILTER_POINT_SAMPLED,
		  GR_TEXTUREFILTER_POINT_SAMPLED );

  grTexMipMapMode(local_ptr->curtmu,
		  GR_MIPMAP_NEAREST,
		  FXFALSE);
  grTexMipMapMode(local_ptr->othertmu,
		  GR_MIPMAP_NEAREST,
		  FXFALSE);

  grTexLodBiasValue(local_ptr->curtmu, 0.0f);
  grTexLodBiasValue(local_ptr->othertmu, 0.0f);
}

void texture_gen(int num, FxU16 *buffer, test_data *local_ptr, int alpha)
{
  int i,j;
  FxU16 color;

  local_ptr->info.smallLodLog2 = GR_LOD_LOG2_1;
  local_ptr->info.largeLodLog2 = GR_LOD_LOG2_256;
  local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
  local_ptr->info.format = GR_TEXFMT_ARGB_4444;

  local_ptr->info.data = buffer;

  // fill buffer with different color for each MIP level
  for(i=8; i>=0; i--) {
    if(num) {
      color = colors[i];
    } else {
      color = colors1[i];
    }
    
    for(j=0; j < (1<<(i*2)) ; j++) {
      if(alpha&(num==0)) {
	if((i-2) >= 0) {
	  if(((j>>(i-2))%2)==0) {
	    *buffer++ = color|0xf000;
	  } else {
	    *buffer++ = color;
	  }
	} else {
	  *buffer++ = color|0xf000;
	}
      } else {
	*buffer++ = color|0xf000;
      }
    }
  }

  // setup vertex data for all TMUs
  for(i = 0; i < GRVERTEX_NUM_TMU; i++) {
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

// vertex list
struct
{
  float x, y;         /* X and Y in screen space */
} vertlist[6] = {
  // triangle #1
22.0f, 22.0f,
256.0f + 22.0f, 22.0f,
256.0f + 22.0f, 128.0f + 22.0f,
  // triangle #2
22.0f, 22.0f,
22.0f, 256.0f + 22.0f,
128.0f + 22.0f, 256.0f + 22.0f,
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
  
  local_ptr->verts[4].x = vertlist[(vertindx*4)+4].x;  
  local_ptr->verts[4].y = vertlist[(vertindx*4)+4].y;  
  
  local_ptr->verts[5].x = vertlist[(vertindx*4)+5].x;  
  local_ptr->verts[5].y = vertlist[(vertindx*4)+5].y;  
}

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
#include <texus.h>

static char *test_name = "texture-load-ncc";
static char *test_description = "Test NCC texture formats";

// local data
typedef struct {
  GrVertex verts[4];  // Temporary vertex storage
  GrTexInfo info;
  Gu3dfInfo guinfo;
  GrChipID_t curtmu;
  FxU32 *buffer;
  int start;
  int end;
  void *ncc;
} test_data;

// Prototypes
void init_verts(test_data *local_ptr, int vertindx);
void setup_prim(conform_state *state);
void texture_gen(int aspect, int format, int table, test_data *local_ptr);
void build_texture(FxU32 *tmp, int height, int width, 
		   FxU32 red, FxU32 grn, FxU32 blu_a, FxU32 blu);

#define NUM_FORMAT 2

int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;
  int num_tmu;
  int format;

  state->test_name = test_name;
  state->test_description = test_description;

  if((state->local_data = malloc(sizeof(test_data))) == NULL) {
    // Could not allocate data for local memory
    log_perror(state,"Could not allocate memory for local data");
    return(0);
  } 

  local_ptr = (test_data *)state->local_data;

  // cycle all textures
  local_ptr->start = 0;
  local_ptr->end = (NUM_FORMAT-1);

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
    } else if(!_stricmp(*local_argv,"-form") || // force single format
	      !_stricmp(*local_argv,"/form")) {
      ++local_argv;
      if(!sscanf(*local_argv,"%d",&format)) {
        // bad argument
      } else {
	if(format <= NUM_FORMAT) {
	  if(format == 0) {
	    local_ptr->start = nrand(NUM_FORMAT);
	    local_ptr->end = local_ptr->start;
	  } else {
	    local_ptr->start = format-1;
	    local_ptr->end = local_ptr->start;
	  }
	}
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

  // use 100x100 image area
  state->conform_width = 100;
  state->conform_height = 100;

  // tiling not supported
  state->tile = 0;

  // xycheck and xylimit not supported (fix this when adding self-checking)
  state->xycheck = 0;
  state->xylimit = 0;
 
  // setup image file names
  // this is manditory
  frame_name("texture-load-ncc",state);
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
  int format;
  int aspect;
  int table;
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
    
    // randomize texture
    local_ptr->start = nrand(NUM_FORMAT);
    local_ptr->end = local_ptr->start;
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

  grViewport(state->conform_x, state->conform_y, 
             state->conform_width, state->conform_height);

  setup_prim(state);

  if((local_ptr->guinfo.data = malloc(64*64*2)) == NULL) {
    // fail
    log_message(state,"Could not allocate memory buffer for texture map");
  }

  if((local_ptr->buffer = (FxU32 *)malloc(64*64*4)) == NULL) {
    // fail
    log_message(state,"Could not allocate memory buffer for texture map");
  }

  for(format = local_ptr->start; format < (local_ptr->end+1); format++) {
    for(aspect = 0; aspect < 7; aspect++) { 
      for(table = 0; table < 2; table++) {
	grBufferClear(clearcolor,0x0000,0x00000000);
	
	local_ptr->info.data = local_ptr->guinfo.data;
	texture_gen(aspect, format, table, local_ptr);
	
	// get random texture address
    baseaddr = texture_base(state, local_ptr->curtmu, GR_MIPMAPLEVELMASK_BOTH, &(local_ptr->info));
	
	// download texture
	switch(table) {
	case 0:
	  grTexDownloadTable(GR_TEXTABLE_NCC0,
			     &(local_ptr->guinfo.table.nccTable)); 
	  grTexNCCTable(GR_NCCTABLE_NCC0);
	  break;
	default:
	  grTexDownloadTable(GR_TEXTABLE_NCC1,
			     &(local_ptr->guinfo.table.nccTable)); 
	  grTexNCCTable(GR_NCCTABLE_NCC1);
	  break;
	}

	grTexDownloadMipMap(local_ptr->curtmu,
			    baseaddr,
			    GR_MIPMAPLEVELMASK_BOTH,
			    &(local_ptr->info));
	
	grTexSource(local_ptr->curtmu,
		    baseaddr,
		    GR_MIPMAPLEVELMASK_BOTH,
		    &(local_ptr->info));
	
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
  }
  
  free(local_ptr->buffer);
  free(local_ptr->guinfo.data);
  
  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -tmu <n>   texture unit to test\n");
  fprintf(stderr,"   -form <n>  force single format test\n");
  fprintf(stderr,"          0   choose random format\n");
  fprintf(stderr,"          1   GR_TEXFMT_YIQ_422\n");
  fprintf(stderr,"          2   GR_TEXFMT_AYIQ_8422\n");
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

  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		 GR_COMBINE_FACTOR_ONE,
		 GR_COMBINE_LOCAL_NONE,  
		 GR_COMBINE_OTHER_TEXTURE,
		 FXFALSE);        

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
  grAlphaTestFunction(GR_CMP_NOTEQUAL);

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
	       GR_COMBINE_FUNCTION_LOCAL,
	       GR_COMBINE_FACTOR_ONE,
	       FXFALSE, FXFALSE );
  
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


void texture_gen(int aspect, int format, int table, test_data *local_ptr)
{
  int width;
  int height;
  int i;

  // Apply aspect ratio (WxH)
  switch(aspect) {
  case 1: // 8x1
    height = 8;
    width = 64;
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_8x1;
    break;
  case 2: // 4x1
    height = 16;
    width = 64;
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_4x1;
    break;
  case 3: // 2x1
    height = 32;
    width = 64;
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_2x1;
    break;
  case 4: // 1x2
    height = 64;
    width = 32;
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x2;
    break;
  case 5: // 1x4
    height = 64;
    width = 16;
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x4;
    break;
  case 6: // 1x8
    height = 64;
    width = 8;
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x8;
    break;
  default: // 1x1
    height = 64;
    width = 64;
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
    break;
  }
  
  local_ptr->info.smallLodLog2 = GR_LOD_LOG2_64;
  local_ptr->info.largeLodLog2 = GR_LOD_LOG2_64;

  switch(format) {
  case 0:
    local_ptr->info.format = GR_TEXFMT_YIQ_422;
    break;
  case 1:
    local_ptr->info.format = GR_TEXFMT_AYIQ_8422;
    break;
  default:
    break;
  }
  
  if(table) {
    build_texture(local_ptr->buffer, height, width,
		  0xff0000ff, 0xff00ff00, 0xffff0000, 0x00ff0000);
  } else {
    build_texture(local_ptr->buffer, height, width,
		  0xffff0000, 0xff00ff00, 0xff0000ff, 0x000000ff);
  }

  txInit3dfInfo(&(local_ptr->guinfo), local_ptr->info.format,
		&width, &height,
		1, TX_AUTORESIZE_DISABLE );

  txConvert( &(local_ptr->guinfo), GR_TEXFMT_ARGB_8888,
             width, height,
             local_ptr->buffer,
	     TX_DITHER_NONE | TX_COMPRESSION_STATISTICAL,
	     NULL );

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
22.0f, 22.0f,
64.0f + 22.0f, 22.0f,
64.0f + 22.0f, 64.0f + 22.0f,
22.0f, 64.0f + 22.0f,

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

void build_texture(FxU32 *tmp, int height, int width, 
		   FxU32 red, FxU32 grn, FxU32 blu_a, FxU32 blu)
{
  int i,j;

  for(i=0; i<height; i++) {
    for(j=0; j<width ; j++) {
      if((i==0) || (i==(height-2)) ||
	 (j==0) || (j==(width-2))) {
	// red border
	*tmp++ = red;
      } else if ((i==1) || (i==(height-1)) ||
		 (j==1) || (j==(width-1))) {
	// green interior border
	*tmp++ = grn;
      } else {
	// blue center
	if((i%3)&&(j%3)) {
	  // alpha != 0
	  *tmp++ = blu_a;
	} else {
	  // alpha == 0
	  *tmp++ = blu;
	}
      }
    }
  }
}

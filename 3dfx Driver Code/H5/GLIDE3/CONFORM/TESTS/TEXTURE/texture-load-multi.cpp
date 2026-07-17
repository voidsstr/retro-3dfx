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

static char *test_name = "texture-load-multi";
static char *test_description = "Test loading of Multibase textures";

// local data
typedef struct {
  GrVertex verts[4];  // Temporary vertex storage
  GrTexInfo info;
  GrChipID_t curtmu;
} test_data;

// Color Array
FxU16 colors[3*9] = {
0xfc00, 0x83e0, 0x801f, // red, green, blue
0x83e0, 0x801f, 0xfc00, // green, blue, red
0x801f, 0xfc00, 0x83e0, // blue, red, green
0xfc00, 0x801f, 0x83e0, // red, blue, green
0x801f, 0x83e0, 0xfc00, // blue, green, red
0x83e0, 0xfc00, 0x801f, // green, red, blue
0xfc00, 0x801f, 0x83e0, // red, blue, green 
0xfc00, 0x83e0, 0x801f, // red, green, blue
0x83e0, 0x801f, 0xfc00, // green, blue, red
};

// Prototypes
void init_verts(test_data *local_ptr, int vertindx);
void setup_prim(conform_state *state);
void texture_gen(int aspect, int level, int mip, test_data *local_ptr, 
		 FxU16 **tmp, float oow,
		 FxU16 color1, FxU16 color2, FxU16 color3);

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

  // use 300x300 image area
  state->conform_width = 300;
  state->conform_height = 300;

  // tiling or multiple reps not supported
  state->tile = 0;
  state->num_reps = 1;

  // xycheck and xylimit not supported (fix this when adding self-checking)
  state->xycheck = 0;
  state->xylimit = 0;
 
  // setup image file names
  // this is manditory
  frame_name("texture-load-multi",state);
  img_file_type(IMG_P6,state);

  // Initialization complete
  return(-1);
}

void close_conform(conform_state *state)
{
  // Free up local data
  free(state->local_data);
}

struct {
  GrTexBaseRange_t range;
  GrLOD_t small;
  GrLOD_t large;
} segdat[4] = {
GR_TEXBASE_32_TO_1, GR_LOD_LOG2_1, GR_LOD_LOG2_32,
GR_TEXBASE_64, GR_LOD_LOG2_64, GR_LOD_LOG2_64,
GR_TEXBASE_128, GR_LOD_LOG2_128, GR_LOD_LOG2_128,
GR_TEXBASE_256, GR_LOD_LOG2_256, GR_LOD_LOG2_256
};
      
int do_conform(conform_state *state)
{
  test_data *local_ptr;
  GrColor_t clearcolor;
  int level;
  int mip;
  int i,j;
  int done;
  int aspect;
  int vertindx;
  FxU16 *buffer;
  FxU16 *tmp;
  FxU32 baseaddr[4];
  int basesize;
  FxU32 baseend[4];
  int level_full[4];
  FxU16 *segment[4];

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

  grViewport(state->conform_x, state->conform_y, 
             state->conform_width, state->conform_height);

  setup_prim(state);

  if((buffer = (FxU16 *)malloc(256*256*4)) == NULL) {
    // fail
    log_message(state,"Could not allocate memory buffer for texture map");
  }

  grTexMultibase(local_ptr->curtmu, 1);
  
  for(aspect = 0; aspect < 7; aspect++) { 
    for(level = 0; level < 9; level++) {
      // Create and load textures
      // setup buffer for four regions in inverse order
      segment[0] = buffer; // base of LOD 32 and below
      segment[1] = segment[0] + (32*32*2); // base of LOD 64
      segment[2] = segment[1] + (64*64); // base of LOD 128
      segment[3] = segment[2] + (128*128); // base of LOD 256
      
      // generate LOD 256
      if(level >= 8) {
	tmp = segment[3];
	texture_gen(aspect, level, 8, local_ptr, &tmp, 1.0f,
		    colors[0],
		    colors[1],
		    colors[2]);
      } else {
	segment[3] = NULL;
      }

      // generate LOD 128
      if(level >= 7) {
	tmp = segment[2];
	texture_gen(aspect, level, 7, local_ptr, &tmp, 1.0f,
		    colors[3],
		    colors[4],
		    colors[5]);
      } else {
	segment[2] = NULL;
      }

      // generate LOD 64
      if(level >= 6) {
	tmp = segment[1];
	texture_gen(aspect, level, 6, local_ptr, &tmp, 1.0f,
		    colors[6],
		    colors[7],
		    colors[8]);
      } else {
	segment[1] = NULL;
      }

      // generate LOD 32 - LOD 1
      tmp = segment[0];
      for(i=5;i>=0;i--) {
	texture_gen(aspect, level, i, local_ptr, &tmp, 1.0f,
                    colors[(8-i)*3],
		    colors[((8-i)*3)+1],
		    colors[((8-i)*3)+2]);
      }

      // download texture in random level order
      for(i=0;i<4;i++) {
	level_full[i] = 0;
      }

      for(i=0;i<4;i++) {
        // pick an untouched segment
	do {
	  mip = nrand(4);
	} while(level_full[mip]);
	level_full[mip] = 1;

	local_ptr->info.smallLodLog2 = segdat[mip].small;
	local_ptr->info.largeLodLog2 = segdat[mip].large;

	// get a random base address for each segment
	switch(mip) {
	case 1: // RANGE 64
	  basesize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,
					     &(local_ptr->info));
	  break;
	case 2: // RANGE 128
	  basesize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,
					     &(local_ptr->info));
	  break;
	case 3: // RANGE 256
	  basesize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,
					     &(local_ptr->info));
	  break;
	default: // RANGE 32 - 1
	  basesize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,
					     &(local_ptr->info));
	  break;
	}

	done = 0;
	while(!done) {
	  baseaddr[mip] = texture_base(state, local_ptr->curtmu, GR_MIPMAPLEVELMASK_BOTH, &(local_ptr->info));
	  baseend[mip] = baseaddr[mip] + basesize -1;
	  done = 1;
	  if(i != 0) {
	    // make sure address does not conflict
	    for(j=0;j<4;j++) {
	      if((j != mip) && level_full[j]) {
		if((baseaddr[j] >= baseaddr[mip]) && 
		   (baseaddr[j] <= baseend[mip]))
		  done = 0;
		if((baseend[j] >= baseaddr[mip]) && 
		   (baseend[j] <= baseend[mip]))
		  done = 0;
	      }
	    }
	  }
	}

	// load it
	if(segment[mip] != NULL) {
	  local_ptr->info.data = segment[mip];

	  grTexMultibaseAddress(local_ptr->curtmu,
				segdat[mip].range,
				baseaddr[mip],
				GR_MIPMAPLEVELMASK_BOTH,
				&(local_ptr->info));
	  grTexDownloadMipMap(local_ptr->curtmu,
			      baseaddr[mip],
			      GR_MIPMAPLEVELMASK_BOTH,
			      &(local_ptr->info));
	}
      }
      
      local_ptr->info.smallLodLog2 = GR_LOD_LOG2_1;
      switch(level) {
      case 8:
	local_ptr->info.largeLodLog2 = GR_LOD_LOG2_256;
	break;
      case 7:
	local_ptr->info.largeLodLog2 = GR_LOD_LOG2_128;
	break;
      case 6:
	local_ptr->info.largeLodLog2 = GR_LOD_LOG2_64;
	break;
      case 5:
	local_ptr->info.largeLodLog2 = GR_LOD_LOG2_32;
	break;
      case 4:
	local_ptr->info.largeLodLog2 = GR_LOD_LOG2_16;
	break;
      case 3:
	local_ptr->info.largeLodLog2 = GR_LOD_LOG2_8;
	break;
      case 2:
	local_ptr->info.largeLodLog2 = GR_LOD_LOG2_4;
	break;
      case 1:
	local_ptr->info.largeLodLog2 = GR_LOD_LOG2_2;
	break;
      default:
	local_ptr->info.largeLodLog2 = GR_LOD_LOG2_1;
      }

      switch(level) {
      case 8:
	grTexSource(local_ptr->curtmu,
		    baseaddr[3],
		    GR_MIPMAPLEVELMASK_BOTH,
		    &(local_ptr->info));
	break;
      case 7:
	grTexSource(local_ptr->curtmu,
		    baseaddr[2],
		    GR_MIPMAPLEVELMASK_BOTH,
		    &(local_ptr->info));
	break;
      case 6:
	grTexSource(local_ptr->curtmu,
		    baseaddr[1],
		    GR_MIPMAPLEVELMASK_BOTH,
		    &(local_ptr->info));
	break;
      default:
	grTexSource(local_ptr->curtmu,
		    baseaddr[0],
		    GR_MIPMAPLEVELMASK_BOTH,
		    &(local_ptr->info));
	break;
      }
      grBufferClear(clearcolor,0x0000,0x00000000);
      vertindx = 0;

      for(mip = 8; mip >= 0; mip--) {
        // skip topmost level for small textures
        if((level != 8)&&(mip == 8)) {
	  mip--;
	  vertindx++ ;
	}

	// Init vertex data
	init_verts(local_ptr, vertindx);
      
	// Draw square
	grDrawTriangle(&(local_ptr->verts[0]), 
		       &(local_ptr->verts[1]), 
		       &(local_ptr->verts[2]));
	
	grDrawTriangle(&(local_ptr->verts[0]), 
		       &(local_ptr->verts[2]), 
		       &(local_ptr->verts[3]));
	
      
	if(mip == 8) {
	  end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
	  grBufferClear(clearcolor,0x0000,0x00000000);
	}	       
      
	vertindx++;
      }

      end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
    }
  }

  grTexMultibase(local_ptr->curtmu, 0);

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
  GrChipID_t passtmu;
  int i;

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
  
  grTexClampMode(local_ptr->curtmu,
		 GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

  grTexFilterMode(local_ptr->curtmu,
		  GR_TEXTUREFILTER_POINT_SAMPLED,
		  GR_TEXTUREFILTER_POINT_SAMPLED );

  grTexMipMapMode(local_ptr->curtmu,
		  GR_MIPMAP_NEAREST,
		  FXFALSE );

  grTexLodBiasValue(local_ptr->curtmu,0.0f);
}


void texture_gen(int aspect, int level, int mip, test_data *local_ptr, 
		 FxU16 **tmp, float oow,
		 FxU16 color1, FxU16 color2, FxU16 color3)
{
  int max;
  int width;
  int height;
  int i,j;
  
  // calculate max dimension from level
  if(mip < 0) {
    max = 0;
  } else {
    max = mip;
  }
  max = 1 << max;
  
  // Apply aspect ratio (WxH)
  switch(aspect) {
  case 1: // 8x1
    if(max < 8) {
      height = 1;
    } else {
      height = max/8;
    }
    width = max;
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_8x1;
    break;
  case 2: // 4x1
    if(max < 4) {
      height = 1;
    } else {
      height = max/4;
    }
    width = max;
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_4x1;
    break;
  case 3: // 2x1
    if(max < 2) {
      height = 1;
    } else {
      height = max/2;
    }
    width = max;
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_2x1;
    break;
  case 4: // 1x2
    height = max;
    if(max < 2) {
      width = 1;
    } else {
      width = max/2;
    }
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x2;
    break;
  case 5: // 1x4
    height = max;
    if(max < 4) {
      width = 1;
    } else {
      width = max/4;
    }
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x4;
    break;
  case 6: // 1x8
    height = max;
    if(max < 8) {
      width = 1;
    } else {
      width = max/8;
    }
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x8;
    break;
  default: // 1x1
    height = max;
    width = max;
    local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
    break;
  }
  
  // fill buffer with test pattern
  switch(width+height) {
  case 9: // 8x1 & 1x8
  case 5: // 4x1 & 1x4
  case 4: // 2x2
  case 3: // 2x1 & 1x2
    // rotate thru available colors
    *(*tmp)++ = color1;
    *(*tmp)++ = color2;
    *(*tmp)++ = color3;
    *(*tmp)++ = color1;
    *(*tmp)++ = color2;
    *(*tmp)++ = color3;
    *(*tmp)++ = color1;
    *(*tmp)++ = color2;
    break;
  case 2: // 1x1
    // each texture gets a different color
    switch(level) {
    case -1:
      *(*tmp)++ = color2;
      break;
    case -2:
      *(*tmp)++ = color3;
      break;
    default:
      *(*tmp)++ = color1;
      break;
    }
    break;
  case 6: // 4x2 & 2x4
  case 10: // 8x2 & 2x8
  case 18: // 16x2 & 2x16
    for(i=0; i<height; i++) {
      for(j=0; j<width ; j++) {
        if((j == 0) || (i == 0)) {
	  *(*tmp)++ = color1; // red
	} else {
	  if(width > height) {
	    // wide texture
            switch(j%3) {
	    case 0:
	      *(*tmp)++ = color1;
	      break;
	    case 1:
	      *(*tmp)++ = color2;
	      break;
	    case 2:
	      *(*tmp)++ = color3;
	      break;
	    }
	  } else {
	    // high texture
            switch(j%3) {
	    case 0:
	      *(*tmp)++ = color1;
	      break;
	    case 1:
	      *(*tmp)++ = color2;
	      break;
	    case 2:
	      *(*tmp)++ = color3;
	      break;
	    }
	  }
	}
      }
    }
    break;
  case 8: // 4x4
  case 12: // 8x4 & 4x8
  case 20: // 16x4 & 4x16
  case 36: // 32x4 & 4x32
    for(i=0; i<height; i++) {
      for(j=0; j<width ; j++) {
        if((j == 0) || (i == 0)) {
	  *(*tmp)++ = color1; // red
	} else {
	  if(width > height) {
	    // wide texture
            switch(i%3) {
	    case 0:
              *(*tmp)++ = color2;
	      break;
	    case 1:
              *(*tmp)++ = color3;
	      break;
	    case 2:
              *(*tmp)++ = color1;
	      break;
	    }
	  } else {
	    // high texture
            switch(j%3) {
	    case 0:
              *(*tmp)++ = color2;
	      break;
	    case 1:
              *(*tmp)++ = color3;
	      break;
	    case 2:
              *(*tmp)++ = color1;
	      break;
	    }
	  }
	}
      }
    }
    break;
  default:
    for(i=0; i<height; i++) {
      for(j=0; j<width ; j++) {
        if((i==0) || (i==(height-2)) ||
           (j==0) || (j==(width-2))) {
	  // color1 border
	  *(*tmp)++ = color1;
        } else if ((i==1) || (i==(height-1)) ||
                   (j==1) || (j==(width-1))) {
	  // color2 interior border
	  *(*tmp)++ = color2;
	} else {
	  // creamy color3 center
	  *(*tmp)++ = color3;
	}
      }
    }
    break;
  }
  
  local_ptr->info.smallLodLog2 = GR_LOD_LOG2_1;
  switch(level) {
  case 8:
    local_ptr->info.largeLodLog2 = GR_LOD_LOG2_256;
    break;
  case 7:
    local_ptr->info.largeLodLog2 = GR_LOD_LOG2_128;
    break;
  case 6:
    local_ptr->info.largeLodLog2 = GR_LOD_LOG2_64;
    break;
  case 5:
    local_ptr->info.largeLodLog2 = GR_LOD_LOG2_32;
    break;
  case 4:
    local_ptr->info.largeLodLog2 = GR_LOD_LOG2_16;
    break;
  case 3:
    local_ptr->info.largeLodLog2 = GR_LOD_LOG2_8;
    break;
  case 2:
    local_ptr->info.largeLodLog2 = GR_LOD_LOG2_4;
    break;
  case 1:
    local_ptr->info.largeLodLog2 = GR_LOD_LOG2_2;
    break;
  default:
    local_ptr->info.largeLodLog2 = GR_LOD_LOG2_1;
    break;
  }

  // setup vertex data
  for(i=0; i<GRVERTEX_NUM_TMU; i++) {
    local_ptr->verts[0].tmuvtx[i].sow = 0.0f;
    local_ptr->verts[0].tmuvtx[i].tow =0.0f;
    local_ptr->verts[0].oow = oow;  
    local_ptr->verts[0].tmuvtx[i].oow = oow;
    
    local_ptr->verts[1].tmuvtx[i].sow = 256.0f;
    local_ptr->verts[1].tmuvtx[i].tow = 0.0f;
    local_ptr->verts[1].oow = oow;  
    local_ptr->verts[1].tmuvtx[i].oow = oow;
    
    local_ptr->verts[2].tmuvtx[i].sow = 256.0f;
    local_ptr->verts[2].tmuvtx[i].tow = 256.0f;
    local_ptr->verts[2].oow = oow;  
    local_ptr->verts[2].tmuvtx[i].oow = oow;
    
    local_ptr->verts[3].tmuvtx[i].sow = 0.0f;
    local_ptr->verts[3].tmuvtx[i].tow = 256.0f;
    local_ptr->verts[3].oow = oow;  
    local_ptr->verts[3].tmuvtx[i].oow = oow;
  }

  local_ptr->info.format = GR_TEXFMT_ARGB_1555;
}

// vertex list
struct
{
  float x, y;         /* X and Y in screen space */
} vertlist[44] = {
// 256x256 
22.0f, 22.0f,
256.0f + 22.0f, 22.0f,
256.0f + 22.0f, 256.0f + 22.0f,
22.0f, 256.0f + 22.0f,

// 128x128
22.0f, 22.0f,
128.0f + 22.0f, 22.0f,
128.0f + 22.0f, 128.0f + 22.0f,
22.0f, 128.0f + 22.0f,

// 64 x 64
32.0f + 22.0f, 160.0f + 22.0f,
32.0f + 64.0f + 22.0f, 160.0f + 22.0f,
32.0f + 64.0f + 22.0f, 160.0f + 64.0f + 22.0f,
32.0f + 22.0f, 160.0f + 64.0f + 22.0f,

// 32 x 32
144.0f + 22.0f, 16.0f + 22.0f,
144.0f + 32.0f + 22.0f, 16.0f + 22.0f,
144.0f + 32.0f + 22.0f, 16.0f + 32.0f + 22.0f,
144.0f + 22.0f, 16.0f + 32.0f + 22.0f,
  
// 16 x 16
216.0f + 22.0f, 24.0f + 22.0f,
216.0f + 16.0f + 22.0f, 24.0f + 22.0f,
216.0f + 16.0f + 22.0f, 24.0f + 16.0f + 22.0f,
216.0f + 22.0f, 24.0f + 16.0f + 22.0f,
  
// 8 x 8
156.0f + 22.0f, 92.0f + 22.0f,
156.0f + 8.0f + 22.0f, 92.0f + 22.0f,
156.0f + 8.0f + 22.0f, 92.0f + 8.0f + 22.0f,
156.0f + 22.0f, 92.0f + 8.0f + 22.0f,

// 4 x 4
222.0f + 22.0f, 94.0f + 22.0f,
222.0f + 4.0f + 22.0f, 94.0f + 22.0f,
222.0f + 4.0f + 22.0f, 94.0f + 4.0f + 22.0f,
222.0f + 22.0f, 94.0f + 4.0f + 22.0f,

// 2 x 2
159.0f + 22.0f, 159.0f + 22.0f,
159.0f + 2.0f + 22.0f, 159.0f + 22.0f,
159.0f + 2.0f + 22.0f, 159.0f + 2.0f + 22.0f,
159.0f + 22.0f, 159.0f + 2.0f + 22.0f,

// 1 x 1 # 1
224.0f + 22.0f, 160.0f + 22.0f,
224.0f + 1.0f + 22.0f, 160.0f + 22.0f,
224.0f + 1.0f + 22.0f, 160.0f + 1.0f + 22.0f,
224.0f + 22.0f, 160.0f + 1.0f + 22.0f,

// 1 x 1 # 2
160.0f + 22.0f, 224.0f + 22.0f,
160.0f + 1.0f + 22.0f, 224.0f + 22.0f,
160.0f + 1.0f + 22.0f, 224.0f + 1.0f + 22.0f,
160.0f + 22.0f, 224.0f + 1.0f + 22.0f,

// 1 x 1 # 3
224.0f + 22.0f, 224.0f + 22.0f,
224.0f + 1.0f + 22.0f, 224.0f + 22.0f,
224.0f + 1.0f + 22.0f, 224.0f + 1.0f + 22.0f,
224.0f + 22.0f, 224.0f + 1.0f + 22.0f
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

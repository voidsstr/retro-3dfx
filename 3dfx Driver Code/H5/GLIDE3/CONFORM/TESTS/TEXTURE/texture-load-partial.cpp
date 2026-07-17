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

static char *test_name = "texture-load-partial";
static char *test_description = "Test loading of MIP mapped texture chunks";

// local data
typedef struct {
  GrVertex verts[4];  // Temporary vertex storage
  GrTexInfo info;
  GrChipID_t curtmu;
  struct {
    unsigned char line_full[256];
    FxU16 *level_ptr;
  } level[9];
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
void load_texture_chunks(conform_state *state, int aspect, int level,
			 int mip, FxU32 baseaddr);

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
  frame_name("texture-load-partial",state);
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
  int level;
  int mip;
  int aspect;
  int vertindx;
  FxU16 *buffer;
  FxU16 *tmp;
  FxU32 baseaddr;

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

  for(aspect = 0; aspect < 7; aspect++) { 
    for(level = 8; level >= 0; level--) {
      // generate entire texture
      tmp = buffer;
      for(mip = level; mip >= 0; mip--) {
	local_ptr->level[mip].level_ptr = tmp;
	texture_gen(aspect, level, mip, local_ptr, &tmp, 1.0f,
                    colors[(level-mip)*3],
		    colors[((level-mip)*3)+1],
		    colors[((level-mip)*3)+2]);
      }

      // get random texture address
      baseaddr = texture_base(state, local_ptr->curtmu, GR_MIPMAPLEVELMASK_BOTH, &(local_ptr->info));

      // download lines in random level order
      load_texture_chunks(state, aspect, level, mip, baseaddr);
      
      grTexSource(local_ptr->curtmu,
		  baseaddr,
		  GR_MIPMAPLEVELMASK_BOTH,
		  &(local_ptr->info));

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

void load_texture_chunks(conform_state *state, int aspect, int level,
			 int mip, FxU32 baseaddr)
{
  int start;
  int end;
  int num_lines;
  int stride;
  int i,j;
  int done;
  FxU16 *line_ptr;
  GrLOD_t curLod;
  int level_full[9];
  test_data *local_ptr;

  // point to test data
  local_ptr = (test_data *)state->local_data;

  // set entire texture to empty
  for(i=0;i<=level;i++) {
    level_full[i] = 0;
    for(j=0;j<256;j++) {
      local_ptr->level[i].line_full[j] = 0;
    }
  }
  
  
  done = 0;
  while(!done) {
    // pick an untouched level
    do {
      mip = nrand(level+1);
    } while(level_full[mip]);
    
    switch(mip) {
    case 1:
      curLod = GR_LOD_LOG2_2;
      break;
    case 2:
      curLod = GR_LOD_LOG2_4;
      break;
    case 3:
      curLod = GR_LOD_LOG2_8;
      break;
    case 4:
      curLod = GR_LOD_LOG2_16;
      break;
    case 5:
      curLod = GR_LOD_LOG2_32;
      break;
    case 6:
      curLod = GR_LOD_LOG2_64;
      break;
    case 7:
      curLod = GR_LOD_LOG2_128;
      break;
    case 8:
      curLod = GR_LOD_LOG2_256;
      break;
    default:
      curLod = GR_LOD_LOG2_1;
      break;
    }
    
    // Apply aspect ratio (WxH)
    switch(aspect) {
    case 1: // 8x1
      if((1<<mip) < 8) {
	num_lines = 1;
      } else {
	num_lines = (1<<mip)/8;
      }
      stride = (1<<mip);
      break;
    case 2: // 4x1
      if((1<<mip) < 4) {
	num_lines = 1;
      } else {
	num_lines = (1<<mip)/4;
      }
      stride = (1<<mip);
      break;
    case 3: // 2x1
      if((1<<mip) < 2) {
	num_lines = 1;
      } else {
	num_lines = (1<<mip)/2;
      }
      stride = (1<<mip);
      break;
    case 4: // 1x2
      num_lines = (1<<mip);
      if((1<<mip) < 2) {
	stride = 1;
      } else {
	stride = (1<<mip)/2;
      }
      break;
    case 5: // 1x4
      num_lines = (1<<mip);
      if((1<<mip) < 4) {
	stride = 1;
      } else {
	stride = (1<<mip)/4;
      }
      break;
    case 6: // 1x8
      num_lines = (1<<mip);
      if((1<<mip) < 8) {
	stride = 1;
      } else {
	stride = (1<<mip)/8;
      }
      break;
    default: // 1x1
      num_lines = (1<<mip);
      stride = (1<<mip);
      break;
    }
    
    // pick a start line
    do {
      start = nrand(num_lines);
    } while(local_ptr->level[mip].line_full[start]);
    
    line_ptr = local_ptr->level[mip].level_ptr + (start * stride);
    
    // pick an end line within range
    end = nrand(1<<mip) + start;
    if(end >= num_lines)
      end = num_lines-1;

    // limit end to continous empty lines 
    for(i = start; i <= end; i++) {
      if(!local_ptr->level[mip].line_full[i]) {
	local_ptr->level[mip].line_full[i] = 1;
      } else {
	end = i-1;
	break;
      }
    }
    
    // load the texture chunk
    grTexDownloadMipMapLevelPartial(local_ptr->curtmu, baseaddr, curLod,
				    local_ptr->info.largeLodLog2,
				    local_ptr->info.aspectRatioLog2,
				    local_ptr->info.format,
				    GR_MIPMAPLEVELMASK_BOTH,
				    line_ptr, start, end);
    
    // check for this level complete
    // assume complete
    level_full[mip] = 1;
    for(i=0;i<num_lines;i++) {
      if(!local_ptr->level[mip].line_full[i]) {
        // found empty line, level not complete
	level_full[mip] = 0;
	break;
      }
    }

    // check for complete texture
    // assume complete
    done = 1;
    for(i=0; i<(level+1); i++) {
      if(!level_full[i]) {
	// found incomplete level, texture not complete
	done = 0;
	break;
      }
    }
  }
}

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

static char *test_name = "ext-tex-chroma";
static char *test_description = "Test texture chroma range";

// local data
typedef struct {
  GrVertex verts[4];  // Temporary vertex storage
  GrTexInfo info;
  GrChipID_t curtmu;
} test_data;

// Prototypes
void setup_prim(conform_state *state);
void setup_verts(conform_state *state);
void texture_gen(test_data *local_ptr, FxU16 *buffer);

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
  frame_name("ext-tex-chroma",state);
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
  GrColor_t chromacolor;
  GrColor_t chromacolor1;
  FxU16 *buffer;
  FxU32 baseaddr;
  FxU32 color;
  int channel;
  char *ext;
//  GrProc grTexChromaModeExt(GrChipID_t tmu, GrTexChromakeyMode_t mode);
//  GrProc grTexChromaRangeExt(GrChipID_t tmu, GrColor_t color0,
//                             GrColor_t color1, GrTexChromakeyMode_t mode);
  int (FX_CALL *grTexChromaModeExt)(GrChipID_t tmu, GrChromakeyMode_t mode);
  int (FX_CALL *grTexChromaRangeExt)(GrChipID_t tmu, 
				       GrColor_t color0,
				       GrColor_t color1,  
				       GrTexChromakeyMode_t mode);
  
  ext = (char *)grGetString(GR_EXTENSION);
  
  if(!strstr(ext,"TEXCHROMA")) {
    log_perror(state,"Accelerator does not support TEXCHROMA extension");
    return(0);
  }
  
  grTexChromaModeExt = (int (FX_CALL *)(GrChipID_t tmu, 
					  GrTexChromakeyMode_t mode))
    grGetProcAddress("grTexChromaModeExt");
  grTexChromaRangeExt = (int (FX_CALL *)(GrChipID_t tmu, 
					   GrColor_t color0,
					   GrColor_t color1,
					   GrTexChromakeyMode_t mode))
    grGetProcAddress("grTexChromaRangeExt");
  
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

  setup_verts(state);

  if((buffer = (FxU16 *)malloc(64*64*2)) == NULL) {
    // fail
    log_message(state,"Could not allocate memory buffer for texture map");
  }

  // generate texture
  texture_gen(local_ptr, buffer);

  grChromakeyMode(GR_CHROMAKEY_DISABLE);
  (*grTexChromaModeExt)(local_ptr->curtmu, GR_TEXCHROMA_ENABLE_EXT);

  for(channel = 0; channel < 4; channel++) {
    for(color = 0; color < 6; color++) {
      
      switch(channel) {
      case 1:
	clearcolor = rgbacolor(state, 0x7f7f7fff);
	chromacolor = ((35*color)+8) << 24;
	chromacolor1 = (((35*color)+(color*4))+8) << 24;
	break;
      case 2:
	clearcolor = rgbacolor(state, 0x7f7f7fff);
	chromacolor = ((35*color)+16) << 16;
	chromacolor1 =(((35*color)+(color*10))+16) << 16;
	break;
      case 3:
	clearcolor = rgbacolor(state, 0x7f7f7fff);
	chromacolor = ((35*color)+34) << 8;
	chromacolor1 = (((35*color)+(color*14))+34) << 8;
	break;
      default:
	clearcolor = rgbacolor(state, 0x7f0000ff);
	chromacolor = (((35*color)) << 24) |(((35*color)) << 16) |
				(((35*color)) << 8);
	chromacolor1 = chromacolor;
	break;
      }

      // compensate for 565 texture
      chromacolor &= 0xf8fcf800;
      chromacolor1 = chromacolor1 |= 0x07030700;
      chromacolor = rgbacolor(state,chromacolor);
      chromacolor1 = rgbacolor(state,chromacolor1);

      grBufferClear(clearcolor,0x0000,0x00000000);
      
      (*grTexChromaRangeExt)(local_ptr->curtmu, chromacolor,chromacolor1,
			     GR_TEXCHROMARANGE_RGB_ALL_EXT);
      
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
      
      grTexFilterMode(local_ptr->curtmu,
		      GR_TEXTUREFILTER_POINT_SAMPLED,
		      GR_TEXTUREFILTER_POINT_SAMPLED );
      
      // Draw quad
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
	       GR_COMBINE_FUNCTION_LOCAL,
	       GR_COMBINE_FACTOR_NONE,
	       FXFALSE, FXFALSE );
  
  grTexClampMode(local_ptr->curtmu, 
		 GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

  grTexMipMapMode(local_ptr->curtmu,
		  GR_MIPMAP_NEAREST,
		  FXFALSE );

  grTexLodBiasValue(local_ptr->curtmu, 0.0f);

  grAlphaCombine(GR_COMBINE_FUNCTION_BLEND_OTHER,
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
}

void setup_verts(conform_state *state)
{
  test_data *local_ptr;

  local_ptr = (test_data *)state->local_data;

  local_ptr->verts[0].x = 18.0f;
  local_ptr->verts[0].y = 18.0f;
  local_ptr->verts[0].oow = 1.0f;
  
  local_ptr->verts[1].x = 18.0f + 64.0f;
  local_ptr->verts[1].y = 18.0f;
  local_ptr->verts[1].oow = 1.0f;

  local_ptr->verts[2].x = 18.0f + 64.0f;
  local_ptr->verts[2].y = 18.0f + 64.0f;
  local_ptr->verts[2].oow = 1.0f;

  local_ptr->verts[3].x = 18.0f;
  local_ptr->verts[3].y = 18.0f + 64.0f;
  local_ptr->verts[3].oow = 1.0f;
}

void texture_gen(test_data *local_ptr, FxU16 *buffer)
{
  int i;
  int x, y;
  FxU16 red, grn, blu;
  FxU16 color;

  local_ptr->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
  local_ptr->info.smallLodLog2 = GR_LOD_LOG2_64;
  local_ptr->info.largeLodLog2 = GR_LOD_LOG2_64;
  local_ptr->info.format = GR_TEXFMT_RGB_565;
  local_ptr->info.data = buffer;

  // setup vertex data for all TMUs
  for(i = 0; i < GRVERTEX_NUM_TMU; i++) {
    local_ptr->verts[0].tmuvtx[i].sow = 0.0f;
    local_ptr->verts[0].tmuvtx[i].tow =0.0f;
    local_ptr->verts[0].tmuvtx[i].oow = 1.0f;
    
    local_ptr->verts[1].tmuvtx[i].sow = 256.0f;
    local_ptr->verts[1].tmuvtx[i].tow = 0.0f;
    local_ptr->verts[1].tmuvtx[i].oow = 1.0f;
    
    local_ptr->verts[2].tmuvtx[i].sow = 256.0f;
    local_ptr->verts[2].tmuvtx[i].tow = 256.0f;
    local_ptr->verts[2].tmuvtx[i].oow = 1.0f;
    
    local_ptr->verts[3].tmuvtx[i].sow = 0.0f;
    local_ptr->verts[3].tmuvtx[i].tow = 256.0f;
    local_ptr->verts[3].tmuvtx[i].oow = 1.0f;
  }


  for(y = 0; y < 64; y++) {
    for(x = 0; x < 64; x++) {
      if(y < 16) {
	// grey ramp
	red = (FxU16)(((y*64)+x)/4);
	grn = (FxU16)(((y*64)+x)/4);
	blu = (FxU16)(((y*64)+x)/4);
      } else if (y < 32) {
	// red ramp
	red = (FxU16)((((y-16)*64)+x)/4);
	grn = 0;
	blu = 0;
      } else if (y < 48) {
	// green ramp
	red = 0;
	grn = (FxU16)((((y-32)*64)+x)/4);
	blu = 0;
      } else {
	// blue ramp
	red = 0;
	grn = 0;
	blu = (FxU16)((((y-48)*64)+x)/4);
      }

      color = ((red&0xf8)>>3)<<11;
      color |= ((grn&0xfc)>>2)<<5;
      color |= ((blu&0xf8)>>3);
      
      *buffer++ = color;
    }
  }
}


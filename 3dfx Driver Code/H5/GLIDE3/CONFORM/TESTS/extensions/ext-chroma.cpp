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

static char *test_name = "ext-chroma";
static char *test_description = "Test chroma range";

// local data
typedef struct {
  GrVertex verts[4];  // Temporary vertex storage
} test_data;

// Prototypes
void setup_prim(conform_state *state);
void color_gen(conform_state *state, int x, int y);

int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;

  state->test_name = test_name;
  state->test_description = test_description;

  if((state->local_data = malloc(sizeof(test_data))) == NULL) {
    // Could not allocate data for local memory
    log_perror(state,"Could not allocate memory for local data");
    return(0);
  } 

  local_ptr = (test_data *)state->local_data;

  // print out test dependent args
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
    ++local_argv;
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
  frame_name("ext-chroma",state);
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
  FxU32 color;
  int channel;
  int x, y;
  char *ext;
//  GrProc grChromaRangeModeExt(GrChromakeyMode_t mode);
//  GrProc grChromaRangeExt(GrColor_t color0, GrColor_t color1, FxU32 mode);
  int (FX_CALL  *grChromaRangeModeExt)(GrChromakeyMode_t mode);
  int (FX_CALL  *grChromaRangeExt)(GrColor_t color0, GrColor_t color1,
				    FxU32 mode);

  ext = (char *)grGetString(GR_EXTENSION);

  if(!strstr(ext,"CHROMARANGE")) {
    log_perror(state,"Accelerator does not support PALETTE6666 extension");
    return(0);
  }

  grChromaRangeModeExt = (int (FX_CALL *)(GrChromakeyMode_t mode))
    grGetProcAddress("grChromaRangeModeExt");
  grChromaRangeExt = (int (FX_CALL *)(GrColor_t color0,
					GrColor_t color1,
					FxU32 mode))
    grGetProcAddress("grChromaRangeExt");
  
  local_ptr = (test_data *)state->local_data;

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

  grChromakeyMode(GR_CHROMAKEY_DISABLE);
  (*grChromaRangeModeExt)(GR_CHROMARANGE_ENABLE_EXT);

  for(channel = 0; channel < 4; channel++) {
    for(color = 0; color < 8; color++) {
      
      switch(channel) {
      case 1:
	clearcolor = rgbacolor(state, 0x7f7f7fff);
	chromacolor = rgbacolor(state, ((35*color)+1) << 24);
	chromacolor1 = rgbacolor(state, (((35*color)+(color*2))+1) << 24);
	break;
      case 2:
	clearcolor = rgbacolor(state, 0x7f7f7fff);
	chromacolor = rgbacolor(state, ((35*color)+2) << 16);
	chromacolor1 = rgbacolor(state, (((35*color)+(color*5))+2) << 16);
	break;
      case 3:
	clearcolor = rgbacolor(state, 0x7f7f7fff);
	chromacolor = rgbacolor(state, ((35*color)+4) << 8);
	chromacolor1 = rgbacolor(state, (((35*color)+(color*7))+4) << 8);
	break;
      default:
	clearcolor = rgbacolor(state, 0x7f0000ff);
	chromacolor = rgbacolor(state, (((35*color)+7) << 24) |
				(((35*color)+7) << 16) |
				(((35*color)+7) << 8));
	chromacolor1 = chromacolor;
	break;
      }
      grBufferClear(clearcolor,0x0000,0x00000000);
      
      (*grChromaRangeExt)(chromacolor,chromacolor1,
			  GR_CHROMARANGE_RGB_ALL_EXT);
      
      for(y = 0; y < 64; y++) {
	for(x = 0; x < 64; x++) {
	  // Init vertex data
	  local_ptr->verts[0].x = 18.0f + (float)x;
	  local_ptr->verts[0].y = 18.0f + (float)y;
	  local_ptr->verts[1].x = 18.9f + (float)x;
	  local_ptr->verts[1].y = 18.0f + (float)y;
	  local_ptr->verts[2].x = 18.9f + (float)x;
	  local_ptr->verts[2].y = 18.9f + (float)y;
	  local_ptr->verts[3].x = 18.0f + (float)x;
	  local_ptr->verts[3].y = 18.9f + (float)y;
	  
	  color_gen(state, x, y);
	  
	  // Draw tiny 1 pixel square
	  grDrawTriangle(&(local_ptr->verts[0]), 
			 &(local_ptr->verts[1]), 
			 &(local_ptr->verts[2]));
	  
	  grDrawTriangle(&(local_ptr->verts[0]), 
			 &(local_ptr->verts[2]), 
			 &(local_ptr->verts[3]));
	}
      }
      
      end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
    }
  }

  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"    No test specific args\n");
}


void setup_prim(conform_state *state)
{
  test_data *local_ptr;

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

  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
		 GR_COMBINE_FACTOR_NONE,
		 GR_COMBINE_LOCAL_CONSTANT,  
		 GR_COMBINE_OTHER_NONE,
		 FXFALSE);        
  
  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
		 GR_COMBINE_FACTOR_NONE,
		 GR_COMBINE_LOCAL_CONSTANT,  
		 GR_COMBINE_OTHER_NONE,
		 FXFALSE);        
  
  grAlphaBlendFunction(GR_BLEND_ONE,
		       GR_BLEND_ZERO,
		       GR_BLEND_ONE,
		       GR_BLEND_ZERO);

  grAlphaTestReferenceValue(0x00);
  grAlphaTestFunction(GR_CMP_NOTEQUAL);
}

void color_gen(conform_state *state, int x, int y)
{
  FxU8 red, grn, blu, alph;
  FxU32 color;

  alph = (FxU8)(x*4);

  if(y < 16) {
    // grey ramp
    red = (FxU8)(((y*64)+x)/4);
    grn = (FxU8)(((y*64)+x)/4);
    blu = (FxU8)(((y*64)+x)/4);
  } else if (y < 32) {
    // red ramp
    red = (FxU8)((((y-16)*64)+x)/4);
    grn = 0;
    blu = 0;
  } else if (y < 48) {
    // green ramp
    red = 0;
    grn = (FxU8)((((y-32)*64)+x)/4);
    blu = 0;
  } else {
    // blue ramp
    red = 0;
    grn = 0;
    blu = (FxU8)((((y-48)*64)+x)/4);
  }

  color = ((FxU32)red)<<24;
  color |= ((FxU32)grn)<<16;
  color |= ((FxU32)blu)<<8;
  color |= (FxU32)alph;

  grConstantColorValue(rgbacolor(state,color));
}


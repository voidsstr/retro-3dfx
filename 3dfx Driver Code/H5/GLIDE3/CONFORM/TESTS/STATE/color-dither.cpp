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

static char *test_name = "color-dither";
static char *test_description = "Test dithering";

// local data
typedef struct {
  GrVertex verts[4];  // Temporary vertex storage
} test_data;

// Prototypes
void setup_prim(conform_state *state);

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
  frame_name("color-dither",state);
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
  int type;
  int ramp;
  float redmax, grnmax, blumax;

  local_ptr = (test_data *)state->local_data;

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

  for(type = 0; type < 3; type++) {
    for(ramp = 0; ramp < 4; ramp++) {

      grBufferClear(clearcolor,0x0000,0x00000000);
      
      // choose dither
      switch(type) {
      case 1:
	grDitherMode(GR_DITHER_2x2);
	break;
      case 2:
	grDitherMode(GR_DITHER_4x4);
	break;
      default:
	grDitherMode(GR_DITHER_DISABLE);
	break;
      }

      // Init vertex data
      local_ptr->verts[0].x = 22.0f;
      local_ptr->verts[0].y = 22.0f;
      local_ptr->verts[1].x = 22.0f + 64.0f;
      local_ptr->verts[1].y = 22.0f;
      local_ptr->verts[2].x = 22.0f + 64.0f;
      local_ptr->verts[2].y = 22.0f + 64.0f;
      local_ptr->verts[3].x = 22.0f;
      local_ptr->verts[3].y = 22.0f + 64.0f;
      
      // Init colors
      switch(ramp) {
      case 1:
	redmax = 255.0f;
	grnmax = 0.0f;
	blumax = 0.0f;
	break;
      case 2:
	redmax = 0.0f;
	grnmax = 255.0f;
	blumax = 0.0f;
	break;
      case 3:
	redmax = 0.0f;
	grnmax = 0.0f;
	blumax = 255.0f;
	break;
      default:
	redmax = 255.0f;
	grnmax = 255.0f;
	blumax = 255.0f;
	break;
      }
      
      local_ptr->verts[0].a = 255.0f;
      local_ptr->verts[0].r = 0.0f;
      local_ptr->verts[0].g = 0.0f;
      local_ptr->verts[0].b = 0.0f;
      local_ptr->verts[1].a = 255.0f;
      local_ptr->verts[1].r = 0.0f;
      local_ptr->verts[1].g = 0.0f;
      local_ptr->verts[1].b = 0.0f;
      local_ptr->verts[2].a = 255.0f;
      local_ptr->verts[2].r = redmax;
      local_ptr->verts[2].g = grnmax;
      local_ptr->verts[2].b = blumax;
      local_ptr->verts[3].a = 255.0f;
      local_ptr->verts[3].r = redmax;
      local_ptr->verts[3].g = grnmax;
      local_ptr->verts[3].b = blumax;
      
      
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
  grVertexLayout(GR_PARAM_RGB,  GR_VERTEX_R_OFFSET << 2, GR_PARAM_ENABLE);  
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
		 GR_COMBINE_LOCAL_ITERATED,  
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







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

static char *test_name = "depth-zbuffer";
static char *test_description = "Test basic Z buffering";

// local data
typedef struct {
  GrVertex verts[12];  // Temporary vertex storage
} test_data;

// Prototypes
void setup_prim(conform_state *state);
void setup_verts(conform_state *state);

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
    fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
    ++local_argv;
  }

  // need an aux buffer for depth buffering
  state->num_aux_bufs = 1;

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
  frame_name("depth-zbuffer",state);
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
  long depthrange[2];
  int function;

  local_ptr = (test_data *)state->local_data;
  
  // clear front buffer to black
  clearcolor = rgbacolor(state, 0x000000ff);
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
  
  setup_verts(state);
  
  setup_prim(state);
  
  // get depth range
  grGet(GR_ZDEPTH_MIN_MAX, 8, depthrange);

  for(function = 0; function < 12; function++) {
    // Setup Z buffering
    grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);

    // pick zbuffer function
    switch(function) {
    case 1:
      grDepthBufferFunction(GR_CMP_NEVER);
      // clear the buffer
      grDepthMask(FXTRUE);
      grBufferClear(clearcolor,0x0000,depthrange[1]);
      break;
    case 2:
      grDepthBufferFunction(GR_CMP_LESS);
      // clear the buffer
      grDepthMask(FXTRUE);
      grBufferClear(clearcolor,0x0000,depthrange[0]);
      break;
    case 3:
      grDepthBufferFunction(GR_CMP_EQUAL);
      // clear the buffer
      grDepthMask(FXTRUE);
      grBufferClear(clearcolor,0x0000,depthrange[0]);
      break;
    case 4:
      grDepthBufferFunction(GR_CMP_EQUAL);
      // clear the buffer
      grDepthMask(FXTRUE);
      grBufferClear(clearcolor,0x0000,depthrange[1]);
      break;
    case 5:
      grDepthBufferFunction(GR_CMP_LEQUAL);
      // clear the buffer
      grDepthMask(FXTRUE);
      grBufferClear(clearcolor,0x0000,depthrange[0]);
      break;
    case 6:
      grDepthBufferFunction(GR_CMP_GREATER);
      // clear the buffer
      grDepthMask(FXTRUE);
      grBufferClear(clearcolor,0x0000,depthrange[1]);
      break;
    case 7:
      grDepthBufferFunction(GR_CMP_GEQUAL);
      // clear the buffer
      grDepthMask(FXTRUE);
      grBufferClear(clearcolor,0x0000,depthrange[1]);
      break;
    case 8:
      grDepthBufferFunction(GR_CMP_NOTEQUAL);
      // clear the buffer
      grDepthMask(FXTRUE);
      grBufferClear(clearcolor,0x0000,depthrange[1]);
      break;
    case 9:
      grDepthBufferFunction(GR_CMP_NOTEQUAL);
      // clear the buffer
      grDepthMask(FXTRUE);
      grBufferClear(clearcolor,0x0000,depthrange[0]);
      break;
    case 10:
      grDepthBufferFunction(GR_CMP_NOTEQUAL);
      // clear the buffer
      grDepthMask(FXTRUE);
      grBufferClear(clearcolor,0x0000,depthrange[1]);
      grDepthMask(FXFALSE);
      break;
    case 11:
      grDepthBufferFunction(GR_CMP_NOTEQUAL);
      // clear the buffer
      grDepthMask(FXTRUE);
      grBufferClear(clearcolor,0x0000,depthrange[0]);
      grDepthMask(FXFALSE);
      break;
    default:
      grDepthBufferFunction(GR_CMP_ALWAYS);
      // clear the buffer
      grBufferClear(clearcolor,0x0000,depthrange[1]);
      break;
    }
    
    // Draw squares
    // Init primitive color to red
    grConstantColorValue(rgbacolor(state,0xff0000ff));
    
    grDrawTriangle(&(local_ptr->verts[0]), 
		   &(local_ptr->verts[1]), 
		   &(local_ptr->verts[2]));
    
    grDrawTriangle(&(local_ptr->verts[0]), 
		   &(local_ptr->verts[2]), 
		   &(local_ptr->verts[3]));
    
    // Init primitive color to green
    grConstantColorValue(rgbacolor(state,0x00ff00ff));
  
    grDrawTriangle(&(local_ptr->verts[4]), 
		   &(local_ptr->verts[5]), 
		   &(local_ptr->verts[6]));
    
    grDrawTriangle(&(local_ptr->verts[4]), 
		   &(local_ptr->verts[6]), 
		   &(local_ptr->verts[7]));

    // Init primitive color to blue
    grConstantColorValue(rgbacolor(state,0x0000ffff));
  
    grDrawTriangle(&(local_ptr->verts[8]), 
		   &(local_ptr->verts[9]), 
		   &(local_ptr->verts[10]));
    
    grDrawTriangle(&(local_ptr->verts[8]), 
		   &(local_ptr->verts[10]), 
		   &(local_ptr->verts[11]));
    
    
    end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
  }

  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -tmu <n>   texture unit to test\n");
}


void setup_prim(conform_state *state)
{
  test_data *local_ptr;

  local_ptr = (test_data *)state->local_data;

  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_Q , GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_Z, GR_VERTEX_OOZ_OFFSET << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_A,  GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE);  
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

void setup_verts(conform_state *state)
{
  test_data *local_ptr;
  long depthrange[2];
  float zsmall,zbig;

  local_ptr = (test_data *)state->local_data;

  // get depth range
  grGet(GR_ZDEPTH_MIN_MAX, 8, depthrange);

  // Init vertex data
  zbig = (float)depthrange[0];
  zsmall = 1.0f;

  // ramp
  local_ptr->verts[0].x = 18.0f;
  local_ptr->verts[0].y = 18.0f;
  local_ptr->verts[0].oow = 1.0f;
  local_ptr->verts[0].ooz = (1.0f/zbig)*65535.0f;
  local_ptr->verts[1].x = 18.0f + 64.0f;
  local_ptr->verts[1].y = 18.0f;
  local_ptr->verts[1].oow = 1.0f;
  local_ptr->verts[1].ooz = (1.0f/zsmall)*65535.0f;
  local_ptr->verts[2].x = 18.0f + 64.0f;
  local_ptr->verts[2].y = 18.0f + 64.0f;
  local_ptr->verts[2].oow = 1.0f;
  local_ptr->verts[2].ooz = (1.0f/zsmall)*65535.0f;
  local_ptr->verts[3].x = 18.0f;
  local_ptr->verts[3].y = 18.0f + 64.0f;
  local_ptr->verts[3].oow = 1.0f;
  local_ptr->verts[3].ooz = (1.0f/zbig)*65535.0f;

  // small
  local_ptr->verts[4].x = 18.0f;
  local_ptr->verts[4].y = 18.0f + 8.0f;
  local_ptr->verts[4].oow = 1.0f;
  local_ptr->verts[4].ooz = (1.0f/zsmall)*65535.0f;
  local_ptr->verts[5].x = 18.0f + 64.0f;
  local_ptr->verts[5].y = 18.0f + 8.0f;
  local_ptr->verts[5].oow = 1.0f;
  local_ptr->verts[5].ooz = (1.0f/zsmall)*65535.0f;
  local_ptr->verts[6].x = 18.0f + 64.0f;
  local_ptr->verts[6].y = 18.0f + 32.0f + 8.0f;
  local_ptr->verts[6].oow = 1.0f;
  local_ptr->verts[6].ooz = (1.0f/zsmall)*65535.0f;
  local_ptr->verts[7].x = 18.0f;
  local_ptr->verts[7].y = 18.0f + 32.0f + 8.0f;
  local_ptr->verts[7].oow = 1.0f;
  local_ptr->verts[7].ooz = (1.0f/zsmall)*65535.0f;

  // big
  local_ptr->verts[8].x = 18.0f;
  local_ptr->verts[8].y = 18.0f + 24.0f;
  local_ptr->verts[8].oow = 1.0f;
  local_ptr->verts[8].ooz = 0.0f;
  local_ptr->verts[9].x = 18.0f + 64.0f;
  local_ptr->verts[9].y = 18.0f + 24.0f;
  local_ptr->verts[9].oow = 1.0f;
  local_ptr->verts[9].ooz = 0.0f;
  local_ptr->verts[10].x = 18.0f + 64.0f;
  local_ptr->verts[10].y = 18.0f + 32.0f + 24.0f;
  local_ptr->verts[10].oow = 1.0f;
  local_ptr->verts[10].ooz = 0.0f;
  local_ptr->verts[11].x = 18.0f;
  local_ptr->verts[11].y = 18.0f + 32.0f + 24.0f;
  local_ptr->verts[11].oow = 1.0f;
  local_ptr->verts[11].ooz = 0.0f;
}


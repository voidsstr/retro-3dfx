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

static char *test_name = "alpha-buffer";
static char *test_description = "Test alpha blending with destination alpha";

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

  // need aux buffer for destination alpha
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
  frame_name("alpha-buffer",state);
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
  int src;
  int dst;
  GrAlphaBlendFnc_t sf;
  GrAlphaBlendFnc_t df;
  
  local_ptr = (test_data *)state->local_data;
  
  // clear front buffer to grey
  clearcolor = rgbacolor(state, 0x2f2f2fff);
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grColorMask(FXTRUE,FXTRUE);
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
  
  // test out alpha operations
  for(src = 0; src < 2; src++) {
    for(dst = 0; dst < 2; dst++) {
      grBufferClear(clearcolor,0xff,0x00000000);
      
      // Glide only supports ZERO and ONE for alpha
      switch(src) {
      case 1:
	sf = GR_BLEND_ONE;
	break;
      default:
	sf = GR_BLEND_ZERO;
	break;
      }
      
      // Glide only supports ZERO and ONE for alpha
      switch(dst) {
      case 1:
	df = GR_BLEND_ONE;
	break;
      default:
	df = GR_BLEND_ZERO;
	break;
      }
      
      // Draw destination square
      grAlphaBlendFunction(GR_BLEND_ONE,
			   GR_BLEND_ZERO,
			   GR_BLEND_ONE,
			   GR_BLEND_ZERO);
      
      grDrawTriangle(&(local_ptr->verts[0]), 
		     &(local_ptr->verts[1]), 
		     &(local_ptr->verts[2]));
      
      grDrawTriangle(&(local_ptr->verts[0]), 
		     &(local_ptr->verts[2]), 
		     &(local_ptr->verts[3]));
      
      // Draw source square
      grAlphaBlendFunction(GR_BLEND_ONE,
			   GR_BLEND_ZERO,
			   sf,
			   df);
      
      grDrawTriangle(&(local_ptr->verts[4]), 
		     &(local_ptr->verts[5]), 
		     &(local_ptr->verts[6]));
      
      grDrawTriangle(&(local_ptr->verts[4]), 
		     &(local_ptr->verts[6]), 
		     &(local_ptr->verts[7]));
      
      // make destination alpha visible by using it to blend color
      grAlphaBlendFunction(GR_BLEND_DST_ALPHA,
			   GR_BLEND_ZERO,
			   GR_BLEND_ZERO,
			   GR_BLEND_ONE);
      
      grDrawTriangle(&(local_ptr->verts[8]), 
		     &(local_ptr->verts[9]), 
		     &(local_ptr->verts[10]));
      
      grDrawTriangle(&(local_ptr->verts[8]), 
		     &(local_ptr->verts[10]), 
		     &(local_ptr->verts[11]));

      end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
    }
  }

  // check out color operations with destination alpha
  for(src = 0; src < 3; src++) {
    for(dst = 0; dst < 2; dst++) {
      grBufferClear(clearcolor,0xff,0x00000000);
      
      switch(src) {
      case 1:
	sf = GR_BLEND_ONE_MINUS_DST_ALPHA;
	break;
      case 2:
	sf = GR_BLEND_ALPHA_SATURATE;
	break;
      default:
	sf = GR_BLEND_DST_ALPHA;
	break;
      }
      
      switch(dst) {
      case 1:
	df = GR_BLEND_ONE_MINUS_DST_ALPHA;
	break;
      default:
	df = GR_BLEND_DST_ALPHA;
	break;
      }
      
      // Draw destination square
      grAlphaBlendFunction(GR_BLEND_ONE,
			   GR_BLEND_ZERO,
			   GR_BLEND_ONE,
			   GR_BLEND_ZERO);
      
      grDrawTriangle(&(local_ptr->verts[0]), 
		     &(local_ptr->verts[1]), 
		     &(local_ptr->verts[2]));
      
      grDrawTriangle(&(local_ptr->verts[0]), 
		     &(local_ptr->verts[2]), 
		     &(local_ptr->verts[3]));
      
      // Draw source square
      grAlphaBlendFunction(sf,
			   df,
			   GR_BLEND_ONE,
			   GR_BLEND_ZERO);
      
      grDrawTriangle(&(local_ptr->verts[4]), 
		     &(local_ptr->verts[5]), 
		     &(local_ptr->verts[6]));
      
      grDrawTriangle(&(local_ptr->verts[4]), 
		     &(local_ptr->verts[6]), 
		     &(local_ptr->verts[7]));
      
      end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
    }
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
		 GR_COMBINE_LOCAL_ITERATED,  
		 GR_COMBINE_OTHER_NONE,
		 FXFALSE);        

  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
		 GR_COMBINE_FACTOR_NONE,
		 GR_COMBINE_LOCAL_ITERATED,  
		 GR_COMBINE_OTHER_NONE,
		 FXFALSE);        

  grAlphaTestReferenceValue(0x00);
  grAlphaTestFunction(GR_CMP_NOTEQUAL);
}

void setup_verts(conform_state *state)
{
  test_data *local_ptr;

  local_ptr = (test_data *)state->local_data;

  // destination
  local_ptr->verts[0].x = 18.0f;
  local_ptr->verts[0].y = 18.0f;
  local_ptr->verts[0].oow = 1.0f;
  local_ptr->verts[0].a = 255.0f;
  local_ptr->verts[0].r = 255.0f;
  local_ptr->verts[0].g = 255.0f;
  local_ptr->verts[0].b = 255.0f;
  local_ptr->verts[1].x = 18.0f + 64.0f;
  local_ptr->verts[1].y = 18.0f;
  local_ptr->verts[1].oow = 1.0f;
  local_ptr->verts[1].a = 0.0f;
  local_ptr->verts[1].r = 255.0f;
  local_ptr->verts[1].g = 255.0f;
  local_ptr->verts[1].b = 255.0f;
  local_ptr->verts[2].x = 18.0f + 64.0f;
  local_ptr->verts[2].y = 18.0f + 64.0f;
  local_ptr->verts[2].oow = 1.0f;
  local_ptr->verts[2].a = 0.0f;
  local_ptr->verts[2].r = 0.0f;
  local_ptr->verts[2].g = 0.0f;
  local_ptr->verts[2].b = 0.0f;
  local_ptr->verts[3].x = 18.0f;
  local_ptr->verts[3].y = 18.0f + 64.0f;
  local_ptr->verts[3].oow = 1.0f;
  local_ptr->verts[3].a = 255.0f;
  local_ptr->verts[3].r = 0.0f;
  local_ptr->verts[3].g = 0.0f;
  local_ptr->verts[3].b = 0.0f;

  // source
  local_ptr->verts[4].x = 18.0f;
  local_ptr->verts[4].y = 18.0f;
  local_ptr->verts[4].oow = 1.0f;
  local_ptr->verts[4].a = 0.0f;
  local_ptr->verts[4].r = 0.0f;
  local_ptr->verts[4].g = 0.0f;
  local_ptr->verts[4].b = 0.0f;
  local_ptr->verts[5].x = 18.0f + 64.0f;
  local_ptr->verts[5].y = 18.0f;
  local_ptr->verts[5].oow = 1.0f;
  local_ptr->verts[5].a = 255.0f;
  local_ptr->verts[5].r = 0.0f;
  local_ptr->verts[5].g = 0.0f;
  local_ptr->verts[5].b = 0.0f;
  local_ptr->verts[6].x = 18.0f + 64.0f;
  local_ptr->verts[6].y = 18.0f + 64.0f;
  local_ptr->verts[6].oow = 1.0f;
  local_ptr->verts[6].a = 255.0f;
  local_ptr->verts[6].r = 255.0f;
  local_ptr->verts[6].g = 0.0f;
  local_ptr->verts[6].b = 0.0f;
  local_ptr->verts[7].x = 18.0f;
  local_ptr->verts[7].y = 18.0f + 64.0f;
  local_ptr->verts[7].oow = 1.0f;
  local_ptr->verts[7].a = 0.0f;
  local_ptr->verts[7].r = 255.0f;
  local_ptr->verts[7].g = 0.0f;
  local_ptr->verts[7].b = 0.0f;

  // show alpha
  local_ptr->verts[8].x = 18.0f;
  local_ptr->verts[8].y = 18.0f;
  local_ptr->verts[8].oow = 1.0f;
  local_ptr->verts[8].a = 255.0f;
  local_ptr->verts[8].r = 255.0f;
  local_ptr->verts[8].g = 255.0f;
  local_ptr->verts[8].b = 255.0f;
  local_ptr->verts[9].x = 18.0f + 64.0f;
  local_ptr->verts[9].y = 18.0f;
  local_ptr->verts[9].oow = 1.0f;
  local_ptr->verts[9].a = 255.0f;
  local_ptr->verts[9].r = 255.0f;
  local_ptr->verts[9].g = 255.0f;
  local_ptr->verts[9].b = 255.0f;
  local_ptr->verts[10].x = 18.0f + 64.0f;
  local_ptr->verts[10].y = 18.0f + 64.0f;
  local_ptr->verts[10].oow = 1.0f;
  local_ptr->verts[10].a = 255.0f;
  local_ptr->verts[10].r = 255.0f;
  local_ptr->verts[10].g = 255.0f;
  local_ptr->verts[10].b = 255.0f;
  local_ptr->verts[11].x = 18.0f;
  local_ptr->verts[11].y = 18.0f + 64.0f;
  local_ptr->verts[11].oow = 1.0f;
  local_ptr->verts[11].a = 255.0f;
  local_ptr->verts[11].r = 255.0f;
  local_ptr->verts[11].g = 255.0f;
  local_ptr->verts[11].b = 255.0f;
}

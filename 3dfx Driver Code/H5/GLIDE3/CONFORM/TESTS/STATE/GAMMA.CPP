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
#include <math.h>

static char *test_name = "gamma";
static char *test_description = "Test gr and gu versions of gamma correction";

#define MAX_GAMMA 256

// local data
typedef struct {
  GrVertex verts[4];  // Temporary vertex storage
  FxU32 red_gamma[MAX_GAMMA];
  FxU32 grn_gamma[MAX_GAMMA];
  FxU32 blu_gamma[MAX_GAMMA];
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

  // no aux buffer
  state->num_aux_bufs = 0;

  // force interactive, must visually verify
  state->interactive = 1;

  // use 100x100 image area
  state->conform_width = 100;
  state->conform_height = 100;

  // tiling not supported
  state->tile = 0;

  // setup image file names
  // this is manditory
  frame_name("gamma",state);
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
  int channel;
  int function;
  float gamma;
  float rgamma, ggamma, bgamma;
  int i;
  long num_gamma;
  long bits_gamma;
  double val;
  double range;

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
  
  for(function = 0; function < 2; function++) {
    for(type = 0; type < 5; type++) {
      for(channel = 0; channel < 4; channel++) {
	
	switch(type) {
	case 1:
	  gamma = 0.25f;
	  break;
	case 2:
	  gamma = 0.5f;
	  break;
	case 3:
	  gamma = 2.0f;
	  break;
	case 4:
	  gamma = 4.0f;
	  break;
	default:
	  gamma = 1.0f;
	  break;
	}
	
	switch(channel) {
	case 0:
	  rgamma = gamma;
	  bgamma = ggamma = 1.0f;
	  break;
	case 1:
	  ggamma = gamma;
	  rgamma = bgamma = 1.0f;
	  break;
	case 2:
	  bgamma = gamma;
	  rgamma = ggamma = 1.0f;
	  break;
	default:
	  rgamma = bgamma = ggamma = gamma;
	  break;
	}
	
	if(function) {
	  // get table params
	  grGet(GR_GAMMA_TABLE_ENTRIES, 4, &num_gamma);
	  grGet(GR_BITS_GAMMA, 4, &bits_gamma);

	  printf("num_entries = %ld, bits_gamma = %ld\n",
		 num_gamma, bits_gamma);

	  // white is white
	  local_ptr->red_gamma[num_gamma-1] = 255;
	  local_ptr->grn_gamma[num_gamma-1] = 255;
	  local_ptr->blu_gamma[num_gamma-1] = 255;
	  
	  // black is black
	  local_ptr->red_gamma[0] = 0;
	  local_ptr->grn_gamma[0] = 0;
	  local_ptr->blu_gamma[0] = 0;
	  
	  // build the gamma table
	  for(i = 1; i < (num_gamma-1); i++) {
	    val = ((double)i)/((double)num_gamma);
	    range = (double)((1<<bits_gamma)-1);
	    local_ptr->red_gamma[i] = (FxU32)(range*pow(val,(1.0/rgamma)));
	    local_ptr->grn_gamma[i] = (FxU32)(range*pow(val,(1.0/ggamma)));
	    local_ptr->blu_gamma[i] = (FxU32)(range*pow(val,(1.0/bgamma)));
	  }

	  // apply gamma using table
	  grLoadGammaTable(num_gamma,
			   local_ptr->red_gamma,
			   local_ptr->grn_gamma,
			   local_ptr->blu_gamma);
			   
	} else {
	  // apply gamma correction using floats
	  guGammaCorrectionRGB(rgamma, ggamma, bgamma);
	}
	
	// Draw square
	grDrawTriangle(&(local_ptr->verts[0]), 
		       &(local_ptr->verts[1]), 
		       &(local_ptr->verts[2]));
	
	grDrawTriangle(&(local_ptr->verts[0]), 
		       &(local_ptr->verts[2]), 
		       &(local_ptr->verts[3]));
	
	end_frame(END_OP_NOOP, state, NULL, NULL);
      }
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

  grAlphaBlendFunction(GR_BLEND_ONE,
		       GR_BLEND_ZERO,
		       GR_BLEND_ONE,
		       GR_BLEND_ZERO);

  grAlphaTestReferenceValue(0x00);
  grAlphaTestFunction(GR_CMP_ALWAYS);
}

void setup_verts(conform_state *state)
{
  test_data *local_ptr;

  local_ptr = (test_data *)state->local_data;

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
}

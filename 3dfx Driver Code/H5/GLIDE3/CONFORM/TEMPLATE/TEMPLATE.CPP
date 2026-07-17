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

static char *test_name = "T0001";
static char *test_description = "Test Template (sample conformance test)";

typedef struct {
  int test_pass;
  int cur_rep;
} test_data;

int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;
  int num_arg;
  char tmp_str[128];

  state->test_name = test_name;
  state->test_description = test_description;

  if((state->local_data = malloc(sizeof(test_data))) == NULL) {
    // Could not allocate data for local memory
    log_perror(state,"Could not allocate memory for local data");
    return(0);
  } 

  local_ptr = (test_data *)state->local_data;
  local_ptr->test_pass = 0;
  local_ptr->cur_rep = 0;

  // print out test dependent args
  num_arg = 0;
  while(--local_argc) {
    sprintf(tmp_str,"arg %d -> %s",num_arg++,*++local_argv);
    log_message(state,tmp_str);
  }

  // don't use any visuals with aux buffers
  state->num_aux_bufs = 0;

  // use 20x20 image area
  state->conform_width = 20;
  state->conform_height = 20;

  // setup image file names
  // this is manditory
  frame_name("template",state);
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
  char tmp_str[128];
  GrColor_t clearcolor;
  
  local_ptr = (test_data *)state->local_data;

  sprintf(tmp_str,"pass = %d sx= %d sy = %d wid = %d hei = %d",
	  local_ptr->test_pass++,
	  state->conform_x,
	  state->conform_y,
	  state->conform_width,
	  state->conform_height);
  log_message(state,tmp_str);

  // set color to green
  clearcolor = rgbacolor(state, 0x00ff0000);

  // clear 1/4 of front buffer to green
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grColorMask(FXTRUE,FXFALSE);
  grDepthMask(FXFALSE);
  grClipWindow(state->conform_x,
	       state->conform_y,
	       state->conform_x+9,
	       state->conform_y+9);

  grBufferClear(clearcolor,0x0000,0x00000000);
	       
  sprintf(tmp_str,"test data %d\n",local_ptr->cur_rep);
  add_data(state,tmp_str,strlen(tmp_str));
  local_ptr->cur_rep++;

  if(state->tile) {
    if(state->last_rep) {
      int x,y,width,height;

      // save conform width
      x = state->conform_x;
      y = state->conform_y;
      width = state->conform_width;
      height = state->conform_height;

      // set to full frame
      state->conform_x = 0;
      state->conform_y = 0;
      state->conform_width = state->screen_width;
      state->conform_height = state->screen_height;

      // save tiled frame
      end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);

      // restore conform width
      state->conform_x = x;
      state->conform_y = y;
      state->conform_width = width;
      state->conform_height = height;
    }
  } else {
    end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);
  }

  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"    No test specific args\n");
}







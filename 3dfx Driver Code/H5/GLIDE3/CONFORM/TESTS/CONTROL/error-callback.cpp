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

#include <math.h>

#include "conform.h"
#include "vertex.h"
#include "matrix.h"

static char *test_name = "error-callback";
static char *test_description = "Verify that grErrorSetCallback() works";

// Data specific to this test.
typedef struct {
  int test_pass;
} test_data;

int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;
  int num_arg;

  state->test_name = test_name;
  state->test_description = test_description;

  if((state->local_data = calloc(1, sizeof(test_data))) == NULL) {
    // Could not allocate data for local memory
    log_perror(state,"Could not allocate memory for local data");
    return(0);
  } 

  // Set some default local state
  local_ptr = (test_data *)state->local_data;
  local_ptr->test_pass = 0;

  // print out test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
    ++local_argv;
    ++num_arg;
  }

  state->tile = 0;
  state->num_reps = 1;

  if (state->num_color_bufs < 1) {
    state->num_color_bufs = 1; // need at least 1 color buf
  }

  // setup image file names
  // this is manditory
  char tmp_str[128];
  sprintf(tmp_str,"%s",test_name);

  frame_name(tmp_str,state);
  img_file_type(IMG_P6,state);

  return(-1);
}

void close_conform(conform_state *state)
{
  // Free up local data
  test_data *local_ptr = (test_data*)state->local_data;
  if (local_ptr) {
    free(local_ptr);
  }
}

static TestResult passed = TEST_FAIL;

void my_error_callback(const char *msg, FxBool fatal)
{
  // If we got here, we passed.
  passed = TEST_PASS;
}

int do_conform(conform_state *state)
{
  FxI32 num_boards = 0;

  test_data *local_ptr = (test_data*)state->local_data;

  local_ptr = (test_data *)state->local_data;

  // Install callback
  grErrorSetCallback(my_error_callback); 

  // Get the number of boards in the system. 
  grGet(GR_NUM_BOARDS, sizeof(num_boards), &num_boards);
  
  // Try to select one that doesn't exist. This should trigger the
  // error callback.

  grSstSelect(num_boards+10);

  log_status(state, passed, "error callback installation and testing"); 

  return(TEST_PASS); // ignored
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"    No test specific args\n");
}


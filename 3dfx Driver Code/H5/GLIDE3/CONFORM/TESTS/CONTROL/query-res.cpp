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

static char *test_name = "query-res";
static char *test_description = 
 "Make sure all resolutions returned by GrQueryReolutions() will open";

// Data specific to this test.
typedef struct {
  int test_pass;
} test_data;

static FxI32 res_list[] = {
  GR_RESOLUTION_320x200,
  GR_RESOLUTION_320x240,
  GR_RESOLUTION_400x256,
  GR_RESOLUTION_400x300,
  GR_RESOLUTION_512x384,
  GR_RESOLUTION_640x200,
  GR_RESOLUTION_640x350,
  GR_RESOLUTION_640x400,
  GR_RESOLUTION_640x480,
  GR_RESOLUTION_800x600,
  GR_RESOLUTION_960x720,
  GR_RESOLUTION_856x480,
  GR_RESOLUTION_512x256,
  GR_RESOLUTION_1024x768,
  GR_RESOLUTION_1280x1024,
  GR_RESOLUTION_1600x1200, 
};
#define NUM_RESOLUTIONS (sizeof(res_list)/sizeof(FxU32))

void make_printable(char *buf, GrResolution *res)
{
  char *res_name;
  char *refresh;

  switch(res->resolution) {
    case GR_RESOLUTION_320x200:
      res_name = "GR_RESOLUTION_320x200";
      break;
    case GR_RESOLUTION_320x240:
      res_name = "GR_RESOLUTION_320x240";
      break;
    case GR_RESOLUTION_400x256:
      res_name = "GR_RESOLUTION_400x256";
      break;
    case GR_RESOLUTION_400x300:
      res_name = "GR_RESOLUTION_400x300";
      break;
    case GR_RESOLUTION_512x384:
      res_name = "GR_RESOLUTION_512x384";
      break;
    case GR_RESOLUTION_640x200:
      res_name = "GR_RESOLUTION_640x200";
      break;
    case GR_RESOLUTION_640x350:
      res_name = "GR_RESOLUTION_640x350";
      break;
    case GR_RESOLUTION_640x400:
      res_name = "GR_RESOLUTION_640x400";
      break;
    case GR_RESOLUTION_640x480:
      res_name = "GR_RESOLUTION_640x480";
      break;
    case GR_RESOLUTION_800x600:
      res_name = "GR_RESOLUTION_800x600";
      break;
    case GR_RESOLUTION_960x720:
      res_name = "GR_RESOLUTION_960x720";
      break;
    case GR_RESOLUTION_856x480:
      res_name = "GR_RESOLUTION_856x480";
      break;
    case GR_RESOLUTION_512x256:
      res_name = "GR_RESOLUTION_512x256";
      break;
    case GR_RESOLUTION_1024x768:
      res_name = "GR_RESOLUTION_1024x768";
      break;
    case GR_RESOLUTION_1280x1024:
      res_name = "GR_RESOLUTION_1280x1024";
      break;
    case GR_RESOLUTION_1600x1200: 
      res_name = "GR_RESOLUTION_1600x1200";
    default:
      res_name = "UNKNOWN";
  }

  switch(res->refresh) {
    case GR_REFRESH_60Hz:
      refresh = "GR_REFRESH_60Hz";
      break;
    case GR_REFRESH_70Hz:
      refresh = "GR_REFRESH_70Hz";
      break;
    case GR_REFRESH_72Hz:
      refresh = "GR_REFRESH_72Hz";
      break;
    case GR_REFRESH_75Hz:
      refresh = "GR_REFRESH_75Hz";
      break;
    case GR_REFRESH_80Hz:
      refresh = "GR_REFRESH_80Hz";
      break;
    case GR_REFRESH_85Hz:
      refresh = "GR_REFRESH_85Hz";
      break;
    case GR_REFRESH_90Hz:
      refresh = "GR_REFRESH_90Hz";
      break;
    case GR_REFRESH_100Hz:
      refresh = "GR_REFRESH_100Hz";
      break;
    case GR_REFRESH_120Hz:
      refresh = "GR_REFRESH_120Hz";
      break;
    default:
      refresh = "UNKNOWN";
      break;
  }
  

  sprintf(buf, "%s / %s / %d / %d", 
          res_name,
          refresh,
          res->numColorBuffers,
          res->numAuxBuffers);
}

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

int do_conform(conform_state *state)
{
  char buf[256];
  char print_buf[128];
  TestResult status = TEST_PASS;
  int i;

  test_data *local_ptr = (test_data*)state->local_data;

  local_ptr = (test_data *)state->local_data;

  GrResolution	query;
  GrResolution	*list;
  int	list_size;

  /* find all possible modes */
  query.resolution	= GR_QUERY_ANY;
  query.refresh	        = GR_QUERY_ANY;
  query.numColorBuffers	= GR_QUERY_ANY;
  query.numAuxBuffers	= GR_QUERY_ANY;

  list_size = grQueryResolutions( &query, NULL );
  list = (GrResolution*)malloc( list_size );

  if (!list) {
    log_perror(state, "Couldn't alloc memory for query list!");
    return 0;
  }

  grQueryResolutions( &query, list );
  int num_entries = list_size / sizeof(query);

  for (i = 0; i < num_entries; i++) {

    if (state->context) { 
      grSstWinClose(state->context);
    }
    
    state->context = grSstWinOpen(state->window_handle,
                                list[i].resolution,
                                list[i].refresh,
                                GR_COLORFORMAT_RGBA,
                                GR_ORIGIN_UPPER_LEFT,
                                list[i].numColorBuffers,
                                list[i].numAuxBuffers);

    make_printable(print_buf, &list[i]);

    printf("%s\n", print_buf);

    sprintf(buf, "open list[%d], %s", i, print_buf);
    if (!log_status(state, 
                   (state->context? TEST_PASS: TEST_FAIL), 
                    buf)) {
      if (state->context) { 
        grSstWinClose(state->context);
      }
      return TEST_FAIL;
    }
  }
 
  if (state->context) { 
    grSstWinClose(state->context);
  }
 
  // Now reset the context to the way it was when we got called.
  state->context = grSstWinOpen(state->window_handle,
                                state->resolution,
                                state->refresh,
                                state->colorformat,
                                state->origin,
                                state->num_color_bufs,
                                state->num_aux_bufs);
  
  free(list);

  // Now do some filtering checks. Note that we ask for some 
  // configurations that aren't supported (invalid number of 
  // color or aux buffers). We should just get a 0-length list
  // back in this case. 
  
  for (int numColorBuffers = 0; numColorBuffers < 5; numColorBuffers++) {
    for (int numAuxBuffers = 0; numAuxBuffers < 3; numAuxBuffers++) {
      for (int res= 0; res< NUM_RESOLUTIONS; res++) {
        query.resolution = res_list[res];
        query.refresh	= GR_QUERY_ANY;
        query.numColorBuffers	= numColorBuffers;
        query.numAuxBuffers	= numAuxBuffers;

        list_size = grQueryResolutions( &query, NULL );

        if (list_size == 0) {
          continue;
        }

        list = (GrResolution*)malloc( list_size );
   
        if (!list) {
          log_perror(state, "Couldn't alloc memory for query list!");
          break;
        }

        grQueryResolutions( &query, list );
        num_entries = list_size / sizeof(query);

        for(i = 0; i < num_entries; i++) {
          status = TEST_PASS;
          if (list[i].resolution != res_list[res]) {
            sprintf(print_buf, "asked for res 0x%08X, but got 0x%08X",
                               query.resolution, list[i].resolution);
            log_message(state, print_buf);
            status = TEST_FAIL;
          }

          if (list[i].numColorBuffers != numColorBuffers) {
            sprintf(print_buf, "asked for %d color bufs, but got %d",
                               query.numColorBuffers, list[i].numColorBuffers);
            log_message(state, print_buf);
            status = TEST_FAIL;
          }
 
          if (list[i].numAuxBuffers != numAuxBuffers) {
            sprintf(print_buf, "asked for %d aux bufs, but got %d",
                               query.numAuxBuffers, list[i].numAuxBuffers);
            log_message(state, print_buf);
            status = TEST_FAIL;
          }

          if (!log_status(state, status, "query list check")) {
              return TEST_FAIL;
          }
        }
        free(list);
      }
    }
  }

  return(-1);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"    No test specific args\n");
}


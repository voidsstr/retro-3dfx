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
#include <sys/stat.h> // for 'stat'
#include <sys/types.h>

/*-----------------------------------------------------------------------------
@func main
@date 8/14/98
@arg int argc - argument count
@arg char ** argv - list of argument strings
@return int - 0 if test passes, != 0 if test fails
@imp
@key
@sect
@html
The main wrapper function for the conformance tests.  Opens the logfile,
selects the visual type, opens the hardware, inits and runs the conformance
test and then closes the hardware and logfile.
@end
-----------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
  conform_state *state;
  int i, local_argc;
  char *local_argv[MAX_LOCAL_ARGS];
  char *logfilename;
  char *output_dir;
  char *dbg_string;
  int dbg_level;
  int rep;
  int orig_argc;
  char **orig_argv;
  char *cur_arg;
  char tmp_str[256];
  struct stat stat_buf;

  if((state = (conform_state *)malloc(sizeof(conform_state))) == NULL) {
    // Could not allocate memory for conform state
    fprintf(stderr,"Could not allocate memory for conformance state\n");
    exit(0);
  }

  // Load any platform-specific function pointers
  init_platform_function_ptrs(state);

  // Locate output directory
  if((output_dir = getenv(CONFORM_OUTPUTDIRENV)) == NULL) {
    // Use local file 
    strcpy(state->output_dir, ".");
  }  else {
    strcpy(state->output_dir, output_dir);
  }

  if (stat(state->output_dir, &stat_buf) == -1) {
    fprintf(stderr, "Output directory '%s' does not exist.\n", state->output_dir);
    exit(0);
  }
 
  // Locate log file
  if((logfilename = getenv(CONFORM_LOGENV)) == NULL) {
    // Use local file 
    logfilename = "conform.log";
  } 

  // Prepend the output directory to the logfilename
  sprintf(tmp_str, "%s/%s", state->output_dir, logfilename);

  // Open logfile for append
  if((state->logfile = fopen(tmp_str,"at")) == NULL) {
    // could not open logfile
    fprintf(stderr,"Could not open logfile: %s\n", tmp_str);
    exit(0);
  }
  
  // Get debug level
  state->dbg_level = 0;
  if((dbg_string = getenv(CONFORM_DBGLEVEL)) != NULL) {
    // parse it
    if(sscanf(dbg_string,"%d",&dbg_level) == 1) {
      // copy to state
      state->dbg_level = dbg_level;
    }
  }

  // Save pointer to full arg list
  state->argc = argc;
  state->argv = argv;
  
  // Initial state
  state->cur_board = 0;
  state->seed = 1;
  state->conform_width = -1;
  state->conform_height = -1;
  state->conform_x = 0;
  state->conform_y = 0;
  state->tile = 0;
  state->num_reps = 1;
  state->dumpall = 0;
  state->save_images = 1;
  state->disable_dither = 1; // dither is OFF by default
  state->datahead = NULL; 
  state->datatail = NULL; 
  state->interactive = 0;
  state->argdepth = 0;
  state->numargs = 0;
  state->window_handle = 0;
  state->context = 0;
  state->use_surface_extensions = 0;
  state->surface_front = 0;
  state->surface_back = 0;
  state->surface_aux = 0;
  for (i = 0; i < CONFORM_MAX_TMUS; i++) {
    state->surface_textures[i] = NULL;
  }
  state->num_failures = 0;
  state->num_passes = 0;
  state->failure_limit = 10; // number of failures allowed before exiting
  state->xylimit = 0; 
  state->xycheck = 0;

  // display params completely random
  state->resolution = GR_QUERY_ANY;
  state->refresh = GR_QUERY_ANY;
  state->num_color_bufs = GR_QUERY_ANY;
  state->num_aux_bufs = GR_QUERY_ANY;
  state->origin = GR_QUERY_ANY;
  state->colorformat = GR_QUERY_ANY;

  // Process argument list
  local_argc = 1;
  local_argv[0] = argv[0];
  orig_argc = argc;
  orig_argv = argv;
  argv++;
  while((cur_arg = next_arg(state,&argc,&argv)) != NULL) {
    if(!_stricmp(cur_arg,"-h") ||
       !_stricmp(cur_arg,"/h") ||
       !_stricmp(cur_arg,"-help") ||
       !_stricmp(cur_arg,"/help")) {
      // process help
      fprintf(stderr,"command line arguments:\n");
      fprintf(stderr,"   -h           print help\n");
      fprintf(stderr,"   -bn <n>      board number\n");
      fprintf(stderr,"   -seed <n>    random number seed\n");

      fprintf(stderr,"   -res <n>     display resolution\n");
      fprintf(stderr,"   -cbuf <n>    number of color buffers\n");
      fprintf(stderr,"   -abuf <n>    number of aux buffers\n");
      fprintf(stderr,"   -orig <xx>   origin\n");
      fprintf(stderr,"          ul    upper left\n");
      fprintf(stderr,"          ll    lower left\n");
      fprintf(stderr,"   -cf <xxxx>   color format\n");
      fprintf(stderr,"        RGBA    \n");
      fprintf(stderr,"        ARGB    \n");
      fprintf(stderr,"        BGRA    \n");
      fprintf(stderr,"        ARGB    \n");
      fprintf(stderr,"   -wid <n>     active width\n");
      fprintf(stderr,"   -hei <n>     active height\n");
      fprintf(stderr,"   -tile        tile\n");
      fprintf(stderr,"   -n <n>       repeat count\n");
      fprintf(stderr,"   -dump        generate image of all frames\n");
      fprintf(stderr,"   -noimages    don't save any images to disk\n");
      fprintf(stderr,"   -dither      turn on HW dithering by default\n");
      fprintf(stderr,"   -i           interactive, wait for input after each frame.\n");
      fprintf(stderr,"   -rand        use random seed\n");
      fprintf(stderr,"   -maxfail <n> maximum allowed number of failures [%d]\n",
                     state->failure_limit);
      fprintf(stderr,"   -xylimit <n> allow conform window to be up to n pixels\n");
      fprintf(stderr,"                into the guardband.  A value of 0 (the default)\n");
      fprintf(stderr,"                means the conform window will be completely on\n");
      fprintf(stderr,"                screen. A positive value means that it can be\n");
      fprintf(stderr,"                up to that many pixels off the edge of the screen.\n");
      fprintf(stderr,"                A negative value means it can be as far offscreen\n");
      fprintf(stderr,"                as the hardware supports.\n");
      fprintf(stderr,"   -xycheck <n> add a border of <n> pixels to the conform window\n");
      fprintf(stderr,"                when checking pixels.\n");
      fprintf(stderr,"   -f <file>    read arguments from file\n");

      fprintf(stderr,"\n");

      // call local help
      help_conform(state);

      fprintf(stderr,"\n");

      fprintf(stderr,"    note: either \"-\" or \"/\" can preceed argument\n");

      exit(0);
    } else if(!_stricmp(cur_arg,"-bn") ||
	      !_stricmp(cur_arg,"/bn")) {
      // process board number
      cur_arg = next_arg(state,&argc,&argv);
      if(!sscanf(cur_arg,"%d",&(state->cur_board))) {
	// bad argument
	log_message(state, "No value specified for -bn");
      }

    } else if(!_stricmp(cur_arg,"-seed") ||
	      !_stricmp(cur_arg,"/seed")) {
      // process random number seed
      cur_arg = next_arg(state,&argc,&argv);
      if(!sscanf(cur_arg,"%d",&(state->seed))) {
	// bad argument
	log_message(state, "No value specified for -seed");
      }

    } else if(!_stricmp(cur_arg,"-res") ||
	      !_stricmp(cur_arg,"/res")) {
      // set resolution
      cur_arg = next_arg(state,&argc,&argv);
      string_to_res(cur_arg,state);

    } else if(!_stricmp(cur_arg,"-cbuf") ||
	      !_stricmp(cur_arg,"/cbuf")) {
      // number of color buffers
      cur_arg = next_arg(state,&argc,&argv);
      if(!sscanf(cur_arg,"%d",&(state->num_color_bufs))) {
	// bad argument
	log_message(state, "No value specified for -cbuf");
      }
    } else if(!_stricmp(cur_arg,"-abuf") ||
	      !_stricmp(cur_arg,"/abuf")) {
      // number of aux buffers
      cur_arg = next_arg(state,&argc,&argv);
      if(!sscanf(cur_arg,"%d",&(state->num_aux_bufs))) {
	// bad argument
	log_message(state, "No value specified for -abuf");
      }
    } else if(!_stricmp(cur_arg,"-orig") ||
	      !_stricmp(cur_arg,"/orig")) {
      // screen origin
      cur_arg = next_arg(state,&argc,&argv);
      if(!_stricmp(cur_arg,"ul")) {
	state->origin = GR_ORIGIN_UPPER_LEFT;
      } else if(!_stricmp(cur_arg,"ll")) {
	state->origin = GR_ORIGIN_LOWER_LEFT;
      } else {
	// bad argument
	log_message(state, "Bad value specified for -orig");
      }

    } else if(!_stricmp(cur_arg,"-cf") ||
	      !_stricmp(cur_arg,"/cf")) {
      // color ordering
      cur_arg = next_arg(state,&argc,&argv);
      string_to_order(cur_arg,state);
    } else if(!_stricmp(cur_arg,"-wid") ||
	      !_stricmp(cur_arg,"/wid")) {
      // test active width
      cur_arg = next_arg(state,&argc,&argv);
      if(!sscanf(cur_arg,"%d",&(state->conform_width))) {
	// bad argument
	log_message(state, "No value specified for -wid");
      }
    } else if(!_stricmp(cur_arg,"-hei") ||
	      !_stricmp(cur_arg,"/hei")) {
      // test active height
      cur_arg = next_arg(state,&argc,&argv);
      if(!sscanf(cur_arg,"%d",&(state->conform_height))) {
	// bad argument
	log_message(state, "No value specified for -hei");
      }
    } else if(!_stricmp(cur_arg,"-tile") ||
	      !_stricmp(cur_arg,"/tile")) {
      // enable tiling
      state->tile = 1;
    } else if(!_stricmp(cur_arg,"-n") ||
	      !_stricmp(cur_arg,"/n")) {
      // repeat count
      cur_arg = next_arg(state,&argc,&argv);
      if(!sscanf(cur_arg,"%d",&(state->num_reps))) {
	// bad argument
	log_message(state, "No value specified for -n");
      }
    } else if(!_stricmp(cur_arg,"-dump") ||
	      !_stricmp(cur_arg,"/dump")) {
      // dump frames
      state->dumpall = 1;
    } else if(!_stricmp(cur_arg,"-noimages") ||
	      !_stricmp(cur_arg,"/noimages")) {
      // don't save any images to disk
      state->save_images = 0;
    } else if(!_stricmp(cur_arg,"-dither") ||
	      !_stricmp(cur_arg,"/dither")) {
      // don't turn off HW dither before calling do_conform()
      state->disable_dither = 0;
    } else if(!_stricmp(cur_arg,"-i") ||
	      !_stricmp(cur_arg,"/i")) {
      // interactive mode
      state->interactive = 1;
    } else if(!_stricmp(cur_arg,"-rand") ||
	      !_stricmp(cur_arg,"/rand")) {
      // interactive mode
      state->seed = (unsigned int)time(NULL);
    } else if(!_stricmp(cur_arg,"-maxfail") ||
	      !_stricmp(cur_arg,"/maxfail")) {
      // max number of failures
      cur_arg = next_arg(state,&argc,&argv);
      if(!sscanf(cur_arg,"%d",&(state->failure_limit))) {
	// bad argument
	log_message(state, "No value specified for -maxfail");
      }
    } else if(!_stricmp(cur_arg,"-xycheck") ||
	      !_stricmp(cur_arg,"/xycheck")) {
      // max number of failures
      cur_arg = next_arg(state,&argc,&argv);
      if(!sscanf(cur_arg,"%d",&(state->xycheck))) {
	// bad argument
	log_message(state, "No value specified for -xycheck");
      }
    } else if(!_stricmp(cur_arg,"-xylimit") ||
	      !_stricmp(cur_arg,"/xylimit")) {
      // max number of failures
      cur_arg = next_arg(state,&argc,&argv);
      if(!sscanf(cur_arg,"%d",&(state->xylimit))) {
	// bad argument
	log_message(state, "No value specified for -xylimit");
      }
    } else {
      // pass unrecognized arguments thru to test
      if(local_argc < MAX_LOCAL_ARGS) {
	local_argv[local_argc] = cur_arg;
	local_argc++;
      } else {
	// Too many unrecognized arguments
	log_message(state, "Too many local arguments");
      }
    }
  }

  // Get initialization information
  grGlideInit();

  if(!grGet(GR_NUM_BOARDS, sizeof(long), &(state->num_boards))) {
    // call failed - system error
    log_message(state, "FAIL: Could not get GR_NUM_BOARDS from Glide state");
    grGlideShutdown(); // shut down glide
    exit(0);
  }

  if(state->num_boards == 0) {
    // no 3Dfx boards found
    log_message(state, "FAIL: No 3Dfx boards found");
    grGlideShutdown(); // shut down glide
    exit(0);
  }

  if(state->cur_board > (state->num_boards-1)) {
    // unsupported board
    log_message(state, "FAIL: User specified board number out of range");
    grGlideShutdown(); // shut down glide
    exit(0);
  }

  grSstSelect(state->cur_board);

  fill_in_static_state(state);

  // seed random number generator
  srand(state->seed);

  // Call init for test
  if(!init_conform(state, local_argc, local_argv)) {
    // Test initialization failed due to system error
    log_message(state, "FAIL: test initialization failed");
    grGlideShutdown(); // shut down glide
    exit(0);
  }

  // Open accelerator and set mode
  if(!set_mode(state)) {
    // Invalid mode
    log_message(state, "FAIL: Invalid display mode");
    grGlideShutdown(); // shut down glide
    exit(0);
  }

  // re-seed the random number generator, so that tests run on different
  // platforms start equally. (because set_mode(), above, can call the
  // random number generator different numbers of times on different
  // platforms).
  srand(state->seed);

  // Append test state
  log_test_state(state, orig_argc, orig_argv);

  // override reps to fill screen
  if(state->tile && (state->num_reps == 1)) {
    state->num_reps = state->screen_width/state->conform_width;
    state->num_reps *= state->screen_height/state->conform_height;
  }

  // repeat test required number of times
  for(rep=0;rep<state->num_reps;rep++) {
   
    if(rep == (state->num_reps-1)) {
      state->last_rep = 1;
    } else {
      state->last_rep = 0;
    }
    if(state->tile) { // tile test across screen

      if (state->disable_dither) { // turn off HW dither if applicable
        grDitherMode(GR_DITHER_DISABLE);
      }

      // call the test    
      do_conform(state);

      // bump to next tile
      if((state->conform_x + (2*state->conform_width))
	 <= state->screen_width) {
	// move tile left
	state->conform_x += state->conform_width;
      } else {
	// end of scanline, move tile down
	state->conform_x = 0;
	if((state->conform_y + (2*state->conform_height))
	   <= state->screen_height) {
	  // move tile down
	  state->conform_y += state->conform_height;
	} else {
	  // end of screen
	  state->conform_y = 0;
	}
      }
    } else { // repeat test without tiling

      // if xylimit is set, adjust the conform window
      if (state->xylimit > 0) { 

        // Choose a random location, placing the window so it can be
        // up to xylimit off the edges of the screen.
        // This means that the conform_window is within the rectangle
        // [-xylimit, -xylimit] to 
        // [screen_width + screen_width, screen_height + xylimit].
        int maxx = state->screen_width + 
                   (2 * state->xylimit) - state->conform_width;
        int maxy = state->screen_height + 
                   (2 * state->xylimit) - state->conform_height;

        if (maxx > 0) {
          state->conform_x = nrand(maxx) - state->xylimit;
        }
        if (maxy > 0) {
          state->conform_y = nrand(maxy) - state->xylimit;
        }
        // printf("(%d, %d)\n", state->conform_x, state->conform_y);

      } else if (state->xylimit < 0) {
        // TODO:
        // set the conform_window anywhere within the bounds of the
        // max hardware guardband. 
      }

      // Set the check_region to default to the conform window +/- the
      // xycheck value. Don't worry about clamping it yet. It'll be
      // clamped at vmgrGetPixels() time.

      state->check_region.min_x = state->conform_x - state->xycheck;
      state->check_region.min_y = state->conform_y - state->xycheck;
      state->check_region.max_x = 
        state->conform_x + state->conform_width + state->xycheck - 1;
      state->check_region.max_y = 
        state->conform_y + state->conform_height + state->xycheck - 1;

      if (state->disable_dither) { // turn off HW dither if applicable
        grDitherMode(GR_DITHER_DISABLE);
      }

      do_conform(state); // call the test
    }

    if (state->num_failures > state->failure_limit) {
      // too many failures, break out.
      break;
    }
  }

  // Report the test results.
  log_test_results(state);

  // cleanup file arguments
  clear_fileargs(state);

  // Call test cleanup
  close_conform(state);

  // Release context, if necessary
  if (state->context) {
    if (state->use_surface_extensions) {
      release_surfaces(state);
      surface_release_context(state, state->context);
    } else {
      // disable call to close this will get the gdEndTest function invoked and
      // the window closed from grGlideShutdown
      // grSstWinClose(state->context);
    }
  }

  // Destory Win32 Window
  destroy_window(state);

  // Write end to log
  log_end(state);

  // Close logfile
  fclose(state->logfile);

  // Free global state
  free((void *)state);

  // Shut down glide
  grGlideShutdown();
  
  return -1;
}


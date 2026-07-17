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

/*-----------------------------------------------------------------------------
@func log_perror
@date 8/14/98
@arg conform_state *state - current conformance state
@arg char *msg
@return none
@imp
@key
@sect
@html
Print a system error message to the conformance log
@end
-----------------------------------------------------------------------------*/
void log_perror(conform_state *state, char *msg)
{
  fprintf(state->logfile,"%s : SYS_ERROR %s %s\n",
	  state->test_name,
	  strerror(errno),
	  msg);
  fflush(state->logfile);
}

/*-----------------------------------------------------------------------------
@func log_message
@date 8/14/98
@arg conform_state *state - current conformance state
@arg char *msg
@return none
@imp
@key
@sect
@html
Print a message to the conformance log
@end
-----------------------------------------------------------------------------*/
void log_message(conform_state *state, char *msg)
{
  fprintf(state->logfile,"%s : MSG %s\n",
	  state->test_name,
	  msg);
  fflush(state->logfile);
}

/*-----------------------------------------------------------------------------
@func log_frame
@date 11/13/98
@arg conform_state *state - current conformance state
@arg char *msg
@return none
@imp
@key
@sect
@html
Print an end_frame message to the conformance log
@end
-----------------------------------------------------------------------------*/
void log_frame(conform_state *state, char *msg)
{
  fprintf(state->logfile,"%s : FRAME %s\n",
	  state->test_name,
	  msg);
  fflush(state->logfile);
}

/*-----------------------------------------------------------------------------
@func log_debug
@date 10/29/98
@arg conform_state *state - current conformance state
@arg int level - debug level associated with message
@arg char *msg
@return none
@imp
@key
@sect
@html
Print a debug message to the conformance log if the debug level is less than
the current debug level
@end
-----------------------------------------------------------------------------*/
void log_debug(conform_state *state, int level, char *msg)
{
  if(state->dbg_level > level) {
    fprintf(state->logfile,"%s : DBG %s\n",
	    state->test_name,
	    msg);
    fflush(state->logfile);
  }
}

/*-----------------------------------------------------------------------------
@func log_test_state
@date 8/14/98
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Print a start test message including data and time followed by a detailed
description of the hardware state to the conformance log
@end
-----------------------------------------------------------------------------*/
void log_test_state(conform_state *state, int argc, char **argv)
{
  char tmpstr[256];
  int argnum;
  char *curarg;
  time_t tloc;

  // date & time
  fprintf(state->logfile,"\n%s : START ",state->test_name);
  _tzset();
  time(&tloc);
  fprintf(state->logfile,"%s", ctime(&tloc)); // includes newline

  // description
  fprintf(state->logfile,"%s : DESCR   %s\n",state->test_name,
	 state->test_description);

  // hardware description
  fprintf(state->logfile,"%s : STATE   Glide %s %s\n",state->test_name,
	  state->version,
	  state->hardware);

  // board description
  fprintf(state->logfile,"%s : STATE   board %d of %ld\n",state->test_name,
	 state->cur_board, state->num_boards);

  // fb description
  fprintf(state->logfile,"%s : STATE   %ld frame buffers rev %ld memory %ld\n",
	  state->test_name,
	  state->num_fb, state->fb_revision, state->fb_mem);

  // tmu description
  fprintf(state->logfile,"%s : STATE   %ld texture units rev %ld memory %ld\n",
	  state->test_name,
	  state->num_tmu, state->tmu_revision, state->tmu_mem);

  // Current state
  fprintf(state->logfile,"%s : STATE   res ",
	  state->test_name);
  print_res(state);
  fprintf(state->logfile," refresh ");
  print_ref(state);
  fprintf(state->logfile," colorbuf %d auxbuf %d\n",
	  state->num_color_bufs, state->num_aux_bufs);
  fprintf(state->logfile,"%s : STATE   origin ",
	  state->test_name);
  print_origin(state);
  fprintf(state->logfile," format ");
  print_format(state);
  fprintf(state->logfile,"\n");

  // conform width and height 
  fprintf(state->logfile,"%s : STATE   conform width, height = %d, %d\n",
	  state->test_name,
          state->conform_width,
          state->conform_height);

  // random number seed
  fprintf(state->logfile,"%s : STATE   random number seed %ld\n",
	  state->test_name,
	  state->seed);

  // One line synopsis for easier log analysis.
  sprintf(tmpstr, "SYNOP   %s:%s:%d:%d:%ld:%ld:%ld:%ld:%ld:%ld:%X:%X:%d:%d:%X:%X:%d:%d:%ld",
          state->version, state->hardware,
          state->cur_board, state->num_boards,
          state->num_fb, state->fb_revision, state->fb_mem,
          state->num_tmu, state->tmu_revision, state->tmu_mem,
          state->resolution, state->refresh,
          state->num_color_bufs, state->num_aux_bufs,
          state->origin, state->colorformat,
          state->conform_width, state->conform_height,
          state->seed);

  fprintf(state->logfile,"%s : %s\n", state->test_name, tmpstr);

  // print out runtime arguments
  argnum = 0;
  argv++;
  while((curarg = next_arg(state,&argc,&argv)) != NULL) {
    fprintf(state->logfile,"%s : ARGS    [%d] = <%s>\n",
	    state->test_name,
	    ++argnum,curarg);
  }

  fflush(state->logfile);
}

/*-----------------------------------------------------------------------------
@func log_end
@date 8/14/98
@arg conform_state *state - current conformance state
@return none
@imp
@key
@sect
@html
Print a test end message including date and time to the conformance log
@end
-----------------------------------------------------------------------------*/
void log_end(conform_state *state)
{
  time_t tloc;

  // date & time
  fprintf(state->logfile,"%s : DONE ",state->test_name);
  _tzset();
  time(&tloc);
  fprintf(state->logfile,"%s ", ctime(&tloc));
  fflush(state->logfile);
}

/*-----------------------------------------------------------------------------
@func log_status
@date 03/05/99
@arg conform_state *state - current conformance state
@arg TestResult tr - the status of the frame test that we're logging
@arg char *msg - a string to print with this log entry
@return FXTRUE if we can continue, FXFALSE if we've exceeded our allowed
               number of failures and should stop.
@imp
@key
@sect
@html
Enter a PASS or FAIL message into the log, optionally report to stdout,
and check if we've passed the maximum allowed number of failures.
@end
-----------------------------------------------------------------------------*/
FxBool log_status(conform_state *state, TestResult tr, char *msg)
{
  FxBool status = FXTRUE;
  char buf[256];
  if (TEST_PASS == tr) {
    state->num_passes++;
    putchar ('.');
  } else {
    state->num_failures++;
    sprintf(buf, "FAIL: frame %d, %s", state->framecnt-1, msg);
    log_message(state, buf);
    printf("%s\n", buf);
    if (state->num_failures > state->failure_limit){
      status = FXFALSE;
    }
  }
  return status;
}

/*-----------------------------------------------------------------------------
@func log_test_results
@date 03/05/99
@arg conform_state *state - current conformance state
@return void
@imp
@key
@sect
@html
Report the final results of the test.
@end
-----------------------------------------------------------------------------*/
void log_test_results(conform_state *state)
{
  char buf[256];

  // if we were self-checking (we would report passes and failures)
  if ((state->num_passes > 0) || (state->num_failures > 0)) {
    sprintf(buf, "Test Completed: %s: -- Test reported %d passes and %d failures.",
            (state->num_failures? "FAIL" : "PASS"),
            state->num_passes,
            state->num_failures);
  } else { // we were image-based and no passes/failures were reported
    sprintf(buf, "Test Completed.");
  }

  log_message(state, buf);
  printf("\n%s\n", buf);
}

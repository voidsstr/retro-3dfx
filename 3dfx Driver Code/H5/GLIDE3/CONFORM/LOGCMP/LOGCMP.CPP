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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "logcmp.h"

Log testLog, refLog; // the two logs we'll be comparing

int Debug = 0; // set to 1 for verbose output


/*---------------------------------------------------------------------
@func dump_log
@arg Log *log - the log to dump
@html
A diagnostic routine to dump the contents of a Log object.
---------------------------------------------------------------------*/
void 
dump_log(Log *log)
{
  printf("Name        : %s\n", log->logName);
  printf("Directory   : %s\n", log->dirName);
  printf("NumTests    : %d\n", log->numTests);

  // Dump the list of tests...
  Test *ti = log->test;
  while (ti) {
    printf(" Test Name: %s\n", ti->name);
    printf(" NumFrames: %d\n", ti->numFrames);
    Frame *fi = ti->frame;
    while (fi) {
      printf("    0x%08X - %s\n", fi->checksum, fi->name);
      fi = fi->next;
    }
    ti = ti->next;
  }   
}


/*---------------------------------------------------------------------
@func free_log
@arg Log *log - the log to free
@html
Free all alloc'd memory contained in a Log object
---------------------------------------------------------------------*/
void
free_log(Log *log)
{
  Test *tip, *ti = log->lastTest;
  while (ti) { 
    tip = ti->prev; 
    Frame *fip, *fi = ti->frame;
    while (fi) {
      fip = fi->prev; 
      free(fi);
      fi = fip;
    }
    free(ti);
    ti = tip;
  }
}


/*---------------------------------------------------------------------
@func init_log
@arg Log *log - the log to initialize
@html
Initialize a Log object to zeros and set the default name of
the log to be "conform.log"
---------------------------------------------------------------------*/
void
init_log(Log *log, char* dirName)
{

   // Initialize a Logi structure
   memset(log, 0, sizeof(Log));
   strcpy(log->logName, "conform.log");
   strcpy(log->dirName, dirName);
}



/*---------------------------------------------------------------------
@func usage
@html
Print a message showing our usage.
---------------------------------------------------------------------*/
void usage(char *me) 
{
  char *ptr = &me[strlen(me)];
  while (*ptr != '\\') --ptr;
  ++ptr;
  
  printf("\n");
  printf("Usage: %s [-ia] [-help] test-directory reference-directory\n", ptr);
  printf(" Where test-directory and reference-directory are directories\n");
  printf(" containing a conform.log and a set of output images.\n");
  printf(" The test and frame entries in test-directory/conform.log will\n");
  printf(" be compared against entries in reference-directory/conform.log.\n");
  printf("  -help  - show this message\n");
  printf("  -ia    - when comparing two tests, ignore whether their command\n");
  printf("           line arguments were the same or not.\n");

  printf("\n");
}


/*---------------------------------------------------------------------
@func main
@arg int argc - the number of arguments being passed in
@arg char *argv[] - the array of arguments
@html
The main entrypoint
---------------------------------------------------------------------*/
int 
main(int argc, char *argv[])
{

  unsigned int flags = FLAGS_NONE;

  if (argc < 3) {
    usage(argv[0]);
    exit(-1);
  }

  char *dirName1 = argv[argc-2]; // the directories are always the last 
  char *dirName2 = argv[argc-1]; // two command line arguments

  ++argv;    // skip argv[0] (the test name)
  argc -= 2; // don't look at last two args (they're the dir names)
  while(--argc) {
    if(!_stricmp(*argv,"-help") ||      // help
              !_stricmp(*argv,"-h") ||
              !_stricmp(*argv,"/help") ||
              !_stricmp(*argv,"/h")) {
      usage(argv[0]);
      exit(-1);
    } else if(!_stricmp(*argv,"-ia") || // don't look at test args
              !_stricmp(*argv,"/ia")) {
       flags |= FLAGS_IGNORE_TEST_ARGS;
       ++argv;
    } else {
      fprintf(stderr, "Invalid argument '%s' - ignored.\n", *argv);
      ++argv;
    }
  }

  init_log(&testLog, dirName1);
  init_log(&refLog, dirName2);

  if (!parse_log(&testLog) ||
      !parse_log(&refLog)) {
    printf("Error: Couldn't read one or both logs - Stopping.\n");
    return 2;
  }

  compare_logs(flags, &testLog, &refLog);

  report(&testLog, &refLog);

  free_log(&testLog);
  free_log(&refLog);
  
  return 1; 
}

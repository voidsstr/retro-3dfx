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

/*----------------------------------------------------------------------
@func parse_log
@arg Log *log
@return  0 if couldn't open logs or out of memory, 1 otherwise
@html
Parse the contents of a conform.log file, creating a linked list
of tests contained in the log. Each Test contains a linked list
of Frames.
-----------------------------------------------------------------------*/
int
parse_log(Log *log)
{
   char tmpStr[512];
   char typeStr[64];

   // Try to open the file.
   sprintf(tmpStr, "%s/%s", log->dirName, log->logName);
   FILE *fp = fopen(tmpStr, "r");
   if (!fp) {
     printf("Couldn't open log: %s\n", tmpStr);
     return 0;
   }

   // Parse the log file
   char lineBuf[512];
   char lastTest[128];
   char thisTest[128];
   lastTest[0] = 0;

   int state = TEST_DONE;

   while (fgets(lineBuf, sizeof(lineBuf), fp)) {

     if ((lineBuf[0] == '\n') || 
         (lineBuf[0] == '\t')) {
       continue;
     }

     // Eg. color-combine : START
     sscanf(lineBuf, "%s : %s %s", thisTest, typeStr, tmpStr);

     if (!strcmp(typeStr, "START")) { // new test starting

       if (state != TEST_DONE) {
         fprintf(stderr, "Warning: Test %s <%s> did not finish properly!\n",
                 lastTest, (log->lastTest?log->lastTest->args:""));
       }
       strcpy(lastTest, thisTest);
       state = TEST_STARTED;
      
       // This is a new test starting. Add it to the linked list.
       Test *ti = (Test*) calloc(1, sizeof(Test));
       if (!ti) {
         fprintf(stderr, "Couldn't allocate Test block! - Aborting.\n");
         return 0;
       }

       // Initialize the new Test struct
       strcpy(ti->name, thisTest);
       ti->resWidth = 0; // TODO
       ti->resHeight = 0; // TODO
       ti->conformWidth = 0; // TODO
       ti->conformWidth = 0; // TODO
       ti->frame = NULL;
       ti->lastFrame = NULL;
       ti->numFrames = 0;
       ti->flags = 0;

       // Add it to the linked list of tests in the log
       ti->prev = log->lastTest;
       if (log->lastTest) {
         log->lastTest->next = ti;
       }
       log->lastTest = ti;
       ti->next = NULL;

       if (!log->test) { // first time through, set the test ptr
        log->test = ti;
       }

       ti->log = (void*) log; // pointer to containing log
 
       ++log->numTests;

     } else if (!strcmp(typeStr, "DONE")) {

       if (state != TEST_STARTED) {
         fprintf(stderr, "**Warning: Test %s did not start(?) properly!\n",
                 lastTest);
       }
       strcpy(thisTest, "Unknown");  
       strcpy(typeStr, "Unknown");  
       state = TEST_DONE;

     } else  if (!strcmp(typeStr, "FRAME")) { // Add a frame to this test.
       ASSERT(state == TEST_STARTED);

       // Create a new frame node for the list
       Frame *fi = (Frame*) calloc(1, sizeof(Frame));
       if (!fi) {
         fprintf(stderr, "Couldn't allocate Frame block! - Aborting.\n");
         return 0;
       }
       // Eg. ras-tri : FRAME checksum: 0x97E91354, image: ras-tri-aa_fb_03
       char *scan = strstr(lineBuf, "checksum: ");
       sscanf(scan, "checksum: 0x%X, image: %s\n",
              &fi->checksum, &fi->name);


       Test *ti = log->lastTest;

       // Add it to the linked list of tests in the log
       fi->prev = ti->lastFrame;
       if (ti->lastFrame) {
         ti->lastFrame->next = fi;
       }
       ti->lastFrame = fi;
       fi->next = NULL;

       if (!ti->frame) { // first time through, set the frame ptr
        ti->frame = fi;
       }
   
       fi->test = (void *) ti; // a pointer to the test that contains me

       ti->numFrames++;
       log->numFrames++;
 
     } else if (!strcmp(typeStr, "ARGS")) {   // Handle an ARGS line

       ASSERT(state == TEST_STARTED);

       char argBuf[32];
       int argNum;

       // Eg. simple-ras : ARGS    [1] = <-wc>
       char *scan = strstr(lineBuf, "ARGS");
       sscanf(scan, "ARGS [%d] = <%s>\n", &argNum, argBuf);

       // Bug in VC?  Remove trailing ">" from scanf'd argBuf
       if (argBuf[strlen(argBuf)-1] == '>') {
         argBuf[strlen(argBuf)-1] = 0;
       }

       // Add a space to it.
       argBuf[strlen(argBuf)] = ' ';
       argBuf[strlen(argBuf)+1] = 0;
 
       // Now add the argument to the current test's arg list.
       strcat(log->lastTest->args, argBuf); 

     } else if (!strcmp(typeStr, "STATE")) {   // Handle a STATE line

       ASSERT(state == TEST_STARTED);

       char stateBuf[80];

       // Eg. lfb : STATE   res 512x384 refresh 60Hz colorbuf 2 auxbuf 1
       char *scan = strstr(lineBuf, "STATE");
       sscanf(scan, "STATE  %s\n", stateBuf);
 
       // Now add the argument to the current test's arg list.
       strcat(log->lastTest->state, stateBuf); 

     } else if (!strcmp(typeStr, "SYNOP")) {

       ASSERT(state == TEST_STARTED);

       Test *ti = log->lastTest; // pointer to current test

       char *scan = strstr(lineBuf, "SYNOP");
       sscanf(scan, 
         "SYNOP   %s:%s:%d:%d:%ld:%ld:%ld:%ld:%ld:%ld:%X:%X:%d:%d:%X:%X:%d:%d:%ld",
          &ti->version, &ti->hardware,
          &ti->cur_board, &ti->num_boards,
	  &ti->num_fb, &ti->fb_revision, &ti->fb_mem,
	  &ti->num_tmu, &ti->tmu_revision, &ti->tmu_mem,
          &ti->resolution, &ti->refresh,
          &ti->num_color_bufs, &ti->num_aux_bufs,
          &ti->origin, &ti->colorformat,
          &ti->conform_width, &ti->conform_height,
          &ti->seed);

     } else if (!strcmp(typeStr, "MSG")) {  
        if (strstr(lineBuf, "FAIL")) { 
          log->numFailMessages++;
        }
     } else {
       // TODO: Process MSGs, files, etc.
     } 
   }

   return 1; 
}

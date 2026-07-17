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
#include <search.h>

#include "logcmp.h"


#define ABS(_d) ((_d)<0 ? -(_d) : (_d)) // absolute value


int
sort_compare_func(const void *elem1, const void *elem2)
{
    Frame *f1 = *(Frame**)elem1;
    Frame *f2 = *(Frame**)elem2;
    long delta = f1->aggregateColorDistance - f2->aggregateColorDistance;
    int rv = 0;

    if (delta > 0) {  
      rv = -1;
    } else if (delta < 0) {
      rv =  1;
    }

    return rv;
}

void
report_summary(Log *log)
{
  printf("Log: %s/%s\n", log->dirName, log->logName);
  printf("  # Tests           : %d\n", log->numTests);
  printf("  # Frames          : %d\n", log->numFrames);
  printf("  # Identical Frames: %d (%05.2f%%) [same checksums in both logs]\n", 
         log->numIdenticalFrames, 
         (float)log->numIdenticalFrames/(float)log->numFrames * 100.0f);
  printf("  # FAIL messages   : %d\n", log->numFailMessages);
  printf("\n");
}


void
report(Log *testLog, Log *refLog) 
{

  Test *testTest, *refTest;
  int i;
  Frame **framePtrList;
  int framePtrCount = 0;

  printf("\n");
  printf("Comparing %s/%s to (reference log) %s/%s:\n\n", 
         testLog->dirName, testLog->logName,
         refLog->dirName, refLog->logName);

  report_summary(testLog);  // show header info
  report_summary(refLog);   // show header info


  // Report the differences, if there are any.
  if (testLog->numFrames != testLog->numIdenticalFrames) {

    // Alloc space for a list of pointers to tests had differences.
    framePtrList = 
      (Frame**) calloc((testLog->numFrames - testLog->numIdenticalFrames),
                      sizeof(Frame*));
    if (!framePtrList) {
      printf("Error: Couldn't allocate memory for sorted frame list - exiting.\n");
      return;
    }

    // Walk through the tests, keeping a list of the ones that had
    // differences.
    testTest = testLog->test;
    while (testTest) {
      if (testTest->flags & FLAG_COMPARED_DIFF) { // a frame was different
        Frame *f = testTest->frame;
        while (f) {
          if (f->flags & FLAG_COMPARED_DIFF) { 
            framePtrList[framePtrCount++] = f; // add to the list
          }
          f = f->next;
        }
      } 
      testTest = testTest->next;
    }

    // Now sort and display the different frames in short form.
    if (framePtrCount && framePtrList) { 

      char *header = 
"\n"
"                     ###   Difference Summary   ###\n"
"\n"
"Definitions:\n"
" 'Total Diff'   is the accumulated total of distance between pixel colors.\n"
" 'Max Diff'     is the maximum color distance between two pixels.\n"
" '# Diff'       is the number of pixels that didn't match.\n"
" '# Less Black' is the number of non-zero value pixels in the test \n"
"                frame that were zero in the reference frame.\n"
" '# More Black' is the number of zero-value pixels in the test frame\n"
"                that were non-zero in the reference frame.\n"
"\n"
"   Total   |  Max   |  #     |# Less |# Extra|\n"
"   Diff    |  Diff  |  Diff  | Black | Black |    Frame Name\n"
"-----------+--------+--------+-------+-------+--------------------------";
 
      puts(header);

      // Sort the list in descending order of aggregate difference
      qsort(framePtrList, framePtrCount, sizeof(Frame*), sort_compare_func);


      for (i = 0; i < framePtrCount; i++) {
        Frame *f = framePtrList[i];
        printf(" %10lu %7d  %7d  %6d  %6d   %s\n", 
               f->aggregateColorDistance,
               f->maxColorDistance,
               f->numDiffPixels,
               f->numExtraColPixels,
               f->comparedTo->numExtraColPixels,
               f->name);
      }
    }
  }

  // Now report which tests were found only in one log or the other,
  // but not in both.

  Test **testPtrList = (Test**)framePtrList;
  int testPtrCount = 0;

  // First do the test log
  testTest = testLog->test;
  while(testTest) {
    if (!(testTest->flags & FLAG_IN_BOTH)) {
      testPtrList[testPtrCount++] = testTest;
    }
    testTest = testTest->next;
  }

  if (testPtrCount) {
    printf("\n## Tests found only in %s/%s:\n",
           testLog->dirName, testLog->logName);
    for (int i = 0; i < testPtrCount; i++) {   
      Test *test = testPtrList[i];
      printf(" %s <%s>\n", test->name, test->args);
    }
  }

  // Next the reference log
  testPtrCount = 0;
  refTest = refLog->test;
  while(refTest) {
    if (!(refTest->flags & FLAG_IN_BOTH)) {
      testPtrList[testPtrCount++] = refTest;
    }
    refTest = refTest->next;
  }

  if (testPtrCount) {
    printf("\n## Tests found only in %s/%s:\n",
           refLog->dirName, refLog->logName);
    for (int i = 0; i < testPtrCount; i++) {   
      Test *test = testPtrList[i];
      printf(" %s <%s>\n", test->name, test->args);
    }
  }

  // Free up the frame pointer list
  free(framePtrList);

}


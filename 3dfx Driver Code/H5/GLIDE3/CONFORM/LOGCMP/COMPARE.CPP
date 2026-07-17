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

#include <glide.h>

extern "C" {
#include <fximg.h> // we're dependent on fxmisc.lib
}

#ifdef EXTENDED_COMMENT

// This was taken from: 
//   http://ourworld.compuserve.com/homepages/compuphase/cmetric.htm
// and is reputed to be similar to, and slightly better than, a
// cie l*u*v* distance calculation.

typedef struct {
   unsigned char r, g, b;
} RGB;

long ColourDistance(RGB e1, RGB e2)
{
  long r,g,b;
  long rmean;

  rmean = ( (int)e1.r + (int)e2.r ) / 2;
  r = (int)e1.r - (int)e2.r;
  g = (int)e1.g - (int)e2.g;
  b = (int)e1.b - (int)e2.b;
  return (((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8);
}
#endif // EXTENDED_COMMENT


#define ABS(_x) ((_x) > 0 ? (_x) : -(_x))

/*----------------------------------------------------------------------
@func pixel_color_distance
@arg FxU32 pix1 - a BGRA format pixel
@arg FxU32 pix2 - a BGRA format pixel
@return the "distance" between pix1 and pix2 in a CIE/LUV-like colorspace
@html
Convert the two RGB pixels into a color space similar to CIE/LUV,
and the compute how far apart the two pixels are visually. The idea
is to see if we can come up with some sort of measurement that
indicates how dis-similar two pixels look on the screen.
-----------------------------------------------------------------------*/
long pixel_color_distance(FxU32 pix1, FxU32 pix2) 
{
  long r, g, b;
  long rmean;
  int r1, r2, g1, g2, b1, b2;

  // pixels are in BGRA format
  r1 = (pix1 >>  8) & 0xff;
  g1 = (pix1 >> 16) & 0xff;
  b1 = (pix1 >> 24) & 0xff;

  r2 = (pix2 >>  8) & 0xff;
  g2 = (pix2 >> 16) & 0xff;
  b2 = (pix2 >> 24) & 0xff;

  rmean = ( r1 + r2 ) / 2;
  r = r1 - r2;
  g = g1 - g2;
  b = b1 - b2;

  return ABS((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8));
 
}

/*----------------------------------------------------------------------
@func compare_images
@arg Frame *testFrame
@arg Frame *refFrame
@html
Given two Frames, do a pixel by pixel comparison.
-----------------------------------------------------------------------*/
void
compare_images(Frame *testFrame, Frame *refFrame)
{
  ImgInfo testInfo, refInfo;
  char testFileName[256];
  char refFileName[256];
  int maxedOut = 0;

  Test *testTest = (Test*)testFrame->test;
  Test *refTest = (Test*)refFrame->test;
  Log  *testLog = (Log*)testTest->log;
  Log  *refLog = (Log*)refTest->log;


  sprintf(testFileName, "%s/%s.ppm", testLog->dirName, testFrame->name);
  sprintf(refFileName, "%s/%s.ppm", refLog->dirName, refFrame->name);

  if (Debug) {
    printf("Comparing %s to %s\n", testFileName, refFileName);
  }

  imgReadFile((const char *)testFileName, &testInfo); 
  imgReadFile((const char *)refFileName, &refInfo); 

  FxU32 *testPixel, *refPixel;
  testPixel = (FxU32 *)testInfo.any.data; 
  refPixel = (FxU32 *)refInfo.any.data; 

  if ((testInfo.any.height != refInfo.any.height) ||
      (testInfo.any.width != refInfo.any.width)) {
    
    fprintf(stderr, "Images %s and %s are of different sizes.\n", testFileName, refFileName);
    free(testInfo.any.data); 
    free(refInfo.any.data); 
    return;
  }


  // Pixels are in BGRA format.
  int numPixels = testInfo.any.height * refInfo.any.width;
  testFrame->numPixels = numPixels;
  refFrame->numPixels = numPixels;

  for (int i = 0; i < numPixels; i++) {
    // Keep a count of non-black pixels.
    if (*testPixel != 0x0) { 
      testFrame->numColPixels++;
    }

    if (*refPixel != 0x0) { 
      refFrame->numColPixels++;
    }

    if (*testPixel != *refPixel) {
      refFrame->numDiffPixels++;
      testFrame->numDiffPixels++;
      if (*testPixel == 0x0) {
        refFrame->numExtraColPixels++; // different, other black
      } else if (*refPixel == 0x0) {
        testFrame->numExtraColPixels++; // different, other black
      } else {
        testFrame->numDiffColPixels++; // different, both not black
        refFrame->numDiffColPixels++;
      }

      // Now compute how different the colors are.
      long distance = pixel_color_distance(*testPixel, *refPixel);

      if (distance > testFrame->maxColorDistance) {
        testFrame->maxColorDistance = distance;
        refFrame->maxColorDistance = distance;
      }

      // Add to the total distance, if we haven't maxed it out yet...
      if (!maxedOut) {
        testFrame->aggregateColorDistance += distance;
        refFrame->aggregateColorDistance += distance;

        // Clamp the agg distance. (set to max if it wrapped)
        if(testFrame->aggregateColorDistance < 0) {
          refFrame->aggregateColorDistance  = 
            testFrame->aggregateColorDistance = 2147483647; // MAXINT
          maxedOut = 1;
        }
      }
    }

    ++testPixel;
    ++refPixel;
  }

  free(testInfo.any.data); 
  free(refInfo.any.data); 
}

/*----------------------------------------------------------------------
@func match_tests
@arg unsigned int flags - the flags to apply
@arg Test *testTest 
@arg Test *refTest
@return 1 if the tests match, 0 otherwise
@html
Given two Tests, return 1 if the two tests match. This means verifying
that they were run at the same resolution, with the same colorformat,
with the same origin, etc.
-----------------------------------------------------------------------*/
int 
match_tests(unsigned int flags, Test *testTest, Test *refTest)
{
  if (!strcmp(testTest->name, refTest->name)) {
     // test names are the same, make sure the arguments are, too.
     if ((flags & FLAGS_IGNORE_TEST_ARGS) || 
          !strcmp(testTest->args, refTest->args)) {
       // we're not looking at the args, or the arguments are the same, 
       // make sure some of the other state is, too.
       if ((testTest->resolution == refTest->resolution) &&
           (testTest->conform_width == refTest->conform_width) &&
           (testTest->conform_height == refTest->conform_height) &&
           (testTest->colorformat == refTest->colorformat) &&
           (testTest->origin == refTest->origin) &&
           (testTest->seed == refTest->seed)) {
         return 1;
       }
     } 
  }
  return 0;
}

/*----------------------------------------------------------------------
@func match_frames
@arg Frame *testFrame
@arg Frame *refFrame
@return 1 if the frames match, 0 otherwise
@html
Given two Frames, return 1 if the match. At the moment, all we do
is compare the names of the images.
-----------------------------------------------------------------------*/
int 
match_frames(Frame *testFrame, Frame *refFrame)
{
  if (!strcmp(testFrame->name, refFrame->name)) {
     // Is more robust checking needed?
     return 1;
  }
  return 0;
}

/*----------------------------------------------------------------------
@func find_frame_in_list
@arg Test *test
@arg Frame *frame
@return 1 if the frames match, 0 otherwise
@html
Walk through log 'log', looking for a test that matches the data
supplied in 'ti'. If a match is found, return a pointer to it.
-----------------------------------------------------------------------*/
Frame*
find_frame_in_list(Test *test, Frame *frame)
{
  Frame *myFi = test->frame;
  while (myFi) {
    if (match_frames(myFi, frame)) {
      return myFi;
    }
    myFi = myFi->next;
  }
  return NULL;
}


/*----------------------------------------------------------------------
@func compare_frame_lists
@arg Test *testTest
@arg Test *refTest
@html
Given two tests, walk through testTest's frame list comparing each to
their analog in refTest's frame list.
-----------------------------------------------------------------------*/
void
compare_frame_lists(Test *testTest, Test *refTest)
{
  Frame *testFrame = testTest->frame;
  Frame *refFrame = refTest->frame;
  while(testFrame) {

    if (refFrame = find_frame_in_list(refTest, testFrame)) {

      testFrame->comparedTo = refFrame; // keep a pointer to the other half
      refFrame->comparedTo = testFrame; // keep a pointer to the other half

      refFrame->flags |= FLAG_IN_BOTH; // mark as being in both logs
      testFrame->flags |= FLAG_IN_BOTH; // mark as being in both logs

      if (Debug) {
        printf(" %s: found in both logs.\n", testFrame->name);
      }

      if (testFrame->checksum == refFrame->checksum) {

        testFrame->flags |= FLAG_COMPARED_SAME;
        refFrame->flags |= FLAG_COMPARED_SAME;
        testTest->numIdenticalFrames++; 
        refTest->numIdenticalFrames++; 

      } else {

        testFrame->flags |= FLAG_COMPARED_DIFF;
        refFrame->flags |= FLAG_COMPARED_DIFF;
        testTest->flags |= FLAG_COMPARED_DIFF;
        refTest->flags |= FLAG_COMPARED_DIFF;
        compare_images(testFrame, refFrame);

      }

    } else {

      if (Debug) {
        printf(" %s: NOT found in both logs.\n", testFrame->name);
      }

    }
    testFrame = testFrame->next;
  }
}

/*----------------------------------------------------------------------
@func find_test_in_log
@arg unsigned int flags - the flags to apply
@arg Log *log
@arg Test *test
@return a pointer to a matching test if found, NULL otherwise.
@html
Given a Log and a Test, try to find a matching Test in Log. If
found, return a pointer to the matching test. Else, return NULL.
-----------------------------------------------------------------------*/
Test*
find_test_in_log(unsigned int flags, Log *log, Test *test)
{
  Test *myTi = log->test;
  while (myTi) {
    if (match_tests(flags, myTi, test)) {
      return myTi;
    }
    myTi = myTi->next;
  }
  return NULL;
}

/*----------------------------------------------------------------------
@func compare_logs
@arg unsigned int flags - the flags to apply
@arg Log *testLog
@arg Log *refLog
@html
Compare testLog to refLog by walking through them test by test and
frame by frame.
-----------------------------------------------------------------------*/
void
compare_logs(unsigned int flags, Log *testLog, Log *refLog)
{
  // Walk through testLog, looking for matching tests in refLog

  Test *testTest = testLog->test;
  Test *refTest;

  while(testTest) {
    if (refTest = find_test_in_log(flags, refLog, testTest)) {
      refTest->flags |= FLAG_IN_BOTH; // mark as being in both logs
      testTest->flags |= FLAG_IN_BOTH; // mark as being in both logs
      if (Debug) {
        printf("Found %s <%s> in both logs.\n", testTest->name, testTest->args);
      }
      compare_frame_lists(testTest, refTest);
      testLog->numIdenticalFrames += testTest->numIdenticalFrames;
      refLog->numIdenticalFrames += refTest->numIdenticalFrames;
    } else { 
    }
    testTest = testTest->next;
  }

}


#ifndef _LOGINFO_H_
#define _LOGINFO_H_

//  about a frame/image. One for each frame/image per test.
typedef struct _Frame {
   char name[256];  // image name
   long checksum;
   int flags;
   void *test; // the test that I belong to.
   int numColPixels;      // pixels that aren't black
   int numDiffPixels;     // pixels that differ
   int numDiffColPixels;  // pixels that aren't black, and differ
   int numExtraColPixels; // pixels that should be black, but aren't
   int numPixels;
   long maxColorDistance;       // worst pixel difference
   long aggregateColorDistance; // total pixel differences
   _Frame *prev, *next;
   _Frame *comparedTo;
} Frame;


//  about a test. One per test per log.
typedef struct _Test {
  char name[128]; // test name
  char args[128]; // test args
  char state[512]; // test state
  int resWidth, resHeight;
  int conformWidth, conformHeight;
  int numFrames;
  int numIdenticalFrames;
  int numArgs;
  int flags;

  // State
  char version[16];
  char hardware[32];
  int cur_board;
  long num_boards;
  long num_fb;
  long fb_revision;       // Pixelfx revision
  long fb_mem;            // frame buffer memory per Pixelfx
  long num_tmu;           // number of texture management units 
  long tmu_mem;           // texture memory per Texelfx
  long tmu_revision;      // Texelfx revision
  int resolution;    
  int refresh;            // accelerator display refresh rate
  int num_color_bufs;     // accelerator color buffers
  int num_aux_bufs;       // accelerator auxilary buffers
  int origin;             // accelerator display origin
  int colorformat;        // accelerator LFB colorformat 
  int conform_width;
  int conform_height;
  unsigned int seed;

  void *log;                // the log I belong to

  Frame *frame, *lastFrame; // my frames
  _Test *prev, *next;   

} Test;


//  for a log. One per log.
typedef struct {
  char logName[256];
  char dirName[256];
  int numTests;
  int numFrames;
  int numIdenticalFrames;
  int numFailMessages;
  Test *test, *lastTest;
} Log;


#define FLAG_NONE          0x0
#define FLAG_IN_BOTH       0x1
#define FLAG_COMPARED_DIFF 0x2
#define FLAG_COMPARED_SAME 0x4



#endif // not _LOGINFO_H_

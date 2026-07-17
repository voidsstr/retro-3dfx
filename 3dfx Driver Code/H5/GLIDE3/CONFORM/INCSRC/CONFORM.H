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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <glide.h>

// remove C++ name decoration
extern "C" {
#if defined(__unix__)
int _stricmp(const char *, const char *);
void _tzset(void);
#endif
#include <fximg.h>
}

#ifdef HWC_CSIM
# include <sst2ext.h>
#endif // HWC_CSIM

#ifdef NAPALM
# include <g3ext.h>
#endif

#include <vertex.h>

// Definitions

#define CONFORM_MAX_TMUS 8

#define MAX_LOCAL_ARGS 32
#define MAX_FILE_DEPTH 32
#define MAX_FILE_ARGS 64

#define MAX_NUM_COLOR_BUFS 3  // as per fxglide.h GR_MAX_COLOR_BUF
#define MAX_NUM_AUX_BUFS 1    // as per fxglide.h GR_MAX_AUX_BUF
#define MAX_ATTEMPTS 10

#define CONFORM_LOGENV       "GLIDE_CONFORM_LOGFILE"
#define CONFORM_OUTPUTDIRENV "GLIDE_CONFORM_OUTPUTDIR"
#define CONFORM_DBGLEVEL     "GCDBG_LEVEL"

#define END_OP_NOOP               0x00
#define END_OP_SWAP               0x01
#define END_OP_SAVE_TO_FILE      0x02
#define END_OP_SAVE_TO_MEM       0x04

#define END_BUFFER_FRONTBUFFER   0x01
#define END_BUFFER_BACKBUFFER    0x02
#define END_BUFFER_AUXBUFFER     0x04
#define END_BUFFER_DEPTHBUFFER   0x08
#define END_BUFFER_ALPHABUFFER   0x10
#define END_BUFFER_TRIPLEBUFFER  0x20
#define END_MAX_BUFFER_TYPE       6

extern int errno;

// frame data linked list 
typedef struct listnode {
  struct listnode *next;
  void *data;
  FxU32 size;
} listentry, *listptr;

typedef struct {            // state to be used by vm for self checking
  VertexInfo vtx_info;      // what vertex parameters are present
  struct {
    FxBool npt;             // texture is NPT
    GrAspectRatio_t aspect; // texture aspect ratio
  } tex_info[MAX_TMU];
  struct {
    FxI32 x, y;
    FxI32 width, height;
  } viewport;
  struct {
    FxFloat n, f;
  } depth_range;
  struct {
    FxU32 minx, miny, maxx, maxy;
  } clip_window;
} render_state;

typedef struct {
  void *pause;
  void *lfb_read;
  void *lfb_write;
  void *get_pixels;
} platform_func_ptrs;

// Conformance state
typedef struct {
  int argc;               // count of all args
  char **argv;            // pointer to all args

  FILE *logfile;          // logfile pointer
  int dbg_level;          // current debug level
  char *test_name;        // user supplied test name
  char *test_description; // user supplied test description
  void *local_data;       // test specific global data pointer

  long num_boards;        // number of 3Dfx cards in system
  int cur_board;          // current 3Dfx board

  long num_fb;            // number of frame buffers 
  long fb_revision;       // Pixelfx revision
  long fb_mem;            // frame buffer memory per Pixelfx
  long fb_total;          // frame buffer memory per board

  long num_tmu;           // number of texture management units 
  long tmu_revision;      // Texelfx revision
  long tex_align;         // texture alignment
  long max_tex_size;      // maximum texture size 
  long max_tex_aspect;    // maximum texture aspect ratio
  long tmu_mem;           // texture memory per Texelfx
  long tmu_total;         // texture memory per board

  long passthru;          // supports passthru

  unsigned int seed;      // random number seed

  const char *hardware;   // hardware type 
  const char *version;    // glide version

  int resolution;         // accelerator resolution
  int refresh;            // accelerator display refresh rate
  int num_color_bufs;     // accelerator color buffers
  int num_aux_bufs;       // accelerator auxilary buffers
  int origin;             // accelerator display origin
  int colorformat;        // accelerator LFB colorformat

  GrContext_t context;    // Current glide context

  int screen_width;       // display width
  int screen_height;      // display height

  int conform_width;      // test minimum width
  int conform_height;     // test minimum height
  int conform_x;          // test image start x
  int conform_y;          // test image start y

  int tile;               // use tiling
  int num_reps;           // number of times to run tests
  int last_rep;           // asserted during last repetition
  int interactive;        // wait for user input when appropriate

  char output_dir[256];   // path to output directory
  char frameroot[64];     // frame id string
  int framecnt;           // count of named frames;
  ImgType imagetype;      // image file format
  int dumpall;            // dump all frames for debug
  int save_images;        // save images to disk, or not
  int disable_dither;     // turn off HW dithering by default

  listptr datahead;       // head of frame data list
  listptr datatail;       // tail of frame data list (points to last block)

  FILE *argstack[MAX_FILE_DEPTH];  // argument file stack
  int argdepth;           // file recursion depth

  char *fileargs[MAX_FILE_ARGS];  // static storage for file arguments
  int numargs;            // number of file arguments

  FxU32 window_handle;    // window handle of the test (hWnd or X window ID)
  int use_surface_extensions; // use glide surface extensions instead of
                              // grSstWinOpen();
  void *surface_front;    // pointer to surface extension surface
  void *surface_back;     // pointer to surface extension surface
  void *surface_aux;      // pointer to surface extension surface
  void *surface_textures[CONFORM_MAX_TMUS];  // pointer to surface extension surfaces
  render_state rstate;
  int num_failures;      // the # of failures the current test has reported
  int num_passes;        // the # of passes the current test has reported
  int failure_limit;     // the maxmimum # of failures allowed before stopping
  int xycheck;           // border size to add to the conform window for checking
  int xylimit;           // border around screen that is valid for conform window
  BBox check_region;
  platform_func_ptrs pfp; // pointers to platform-specific functions
} conform_state;


// The type used to pass colors around the self-checking routines
typedef struct {
  double red;
  double green;
  double blue;
  double alpha;
} Color4D;

// Utility macros to convert to Color4D format.
#define RGBA8888_TO_Color4D(_rgba8888, _c4d) {                         \
  (_c4d)->red   = (double)(((_rgba8888) & 0xff000000) >> 24) / 255.0f; \
  (_c4d)->green = (double)(((_rgba8888) & 0x00ff0000) >> 16) / 255.0f; \
  (_c4d)->blue  = (double)(((_rgba8888) & 0x0000ff00) >> 8) / 255.0f;  \
  (_c4d)->alpha = (double)( (_rgba8888) & 0x000000ff) / 255.0f;        \
}

#define RGB565_TO_Color4D(_rgb565, _c4d) {                             \
  (_c4d)->red   = (double)(((_rgb565) & 0xf800) >> 11) / 31.0f;        \
  (_c4d)->green = (double)(((_rgb565) & 0x07e0) >> 5) / 63.0f;         \
  (_c4d)->blue  = (double)( (_rgb565) & 0x001f) / 31.0f;               \
}

typedef FxI32 TestResult;
#define TEST_PASS -1
#define TEST_FAIL 0

// Utility Prototypes: log.cpp
void log_perror(conform_state *state, char *msg);
void log_message(conform_state *state, char *msg);
void log_frame(conform_state *state, char *msg);
void log_debug(conform_state *state, int level, char *msg);
void log_test_state(conform_state *state, int argc, char **argv);
void log_end(conform_state *state);
FxBool log_status(conform_state *state, TestResult tr, char *msg);
void log_test_results(conform_state *state);
void print_res(conform_state *state);
void print_ref(conform_state *state);
void print_origin(conform_state *state);
void print_format(conform_state *state);

// Utility Prototypes: mode.cpp
void fill_in_static_state(conform_state *state);
int set_mode(conform_state *state);
void mode_to_res(conform_state *state);
void string_to_res(char *val, conform_state *state);
void string_to_order(char *val, conform_state *state);

// Utility Prototypes: utility.cpp
int nrand(int num);
char *next_arg(conform_state *state, int *argv, char ***argc);
void clear_fileargs(conform_state *state);
GrColor_t rgbacolor(conform_state *state, FxU32 color);
GrColor_t argbcolor(conform_state *state, FxU32 color);
GrColor_t bgracolor(conform_state *state, FxU32 color);
GrColor_t abgrcolor(conform_state *state, FxU32 color);
TestResult compare_color(conform_state *state, 
                         FxU32 screen, Color4D *expected);
void expand_native_buffer(conform_state *state, GrLfbInfo_t *linfo,  
                          void* buf, int width, int height);
void unpack_native_buffer(conform_state *state, FxU32* buf, int buflen);

// Utility Prototypes: image.cpp
void frame_name(char *name, conform_state *state);
void img_file_type(ImgType type, conform_state *state);
void end_frame(int op, conform_state *state, void **mem, int  buffer_mask);
FxBool save_image_to_file(conform_state *state, char *filename, 
                          FxU32 *data, int width, int height, 
                          FxBool unpackFlag);

// Utility Prototypes: data.cpp
void add_data(conform_state *state, void *buffer, FxU32 bufferlen);
void add_vertex_data(conform_state *state, void *verts, int num_verts);
void add_command_data(conform_state *state,
                      char *command_name, int argc, ... /* args */);

// Utility Prototypes: intrface.cpp
void init_platform_function_ptrs(conform_state *state);
void pause(conform_state *state);
void lfb_read_data(conform_state *state, FxU8 *memptr, FxU8 *lfbptr, int num_bytes);
void lfb_write_data(conform_state *state, FxU8 *lfbptr, FxU8 *memptr, int num_bytes);

// System-dependent Utility Prototypes: win32.cpp (or x11.cpp...)
FxU32 create_window(conform_state *state);
void destroy_window(conform_state *state);
void process_window_events(conform_state *state);
void wait_for_keypress(conform_state *state);
GrContext_t surface_create_context(conform_state *state);
void surface_release_context(conform_state *state, GrContext_t ctx);

// Platform-dependent surface prototypes: 
//  win32_ddsurf.cpp, or csim_surf.cpp, or xlib_surf.cpp ...
FxBool create_surfaces(conform_state *state);
FxBool create_texture_surface(conform_state *state, 
                              GrChipID_t tmu, GrTexInfo *texInfo);
void release_surfaces(conform_state *state);
FxBool get_surface_pixel(conform_state *state, void *src, 
                         FxU32 format, GrOriginLocation_t origin,
                         int x, int y, FxU32 *pixel);
FxBool blt_surface(conform_state *state, void *dst, void *src);

void* lock_surface(conform_state *state, void *surface);
void unlock_surface(conform_state *state, void *surface, void *ptr);


// Conformance Test Prototypes
int init_conform(conform_state *state, int local_argc, char **local_argv);
void close_conform(conform_state *state);
int do_conform(conform_state *state);
void help_conform(conform_state *state);

// CRC32 Checksum Utility Prototypes: checksum.cpp
unsigned int compute_checksum(unsigned char *bufp, int len);

// allocate a texture at a random address
FxU32 texture_base(conform_state *state, GrChipID_t tmu, FxU32 evenOdd, GrTexInfo *info);


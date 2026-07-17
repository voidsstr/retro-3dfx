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
#include "vmglide.h"

static char *test_name = "ras-point";
static char *test_description = "Test point rasterization, AA, and adjacency";


// Specify the pattern and color of points to draw
static char *color_data[] = {
  "GRRRRRRRRRRRRRRRRRRRG",
  "BRG   G       G   GRB",
  "BR  R   BBBBBG  G  RB",
  "BRG   G       G   GRB",
  "BGGGGGGGGGGGGGGGGGGGB",
  "BGGGG G R G B G GGGGB",
  "BGGG G G G G G G GGGB",
  "BGG G G GRGBG G G GGB",
  "BGG  R GBGRGBG B  GGB",
  "BGG G G GRGBG G G GGB",
  "BGG  B GRGBGRG R  GGB",
  "BGG G G GBGRG G G GGB",
  "BGGG G G G G G G GGGB",
  "BGGGG G R G B G GGGGB",
  "BGGGGGGGGGGGGGGGGGGGB",
  "BBG   G       G   GBB",
  "BB  G  GRRRRR   B  BB",
  "BBG   G       G   GBB",
  "GRRRRRRRRRRRRRRRRRRRG",
};
#define NUM_COLOR_PIXEL_ROWS (sizeof(color_data)/sizeof(char*))


// Specify the pattern and color of points to draw for the AA test
static char *aa_data[] = {
  "                   ",
  "                   ",
  "   R    G   B GR   ",
  "                   ",
  "                   ",
  "                   ",
  "   B    G   R GR   ",
  "                   ",
  "                   ",
  "   R    G   B GR   ",
  "                   ",
  "   R    G   B GR   ",
  "   G    B   R BG   ",
  "   R    G   B GRB  ",
  "              BGRB ",
  "              RGBR ",
};
#define NUM_AA_PIXEL_ROWS (sizeof(aa_data)/sizeof(char*))


// Data specific to this test.
typedef struct {
  GrVertex *verts; // pointer to all of our primitive verts
  int num_verts;             // number of vertices currently in use
  int do_aa;              // enable anti-aliasing
  int use_window_coords;     // use window coords instead of clip coords
  float dc_scale_x;          // 1 screen pixel in x, when in clip coords
  float dc_scale_y;          // 1 screen pixel in y, when in clip coords
  int num_pixel_rows;
  char **pixel_data;
  FxU32 *screen;
  int debug;
} test_data;

/**
 * Debug routine. Dump our verts to stdout.
 */
void dump_verts(int num_verts, GrVertex *verts) {
  for (int i = 0; i < num_verts; i++) {
    printf("  [%d](x,y,w) = (%6.3f, %6.3f, %6.3f)\n", i,
            verts[i].x,
            verts[i].y,
            verts[i].w);
  }
}

/*----------------------------------------------------------------
@func init_verts
@arg test_data *td - a pointer to our local_data structure (which holds
                     the vertices we'll be init'ing)
@html
 Initialize our point vertices.
@end
--------------------------------------------------------------------*/
void init_verts(conform_state *state, test_data *local_ptr) {
  float max_rgb;
  float alpha;
  GrVertex *verts;
  float cur_x_loc, cur_y_loc;
  int i;
  char *pix; 

  // Specify a vertex layout
  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_A,  GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_W,  GR_VERTEX_W_OFFSET << 2, GR_PARAM_ENABLE);  
  grVertexLayout(GR_PARAM_RGB,  GR_VERTEX_R_OFFSET << 2, GR_PARAM_ENABLE);  

  // Set RGB and alpha values for window or clip coords
  if (local_ptr->use_window_coords) {
    max_rgb = 255.0f;
    alpha   = 127.5f;
  } else {
    max_rgb = 1.0f;
    alpha   = 0.5f;
  }

  // If we're alpha blending, use full alpha. 
  if (local_ptr->do_aa) {
    alpha *= 2.0f;
  }

  // Compute how many vertices we have.
  if (!local_ptr->num_verts) {
    for (i = 0; i < local_ptr->num_pixel_rows; i++) {
      local_ptr->num_verts += strlen(local_ptr->pixel_data[i]);
    }
  }

  // Allocate storage for them. 
  if (!local_ptr->verts) {
    local_ptr->verts = verts = (GrVertex*)calloc(local_ptr->num_verts, sizeof(GrVertex));
    if (!verts) {
      log_perror(state, "Couldn't allocate storage for vertices!");
      return;
    }
  } else {
    // we've already alloc'd in a previous pass
    verts = local_ptr->verts;
  }

  // Get the width of the pixel data we're using. (assume a rectangle)
  int data_width = strlen(local_ptr->pixel_data[0]);

  // Lay out the points, based on the image data. Start putting points
  // at the origin of the window we were given.
  if (local_ptr->use_window_coords) {
    cur_y_loc = (float)state->conform_y;
  } else {
    cur_y_loc = -1.0f;
  }

  // Lay out a horizontal row
  for (i = 0; i < local_ptr->num_pixel_rows; i++) { // for each row
    pix = local_ptr->pixel_data[i];
  
    if (local_ptr->use_window_coords) {
      cur_x_loc = (float)state->conform_x;
    } else {
      cur_x_loc = -1.0f;
    }

    while(*pix) {  // for each char in the row

      verts->x = cur_x_loc;
      verts->y = cur_y_loc;
      verts->a = alpha;
      verts->w = 1.0f;

      // choose color
      switch(*pix) {
        case 'G': verts->r = 0.0f; verts->g = max_rgb; verts->b = 0.0f; break;
        case 'R': verts->r = max_rgb; verts->g = 0.0f; verts->b = 0.0f; break;
        case 'B': verts->r = 0.0f; verts->g = 0.0f; verts->b = max_rgb; break;

        case ' ':  
        default: verts->r = 0.0f; verts->g = 0.0f; verts->b = 0.0f; break;
      }
     
      cur_x_loc += local_ptr->dc_scale_x;
      ++verts;
      ++pix;
    }
    cur_y_loc += local_ptr->dc_scale_y;
  }
}

/*----------------------------------------------------------------
@func save_debug_image
@date 3/12/99
@arg conform_state *state - the framework state
@html
Save a ppm image of local_ptr->screen to disk.
@end
----------------------------------------------------------------*/
void save_debug_image(conform_state *state)
{
  char filename[64];
  test_data *local_ptr = (test_data *)state->local_data;
  int width = state->check_region.max_x - state->check_region.min_x + 1;
  int height = state->check_region.max_y - state->check_region.min_y + 1;
  sprintf(filename, "%s/%s-%02d-dbg", 
          state->output_dir, test_name, state->framecnt-1);
  save_image_to_file(state, 
                     filename,
                     local_ptr->screen, 
                     width, height,
                     FXTRUE);
}
/*----------------------------------------------------------------
@func check_frame
@arg conform_state *state - the framework state
@return TestResult - TEST_PASS if the frame is ok, TEST_FAIL otherwise
@html
Read in a region of the screen and check that we've drawn what we 
think we should have.
@end
--------------------------------------------------------------------*/
TestResult check_frame(conform_state *state)
{
  test_data *local_ptr = (test_data *)state->local_data;
  TestResult status = TEST_PASS;
  FxU32 * screen = local_ptr->screen;
  int x, y;
  char outstr[128];

  end_frame(END_OP_NOOP, state, (void **)&local_ptr->screen, 
            END_BUFFER_FRONTBUFFER);

  // Get the pixels in the check_region. Since we're passing in a NULL
  // BBox pointer to vmgrGetPixels(), it'll use the one on the conform_state
  // (and adjust it, if necessary).

  if (!vmgrGetPixels(state, local_ptr->screen, GR_BUFFER_FRONTBUFFER, NULL)) {
    log_message(state, "Couldn't read color buffer");
    return TEST_FAIL;
  }

  BBox *pBox = &state->check_region; // updated in vmgrGetPixels()

 // the dimensions of the region we're checking
  int width  = pBox->max_x - pBox->min_x + 1; 
  int height = pBox->max_y - pBox->min_y + 1;

  // Now we need to figure out where our pixels should be. 

  // If the check_region was entirely onscreen, then the origin of our
  // pixels would be at (xycheck, xycheck) in the image we were handed.
  int xOffset = state->xycheck;
  int yOffset = state->xycheck;
  int checkWidth = state->conform_width + (2 * state->xycheck);
  int checkHeight = state->conform_height + (2 * state->xycheck);

  // But, if the check_region was partially offscreen, we need to
  // figure out which side was clipped and adjust our origin
  // accordingly. 
  if (width < checkWidth) {
    // Okay, we know the check_region was clipped along x. Now we need to
    // know which side it was clipped on.
    if (state->conform_x < state->xycheck) {
      // we are clipped on the left hand side.
      xOffset = state->conform_x;
     }
  }

  // do the same for y
  if (height < checkHeight) {
    if (state->conform_y < state->xycheck) {
      // we are clipped on the top side.
      yOffset = state->conform_y;
    }
  }

  // the dimensions of the array of chars that specify the pixels we drew
  int num_rows = local_ptr->num_pixel_rows;
  int num_cols = local_ptr->num_verts/num_rows;

  int data_x, data_y;

  for ( y = 0; y < height; y++ ) {
    for ( x = 0; x < width; x++ ) {
      FxU32 *screen_pix = &screen[y*width+x];
      if (*screen_pix) {
        data_x = x - xOffset;
        data_y = y - yOffset;
        if ((data_x >= num_cols) || (data_y >= num_rows) ||
            (data_x < 0) || (data_y < 0)) { // outside bounds of data
          status = TEST_FAIL;
          sprintf(outstr, "pixel @ d(%d, %d) s(%d, %d) [0x%x] should not be lit", 
                  data_x, data_y, x, y, *screen_pix);
          log_message(state,outstr);
        } else { // check that it's the right color.
          // make sure we're only looking at the on screen portion.
          if ((data_x >= 0) && (data_y >= 0) && 
              (data_x < num_cols) && (data_y < num_rows)) {
            FxU8 screen_data = local_ptr->pixel_data[data_y][data_x];
            Color4D expected_color = {0.0, 0.0, 0.0, 0.0};
            switch (screen_data) {
              case 'R': 
                expected_color.red   = 0.5; // red, half alpha     
                break;
              case 'G': 
                expected_color.green = 0.5; // green, half alpha     
                break;
              case 'B': 
                expected_color.blue  = 0.5; // blue, half alpha     
                break;
              default:
                break;
            }
            if (TEST_FAIL == compare_color(state, *screen_pix, &expected_color)) {
              status = TEST_FAIL;
              sprintf(outstr, "pixel @ d(%d, %d) s(%d, %d) [0x%x] should be '%c'", 
                      data_x, data_y, x, y, *screen_pix, screen_data);
              if (local_ptr->debug) {
                *screen_pix = 0xffffffff; // bad pixels go white
              }
              
              log_message(state,outstr);
            } else {
              if (local_ptr->debug) {
                *screen_pix = 0x000003ff; // good pixels are cyan-ish
              }
            }
          }
        }
      }
    }
  }

  if (local_ptr->debug) {
    save_debug_image(state);
  }

  return status;
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
  local_ptr->num_verts = 0;
  local_ptr->verts = NULL;
  local_ptr->do_aa = 0; // aa off by default
  local_ptr->use_window_coords = 0; // use clip coords by default
  local_ptr->dc_scale_x = 1.0f;
  local_ptr->dc_scale_y = 1.0f;
  local_ptr->pixel_data = color_data;
  local_ptr->num_pixel_rows = NUM_COLOR_PIXEL_ROWS;
  local_ptr->debug = 0;
  local_ptr->screen = NULL;

  // print out test dependent args
  num_arg = 0;
  ++local_argv; // skip argv[0] (the test name)
  while(--local_argc) {
    if(!_stricmp(*local_argv,"-aa") || // anti aliasing 
       !_stricmp(*local_argv,"/aa")) {
      local_ptr->do_aa = 1;
      local_ptr->pixel_data = aa_data;
      local_ptr->num_pixel_rows = NUM_AA_PIXEL_ROWS;
    } else if(!_stricmp(*local_argv,"-wc") || // use window coords
              !_stricmp(*local_argv,"/wc")) {
      local_ptr->use_window_coords = 1;
    } else if(!_stricmp(*local_argv,"-debug") || // enable debugging output
              !_stricmp(*local_argv,"/debug")) {
      local_ptr->debug = 1;
    } else {
      fprintf(stderr, "Invalid argument '%s' - ignored.\n", *local_argv);
      ++local_argv;
      ++num_arg;
    }
    ++local_argv;
    ++num_arg;
  }

  // don't use any visuals with aux buffers
  state->num_aux_bufs = 0;
  
  // we need at least one color buffer
  if (state->num_color_bufs < 1) {
    state->num_color_bufs = 1;
  }

  // tiling or multiple reps not supported
  state->tile = 0;
  state->num_reps = 100;

  // setup image file names
  // this is manditory
  char tmp_str[128];
  sprintf(tmp_str,"%s%s%s", 
          test_name,
          (local_ptr->use_window_coords? "-wc" : ""),
          (local_ptr->do_aa? "-aa" : ""));

  frame_name(tmp_str,state);
  img_file_type(IMG_P6,state);

  return(-1);
}

void close_conform(conform_state *state)
{
  // Free up local data
  test_data *local_ptr = (test_data*)state->local_data;
  if (local_ptr) {
    if (local_ptr->verts) {  
      free(local_ptr->verts);
    }

    if(local_ptr->screen != NULL ) {
      free(local_ptr->screen);
    }
    free(local_ptr);
  }
}

int do_conform(conform_state *state)
{
  test_data *local_ptr;
  FxFloat    vnear = 0.f, vfar = 1.f;
  int v = 0;

  local_ptr = (test_data *)state->local_data;

  // allocate screen for automated compare
  if(local_ptr->screen == NULL ) {
      if((local_ptr->screen = 
          (FxU32 *)malloc(state->screen_width*state->screen_height*sizeof(FxU32))) == NULL) {
        // fail
        log_message(state,"Could not allocate memory buffer for screen image");
      }
  }

  // First, clear the whole window
  vmgrClipWindow(state, 0, 0, state->screen_width, state->screen_height);
  grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  grBufferClear(0, 0, 0);

  // Now set the clip window to just the check_region. (which can be bigger
  // than the conform window).
  vmgrClipWindow(state,
                 state->check_region.min_x,
                 state->check_region.min_y,
                 state->check_region.max_x,
                 state->check_region.max_y);
 
  if (!local_ptr->use_window_coords) {
    grCoordinateSpace(GR_CLIP_COORDS);
    grDepthRange( vnear, vfar );
  }

  vmgrViewport(state, state->conform_x, state->conform_y, 
               state->conform_width, state->conform_height);

  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_ITERATED,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );

  // Always draw with alpha enabled, so we can look for double pixel
  // writing.
  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_ITERATED,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );

  grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                       GR_BLEND_ZERO, GR_BLEND_ZERO );

  // Turn on AA, if applicable.
  if (local_ptr->do_aa) {
    grEnable(GR_AA_ORDERED);
  } else {
    grDisable(GR_AA_ORDERED);
  }

  // Compute the distance between pixels in the clip coordinate space,
  // which is the coordinate space in which we define our primitives.
  // This will be used in init_*_verts().
  if (local_ptr->use_window_coords) {
    local_ptr->dc_scale_x = 1.0f;
    local_ptr->dc_scale_y = 1.0f;
   } else {
    local_ptr->dc_scale_x = 2.0f / state->conform_width;
    local_ptr->dc_scale_y = 2.0f / state->conform_height;
   }

  init_verts(state, local_ptr); 

  // Draw the points
  for (int i = 0; i < local_ptr->num_verts; i++) {
    grDrawPoint(&local_ptr->verts[i]);   
  }
  add_command_data(state, "grDrawPoint()", 0);
  add_vertex_data(state, &local_ptr->verts[0], local_ptr->num_verts);

  if(state->tile) {
    if(state->last_rep) {
      int x,y,width,height;

      // save conform width
      x = state->conform_x;
      y = state->conform_y;
      width = state->conform_width;
      height = state->conform_height;

      // set to full frame
      state->conform_x = 0;
      state->conform_y = 0;
      state->conform_width = state->screen_width;
      state->conform_height = state->screen_height;

      // save tiled frame
      end_frame(END_OP_SAVE_TO_FILE, state, NULL, END_BUFFER_FRONTBUFFER);

      // restore conform width
      state->conform_x = x;
      state->conform_y = y;
      state->conform_width = width;
      state->conform_height = height;
    }
  } else {
    if (!log_status(state, check_frame(state), "point check")) {
      return(TEST_FAIL);
    }
  }

  return(TEST_PASS);
}

void help_conform(conform_state *state)
{
  fprintf(stderr, "   -aa     enable anti-aliasing\n");
  fprintf(stderr, "   -wc     use window coords instead of clip coords\n");
  fprintf(stderr, "   -debug  save debug images\n");
  
}


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
#include "vertex.h"

static char *test_name = "ext-surface";
static char *test_description = "Test surface extensions";

// local data
typedef struct {
  int cur_pass;
} test_data;

char *PassDescriptions[] = {
 "Color Buffers",
 "Depth Buffer",
 "Texture Buffer",
};
#define NUM_PASSES (sizeof(PassDescriptions)/sizeof(char *))

#define RGB_565(_r,_g,_b) \
  (FxU16)(((_r & 0xf8) << 8) | ((_g & 0xfc) << 3) | (_b & 0xfF) >> 3)

// Z Depths
#define SMALLER 100
#define MID     300
#define BIGGER  500

// Colors (rgb888)
#define COLOR_RED   0x00FF0000
#define COLOR_GREEN 0x0000FF00
#define COLOR_BLUE  0x000000FF


/*----------------------------------------------------------------------
@func report_msg
@arg conform_state *state - the frameworks current state
@arg char *s - a string describing what happened
@arg int n - the line number where it happened
@return
@html
Called to make an entry in the log file.
Also prints to stderr. The message includes the line number where
the failure happened.
@end
----------------------------------------------------------------------*/
void report_msg(conform_state* state, char *s, int n) {
  char buf[256];
  sprintf(buf, "%s, @line %d", s, n);
  log_message(state, buf);
  printf("%s\n", buf);
}


/*-----------------------------------------------------------------------
@func draw_box
@arg int size - the size of the box to draw
@arg int x - the x location at which to center the box
@arg int y - the y location at which to center the box
@return
@html
Draw a sizeXsize pixel box centered around a given (x,y)
@end
------------------------------------------------------------------------*/
void draw_box(int size, int x, int y)
{
  GrVertex v[4];
  int offset = size/2;
  
  /*
      0---1
      | . | 
      2---3
   */

  v[0].x = (float)(x - offset);
  v[0].y = (float)(y - offset); 
  v[0].oow = 1.0f;
  v[0].tmuvtx[0].sow = 0.0f;
  v[0].tmuvtx[0].tow = 0.0f;
  v[0].tmuvtx[0].oow = 1.0f;
 
  v[1].x = (float)(x + offset);
  v[1].y = (float)(y - offset);
  v[0].oow = 1.0f;
  v[1].tmuvtx[0].sow = 256.0f;
  v[1].tmuvtx[0].tow = 0.0f;
  v[1].tmuvtx[0].oow = 1.0f;

  v[2].x = (float)(x - offset);
  v[2].y = (float)(y + offset);
  v[0].oow = 1.0f;
  v[2].tmuvtx[0].sow = 0.0f;
  v[2].tmuvtx[0].tow = 256.0f;
  v[2].tmuvtx[0].oow = 1.0f;

  v[3].x = (float)(x + offset);
  v[3].y = (float)(y + offset);
  v[0].oow = 1.0f;
  v[3].tmuvtx[0].sow = 256.0f;
  v[3].tmuvtx[0].tow = 256.0f;
  v[3].tmuvtx[0].oow = 1.0f;

  grDrawTriangle(&v[0], &v[1], &v[2]);
  grDrawTriangle(&v[2], &v[1], &v[3]);
}

/*-------------------------------------------------------------------
@func test_color_buffers
@arg conform_state *state - the framework's state
@return FXTRUE if passed, FXFALSE otherwise
@html
Test to make sure that simple drawing to the render buffer and
blitting it to the front buffer works.
@end
---------------------------------------------------------------------*/
FxBool test_color_buffers(conform_state *state)
{ 
  FxBool status = FXTRUE;
  int i;

  FxU32 test_colors[] = {
    0x00FFFF00, // RGB yellow
    0x00FF0000, // RGB red
    0x00FF00FF, // RGB magenta
    0x0000FF00, // RGB green
    0x0000FFFF, // RGB cyan
    0x000000FF, // RGB blue
  };

  int num_colors = (sizeof(test_colors) / sizeof(FxU32));
 
  int x = state->conform_width/(num_colors+1);
  int y = state->conform_height/2;

  for (i = 0; i < num_colors; i++) {
    grConstantColorValue(test_colors[i]); 
    draw_box(10, x * (i+1), y);
  }

  grFlush();

  // At this point, we should have an RGB triangle on a black
  // background in the back buffer. Blt it to the front.

  blt_surface(state, state->surface_front, state->surface_back);

  // Now the triangle should be on the displayed buffer. Check
  // some pixels to make sure of it.
 
  FxU32 read_color = 0;
   
  for (i = 0; i < num_colors; i++) {
    if (get_surface_pixel(state, 
                          state->surface_front, 
                          GR_COLORFORMAT_ARGB, // format
                          GR_ORIGIN_UPPER_LEFT, // origin
                          x * (i+1), y,
                          &read_color)) {

      if (read_color != test_colors[i]) {
        char buf[256];
        sprintf(buf, 
                "box %d: wrong value read back. Read 0x%08X, expected 0x%08X",
                i, read_color, test_colors[i]);
        report_msg(state, buf, __LINE__);
        status = FXFALSE;
      }
    } else {
      report_msg(state, "couldn't read pixel", __LINE__);
      status = FXFALSE;
    }
  }
  return status;
}


/*-------------------------------------------------------------------
@func test_aux_buffer
@arg conform_state *state - the framework's state
@return FXTRUE if passed, FXFALSE otherwise
@html
Test that the aux buffer extension works (by using it...). 
 1) Draw two overlapping triangles at different depths to make
    sure that it's being used and working.
 2) Read some values from it to make sure it's really the 
    surface extension surface that we handed in.
@end
---------------------------------------------------------------------*/
FxBool test_aux_buffer(conform_state *state)
{ 
  FxBool status = FXTRUE;
  GrVertex v[6];
  FxU32 depth_pix1 = 0; 
  FxU32 depth_pix2 = 0;
  FxU32 color_pix = 0;

  // Add depth to vertex layout
  grVertexLayout(GR_PARAM_Z , GR_VERTEX_Z_OFFSET << 2, GR_PARAM_ENABLE);

  /* Setup two overlapping triangles. 0,1,2 and 3,4,5

     0      4
       3  1
     2      5

   */

  v[0].x = (float)state->conform_x + (state->conform_width * .25f);
  v[0].y = (float)state->conform_y + (state->conform_height * .25f);
  v[0].z = (float)SMALLER;
 
  v[1].x = (float)state->conform_x + (state->conform_width * 0.75f);
  v[1].y = (float)state->conform_y + (state->conform_height * 0.5f);
  v[1].z = (float)SMALLER;

  v[2].x = (float)state->conform_x + (state->conform_width * .25f);
  v[2].y = (float)state->conform_y + (state->conform_height * .75f);
  v[2].z = (float)SMALLER;

  v[3].x = (float)state->conform_x + (state->conform_width * 0.25f);
  v[3].y = (float)state->conform_y + (state->conform_height * 0.5f);
  v[3].z = (float)BIGGER;
 
  v[4].x = (float)state->conform_x + (state->conform_width * .75f);
  v[4].y = (float)state->conform_y + (state->conform_height *.25f);
  v[4].z = (float)BIGGER;

  v[5].x = (float)state->conform_x + (state->conform_width * .75f);
  v[5].y = (float)state->conform_y + (state->conform_height * .75f);
  v[5].z = (float)BIGGER;


  // Clear color and depth buffers to 0
  grDepthMask(FXTRUE);
  grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
  grBufferClear(COLOR_BLUE, 0, 0);
  grDepthBufferFunction(GR_CMP_GREATER); 
  grFlush();

  int cx, cy;
  cx = state->conform_x + (state->conform_width/2);  // center of overlap
  cy = state->conform_y + (state->conform_height/2);  // center of overlap

  // We've cleared the depth buffer to 0, make sure it's that.
  if (!get_surface_pixel(state, 
                    state->surface_aux, 
                    GR_COLORFORMAT_ARGB, // format
                    GR_ORIGIN_UPPER_LEFT, // origin
                    cx, cy, &depth_pix1)) {
    printf("Couldn't get pixel!\n"); 
    report_msg(state, "Couldn't read pixel", __LINE__);
    status = FXFALSE;
  } else {
    // Make sure it's 0
    if (depth_pix1 != 0) {
      report_msg(state, "Depth buffer pixel wasn't 0 after clear to 0",
                        __LINE__);
      status = FXFALSE;
    }
  }

  // Now clear it to a known value. Blue & midrange for our purposes.
  grBufferClear(COLOR_BLUE, 0, MID);
  grFlush();

  // We've cleared the depth buffer to !0, make sure it's that.
  if (!get_surface_pixel(state, 
                    state->surface_aux, 
                    GR_COLORFORMAT_ARGB, // format
                    GR_ORIGIN_UPPER_LEFT, // origin
                    cx, cy, &depth_pix1)) {
    printf("Couldn't get pixel!\n"); 
    report_msg(state, "Couldn't read pixel", __LINE__);
    status = FXFALSE;
  } else {
    // Make sure it's != 0
    if (depth_pix1 == 0) {
      report_msg(state, "Depth buffer pixel still 0 after clear to !0",
                        __LINE__);
      status = FXFALSE;
    }
  }

  // Draw the triangle on the left half of the screen, and blt it to
  // the visible buffer. Our DepthBufferFunction, GREATER, means that 
  // it should NOT have drawn.
  grConstantColorValue(COLOR_GREEN); 
  grDrawTriangle(&v[0], &v[1], &v[2]);
  grFlush();
  blt_surface(state, state->surface_front, state->surface_back);

  // Check that it DID NOT draw. We should still have a blue pixel
  // in the middle.
  if (!get_surface_pixel(state, 
                    state->surface_front, 
                    GR_COLORFORMAT_ARGB, // format
                    GR_ORIGIN_UPPER_LEFT, // origin
                    cx, cy, &color_pix)) {
    report_msg(state, "Couldn't read pixel", __LINE__);
  } else {
    if (color_pix != COLOR_BLUE) {
      report_msg(state, "1st (left) triangle drew when it shouldn't have",
                        __LINE__);
      status = FXFALSE;
    }
  }

  // The aux buffer should not have changed since we didn't draw.
  if (!get_surface_pixel(state, 
                    state->surface_aux, 
                    GR_COLORFORMAT_ARGB, // format
                    GR_ORIGIN_UPPER_LEFT, // origin
                    cx, cy, &depth_pix2)) {
    printf("Couldn't get pixel!\n"); 
    report_msg(state, "Couldn't read pixel", __LINE__);
    status = FXFALSE;
  } else {
    // Make sure it hasn't changed.
    if (depth_pix2 != depth_pix1) {
      report_msg(state, "Depth buffer changed when it shouldn't have",
                        __LINE__);
      status = FXFALSE;
    }
  }
  pause(state);

  // Now draw the right side triangle (red). It should draw. Once again
  // copy the back buffer to the visible buffer, and check the center 
  // pixel. It should be red.
  grConstantColorValue(COLOR_RED); 
  grDrawTriangle(&v[3], &v[4], &v[5]);
  grFlush();
  blt_surface(state, state->surface_front, state->surface_back);

  if (!get_surface_pixel(state, 
                    state->surface_front, 
                    GR_COLORFORMAT_ARGB, // format
                    GR_ORIGIN_UPPER_LEFT, // origin
                    cx, cy, &color_pix)) {
    report_msg(state, "Couldn't read pixel", __LINE__);
  } else {
    if (color_pix != COLOR_RED) {
      report_msg(state, "2nd (right) triangle didn't draw when it should have",
                        __LINE__);
      status = FXFALSE;
    }
  }

  // The aux buffer should have changed when we drew the red triangle.
  if (!get_surface_pixel(state, 
                    state->surface_aux, 
                    GR_COLORFORMAT_ARGB, // format
                    GR_ORIGIN_UPPER_LEFT, // origin
                    cx, cy, &depth_pix2)) {
    printf("Couldn't get pixel!\n"); 
    report_msg(state, "Couldn't read pixel", __LINE__);
    status = FXFALSE;
  } else {
    // Make sure it hasn't changed.
    if (depth_pix2 == depth_pix1) {
      report_msg(state, "Depth buffer didn't change when it should have",
                        __LINE__);
      status = FXFALSE;
    }
  }


  // Disable depth buffering.
  grBufferClear(0, 0, 0);
  grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
  grDepthMask(FXFALSE);

  return status;
}

/*-------------------------------------------------------------------
@func test_texture_buffer
@arg conform_state *state - the framework's state
@return FXTRUE if passed, FXFALSE otherwise
@html
Test that the buffer extensions work. Right now it only tests a
single TMU with a single texture surface.
1) create a direct draw texture surface for a simple (256x256) texture.
2) lock the DD surface, fill it with a color, unlock
3) draw a box, using it as the source texture
4) goto 2, for a few boxes
5) blt to the front dd surface
6) read some pixels back from the front dd surface, making
   sure that they're the expected colors
@end
---------------------------------------------------------------------*/
FxBool test_texture_buffer(conform_state *state)
{ 
  FxBool status = FXTRUE;
  int i;

  GrTexInfo texInfo;

  // Get the dimensions we need. One LOD. 256x256
  texInfo.smallLodLog2 = GR_LOD_LOG2_256;
  texInfo.largeLodLog2 = GR_LOD_LOG2_256;
  texInfo.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
  texInfo.format = GR_TEXFMT_RGB_565;
  texInfo.data = NULL;

  status = create_texture_surface(state, GR_TMU0, &texInfo);

  if (status) { 

    grTexSource(GR_TMU0, 0x00UL,
                GR_MIPMAPLEVELMASK_BOTH, 
                &texInfo);

    grTexCombine(GR_TMU0,
               GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
               GR_COMBINE_FUNCTION_NONE, GR_COMBINE_FACTOR_NONE,
               FXFALSE, FXFALSE );

    grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                   GR_COMBINE_FACTOR_ONE,
                   GR_COMBINE_LOCAL_NONE,
                   GR_COMBINE_OTHER_TEXTURE,
                   FXFALSE );

    grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

    grTexFilterMode(GR_TMU0,
                    GR_TEXTUREFILTER_POINT_SAMPLED,
                    GR_TEXTUREFILTER_POINT_SAMPLED );

    grTexMipMapMode(GR_TMU0,
                    GR_MIPMAP_NEAREST,
                    FXFALSE);

    grTexLodBiasValue(GR_TMU0, 0.0f);

 
    // Add texture coords to the vertex layout 
    grVertexLayout(GR_PARAM_ST0, GR_VERTEX_SOW_TMU0_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_Q0, GR_VERTEX_OOW_TMU0_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_Q , GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);

    // draw 3 boxes, each with a random colored texture

    FxU16 test_colors[] = {
       RGB_565(0, 255, 255),  
       RGB_565(255, 0, 255),  
       RGB_565(0, 0, 255), 
       RGB_565(255, 255, 0), 
       RGB_565(255, 0, 0), 
       RGB_565(0, 255, 0), 
    };

    int num_colors = sizeof(test_colors) / sizeof(FxU16);
    int xloc = state->conform_width/(num_colors + 1);
    int yloc = state->conform_height/2;

    grBufferClear(0, 0, 0);

    for (i = 0; i < num_colors; i++) {
      // pick a random color
      //test_color[i] = RGB_565(nrand(0xff), nrand(0xff), nrand(0xff));

      void *ptr = lock_surface(state, state->surface_textures[GR_TMU0]);

      if (ptr) {
        // we know it's 256x256, 16bits deep
        int x, y;
        FxU16 *pix = (FxU16 *)ptr;
        for (y = 0; y < 256; y++) {
          for (x = 0; x < 256; x++) {
            *pix++ = test_colors[i];
          }
        }
        unlock_surface(state, state->surface_textures[GR_TMU0], ptr);
        draw_box(10, xloc * (i+1), yloc);
        //grFlush();
        grFinish(); // use grFinish() to make sure rendering has completed
      }
    }

    grFlush();
    blt_surface(state, state->surface_front, state->surface_back);

    // The surface should now be visible. Test that the three boxes
    // were drawn with the proper colors.

    for (i = 0; i < num_colors; i++) {
      FxU32 read_color = 0;
      if (get_surface_pixel(state, 
                            state->surface_front, 
                            GR_COLORFORMAT_ARGB, // format
                            GR_ORIGIN_UPPER_LEFT, // origin
                            xloc * (i+1), yloc,
                            &read_color)) {

        // test_colors[] is in RGB_565 format. 
        // read_color is in Windows COLORREF format, which is just 0x00RRGGBB

        // convert the read color into our truncated 565 form for comparison                      
        FxU32 munged_color = RGB_565(((read_color >> 16) & 0xff),
                                     ((read_color >> 8) & 0xff),
                                     (read_color & 0xff));

        if (munged_color != test_colors[i]) {
          char buf[256];
          sprintf(buf, 
                  "box %d: wrong value read back. Read 0x%08X, expected 0x%08X",
                  i, munged_color, test_colors[i]);
          report_msg(state, buf, __LINE__);
          status = FXFALSE;
        } 
      } else {
        report_msg(state, "Couldn't read pixel", __LINE__);
        status = FXFALSE;
      }
    }
  } else {
    report_msg(state, "create_texture_surfaces() failed.", __LINE__);
    status = FXFALSE;
  }

  return status;
}

/**
 ** Framework entrypoints
 **/

int init_conform(conform_state *state, int local_argc, char **local_argv)
{
  test_data *local_ptr;

  state->test_name = test_name;
  state->test_description = test_description;

  if((state->local_data = malloc(sizeof(test_data))) == NULL) {
    // Could not allocate data for local memory
    log_perror(state,"Could not allocate memory for local data");
    return(0);
  } 

  local_ptr = (test_data *)state->local_data;
  local_ptr->cur_pass = 0;

  state->num_aux_bufs = 1; // need a depth buffer

  // tiling not supported
  state->tile = 0;

  // xycheck and xylimit not supported (fix this when adding self-checking)
  state->xycheck = 0;
  state->xylimit = 0;

  // use the surface extentions
  state->use_surface_extensions = 1;

  state->num_reps = NUM_PASSES;

  // setup image file names
  // this is manditory
  frame_name(test_name, state);
  img_file_type(IMG_P6,state);

  // Initialization complete
  return(-1);
}

void close_conform(conform_state *state)
{
  // Free up local data
  free(state->local_data);
}

int do_conform(conform_state *state)
{
  int status = -1;

  test_data *local_ptr = (test_data *)state->local_data;

  grCoordinateSpace(GR_WINDOW_COORDS);

  grViewport(state->conform_x, state->conform_y, 
             state->conform_width, state->conform_height);
  grClipWindow(state->conform_x,    
               state->conform_y, 
               state->conform_x + state->conform_width, 
               state->conform_y + state->conform_height);

  grReset(GR_VERTEX_PARAMETER);
  grVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE); 

  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_CONSTANT,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE );

  grBufferClear(0, 0, 0); // clear to black

  switch (local_ptr->cur_pass) {
    default:
    case 0:  // color buffers
      if (test_color_buffers(state)) {
        report_msg(state, "PASS: render/display buffer extension test", __LINE__);
      } else {
        report_msg(state, "FAIL: render/display buffer extension test", __LINE__);
      }
      break;

    case 1: // aux buffer
      if (test_aux_buffer(state)) {
        report_msg(state, "PASS: aux buffer extension test", __LINE__);
      } else {
        report_msg(state, "FAIL: aux buffer extension test", __LINE__);
      }
      break;

    case 2: // texture buffer
      if (test_texture_buffer(state)) {
        report_msg(state, "PASS: texture buffer extension test", __LINE__);
      } else {
        report_msg(state, "FAIL: texture buffer extension test", __LINE__);
      }
      break;
  }


  pause(state); // will pause if in interactive mode

  local_ptr->cur_pass++;

  return(status);
}

void help_conform(conform_state *state)
{
  fprintf(stderr,"   -tmu <n>   texture unit to test\n");
}

